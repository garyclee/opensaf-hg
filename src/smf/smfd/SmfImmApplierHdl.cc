/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Ericsson AB
 *
 */

// TODO(elunlen): Include directory for .h files when new directory structure
// is in place
#include "smf/smfd/SmfImmApplierHdl.h"
#include <string>
#include <atomic>
// TODO(Lennart): Replace with base::mutex when available
#include <mutex>

#include "smf/smfd/smfd.h"
#include "smf/smfd/SmfUtils.h"

#include "base/saf_error.h"
#include "base/logtrace.h"
#include "base/osaf_utility.h"
#include "imm/saf/saImmOi.h"
#include "osaf/immutil/immutil.h"
#include "base/time.h"
#include "base/osaf_poll.h"
#include "base/osaf_extended_name.h"

/**
 * Locals not needed outside of this file
 */
// Must be delclared here since it is used both in C callback functions and in
// SmfImmApplierHdl class
static std::string object_name_ = "";
static std::string attribute_name_ = "";
static std::string attribute_value_ = "";
static bool attribute_value_is_valid_ = false;

// Used to abort ongoing handler activity in case of Stop command
// For example ongoing TRY_AGAIN handling in IMM APIs must be terminated
static std::atomic<bool> is_cancel{false};

// Mutex protecting attribute value
// TODO(elunlen): Replace std::mutex with base::mutex when base::mutex is
// available
std::mutex attribute_value_lock;

/**
 * Local functions containing IMM operations needed here
 **/

/**
 * Update the monitored value protected
 * Note: Is not a method in SmfImmApplierHdl class since used by both
 * C functions and in SmfImmApplierHdl class
 */
static void SetAttributeValue(const std::string &value) {
  attribute_value_lock.lock();
  attribute_value_ = value;
  attribute_value_lock.unlock();
}

/**
 * Read the value of the object and attribute
 * Note1: This function uses an SMF specific IMM wrapper
 * Note2: This may be replaced with a generic IMM wrapper when one has
 * been implemented.
 * Note3: This function can only read one attribute of type SaUint32T
 *
 */
static std::string GetAttributeValueFromImm(
    const std::string &i_object_name, const std::string &i_attribute_name) {
  SaImmAttrValuesT_2 **attributes;
  SmfImmUtils ImmCfg;

  TRACE_ENTER();
  if (i_object_name == "" || i_attribute_name == "") {
    TRACE_LEAVE2("No object or attribute");
    return "";
  }

  if (ImmCfg.getObject(i_object_name, &attributes) == false) {
    LOG_WA("%s: Could not read IMM config object from IMM %s", __FUNCTION__,
           IMM_CONFIG_OBJECT_DN);
  }

  const SaUint32T *attribute_value = immutil_getUint32Attr(
      (const SaImmAttrValuesT_2 **)attributes, i_attribute_name.c_str(), 0);

  std::string long_dn_setting_as_string = "";
  if (attribute_value != nullptr) {
    long_dn_setting_as_string = std::to_string(*attribute_value);
  } else {
    LOG_NO("%s: Fail longDnsAllowed has no value", __FUNCTION__);
  }

  TRACE_LEAVE();
  return long_dn_setting_as_string;
}

/**
 * IMM API wrapper functions used instead of immutil.c
 * All return false if Fail and log errors
 * Note: To initialize IMM OI may take up to 60 seconds in a TRY AGAIN loop.
 *       In this loop the is_cancel flag is checked and if true the TRY AGAIN
 *       loop is canceled meaning that the applier is still not initialized
 *
 * TODO Replace with generic functions when implemented
 */

static bool InitializeImmOi(SaImmOiHandleT *o_immOiHandle,
                            const SaImmOiCallbacksT_2 *i_immOiCallbacks,
                            const SaVersionT *i_version) {
  // It may take up to one minute to initialize if IMM is synchronizing
  base::Timer adminOpTimer(60000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  bool rc = true;

  SaVersionT version = *i_version;
  while (adminOpTimer.is_timeout() == false) {
    ais_rc = saImmOiInitialize_2(o_immOiHandle, i_immOiCallbacks, &version);
    if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
      base::Sleep(base::kFiveHundredMilliseconds);
      if (is_cancel) {
        is_cancel = false;
        rc = false;
        break;
      }
    } else if (ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiInitialize_2() Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
      rc = false;
      break;
    } else {
      // Done
      break;
    }
  }
  if (adminOpTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
    LOG_WA("%s: saImmOiInitialize_2() timeout Fail '%s'", __FUNCTION__,
           saf_error(ais_rc));
  }

  return rc;
}

