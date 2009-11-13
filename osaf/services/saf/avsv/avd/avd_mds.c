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

  This module does the initialisation of MDS and provides callback functions 
  at Availability Directors. It contains the receive callback that
  is called by MDS for the messages addressed to AvD.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_mds_reg_def - registers the AVD role Service with MDS.
  avd_mds_reg - registers the AVD Service with MDS.

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#include "avd.h"

/* these will go away */
#define m_AVD_LOG_MSG_MDS_RCV_INFO(a, b, c)

const MDS_CLIENT_MSG_FORMAT_VER avd_avnd_msg_fmt_map_table[AVD_AVND_SUBPART_VER_MAX] =
    { AVSV_AVD_AVND_MSG_FMT_VER_1, AVSV_AVD_AVND_MSG_FMT_VER_2 };
const MDS_CLIENT_MSG_FORMAT_VER avd_bam_msg_fmt_map_table[AVD_BAM_SUBPART_VER_MAX] = { AVSV_AVD_BAM_MSG_FMT_VER_1 };
const MDS_CLIENT_MSG_FORMAT_VER avd_avm_msg_fmt_map_table[AVD_AVM_SUBPART_VER_MAX] = { AVSV_AVD_AVM_MSG_FMT_VER_1 };
const MDS_CLIENT_MSG_FORMAT_VER avd_avd_msg_fmt_map_table[AVD_AVD_SUBPART_VER_MAX] = { AVD_AVD_MSG_FMT_VER_1 };

/* fwd decl */

static uns32 avd_mds_svc_evt(uns32 *cb_hdl, MDS_CALLBACK_SVC_EVENT_INFO *evt_info);
static uns32 avd_mds_qsd_ack_evt(uns32 cb_hdl, MDS_CALLBACK_QUIESCED_ACK_INFO *evt_info);

