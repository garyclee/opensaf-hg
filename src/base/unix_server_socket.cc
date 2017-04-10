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

#include "base/unix_server_socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

namespace base {

UnixServerSocket::UnixServerSocket(const std::string& path)
    : UnixSocket{path} {}

UnixServerSocket::~UnixServerSocket() {
  if (get_fd() >= 0) UnixServerSocket::CloseHook();
}

bool UnixServerSocket::OpenHook(int sock) {
  int tmp_sock = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  int connect_result;
  int connect_errno;
  do {
    connect_result = connect(tmp_sock, addr(), addrlen());
    connect_errno = errno;
  } while (connect_result != 0 && connect_errno == EINTR);
  close(tmp_sock);
  if (connect_result != 0 && connect_errno == ECONNREFUSED) unlink(path());
  return bind(sock, addr(), addrlen()) == 0;
}

void UnixServerSocket::CloseHook() { unlink(path()); }

}  // namespace base
