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

#include "log/logd/lgs_unixsock_dest.h"

#include <algorithm>

#include "log/logd/lgs_util.h"
#include "log/logd/lgs_nildest.h"
#include "base/logtrace.h"

// RFC5424 syslog msg format
// =========================
// 1) HEADER
// PRI             = 16*8 + SaLogSeverityT
// VERSION         = 1
// TIMESTAMP       = When a log record was actually logged
// HOSTNAME        = FQDN = <hostname>[.<networkname>]
// APP-NAME        = App name who generated log record
// PROCID          = NILVALUE
// MSGID           = log stream name + 'C'/'R' (e.g:saLogSystemC)
//                   or a hash number generated from log stream DN.
// STRUCTURED-DATA = NILVALUE
//
// 2) MSG BODY
// MSG             = The log record written to local file
//

static const char kDelimeter[] = ";";

//==============================================================================
// UnixSocketHandler class
//==============================================================================
UnixSocketHandler::UnixSocketHandler(const char* socket_name)
    : sock_path_{socket_name}
    , sock_{socket_name}
    , status_{DestinationStatus::kFailed} {
      // Open the unix socket & and flush the destination status to @status_
      Open();
};

void UnixSocketHandler::FlushStatus() {
  if (sock_.fd() >= 0) {
    status_ = DestinationStatus::kActive;
  } else {
    status_ = DestinationStatus::kFailed;
  }
}

DestinationStatus UnixSocketHandler::GetSockStatus() {
  Open();
  return status_;
}

void UnixSocketHandler::Open() {
  FlushStatus();
}

void UnixSocketHandler::FormRfc5424(
    const DestinationHandler::RecordInfo& msg,
    RfcBuffer* buf) {
  base::LogMessage::Severity sev{Sev(msg.severity)};
  base::LogMessage::HostName hostname{msg.origin};
  base::LogMessage::ProcId procid{""};
  base::LogMessage::AppName appname{msg.app_name};

  base::LogMessage::Write(base::LogMessage::Facility::kLocal0,
                          sev,
                          msg.time,
                          hostname,
                          appname,
                          procid,
                          base::LogMessage::MsgId{msg.msgid},
                          {},
                          std::string{msg.log_record},
                          buf);
}

ErrCode UnixSocketHandler::Send(const DestinationHandler::RecordInfo& msg) {
  TRACE_ENTER();
  RfcBuffer buffer;
  ErrCode ret = ErrCode::kOk;

  FormRfc5424(msg, &buffer);

  ssize_t length = buffer.size();
  ssize_t len = sock_.Send(buffer.data(), length);
  // Resend as probably receiver has just been restarted.
  // If it is the case, the holding file descriptor could be invalid
  // and it will get failed to send log record to destination even the receiver
  // is up already. If we do re-send once, the file descriptor will be renew
  // within the UnixSocket::Send() and probably get successful to send
  // log record to the destination.
  if (len != length) {
    TRACE("The receiver might be just restarted");
    len = sock_.Send(buffer.data(), length);
  }

  FlushStatus();

  if (len != length) {
    LOG_NO("Failed to send log record to socket destination.");
    ret = ErrCode::kErr;
  }

  TRACE_LEAVE();
  return ret;
}

void UnixSocketHandler::Close() {
  // Do nothing
}

// Convert AIS sev @a sev to Severity class
base::LogMessage::Severity UnixSocketHandler::Sev(uint16_t sev) {
  switch (sev) {
    case SA_LOG_SEV_EMERGENCY:
      return base::LogMessage::Severity::kEmerg;
    case SA_LOG_SEV_ALERT:
      return base::LogMessage::Severity::kAlert;
    case SA_LOG_SEV_CRITICAL:
      return base::LogMessage::Severity::kCrit;
    case SA_LOG_SEV_ERROR:
      return base::LogMessage::Severity::kErr;
    case SA_LOG_SEV_WARNING:
      return base::LogMessage::Severity::kWarning;
    case SA_LOG_SEV_NOTICE:
      return base::LogMessage::Severity::kNotice;
    case SA_LOG_SEV_INFO:
      return base::LogMessage::Severity::kInfo;
    default:
      LOG_ER("Unknown severity level (%d)", (int)sev);
      return base::LogMessage::Severity::kErr;
  }
}

UnixSocketHandler::~UnixSocketHandler() {
  // Destination is deleted
  status_ = DestinationStatus::kFailed;
  // The parent class will do closing the connection
  // and other resources.
}

//==============================================================================
// UnixSocketType class
//==============================================================================
UnixSocketType UnixSocketType::me_;
const char UnixSocketType::name_[] = "UNIX_SOCKET";

UnixSocketType::UnixSocketType() {}