/****************************************************************************
  Name          : avd_mds_reg_def
 
  Description   : This routine registers the AVD role Service with MDS on the
                  default Aaddress.
 
  Arguments     : cb - ptr to the AVD control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_mds_reg_def(AVD_CL_CB *cb)
{
	uns32 rc;
	EDU_ERR err = EDU_NORMAL;

	/* Initialise the EDU to be used with MDS */
	m_NCS_EDU_HDL_INIT(&cb->edu_hdl);

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_dnd_msg, &err);
	if (rc != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_INVALID_VAL_ERROR(err);
		/* EDU cleanup */
		m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
		return rc;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avd_mds_set_vdest_role
 
  Description   : This routine is used for setting the VDEST role.
 
  Arguments     : cb - ptr to the AVD control block
                  role - Set the role.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_mds_set_vdest_role(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	NCSVDA_INFO vda_info;

	memset(&vda_info, '\0', sizeof(NCSVDA_INFO));

	/* set the role of the vdest */
	vda_info.req = NCSVDA_VDEST_CHG_ROLE;
	vda_info.info.vdest_chg_role.i_new_role = role;
	vda_info.info.vdest_chg_role.i_vdest = cb->vaddr;

	if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_VDEST_ROL);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avd_mds_reg
 
  Description   : This routine registers the AVD Service with MDS with the 
  AVDs Vaddress. 
 
  Arguments     : cb - ptr to the AVD control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_mds_reg(AVD_CL_CB *cb)
{
	NCSVDA_INFO vda_info;
	NCSMDS_INFO svc_to_mds_info;
	MDS_SVC_ID svc_id[1];
	uns32 rc;
	NCSADA_INFO ada_info;

	/* prepare the cb with the vaddress */
	m_NCS_SET_VDEST_ID_IN_MDS_DEST(cb->vaddr, AVSV_AVD_VCARD_ID);

	memset(&vda_info, '\0', sizeof(NCSVDA_INFO));

	vda_info.req = NCSVDA_VDEST_CREATE;
	vda_info.info.vdest_create.i_persistent = FALSE;
	vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
	vda_info.info.vdest_create.i_create_oac = TRUE;
	vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
	vda_info.info.vdest_create.info.specified.i_vdest = cb->vaddr;

	/* create Vdest address */
	if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_VDEST_CRT);
		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_VDEST_CRT);

	/* store the info returned by MDS */
	cb->vaddr_pwe_hdl = vda_info.info.vdest_create.o_mds_pwe1_hdl;
	cb->vaddr_hdl = vda_info.info.vdest_create.o_mds_vdest_hdl;

	/* Install on mds VDEST */
	svc_to_mds_info.i_mds_hdl = cb->vaddr_pwe_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_AVD;
	svc_to_mds_info.i_op = MDS_INSTALL;
	svc_to_mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)cb->cb_handle;
	svc_to_mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
	svc_to_mds_info.info.svc_install.i_svc_cb = avd_mds_cbk;
	svc_to_mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	svc_to_mds_info.info.svc_install.i_mds_svc_pvt_ver = AVD_MDS_SUB_PART_VERSION;

	if (ncsmds_api(&svc_to_mds_info) != NCSCC_RC_SUCCESS) {
		memset(&vda_info, '\0', sizeof(NCSVDA_INFO));
		vda_info.req = NCSVDA_VDEST_DESTROY;
		vda_info.info.vdest_destroy.i_vdest = cb->vaddr;
		ncsvda_api(&vda_info);
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_INSTALL);
		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_INSTALL);

   /*** subscribe to AVD mds event ***/
	svc_to_mds_info.i_op = MDS_RED_SUBSCRIBE;
	svc_to_mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	svc_to_mds_info.info.svc_subscribe.i_svc_ids = svc_id;
	svc_to_mds_info.info.svc_subscribe.i_num_svcs = 1;
	svc_id[0] = NCSMDS_SVC_ID_AVD;

	rc = ncsmds_api(&svc_to_mds_info);
	if (NCSCC_RC_SUCCESS != rc) {
		memset(&vda_info, '\0', sizeof(NCSVDA_INFO));
		vda_info.req = NCSVDA_VDEST_DESTROY;
		vda_info.info.vdest_destroy.i_vdest = cb->vaddr;
		ncsvda_api(&vda_info);
		/*avd_mds_unreg(cb); */
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_INSTALL);
		return NCSCC_RC_FAILURE;
	}

	/* AVD ADEST create install and subscription */
	memset(&ada_info, 0, sizeof(ada_info));

	ada_info.req = NCSADA_GET_HDLS;
	ada_info.info.adest_get_hdls.i_create_oac = FALSE;

	if (ncsada_api(&ada_info) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_INSTALL);
		return NCSCC_RC_FAILURE;
	}

	/* Store values returned by ADA */
	cb->adest_hdl = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
	cb->avm_mds_dest = ada_info.info.adest_get_hdls.o_adest;

	/* Install on mds ADEST */
	svc_to_mds_info.i_mds_hdl = cb->adest_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_AVD;
	svc_to_mds_info.i_op = MDS_INSTALL;
	svc_to_mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)cb->cb_handle;
	svc_to_mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
	svc_to_mds_info.info.svc_install.i_svc_cb = avd_mds_cbk;
	svc_to_mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	svc_to_mds_info.info.svc_install.i_mds_svc_pvt_ver = AVD_MDS_SUB_PART_VERSION;

	if (ncsmds_api(&svc_to_mds_info) != NCSCC_RC_SUCCESS) {
		memset(&vda_info, '\0', sizeof(NCSVDA_INFO));
		vda_info.req = NCSVDA_VDEST_DESTROY;
		vda_info.info.vdest_destroy.i_vdest = cb->vaddr;
		ncsvda_api(&vda_info);
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_INSTALL);
		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_INSTALL);

   /*** subscribe to avm mds event ***/
	svc_to_mds_info.i_mds_hdl = cb->adest_hdl;
	svc_to_mds_info.i_op = MDS_SUBSCRIBE;
	svc_to_mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_INTRANODE;
	svc_to_mds_info.info.svc_subscribe.i_svc_ids = svc_id;
	svc_to_mds_info.info.svc_subscribe.i_num_svcs = 1;
	svc_id[0] = NCSMDS_SVC_ID_AVM;

	rc = ncsmds_api(&svc_to_mds_info);
	if (NCSCC_RC_SUCCESS != rc) {
		memset(&vda_info, '\0', sizeof(NCSVDA_INFO));
		vda_info.req = NCSVDA_VDEST_DESTROY;
		vda_info.info.vdest_destroy.i_vdest = cb->vaddr;
		ncsvda_api(&vda_info);
		/*avd_mds_unreg(cb); */
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_INSTALL);
		return NCSCC_RC_FAILURE;
	}

   /*** subscribe to avnd, just for not delaying bcast ***/
	svc_to_mds_info.i_mds_hdl = cb->adest_hdl;
	svc_to_mds_info.i_op = MDS_SUBSCRIBE;
	svc_to_mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	svc_to_mds_info.info.svc_subscribe.i_svc_ids = svc_id;
	svc_to_mds_info.info.svc_subscribe.i_num_svcs = 1;
	svc_id[0] = NCSMDS_SVC_ID_AVND;

	rc = ncsmds_api(&svc_to_mds_info);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_INSTALL);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avd_mds_cbk

  Description   : This is a callback routine invoked by MDS for delivering
  messages and events. It also calls back for encoding or decoding messages
  on the Vdest address (from node director).
 
  Arguments     : info:  MDS call back information
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uns32 avd_mds_cbk(struct ncsmds_callback_info *info)
{
	uns32 cb_hdl;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (info == NULL) {
		m_AVD_LOG_INVALID_VAL_ERROR(0);
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_RCV_CBK);
		return NCSCC_RC_FAILURE;
	}

	cb_hdl = (uns32)info->i_yr_svc_hdl;
	switch (info->i_op) {
	case MDS_CALLBACK_RECEIVE:
		/* check that the node director sent this
		 * message to the director.
		 */
		if ((info->info.receive.i_fr_svc_id == NCSMDS_SVC_ID_AVND) &&
		    (info->info.receive.i_to_svc_id == NCSMDS_SVC_ID_AVD)) {
			NODE_ID node_id = 0;
			uns16 msg_fmt_ver = 0;
			m_AVD_LOG_MSG_DND_RCV_INFO(AVD_LOG_RCVD_MSG,
						   ((AVD_DND_MSG *)info->info.receive.i_msg),
						   info->info.receive.i_node_id);
			node_id = m_NCS_NODE_ID_FROM_MDS_DEST(info->info.receive.i_fr_dest);
			msg_fmt_ver = info->info.receive.i_msg_fmt_ver;
			rc = avd_n2d_msg_rcv(cb_hdl, (AVD_DND_MSG *)info->info.receive.i_msg, node_id, msg_fmt_ver);
			info->info.receive.i_msg = (NCSCONTEXT)0;
			if (rc != NCSCC_RC_SUCCESS) {
				m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_RCV_CBK);
				return rc;
			}
		}
		/* check if AvM sent this
		 ** message to the director if not return error.
		 */
		else if ((info->info.receive.i_fr_svc_id == NCSMDS_SVC_ID_AVM) &&
			 (info->info.receive.i_to_svc_id == NCSMDS_SVC_ID_AVD)) {
			m_AVD_LOG_MSG_MDS_RCV_INFO(AVD_LOG_RCVD_MSG,
						   ((AVM_AVD_MSG_T *)info->info.receive.i_msg),
						   info->info.receive.i_node_id);

			rc = avd_avm_rcv_msg(cb_hdl, (AVM_AVD_MSG_T *)info->info.receive.i_msg);
			info->info.receive.i_msg = (NCSCONTEXT)0;
			if (rc != NCSCC_RC_SUCCESS) {
				m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_RCV_CBK);
				return rc;
			}
		}
		/* check if AvD sent this
		 ** message to the director if not return error.
		 */
		else if ((info->info.receive.i_fr_svc_id == NCSMDS_SVC_ID_AVD) &&
			 (info->info.receive.i_to_svc_id == NCSMDS_SVC_ID_AVD)) {
			m_AVD_LOG_MSG_MDS_RCV_INFO(AVD_LOG_RCVD_MSG,
						   ((AVM_AVD_MSG_T *)info->info.receive.i_msg),
						   info->info.receive.i_node_id);

			rc = avd_d2d_msg_rcv(cb_hdl, (AVD_D2D_MSG *)info->info.receive.i_msg);
			info->info.receive.i_msg = (NCSCONTEXT)0;
			if (rc != NCSCC_RC_SUCCESS) {
				m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_RCV_CBK);
				return rc;
			}
		} else {
			m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_RCV_CBK);
			return NCSCC_RC_FAILURE;
		}
		break;
	case MDS_CALLBACK_SVC_EVENT:
		{
			rc = avd_mds_svc_evt(&cb_hdl, &info->info.svc_evt);
			if (rc != NCSCC_RC_SUCCESS) {
				/*m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SVC_CBK); */
				return rc;
			}
		}
		break;
	case MDS_CALLBACK_COPY:
		{
			if ((info->i_yr_svc_id == NCSMDS_SVC_ID_AVD) &&
			    (info->info.cpy.i_to_svc_id == NCSMDS_SVC_ID_AVM)) {
				info->info.cpy.o_msg_fmt_ver =
				    avd_avm_msg_fmt_map_table[info->info.cpy.i_rem_svc_pvt_ver - 1];

				rc = avd_avm_mds_cpy(&info->info.cpy);
				if (rc != NCSCC_RC_SUCCESS) {
					m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_CPY_CBK);
					return rc;
				}
			} else {
				info->info.cpy.o_msg_fmt_ver =
				    avd_avnd_msg_fmt_map_table[info->info.cpy.i_rem_svc_pvt_ver - 1];

				rc = avd_mds_cpy(&info->info.cpy);
				if (rc != NCSCC_RC_SUCCESS) {
					m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_CPY_CBK);
					return rc;
				}
			}
		}
		break;
	case MDS_CALLBACK_ENC:
	case MDS_CALLBACK_ENC_FLAT:
		{

			if ((info->i_yr_svc_id == NCSMDS_SVC_ID_AVD) &&
			    (info->info.enc.i_to_svc_id == NCSMDS_SVC_ID_AVD)) {
				info->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
										     AVD_AVD_SUBPART_VER_MIN,
										     AVD_AVD_SUBPART_VER_MAX,
										     avd_avd_msg_fmt_map_table);

				if (info->info.enc.o_msg_fmt_ver < AVD_AVD_MSG_FMT_VER_1) {
					/* log the problem */
					m_AVD_LOG_MDS_CRITICAL(AVSV_LOG_MDS_ENC_CBK);
					return NCSCC_RC_FAILURE;
				}

				avd_mds_d_enc(cb_hdl, &info->info.enc);
			} else if ((info->i_yr_svc_id == NCSMDS_SVC_ID_AVD) &&
				   (info->info.enc.i_to_svc_id == NCSMDS_SVC_ID_AVND)) {
				info->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
										     AVD_AVND_SUBPART_VER_MIN,
										     AVD_AVND_SUBPART_VER_MAX,
										     avd_avnd_msg_fmt_map_table);

				if (info->info.enc.o_msg_fmt_ver < AVSV_AVD_AVND_MSG_FMT_VER_1) {
					/* log the problem */
					m_AVD_LOG_MDS_CRITICAL(AVSV_LOG_MDS_ENC_CBK);
					return NCSCC_RC_FAILURE;
				}

				rc = avd_mds_enc(cb_hdl, &info->info.enc);
				if (rc != NCSCC_RC_SUCCESS) {
					m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_ENC_CBK);
					return rc;
				}
			} else {
				m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_ENC_CBK);
				return NCSCC_RC_FAILURE;
			}
		}

		break;
	case MDS_CALLBACK_DEC:
	case MDS_CALLBACK_DEC_FLAT:
		{

			if ((info->i_yr_svc_id == NCSMDS_SVC_ID_AVD)
			    && (info->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_AVD)) {
				if (!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
							       AVD_AVD_SUBPART_VER_MIN,
							       AVD_AVD_SUBPART_VER_MAX, avd_avd_msg_fmt_map_table)) {
					/* log the problem */
					m_AVD_LOG_MDS_CRITICAL(AVSV_LOG_MDS_DEC_CBK);
					return NCSCC_RC_FAILURE;
				}

				avd_mds_d_dec(cb_hdl, &info->info.dec);
			} else if ((info->i_yr_svc_id == NCSMDS_SVC_ID_AVD)
				   && (info->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_AVND)) {
				if (!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
							       AVD_AVND_SUBPART_VER_MIN,
							       AVD_AVND_SUBPART_VER_MAX, avd_avnd_msg_fmt_map_table)) {
					/* log the problem */
					m_AVD_LOG_MDS_CRITICAL(AVSV_LOG_MDS_DEC_CBK);
					return NCSCC_RC_FAILURE;
				}

				rc = avd_mds_dec(cb_hdl, &info->info.dec);
			} else {
				m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_DEC_CBK);
				return NCSCC_RC_FAILURE;
			}

			if (rc != NCSCC_RC_SUCCESS) {
				m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_DEC_CBK);
				return rc;
			}
		}

		break;
	case MDS_CALLBACK_QUIESCED_ACK:
		{
			rc = avd_mds_qsd_ack_evt(cb_hdl, &info->info.quiesced_ack);
			if (rc != NCSCC_RC_SUCCESS) {
				/*m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SVC_CBK); */
				return rc;
			}
		}
		break;
	default:
		m_AVD_LOG_INVALID_VAL_ERROR(info->i_op);
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_RCV_CBK);
		return NCSCC_RC_FAILURE;
		break;
	}

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_RCV_CBK);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avd_mds_svc_evt
 
  Description   : This routine is invoked to inform AvD of MDS events. AvD 
                  had subscribed to these events during MDS registration.
 
  Arguments     : cb       - ptr to the AvD control block
                  evt_info - ptr to the MDS event info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
