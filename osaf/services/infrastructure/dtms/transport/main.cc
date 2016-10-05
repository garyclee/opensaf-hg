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
#include <unistd.h>
#include <cstdlib>
#include "osaf/libs/core/common/include/daemon.h"
#include "osaf/libs/core/include/ncssysf_def.h"
#include "osaf/services/infrastructure/dtms/transport/transport_monitor.h"

constexpr static const int kDaemonStartWaitTimeInSeconds = 15;

int main(int argc, char** argv) {
  opensaf_reboot_prepare();
  daemonize(argc, argv);
  int term_fd;
  daemon_sigterm_install(&term_fd);
  TransportMonitor monitor{term_fd};
  pid_t pid = pid_t{0};
  if (!monitor.use_tipc()) {
    pid = monitor.WaitForDaemon("osafdtmd", kDaemonStartWaitTimeInSeconds);
  }
  const char* msg;
  if (pid != pid_t{-1}) {
    monitor.RotateMdsLogs(pid);
    msg = "osafdtmd Process down, Rebooting the node";
  } else {
    msg = "osafdtmd failed to start";
  }
  if (!monitor.Sleep(0)) {
    opensaf_reboot(0, nullptr, msg);
  } else {
    daemon_exit();
  }
  return EXIT_FAILURE;
}