static bool FinalizeImmOi(SaImmOiHandleT i_immOiHandle) {
  base::Timer adminOpTimer(20000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  bool rc = true;

  if (i_immOiHandle != 0) {
    // Only if there is anything to finalize
    while (adminOpTimer.is_timeout() == false) {
      ais_rc = saImmOiFinalize(i_immOiHandle);
      if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
        base::Sleep(base::kFiveHundredMilliseconds);
      } else if (ais_rc != SA_AIS_OK) {
        LOG_WA("%s: saImmOiFinalize() Fail '%s'", __FUNCTION__,
               saf_error(ais_rc));
        rc = false;
        break;
      } else {
        // Done
        break;
      }
    }
    if (adminOpTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiFinalize() timeout Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
    }
  }

  return rc;
}

static bool GetImmOiSelectionObject(SaImmOiHandleT i_immOiHandle,
                                    SaSelectionObjectT *selectionObject) {
  base::Timer adminOpTimer(20000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  bool rc = true;
  TRACE_ENTER();

  while (adminOpTimer.is_timeout() == false) {
    ais_rc = saImmOiSelectionObjectGet(i_immOiHandle, selectionObject);
    if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
      base::Sleep(base::kFiveHundredMilliseconds);
    } else if (ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiSelectionObjectGet() Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
      rc = false;
      break;
    } else {
      // Done
      break;
    }
  }
  if (adminOpTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
    LOG_WA("%s: saImmOiSelectionObjectGet() timeout Fail '%s'", __FUNCTION__,
           saf_error(ais_rc));
  }

  TRACE_LEAVE();
  return rc;
}

static bool ImmOiSet(SaImmOiHandleT i_immOiHandle,
                     const std::string &i_implementerName) {
  base::Timer adminOpTimer(20000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  bool rc = true;
  SaImmOiImplementerNameT implementerName =
      const_cast<SaImmOiImplementerNameT>(i_implementerName.c_str());
  TRACE_ENTER();

  while (adminOpTimer.is_timeout() == false) {
    ais_rc = saImmOiImplementerSet(i_immOiHandle, implementerName);
    if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
      base::Sleep(base::kFiveHundredMilliseconds);
    } else if (ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiImplementerSet() Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
      rc = false;
      break;
    } else {
      // Done
      break;
    }
  }
  if (adminOpTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
    LOG_WA("%s: saImmOiImplementerSet() timeout Fail '%s'", __FUNCTION__,
           saf_error(ais_rc));
  }

  TRACE_LEAVE();
  return rc;
}

static bool ImmOiClear(SaImmOiHandleT i_immOiHandle) {
  base::Timer adminOpTimer(20000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  bool rc = true;

  while (adminOpTimer.is_timeout() == false) {
    ais_rc = saImmOiImplementerClear(i_immOiHandle);
    if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
      base::Sleep(base::kFiveHundredMilliseconds);
    } else if (ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiImplementerClear() Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
      rc = false;
      break;
    } else {
      // Done
      break;
    }
  }
  if (adminOpTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
    LOG_WA("%s: saImmOiImplementerClear() timeout Fail '%s'", __FUNCTION__,
           saf_error(ais_rc));
  }

  return rc;
}

/**
 * Become applier for the given object. This starts the applier
 */
static bool SetImmOiForObject(SaImmOiHandleT i_immOiHandle,
                              const std::string &i_objectName,
                              SaImmScopeT i_scope) {
  base::Timer adminOpTimer(20000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  bool rc = true;
  SaNameT object_name;
  osaf_extended_name_lend(i_objectName.c_str(), &object_name);

  while (adminOpTimer.is_timeout() == false) {
    ais_rc = saImmOiObjectImplementerSet(i_immOiHandle, &object_name, i_scope);
    if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
      base::Sleep(base::kFiveHundredMilliseconds);
    } else if (ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiObjectImplementerSet() Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
      rc = false;
      break;
    } else {
      // Done
      break;
    }
  }
  if (adminOpTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
    LOG_WA("%s: saImmOiObjectImplementerSet() timeout Fail '%s'", __FUNCTION__,
           saf_error(ais_rc));
  }

  return rc;
}

/**
 * Release the applier for the given object. This stops the applier
 */
