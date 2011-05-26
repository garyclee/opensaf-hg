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
  FILE NAME: CPD_LOGSTR.C

  DESCRIPTION: This file contains all the log string related to CPD.

******************************************************************************/

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "cpd_log.h"
#include "cpsv.h"
#include "cpnd_log.h"
#include "cpa_log.h"

#if(NCS_DTS == 1)

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR cpd_hdln_set[] = {
	{CPD_CREATE_SUCCESS, "CPD - Instance Create Success"},
	{CPD_CREATE_FAILED, "CPD - Insatnce Create Failed"},
	{CPD_INIT_SUCCESS, "CPD - Instance Initialization Success"},
	{CPD_INIT_FAILED, "CPD - Insatnce Initialization Failed"},
	{CPD_CB_HDL_CREATE_FAILED, "CPD - Handle Registration Success"},
	{CPD_CREATE_HDL_FAILED, "CPD - Handle Registration Failed"},
	{CPD_CB_HDL_TAKE_FAILED, "CPD - CPD_CB Hdl Take Failed"},
	{CPD_AMF_INIT_SUCCESS, "CPD - AMF Registration Success"},
	{CPD_AMF_INIT_FAILED, "CPD - AMF Registration Failed"},
	{CPD_LM_INIT_SUCCESS, "CPD - Layer Management Initialization Success"},
	{CPD_LM_INIT_FAILED, "CPD - Layer Management Initialization Success"},
	{CPD_MDS_INIT_SUCCESS, "CPD - MDS Registration Success"},
	{CPD_MDS_INIT_FAILED, "CPD - MDS Registration Failed"},
	{CPD_REG_COMP_SUCCESS, "CPD - AMF Component Registration Success"},
	{CPD_REG_COMP_FAILED, "CPD - AMF Component Registration Failed"},
	{CPD_BIND_SUCCESS, "CPD - Bind Success"},
	{CPD_EDU_BIND_SUCCESS, "CPD - EDU Bind Success"},
	{CPD_EDU_UNBIND_SUCCESS, "CPD - EDU Unbind Success"},
	{CPD_UNBIND_SUCCESS, "CPD - UNBIND Success"},
	{CPD_DEREG_COMP_SUCCESS, "CPD - AMF Component Deregistration Success"},
	{CPD_MDS_SHUT_SUCCESS, "CPD - MDS Deregistration Success"},
	{CPD_MDS_SHUT_FAILED, "CPD - MDS Deregistration Failed"},
	{CPD_LM_SHUT_SUCCESS, "CPD - Layer Management Destroy Success"},
	{CPD_AMF_SHUT_SUCCESS, "CPD - AMF Deregistration Success"},
	{CPD_AMF_DESTROY_FAILED, "CPD - AMF Destroy Failed"},
	{CPD_DESTROY_SUCCESS, "CPD - Instance Destroy Success"},
	{CPD_COMP_NOT_INSERVICE, "CPD - Component Is Not In Service"},
	{CPD_DONOT_EXIST, "CPD - Instance Doesn't Exist"},
	{CPD_MEMORY_ALLOC_FAIL, "CPD - Failed To Allocate Memeory"},
	{CPD_CKPT_INFO_NODE_ADD_FAILED, "CPD - Ckpt Node Add Failed"},
	{CPD_CKPT_INFO_NODE_GET_FAILED, "CPD - Ckpt Node get Failed"},
	{CPD_CKPT_MAP_INFO_ADD_FAILED, "CPD - Ckpt Map Info Add Failed"},
	{CPD_CPND_INFO_NODE_FAILED, "CPD - Cpnd Info Node Add Failed"},
	{CPD_CPND_INFO_NODE_GET_FAILED, "CPD - Cpd Cpnd Info Node Failed"},
	{CPD_IPC_CREATE_FAIL, "CPD - IPC Create Failed"},
	{CPD_IPC_ATTACH_FAIL, "CPD - IPC Attach Failed"},
	{CPD_TASK_CREATE_FAIL, "CPD - Task Create Failed"},
	{CPD_TASK_START_FAIL, "CPD - Task Start Failed"},
	{CPD_HEALTHCHECK_START_FAIL, "CPD - Healthcheck start in cpd_lib_init Failed"},
	{CPD_MDS_REGISTER_FAILED, "CPD - MDS Register Failed"},
	{CPD_AMF_REGISTER_FAILED, "CPD - AMF Regiser Failed"},
	{CPD_INIT_FAIL, "CPD - Cpd Initialisation Failed"},
	{CPD_DESTROY_FAIL, "CPD - Cpd Destroy Failed"},
	{CPD_CB_RETRIEVAL_FAILED, "CPD - CB Retrieval Failed"},
	{CPD_AMF_GET_SEL_OBJ_FAILURE, "CPD - AMF Selection Obj Get Failed"},
	{CPD_AMF_DISPATCH_FAILURE, "CPD - AMF Dispatch Failed"},
	{CPD_MBCSV_DISPATCH_FAILURE, "CPD - MBCSv Dispatch Failed"},
	{CPD_MDS_GET_HDL_FAILED, "CPD - MDS Get Hdl Failed"},
	{CPD_MDS_INSTALL_FAILED, "CPD - MDS Install Failed"},
	{CPD_MDS_UNREG_FAILED, "CPD - MDS Unregister Failed"},
	{CPD_MDS_SEND_FAIL, "CPD - MDS Send Failed"},
	{CPD_PROCESS_WRONG_EVENT, "CPD - Event at Standby Does not exist"},
	{CPD_A2S_CKPT_CREATE_FAILED, " CPD - A2S Ckpt Create Failed"},
	{CPD_CLM_REGISTER_FAIL, " CPD - CLM REGISTER FAILED"},
	{CPD_MAP_INFO_GET_FAILED, "CPD - MAP INFO GET FAILED"},
	{CPD_CPND_NODE_DOES_NOT_EXIST, "CPD - CPND node does not exist"},
	{CPD_MDS_DEC_FAILED, "CPD - MDS DECODE FAILED"},
	{CPD_MDS_ENC_FLAT_FAILED, "CPD - MDS ENCODE FLAT FAILED"},
	{CPD_MDS_DEC_FLAT_FAILED, "CPD - MDS DECODE FLAT FAILED"},
	{CPD_IPC_SEND_FAILED, "CPD - IPC SEND FAILED"},
	{CPD_MDS_VDEST_CREATE_FAILED, "CPD - VDEST CREATE FAILED"},
	{CPD_MAP_NODE_DELETE_FAILED, "CPD - MAP NODE DELETE FROM PAT TREE FAILED"},
	{CPD_CPND_INFO_NODE_DELETE_FAILED, "CPD - CPND INFO NODE DELETE FROM PAT TREE FAILED"},
	{CPD_CKPT_TREE_INIT_FAILED, "CPD - CKPT TREE INIT FAILED"},
	{CPD_MAP_TREE_INIT_FAILED, "CPD - MAP TREE INIT FAILED"},
	{CPD_CPND_INFO_TREE_INIT_FAILED, "CPD - CPND INFO TREE INIT FAILED"},
	{CPD_CKPT_REPLOC_TREE_INIT_FAILED, "CPD - REPLOC TREE INIT FAILED"},
	{CPD_SCALARTBL_REG_FAILED, "CPD - Scalar Table Registration Failed"},
	{CPD_CKPTTBL_REG_FAILED, "CPD - Checkpoint Table Registration Failed"},
	{CPD_REPLOCTBL_REG_FAILED, "CPD - Replica Location Table Registration Failed"},
	{CPD_CKPT_CREATE_SUCCESS, "CPD - Ckpt Create Success:(ckpt_name)/cpnd_mdest/ckpt_id"},
	{CPD_CKPT_CREATE_FAILURE, "CPD - Ckpt Create Failure: ckpt_name/mdest"},
	{CPD_NON_COLOC_CKPT_CREATE_SUCCESS, "CPD - Non col rep create Sent to scxb cpnd :ckpt Name/scxb_mdest"},
	{CPD_NON_COLOC_CKPT_CREATE_FAILURE, "CPD - Non col rep create Failed for scxb:ckpt Name/scxb_mdest"},
	{CPD_NON_COLOC_CKPT_DESTROY_SUCCESS, "CPD - Non col rep Delete Sent to scxb cpnd :ckpt Name/scxb_mdest"},
	{CPD_EVT_UNLINK_SUCCESS, "CPD - Ckpt Unlink Success :ckpt_name"},
	{CPD_PROC_UNLINK_SET_FAILED, "CPD - Unlink Set Failed"},
	{CPD_CKPT_RDSET_SUCCESS, "CPD - CKPT RDSET SUCCESS"},
	{CPD_CKPT_ACTIVE_SET_SUCCESS, "CPD - Ckpt Active Replica Set Success: ckpt_id/New active_mdest"},
	{CPD_CKPT_ACTIVE_CHANGE_SUCCESS, "CPD - Ckpt Active Replica changed :ckpt_id/New active_mdest"},
	{CPD_CLM_GET_SEL_OBJ_FAILURE, "CPD - CLM Selection Obj Get Failed"},
	{CPD_CLM_DISPATCH_FAILURE, "CPD - CLM Dispatch Failed"},
	{CPD_CLM_CLUSTER_TRACK_FAIL, " CPD - CLM CLUSTER TRACK FAILED"},

	{0, 0}
};

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR cpd_db_set[] = {
	{CPD_DB_ADD_SUCCESS, "CPD - Ckpt Create - Database Operation (ADD) Success"},
	{CPD_DB_ADD_FAILED, "CPD - Ckpt Create - Database Operation (ADD) Failed"},
	{CPD_REP_DEL_SUCCESS, "CPD - Ckpt Replica delete Sent to all cpnd's :ckpt_id/mdest deleted"},
	{CPD_REP_DEL_FAILED, "CPD - Ckpt Replica delete failed :ckpt_id/mdest added"},
	{CPD_REP_ADD_SUCCESS, "CPD - Ckpt Replica add sent to cpnd's :ckpt_id/mdest added"},
	{CPD_CKPT_DEL_SUCCESS, "CPD - Checkpoint Deleted form Cluster :ckpt_id/mdest"},
	{CPD_DB_DEL_FAILED, "CPD -  Ckpt Delete -Database Operation (DEL) Failed :ckpt_id"},
	{CPD_DB_UPD_SUCCESS, "CPD - Database Operation (UPD) Success"},
	{CPD_DB_UPD_FAILED, "CPD - Database Operation (UPD) Failed"},
	{0, 0}
};

