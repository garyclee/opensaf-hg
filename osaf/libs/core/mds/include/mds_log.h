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
  DESCRIPTION:  MDS LOG header

******************************************************************************
*/

#ifndef _MDS_LOG_H
#define _MDS_LOG_H

#include <ncsgl_defs.h>
#include "ncs_lib.h"

/* Logging utility functions  - Phani */

extern uint32_t gl_mds_log_level;

#define NCSMDS_LC_CRITICAL       1
#define NCSMDS_LC_ERR            2
#define NCSMDS_LC_NOTIFY         3
#define NCSMDS_LC_INFO           4
#define NCSMDS_LC_DBG            5

#define m_MDS_LOG_CRITICAL  if (gl_mds_log_level < NCSMDS_LC_CRITICAL) {} else log_mds_critical

#define m_MDS_LOG_ERR if (gl_mds_log_level < NCSMDS_LC_ERR) {} else log_mds_err

#define m_MDS_LOG_NOTIFY if (gl_mds_log_level < NCSMDS_LC_NOTIFY) {} else log_mds_notify

#define m_MDS_LOG_INFO if (gl_mds_log_level < NCSMDS_LC_INFO) {} else log_mds_info

#define m_MDS_LOG_DBG if (gl_mds_log_level < NCSMDS_LC_DBG) {} else log_mds_dbg

extern void log_mds_dbg(char *fmt, ...);
extern void log_mds_info(char *fmt, ...);
extern void log_mds_notify(char *fmt, ...);
extern void log_mds_err(char *fmt, ...);
extern void log_mds_critical(char *fmt, ...);

#endif
