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
 */

/*
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
..............................................................................

  DESCRIPTION:

  This file contains PDRBD specific MDS routines.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  pdrbd_mds_rcv    - rcv something from MDS lower layer.
  pdrbd_mds_evt - subscription event entry point
  pdrbd_mds_enc    - MDS encode function
  pdrbd_mds_dec    - MDS decode function
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "pdrbd.h"

/****************************************************************************
 * Name          : pdrbd_get_ada_hdl
 *
 * Description   : This function Gets the Handles of local MDS destination
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pdrbd_get_ada_hdl(void)
{
	NCSADA_INFO ada_info;

	memset(&ada_info, 0, sizeof(ada_info));

	ada_info.req = NCSADA_GET_HDLS;

	if (ncsada_api(&ada_info) != NCSCC_RC_SUCCESS) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_GET_ADEST_HDL_FAILED, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	/* Store values returned by ADA */
	pseudoCB.mds_hdl = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
	pseudoCB.my_pdrbd_dest = ada_info.info.adest_get_hdls.o_adest;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pdrbd_mds_install_and_subscribe
 *
 * Description   : This function registers the PDRBD Service with MDS.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pdrbd_mds_install_and_subscribe(void)
{
	NCSMDS_INFO mds_info;
	MDS_SVC_ID svc_ids_array[2];

	if (pseudoCB.mds_hdl == 0) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_HDL_IS_NULL, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	memset(&mds_info, 0, sizeof(mds_info));

	/* Do common stuff */
	mds_info.i_mds_hdl = pseudoCB.mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_PDRBD;

   /*****************************************************\
                   STEP: Install with MDS
   \*****************************************************/
	mds_info.i_op = MDS_INSTALL;
	mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	mds_info.info.svc_install.i_svc_cb = pdrbd_mds_callback;
	mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)((long)pseudoCB.cb_hdl);
	mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;	/* Total PWE scope */
	if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_INSTALL_SUBSCRIBE_FAILED, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

   /*****************************************************\
                   STEP: Subscribe to service(s)
   \*****************************************************/
	mds_info.i_op = MDS_SUBSCRIBE;
	mds_info.info.svc_subscribe.i_num_svcs = 1;
	mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	svc_ids_array[0] = NCSMDS_SVC_ID_PDRBD;
	mds_info.info.svc_subscribe.i_svc_ids = svc_ids_array;

	if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS) {
		pdrbd_mds_uninstall();
		m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_SUBSCRIPTION_FAILED, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pdrbd_mds_uninstall
 *
 * Description   : This function un-registers the PDRBD Service with MDS.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pdrbd_mds_uninstall(void)
{
	NCSMDS_INFO arg;
	uns32 rc;

	/* Un-install your service into MDS. */
	memset(&arg, 0, sizeof(NCSMDS_INFO));

	arg.i_mds_hdl = pseudoCB.mds_hdl;
	arg.i_svc_id = NCSMDS_SVC_ID_PDRBD;
	arg.i_op = MDS_UNINSTALL;

	rc = ncsmds_api(&arg);

	if (rc != NCSCC_RC_SUCCESS) {
		return rc;
	}

	return rc;
}

/****************************************************************************
 * Name          : pdrbd_mds_callback
 *
 * Description   : Call back function provided to MDS. MDS will call this
 *                 function for enc/dec/cpy/rcv/svc_evt operations.
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pdrbd_mds_callback(NCSMDS_CALLBACK_INFO *cbinfo)
{
	uns32 status = NCSCC_RC_SUCCESS;

	switch (cbinfo->i_op) {
	case MDS_CALLBACK_ENC:
		/* Treating both encode types as same */
		status = pdrbd_mds_enc(cbinfo->i_yr_svc_hdl, cbinfo->info.enc.i_msg,
				       cbinfo->info.enc.i_to_svc_id, cbinfo->info.enc.io_uba);
		break;

	case MDS_CALLBACK_DEC:
		/* Treating both decode types as same */
		status = pdrbd_mds_dec(cbinfo->i_yr_svc_hdl, &cbinfo->info.dec.o_msg,
				       cbinfo->info.dec.i_fr_svc_id, cbinfo->info.dec.io_uba);
		break;

	case MDS_CALLBACK_RECEIVE:
		status = pdrbd_mds_rcv(cbinfo->i_yr_svc_hdl, cbinfo->info.receive.i_msg);
		break;

	case MDS_CALLBACK_SVC_EVENT:
		pdrbd_mds_evt(cbinfo->info.svc_evt, cbinfo->i_yr_svc_hdl);

		status = NCSCC_RC_SUCCESS;
		break;

	case MDS_CALLBACK_ENC_FLAT:
	case MDS_CALLBACK_COPY:
	case MDS_CALLBACK_DEC_FLAT:
	default:
		m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_CBK_WITH_WRONG_OPERATION, NCSFL_SEV_ERROR, NULL);
		status = NCSCC_RC_FAILURE;
		break;
	}

	return status;
}

