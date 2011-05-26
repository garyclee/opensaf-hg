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
  FILE NAME: MQA_LOGSTR.C

  DESCRIPTION: This file contains all the log string related to MQA.

******************************************************************************/

#if(NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "mqa_log.h"
#include "mqsv.h"

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR mqa_hdln_set[] = {
	{MQA_CB_CREATE_FAILED, "MQA - Instance Create Failed"},
	{MQA_CB_DESTROY_FAILED, "MQA - Insatnce destroy Failed"},
	{MQA_CLIENT_TREE_INIT_FAILED, "MQA - Client database Initialization Failed"},
	{MQA_CLIENT_TREE_ADD_FAILED, "MQA - Client database Registration Failed"},
	{MQA_CLIENT_TREE_DEL_FAILED, "MQA - Client database Deregistration Failed"},
	{MQA_CLIENT_TREE_DESTROY_FAILED, "MQA - Client database Finalization Failed"},
	{MQA_QUEUE_TREE_INIT_FAILED, "MQA - Queue database Initialization Failed"},
	{MQA_QUEUE_TREE_ADD_FAILED, "MQA - Queue database Registration Failed"},
	{MQA_QUEUE_TREE_DEL_FAILED, "MQA - Queue database Deregistration Failed"},
	{MQA_QUEUE_TREE_DESTROY_FAILED, "MQA - Queue database Finalization Failed"},
	{MQA_TAKE_HANDLE_FAILED, "MQA - Handle acquisition Failed"},
	{MQA_CREATE_HANDLE_FAILED, "MQA - Handle Registration Failed"},
	{MQA_MDS_REGISTER_SUCCESS, "MQA - MDS Registration Success"},
	{MQA_MDS_REGISTER_FAILED, "MQA - MDS registration Failed"},
	{MQA_ASAPi_REGISTER_FAILED, "MQA - ASAPi Bind Failed"},
	{MQA_ASAPi_UNREGISTER_FAILED, "MQA - ASAPi Deregisteration Failed"},
	{MQA_ASAPi_OPERATION_SUCCESS, "MQA - ASAPi Operation Success"},
	{MQA_ASAPi_OPERATION_FAILED, "MQA - ASAPi Operation Failed"},
	{MQA_SE_API_CREATE_SUCCESS, "MQA - MsgQ Svc Registration Success"},
	{MQA_SE_API_CREATE_FAILED, "MQA - MsgQ Svc Registration Failed"},
	{MQA_SE_API_DESTROY_SUCCESS, "MQA - MsgQ Svc Deregistration Success"},
	{MQA_SE_API_DESTROY_FAILED, "MQA - MsgQ Svc Deregistration Failed"},
	{MQA_SE_API_UNKNOWN_REQUEST, "MQA - MsgQ Svc Req unknown"},
	{MQA_CB_RETRIEVAL_FAILED, "MQA - Control block retrieval failed"},
	{MQA_VERSION_INCOMPATIBLE, "MQA - version check failed"},
	{MQA_MDS_GET_HANDLE_FAILED, "MQA - MDS registration failed"},
	{MQA_MDS_CALLBK_FAILURE, "MQA - MDS callback failed"},
	{MQA_MDS_CALLBK_UNKNOWN_OP, "MQA - MDS callback unknown operation"},
	{MQA_TIMER_TABLE_INIT_FAILED, "MQA - Tmr Initialization Failed"},
	{MQA_TRACK_TREE_ADD_FAILED, "MQA - Track Database Registration Failed"},
	{MQA_TRACK_TREE_DEL_FAILED, "MQA - Track Database Deregistration Failed"},
	{MQA_API_MSG_GROUP_ADD_FAIL, "MQA - Adding a member to Group is Failed"},
	{MQA_API_MSG_GROUP_DEL_FAIL, "MQA - Deletion of Group is Failed"},
	{MQA_API_MSG_GROUP_DEL_SUCCESS, "MQA - Deletion of a Group is Success"},
	{MQA_API_MSG_GROUP_INSERT_FAIL, "MQA - Insertion of a Member to a Group is Failed"},
	{MQA_API_MSG_GROUP_INSERT_SUCCESS, "MQA - Insertion of a Member to a Group is Success"},
	{MQA_API_MSG_GROUP_REMOVE_FAIL, "MQA - Removing the member from the Group is Failed"},
	{MQA_API_MSG_GROUP_REMOVE_SUCCESS, "MQA - Removing the member from the Group is Successfull"},
	{MQA_CB_LOCK_INIT_FAILED, "MQA - Cotnrol Block Lock Initialisation Failed"},
	{MQA_EDU_HDL_INIT_FAILED, "MQA - Edu Handle Initialization Failed "},
	{MQA_REGISTER_WITH_ASAPi_FAILED, "MQA - Registration with ASAPi Failed"},
	{MQA_SYNC_WITH_MQD_UP, "MQA - MQD synced up with the MQA "},
	{MQA_SYNC_WITH_MQND_UP, "MQA - MQND synced up with the MQA "},
	{MQA_MQD_ALREADY_UP, "MQA - MQD is already up with the MQA "},
	{MQA_MQND_ALREADY_UP, "MQA - MQND is already up with the MQA "},
	{MQA_CREATE_HANDLE_SUCCESS, "MQA - Handle Registration Success"},
	{MQA_CB_LOCK_INIT_SUCCESS, "MQA - Control Block lock initialization Success"},
	{MQA_CLIENT_TREE_INIT_SUCCESS, "MQA - Client Database Initialization Success"},
	{MQA_QUEUE_TREE_INIT_SUCCESS, "MQA - Queue Database Initialization Success"},
	{MQA_EDU_HDL_INIT_SUCCESS, "MQA - EDU Handle Initialization Success"},
	{MQA_MDS_INSTALL_FAILED, "MQA - MDS Service installation Failed"},
	{MQA_MDS_SUBSCRIPTION_ND_FAILED, "MQA - MDS Subscription to MQND up and down events failed"},
	{MQA_MDS_SUBSCRIPTION_D_FAILED, "MQA - MDS Subscription to MQD up and down events failed"},
	{MQA_MDS_DEREGISTER_FAILED, "MQA - MDS Deregistration Failed"},
	{MQA_MDS_ENC_FAILED, "MQA - MDS Encoding Failed "},
	{MQA_MDS_DEC_FAILED, "MQA - MDS Decoding Failed "},
	{MQA_STOP_DELETE_TMR_SUCCESS, "MQA - Stop and Delete Tmr Success"},
	{MQA_STOP_DELETE_TMR_FAILED, "MQA - Stop and Delete Tmr Failed"},
	{MQA_ASAPi_MSG_RECEIVE_FAILED, "MQA - ASAPi Message Receive Failed with the returnvalue"},
	{MQA_MQND_DOWN, "MQA - MQND is down with nodeid"},
	{MQA_MQD_DOWN, "MQA - MQD is down "},
	{MQA_MQND_UP, "MQA - MQND is up on the nodeid "},
	{MQA_MQD_UP, "MQA - MQD is up "},
	{MQA_CB_TIMER_INIT_FAILED, "MQA - Tmr initialization Failed "},
	{MQA_CB_TMR_DELETE_FAILED, "MQA - Tmr Deletion Failed "},
	{MQA_CB_TMR_STOP_FAILED, "MQA - Tmr Stop Failed "},
	{MQA_CB_TMR_START_FAILED, "MQA - Tmr Start Failed"},
	{MQA_CB_TMR_CREATE_FAILED, "MQA - Tmr Create Failed "},
	{MQA_CLIENT_TREE_FIND_FAILED, "MQA - Client Database Find Failed"},
	{MQA_QUEUE_READER_TASK_CREATE_FAILED, "MQA - Queue Reader Thread Task Create Failed"},
	{MQA_QUEUE_READER_TASK_START_FAILED, "MQA - Queue Reader Thread Task Start Failed"},
	{MQA_QUEUE_TREE_FIND_FAILED, "MQA - Queue Database Find Failed"},
	{MQA_TRACK_TREE_FIND_AND_DEL_FAILED, "MQA - Track Tree Find and Delete Failed"},
	{MQA_CLBK_QUEUE_WRITE_FAILED, "MQA - Call back Queue Write Failed"},
	{MQA_NOTIFY_CHANGES_FAILED, "MQA - Notify Changes Failed"},
	{MQA_NOTIFY_CHANGES_ONLY_FAILED, "MQA - Notify Changes Only Failed"},
	{MQA_CLBK_QUEUE_INIT_FAILED, "MQA - Callback Queue Initialization Failed"},
	{MQA_NCS_AGENTS_START_FAILED, "MQA - NCS Agents Startup Failed"},
	{MQA_INVALID_PARAM, "MQA - Invalid Parameter as input"},
	{MQA_MQD_OR_MQND_DOWN, "MQA - MQD or MQND is down "},
	{MQA_LOCK_WRITE_FAILED, "MQA - Locking the control block for the write is failed"},
	{MQA_FINALIZE_CLOSE_FAILED, "MQA - Close as part of Finalize failed "},
	{MQA_OPEN_CALLBACK_MISSING, "MQA - Open Call back incase of Async open,  not given as input "},
	{MQA_RECEIVE_CALLBACK_MISSING, "MQA - Receiver Call back is not given as input "},
	{MQA_CREATE_AND_START_TIMER_FAILED, "MQA - Create and Start Tmr Failed "},
	{MQA_QUEUES_NOT_AVLBL_IN_GROUP, "MQA - There are no queues in the group"},
	{MQA_INVALID_MSG_SIZE, "MQA - Message size is greater than the system defined size"},
	{MQA_DELIVERED_CALLBACK_MISSING, "MQA - Delivered Callback is not defined after mentioning the async send"},
	{MQA_ASAPi_GETDEST_FAILED, "MQA - The ASAPi Get Dest Operation Failed"},
	{MQA_ASAPi_GETDEST_CACHE_NOT_EXIST, "MQA - The ASAPi Get Dest Operation's result Cache does not exist"},
	{MQD_ASAPi_REG_FAILED, "MQA - The ASAPi Registration Req Failed "},
	{MQD_ASAPi_DEREG_FAILED, "MQA - The ASAPi Deregistration Req Failed "},
	{MQD_ASAPi_GROUP_INSERT_FAILED, "MQA - The ASAPi Group Insert Req Failed"},
	{MQD_ASAPi_GROUP_REMOVE_FAILED, "MQA - The ASAPi Group Remove Req Failed"},
	{MQD_ASAPi_TRACK_FAILED, "MQA - The ASAPi Track Req Failed"},
	{MQA_MSG_GET_NATIVE_QUEUE_ERROR, "MQA - Message get failed due to native msgrcv error"},
	{MQA_MSG_UNABLE_TO_PUT_GENUINE_MESSAGE, "MQA - Unable to put back the genuine message in msgget call"},
	{MQA_MSG_UNABLE_TO_PUT_STOP_TIMER_MESSAGE,
	 "MQA - Unable to put back the stop Tmr message which is meant for a different msgget"},
	{MQA_SENDERID_LIST_INSERTION_FAILED, "MQA - Insertion of the sender id into the senderid list failed"},
	{MQA_MSG_UNABLE_TO_PUT_CANCEL_MESSAGE, "MQA - Unable to put the cancel message in the queue"},
	{MQA_INVALID_MSG_PRIORITY, "MQA - Invalid message priority of the message queue"},
	{MQA_SENDRECV_SEND_FAILED, "MQA - The send part of the message sendreceive failed"},
	{MQA_MSG_NO_SPACE, "MQA - There is no space synchronization in reply and sendrecive calls "},
	{MQA_SENDER_INFO_DOES_NOT_EXIST, "MQA - Sender info does not exist in the queue"},
	{MQA_INVALID_GROUP_POLICY, "MQA - Invalid Group Policy Input"},
	{MQA_INVALID_TRACK_FLAGS, "MQA - Invalid Track Flags"},
	{MQA_CLIENT_TREE_FIND_AND_ADD_FAILED, "MQA - Unable to add to the Client Database"},
	{MQA_TRACK_TREE_FIND_FAILED, "MQA - Failed to find the track tree node"},

	/* Memory allocation failures flex */
	{MQA_CB_ALLOC_FAILED, "MQA - Control block creation failed"},
	{MQA_CLIENT_ALLOC_FAILED, "MQA - Client database creation failed"},
	{MQA_QUEUE_ALLOC_FAILED, "MQA - Queue database creation failed"},
	{MQA_EVT_ALLOC_FAILED, "MQA - Event database creation failed"},
	{MQA_TIMER_NODE_ALLOC_FAILED, "MQA - Tmrs database creation failed"},
	{MQA_TRACK_ALLOC_FAILED, "MQA - Track database creation failed"},
	{MQP_ASYNC_RSP_MSG_ALLOC_FAILED, "MQA - MQP Async Rsp Message Allocation Failed"},
	{MQP_OPEN_RSP_ALLOC_FAILED, "MQA - MQP Open Rsp Message Allocatiion Failed"},
	{MQA_TRACK_BUFFER_INFO_ALLOC_FAILED, "MQA - Track Buffer Info Allocation Failed"},
	{MQA_CANCEL_REQ_ALLOC_FAILED, "MQA - Track Buffer Info Allocation Failed"},
	{MQA_MEMORY_ALLOC_FAILED, "MQA - Memory allocation failed"},

	/* API Failure Flex */
	{MQA_API_MSG_INITIALIZE_FAIL, "MQA - MsgQ Svc initialization failed"},
	{MQA_API_MSG_INITIALIZE_SUCCESS, "MQA - MsgQ Svc initialization Success"},
	{MQA_API_MSG_SELECTION_OBJECT_FAIL, "MQA - MsgQ Svc selection object retrieval failed"},
	{MQA_API_MSG_SELECTION_OBJECT_SUCCESS, "MQA - MsgQ Svc selection object retrievval Success"},
	{MQA_API_MSG_DISPATCH_FAIL, "MQA - MsgQ Svc event dispatch failed"},
	{MQA_API_MSG_DISPATCH_SUCCESS, "MQA - MsgQ Svc event dispatch succeeded"},
	{MQA_API_MSG_FINALIZE_FAIL, "MQA - MsgQ Svc finalize failed"},
	{MQA_API_MSG_FINALIZE_SUCCESS, "MQA - MsgQ Svc finalize succeeded"},
	{MQA_API_QUEUE_OPEN_SYNC_FAIL, "MQA - MsgQ Svc Queue Open failed"},
	{MQA_API_QUEUE_OPEN_SYNC_SUCCESS, "MQA - Message servicce Queue Open Success"},
	{MQA_API_QUEUE_OPEN_ASYNC_FAIL, "MQA - MsgQ Svc asynchronous Queue Open failed"},
	{MQA_API_QUEUE_OPEN_ASYNC_SUCCESS, "MQA - MsgQ Svc asynchronous Queue Open success"},
	{MQA_API_QUEUE_CLOSE_SYNC_FAIL, "MQA - MsgQ Svc Queue close failed"},
	{MQA_API_QUEUE_CLOSE_SYNC_SUCCESS, "MQA - MsgQ Svc Queue close Success"},
	{MQA_API_QUEUE_STATUS_GET_SYNC_FAIL, "MQA - MsgQ Svc Queue status retrieval  failed"},
	{MQA_API_QUEUE_STATUS_GET_SYNC_SUCCESS, "MQA - MsgQ Svc Queue status retreival success"},
	{MQA_API_QUEUE_UNLINK_SYNC_FAIL, "MQA - MsgQ Svc Queue unlink failed"},
	{MQA_API_QUEUE_UNLINK_SYNC_SUCCESS, "MQA - MsgQ Svc Queue unlink success"},
	{MQA_DISPATCH_ALL_CALLBK_FAIL, "MQA - MsgQ Svc callback dispatch all failed"},
	{MQA_API_QUEUE_SEND_SYNC_FAIL, "MQA - MsgQ Svc syncsend failed"},
	{MQA_API_QUEUE_SEND_SYNC_SUCCESS, "MQA - MsgQ Svc sync send success"},
	{MQA_API_QUEUE_SEND_ASYNC_FAIL, "MQA - MsgQ Svc async send failed"},
	{MQA_API_QUEUE_SEND_ASYNC_SUCCESS, "MQA - MsgQ Svc async send success"},
	{MQA_API_QUEUE_MESSAGE_GET_FAIL, "MQA - MsgQ Svc msg get failed"},
	{MQA_API_QUEUE_MESSAGE_GET_SUCCESS, "MQA - MsgQ Svc msg get success"},
	{MQA_API_QUEUE_MESSAGE_CANCEL_FAIL, "MQA - MsgQ Svc msg cancel failed"},
	{MQA_API_QUEUE_MESSAGE_CANCEL_SUCCESS, "MQA - MsgQ Svc msg cancel success"},
	{MQA_API_QUEUE_SENDRCV_FAIL, "MQA - MsgQ Svc msgsendreceive failed"},
	{MQA_API_QUEUE_SENDRCV_SUCCESS, "MQA - MsgQ Svc msgsendreceive success"},
	{MQA_API_QUEUE_REPLY_SYNC_FAILED, "MQA - MsgQ Svc msgreply sync failed"},
	{MQA_API_QUEUE_REPLY_SYNC_SUCCESS, "MQA - MsgQ Svc msgreply sync success"},
	{MQA_API_QUEUE_REPLY_ASYNC_FAILED, "MQA - MsgQ Svc msgreply async failed"},
	{MQA_API_QUEUE_REPLY_ASYNC_SUCCESS, "MQA - MsgQ Svc msgreply async success"},
	{MQD_API_GROUP_CREATE_FAILED, "MQA - MsgQ Svc group create failed"},
	{MQD_API_GROUP_CREATE_SUCCESS, "MQA - MsgQ Svc group create success"},
	{MQA_API_MSG_GROUP_TRACK_FAIL, "MQA - MsgQ Svc group track failed"},
	{MQA_API_MSG_GROUP_TRACK_SUCCESS, "MQA - MsgQ Svc group track success"},
	{MQA_API_MSG_GROUP_TRACK_STOP_FAIL, "MQA - MsgQ Svc group trackstop failed"},
	{MQA_API_MSG_GROUP_TRACK_STOP_SUCCESS, "MQA - MsgQ Svc group trackstop success"},

	/* MDS Send Failure Flex */
	{MQA_MDS_SEND_FAILURE, "MQA - Message Send through MDS Failure"},
	{MQA_MDS_SEND_TIMEOUT, "MQA - Message Send through MDS Timeout"},
	{MQA_MSG_FRMT_VER_INVALID, "MQA - Message Format version Invalid"},
	{0, 0}
};

