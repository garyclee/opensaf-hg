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

  MODULE NAME: rde.h

..............................................................................

  DESCRIPTION: Top level include file for RDE

..............................................................................

******************************************************************************
*/

#ifndef RDE_H
#define RDE_H

/*****************************************************************************\
 *                                                                             *
 *                          Required includes                                  *
 *                                                                             *
\*****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>

#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#include "ncs_svd.h"
#include "ncs_hdl_pub.h"
#include "ncssysf_lck.h"
#include "ncsusrbuf.h"
#include "ncssysf_def.h"
#include "ncssysfpool.h"
#include "ncssysf_tmr.h"
#include "ncssysf_mem.h"
#include "ncssysf_ipc.h"
#include "ncssysf_tsk.h"
#include "ncssysf_sem.h"
#include "ncspatricia.h"
#include "ncs_mds_def.h"

#include "ncs_queue.h"
#include "ncs_scktprm.h"

/* 
 * DTS header file 
 */
#include "dta_papi.h"
#include "dts_papi.h"
#include "rde_log.h"

/* 
 * Get General Definations 
 */
#include <ncsgl_defs.h>

/* 
 * Get target's suite of header files...
 */
#include "ncs_log.h"
#include "ncs_lib.h"
#include "nid_api.h"

#endif   /* RDE_H */