/****************************************************************************\
 * Name          : pdrbd_mds_rcv
 *
 * Description   : PDRBD MDS receive call back function.
 *
 * Arguments     : yr_svc_hdl : PDRBD service handle.
 *                 msg        : Message received from the DTS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 pdrbd_mds_rcv(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg)
{
	PSEUDO_CB *cb;

	if (NULL == msg)
		return NCSCC_RC_FAILURE;

	/* retrieve pDRBD cb */
	if (0 == (cb = (PSEUDO_CB *)ncshm_take_hdl(NCS_SERVICE_ID_PDRBD, (uns32)yr_svc_hdl))) {
		m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_CB_GET_FAILURE, NCSFL_SEV_ERROR, 1);
		return NCSCC_RC_FAILURE;
	}

	m_PDRBD_SND_MSG(&cb->mailBox, msg, NCS_IPC_PRIORITY_HIGH);

	if (cb)
		ncshm_give_hdl((uns32)yr_svc_hdl);
	return NCSCC_RC_SUCCESS;;

}

/****************************************************************************\
 * Name          : pdrbd_mds_evt
 *
 * Description   : PDRBD is informed when MDS events occurr that he has subscribed to.
 *
 * Arguments     : svc_info   : Service event Info.
 *                 yr_svc_hdl : PDRBD service handle.
 *
 * Return Values : None.
 *
 * Notes         : None.
\*****************************************************************************/
void pdrbd_mds_evt(MDS_CALLBACK_SVC_EVENT_INFO svc_info, MDS_CLIENT_HDL yr_svc_hdl)
{
	int32 i;
	PSEUDO_CB *cb;

	m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_EVT_RECD, NCSFL_SEV_INFO, NULL);

	/* retrieve pDRBD cb */
	if (0 == (cb = (PSEUDO_CB *)ncshm_take_hdl(NCS_SERVICE_ID_PDRBD, (uns32)yr_svc_hdl))) {
		m_LOG_PDRBD_SCRIPT(PDRBD_SCRIPT_CB_GET_FAILURE, NCSFL_SEV_ERROR, 2);
		goto done;
	}

	switch (svc_info.i_change) {	/* Review change type */
	case NCSMDS_DOWN:
		if (svc_info.i_svc_id == NCSMDS_SVC_ID_PDRBD) {
			m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_DOWN_EVT_RCVD, NCSFL_SEV_INFO, NULL);

			/* If down is for my peer then clean up its adest */
			if (svc_info.i_dest == cb->peer_pdrbd_dest)
				cb->peer_pdrbd_dest = 0;

			/* Zero all the StandAlone attempt counts since the MDS is down */
			for (i = 0; i < PDRBD_MAX_PROXIED + 1; i++)
				pseudoCB.proxied_info[i].stdalone_cnt = 0;
		}

		break;

	case NCSMDS_UP:
		if (svc_info.i_svc_id == NCSMDS_SVC_ID_PDRBD) {
			/* If up is for my peer then update adest */
			m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_UP_EVT_RCVD, NCSFL_SEV_INFO, NULL);

			if (svc_info.i_dest != cb->my_pdrbd_dest)
				cb->peer_pdrbd_dest = svc_info.i_dest;
		}
		break;

	case NCSMDS_NO_ACTIVE:
	case NCSMDS_NEW_ACTIVE:
	default:
		break;
	}

	if (cb)
		ncshm_give_hdl((uns32)yr_svc_hdl);
 done:
	return;

}