/*****************************************************************************\
 Build up the canned constant strings for the ASCII SPEC
\******************************************************************************/
NCSFL_SET mqa_str_set[] = {
	{MQA_FC_HDLN, 0, (NCSFL_STR *)mqa_hdln_set},
	{0, 0, 0}
};

NCSFL_FMAT mqa_fmat_set[] = {
	{MQA_LID_HDLN, NCSFL_TYPE_TCLIL, "MQSv %s %12s:%5lu:%s:%lu\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC mqa_ascii_spec = {
	NCS_SERVICE_ID_MQA,	/* MQA subsystem */
	MQSV_LOG_VERSION,	/* MQA (MQSv-MQA) revision ID */
	"mqa",			/* MQA opening Banner in log */
	(NCSFL_SET *)mqa_str_set,	/* MQA const strings referenced by index */
	(NCSFL_FMAT *)mqa_fmat_set,	/* MQA string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/****************************************************************************\
  PROCEDURE NAME : mqa_log_ascii_reg
 
  DESCRIPTION    : This is the function which registers the MQA's logging
                   ascii set with the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uint32_t mqa_log_ascii_reg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;
	uint32_t rc = NCSCC_RC_SUCCESS;

	memset(&arg, 0, sizeof(arg));

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &mqa_ascii_spec;

	/* Regiser MQA Canned strings with DTSv */
	rc = ncs_dtsv_ascii_spec_api(&arg);
	return rc;
}

/****************************************************************************\
  PROCEDURE NAME : mqa_log_ascii_dereg
 
  DESCRIPTION    : This is the function which deregisters the MQA's logging
                   ascii set from the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
void mqa_log_ascii_dereg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_MQA;
	arg.info.dereg_ascii_spec.version = MQSV_LOG_VERSION;

	/* Deregister MQA Canned strings from DTSv */
	ncs_dtsv_ascii_spec_api(&arg);
	return;
}

#endif   /* (NCS_DTS == 1) */
