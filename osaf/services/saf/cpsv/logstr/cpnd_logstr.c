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
  FILE NAME: CPND_LOGSTR.C

  DESCRIPTION: This file contains all the log string related to CPND.

******************************************************************************/

#if(NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "cpnd_log.h"
#include "cpsv.h"

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/

const NCSFL_STR cpnd_hdln_set[] = {

	{CPND_CB_INIT_FAILED, "CPND - Instance Create Failed"},
	{CPND_IPC_CREATE_FAIL, "CPND - IPC Create Failed"},
	{CPND_IPC_ATTACH_FAIL, "CPND - IPC Attach Failed"},
	{CPND_TASK_CREATE_FAIL, "CPND - Task Creation Failed"},
	{CPND_TASK_START_FAIL, "CPND - Task Start Failed"},
	{CPND_CPD_SERVICE_WENT_DOWN, "CPND - Director Service Went Down"},
	{CPND_CPD_SERVICE_IS_DOWN, "CPND - Director Service Is Down"},
	{CPND_CPD_SERVICE_CAME_UP, "CPND - Director Service Is UP"},
	{CPND_CPD_SERVICE_NOACTIVE, "CPND - Director Service in NOACTIVE state"},
	{CPND_CPD_SERVICE_NEWACTIVE, "CPND - Director Service Is NEWACTIVE state"},
	{CPND_MDS_REGISTER_FAILED, "CPND - MDS Registeration Failed"},
	{CPND_MDS_GET_HDL_FAILED, "CPND - MDS Handle Get Failed"},
	{CPND_MDS_INSTALL_FAILED, "CPND - MDS Install Failed "},
	{CPND_MDS_SUBSCRIBE_FAILED, "CPND - MDS Subscription Failed"},
	{CPA_MDS_UNREG_FAILED, "CPND - MDS Unregister Failed"},
	{CPND_MDS_SEND_FAIL, "CPND - MDS Send Failed"},
	{CPND_MDS_SEND_TRYAGAIN, "CPND - MDS TRY AGAIN Being sent to the CPND(to avoid Deadlock)"},
	{CPND_CB_HDL_CREATE_FAILED, "CPND - Handle Creation Failed"},
	{CPND_CB_HDL_TAKE_FAILED, "CPND - Handle Take Failed"},
	{CPND_CB_HDL_GIVE_FAILED, "CPND - Give Handle Failed"},
	{CPND_CB_HDL_DESTROY_FAILED, "CPND - Handle Destroy Failed"},
	{CPND_INIT_SUCCESS, "CPND - Initialisation Succeeded"},
	{CPND_INIT_FAIL, "CPND - Initialisation Failed"},
	{CPND_DESTROY_SUCCESS, "CPND - Destroy Succeeded"},
	{CPND_DESTROY_FAIL, "CPND - Destroy Failed"},
	{CPND_AMF_REGISTER_FAILED, "CPND - Amf Register Failed"},
	{CPND_AMF_DEREGISTER_FAILED, "CPND - Amf Deregister Failed"},
	{CPND_AMF_INIT_FAILED, "CPND - Amf Initialisation Failed"},
	{CPND_AMF_DESTROY_FAILED, "CPND - Amf Destroy Failed"},
	{CPND_AMF_RESPONSE_FAILED, "CPND - Amf Response Failed"},
	{CPND_AMF_GET_SEL_OBJ_FAILURE, "CPND - Amf Selection Object Failed"},
	{CPND_AMF_DISPATCH_FAILURE, "CPND - Amf Dispatch Failed"},
	{CPND_CB_RETRIEVAL_FAILED, "CPND - CB retrieval from hdl Failed"},

	/* TBD */
	{CPND_CKPT_SECT_CREATE_FAILED, "CPND - Ckpt Section Create Failed"},
	{CPND_CKPT_SECT_WRITE_FAILED, "CPND - Ckpt Write Failed , ckpt_id"},
	{CPND_CKPT_SECT_READ_FAILED, "CPND - Ckpt Read Failed , ckpt_id"},
	{CPND_CKPT_HDR_UPDATE_FAILED, "CPND - Ckpt Header Update Failed"},
	{CPND_CLIENT_HDL_GET_FAILED, "CPND - Client went down so no response"},
	{CPND_MDS_ACTIVE_TO_REMOTE_SEND_FAILED, "CPND - MDS Send from Active to Remote Failed"},
	{CPND_AMF_COMP_NAME_GET_FAILED, "CPND - AMF Component Name Get Failed"},
	{CPND_AMF_COMP_REG_FAILED, "CPND - AMF Component Registration Failed"},
	{CPND_CLM_INIT_FAILED, "CPND - CLM Initialization Failed"},
	{CPND_CLM_NODE_GET_FAILED, "CPND - CLM Node Get Failed"},
	{CPND_AMF_HLTH_CHK_START_FAILED, "CPND - AMF Health Check Start Failed"},
	{CPND_SECT_HDR_UPDATE_FAILED, "CPND - Section Header Update Failed"},
	{CPND_ACTIVE_REP_SET_SUCCESS, "CPND - Active Replica been Set for ckpt_id/active_mdest"},
	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for Mem Fails
\*****************************************************************************/
const NCSFL_STR cpnd_memfail_set[] = {

	{CPND_CB_ALLOC_FAILED, "CPND - Control Block Alloc Failed"},
	{CPND_EVT_ALLOC_FAILED, "CPND - Cpsv Evt Alloc Failed"},
	{CPND_CKPT_ALLOC_FAILED, "CPND - Cpnd Ckpt Alloc Failed"},
	{CPND_SECT_ALLOC_FAILED, "CPND - Cpnd Section Alloc Failed"},
	{CPND_CKPT_LIST_ALLOC_FAILED, "CPND -Cpnd Ckpt List Alloc Failed"},
	{CPND_CKPT_CLIST_ALLOC_FAILED, "CPND - Cpnd Ckpt Client List Alloc Failed"},
	{CPND_DEFAULT_ALLOC_FAILED, "CPND - Cpnd Buffer Alloc Failed"},
	{CPND_DEST_INFO_ALLOC_FAILED, "CPND - Cpnd Mds Dest Info alloc Failed"},
	{CPND_CKPT_DATA_ALLOC_FAILED, "CPND - Cpnd Ckpt Data Alloc Failed"},
	{CPND_CLIENT_ALLOC_FAILED, "CPND - Client Alloc Failed"},
	{CPND_CKPT_SECTION_INFO_FAILED, "CPND - Ckpt Section Info Failed"},
	{CPND_SYNC_SEND_NODE_ALLOC_FAILED, "CPND - Sync Send Node Alloc Failed"},
	{CPND_CPD_DEFERRED_REQ_NODE_ALLOC_FAILED, "CPND - CPD Deferred Req Node Alloc Failed"},
	{CPND_CPSV_ND2A_READ_MAP_ALLOC_FAIL, "CPND - CPSv ND2A Read Map Alloc Failed"},
	{CPND_CPSV_ND2A_READ_DATA_ALLOC_FAIL, "CPND - CPSv ND2A Read Data Alloc Failed"},
	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for Evt Fails
\*****************************************************************************/
const NCSFL_STR cpnd_evtfail_set[] = {
	{CPND_EVT_UNKNOWN, "CPND - Unknown Event"},
	{CPND_EVT_PROCESS_FAILURE, "CPND - Event Process Failed"},
	{CPND_EVT_RECIEVED, "CPND -Evtn Recieved"},
	{CPND_CPD_NEW_ACTIVE_DESTROY_FAILED, "CPND - Destroy Event Failed when CPD is NEW ACTIVE"},
	{CPND_CPD_NEW_ACTIVE_DESTROY_BYNAME_FAILED, "CPND - Destroy By Name Event Failed when CPD is NEW ACTIVE"},
	{CPND_CPD_NEW_ACTIVE_UNLINK_FAILED, "CPND - Unlink Event failed when CPD is NEW ACTIVE"},
	{CPND_CPD_NEW_ACTIVE_RDSET_FAILED, "CPND - Rdset Event failed when CPD is NEW ACTIVE"},
	{CPND_CPD_NEW_ACTIVE_AREP_SET_FAILED, "CPND - Active Replica Set failed when CPD is NEW ACTIVE"},
	{0, 0}
};

/*****************************************************************************\
                       Logging stuff for Lock Fails
\*****************************************************************************/
const NCSFL_STR cpnd_lockfail_set[] = {
	/* Sashi: Use this only if Required */
	/* TBD */
	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for Evt Fails
\*****************************************************************************/
const NCSFL_STR cpnd_syscallfail_set[] = {
	{CPND_CKPT_OPEN_FAILED, "CPND - Shared Memory Open Failed"},
	{CPND_CKPT_UNLINK_FAILED, "CPND - Shared Memory Unlink Failed"},
	{CPND_CKPT_CLOSE_FAILED, "CPND - Shared Memory Close Failed"},

	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for CPND Restart  
\*****************************************************************************/

const NCSFL_STR cpnd_restart_set[] = {
	{CPND_COMING_UP_FIRST_TIME, "CPND - Coming Up For the First Time"},
	{CPND_NEW_SHM_CREATE_SUCCESS, "CPND - New Shared memory for Restart Created Success"},
	{CPND_RESTARTED, " CPND - Has Restarted"},
	{CPND_CLIENT_INFO_READ_SUCCESS, "CPND - Client Info Read Success"},
	{CPND_OPEN_REQ_FAILED, "CPND - New Shared Memory Creation Failed"},
	{CPND_NUM_CLIENTS_READ, "CPND - Number of Clients from Shared Memory Read Successfully"},
	{CPND_CLIENT_HDL_EXTRACTED, "CPND - Client Handle bit extracted successfully"},
	{CPND_MAX_CLIENTS_REACHED, "CPND - Max Clients Reached , no more entry "},
	{CPND_CLIENT_FREE_BLOCK_SUCCESS, "CPND - Found free block for Client"},
	{CPND_CLIENT_FREE_BLOCK_FAILED, "CPND - Free block for Client Failed"},
	{CPND_MAX_CKPTS_REACHED, "CPND - Max Ckpts Reached , no more entry"},
	{CPND_CKPT_FREE_BLOCK_SUCCESS, "CPND - Found free block for Checkpoint"},
	{CPND_CKPT_WRITE_HEADER_SUCCESS, "CPND - Update the ckpt header with number of ckpts"},
	{CPND_CLI_INFO_WRITE_HEADER_SUCCESS, "CPND - Update the client info with number of clients"},

	{CPND_CLIENT_INF0_UPDATE_FAILED, "CPND - Client update failed , cli_hdl"},
	{CPND_CLIENT_INF0_UPDATE_SUCCESS, "CPND - Client Init success , cli_hdl"},
	{CPND_CKPT_INF0_READ_FAILED, "CPND - Ckpt information Read Failed "},
	{CPND_CKPT_INF0_WRITE_FAILED, "CPND - Ckpt Information Write Failed "},
	{CPND_CKPT_INF0_WRITE_SUCCESS, "CPND - Ckpt Information Write Success "},
	{CPND_CLIENT_BITMAP_RESET_SUCCESS, "CPND - Client Bitmap reset Success "},
	{CPND_CKPT_REPLICA_CREATE_FAILED, "CPND - Reading from Existing Shared Memory Failed"},
	{CPND_SECT_HDR_READ_FAILED, "CPND - Reading from Section Header Failed"},
	{CPND_CLIHDR_INFO_READ_FAILED, "CPND - Client Header Read Failed"},
	{CPND_CLIENT_INF0_READ_FAILED, " CPND - Client info Read Failed"},
	{CPND_RESTART_SHM_CKPT_UPDATE_FAILED, "CPND - Updating the shared memory Failed"},
	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for Evt Fails
\*****************************************************************************/

const NCSFL_STR cpnd_api_set[] = {

	{CPND_CLIENT_TREE_ADD_FAILED, "CPND - Client Tree Add Failed , cli_hdl"},
	{CPND_CLIENT_TREE_DEL_FAILED, "CPND - Client Tree Del Failed , cli_hdl"},
	{CPND_CLIENT_TREE_GET_FAILED, "CPND - Client Tree Get Failed , cli_hdl"},

	{CPND_CKPT_NODE_ADD_FAILED, "CPND - Ckpt Node Add Failed , ckpt_id"},
	{CPND_CKPT_NODE_FIND_FAILED, "CPND - Ckpt Node Find Failed , ckpt_id"},
	{CPND_CKPT_NODE_DEL_FAILED, "CPND - Ckpt Node Del Failed , ckpt_id"},
	{CPND_CKPT_NODE_GET_FAILED, "CPND - Ckpt Node Get Failed  , ckpt_id"},
	{CPND_CKPT_NODE_FIND_BY_NAME_FAILED, "CPND - Ckpt Node Find by name Failed"},
	{CPND_CLIENT_NODE_GET_FAILED, " CPND - Client Node Get Failed for cli_hdl"},

	{CPND_CKPT_SECT_ADD_FAILED, "CPND - Ckpt Sect Add Failed , ckpt_id"},
	{CPND_CKPT_SECT_DEL_FAILED, "CPND - Ckpt Sect Del Failed , ckpt_id"},
	{CPND_CKPT_SECT_GET_FAILED, "CPND - Ckpt Sect Get Failed , ckpt_id"},
	{CPND_CKPT_SECT_FIND_FAILED, "CPND - Ckpt Sect Find Failed , ckpt_id"},
	{CPND_RMT_CPND_FIND_FAILED, "CPND - Remote Cpnd Find Failed"},
	{CPND_REP_ADD_SUCCESS, "CPND - Remote Cpnd Added to destlist ckpt_d/added_dest"},
	{CPND_REP_DEL_SUCCESS, "CPND - Remote Cpnd Deleted from Destlist ckpt_id/del_dest"},
	{CPND_PROC_SEC_EXPIRY_FAILED, "CPND - Section Expiry processing Failed "},
	{CPND_PROC_RT_EXPIRY_FAILED, "CPND - Retention Timer Expiry Processing Failed "},
	{CPND_CB_DB_INIT_FAILED, "CPND - CB DB Init Failed"},
	{CPND_CKPT_NODE_TREE_INIT_FAILED, "CPND - Cpnd ckpt node tree init failed "},
	{CPND_CLIENT_NODE_TREE_INIT_FAILED, "CPND - Cpnd client node tree init failed "},
	{CPND_RMT_CPND_ENTRY_DOES_NOT_EXIST, "CPND - Remote Cpnd entry does not exist in dest list"},
	{CPND_CLIENT_FINALIZE_SUCCESS, "CPND - Client Finalize event Success , cli_hdl"},
	{CPND_CKPT_OPEN_SUCCESS,
	 "CPND - Checkpoint Open Success , (ckpt_name)/cli_hdl/ckpt_id/(active_mdest):lcl_ref_cnt"},
	{CPND_CKPT_OPEN_FAILURE, "CPND - Checkpoint Open Failed , cli_hdl/ckpt_id/ckpt_name:lcl_ref_cnt"},
	{CPND_CLIENT_CKPT_CLOSE_SUCCESS, "CPND - Checkpoint Close Success , cli_hdl/ckpt_id/lcl_ref_cnt"},
	{CPND_PROC_CKPT_UNLINK_SUCCESS, "CPND - Ckpt Unlink event Success ,ckpt_name "},
	{CPND_OPEN_ACTIVE_SYNC_EXPIRY_FAILED, "CPND - Open active Sync receive Failed "},
	{0, 0}

};

const NCSFL_STR cpnd_generic_set[] = {

	{CPND_CKPT_REPLICA_DESTROY_SUCCESS, "CPND - Ckpt Replica Destroy Success ,ckpt_id"},
	{CPND_CKPT_REPLICA_DESTROY_FAILED, "CPND - Ckpt Replica Destroy Failed"},
	{CPND_CKPT_REPLICA_CLOSE_FAILED, "CPND - Ckpt Replica Close Failed"},
	{CPND_CKPT_REP_CREATE_FAILED, "CPND - Ckpt Replica Create Failed"},
	{CPND_REPLICA_HAS_NO_SECTIONS, "CPND - Ckpt Replica has no sections "},
	{CPND_SECTION_BOUNDARY_VIOLATION, "CPND - section boundary violated "},
	{CPND_MDS_DECODE_FAILED, "CPND - MDS Decode Failed"},
	{CPND_MDS_ENC_FLAT_FAILED, "CPND - MDS Encode Flat Failed"},
	{CPND_NCS_ENQUEUE_EVT_FAILED, "CPND - NCS Enqueue of Event Failed"},
	{CPND_NCS_IPC_SEND_FAILED, "CPND - NCS IPC Send Failed"},
	{CPND_CREATING_MORE_THAN_MAX_SECTIONS, "CPND - Trying to create more than max sections"},
	{CPND_MDS_DEC_FLAT_FAILED, "CPND - MDS Decode Flat Failed"},
	{CPND_AMF_HLTH_CB_SUCCESS, "CPND - AMF Health Check Callback Success"},
	{CPND_AMF_REGISTER_SUCCESS, "CPND - AMF Register Success"},
	{CPND_AMF_COMP_UNREG_FAILED, "CPND - AMF Component Unregistered Failed"},
	{CPND_AMF_TERM_CB_INVOKED, "CPND - AMF Terminate Callback Invoked"},
	{CPND_CSI_RMV_CB_INVOKED, "CPND - AMF CSI Remove Callback Invoked"},
	{CPND_CSI_CB_INVOKED, "CPND - CSI Set Callback Invoked"},
	{CPND_MDS_SUBSCRIBE_CPD_FAILED, "CPND - MDS Subscribe with CPD failed"},
	{CPND_MDS_SUBSCRIBE_CPA_FAILED, "CPND - MDS Subscribe with CPA failed"},
	{0, 0}

};

const NCSFL_STR cpnd_mdsfail_set[] = {
	{CPND_ACTIVE_TO_REMOTE_MDS_SEND_FAIL, "CPND - MDS send failed from Active Dest to Remote Dest"},
	{CPND_REMOTE_TO_ACTIVE_MDS_SEND_FAIL, "CPND - MDS send failed from Remote Dest to Active Dest"},
	{CPND_SYNC_SEND_TO_CPD_FAILED, "CPND - MDS sync send to CPD in New Active State failed so enqueue the event "},
	{0, 0}

};

const NCSFL_STR cpnd_ckptinfo_set[] = {

	{CPND_CKPT_NODE_ADDITION_FAILED, "CPND - Checkpoint Node Addition Failed , ckpt_id"},
	{CPND_CHECKPOINT_NODE_GET_FAILED, "CPND - Checkpoint Node Get Failed"},
	{CPND_CKPT_RET_TMR_SUCCESS, "CPND - Checkpoint retention tmr started , ckpt_id"},
	{CPND_PROC_CKPT_UNLINK_SET, "CPND - Checkpoint Unlink flag set , ckpt_id"},
	{CPND_CKPT_REPLICA_CREATE_SUCCESS, "CPND - Checkpoint Replica Create Success ,ckpt_id"},
	{CPND_NON_COLLOC_CKPT_REPLICA_CREATE_SUCCESS, "CPND - Non collocated rep create Success ,ckpt_id"},
	{CPND_CKPT_CLIENT_DEL_SUCCESS, "CPND - Client Del Success/CPA Down ,cli_hdl"},
	{CPND_OPEN_ACTIVE_SYNC_START_TMR_SUCCESS, "CPND - Checkpoint open active sync tmr started , ckpt_id"},
	{CPND_OPEN_ACTIVE_SYNC_STOP_TMR_SUCCESS, "CPND - Checkpoint open active sync tmr stoped , ckpt_id"},
	{0, 0}
};

/*****************************************************************************\
 Build up the canned constant strings for the ASCII SPEC
\******************************************************************************/
NCSFL_SET cpnd_str_set[] = {
	{CPND_FC_HDLN, 0, (NCSFL_STR *)cpnd_hdln_set},
	{CPND_FC_MEMFAIL, 0, (NCSFL_STR *)cpnd_memfail_set},
	{CPND_FC_EVT, 0, (NCSFL_STR *)cpnd_evtfail_set},
	{CPND_FC_NCS_LOCK, 0, (NCSFL_STR *)cpnd_lockfail_set},
	{CPND_FC_SYS_CALL, 0, (NCSFL_STR *)cpnd_syscallfail_set},
	{CPND_FC_RESTART, 0, (NCSFL_STR *)cpnd_restart_set},
	{CPND_FC_API, 0, (NCSFL_STR *)cpnd_api_set},
	{CPND_FC_GENERIC, 0, (NCSFL_STR *)cpnd_generic_set},
	{CPND_FC_MDSFAIL, 0, (NCSFL_STR *)cpnd_mdsfail_set},
	{CPND_FC_CKPTINFO, 0, (NCSFL_STR *)cpnd_ckptinfo_set},
	{0, 0, 0}
};

NCSFL_FMAT cpnd_fmat_set[] = {
	{CPND_LID_TILCL, NCSFL_TYPE_TILCL, "CPSv %s : %s : %lu : %s:%lu\n"},
	{CPND_LID_TICL, NCSFL_TYPE_TICL, "CPSv %s : %s : %s : %lu\n"},
	{CPND_LID_TIFCL, NCSFL_TYPE_TIFCL, "CPSv %s : %s : %s : %s:%lu\n"},
	{CPND_LID_TICCL, NCSFL_TYPE_TICCL, "CPSv %s : %s : %s : %s:%lu\n"},
	{CPND_LID_TIFFCL, NCSFL_TYPE_TIFFCL, "CPSv %s : %s : %s : %s : %s:%lu\n"},
	{CPND_LID_TIFFLCL, NCSFL_TYPE_TIFFLCL, "CPSv %s : %s : %s : %s : %lu : %s:%lu\n"},
	{CPND_LID_TIFFFLCL, NCSFL_TYPE_TIFFFLCL, "CPSv %s : %s : %s : %s : %s : %lu : %s:%lu\n"},
	{CPND_LID_TICFCL, NCSFL_TYPE_TICFCL, "CPSv %s : %s : %s : %s : %s:%lu\n"},
	{CPND_LID_TIFLCL, NCSFL_TYPE_TIFLCL, "CPSv %s : %s : %s : %lu : %s:%lu\n"},
	{CPND_LID_TICFFFCL, NCSFL_TYPE_TICFFFCL, "CPSv %s : %s : %s : %s: %s : %s : %s :%lu\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC cpnd_ascii_spec = {
	NCS_SERVICE_ID_CPND,	/* CPND subsystem */
	CPSV_LOG_VERSION,	/* CPND (CPSv-CPND) log version ID */
	"cpnd",			/* CPND opening Banner in log */
	(NCSFL_SET *)cpnd_str_set,	/* CPND const strings referenced by index */
	(NCSFL_FMAT *)cpnd_fmat_set,	/* CPND string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/****************************************************************************\
  PROCEDURE NAME : cpnd_log_ascii_reg
 
  DESCRIPTION    : This is the function which registers the CPND's logging
                   ascii set with the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uint32_t cpnd_log_ascii_reg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;
	uint32_t rc = NCSCC_RC_SUCCESS;

	memset(&arg, 0, sizeof(arg));

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &cpnd_ascii_spec;

	/* Regiser CPND Canned strings with DTSv */
	rc = ncs_dtsv_ascii_spec_api(&arg);
	return rc;
}

/****************************************************************************\
  PROCEDURE NAME : cpnd_log_ascii_dereg
 
  DESCRIPTION    : This is the function which deregisters the CPND's logging
                   ascii set from the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
void cpnd_log_ascii_dereg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_CPND;
	arg.info.dereg_ascii_spec.version = CPSV_LOG_VERSION;

	/* Deregister CPND Canned strings from DTSv */
	ncs_dtsv_ascii_spec_api(&arg);
	return;
}

#endif   /* (NCS_DTS == 1) */
