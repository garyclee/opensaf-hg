/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
 *
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

#ifndef SMF_SMFD_SMFIMMAPPLIERHDL_H_
#define SMF_SMFD_SMFIMMAPPLIERHDL_H_

#include <string>
#include <atomic>

#include "base/macros.h"
#include "imm/saf/saImmOi.h"

/**
 * This class creates and handle an IMM applier
 * The handler must be used in an "outer" thread that implements a poll loop
 * The thread shall get a selection object (poll fd) and call the
 * ExecuteImmCallback() method when released
 *
 * Note/Warning:
 * The IMM object applier handle may be set to invalid by IMM. This will be
 * detected and handled in the ExecuteImmCallback() method. This method will
 * not return until the applier is restored or timed out which may take up to
 * one minute.
 * Do not use this handler in a thread that cannot handle this!
 *
 * In this version the following limitations apply:
 * - Only one IMM object can be monitored
 * - Only modifications of one attribute can be handled
 * - All values are given as a std::string. The user has to convert to other
 *   type if needed
 */

// In-parameter for SmfImmApplierHdl Create())
struct ApplierSetupInfo {
  std::string object_name;
  std::string attribute_name;
  // SaImmOiCcbApplyCallbackT ApplyCallbackFunction;
};

enum class ExecuteImmCallbackReply { kOk = 0, kGetNewSelctionObject, kError };

class SmfImmApplierHdl {
 public:
  SmfImmApplierHdl(void);
  ~SmfImmApplierHdl(void);

  // Create an applier for monitoring changes of one attribute in one object
  // The applier is not started. To start use Start() method
  // Note: Create() must be called before Start() and Stop() can be used
  // Return false if Fail
  bool Create(const ApplierSetupInfo& setup_info);

  // Cancel an ongoing create operation that may take a long time.
  // The saImmOiInitialize_2() (see IMM documentation) must have a TRY AGAIN
  // loop that may take up to one minute.
  // This method can be used by an applier that uses a thread for IMM handling
  // from outside the thread to cancel an ongoing create operation that may
  // hang the thread
  void CancelCreate(void);

  // Start the applier
  // Return false if Fail
  bool Start(void);

  // Finalize all resources used by the applier
  // Return false if Fail
  bool Stop(void);

  // Remove the applier by freeing all IMM resources
  void Remove(void);

  // Get the monitored value
  // If there is no valid value an empty string is returned
  std::string get_value(void);

  // Used with poll loop (fd)
  SaSelectionObjectT get_selection_object(void) { return imm_appl_selobj_; }

  // Uses saImmOiDispatch to call the appropriate callback function
  // Shall be called when the selection object releases the poll loop
  // If return code is kGetNewSelctionObject the applier has been restored and
  // the selection object has to be updated
  ExecuteImmCallbackReply ExecuteImmCallback(void);

 private:
  const SaVersionT kImmVersion = {'A', 2, 17};
  static std::atomic<unsigned int> instance_number_;
  bool isCreated_;
  std::string applier_name_;
  SaImmOiHandleT imm_appl_hdl_;
  SaSelectionObjectT imm_appl_selobj_;
};

#endif  // SMF_SMFD_SMFIMMAPPLIERHDL_H_
