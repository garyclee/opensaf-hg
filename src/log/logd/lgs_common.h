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

#ifndef SRC_LOG_LOGD_LGS_COMMON_H_
#define SRC_LOG_LOGD_LGS_COMMON_H_

#include <string>
#include <vector>

// Status of connection to destination
enum DestinationStatus {
  // Destination is configured and is being connected to
  kActive = 0,
  // Destination is configured but not able to connected to
  // or not given any configuration value.
  kFailed = 1,
};

// This error code is introduced in alternative destination ticket #2258.
// and for now, only be used internally in lgs_dest files.
enum ErrCode {
  // No error happens
  kOk = 0,
  // Error happens in general
  kErr,
  // Passing invalid parameters
  kInvalid,
  // Not able to serve the request as the requester is being busy
  kBusy,
  // The request is timeout
  kTimeOut,
  // No memory
  kNoMem,
  // The request/msg is drop as the precondition is not met
  kDrop
};

// Type of modification on multiple attribute values.
// Introduce this own enum as (xvunguy) don't want to
// have dependent to IMM header file just because we need IMM modification type.
// Their meanings are equivelent to `SaImmAttrModificationTypeT`
enum ModifyType { kAdd = 1, kDelete = 2, kReplace = 3 };

//>
// Define the "token" possition in destination configuration.
// Each destination configiration has format "name;type;value".
// After parsing, the output will put to the vector of strings
// with format {"name", "type", "value"}.
// kName : use this index to get "name"
// kType : use this index to get "type"
// kValue: use this index to get "value"
// kSize : this is the maximum size of the dest configuration vector.
//<
enum { kName = 0, kType, kValue, kSize };

// Maximum length of destination name
#define kNameMaxLength 256

//==============================================================================
// Two interfaces are published to outside world
//==============================================================================

// Typedef for shorten declaration
// This type could be used to hold set of strings with formats
// 1) {"name;type;value", etc.}
// 2) {"name", "type", "value"}
// 3) etc.
using VectorString = std::vector<std::string>;

#endif  // SRC_LOG_LOGD_LGS_COMMON_H_
