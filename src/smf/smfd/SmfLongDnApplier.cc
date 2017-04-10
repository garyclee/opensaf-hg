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

// TODO(elunlen): Include directory for .h files when new directory structure
// is in place
#include <thread>
#include <atomic>
#include <memory>

#include "smf/smfd/SmfLongDnApplier.h"
#include "smf/smfd/SmfImmApplierHdl.h"
#include "smf/smfd/smfd.h"
#include "smf/smfd/SmfUtils.h"

#include "base/saf_error.h"
#include "base/logtrace.h"
#include "base/osaf_utility.h"
#include "base/osaf_time.h"
#include "base/osaf_poll.h"
#include "base/time.h"

/**
 * Local definitions not needed outside of this file
 */

/* Inter thread communication (via socket pair) */
static const char* CMD_STOP = "s";
static const char* CMD_START = "a";
static const char* CMD_REMOVE = "r";
static const size_t CMD_SIZE = 2;

/* States for the applier thread. Stopping and starting the applier is handled
 * differently dependent on the thread state
 */
enum class th_state {
  /* Thread not started */
  TH_NOT_CREATED = 0,
  /* Thread is started but has not yet entered the event handling poll loop */
  TH_CREATING,
  /* Thread is fully started and running as applier */
  TH_IS_APPLIER,
  /* Thread is fully started but is not running as applier. In this state
   * it can service commands e.g. start applier command but has never
   * become an applier or have given up the applier role
   */
  TH_IDLE,
  /* Thread is terminating */
  TH_TERMINATING
};

static std::atomic<th_state> th_state_info{th_state::TH_NOT_CREATED};

/**
 * These functions should be here so that also TRACEes can be made in the future
 * that prints the thread command and states as strings for better readability
 */
// cppcheck-suppress unusedFunction
const char* th_statestr(th_state state) {
  switch (state) {
    case th_state::TH_CREATING:
      return "CREATING";
    case th_state::TH_IDLE:
      return "IDLE";
    case th_state::TH_IS_APPLIER:
      return "IS APPLIER";
    case th_state::TH_NOT_CREATED:
      return "NOT_CREATED";
    case th_state::TH_TERMINATING:
      return "TERMINATING";
    default:
      return "UNKNOWN STATE";
  }
}

// cppcheck-suppress unusedFunction
const char* th_cmdstr(th_cmd cmd) {
  switch (cmd) {
    case th_cmd::AP_REMOVE:
      return "REMOVE";
    case th_cmd::AP_START:
      return "START";
    case th_cmd::AP_STOP:
      return "STOP";
    default:
      return "UNKNOWN CMD";
  }
}

