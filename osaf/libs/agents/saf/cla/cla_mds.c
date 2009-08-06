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

  DESCRIPTION:

  This file contains routines used by CLA library for MDS interaction.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "cla.h"

/* static function declarations */

static uns32 cla_mds_rcv(CLA_CB *, MDS_CALLBACK_RECEIVE_INFO *);

static uns32 cla_mds_svc_evt(CLA_CB *, MDS_CALLBACK_SVC_EVENT_INFO *);

static uns32 cla_mds_enc_flat(CLA_CB *, MDS_CALLBACK_ENC_INFO *);

static uns32 cla_mds_dec_flat(CLA_CB *, MDS_CALLBACK_DEC_INFO *);

static uns32 cla_mds_param_get(CLA_CB *);

static uns32 cla_mds_msg_normal_send(CLA_CB *, NCSMDS_INFO *, AVSV_NDA_CLA_MSG *);

static uns32 cla_mds_msg_syn_send(CLA_CB *, NCSMDS_INFO *, AVSV_NDA_CLA_MSG *, uns32 timeout);

static uns32 cla_mds_enc(CLA_CB *, MDS_CALLBACK_ENC_INFO *);

static uns32 cla_mds_dec(CLA_CB *, MDS_CALLBACK_DEC_INFO *);

const MDS_CLIENT_MSG_FORMAT_VER cla_avnd_msg_fmt_map_table[CLA_AVND_SUBPART_VER_MAX] = { AVSV_AVND_CLA_MSG_FMT_VER_1 };

