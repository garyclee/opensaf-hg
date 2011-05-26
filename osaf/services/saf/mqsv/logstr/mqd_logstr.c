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
  FILE NAME: MQD_LOGSTR.C

  DESCRIPTION: This file contains all the log string related to MQD.

******************************************************************************/

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "mqd_log.h"
#include "mqd_logstr.h"
#include "mqsv.h"
#include "mqnd_log.h"
#include "mqa_log.h"

#if(NCS_DTS == 1)

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR mqd_hdln_set[] = {
	{MQD_CREATE_SUCCESS, "MQD - Instance Create Success"},
	{MQD_CREATE_FAILED, "MQD - Insatnce Create Failed"},
	{MQD_INIT_SUCCESS, "MQD - Instance Initialization Success"},
	{MQD_INIT_FAILED, "MQD - Insatnce Initialization Failed"},
	{MQD_CREATE_HDL_SUCCESS, "MQD - Handle Registration Success"},
	{MQD_CREATE_HDL_FAILED, "MQD - Handle Registration Failed"},
	{MQD_CB_HDL_TAKE_FAILED, "MQD - CB Handle Take Failed"},
	{MQD_AMF_INIT_SUCCESS, "MQD - AMF Registration Success"},
	{MQD_AMF_INIT_FAILED, "MQD - AMF Registration Failed"},
	{MQD_LM_INIT_SUCCESS, "MQD - Layer Management Initialization Success"},
	{MQD_LM_INIT_FAILED, "MQD - Layer Management Initialization Failed"},
	{MQD_MDS_INIT_SUCCESS, "MQD - MDS Registration Success"},
	{MQD_MDS_INIT_FAILED, "MQD - MDS Registration Failed"},
	{MQD_REG_COMP_SUCCESS, "MQD - AMF Component Registration Success"},
	{MQD_REG_COMP_FAILED, "MQD - AMF Component Registration Failed"},
	{MQD_ASAPi_BIND_SUCCESS, "MQD - ASAPi Bind Success"},
	{MQD_EDU_BIND_SUCCESS, "MQD - EDU Bind Success"},

	{MQD_EDU_UNBIND_SUCCESS, "MQD - EDU Unbind Success"},
	{MQD_ASAPi_UNBIND_SUCCESS, "MQD - ASAPi Unbind Success"},
	{MQD_DEREG_COMP_SUCCESS, "MQD - AMF Component Deregistration Success"},
	{MQD_MDS_SHUT_SUCCESS, "MQD - MDS Deregistration Success"},
	{MQD_MDS_SHUT_FAILED, "MQD - MDS Deregistration Failed"},
	{MQD_LM_SHUT_SUCCESS, "MQD - Layer Management Destroy Success"},
	{MQD_AMF_SHUT_SUCCESS, "MQD - AMF Deregistration Success"},
	{MQD_DESTROY_SUCCESS, "MQD - Instance Destroy Success"},

	{MQD_COMP_NOT_INSERVICE, "MQD - Component Is Not In Service"},
	{MQD_DONOT_EXIST, "MQD - Instance Doesn't Exist"},
	{MQD_MEMORY_ALLOC_FAIL, "MQD - Failed To Allocate Memeory....."},

	{MQD_ASAPi_REG_MSG_RCV, "MQD - ASAPi Registration Message Received"},
	{MQD_ASAPi_DEREG_MSG_RCV, "MQD - ASAPi Deregistration Message Received"},
	{MQD_ASAPi_NRESOLVE_MSG_RCV, "MQD - ASAPi Name Resolution Message Received"},
	{MQD_ASAPi_GETQUEUE_MSG_RCV, "MQD - ASAPi Getqueue Message Received"},
	{MQD_ASAPi_TRACK_MSG_RCV, "MQD - ASAPi Track Message Received"},
	{MQD_ASAPi_REG_RESP_MSG_SENT, "MQD - ASAPi Registration Response Message Sent"},
	{MQD_ASAPi_DEREG_RESP_MSG_SENT, "MQD - ASAPi Deregistration Response Message Sent"},
	{MQD_ASAPi_NRESOLVE_RESP_MSG_SENT, "MQD - ASAPi Name Resolution Response Message Sent"},
	{MQD_ASAPi_GETQUEUE_RESP_MSG_SENT, "MQD - ASAPi Getqueue Response Message Sent"},
	{MQD_ASAPi_TRACK_RESP_MSG_SENT, "MQD - ASAPi Track Response Message Sent"},
	{MQD_ASAPi_TRACK_NTFY_MSG_SENT, "MQD - ASAPi Track Notification Sent"},
	{MQD_ASAPi_REG_RESP_MSG_ERR, "MQD - Couldn't Send ASAPi Registration Response Message"},
	{MQD_ASAPi_DEREG_RESP_MSG_ERR, "MQD - Couldn't Send ASAPi Deregistration Response Message"},
	{MQD_ASAPi_NRESOLVE_RESP_MSG_ERR, "MQD - Couldn't Send ASAPi Name Resolution Response Message"},
	{MQD_ASAPi_GETQUEUE_RESP_MSG_ERR, "MQD - Couldn't Send ASAPi Getqueue Response Message"},
	{MQD_ASAPi_TRACK_RESP_MSG_ERR, "MQD - Couldn't Send ASAPi Track Response Message"},
	{MQD_ASAPi_TRACK_NTFY_MSG_ERR, "MQD - Couldn't Send ASAPi Track Notification"},
	{MQD_ASAPi_EVT_COMPLETE_STATUS, "MQD - ASAPi Event processed completely with return value"},
	{MQD_OBJ_NODE_GET_FAILED, "MQD - RED Patricia Get is failed for MQD_OBJ_NODE"},
	{MQD_DB_ADD_SUCCESS, "MQD - Database Operation (ADD) Success"},
	{MQD_DB_ADD_FAILED, "MQD - Database Operation (ADD) Failed"},
	{MQD_DB_DEL_SUCCESS, "MQD - Database Operation (DEL) Success"},
	{MQD_DB_DEL_FAILED, "MQD - Database Operation (DEL) Failed"},
	{MQD_DB_UPD_SUCCESS, "MQD - Database Operation (UPD) Success"},
	{MQD_DB_UPD_FAILED, "MQD - Database Operation (UPD) Failed"},
	{MQD_DB_TRACK_ADD, "MQD - Database Operation (TRACK) Added"},
	{MQD_DB_TRACK_DEL, "MQD - Database Operation (TRACK) Deleted"},
	{MQD_CB_ALLOC_FAILED, "MQD - Control Block Allocation Failed"},
	{MQD_A2S_EVT_ALLOC_FAILED, "MQD - Active to Standby Event Failed"},
	{MQD_OBJ_NODE_ALLOC_FAILED, "MQD - OBJ_NODE allocation Failed"},
	{MQD_RED_DB_NODE_ALLOC_FAILED, "MQD - RED_DB_NODE allocation Failed"},
	{NCS_ENC_RESERVE_SPACE_FAILED, "MQD - Encode Reserve Space for Async Count Failed "},
	{MQD_RED_TRACK_OBJ_ALLOC_FAILED, "MQD - TRACK_OBJ allocation Failed"},
	{MQD_RED_BAD_A2S_TYPE, "MQD - Bad Active to Standby Message Type"},
	{MQD_RED_MBCSV_INIT_FAILED, "MQD - MBCSV initialization Failed"},
	{MQD_RED_MBCSV_OPEN_FAILED, "MQD - MBCSV Open Failed "},
	{MQD_RED_MBCSV_FINALIZE_FAILED, "MQD - MBCSV Finalize Failed"},
	{MQD_RED_MBCSV_SELOBJGET_FAILED, "MQD - MBCSV SelObjGet Failed"},
	{MQD_RED_MBCSV_CHGROLE_FAILED, "MQD - MBCSV ChangeRole Failed"},
	{MQD_RED_MBCSV_CHGROLE_SUCCESS, "MQD - MBCSV ChangeRole Success"},
	{MQD_RED_MBCSV_DISPATCH_FAILURE, "MQD - MBCSV Dispatch Failure"},
	{MQD_RED_MBCSV_DISPATCH_SUCCESS, "MQD - MBCSV Dispatch Success"},
	{MQD_RED_MBCSV_ASYNCUPDATE_FAILURE, "MQD - MBCSV AsyncUpdate Failure"},
	{MQD_RED_MBCSV_ASYNCUPDATE_SUCCESS, "MQD - MBCSV   AsyncUpdate Success"},
	{MQD_RED_MBCSV_DATA_REQ_SEND_FAILED, "MQD - MBCSV Data Request Send Failed"},
	{MQD_RED_MBCSV_ASYNCUPDATE_REG_ENC_EDU_ERROR, "MQD - MBCSV Reg Encode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_DEREG_ENC_EDU_ERROR, "MQD - MBCSV Dereg Encode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_TRACK_ENC_EDU_ERROR, "MQD - MBCSV Track Encode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_USEREVT_ENC_EDU_ERROR, "MQD - MBCSV Userevt Encode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_NDSTAT_ENC_EDU_ERROR, "MQD - MBCSV Ndstat Encode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_NDTMREXP_ENC_EDU_ERROR, "MQD - MBCSV Ndtimer Expiry Encode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_REG_DEC_EDU_ERROR, "MQD - MBCSV Reg Decode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_DEREG_DEC_EDU_ERROR, "MQD - MBCSV Dereg Decode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_TRACK_DEC_EDU_ERROR, "MQD - MBCSV Track Decode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_USEREVT_DEC_EDU_ERROR, "MQD - MBCSV Userevt Decode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_NDSTAT_DEC_EDU_ERROR, "MQD - MBCSV Ndstat Decode EDU Error"},
	{MQD_RED_MBCSV_ASYNCUPDATE_NDTMREXP_DEC_EDU_ERROR, "MQD - MBCSV Ndtimer Expiry Decode EDU Error"},
	{MQD_RED_STANDBY_REG_FAILED, "MQD - ASAPi_REG_INFO processing at Standby failed"},
	{MQD_RED_STANDBY_DEREG_FAILED, "MQD - ASAPi_DEREG_INFO processing at Standby failed"},
	{MQD_RED_STANDBY_TRACK_FAILED, "MQD - ASAPi_TRACK_INFO processing at Standby failed"},
	{MQD_RED_STANDBY_USEREVT_FAILED, "MQD - MQD_A2S_USEREVT_INFO processing at Standby failed"},
	{MQD_RED_STANDBY_NDSTATINFO_FAILED, "MQD - MQD_A2S_ND_STAT_INFO processing at Standby failed"},
	{MQD_RED_STANDBY_NDTMREXP_FAILED, "MQD - MQD_A2S_ND_TIMER_EXP_INFO processing at Standby failed"},
	{MQD_RED_STANDBY_PROCESSING_FAILED, "MQD - Processing at Standby is failed"},
	{MQD_RED_STANDBY_PROCESS_REG_SUCCESS, "MQD - Standby MQD processing for REG message Success"},
	{MQD_RED_STANDBY_QUEUE_NODE_NOT_PRESENT, "MQD - Standby Processing Queue with the given name is not present"},
	{MQD_RED_STANDBY_PROCESS_DEREG_SUCCESS, "MQD - Standby Processing for DEREG message Success"},
	{MQD_RED_STANDBY_PROCESS_DEREG_FAILURE, "MQD - Standby Processing for DEREG message Failed"},
	{MQD_RED_STANDBY_PROCESS_TRACK_SUCCESS, "MQD - Standby Processing for TRACK message Success"},
	{MQD_RED_STANDBY_PROCESS_TRACK_FAILURE, "MQD - Standby Processing for TRACK message Failed"},
	{MQD_RED_STANDBY_PROCESS_USEREVT_SUCCESS, "MQD - Standby Processing for USEREVT message success"},
	{MQD_RED_STANDBY_COLD_SYNC_RESP_DECODE_SUCCESS,
	 "MQD - Standby: Decode call back of Cold Sync Response success"},
	{MQD_RED_STANDBY_COLD_SYNC_RESP_DECODE_FAILURE, "MQD - Standby: Decode call back of Cold Sync Response Failed"},
	{MQD_RED_ACTIVE_COLD_SYNC_RESP_ENCODE_SUCCESS, "MQD - Active: Encode call back of Cold Sync Response success"},
	{MQD_RED_ACTIVE_COLD_SYNC_RESP_ENCODE_FAILURE, "MQD - Active: Encode call back of Cold Sync Response Failed"},
	{MQD_RED_STANDBY_WARM_SYNC_RESP_DECODE_SUCCESS,
	 "MQD - Standby: Decode call back of Warm Sync Response success"},
	{MQD_RED_STANDBY_WARM_SYNC_RESP_DECODE_FAILURE, "MQD - Standby: Decode call back of Warm Sync Response Failed"},
	{MQD_RED_ACTIVE_WARM_SYNC_RESP_ENCODE_SUCCESS, "MQD - Active: Encode call back of Warm Sync Response success"},
	{MQD_RED_ACTIVE_WARM_SYNC_RESP_ENCODE_FAILURE, "MQD - Active: Encode call back of Warm Sync Response Failed"},
	{MQD_RED_ACTIVE_DATA_RESP_ENCODE_SUCCESS, "MQD - Active: Encode call back of Data Response Success"},
	{MQD_RED_ACTIVE_DATA_RESP_ENCODE_FAILURE, "MQD - Active: Encode call back of Data Response Failed"},
	{MQD_RED_STANDBY_DATA_RESP_DECODE_SUCCESS, "MQD - Standby: Decode call back of Data Response Success"},
	{MQD_RED_STANDBY_DATA_RESP_DECODE_FAILURE, "MQD - Standby: Decode call back of Data Response Failed"},
	{MQD_AMF_HEALTH_CHECK_START_SUCCESS, "MQD - Health check Success"},
	{MQD_AMF_HEALTH_CHECK_START_FAILED, "MQD - Health check Failed"},
	{MQD_MDS_INSTALL_FAILED, "MQD - MDS install failed "},
	{MQD_MDS_SUBSCRIPTION_FAILED, " MQD - MDS Subscription Failed"},
	{MQD_VDS_CREATE_FAILED, "MQD - VDS Create Failed"},
	{MQD_MDS_UNINSTALL_FAILED, "MQD - MDS Uninstall Failed"},
	{MQD_VDEST_DESTROY_FAILED, "MQD - VDEST Destroy Failed"},
	{MQD_MDS_ENCODE_FAILED, "MQD - MDS Encode is Failed at EDU"},
	{MQD_MDS_DECODE_FAILED, "MQD - MDS Decode is Failed at EDU"},
	{MQD_MDS_RCV_SEND_FAILED, "MQD - MDS Receive Send to Mail box Failed "},
	{MQD_MDS_SVC_EVT_MQA_DOWN_EVT_SEND_FAILED, "MQD - MDS SVC EVT MQA down event sending to the mail box failed"},
	{MQD_MDS_SVC_EVT_MQND_DOWN_EVT_SEND_FAILED, "MQD - MDS SVC EVT MQND down event sending to the mail box failed"},
	{MQD_MDS_SVC_EVT_MQND_UP_EVT_SEND_FAILED, "MQD - MDS SVC EVT MQND up event sending to the mail box failed"},
	{MQD_MDS_SEND_API_FAILED, "MQD - MDS Send Api Failed"},
	{MQD_MDS_QUISCED_EVT_SEND_FAILED, "MQD - MDS Quisced Send Event to mailbox failed"},
	{MQD_VDEST_CHG_ROLE_FAILED, " MQD - In CSI SET Callback the vdest changerole failed"},
	{MQD_MDS_MSG_COMP_EVT_SEND_FAILED,
	 "MQD - In CSI SET Callback the message of type COMP sending to the mailbox failed"},
	{MQD_QUISCED_VDEST_CHGROLE_FAILED,
	 "MQD - During processing of Quisced state, VDEST Role change to Queisced Failed"},
	{MQD_QUISCED_VDEST_CHGROLE_SUCCESS,
	 "MQD - During processing of Quisced state, VDEST Role change to Queisced Successfully"},
	{MQD_CSI_SET_ROLE, "MQD - CSI SET Called with HaState as"},
	{MQD_CSI_REMOVE_CALLBK_CHGROLE_FAILED, "MQD - CSI Remove During Role change to Standby Failed"},
	{MQD_CSI_REMOVE_CALLBK_SUCCESSFULL, "MQD - CSI Remove During Role change to Standby Successfull"},
	{MQD_REG_HDLR_DB_UPDATE_FAILED, "MQD - Register DB Update processing Failed"},
	{MQD_REG_HDLR_DB_UPDATE_SUCCESS, "MQD - Register DB Update processing Success"},
	{MQD_DEREG_HDLR_DB_UPDATE_FAILED, "MQD - Deregister DB Update Processing Failed"},
	{MQD_DEREG_HDLR_DB_UPDATE_SUCCESS, "MQD - Deregister DB Update Processing Success"},
	{MQD_GROUP_REMOVE_QUEUE_SUCCESS, "MQD - Removing a queue from Queue Group Success"},
	{MQD_GROUP_DELETE_SUCCESS, "MQD - Deleting the Queue Group Success"},
	{MQD_QUEUE_DELETE_SUCCESS, "MQD - Destroying the Queue Success(Unlink or Closecase)"},
	{MQD_GROUP_TRACK_DB_UPDATE_SUCCESS, "MQD - Track DB Update is Success"},
	{MQD_GROUP_TRACK_DB_UPDATE_FAILURE, "MQD - Track DB Update is Failed"},
	{MQD_GROUP_TRACKSTOP_DB_UPDATE_SUCCESS, "MQD - TrackStop DB Update is Success"},
	{MQD_GROUP_TRACKSTOP_DB_UPDATE_FAILURE, "MQD - Trackstop DB Update is Failed"},
	{MQD_REG_DB_UPD_ERR_EXIST, "MQD - The queue is already member of the queue group"},
	{MQD_REG_DB_QUEUE_UPDATE_SUCCESS, "MQD - Queue updation at the MQD is Successfull"},
	{MQD_REG_DB_QUEUE_CREATE_SUCCESS, "MQD - Queue Creation at the MQD is Successfull"},
	{MQD_REG_DB_QUEUE_CREATE_FAILED, "MQD - Queue Creation at the MQD is Failed"},
	{MQD_REG_DB_QUEUE_GROUP_CREATE_SUCCESS, "MQD - Queue Group Creation at the MQD is Successfull"},
	{MQD_REG_DB_QUEUE_GROUP_CREATE_FAILED, "MQD - Queue Group Creation at the MQD is Failed"},
	{MQD_EVT_QUISCED_PROCESS_SUCCESS, "MQD - Quisced Processing at MQD is successfull"},
	{MQD_EVT_QUISCED_PROCESS_MBCSVCHG_ROLE_FAILURE, "MQD - Quiesced Processing at MQD ,MBCSV Changerole failed "},
	{MQD_EVT_UNSOLICITED_QUISCED_ACK, "MQD - Received Unsolicited Quisced Ack at MQD"},
	{MQD_REG_DB_GRP_INSERT_SUCCESS, "MQD - Group Insert is Successfull"},
	{MQD_NRESOLV_HDLR_DB_ACCESS_FAILED, "MQD - Nameresolvehandler Database access not found"},
	{MQD_DB_UPD_MQND_DOWN, "MQD - The corresponding MQND is down "},
	{MQD_ASAPi_QUEUE_MAKE_SUCCESS, "MQD - The function queue_make is able to make the Queue list for update"},
	{MQD_ASAPi_QUEUE_MAKE_FAILED, "MQD - The function queue_make is able to make the Queue list for update"},
	{MQD_TMR_EXPIRED, "MQD - The Timer expired with node id "},
	{MQD_TMR_START, "MQD - The timer started with the duration as "},
	{MQD_TMR_STOPPED, "MQD - The timer stopped "},
	{MQD_MSG_FRMT_VER_INVALID, "MQD - Message Format version invalid"},
	{0, 0},
};

