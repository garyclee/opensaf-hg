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
#include "glnd_log.h"
#include "glsv_logstr.h"

const NCSFL_STR glnd_hdln_set[] = {
	{GLND_CB_CREATE_FAILED, "GLND CB CREATION FAILED"},
	{GLND_CB_CREATE_SUCCESS, "GLND CB CREATION SUCCESS"},
	{GLND_TASK_CREATE_FAILED, "GLND TASK CREATION FAILED"},
	{GLND_TASK_CREATE_SUCCESS, "GLND TASK CREATION SUCCESS"},
	{GLND_TASK_START_FAILED, "GLND TASK START FAILED"},
	{GLND_TASK_START_SUCCESS, "GLND TASK START SUCCESS"},
	{GLND_CB_DESTROY_FAILED, "GLND CB DESTROY FAILED"},
	{GLND_CB_TAKE_HANDLE_FAILED, "GLND CB TAKE HANDLE FAILED"},
	{GLND_AMF_GET_SEL_OBJ_FAILURE, "GLND AMF GET SEL OBJ ERROR"},
	{GLND_AMF_DISPATCH_FAILURE, "GLND AMF DISPATCH FAILURE"},
	{GLND_CLIENT_TREE_INIT_FAILED, "GLND CLIENT TREE INIT FAILED"},
	{GLND_AGENT_TREE_INIT_FAILED, "GLND AGENT TREE INIT FAILED"},
	{GLND_RSC_TREE_INIT_FAILED, "GLND RSC TREE INIT FAILED"},
	{GLND_IPC_CREATE_FAILED, "GLND IPC CREATE FAILED"},
	{GLND_IPC_ATTACH_FAILED, "GLND IPC ATTACH FAILED"},
	{GLND_MDS_REGISTER_FAILED, "GLND MDS REGISTER FAILED"},
	{GLND_MDS_REGISTER_SUCCESS, "GLND MDS REGISTER SUCCESS"},
	{GLND_AMF_INIT_FAILED, "GLND AMF INIT FAILED"},
	{GLND_AMF_INIT_SUCCESS, "GLND AMF INIT SUCCESS"},
	{GLND_AMF_DESTROY_FAILED, "GLND AMF DESTROY FAILED"},
	{GLND_AMF_REGISTER_FAILED, "GLND AMF REGISTER FAILED"},
	{GLND_AMF_REGISTER_SUCCESS, "GLND AMF REGISTER SUCCESS"},
	{GLND_AMF_RESPONSE_FAILED, "GLND AMF RESPONSE FAILED"},
	{GLND_MDS_GET_HANDLE_FAILED, "GLND MDS GET HANDLE FAILED"},
	{GLND_MDS_UNREGISTER_FAILED, "GLND MDS UNREGISTER FAILED"},
	{GLND_MDS_CALLBACK_PROCESS_FAILED, "GLND MDS CALLBACK PROCESS FAILED"},
	{GLND_AMF_HEALTHCHECK_START_FAILED, "GLND AMF HEALTHCHECK START FAILED"},
	{GLND_AMF_HEALTHCHECK_START_SUCCESS, "GLND AMF HEALTHCHECK START SUCCESS"},
	{GLND_RSC_REQ_CREATE_HANDLE_FAILED, "GLND RSC REQ CREATE HANDLE FAILED"},
	{GLND_RSC_NODE_ADD_SUCCESS, "GLND RSC NODE ADD SUCCESS"},
	{GLND_RSC_NODE_DESTROY_SUCCESS, "GLND RSC NODE DESTROY SUCCESS"},
	{GLND_RSC_LOCK_REQ_DESTROY, "GLND RSC LOCK REQ DESTROY"},
	{GLND_RSC_LOCK_GRANTED, "GLND RSC LOCK GRANTED"},
	{GLND_RSC_LOCK_QUEUED, "GLND RSC LOCK QUEUED"},
	{GLND_RSC_UNLOCK_SUCCESS, "GLND RSC UNLOCK SUCCESS"},
	{GLND_NEW_MASTER_RSC, "GLND NEW MASTER FOR RESOURCE"},
	{GLND_MASTER_ELECTION_RSC, "GLND MASTER ELECTION RSC"},
	{GLND_MSG_FRMT_VER_INVALID, "GLND MSG FORMAT VERSION INVALID"},
	{GLND_DEC_FAIL, "GLND DEC FAILED"},
	{GLND_ENC_FAIL, "GLND ENC FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/
const NCSFL_STR glnd_memfail_set[] = {
	{GLND_CB_ALLOC_FAILED, "CONTROL BLOCK ALLOC FAILED"},
	{GLND_CLIENT_ALLOC_FAILED, "GLND CLIENT NODE ALLOC FAILED"},
	{GLND_RSC_REQ_LIST_ALLOC_FAILED, "GLND RSC REQ LIST ALLOC FAILED"},
	{GLND_AGENT_ALLOC_FAILED, "GLND AGENT ALLOC FAILED"},
	{GLND_CLIENT_RSC_LIST_ALLOC_FAILED, "GLND CLIENT RSC LIST ALLOC FAILED"},
	{GLND_CLIENT_RSC_LOCK_LIST_ALLOC_FAILED, "GLND CLIENT RSC LOCK LIST ALLOC FAILED"},
	{GLND_EVT_ALLOC_FAILED, "GLND EVT ALLOC FAILED"},
	{GLND_RSC_NODE_ALLOC_FAILED, "GLND RSC NODE ALLOC FAILED"},
	{GLND_RESTART_RSC_NODE_ALLOC_FAILED, "GLND RESTART RSC NODE ALLOC FAILED"},
	{GLND_RESTART_RSC_INFO_ALLOC_FAILED, "GLND RESTART RSC INFO ALLOC FAILED"},
	{GLND_RESTART_BACKUP_EVENT_ALLOC_FAILED, "GLND RESTART BACKUP EVENT ALLOC FAILED"},
	{GLND_RSC_LOCK_LIST_ALLOC_FAILED, "GLND RSC LOCK LIST ALLOC FAILED"},
	{GLND_RESTART_RSC_LOCK_LIST_ALLOC_FAILED, "GLND RESTART RSC LOCK LIST ALLOC FAILED"},
	{GLND_LOCK_LIST_ALLOC_FAILED, "GLND LOCK LIST ALLOC FAILED"},
	{GLND_IO_VECTOR_ALLOC_FAILED, "GLND IO VECTOR ALLOC FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for API 
 ******************************************************************************/
const NCSFL_STR glnd_api_set[] = {
	{GLND_AGENT_TREE_ADD_FAILED, "GLND AGENT TREE ADD FAILED"},
	{GLND_AGENT_TREE_DEL_FAILED, "GLND AGENT TREE DEL FAILED"},
	{GLND_AGENT_NODE_FIND_FAILED, "GLND AGENT NODE FIND FAILED"},
	{GLND_CLIENT_TREE_ADD_FAILED, "GLND CLIENT TREE ADD FAILED"},
	{GLND_CLIENT_TREE_DEL_FAILED, "GLND CLIENT TREE DEL FAILED"},
	{GLND_CLIENT_NODE_FIND_FAILED, "GLND CLIENT NODE FIND FAILED"},
	{GLND_RSC_REQ_NODE_ADD_FAILED, "GLND RSC REQ NODE ADD FAILED"},
	{GLND_RSC_NODE_FIND_FAILED, "GLND RSC NODE FIND FAILED"},
	{GLND_RSC_NODE_ADD_FAILED, "GLND RSC NODE ADD FAILED"},
	{GLND_RSC_NODE_DESTROY_FAILED, "GLND RSC NODE DESTROY FAILED"},
	{GLND_RSC_LOCAL_LOCK_REQ_FIND_FAILED, "GLND RSC LOCAL LOCK REQ FIND FAILED"},
	{GLND_RSC_GRANT_LOCK_REQ_FIND_FAILED, "GLND RSC GRANT LOCK REQ FIND FAILED"},
	{GLND_AGENT_NODE_NOT_FOUND, "GLND AGENT NODE NOT FOUND"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Event 
 ******************************************************************************/
const NCSFL_STR glnd_evt_set[] = {
	{GLND_EVT_UNKNOWN, "UNKNOWN GLND EVT RCVD"},
	{GLND_EVT_PROCESS_FAILURE, "GLND EVT PROCESS FAILURE"},
	{GLND_EVT_RECIEVED, "GLND EVT RECIEVED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Data send
 ******************************************************************************/
const NCSFL_STR glnd_data_send_set[] = {
	{GLND_MDS_SEND_FAILURE, "GLND MDS SEND FAILURE "},
	{GLND_MDS_GLD_DOWN, "GLND MDS GLD DOWN"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Timer Events
 ******************************************************************************/
const NCSFL_STR glnd_timer_set[] = {
	{GLND_TIMER_START_FAIL, "GLND TIMER START FAILURE "},
	{GLND_TIMER_STOP_FAIL, "GLND TIMER STOP FAILURE "},
	{0, 0}
};

/******************************************************************************
 Logging stuff for GLND log 
 ******************************************************************************/
const NCSFL_STR glnd_log_set[] = {
	{GLND_COMING_UP_FIRST_TIME, "GLND COMING UP FIRST TIME"},
	{GLND_RESTARTED, "GLND RESTARTED"},
	{GLND_SHM_CREATE_FAILURE, "GLND  SHM CREATE FAILURE "},
	{GLND_SHM_CREATE_SUCCESS, "GLND  SHM CREATE SUCCESS"},
	{GLND_SHM_OPEN_FAILURE, "GLND SHM OPEN FAILURE "},
	{GLND_SHM_OPEN_SUCCESS, "GLND SHM OPEN SUCCESS"},
	{GLND_RESOURCE_SHM_WRITE_FAILURE, "GLND RESOURCE SHM WRITE FAILURE"},
	{GLND_RESOURCE_SHM_READ_FAILURE, "GLND RESOURCE SHM READ  FAILURE"},
	{GLND_LCK_LIST_SHM_WRITE_FAILURE, "GLND LCK_LIST SHM WRITE FAILURE"},
	{GLND_LCK_LIST_SHM_READ_FAILURE, "GLND LCK_LIST SHM READ  FAILURE"},
	{GLND_EVT_LIST_SHM_WRITE_FAILURE, "GLND EVT_LIST SHM WRITE FAILURE"},
	{GLND_EVT_LIST_SHM_READ_FAILURE, "GLND EVT_LIST SHM READ  FAILURE"},
	{GLND_RESTART_BUILD_DATABASE_SUCCESS, "GLND RESTART BUILD DATABASE SUCCESS"},
	{GLND_RESTART_BUILD_DATABASE_FAILURE, "GLND RESTART BUILD DATABASE FAILURE"},
	{GLND_RESTART_RESOURCE_TREE_BUILD_SUCCESS, "GLND RESTART RESOURCE BUILD SUCCESS"},
	{GLND_RESTART_RESOURCE_TREE_BUILD_FAILURE, "GLND RESTART RESOURCE BUILD FAILURE"},
	{GLND_RESTART_LCK_LIST_BUILD_SUCCESS, "GLND RESTART LOCK_LIST BUILD SUCCESS"},
	{GLND_RESTART_LCK_LIST_BUILD_FAILURE, "GLND RESTART LOCK_LIST BUILD FAILURE"},
	{GLND_RESTART_EVT_LIST_BUILD_SUCCESS, "GLND RESTART EVT_LIST BUILD SUCCESS"},
	{GLND_RESTART_EVT_LIST_BUILD_FAILURE, "GLND RESTART EVT_LIST BUILD FAILURE"},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET glnd_str_set[] = {
	{GLND_FC_HDLN, 0, (NCSFL_STR *)glnd_hdln_set},
	{GLND_FC_HDLN_TIL, 0, (NCSFL_STR *)glnd_hdln_set},
	{GLND_FC_HDLN_TILL, 0, (NCSFL_STR *)glnd_hdln_set},
	{GLND_FC_HDLN_TILLL, 0, (NCSFL_STR *)glnd_hdln_set},
	{GLND_FC_MEMFAIL, 0, (NCSFL_STR *)glnd_memfail_set},
	{GLND_FC_API, 0, (NCSFL_STR *)glnd_api_set},
	{GLND_FC_EVT, 0, (NCSFL_STR *)glnd_evt_set},
	{GLND_FC_DATA_SEND, 0, (NCSFL_STR *)glnd_data_send_set},
	{GLND_FC_TIMER, 0, (NCSFL_STR *)glnd_timer_set},
	{GLND_FC_LOG, 0, (NCSFL_STR *)glnd_log_set},
	{0, 0, 0}
};

NCSFL_FMAT glnd_fmat_set[] = {
	{GLND_LID_HDLN, NCSFL_TYPE_TICL, "%s GLND HEADLINE : %s : %s : %lu \n"},
	{GLND_LID_HDLN_TIL, NCSFL_TYPE_TICLL, "%s GLND HEADLINE : %s : %s : %lu : Res id %ld \n"},
	{GLND_LID_HDLN_TILL, NCSFL_TYPE_TICLLL, "%s GLND HEADLINE : %s : %s : %lu : Res id %ld : Node id %ld \n"},
	{GLND_LID_HDLN_TILLL, NCSFL_TYPE_TICLLLL,
	 "%s GLND HEADLINE : %s : %s : %lu : Handle  %ld : Res id %ld : lockid %ld \n"},
	{GLND_LID_MEMFAIL, NCSFL_TYPE_TICL, "%s GLND MEMERR: %s : %s : %lu \n"},
	{GLND_LID_API, NCSFL_TYPE_TICL, "%s GLND API: %s : %s : %lu \n"},
	{GLND_LID_EVT, NCSFL_TYPE_TICLLLLLL,
	 "%s GLND EVT: %s : %s : %lu : event_id %ld : Node %ld : Hdl %ld : Rsc %ld : Lock %ld \n"},
	{GLND_LID_DATA_SEND, NCSFL_TYPE_TICLLL, "%s GLND DATA SEND: %s : %s : %lu : node_id %ld : evt %ld \n"},
	{GLND_LID_TIMER, NCSFL_TYPE_TICLL, "%s GLND TIMER: %s : %s : %lu : type %ld \n"},
	{GLND_LID_LOG, NCSFL_TYPE_TCLILLLL,
	 "%s GLND LOG: %s : %lu : %s : Ret %lu : Handle %ld : Res id %ld : lockid %ld \n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC glnd_ascii_spec = {
	NCS_SERVICE_ID_GLND,	/* GLND subsystem */
	GLSV_LOG_VERSION,	/* GLND revision ID */
	"GLND",
	(NCSFL_SET *)glnd_str_set,	/* GLND const strings referenced by index */
	(NCSFL_FMAT *)glnd_fmat_set,	/* GLND string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  PROCEDURE NAME:    glnd_reg_strings

  DESCRIPTION:       Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uint32_t glnd_reg_strings()
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &glnd_ascii_spec;
	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_unreg_strings

  DESCRIPTION:       Function is used for deregistering the canned strings with the DTS.

*****************************************************************************/
uint32_t glnd_unreg_strings()
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_GLND;
	arg.info.dereg_ascii_spec.version = GLSV_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

#endif
