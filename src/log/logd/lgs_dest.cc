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

#include "log/logd/lgs_dest.h"

#include <algorithm>

#include "log/logd/lgs_util.h"
#include "log/logd/lgs_config.h"
#include "log/logd/lgs_nildest.h"
#include "log/logd/lgs_unixsock_dest.h"
#include "base/logtrace.h"
#include "base/hash.h"

static const char kDelimeter[] = ";";

//==============================================================================
// DestinationHandler class
//==============================================================================
DestinationHandler DestinationHandler::me_;

// Only support "kUnix" and "kNilDest" so far
DestinationHandler::DestType DestinationHandler::GetDestType(
    const std::string& type) {
  if (type == UnixSocketType::Type()) {
    return DestType::kUnix;
  } else if (type == NilDestType::Type()) {
    return DestType::kNilDest;
  } else {
    LOG_NO("Unknown destination type %s", type.c_str());
    return DestType::kUnknown;
  }
}

std::string DestinationHandler::GenerateMsgId(const std::string& dn,
                                              bool isRtStream) {
  const std::string parent = ",safApp=safLogService";
  std::string msgid{""};
  size_t posa = 0, posb = 0;

  // Extract stream name from stream DN.
  posa = dn.find('=');
  if ((posb = dn.find(',')) == std::string::npos) {
    posb = dn.length();
  }
  std::string sname = dn.substr(posa + 1, posb - posa - 1);

  //>
  // Rules to generate msgid string:
  // 1) Use stream name (e.g: saLogSystem) + 'C'/'R' if it is not over 31 chars
  //    and the stream DN contains the @parent ",safApp=safLogService";
  //    Note: 'C' means configuration stream, 'R' means runtime stream.
  //           Why 31? It is due to MSGID field length limitation
  //           (max is 32 chars length).
  // 2) Otherwise, generate a hash number from stream DN
  //    if stream name is over 31 chars.
  //<
  if (sname.length() < 32 && dn.find(parent) != std::string::npos) {
    msgid = ((isRtStream == true) ? std::string{sname + 'R'}
                                  : std::string{sname + 'C'});
  } else {
    // Do `InitializeHashFunction()` once
    static bool init_invoked = false;
    if (init_invoked == false) {
      base::InitializeHashFunction();
      init_invoked = true;
    }
    msgid = base::Hash(dn);
  }

  return msgid;
}

DestinationHandler::DestType DestinationHandler::ToDestType(
    const std::string& name) {
  if (nametype_map_.find(name) == nametype_map_.end())
    return DestType::kUnknown;
  return nametype_map_[name];
}

void DestinationHandler::UpdateDestTypeDb(
    const DestinationHandler::HandleMsg& msg) {
  MsgType mtype = msg.type;

  switch (mtype) {
    case MsgType::kCfgDest: {
      const CfgDestMsg& cdest = msg.info.cfg;
      TRACE("%s kCfgDest (%s)", __func__, cdest.name);
      nametype_map_[std::string{cdest.name}] =
          GetDestType(std::string{cdest.type});
      break;
    }

    case MsgType::kDelDest: {
      const DelDestMsg& deldest = msg.info.del;
      TRACE("%s kDelDest (%s)", __func__, deldest.name);
      nametype_map_.erase(std::string{deldest.name});
      break;
    }

    case MsgType::kNoDest: {
      TRACE("%s kNoDest", __func__);
      for (auto& it : nametype_map_) nametype_map_.erase(it.first);
      break;
    }

    default:
      LOG_WA("Unknown Dispatch message type (%d)", static_cast<int>(mtype));
      break;
  }
}

ErrCode DestinationHandler::DispatchTo(const DestinationHandler::HandleMsg& msg,
                                       const std::string& name) {
  ErrCode rc = ErrCode::kOk;
  MsgType mtype = msg.type;
  // All destinations has been deleted. Don't care of destination types.
  // Delete all existing connections of all types.
  if (mtype == MsgType::kNoDest) {
    NilDestType::Instance().ProcessMsg(msg);
    rc = UnixSocketType::Instance().ProcessMsg(msg);
    // Update the destination names from @DestTypeDb
    UpdateDestTypeDb(msg);
    UpdateRtDestStatus();
    return rc;
  }

  // Update the @DestTypeDb first before processing the DestinationHandler msg.
  // Otherwise, finding destination @type will result nothing.
  // Then, the msg will be drop as no destination type found.
  // But with @MsgType::kDelDest, updating should be last
  // after fowarding the msg to @UnixSocketType.
  // Otherwise, resource might be leaked (e.g: connection is not closed)
  if (mtype == MsgType::kCfgDest) {
    UpdateDestTypeDb(msg);
  }

  DestType destType = ToDestType(name);
  switch (destType) {
    // Pass the DestinationHandler msg to Local Unix Domain
    // socket destination type for further processing.
    case DestType::kUnix:
      rc = UnixSocketType::Instance().ProcessMsg(msg);
      break;

    case DestType::kNilDest:
      rc = NilDestType::Instance().ProcessMsg(msg);
      break;

    default:
      LOG_ER("Not support this destination type(%d), or no dest configured!",
             static_cast<int>(destType));
      rc = ErrCode::kDrop;
      break;
  }

  // Delete destination names from @DestTypeDb
  if (mtype == MsgType::kDelDest) {
    UpdateDestTypeDb(msg);
  }

  UpdateRtDestStatus();
  return rc;
}