/****************************************************************************
  Name          : cla_mds_reg
 
  Description   : This routine registers the CLA service with MDS. It does 
                  the following:
                  a) Gets the MDS handle & CLA MDS address
                  b) installs CLA service with MDS
                  c) Subscribes to MDS events
 
  Arguments     : cb - ptr to the CLA control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_reg(CLA_CB *cb)
{
	NCSMDS_INFO mds_info;
	MDS_SVC_ID svc_id;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* get the mds-hdl & cla mds address */
	rc = cla_mds_param_get(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		m_CLA_LOG_MDS(AVSV_LOG_MDS_PRM_GET, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
		goto done;
	}

	/* fill common fields */
	memset(&mds_info, 0, sizeof(NCSMDS_INFO));
	mds_info.i_mds_hdl = cb->avnd_intf.mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_CLA;

   /*** install cla service with mds ***/
	mds_info.i_op = MDS_INSTALL;
	mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	mds_info.info.svc_install.i_svc_cb = cla_mds_cbk;
	mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)cb->cb_hdl;
	mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_INTRANODE;
	mds_info.info.svc_install.i_mds_svc_pvt_ver = CLA_MDS_SUB_PART_VERSION;

	rc = ncsmds_api(&mds_info);
	if (NCSCC_RC_SUCCESS != rc) {
		m_CLA_LOG_MDS(AVSV_LOG_MDS_INSTALL, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
		goto done;
	}

   /*** subscribe to avnd mds event ***/
	mds_info.i_op = MDS_SUBSCRIBE;
	mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_INTRANODE;
	mds_info.info.svc_subscribe.i_svc_ids = &svc_id;
	mds_info.info.svc_subscribe.i_num_svcs = 1;
	svc_id = NCSMDS_SVC_ID_AVND;
	rc = ncsmds_api(&mds_info);
	if (NCSCC_RC_SUCCESS != rc) {
		m_CLA_LOG_MDS(AVSV_LOG_MDS_SUBSCRIBE, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
		goto done;
	}

 done:
	if (NCSCC_RC_SUCCESS != rc)
		rc = cla_mds_unreg(cb);

	return rc;
}

/****************************************************************************
  Name          : cla_mds_unreg
 
  Description   : This routine unregisters the CLA service from MDS.
 
  Arguments     : cb - ptr to the CLA control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_unreg(CLA_CB *cb)
{
	NCSMDS_INFO mds_info;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&mds_info, 0, sizeof(NCSMDS_INFO));

	mds_info.i_mds_hdl = cb->avnd_intf.mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_CLA;
	mds_info.i_op = MDS_UNINSTALL;

	rc = ncsmds_api(&mds_info);

	return rc;
}

/****************************************************************************
  Name          : cla_mds_cbk
 
  Description   : This routine is a callback routine that is provided to MDS.
                  MDS calls this routine for encode, decode, copy, receive &
                  service event notification operations.
 
  Arguments     : info - ptr to the MDS callback info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_cbk(NCSMDS_CALLBACK_INFO *info)
{
	CLA_CB *cb = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (!info)
		goto done;

	/* retrieve cla cb */
	if (0 == (cb = (CLA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, (uns32)info->i_yr_svc_hdl))) {
		m_CLA_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		rc = NCSCC_RC_SUCCESS;
		goto done;
	}

	switch (info->i_op) {
	case MDS_CALLBACK_RECEIVE:
		{
			/* acquire cb write lock */
			m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

			/* process the received msg */
			rc = cla_mds_rcv(cb, &info->info.receive);

			/* release cb write lock */
			m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);

			if (NCSCC_RC_SUCCESS != rc)
				m_CLA_LOG_MDS(AVSV_LOG_MDS_RCV_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
			else
				m_CLA_LOG_MDS(AVSV_LOG_MDS_RCV_CBK, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);
		}
		break;

	case MDS_CALLBACK_COPY:
		m_AVSV_ASSERT(0);	/* CLA never resides with AvND */

	case MDS_CALLBACK_SVC_EVENT:
		{
			rc = cla_mds_svc_evt(cb, &info->info.svc_evt);

			if (NCSCC_RC_SUCCESS != rc)
				m_CLA_LOG_MDS(AVSV_LOG_MDS_SVEVT_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
			else
				m_CLA_LOG_MDS(AVSV_LOG_MDS_SVEVT_CBK, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);
		}
		break;

	case MDS_CALLBACK_ENC_FLAT:
		{
			info->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.enc_flat.i_rem_svc_pvt_ver,
									     CLA_AVND_SUBPART_VER_MIN,
									     CLA_AVND_SUBPART_VER_MAX,
									     cla_avnd_msg_fmt_map_table);

			if (info->info.enc_flat.o_msg_fmt_ver < AVSV_AVND_CLA_MSG_FMT_VER_1) {
				/* log the problem */
				m_CLA_LOG_MDS(AVSV_LOG_MDS_FLAT_ENC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);

				return NCSCC_RC_FAILURE;
			}

			rc = cla_mds_enc_flat(cb, &info->info.enc_flat);

			if (NCSCC_RC_SUCCESS != rc)
				m_CLA_LOG_MDS(AVSV_LOG_MDS_FLAT_ENC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
			else
				m_CLA_LOG_MDS(AVSV_LOG_MDS_FLAT_ENC_CBK, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);
		}
		break;

	case MDS_CALLBACK_DEC_FLAT:
		{
			if (!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec_flat.i_msg_fmt_ver,
						       CLA_AVND_SUBPART_VER_MIN,
						       CLA_AVND_SUBPART_VER_MAX, cla_avnd_msg_fmt_map_table)) {
				/* log the problem */
				m_CLA_LOG_MDS(AVSV_LOG_MDS_DEC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);

				return NCSCC_RC_FAILURE;
			}

			rc = cla_mds_dec_flat(cb, &info->info.dec_flat);

			if (NCSCC_RC_SUCCESS != rc)
				m_CLA_LOG_MDS(AVSV_LOG_MDS_FLAT_DEC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
			else
				m_CLA_LOG_MDS(AVSV_LOG_MDS_FLAT_DEC_CBK, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);
		}
		break;

	case MDS_CALLBACK_DEC:
		{
			if (!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
						       CLA_AVND_SUBPART_VER_MIN,
						       CLA_AVND_SUBPART_VER_MAX, cla_avnd_msg_fmt_map_table)) {
				/* log the problem */
				m_CLA_LOG_MDS(AVSV_LOG_MDS_DEC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);

				return NCSCC_RC_FAILURE;
			}

			rc = cla_mds_dec(cb, &info->info.dec);

			if (NCSCC_RC_SUCCESS != rc)
				m_CLA_LOG_MDS(AVSV_LOG_MDS_DEC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
			else
				m_CLA_LOG_MDS(AVSV_LOG_MDS_DEC_CBK, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);
		}
		break;

	case MDS_CALLBACK_ENC:
		{
			info->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
									     CLA_AVND_SUBPART_VER_MIN,
									     CLA_AVND_SUBPART_VER_MAX,
									     cla_avnd_msg_fmt_map_table);

			if (info->info.enc.o_msg_fmt_ver < AVSV_AVND_CLA_MSG_FMT_VER_1) {
				/* log the problem */
				m_CLA_LOG_MDS(AVSV_LOG_MDS_ENC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);

				return NCSCC_RC_FAILURE;
			}

			rc = cla_mds_enc(cb, &info->info.enc);

			if (NCSCC_RC_SUCCESS != rc)
				m_CLA_LOG_MDS(AVSV_LOG_MDS_ENC_CBK, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
			else
				m_CLA_LOG_MDS(AVSV_LOG_MDS_ENC_CBK, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);
		}
		break;

	default:
		m_AVSV_ASSERT(0);
	}

 done:
	/* return cla cb */
	if (cb)
		ncshm_give_hdl((uns32)info->i_yr_svc_hdl);

	return rc;
}

/****************************************************************************
  Name          : cla_mds_send
 
  Description   : This routine sends the CLA message to AvND. The send 
                  operation may be a 'normal' send or a synchronous call that 
                  blocks until the response is received from AvND.
 
  Arguments     : cb  - ptr to the CLA CB
                  i_msg - ptr to the CLA message
                  o_msg - double ptr to CLA message response
                  timeout - for sync messages Timeout duration in 10ms units
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/timeout
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_send(CLA_CB *cb, AVSV_NDA_CLA_MSG *i_msg, AVSV_NDA_CLA_MSG **o_msg, uns32 timeout)
{
	NCSMDS_INFO mds_info;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (!i_msg)
		return NCSCC_RC_FAILURE;	/* nothing to send */

	if ((cb->avnd_intf.avnd_up == FALSE) || (m_NCS_NODE_ID_FROM_MDS_DEST(cb->avnd_intf.avnd_mds_dest) == 0))
		return NCSCC_RC_FAILURE;

	memset(&mds_info, 0, sizeof(NCSMDS_INFO));

	mds_info.i_mds_hdl = cb->avnd_intf.mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_CLA;
	mds_info.i_op = MDS_SEND;

	if (o_msg) {
		/* synchronous blocking send */
		rc = cla_mds_msg_syn_send(cb, &mds_info, i_msg, timeout);
		if (NCSCC_RC_SUCCESS == rc) {
			/* retrieve the response */
			*o_msg = (AVSV_NDA_CLA_MSG *)mds_info.info.svc_send.info.sndrsp.o_rsp;
			mds_info.info.svc_send.info.sndrsp.o_rsp = 0;
		}
	} else
		/* just a 'normal' send */
		rc = cla_mds_msg_normal_send(cb, &mds_info, i_msg);

	if (NCSCC_RC_SUCCESS != rc)
		m_CLA_LOG_MDS(AVSV_LOG_MDS_SEND, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
	else
		m_CLA_LOG_MDS(AVSV_LOG_MDS_SEND, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

	return rc;
}

/****************************************************************************
  Name          : cla_mds_rcv
 
  Description   : This routine is invoked when CLA message is received from 
                  AvND.
 
  Arguments     : cb       - ptr to the CLA control block
                  rcv_info - ptr to the MDS receive info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_rcv(CLA_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
	AVSV_NDA_CLA_MSG *msg = (AVSV_NDA_CLA_MSG *)rcv_info->i_msg;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (!msg)
		return NCSCC_RC_FAILURE;

	/* process the message */
	rc = cla_avnd_msg_prc(cb, msg);

	/* free the message structure */
	if (msg)
		avsv_nda_cla_msg_free(msg);

	return rc;
}

/****************************************************************************
  Name          : cla_mds_svc_evt
 
  Description   : This routine is invoked to inform CLA of MDS events. CLA 
                  had subscribed to these events during MDS registration.
 
  Arguments     : cb       - ptr to the CLA control block
                  evt_info - ptr to the MDS event info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_svc_evt(CLA_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *evt_info)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* assign mds-dest values for AVD, AVND & CLA as per the MDS event */
	switch (evt_info->i_change) {
	case NCSMDS_UP:
		switch (evt_info->i_svc_id) {
		case NCSMDS_SVC_ID_CLA:
			/* I already know my mds dest. Nothing to be done */
			break;

		case NCSMDS_SVC_ID_AVND:
			cb->avnd_intf.avnd_mds_dest = evt_info->i_dest;
			cb->avnd_intf.avnd_up = TRUE;

			/* Protect FD_VALID_FLAG with lock */
			m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);
			if (m_CLA_FLAG_IS_FD_VALID(cb))
				/* write into the fd */
				m_NCS_SEL_OBJ_IND(cb->sel_obj);
			m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);
			break;

		default:
			break;
		}
		break;

	case NCSMDS_DOWN:
		switch (evt_info->i_svc_id) {
		case NCSMDS_SVC_ID_CLA:
			memset(&cb->avnd_intf.cla_mds_dest, 0, sizeof(MDS_DEST));
			/* Is there anything that needs to be done? Discuss TBD */
			break;

		case NCSMDS_SVC_ID_AVND:
			cb->avnd_intf.avnd_up = FALSE;
			memset(&cb->avnd_intf.avnd_mds_dest, 0, sizeof(MDS_DEST));
			/* Is there anything that needs to be done? Discuss TBD */
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return rc;
}

