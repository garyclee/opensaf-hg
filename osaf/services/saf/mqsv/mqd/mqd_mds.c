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
..............................................................................

..............................................................................

  DESCRIPTION: This file contains MQD's MDS routines:
   
   mqd_mds_init.................Register routine for registering MQD
   mqd_mds_shut.................Unregister routine for unregistering MQD
   mqd_mds_vdest_create.........Routine to create MDS vdest
   mqd_mds_vdest_destroy........Routine to destroy MDS vdest
   mqd_mds_callback.............MQD callback handler
   mqd_mds_cpy..................Routine for MDS copy message
   mqd_mds_enc..................Routine to encode message
   mqd_mds_dec..................Routine to decode message
   mqd_mds_rcv..................Routine to handle received message
   mqd_mds_svc_evt..............Routine to process service events
   mqd_mds_quiesced_process.....Routine to handle Quiesced State
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#include "mqd.h"

static uns32 mqd_mds_vdest_create(MQD_CB *);
static uns32 mqd_mds_vdest_destroy(MQD_CB *);
static uns32 mqd_mds_callback(NCSMDS_CALLBACK_INFO *);
static uns32 mqd_mds_cpy(MQD_CB *, MDS_CALLBACK_COPY_INFO *);
static uns32 mqd_mds_enc(MQD_CB *, MDS_CALLBACK_ENC_INFO *);
static uns32 mqd_mds_dec(MQD_CB *, MDS_CALLBACK_DEC_INFO *);
static uns32 mqd_mds_rcv(MQD_CB *, MDS_CALLBACK_RECEIVE_INFO *);
static void mqd_mds_svc_evt(MQD_CB *, MDS_CALLBACK_SVC_EVENT_INFO *);
static uns32 mqd_mds_quiesced_process(MQD_CB *pMqd);

MSG_FRMT_VER mqd_mqa_msg_fmt_table[MQD_WRT_MQA_SUBPART_VER_RANGE] = { 0, 2 };	/*With version 1 it is not backward compatible */
MSG_FRMT_VER mqd_mqnd_msg_fmt_table[MQD_WRT_MQND_SUBPART_VER_RANGE] = { 0, 2 };	/*With version 1 it is not backward compatible */

/****************************************************************************
   PROCEDURE NAME :  mqd_mds_callback

   DESCRIPTION    :  Call back function provided to MDS. MDS will call this
                     function for enc/dec/cpy/rcv/svc_evt operations.                   

   ARGUMENTS      :  NCSMDS_CALLBACK_INFO *callbk: call back info.

   RETURNS        :  SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
******************************************************************************/
static uns32 mqd_mds_callback(NCSMDS_CALLBACK_INFO *callbk)
{
	uns32 rc = NCSCC_RC_SUCCESS, mqd_hdl = 0;
	MQD_CB *pMqd = 0;

	if (!callbk) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	mqd_hdl = (uns32)callbk->i_yr_svc_hdl;
	pMqd = ncshm_take_hdl(NCS_SERVICE_ID_MQD, mqd_hdl);
	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	switch (callbk->i_op) {
	case MDS_CALLBACK_COPY:
		rc = mqd_mds_cpy(pMqd, &callbk->info.cpy);
		break;

	case MDS_CALLBACK_ENC:
		rc = mqd_mds_enc(pMqd, &callbk->info.enc);
		break;

	case MDS_CALLBACK_ENC_FLAT:
		rc = mqd_mds_enc(pMqd, &callbk->info.enc_flat);
		break;

	case MDS_CALLBACK_DEC:
		rc = mqd_mds_dec(pMqd, &callbk->info.dec);
		break;

	case MDS_CALLBACK_DEC_FLAT:
		rc = mqd_mds_dec(pMqd, &callbk->info.dec_flat);
		break;

	case MDS_CALLBACK_RECEIVE:
		rc = mqd_mds_rcv(pMqd, &callbk->info.receive);
		break;

	case MDS_CALLBACK_SVC_EVENT:
		mqd_mds_svc_evt(pMqd, &callbk->info.svc_evt);
		break;

	case MDS_CALLBACK_QUIESCED_ACK:
		rc = mqd_mds_quiesced_process(pMqd);
		break;

	default:
		rc = NCSCC_RC_FAILURE;
		break;
	}

	ncshm_give_hdl(pMqd->hdl);
	return rc;
}	/* End of mqd_mds_callback() */

