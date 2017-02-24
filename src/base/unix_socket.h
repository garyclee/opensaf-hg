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

#ifndef BASE_UNIX_SOCKET_H_
#define BASE_UNIX_SOCKET_H_

#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <cerrno>
#include <string>
#include "base/macros.h"

namespace base {

// A class implementing non-blocking operations on a UNIX domain socket. This
// class is not thread-safe and must be protected by a mutex if shared between
// multiple threads.
class UnixSocket {
 public:
  // Close the socket.
  virtual ~UnixSocket();
  // Returns the current file descriptor for this UNIX socket, or -1 if the
  // socket could not be opened. Note that the Send() and Recv() methods may
  // open and/or close the socket, and the file descriptor can therefore
  // potentially be change as a result of calling any of these two methods. If a
  // UnixSocket instance is shared between multiple threads (and therefore
  // protected with a mutex), this means that the result from the fd() method is
  // only valid for as long as the mutex protecting the instance is held.
  int fd() {
    int sock = fd_;
    if (sock < 0) sock = Open();
    return sock;
  }
  // Send a message in non-blocking mode. This call will open the socket if it
  // was not already open. The EINTR error code from the send() libc function is
  // handled by retrying the send() call in a loop. In case of other errors, -1
  // is returned and errno contains one of the codes listed in the recv() libc
  // function. The socket will be closed in case the error was not EINTR, EAGAIN
  // or EWOULDBLOCK.
  ssize_t Send(const void* buffer, size_t length) {
    int sock = fd();
    ssize_t result = -1;
    if (sock >= 0) {
      do {
        result = send(sock, buffer, length, MSG_NOSIGNAL);
      } while (result < 0 && errno == EINTR);
      if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) Close();
    }
    return result;
  }
  // Receive a message in non-blocking mode. This call will open the socket if
  // it was not already open. The EINTR error code from the recv() libc function
  // is handled by retrying the recv() call in a loop. In case of other errors,
  // -1 is returned and errno contains one of the codes listed in the recv()
  // libc function. The socket will be closed in case the error was not EINTR,
  // EAGAIN or EWOULDBLOCK.
  ssize_t Recv(void* buffer, size_t length) {
    int sock = fd();
    ssize_t result = -1;
    if (sock >= 0) {
      do {
        result = recv(sock, buffer, length, 0);
      } while (result < 0 && errno == EINTR);
      if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) Close();
    }
    return result;
  }

 protected:
  explicit UnixSocket(const std::string& path);
  virtual bool OpenHook(int sock);
  virtual void CloseHook();
  const struct sockaddr* addr() const {
    return reinterpret_cast<const struct sockaddr*>(&addr_);
  }
  static socklen_t addrlen() { return sizeof(addr_); }
  int get_fd() const { return fd_; }
  const char* path() const { return addr_.sun_path; }

 private:
  int Open();
  void Close();

  int fd_;
  struct sockaddr_un addr_;
  struct timespec last_failed_open_;
  int saved_errno_;

  DELETE_COPY_AND_MOVE_OPERATORS(UnixSocket);
};

}  // namespace base

#endif  // BASE_UNIX_SOCKET_H_
