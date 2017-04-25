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

#ifndef SRC_LOG_LOGD_LGS_DEST_H_
#define SRC_LOG_LOGD_LGS_DEST_H_

#include <string>
#include <map>

#include "log/logd/lgs_common.h"
#include "base/time.h"

//>
// @CfgDestination - Configure Destinations.
// @input vdest: a vector of destination configurations.
// @input type : modification type on destination configuration attribute.
//               It has equivalent value to @SaImmAttrModificationTypeT.
// @return: true if everything is OK, otherwise false.
//
// Call this function in CCB modified apply callback context,
// when changes on configuration destination has been verified.
// @type could be ModifyType::kAdd, ModifyType::kReplace, or ModifyType::kDelete
//
// E.g:
// 1) Case #1: create 02 new destinations (@type = ModifyType::kAdd)
//    Then, @vdest should contain data {"a;b;c", "d;e;f"}
// 2) Case #2: modify one destinations. E.g: dest name @d -> @d1
//    Then, @type = ModifyType::kReplace and @vdest contains data {"d1;e;f"}
// 3) Case #3: delete one destination. Eg: dest name @a
//    Then, @type = ModifyType::kDelete and @vdest should contains {"a;b;c"}
// 4) Case #4: delete all destinations
//    Then, @type = ModifyType::kDelete and @vdest contains empty vector {}
//
// The function will parse the provided data @vdest, getting
// destination @name, destination @type, @destination value if any.
// Then form the right handle msg, and forward it to right destination.
//
// This function is blocking. The caller will be blocked until
// the request is done.
//<
bool CfgDestination(const VectorString& vdest, ModifyType type);

//>
// Carry necessary information to form RFC5424 syslog message.
// Aim to reduce number of parameter passing to @WriteToDestination
// @name       : log stream distinguish name
// @logrec     : log record that written to local log file
// @hostname   : log record originator
// @networkname: network name
// @recordId   : record id of @logrec
// @sev        : severity of log record @logrec
// @time       : Same timestamp as in the log record @logrec
//<
struct RecordData {
  const char* name;
  const char* logrec;
  const char* hostname;
  const char* networkname;
  const char* appname;
  const char* msgid;
  bool isRtStream;
  uint32_t recordId;
  uint16_t sev;
  timespec time;

  RecordData()
      : name{nullptr},
        logrec{nullptr},
        hostname{nullptr},
        networkname{nullptr},
        appname{nullptr},
        msgid{nullptr},
        isRtStream{false},
        recordId{0},
        sev{0} {
    time = base::ReadRealtimeClock();
  }
};

//>
// @WriteToDestination - Write @data To Destinations
// @input:
//   @data     : Carry necessary infos including log record.
//   @destnames: Destination names wants to send log record to.
// @return: true if successfully write to destinations. Oherwise, false.
//
// This function is blocking. The caller will be blocked until
// the request is done.
//<
bool WriteToDestination(const RecordData& data, const VectorString& destnames);

//>
// @GetDestinationStatus - Get all destination status
// @input : none
// @output: vector of destination status with format
//          {"name,status", "name1,status", etc.}
//<
VectorString GetDestinationStatus();

//>
// @DestinationHandler class: Dispatch msg to right destination
// All msgs, if want to come to destination handlers,
// must go though the DestinationHandler via
// the interface DestinationHandler::DispachTo()
//
// Three kind of DestinationHandler messages supported.
// 1) Msg contains destination configurations such as
//    destination name, destination type and info (local socket path).
//
// 2) Msg contains log record which is going to write to destination.
//    Beside log record, msg may contain other info such as
//    the stream name(DN) it writting to, log record severity,
//    log record originator, log record id, or timestamp.
//
// 3) Msg contains destinations that is going to be deleted.
//    This message may have list of deleted destination names.
//
// The proper order of DestinationHandler messages should be:
// (1) -> (2) -> (3) or (1) -> (3).
//
// So, if msg contains log record comes(2) to the DestinationHandler first
// somehow (this probably due to an coding hole, that does not
// have a good check before sending msg to Dispacher) before
// the configurating destination msg(1), the DestinationHandler will drop
// the message, and log info to syslog.
// The same to the case if deleting destination msg(3) comes first
// before configuring destination(1), that msg(3) will be drop
// and log info to syslog.
//
// According to information in that messages, the DestinationHandler
// will do its jobs: interpret the msg, take the right destination
// and transfer that the msg to the destination type for futher processing.
//<
class DestinationHandler {
 public:
  enum { kMaxChar = 255 };
  enum class DestType { kUnix = 0, kNilDest, kUnknown };
  //>
  // Handle message types
  // kSendRecord: for writing log record to destination(s).
  // kCfgDest   : for configuring destination(s).
  // kDelDest   : for deleting destination(s).
  // kNoDest    : for deleting all destination(s).
  //<
  enum class MsgType { kSendRecord = 0, kCfgDest, kDelDest, kNoDest };

