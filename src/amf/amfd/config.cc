/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 * Copyright Ericsson AB 2017 - All Rights Reserved.
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

#include "amf/amfd/util.h"
#include "amf/common/amf_util.h"
#include "amf/amfd/imm.h"
#include "amf/amfd/node.h"
#include "amf/amfd/config.h"

static Configuration _configuration;
Configuration *configuration = &_configuration;

static void ccb_apply_modify_hdlr(struct CcbUtilOperationData *opdata) {
  TRACE_ENTER();
  const SaImmAttrModificationT_2 *attr_mod;
  int i = 0;

  while ((attr_mod = opdata->param.modify.attrMods[i++]) != nullptr) {
    if (!strcmp(attr_mod->modAttr.attrName, "osafAmfRestrictAutoRepairEnable")) {
      bool enabled = false; // set to disabled if attribute is blank
      if (attr_mod->modType != SA_IMM_ATTR_VALUES_DELETE &&
        attr_mod->modAttr.attrValues != nullptr) {
        enabled =
          static_cast<bool>(*((SaUint32T *)attr_mod->modAttr.attrValues[0]));
      }
      TRACE("osafAmfRestrictAutoRepairEnable changed to '%d'", enabled);
      configuration->restrict_auto_repair(enabled);
    }
  }
  TRACE_LEAVE();
}

static SaAisErrorT ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata) {
  TRACE_ENTER();
  SaAisErrorT rc = SA_AIS_OK;
  TRACE_LEAVE();
  return rc;
}

static SaAisErrorT configuration_ccb_completed_cb(
  CcbUtilOperationData_t *opdata) {
  SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

  TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId,
               osaf_extended_name_borrow(&opdata->objectName));

  switch (opdata->operationType) {
    case CCBUTIL_CREATE:
      report_ccb_validation_error(opdata,
                                  "OpenSafAmfConfig objects cannot be created");
      goto done;
      break;
    case CCBUTIL_MODIFY:
      rc = ccb_completed_modify_hdlr(opdata);
      break;
    case CCBUTIL_DELETE:
      report_ccb_validation_error(opdata,
                                  "OpenSafAmfConfig objects cannot be deleted");
      goto done;
      break;
    default:
      osafassert(0);
      break;
  }

done:
  TRACE_LEAVE();
  return rc;
}

static void configuration_ccb_apply_cb(CcbUtilOperationData_t *opdata) {
  TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId,
               osaf_extended_name_borrow(&opdata->objectName));

  switch (opdata->operationType) {
    case CCBUTIL_MODIFY:
      ccb_apply_modify_hdlr(opdata);
      break;
    case CCBUTIL_CREATE:
    case CCBUTIL_DELETE:
    /* fall through */
    default:
      osafassert(0);
      break;
  }
  TRACE_LEAVE();
}

static void configuration_admin_op_cb(SaImmOiHandleT immOiHandle,
    SaInvocationT invocation,
    const SaNameT *object_name,
    SaImmAdminOperationIdT op_id,
    const SaImmAdminOperationParamsT_2 **params) {
  TRACE_ENTER();
  switch (op_id) {
    default:
      report_admin_op_error(immOiHandle, invocation, SA_AIS_ERR_NOT_SUPPORTED,
                            nullptr, "Not supported");
      break;
  }
  TRACE_LEAVE();
}

/**
 * Get configuration for the AMF configuration object from IMM
 */
SaAisErrorT Configuration::get_config(void) {
  SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
  SaImmSearchHandleT searchHandle;
  SaImmSearchParametersT_2 searchParam;
  SaNameT dn;
  const SaImmAttrValuesT_2 **attributes;
  const char *className = "OpenSafAmfConfig";

  TRACE_ENTER();

  searchParam.searchOneAttr.attrName =
      const_cast<SaImmAttrNameT>("SaImmAttrClassName");
  searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
  searchParam.searchOneAttr.attrValue = &className;

  if (immutil_saImmOmSearchInitialize_2(
          avd_cb->immOmHandle, nullptr, SA_IMM_SUBTREE,
          SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
          nullptr, &searchHandle) != SA_AIS_OK) {
    LOG_WA("saImmOmSearchInitialize_2 failed: %u", error);
    goto done1;
  }

  while (immutil_saImmOmSearchNext_2(searchHandle, &dn,
    (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
    uint32_t value;
    TRACE("reading configuration '%s'", osaf_extended_name_borrow(&dn));
    if (immutil_getAttr("osafAmfRestrictAutoRepairEnable", attributes, 0, &value)
      == SA_AIS_OK) {
      configuration->restrict_auto_repair(static_cast<bool>(value));
    }
  }

  error = SA_AIS_OK;
  TRACE("osafAmfRestrictAutoRepairEnable set to '%d'",
    restrict_auto_repair_enabled());

  (void)immutil_saImmOmSearchFinalize(searchHandle);
done1:
  TRACE_LEAVE();
  return error;
}

Configuration::Configuration() : restrict_auto_repair_(false) // default
{
  avd_class_impl_set("OpenSafAmfConfig", nullptr, configuration_admin_op_cb,
    configuration_ccb_completed_cb, configuration_ccb_apply_cb);
}

Configuration::~Configuration()
{
}

void Configuration::restrict_auto_repair(bool enable)
{
  TRACE_ENTER();
  restrict_auto_repair_ = enable;

  // notify AMFNDs of this ...
  for (const auto &value : *node_name_db) {
    AVD_AVND *avnd = value.second;
    osafassert(avnd != nullptr);
    for (const auto &su : avnd->list_of_ncs_su) {
        su->set_su_maintenance_campaign();
    }
    for (const auto &su : avnd->list_of_su) {
        su->set_su_maintenance_campaign();
    }
  }
  TRACE_LEAVE();
}

bool Configuration::restrict_auto_repair_enabled()
{
  return restrict_auto_repair_;
}
