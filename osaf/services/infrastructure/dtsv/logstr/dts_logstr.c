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

  dts_reg_strings..........Function used for registering the ASCII_SPEC table
  dts_dereg_strings........Function used for de-registering ASCII_SPEC table.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "dts.h"

#if (DTS_LOG == 1)

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/

const NCSFL_STR dts_hdln_set[] = {
	{DTS_HDLN_DTS_CREATE_FAILED, "Failed to create DTS"},
	{DTS_HDLN_DTS_CREATE_SUCCESS, "DTS created"},
	{DTS_HDLN_DTS_DESTROY_FAILED, "Failed to destroy DTS"},
	{DTS_HDLN_DTS_DESTROY_SUCCESS, "DTS destroyed"},
	{DTS_HDLN_NULL_INST, "DTS null instance"},
	{DTS_HDLN_RECEIVED_CLI_REQ, "Received CLI request"},
	{DTS_HDLN_CLI_INV_TBL, "Invalid table ID received"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Service Provider (MDS) 
 ******************************************************************************/

const NCSFL_STR dts_svc_prvdr_set[] = {
	{DTS_SP_MDS_INSTALL_FAILED, "Failed to install DTS into MDS"},
	{DTS_SP_MDS_INSTALL_SUCCESS, "DTS installed into MDS"},
	{DTS_SP_MDS_UNINSTALL_FAILED, "Failed to uninstall DTS from MDS"},
	{DTS_SP_MDS_UNINSTALL_SUCCESS, "DTS uninstalled from MDS"},
	{DTS_SP_MDS_SUBSCR_FAILED, "DTS failed to subscribe to MDS evts"},
	{DTS_SP_MDS_SUBSCR_SUCCESS, "DTS subscribed to MDS evts"},
	{DTS_SP_MDS_SND_MSG_SUCCESS, "DTS sent MDS msg successfully"},
	{DTS_SP_MDS_SND_MSG_FAILED, "DTS failed to send MDS msg"},
	{DTS_SP_MDS_RCV_MSG, "DTS recvd MDS msg"},
	{DTS_SP_MDS_RCV_EVT, "DTS recvd MDS evt"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Locks 
 ******************************************************************************/

const NCSFL_STR dts_lock_set[] = {
	{DTS_LK_LOCKED, "DTS locked"},
	{DTS_LK_UNLOCKED, "DTS unlocked"},
	{DTS_LK_CREATED, "DTS lock created"},
	{DTS_LK_DELETED, "DTS lock destroyed"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/

const NCSFL_STR dts_memfail_set[] = {
	{DTS_MF_DTSMSG_CREATE, "Failed to create DTS MSG"},
	{DTS_MF_DTSTBL_CREATE, "Failed to create DTS Table"},
	{DTS_MF_DTS_REGTBL_CREATE, "Failed to create DTS Registration table Table"},
	{DTS_MF_DTS_VCD_TBL_CREATE, "Failed to create DTS V-Card registration Table"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/

const NCSFL_STR dts_api_set[] = {
	{DTS_API_SVC_CREATE, "ncsdts_lm(CREATE)"},
	{DTS_API_SVC_DESTROY, "ncsdts_lm(DESTROY)"},
	{DTS_AMF_INIT_SUCCESS, "DTS AMF Initialization Success"},
	{DTS_FAIL_OVER_SUCCESS, "DTS Fail-over/Switch-over successful"},
	{DTS_FAIL_OVER_FAILED, "DTS Fail-over/Switch-over failed"},
	{DTS_AMF_ROLE_CHG, "DTS HA State changed"},
	{DTS_AMF_ROLE_FAIL, "DTS role assignment failed"},
	{DTS_REG_TBL_CLEAR, "Clearing DTS service registration & DTA entries"},
	{DTS_AMF_FINALIZE, "DTS AMF Finalize successful"},
	{DTS_CSI_SET_CB_RESP, "Sent saAmfResponse for CSI set callback"},
	{DTS_AMF_HEALTH_CHECK, "Health check callback called"},
	{DTS_CSI_RMV_CB_RESP, "Sent saAmfResponse for CSI remove callback"},
	{DTS_CSI_TERM_CB, "CSI Terminate callback, initiating DTS lib destroy"},
	{DTS_AMF_UP_SIG, "DTS received signal for AMF up"},
	{DTS_MBCSV_REG, "DTS registers with MBCSv"},
	{DTS_OPEN_CKPT, "DTS calls MBCSv open checkpoint"},
	{DTS_MBCSV_DEREG, "DTS de-registers with MBCSv"},
	{DTS_CLOSE_CKPT, "DTS closes MBCSv checkpoint successfully"},
	{DTS_MBCSV_FIN, "DTS MBCSv finalize successful"},
	{DTS_MDS_ROLE_CHG_FAIL, "DTS MDS role change failed"},
	{DTS_CKPT_CHG_FAIL, "DTS MBCSv set checkpoint role failed"},
	{DTS_INIT_ROLE_ACTIVE, "DTS init role - Active"},
	{DTS_INIT_ROLE_STANDBY, "DTS init role - Standby"},
	{DTS_SPEC_RELOAD_CMD, "Received ASCII_SPEC reload command from CLI"},
	{DTS_LOG_DEL_FAIL, "Deletion from an empty log file list"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Event 
 ******************************************************************************/

const NCSFL_STR dts_evt_set[] = {
	{DTS_EV_SVC_REG_REQ_RCV, "Service registration request received for"},
	{DTS_EV_SVC_DE_REG_REQ_RCV, "Service de-registration request received for"},
	{DTS_EV_SVC_REG_SUCCESSFUL, "Service registration successful for"},
	{DTS_EV_SVC_REG_FAILED, "Service registration failed for"},
	{DTS_EV_SVC_DEREG_SUCCESSFUL, "Service de-registration successful for"},
	{DTS_EV_SVC_DEREG_FAILED, "Service de-registration failed for"},
	{DTS_EV_SVC_ALREADY_REG, "Service is already registered with DTS"},
	{DTS_EV_SVC_REG_ENT_ADD, "Service registration entry added"},
	{DTS_EV_SVC_REG_ENT_ADD_FAIL, "Service registration entry add failed"},
	{DTS_EV_SVC_REG_ENT_RMVD, "Service registration entry removed"},
	{DTS_EV_SVC_REG_ENT_UPDT, "Service registration entry updated"},
	{DTS_EV_LOG_SVC_KEY_WRONG, "No corresponding node/svc entry for supplied key in log message"},
	{DTS_EV_DTA_DOWN, "Received DTA down event for"},
	{DTS_EV_DTA_UP, "Received DTA up event for"},
	{DTS_EV_DTA_DEST_ADD_SUCC, "DTA dest add successful for"},
	{DTS_EV_DTA_DEST_RMV_SUCC, "DTA dest remove success for"},
	{DTS_EV_DTA_DEST_ADD_FAIL, "DTA dest add fail for"},
	{DTS_EV_DTA_DEST_RMV_FAIL, "DTA dest remove fail for"},
	{DTS_EV_DTA_DEST_PRESENT, "DTA dest entry already present"},
	{DTS_EV_SVC_DTA_ADD, "Service's DTA list added with DTA entry"},
	{DTS_EV_SVC_DTA_RMV, "DTA entry removed frm Service's DTA list"},
	{DTS_EV_DTA_SVC_ADD, "DTA's svc list added with svc_reg entry"},
	{DTS_EV_DTA_SVC_RMV, "svc_reg entry removed from DTA's svc list"},
	{DTS_EV_DTA_SVC_RMV_FAIL, "Failed to remove svc_reg entry from DTA's svc list"},
	{DTS_EV_SVC_DTA_RMV_FAIL, "Failed to remove adest entry frm service's DTA list"},
	{DTS_EV_SVC_REG_NOTFOUND, "Service registration entry not found for"},
	{DTS_MDS_VDEST_CHG, "Change in DTS MDS VDEST to"},
	{DTS_SET_FAIL, "SET command failed for"},
	{DTS_LOG_DEV_NONE, "SET req removing all log devices for"},
	{DTS_SPEC_SVC_NAME_NULL, "Service Name string is NULL"},
	{DTS_SPEC_SVC_NAME_ERR, "Service Name string size exceeds limits. Check your service name"},
	{DTS_SPEC_SET_ERR, "String set ID mismatch. Check your strung set ID"},
	{DTS_SPEC_STR_ERR, "String Index ID mismatched. Check your string index ID's"},
	{DTS_SPEC_FMAT_ERR, "String format ID mismatch. Check your string format ID's"},
	{DTS_SPEC_REG_SUCC, "Regsitered Successfully"},
	{DTS_SPEC_REG_FAIL, "Register Unsuccessful"},
	{DTS_LOG_SETIDX_ERR, "String set ID exceeds the max string set."},
	{DTS_LOG_STRIDX_ERR, "String index ID exceeds the maximum number of string ID count."},
	{DTS_SPEC_NOT_REG, "ASCII_SPEC Table not registered with DTS."},
	{DTS_INVALID_FMAT, "Format ID received in the message is invalid. Exceeds the max."},
	{DTS_FMAT_ID_MISMATCH, "Format ID mismatch."},
	{DTS_FMAT_TYPE_MISMATCH, "Format type received with the message does not match with the ASCII_SPEC."},
	{DTS_LOG_CONV_FAIL, "Unable to convert log message to string."},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Checkpointing Operations 
 ******************************************************************************/

const NCSFL_STR dts_chkop_set[] = {
	{DTS_CSYNC_ENC_START, "ColdSync encode started"},
	{DTS_CSYNC_ENC_FAILED, "ColdSync encode failed"},
	{DTS_CSYNC_ENC_COMPLETE, "ColdSync encode completed successfully"},
	{DTS_CSYNC_ENC_SVC_REG, "ColdSync encode for Service Registration table"},
	{DTS_CSYNC_ENC_DTA_DEST, "ColdSync encode for DTA Destination List"},
	{DTS_CSYNC_ENC_GLOBAL_POLICY, "ColdSync encode for Global Policy"},
	{DTS_CSYNC_ENC_UPDT_COUNT, "ColdSync encode for Async Update count"},
	{DTS_CSYNC_DEC_START, "ColdSync decode started"},
	{DTS_CSYNC_DEC_FAILED, "ColdSync decode failed"},
	{DTS_CSYNC_DEC_COMPLETE, "ColdSync decode completed successfully"},
	{DTS_CSYNC_DEC_SVC_REG, "ColdSync decode for Service Registration table"},
	{DTS_CSYNC_DEC_DTA_DEST, "ColdSync decode for DTA Destination List"},
	{DTS_CSYNC_DEC_GLOBAL_POLICY, "ColdSync decode for Global Policy"},
	{DTS_CSYNC_DEC_UPDT_COUNT, "ColdSync decode for Async Update count"},
	{DTS_WSYNC_ENC_START, "WarmSync encode started"},
	{DTS_WSYNC_ENC_FAILED, "WarmSync encode failed"},
	{DTS_WSYNC_ENC_COMPLETE, "WarmSync encode completed"},
	{DTS_WSYNC_DEC_START, "WarmSync decode started"},
	{DTS_WSYNC_DEC_FAILED, "WarmSync decode failed"},
	{DTS_WSYNC_DEC_COMPLETE, "WarmSync decode completed"},
	{DTS_WSYNC_FAILED, "WarmSync failed"},
	{DTS_WSYNC_DATA_MISMATCH, "WarmSync detected Async update mismatch"},
	{DTS_ASYNC_SVC_REG, "Async update for service registration table"},
	{DTS_ASYNC_DTA_DEST, "Async update for DTA destination list"},
	{DTS_ASYNC_GLOBAL_POLICY, "Async update for global policy change"},
	{DTS_ASYNC_LOG_FILE, "Async update for log file creation/deletion"},
	{DTS_ASYNC_FAILED, "Async update failed"},
	{DTS_COLD_SYNC_TIMER_EXP, "Cold Sync response timer expired"},
	{DTS_COLD_SYNC_CMPLT_EXP, "Cold Sync complete timer expired"},
	{DTS_WARM_SYNC_TIMER_EXP, "Warm Sync response timer expired"},
	{DTS_WARM_SYNC_CMPLT_EXP, "Warm Sync complete timer expired"},
	{DTS_DEC_DATA_RESP, "Decode data sync response started"},
	{DTS_DEC_DATA_REQ, "Decode data request received"},
	{DTS_ENC_DATA_RESP, "Encoding data sync response"},
	{DTS_DATA_RESP_CMPLT_EXP, "Data Response complete timer expired"},
	{DTS_DATA_RESP_TERM, "Data response terminated"},
	{DTS_ASYNC_CNT_MISMATCH, "Async update count mismatch"},
	{DTS_ACT_ASSIGN_NOT_INSYNC, "Active assignment received for Stby not insync"},
	{DTS_CSYNC_IN_CSYNC, "ColdSync received while in coldsync"},
	{DTS_CSYNC_REQ_ENC, "Encoding ColdSync request"},
	{DTS_DATA_SYNC_CMPLT, "Data Sync Completed. STBY is in sync with ACT."},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Circular buffer Operation 
 ******************************************************************************/

const NCSFL_STR dts_cbop_set[] = {
	{DTS_CBOP_ALLOCATED, "Circular Buffer is allocated"},
	{DTS_CBOP_FREED, "Circular Buffer is freed"},
	{DTS_CBOP_CLEARED, "Circular Buffer is cleared"},
	{DTS_CBOP_DUMPED, "Circular Buffer log is dumped"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Debug Strings 
 ******************************************************************************/

const NCSFL_STR dts_dbg_set[] = {
	{DTS_GLOBAL, "GLOBAL"},
	{DTS_NODE, "NODE"},
	{DTS_SERVICE, "SERVICE"},
	{DTS_IGNORE, ""},
	{0, 0}
};

/******************************************************************************************/

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET dts_str_set[] = {
	{DTS_FC_HDLN, 0, (NCSFL_STR *)dts_hdln_set},
	{DTS_FC_SVC_PRVDR_FLEX, 0, (NCSFL_STR *)dts_svc_prvdr_set},
	{DTS_FC_LOCKS, 0, (NCSFL_STR *)dts_lock_set},
	{DTS_FC_MEMFAIL, 0, (NCSFL_STR *)dts_memfail_set},
	{DTS_FC_API, 0, (NCSFL_STR *)dts_api_set},
	{DTS_FC_EVT, 0, (NCSFL_STR *)dts_evt_set},
	{DTS_FC_CIRBUFF, 0, (NCSFL_STR *)dts_cbop_set},
	{DTS_FC_STR, 0, (NCSFL_STR *)dts_dbg_set},
	{DTS_FC_UPDT, 0, (NCSFL_STR *)dts_chkop_set},
	{0, 0, 0}
};

NCSFL_FMAT dts_fmat_set[] = {
	{DTS_LID_HDLN, "TI", "DTS %s HEADLINE : %s\n\n"},
	{DTS_LID_SVC_PRVDR_FLEX, "TI", "DTS %s SVC PRVDR: %s\n\n"},
	{DTS_LID_LOCKS, "TIL", "DTS %s LOCK     : %s (%p)\n\n"},
	{DTS_LID_MEMFAIL, "TI", "DTS %s MEMERR   : %s\n\n"},
	{DTS_LID_API, "TI", "DTS %s API      : %s\n\n"},
	{DTS_LID_EVT, "TILLL", "DTS %s EVENT    : %s \n svc_id = %ld, node_id = 0x%08lx dest process_id = %ld. \n\n"},
	{DTS_LID_CB_LOG, "TILL", "DTS %s CBOP     : %s svc_id = %ld, node_id = 0x%08lx\n\n"},
	{DTS_LID_STR, "TIC", "DTS %s DBG INFO : %s %s\n\n"},
	{DTS_LID_STRL, "TICCL", "DTS %s DBG INFO : %s %s \n %s %ld\n\n"},
	{DTS_LID_STRL_SVC, "TICCLL", "DTS %s DBG INFO : %s %s \n %s %ld , SVC = %ld\n\n"},
	{DTS_LID_STRL_SVC_NAME, "TICCLC", "DTS %s DBG INFO : %s %s \n %s %ld , SVC_NAME = %s\n\n"},
	{DTS_LID_STRLL, "TICLL", "DTS %s DBG INFO : %s %s %ld %ld\n\n"},
	{DTS_LID_LFILE, "TCLL", "DTS %s New Log file created : %s. Node: %ld, Svc: %ld \n\n"},
	{DTS_LID_LOGDEL, "TLLL", "DTS %s Deleting old log file. Node: %ld, Svc:%ld, Count:%ld \n\n"},
	{DTS_LID_CHKP, "TI", "DTS %s CHKP EVT  : %s\n\n"},
	{DTS_LID_MDS_EVT, "TIL", "DTS %s MDS EVT : %s %ld\n\n"},
	{DTS_LID_AMF_EVT, "TILL", "DTS %s AMF EVT : %s from:%ld to:%ld\n\n"},
	{DTS_LID_SPEC_ERR_EVT, "TILLC", "DTS %s ASCII_SPEC ERROR:%s %ld SVC:%ld SVC-NAME:%s\n\n"},
	{DTS_LID_SPEC_REG, "TILCL", "DTS %s ASCII_SPEC %s SVC:%ld SVC-NAME:%s VERSION:%ld\n\n"},
	{DTS_LID_LOG_ERR, "TILLC", "DTS %s LOGGING ERROR:%s %ld SVC:%ld SVC_NAME:%s\n\n"},
	{DTS_LID_LOG_ERR1, "TILLLC",
	 "DTS %s LOGGING ERROR:%s Value Passed:%ld Value Compared:%ld SVC:%ld, SVC_NAME:%s\n\n"},
	{DTS_LID_LOG_ERR2, "TILL", "DTS %s LOGGING ERROR:%s %ld SVC:%ld\n\n"},
	{DTS_LID_WSYNC_ERR, "TILLLLLLLL",
	 "DTS %s WARM SYNC ERROR: %s SENT UPDT COUNT:{svc_reg=%ld dta_list=%ld global_policy=%ld log_updt=%ld} RECEIVED UPDT COUNT:{svc_reg=%ld dta_list=%ld global_policy=%ld log_updt=%ld}\n\n"},
	{DTS_LID_ASYNC_UPDT, "TILLLL", "DTS %s CKPT EVT: %s OP:%ld NODE:%ld SVC:%ld MDS_DEST:%ld\n\n"},
	{DTS_LID_FLOW_UP, "TLL",
	 "DTS %s CONGESTION EVENT HIT. INFO & DEBUG severity logs will be dropped for NODE: 0x%08lx & Process Id: %ld\n\n"},
	{DTS_LID_FLOW_DOWN, "TLL", "DTS %s CONGESTION EVENT CLEARED. NODE: 0x%08lx Process Id: %ld\n\n"},
	{DTS_LID_GENLOG, NCSFL_TYPE_TC, "%s %s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC dts_ascii_spec = {
	NCS_SERVICE_ID_DTSV,	/* DTS subsystem */
	DTS_LOG_VERSION,	/* DTS log version no. */
	"DTS",
	(NCSFL_SET *)dts_str_set,	/* DTS const strings referenced by index */
	(NCSFL_FMAT *)dts_fmat_set,	/* DTS string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  ncsdts_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/

uint32_t dts_reg_strings()
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &dts_ascii_spec;
	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  ncsdts_dereg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/

uint32_t dts_dereg_strings()
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_DTSV;
	arg.info.dereg_ascii_spec.version = DTS_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : dts_log_str_lib_req
 *
 * Description   : Library entry point for loading dtsv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which is passed from DTS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t dts_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = dts_reg_strings();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = dts_dereg_strings();
		break;

	default:
		break;
	}
	return (res);
}
#else				/* (DTS_LOG == 0) */

/*****************************************************************************

  dts_reg_strings

  DESCRIPTION: When DTS_LOG is disabled, we don't register a thing,

*****************************************************************************/

uint32_t dts_reg_strings(char *fname)
{
	return NCSCC_RC_SUCCESS;
}

#endif
