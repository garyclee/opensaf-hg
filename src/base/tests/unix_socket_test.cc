/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
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

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include "base/unix_server_socket.h"
#include "gtest/gtest.h"

class UnixSocketTest : public ::testing::Test {
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

  virtual void SetUp() {
    char dir_tmpl[] = "/tmp/unix_socket_testXXXXXX";
    char* dir_name = mkdtemp(dir_tmpl);
    ASSERT_NE(dir_name, nullptr);
    tmpdir_ = std::string{dir_name};
  }

  virtual void TearDown() {
    if (!tmpdir_.empty()) {
      unlink(SocketName().c_str());
      rmdir(tmpdir_.c_str());
    }
  }

  std::string tmpdir_;
};

TEST_F(UnixSocketTest, DestructorShallUnlinkSocket) {
  ASSERT_FALSE(SocketExists());
  base::UnixServerSocket* sock = new base::UnixServerSocket(SocketName());
  EXPECT_FALSE(SocketExists());
  char buf[32];
  sock->Recv(buf, sizeof(buf));
  EXPECT_TRUE(SocketExists());
  delete sock;
  EXPECT_FALSE(SocketExists());
}

TEST_F(UnixSocketTest, OpenShallUnlinkExistingSocket) {
  ASSERT_FALSE(SocketExists());
  int sock = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  ASSERT_GE(sock, 0);
  struct sockaddr_un addr{AF_UNIX, {}};
  memcpy(addr.sun_path, SocketName().c_str(), SocketName().size() + 1);
  ASSERT_EQ(bind(sock,
                 reinterpret_cast<const struct sockaddr*>(&addr),
                 sizeof(addr)), 0);
  ASSERT_EQ(close(sock), 0);
  EXPECT_TRUE(SocketExists());
  base::UnixServerSocket* server = new base::UnixServerSocket(SocketName());
  char buf[32];
  ssize_t result = server->Recv(buf, sizeof(buf));
  int saved_errno = errno;
  EXPECT_EQ(result, -1);
  EXPECT_EQ(saved_errno, EAGAIN);
  EXPECT_GE(server->fd(), 0);
  delete server;
}

TEST_F(UnixSocketTest, DontUnlinkExistingSocketIfServerStillAlive) {
  ASSERT_FALSE(SocketExists());
  base::UnixServerSocket* sock = new base::UnixServerSocket(SocketName());
  char buf[32];
  ssize_t result = sock->Recv(buf, sizeof(buf));
  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EAGAIN);
  base::UnixServerSocket* sock2 = new base::UnixServerSocket(SocketName());
  result = sock2->Recv(buf, sizeof(buf));
  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EADDRINUSE);
  delete sock2;
  EXPECT_TRUE(SocketExists());
  delete sock;
  EXPECT_FALSE(SocketExists());
}

TEST_F(UnixSocketTest, FdMethodShallOpenTheSocket) {
  ASSERT_FALSE(SocketExists());
  base::UnixServerSocket* sock = new base::UnixServerSocket(SocketName());
  int fd = sock->fd();
  EXPECT_GE(fd, 0);
  EXPECT_TRUE(SocketExists());
  delete sock;
}
