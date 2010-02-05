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

#if (NCS_HISV_LOG == 1)

#include "dts.h"
#include "hcd_log.h"
#include "hisv_logstr.h"

/******************************************************************************
 Logging stuff for Headlines
 ******************************************************************************/
const NCSFL_STR hcd_hdln_set[] = {
	{HCD_CREATE_HANDLE_FAILED, "HANDLE CREATE FAILED"},
	{HCD_TAKE_HANDLE_FAILED, "HANDLE TAKE FAILED"},
	{HCD_IPC_TASK_INIT, "FAILURE IN TASK INITIATION"},
	{HCD_IPC_SEND_FAIL, "IPC SEND FAILED"},
	{HCD_UNKNOWN_EVT_RCVD, "UNKNOWN EVENT RCVD"},
	{HCD_EVT_PROC_FAILED, "EVENT PROCESSING FAILED"},
	{HCD_UNKNOWN_HAM_EVT, "UNKNOWN EVENT FROM HAM"},
	{HCD_HEALTH_KEY_DEFAULT_SET, "HCD HEALTH KEY DEFAULT SET"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Mem Fail
 ******************************************************************************/
const NCSFL_STR hcd_memfail_set[] = {
	{HCD_CB_ALLOC_FAILED, "CONTROL BLOCK ALLOC FAILED"},
	{HCD_NODE_DETAILS_ALLOC_FAILED, "NODE_DETAILS ALLOC FAILED"},
	{HCD_EVT_ALLOC_FAILED, "EVENT ALLOC FAILED"},
	{HCD_RSC_INFO_ALLOC_FAILED, "RSC INFO ALLOC FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for API
 ******************************************************************************/
const NCSFL_STR hcd_api_set[] = {
	{HCD_SE_API_CREATE_SUCCESS, "HCD API CREATE SUCCESS"},
	{HCD_SE_API_CREATE_FAILED, "HCD API CREATE FAILED"},
	{HCD_SE_API_DESTROY_SUCCESS, "HCD API DESTROY SUCCESS"},
	{HCD_SE_API_DESTROY_FAILED, "HCD API DESTROY FAILED"},
	{HCD_SE_API_UNKNOWN, "HCD API UNKNOWN CALL"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for Firmware progress events.
 ******************************************************************************/
const NCSFL_STR hcd_fwprog_set[] = {
	/* Firmware progress event logging messages */
	{HCD_SE_FWPROG_BOOT_SUCCESS, "Completed System Boot Process",},
	{HCD_SE_FWPROG_NODE_INIT_SUCCESS, "Node Initialization successful"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Firmware error events.
 ******************************************************************************/
const NCSFL_STR hcd_fwerr_set[] = {
	/* Firmware error event logging messages */
	{HCD_SE_FWERR_HPM_INIT_FAILED, "HAPS[HPM] Initialization Failed"},
	{HCD_SE_FWERR_HLFM_INIT_FAILED, "HAPS[HLFM] Initialization Failed"},
	{HCD_SE_FWERR_SWITCH_INIT_FAILED, "HAPS[SW Node Director] Initialization Failed"},
	{HCD_SE_FWERR_LHC_DMN_INIT_FAILED, "HAPS[LHCp Daemon] Initialization Failed"},
	{HCD_SE_FWERR_LHC_RSP_INIT_FAILED, "HAPS[LHC Responder] Initialization Failed"},
	{HCD_SE_FWERR_NW_SCRIPT_INIT_FAILED, "HAPS[N/W Script] Initialization Failed"},
	{HCD_SE_FWERR_DRBD_INIT_FAILED, "HAPS[DRBD Device] Initialization Failed"},
	{HCD_SE_FWERR_TIPC_INIT_FAILED, "NCS[TIPC] Initialization Failed"},
	{HCD_SE_FWERR_IFSD_INIT_FAILED, "NCS[IFSD] Initialization Failed"},
	{HCD_SE_FWERR_DTSV_INIT_FAILED, "NCS[DTSV] Initialization Failed"},
	{HCD_SE_FWERR_GLND_INIT_FAILED, "NCS[GLND] Initialization Failed"},
	{HCD_SE_FWERR_EDSV_INIT_FAILED, "NCS[EDSV] Initialization Failed"},
	{HCD_SE_FWERR_NCS_INIT_FAILED, "NCS Initialization Failed"},
	{HCD_SE_FWERR_UNKNOWN_EVT_FAILED, "NCS Initialization Unknown Error"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for Event
 ******************************************************************************/
const NCSFL_STR hcd_evt_set[] = {
	{HCD_EVT_NULL_STR, " "},
	{HCD_EVT_RSC_CLOSE, "EVT Processing rsc close "},
	{HCD_EVT_SET_ORPHAN, "EVT Processing set orphan "},
	{HCD_EVT_MDS_HAM_UP, "EVT Processing MDS HAM UP "},
	{HCD_EVT_MDS_HAM_DOWN, "EVT Processing MDS HAM DOWN"},
	{0, 0}
};

/******************************************************************************
 Logging Hotswap event status
 ******************************************************************************/
const NCSFL_STR hcd_evt_hotswap[] = {
	{HCD_HOTSWAP_NULL_STR, " "},
	{HCD_HOTSWAP_CURR_STATE, "Current Hotswap state of board in physical slot"},
	{HCD_HOTSWAP_PREV_STATE, "Previous Hotswap state of board in physical slot"},
	{HCD_NON_HOTSWAP_TYPE, "Non hotswap HPI event from entity type"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Service providers (MDS, AMF)
 ******************************************************************************/
const NCSFL_STR hcd_svc_prvdr_set[] = {
	{HCD_AMF_READINESS_CB_OUT_OF_SERVICE, "AMF ready state = OUT OF SERVICE"},
	{HCD_AMF_READINESS_CB_IN_SERVICE, "AMF ready state = IN SERVICE"},
	{HCD_AMF_READINESS_CB_STOPPING, "AMF ready state = STOPPING"},
	{HCD_AMF_CSI_SET_HA_STATE_ACTIVE, "AMF HA state = ACTIVE"},
	{HCD_AMF_CSI_SET_HA_STATE_STANDBY, "AMF HA state = STANDBY"},
	{HCD_AMF_CSI_SET_HA_STATE_QUIESCED, "AMF HA state = QUIESCED"},
	{HCD_AMF_RCVD_CONFIRM_CALL_BACK, "AMF RCVD confirmation callback"},
	{HCD_AMF_RCVD_HEALTHCHK, "AMF RCVD health check"},
	{HCD_AMF_INIT_SUCCESS, "AMF Initialize success"},
	{HCD_AMF_INIT_ERROR, "AMF Initialize error"},
	{HCD_AMF_REG_ERROR, "AMF Registration Failed"},
	{HCD_AMF_SEL_OBJ_GET_ERROR, "AMF Selection object get error"},
	{HCD_AMF_DISPATCH_ERROR, "AMF Selection object get error"},
	{HCD_AMF_HLTH_CHK_START_DONE, "AMF Health Check started"},
	{HCD_AMF_HLTH_CHK_START_FAIL, "AMF Health Check start failed"},
	{HCD_MDS_INSTALL_FAIL, "MDS Install failed"},
	{HCD_MDS_UNINSTALL_FAIL, "MDS Uninstall failed"},
	{HCD_MDS_SUBSCRIBE_FAIL, "MDS Subscription failed"},
	{HCD_MDS_SEND_ERROR, "MDS Send failed"},
	{HCD_MDS_CALL_BACK_ERROR, "MDS Call back error"},
	{HCD_MDS_UNKNOWN_SVC_EVT, "MDS unsubscribed svc evt"},
	{HCD_MDS_VDEST_DESTROY_FAIL, "MDS vdest destroy failed"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Global lock operations
 ******************************************************************************/
const NCSFL_STR hcd_lck_oper_set[] = {
	{HCD_OPER_RSC_OPER_ERROR, "Rsc operation for unopened rsc"},
	{HCD_OPER_RSC_ID_ALLOC_FAIL, "Unable to allocate Rsc id"},
	{HCD_OPER_MASTER_RENAME_ERR, "error in designating new master"},
	{HCD_OPER_SET_ORPHAN_ERR, "Set orphan req from non-master"},
	{HCD_OPER_INCORRECT_STATE, "Incorrect state"},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET hcd_str_set[] = {
	{HCD_FC_HDLN, 0, (NCSFL_STR *)hcd_hdln_set},
	{HCD_FC_MEMFAIL, 0, (NCSFL_STR *)hcd_memfail_set},
	{HCD_FC_API, 0, (NCSFL_STR *)hcd_api_set},
	{HCD_FC_FWPROG, 0, (NCSFL_STR *)hcd_fwprog_set},
	{HCD_FC_FWERR, 0, (NCSFL_STR *)hcd_fwerr_set},
	{HCD_FC_EVT, 0, (NCSFL_STR *)hcd_evt_set},
	{HCD_FC_SVC_PRVDR, 0, (NCSFL_STR *)hcd_svc_prvdr_set},
	{HCD_FC_LCK_OPER, 0, (NCSFL_STR *)hcd_lck_oper_set},
	{HCD_FC_HOTSWAP, 0, (NCSFL_STR *)hcd_evt_hotswap},
	{0, 0, 0}
};

NCSFL_FMAT hcd_fmat_set[] = {
	{HCD_LID_HDLN, NCSFL_TYPE_TI, "%s HCD HEADLINE : %s\n"},
	{HCD_LID_MEMFAIL, NCSFL_TYPE_TI, "%s HCD MEMERR: %s\n"},
	{HCD_LID_API, NCSFL_TYPE_TI, "%s HCD API: %s\n"},
	{HCD_LID_FWPROG, NCSFL_TYPE_TILL, "%s HCD FWPROG: %s : Board Number %ld\n"},
	{HCD_LID_FWERR, NCSFL_TYPE_TILL, "%s HCD FWERR: %s : Board Number %ld\n"},
	{HCD_LID_EVT, NCSFL_TYPE_TILL, "%s HCD EVT: %s <rsc id:%ld> <rcvd frm:%ld>\n"},
	{HCD_LID_SVC_PRVDR, NCSFL_TYPE_TI, "%s HCD SVC PRVDR: %s\n"},
	{HCD_LID_LCK_OPER, NCSFL_TYPE_TICLL, "%s HCD LCK OPER: %s <rsc name:%s> <rsc id:%ld> <rcvd frm:%ld>\n"},
	{HCD_LID_GEN_INFO, NCSFL_TYPE_TC, "%s HCD LOG: %s "},
	{HCD_LID_GEN_INFO2, "TCP", "%s HCD GEN: %s %s "},
	{HCD_LID_GEN_INFO3, "TCC", "%s HCD GEN: %s %s "},
	{HCD_LID_GEN_INFO4, NCSFL_TYPE_TILL, "%s HCD EVT: %s %ld is:\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC hcd_ascii_spec = {
	NCS_SERVICE_ID_HCD,	/* HCD subsystem */
	HISV_LOG_VERSION,	/* HCD log version ID */
	"HISV",
	(NCSFL_SET *)hcd_str_set,	/* HCD const strings referenced by index */
	(NCSFL_FMAT *)hcd_fmat_set,	/* HCD string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  hisv_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uns32 hisv_reg_strings()
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &hcd_ascii_spec;
	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : hisv_log_str_lib_req
 *
 * Description   : Library entry point for hisv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 hisv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = hisv_reg_strings();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = hisv_dereg_strings();
		break;

	default:
		break;
	}
	return (res);
}

/*****************************************************************************

  hisv_dereg_strings

  DESCRIPTION: Function is used for registering the canned strings with the HISV.

*****************************************************************************/

uns32 hisv_dereg_strings()
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_HCD;
	arg.info.dereg_ascii_spec.version = HISV_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}
#else				/* (NCS_HISV_LOG == 0) */

/*****************************************************************************

  hisv_reg_strings

  DESCRIPTION: When NCS_HISV_LOG is disabled, we don't register a thing,

*****************************************************************************/

uns32 hisv_reg_strings(char *fname)
{
	return NCSCC_RC_SUCCESS;
}

#endif
