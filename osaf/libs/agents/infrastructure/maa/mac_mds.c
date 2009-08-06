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

  This file contains MAC specific MDS routines, being:

  mac_mds_rcv - rcv something from MDS lower layer.
  mac_mds_evt - subscription event entry point
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_MAB == 1)

#include "mab.h"
#include "mac.h"
#include "ncs_util.h"

MDS_CLIENT_MSG_FORMAT_VER mac_mas_msg_fmt_table[MAA_WRT_MAS_SUBPART_VER_RANGE] = { 1, 2 };
MDS_CLIENT_MSG_FORMAT_VER mac_oac_msg_fmt_table[MAA_WRT_OAC_SUBPART_VER_RANGE] = { 1, 2 };

/****************************************************************************\
*  Name:          mac_mds_rcv                                                * 
*                                                                            *
*  Description:   Posts a message to MAA mailbox from MDS thread.            * 
*                                                                            *
*  Arguments:     NCSCONTEXT - MAA CB, registered with MDS.                  *
*                 NCSCONTEXT - Pinter to the decoded MAB_MSG*                *
*                 MDS_DEST   - fr_card and to_card                           *
*                 SS_SVC_ID  - Service ids                                   *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
\****************************************************************************/
static uns32
mac_mds_rcv(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
	    MDS_DEST fr_card, SS_SVC_ID fr_svc, MDS_DEST to_card, SS_SVC_ID to_svc)
{
	uns32 status;
	MAC_INST *inst = (MAC_INST *)ncshm_take_hdl(NCS_SERVICE_ID_MAB, (uns32)yr_svc_hdl);

	if (inst == NULL) {
		m_LOG_MAB_NO_CB("mac_mds_rcv()");
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (inst) {
		m_LOG_MAB_SVC_PRVDR_MSG(NCSFL_SEV_DEBUG, MAB_SP_MAC_MDS_RCV_MSG, fr_svc, fr_card, ((MAB_MSG *)msg)->op);
	}

	/* plant MAB subcomponent's control block in MAB_MSG */
	((MAB_MSG *)msg)->yr_hdl = inst;

	/* Put it in MAC's work queue */
	status = m_MAC_SND_MSG(inst->mbx, msg);
	if (status != NCSCC_RC_SUCCESS) {
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL,
				   MAB_MAC_ERR_MDS_TO_MBX_SND_FAILED, status, ((MAB_MSG *)msg)->op);
		ncshm_give_hdl((uns32)yr_svc_hdl);
		return m_MAB_DBG_SINK(status);
	}

	ncshm_give_hdl((uns32)yr_svc_hdl);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          mac_mds_evt_cb                                             * 
*                                                                            *
*  Description:   processes MDS UP/DOWN events of MAS and PSS                * 
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO - Info about MAS and PSS from MDS     *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
\****************************************************************************/
static uns32 mac_mds_evt_cb(NCSMDS_CALLBACK_INFO *cbinfo)
{
	MDS_CLIENT_HDL yr_svc_hdl = cbinfo->i_yr_svc_hdl;
	MAC_INST *inst = (MAC_INST *)ncshm_take_hdl(NCS_SERVICE_ID_MAB, (uns32)yr_svc_hdl);

	if (inst == NULL) {
		m_LOG_MAB_NO_CB("mac_mds_evt_cb()");
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_MAC_LK_INIT;

	m_MAC_LK(&inst->lock, NCS_LOCK_WRITE);
	m_LOG_MAB_LOCK(MAB_LK_MAC_LOCKED, &inst->lock);

	m_LOG_MAB_SVC_PRVDR_EVT(NCSFL_SEV_DEBUG, MAB_SP_MAC_MDS_RCV_EVT,
				cbinfo->info.svc_evt.i_svc_id,
				cbinfo->info.svc_evt.i_dest, cbinfo->info.svc_evt.i_change, cbinfo->info.svc_evt.i_anc);

	switch (cbinfo->info.svc_evt.i_change) {	/* Review change type */
	case NCSMDS_DOWN:
	case NCSMDS_NO_ACTIVE:
		{
			if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_MAS) {
				inst->mas_here = FALSE;
			}

			if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_PSS) {
				inst->psr_here = FALSE;

				if (inst->playback_session == TRUE) {
					/* If playback session is already in progress, clear the 
					 * information, since this will be treated as an error 
					 * condition.
					 */
					inst->playback_session = FALSE;
					if (inst->plbck_tbl_list != NULL) {
						m_MMGR_FREE_MAC_TBL_BUFR(inst->plbck_tbl_list);
						inst->plbck_tbl_list = NULL;
					}
				}
			}

			break;
		}

	case NCSMDS_UP:
	case NCSMDS_NEW_ACTIVE:
		{
			if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_MAS) &&
			    (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY)) {
				if ((FALSE == inst->mas_here)
				    && (inst->mas_sync_sel.raise_obj != 0 || inst->mas_sync_sel.rmv_obj != 0)) {
					/* below statement is purposefully added.indicating on FD may cause mac thread got switched. */
					/* So, it is better to set that MAS is UP in the system from nowitself. */
					inst->mas_here = TRUE;
					m_NCS_SEL_OBJ_IND(inst->mas_sync_sel);
				}
				inst->mas_here = TRUE;
				inst->mas_vcard = cbinfo->info.svc_evt.i_dest;
			}

			if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_PSS) &&
			    (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY)) {
				inst->psr_here = TRUE;
				inst->psr_vcard = cbinfo->info.svc_evt.i_dest;
				if (inst->playback_session == TRUE) {
					/* If playback session is already in progress, clear the 
					 * information, since this will be treated as an error 
					 * condition.
					 */
					inst->playback_session = FALSE;
					if (inst->plbck_tbl_list != NULL) {
						m_MMGR_FREE_MAC_TBL_BUFR(inst->plbck_tbl_list);
						inst->plbck_tbl_list = NULL;
					}
				}
			}
			break;
		}

	default:
		break;
	}

	m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);
	m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);

	ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          mac_mds_cb                                                 * 
