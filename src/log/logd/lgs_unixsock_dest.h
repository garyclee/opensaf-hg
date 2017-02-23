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

#ifndef SRC_LOG_LOGD_LGS_UNIXSOCK_DEST_H_
#define SRC_LOG_LOGD_LGS_UNIXSOCK_DEST_H_

#include <string>
#include <map>

#include "base/log_message.h"
#include "base/time.h"
#include "base/unix_client_socket.h"
#include "log/logd/lgs_common.h"
#include "log/logd/lgs_dest.h"

//>
// Represent connection to local Unix Domain socket
// @UnixSocketHandler = Local Socket Destination class
//
// Each destination name will have its own instance
// of this class. It represents the connection
// to the destination. So, it contains connection status,
// and the sequence message ID @msg_id_, showing how many logrecord
// has been written to the socket. This sequence number
// helps to detect if there is any msg lost so far.
//
// When getting a send log record request, this class
// will form message complying with RFC5424 protocol, putting
// the log record which has been writen to local file at MSG field,
// also do fill information to other fields in protocol header.
//
// The instance of this class is only deleted if destination info (@value)
// is deleted from Destination Configuration attribute.
//
// It would be wrong if more than one instance of this class
// pointed to same Domain Unix Socket (same local path name).
//
// Regarding connection status, the status will be flushed to
// latest one at points:
// 1) Creating new destination:
//    The instance will open a connection to destination,
//    then update its status based on the file descriptor.
//
// 2) Change destination to new one:
//    The previous connection is closed and opened a new one
//    toward the destination, then update the status
//    based on the file descriptor.
//
// 3) When writing log record to destination:
//    This class is not aware of the destination whether
//    the destination is dead or restarted.
//    If it gets failed to write to socket, the status will be updated.
//    just do that to detect the receiver on destination is dead/restarted.
//
// 4) Delete the destination
//    When deleing destination, the instance will be deleted.
//    Then, the status will be updated accordingly.
//
// All methods provided by this class should grant access
// to @UnixSocketType only. No one else can directly
// invoke any method of this class.
//
// With @Send(), if the msg get failed to send to destination,
// will log info to syslog and return the error code to upper layer.
//
// The other information on this class is that, within @Send()
// method, creating a very big stack buffer - over 65*1024 bytes
// even the sent RFC msg is quite short (e.g: rfc header + 150 bytes).
//<
class UnixSocketHandler {
 public:
  // Set stack size of buffer to maximum size.
  static const uint32_t kBufMaxSize = 65*1024 + 1024;
  // typedef, aim to shorten declaration.
  using RfcBuffer = base::Buffer<kBufMaxSize>;

  // This class is only dedicated to UnixSocketType.
  friend class UnixSocketType;
  // Open and connect to the socket
  void Open();
  // @Close Do nothing
  void Close();
  // Form RFC5424 and send to the socket
  ErrCode Send(const DestinationHandler::RecordInfo&);
  // Get the socket status
  DestinationStatus GetSockStatus();
  // Form rfc5424 syslog format
  static void FormRfc5424(
      const DestinationHandler::RecordInfo& msg,
      RfcBuffer* buf);

 private:
  explicit UnixSocketHandler(const char*);
  ~UnixSocketHandler();

  // Get the @status_ up-to-date
  void FlushStatus();
  // Convert AIS log stream severity to syslog severity
  static base::LogMessage::Severity Sev(uint16_t);

  // Hold destination info (@value)
  std::string sock_path_;
  base::UnixClientSocket sock_;
  // Hold the connection status
  DestinationStatus status_;

  DELETE_COPY_AND_MOVE_OPERATORS(UnixSocketHandler);
};