// Update the `logRecordDestinationStatus` here whenever
// getting CfgDestination or WriteToDestination (?)
void DestinationHandler::UpdateRtDestStatus() {
  VectorString vstatus = GetDestinationStatus();
  lgs_config_chg_t config_data = {NULL, 0};
  lgs_cfgupd_mutival_replace(LOG_RECORD_DESTINATION_STATUS, vstatus,
                             &config_data);
  // Update configuration data store, no need to checkpoint the status to stb.
  int ret = lgs_cfg_update(&config_data);
  if (ret == -1) {
    LOG_WA("%s lgs_cfg_update Fail", __func__);
  }

  // Free memory allocated for the config_data buffer
  if (config_data.ckpt_buffer_ptr != nullptr) free(config_data.ckpt_buffer_ptr);
}

void DestinationHandler::FormCfgDestMsg(const std::string& dest,
                                        CfgDestMsg* msg) {
  osafassert(msg != nullptr);
  const VectorString tmp = logutil::Parser(dest, kDelimeter);
  strncpy(msg->name, tmp[kName].c_str(), kMaxChar);
  if (tmp.size() > 1) {
    strncpy(msg->type, tmp[kType].c_str(), kMaxChar);
  }
  if (tmp.size() == kSize) strncpy(msg->value, tmp[kValue].c_str(), kMaxChar);
}

void DestinationHandler::FormDelDestMsg(const std::string& dest,
                                        DelDestMsg* msg) {
  osafassert(msg != nullptr);
  const VectorString tmp = logutil::Parser(dest, kDelimeter);
  strncpy(msg->name, tmp[kName].c_str(), kMaxChar);
}

bool DestinationHandler::VectorFind(const VectorString& vec,
                                    const std::string& name) {
  return std::find(vec.begin(), vec.end(), name) != vec.end();
}

ErrCode DestinationHandler::AddDestConfig(const VectorString& vdest) {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  CfgDestMsg cmsg;
  // Go thought all destinations and configure one by one
  for (unsigned i = 0; i < vdest.size(); i++) {
    HandleMsg msg{};
    FormCfgDestMsg(vdest[i], &cmsg);
    msg.type = MsgType::kCfgDest;
    memcpy(&msg.info.cfg, &cmsg, sizeof(cmsg));
    ret = DispatchTo(msg, cmsg.name);
    allcfgdests_map_.push_back(vdest[i]);
  }
  TRACE_LEAVE();
  return ret;
}

ErrCode DestinationHandler::DelDestConfig(const VectorString& vdest) {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  DelDestMsg dmsg;
  // Go thought all destinations and configure one by one
  for (unsigned i = 0; i < vdest.size(); i++) {
    if (VectorFind(allcfgdests_map_, vdest[i]) == false) {
      LOG_NO("Not have such dest configuration (%s)", vdest[i].c_str());
      ret = ErrCode::kDrop;
      continue;
    }

    HandleMsg msg{};
    FormDelDestMsg(vdest[i], &dmsg);
    msg.type = MsgType::kDelDest;
    memcpy(&msg.info.del, &dmsg, sizeof(dmsg));
    ret = DispatchTo(msg, dmsg.name);
    // Remove deleted destination from internal database
    allcfgdests_map_.erase(
        std::remove(allcfgdests_map_.begin(), allcfgdests_map_.end(), vdest[i]),
        allcfgdests_map_.end());
  }
  TRACE_LEAVE();
  return ret;
}

ErrCode DestinationHandler::DelAllCfgDest() {
  TRACE_ENTER();
  ErrCode ret = ErrCode::kOk;
  // Don't care destination name
  const std::string name{""};
  HandleMsg msg{};
  msg.type = MsgType::kNoDest;
  ret = DispatchTo(msg, name);
  allcfgdests_map_.clear();
  TRACE_LEAVE();
  return ret;
}

