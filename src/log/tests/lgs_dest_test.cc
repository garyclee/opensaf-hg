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

#define protected public
#define private public

#include "log/logd/lgs_dest.h"
#include "log/logd/lgs_unixsock_dest.h"
#include "log/logd/lgs_config.h"
#include <string>
#include <vector>
#include "base/unix_server_socket.h"
#include "gtest/gtest.h"

//==============================================================================
// Dummy functions
//==============================================================================
int lgs_cfg_update(const lgs_config_chg_t *config_data) {
  return 0;
}

void lgs_cfgupd_mutival_replace(const std::string attribute_name,
                                const std::vector<std::string> value_list,
                                lgs_config_chg_t *config_data) {
}

//==============================================================================
// logutil namespace
//==============================================================================
namespace logutil {

// Split a string @str with delimiter @delimiter
// and return an vector of strings.
std::vector<std::string> Parser(const std::string& str,
                                const std::string& delimiter) {
  std::vector<std::string> temp;
  std::string s{str};
  size_t pos = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    temp.push_back(s.substr(0, pos));
    s.erase(0, pos + delimiter.length());
  }
  temp.push_back(s);
  return temp;
}

bool isValidName(const std::string& name) {
  // Valid name if @name not contain any characters outside
  // of below strings.
  const std::string validChar = "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "01234567890_-";
  if (name.find_first_not_of(validChar) != std::string::npos)
    return false;

  return true;
}
};  // namespace logutil

//==============================================================================
// CfgDestination() interface
//==============================================================================
// Verify it is OK to add one valid destination configuration
TEST(CfgDestination, AddOneDestination) {
  const std::vector<std::string> vdest {"test;UNIX_SOCKET;/tmp/sock.sock"};
  ASSERT_EQ(CfgDestination(vdest, ModifyType::kAdd), true);
}

// Verify it is Ok to add multiple destination configurations
TEST(CfgDestination, AddMultipleDestination) {
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;/tmp/sock1.sock",
    "test2;UNIX_SOCKET;/tmp/sock2.sock"
  };
  ASSERT_EQ(CfgDestination(vdest, ModifyType::kAdd), true);
}

// Verify it is Ok to add NIL destination
TEST(CfgDestination, AddNilDestination) {
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;/tmp/sock1.sock",
    "test2;UNIX_SOCKET;"
  };
  ASSERT_EQ(CfgDestination(vdest, ModifyType::kAdd), true);
}

// Verify it is OK to delete one destination configuration
TEST(CfgDestination, DelOneDestination) {
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;/tmp/sock1.sock",
    "test2;UNIX_SOCKET;"
   };
  CfgDestination(vdest, ModifyType::kAdd);
  // Delete destination
  const std::vector<std::string> vdeldest {"test1;UNIX_SOCKET;/tmp/sock1.sock"};
  ASSERT_EQ(CfgDestination(vdeldest, ModifyType::kDelete), true);
}

// Verify it is OK to delete all destinations
TEST(CfgDestination, DelAllDestinations) {
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;/tmp/sock1.sock",
    "test2;UNIX_SOCKET;"
  };
  CfgDestination(vdest, ModifyType::kAdd);
  // Delete all destination configurations
  const std::vector<std::string> vdeldest0{};
  ASSERT_EQ(CfgDestination(vdeldest0, ModifyType::kDelete), true);
}

// Verify the request is drop if the delete request
// come before adding a destination configuation.
TEST(CfgDestination, DelDestButNoCfgSentYet) {
  // Delete all destination configurations
  const std::vector<std::string> vdest {};
  CfgDestination(vdest, ModifyType::kDelete);
  const std::vector<std::string> vdeldest3 {
    "test1;UNIX_SOCKET;/tmp/sock1.sock"
  };
  ASSERT_EQ(CfgDestination(vdeldest3, ModifyType::kDelete), false);
}