static bool ReleaseImmOiForObject(SaImmOiHandleT i_immOiHandle,
                                  const std::string &i_objectName,
                                  SaImmScopeT i_scope) {
  base::Timer adminOpTimer(20000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  bool rc = true;
  SaNameT object_name;
  osaf_extended_name_lend(i_objectName.c_str(), &object_name);

  while (adminOpTimer.is_timeout() == false) {
    ais_rc =
        saImmOiObjectImplementerRelease(i_immOiHandle, &object_name, i_scope);
    if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
      base::Sleep(base::kFiveHundredMilliseconds);
    } else if (ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiObjectImplementerRelease() Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
      rc = false;
      break;
    } else {
      // Done
      break;
    }
  }
  if (adminOpTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
    LOG_WA("%s: saImmOiObjectImplementerRelease() timeout Fail '%s'",
           __FUNCTION__, saf_error(ais_rc));
  }

  return rc;
}

static SaAisErrorT DispatchOiCallback(SaImmOiHandleT i_immOiHandle,
                                      SaDispatchFlagsT i_dispatchFlags) {
  base::Timer dispatchTimer(20000);
  SaAisErrorT ais_rc = SA_AIS_OK;
  TRACE_ENTER();

  while (dispatchTimer.is_timeout() == false) {
    ais_rc = saImmOiDispatch(i_immOiHandle, i_dispatchFlags);

    if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
      base::Sleep(base::kFiveHundredMilliseconds);
    } else if (ais_rc != SA_AIS_OK) {
      LOG_WA("%s: saImmOiDispatch() Fail '%s'", __FUNCTION__,
             saf_error(ais_rc));
      break;
    } else {
      // Done
      break;
    }
  }
  if (dispatchTimer.is_timeout() == true && ais_rc != SA_AIS_OK) {
    LOG_WA("%s: saImmOiDispatch() timeout Fail '%s'", __FUNCTION__,
           saf_error(ais_rc));
  }
  TRACE_LEAVE();
  return ais_rc;
}

/**
 * The applier callback functions
 * These are C functions as specified by IMM AIS SaImmOiCallbacksT_2
 * Note: CCB is "collected" using existing list handling in immutil.c
 **/

/**
 * Save ccb info if OpensafConfig object is modified
 *
 */
static SaAisErrorT CcbObjectModifyCallback(
    SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId, const SaNameT *objectName,
    const SaImmAttrModificationT_2 **attrMods) {
  SaAisErrorT rc = SA_AIS_OK;
  struct CcbUtilCcbData *ccbUtilCcbData;

  TRACE_ENTER();

  if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == nullptr) {
    if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == nullptr) {
      TRACE("%s: Failed to get CCB object for %llu", __FUNCTION__, ccbId);
      rc = SA_AIS_ERR_NO_MEMORY;
      goto done;
    }
  }

  ccbutil_ccbAddModifyOperation(ccbUtilCcbData, objectName, attrMods);

done:
  TRACE_LEAVE();
  return rc;
}

/**
 * Get the new value of the attribute
 * Note: This function is limited to handle one attribute in one object and
 * the type of the attribute must be SaUint32T
 */
static void CcbApplyCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId) {
  struct CcbUtilCcbData *ccbUtilCcbData;
  struct CcbUtilOperationData *opdata;
  const SaImmAttrModificationT_2 *attrMod;
  SaConstStringT objName;
  SaImmAttrValuesT_2 attribute;

  // Note: Only one value of type SaUint32T can be handled
  SaUint32T *value = nullptr;

  TRACE_ENTER();

  /* Verify the ccb */
  if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == nullptr) {
    LOG_NO("%s: Failed to find CCB object for id %llu", __FUNCTION__, ccbId);
    goto done;
  }

  /* We don't need to loop since the only possible operation is MODIFY
   * and only one object is monitored
   */
  opdata = ccbUtilCcbData->operationListHead;
  if (opdata->operationType != CCBUTIL_MODIFY) {
    TRACE("%s: Not a modify operation", __FUNCTION__);
    goto done;
  }

  objName = osaf_extended_name_borrow(&opdata->objectName);
  if (object_name_.compare(objName) != 0) {
    LOG_NO("%s: Object \"%s\" wrong object", __FUNCTION__, objName);
    goto done;
  }

  /* Read value
   * Note: Only one attribute is handled
   */
  TRACE("%s: Read value in attributes", __FUNCTION__);
  attrMod = opdata->param.modify.attrMods[0];
  attribute = attrMod->modAttr;

  for (int i = 1; attrMod != nullptr; i++) {
    /* Get the value */
    if (attribute_name_.compare(attribute.attrName) != 0) {
      // Not found
      attrMod = opdata->param.modify.attrMods[i];
      attribute = attrMod->modAttr;
      continue;
    }

    // Attribute found
    value = static_cast<SaUint32T *>(attribute.attrValues[0]);
    TRACE("Attribute found: attrName '%s', value = %d",
          attribute.attrName, *value);
    break;
  }

  if (attrMod != nullptr) {
    if (value == nullptr) {
      TRACE("%s: Value is nullptr", __FUNCTION__);
      SetAttributeValue("");
      attribute_value_is_valid_ = false;
    } else {
      SetAttributeValue(std::to_string(*value));
      attribute_value_is_valid_ = true;
    }
  }

