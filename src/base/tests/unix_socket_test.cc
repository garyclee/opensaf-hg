/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
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

#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include "base/unix_server_socket.h"
#include "gtest/gtest.h"

class UnixSocketTest : public ::testing::Test {
 public:

 protected:

  UnixSocketTest() {
  }

  virtual ~UnixSocketTest() {
  }

  std::string SocketName() {
    return tmpdir_ + "/sock";
  }

  bool SocketExists() {
    struct stat buf;
    int result = stat(SocketName().c_str(), &buf);
    return result == 0 && S_ISSOCK(buf.st_mode);
  }

  // cppcheck-suppress unusedFunction
  virtual void SetUp() {
    char dir_tmpl[] = "/tmp/unix_socket_testXXXXXX";
    char* dir_name = mkdtemp(dir_tmpl);
    ASSERT_NE(dir_name, nullptr);
    tmpdir_ = std::string{dir_name};
  }

  // cppcheck-suppress unusedFunction
  virtual void TearDown() {
    if (!tmpdir_.empty()) {
      unlink(SocketName().c_str());
      rmdir(tmpdir_.c_str());
    }
  }

  std::string tmpdir_;
};

TEST_F(UnixSocketTest, CheckThatServerUnlinksSocket) {
  EXPECT_FALSE(SocketExists());
  base::UnixServerSocket* sock = new base::UnixServerSocket(SocketName());
  EXPECT_FALSE(SocketExists());
  char buf[32];
  sock->Recv(buf, sizeof(buf));
  EXPECT_TRUE(SocketExists());
  delete sock;
  EXPECT_FALSE(SocketExists());
}