const NCSFL_STR cpd_mem_set[] = {
	{CPD_CB_ALLOC_FAILED, "CPD - CB alloc Failed"},
	{CPD_EVT_ALLOC_FAILED, "CPD - Evt Alloc Failed"},
	{CPD_CREF_INFO_ALLOC_FAIL, "CPD - CRef Info Alloc Failed"},
	{CPD_CPND_DEST_INFO_ALLOC_FAILED, " CPD - Cpnd_Dest_Info_Alloc_Failed"},
	{NCS_ENC_RESERVE_SPACE_FAILED, "CPD - Encode Reserve Space for Async count Failed"},
	{CPD_MBCSV_MSG_ALLOC_FAILED, "CPD - CPD Mbcsv Msg Alloc Failed"},
	{CPD_CPND_INFO_ALLOC_FAILED, "CPD - CPD CPND INFO ALLOC FAILED"},
	{0, 0}
};

const NCSFL_STR cpd_mbcsv_set[] = {
	{CPD_NCS_MBCSV_SVC_FAILED, " CPD - MBCSV Svc Failed"},
	{CPD_MBCSV_INIT_FAILED, " CPD - MBCSv INIT FAILED "},
	{CPD_MBCSV_OPEN_FAILED, " CPD - MBCSv Open Failed"},
	{CPD_MBCSV_GET_SEL_OBJ_FAILURE, "CPD - MBCSv Selection Object Get Failed"},
	{CPD_MBCSV_CHGROLE_FAILED, " CPD - MBCSv Change Role Failed"},
	{CPD_MBCSV_FINALIZE_FAILED, "CPD - MBCSv Finalize Failed"},
	{CPD_SB_PROC_RETENTION_SET_FAILED, "CPD - Retention Set Event at Standby Failed"},
	{CPD_SB_PROC_ACTIVE_SET_FAILED, " CPD - Active Set Event at Standby Failed"},
	{CPD_STANDBY_MSG_PROCESSING, "CPD - Succesful in receiving Standby Event from mailbox"},
	{CPD_PROCESS_SB_MSG_FAILED, " CPD - Processing an Event at Standby Failed"},
	{CPD_STANDBY_CREATE_EVT, "CPD - Create Event at Standby Success"},
	{CPD_A2S_CKPT_CREATE_ASYNC_SUCCESS, "CPD - Async Update info for CKPT Create Success"},
	{CPD_A2S_CKPT_CREATE_ASYNC_FAILED, "CPD - Async Update info for CKPT Create Failed"},
	{CPD_A2S_CKPT_UNLINK_ASYNC_SUCCESS, "CPD - Async Update info for CKPT Unlink Success"},
	{CPD_A2S_CKPT_UNLINK_ASYNC_FAILED, "CPD - Async Update info for CKPT Unlink Failed"},
	{CPD_A2S_CKPT_RDSET_ASYNC_SUCCESS, "CPD - Async Update info for CKPT RDSET Success"},
	{CPD_A2S_CKPT_RDSET_ASYNC_FAILED, "CPD - Async Update info for CKPT RDSET Failed"},
	{CPD_A2S_CKPT_AREP_SET_ASYNC_SUCCESS, "CPD - Async Update info for CKPT AREP Set Success"},
	{CPD_A2S_CKPT_AREP_SET_ASYNC_FAILED, "CPD - Async Update info for CKPT AREP Set Failed"},
	{CPD_A2S_CKPT_DESTADD_ASYNC_SUCCESS, "CPD - Async Update info for CKPT Dest Add Success"},
	{CPD_A2S_CKPT_DESTADD_ASYNC_FAILED, "CPD - Async Update info for CKPT Dest Add Failed"},
	{CPD_A2S_CKPT_DESTDEL_ASYNC_SUCCESS, "CPD - Async Update info for CKPT Dest Del Success"},
	{CPD_A2S_CKPT_DESTDEL_ASYNC_FAILED, "CPD - Async Update info for CKPT Dest Del Failed"},
	{EDU_EXEC_ASYNC_CREATE_FAILED, "CPD - EDU Exec for Async Ckpt Create Failed"},
	{EDU_EXEC_ASYNC_UNLINK_FAILED, "CPD - EDU Exec for Async Ckpt Unlink Failed"},
	{EDU_EXEC_ASYNC_RDSET_FAILED, "CPD - EDU Exec for Async Ckpt RdSet Failed"},
	{EDU_EXEC_ASYNC_AREP_FAILED, "CPD - EDU Exec for Async Ckpt ARep Set Failed"},
	{EDU_EXEC_ASYNC_DESTADD_FAILED, "CPD - EDU Exec for Async Ckpt Dest Add Failed"},
	{EDU_EXEC_ASYNC_DESTDEL_FAILED, "CPD - EDU Exec for Async Ckpt Dest Del Failed"},
	{EDU_EXEC_ASYNC_USRINFO_FAILED, "CPD - EDU Exec for Async User Info Failed"},
	{CPD_STANDBY_CREATE_EVT_FAILED, "CPD - CKPT_CREATE at Standby Failed"},
	{CPD_STANDBY_UNLINK_EVT_FAILED, "CPD - CKPT_UNLINK at Standby Failed"},
	{CPD_STANDBY_RDSET_EVT_FAILED, "CPD - CKPT_RDSET at Standby Failed"},
	{CPD_STANDBY_AREP_EVT_FAILED, "CPD - CKPT_AREP_SET at Standby Failed"},
	{CPD_STANDBY_DESTADD_EVT_FAILED, "CPD - CKPT_DEST_ADD at Standby Failed"},
	{CPD_STANDBY_DESTDEL_EVT_FAILED, "CPD - CKPT_DEST_DEL at Standby Failed"},
	{CPD_MBCSV_CLOSE_FAILED, "CPD - MBCSv Close Failed"},
	{CPD_MBCSV_CALLBACK_FAILED, "CPD - MBCSv Callback Failed"},
	{CPD_MBCSV_CALLBACK_SUCCESS, "CPD - MBCSv Callback Success"},
	{CPD_A2S_CKPT_USR_INFO_FAILED, "CPD - MBCSv CKPT_USR_INFO from Active Failed"},
	{CPD_A2S_CKPT_USR_INFO_SUCCESS, "CPD - MBCSv CKPT_USR_INFO from Active Success"},
	{CPD_MBCSV_ASYNC_UPDATE_SEND_FAILED, "CPD - MBCSv Async Update Send Failed"},
	{CPD_MBCSV_WARM_SYNC_COUNT_MISMATCH, "CPD - There is an async update count mismatch in warm sync"},
	{CPD_STANDBY_DESTADD_EVT_SUCCESS, "CPD - CKPT_DEST_ADD event at Standby Success"},
	{CPD_STANDBY_DESTDEL_EVT_INVOKED, "CPD - CKPT_DEST_DEL event at Standby got invoked"},
	{0, 0}
};