// Verify the request is drop if deleting non-exist destination configuration.
TEST(CfgDestination, DelNonExistDestination) {
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;/tmp/sock1.sock",
    "test2;UNIX_SOCKET;"
  };
  CfgDestination(vdest, ModifyType::kAdd);
  const std::vector<std::string> vdeldest4 {
    "test3;UNIX_SOCKET;/tmp/sock3.sock"
  };
  ASSERT_EQ(CfgDestination(vdeldest4, ModifyType::kDelete), false);
}

//==============================================================================
// WriteToDestination() interface
//==============================================================================
static const char rec[] = "hello world";
static const char hostname[] = "sc-1";
static const char networkname[] = "networkA";
static const uint16_t sev = 3;
static const char appname[] = "lgs_dest_test";
static const char dn[] = "safLgStrCfg=test";
void initData(RecordData* data) {
  data->name = dn;
  data->logrec = rec;
  data->hostname = hostname;
  data->networkname = networkname;
  data->appname = appname;
  data->isRtStream = 0;
  data->recordId = 1;
  data->sev = 3;
  data->time = base::ReadRealtimeClock();
}

// No destination name & no destination configuration exist
// Verify the sending log record request is drop.
TEST(WriteToDestination, NoDestNameAndNonExistDest) {
  RecordData data;
  // No destination configured at all.
  const std::vector<std::string> vdeldest5 {};
  CfgDestination(vdeldest5, ModifyType::kDelete);
  // Write the log record to sk
  initData(&data);
  // No destination name set.
  bool ret = WriteToDestination(data, {});
  ASSERT_EQ(ret, true);
}

// Have destination name set, but no destination configuration exist.
// Verify the sending record request is drop.
TEST(WriteToDestination, HaveDestNameButNonExistDest) {
  RecordData data;
  // No destination configured at all
  const std::vector<std::string> vdeldest6 {};
  CfgDestination(vdeldest6, ModifyType::kDelete);
  // Write the log record to sk
  initData(&data);
  // Have destination name set
  bool ret = WriteToDestination(data, {"test"});
  ASSERT_EQ(ret, false);
}

// Verify the sending record to destination is drop
// if having NIL destination with such destination name.
TEST(WriteToDestination, HaveDestNameButNilDest) {
  RecordData data;
  // Have nil destination
  const std::vector<std::string> nildest {"test;UNIX_SOCKET;"};
  CfgDestination(nildest, ModifyType::kReplace);
  // Write the log record to sk
  initData(&data);
  bool ret = WriteToDestination(data, {"test"});
  ASSERT_EQ(ret, false);
}

//
// Sending log record to destination and verify
// if the end receives data correctly as sent data.
//
static const uint64_t kMaxSize = 65*1024 + 1024;

// The function is used to form the rfc5424 syslog msg
// by passing @DestinationHandler::RecordInfo.
void FormRfc5424Test(const DestinationHandler::RecordInfo& info,
                     base::Buffer<kMaxSize>* out) {
  UnixSocketHandler::FormRfc5424(info, out);
}

// Send a record @rec, then verify if receiving data
// and sent data is matched.
TEST(WriteToDestination, HaveDestNameAndDestCfg) {
  char buf[1024] = {0};
  RecordData data;
  DestinationHandler::RecordInfo info{};
  base::Buffer<kMaxSize> testbuf;
  std::string nkname{""};
  std::string origin{""};

  // Create the server listen to the local socket
  static base::UnixServerSocket server{"/tmp/test.sock"};
  server.Open();

  const std::vector<std::string> dest {"test;UNIX_SOCKET;/tmp/test.sock"};
  CfgDestination(dest, ModifyType::kReplace);

  // Write the log record to sk
  initData(&data);

  nkname = (std::string{data.networkname}.length() > 0) ?
          ("." + std::string {data.networkname}) : "";

  // Origin is FQDN = <hostname>[.<networkname>]
  origin = std::string {data.hostname} + nkname;

  info.msgid      = DestinationHandler::GenerateMsgId(
                    data.name, data.isRtStream).c_str();
  info.log_record = data.logrec;
  info.record_id  = data.recordId;
  info.stream_dn  = data.name;
  info.app_name   = data.appname;
  info.severity   = data.sev;
  info.time       = data.time;
  info.origin     = origin.c_str();

  WriteToDestination(data, {"test"});

  // Form the expected/tested buffer.
  FormRfc5424Test(info, &testbuf);

  // Server reads sent data.
  server.Recv(buf, 1024);

  // Verify if the sent/received buf is matched.
  ASSERT_EQ(strncmp(buf, testbuf.data(), 1024), 0);
}

