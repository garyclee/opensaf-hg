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

#ifndef CPND_TMR_H
#define CPND_TMR_H

#include "ncssysf_tmr.h"

typedef enum cpndq_tmr_type {
	CPND_TMR_TYPE_RETENTION = 1,
	CPND_TMR_TYPE_SEC_EXPI,
	CPND_ALL_REPL_RSP_EXPI,
	CPND_TMR_OPEN_ACTIVE_SYNC,
	CPND_TMR_TYPE_NON_COLLOC_RETENTION,
	CPND_TMR_TYPE_MAX = CPND_TMR_TYPE_NON_COLLOC_RETENTION,
} CPND_TMR_TYPE;

typedef struct cpnd_tmr {
	CPND_TMR_TYPE type;
	tmr_t tmr_id;
	SaCkptCheckpointHandleT ckpt_id;
	MDS_DEST agent_dest;
	uint32_t lcl_sec_id;
	uint32_t uarg;
	NCS_BOOL is_active;
	SaUint32T write_type;
	CPSV_SEND_INFO sinfo;
	SaInvocationT invocation;
	SaCkptCheckpointHandleT lcl_ckpt_hdl;
	NCS_BOOL is_active_sync_err;
} CPND_TMR;

uint32_t cpnd_tmr_start(CPND_TMR *tmr, SaTimeT duration);

#endif
