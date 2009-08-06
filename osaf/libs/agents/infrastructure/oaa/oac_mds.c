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

  This file contains OAA specific MDS routines, being:
    oac_mds_rcv() 
    oac_mds_evt_cb()
    oac_mds_cb()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mab.h"
#include "ncs_util.h"

MDS_CLIENT_MSG_FORMAT_VER oac_mas_msg_fmt_table[OAC_WRT_MAS_SUBPART_VER_RANGE] = { 1, 2 };
MDS_CLIENT_MSG_FORMAT_VER oac_mac_msg_fmt_table[OAC_WRT_MAC_SUBPART_VER_RANGE] = { 1, 2 };
uns8 oac_pss_msg_fmt_table[OAC_WRT_PSS_SUBPART_VER_RANGE] = { 1 };

#if (NCS_MAB == 1)

/****************************************************************************\
*  Name:          oac_mds_rcv                                                * 
*                                                                            *
*  Description:   Posts a message to OAA mailbox from MDS thread.            * 
*                                                                            *
*  Arguments:     NCSCONTEXT - OAA CB, registered with MDS.                  *
*                 NCSCONTEXT - Pinter to the decoded MAB_MSG*                *
*                 MDS_DEST   - fr_card and to_card                           *
*                 SS_SVC_ID  - Service ids                                   *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32
oac_mds_rcv(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
	    MDS_DEST fr_card, SS_SVC_ID fr_svc, MDS_DEST to_card, SS_SVC_ID to_svc)
{
	uns32 status;
	OAC_TBL *inst = (OAC_TBL *)m_OAC_VALIDATE_HDL((uns32)yr_svc_hdl);

	if (inst == NULL) {
		m_LOG_MAB_NO_CB("oac_mds_rcv()");
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (inst) {
		m_LOG_MAB_SVC_PRVDR_MSG(NCSFL_SEV_DEBUG, MAB_SP_OAC_MDS_RCV_MSG, fr_svc, fr_card, ((MAB_MSG *)msg)->op);
	}

	/* plant MAB subcomponent's control block in MAB_MSG */
	((MAB_MSG *)msg)->yr_hdl = inst;

	/* Put it in OAC's work queue */
	status = m_OAC_SND_MSG(inst->mbx, msg);
	if (status != NCSCC_RC_SUCCESS) {
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL,
				   MAB_OAC_ERR_MDS_TO_MBX_SND_FAILED, status, ((MAB_MSG *)msg)->op);
		ncshm_give_hdl((uns32)yr_svc_hdl);
		return m_MAB_DBG_SINK(status);
	}

	ncshm_give_hdl((uns32)yr_svc_hdl);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          oac_mds_evt_cb                                             * 
*                                                                            *
*  Description:   Posts an MDS UP/DOWN message to OAA Mailbox                * 
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO  - Information from MDS               *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*                 NCSCC_RC_OUT_OF_MEM - No memory in the system              *
\****************************************************************************/