/*****************************************************************************\
 Build up the canned constant strings for the ASCII SPEC
\******************************************************************************/
NCSFL_SET mqd_str_set[] = {
	{MQD_FC_HDLN, 0, (NCSFL_STR *)mqd_hdln_set},
	{0, 0, 0}
};

NCSFL_FMAT mqd_fmat_set[] = {
/*    {MQD_LID_HDLN, NCSFL_TYPE_TILCL, "MQSv %s : %s : %u:%s:%u\n"},*/
	{MQD_LID_HDLN, NCSFL_TYPE_TCLIL, "MQSv %s %14s:%5lu:%s:%lu\n"},
	{MQD_LID_GENLOG, NCSFL_TYPE_TC, "%s %s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC mqd_ascii_spec = {
	NCS_SERVICE_ID_MQD,	/* MQD subsystem */
	MQSV_LOG_VERSION,	/* MQD (MQSv-MQD) revision ID */
	"mqd",			/* MQD opening Banner in log */
	(NCSFL_SET *)mqd_str_set,	/* MQD const strings referenced by index */
	(NCSFL_FMAT *)mqd_fmat_set,	/* MQD string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/****************************************************************************\
  PROCEDURE NAME : mqd_log_ascii_reg
 
  DESCRIPTION    : This is the function which registers the MQD's logging
                   ascii set with the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uint32_t mqd_log_ascii_reg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;
	uint32_t rc = NCSCC_RC_SUCCESS;

	memset(&arg, 0, sizeof(arg));

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &mqd_ascii_spec;

	/* Regiser MQD Canned strings with DTSv */
	rc = ncs_dtsv_ascii_spec_api(&arg);
	return rc;
}

/****************************************************************************\
  PROCEDURE NAME : mqd_log_ascii_dereg
 
  DESCRIPTION    : This is the function which deregisters the MQD's logging
                   ascii set from the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
void mqd_log_ascii_dereg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_MQD;
	arg.info.dereg_ascii_spec.version = MQSV_LOG_VERSION;

	/* Deregister MQD Canned strings from DTSv */
	ncs_dtsv_ascii_spec_api(&arg);
	return;
}

/****************************************************************************
 * Name          : mqsv_log_str_lib_req
 *
 * Description   : Library entry point for mqsv log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t mqsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = mqa_log_ascii_reg();
		res = mqnd_log_ascii_reg();
		res = mqd_log_ascii_reg();
		break;

	case NCS_LIB_REQ_DESTROY:
		mqa_log_ascii_dereg();
		mqnd_log_ascii_dereg();
		mqd_log_ascii_dereg();
		break;

	default:
		break;
	}
	return (res);
}

#endif   /* (NCS_DTS == 1) */
