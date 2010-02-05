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

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "saAis.h"
#include "fma_log.h"

#if (NCS_DTS == 1)

/******************************************************************************
                          Canned string for SE-API
 ******************************************************************************/
const NCSFL_STR fma_seapi_set[] = {
	{FMA_LOG_SEAPI_CREATE, "FMA Create"},
	{FMA_LOG_SEAPI_DESTROY, "FMA Destroy"},
	{FMA_LOG_SEAPI_INSTANTIATE, "FMA Instantiate"},
	{FMA_LOG_SEAPI_UNINSTANTIATE, "FMA Uninstantiate"},
	{FMA_LOG_SEAPI_SUCCESS, "Success"},
	{FMA_LOG_SEAPI_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                          Canned string for MDS
 ******************************************************************************/
const NCSFL_STR fma_mds_set[] = {
	{FMA_LOG_MDS_HDLS_GET, "MDS Adest Get Hdl"},
	{FMA_LOG_MDS_REG, "MDS Register"},
	{FMA_LOG_MDS_INSTALL, "MDS Install"},
	{FMA_LOG_MDS_SUBSCRIBE, "MDS Subscribe"},
	{FMA_LOG_MDS_UNREG, "MDS Un-Register"},
	{FMA_LOG_MDS_SEND, "MDS Send"},
	{FMA_LOG_MDS_SENDRSP, "MDS Send-Rsp"},
	{FMA_LOG_MDS_RCV_CBK, "MDS Rcv Cbk"},
	{FMA_LOG_MDS_CPY_CBK, "MDS Copy Cbk"},
	{FMA_LOG_MDS_SVEVT_CBK, "MDS Svc Evt Cbk"},
	{FMA_LOG_MDS_ENC_CBK, "MDS Enc Cbk"},
	{FMA_LOG_MDS_FLAT_ENC_CBK, "MDS Flat Enc Cbk"},
	{FMA_LOG_MDS_DEC_CBK, "MDS Dec Cbk"},
	{FMA_LOG_MDS_FLAT_DEC_CBK, "MDS Flat Dec Cbk"},
	{FMA_LOG_MDS_SUCCESS, "Success"},
	{FMA_LOG_MDS_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                          Canned string for LOCK
 ******************************************************************************/
const NCSFL_STR fma_lock_set[] = {
	{FMA_LOG_LOCK_INIT, "Lock Init"},
	{FMA_LOG_LOCK_FINALIZE, "Lock Finalize"},
	{FMA_LOG_LOCK_TAKE, "Lock Take"},
	{FMA_LOG_LOCK_GIVE, "Lock Give"},
	{FMA_LOG_LOCK_SUCCESS, "Success"},
	{FMA_LOG_LOCK_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                       Canned string for control block
 ******************************************************************************/
const NCSFL_STR fma_cb_set[] = {
	{FMA_LOG_CB_CREATE, "CB Create"},
	{FMA_LOG_CB_DESTROY, "CB Destroy"},
	{FMA_LOG_CB_HDL_ASS_CREATE, "CB Handle Manager Association Create"},
	{FMA_LOG_CB_HDL_ASS_REMOVE, "CB Handle Manager Association Remove"},
	{FMA_LOG_CB_RETRIEVE, "CB Retrieve"},
	{FMA_LOG_CB_RETURN, "CB Return"},
	{FMA_LOG_CB_SUCCESS, "Success"},
	{FMA_LOG_CB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for selection object
 ******************************************************************************/
const NCSFL_STR fma_sel_obj_set[] = {
	{FMA_LOG_SEL_OBJ_CREATE, "Selection Object Create"},
	{FMA_LOG_SEL_OBJ_DESTROY, "Selection Object Destroy"},
	{FMA_LOG_SEL_OBJ_IND_SEND, "Selection Object Ind Send"},
	{FMA_LOG_SEL_OBJ_IND_REM, "Selection Object Ind Rem"},
	{FMA_LOG_SEL_OBJ_SUCCESS, "Success"},
	{FMA_LOG_SEL_OBJ_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                          Canned string for FMA APIs
 ******************************************************************************/
/* ensure that the string set ordering matches that of API type definition */
const NCSFL_STR fma_api_set[] = {
	{FMA_LOG_API_FINALIZE, "fmFinalize()"},
	{FMA_LOG_API_RESP, "fmResponse()"},
	{FMA_LOG_API_INITIALIZE, "fmInitialize()"},
	{FMA_LOG_API_SEL_OBJ_GET, "fmSelectionObjectGet()"},
	{FMA_LOG_API_DISPATCH, "fmDispatch()"},
	{FMA_LOG_API_NODE_RESET_IND, "fmNodeResetInd()"},
	{FMA_LOG_API_CAN_SWITCHOVER, "fmCanSwitchoverProceed()"},
	{FMA_LOG_API_NODE_HB_IND, "fmNodeHeartbeatInd()"},
	{FMA_LOG_API_ERR_SA_OK, "Ok"},
	{FMA_LOG_API_ERR_SA_LIBRARY, "Library Error"},
	{FMA_LOG_API_ERR_SA_VERSION, "Version Error"},
	{FMA_LOG_API_ERR_SA_INVALID_PARAM, "Invalid Parameter Error"},
	{FMA_LOG_API_ERR_SA_NO_MEMORY, "No Memory Error"},
	{FMA_LOG_API_ERR_SA_BAD_HANDLE, "Bad Handle Error"},
	{FMA_LOG_API_ERR_SA_EMPTY_LIST, "List is empty"},
	{FMA_LOG_API_ERR_FAILED_OPERATION, "Operation Failed"},
	{0, 0}
};

/******************************************************************************
                     Canned string for FMA handle database
 ******************************************************************************/
const NCSFL_STR fma_hdl_db_set[] = {
	{FMA_LOG_HDL_DB_CREATE, "Handle Database Create "},
	{FMA_LOG_HDL_DB_DESTROY, "Handle Database Destroy "},
	{FMA_LOG_HDL_DB_REC_CREATE, "Handle DataBase Record Create"},
	{FMA_LOG_HDL_DB_REC_DESTROY, "Handle DataBase Record Destroy"},
	{FMA_LOG_HDL_DB_REC_ADD, "Handle DataBase Record Add"},
	{FMA_LOG_HDL_DB_REC_CBK_ADD, "Handle DataBase Record Callback Add"},
	{FMA_LOG_HDL_DB_REC_DEL, "Handle DataBase Record Delete"},
	{FMA_LOG_HDL_DB_REC_GET, "Handle DataBase Record Get"},
	{FMA_LOG_HDL_DB_SUCCESS, "Success"},
	{FMA_LOG_HDL_DB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
   Logging stuff for Mem Fail
 ******************************************************************************/
const NCSFL_STR fma_mem_set[] = {
	{FMA_LOG_MBX_EVT_ALLOC, "Mbx Event Alloc "},
	{FMA_LOG_FMA_FM_MSG_ALLOC, "FMA FM Msg alloc"},
	{FMA_LOG_HDL_REC_ALLOC, "HDL REC Alloc"},
	{FMA_LOG_CB_ALLOC, "CB Alloc"},
	{FMA_LOG_PEND_CBK_REC_ALLOC, "Pending Callback Record Alloc"},
	{FMA_LOG_CBK_INFO_ALLOC, "HDL REC Callback Info Alloc"},
	{FMA_LOG_MEM_ALLOC_SUCCESS, "Success "},
	{FMA_LOG_MEM_ALLOC_FAILURE, "Failure "},
	{0, 0}
};

/******************************************************************************
                       Canned string for Mailbox
 ******************************************************************************/
const NCSFL_STR fma_mbx_set[] = {
	{FMA_LOG_MBX_CREATE, "Mbx Create"},
	{FMA_LOG_MBX_ATTACH, "Mbx Attach"},
	{FMA_LOG_MBX_DESTROY, "Mbx Destroy"},
	{FMA_LOG_MBX_DETACH, "Mbx Detach"},
	{FMA_LOG_MBX_SEND, "Mbx Send"},
	{FMA_LOG_MBX_SUCCESS, "Success"},
	{FMA_LOG_MBX_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                       Canned string for Task
 ******************************************************************************/
const NCSFL_STR fma_task_set[] = {
	{FMA_LOG_TASK_CREATE, "Task Create"},
	{FMA_LOG_TASK_START, "Task Start"},
	{FMA_LOG_TASK_RELEASE, "Task Release"},
	{FMA_LOG_TASK_SUCCESS, "Success"},
	{FMA_LOG_TASK_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                       Canned string for Callbacks
 ******************************************************************************/
const NCSFL_STR fma_cbk_set[] = {
	{FMA_LOG_CBK_NODE_RESET_IND, "Node Reset Indication Callback: "},
	{FMA_LOG_CBK_SWOVER_REQ, "Switchover Request Callback: "},
	{FMA_LOG_CBK_SUCCESS, "Success"},
	{FMA_LOG_CBK_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                    Canned string for miscellaneous FMA events
 ******************************************************************************/
const NCSFL_STR fma_misc_set[] = {
	{FMA_FUNC_ENTRY, "Function Entry "},
	{FMA_INVALID_PARAM, "Invalid Parameter"},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/
NCSFL_SET fma_str_set[] = {
	{FMA_FC_SEAPI, 0, (NCSFL_STR *)fma_seapi_set},
	{FMA_FC_MDS, 0, (NCSFL_STR *)fma_mds_set},
	{FMA_FC_LOCK, 0, (NCSFL_STR *)fma_lock_set},
	{FMA_FC_CB, 0, (NCSFL_STR *)fma_cb_set},
	{FMA_FC_SEL_OBJ, 0, (NCSFL_STR *)fma_sel_obj_set},
	{FMA_FC_API, 0, (NCSFL_STR *)fma_api_set},
	{FMA_FC_HDL_DB, 0, (NCSFL_STR *)fma_hdl_db_set},
	{FMA_FC_MEM, 0, (NCSFL_STR *)fma_mem_set},
	{FMA_FC_MBX, 0, (NCSFL_STR *)fma_mbx_set},
	{FMA_FC_TASK, 0, (NCSFL_STR *)fma_task_set},
	{FMA_FC_MISC, 0, (NCSFL_STR *)fma_misc_set},
	{FMA_FC_CBK, 0, (NCSFL_STR *)fma_cbk_set},
	{0, 0, 0}
};

NCSFL_FMAT fma_fmat_set[] = {
	/* <SE-API Create/Destroy> <Success/Failure> */
	{FMA_LID_SEAPI, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <MDS Register/Install/...> <Success/Failure> */
	{FMA_LID_MDS, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <Lock Init/Finalize/Take/Give> <Success/Failure> */
	{FMA_LID_LOCK, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <CB Create/Destroy/...> <Success/Failure> */
	{FMA_LID_CB, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <Sel Obj Create/Destroy/...> <Success/Failure> */
	{FMA_LID_SEL_OBJ, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* Processed <API Name>: <Ok/...(fma-err-code)>             */
	{FMA_LID_API, NCSFL_TYPE_TII, "[%s] Processed %s: %s \n"},

	/* <Hdl DB Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
	{FMA_LID_HDL_DB, NCSFL_TYPE_TIIL, "[%s] %s %s for Handle: %u\n"},

	{FMA_LID_MEM, NCSFL_TYPE_TIICL, "%s FMA: %s %s at %s:%d\n"},

	{FMA_LID_MBX, NCSFL_TYPE_TII, "%s FMA: %s %s \n"},

	{FMA_LID_TASK, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	{FMA_LID_MISC, NCSFL_TYPE_TIC, "[%s] %s %s\n"},

	{FMA_LID_CBK, NCSFL_TYPE_TII, "[%s] %s Processing %s\n"},

	{0, 0, 0}
};

NCSFL_ASCII_SPEC fma_ascii_spec = {
	NCS_SERVICE_ID_FMA,	/* FMA service id */
	FMA_LOG_VERSION,	/* FMA log version id */
	"FMA",			/* used for generating log filename */
	(NCSFL_SET *)fma_str_set,	/* FMA const strings ref by index */
	(NCSFL_FMAT *)fma_fmat_set,	/* FMA string format info */
	0,			/* placeholder for str_set cnt */
	0			/* placeholder for fmat_set cnt */
};

/****************************************************************************
  Name          : fma_log_str_lib_req

  Description   : Register ASCII specs with DTSV. 

  Arguments     : Pointer to NCS_LIB_REQ_INFO

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..

  Notes         : None.
 *****************************************************************************/
uns32 fma_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 res = NCSCC_RC_FAILURE;
	NCS_DTSV_REG_CANNED_STR arg;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
		arg.info.reg_ascii_spec.spec = &fma_ascii_spec;
		res = ncs_dtsv_ascii_spec_api(&arg);
		break;

	case NCS_LIB_REQ_DESTROY:
		arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
		arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_FMA;
		arg.info.dereg_ascii_spec.version = FMA_LOG_VERSION;
		res = ncs_dtsv_ascii_spec_api(&arg);
		break;

	default:
		break;

	}

	return (res);
}

#endif