*                                                                            *
*  Description:   MAA's MDS SE callback function                             * 
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO - Information from MDS thread for MAA *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 mac_mds_cb(NCSMDS_CALLBACK_INFO *cbinfo)
{
	uns32 status = NCSCC_RC_SUCCESS;

	if (cbinfo == NULL) {
		m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_MAC_MDS_CBINFO_NULL, 0, 0);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	switch (cbinfo->i_op) {
	case MDS_CALLBACK_COPY:
		status = mab_mds_cpy(cbinfo->i_yr_svc_hdl,
				     cbinfo->info.cpy.i_msg,
				     cbinfo->info.cpy.i_to_svc_id, &cbinfo->info.cpy.o_cpy, cbinfo->info.cpy.i_last);
		break;

	case MDS_CALLBACK_ENC:
	case MDS_CALLBACK_ENC_FLAT:
		cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
								       MAA_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT,
								       MAA_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT,
								       mac_mas_msg_fmt_table);
		if (cbinfo->info.enc.o_msg_fmt_ver == 0) {
			/* log the error */
			m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, MAB_HDLN_MAC_LOG_MDS_ENC_FAILURE,
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
							   MAA_WRT_MAS_SUBPART_VER_AT_MIN_MSG_FMT,
							   MAA_WRT_MAS_SUBPART_VER_AT_MAX_MSG_FMT,
							   mac_mas_msg_fmt_table)) {
				/* log the error */
				m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, MAB_HDLN_MAC_LOG_MDS_DEC_FAILURE,
						  cbinfo->info.dec.i_fr_svc_id, cbinfo->info.dec.i_msg_fmt_ver);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		} else if (cbinfo->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_OAC) {
			if (0 == m_NCS_MSG_FORMAT_IS_VALID(cbinfo->info.dec.i_msg_fmt_ver,
							   MAA_WRT_OAC_SUBPART_VER_AT_MIN_MSG_FMT,
							   MAA_WRT_OAC_SUBPART_VER_AT_MAX_MSG_FMT,
							   mac_oac_msg_fmt_table)) {
				/* log the error */
				m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, MAB_HDLN_MAC_LOG_MDS_DEC_FAILURE,
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
		status = mac_mds_rcv(cbinfo->i_yr_svc_hdl,
				     cbinfo->info.receive.i_msg,
				     cbinfo->info.receive.i_fr_dest,
				     cbinfo->info.receive.i_fr_svc_id,
				     cbinfo->info.receive.i_to_dest, cbinfo->info.receive.i_to_svc_id);
		break;

	case MDS_CALLBACK_SVC_EVENT:
		status = mac_mds_evt_cb(cbinfo);
		break;

	default:
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_MDS_CB_INVALID_OP, cbinfo->i_op);
		status = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;
	}

	return status;
}

#endif   /* (NCS_MAB == 1) */
