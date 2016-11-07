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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "osaf/services/infrastructure/dtms/transport/transport_monitor.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include "logtrace.h"
#include "osaf_time.h"
#include "osaf/libs/core/common/include/osaf_poll.h"
#include "osaf/libs/core/cplusplus/base/getenv.h"
#include "osaf/libs/core/cplusplus/base/file_notify.h"


TransportMonitor::TransportMonitor(int term_fd)
    : term_fd_{term_fd},
      pkgpiddir_{base::GetEnv<std::string>("pkgpiddir", PKGPIDDIR)},
      proc_path_{"/proc/"},
      mds_log_file_{base::GetEnv<std::string>("pkglogdir", PKGLOGDIR)
          + "/mds.log"},
      old_mds_log_file_{mds_log_file_ + ".old"},
      use_tipc_{base::GetEnv<std::string>("MDS_TRANSPORT", "TCP") == "TIPC"} {
}

TransportMonitor::~TransportMonitor() {
}

pid_t TransportMonitor::WaitForDaemon(const std::string& daemon_name,
                                      int64_t seconds_to_wait) {
  std::vector<int> user_fds {term_fd_};
  std::string pidfile = pkgpiddir_ + "/" + daemon_name + ".pid";
  pid_t pid = pid_t{-1};
  base::FileNotify file_notify;
  base::FileNotify::FileNotifyErrors rc =
      file_notify.WaitForFileCreation(pidfile, user_fds,
                                      seconds_to_wait * kMillisPerSec);

  if (rc == base::FileNotify::FileNotifyErrors::kOK) {
    if (!(std::ifstream{pidfile} >> pid).fail()) {
      if (IsDir(proc_path_ + std::to_string(pid))) {
        return pid;
      }
    }
  }
  return pid_t{-1};
}

bool TransportMonitor::Sleep(int64_t seconds_to_wait) {
  return osaf_poll_one_fd(term_fd_, seconds_to_wait * kMillisPerSec) != 0;
}

bool TransportMonitor::IsDir(const std::string& path) {
  struct stat stat_buf;
  int stat_result = stat(path.c_str(), &stat_buf);
  return stat_result == 0 && S_ISDIR(stat_buf.st_mode) != 0;
}

void TransportMonitor::RotateMdsLogs(pid_t pid_to_watch) {
  int fifo_fd{-1};
  std::vector<int> user_fds {term_fd_};
  base::FileNotify file_notify;
  std::string pid_path{proc_path_ + std::to_string(pid_to_watch)};

  if (pid_to_watch != pid_t{0}) {
    do {
      fifo_fd = open(fifo_file_.c_str(), O_WRONLY|O_NONBLOCK);
    } while (fifo_fd == -1 && errno == EINTR);

    if (fifo_fd == -1) {
      LOG_WA("open fifo file failed");
    } else {
      user_fds.emplace_back(fifo_fd);
    }
  }

  for (;;) {
    if (FileSize(mds_log_file_) > kMaxFileSize) {
      unlink(old_mds_log_file());
      rename(mds_log_file(), old_mds_log_file());
    }
    if (pid_to_watch != pid_t{0}) {
      base::FileNotify::FileNotifyErrors rc =
          file_notify.WaitForFileDeletion(
              pid_path,
              user_fds, kLogRotationIntervalInSeconds * kMillisPerSec);
      if (rc != base::FileNotify::FileNotifyErrors::kTimeOut) {
        close(fifo_fd);
        return;
      } else {
        TRACE("Timeout received, continuing...");
      }
    } else {
      if (Sleep(kLogRotationIntervalInSeconds)) {
        return;
      }
    }
  }
}

uint64_t TransportMonitor::FileSize(const std::string& path) {
  struct stat stat_buf;
  uint64_t file_size;
  if (stat(path.c_str(), &stat_buf) == 0 && S_ISREG(stat_buf.st_mode) != 0) {
    file_size = static_cast<uint64_t>(stat_buf.st_blocks) * 512;
  } else {
    file_size = 0;
  }
  return file_size;
}
