/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
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
 * Author(s): Emerson Network Power
 *
 */

/*****************************************************************************
..............................................................................
  DESCRIPTION:  MDS LOG APIs

  ******************************************************************************
  */

#include "mds/mds_log.h"
#include <inttypes.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include "base/buffer.h"
#include "base/log_message.h"
#include "base/macros.h"
#include "base/ncsgl_defs.h"
#include "base/osaf_utility.h"
#include "base/time.h"
#include "base/unix_client_socket.h"
#include "mds/mds_dt2c.h"
#include "mds/mds_papi.h"
#include "osaf/configmake.h"

class MdsLog {
 public:
  static bool Init();
  static void Log(base::LogMessage::Severity severity, const char *fmt,
                  va_list ap);

 private:
  MdsLog(const char* host_name, const char* app_name,
            uint32_t proc_id, const char* socket_name);
  void LogInternal(base::LogMessage::Severity severity, const char *fmt,
                   va_list ap);
  static MdsLog* instance_;
  const base::LogMessage::HostName host_name_;
  const base::LogMessage::AppName app_name_;
  const base::LogMessage::ProcId proc_id_;
  std::atomic<uint64_t> msg_id_;
  base::UnixClientSocket log_socket_;

  DELETE_COPY_AND_MOVE_OPERATORS(MdsLog);
};

int gl_mds_log_level = 3;
MdsLog* MdsLog::instance_ = nullptr;

MdsLog::MdsLog(const char* host_name, const char* app_name,
                     uint32_t proc_id, const char* socket_name) :
    host_name_{base::LogMessage::HostName{host_name}},
    app_name_{base::LogMessage::AppName{app_name}},
    proc_id_{base::LogMessage::ProcId{std::to_string(proc_id)}},
    msg_id_{0},
    log_socket_{socket_name} {
}

/*****************************************************
 Function NAME: get_process_name()
 Returns : <process_name>[<pid> or <tipc_port_ref>]
*****************************************************/
bool MdsLog::Init() {
  if (instance_ != nullptr) return false;
  char app_name[MDS_MAX_PROCESS_NAME_LEN];
  char node_name[256];
  char pid_path[1024];
  uint32_t process_id = getpid();
  char *token, *saveptr;
  char *pid_name = nullptr;
  const char *node_name_file = PKGSYSCONFDIR "/node_name";

  snprintf(pid_path, sizeof(pid_path), "/proc/%" PRIu32 "/cmdline", process_id);
  FILE* f = fopen(pid_path, "r");
  if (f != nullptr) {
    size_t size;
    size = fread(pid_path, sizeof(char), 1024, f);
    if (size > 0) {
      if ('\n' == pid_path[size-1])
        pid_path[size-1] = '\0';
    }
    fclose(f);
  } else {
    pid_path[0] = '\0';
  }
  token = strtok_r(pid_path, "/", &saveptr);
  while (token != nullptr) {
    pid_name = token;
    token = strtok_r(nullptr, "/", &saveptr);
  }
  if (snprintf(app_name, sizeof(app_name), "%s", pid_name) < 0) {
    app_name[0] = '\0';
  }
  FILE* fp = fopen(node_name_file, "r");
  if (fp != nullptr) {
    if (fscanf(fp, "%255s", node_name) != 1) node_name[0] = '\0';
    fclose(fp);
  } else {
    node_name[0] = '\0';
  }
  instance_ = new MdsLog{node_name, app_name, process_id,
                         PKGLOCALSTATEDIR "/mds_log.sock"};
  return instance_ != nullptr;
}

void MdsLog::Log(base::LogMessage::Severity severity, const char *fmt,
                 va_list ap) {
  if (instance_ != nullptr) instance_->LogInternal(severity, fmt, ap);
}

void MdsLog::LogInternal(base::LogMessage::Severity severity, const char *fmt,
                         va_list ap) {
  uint64_t id = msg_id_++;
  base::Buffer<256> buffer;
  base::LogMessage::Write(base::LogMessage::Facility::kLocal0,
                          severity,
                          base::ReadRealtimeClock(),
                          host_name_,
                          app_name_,
                          proc_id_,
                          base::LogMessage::MsgId{std::to_string(id)},
                          {},
                          fmt,
                          ap,
                          &buffer);
  log_socket_.Send(buffer.data(), buffer.size());
}

/*******************************************************************************
 * Funtion Name   :    mds_log_init
 *
 * Purpose        :
 *
 * Return Value   :    None
 *
 *******************************************************************************/
uint32_t mds_log_init(const char*) {
  if (!MdsLog::Init()) return NCSCC_RC_FAILURE;
  tzset();
  log_mds_notify("BEGIN MDS LOGGING| ARCHW=%x|64bit=%zu\n",
                 MDS_SELF_ARCHWORD, MDS_WORD_SIZE_TYPE);
  return NCSCC_RC_SUCCESS;
}

/*******************************************************************************
 * Funtion Name   :    log_mds_critical
 *
 * Purpose        :
 *
 * Return Value   :    None
 *
 *******************************************************************************/
void log_mds_critical(const char *fmt, ...) {
  if (gl_mds_log_level < NCSMDS_LC_CRITICAL) return;
  va_list ap;
  va_start(ap, fmt);
  MdsLog::Log(base::LogMessage::Severity::kCrit, fmt, ap);
  va_end(ap);
}

/*******************************************************************************
 * Funtion Name   :    log_mds_err
 *
 * Purpose        :
 *
 * Return Value   :    None
 *
 *******************************************************************************/
void log_mds_err(const char *fmt, ...) {
  if (gl_mds_log_level < NCSMDS_LC_ERR) return;
  va_list ap;
  va_start(ap, fmt);
  MdsLog::Log(base::LogMessage::Severity::kErr, fmt, ap);
  va_end(ap);
}

/*******************************************************************************
 * Funtion Name   :    log_mds_notify
 *
 * Purpose        :
 *
 * Return Value   :    None
 *
 *******************************************************************************/
void log_mds_notify(const char *fmt, ...) {
  if (gl_mds_log_level < NCSMDS_LC_NOTIFY) return;
  va_list ap;
  va_start(ap, fmt);
  MdsLog::Log(base::LogMessage::Severity::kNotice, fmt, ap);
  va_end(ap);
}

/*******************************************************************************
 * Funtion Name   :    log_mds_info
 *
 * Purpose        :
 *
 * Return Value   :    None
 *
 *******************************************************************************/
void log_mds_info(const char *fmt, ...) {
  if (gl_mds_log_level < NCSMDS_LC_INFO) return;
  va_list ap;
  va_start(ap, fmt);
  MdsLog::Log(base::LogMessage::Severity::kInfo, fmt, ap);
  va_end(ap);
}

/*******************************************************************************
 * Funtion Name   :    log_mds_dbg
 *
 * Purpose        :
 *
 * Return Value   :    NCSCC_RC_SUCCESS
 *                     NCSCC_RC_FAILURE
 *
 *******************************************************************************/
void log_mds_dbg(const char *fmt, ...) {
  if (gl_mds_log_level < NCSMDS_LC_DBG) return;
  va_list ap;
  va_start(ap, fmt);
  MdsLog::Log(base::LogMessage::Severity::kDebug, fmt, ap);
  va_end(ap);
}