const NCSFL_STR cpd_generic_set[] = {
	{CPD_HEALTH_CHK_CB_SUCCESS, "CPD - CPD Health Check Callback Success"},
	{CPD_VDEST_CHG_ROLE_FAILED, "CPD - CPD VDEST Change Role Failed"},
	{CPD_CSI_SET_CB_SUCCESS, "CPD - CPD CSI Set Callback Success :CPD State:"},
	{CPD_AMF_COMP_NAME_GET_FAILED, "CPD - AMF Component Name Get Failed"},
	{CPD_AMF_COMP_REG_SUCCESS, "CPD - AMF Component Registration Success"},
	{CPD_AMF_COMP_REG_FAILED, "CDP - AMF Component Registration Failed"},
	{CPD_AMF_COMP_TERM_CB_INVOKED, "CPD - AMF Component Termination Callback Invoked"},
	{CPD_AMF_CSI_RMV_CB_INVOKED, "CPD - AMF CSI Remove Callback Invoked"},
	{CPD_MDS_REG_SUCCESS, "CDP - MDS Registration Success"},
	{CPD_MDS_SUBSCRIBE_FAILED, "CPD - MDS Subscription Failed"},
	{0, 0}
};

/*****************************************************************************\
 Build up the canned constant strings for the ASCII SPEC
\******************************************************************************/
NCSFL_SET cpd_str_set[] = {
	{CPD_FC_HDLN, 0, (NCSFL_STR *)cpd_hdln_set},
	{CPD_FC_DB, 0, (NCSFL_STR *)cpd_db_set},
	{CPD_FC_MEMFAIL, 0, (NCSFL_STR *)cpd_mem_set},
	{CPD_FC_MBCSV, 0, (NCSFL_STR *)cpd_mbcsv_set},
	{CPD_FC_GENERIC, 0, (NCSFL_STR *)cpd_generic_set},
	{0, 0, 0}
};