//>
// @UnixSocketType: Local Socket Destination Type
//
// This class represents local socket destination type.
// Every destinations name with local socket destination type
// must refer to one and only one instance of this class.
// Therefore, it has been designed as a singleton pattern.
//
// Outside world are not allowed to see any methods of this class
// except DestinationHandler. Right after analyzing DestinationHandler messages,
// knowing what is the destination type, DestinationHandler will pass
// the message to this class via the unique instance.
//
// This class holds a map b/w destination name and @UnixSocketHandler instance
// that represents the connection to destination for that destination name.
//
// Based on what type of messages come to, it will take specific actions
// accordingly.
// 1) Create/Update destination message
//    The message will contain destination name, destination type,
//    and probably destination info (local socket path).
//    Basing on the destination name, the Handler will sure that
//    a) Any such destination name has been created before?
//    b) And any @UnixSocketHandler instance is mapped to that
//       destination name?
//    If no destination name exist in the @map, then Hander will
//    create a new pair <name, @UnixSocketHandler> and update its destination.
//    If the destination name already exist, will check if configuration info
//    provided in the message is different with the existing or not.
//    We expect it should be different with the existing, otherwise
//    we have code fault somewhere. If different from the existing, closing
//    the old connection, and creating new one and re-map to that name.
//
// 2) Sending log record message
//    The message will contain destination name, log record, log stream DN
//    and other infos. Basing on the destination name, the Handler will sure:
//    a) Any such destination name has been created before?
//    b) And any @UnixSocketHandler instance is mapped to that
//       destination name?
//    With this message, the Handler expects the destination name and
//    @UnixSocketHandler must exist. Otherwise, there is a code
//    fault somewhere.
//    and the Handler will drop that message and log info to syslog.
//    If as expected, the Handler will send the message to
//    @UnixSocketHandler instance, in turn,
//    it will form that message according to RF5424 format and
//    send that message to local unix socket.
//
// 3) Delete destination message
//    This message will contain only destination name. Basing on that info,
//    the Hanlder will find that name in the @map to see any name is there.
//    If have, do:
//    1) Close the connection
//    2) Free resources connected to that destination name
//    Once destination name is deleted, any log record sent to destination name
//    will be drop or no writing to the destination happens.
//<
class UnixSocketType {
 public:
  // @UnixSocketType class is dedicated to @DestinationHandler class.
  // Outside world is not granted to access to this class resources
  friend class DestinationHandler;

  // Get @type that represents for @UnixSocketType. This type must be unique.
  static const std::string Type() { return std::string{name_}; }

 private:
  // typedef for the map. Aim to shorten the type declaration.
  using NameSockHdlrMap =  std::map<std::string, UnixSocketHandler*>;

  UnixSocketType();
  ~UnixSocketType();

  // Get @UnixSocketHandler instance which has been assigned
  // to this destination @name
  UnixSocketHandler* GetDestHandle(const std::string& name);
  // Get configuration value that goes with this destination name @name
  const char* GetCfgValue(const std::string& name);
  // Find if any destination @name configured so far
  bool FindName(const std::string&) const;
  // Interpret the DestinationHandler msg and forward to right handle
  ErrCode ProcessMsg(const DestinationHandler::HandleMsg&);
  // Processing @Dispacher::RecordMsg msg, then forward to its instance
  // @UnixSocketHandler to form RFC5424 syslog format before writing to socket.
  ErrCode HandleRecordMsg(const DestinationHandler::RecordMsg&);
  // Processing @DestinationHandler::CfgDestMsg msg. Go thought all destinations
  // and create or update destination configuration depending on contents.
  ErrCode HandleCfgMsg(const DestinationHandler::CfgDestMsg&);
  // Processing @DestinationHandler::CfgDestMsg msg. Go thought all deleted
  // destinations names, then do close connections and free resources.
  ErrCode HandleDelCfgMsg(const DestinationHandler::DelDestMsg&);
  // Get destination status for destination @name.
  // The value could be:
  // 1) "CONNECTED": destination @name is configured and connected to other end.
  // 2) "FAILED": destination @name is configured, but not able co connect to.
  //    or no destination value has been given.
  const std::string GetDestinationStatus(const std::string& name);
  // Convert enum status to string
  const std::string DestStatusToStr(DestinationStatus status);
  // Get the unique instance represents this unique type
  // This method should only be used by @DestinationHandler class
  // Get all destination status for Local Socket Destination Typen
  // @return a vector of status, with format
  // {"name,active", "name2,inactive, etc."}
  static const VectorString GetAllDestStatus();
  // Unique object instance for this @UnixSocketType class
  static UnixSocketType& Instance() { return me_; }
  // Unique instance for this class
  static UnixSocketType me_;
  // The type name of this @UnixSocketType (my identity).
  // Must use this "type" name in destination configuration
  // if want to destinate to local unix socket.
  static const char name_[];

  // Map b/w dest name and its own @UnixSocketHandler instance.
  // Same @name must have same @UnixSocketHandler instance.
  NameSockHdlrMap name_sockethdlr_map_;

  DELETE_COPY_AND_MOVE_OPERATORS(UnixSocketType);
};

#endif  // SRC_LOG_LOGD_LGS_UNIXSOCK_DEST_H_
