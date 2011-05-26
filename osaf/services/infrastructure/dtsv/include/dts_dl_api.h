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

..............................................................................

  DESCRIPTION:

 _Public_ Flex Log Server (DTS) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef DTS_DL_API_H
#define DTS_DL_API_H

uns32 dts_lib_req(NCS_LIB_REQ_INFO *req_info);

uns32 dts_apps_ascii_spec_load(uint8_t *file_name, uns32 what_to_do);

NCSCONTEXT dts_ascii_spec_load(char *svc_name, uns16 version, DTS_SPEC_ACTION action);

uns32 dts_lib_init(NCS_LIB_REQ_INFO *req_info);

uns32 dts_lib_destroy(void);

/* Extern declarations for console printing functions */
void dts_cons_init(void);
int32 dts_cons_open(uns32 mode);

/* Declaration for signal handler function */
typedef void (*SIG_HANDLR) (int);
int32 dts_app_signal_install(int i_sig_num, SIG_HANDLR i_sig_handler);

/* Defines for Console printing */
#define DTS_CNSL "/dev/console"
#define DTS_VT_MASTER "/dev/tty0"

/* Defines for lib & function name for ASCII Spec table loading */
#define DTS_MAX_LIBNAME   255
#define DTS_MAX_FUNCNAME  255
#define DTS_MAX_LIB_DBG   300

#if (DTS_FLOW == 1)

/* Define threshold value for DTS mailbox */
#define DTS_MAX_THRESHOLD 30000
#define DTS_AVG_THRESHOLD 27000
#define DTS_MIN_THRESHOLD 20000
#endif

#endif   /* DTS_DL_API_H */
