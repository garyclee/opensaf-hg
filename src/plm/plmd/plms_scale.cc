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
 * Author(s): Genband
 *
 */

/*****************************************************************************
 *   FILE NAME: plms_scale.cc
 *
 *   DESCRIPTION: C++ implementation of EE scaling
 *
 ****************************************************************************/

#include "plm/common/plms_scale.h"
#include "plm/common/plms.h"

extern "C" SaUint32T plms_scale(const PLMS_PLMC_EVT *evt) {
  TRACE_ENTER();

  SaUint32T rc(NCSCC_RC_SUCCESS);

  static bool init(false);
  static bool scalingEnabled(false);
  static PlmsScaleThread *scaleThread(0);

  if (!init) {
    char *scale_out_exe(getenv("OPENSAF_PLMS_CLUSTERAUTO_SCALE"));

    if (scale_out_exe) {
      scalingEnabled = true;
      scaleThread = new PlmsScaleThread(scale_out_exe);
    }

    init = true;
  }

  if (scalingEnabled) {
    if (evt->plmc_evt_type == PLMS_PLMC_EE_INSTING ||
        evt->plmc_evt_type == PLMS_PLMC_EE_INSTED ||
        evt->plmc_evt_type == PLMS_PLMC_EE_TCP_CONCTED) {
      // need to put the request on a thread so PLMs can service any IMM
      // requests
      PlmsScaleThread::Node node;
      node.eeName = std::string(
          reinterpret_cast<const char *>(evt->ee_id.value), evt->ee_id.length);

      TRACE("adding node %s to thread", node.eeName.c_str());

      std::thread addNodeThread(&PlmsScaleThread::add, scaleThread, node);

      addNodeThread.detach();
    }
  } else {
    TRACE("scaling not enabled");
    rc = NCSCC_RC_FAILURE;
  }

  TRACE_LEAVE();

  return rc;
}

PlmsScaleThread::PlmsScaleThread(const std::string &outFile)
    : std::thread(_main, this), scaleOutFile(outFile) {
  TRACE_ENTER();
  TRACE_LEAVE();
}

PlmsScaleThread::~PlmsScaleThread(void) {
  TRACE_ENTER();
  TRACE_LEAVE();
}

void PlmsScaleThread::main(void) {
  TRACE_ENTER2("Scale thread starting");

  while (true) {
    std::unique_lock<std::mutex> lk(nodeListMutex);

    while (nodeList.empty()) nodeListCv.wait(lk);

    int argc(1);
    char **argv(new char *[nodeList.size() + argc + 1]);
    argv[0] = new char[scaleOutFile.length() + 1];
    snprintf(argv[0], scaleOutFile.length() + 1, "%s", scaleOutFile.c_str());

    // call the scale script with the EEs that are trying to come in
    for (NodeList::iterator it(nodeList.begin()); it != nodeList.end(); ++it) {
      argv[argc] = new char[it->eeName.length() + 1];
      snprintf(argv[argc++], it->eeName.length() + 1, "%s", it->eeName.c_str());
    }

    // null terminate argv
    argv[nodeList.size() + 1] = 0;

    executeScaleScript(argc, argv);

    for (int i(argc); i >= 0; i--) delete[] argv[i];

    delete[] argv;

    nodeList.clear();
  }

  TRACE_LEAVE2("Scale thread exiting");
}

void PlmsScaleThread::executeScaleScript(int argc, char **argv) {
  TRACE_ENTER();

  LOG_IN("executing scale script for %i nodes", argc - 1);

  pid_t pid(fork());

  if (pid > 0) {
    // parent
    int status;
    pid_t wait_pid;

    do {
      wait_pid = waitpid(pid, &status, 0);
    } while (wait_pid == -1 && errno == EINTR);

    if (wait_pid != -1) {
      if (!WIFEXITED(status)) {
        LOG_ER("Scale out script %s terminated abnormally", argv[0]);
      } else if (WEXITSTATUS(status) != 0) {
        if (WEXITSTATUS(status) == 123) {
          LOG_ER("Scale out script %s could not be executed", argv[0]);
        } else {
          LOG_ER("Scale out script %s failed with exit code %d", argv[0],
                 WEXITSTATUS(status));
        }
      } else {
        LOG_IN("Scale out script %s exited successfully", argv[0]);
      }
    } else {
      LOG_ER("Scale out script %s failed in waitpid(%i): %s", argv[0], wait_pid,
             strerror(errno));
    }
  } else if (pid == 0) {
    // child
    static char scaleOutPathEnv[] =
        "PATH=/usr/local/sbin:/usr/local/bin:"
        "/usr/sbin:/usr/bin:/sbin:/bin";

    char *env[] = {scaleOutPathEnv, 0};

    const int nofile(1024);

    for (int fd(3); fd < nofile; ++fd) close(fd);

    if (execve(argv[0], argv, env) < 0)
      LOG_ER("error executing plms_scale_out script: %s: %i",
             scaleOutFile.c_str(), errno);
    _Exit(123);
  } else {
    LOG_ER("unable to fork plms_scale_out script: %s: %i", scaleOutFile.c_str(),
           errno);
  }

  TRACE_LEAVE();
}

void PlmsScaleThread::add(const Node node) {
  TRACE_ENTER();

  std::lock_guard<std::mutex> guard(nodeListMutex);

  nodeList.insert(node);

  nodeListCv.notify_one();

  TRACE_LEAVE();
}