/****************************************************************************
  Name          : cla_mds_enc_flat
 
  Description   : This routine is invoked to encode AvND messages.
 
  Arguments     : cb       - ptr to the CLA control block
                  enc_info - ptr to the MDS encode info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : This routine also frees the message after encoding it in 
                  the userbuf.
******************************************************************************/
uns32 cla_mds_enc_flat(CLA_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{
	AVSV_NDA_CLA_MSG *msg;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* get the message ptr */
	msg = (AVSV_NDA_CLA_MSG *)enc_info->i_msg;

	switch (msg->type) {
	case AVSV_CLA_API_MSG:
		/* encode into userbuf */
		rc = ncs_encode_n_octets_in_uba(enc_info->io_uba, (uns8 *)msg, sizeof(AVSV_NDA_CLA_MSG));
		break;

	case AVSV_AVND_CLM_CBK_MSG:
	case AVSV_AVND_CLM_API_RESP_MSG:
	case AVSV_NDA_CLA_MSG_MAX:
	default:
		m_AVSV_ASSERT(0);
	}			/* switch */

	return rc;
}

/****************************************************************************
  Name          : cla_mds_dec_flat
 
  Description   : This routine is invoked to decode AvND messages.
 
  Arguments     : cb       - ptr to the CLA control block
                  rcv_info - ptr to the MDS decode info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_dec_flat(CLA_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
	AVSV_NDA_CLA_MSG *msg = 0;
	uns32 size, rc = NCSCC_RC_SUCCESS;

	/* allocate the msg */
	msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG;
	if (!msg) {
		rc = NCSCC_RC_FAILURE;
		goto err;
	}
	memset(msg, 0, sizeof(AVSV_NDA_CLA_MSG));

	/* decode the top level CLA msg contents */
	rc = ncs_decode_n_octets_from_uba(dec_info->io_uba, (uns8 *)msg, sizeof(AVSV_NDA_CLA_MSG));
	if (NCSCC_RC_SUCCESS != rc)
		goto err;

	/* decode individual cla msg (if not decoded above) */
	switch (msg->type) {
	case AVSV_AVND_CLM_CBK_MSG:
		{
			if (msg->info.cbk_info.type == AVSV_CLM_CBK_TRACK) {
				/* alloc notification buffer */
				msg->info.cbk_info.param.track.notify.notification = 0;
				size = (msg->info.cbk_info.param.track.notify.numberOfItems) *
				    (sizeof(SaClmClusterNotificationT));
				msg->info.cbk_info.param.track.notify.notification =
				    (SaClmClusterNotificationT *)m_MMGR_ALLOC_AVSV_CLA_DEFAULT_VAL(size);

				if (0 == msg->info.cbk_info.param.track.notify.notification) {
					rc = NCSCC_RC_FAILURE;
					goto err;
				}

				memset(msg->info.cbk_info.param.track.notify.notification, 0, size);

				/* decode cbk-info */
				rc = ncs_decode_n_octets_from_uba(dec_info->io_uba,
								  (uns8 *)msg->info.cbk_info.param.track.notify.
								  notification, size);

				if (NCSCC_RC_SUCCESS != rc)
					goto err;
			}
		}
		break;

	case AVSV_AVND_CLM_API_RESP_MSG:
		{
			if (msg->info.api_resp_info.type == AVSV_CLM_TRACK_START) {
				/* alloc notification buffer */
				msg->info.api_resp_info.param.track.notify = 0;
				size = (msg->info.api_resp_info.param.track.num) * (sizeof(SaClmClusterNotificationT));
				msg->info.api_resp_info.param.track.notify =
				    (SaClmClusterNotificationT *)m_MMGR_ALLOC_AVSV_CLA_DEFAULT_VAL(size);

				if (!msg->info.api_resp_info.param.track.notify) {
					rc = NCSCC_RC_FAILURE;
					goto err;
				}

				memset(msg->info.api_resp_info.param.track.notify, 0, size);

				/* decode cbk-info */
				rc = ncs_decode_n_octets_from_uba(dec_info->io_uba,
								  (uns8 *)msg->info.api_resp_info.param.track.notify,
								  size);

				if (NCSCC_RC_SUCCESS != rc)
					goto err;
			}
		}		/* already decoded above */
		break;

	case AVSV_CLA_API_MSG:
	default:
		m_AVSV_ASSERT(0);
	}			/* switch */

	/* decode successful */
	dec_info->o_msg = (NCSCONTEXT)msg;

	return rc;

 err:
	if (msg)
		avsv_nda_cla_msg_free(msg);
	dec_info->o_msg = 0;
	return rc;
}