NCSFL_FMAT cpd_fmat_set[] = {
	{CPD_LID_HEADLINE, NCSFL_TYPE_TI, "CPSv %s : %s\n"},
	{CPD_LID_DB, NCSFL_TYPE_TIC, "CPSv %s : %s : %s\n"},
	{CPD_LID_MEMFAIL, NCSFL_TYPE_TIC, "CPSv %s : %s : %s\n"},
	{CPD_LID_MBCSV, NCSFL_TYPE_TIC, "CPSv %s : %s : %s\n"},
	{CPD_LID_TIFCL, NCSFL_TYPE_TIFCL, "CPSv %s : %s : %s : %s:%lu\n"},
	{CPD_LID_TICL, NCSFL_TYPE_TICL, "CPSv %s : %s : %s:%lu\n"},
	{CPD_LID_TILCL, NCSFL_TYPE_TILCL, "CPSv %s : %s : %lu : %s:%lu\n"},
	{CPD_LID_TICFCL, NCSFL_TYPE_TICFCL, "CPSv %s : %s : %s : %s : %s:%lu\n"},
	{CPD_LID_TILLCL, NCSFL_TYPE_TILLCL, "CPSv %s : %s : %lu : %lu : %s:%lu\n"},
	{CPD_LID_TICCL, NCSFL_TYPE_TICCL, "CPSv %s : %s : %s : %s:%lu\n"},
	{CPD_LID_TIFFCL, NCSFL_TYPE_TIFFCL, "CPSv %s : %s : %s : %s : %s:%lu\n"},
	{CPD_LID_TICFFCL, NCSFL_TYPE_TICFFCL, "CPSv %s :%s : %s : %s : %s : %s:%lu\n"},
	{CPD_LID_GENLOG, NCSFL_TYPE_TC, "CPSv %s %s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC cpd_ascii_spec = {
	NCS_SERVICE_ID_CPD,	/* CPD subsystem */
	CPSV_LOG_VERSION,	/* CPD (CPSv-CPD) log version ID */
	"cpd",			/* CPD opening Banner in log */
	(NCSFL_SET *)cpd_str_set,	/* CPD const strings referenced by index */
	(NCSFL_FMAT *)cpd_fmat_set,	/* CPD string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/****************************************************************************\
  PROCEDURE NAME : cpd_log_ascii_reg
 
  DESCRIPTION    : This is the function which registers the CPD's logging
                   ascii set with the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uint32_t cpd_log_ascii_reg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;
	uint32_t rc = NCSCC_RC_SUCCESS;

	memset(&arg, 0, sizeof(arg));

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &cpd_ascii_spec;

	/* Regiser CPD Canned strings with DTSv */
	rc = ncs_dtsv_ascii_spec_api(&arg);
	return rc;
}

/****************************************************************************\
  PROCEDURE NAME : cpd_log_ascii_dereg
 
  DESCRIPTION    : This is the function which deregisters the CPD's logging
                   ascii set from the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
void cpd_log_ascii_dereg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_CPD;
	arg.info.dereg_ascii_spec.version = CPSV_LOG_VERSION;

	/* Deregister CPD Canned strings from DTSv */
	ncs_dtsv_ascii_spec_api(&arg);
	return;
}

/****************************************************************************
 * Name          : cpsv_log_str_lib_req
 *
 * Description   : Library entry point for cpsv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t cpsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_SUCCESS;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = cpa_log_ascii_reg();
		res = cpnd_log_ascii_reg();
		res = cpd_log_ascii_reg();
		break;

	case NCS_LIB_REQ_DESTROY:
		cpa_log_ascii_dereg();
		cpnd_log_ascii_dereg();
		cpd_log_ascii_dereg();
		break;

	default:
		break;
	}
	return (res);
}

#endif   /* (NCS_DTS == 1) */
