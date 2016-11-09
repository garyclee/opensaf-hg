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

#include "osaf/libs/core/mds/include/mds_log.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "osaf/libs/core/include/mds_papi.h"
#include "osaf/libs/core/include/ncsgl_defs.h"
#include "osaf/libs/core/mds/include/mds_dt2c.h"

constexpr static const size_t kMaxMdsFileNameLen = 256;

static void log_mds(const char *str);

int gl_mds_log_level = 3;
static char *lf = nullptr;
static char process_name[MDS_MAX_PROCESS_NAME_LEN];
static char mds_log_fname[kMaxMdsFileNameLen];

/*****************************************************
 Function NAME: get_process_name()
 Returns : <process_name>[<pid> or <tipc_port_ref>]
*****************************************************/
static void get_process_name() {
  char pid_path[1024];
  uint32_t process_id = getpid();
  char *token, *saveptr;
  char *pid_name = nullptr;

  snprintf(pid_path, sizeof(pid_path), "/proc/%d/cmdline", process_id);
  FILE* f = fopen(pid_path, "r");
  if (f) {
    size_t size;
    size = fread(pid_path, sizeof(char), 1024, f);
    if (size > 0) {
      if ('\n' == pid_path[size-1])
        pid_path[size-1] = '\0';
    }
    fclose(f);
  }
  token = strtok_r(pid_path, "/", &saveptr);
  while (token != nullptr) {
    pid_name = token;
    token = strtok_r(nullptr, "/", &saveptr);
  }
  snprintf(process_name, MDS_MAX_PROCESS_NAME_LEN, "%s[%d]", pid_name,
           process_id);
  return;
}
/*******************************************************************************
 * Funtion Name   :    mds_log_init
 *
 * Purpose        :
 *
 * Return Value   :    None
 *
 *******************************************************************************/
uint32_t mds_log_init(const char *log_file_name) {
  FILE *fh;
  memset(process_name, 0, MDS_MAX_PROCESS_NAME_LEN);
  tzset();
  get_process_name();

  if (lf != nullptr)
    return NCSCC_RC_FAILURE;

  if (strlen(log_file_name) >= kMaxMdsFileNameLen)
    return NCSCC_RC_FAILURE;

  snprintf(mds_log_fname, sizeof(mds_log_fname), "%s", log_file_name);

  lf = mds_log_fname;

  if ((fh = fopen(lf, "a+")) != nullptr) {
    fclose(fh);
    log_mds_notify("BEGIN MDS LOGGING| PID=<%s> | ARCHW=%x|64bit=%zu\n",
                   process_name, MDS_SELF_ARCHWORD, MDS_WORD_SIZE_TYPE);
  }

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
  char str[MDS_MAX_PROCESS_NAME_LEN + 32];
  int i;
  va_list ap;

  i = snprintf(str, sizeof(str), "%s CRITICAL  |", process_name);
  va_start(ap, fmt);
  vsnprintf(str + i, sizeof(str) - i, fmt, ap);
  va_end(ap);
  log_mds(str);
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
  char str[MDS_MAX_PROCESS_NAME_LEN + 32];
  int i;
  va_list ap;

  i = snprintf(str, sizeof(str), "%s ERR  |", process_name);
  va_start(ap, fmt);
  vsnprintf(str + i, sizeof(str) - i, fmt, ap);
  va_end(ap);
  log_mds(str);
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
  char str[MDS_MAX_PROCESS_NAME_LEN + 32];
  int i;
  va_list ap;

  i = snprintf(str, sizeof(str), "%s NOTIFY  |", process_name);
  va_start(ap, fmt);
  vsnprintf(str + i, sizeof(str) - i, fmt, ap);
  va_end(ap);
  log_mds(str);
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
  char str[MDS_MAX_PROCESS_NAME_LEN + 32];
  int i;
  va_list ap;

  i = snprintf(str, sizeof(str), "%s INFO  |", process_name);
  va_start(ap, fmt);
  vsnprintf(str + i, sizeof(str) - i, fmt, ap);
  va_end(ap);
  log_mds(str);
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
  char str[MDS_MAX_PROCESS_NAME_LEN + 32];
  int i;
  va_list ap;

  i = snprintf(str, sizeof(str), "%s DBG  |", process_name);
  va_start(ap, fmt);
  vsnprintf(str + i, sizeof(str) - i, fmt, ap);
  va_end(ap);
  log_mds(str);
}

/*******************************************************************************
 * Funtion Name   :    log_mds
 *
 * Purpose        :
 *
 * Return Value   :    None
 *
 *******************************************************************************/
static void log_mds(const char *str) {
  FILE *fp;

  if (lf != nullptr && ((fp = fopen(lf, "a+")) != nullptr)) {
    struct tm tm_info;
    struct timeval tv;
    char asc_tod[128];
    char log_string[512];
    int i;

    gettimeofday(&tv, nullptr);
    struct tm* tstamp_data = localtime_r(&tv.tv_sec, &tm_info);
    osafassert(tstamp_data);

    strftime(asc_tod, sizeof(asc_tod), "%b %e %k:%M:%S", tstamp_data);
    i = snprintf(log_string, sizeof(log_string), "%s.%06ld %s",
                 asc_tod, tv.tv_usec, str);

    if (i < 0) {
      i = 1;
      log_string[i - 1] = '\n';
    } else if (static_cast<size_t>(i) >= sizeof(log_string)) {
      i = sizeof(log_string);
      log_string[i - 1] = '\n';
    } else if (log_string[i - 1] != '\n') {
      log_string[i] = '\n';
      i++;
    }

    if (fwrite(log_string, 1, i, fp) != static_cast<size_t>(i)) {
      fclose(fp);
      return;
    }

    fclose(fp);
  }
}