/****************************************************************************
  Name          : cla_mds_msg_normal_send
 
  Description   : This routine sends the CLA message to AvND.
 
  Arguments     : cb  - ptr to the CLA CB
                  mds_info - ptr to MDS info
                  i_msg - ptr to the CLA message
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_msg_normal_send(CLA_CB *cb, NCSMDS_INFO *mds_info, AVSV_NDA_CLA_MSG *i_msg)
{
	MDS_SEND_INFO *send_info = &mds_info->info.svc_send;
	MDS_SENDTYPE_SND_INFO *send = &send_info->info.snd;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* populate the send info */
	send_info->i_msg = (NCSCONTEXT)i_msg;
	send_info->i_to_svc = NCSMDS_SVC_ID_AVND;
	send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;	/* Discuss the priority assignments TBD */
	send_info->i_sendtype = MDS_SENDTYPE_SND;
	send->i_to_dest = cb->avnd_intf.avnd_mds_dest;

	/* release the cb lock */
	if ((AVSV_CLA_API_MSG == i_msg->type) &&
	    ((AVSV_CLM_INITIALIZE == i_msg->info.api_info.type) || (AVSV_CLM_FINALIZE == i_msg->info.api_info.type)))
		m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);
	else
		m_NCS_UNLOCK(&cb->lock, NCS_LOCK_READ);

	/* send the message */
	rc = ncsmds_api(mds_info);

	/* reacquire the cb lock */
	if ((AVSV_CLA_API_MSG == i_msg->type) &&
	    ((AVSV_CLM_INITIALIZE == i_msg->info.api_info.type) || (AVSV_CLM_FINALIZE == i_msg->info.api_info.type)))
		m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);
	else
		m_NCS_LOCK(&cb->lock, NCS_LOCK_READ);

	return rc;
}