done:
  if (ccbUtilCcbData != nullptr) ccbutil_deleteCcbData(ccbUtilCcbData);

  TRACE_LEAVE();
}

/**
 * Cleanup by deleting saved ccb info if aborted
 *
 */
static void CcbAbortCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId) {
  struct CcbUtilCcbData *ccbUtilCcbData;

  TRACE_ENTER2("CCB ID %llu", ccbId);

  if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) != nullptr)
    ccbutil_deleteCcbData(ccbUtilCcbData);
  else
    TRACE("%s: Failed to find CCB object for %llu", __FUNCTION__, ccbId);

  TRACE_LEAVE();
}

/**
 * An object of OpensafConfig class is created. We don't have to do anything
 * Cannot be replaced by nullptr pointer in callback list
 */
static SaAisErrorT CcbCreateCallback(SaImmOiHandleT immOiHandle,
                                     SaImmOiCcbIdT ccbId,
                                     const SaImmClassNameT className,
                                     const SaNameT *parentName,
                                     const SaImmAttrValuesT_2 **attr) {
  TRACE_ENTER();
  TRACE_LEAVE();
  return SA_AIS_OK;
}

/**
 * An object of OpensafConfig class is deleted. We don't have to do anything
 * Cannot be replaced by nullptr pointer in callback list
 */
static SaAisErrorT CcbDeleteCallback(SaImmOiHandleT immOiHandle,
                                     SaImmOiCcbIdT ccbId,
                                     const SaNameT *objectName) {
  TRACE_ENTER();
  TRACE_LEAVE();
  return SA_AIS_OK;
}

/* Callback function list given to IMM when OI is initialized
 * We need:
 * Modify for saving modify ccb
 * Apply for knowing that the modification has been applied
 * Abort for removing saved ccb in case of an abortion of ccb
 */
static const SaImmOiCallbacksT_2 callbacks = {
    .saImmOiAdminOperationCallback = nullptr,
    .saImmOiCcbAbortCallback = CcbAbortCallback,
    .saImmOiCcbApplyCallback = CcbApplyCallback,
    .saImmOiCcbCompletedCallback = nullptr,
    .saImmOiCcbObjectCreateCallback = CcbCreateCallback,
    .saImmOiCcbObjectDeleteCallback = CcbDeleteCallback,
    .saImmOiCcbObjectModifyCallback = CcbObjectModifyCallback,
    .saImmOiRtAttrUpdateCallback = nullptr};

// =====================================
// SmfImmApplierHdl Class implementation
// =====================================
// Make next_instance_number_ thread safe
std::atomic<unsigned int> SmfImmApplierHdl::instance_number_{1};
SmfImmApplierHdl::SmfImmApplierHdl()
    : isCreated_{false}, imm_appl_hdl_{0}, imm_appl_selobj_{0} {
  applier_name_ = "@safSmf_applier" + std::to_string(instance_number_++);
}

SmfImmApplierHdl::~SmfImmApplierHdl() {
  TRACE_ENTER();
  Remove();
  TRACE_LEAVE();
}

void SmfImmApplierHdl::Remove() {
  // Release all resources, give up applier, remove applier name and finalize OI
  if (Stop() == false) {
    LOG_NO("%s: ReleaseImmOiForObject Fail", __FUNCTION__);
  }
  if (ImmOiClear(imm_appl_hdl_) == false) {
    LOG_NO("%s: ImmOiClear() Fail", __FUNCTION__);
  }
  if (FinalizeImmOi(imm_appl_hdl_) == false) {
    LOG_NO("%s: FinalizeImmOi() Fail", __FUNCTION__);
  }
}