UnixSocketType::~UnixSocketType() {
  // Close all connections & stop the thread
}

const std::string UnixSocketType::DestStatusToStr(DestinationStatus status) {
  switch (status) {
    // Destination configured and is being connected to destination
    case DestinationStatus::kActive: return "CONNECTED";
      // Destination configured and is being disconnected to destination
    case DestinationStatus::kFailed: return "FAILED";
    default: return "UNKNOWN";
  }
}

const std::string UnixSocketType::GetDestinationStatus(
    const std::string& name) {
  if (FindName(name) == false) return std::string{"UNKNOWN"};
  const UnixSocketHandler* sk = name_sockethdlr_map_[name];
  if (sk == nullptr) return std::string{"FAILED"};
  return DestStatusToStr(name_sockethdlr_map_[name]->GetSockStatus());
}

const VectorString UnixSocketType::GetAllDestStatus() {
  VectorString output{};
  if (me_.name_sockethdlr_map_.size() == 0) return output;
  for (const auto& it : me_.name_sockethdlr_map_) {
    std::string status = it.first + "," + me_.GetDestinationStatus(it.first);
    TRACE("%s status = %s", __func__, status.c_str());
    output.push_back(status);
  }
  return output;
}

bool UnixSocketType::FindName(const std::string& name) const {
  return ((name_sockethdlr_map_.find(name) == name_sockethdlr_map_.end()) ?
          (false) : (true));
}

UnixSocketHandler* UnixSocketType::GetDestHandle(const std::string& name) {
  if (FindName(name) == false) return nullptr;
  return name_sockethdlr_map_[name];
}

const char* UnixSocketType::GetCfgValue(const std::string& name) {
  if (FindName(name) == false) return nullptr;
  UnixSocketHandler* sk = name_sockethdlr_map_[name];
  return (sk == nullptr) ? (nullptr) :
      (name_sockethdlr_map_[name]->sock_path_.c_str());
}

ErrCode UnixSocketType::HandleRecordMsg(
    const DestinationHandler::RecordMsg& msg) {
  ErrCode ret = ErrCode::kOk;
  // No destination configuration with this destination name
  if (FindName(std::string{msg.name}) == false) {
    LOG_WA("No such destination name(%s) configured", msg.name);
    ret = ErrCode::kDrop;
    return ret;
  }

  // NIL destination - only have "name" and "type", but no "value".
  if (GetDestHandle(std::string{msg.name}) == nullptr) {
    LOG_NO("The destination name(%s) not yet configured", msg.name);
    ret = ErrCode::kDrop;
    return ret;
  }

  // Send to destination end.
  osafassert(name_sockethdlr_map_[std::string{msg.name}] != nullptr);
  return name_sockethdlr_map_[std::string{msg.name}]->Send(msg.rec);
}

ErrCode UnixSocketType::HandleCfgMsg(
    const DestinationHandler::CfgDestMsg& msg) {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  const std::string name{msg.name};
  const char* newcfg = msg.value;

  // Close and free previous @LocakSkDestHanlde instance if existing.
  if (GetDestHandle(name) != nullptr) {
    TRACE("%s delete old destination value for name (%s)", __func__, msg.name);
    delete name_sockethdlr_map_[name];
    name_sockethdlr_map_[name] = nullptr;
  }

  // Request to create destination handler
  if ((newcfg != nullptr) && (strlen(newcfg) > 0)) {
    TRACE("%s add new destination name (%s)", __func__, msg.name);
    // Create a new destination handler @UnixSocketHandler for @name
    name_sockethdlr_map_[name] = new UnixSocketHandler(newcfg);
    ret = (name_sockethdlr_map_[name] == nullptr) ? ErrCode::kNoMem : ret;
    return ret;
  } else {
    TRACE("%s create nildest with destination name (%s)", __func__, msg.name);
    // nildest case with format "name;type;"
    name_sockethdlr_map_[name] = nullptr;
  }

  TRACE_LEAVE();
  return ret;
}

ErrCode UnixSocketType::HandleDelCfgMsg(
    const DestinationHandler::DelDestMsg& msg) {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  TRACE("%s remove destination name (%s)", __func__, msg.name);
  const std::string name{msg.name};
  // Close and free @LocakSkDestHanlde instance if existing.
  if (GetDestHandle(name) != nullptr) delete name_sockethdlr_map_[name];
  // Erase from the map
  name_sockethdlr_map_.erase(name);
  TRACE_LEAVE();
  return ret;
}

ErrCode UnixSocketType::ProcessMsg(
    const DestinationHandler::HandleMsg& msg) {
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

      for (auto& it : name_sockethdlr_map_) {
        if (it.second != nullptr) delete it.second;
        name_sockethdlr_map_.erase(it.first);
      }
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
