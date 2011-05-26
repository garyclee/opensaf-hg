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
  FILE NAME: mqnd_logstr.c
                                                                                                                             
  DESCRIPTION: This file contains all the log string related to MQND.
                                                                                                                             
******************************************************************************/

#ifndef MQND_LOGSTR_H
#define MQND_LOGSTR_H
#if(NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "mqnd_log.h"
#include "mqsv.h"

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR mqnd_hdln_set[] = {
	{MQND_CB_INIT_FAILED, "MQND - Controlblock Init Failed"},
	{MQND_CB_DB_INIT_FAILED, "MQND - CB Database Init Failed"},
	{MQND_CB_DB_INIT_SUCCESS, "MQND - CB Database Init Success"},
	{MQND_CB_HDL_CREATE_SUCCESS, "MQND - CB Handle Creation Successfull"},
	{MQND_IPC_CREATE_FAIL, "MQND - IPC Create Failed"},
	{MQND_IPC_CREATE_SUCCESS, "MQND - IPC Create Successfull"},
	{MQND_IPC_ATTACH_FAIL, "MQND - IPC attatch failed"},
	{MQND_IPC_ATTACH_SUCCESS, "MQND - IPC attatch Successfull"},
	{MQND_TASK_CREATE_FAIL, "MQND - Tast Create Failed"},
	{MQND_TASK_CREATE_SUCCESS, "MQND - Task Create is Successfull"},
	{MQND_TASK_START_FAIL, "MQND - Task Start Failed"},
	{MQND_TASK_START_SUCCESS, "MQND - Task Start Successfull"},

	{MQND_MQD_SERVICE_WENT_DOWN, "MQND - MQD Service Went Down"},
	{MQND_MQD_SERVICE_IS_DOWN, "MQND - MQD Service Is Down"},

	{MQND_CB_ALLOC_FAILED, "MQND - CB Creation Failed"},
	{MQND_EVT_ALLOC_FAILED, "MQND - Event Database Creation Failed"},
	{MQND_MQA_LNODE_ALLOC_FAILED, "MQND - MQA List node Creation Failed"},
	{MQND_ALLOC_QUEUE_NODE_FAILED, "MQND - MQND QueueNode Allocation Failed"},
	{MQND_ALLOC_QNAME_NODE_FAILED, "MQND - MQND QueueNode Allocation Failed"},
	{MQND_ALLOC_QTRANS_EVT_NODE_FAILED, "MQND - Queue Transfer Event node Alloc Failed"},
	{MQND_MDS_REGISTER_FAILED, "MQND - Reg with MDS Failed"},
	{MQND_MDS_REGISTER_SUCCESS, "MQND - Reg with MDS Successfull"},
	{MQND_MDS_GET_HDL_FAILED, "MQND - Cannot Get MDS Handle -Failed"},
	{MQND_MDS_INSTALL_FAILED, "MQND - Installing of MDS with MQND Failed"},
	{MQND_MDS_SUBSCRIBE_FAILED, "MQND - Subscribing With MQND Failed"},
	{MQND_MDS_UNREG_FAILED, "MQND - Unregestreing With MDS Failed"},
	{MQND_MDS_SEND_FAIL, "MQND - MDS Send Failed"},
	{MQND_MDS_ENC_FAILED, "MQND - MDS Encode Failed"},
	{MQND_MDS_DEC_FAILED, "MQND - MDS Decode Failed"},
	{MQND_MDS_SND_TO_MAILBOX_FAILED, "MQND - Sending the event to the MQND Mail Box failed"},
	{MQND_MDS_SNDDIRECT_RCV, "MQND - Direct receive with the endianness flag set to "},
	{MQND_MQA_SERVICE_WENT_DOWN, "MQND - MQA is down with the nodeid "},
	{MQND_MQD_SERVICE_CAME_UP, "MQND - MQD service is up "},
	{MQND_MQA_CAME_UP, "MQND - MQA came up"},
	{MQND_MDS_SND_RSP_DIRECT_FAILED, "MQND - Mds Send Response Direct Failed"},
	{MQND_MDS_BCAST_SEND_FAIL, "MQND - Mds Broadcat Send Failed"},

	{MQND_CB_HDL_CREATE_FAILED, "MQND - CB Handle Creation Failed"},
	{MQND_CB_HDL_TAKE_FAILED, "MQND - CB Take Failed"},
	{MQND_CB_HDL_GIVE_FAILED, "MQND - CB Give Failed"},
	{MQND_CB_HDL_DESTROY_FAILED, "MQND - CB Destroy Failed"},

	{MQND_CLM_DISPATCH_FAILURE, "MQND - CLM Dispatch Failed"},

	{MQND_ASAPI_BAD_RESP, "MQND - ASAPi Bad Response Received"},

	{MQND_INIT_SUCCESS, "MQND - Init Success"},
	{MQND_INIT_FAIL, "MQND - Init Failed"},

	{MQND_DESTROY_SUCCESS, "MQND - Destroy Success"},
	{MQND_DESTROY_FAIL, "MQND - Destroy Failed"},

	{MQND_AMF_REGISTER_FAILED, "MQND - Reg With AMF Failed"},
	{MQND_AMF_REGISTER_SUCCESS, "MQND - Reg with AMF Successfull"},
	{MQND_AMF_INIT_FAILED, "MQND - Init With AMF Failed"},
	{MQND_AMF_INIT_SUCCESS, "MQND - Init With AMF Successfull"},
	{MQND_AMF_DESTROY_FAILED, "MQND - Destroy with AMF Failed"},
	{MQND_AMF_RESPONSE_FAILED, "MQND - Response From AMF Failed"},
	{MQND_AMF_GET_SEL_OBJ_FAILURE, "MQND - Get Selection Object with AMF Failed"},
	{MQND_AMF_GET_SEL_OBJ_SUCCESS, "MQND - Get Selection Object with AMF Successfull"},
	{MQND_AMF_DISPATCH_FAILURE, "MQND - Dispatch with AMF Failed"},
	{MQND_AMF_DISPATCH_SUCCESS, "MQND - Dispatch with AMF Successfull"},
	{MQND_AMF_COMP_NAME_GET_FAILED, "MQND - Amf Component Get Failed"},
	{MQND_AMF_COMP_NAME_GET_SUCCESS, "MQND - Amf Component Get Successfull"},
	{MQND_AMF_COMP_UNREGISTER_FAILED, "MQND - Amf Componet Unreg Failed"},
	{MQND_AMF_COMP_UNREGISTER_SUCCESS, "MQND - Amf Componet Unreg Successfull"},
	{MQND_AMF_TERM_CLBK_CALLED, "MQND - Amf Terminate Call back called"},
	{MQND_AMF_CSI_SET_CLBK_CALLED, "MQND - Amf CSI set Call back called with new state as "},
	{MQND_AMF_CSI_RMV_CLBK_CALLED, "MQND - Amf CSI Remove Callback Called"},
	{MQND_MQA_TMR_STARTED, "MQND - MQA timer Started"},

	{MQND_CPSV_CKPT_INIT_FAILURE, "MQND - Chekckpoint Init Failed"},
	{MQND_RESTART_INIT_OPEN_SUCCESS, "MQND - MQND Restarting with Existing Ckpt Open Success"},
	{MQND_RESTART_BUILD_DB_FROM_CKPTSVC_FAILED, "MQND - Building Database from Ckptsvc Failed"},
	{MQND_RESTART_CKPT_READ_FAILED, "MQND - Reading from the Ckpt Failed"},
	{MQND_RESTART_CKPT_SECTION_DELETE_FAILED, "MQND - Deleting from the Ckpt Failed"},
	{MQND_RESTART_CPSV_INIT_FAILED, "MQND - Reg with CPSV Failed"},
	{MQND_AMF_REGISTRATION_SUCCESS, "MQND - AMF Reg Success"},
	{MQND_AMF_INITIALISATION_SUCCESS, "MQND - AMF Init Success"},
	{MQND_RESTART_INIT_FIRST_TIME, "MQND - First time starting the MQND"},
	{MQND_RESTART_NO_MQA_FOUND, "MQND - No mqa is up till this moment"},

	{MQND_HEALTH_CHECK_START_FAILED, "MQND - Health Check Start Failed"},
	{MQND_HEALTH_CHECK_START_SUCCESS, "MQND - Health Check Start Successfull"},
	{MQND_MDS_GET_HDL_SUCCESS, "MQND - MDS Get Handle is Successfull"},
	{MQND_MDS_INSTALL_SUCCESS, "MQND - MDS Install is successfull"},
	{MQND_MDS_SUBSCRIBE_SUCCESS, "MQND - MDS Subscriptio to the Agent and director are successfull"},
	{MQND_MDS_UNREG_SUCCESS, "MQND - MDS Unreging is Successfull"},
	{MQND_MDS_CLBK_COMPLETE, "MQND - MDS callback is completed with clbk type as "},
	{MQND_MSGQ_CREATE_FAILED, "MQND - MSGQ Creation Failed "},
	{MQND_LISTENERQ_CREATE_FAILED, "MQND - Listener Queue creation Failed"},
	{MQND_QNODE_ADD_DB_FAILED, "MQND - Adding the node to the database failed"},
	{MQND_QNAMENODE_ADD_DB_FAILED, "MQND - Adding the Queue name node to the database failed "},
	{MQND_QTRANS_EVT_NODE_ADD_DB_FAILED, "MQND - Adding the Q Trans Evt node to the DB failed "},
	{MQND_CKPT_SECTION_CREATE_FAILED, "MQND - Check point Section Create Failed"},
	{MQND_Q_REG_WITH_MQD_FAILED, "MQND - Queue Reg with MQD failed "},
	{MQND_QUEUE_CREATE_SUCCESS, "MQND - Queue Creation is Successfull"},
	{MQND_CKPT_SECTION_DELETE_FAILED, "MQND - Ckpt Service Section delete Failed"},
	{MQND_Q_CREATE_ATTR_GET, "MQND - Queue Creation Attributes Get Function rc is "},
	{MQND_Q_REG_WITH_MQD_SUCCESS, "MQND - Queue Reg with MQD is successfull"},
	{MQND_ASAPI_REG_HDLR_FAILED, "MQND - Asapi Opr handler for Queue Register Failed"},
	{MQND_ASAPI_DEREG_HDLR_FAILED, "MQND - Asapi Opr Handler for Queue Dereg Failed"},
	{MQND_MEM_ALLOC_FAILED, "MQND - Memory Allocation Failed"},
	{MQND_MSGQ_TBL_EXACT_FLTR_REGISTER_FAILED, "MQND - For MSGQ Tbl Exact Filet Reg Failed"},
	{MQND_MSGQ_TBL_EXACT_FLTR_REGISTER_SUCCESS, "MQND - For MSGQ Tbl Exact Filet Reg Success"},
	{MQND_MSGQPR_TBL_EXACT_FLTR_REGISTER_FAILED, "MQND - Exact Filter Reg for MSGQ Priority table Failed"},
	{MQND_MSGQPR_TBL_EXACT_FLTR_REGISTER_SUCCESS, "MQND - Exact Filter Reg for MSGQ Priority table Success"},
	{MQND_MQA_TMR_EXPIRED, "MQND - The MQA timer expired "},
	{MQND_TMR_STARTED, "MQND - Timer Started"},
	{MQND_EVT_RECEIVED, "MQND - Event received of type "},
	{MQND_CTRL_EVT_RECEIVED, "MQND - Control Event received of type "},
	{MQND_MQA_DOWN, "MQND - The Agent is down on the node id :"},
	{MQND_INITIALIZATION_INCOMPLETE, "MQND - MQND is not completely Initialized"},
	{MQND_FINALIZE_CLOSE_STATUS, "MQND - The Queue Is not Properly Closed as part of Finalizing"},
	{MQND_FINALIZE_RESP_SENT_FAILED, "MQND - The Finalize response isnot sent properly to the agent"},
	{MQND_FINALIZE_RESP_SENT_SUCCESS, "MQND - The Finalize response is sent properly to the agent"},
	{MQND_INIT_RESP_SENT_FAILED, "MQND - The Initialize response is not sent properly to the Agent"},
	{MQND_INIT_RESP_SENT_SUCCESS, "MQND - The Initialize response is sent properly to the Agent"},
	{MQND_ASAPI_NRESOLVE_HDLR_FAILED, " MQND - Asapi Opr Handler for NameResolve Request Failed"},
	{MQND_PROC_QUEUE_OPEN_FAILED, "MQND - The procedure to open the Queue Failed "},
	{MQND_GET_QNODE_FAILED, "MQND - Get queue node Failed  "},
	{MQND_PROC_QUEUE_CLOSE_FAILED, "MQND - The procedure to close the queue Failed"},
	{MQND_PROC_QUEUE_CLOSE_SUCCESS, "MQND - The procedure to close the queue Success"},
	{MQND_QUEUE_CLOSE_RESP_FAILED, "MQND - Sending the close response Failed"},
	{MQND_QUEUE_CLOSE_RESP_SUCCESS, "MQND - Sending the close response Success"},
	{MQND_QUEUE_UNLINK_RESP_FAILED, "MQND - Sending the unlink response Failed"},
	{MQND_QUEUE_UNLINK_RESP_SUCCESS, "MQND - Sending the unlink response Successfull"},
	{MQND_QUEUE_STAT_REQ_RESP_FAILED, "MQND - Sending the Status request response Failed"},
	{MQND_QUEUE_STAT_REQ_RESP_SUCCESS, "MQND - Sending the Status request response Successfull"},
	{MQND_UNDER_TRANSFER_STATE, "MQND - The queue is under transfer Process"},
	{MQND_QUEUE_FULL, "MQND - The queue is full"},
	{MQND_QUEUE_GET_ATTR_FAILED, "MQND - Unable to get the queue attributes from the queue"},
	{MQND_QUEUE_RESIZE_FAILED, "MQND - Unable to resize the queue to the given size"},
	{MQND_SEND_FAILED, "MQND - Unable to send the message to the Queue"},
	{MQND_LISTENER_SEND_FAILED, "MQND - Unable to send the message to the listener Queue"},
	{MQND_MDS_SND_FAILED, "MQND - Unable to send the respons in async case"},
	{MQND_MDS_SND_RSP_FAILED, "MQND - Queue Attribute get :Mds Send Response Failed"},
	{MQND_MDS_SND_RSP_SUCCESS, "MQND - Queue Attribute get: Mds Send Response Success"},
	{MQND_MQA_ADD_NODE_FAILED, "MQND - Unable to add the MQA node into the MQAList"},
	{MQND_RETENTION_TMR_EXPIRED, "MQND - MQND Retention timer expired"},
	{MQND_QTRANSFER_NODE1_TMR_EXPIRED, "MQND - MQND QTrans Timer on Node 1 expired"},
	{MQND_QTRANSFER_NODE2_TMR_EXPIRED, "MQND - MQND QTrans Timer on Node 2 expired"},
	{MQND_MQ_DESTROY_FAILED, "MQND - MSGQ Destroy routine Failed"},
	{MQND_QUEUE_ERR_BUSY, "MQND - MSGQ is Busy (Opened by someone)"},
	{MQND_QUEUE_TRANS_IN_PROGRESS, "MQND - MSGQ Transfer in progress"},
	{MQND_QUEUE_CREATE_FAILED, "MQND - Queue Creation Failed"},
	{MQND_TRANSER_REQ_FAILED, "MQND - Transfer Request to the remote MQND is failed"},
	{MQND_MQUEUE_EMPTY_FAILED, "MQND - MSGQ Empty is not Successfull"},
	{MQND_Q_ATTR_GET_FAILED, "MQND - Unable to get the MSGQ Creation Attributes"},
	{MQND_Q_ATTR_COMPARE_FAILED,
	 "MQND - Comparison between the Receive attributes and existing queue attributes Failed"},
	{MQND_TRANSFER_OWNER_REQ_FAILED, "MQND - The Queue Transfer owner request Failed"},
	{MQND_EXISTING_LOCAL_QUEUE_OPEN_FAILED, "MQND - Attempt to open the existing local message queue failed"},
	{MQND_QUEUE_CREAT_FLAG_NOT_SET, "MQND - Queue Creation is not set for a non existing queue"},
	{MQND_RETENTION_TMR_STARTED, "MQND - Retention timer has been strted for the queue"},
	{MQND_QTRANSFER_NODE1_TMR_STARTED, "MQND - QTransfer Tmr on Node 1 started(INFO)/Failed(ERROR)"},
	{MQND_QTRANSFER_NODE2_TMR_STARTED, "MQND - QTransfer Tmr on Node 2 started(INFO)/Failed(ERROR)"},
	{MQND_MQA_LNODE_ADD_FAILED, "MQND - Adding the mqa node to the mqa list failed"},
	{MQND_CPSV_CKPT_INIT_SUCCESS, "MQND - Ckpt Initialize is successfull"},
	{MQND_RESTART_OPEN_FIRST_TIME_FAILED, "MQND - Ckpt Open Failed in the first time"},
	{MQND_CKPT_OPEN_FAILED, "MQND - Ckpt opening Failed"},
	{MQND_CKPT_ITERATION_INIT_FAILED, "MQND - Ckpt Iteration Initialize Failed"},
	{MQND_CKPT_BUILD_DATABASE_FAILED, "MQND - Ckpt Building the database Failed"},
	{MQND_CKPT_SECTION_OVERWRITE_FAILED, "MQND - Ckpt Overwrite Failed "},
	{MQND_QNODE_ADD_TO_DATABASE_FAILED,
	 "MQND - During restart building, the database node is not added to the Database"},
	{MQND_CKPT_READ_SUCCESS, "MQND - Reading from the Ckpt Success"},
	{MQND_SHM_CREATE_FAILED, "MQND - Shared memory creation failed"},
	{MQND_MSG_FRMT_VER_INVALID, "MQND - Message Format Version Invalid"},
	{MQND_CLM_INIT_FAILED, "MQND - CLM init failed"},
	{MQND_CLM_NODE_GET_FAILED, "MQND - CLM node get failed"},
	{0, 0}
};

