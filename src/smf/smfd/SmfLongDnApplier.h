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

#ifndef SMF_SMFD_SMFLONGDNAPPLIER_H_
#define SMF_SMFD_SMFLONGDNAPPLIER_H_

#include <string>
#include <memory>

#include "base/macros.h"
#include "smf/smfd/SmfImmApplierHdl.h"

// Commands that can be used with thread
enum class th_cmd { AP_START, AP_STOP, AP_REMOVE };

/**
 * This class implements an IMM applier for monitoring long Dn setting
 * The applier is running in a separate thread.
 * The thread implements a poll loop handling IMM callback and start/stop
 * commands using the SmfImmApplierHdl class
 * For communicating commands to the thread a socket pair is used
 */
class SmfLongDnApplier {
 public:
  SmfLongDnApplier(const std::string& object_name,
                   const std::string& attribute_name);

  ~SmfLongDnApplier(void);

  // Creates the applier thread
  // The thread will create but not start the applier
  void Create(void);

  // Start an applier (in a separate thread).
  void Start(void);

  // Stop the applier.
  // Wait until the applier is stopped
  void Stop(void);

  // Return true if long-dn is allowed based on setting in IMM configuration
  // object
  bool get_value(void) {
    std::string value_str = applierHdl_->get_value();
    if (value_str != "") {
      return std::stoul(value_str, nullptr, 0) != 0 ? true : false;
    } else {
      return false;
    }
  }

 private:
  void Remove(void);
  void CloseSockets(void);
  void SendCommandToThread(th_cmd);

  std::string object_name_;
  std::string attribute_name_;
  SmfImmApplierHdl* applierHdl_;
  int socket_fd_thread_ = 0;   // Socket for thread to read commands
  int socket_fd_command_ = 0;  // Socket for commands to thread

  DELETE_COPY_AND_MOVE_OPERATORS(SmfLongDnApplier);
};

#endif  // SMF_SMFD_SMFLONGDNAPPLIER_H_
