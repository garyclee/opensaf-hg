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
  FILE NAME: CPA_LOGSTR.C

  DESCRIPTION: This file contains all the log string related to CPA.

******************************************************************************/

#if(NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "cpa_log.h"
#include "cpsv.h"

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR cpa_hdln_set[] = {
	{CPA_SE_API_CREATE_SUCCESS, "Library Init Success"},
	{CPA_SE_API_CREATE_FAILED, "Library Init Failed"},
	{CPA_SE_API_DESTROY_SUCCESS, "CPA - Agent Destroy Success"},
	{CPA_SE_API_DESTROY_FAILED, "CPA - Agent Destroy Failed"},
	{CPA_SE_API_UNKNOWN_REQUEST, "CPA - SE API Unknown Request"},
	{CPA_CREATE_HANDLE_FAILED, "CPA - Handle Create Failed"},
	{CPA_CB_RETRIEVAL_FAILED, "CPA - Handle Retrival Failed"},
	{CPA_CB_LOCK_INIT_FAILED, "CPA - Lock init Failed"},
	{CPA_CB_LOCK_TAKE_FAILED, "CPA - Lock Take Failed"},
	{CPA_EDU_INIT_FAILED, "CPA - EDU Init Failed"},
	{CPA_MDS_REGISTER_FAILED, "CPA - MDS Register Failed"},
	{CPA_MDS_GET_HANDLE_FAILED, "CPA - MDS Get Handle Failed"},
	{CPA_MDS_INSTALL_FAILED, "CPA - MDS Install Failed"},
	{CPA_MDS_CALLBK_FAILURE, "CPA - MDS Callback Failed"},
	{CPA_MDS_CALLBK_UNKNOWN_OP, "CPA - MDS Callback Unknown"},
	{CPA_MDS_ENC_FLAT_FAILURE, "CPA - MDS Encode Flat Failed"},
	{CPA_VERSION_INCOMPATIBLE, "CPA - Version Incompatible"},
	{CPA_CLIENT_NODE_ADD_FAILED, "CPA - Client Node Add Failed"},
	{CPA_LOCAL_CKPT_NODE_ADD_FAILED, "CPA - Lcoal Ckpt Node Add Failed"},
	{CPA_GLOBAL_CKPT_NODE_ADD_FAILED, "CAP - Global Ckpt Node Add Failed"},
	{CPA_CB_CREATE_FAILED, "CPA - Instance Create Failed"},
	{CPA_CB_DESTROY_FAILED, "CPA - Insatnce destroy Failed"},
	{CPA_CPND_IS_DOWN, "CPA - CPND is Down "},
	{CPA_CLIENT_NODE_GET_FAILED, "CPA - Client Node Get Failed"},
	{CPA_IOVECTOR_CHECK_FAILED, "CPA - IOVector Check Failed in Write"},

	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR cpa_memfail_set[] = {
	{CPA_CB_ALLOC_FAILED, "CPA - Control Block Memory Alloc failed"},
	{CPA_EVT_ALLOC_FAILED, "CPA - CPA Evt Allocation Failed"},
	{CPA_CLIENT_NODE_ALLOC_FAILED, "CPA - Client Node Allocation Failed"},
	{CPA_LOCAL_CKPT_NODE_ALLOC_FAILED, "CPA - Local Ckpt Node Allocation Failed"},
	{CPA_GLOBAL_CKPT_NODE_ALLOC_FAILED, "CPA - Global Ckpt Node Allocation Failed"},
	{CPA_SECT_ITER_NODE_ALLOC_FAILED, "CPA - Section Iteration Node Allocation Failed"},
	{CPA_CALLBACK_INFO_ALLOC_FAILED, "CPA - Callback Allocation Failed"},
	{CPA_DATA_BUFF_ALLOC_FAILED, "CPA - User Buffer Allocation Failed"},
	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR cpa_api_set[] = {

	{CPA_API_CKPT_INIT_SUCCESS, "CPA - Ckpt Init Success"},
	{CPA_API_CKPT_INIT_FAIL, "CPA - Ckpt Init Failed"},
	{CPA_API_CKPT_SEL_OBJ_GET_SUCCESS, "CPA - Ckpt Selection Object Get Success"},
	{CPA_API_CKPT_SEL_OBJ_GET_FAIL, "CPA - Ckpt Selection Object Get Failed"},
	{CPA_API_CKPT_DISPATCH_SUCCESS, "CPA - Ckpt Dispatch Sucess"},
	{CPA_API_CKPT_DISPATCH_FAIL, "CPA - Ckpt Dispatch Failed"},
	{CPA_API_CKPT_FINALIZE_SUCCESS, "CPA - Ckpt Finalise Success"},
	{CPA_API_CKPT_FINALIZE_FAIL, "CPA - Ckpt Finalise Failed"},
	{CPA_API_CKPT_OPEN_SUCCESS, "CPA - Ckpt Open Success"},
	{CPA_API_CKPT_OPEN_FAIL, "CPA - Ckpt Open Failed"},
	{CPA_API_CKPT_ASYNC_OPEN_SUCCESS, "CPA - Ckpt Async Open Success"},
	{CPA_API_CKPT_ASYNC_OPEN_FAIL, "CPA - Ckpt Async Open Fail"},
	{CPA_API_CKPT_CLOSE_SUCCESS, "CPA - Ckpt Close Success"},
	{CPA_API_CKPT_CLOSE_FAIL, "CPA - Ckpt Close Fail"},
	{CPA_API_CKPT_UNLINK_SUCCESS, " CPA - Ckpt Unlink Success"},
	{CPA_API_CKPT_UNLINK_FAIL, "CPA - Ckpt Unlink Failed"},
	{CPA_API_CKPT_RDSET_SUCCESS, " CPA - Ckpt Retention Duration Success"},
	{CPA_API_CKPT_RDSET_FAILED, "CPA - Ckpt Retention Duration Failed"},
	{CPA_API_CKPT_AREP_SET_SUCCESS, "CPA - Ckpt Active Replica Set Success"},
	{CPA_API_CKPT_AREP_SET_FAILED, "CPA - Ckpt Active Replica Set Failed"},
	{CPA_API_CKPT_STATUS_GET_SUCCESS, "CPA - Ckpt Status Get Success"},
	{CPA_API_CKPT_STATUS_GET_FAILED, "CPA - Ckpt Status Get Failed"},
	{CPA_API_CKPT_SECT_CREAT_SUCCESS, "CPA - Ckpt Section Create Success"},
	{CPA_API_CKPT_SECT_CREAT_FAILED, " CPA - Ckpt Section Create Failed"},
	{CPA_API_CKPT_SECT_DEL_SUCCESS, "CPA - Ckpt Section Delete Success"},
	{CPA_API_CKPT_SECT_DEL_FAILED, "CPA - Ckpt Section Delete Failed"},
	{CPA_API_CKPT_SECT_EXP_SET_SUCCESS, "CPA - Ckpt Section Expiration Time Set Success"},
	{CPA_API_CKPT_SECT_EXP_SET_FAILED, "CPA - Ckpt Section Expiration Time Set Failed"},
	{CPA_API_CKPT_SECT_ITER_INIT_SUCCESS, "CPA - Ckpt Section Iteration Initialize Success"},
	{CPA_API_CKPT_SECT_ITER_INIT_FAILED, "CPA - Ckpt Section Iteration Initialize Failed"},
	{CPA_API_CKPT_SECT_ITER_NEXT_SUCCESS, "CPA - Ckpt Section Iteration Next Success"},
	{CPA_API_CKPT_SECT_ITER_NEXT_FAILED, "CPA - Ckpt Section Iteration Next Failed"},
	{CPA_API_CKPT_SECT_ITER_FIN_SUCCESS, "CPA - Ckpt Section Iteration Finalize Success"},
	{CPA_API_CKPT_SECT_ITER_FIN_FAILED, "CPA - Ckpt Section Iteration Finalize Failed"},
	{CPA_API_CKPT_WRITE_SUCCESS, "CPA - Ckpt Write Success"},
	{CPA_API_CKPT_WRITE_FAILED, "CPA - Ckpt Write Failed"},
	{CPA_API_CKPT_OVERWRITE_SUCCESS, "CPA - Ckpt OverWrite Success"},
	{CPA_API_CKPT_OVERWRITE_FAILED, "CPA - Ckpt OverWrite Failed"},
	{CPA_API_CKPT_READ_SUCCESS, " CPA - Ckpt Read Success"},
	{CPA_API_CKPT_READ_FAILED, " CPA - Ckpt Read Failed"},
	{CPA_API_CKPT_SYNC_SUCCESS, " CPA - Ckpt Sync Success"},
	{CPA_API_CKPT_SYNC_FAILED, " CPA - Ckpt Sync Failed"},
	{CPA_API_CKPT_SYNC_ASYNC_SUCCESS, "CPA - Ckpt Sync Async Success"},
	{CPA_API_CKPT_SYNC_ASYNC_FAILED, "CPA - Ckpt Sync Async Failed"},
	{CPA_API_CKPT_REG_ARR_CBK_SUCCESS, "CPA - Register Arrival Callback Success"},
	{CPA_API_CKPT_REG_ARR_CBK_FAILED, "CPA - Register Arrival Callback Failed"},
	{CPA_API_CKPT_SECT_ID_FREE_SUCCESS, "CPA - Section Id Free Success"},
	{CPA_API_CKPT_SECT_ID_FREE_FAILED, "CPA - Section Id Free Failed "},
	{0, 0}
};

const NCSFL_STR cpa_datasend_set[] = {
	{CPA_MDS_SEND_FAILURE, "CPA MDS Send Failed"},
	{CPA_MDS_SEND_TIMEOUT, "CPA MDS Send Timer Expiry"},
	{0, 0}
};

const NCSFL_STR cpa_db_set[] = {
	{CPA_LCL_CKPT_NODE_GET_FAILED, "CPA - Local Ckpt Get Failed"},
	{CPA_GBL_CKPT_FIND_ADD_FAILED, " CPA - Global Ckpt Find Add Failed"},
	{CPA_SECT_ITER_NODE_ADD_FAILED, " CPA - Section Iteration Node Add Failed"},
	{CPA_SECT_ITER_NODE_GET_FAILED, "CPA - Section Iteration Node Get Failed"},
	{CPA_SECT_ITER_NODE_DELETE_FAILED, "CPA - Section Iteration Node Delete Failed"},
	{CPA_BUILD_DATA_ACCESS_FAILED, "CPA - Build CPSV_CKPT_DATA From IOVector Failed"},
	{CPA_PROC_REPLICA_READ_FAILED, "CPA - Replica Read Failed "},
	{CPA_PROC_RMT_REPLICA_READ_FAILED, "CPA - Remote Replica Read Failed "},

	{0, 0}
};

const NCSFL_STR cpa_generic_set[] = {
	{CPA_API_SUCCESS, "API Processing Success"},
	{CPA_API_FAILED, "API Processing Failed"},
	{CPA_PROC_SUCCESS, "Processing Success"},
	{CPA_PROC_FAILED, "Processing Failed"},
	{CPA_MEM_ALLOC_FAILED, "Memory Alloc Failed"},
	{0, 0}
};

/*****************************************************************************\
 Build up the canned constant strings for the ASCII SPEC
\******************************************************************************/
NCSFL_SET cpa_str_set[] = {
	{CPA_FC_GEN, 0, (NCSFL_STR *)cpa_generic_set},
	{CPA_FC_HDLN, 0, (NCSFL_STR *)cpa_hdln_set},
	{CPA_FC_MEMFAIL, 0, (NCSFL_STR *)cpa_memfail_set},
	{CPA_FC_API, 0, (NCSFL_STR *)cpa_api_set},
	{CPA_FC_DATA_SEND, 0, (NCSFL_STR *)cpa_datasend_set},
	{CPA_FC_DB, 0, (NCSFL_STR *)cpa_db_set},
	{0, 0, 0}
};

NCSFL_FMAT cpa_fmat_set[] = {
	{CPA_LID_HDLN, NCSFL_TYPE_TI, "CPA %s : %s\n"},
	{CPA_LID_MEMFAIL, NCSFL_TYPE_TI, "CPA %s : %s\n"},
	{CPA_LID_API, NCSFL_TYPE_TI, "CPA %s : %s\n"},
	{CPA_LID_DATA_SEND, NCSFL_TYPE_TILL, "CPA %s : %s\n"},
	{CPA_LID_DB, NCSFL_TYPE_TI, "CPA %s : %s\n"},
	{CPA_LID_TICCL, NCSFL_TYPE_TICCL, "CPA: %s : %s: %s : %s:%lu \n"},
	{CPA_LID_TICCLL, NCSFL_TYPE_TICCLL, "CPA: %s : %s:  %s : %s:%lu : %lu \n"},
	{CPA_LID_TICCLLF, NCSFL_TYPE_TICCLLF, "CPA: %s : %s : %s : %s:%lu : %lu : %s\n"},
	{CPA_LID_TICCLLFF, NCSFL_TYPE_TICCLLFF, "CPA: %s : %s : %s : %s:%lu : %lu : %s : %s\n"},
	{CPA_LID_TICCLFFF, NCSFL_TYPE_TICCLFFF, "CPA: %s : %s : %s : %s:%lu : %s : %s : %s\n"},
	{CPA_LID_TICCLLFFF, NCSFL_TYPE_TICCLLFFF, "CPA: %s : %s : %s : %s:%lu : %lu : %s : %s : %s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC cpa_ascii_spec = {
	NCS_SERVICE_ID_CPA,	/* CPA subsystem */
	CPSV_LOG_VERSION,	/* CPA (CPSv-CPA) log version ID */
	"cpa",			/* CPA opening Banner in log */
	(NCSFL_SET *)cpa_str_set,	/* CPA const strings referenced by index */
	(NCSFL_FMAT *)cpa_fmat_set,	/* CPA string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/****************************************************************************\
  PROCEDURE NAME : cpa_log_ascii_reg
 
  DESCRIPTION    : This is the function which registers the CPA's logging
                   ascii set with the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uint32_t cpa_log_ascii_reg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;
	uint32_t rc = NCSCC_RC_SUCCESS;

	memset(&arg, 0, sizeof(arg));

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &cpa_ascii_spec;

	/* Regiser CPA Canned strings with DTSv */
	rc = ncs_dtsv_ascii_spec_api(&arg);
	return rc;
}

/****************************************************************************\
  PROCEDURE NAME : cpa_log_ascii_dereg
 
  DESCRIPTION    : This is the function which deregisters the CPA's logging
                   ascii set from the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
void cpa_log_ascii_dereg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_CPA;
	arg.info.dereg_ascii_spec.version = CPSV_LOG_VERSION;

	/* Deregister CPA Canned strings from DTSv */
	ncs_dtsv_ascii_spec_api(&arg);
	return;
}

#endif   /* (NCS_DTS == 1) */
