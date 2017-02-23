/*      -*- OpenSAF  -*-
 *
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

#ifndef SRC_LOG_LOGD_LGS_NILDEST_H_
#define SRC_LOG_LOGD_LGS_NILDEST_H_

#include <string>

#include "log/logd/lgs_common.h"
#include "log/logd/lgs_dest.h"

//>
// @NilDestType - represent NILDEST destination type
//
//<
class NilDestType {
 public:
  // @NilDestType class is dedicated to @DestinationHandler class.
  // Outside world is not granted to access to this class resources
  friend class DestinationHandler;

  // Get @type that represents for @NilDestType. This type must be unique.
  static const std::string Type() { return std::string{name_}; }

 private:
  NilDestType();
  ~NilDestType();

  // Find if any destination @name configured so far
  bool FindName(const std::string&) const;
  // Interpret the DestinationHandler msg and forward to right handle
  ErrCode ProcessMsg(const DestinationHandler::HandleMsg&);
  // The msg will be droped if sending to @NILDEST type
  ErrCode HandleRecordMsg(const DestinationHandler::RecordMsg&);
  // Put the destination name to @destnames_ for querying destination status
  ErrCode HandleCfgMsg(const DestinationHandler::CfgDestMsg&);
  // Remove from @destnames_
  ErrCode HandleDelCfgMsg(const DestinationHandler::DelDestMsg&);
  // Get all destination status for @NILDEST type
  // @return a vector of status, with format
  // {"name,NOT_CONNECTED", "name2,NOT_CONNECTED, etc."}
  static const VectorString GetAllDestStatus();
  // Unique object instance for this @NilDestType class
  static NilDestType& Instance() { return me_; }
  // Unique instance for this class
  static NilDestType me_;
  // The type name of this @NilDestType (my identity).
  // Must use this "type" name in destination configuration
  // if want to destinate to @NilDestType
  static const char name_[];

  // Hold all destination names having @NILDEST type
  VectorString destnames_;

  DELETE_COPY_AND_MOVE_OPERATORS(NilDestType);
};

#endif  // SRC_LOG_LOGD_LGS_NILDEST_H_
