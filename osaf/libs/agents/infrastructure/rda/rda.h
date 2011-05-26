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

  MODULE NAME: rda.h

..............................................................................

  DESCRIPTION:  This file declares the RDA functions to interact with RDE

******************************************************************************
*/

#ifndef RDA_H
#define RDA_H

/*
** includes
*/
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
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
#include "ncssysf_lck.h"
#include "ncs_main_papi.h"

/*
**
*/
#include "rda_papi.h"
#include "rde_rda_common.h"

/*
** Structure declarations
*/
typedef struct {
	NCSCONTEXT task_handle;
	NCS_BOOL task_terminate;
	PCS_RDA_CB_PTR callback_ptr;
	uns32 callback_handle;
	int sockfd;

} RDA_CALLBACK_CB;

typedef struct {
	struct sockaddr_un sock_address;

} RDA_CONTROL_BLOCK;

#endif   /* RDA_H */