/****************************************************************************
 PROCEDURE NAME : mqd_mds_init

 DESCRIPTION    : This function intializes the MDS procedures for MQD 

 ARGUMENTS      : pMqd : MQD Control Block pointer.

 RETURNS        : NCSCC_RC_SUCCESS/Error Code.
*****************************************************************************/
uns32 mqd_mds_init(MQD_CB *pMqd)
{
	NCSMDS_INFO arg;
	MDS_SVC_ID svc_id[] = { NCSMDS_SVC_ID_MQND, NCSMDS_SVC_ID_MQA };
	uns32 rc = NCSCC_RC_SUCCESS;

	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	/* Create the virtual Destination for MQD */
	rc = mqd_mds_vdest_create(pMqd);
	if (NCSCC_RC_SUCCESS != rc) {
		return rc;
	}

	/* Install your service into MDS */
	memset(&arg, 0, sizeof(arg));
	arg.i_mds_hdl = pMqd->my_mds_hdl;
	arg.i_svc_id = NCSMDS_SVC_ID_MQD;
	arg.i_op = MDS_INSTALL;

	arg.info.svc_install.i_yr_svc_hdl = pMqd->hdl;
	arg.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
	arg.info.svc_install.i_svc_cb = mqd_mds_callback;
	arg.info.svc_install.i_mds_q_ownership = FALSE;
	arg.info.svc_install.i_mds_svc_pvt_ver = MQD_PVT_SUBPART_VERSION;

	/* Bind with MDS */
	rc = ncsmds_api(&arg);
	if (NCSCC_RC_SUCCESS != rc) {
		m_LOG_MQSV_D(MQD_MDS_INSTALL_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		mqd_mds_vdest_destroy(pMqd);
		return rc;
	}

	/* MQD is subscribing for MQND MDS service */
	memset(&arg, 0, sizeof(arg));
	arg.i_mds_hdl = pMqd->my_mds_hdl;
	arg.i_svc_id = NCSMDS_SVC_ID_MQD;
	arg.i_op = MDS_SUBSCRIBE;
	arg.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	arg.info.svc_subscribe.i_num_svcs = sizeof(svc_id) / sizeof(svc_id[0]);
	arg.info.svc_subscribe.i_svc_ids = svc_id;

	/* Subscribe for events */
	rc = ncsmds_api(&arg);
	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_D(MQD_MDS_SUBSCRIPTION_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		mqd_mds_shut(pMqd);
		return rc;
	}
	return rc;		/*NCSCC_RC_SUCCESS; */
}	/* End of mqd_mds_init() */

/****************************************************************************\
 PROCEDURE NAME : mqd_mds_vdest_create

 DESCRIPTION    : This function Creates the Virtual destination for MQD

 ARGUMENTS      : pMqd : MQD control Block pointer.

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
\*****************************************************************************/
static uns32 mqd_mds_vdest_create(MQD_CB *pMqd)
{
	NCSVDA_INFO arg;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	pMqd->my_dest = MQD_VDEST_ID;

	memset(&arg, 0, sizeof(arg));

	arg.req = NCSVDA_VDEST_CREATE;
	arg.info.vdest_create.i_persistent = FALSE;
	arg.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
	arg.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
	arg.info.vdest_create.info.specified.i_vdest = pMqd->my_dest;

	/* Create VDEST */
	rc = ncsvda_api(&arg);
	if (NCSCC_RC_SUCCESS != rc) {
		m_LOG_MQSV_D(MQD_VDS_CREATE_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return rc;
	}

	pMqd->my_mds_hdl = arg.info.vdest_create.o_mds_pwe1_hdl;

	return rc;
}	/* End of mqd_mds_vdest_create() */

/****************************************************************************
 PROCEDURE NAME : mqd_mds_shut

 DESCRIPTION    : This function shuts the MDS for MQD 

 ARGUMENTS      : pMqd : MQD Control Block pointer.

 RETURNS        : NCSCC_RC_SUCCESS/Error Code.
*****************************************************************************/
uns32 mqd_mds_shut(MQD_CB *pMqd)
{
	NCSMDS_INFO arg;
	uns32 rc;

	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	/* Un-install your service into MDS */
	memset(&arg, 0, sizeof(arg));

	arg.i_mds_hdl = pMqd->my_mds_hdl;
	arg.i_svc_id = NCSMDS_SVC_ID_MQD;
	arg.i_op = MDS_UNINSTALL;

	rc = ncsmds_api(&arg);
	if (NCSCC_RC_SUCCESS != rc) {
		m_LOG_MQSV_D(MQD_MDS_UNINSTALL_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return rc;
	}
	/* Destroy the vertual Destination of MQD */
	rc = mqd_mds_vdest_destroy(pMqd);
	return rc;
}	/* End of mqd_mds_shut() */

/****************************************************************************\
 PROCEDURE NAME : mqd_mds_vdest_destroy

 DESCRIPTION    : This function deletes the Virtual destination for MQD

 ARGUMENTS      : pMqd : MQD control Block pointer.

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
\*****************************************************************************/
static uns32 mqd_mds_vdest_destroy(MQD_CB *pMqd)
{
	NCSVDA_INFO arg;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	memset(&arg, 0, sizeof(NCSVDA_INFO));

	arg.req = NCSVDA_VDEST_DESTROY;
	arg.info.vdest_destroy.i_vdest = pMqd->my_dest;
	arg.info.vdest_destroy.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
	arg.info.vdest_destroy.i_make_vdest_non_persistent = FALSE;
	rc = ncsvda_api(&arg);
	if (rc != NCSCC_RC_SUCCESS)
		m_LOG_MQSV_D(MQD_VDEST_DESTROY_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);

	return rc;
}	/* End of mqd_mds_vdest_destroy() */

/****************************************************************************\
 PROCEDURE NAME : mqd_mds_cpy

 DESCRIPTION    : This rountine is invoked when MQD sender & receiver both on 
                  the lies in the same process space.

 ARGUMENTS      : pMqd : MQD control Block.
                  cpy  : copy info.

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

 NOTES          : we assume that MQD & the user should not lie on the same 
                  process, so hitting this function is not possible, any way
                  for the compatibility we have written this callback 
                  function.
\*****************************************************************************/
static uns32 mqd_mds_cpy(MQD_CB *pMqd, MDS_CALLBACK_COPY_INFO *cpy)
{
	MQSV_EVT *pEvt = 0;
	uns32 rc = NCSCC_RC_SUCCESS;
	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	if (!cpy) {
		return NCSCC_RC_FAILURE;
	}
	pEvt = m_MMGR_ALLOC_MQSV_EVT(pMqd->my_svc_id);
	if (pEvt) {
		memcpy(pEvt, cpy->i_msg, sizeof(MQSV_EVT));
		if (MQSV_EVT_ASAPI == pEvt->type) {
			pEvt->msg.asapi->usg_cnt++;	/* Increment the use count */
		}
		cpy->o_cpy = pEvt;
		return NCSCC_RC_SUCCESS;
	} else {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
}	/* End of mqd_mds_cpy() */

/*****************************************************************************\
  PROCEDURE NAME : mqd_mds_enc

  DESCRIPTION    : This function encodes an events sent from MQD.

  ARGUMENTS      : pMqd : MQD control Block.
                   enc  : Info for encoding

  RETURNS        : NCSCC_RC_SUCCESS/Error Code.
\*****************************************************************************/
static uns32 mqd_mds_enc(MQD_CB *pMqd, MDS_CALLBACK_ENC_INFO *enc)
{
	MQSV_EVT *pEvt = 0;
	EDU_ERR err = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	if (!enc) {
		return NCSCC_RC_FAILURE;
	}

	/* Get the Msg Format version from the SERVICE_ID & RMT_SVC_PVT_SUBPART_VERSION */
	if (enc->i_to_svc_id == NCSMDS_SVC_ID_MQA) {
		enc->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(enc->i_rem_svc_pvt_ver,
							   MQD_WRT_MQA_SUBPART_VER_AT_MIN_MSG_FMT,
							   MQD_WRT_MQA_SUBPART_VER_AT_MAX_MSG_FMT,
							   mqd_mqa_msg_fmt_table);
	} else if (enc->i_to_svc_id == NCSMDS_SVC_ID_MQND) {
		enc->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(enc->i_rem_svc_pvt_ver,
							   MQD_WRT_MQND_SUBPART_VER_AT_MIN_MSG_FMT,
							   MQD_WRT_MQND_SUBPART_VER_AT_MAX_MSG_FMT,
							   mqd_mqnd_msg_fmt_table);
	}

	if (enc->o_msg_fmt_ver) {
		pEvt = (MQSV_EVT *)enc->i_msg;

		rc = (m_NCS_EDU_EXEC(&pMqd->edu_hdl, mqsv_edp_mqsv_evt, enc->io_uba, EDP_OP_TYPE_ENC, pEvt, &err));
		if (rc != NCSCC_RC_SUCCESS)
			m_LOG_MQSV_D(MQD_MDS_ENCODE_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
		return rc;
	} else {
		/* Drop The Message */
		m_LOG_MQSV_D(MQD_MSG_FRMT_VER_INVALID, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR,
			     enc->o_msg_fmt_ver, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
}	/* End of mqd_mds_enc() */

/*****************************************************************************\
  PROCEDURE NAME : mqd_mds_dec

  DESCRIPTION    : This function decodes a buffer containing an MQD events 
                   that was received from off card.

  ARGUMENTS      : pMqd : MQD control Block.
                   dec  : Info for decoding

  RETURNS        : NCSCC_RC_SUCCESS/Error Code.
\*****************************************************************************/
static uns32 mqd_mds_dec(MQD_CB *pMqd, MDS_CALLBACK_DEC_INFO *dec)
{
	MQSV_EVT *pEvt = 0;
	EDU_ERR err = 0;
	uns32 rc = NCSCC_RC_SUCCESS;
	NCS_BOOL is_valid_msg_fmt = FALSE;

	if (!pMqd) {
		rc = NCSCC_RC_FAILURE;
		m_LOG_MQSV_D(MQD_DONOT_EXIST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	if (!dec) {
		return NCSCC_RC_FAILURE;
	}

	if (dec->i_fr_svc_id == NCSMDS_SVC_ID_MQA) {
		is_valid_msg_fmt = m_NCS_MSG_FORMAT_IS_VALID(dec->i_msg_fmt_ver,
							     MQD_WRT_MQA_SUBPART_VER_AT_MIN_MSG_FMT,
							     MQD_WRT_MQA_SUBPART_VER_AT_MAX_MSG_FMT,
							     mqd_mqa_msg_fmt_table);
	} else if (dec->i_fr_svc_id == NCSMDS_SVC_ID_MQND) {
		is_valid_msg_fmt = m_NCS_MSG_FORMAT_IS_VALID(dec->i_msg_fmt_ver,
							     MQD_WRT_MQND_SUBPART_VER_AT_MIN_MSG_FMT,
							     MQD_WRT_MQND_SUBPART_VER_AT_MAX_MSG_FMT,
							     mqd_mqnd_msg_fmt_table);
	}

	if (is_valid_msg_fmt && (dec->i_msg_fmt_ver != 1)) {
		pEvt = m_MMGR_ALLOC_MQSV_EVT(pMqd->my_svc_id);
		if (!pEvt) {
			rc = NCSCC_RC_FAILURE;
			m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
			return NCSCC_RC_FAILURE;
		}

		memset(pEvt, 0, sizeof(MQSV_EVT));
		dec->o_msg = (NCSCONTEXT)pEvt;

		rc = m_NCS_EDU_EXEC(&pMqd->edu_hdl, mqsv_edp_mqsv_evt,
				    dec->io_uba, EDP_OP_TYPE_DEC, (MQSV_EVT **)&dec->o_msg, &err);
		if (NCSCC_RC_SUCCESS != rc) {
			m_LOG_MQSV_D(MQD_MDS_DECODE_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
			if ((MQSV_EVT_ASAPI == pEvt->type) && (pEvt->msg.asapi))
				asapi_msg_free(&pEvt->msg.asapi);
			m_MMGR_FREE_MQSV_EVT(pEvt, pMqd->my_svc_id);
		}
		return rc;
	} else {
		/* Drop The Message */
		m_LOG_MQSV_D(MQD_MSG_FRMT_VER_INVALID, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR,
			     is_valid_msg_fmt, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
}	/* End of mqd_mds_dec() */

/****************************************************************************\
 PROCEDURE NAME : mqd_mds_rcv

 DESCRIPTION    : MDS will call this function on receiving MQD messages.

 ARGUMENTS      : pMqd : MQD control Block
                  rcv  : Received information.

 RETURNS        : NCSCC_RC_SUCCESS/Error Code.
\*****************************************************************************/
static uns32 mqd_mds_rcv(MQD_CB *pMqd, MDS_CALLBACK_RECEIVE_INFO *rcv)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	MQSV_EVT *pEvt = (MQSV_EVT *)rcv->i_msg;

	pEvt->sinfo.ctxt = rcv->i_msg_ctxt;
	pEvt->sinfo.dest = rcv->i_fr_dest;
	pEvt->sinfo.to_svc = rcv->i_fr_svc_id;
	if (rcv->i_rsp_reqd) {
		pEvt->sinfo.stype = MDS_SENDTYPE_RSP;
	} else
		pEvt->sinfo.stype = 0;

	/* Put it in MQD's Event Queue */
	rc = m_MQD_EVT_SEND(&pMqd->mbx, pEvt, NCS_IPC_PRIORITY_NORMAL);
	if (NCSCC_RC_SUCCESS != rc) {
		m_LOG_MQSV_D(MQD_MDS_RCV_SEND_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		if ((MQSV_EVT_ASAPI == pEvt->type) && (pEvt->msg.asapi)) {
			asapi_msg_free(&pEvt->msg.asapi);
		}
		m_MMGR_FREE_MQSV_EVT(pEvt, pMqd->my_svc_id);
	}
	return rc;
}	/* End of mqd_mds_rcv() */

/****************************************************************************\
 PROCEDURE NAME : mqd_mds_svc_evt

 DESCRIPTION    : MQD is informed when MDS events occurr that he has subscribed to

 ARGUMENTS      : pMqd : MQD control Block
                  svc  : Svc evt info.

 RETURNS        : None
\*****************************************************************************/
static void mqd_mds_svc_evt(MQD_CB *pMqd, MDS_CALLBACK_SVC_EVENT_INFO *svc)
{
	MQSV_EVT *pNdEvent = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	switch (svc->i_change) {
	case NCSMDS_DOWN:
		{
			/* Need to clean up all the track information for the specific link 
			 * that gone or is down 
			 */
			MQSV_EVT *pEvt = 0;
			if ((svc->i_svc_id == NCSMDS_SVC_ID_MQA)) {
				pEvt = m_MMGR_ALLOC_MQSV_EVT(pMqd->my_svc_id);
				if (pEvt) {
					memset(pEvt, 0, sizeof(MQSV_EVT));
					pEvt->type = MQSV_EVT_MQD_CTRL;
					pEvt->msg.mqd_ctrl.type = MQD_MSG_USER;
					pEvt->msg.mqd_ctrl.info.user = svc->i_dest;

					/* Put it in MQD's Event Queue */
					rc = m_MQD_EVT_SEND(&pMqd->mbx, pEvt, NCS_IPC_PRIORITY_NORMAL);
					if (NCSCC_RC_SUCCESS != rc) {
						m_LOG_MQSV_D(MQD_MDS_SVC_EVT_MQA_DOWN_EVT_SEND_FAILED,
							     NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
							     __LINE__);
						m_MMGR_FREE_MQSV_EVT(pEvt, pMqd->my_svc_id);
					}
				} else {
					m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc,
						     __FILE__, __LINE__);
				}
			}
			if (svc->i_svc_id == NCSMDS_SVC_ID_MQND) {
				pNdEvent = m_MMGR_ALLOC_MQSV_EVT(pMqd->my_svc_id);
				if (pNdEvent) {
					memset(pNdEvent, 0, sizeof(MQSV_EVT));
					pNdEvent->type = MQSV_EVT_MQD_CTRL;
					pNdEvent->msg.mqd_ctrl.type = MQD_ND_STATUS_INFO_TYPE;
					pNdEvent->msg.mqd_ctrl.info.nd_info.dest = svc->i_dest;
					pNdEvent->msg.mqd_ctrl.info.nd_info.is_up = FALSE;

					m_GET_TIME_STAMP(pNdEvent->msg.mqd_ctrl.info.nd_info.event_time);
					/* Put it in MQD's Event Queue */
					rc = m_MQD_EVT_SEND(&pMqd->mbx, pNdEvent, NCS_IPC_PRIORITY_NORMAL);
					TRACE("MQND MDS DOWN EVENT POSTED");
					if (NCSCC_RC_SUCCESS != rc) {
						m_MMGR_FREE_MQSV_EVT(pNdEvent, pMqd->my_svc_id);
						m_LOG_MQSV_D(MQD_MDS_SVC_EVT_MQND_DOWN_EVT_SEND_FAILED,
							     NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
							     __LINE__);
					}
				} else {
					m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc,
						     __FILE__, __LINE__);
				}

			}
		}
		break;

	case NCSMDS_UP:
		{
			if (svc->i_svc_id == NCSMDS_SVC_ID_MQND) {
				uint16_t to_dest_slotid, o_msg_fmt_ver;
				to_dest_slotid = mqsv_get_phy_slot_id(svc->i_dest);

				o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(svc->i_rem_svc_pvt_ver,
								      MQD_WRT_MQND_SUBPART_VER_AT_MIN_MSG_FMT,
								      MQD_WRT_MQND_SUBPART_VER_AT_MAX_MSG_FMT,
								      mqd_mqnd_msg_fmt_table);

				if (!o_msg_fmt_ver)
					/*Log informing the existence of Non compatible MQD version, Slot id being logged */
					m_LOG_MQSV_D(MQD_MSG_FRMT_VER_INVALID, NCSFL_LC_MQSV_INIT,
						     NCSFL_SEV_ERROR, to_dest_slotid, __FILE__, __LINE__);

				pNdEvent = m_MMGR_ALLOC_MQSV_EVT(pMqd->my_svc_id);
				if (pNdEvent) {
					memset(pNdEvent, 0, sizeof(MQSV_EVT));
					pNdEvent->type = MQSV_EVT_MQD_CTRL;
					pNdEvent->msg.mqd_ctrl.type = MQD_ND_STATUS_INFO_TYPE;
					pNdEvent->msg.mqd_ctrl.info.nd_info.dest = svc->i_dest;
					pNdEvent->msg.mqd_ctrl.info.nd_info.is_up = TRUE;

					m_GET_TIME_STAMP(pNdEvent->msg.mqd_ctrl.info.nd_info.event_time);
					/* Put it in MQD's Event Queue */
					rc = m_MQD_EVT_SEND(&pMqd->mbx, pNdEvent, NCS_IPC_PRIORITY_NORMAL);
					TRACE("MQND MDS UP EVENT POSTED");
					if (NCSCC_RC_SUCCESS != rc) {
						m_LOG_MQSV_D(MQD_MDS_SVC_EVT_MQND_UP_EVT_SEND_FAILED,
							     NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
							     __LINE__);
						m_MMGR_FREE_MQSV_EVT(pNdEvent, pMqd->my_svc_id);
					}
				} else {
					m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc,
						     __FILE__, __LINE__);
				}
			}
		}
		break;
	default:
		break;
	}
	return;
}	/* End of mqd_mds_svc_evt() */

/****************************************************************************\
 PROCEDURE NAME : mqd_mds_quiesced_process

 DESCRIPTION    : MQD sending the event to it's mail box

 ARGUMENTS      : pMqd : MQD control Block

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 mqd_mds_quiesced_process(MQD_CB *pMqd)
{
	MQSV_EVT *pEvt = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (pMqd->is_quisced_set) {
		pEvt = m_MMGR_ALLOC_MQSV_EVT(pMqd->my_svc_id);
		if (!pEvt) {
			rc = NCSCC_RC_FAILURE;
			m_LOG_MQSV_D(MQD_MEMORY_ALLOC_FAIL, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
			return NCSCC_RC_FAILURE;
		}
		memset(pEvt, 0, sizeof(MQSV_EVT));
		pEvt->type = MQSV_EVT_MQD_CTRL;
		pEvt->msg.mqd_ctrl.type = MQD_QUISCED_STATE_INFO_TYPE;
		pEvt->msg.mqd_ctrl.info.quisced_info.invocation = pMqd->invocation;

		/* Put it in MQD's Event Queue */
		rc = m_MQD_EVT_SEND(&pMqd->mbx, pEvt, NCS_IPC_PRIORITY_NORMAL);
		if (rc != NCSCC_RC_SUCCESS) {
			m_LOG_MQSV_D(MQD_MDS_QUISCED_EVT_SEND_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
			m_MMGR_FREE_MQSV_EVT(pEvt, pMqd->my_svc_id);
		}
	}
	return rc;
}

/****************************************************************************
 * Name          : mqd_mds_send_rsp
 *
 * Description   : Send the Response to Sync Requests
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         :
 *****************************************************************************/

uns32 mqd_mds_send_rsp(MQD_CB *cb, MQSV_SEND_INFO *s_info, MQSV_EVT *evt)
{
	NCSMDS_INFO mds_info;
	uns32 rc;

	memset(&mds_info, 0, sizeof(NCSMDS_INFO));
	mds_info.i_mds_hdl = cb->my_mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_MQD;
	mds_info.i_op = MDS_SEND;

	/* fill the send structure */
	mds_info.info.svc_send.i_msg = (NCSCONTEXT)evt;
	mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;

	mds_info.info.svc_send.i_to_svc = s_info->to_svc;
	mds_info.info.svc_send.i_sendtype = s_info->stype;
	mds_info.info.svc_send.info.rsp.i_msg_ctxt = s_info->ctxt;
	mds_info.info.svc_send.info.rsp.i_sender_dest = s_info->dest;

	/* send the message */
	rc = ncsmds_api(&mds_info);
	if (rc != NCSCC_RC_SUCCESS) {
		/*  m_LOG_MQSV_ND(MQD_MDS_SND_RSP_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__); */
	}
	return rc;
}