ErrCode DestinationHandler::ProcessCfgChange(const VectorString& vdest,
                                             ModifyType type) {
  ErrCode ret = ErrCode::kOk;
  TRACE_ENTER();

  // Case #1: perform all destination delete, but no existing at all.
  if ((vdest.size() == 0) && (allcfgdests_map_.size() == 0)) {
    return ErrCode::kOk;
  }

  // Case #2: Delete all existing destinations
  if ((vdest.size() == 0) && (allcfgdests_map_.size() != 0)) {
    return DelAllCfgDest();
  }

  // Case #3: Delete destination configurations while no dest cfg yet
  if ((type == ModifyType::kDelete) && (vdest.size() != 0) &&
      (allcfgdests_map_.size() == 0)) {
    return ErrCode::kDrop;
  }

  // Case #4: Delete all destination configurations
  if ((type == ModifyType::kDelete) && (vdest.size() == 0) &&
      (allcfgdests_map_.size() != 0)) {
    return DelAllCfgDest();
  }

  // Case #5: Delete all destinations but no one existing.
  if ((type == ModifyType::kDelete) && (vdest.size() == 0) &&
      (allcfgdests_map_.size() == 0)) {
    return ErrCode::kDrop;
  }

  switch (type) {
    case ModifyType::kAdd:
      // Add new destinations to existing destinations.
      ret = AddDestConfig(vdest);
      break;

    case ModifyType::kReplace:
      // Step #1: Delete all existing destinations if any.
      if (nametype_map_.size() != 0) {
        DelAllCfgDest();
      }
      // Step #2: Create new destinations
      ret = AddDestConfig(vdest);
      break;

    case ModifyType::kDelete:
      ret = DelDestConfig(vdest);
      break;

    default:
      LOG_ER("Unknown modify type (%d)", static_cast<int>(type));
      ret = ErrCode::kErr;
      break;
  }

  TRACE_LEAVE2("ret = %d", static_cast<int>(ret));
  return ret;
}

ErrCode DestinationHandler::ProcessWriteReq(const RecordInfo& info,
                                            const std::string& name) {
  HandleMsg msg{};
  RecordMsg record;

  msg.type = MsgType::kSendRecord;
  strncpy(record.name, name.c_str(), kMaxChar);
  memcpy(&record.rec, &info, sizeof(RecordInfo));
  memcpy(&msg.info.rec, &record, sizeof(RecordMsg));
  return DispatchTo(msg, name);
}

VectorString DestinationHandler::GetDestinationStatus() {
  VectorString unixsock_type{};
  VectorString nildest_type{};
  // Go thought all 02 supported destination types
  unixsock_type = UnixSocketType::Instance().GetAllDestStatus();
  nildest_type = NilDestType::Instance().GetAllDestStatus();
  unixsock_type.insert(unixsock_type.end(), nildest_type.begin(),
                       nildest_type.end());

  return unixsock_type;
}

//==============================================================================
// Exposed interfaces to outer world
//==============================================================================
bool CfgDestination(const VectorString& vdest, ModifyType type) {
  return (DestinationHandler::Instance().ProcessCfgChange(vdest, type) ==
          ErrCode::kOk);
}

bool WriteToDestination(const RecordData& data, const VectorString& destnames) {
  osafassert(data.name != nullptr && data.logrec != nullptr);
  osafassert(data.hostname != nullptr && data.networkname != nullptr);

  if (destnames.size() == 0) {
    TRACE("%s No destination goes with this stream (%s)", __func__, data.name);
    return true;
  }

  bool ret = true;
  DestinationHandler::RecordInfo info;
  std::string networkname = (std::string{data.networkname}.length() > 0)
                                ? ("." + std::string{data.networkname})
                                : "";

  // Origin is FQDN = <hostname>[.<networkname>]
  // hostname = where the log record comes from (not active node)
  const std::string origin = std::string{data.hostname} + networkname;

  info.msgid = data.msgid;
  info.log_record = data.logrec;
  info.record_id = data.recordId;
  info.stream_dn = data.name;
  info.app_name = data.appname;
  info.severity = data.sev;
  info.time = data.time;
  info.origin = origin.c_str();

  // Go thought all destination names and send to the destination
  // that go with that destination name
  for (const auto& destname : destnames) {
    ret = (DestinationHandler::Instance().ProcessWriteReq(info, destname) ==
           ErrCode::kOk);
  }

  return ret;
}

VectorString GetDestinationStatus() {
  return DestinationHandler::Instance().GetDestinationStatus();
}
