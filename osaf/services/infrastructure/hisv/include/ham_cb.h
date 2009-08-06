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
  This file containts the control block structure of HAM

******************************************************************************
*/
#ifndef HAM_CB_H
#define HAM_CB_H

/* macro definitions */
/* system event log clear timer period */
#define HPI_SYS_LOG_CLEAR_INTERVAL  54000

/* ADEST List of HPL users here */
typedef struct ham_adest_list {
	MDS_DEST addr;
	struct ham_adest_list *next;
} HAM_ADEST_LIST;

/* structure for HPI private library information */
typedef struct his_cb_info {
	uns32 cb_hdl;		/* CB hdl returned by hdl mngr */
	uns8 pool_id;		/* pool-id used by hdl mngr */
	uns32 prc_id;		/* process identifier */
	tmr_t tmr_id;		/* SEL clear timer handler */
	HPI_SESSION_ARGS *args;	/* HPI session arguments */
	NCSCONTEXT task_hdl;	/* HAM task handle */
	MDS_HDL mds_hdl;	/* PWE Handle for interacting with HPL  */
	MDS_HDL mds_vdest_hdl;	/* VDEST hdl for global services */
	V_DEST_QA my_anc;	/* required for MDS VDEST */
	V_DEST_RL mds_role;	/* mds role   */
	MDS_VDEST_ID ham_vdest;	/* HAM MDS VDEST */
	MDS_DEST hpl_dest;	/* HPL MDS ADEST */
	SYSF_MBX mbx;		/* HAM's mailbox       */
	NCS_LOCK cb_lock;	/* Lock for this control Block */
	HAM_ADEST_LIST *dest;	/* List of ADEST of HPL users */

} HAM_CB;

typedef uns32 (*HAM_EVT_REQ_HDLR) (HISV_EVT *evt);

/**************************************************************************
 * function declarations
 */
EXTERN_C uns32 ham_process_evt(HISV_EVT *evt);
EXTERN_C uns32 ham_mds_change_role(HAM_CB *cb);
EXTERN_C uns32 hisv_evt_destroy(HISV_EVT *evt);
EXTERN_C uns32 ham_start_tmr(HAM_CB *cb, SaTimeT period);
EXTERN_C void ham_stop_tmr(HAM_CB *cb);
EXTERN_C void ham_tmr_exp(void *uarg);

#endif   /* HAM_CB_H */
