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
#include "gla_log.h"
#include "glsv_logstr.h"

const NCSFL_STR gla_hdln_set[] = {
	{GLA_CB_CREATE_FAILED, "GLA CB CREATION FAILED"},
	{GLA_CB_DESTROY_FAILED, "GLA CB DELETION FAILED"},
	{GLA_CLIENT_TREE_INIT_FAILED, "GLA CLIENT TREE INIT FAILED"},
	{GLA_CLIENT_TREE_ADD_FAILED, "GLA CLIENT TREE ADD FAILED"},
	{GLA_CLIENT_TREE_DEL_FAILED, "GLA CLIENT TREE DEL FAILED"},
	{GLA_TAKE_HANDLE_FAILED, "GLA TAKE HANDLE FAILED"},
	{GLA_CREATE_HANDLE_FAILED, "GLA HANDLE CREATION FAILED"},
	{GLA_MDS_REGISTER_SUCCESS, "GLA MDS REGISTER SUCCESS"},
	{GLA_MDS_REGISTER_FAILED, "GLA MDS REGISTER FAILED"},
	{GLA_SE_API_CREATE_SUCCESS, "GLA SE LIB API CREATE SUCCESS"},
	{GLA_SE_API_CREATE_FAILED, "GLA SE LIB API CREATE FAILED"},
	{GLA_SE_API_DESTROY_SUCCESS, "GLA SE LIB API DESTROY SUCCESS"},
	{GLA_SE_API_DESTROY_FAILED, "GLA SE LIB API DESTROY FAILED"},
	{GLA_CB_RETRIEVAL_FAILED, "GLA CB RETRIEVAL FAILED"},
	{GLA_VERSION_INCOMPATIBLE, "GLA SAF VERSION INCOMPATIBLE"},
	{GLA_MDS_GET_HANDLE_FAILED, "GLA MDS GET HANDLE FAILED "},
	{GLA_MDS_CALLBK_FAILURE, "GLA MDS CALLBACK CALL FAILURE"},
	{GLA_GLND_SERVICE_UP, "GLA RECIEVED GLND SERVICE UP"},
	{GLA_GLND_SERVICE_DOWN, "GLA RECIEVED  GLND SERVICE DOWN "},
	{GLA_MDS_ENC_FLAT_FAILURE, "GLA MDS ENC FLAT FAILURE"},
	{GLA_MDS_DEC_FLAT_FAILURE, "GLA MDS DEC FLAT FAILURE"},
	{GLA_AGENT_REGISTER_FAILURE, "GLA AGENT REGISTER FAILURE"},
	{GLA_MSG_FRMT_VER_INVALID, "GLA MESSAGE FORMAT VERSION INVALID"},
	{GLA_MDS_DEC_FAILURE, "GLA MDS DEC FAILURE"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/
const NCSFL_STR gla_memfail_set[] = {
	{GLA_CB_ALLOC_FAILED, "CONTROL BLOCK ALLOC FAILED"},
	{GLA_CLIENT_ALLOC_FAILED, "GLA CLIENT NODE ALLOC FAILED"},
	{GLA_EVT_ALLOC_FAILED, "GLA EVT ALLOC FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/
const NCSFL_STR gla_api_set[] = {
	{GLA_API_LCK_INITIALIZE_FAIL, "GLA API LOCK  INIT FAILED"},
	{GLA_API_LCK_SELECTION_OBJECT_FAIL, "GLA API LOCK SEL OBJ FAILED"},
	{GLA_API_LCK_OPTIONS_CHECK_FAIL, "GLA API LOCK OPTIONS CHECK FAILED"},
	{GLA_API_LCK_DISPATCH_FAIL, "GLA API LOCK DISPATCH FAILED"},
	{GLA_API_LCK_FINALIZE_FAIL, "GLA API LOCK FINALIZE FAILED"},
	{GLA_API_LCK_RESOURCE_OPEN_SYNC_FAIL, "GLA API LOCK RES OPEN SYNC FAILED"},
	{GLA_API_LCK_RESOURCE_OPEN_ASYNC_FAIL, "GLA API LOCK RES OPEN ASYNC FAILED"},
	{GLA_API_LCK_RESOURCE_LOCK_SYNC_FAIL, "GLA API LOCK RES LOCK SYNC FAILED"},
	{GLA_API_LCK_RESOURCE_LOCK_ASYNC_FAIL, "GLA API LOCK RES LOCK ASYNC FAILED"},
	{GLA_API_LCK_RESOURCE_UNLOCK_SYNC_FAIL, "GLA API LOCK RES UNLOCK SYNC FAILED"},
	{GLA_API_LCK_RESOURCE_UNLOCK_ASYNC_FAIL, "GLA API LOCK RES UNLOCK ASYNC FAILED"},
	{GLA_API_LCK_RESOURCE_CLOSE_FAIL, "GLA API LOCK RES CLOSE FAILED"},
	{GLA_API_LCK_RESOURCE_PURGE_FAIL, "GLA API LOCK RES PURGE FAILED"},
	{GLA_DISPATCH_ALL_CALLBK_FAIL, "GLA DISPATCH CALLBACK ALL FAILED"},
	{GLA_DISPATCH_ALL_RMV_IND, "GLA DISPATCH CLBK REM IND FAILED"},
	{GLA_DISPATCH_BLOCK_CLIENT_DESTROYED, "GLA DISPATCH BLOCK CLIENT DESTROYED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for ncs lock  
 ******************************************************************************/
const NCSFL_STR gla_lock_set[] = {
	{GLA_CB_LOCK_INIT_FAILED, "GLA CB LOCK INIT FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for system call 
 ******************************************************************************/
const NCSFL_STR gla_evt_sys_call_set[] = {
	{GLA_GET_SEL_OBJ_FAIL, "GLA SELECTION GET FAILED"},
	{GLA_SEL_OBJ_RMV_IND_FAIL, "GLA SEL OBJ RMV IND FAIL"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Data send
 ******************************************************************************/
const NCSFL_STR gla_evt_data_send_set[] = {
	{GLA_MDS_SEND_FAILURE, "GLA MDS SEND FAILURE "},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET gla_str_set[] = {
	{GLA_FC_HDLN, 0, (NCSFL_STR *)gla_hdln_set},
	{GLA_FC_MEMFAIL, 0, (NCSFL_STR *)gla_memfail_set},
	{GLA_FC_API, 0, (NCSFL_STR *)gla_api_set},
	{GLA_FC_NCS_LOCK, 0, (NCSFL_STR *)gla_lock_set},
	{GLA_FC_SYS_CALL, 0, (NCSFL_STR *)gla_evt_sys_call_set},
	{GLA_FC_DATA_SEND, 0, (NCSFL_STR *)gla_evt_data_send_set},
	{0, 0, 0}
};

NCSFL_FMAT gla_fmat_set[] = {
	{GLA_LID_HDLN, NCSFL_TYPE_TICL, "%s GLA HEADLINE : %s : %s : %lu \n"},
	{GLA_LID_MEMFAIL, NCSFL_TYPE_TICL, "%s GLA MEMERR: %s : %s : %lu \n"},
	{GLA_LID_API, NCSFL_TYPE_TICL, "%s GLA API: %s : %s : %lu \n"},
	{GLA_LID_NCS_LOCK, NCSFL_TYPE_TICL, "%s GLA NCSLOCK: %s : %s : %lu \n"},
	{GLA_LID_SYS_CALL, NCSFL_TYPE_TICLL, "%s GLA SYSCALL: %s : %s : %lu : handle_id  %ld \n"},
	{GLA_LID_DATA_SEND, NCSFL_TYPE_TICLLL, "%s GLA DATASEND: %s : %s : %lu : node_id %ld : evt %ld \n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC gla_ascii_spec = {
	NCS_SERVICE_ID_GLA,	/* GLA subsystem */
	GLSV_LOG_VERSION,	/* GLA revision ID */
	"GLA",
	(NCSFL_SET *)gla_str_set,	/* GLA const strings referenced by index */
	(NCSFL_FMAT *)gla_fmat_set,	/* GLA string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  PROCEDURE NAME:    gla_reg_strings

  DESCRIPTION:       Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uint32_t gla_reg_strings()
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &gla_ascii_spec;
	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    gla_unreg_strings

  DESCRIPTION:       Function is used for deregistering the canned strings with the DTS.

*****************************************************************************/
uint32_t gla_unreg_strings()
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_GLA;
	arg.info.dereg_ascii_spec.version = GLSV_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

#endif