static uns32 avd_mds_svc_evt(uns32 *cb_hdl, MDS_CALLBACK_SVC_EVENT_INFO *evt_info)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	AVD_CL_CB *cb;

	/* get the CB from the handle manager */
	if ((cb = ncshm_take_hdl(NCS_SERVICE_ID_AVD, *cb_hdl)) == NULL)
		return NCSCC_RC_FAILURE;

	switch (evt_info->i_change) {
	case NCSMDS_UP:
		switch (evt_info->i_svc_id) {
		case NCSMDS_SVC_ID_AVM:
			if (m_NCS_MDS_DEST_EQUAL(&cb->avm_mds_dest, &evt_info->i_dest)) {
				cb->avm_mds_dest = evt_info->i_dest;
			}
			break;

		case NCSMDS_SVC_ID_AVD:
			/* if((Is this up from other node) && (Is this Up from an Adest)) */
			if ((evt_info->i_node_id != cb->node_id_avd) && (m_MDS_DEST_IS_AN_ADEST(evt_info->i_dest))) {
				cb->node_id_avd_other = evt_info->i_node_id;
				cb->other_avd_adest = evt_info->i_dest;
			}
			break;

		case NCSMDS_SVC_ID_AVND:
			break;

		case NCSMDS_SVC_ID_BAM:
		default:
			assert(0);
		}
		break;

	case NCSMDS_DOWN:
		switch (evt_info->i_svc_id) {
		case NCSMDS_SVC_ID_AVM:
			if (m_NCS_MDS_DEST_EQUAL(&cb->avm_mds_dest, &evt_info->i_dest)) {
				memset(&cb->avm_mds_dest, 0, sizeof(MDS_DEST));
			}
			break;

		case NCSMDS_SVC_ID_AVD:

			/* if(Is this down from an Adest) && (Is this adest same as Adest in CB) */
			if (m_MDS_DEST_IS_AN_ADEST(evt_info->i_dest)
			    && m_NCS_MDS_DEST_EQUAL(&evt_info->i_dest, &cb->other_avd_adest)) {
				memset(&cb->other_avd_adest, '\0', sizeof(MDS_DEST));
			}
			/* cb->node_id_avd_other should not be made 0, because heart beat loss message to AvM relies on this field -  */
			break;

		case NCSMDS_SVC_ID_AVND:
			break;

		case NCSMDS_SVC_ID_BAM:
		default:
			assert(0);
		}
		break;

	default:
		break;
	}

	return rc;
}