/****************************************************************************
  Name          : cla_mds_msg_syn_send
 
  Description   : This routine sends the CLA message to AvND & blocks for 
                  the response from AvND.
 
  Arguments     : cb  - ptr to the CLA CB
                  mds_info - ptr to MDS info
                  i_msg - ptr to the CLA message
                  timeout - for sync messages Timeout duration in 10ms units
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_mds_msg_syn_send(CLA_CB *cb, NCSMDS_INFO *mds_info, AVSV_NDA_CLA_MSG *i_msg, uns32 timeout)
{
	MDS_SEND_INFO *send_info = &mds_info->info.svc_send;
	MDS_SENDTYPE_SNDRSP_INFO *send = &send_info->info.sndrsp;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* populate the send info */
	send_info->i_msg = (NCSCONTEXT)i_msg;
	send_info->i_to_svc = NCSMDS_SVC_ID_AVND;
	send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;	/* Discuss the priority assignments TBD */
	send_info->i_sendtype = MDS_SENDTYPE_SNDRSP;
	send->i_to_dest = cb->avnd_intf.avnd_mds_dest;
	if (timeout)
		send->i_time_to_wait = timeout;
	else
		send->i_time_to_wait = SYSF_CLA_API_RESP_TIME;

	/* release the cb lock */
	if ((AVSV_CLA_API_MSG == i_msg->type) &&
	    ((AVSV_CLM_INITIALIZE == i_msg->info.api_info.type) || (AVSV_CLM_FINALIZE == i_msg->info.api_info.type)))
		m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);
	else
		m_NCS_UNLOCK(&cb->lock, NCS_LOCK_READ);

	/* send the message & block until AvND responds or operation times out */
	rc = ncsmds_api(mds_info);

	/* reacquire the cb lock */
	if ((AVSV_CLA_API_MSG == i_msg->type) &&
	    ((AVSV_CLM_INITIALIZE == i_msg->info.api_info.type) || (AVSV_CLM_FINALIZE == i_msg->info.api_info.type)))
		m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);
	else
		m_NCS_LOCK(&cb->lock, NCS_LOCK_READ);

	return rc;
}