/****************************************************************************\
 * Name          : pdrbd_mds_enc
 *
 * Description   : Encode a PDRBD message headed out
 *
 * Arguments     : yr_svc_hdl : PDRBD service handle.
 *                 msg        : Message received from the DTS.
 *                 to_svc     : Service ID to send.
 *                 uba        : User Buffer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 pdrbd_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg, SS_SVC_ID to_svc, NCS_UBAID *uba)
{
	uns8 *data;
	PDRBD_EVT *mm;

	if (uba == NULL) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_RCVD_NULL_USR_BUFF, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	mm = (PDRBD_EVT *)msg;

	if (mm == NULL) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_RCVD_NULL_MSG_DATA, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	data = ncs_enc_reserve_space(uba, sizeof(uns8));
	if (data == NULL) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_NCS_ENC_RESERVE_SPC_NULL, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	ncs_encode_8bit(&data, mm->evt_type);

	ncs_enc_claim_space(uba, sizeof(uns8));

	switch (mm->evt_type) {
	case PDRBD_PEER_CLEAN_MSG_EVT:
	case PDRBD_PEER_ACK_MSG_EVT:
		{
			data = ncs_enc_reserve_space(uba, sizeof(uns32));
			ncs_encode_32bit(&data, mm->evt_data.number);
			ncs_enc_claim_space(uba, sizeof(uns32));
			return NCSCC_RC_SUCCESS;
		}
		break;

	default:
		m_LOG_PDRBD_MISC(PDRBD_MISC_WRONG_MSG_TYPE_RCVD, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Name          : pdrbd_mds_dec
 *
 * Description   : Decode a PDRBD message coming in.
 *
 * Arguments     : yr_svc_hdl : PDRBD service handle.
 *                 msg        : Message received from the DTS.
 *                 to_svc     : Service ID to send.
 *                 uba        : User Buffer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 pdrbd_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT *msg, SS_SVC_ID to_svc, NCS_UBAID *uba)
{
	uns8 *data;
	PDRBD_EVT *mm;
	uns8 data_buff[32];
	uns32 status = NCSCC_RC_SUCCESS;

	if (uba == NULL) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_RCVD_NULL_USR_BUF_FOR_DEC, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	if (msg == NULL) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_RCVD_NULL_MSG_BUF_FOR_DEC, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	mm = m_MMGR_ALLOC_PDRBD_EVT;

	if (mm == NULL) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_MEM_ALLOC_FAILED_FOR_DEC, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	memset(mm, '\0', sizeof(PDRBD_EVT));

	*msg = mm;

	data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns8));
	if (data == NULL) {
		m_LOG_PDRBD_MISC(PDRBD_MISC_DEC_FLAT_SPACE_RET_NULL, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}

	mm->evt_type = ncs_decode_8bit(&data);

	ncs_dec_skip_space(uba, sizeof(uns8));

	switch (mm->evt_type) {
	case PDRBD_PEER_CLEAN_MSG_EVT:
	case PDRBD_PEER_ACK_MSG_EVT:
		{
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
			if (data == NULL) {
				m_LOG_PDRBD_MISC(PDRBD_MISC_DEC_FLAT_SPACE_RET_NULL, NCSFL_SEV_ERROR, NULL);
				return NCSCC_RC_FAILURE;
			}

			mm->evt_data.number = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(uns32));
			break;
		}

	default:
		m_LOG_PDRBD_MISC(PDRBD_MISC_WRNG_MSG_TYPE_RCVD_DEC, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}
	return status;
}

/****************************************************************************
  Name          : pdrbd_mds_async_send

  Description   : This routine sends the PDRBD message to other PDRBD.

  Arguments     : msg  - Message to be sent
                  inst - PDRBD control blovk

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
uns32 pdrbd_mds_async_send(PSEUDO_CB *inst, PDRBD_EVT_TYPE evt_type, uns32 rsrc_num)
{
	NCSMDS_INFO mds_info;
	PDRBD_EVT msg;

	/* Fill event contents */
	memset(&msg, 0, sizeof(PDRBD_EVT));
	msg.evt_type = evt_type;
	msg.evt_data.number = rsrc_num;

	/* Fill MDS info */
	memset(&mds_info, 0, sizeof(mds_info));

	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_PDRBD;
	mds_info.i_op = MDS_SEND;

	mds_info.info.svc_send.i_msg = (NCSCONTEXT)&msg;
	mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_LOW;
	mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
	mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_PDRBD;

	mds_info.info.svc_send.info.snd.i_to_dest = inst->peer_pdrbd_dest;

	if (ncsmds_api(&mds_info) == NCSCC_RC_SUCCESS) {
		return NCSCC_RC_SUCCESS;
	} else {
		m_LOG_PDRBD_MISC(PDRBD_MISC_MDS_ASYNC_SEND_FAILED, NCSFL_SEV_ERROR, NULL);
		return NCSCC_RC_FAILURE;
	}
}