bool SmfImmApplierHdl::Create(const ApplierSetupInfo &setup_info) {
  bool do_continue = true;
  TRACE_ENTER();

  is_cancel = false;
  if (imm_appl_hdl_ == 0) {
    // If initialize is needed
    TRACE("%s: Do create", __FUNCTION__);
    object_name_ = setup_info.object_name;
    attribute_name_ = setup_info.attribute_name;
    attribute_value_is_valid_ = false;

    // Initialize
    do_continue = InitializeImmOi(&imm_appl_hdl_, &callbacks, &kImmVersion);
    TRACE("%s: InitializeImmOi() = %s", __FUNCTION__,
          do_continue ? "true" : "false");

    // Get selection object
    if (do_continue) {
      do_continue = GetImmOiSelectionObject(imm_appl_hdl_, &imm_appl_selobj_);
      TRACE("%s: GetImmOiSelectionObject() = %s", __FUNCTION__,
            do_continue ? "true" : "false");
    }
  }

  TRACE_LEAVE();
  return do_continue;
}

bool SmfImmApplierHdl::Start() {
  std::string value;
  bool do_continue = true;

  TRACE_ENTER();
  is_cancel = false;

  // OI Set
  // Note: If the applier name already exist it is because SMF on another
  // node has created an applier with the same name. This not ok for an
  // object implementer but is ok for an applier.
  // Create a new name and try again. Limit number of retries
  for (int i = 0; i < 3; i++) {
    do_continue = ImmOiSet(imm_appl_hdl_, applier_name_);

    if (do_continue) {
      break;
    } else {
      // Create a new name and try again
      applier_name_ = "@safSmf_applier" + std::to_string(instance_number_++);
      LOG_NO("%s: A new applier name is created '%s'", __FUNCTION__,
             applier_name_.c_str());
    }
  }

  // Set which object to monitor
  if (do_continue) {
    do_continue = SetImmOiForObject(imm_appl_hdl_, object_name_, SA_IMM_ONE);
    TRACE("%s: SetImmOiForObject() = %s", __FUNCTION__,
          do_continue ? "true" : "false");

    if (do_continue) {
      if (attribute_value_is_valid_ == false) {
        // Get the current value from IMM
        value = GetAttributeValueFromImm(object_name_, attribute_name_);
        TRACE("%s: GetAttributeValueFromImm() = '%s'", __FUNCTION__,
              value.c_str());
      }
      if (value != "") {
        attribute_value_is_valid_ = true;
      }
      SetAttributeValue(value);
      TRACE(
          "%s: SetAttributeValue(%s), "
          "attribute_value_is_valid_ = %s",
          __FUNCTION__, value.c_str(),
          attribute_value_is_valid_ ? "true" : "false");
    }
  }

  if (do_continue == false) {
    LOG_WA("%s: Applier could not be started", __FUNCTION__);
  }

  TRACE_LEAVE();
  return do_continue;
}

bool SmfImmApplierHdl::Stop() {
  bool rc = true;

  TRACE_ENTER();

  is_cancel = true;
  attribute_value_is_valid_ = false;
  SetAttributeValue("");

  if (imm_appl_hdl_ != 0) {
    rc = ReleaseImmOiForObject(imm_appl_hdl_, object_name_, SA_IMM_ONE);
    if (rc == false) {
      LOG_NO("%s: ReleaseImmOiForObject() Fail", __FUNCTION__);
    }

    rc = ImmOiClear(imm_appl_hdl_);
    if (rc == false) {
      LOG_NO("%s: ImmOiClear Fail", __FUNCTION__);
    }
  }
  TRACE_LEAVE2("rc = %s", rc ? "true" : "false");
  return rc;
}

void SmfImmApplierHdl::CancelCreate() { is_cancel = true; }

std::string SmfImmApplierHdl::get_value() {
  attribute_value_lock.lock();
  std::string value = attribute_value_;
  attribute_value_lock.unlock();
  return value;
}

ExecuteImmCallbackReply SmfImmApplierHdl::ExecuteImmCallback() {
  ExecuteImmCallbackReply method_rc = ExecuteImmCallbackReply::kOk;

  TRACE_ENTER();
  SaAisErrorT ais_rc = DispatchOiCallback(imm_appl_hdl_, SA_DISPATCH_ALL);

  if (ais_rc == SA_AIS_OK) {
    method_rc = ExecuteImmCallbackReply::kOk;
  } else if (ais_rc == SA_AIS_ERR_BAD_HANDLE) {
    struct ApplierSetupInfo setup;
    setup.object_name = object_name_;
    setup.attribute_name = attribute_name_;
    imm_appl_hdl_ = 0;
    bool is_ok = Create(setup);
    if (is_ok) {
      method_rc = ExecuteImmCallbackReply::kGetNewSelctionObject;
    } else {
      method_rc = ExecuteImmCallbackReply::kError;
    }
  } else {
    method_rc = ExecuteImmCallbackReply::kError;
  }

  TRACE_LEAVE();
  return method_rc;
}