/****************************************************************************
 * Name          : cla_mds_param_get
 *
 * Description   : This routine gets the mds handle & CLA MDS address.
 *
 * Arguments     : cb - ptr to the CLA control block
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 cla_mds_param_get(CLA_CB *cb)
{
	NCSADA_INFO ada_info;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&ada_info, 0, sizeof(ada_info));

	ada_info.req = NCSADA_GET_HDLS;
	ada_info.info.adest_get_hdls.i_create_oac = FALSE;

	/* invoke ada request */
	rc = ncsada_api(&ada_info);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

	/* store values returned by ada */
	cb->avnd_intf.mds_hdl = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
	cb->avnd_intf.cla_mds_dest = ada_info.info.adest_get_hdls.o_adest;

 done:
	return rc;
}

/****************************************************************************
  Name          : cla_mds_enc

  Description   : This routine is invoked to encode AVND message.

  Arguments     : cb       - ptr to the AvA control block
                  enc_info - ptr to the MDS encode info

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
uns32 cla_mds_enc(CLA_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{
	EDU_ERR ederror = 0;
	AVSV_NDA_CLA_MSG *msg = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	msg = (AVSV_NDA_CLA_MSG *)enc_info->i_msg;

	switch (msg->type) {
	case AVSV_CLA_API_MSG:
		rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_nd_cla_msg, enc_info->io_uba,
					EDP_OP_TYPE_ENC, msg, &ederror, enc_info->o_msg_fmt_ver);
		return rc;
		break;

	default:
		m_AVSV_ASSERT(0);
		break;
	}

	return rc;
}

/****************************************************************************
  Name          : cla_mds_dec

  Description   : This routine is invoked to decode AVND message.

  Arguments     : cb       - ptr to the AvA control block
                  dec_info - ptr to the MDS decode info

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
uns32 cla_mds_dec(CLA_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
	EDU_ERR ederror = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	switch (dec_info->i_fr_svc_id) {
	case NCSMDS_SVC_ID_AVND:

		dec_info->o_msg = NULL;

		rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_nd_cla_msg, dec_info->io_uba,
					EDP_OP_TYPE_DEC, (AVSV_NDA_CLA_MSG **)&dec_info->o_msg, &ederror,
					dec_info->i_msg_fmt_ver);

		if (rc != NCSCC_RC_SUCCESS) {
			if (dec_info->o_msg != NULL) {
				avsv_nda_cla_msg_free(dec_info->o_msg);
				dec_info->o_msg = NULL;
			}
			return rc;
		}
		break;

	default:
		m_AVSV_ASSERT(0);
		break;
	}

	return rc;
}
