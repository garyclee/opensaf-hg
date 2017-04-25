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

#include "log/logd/lgs_nildest.h"

#include <algorithm>

#include "base/logtrace.h"

static const char kDelimeter[] = ";";

//==============================================================================
// NilDestType class
//==============================================================================
NilDestType NilDestType::me_;
const char NilDestType::name_[] = "NILDEST";

NilDestType::NilDestType() {}

NilDestType::~NilDestType() {}

const VectorString NilDestType::GetAllDestStatus() {
  VectorString output{};
  if (me_.destnames_.size() == 0) return output;
  for (const auto& it : me_.destnames_) {
    std::string status = it + "," + "NOT_CONNECTED";
    TRACE("%s status = %s", __func__, status.c_str());
    output.push_back(status);
  }
  return output;
}

bool NilDestType::FindName(const std::string& name) const {
  return (std::find(destnames_.begin(), destnames_.end(), name) !=
          destnames_.end());
}

ErrCode NilDestType::HandleRecordMsg(const DestinationHandler::RecordMsg& msg) {
  ErrCode ret = ErrCode::kDrop;
  TRACE("Destination name(%s) is NILDEST. Drop msg!", msg.name);
  return ret;
}

ErrCode NilDestType::HandleCfgMsg(const DestinationHandler::CfgDestMsg& msg) {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  const std::string name{msg.name};
  TRACE("%s Add NILDEST with name (%s)", __func__, msg.name);
  destnames_.push_back(name);
  TRACE_LEAVE();
  return ret;
}

ErrCode NilDestType::HandleDelCfgMsg(
    const DestinationHandler::DelDestMsg& msg) {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  TRACE("%s remove NILDEST destination name (%s)", __func__, msg.name);
  const std::string name{msg.name};
  destnames_.erase(std::remove(destnames_.begin(), destnames_.end(), name),
                   destnames_.end());
  TRACE_LEAVE();
  return ret;
}

ErrCode NilDestType::ProcessMsg(const DestinationHandler::HandleMsg& msg) {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  static bool cfgsent = false;
  // The proper sequence should be:
  // 1) kCfgDest   : for configure destination handlers
  // 2) kSendRecord: write log record to destination
  // 3) kDelDest   : delete specific destinations
  // 4) kNoDest    : delete all destinations
  // Msg could be drop if the sequence is not correct.
  switch (msg.type) {
    case DestinationHandler::MsgType::kSendRecord: {
      // No destination yet configured. Drop the message.
      TRACE("%s %s", __func__, "RecordMsg msg");
      const DestinationHandler::RecordMsg& rmsg = msg.info.rec;

      // No destination has been configured so far.
      if (cfgsent == false) {
        LOG_WA("Destination handler not yet initialized. Drop msg.");
        ret = ErrCode::kDrop;
      } else {
        ret = HandleRecordMsg(rmsg);
      }
      break;
    }

    case DestinationHandler::MsgType::kCfgDest: {
      const DestinationHandler::CfgDestMsg& cmsg = msg.info.cfg;
      TRACE("%s %s", __func__, "CfgDestMsg msg");
      ret = HandleCfgMsg(cmsg);
      // Notice there at leat one destination configured.
      if (ret == ErrCode::kOk) cfgsent = true;
      break;
    }

    case DestinationHandler::MsgType::kDelDest: {
      const DestinationHandler::DelDestMsg& dmsg = msg.info.del;
      TRACE("%s %s", __func__, "DelDestMsg msg");
      // No destination yet configured. Drop the message.
      if (cfgsent == false) {
        LOG_NO("No destination configured yet. Drop msg.");
        return ErrCode::kDrop;
      }
      ret = HandleDelCfgMsg(dmsg);
      break;
    }

    case DestinationHandler::MsgType::kNoDest: {
      LOG_NO("%s %s", __func__, "DelAllCfgDest msg");
      if (cfgsent == false) {
        LOG_NO("No destination configured yet");
        return ErrCode::kDrop;
      }
      destnames_.clear();
      cfgsent = false;
      break;
    }

    default:
      LOG_ER("Unknown dispatch message type %d", static_cast<int>(msg.type));
      ret = ErrCode::kInvalid;
  }

  TRACE_LEAVE();
  return ret;
}