/*****************************************************************************\
 Build up the canned constant strings for the ASCII SPEC
\******************************************************************************/
NCSFL_SET mqnd_str_set[] = {
	{MQND_FC_HDLN, 0, (NCSFL_STR *)mqnd_hdln_set},
	{0, 0, 0}
};

NCSFL_FMAT mqnd_fmat_set[] = {
	{MQND_LID_HDLN, NCSFL_TYPE_TCLIL, "MQSv %s %14s:%5lu:%s:%lu\n"},
	{MQND_LID_GENLOG, NCSFL_TYPE_TC, "%s %s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC mqnd_ascii_spec = {
	NCS_SERVICE_ID_MQND,	/* MQND subsystem */
	MQSV_LOG_VERSION,	/* MQND (MQSv-MQND) revision ID */
	"mqnd",			/* MQND opening Banner in log */
	(NCSFL_SET *)mqnd_str_set,	/* MQND const strings referenced by index */
	(NCSFL_FMAT *)mqnd_fmat_set,	/* MQND string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/****************************************************************************\
  PROCEDURE NAME : mqnd_log_ascii_reg
                                                                                                                             
  DESCRIPTION    : This is the function which registers the MQND's logging
                   ascii set with the DTSv Log server.
                                                                                                                             
  ARGUMENTS      : None
                                                                                                                             
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uint32_t mqnd_log_ascii_reg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;
	uint32_t rc = NCSCC_RC_SUCCESS;

	memset(&arg, 0, sizeof(arg));

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &mqnd_ascii_spec;

	/* Register MQND Canned strings with DTSv */
	rc = ncs_dtsv_ascii_spec_api(&arg);
	return rc;
}

/****************************************************************************\
  PROCEDURE NAME : mqnd_log_ascii_dereg
                                                                                                                             
  DESCRIPTION    : This is the function which deregisters the MQND's logging
                   ascii set from the DTSv Log server.
                                                                                                                             
  ARGUMENTS      : None
                                                                                                                             
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
void mqnd_log_ascii_dereg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_MQND;
	arg.info.dereg_ascii_spec.version = MQSV_LOG_VERSION;

	/* Dereg MQND Canned strings from DTSv */
	ncs_dtsv_ascii_spec_api(&arg);
	return;
}
#endif   /* (NCS_DTS == 1) */

#endif