  //>
  // RecordMsg hold necessary information to forming RFC5424 protocol.
  // @severity  : log severity of sent log record.
  // @record_id : record id attached to sent log record.
  // @hash      : hash number generated from log stream name (DN)
  // @time      : time attached to log record. Using realtime if no set.
  // @origin    : node originator of the log record (host & network name).
  // @log_record: the log record that writting to local file.
  // @stream_dn : log stream name(DN).
  //<
  struct RecordInfo {
    const char* stream_dn;
    const char* msgid;
    const char* origin;
    const char* log_record;
    const char* app_name;
    uint16_t severity;
    uint32_t record_id;
    struct timespec time;

    // Set default value
    RecordInfo()
        : stream_dn{nullptr},
          msgid{nullptr},
          origin{nullptr},
          log_record{nullptr},
          app_name{nullptr},
          severity{0},
          record_id{0} {
      time = base::ReadRealtimeClock();
    }
  };

  //>
  // @RecordMsg - send record @rec to destination name @name
  // @name: destination name
  // @rec : record message
  //>
  struct RecordMsg {
    char name[kMaxChar];
    RecordInfo rec;
    RecordMsg() : name{0}, rec{} {}
  };

  //>
  // CfgDestMsg - holds information for one destination.
  // @name : name of destination.
  // @type : type of destination.
  // @value: destination configuration
  //<
  struct CfgDestMsg {
    char name[kMaxChar];
    char type[kMaxChar];
    char value[kMaxChar];
    // Clear the values of all types
    CfgDestMsg() : name{0}, type{0}, value{0} {}
  };

  //>
  // DelDestMsg - hold information for one destintaion name
  // @name: refer to the destination name that is going to delete.
  //<
  struct DelDestMsg {
    char name[kMaxChar];
    DelDestMsg() : name{0} {}
  };

  //>
  // @HandleMsg - Dispatch message format
  // @type: the value depends on @info
  // 1) kSendRecord if @info is RecordMsg
  // 2) kCfgDest if @info is CfgDestMsg
  // 3) kDelDest if @info is DelDestMsg
  // 4) kNoDest if not above
  //<
  struct HandleMsg {
    MsgType type;
    union {
      RecordMsg rec;
      CfgDestMsg cfg;
      DelDestMsg del;
    } info;
  };

  // Unique object instance of @DestinationHandler class
  static DestinationHandler& Instance() { return me_; }
  // Extract the stream name from stream DN
  static std::string GenerateMsgId(const std::string&, bool);

  // Do destination configuration basing on input @vdest and @type.
  ErrCode ProcessCfgChange(const VectorString& vdest, ModifyType type);
  // Do write log record @info to destinations @name
  ErrCode ProcessWriteReq(const RecordInfo& info, const std::string& name);
  // Get all destination status with format
  // {"name,status", "name1,status1", etc.}
  VectorString GetDestinationStatus();

 private:
  // typedef, aim for shorten declaration
  using NameTypeMap = std::map<std::string, DestType>;
  DestinationHandler() : nametype_map_{} {}
  ~DestinationHandler() {}

  // Dispatch the @HandleMsg msg to destination @name.
  // Based on the destination @name, the method will find the right
  // destination @type, then forward the message to for futher processing.
  ErrCode DispatchTo(const HandleMsg& msg, const std::string& name);
  // Pack @CfgDestMsg from the string @dest which have pure format
  // "name;type;value".
  void FormCfgDestMsg(const std::string& dest, CfgDestMsg* msg);
  // Pack @DelDestMsg basing on pure destination format
  // "name;type;value".
  void FormDelDestMsg(const std::string& dest, DelDestMsg* msg);
  // Form @CfgDestMsg and dispatch the msg.
  ErrCode AddDestConfig(const VectorString& vdest);
  // Form @DelDestMsg and dispatch the msg.
  ErrCode DelDestConfig(const VectorString& vdest);
  // Form @kNoDest dispatch msg
  ErrCode DelAllCfgDest();
  // Find name @name from @vec
  bool VectorFind(const VectorString& vec, const std::string& name);
  // Return @DestType based on the destination string @type
  DestType GetDestType(const std::string& type);
  DestType ToDestType(const std::string& name);
  // Update the @nametype_map_ whenever getting
  // destination configuration change.
  void UpdateDestTypeDb(const DestinationHandler::HandleMsg&);
  // Update the `logRecordDestinationStatus`
  void UpdateRtDestStatus();

  // The map between destination @name and destination @type
  NameTypeMap nametype_map_;
  // Hold all existing destinations with pure format {"a;b;c", etc}
  VectorString allcfgdests_map_;
  static DestinationHandler me_;
};

#endif  // SRC_LOG_LOGD_LGS_DEST_H_