//==============================================================================
// Verify GetDestinationStatus() interface
//==============================================================================

// Verify the destination connection status is reflected correctly.
TEST(GetDestinationStatus, AddOneDestination) {
  const VectorString vstatus {"test,FAILED"};
  const std::vector<std::string> vdest {"test;UNIX_SOCKET;/tmp/sock.sock"};
  CfgDestination(vdest, ModifyType::kReplace);
  ASSERT_EQ(GetDestinationStatus(), vstatus);
}

// Verify the destination connection status is reflected correctly
// in case of adding multiple destinations
TEST(GetDestinationStatus, AddMultipleDestination) {
  const VectorString vstatus {
    "test,FAILED",
    "test1,FAILED",
    "test2,FAILED"
  };
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;/tmp/sock1.sock",
    "test2;UNIX_SOCKET;/tmp/sock2.sock"
  };

  CfgDestination(vdest, ModifyType::kReplace);
  ASSERT_EQ(GetDestinationStatus(), vstatus);
}

// Verify the destination connection status is reflected correctly
// in case of adding multiple destinations including NILDEST type.
TEST(GetDestinationStatus, AddMultipleDestinationWithNilDest) {
  const VectorString vstatus {
    "test,FAILED",
    "test1,FAILED",
    "test2,NOT_CONNECTED"
  };
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;",
    "test2;NILDEST;"};
  CfgDestination(vdest, ModifyType::kReplace);
  ASSERT_EQ(GetDestinationStatus(), vstatus);
}

TEST(GetDestinationStatus, AddMultipleDestinationWithNilDest02) {
  const VectorString vstatus {
    "test,FAILED",
    "test1,FAILED",
    "test2,NOT_CONNECTED",
    "test3,NOT_CONNECTED"
  };
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/sock.sock",
    "test1;UNIX_SOCKET;",
    "test2;NILDEST;",
    "test3;NILDEST;/tmp/sock.sock"};
  CfgDestination(vdest, ModifyType::kReplace);
  ASSERT_EQ(GetDestinationStatus(), vstatus);
}

// Verify the destination connection status is reflected correctly
// in case of adding multiple destinations including NILDEST type.
// and have destination receiver.
TEST(GetDestinationStatus, AddMultipleDestinationWithDestReceiver) {
  const VectorString vstatus {
    "test,CONNECTED",
    "test1,FAILED",
    "test2,NOT_CONNECTED"
  };
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/test.sock",
    "test1;UNIX_SOCKET;",
    "test2;NILDEST;"
  };

  // Create the server listen to the local socket
  static base::UnixServerSocket server{"/tmp/test.sock"};
  server.Open();
  CfgDestination(vdest, ModifyType::kReplace);

  ASSERT_EQ(GetDestinationStatus(), vstatus);
}

// Verify the destination connection status is reflected correctly
// in case of removing one of destination from
// multiple destinations including NILDEST type.
// and have destination receiver.
TEST(GetDestinationStatus, RemoveDestFromMultipleDestinationWithDestReceiver) {
  const VectorString vstatus {
    "test,CONNECTED",
    "test2,NOT_CONNECTED"
  };
  const std::vector<std::string> vdest {
    "test;UNIX_SOCKET;/tmp/test.sock",
    "test1;UNIX_SOCKET;",
    "test2;NILDEST;"
  };

  // Create the server listen to the local socket
  static base::UnixServerSocket server{"/tmp/test.sock"};
  server.Open();
  CfgDestination(vdest, ModifyType::kReplace);
  CfgDestination({"test1;UNIX_SOCKET;"}, ModifyType::kDelete);

  ASSERT_EQ(GetDestinationStatus(), vstatus);
}