static uns32 oac_mds_evt_cb(NCSMDS_CALLBACK_INFO *cbinfo)
{
	uns32 status;
	OAC_TBL *inst;
	MAB_MSG *mm = NULL;
	NCS_BOOL post_to_mbx = FALSE;

	inst = (OAC_TBL *)m_OAC_VALIDATE_HDL((uns32)cbinfo->i_yr_svc_hdl);
	if (inst == NULL) {
		m_LOG_MAB_NO_CB("oac_mds_evt_cb()");
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	switch (cbinfo->info.svc_evt.i_change) {	/* Review change type */
	case NCSMDS_DOWN:
	case NCSMDS_UP:
		{
			if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_MAS) &&
			    (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY)) {
				post_to_mbx = TRUE;
			} else if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_PSS) &&
				   (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY)) {
				post_to_mbx = TRUE;
			} else if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC) &&
				   (inst->my_vcard == cbinfo->info.svc_evt.i_dest)) {
				if (m_NCS_NODE_ID_FROM_MDS_DEST(inst->my_vcard) != 0) {
					/* This is ADEST, and a self-event is detected. */
					post_to_mbx = TRUE;
				}
			}
		}
		break;

	case NCSMDS_NO_ACTIVE:
	case NCSMDS_NEW_ACTIVE:
		{
			if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_MAS) &&
			    (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY)) {
				post_to_mbx = TRUE;
			} else if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_PSS) &&
				   (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY)) {
				post_to_mbx = TRUE;
			} else if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC) &&
				   (inst->my_vcard == cbinfo->info.svc_evt.i_dest)) {
				/* This comes only for VDEST OAA's */
				if ((m_NCS_NODE_ID_FROM_MDS_DEST(inst->my_vcard) == 0) &&
				    (cbinfo->info.svc_evt.i_anc == inst->my_anc)) {
					/* This is VDEST, and a self-event is detected. */
					post_to_mbx = TRUE;
				}
			}
		}
		break;

	case NCSMDS_RED_UP:
	case NCSMDS_RED_DOWN:
	case NCSMDS_CHG_ROLE:
		if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC) &&
		    (inst->my_vcard == cbinfo->info.svc_evt.i_dest)) {
			if (m_NCS_NODE_ID_FROM_MDS_DEST(inst->my_vcard) == 0) {
				/* This comes only for VDEST OAAs */
				if (cbinfo->info.svc_evt.i_anc == inst->my_anc) {
					/* A self-event is detected. */
					post_to_mbx = TRUE;
				}
			}
		}
		break;

	default:
		break;
	}

	if (post_to_mbx) {
		mm = m_MMGR_ALLOC_MAB_MSG;
		if (mm == NULL) {
			ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
			return m_MAB_DBG_SINK(NCSCC_RC_OUT_OF_MEM);
		}
		memset(mm, '\0', sizeof(MAB_MSG));
		mm->yr_hdl = (NCSCONTEXT)(long)cbinfo->i_yr_svc_hdl;
		mm->fr_card = cbinfo->info.svc_evt.i_dest;
		mm->fr_svc = cbinfo->info.svc_evt.i_svc_id;
		mm->fr_anc = cbinfo->info.svc_evt.i_anc;
		mm->data.data.oac_mds_svc_evt.change = cbinfo->info.svc_evt.i_change;
		mm->data.data.oac_mds_svc_evt.role = cbinfo->info.svc_evt.i_role;
		mm->data.data.oac_mds_svc_evt.anc = cbinfo->info.svc_evt.i_anc;
		mm->data.data.oac_mds_svc_evt.svc_pwe_hdl = (NCSCONTEXT)(long)cbinfo->info.svc_evt.svc_pwe_hdl;
		mm->op = MAB_OAC_SVC_MDS_EVT;

		/* Put it in OAA's work queue */
		status = m_OAC_SND_MSG(inst->mbx, mm);
		if (status != NCSCC_RC_SUCCESS) {
			m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL,
					   MAB_OAC_ERR_MDS_TO_MBX_SND_FAILED, status, mm->op);
			m_MMGR_FREE_MAB_MSG(mm);
			ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
			return m_MAB_DBG_SINK(status);
		}
	}

	ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          oac_mds_cb                                                * 