// The thread function
// This is the thread where the applier is running.
// For communication with the thread a socket pair is used. Input
// command_socket shall contain the fd to be used in the thread
// Thread can be stopped if CMD_REMOVE command or if applier error.
// If the thread is stopped the applier handler is removed and thread state is
// set to TH_NOT_STARTED
// Note:
// This function is started as a thread using C++11 thread(). Parameters cannot
// be passed as reference in the "normal" way. Also this function is used once
//
// cppcheck-suppress passedByValue
static void ApplierThread(const std::string object_name,
                          // cppcheck-suppress passedByValue
                          const std::string attribute_name,
                          SmfImmApplierHdl* ApplierHdl,
                          const int command_socket) {
  /* Event handling
   * Note:
   * IMM selection object can only be part of polling when IMM OI is
   * initialized. If IMM is finalized the selection object is not valid.
   * This is handled by placing IMM selection fd as last element in fds
   * vector and decrease and decrease nfds when IMM is finalized.
   */
  enum {
    FDA_COM = 0, /* Communication events */
    FDA_IMM,     /* IMM events */
    NUM_FDA
  };

  thread_local struct pollfd fds[NUM_FDA];
  ExecuteImmCallbackReply execute_rc = ExecuteImmCallbackReply::kOk;

  TRACE("%s: %s, %s, %d", __FUNCTION__, object_name.c_str(),
        attribute_name.c_str(), command_socket);

  th_state_info = th_state::TH_CREATING;

  // Create the applier
  ApplierSetupInfo setup_info{object_name, attribute_name};
  bool create_rc = ApplierHdl->Create(setup_info);
  if (create_rc == false) {
    LOG_WA("%s: Creation of long DN applier Fail", __FUNCTION__);
    th_state_info = th_state::TH_NOT_CREATED;
    return;  // Terminate the thread
  }

  // Setup event handling
  fds[FDA_IMM].fd = ApplierHdl->get_selection_object();
  fds[FDA_IMM].events = POLLIN;
  fds[FDA_COM].fd = command_socket;
  fds[FDA_COM].events = POLLIN;
  thread_local nfds_t nfds = NUM_FDA;

  th_state_info = th_state::TH_IDLE;

  TRACE("%s: Applier is created, th_state_info = %s", __FUNCTION__,
        th_statestr(th_state_info));

  while (1) {
    (void)osaf_poll(fds, nfds, -1);

    // Handle IMM callback
    if (fds[FDA_IMM].revents & POLLIN) {
      TRACE("%s: IMM event", __FUNCTION__);

      // Note: ExecuteImmCallback can "hang" for up to one minute if IMM
      // has "invalidated" the OI handle and ExecuteImmCallback has to restore
      // the applier. Such a restore can be canceled and that must be done
      // before sending a stop command to this thread
      execute_rc = ApplierHdl->ExecuteImmCallback();
      if (execute_rc == ExecuteImmCallbackReply::kGetNewSelctionObject) {
        fds[FDA_IMM].fd = ApplierHdl->get_selection_object();
      } else if (execute_rc == ExecuteImmCallbackReply::kError) {
        LOG_WA("%s: ExecuteImmCallback Error", __FUNCTION__);
        th_state_info = th_state::TH_NOT_CREATED;
        break;  // Thread exit
      }
    }

    // Handle commands
    if (fds[FDA_COM].revents & POLLIN) {
      TRACE("%s: COM event", __FUNCTION__);

      char cmd_str[256] = {0};
      int read_rc = 0;
      while (1) {
        read_rc = read(command_socket, cmd_str, 256);
        if ((read_rc == -1) && (errno == EINTR)) {
          // Try again
          continue;
        } else {
          break;
        }
      }

      if (read_rc == -1) {
        LOG_WA("%s: Read socket Fail %s", __FUNCTION__, strerror(errno));
        break;  // Terminate thread
      }

      TRACE("%s: Command is: '%s'", __FUNCTION__, cmd_str);

      if (strcmp(cmd_str, CMD_STOP) == 0) {
        TRACE("%s: STOP received", __FUNCTION__);
        if (ApplierHdl->Stop() == false) {
          LOG_WA("%s: Applier.Stop() Fail", __FUNCTION__);
          th_state_info = th_state::TH_NOT_CREATED;
          ApplierHdl->Remove();
          break;  // Terminate thread
        } else {
          th_state_info = th_state::TH_IDLE;
        }
      } else if (strcmp(cmd_str, CMD_START) == 0) {
        TRACE("%s: START received", __FUNCTION__);
        if (ApplierHdl->Start() == false) {
          LOG_NO("%s: Applier.Start() Fail", __FUNCTION__);
          th_state_info = th_state::TH_TERMINATING;
          ApplierHdl->Remove();
          break;  // Terminate thread
        } else {
          th_state_info = th_state::TH_IS_APPLIER;
        }
      } else if (strcmp(cmd_str, CMD_REMOVE) == 0) {
        TRACE("%s: EXIT received", __FUNCTION__);
        th_state_info = th_state::TH_TERMINATING;
        ApplierHdl->Remove();
        break;  // Thread exit
      }
    }
  } /* END Event handling loop */
  LOG_NO("%s: Thread is terminated", __FUNCTION__);
  th_state_info = th_state::TH_NOT_CREATED;
}

// ===================
// SmLongDnApplier class
// ===================
SmfLongDnApplier::SmfLongDnApplier(const std::string& object_name,
                                   const std::string& attribute_name)
    : object_name_{object_name}, attribute_name_{attribute_name} {
  TRACE_ENTER();
  applierHdl_ = new SmfImmApplierHdl();
  TRACE_LEAVE();
}

SmfLongDnApplier::~SmfLongDnApplier() {
  // Stop/Finalize the applier including thread termination
  TRACE_ENTER2();
  Remove();
  delete applierHdl_;
  TRACE_LEAVE2();
}

void SmfLongDnApplier::Start() {
  TRACE_ENTER();
  if ((th_state_info == th_state::TH_IDLE) ||
      (th_state_info == th_state::TH_CREATING)) {
    TRACE("%s: Sending START to thread", __FUNCTION__);
    SendCommandToThread(th_cmd::AP_START);
  } else {
    LOG_WA("%s: Trying to start an applier that is not created", __FUNCTION__);
    // Try to recover by creating and start again. We may end up in this
    // situation if creation or IMM bad handle recovery was canceled by a
    // stop request
    Create();
    if ((th_state_info == th_state::TH_IDLE) ||
        (th_state_info == th_state::TH_CREATING)) {
      TRACE("%s: Re-sending START to thread", __FUNCTION__);
      SendCommandToThread(th_cmd::AP_START);
    } else {
      LOG_ER(
          "%s: Cannot create and start "
          "the long dn monitoring applier",
          __FUNCTION__);
    }
  }
  TRACE_LEAVE();
}

void SmfLongDnApplier::Stop() {
  TRACE_ENTER();
  // Try to stop applier only if an applier is running
  if (th_state_info == th_state::TH_IS_APPLIER) {
    TRACE("%s: Sending STOP to thread", __FUNCTION__);
    // If there is an ongoing IMM initialization TRY AGAIN loop it must be
    // canceled so that the stop command can be executed immediately
    applierHdl_->CancelCreate();
    SendCommandToThread(th_cmd::AP_STOP);

    // Wait until thread is in idle state before returning
    // This guarantees that the applier is stopped and can be started
    // on another node.
    base::Timer applier_stop_timer(1000);  // ms
    while (applier_stop_timer.is_timeout() == false) {
      if (th_state_info != th_state::TH_IS_APPLIER) break;  // Is stopped
      base::Sleep(base::kTenMilliseconds);
    }
    if (th_state_info == th_state::TH_IS_APPLIER) {
      // Timeout: This should never happen
      LOG_ER("%s: Applier cold not be stopped within 1 sec", __FUNCTION__);
    }
    TRACE("%s: STOP done. Thread state is %s", __FUNCTION__,
          th_statestr(th_state_info));
  } else {
    LOG_WA("%s: Trying to stop an applier that is not running", __FUNCTION__);
  }
  TRACE_LEAVE();
}

