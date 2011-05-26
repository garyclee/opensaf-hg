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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

  mbcsv_reg_strings..........Function used for registering the ASCII_SPEC table
  mbcsv_dereg_strings........Function used for de-registering ASCII_SPEC table.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mbcsv.h"

#if (MBCSV_LOG == 1)

#include "dts_papi.h"

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/

const NCSFL_STR mbcsv_hdln_set[] = {
	{MBCSV_HDLN_MBCSV_CREATE_FAILED, "Failed to create MBCSV."},
	{MBCSV_HDLN_MBCSV_CREATE_SUCCESS, "MBCSV created successfully. MBCSV Services are available now"},
	{MBCSV_HDLN_MBCSV_DESTROY_FAILED, "Failed to destroy MBCSV."},
	{MBCSV_HDLN_MBCSV_DESTROY_SUCCESS,
	 "MBCSV is about to destroy itself. MBCSV services will not be available now."},
	{MBCSV_HDLN_NULL_INST, "MBCSV null instance."},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Service Provider (MDS) 
 ******************************************************************************/

const NCSFL_STR mbcsv_svc_prvdr_set[] = {
	{MBCSV_SP_MDS_INSTALL_FAILED, "Failed to install MBCSV with MDS."},
	{MBCSV_SP_MDS_INSTALL_SUCCESS, "MBCSV installed with MDS successfully."},
	{MBCSV_SP_MDS_UNINSTALL_FAILED, "Failed to uninstall MBCSV from MDS."},
	{MBCSV_SP_MDS_UNINSTALL_SUCCESS, "MBCSV uninstalled from MDS."},
	{MBCSV_SP_MDS_SUBSCR_FAILED, "MBCSV failed to subscribe to MDS evts."},
	{MBCSV_SP_MDS_SUBSCR_SUCCESS, "MBCSV subscribed to MDS evts."},
	{MBCSV_SP_MDS_SND_MSG_SUCCESS, "MBCSV sent MDS msg successfully."},
	{MBCSV_SP_MDS_SND_MSG_FAILED, "MBCSV failed to send MDS msg to peer."},
	{MBCSV_SP_MDS_RCV_MSG, "MBCSV recvd MDS msg."},
	{MBCSV_SP_MDS_RCV_EVT_UP, "MBCSV recvd MDS up evt for peer."},
	{MBCSV_SP_MDS_RCV_EVT_DN, "MBCSV recvd MDS down evt for peer."},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Locks 
 ******************************************************************************/

const NCSFL_STR mbcsv_lock_set[] = {
	{MBCSV_LK_LOCKED, "locked"},
	{MBCSV_LK_UNLOCKED, "unlocked"},
	{MBCSV_LK_CREATED, "lock created"},
	{MBCSV_LK_DELETED, "lock destroyed"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/

const NCSFL_STR mbcsv_memfail_set[] = {
	{MBCSV_MF_MBCSVMSG_CREATE, "Failed to create MBCSV MSG"},
	{MBCSV_MF_MBCSVTBL_CREATE, "Failed to create MBCSV Table"},
	{MBCSV_MF_MBCSV_REGTBL_CREATE, "Failed to create MBCSV Registration table Table"},
	{MBCSV_MF_MBCSV_VCD_TBL_CREATE, "Failed to create MBCSV V-Card registration Table"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/

const NCSFL_STR mbcsv_api_set[] = {
	{MBCSV_API_IDLE, "IDLE"},
	{MBCSV_API_ACTIVE, "ACTIVE"},
	{MBCSV_API_STANDBY, "STANDBY"},
	{MBCSV_API_QUIESCED, "QUIESCED"},
	{MBCSV_API_NONE, "NONE"},

	{MBCSV_API_OP_INITIALIZE, "NCS_MBCSV_OP_INITIALIZE"},
	{MBCSV_API_OP_FINALIZE, "NCS_MBCSV_OP_FINALIZE"},
	{MBCSV_API_OP_SELECT, "NCS_MBCSV_OP_SELECT"},
	{MBCSV_API_OP_DISPATCH, "NCS_MBCSV_OP_DISPATCH"},
	{MBCSV_API_OP_OPEN, "NCS_MBCSV_OP_OPEN"},
	{MBCSV_API_OP_CLOSE, "NCS_MBCSV_OP_CLOSE"},
	{MBCSV_API_OP_CHG_ROLE, "NCS_MBCSV_OP_CHG_ROLE"},
	{MBCSV_API_OP_SEND_CKPT, "NCS_MBCSV_OP_SEND_CKPT"},
	{MBCSV_API_OP_SEND_NTFY, "NCS_MBCSV_OP_SEND_NOTIFY"},
	{MBCSV_API_OP_SUB_ONESH, "NCS_MBCSV_OP_SUB_ONESHOT"},
	{MBCSV_API_OP_SUB_ONEPE, "NCS_MBCSV_OP_SUB_PERSIST"},
	{MBCSV_API_OP_SUB_CANCEL, "NCS_MBCSV_OP_SUB_CANCEL"},
	{MBCSV_API_OP_GET, "NCS_MBCSV_OP_OBJ_GET"},
	{MBCSV_API_OP_SET, "NCS_MBCSV_OP_OBJ_SET"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Peer Event 
 ******************************************************************************/

const NCSFL_STR mbcsv_peer_evt_set[] = {
	/* HA Role */
	{MBCSV_PEER_EV_DBG_IDLE, "IDLE"},
	{MBCSV_PEER_EV_DBG_ACTIVE, "ACTIVE"},
	{MBCSV_PEER_EV_DBG_STANDBY, "STANDBY"},
	{MBCSV_PEER_EV_DBG_QUIESCED, "QUIESCED"},
	{MBCSV_PEER_EV_DBG_NONE, "NONE"},

	/* Peer Events */
	{MBCSV_PEER_EV_PEER_UP, "Peer up event received"},
	{MBCSV_PEER_EV_PEER_DOWN, "Peer Down event received"},
	{MBCSV_PEER_EV_PEER_INFO, "Peer Info event received"},
	{MBCSV_PEER_EV_MBCSV_INFO_RSP, "MBCSV Info response event received"},
	{MBCSV_PEER_EV_CHG_ROLE, "MBCSV Chnage Role Event received"},
	{MBCSV_PEER_EV_MBCSV_UP, "MBCSV up event received"},
	{MBCSV_PEER_EV_MBCSV_DOWN, "MBCSV down event received"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for FSM Event 
 ******************************************************************************/

const NCSFL_STR mbcsv_fsm_evt_set[] = {
	/* HA Role */
	{MBCSV_FSM_EV_DBG_IDLE, "IDLE"},
	{MBCSV_FSM_EV_DBG_ACTIVE, "ACTIVE"},
	{MBCSV_FSM_EV_DBG_STANDBY, "STANDBY"},
	{MBCSV_FSM_EV_DBG_QUIESCED, "QUIESCED"},
	{MBCSV_FSM_EV_DBG_NONE, "NONE"},

	/* FSM Events */
	{MBCSV_FSM_EV_EVENT_ASYNC_UPDATE, "Event Async update"},
	{MBCSV_FSM_EV_EVENT_COLD_SYNC_REQ, "Event Cold sync request"},
	{MBCSV_FSM_EV_EVENT_COLD_SYNC_RESP, "Event Cold Sync Response"},
	{MBCSV_FSM_EV_EVENT_COLD_SYNC_RESP_CPLT, "Event Cold Sync response complete"},
	{MBCSV_FSM_EV_EVENT_WARM_SYNC_REQ, "Event Warm sync request"},
	{MBCSV_FSM_EV_EVENT_WARM_SYNC_RESP, "Event Warm Sync Response"},
	{MBCSV_FSM_EV_EVENT_WARM_SYNC_RESP_CPLT, "Event Warm Sync response complete"},
	{MBCSV_FSM_EV_EVENT_DATA_REQ, "Event Data request"},
	{MBCSV_FSM_EV_EVENT_DATA_RESP, "Event Data Response"},
	{MBCSV_FSM_EV_EVENT_DATA_RESP_CPLT, "Event Data response complete"},
	{MBCSV_FSM_EV_EVENT_NOTIFY, "Event notify"},
	{MBCSV_FSM_EV_SEND_COLD_SYNC_REQ, "Send Cold sync request"},
	{MBCSV_FSM_EV_SEND_WARM_SYNC_REQ, "Send Warm sync request"},
	{MBCSV_FSM_EV_SEND_ASYNC_UPDATE, "Send Async update"},
	{MBCSV_FSM_EV_SEND_DATA_REQ, "Send Data request"},
	{MBCSV_FSM_EV_SEND_COLD_SYNC_RESP, "Send Cold Sync Response"},
	{MBCSV_FSM_EV_SEND_COLD_SYNC_RESP_CPLT, "Send Cold Sync response complete"},
	{MBCSV_FSM_EV_SEND_WARM_SYNC_RESP, "Send Warm Sync Response"},
	{MBCSV_FSM_EV_SEND_WARM_SYNC_RESP_CPLT, "Send Warm Sync response complete"},
	{MBCSV_FSM_EV_SEND_DATA_RESP, "Send Data Response"},
	{MBCSV_FSM_EV_SEND_DATA_RESP_CPLT, "Send Data response complete"},
	{MBCSV_FSM_EV_ENTITY_IN_SYNC, "Event entity in sync"},
	{MBCSV_FSM_EV_SEND_NOTIFY, "Send notify"},
	{MBCSV_FSM_EV_TMR_SEND_COLD_SYNC, "Timer event - Send Cold Sync"},
	{MBCSV_FSM_EV_TMR_SEND_WARM_SYNC, "Timer event - Send Warm Sync"},
	{MBCSV_FSM_EV_TMR_COLD_SYNC_CPLT, "Timer event - Cold Sync Complete"},
	{MBCSV_FSM_EV_TMR_WARM_SYNC_CPLT, "Timer event - Warm Sync Complete"},
	{MBCSV_FSM_EV_TMR_DATA_RESP_CPLT, "Timer event - Data response Complete"},
	{MBCSV_FSM_EV_TMR_TRANSMIT, "Timer event - Transmit timer expired"},
	{MBCSV_FSM_EV_MULTIPLE_ACTIVE, "Event Multiple Active Present"},
	{MBCSV_FSM_EV_MULTIPLE_ACTIVE_CLEAR, "Event Multiple Active Clear"},

	/* FSM Active State */
	{MBCSV_FSM_STATE_ACTIVE_IDLE, "ACTIVE_IDLE"},
	{MBCSV_FSM_STATE_ACTIVE_WFCS, "WAIT_FOR_COLD_SYNC"},
	{MBCSV_FSM_STATE_ACTIVE_KBIS, "KEEP_STANDBY_IN_SYNC"},
	{MBCSV_FSM_STATE_ACTIVE_MUAC, "MULTIPLE_ACTIVE"},

	/* FSM Standby State */
	{MBCSV_FSM_STATE_STBY_IDLE, "STBY_IDLE"},
	{MBCSV_FSM_STATE_STBY_WTCS, "WAIT_TO_COLD_SYNC"},
	{MBCSV_FSM_STATE_STBY_SIS, "STUDY_IN_SYNC"},
	{MBCSV_FSM_STATE_STBY_WTWS, "WAIT_TO_WARM_SYNC"},
	{MBCSV_FSM_STATE_STBY_VWSD, "VERIFY_WARM_SYNC_DATA"},
	{MBCSV_FSM_STATE_STBY_WFDR, "WAIT_FOR_DATA_RESPONSE"},

	{MBCSV_FSM_STATE_QUIESCED, "QUIESCED"},
	{MBCSV_FSM_STATE_NONE, "NONE"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Debug Strings 
 ******************************************************************************/

const NCSFL_STR mbcsv_tmr_set[] = {
	{MBCSV_TMR_IDLE, "IDLE"},
	{MBCSV_TMR_ACTIVE, "ACTIVE"},
	{MBCSV_TMR_STANDBY, "STANDBY"},
	{MBCSV_TMR_QUIESCED, "QUIESCED"},
	{MBCSV_TMR_NONE, "NONE"},

	{MBCSV_TMR_SND_COLD_SYNC, "Send Cold Sync timer"},
	{MBCSV_TMR_SND_WARM_SYNC, "Send Warm Sync timer"},
	{MBCSV_TMR_COLD_SYN_CMP, "Cold Sync Complete timer"},
	{MBCSV_TMR_WARM_SYN_CMP, "Warm Sync Complete timer"},
	{MBCSV_TMR_DATA_RSP_CMP, "Data Response Complete timer"},
	{MBCSV_TMR_TRANSMIT, "Transmit timer"},

	{MBCSV_TMR_START, "Started"},
	{MBCSV_TMR_STOP, "Stopped"},
	{MBCSV_TMR_EXP, "Expired"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Debug Strings 
 ******************************************************************************/

const NCSFL_STR mbcsv_dbg_set[] = {
	{MBCSV_DBG_IDLE, "IDLE"},
	{MBCSV_DBG_ACTIVE, "ACTIVE"},
	{MBCSV_DBG_STANDBY, "STANDBY"},
	{MBCSV_DBG_QUIESCED, "QUIESCED"},
	{MBCSV_DBG_NONE, "NONE"},
	{0, 0}
};

/******************************************************************************************/

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET mbcsv_str_set[] = {
	{MBCSV_FC_HDLN, 0, (NCSFL_STR *)mbcsv_hdln_set},
	{MBCSV_FC_SVC_PRVDR_FLEX, 0, (NCSFL_STR *)mbcsv_svc_prvdr_set},
	{MBCSV_FC_LOCKS, 0, (NCSFL_STR *)mbcsv_lock_set},
	{MBCSV_FC_MEMFAIL, 0, (NCSFL_STR *)mbcsv_memfail_set},
	{MBCSV_FC_API, 0, (NCSFL_STR *)mbcsv_api_set},
	{MBCSV_FC_PEER_EVT, 0, (NCSFL_STR *)mbcsv_peer_evt_set},
	{MBCSV_FC_FSM_EVT, 0, (NCSFL_STR *)mbcsv_fsm_evt_set},
	{MBCSV_FC_TMR, 0, (NCSFL_STR *)mbcsv_tmr_set},
	{MBCSV_FC_STR, 0, (NCSFL_STR *)mbcsv_dbg_set},
	{0, 0, 0}
};

NCSFL_FMAT mbcsv_fmat_set[] = {	/*  T,State,SVCID, pwe, anch, .... */
	{MBCSV_LID_HDLN, "TI", "%s MBCA HEADLINE : %s \n\n"},
	{MBCSV_LID_SVC_PRVDR_FLEX, "TLCI", "%s MDS EVENT for pwe = %ld, peer = %s \n SVC PRVDR: %s \n\n"},
	{MBCSV_LID_GL_LOCKS, "TIL", "%s MBCA GLOBAL LOCK : %s (%p) \n\n"},
	{MBCSV_LID_SVC_LOCKS, "TILL", "%s MBCA SVC LOCK : %s SVC = %ld (%p) \n\n"},
	{MBCSV_LID_MEMFAIL, "TII", "%s %s MBCA MEMERR : %s \n\n"},
	{MBCSV_LID_API, "TILLI", "%s %s MBCA %ld %ld API : %s \n\n"},
	{MBCSV_LID_PEER_EVT, "TILLCI", "%s %s MBCA svc = %ld, pwe = %ld, peer = %s \nPEER_EVENT: %s \n\n"},
	{MBCSV_LID_FSM_EVT, "TILLCII",
	 "%s %s MBCA svc = %ld, pwe = %ld, peer = %s \nFSM_EVENT: fsm state %s event %s \n\n"},
	{MBCSV_LID_TMR, "TILLCII", "%s %s MBCA svc = %ld, pwe = %ld, peer = %s \nTMR : %s %s \n\n"},
	{MBCSV_LID_STR, "TILLC", "%s %s MBCA %ld %ld \nDBG INFO : %s \n\n"},
	{MBCSV_LID_DBG_SNK, "TCCL", "%s MBCA %s \nFile: %s Line: %ld \n\n"},
	{MBCSV_LID_DBG_SNK_SVC, "TCLCL", "%s MBCA %s Svc %ld \nFile %s Line %ld \n\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC mbcsv_ascii_spec = {
	NCS_SERVICE_ID_MBCSV,	/* MBCSV subsystem */
	MBCSV_LOG_VERSION,	/* MBCSV revision ID */
	"MBCA",			/* Name to be printed for log file */
	(NCSFL_SET *)mbcsv_str_set,	/* MBCSV const strings referenced by index */
	(NCSFL_FMAT *)mbcsv_fmat_set,	/* MBCSV string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  ncsmbcsv_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the MBCSV.

*****************************************************************************/

uint32_t mbcsv_reg_strings()
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &mbcsv_ascii_spec;
	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : mbcsv_log_str_lib_req
 *
 * Description   : Library entry point for mbcsv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t mbcsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = mbcsv_reg_strings();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = mbcsv_dereg_strings();
		break;

	default:
		break;
	}
	return (res);
}

/*****************************************************************************

  ncsmbcsv_dereg_strings

  DESCRIPTION: Function is used for registering the canned strings with the MBCSV.

*****************************************************************************/

uint32_t mbcsv_dereg_strings()
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_MBCSV;
	arg.info.dereg_ascii_spec.version = MBCSV_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}
#else				/* (MBCSV_LOG == 0) */

/*****************************************************************************

  mbcsv_reg_strings

  DESCRIPTION: When MBCSV_LOG is disabled, we don't register a thing,

*****************************************************************************/

uint32_t mbcsv_reg_strings(char *fname)
{
	return NCSCC_RC_SUCCESS;
}

#endif
