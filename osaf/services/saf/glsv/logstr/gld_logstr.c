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

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/

#if (NCS_DTS == 1)

#include "dts.h"
#include "gld_log.h"
#include "glsv_logstr.h"

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/
const NCSFL_STR gld_hdln_set[] = {
	{GLD_PATRICIA_TREE_INIT_FAILED, "PATRICIA TREE INIT FAILED"},
	{GLD_PATRICIA_TREE_ADD_FAILED, "PATRICIA TREE ADD FAILED"},
	{GLD_PATRICIA_TREE_GET_FAILED, "PATRICIA TREE GET FAILED"},
	{GLD_PATRICIA_TREE_DEL_FAILED, "PATRICIA TREE DEL FAILED"},
	{GLD_CREATE_HANDLE_FAILED, "HANDLE CREATE FAILED"},
	{GLD_TAKE_HANDLE_FAILED, "HANDLE TAKE FAILED"},
	{GLD_IPC_TASK_INIT, "FAILURE IN TASK INITIATION"},
	{GLD_IPC_SEND_FAIL, "IPC SEND FAILED"},
	{GLD_UNKNOWN_EVT_RCVD, "UNKNOWN EVENT RCVD"},
	{GLD_EVT_PROC_FAILED, "EVENT PROCESSING FAILED"},
	{GLD_A2S_EVT_PROC_FAILED, "ACTIVE TO STANDBY EVENT PROCESSING FAILED"},
	{GLD_MBCSV_DISPATCH_FAILURE, "GLD MBCSV DISPATCH FAILURE"},
	{GLD_UNKNOWN_GLND_EVT, "EVENT FROM UNKNOWN GLND"},
	{GLD_HEALTH_KEY_DEFAULT_SET, "GLD HEALTH KEY DEFAULT SET"},
	{GLD_MSG_FRMT_VER_INVALID, "GLD MSG FMT VERSION INVALID"},
	{GLD_ND_RESTART_WAIT_TMR_EXP, "NODE RESTART WAIT TIMER EXPIRED"},
	{GLD_ACTIVE_RMV_NODE, "NODE GETTING REMOVED ON ACTIVE"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/
const NCSFL_STR gld_memfail_set[] = {
	{GLD_CB_ALLOC_FAILED, "CONTROL BLOCK ALLOC FAILED"},
	{GLD_NODE_DETAILS_ALLOC_FAILED, "NODE_DETAILS ALLOC FAILED"},
	{GLD_EVT_ALLOC_FAILED, "EVENT ALLOC FAILED"},
	{GLD_RSC_INFO_ALLOC_FAILED, "RSC INFO ALLOC FAILED"},
	{GLD_A2S_EVT_ALLOC_FAILED, "A2S EVENT ALLOC FAILED"},
	{GLD_RES_MASTER_LIST_ALLOC_FAILED, "GLD RES MASTER LIST ALLOC FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/
const NCSFL_STR gld_api_set[] = {
	{GLD_SE_API_CREATE_SUCCESS, "GLD API CREATE SUCCESS"},
	{GLD_SE_API_CREATE_FAILED, "GLD API CREATE FAILED"},
	{GLD_SE_API_DESTROY_SUCCESS, "GLD API DESTROY SUCCESS"},
	{GLD_SE_API_DESTROY_FAILED, "GLD API DESTROY FAILED"},
	{GLD_SE_API_UNKNOWN, "GLD API UNKNOWN CALL"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Event 
 ******************************************************************************/
const NCSFL_STR gld_evt_set[] = {
	{GLD_EVT_RSC_OPEN, "EVT Processing rsc open "},
	{GLD_EVT_RSC_CLOSE, "EVT Processing rsc close "},
	{GLD_EVT_SET_ORPHAN, "EVT Processing set orphan "},
	{GLD_EVT_MDS_GLND_UP, "EVT Processing MDS GLND UP "},
	{GLD_EVT_MDS_GLND_DOWN, "EVT Processing MDS GLND DOWN"},
	{GLD_A2S_EVT_RSC_OPEN_SUCCESS, "GLD A2S EVT RSC OPEN SUCCESS"},
	{GLD_A2S_EVT_RSC_OPEN_FAILED, "GLD A2S EVT RSC OPEN FAILED"},
	{GLD_A2S_EVT_RSC_CLOSE_SUCCESS, "GLD A2S EVT RSC CLOSE SUCCESS"},
	{GLD_A2S_EVT_RSC_CLOSE_FAILED, "GLD A2S EVT RSC CLOSE FAILED"},
	{GLD_A2S_EVT_SET_ORPHAN_FAILED, "GLD A2S EVT SET ORPHAN FAILED"},
	{GLD_A2S_EVT_SET_ORPHAN_SUCCESS, "GLD A2S EVT SET ORPHAN SUCCESS"},
	{GLD_A2S_EVT_MDS_GLND_UP, "GLD A2S EVT MDS GLND UP "},
	{GLD_A2S_EVT_MDS_GLND_DOWN, "GLD A2S EVT MDS GLND DOWN"},
	{GLD_A2S_EVT_ADD_NODE_FAILED, "GLD A2S EVT ADD NODE FAILED"},
	{GLD_A2S_EVT_ADD_RSC_FAILED, "GLD A2S EVT ADD RSC FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for MBCSv events
 ******************************************************************************/
const NCSFL_STR gld_mbcsv_set[] = {
	{GLD_NCS_MBCSV_SVC_FAILED, "GLD NCS MBCSV SVC FAILED"},
	{GLD_MBCSV_INIT_SUCCESS, "GLD MBCSV INIT SUCCESS"},
	{GLD_MBCSV_INIT_FAILED, "GLD MBCSV INIT FAILED"},
	{GLD_MBCSV_OPEN_SUCCESS, "GLD MBCSV OPEN SUCCESS"},
	{GLD_MBCSV_OPEN_FAILED, "GLD MBCSV OPEN FAILED"},
	{GLD_MBCSV_CLOSE_FAILED, "GLD MBCSV CLOSE FAILED"},
	{GLD_MBCSV_GET_SEL_OBJ_SUCCESS, "GLD MBCSV GET SEL OBJ SUCCESS  "},
	{GLD_MBCSV_GET_SEL_OBJ_FAILURE, "GLD MBCSV GET SEL OBJ FAILURE"},
	{GLD_MBCSV_CHGROLE_FAILED, "GLD MBCSV CHGROLE FAILED"},
	{GLD_MBCSV_FINALIZE_FAILED, "GLD MBCSV FINALIZE FAILED"},
	{GLD_MBCSV_CALLBACK_SUCCESS, "GLD MBCSV CALLBACK SUCCESS"},
	{GLD_MBCSV_CALLBACK_FAILED, "GLD MBCSV CALLBACK FAILED"},
	{GLD_STANDBY_MSG_PROCESSING, "GLD STANDBY MSG PROCESSING"},
	{GLD_PROCESS_SB_MSG_FAILED, "GLD PROCESS SB MSG FAILED"},
	{GLD_STANDBY_CREATE_EVT, "GLD STANDBY CREATE EVT"},
	{GLD_A2S_RSC_OPEN_ASYNC_SUCCESS, "GLD A2S RSC OPEN ASYNC SUCCESS"},
	{GLD_A2S_RSC_OPEN_ASYNC_FAILED, "GLD A2S RSC OPEN ASYNC FAILED"},
	{GLD_A2S_RSC_CLOSE_ASYNC_SUCCESS, "GLD A2S RSC CLOSE ASYNC SUCCESS"},
	{GLD_A2S_RSC_CLOSE_ASYNC_FAILED, "GLD A2S RSC CLOSE ASYNC FAILED"},
	{GLD_A2S_RSC_SET_ORPHAN_ASYNC_SUCCESS, "GLD A2S RSC SET ORPHAN ASYNC SUCCESS"},
	{GLD_A2S_RSC_SET_ORPHAN_ASYNC_FAILED, "GLD A2S RSC SET ORPHAN ASYNC FAILED"},
	{GLD_A2S_RSC_NODE_DOWN_ASYNC_SUCCESS, "GLD A2S RSC NODE DOWN ASYNC SUCCESS"},
	{GLD_A2S_RSC_NODE_DOWN_ASYNC_FAILED, "GLD A2S RSC NODE DOWN ASYNC FAILED"},
	{GLD_A2S_RSC_NODE_UP_ASYNC_SUCCESS, "GLD A2S RSC NODE UP ASYNC SUCCESS"},
	{GLD_A2S_RSC_NODE_UP_ASYNC_FAILED, "GLD A2S RSC NODE UP ASYNC FAILED"},
	{GLD_A2S_RSC_NODE_OPERATIONAL_ASYNC_SUCCESS, "GLD A2S RSC NODE OPERATIONAL ASYNC SUCCESS"},
	{GLD_A2S_RSC_NODE_OPERATIONAL_ASYNC_FAILED, "GLD A2S RSC NODE OPERATIONAL ASYNC FAILED"},
	{GLD_STANDBY_RSC_OPEN_EVT_FAILED, "GLD STANDBY RSC OPEN EVT FAILED"},
	{GLD_STANDBY_RSC_CLOSE_EVT_FAILED, "GLD STANDBY RSC CLOSE EVT FAILED"},
	{GLD_STANDBY_RSC_SET_ORPHAN_EVT_FAILED, "GLD STANDBY RSC SET ORPHAN EVT FAILED"},
	{GLD_STANDBY_GLND_DOWN_EVT_FAILED, "GLD STANDBY GLND DOWN EVT FAILED"},
	{GLD_STANDBY_DESTADD_EVT_FAILED, "GLD STANDBY DESTADD EVT FAILED"},
	{GLD_STANDBY_DESTDEL_EVT_FAILED, "GLD STANDBY DESTDEL EVT FAILED"},
	{GLD_ENC_RESERVE_SPACE_FAILED, "GLD ENC RESERVE SPACE FAILED"},
	{EDU_EXEC_ASYNC_RSC_OPEN_EVT_FAILED, "EDU EXEC ASYNC RSC OPEN EVT FAILED"},
	{EDU_EXEC_ASYNC_RSC_CLOSE_EVT_FAILED, "EDU EXEC ASYNC RSC CLOSE EVT FAILED"},
	{EDU_EXEC_ASYNC_SET_ORPHAN_EVT_FAILED, "EDU EXEC ASYNC SET ORPHAN EVT FAILED"},
	{EDU_EXEC_ASYNC_GLND_DOWN_EVT_FAILED, "EDU EXEC ASYNC GLND DOWN EVT FAILED"},
	{EDU_EXEC_COLDSYNC_EVT_FAILED, "EDU EXEC COLDSYNC EVT FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Service providers (MDS, AMF)
 ******************************************************************************/
const NCSFL_STR gld_svc_prvdr_set[] = {
	{GLD_AMF_READINESS_CB_OUT_OF_SERVICE, "AMF ready state = OUT OF SERVICE"},
	{GLD_AMF_READINESS_CB_IN_SERVICE, "AMF ready state = IN SERVICE"},
	{GLD_AMF_READINESS_CB_STOPPING, "AMF ready state = STOPPING"},
	{GLD_AMF_CSI_SET_HA_STATE_ACTIVE, "AMF HA state = ACTIVE"},
	{GLD_AMF_CSI_SET_HA_STATE_STANDBY, "AMF HA state = STANDBY"},
	{GLD_AMF_CSI_SET_HA_STATE_QUIESCED, "AMF HA state = QUIESCED"},
	{GLD_AMF_RCVD_CONFIRM_CALL_BACK, "AMF RCVD confirmation callback"},
	{GLD_AMF_RCVD_TERMINATE_CALLBACK, "GLD AMF RCVD TERMINATE CALLBACK"},
	{GLD_AMF_RCVD_HEALTHCHK, "AMF RCVD health check"},
	{GLD_AMF_INIT_SUCCESS, "AMF Initialize success"},
	{GLD_AMF_INIT_ERROR, "AMF Initialize error"},
	{GLD_AMF_REG_ERROR, "AMF Registration Failed"},
	{GLD_AMF_REG_SUCCESS, "AMF Registration Success"},
	{GLD_AMF_SEL_OBJ_GET_ERROR, "AMF Selection object get error"},
	{GLD_AMF_DISPATCH_ERROR, "AMF Selection object get error"},
	{GLD_AMF_HLTH_CHK_START_DONE, "AMF Health Check started"},
	{GLD_AMF_HLTH_CHK_START_FAIL, "AMF Health Check start failed"},
	{GLD_MDS_INSTALL_FAIL, "MDS Install failed"},
	{GLD_MDS_INSTALL_SUCCESS, "MDS Install success"},
	{GLD_MDS_UNINSTALL_FAIL, "MDS Uninstall failed"},
	{GLD_MDS_SUBSCRIBE_FAIL, "MDS Subscription failed"},
	{GLD_MDS_SEND_ERROR, "MDS Send failed"},
	{GLD_MDS_CALL_BACK_ERROR, "MDS Call back error"},
	{GLD_MDS_UNKNOWN_SVC_EVT, "MDS unsubscribed svc evt"},
	{GLD_MDS_VDEST_DESTROY_FAIL, "MDS vdest destroy failed"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Global lock operations
 ******************************************************************************/
const NCSFL_STR gld_lck_oper_set[] = {
	{GLD_OPER_RSC_OPER_ERROR, "Rsc operation for unopened rsc"},
	{GLD_OPER_RSC_ID_ALLOC_FAIL, "Unable to allocate Rsc id"},
	{GLD_OPER_MASTER_RENAME_ERR, "error in designating new master"},
	{GLD_OPER_SET_ORPHAN_ERR, "Set orphan req from non-master"},
	{GLD_OPER_INCORRECT_STATE, "Incorrect state"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Timer Events
 ******************************************************************************/
const NCSFL_STR gld_timer_set[] = {
	{GLD_TIMER_START_FAIL, "GLD TIMER START FAILURE "},
	{GLD_TIMER_STOP_FAIL, "GLD TIMER STOP FAILURE "},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET gld_str_set[] = {
	{GLD_FC_HDLN, 0, (NCSFL_STR *)gld_hdln_set},
	{GLD_FC_MEMFAIL, 0, (NCSFL_STR *)gld_memfail_set},
	{GLD_FC_API, 0, (NCSFL_STR *)gld_api_set},
	{GLD_FC_EVT, 0, (NCSFL_STR *)gld_evt_set},
	{GLD_FC_SVC_PRVDR, 0, (NCSFL_STR *)gld_svc_prvdr_set},
	{GLD_FC_LCK_OPER, 0, (NCSFL_STR *)gld_lck_oper_set},
	{GLD_FC_MBCSV, 0, (NCSFL_STR *)gld_mbcsv_set},
	{GLD_FC_TIMER, 0, (NCSFL_STR *)gld_timer_set},
	{0, 0, 0}
};

NCSFL_FMAT gld_fmat_set[] = {
	{GLD_LID_HDLN, NCSFL_TYPE_TICLL, "%s GLD HEADLINE : %s : %s : %u : node_id %d\n"},
	{GLD_LID_MEMFAIL, NCSFL_TYPE_TICL, "%s GLD MEMERR: %s : %s : %u \n"},
	{GLD_LID_API, NCSFL_TYPE_TICL, "%s GLD API: %s : %s : %u \n"},
	{GLD_LID_EVT, NCSFL_TYPE_TICLLL, "%s GLD EVT: %s : %s : %u <rsc id:%ld> <rcvd frm:%ld>\n"},
	{GLD_LID_SVC_PRVDR, NCSFL_TYPE_TICL, "%s GLD SVC PRVDR: %s : %s : %u \n"},
	{GLD_LID_LCK_OPER, NCSFL_TYPE_TICLCLL,
	 "%s GLD LCK OPER: %s : %s : %u <rsc name:%s> <rsc id:%ld> <rcvd frm:%ld>\n"},
	{GLD_LID_MBCSV, NCSFL_TYPE_TICL, "%s GLD MBCSV: %s : %s : %u \n"},
	{GLD_LID_TIMER, NCSFL_TYPE_TICLL, "%s GLD TIMER: %s : %s : %u type :%ld \n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC gld_ascii_spec = {
	NCS_SERVICE_ID_GLD,	/* GLD subsystem */
	GLSV_LOG_VERSION,	/* GLD revision ID */
	"GLD",
	(NCSFL_SET *)gld_str_set,	/* GLD const strings referenced by index */
	(NCSFL_FMAT *)gld_fmat_set,	/* GLD string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  ncsgld_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uint32_t gld_reg_strings()
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &gld_ascii_spec;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    gld_unreg_strings

  DESCRIPTION:       Function is used for deregistering the canned strings with the DTS.

*****************************************************************************/
uint32_t gld_unreg_strings()
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_GLD;
	arg.info.dereg_ascii_spec.version = GLSV_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : glsv_log_str_lib_req
 *
 * Description   : Library entry point for glsv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t glsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_SUCCESS;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = gla_reg_strings();
		res = glnd_reg_strings();
		res = gld_reg_strings();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = gla_unreg_strings();
		res = glnd_unreg_strings();
		res = gld_unreg_strings();
		break;

	default:
		break;
	}
	return (res);
}

#endif