void SmfLongDnApplier::Create() {
  TRACE_ENTER();
  // Do not try to create if already created is creating or is terminating
  if (th_state_info == th_state::TH_NOT_CREATED) {
    // Create socket pair for communication with applier thread
    // Note: Non block is used. This makes it possible to handle EAGAIN as an
    // error. Commands shall not be queued so the buffer should never be full
    int sock_fd[2] = {0};
    int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fd);
    if (rc == -1) {
      LOG_ER("%s: socketpair() Fail %s", __FUNCTION__, strerror(errno));
      osaf_abort(0);
    }

    fcntl(sock_fd[0], F_SETFL, O_NONBLOCK);
    fcntl(sock_fd[1], F_SETFL, O_NONBLOCK);
    socket_fd_thread_ = sock_fd[0];
    socket_fd_command_ = sock_fd[1];

    // Start the thread
    std::thread t1{ApplierThread, object_name_, attribute_name_, applierHdl_,
                   socket_fd_thread_};
    t1.detach();
    TRACE("%s: Thread is started", __FUNCTION__);

    // For synchronization, Wait for thread to enter CREATING state
    // Note: Thread shall enter CREATING state immediately after it has been
    // started
    int i = 0;
    for (i = 0; i < 10; i++) {
      if (th_state_info != th_state::TH_NOT_CREATED) break;
      base::Sleep(base::kTenMilliseconds);
    }
    if (th_state_info == th_state::TH_NOT_CREATED) {
      LOG_WA("%s: Applier thread has not started within 100 ms", __FUNCTION__);
    }
  } else {
    LOG_WA("%s: Trying to create an already created applier", __FUNCTION__);
  }
  TRACE_LEAVE();
}

// --------------------
// The private method's
// --------------------

// Terminates the applier by sending a remove command to the thread
// The thread will remove the applier and terminate
void SmfLongDnApplier::Remove() {
  TRACE_ENTER();
  // First check if there is any applier to terminate
  if (th_state_info == th_state::TH_NOT_CREATED) {
    LOG_NO("%s: Trying to remove an applier that is not created", __FUNCTION__);
    return;
  }

  // Finalize the applier and terminate the thread
  applierHdl_->CancelCreate();
  SendCommandToThread(th_cmd::AP_REMOVE);

  TRACE("%s: REMOVE command sent. Waiting for termination of thread",
        __FUNCTION__);
  // Wait for thread to terminate. For synchronizing
  base::Timer terminate_timeout(10000);  // ms
  while (terminate_timeout.is_timeout() == false) {
    if (th_state_info == th_state::TH_NOT_CREATED)
      break;  // Thread has terminated
    base::Sleep(base::kOneHundredMilliseconds);
  }
  if (th_state_info != th_state::TH_NOT_CREATED) {
    // Should not be possible
    LOG_ER("%s: Applier thread termination Failed. Thread state is '%s'",
           __FUNCTION__, th_statestr(th_state_info));
    osaf_abort(0);
  }
  TRACE("%s: Thread has terminated thread state is %s", __FUNCTION__,
        th_statestr(th_state_info));

  CloseSockets();
  TRACE_LEAVE();
}

void SmfLongDnApplier::CloseSockets() {
  TRACE_ENTER();
  if (socket_fd_thread_ != 0) {
    close(socket_fd_thread_);
    socket_fd_thread_ = 0;
  }

  if (socket_fd_command_ != 0) {
    close(socket_fd_command_);
    socket_fd_command_ = 0;
  }
  TRACE_LEAVE();
}

void SmfLongDnApplier::SendCommandToThread(th_cmd command) {
  const char* cmd_str = nullptr;
  int rc = 0;

  TRACE_ENTER2("Command %s", th_cmdstr(command));

  switch (command) {
    case th_cmd::AP_START:
      cmd_str = CMD_START;
      break;
    case th_cmd::AP_STOP:
      cmd_str = CMD_STOP;
      break;
    case th_cmd::AP_REMOVE:
      cmd_str = CMD_REMOVE;
      break;
    default:
      LOG_ER("%s: Unknown command", __FUNCTION__);
      osaf_abort(0);
  }

  while (1) {
    rc = write(socket_fd_command_, cmd_str, CMD_SIZE);
    if ((rc == -1) && (errno == EINTR)) /* Try again */
      continue;
    else
      break;
  }

  if (rc == -1) {
    LOG_ER("%s: Write Fail %s", __FUNCTION__, strerror(errno));
    osaf_abort(0);
  }

  TRACE_LEAVE();
}