/****************************************************************************
  Name          : avd_mds_qsd_ack_evt
 
  Description   : This routine is invoked to inform AvD of MDS quiesce ack 
                  events.  
 
  Arguments     : cb       - ptr to the AvD control block
                  evt_info - ptr to the MDS event info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : We will post a message on to ourself when we get the quiesced 
                  ack from mds. This new event will not have any info
                  part, only the msg type is enough 
******************************************************************************/
static uns32 avd_mds_qsd_ack_evt(uns32 cb_hdl, MDS_CALLBACK_QUIESCED_ACK_INFO *evt_info)
{
	AVD_EVT *evt = AVD_EVT_NULL;
	AVD_CL_CB *cb = NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_mds_qsd_ack_evt");

	/* create the message event */
	evt = calloc(1, sizeof(AVD_EVT));
	if (evt == AVD_EVT_NULL) {
		/* log error */
		m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_RCVD_VAL(((long)evt));

	/* get the CB from the handle manager */
	if ((cb = (AVD_CL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVD, cb_hdl)) == NULL) {
		/* log error */
		m_AVD_LOG_INVALID_VAL_FATAL(cb_hdl);
		free(evt);
		return NCSCC_RC_FAILURE;
	}

	evt->cb_hdl = cb_hdl;
	evt->rcv_evt = AVD_EVT_MDS_QSD_ACK;

	m_AVD_LOG_EVT_INFO(AVD_SND_AVND_MSG_EVENT, evt->rcv_evt);

	if (m_NCS_IPC_SEND(&cb->avd_mbx, evt, NCS_IPC_PRIORITY_HIGH)
	    != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
		/* return AvD CB handle */
		ncshm_give_hdl(cb_hdl);
		/* log error */
		/* free the event and return */
		free(evt);

		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);

	/* return AvD CB handle */
	ncshm_give_hdl(cb_hdl);

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_RCV_CBK);
	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
  Name          : avd_avnd_mds_send

  Description   : This routine sends MDS message from AvD to AvND. 

  Arguments     : cb - ptr to the AVD control block

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
******************************************************************************/
uns32 avd_avnd_mds_send(AVD_CL_CB *cb, AVD_AVND *nd_node, AVD_DND_MSG *snd_msg)
{
	NCSMDS_INFO snd_mds;
	uns32 rc;

	m_AVD_LOG_FUNC_ENTRY("avd_d2n_msg_snd");
	m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG, snd_msg, sizeof(AVD_DND_MSG), snd_msg);

	memset(&snd_mds, '\0', sizeof(NCSMDS_INFO));

	snd_mds.i_mds_hdl = cb->adest_hdl;
	snd_mds.i_svc_id = NCSMDS_SVC_ID_AVD;
	snd_mds.i_op = MDS_SEND;
	snd_mds.info.svc_send.i_msg = (NCSCONTEXT)snd_msg;
	snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_AVND;
	snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
	snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
	snd_mds.info.svc_send.info.snd.i_to_dest = nd_node->adest;

	/*
	 * Now do MDS send.
	 */
	if ((rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SEND);
		return rc;
	}

	return rc;
}