*                                                                            *
*  Description:   Callback registered by OAA for MDS interface               * 
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO - Holds the info from MDS thread      *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 oac_mds_cb(NCSMDS_CALLBACK_INFO *cbinfo)
{
	uns32 status = NCSCC_RC_SUCCESS;

	if (cbinfo == NULL) {
		m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_CBINFO_NULL, 0, 0);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	switch (cbinfo->i_op) {
	case MDS_CALLBACK_COPY:
		cbinfo->info.cpy.o_msg_fmt_ver = cbinfo->info.cpy.i_rem_svc_pvt_ver;
		status = mab_mds_cpy(cbinfo->i_yr_svc_hdl,
				     cbinfo->info.cpy.i_msg,
				     cbinfo->info.cpy.i_to_svc_id, &cbinfo->info.cpy.o_cpy, cbinfo->info.cpy.i_last);
		break;

	case MDS_CALLBACK_ENC:
	case MDS_CALLBACK_ENC_FLAT:
		if (cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_MAS)
			cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
									       OAC_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT,
									       OAC_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT,
									       oac_mas_msg_fmt_table);
		else if (cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_MAC)
			cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
									       OAC_WRT_MAC_SUBPART_VER_AT_MIN_MSG_FMT,
									       OAC_WRT_MAC_SUBPART_VER_AT_MAX_MSG_FMT,
									       oac_mac_msg_fmt_table);
		else if (cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_PSS)
			cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
									       OAC_WRT_PSS_SUBPART_VER_AT_MIN_MSG_FMT,
									       OAC_WRT_PSS_SUBPART_VER_AT_MAX_MSG_FMT,
									       oac_pss_msg_fmt_table);
		if (cbinfo->info.enc.o_msg_fmt_ver == 0) {
			/* log the error */
			m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, "MAB_HDLN_OAC_LOG_MDS_ENC_FAILURE",
					  cbinfo->info.enc.i_to_svc_id, cbinfo->info.enc.o_msg_fmt_ver);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		status = mab_mds_enc(cbinfo->i_yr_svc_hdl,
				     cbinfo->info.enc.i_msg,
				     cbinfo->info.enc.i_to_svc_id,
				     cbinfo->info.enc.io_uba, cbinfo->info.enc.o_msg_fmt_ver);
		break;

	case MDS_CALLBACK_DEC:
	case MDS_CALLBACK_DEC_FLAT:

		if (cbinfo->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_MAS) {
			if (0 == m_NCS_MSG_FORMAT_IS_VALID(cbinfo->info.dec.i_msg_fmt_ver,
							   OAC_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT,
							   OAC_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT,
							   oac_mas_msg_fmt_table)) {
				/* log the error */
				m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_LOG_MDS_DEC_FAILURE,
						  cbinfo->info.dec.i_fr_svc_id, cbinfo->info.dec.i_msg_fmt_ver);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		} else if (cbinfo->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_PSS) {
			if (0 == m_NCS_MSG_FORMAT_IS_VALID(cbinfo->info.dec.i_msg_fmt_ver,
							   OAC_WRT_PSS_SUBPART_VER_AT_MIN_MSG_FMT,
							   OAC_WRT_PSS_SUBPART_VER_AT_MAX_MSG_FMT,
							   oac_pss_msg_fmt_table)) {
				/* log the error */
				m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_LOG_MDS_DEC_FAILURE,
						  cbinfo->info.dec.i_fr_svc_id, cbinfo->info.dec.i_msg_fmt_ver);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
		status = mab_mds_dec(cbinfo->i_yr_svc_hdl,
				     &cbinfo->info.dec.o_msg,
				     cbinfo->info.dec.i_fr_svc_id,
				     cbinfo->info.dec.io_uba, cbinfo->info.dec.i_msg_fmt_ver);
		break;

	case MDS_CALLBACK_RECEIVE:
		status = oac_mds_rcv(cbinfo->i_yr_svc_hdl,
				     cbinfo->info.receive.i_msg,
				     cbinfo->info.receive.i_fr_dest,
				     cbinfo->info.receive.i_fr_svc_id,
				     cbinfo->info.receive.i_to_dest, cbinfo->info.receive.i_to_svc_id);
		break;

	case MDS_CALLBACK_SVC_EVENT:
		status = oac_mds_evt_cb(cbinfo);
		break;

	case MDS_CALLBACK_QUIESCED_ACK:
		break;

	default:
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_OAC_ERR_MDS_CB_INVALID_OP, cbinfo->i_op);
		status = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;
	}

	return status;
}

#endif   /* (NCS_MAB == 1) */
