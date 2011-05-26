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

#include <configmake.h>

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/
#include "eds.h"

#if (NCS_EDSV_LOG == 1)
const NCSFL_STR eds_hdln_set[] = {
	{EDS_API_MSG_DISPATCH_FAILED, "EDS API MESSAGE DISPATCH FAILED"},
	{EDS_CB_CREATE_FAILED, "EDS CONTROL BLOCK CREATION FAILED"},
	{EDS_CB_CREATE_HANDLE_FAILED, "EDS CB CREATE HANDLE FAILED"},
	{EDS_CB_DESTROY_FAILED, "EDS CB DESTROY FAILED"},
	{EDS_CB_INIT_FAILED, "EDS CB INIT FAILED"},
	{EDS_MBCSV_INIT_FAILED, "EDS MBCSV INIT FAILED"},
	{EDS_INSTALL_SIGHDLR_FAILED, "EDS INSTALL SIGNAL HANDLER FAILED"},
	{EDS_TABLE_REGISTER_FAILED, "EDS TABLE REGISTER FAILED"},
	{EDS_CB_TAKE_HANDLE_FAILED, "EDS CB TAKE HANDLE FAILED"},
	{EDS_CB_TAKE_RETD_EVT_HDL_FAILED, "EDS CB TAKE RETD EVT HDL FAILED"},
	{EDS_RET_EVT_HANDLE_CREATE_FAILED, "EDS RET EVT HANDLE CREATE FAILED"},
	{EDS_EVENT_PROCESSING_FAILED, "EDS EVENT PROCESSING FAILED"},
	{EDS_TASK_CREATE_FAILED, "EDS TASK CREATION FAILED"},
	{EDS_TASK_START_FAILED, "EDS TASK START FAILED"},
	{EDS_REG_LIST_ADD_FAILED, "EDS REG LIST ADD FAILED"},
	{EDS_REG_LIST_DEL_FAILED, "EDS REG LIST DEL FAILED"},
	{EDS_REG_LIST_GET_FAILED, "EDS REG LIST GET FAILED"},
	{EDS_VERSION_INCOMPATIBLE, "EDS VERSION INCOMPATIBLE"},
	{EDS_AMF_DISPATCH_FAILURE, "EDS AMF DISPATCH FAILURE"},
	{EDS_AMF_INIT_FAILED, "EDS AMF INIT FAILED"},
	{EDS_AMF_REG_FAILED, "EDS AMF REGISTRATION FAILED"},
	{EDS_AMF_DESTROY_FAILED, "EDS AMF DESTROY FAILED"},
	{EDS_AMF_RESPONSE_FAILED, "EDS AMF RESPONSE FAILED"},
	{EDS_IPC_CREATE_FAILED, "EDS IPC CREATE FAILED"},
	{EDS_IPC_ATTACH_FAILED, "EDS IPC ATTACH FAILED"},
	{EDS_MDS_CALLBACK_PROCESS_FAILED, "EDS MDS CALLBACK PROCESS FAILED"},
	{EDS_MDS_GET_HANDLE_FAILED, "EDS MDS GET HANDLE FAILED"},
	{EDS_MDS_INIT_FAILED, "EDS MDS INIT FAILED"},
	{EDS_MDS_UNINSTALL_FAILED, "EDS MDS UNINSTALL FAILED"},
	{EDS_MDS_VDEST_DESTROY_FAILED, "EDS MDS VDEST DESTROY FAILED"},

	/* Memory Allocation Failures */
	{EDS_MEM_ALLOC_FAILED, "EDS MEMORY ALLOCATION FAILED"},

	/* Logging stuff for Service providers (AMF) */
	{EDS_AMF_REMOVE_CALLBACK_CALLED, "EDS AMF Termincate callback called"},
	{EDS_AMF_TERMINATE_CALLBACK_CALLED, "EDS AMF Remove callback called"},
	{EDS_CB_INIT_SUCCESS, "EDS CB init success"},
	{EDS_MAIL_BOX_CREATE_ATTACH_SUCCESS, "EDS Mail Box create and attach success"},
	{EDS_MDS_VDEST_CREATE_SUCCESS, "MDS vdest create success"},
	{EDS_MDS_VDEST_CREATE_FAILED, "MDS vdest create failed"},
	{EDS_MDS_INSTALL_SUCCESS, "MDS install success"},
	{EDS_MDS_INSTALL_FAILED, "MDS install failed "},
	{EDS_MDS_SUBSCRIBE_SUCCESS, "MDS subscribe success"},
	{EDS_MDS_SUBSCRIBE_FAILED, "MDS subscribe failed "},
	{EDS_MDS_INIT_ROLE_CHANGE_SUCCESS, "MDS Initial role change success"},
	{EDS_MDS_INIT_ROLE_CHANGE_FAILED, "MDS Initial role change failed"},
	{EDS_MDS_INIT_SUCCESS, "EDS MDS init success"},
	{EDS_MBCSV_INIT_SUCCESS, "EDS MBCSV init succes"},
	{EDS_INSTALL_SIGHDLR_SUCCESS, "EDS SIGUSR1 sig handler installed success"},
	{EDS_MAIN_PROCESS_START_SUCCESS, "EDS Main process start success"},
	{EDS_TBL_REGISTER_SUCCESS, "EDS Table register success"},
	{EDS_GOT_SIGUSR1_SIGNAL, "EDS Got SIGUSR1 siganl"},
	{EDS_AMF_READINESS_CB_OUT_OF_SERVICE, "AMF ready state = OUT OF SERVICE"},
	{EDS_AMF_READINESS_CB_IN_SERVICE, "AMF ready state = IN SERVICE"},
	{EDS_AMF_READINESS_CB_STOPPING, "AMF ready state = STOPPING"},
	{EDS_AMF_RCVD_CSI_SET_CLBK, "EDS received AMF CSI set callback"},
	{EDS_AMF_CSI_SET_HA_STATE_INVALID, "AMF HA state = INVALID"},
	{EDS_MDS_CSI_ROLE_CHANGE_SUCCESS, "MDS CSI role change success"},
	{EDS_MDS_CSI_ROLE_CHANGE_FAILED, "MDS CSI role change failed"},
	{EDS_AMF_RCVD_CONFIRM_CALL_BACK, "AMF RCVD confirmation callback"},
	{EDS_AMF_RCVD_HEALTHCHK, "AMF RCVD health check"},
	{EDS_AMF_INIT_SUCCESS, "AMF Initialize success"},
	{EDS_AMF_INIT_ERROR, "AMF Initialize error"},
	{EDS_AMF_GET_SEL_OBJ_SUCCESS, "AMF selection object get success"},
	{EDS_AMF_GET_SEL_OBJ_FAILURE, "AMF selection object get failed"},
	{EDS_AMF_ENV_NAME_SET_FAIL, "EDS AMF SA_AMF_COMPONENT_NAME Env variable set failed"},
	{EDS_AMF_COMP_NAME_GET_SUCCESS, "AMF Comp name get success"},
	{EDS_AMF_COMP_NAME_GET_FAIL, "AMF Comp name get failed"},
	{EDS_AMF_INIT_OK, "AMF init success"},
	{EDS_AMF_COMP_REGISTER_SUCCESS, "AMF component register success"},
	{EDS_AMF_COMP_REGISTER_FAILED, "AMF component register failed"},
	{EDS_AMF_REG_SUCCESS, "EDS AMF Registration success"},
	{EDS_AMF_DISPATCH_ERROR, "AMF Selection object get error"},
	{EDS_AMF_HLTH_CHK_START_DONE, "AMF Health Check started"},
	{EDS_AMF_HLTH_CHK_START_FAIL, "AMF Health Check start failed"},
	/* MBCSV RElated logs * */
	{EDS_MBCSV_SUCCESS, "Mbcsv Success"},
	{EDS_MBCSV_FAILURE, "Mbcsv Failure "},
	/* MDS Related logs */
	{EDS_MDS_SUCCESS, "MDS Processing Success"},
	{EDS_MDS_FAILURE, "MDS Processing Failed "},
	/* Timer related logs */
	{EDS_TIMER_START_FAIL, "EDS TIMER START FAILURE "},
	{EDS_TIMER_STOP_FAIL, "EDS TIMER STOP FAILURE "},
	{EDS_LL_PROCESING_FAILURE, "EDS LL Processing Failed"},
	{EDS_LL_PROCESING_SUCCESS, "EDS LL Processing Success"},
	{EDS_INIT_FAILURE, "EDS Initialize Processing Failed"},
	{EDS_INIT_SUCCESS, "EDS Initialize Processing Success"},
	{EDS_FINALIZE_FAILURE, "EDS Finalize Processing Failed"},
	{EDS_FINALIZE_SUCCESS, "EDS Finalize Processing Success"},
	{EDS_CHN_OPEN_SYNC_FAILURE, "EDS Channel Open Sync Processing Failed"},
	{EDS_CHN_OPEN_SYNC_SUCCESS, "EDS Channel Open Sync Processing Success"},
	{EDS_CHN_OPEN_ASYNC_FAILURE, "EDS Channel Open Async Processing Failed"},
	{EDS_CHN_OPEN_ASYNC_SUCCESS, "EDS Channel Open Async Processing Success"},
	{EDS_CHN_CLOSE_FAILURE, "EDS Channel Close Processing Failed"},
	{EDS_CHN_CLOSE_SUCCESS, "EDS Channel Close Processing Success"},
	{EDS_CHN_UNLINK_FAILURE, "EDS Channel Unlink Processing Failed"},
	{EDS_CHN_UNLINK_SUCCESS, "EDS Channel Unlink Processing Success"},
	{EDS_PUBLISH_FAILURE, "EDS Channel Publish Processing Failed"},
	{EDS_PUBLISH_LOST_EVENT, "EDS EVT PUBLISH Lost event"},
	{EDS_SUBSCRIBE_FAILURE, "EDS Channel Subscribe Processing Failed"},
	{EDS_SUBSCRIBE_LOST_EVENT, "EDS EVT SUBSCRIBE Lost event"},
	{EDS_UNSUBSCRIBE_FAILURE, "EDS Channel Unsubscribe Processing Failed"},
	{EDS_UNSUBSCRIBE_SUCCESS, "EDS Channel Unsubscribe Processing Success"},
	{EDS_RETENTION_TMR_CLR_FAILURE, "EDS Channel Retentiontimer Clear Processing Failed"},
	{EDS_RETENTION_TMR_CLR_SUCCESS, "EDS Channel Retentiontimer Clear Processing Success"},
	{EDS_RETENTION_TMR_EXP_FAILURE, "EDS Channel Retentiontimer Expiry Processing Failed"},
	{EDS_EVT_UNKNOWN, "EDS Event Unknown"},
	{EDS_REMOVE_CALLBACK_CALLED, "EDS CSI Remove Callback called"},

	/* CLM related error codes */
	{EDS_CLM_INIT_FAILED, "EDS CLM INIT API FAILED"},
	{EDS_CLM_REGISTRATION_FAILED, "EDS CLM REGISTRATION FAILED"},
	{EDS_CLM_REGISTRATION_SUCCESS, "EDS CLM REGISTRATION SUCCESS"},
	{EDS_CLM_SEL_OBJ_GET_FAILED, "EDS CLM SELECTION OBJECT API FAILED"},
	{EDS_CLM_NODE_GET_FAILED, "EDS CLM NODE GET API FAILED"},
	{EDS_CLM_CLUSTER_TRACK_FAILED, "EDS CLM CLUSTER TRACK API FAILED"},
	{EDS_CLM_CLUSTER_TRACK_CBK_SUCCESS, "EDS CLM CLUSTER TRACK CALLBACK SUCCESS"},
	{EDS_CLM_CLUSTER_TRACK_CBK_FAILED, "EDS CLM CLUSTER TRACK CALLBACK FAILED"},
	{EDS_CLUSTER_CHANGE_NOTIFY_SEND_FAILED, "EDS CLUSTER CHANGE NOTIFY SEND FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for logging Events
 ******************************************************************************/
const NCSFL_STR eds_event_set[] = {
	{EDS_EVENT_HDR_LOG, "EVTPublished:"},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET eds_str_set[] = {
	{EDS_FC_HDLN, 0, (NCSFL_STR *)eds_hdln_set},
	{EDS_FC_HDLNF, 0, (NCSFL_STR *)eds_hdln_set},
	{EDS_FC_EVENT, 0, (NCSFL_STR *)eds_event_set},
	{0, 0, 0}
};

NCSFL_FMAT eds_fmat_set[] = {
	{EDS_LID_HDLN, NCSFL_TYPE_TCLILL, "%s EDS %14s:%5lu:%s:%lu:%lu\n"},
	{EDS_LID_HDLNF, NCSFL_TYPE_TCLILLF, "%s EDS %14s:%5lu:%s:%lu:%lu:%s\n"},
	{EDS_LID_EVENT, NCSFL_TYPE_TICLLLL,
	 "%s EDS EVENT: %s %s, ID:0x%0lx, PubTime:0x%08lx, Prio:%0lx, RetTime:0x%08lx\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC eds_ascii_spec = {
	NCS_SERVICE_ID_EDS,	/* EDS subsystem */
	EDSV_LOG_VERSION,	/* EDS log version ID */
	"EDS",
	(NCSFL_SET *)eds_str_set,	/* EDS const strings referenced by index */
	(NCSFL_FMAT *)eds_fmat_set,	/* EDS string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  PROCEDURE NAME:    eds_flx_log_ascii_set_reg

  DESCRIPTION:       Function is used for registering the canned strings
                     with the DTS.

*****************************************************************************/
uint32_t eds_flx_log_ascii_set_reg(void)
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &eds_ascii_spec;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    eds_flx_log_ascii_set_dereg

  DESCRIPTION:       Function is used for deregistering the canned strings
                     with the DTS.

*****************************************************************************/
uint32_t eds_flx_log_ascii_set_dereg(void)
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_EDS;
	arg.info.dereg_ascii_spec.version = EDSV_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: eds_log_str_lib_req

  DESCRIPTION   : Entry point for eds log string library. 

  ARGUMENTS     : req_info  - A pointer to the input information
                              given by SBOM.
 
  RETURN VALUES : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  NOTES         : None.
 *****************************************************************************/

uint32_t eds_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_SUCCESS;
	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = eds_flx_log_ascii_set_reg();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = eds_flx_log_ascii_set_dereg();
		break;

	default:
		res = NCSCC_RC_FAILURE;
		break;
	}
	return (res);
}

/*****************************************************************************

  PROCEDURE NAME: edsv_log_str_lib_req

  DESCRIPTION   : Entry point for both eds & eda log string library.

  ARGUMENTS     : req_info  - A pointer to the input information
                              given by SBOM.

  RETURN VALUES : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.

  NOTES         : None.
*****************************************************************************/
uint32_t edsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_SUCCESS;

	res = eda_log_str_lib_req(req_info);
	res = eds_log_str_lib_req(req_info);

	return res;
}

#endif
