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

    This has the private functions of the Object Access Client (PSS), 
    a subcomponent of the MIB Access Broker (MAB) subystem.This file 
    contains these groups of private functions

  - The master PSS dispatch loop functions
  - The PSS Table Record Manipulation Routines
  - The PSS Filter Manipulation routines  
  - PSS message handling routines 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/
#if (NCS_MAB == 1)
#if (NCS_PSR == 1)
#include "psr.h"
#include "psr_rfmt.h"

uns32 pss_stdby_oaa_down_list_update(MDS_DEST oaa_addr, NCSCONTEXT yr_hdl, PSS_STDBY_OAA_DOWN_BUFFER_OP buffer_op);
void pss_stdby_oaa_down_list_dump(PSS_STDBY_OAA_DOWN_BUFFER_NODE *pss_stdby_oaa_down_buffer, FILE *fh);

/*****************************************************************************

  The master PSS dispatch loop functions

  OVERVIEW:   The PSS instance or instances has one or more dedicated 
              threads. Each sits in an inifinite loop governed by these
              functions.

  PROCEDURE NAME: pss_do_evts & pss_do_evt

  DESCRIPTION:       

     pss_do_evts      Infinite loop services the passed SYSF_MBX
     pss_do_evt       Master Dispatch function and services off PSS work queue

*****************************************************************************/

/*****************************************************************************
   pss_do_evts
*****************************************************************************/
uns32 pss_do_evts(SYSF_MBX *mbx)
{
	MAB_MSG *msg;

	if ((msg = m_PSS_RCV_MSG(mbx, msg)) != NULL) {
		if (pss_do_evt(msg, TRUE) != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   pss_do_evt
*****************************************************************************/
uns32 pss_do_evt(MAB_MSG *msg, NCS_BOOL free_msg)
{

	NCSCONTEXT yr_svc_hdl = msg->yr_hdl;
	NCSCONTEXT validate_hdl = NULL;
	uns32 status = NCSCC_RC_SUCCESS;

#if (NCS_PSS_RED == 1)
	/*  A NULL STATE check has been added */
	if (((gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_STANDBY) ||
	     (gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_NULL)) && (free_msg == TRUE)) {
		/* Need allow all events to Standy since the same function is being used
		   in the MBCSv callback. */
		pss_free_mab_msg(msg, free_msg);	/* Composite free call for PSS messages */
		return status;
	}
#endif

	/* Do not validate if the message type is AMF initialization retry, 
	   as the MAB message wont contain any Handle */
	if (msg->op != MAB_PSS_AMF_INIT_RETRY) {
		/* Validate the Handle */
		validate_hdl = (NCSCONTEXT)m_PSS_VALIDATE_HDL((long)yr_svc_hdl);

		/* Check if the handle is Invalid. log the Error and 
		   return failure */
		if (validate_hdl == NULL) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);

			pss_free_mab_msg(msg, free_msg);

			return NCSCC_RC_FAILURE;
		}

		/* Now the Handle is valid thus set the Handle to point Control Block */
		msg->yr_hdl = validate_hdl;

		/* Release the Handle, we need not to keep this handle */
		ncshm_give_hdl((long)yr_svc_hdl);
	}

	switch (msg->op) {
	case MAB_PSS_TBL_BIND:
		status = pss_process_tbl_bind(msg);
		break;

	case MAB_PSS_TBL_UNBIND:
		status = pss_process_tbl_unbind(msg);
		break;

	case MAB_PSS_SET_REQUEST:
		status = pss_process_snmp_request(msg);
		break;

	case MAB_PSS_WARM_BOOT:
		status = pss_process_oac_warmboot(msg);
		break;

	case MAB_PSS_MIB_REQUEST:
		status = pss_process_mib_request(msg);
		break;

	case MAB_PSS_SVC_MDS_EVT:
		status = pss_handle_svc_mds_evt(msg);
		break;

	case MAB_PSS_RE_RESUME_ACTIVE_ROLE_FUNCTIONALITY:
		status = pss_resume_active_role_activity(msg);
		break;

	case MAB_PSS_BAM_CONF_DONE:
		status = pss_process_bam_conf_done(msg);
		break;

	case MAB_PSS_AMF_INIT_RETRY:
		{
			SaAisErrorT saf_status = SA_AIS_OK;
			status = pss_amf_initialize(&gl_pss_amf_attribs.amf_attribs, &saf_status);
			if (saf_status != SA_AIS_ERR_TRY_AGAIN) {
				/*  Destroy the selection object if AMF componentization was
				   successful. In case of RETRY from AMF, go back and listen on the object. */
				m_NCS_SEL_OBJ_RMV_IND(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, TRUE, TRUE);
				m_NCS_SEL_OBJ_DESTROY(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj);
				gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.raise_obj = 0;
				gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.rmv_obj = 0;
			}
		}
		break;

	case MAB_PSS_VDEST_ROLE_QUIESCED:
#if (NCS_PSS_RED == 1)
		status = pss_process_vdest_role_quiesced(msg);
#endif
		break;

	default:
		status = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;
	}

#if (NCS_PSS_RED == 1)
	if (gl_pss_amf_attribs.ha_state != NCS_APP_AMF_HA_STATE_STANDBY)
#endif
	{
		/* Check if in non-active state, do we need to free any message??? */
		pss_free_mab_msg(msg, free_msg);	/* Composite free call for PSS messages */
	}

	return status;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_handle_svc_mds_evt

  DESCRIPTION:       Process the MDS SVC UP/DOWN events.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_handle_svc_mds_evt(MAB_MSG *msg)
{
	PSS_PWE_CB *pwe_cb = NULL;
	uns32 status = NCSCC_RC_SUCCESS;

	pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return NCSCC_RC_FAILURE;
	}

	if (msg->data.data.pss_mds_svc_evt.role != NCSFT_ROLE_PRIMARY) {
		/* Even in Standby-role, PSS is allowed to update checkpoint information
		   received from Active via MBCSv through this function call. */
		if ((msg->fr_svc == NCSMDS_SVC_ID_OAC) &&
		    ((msg->data.data.pss_mds_svc_evt.change == NCSMDS_DOWN) ||
		     (msg->data.data.pss_mds_svc_evt.change == NCSMDS_NO_ACTIVE))) {
			if (pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_STANDBY) {
				char addr_str[255] = { 0 };
				/* need to pass PWE Control block handle  */
				status =
				    pss_stdby_oaa_down_list_update(msg->fr_card, (NCSCONTEXT)((long)pwe_cb->hm_hdl),
								   PSS_STDBY_OAA_DOWN_BUFFER_DELETE);
				if (NCSCC_RC_SUCCESS != status) {
					if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
						sprintf(addr_str, "VDEST:%d",
							m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card));
					else
						sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
							m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0,
							(uns32)(msg->fr_card));
					ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_HDLN_C, PSS_FC_HDLN, NCSFL_LC_HEADLINE,
						   NCSFL_SEV_NOTICE, NCSFL_TYPE_TIC,
						   PSS_HDLN_STDBY_OAA_DOWN_LIST_DELETE_FAILED, addr_str);
				} else {
					if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
						sprintf(addr_str, "VDEST:%d",
							m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card));
					else
						sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
							m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0,
							(uns32)(msg->fr_card));
					ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_HDLN_C, PSS_FC_HDLN, NCSFL_LC_HEADLINE,
						   NCSFL_SEV_NOTICE, NCSFL_TYPE_TIC,
						   PSS_HDLN_STDBY_OAA_DOWN_LIST_DELETE_SUCCESS, addr_str);
				}
			}
			status = pss_handle_oaa_down_event(pwe_cb, &msg->fr_card);
			return status;
		}
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN2_IGNORING_NON_PRIMARY_MDS_SVC_EVT);
		return NCSCC_RC_SUCCESS;
	}

	switch (msg->fr_svc) {
	case NCSMDS_SVC_ID_OAC:
		if ((msg->data.data.pss_mds_svc_evt.change == NCSMDS_DOWN) ||
		    (msg->data.data.pss_mds_svc_evt.change == NCSMDS_NO_ACTIVE)) {
			status = pss_handle_oaa_down_event(pwe_cb, &msg->fr_card);
		}
		break;

	case NCSMDS_SVC_ID_MAS:
		switch (msg->data.data.pss_mds_svc_evt.change) {
		case NCSMDS_DOWN:
			pwe_cb->is_mas_alive = FALSE;
			m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MAS_MDS_DOWN);
			break;

		case NCSMDS_NO_ACTIVE:
			/* Treating this as DOWN event */
			pwe_cb->is_mas_alive = FALSE;
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN2_MAS_MDS_NO_ACTIVE);
			break;

		case NCSMDS_NEW_ACTIVE:
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN2_MAS_MDS_NEW_ACTIVE);
			if (pwe_cb->is_mas_alive == FALSE) {
				/* Treating this as UP event */
				pwe_cb->is_mas_alive = TRUE;
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN2_MAS_MDS_NEW_ACTIVE_AS_UP_EVT);

				/* If some services wanted playback to be done from the BAM,
				   forward the list of those PCNs to the BAM now. */
				if (pwe_cb->p_pss_cb->is_bam_alive == TRUE) {
					(void)pss_send_pending_wbreqs_to_bam(pwe_cb);
				}
			}
			break;

		case NCSMDS_UP:
			pwe_cb->is_mas_alive = TRUE;
			m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_MAS_MDS_UP);

			/* If some services wanted playback to be done from the BAM, 
			   forward the list of those PCNs to the BAM now. */
			if (pwe_cb->p_pss_cb->is_bam_alive == TRUE) {
				(void)pss_send_pending_wbreqs_to_bam(pwe_cb);
			}
			break;

		default:
			break;
		}
		break;

	case NCSMDS_SVC_ID_BAM:
		switch (msg->data.data.pss_mds_svc_evt.change) {
		case NCSMDS_DOWN:
			pwe_cb->p_pss_cb->is_bam_alive = FALSE;
			memset(&pwe_cb->p_pss_cb->bam_address, 0, sizeof(MDS_DEST));
			m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_BAM_MDS_DOWN);
			break;

		case NCSMDS_NO_ACTIVE:
			/* Treating this as DOWN event */
			pwe_cb->p_pss_cb->is_bam_alive = FALSE;
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN2_BAM_MDS_NO_ACTIVE);
			break;

		case NCSMDS_NEW_ACTIVE:
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN2_BAM_MDS_NEW_ACTIVE);
			if (pwe_cb->p_pss_cb->is_bam_alive == FALSE) {
				/* Treating this as UP event */
				pwe_cb->p_pss_cb->is_bam_alive = TRUE;
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN2_BAM_MDS_NEW_ACTIVE_AS_UP_EVT);

				pwe_cb->p_pss_cb->bam_address = msg->fr_card;
				/* Since BAM is a local PWE specific service, right now!!! */
				pwe_cb->bam_pwe_hdl = msg->data.data.pss_mds_svc_evt.svc_pwe_hdl;

				/* If some services wanted playback to be done from the BAM,
				   forward the list of those PCNs to the BAM now. */
				if (pwe_cb->is_mas_alive == TRUE) {
					(void)pss_send_pending_wbreqs_to_bam(pwe_cb);
				}
			}
			break;

		case NCSMDS_UP:
			pwe_cb->p_pss_cb->is_bam_alive = TRUE;
			m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_BAM_MDS_UP);

			pwe_cb->p_pss_cb->bam_address = msg->fr_card;

			/* Since BAM is a local PWE specific service, right now!!! */
			pwe_cb->bam_pwe_hdl = msg->data.data.pss_mds_svc_evt.svc_pwe_hdl;

			/* If some services wanted playback to be done from the BAM, 
			   forward the list of those PCNs to the BAM now. */
			if (pwe_cb->is_mas_alive == TRUE) {
				(void)pss_send_pending_wbreqs_to_bam(pwe_cb);
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return status;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_snmp_request

  DESCRIPTION:       Process the successful requests received from the OAC

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_snmp_request(MAB_MSG *msg)
{
	PSS_CB *inst;
	PSS_PWE_CB *pwe_cb;
	uns32 retval, tbl_id;
	PSS_MIB_TBL_INFO *tbl_info;
	USRBUF *ub = NULL;

	pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	inst = pwe_cb->p_pss_cb;
	if (inst == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_INST);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (msg->data.data.snmp == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_SNMP);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	tbl_id = msg->data.data.snmp->i_tbl_id;

	retval = pss_send_ack_for_msg_to_oaa(pwe_cb, msg);

	if (NCSCC_RC_SUCCESS != retval) {
		char addr_str[255] = { 0 };
		uns16 num_of_char = 0;

		if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
			num_of_char = snprintf(addr_str, (size_t)255, "VDEST:%d, SEQ No.: %d ",
					       m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card), msg->data.seq_num);
		else
			num_of_char =
			    snprintf(addr_str, (size_t)255,
				     "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%llu, SEQ No.: %d ",
				     m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0, msg->fr_card, msg->data.seq_num);

		if (num_of_char >= 255)
			addr_str[255 - 1] = '\0';

		m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_OAA_ACK_FAILED, addr_str);

		return retval;
	}

	/* This table has not been registered */
	if ((tbl_id >= MIB_UD_TBL_ID_END) || (inst->mib_tbl_desc[tbl_id] == NULL)) {
		switch (msg->data.data.snmp->i_op) {
		case NCSMIB_OP_RSP_SETROW:
			ub = msg->data.data.snmp->rsp.info.setrow_rsp.i_usrbuf;
			break;
		case NCSMIB_OP_RSP_REMOVEROWS:
			ub = msg->data.data.snmp->rsp.info.removerows_rsp.i_usrbuf;
			break;
		case NCSMIB_OP_RSP_SETALLROWS:
			ub = msg->data.data.snmp->rsp.info.setallrows_rsp.i_usrbuf;
			break;
		case NCSMIB_OP_RSP_MOVEROW:
			ub = msg->data.data.snmp->rsp.info.moverow_rsp.i_usrbuf;
			break;
		default:
			ub = NULL;
			break;
		}

		if (ub != NULL)
			m_MMGR_FREE_BUFR_LIST(ub);

		ncsmib_arg_free_resources(msg->data.data.snmp, FALSE);

		if (tbl_id >= MIB_UD_TBL_ID_END)
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_TBL_ID_TOO_LARGE, pwe_cb->pwe_id, tbl_id);
		else
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_NO_TBL_REG, pwe_cb->pwe_id, tbl_id);

		/* For PSS tables, do not call into m_MAB_DBG_SINK */
		if ((tbl_id == NCSMIB_TBL_PSR_PROFILES) || (tbl_id == NCSMIB_SCLR_PSR_TRIGGER))
			return NCSCC_RC_FAILURE;

		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	tbl_info = inst->mib_tbl_desc[tbl_id];

	switch (msg->data.data.snmp->i_op) {
	case NCSMIB_OP_RSP_SET:
		retval = pss_process_set_request(msg);
		break;

	case NCSMIB_OP_RSP_SETROW:
		retval = pss_process_setrow_request(msg, FALSE);
		ub = msg->data.data.snmp->rsp.info.setrow_rsp.i_usrbuf;
		break;

	case NCSMIB_OP_RSP_MOVEROW:
		retval = pss_process_setrow_request(msg, TRUE);
		ub = msg->data.data.snmp->rsp.info.moverow_rsp.i_usrbuf;
		break;

	case NCSMIB_OP_RSP_REMOVEROWS:
		retval = pss_process_removerows_request(msg);
		ub = msg->data.data.snmp->rsp.info.removerows_rsp.i_usrbuf;
		break;

	case NCSMIB_OP_RSP_SETALLROWS:
		retval = pss_process_setallrows_request(msg);
		ub = msg->data.data.snmp->rsp.info.setallrows_rsp.i_usrbuf;
		break;

	default:
		ncsmib_arg_free_resources(msg->data.data.snmp, FALSE);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_REQUEST_PROCESS_ERR);
		if (ub != NULL)	/* This is un-consumed, only in the case of failure */
			m_MMGR_FREE_BUFR_LIST(ub);
	}

	ncsmib_arg_free_resources(msg->data.data.snmp, FALSE);

	if (inst->mem_in_store > NCS_PSS_MAX_IN_STORE_MEM_SIZE) {
		pss_save_current_configuration(inst);
		if (NCSCC_RC_SUCCESS != pss_save_current_configuration(inst)) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_REQUEST_PROCESS_ERR);
			return NCSCC_RC_FAILURE;
		}
	}
	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_oac_warmboot

  DESCRIPTION:       Send SNMP Set requests to the client after it has
                     requested for the playback. This function works in two
                     modes, one is ACTIVE(A) and other being 
                     STANDBY/QUIESCED-to-ACTIVE(SQ2A). The mode is understood
                     by looking at the pwe_cb->processing_pending_active_events
                     flag. If it is TRUE, it is SQ2A-mode, else it is A-mode.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_oac_warmboot(MAB_MSG *msg)
{
	PSS_CB *inst;
	PSS_PWE_CB *pwe_cb = NULL;
	MAB_PSS_WARMBOOT_REQ *wbreq = NULL;
	PSS_CLIENT_ENTRY *client_node = NULL;
	char *p_pcn = NULL;
	uns32 retval = NCSCC_RC_SUCCESS;
	MAB_PSS_TBL_LIST *tbl_el = NULL;
	NCS_UBAID lcl_uba;

	pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		/* Send EOP to all requests that are queued here. */
		pss_send_eop_status_to_oaa(msg, NCSCC_RC_FAILURE,
					   msg->data.data.oac_pss_warmboot_req.mib_key,
					   msg->data.data.oac_pss_warmboot_req.wbreq_hdl, TRUE);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	inst = pwe_cb->p_pss_cb;
	if (inst == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_INST);
		/* Send EOP to all requests that are queued here. */
		pss_send_eop_status_to_oaa(msg, NCSCC_RC_FAILURE,
					   msg->data.data.oac_pss_warmboot_req.mib_key,
					   msg->data.data.oac_pss_warmboot_req.wbreq_hdl, TRUE);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_ENTER_WBREQ_HNDLR_FUNC);
	memset(&lcl_uba, '\0', sizeof(lcl_uba));

#if (NCS_PSS_RED == 1)
	if (pwe_cb->p_pss_cb->ha_state != SA_AMF_HA_STANDBY) {
		/* Write this request information into pwe_cb the first time. */
		if (pwe_cb->processing_pending_active_events == FALSE) {
			/* A-mode. Store a copy of warmboot-requests into the pwe_cb. */
			pss_updt_in_wbreq_into_cb(pwe_cb, &msg->data.data.oac_pss_warmboot_req);
		} else {
			/* S2A mode. Generate a copy of the warmboot-requests from the pwe_cb. */
			pss_gen_wbreq_from_cb(&pwe_cb->curr_plbck_ssn_info, &msg->data.data.oac_pss_warmboot_req);
		}
	}

	m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);	/* Send async-update to Standby */
#endif

	/* This function works both in A-mode and SQ2A-mode. */
	retval = pss_send_ack_for_msg_to_oaa(pwe_cb, msg);

	if (NCSCC_RC_SUCCESS != retval) {
		char addr_str[255] = { 0 };
		uns16 num_of_char = 0;

		if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
			num_of_char = snprintf(addr_str, (size_t)255, "VDEST:%d, SEQ No.: %d ",
					       m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card), msg->data.seq_num);
		else
			num_of_char =
			    snprintf(addr_str, (size_t)255,
				     "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%llu, SEQ No.: %d ",
				     m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0, msg->fr_card, msg->data.seq_num);

		if (num_of_char >= 255)
			addr_str[255 - 1] = '\0';

		m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_OAA_ACK_FAILED, addr_str);

		return retval;
	}

	/* Just log the info */
	for (wbreq = &msg->data.data.oac_pss_warmboot_req; wbreq != NULL; wbreq = wbreq->next) {
		p_pcn = wbreq->pcn_list.pcn;
		if ((p_pcn == NULL) || (p_pcn[0] == '\0')) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVLD_PCN_IN_WBREQ_TERMINATING_SESSION);
			/* Send EOP to all requests that are queued here. */
			pss_send_eop_status_to_oaa(msg, NCSCC_RC_FAILURE,
						   msg->data.data.oac_pss_warmboot_req.mib_key,
						   msg->data.data.oac_pss_warmboot_req.wbreq_hdl, TRUE);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		if (wbreq->pcn_list.tbl_list == NULL) {
			m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_NOTICE, PSS_WBREQ_RCVD_INFO,
						 wbreq->pcn_list.pcn, pwe_cb->pwe_id, wbreq->is_system_client, 0);
		} else {
			for (tbl_el = wbreq->pcn_list.tbl_list; tbl_el != NULL; tbl_el = tbl_el->next) {
				if (pwe_cb->p_pss_cb->mib_tbl_desc[tbl_el->tbl_id] == NULL) {
					m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_NOTICE, PSS_WBREQ_RCVD_TBL_IS_NOT_PERSISTENT,
								 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
								 wbreq->is_system_client, tbl_el->tbl_id);
					continue;
				} else {
					m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_NOTICE, PSS_WBREQ_RCVD_INFO,
								 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
								 wbreq->is_system_client, tbl_el->tbl_id);
				}
			}
		}
	}

#if (NCS_PSS_RED == 1)
	if (pwe_cb->p_pss_cb->ha_state != SA_AMF_HA_STANDBY)
#endif
	{
		for (wbreq = &msg->data.data.oac_pss_warmboot_req; wbreq != NULL; wbreq = wbreq->next) {
			PSS_WBPLAYBACK_SORT_TABLE lcl_sort_db;
			PSS_SPCN_LIST *spcn_entry = NULL;
			NCS_BOOL snd_evt_to_bam = FALSE;

			m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_DEBUG, PSS_WBREQ_RCVD_PROCESS_START,
						 wbreq->pcn_list.pcn, pwe_cb->pwe_id, wbreq->is_system_client, 0);

			p_pcn = wbreq->pcn_list.pcn;
			if (wbreq->is_system_client) {
				spcn_entry = pss_findadd_entry_frm_spcnlist(inst, p_pcn, FALSE);

				if (spcn_entry != NULL) {
					m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_SPCN_ENTRY_FOUND);
				}
			}
#if (NCS_PSS_RED == 1)
			pwe_cb->curr_plbck_ssn_info.pcn = p_pcn;	/* Double-referencing the same pointer!!! */
			m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
#endif

			client_node = pss_find_client_entry(pwe_cb, p_pcn, FALSE);
			if (client_node == NULL) {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLIENT_ENTRY_NULL);
				m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_INFO, PSS_WBREQ_RCVD_PROCESS_ABORT,
							 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
							 wbreq->is_system_client, 0);
#if (NCS_PSS_RED == 1)
				if (pwe_cb->processing_pending_active_events == FALSE) {
					pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
				}
				pss_indicate_end_of_playback_to_standby(pwe_cb, TRUE);
#endif
				/* Send EOP only to this particular request. */
				pss_send_eop_status_to_oaa(msg, NCSCC_RC_FAILURE, wbreq->mib_key, wbreq->wbreq_hdl,
							   FALSE);
				continue;
			}
			m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_PCN_NODE_FOUND);

			memset(&lcl_sort_db, '\0', sizeof(lcl_sort_db));
			lcl_sort_db.sort_params.key_size = sizeof(PSS_WBSORT_KEY);
			if (ncs_patricia_tree_init(&lcl_sort_db.sort_db, &lcl_sort_db.sort_params)
			    != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_WBSORTDB_INIT_FAIL);
				m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_INFO, PSS_WBREQ_RCVD_PROCESS_ABORT,
							 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
							 wbreq->is_system_client, 0);
#if (NCS_PSS_RED == 1)
				if (pwe_cb->processing_pending_active_events == FALSE) {
					pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
				}
				pss_indicate_end_of_playback_to_standby(pwe_cb, TRUE);
#endif
				/* Send EOP to all requests that are queued here. */
				pss_send_eop_status_to_oaa(msg, NCSCC_RC_FAILURE, wbreq->mib_key, wbreq->wbreq_hdl,
							   TRUE);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			if (wbreq->pcn_list.tbl_list == NULL) {
				NCS_BOOL add_entry = FALSE;

				if (spcn_entry != NULL) {
					if (spcn_entry->plbck_frm_bam) {
						m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_DEBUG,
									 PSS_WBREQ_RCVD_PLBCK_OPTION_SET_TO_BAM,
									 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
									 wbreq->is_system_client, 0);
						pss_flush_mib_entries_from_curr_profile(pwe_cb, spcn_entry,
											client_node);
						pss_send_wbreq_to_bam(pwe_cb, spcn_entry->pcn);
						/* Send EOP only to this particular request, that request was forwarded to BAM. */
						pss_send_eop_status_to_oaa(msg, retval, wbreq->mib_key,
									   wbreq->wbreq_hdl, FALSE);
#if (NCS_PSS_RED == 1)
						if (pwe_cb->processing_pending_active_events == FALSE) {
							pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
						}
						pss_indicate_end_of_playback_to_standby(pwe_cb, FALSE);
#endif
						pss_destroy_wbsort_db(&lcl_sort_db);
						add_entry = FALSE;
						continue;
					} else {
						m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_DEBUG,
									 PSS_WBREQ_RCVD_PLBCK_OPTION_SET_TO_PSS,
									 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
									 wbreq->is_system_client, 0);
						add_entry = TRUE;
					}
				} else {
					add_entry = TRUE;
				}
				if (add_entry) {
					retval = pss_sort_wbreq_instore_tables_with_rank(pwe_cb,
											 &lcl_sort_db, client_node,
											 spcn_entry, &snd_evt_to_bam,
											 &lcl_uba);
					if ((spcn_entry != NULL) && (snd_evt_to_bam == TRUE)) {
						m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_FWDING_REQ_TO_BAM);
						pss_flush_mib_entries_from_curr_profile(pwe_cb, spcn_entry,
											client_node);
						pss_send_wbreq_to_bam(pwe_cb, spcn_entry->pcn);
						/* Send EOP only to this particular request, that request was forwarded to BAM. */
						pss_send_eop_status_to_oaa(msg, retval, wbreq->mib_key,
									   wbreq->wbreq_hdl, FALSE);
#if (NCS_PSS_RED == 1)
						if (pwe_cb->processing_pending_active_events == FALSE) {
							pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
						}
						pss_indicate_end_of_playback_to_standby(pwe_cb, FALSE);
#endif
						pss_destroy_wbsort_db(&lcl_sort_db);
						continue;
					}
				}
			} else {
				NCS_BOOL add_entry = FALSE;

				if (spcn_entry != NULL) {
					if (spcn_entry->plbck_frm_bam) {
						m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_DEBUG,
									 PSS_WBREQ_RCVD_PLBCK_OPTION_SET_TO_BAM,
									 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
									 wbreq->is_system_client, 0);
						pss_flush_mib_entries_from_curr_profile(pwe_cb, spcn_entry,
											client_node);
						pss_send_wbreq_to_bam(pwe_cb, spcn_entry->pcn);
						/* Send EOP only to this particular request, that request was forwarded to BAM. */
						pss_send_eop_status_to_oaa(msg, retval, wbreq->mib_key,
									   wbreq->wbreq_hdl, FALSE);
#if (NCS_PSS_RED == 1)
						if (pwe_cb->processing_pending_active_events == FALSE) {
							pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
						}
						pss_indicate_end_of_playback_to_standby(pwe_cb, FALSE);
#endif
						pss_destroy_wbsort_db(&lcl_sort_db);
						continue;
					} else {
						m_LOG_PSS_RCVD_WBREQ_EVT(NCSFL_SEV_DEBUG,
									 PSS_WBREQ_RCVD_PLBCK_OPTION_SET_TO_PSS,
									 wbreq->pcn_list.pcn, pwe_cb->pwe_id,
									 wbreq->is_system_client, 0);
						add_entry = TRUE;
					}
				} else {
					add_entry = TRUE;
				}
				if (add_entry == TRUE) {
					retval = pss_sort_wbreq_tables_with_rank(pwe_cb,
										 &lcl_sort_db, client_node,
										 spcn_entry, wbreq->pcn_list.tbl_list,
										 &snd_evt_to_bam, &lcl_uba);

					if ((spcn_entry != NULL) && (snd_evt_to_bam == TRUE)) {
						m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_FWDING_REQ_TO_BAM);
						pss_flush_mib_entries_from_curr_profile(pwe_cb, spcn_entry,
											client_node);
						pss_send_wbreq_to_bam(pwe_cb, spcn_entry->pcn);
						/* Send EOP only to this particular request, that request was forwarded to BAM. */
						pss_send_eop_status_to_oaa(msg, retval, wbreq->mib_key,
									   wbreq->wbreq_hdl, FALSE);
#if (NCS_PSS_RED == 1)
						if (pwe_cb->processing_pending_active_events == FALSE) {
							pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
						}
						pss_indicate_end_of_playback_to_standby(pwe_cb, FALSE);
#endif
						pss_destroy_wbsort_db(&lcl_sort_db);
						continue;
					}
				}
			}
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_WBPLBCK_SORT_OP_FAIL);
				/* Send EOP only to this particular request. */
				pss_send_eop_status_to_oaa(msg, retval, wbreq->mib_key, wbreq->wbreq_hdl, FALSE);
				pss_destroy_wbsort_db(&lcl_sort_db);
#if (NCS_PSS_RED == 1)
				if (pwe_cb->processing_pending_active_events == FALSE) {
					pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
				}
				pss_indicate_end_of_playback_to_standby(pwe_cb, TRUE);
#endif
				continue;
			} else {
				if ((snd_evt_to_bam == FALSE) || (spcn_entry == NULL)) {
					/* Send MDS broadcast for PLAYBACK_START event to all MAAs(OAAs don't need this.) */
					pss_signal_start_of_playback(pwe_cb, lcl_uba.start);

					retval =
					    pss_wb_playback_for_sorted_list(pwe_cb, &lcl_sort_db, client_node, p_pcn);
					/* Send EOP only to this particular request. */
					pss_send_eop_status_to_oaa(msg, retval, wbreq->mib_key, wbreq->wbreq_hdl,
								   FALSE);

#if (NCS_PSS_RED == 1)
					if (pwe_cb->processing_pending_active_events == FALSE) {
						pwe_cb->curr_plbck_ssn_info.pcn = NULL;	/* Since it's a double-referenced pointer!!! */
					}

					pss_indicate_end_of_playback_to_standby(pwe_cb, FALSE);
#endif

					/* Send MDS broadcast for PLAYBACK_END event to all MAAs(OAAs don't need this.) */
					pss_signal_end_of_playback(pwe_cb);

					/* m_LOG_PSS_PLBCK_INFO(NCSFL_SEV_INFO, PSS_PLBCK_DONE, wbreq->pcn_list.pcn, 
					   pwe_cb->pwe_id); */
				}
			}
		}		/* end of for () */
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_EXIT_WBREQ_HNDLR_FUNC);

	return NCSCC_RC_SUCCESS;
}

/*************************************************************************
 *         PSS SNMP Request Processing Routines                          *
 *************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    pss_process_set_request

  DESCRIPTION:       Process a SET request

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_set_request(MAB_MSG *msg)
{
	PSS_CB *inst;
	PSS_PWE_CB *pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	char *p_pcn = NULL;
	PSS_TBL_REC *tbl_rec = NULL;
	NCS_PATRICIA_TREE *pTree;
	NCS_PATRICIA_NODE *pNode;
	PSS_MIB_TBL_INFO *tbl_info;
	PSS_MIB_TBL_DATA *tbl_data;
	PSS_CLIENT_ENTRY *client_node;
	NCSMIB_ARG *arg;
	NCS_BOOL created_new = FALSE;
	uns32 tbl_id, retval;
	char addr_str[255] = { 0 };

	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	/* Ensure that a handle exists for this PWE */
	arg = msg->data.data.snmp;
	tbl_id = arg->i_tbl_id;

	inst = pwe_cb->p_pss_cb;

	m_LOG_PSS_SNMP_REQ(NCSFL_SEV_INFO, PSS_SNMP_REQ_SET, pwe_cb->pwe_id, arg->i_tbl_id);
	m_PSS_LOG_NCSMIB_ARG(arg);

	p_pcn = pss_get_pcn_from_mdsdest(pwe_cb, &msg->fr_card, tbl_id);
	if (p_pcn == NULL) {
		/* This MIB table or PCN is not available with PSS. */
		/* convert the MDS_DEST into a string */
		if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
			sprintf(addr_str, "From VDEST:%d,For Tbl:%d", m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card),
				tbl_id);
		else
			sprintf(addr_str, "From ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d,For Tbl:%d",
				m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0, (uns32)(msg->fr_card), tbl_id);

		/* now log the message */
		ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_NO_CB, PSS_FC_HDLN,
			   NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
			   NCSFL_TYPE_TIC, PSS_HDLN_SNMP_REQ_FRM_INV_MDS_DEST_RCVD, addr_str);

		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Get the MIB data for this client */
	client_node = pss_find_client_entry(pwe_cb, p_pcn, FALSE);
	if (client_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLIENT_ENTRY_NULL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Get the Patricia Tree for this table */
	tbl_rec = pss_find_table_tree(pwe_cb, client_node, NULL, tbl_id, FALSE, NULL);
	if (tbl_rec == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_REC_NOT_FND);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (tbl_rec->is_scalar) {
		tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];

		/* Read data from store(if already present). */
		if (pss_find_scalar_node(pwe_cb, tbl_rec, p_pcn, tbl_id, TRUE, &created_new) != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		if (tbl_rec->info.scalar.data == NULL)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		retval = pss_apply_changes_to_sclr_node(tbl_info, tbl_rec,
							created_new, &arg->rsp.info.set_rsp.i_param_val);
		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		if (inst->save_type == PSS_SAVE_TYPE_IMMEDIATE) {
			retval = pss_save_to_sclr_store(pwe_cb, tbl_rec, NULL, p_pcn, tbl_id);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		} else {
			/* Add PSS_TBL_REC to pwe_cb->refresh_tbl_list */
			if (pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) {
				retval = pss_save_to_sclr_store(pwe_cb, tbl_rec, NULL, p_pcn, tbl_id);
				if (retval != NCSCC_RC_SUCCESS)
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
	} else {
		if ((arg->i_idx.i_inst_len == 0) || (arg->i_idx.i_inst_ids == NULL)) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_ZERO_INST_LEN_OR_NULL_INST_IDS, pwe_cb->pwe_id,
					   arg->i_tbl_id);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];
		retval = pss_validate_index(tbl_info, &arg->i_idx);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_INDEX_VALIDATE_FAIL, pwe_cb->pwe_id,
					   arg->i_tbl_id);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		retval = pss_validate_set_mib_obj(tbl_info, &arg->rsp.info.set_rsp.i_param_val);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_PARAMVAL_VALIDATE_FAIL, pwe_cb->pwe_id,
					   arg->i_tbl_id);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		if (tbl_rec->info.other.tree_inited == FALSE) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_TBL_REC_NOT_INITED, pwe_cb->pwe_id,
					   arg->i_tbl_id);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		pTree = &tbl_rec->info.other.data;

		/* Get the patricia node containing the data for this inst ids */
		pNode = pss_find_inst_node(pwe_cb, pTree, p_pcn, tbl_id,
					   arg->i_idx.i_inst_len, (uns32 *)arg->i_idx.i_inst_ids, TRUE);
		if (pNode == NULL) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_ADD_ROW_FAIL, pwe_cb->pwe_id, arg->i_tbl_id);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		tbl_data = (PSS_MIB_TBL_DATA *)pNode;
		retval = pss_apply_changes_to_node(tbl_info, tbl_data, &arg->rsp.info.set_rsp.i_param_val);
		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		/* If this row has been deleted and there is no entry on the disk,
		 * remove it from the Patricia tree itself.
		 */
		if ((tbl_data->deleted == TRUE) && (tbl_data->entry_on_disk == FALSE)) {
			retval = pss_delete_inst_node_from_tree(pTree, pNode);
			pwe_cb->p_pss_cb->mem_in_store -=
			    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		} else if ((tbl_data->dirty == FALSE) && (tbl_data->entry_on_disk == TRUE)) {
			/* If no changes have been made to this row, then delete it
			 * from the Patricia tree. The Patricia tree only stores the
			 * deltas.
			 */
			retval = pss_delete_inst_node_from_tree(pTree, pNode);
			pwe_cb->p_pss_cb->mem_in_store -=
			    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		if (inst->save_type == PSS_SAVE_TYPE_IMMEDIATE) {
			retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		} else {
			/* Add PSS_TBL_REC to pwe_cb->refresh_tbl_list */
			if (pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) {
				retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
				if (retval != NCSCC_RC_SUCCESS)
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
	}

	return NCSCC_RC_SUCCESS;
}

#define PSS_FREE_ERROR_COND_FOR_SETROW \
            if(pa.uba.max > pa.uba.ttl) \
            { \
                /* Free the USRBUF chain, since it is not fully consumed yet. */ \
                m_MMGR_FREE_BUFR_LIST(pa.uba.ub); \
            } \
            if(pv.i_fmat_id == NCSMIB_FMAT_OCT && pv.info.i_oct != NULL) \
                m_MMGR_FREE_MIB_OCT(pv.info.i_oct); \
            if ((tbl_data->deleted == TRUE) && (tbl_data->entry_on_disk == FALSE)) \
            { \
               retval = pss_delete_inst_node_from_tree(pTree, pNode); \
               pwe_cb->p_pss_cb->mem_in_store -= (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length); \
               if (retval != NCSCC_RC_SUCCESS) \
                  return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            } \
            else if (tbl_data->dirty == FALSE) \
            { \
               retval = pss_delete_inst_node_from_tree(pTree, pNode); \
               pwe_cb->p_pss_cb->mem_in_store -= (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length); \
               if (retval != NCSCC_RC_SUCCESS) \
                  return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            } \
            if (pwe_cb->p_pss_cb->save_type == PSS_SAVE_TYPE_IMMEDIATE) \
            { \
               retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id); \
               if (retval != NCSCC_RC_SUCCESS) \
                  return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            } \
            else \
            { \
               if(pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) \
               { \
                  retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id); \
                  if (retval != NCSCC_RC_SUCCESS) \
                     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
               } \
            }

/*****************************************************************************

  PROCEDURE NAME:    pss_process_setrow_request

  DESCRIPTION:       Process a SETROW/MOVEROW request

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_setrow_request(MAB_MSG *msg, NCS_BOOL is_move_row)
{
	PSS_PWE_CB *pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	PSS_TBL_REC *tbl_rec = NULL;
	NCS_PATRICIA_TREE *pTree;
	NCS_PATRICIA_NODE *pNode;
	PSS_MIB_TBL_INFO *tbl_info;
	PSS_CLIENT_ENTRY *client_node;
	PSS_MIB_TBL_DATA *tbl_data = NULL;
	NCSMIB_ARG *arg;
	uns16 pwe_id;
	char *p_pcn = NULL;
	uns32 tbl_id;
	uns32 retval;

	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	pwe_id = pwe_cb->pwe_id;
	arg = msg->data.data.snmp;
	tbl_id = arg->i_tbl_id;

	if (is_move_row)
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_INFO, PSS_SNMP_REQ_MOVEROW, pwe_id, arg->i_tbl_id);
	else
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_INFO, PSS_SNMP_REQ_SETROW, pwe_id, arg->i_tbl_id);

	tbl_id = arg->i_tbl_id;
	m_PSS_LOG_NCSMIB_ARG(arg);

	p_pcn = pss_get_pcn_from_mdsdest(pwe_cb, &msg->fr_card, tbl_id);
	if (p_pcn == NULL) {
		char addr_str[255] = { 0 };

		if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
			sprintf(addr_str, "VDEST:%d", m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card));
		else
			sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
				m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0, (uns32)(msg->fr_card));
		/* This MIB table or PCN is not available with PSS. */
		ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_HDLN_C, PSS_FC_HDLN,
			   NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
			   NCSFL_TYPE_TIC, PSS_HDLN_SNMP_REQ_FRM_INV_MDS_DEST_RCVD, addr_str);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Get the MIB data for this client */
	client_node = pss_find_client_entry(pwe_cb, p_pcn, FALSE);
	if (client_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLIENT_ENTRY_NULL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	arg = msg->data.data.snmp;

	/* Get the Patricia Tree for this table */
	tbl_rec = pss_find_table_tree(pwe_cb, client_node, NULL, tbl_id, FALSE, NULL);
	if (tbl_rec == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_REC_NOT_FND);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (tbl_rec->info.other.tree_inited == FALSE) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_TBL_REC_NOT_INITED, pwe_cb->pwe_id, arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	pTree = &tbl_rec->info.other.data;

	/* Get the patricia node containing the data for this inst ids */
	tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];
	if ((tbl_info->ptbl_info->table_of_scalars == FALSE)
	    && ((arg->i_idx.i_inst_len == 0) || (arg->i_idx.i_inst_ids == NULL))) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_ZERO_INST_LEN_OR_NULL_INST_IDS, pwe_cb->pwe_id,
				   arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	retval = pss_validate_index(tbl_info, &arg->i_idx);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_INDEX_VALIDATE_FAIL, pwe_cb->pwe_id, arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	pNode = pss_find_inst_node(pwe_cb, pTree, p_pcn, tbl_id,
				   arg->i_idx.i_inst_len, (uns32 *)arg->i_idx.i_inst_ids, TRUE);
	if (pNode == NULL) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_ADD_ROW_FAIL, pwe_cb->pwe_id, arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	tbl_data = (PSS_MIB_TBL_DATA *)pNode;
	{
		NCSPARM_AID pa;
		NCSMIB_PARAM_VAL pv;
		USRBUF *ub;

		if (is_move_row == TRUE)
			ub = arg->rsp.info.moverow_rsp.i_usrbuf;
		else
			ub = arg->rsp.info.setrow_rsp.i_usrbuf;
		if (ub == NULL) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_USRBUF);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		if (is_move_row)
			arg->rsp.info.moverow_rsp.i_usrbuf = NULL;
		else
			arg->rsp.info.setrow_rsp.i_usrbuf = NULL;

		ncsparm_dec_init(&pa, ub);

		while ((pa.cnt > 0) && (pa.len > 0)) {
			retval = ncsparm_dec_parm(&pa, &pv, NULL);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETROW_DEC_PARAM_FAIL);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			retval = pss_validate_set_mib_obj(tbl_info, &pv);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_PARAMVAL_VALIDATE_FAIL, pwe_cb->pwe_id,
						   arg->i_tbl_id);
				PSS_FREE_ERROR_COND_FOR_SETROW return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			retval = pss_apply_changes_to_node(tbl_info, tbl_data, &pv);
			if (retval != NCSCC_RC_SUCCESS) {
				PSS_FREE_ERROR_COND_FOR_SETROW return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			switch (pv.i_fmat_id) {
			case NCSMIB_FMAT_OCT:
				m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
				break;

			default:
				break;
			}
		}

		if (ncsparm_dec_done(&pa) != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETROW_DEC_PARAM_DONE_FAIL);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	/* If this row has been deleted and there is no entry on the disk,
	 * remove it from the Patricia tree itself.
	 */
	if ((tbl_data->deleted == TRUE) && (tbl_data->entry_on_disk == FALSE)) {
		retval = pss_delete_inst_node_from_tree(pTree, pNode);
		pwe_cb->p_pss_cb->mem_in_store -=
		    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	} else if (tbl_data->dirty == FALSE) {
		/* If no changes have been made to this row, then delete it
		 * from the Patricia tree. The Patricia tree only stores the
		 * deltas.
		 */
		retval = pss_delete_inst_node_from_tree(pTree, pNode);
		pwe_cb->p_pss_cb->mem_in_store -=
		    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (pwe_cb->p_pss_cb->save_type == PSS_SAVE_TYPE_IMMEDIATE) {
		retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	} else {
		/* Add PSS_TBL_REC to pwe_cb->refresh_tbl_list */
		if (pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) {
			retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	return NCSCC_RC_SUCCESS;
}

#define PSS_FREE_ERROR_COND_FOR_SETALLROWS \
            if(ra.uba.max > ra.uba.ttl) \
            { \
                /* Free the USRBUF chain, since it is not fully consumed yet. */ \
                m_MMGR_FREE_BUFR_LIST(ra.uba.ub); \
            } \
            if(idx.i_inst_ids != NULL) \
               m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids); \
            if(pv.i_fmat_id == NCSMIB_FMAT_OCT && pv.info.i_oct != NULL) \
                m_MMGR_FREE_MIB_OCT(pv.info.i_oct); \
            if(tbl_data != NULL) \
            { \
               if ((tbl_data->deleted == TRUE) && (tbl_data->entry_on_disk == FALSE)) \
               { \
                  retval = pss_delete_inst_node_from_tree(pTree, pNode); \
                  pwe_cb->p_pss_cb->mem_in_store -= (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length); \
                  if (retval != NCSCC_RC_SUCCESS) \
                     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
               } \
               else if (tbl_data->dirty == FALSE) \
               { \
                  retval = pss_delete_inst_node_from_tree(pTree, pNode); \
                  pwe_cb->p_pss_cb->mem_in_store -= (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length); \
                  if (retval != NCSCC_RC_SUCCESS) \
                     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
               } \
               if (pwe_cb->p_pss_cb->save_type == PSS_SAVE_TYPE_IMMEDIATE) \
               { \
                  retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id); \
                  if (retval != NCSCC_RC_SUCCESS) \
                     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
               } \
               else \
               { \
                  if(pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) \
                  { \
                     retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id); \
                     if (retval != NCSCC_RC_SUCCESS) \
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
                  } \
               } \
            }

/*****************************************************************************

  PROCEDURE NAME:    pss_process_setallrows_request

  DESCRIPTION:       Process a SETALLROWS request

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_setallrows_request(MAB_MSG *msg)
{
	PSS_CB *inst;
	PSS_PWE_CB *pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	PSS_TBL_REC *tbl_rec = NULL;
	NCS_PATRICIA_TREE *pTree;
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_MIB_TBL_DATA *tbl_data = NULL;
	PSS_MIB_TBL_INFO *tbl_info;
	NCSMIB_ARG *arg;
	NCSROW_AID ra;
	NCSMIB_IDX idx;
	NCSMIB_PARAM_VAL pv;
	PSS_CLIENT_ENTRY *client_node;
	char *p_pcn = NULL;
	uns32 tbl_id;
	uns32 retval;
	uns32 num_rows;
	uns32 num_params;
	uns32 i, j;

	/* Ensure that a handle exists for this PWE */
	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Ensure that the usrbuf handle is valid */
	arg = msg->data.data.snmp;
	if (arg == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (arg->rsp.info.setallrows_rsp.i_usrbuf == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_USRBUF);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	inst = pwe_cb->p_pss_cb;
	tbl_info = inst->mib_tbl_desc[arg->i_tbl_id];
	if ((tbl_info->ptbl_info->table_of_scalars == TRUE) || (arg->i_idx.i_inst_len == 0)
	    || (arg->i_idx.i_inst_ids == NULL)) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_ZERO_INST_LEN_OR_NULL_INST_IDS, pwe_cb->pwe_id,
				   arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	tbl_id = arg->i_tbl_id;
	m_LOG_PSS_SNMP_REQ(NCSFL_SEV_INFO, PSS_SNMP_REQ_SETALLROWS, pwe_cb->pwe_id, arg->i_tbl_id);
	m_PSS_LOG_NCSMIB_ARG(arg);

	p_pcn = pss_get_pcn_from_mdsdest(pwe_cb, &msg->fr_card, tbl_id);
	if (p_pcn == NULL) {
		char addr_str[255] = { 0 };

		if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
			sprintf(addr_str, "VDEST:%d", m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card));
		else
			sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
				m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0, (uns32)(msg->fr_card));
		/* This MIB table or PCN is not available with PSS. */
		ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_HDLN_C, PSS_FC_HDLN,
			   NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
			   NCSFL_TYPE_TIC, PSS_HDLN_SNMP_REQ_FRM_INV_MDS_DEST_RCVD, addr_str);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Get the MIB data for this client */
	client_node = pss_find_client_entry(pwe_cb, p_pcn, FALSE);
	if (client_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLIENT_ENTRY_NULL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	tbl_id = arg->i_tbl_id;

	/* Get the Patricia Tree for this table */
	tbl_rec = pss_find_table_tree(pwe_cb, client_node, NULL, tbl_id, FALSE, NULL);
	if (tbl_rec == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_REC_NOT_FND);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (tbl_rec->info.other.tree_inited == FALSE) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_TBL_REC_NOT_INITED, pwe_cb->pwe_id, arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	pTree = &tbl_rec->info.other.data;

	num_rows = ncssetallrows_dec_init(&ra, arg->rsp.info.setallrows_rsp.i_usrbuf);
	if (num_rows == 0) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETALLROWS_DEC_INIT_FAIL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	arg->rsp.info.setallrows_rsp.i_usrbuf = NULL;

	/* Process all the rows */
	for (i = 0; i < num_rows; i++) {
		memset(&pv, '\0', sizeof(pv));
		num_params = ncsrow_dec_init(&ra);
		if (num_params == 0) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETALLROWS_DEC_ROW_INIT_FAIL);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		idx.i_inst_ids = NULL;
		idx.i_inst_len = tbl_info->num_inst_ids;
		retval = ncsrow_dec_inst_ids(&ra, &idx, NULL);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETALLROWS_DEC_ROW_INST_IDS_FAIL);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		retval = pss_validate_index(tbl_info, &idx);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_INDEX_VALIDATE_FAIL, pwe_cb->pwe_id,
					   arg->i_tbl_id);
			PSS_FREE_ERROR_COND_FOR_SETALLROWS return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		/* Get the patricia node containing the data for this inst ids */
		pNode = pss_find_inst_node(pwe_cb, pTree, p_pcn, tbl_id, idx.i_inst_len, (uns32 *)idx.i_inst_ids, TRUE);
		if (pNode == NULL) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_ADD_ROW_FAIL, pwe_cb->pwe_id, arg->i_tbl_id);
			PSS_FREE_ERROR_COND_FOR_SETALLROWS return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		tbl_data = (PSS_MIB_TBL_DATA *)pNode;

		/* Process all the parameters in each row */
		for (j = 0; j < num_params; j++) {
			memset(&pv, '\0', sizeof(pv));
			retval = ncsrow_dec_param(&ra, &pv, NULL);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETALLROWS_DEC_PARAM_FAIL);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			retval = pss_validate_set_mib_obj(tbl_info, &pv);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_PARAMVAL_VALIDATE_FAIL, pwe_cb->pwe_id,
						   arg->i_tbl_id);
				PSS_FREE_ERROR_COND_FOR_SETALLROWS return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			retval = pss_apply_changes_to_node(tbl_info, tbl_data, &pv);
			if (retval != NCSCC_RC_SUCCESS) {
				PSS_FREE_ERROR_COND_FOR_SETALLROWS return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			switch (pv.i_fmat_id) {
			case NCSMIB_FMAT_OCT:
				m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
				break;

			default:
				break;
			}
		}		/* for (j = 0; j < num_params; j++) */

		m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
		idx.i_inst_ids = NULL;

		retval = ncsrow_dec_done(&ra);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETALLROWS_ROW_DEC_DONE_FAIL);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		/* If this row has been deleted and there is no entry on the disk,
		 * remove it from the Patricia tree itself.
		 */

		if ((tbl_data->deleted == TRUE) && (tbl_data->entry_on_disk == FALSE)) {
			retval = pss_delete_inst_node_from_tree(pTree, pNode);
			pwe_cb->p_pss_cb->mem_in_store -=
			    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		} else if (tbl_data->dirty == FALSE) {
			/* If no changes have been made to this row, then delete it
			 * from the Patricia tree. The Patricia tree only stores the
			 * deltas.
			 */
			retval = pss_delete_inst_node_from_tree(pTree, pNode);
			pwe_cb->p_pss_cb->mem_in_store -=
			    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		pNode = NULL;
		tbl_data = NULL;
	}			/* for (i = 0; i < num_rows; i++) */

	retval = ncssetallrows_dec_done(&ra);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SETALLROWS_DEC_DONE_FAIL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (inst->save_type == PSS_SAVE_TYPE_IMMEDIATE) {
		retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	} else {
		/* Add PSS_TBL_REC to pwe_cb->refresh_tbl_list */
		if (pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) {
			retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	return NCSCC_RC_SUCCESS;
}

#define PSS_FREE_ERROR_COND_FOR_REMOVEROWS \
            if(rra.uba.max > rra.uba.ttl) \
                m_MMGR_FREE_BUFR_LIST(rra.uba.ub); \
            if(idx.i_inst_ids != NULL) \
                m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids); \
            if((tbl_rec != NULL) && (tbl_rec->dirty == TRUE)) \
            { \
               if (inst->save_type == PSS_SAVE_TYPE_IMMEDIATE) \
               { \
                  retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id); \
                  if (retval != NCSCC_RC_SUCCESS) \
                     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
               } \
               else \
               { \
                  if(pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) \
                  { \
                     retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id); \
                     if (retval != NCSCC_RC_SUCCESS) \
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
                  } \
               } \
            }

/*****************************************************************************

  PROCEDURE NAME:    pss_process_removerows_request

  DESCRIPTION:       Process a REMOVEROWS request

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_removerows_request(MAB_MSG *msg)
{
	PSS_CB *inst = NULL;
	PSS_PWE_CB *pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	PSS_TBL_REC *tbl_rec = NULL;
	NCS_PATRICIA_TREE *pTree;
	NCS_PATRICIA_NODE *pNode;
	PSS_MIB_TBL_DATA *tbl_data;
	PSS_MIB_TBL_INFO *tbl_info;
	NCSMIB_ARG *arg;
	NCSREMROW_AID rra;
	NCSMIB_IDX idx;
	PSS_CLIENT_ENTRY *client_node;
	char *p_pcn = NULL;
	uns32 tbl_id;
	uns32 retval;
	uns32 num_rows;
	uns32 i;

	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	inst = pwe_cb->p_pss_cb;

	/* Ensure that the usrbuf handle is valid */
	arg = msg->data.data.snmp;
	if (arg == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	if (arg->rsp.info.removerows_rsp.i_usrbuf == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_USRBUF);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	tbl_info = inst->mib_tbl_desc[arg->i_tbl_id];
	if ((tbl_info->ptbl_info->table_of_scalars == TRUE) || (arg->i_idx.i_inst_len == 0)
	    || (arg->i_idx.i_inst_ids == NULL)) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_ZERO_INST_LEN_OR_NULL_INST_IDS, pwe_cb->pwe_id,
				   arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	m_LOG_PSS_SNMP_REQ(NCSFL_SEV_INFO, PSS_SNMP_REQ_REMOVEROWS, pwe_cb->pwe_id, arg->i_tbl_id);
	m_PSS_LOG_NCSMIB_ARG(arg);

	inst = pwe_cb->p_pss_cb;
	tbl_id = arg->i_tbl_id;

	p_pcn = pss_get_pcn_from_mdsdest(pwe_cb, &msg->fr_card, tbl_id);
	if (p_pcn == NULL) {
		char addr_str[255] = { 0 };

		if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
			sprintf(addr_str, "VDEST:%d", m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card));
		else
			sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
				m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0, (uns32)(msg->fr_card));
		/* This MIB table or PCN is not available with PSS. */
		ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_HDLN_C, PSS_FC_HDLN,
			   NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
			   NCSFL_TYPE_TIC, PSS_HDLN_SNMP_REQ_FRM_INV_MDS_DEST_RCVD, addr_str);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Get the MIB data for this client. */
	client_node = pss_find_client_entry(pwe_cb, p_pcn, FALSE);
	if (client_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLIENT_ENTRY_NULL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Get the Patricia Tree for this table */
	tbl_rec = pss_find_table_tree(pwe_cb, client_node, NULL, tbl_id, FALSE, NULL);
	if (tbl_rec == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_REC_NOT_FND);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (tbl_rec->info.other.tree_inited == FALSE) {
		m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_TBL_REC_NOT_INITED, pwe_cb->pwe_id, arg->i_tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	pTree = &tbl_rec->info.other.data;

	num_rows = ncsremrow_dec_init(&rra, arg->rsp.info.removerows_rsp.i_usrbuf);
	if (num_rows == 0) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_REMOVEROWS_DEC_INIT_FAIL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	arg->rsp.info.removerows_rsp.i_usrbuf = NULL;

	for (i = 0; i < num_rows; i++) {
		idx.i_inst_len = tbl_info->num_inst_ids;
		idx.i_inst_ids = NULL;

		retval = ncsremrow_dec_inst_ids(&rra, &idx, NULL);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_REMOVEROWS_DEC_INST_IDS_FAIL);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		retval = pss_validate_index(tbl_info, &arg->i_idx);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_SNMP_REQ(NCSFL_SEV_ERROR, PSS_SNMP_REQ_INDEX_VALIDATE_FAIL, pwe_cb->pwe_id,
					   arg->i_tbl_id);
			PSS_FREE_ERROR_COND_FOR_REMOVEROWS return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		/* Get the patricia node containing the data for this inst ids */
		pNode = pss_find_inst_node(pwe_cb, pTree, p_pcn, tbl_id, idx.i_inst_len, (uns32 *)idx.i_inst_ids, TRUE);

		m_MMGR_FREE_MIB_INST_IDS(idx.i_inst_ids);
		if (pNode == NULL)
			continue;

		tbl_data = (PSS_MIB_TBL_DATA *)pNode;
		tbl_data->deleted = TRUE;
		if ((tbl_data->deleted == TRUE) && (tbl_data->entry_on_disk == FALSE)) {
			retval = pss_delete_inst_node_from_tree(pTree, pNode);
			pwe_cb->p_pss_cb->mem_in_store -=
			    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
			if (retval != NCSCC_RC_SUCCESS) {
				PSS_FREE_ERROR_COND_FOR_REMOVEROWS return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}

	}			/* for (i = 0; i < num_rows; i++) */

	retval = ncsremrow_dec_done(&rra);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_REMOVEROWS_DEC_DONE_FAIL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (inst->save_type == PSS_SAVE_TYPE_IMMEDIATE) {
		retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	} else {
		/* Add PSS_TBL_REC to pwe_cb->refresh_tbl_list */
		if (pss_add_to_refresh_tbl_list(pwe_cb, tbl_rec) != NCSCC_RC_SUCCESS) {
			retval = pss_save_to_store(pwe_cb, pTree, NULL, p_pcn, tbl_id);
			if (retval != NCSCC_RC_SUCCESS)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	return NCSCC_RC_SUCCESS;
}

/*************************************************************************
 *         PSS SNMP Request Processing Routines - Helper Functions       *
 *************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    pss_find_client_entry

  DESCRIPTION:       Find/Add the Client node in the "client_table" for the 
                     corresponding PCN.

  RETURNS:           NULL    - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.
                     Pointer - Success

*****************************************************************************/
PSS_CLIENT_ENTRY *pss_find_client_entry(PSS_PWE_CB *pwe_cb, char *p_pcn, NCS_BOOL create)
{
	PSS_CLIENT_KEY client_key;
	PSS_CLIENT_ENTRY *client_node = NULL;
	NCS_PATRICIA_NODE *pat_node;
	uns32 retval;

	memset(&client_key, '\0', sizeof(client_key));
	strncpy(&client_key.pcn, p_pcn, NCSMIB_PCN_LENGTH_MAX - 1);

	pat_node = ncs_patricia_tree_get(&pwe_cb->client_table, (uns8 *)&client_key);
	if (pat_node != NULL) {
		client_node = (PSS_CLIENT_ENTRY *)pat_node;
		if (client_node == NULL) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLIENT_ENTRY_NULL);
			return (PSS_CLIENT_ENTRY *)m_MAB_DBG_SINK(NULL);
		}

		return client_node;
	}
	/* if (pat_node != NULL) */
	if (create == FALSE)	/* No need to create the structure */
		return NULL;

	/* At this point, the node does not exist and we need to create it */
	if ((client_node = m_MMGR_ALLOC_PSS_CLIENT_ENTRY) == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_CLIENT_ENTRY_ALLOC_FAIL, "pss_find_client_entry()");
		return (PSS_CLIENT_ENTRY *)m_MAB_DBG_SINK(NULL);
	}

	pss_client_entry_init(client_node);
	client_node->key = client_key;
	client_node->pat_node.key_info = (uns8 *)&client_node->key;

	retval = ncs_patricia_tree_add(&pwe_cb->client_table, &client_node->pat_node);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLIENT_ENTRY_ADD_FAIL);
		m_MMGR_FREE_PSS_CLIENT_ENTRY(client_node);
		return (PSS_CLIENT_ENTRY *)m_MAB_DBG_SINK(NULL);
	}
	m_LOG_PSS_CLIENT_ENTRY(NCSFL_SEV_INFO, PSS_CLIENT_ENTRY_ADD, p_pcn, pwe_cb->pwe_id);

	return client_node;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_find_table_tree

  DESCRIPTION:       Find patricia tree corresponding to the MIB_DATA

  RETURNS:           NULL    - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.
                     Pointer - Success

*****************************************************************************/
PSS_TBL_REC *pss_find_table_tree(PSS_PWE_CB *pwe_cb, PSS_CLIENT_ENTRY *client_node,
				 PSS_OAA_ENTRY *oaa_node, uns32 tbl_id, NCS_BOOL create, PSS_RET_VAL *o_prc)
{
	uns32 bucket = tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;
	PSS_TBL_REC *tbl_rec = client_node->hash[bucket];
	PSS_OAA_CLT_ID *oaa_clt_node = NULL;
	uns32 retval;
	PSS_TBL_REC *temp = NULL, *prev = NULL;

	if ((tbl_id >= MIB_UD_TBL_ID_END) || (pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id] == NULL)) {
		if (o_prc != NULL) {
			if (tbl_id > MIB_UD_TBL_ID_END) {
				*o_prc = PSS_RET_TBL_ID_BEYOND_MAX_ID;
			} else if (pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id] == NULL) {
				*o_prc = PSS_RET_MIB_DESC_NULL;
			}
		}
		return (PSS_TBL_REC *)m_MAB_DBG_SINK(NULL);
	}

	while (tbl_rec != NULL) {
		if (tbl_rec->tbl_id == tbl_id) {
			char addr_str[255] = { 0 };

			if (oaa_node != NULL) {
				if (m_NCS_NODE_ID_FROM_MDS_DEST(oaa_node->key.mds_dest) == 0)
					sprintf(addr_str, "VDEST:%d, Table-id: %d ",
						m_MDS_GET_VDEST_ID_FROM_MDS_DEST(oaa_node->key.mds_dest), tbl_id);
				else
					sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d, , Table-id: %d",
						m_NCS_NODE_ID_FROM_MDS_DEST(oaa_node->key.mds_dest), 0,
						(uns32)(oaa_node->key.mds_dest), tbl_id);
				/* This MIB table or PCN is not available with PSS. */
				ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_HDLN_C, PSS_FC_HDLN,
					   NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
					   NCSFL_TYPE_TIC, PSS_HDLN_TBL_BIND_FND_TBL_REC_ALREADY_FOR_MDSDEST, addr_str);
			}
			return tbl_rec;
		}
		tbl_rec = tbl_rec->next;
	}

	if (create == FALSE)
		return NULL;

	if (oaa_node == NULL)
		return NULL;

	tbl_rec = m_MMGR_ALLOC_PSS_TBL_REC;
	if (tbl_rec == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_TBL_REC_ALLOC_FAIL, "pss_find_table_tree()");
		return (PSS_TBL_REC *)m_MAB_DBG_SINK(NULL);
	}

	memset(tbl_rec, '\0', sizeof(PSS_TBL_REC));
	tbl_rec->dirty = FALSE;
	tbl_rec->tbl_id = tbl_id;
	if (pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id]->ptbl_info->table_of_scalars) {
		/* Scalar table. */
		tbl_rec->is_scalar = TRUE;
		tbl_rec->info.scalar.row_len = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id]->max_row_length;
	} else {
		tbl_rec->info.other.params.key_size = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id]->max_key_length;
		retval = ncs_patricia_tree_init(&tbl_rec->info.other.data, &tbl_rec->info.other.params);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_REC_PATTREE_INIT_FAIL);
			m_MMGR_FREE_PSS_TBL_REC(tbl_rec);
			return (PSS_TBL_REC *)m_MAB_DBG_SINK(NULL);
		}
		tbl_rec->info.other.tree_inited = TRUE;
	}

	/* Add pointer to the PCN key */
	tbl_rec->pss_client_key = &client_node->key;

	/* Add pointer to the OAA-node */
	tbl_rec->p_oaa_entry = oaa_node;

	if (client_node->hash[bucket] == NULL)
		client_node->hash[bucket] = tbl_rec;
	else {
		temp = client_node->hash[bucket];
		while (temp != NULL) {
			prev = temp;
			temp = temp->next;
		}
		prev->next = tbl_rec;
	}

	/* Increment the client_node->cnt */
	client_node->tbl_cnt++;

	/* Add the node reference in OAA_tree also */
	oaa_clt_node = pss_add_tbl_in_oaa_node(pwe_cb, oaa_node, tbl_rec);
	if (oaa_clt_node == NULL)
		return (PSS_TBL_REC *)m_MAB_DBG_SINK(NULL);

	return tbl_rec;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_client_entry_init

  DESCRIPTION:       Initialize the PSS_CLIENT_ENTRY structure

  RETURNS:           nothing

*****************************************************************************/
void pss_client_entry_init(PSS_CLIENT_ENTRY *client_node)
{
	uns32 i;

	if (client_node == NULL)
		return;

	memset(client_node, '\0', sizeof(PSS_CLIENT_ENTRY));

	for (i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
		client_node->hash[i] = NULL;
	client_node->tbl_cnt = 0;

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_find_inst_node

  DESCRIPTION:       Find patricia node corresponding to inst ids

  RETURNS:           NULL    - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.
                     Pointer - Success

*****************************************************************************/
NCS_PATRICIA_NODE *pss_find_inst_node(PSS_PWE_CB *pwe_cb,
				      NCS_PATRICIA_TREE *pTree,
				      char *p_pcn, uns32 tbl_id, uns32 num_inst_ids, uns32 *inst_ids, NCS_BOOL create)
{
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_MIB_TBL_INFO *tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];
	PSS_MIB_TBL_DATA *tbl_data;
	uns32 retval;
	NCS_BOOL entry_found;
	uns8 pkey[NCS_PATRICIA_MAX_KEY_SIZE];
	uns8 *profile_name = pwe_cb->p_pss_cb->current_profile;

	memset((uns8 *)&pkey, '\0', sizeof(pkey));
	pss_set_key_from_inst_ids(tbl_info, (uns8 *)&pkey, num_inst_ids, inst_ids);
	pNode = ncs_patricia_tree_get(pTree, pkey);
	if (pNode != NULL)
		return pNode;

	if (create == FALSE)
		return NULL;

	/* The entry does not exist in the tree. Create a new one */
	tbl_data = m_MMGR_ALLOC_PSS_MIB_TBL_DATA;
	pwe_cb->p_pss_cb->mem_in_store += sizeof(PSS_MIB_TBL_DATA);
	pNode = (NCS_PATRICIA_NODE *)tbl_data;
	if (pNode == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_TBL_DATA_ALLOC_FAIL, "pss_find_inst_node()");
		return (NCS_PATRICIA_NODE *)m_MAB_DBG_SINK(NULL);
	}

	memset(tbl_data, '\0', sizeof(PSS_MIB_TBL_DATA));
	tbl_data->deleted = FALSE;
	tbl_data->dirty = FALSE;
	tbl_data->key = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_key_length);
	pwe_cb->p_pss_cb->mem_in_store += tbl_info->max_key_length;
	if (tbl_data->key == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_find_inst_node()");
		m_MMGR_FREE_PSS_MIB_TBL_DATA(tbl_data);
		return (NCS_PATRICIA_NODE *)m_MAB_DBG_SINK(NULL);
	}

	tbl_data->data = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_row_length);
	pwe_cb->p_pss_cb->mem_in_store += tbl_info->max_row_length;
	if (tbl_data->data == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_ROW_DATA_ALLOC_FAIL, "pss_find_inst_node()");
		m_MMGR_FREE_PSS_OCT(tbl_data->key);
		m_MMGR_FREE_PSS_MIB_TBL_DATA(tbl_data);
		return (NCS_PATRICIA_NODE *)m_MAB_DBG_SINK(NULL);
	}

	memset(tbl_data->data, 0, tbl_info->max_row_length);
	memset(tbl_data->key, 0, tbl_info->max_key_length);

	pNode->key_info = tbl_data->key;
	memcpy(tbl_data->key, (uns8 *)&pkey, tbl_info->max_key_length);

	retval = ncs_patricia_tree_add(pTree, pNode);
	if (retval != NCSCC_RC_SUCCESS) {
		m_MMGR_FREE_PSS_OCT(tbl_data->key);
		m_MMGR_FREE_PSS_OCT(tbl_data->data);
		m_MMGR_FREE_PSS_MIB_TBL_DATA(tbl_data);
		return (NCS_PATRICIA_NODE *)m_MAB_DBG_SINK(NULL);
	}

	retval = pss_read_from_store(pwe_cb, profile_name, tbl_data->key, p_pcn, tbl_id, pNode, &entry_found);

	if (retval != NCSCC_RC_SUCCESS) {
		ncs_patricia_tree_del(pTree, pNode);
		m_MMGR_FREE_PSS_OCT(tbl_data->key);
		m_MMGR_FREE_PSS_OCT(tbl_data->data);
		m_MMGR_FREE_PSS_MIB_TBL_DATA(tbl_data);
		return (NCS_PATRICIA_NODE *)m_MAB_DBG_SINK(NULL);
	}

	tbl_data->entry_on_disk = entry_found;
	if (entry_found == FALSE) {
		memset(tbl_data->data, 0, tbl_info->max_row_length);
		retval = pss_apply_inst_ids_to_node(tbl_info, tbl_data, num_inst_ids, inst_ids);
		if (retval != NCSCC_RC_SUCCESS) {
			ncs_patricia_tree_del(pTree, pNode);
			m_MMGR_FREE_PSS_OCT(tbl_data->key);
			m_MMGR_FREE_PSS_OCT(tbl_data->data);
			m_MMGR_FREE_PSS_MIB_TBL_DATA(tbl_data);
			return (NCS_PATRICIA_NODE *)m_MAB_DBG_SINK(NULL);
		}
	}

	return pNode;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_find_scalar_node

  DESCRIPTION:       Find scalar-mib-data(latest from table or from store)

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_find_scalar_node(PSS_PWE_CB *pwe_cb, PSS_TBL_REC *tbl_rec,
			   char *p_pcn, uns32 tbl_id, NCS_BOOL create, NCS_BOOL *created_new)
{
	NCS_BOOL entry_found = FALSE;
	uns8 *profile_name = pwe_cb->p_pss_cb->current_profile;
	uns32 retval = NCSCC_RC_SUCCESS;

	*created_new = FALSE;

	if (create == FALSE)
		return NCSCC_RC_SUCCESS;

	/* The scalar-table-entry does not exist in the table. Create a new one */
	if (tbl_rec->info.scalar.data == NULL) {
		tbl_rec->info.scalar.data = m_MMGR_ALLOC_PSS_OCT(tbl_rec->info.scalar.row_len);
		if (tbl_rec->info.scalar.data == NULL) {
			m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_SCLR_MIB_ROW_ALLOC_FAIL, "pss_find_scalar_node()");
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	} else
		return NCSCC_RC_SUCCESS;

	memset(tbl_rec->info.scalar.data, 0, tbl_rec->info.scalar.row_len);

	retval = pss_read_from_sclr_store(pwe_cb, profile_name, tbl_rec->info.scalar.data, p_pcn, tbl_id, &entry_found);
	if (retval != NCSCC_RC_SUCCESS) {
		m_MMGR_FREE_PSS_OCT(tbl_rec->info.scalar.data);
		tbl_rec->info.scalar.data = NULL;
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (!entry_found)
		*created_new = TRUE;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_apply_changes_to_node

  DESCRIPTION:       Apply changes in Param_Val to the patricia node

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_apply_changes_to_node(PSS_MIB_TBL_INFO *tbl_info, PSS_MIB_TBL_DATA *pData, NCSMIB_PARAM_VAL *param)
{
	uns32 index = (uns32)(tbl_info->ptbl_info->objects_local_to_tbl[param->i_param_id]);
	PSS_VAR_INFO *var_info = &(tbl_info->pfields[index]);
	uns32 offset = var_info->offset;
	uns32 retval;
	uns16 len16, max_len = 0x0000;
	uns16 data16;
	NCS_BOOL data_bool, bitmap_already_set = FALSE;

	/* Just a sanity check here */

	switch (param->i_fmat_id) {
	case NCSMIB_FMAT_OCT:
		if (var_info->var_info.obj_type == NCSMIB_OCT_OBJ_TYPE) {
			if (var_info->var_info.obj_spec.stream_spec.max_len < param->i_length)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			max_len = var_info->var_info.obj_spec.stream_spec.max_len;
		} else {	/* Discrete octet strings */

			max_len = ncsmiblib_get_max_len_of_discrete_octet_string(&var_info->var_info);
			if (max_len < param->i_length)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		if (var_info->var_length == TRUE) {
			memcpy(&len16, pData->data + offset, sizeof(uns16));
			if (len16 < param->i_length)
				len16 = param->i_length;

			if (param->i_length != 0) {
				/* To handle null octet strings */
				retval = memcmp(pData->data + offset + sizeof(uns16), param->info.i_oct, len16);
				if (retval == 0) {
					bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData->data, index + 1);
					if (!bitmap_already_set) {
						memset(pData->data + offset, 0, max_len + sizeof(uns16));
						memcpy(pData->data + offset, &param->i_length, sizeof(uns16));
						pData->dirty = TRUE;	/* Need force a set back to the user, during playback */
					}
					break;
				}

				memset(pData->data + offset, 0, max_len + sizeof(uns16));
				memcpy(pData->data + offset, &param->i_length, sizeof(uns16));
				memcpy(pData->data + offset + sizeof(uns16), param->info.i_oct, param->i_length);
				pData->dirty = TRUE;
			} else {
				memset(pData->data + offset, 0, max_len + sizeof(uns16));
				pData->dirty = TRUE;
			}
		} else {
			retval = memcmp(pData->data + offset, param->info.i_oct, param->i_length);
			if (retval == 0) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData->data, index + 1);
				if (!bitmap_already_set)
					pData->dirty = TRUE;	/* Need force a set back to the user, during playback */
			}

			memset(pData->data + offset, 0, var_info->var_info.len);
			memcpy(pData->data + offset, param->info.i_oct, param->i_length);
			pData->dirty = TRUE;
		}

		break;

	case NCSMIB_FMAT_INT:
		switch (tbl_info->pfields[index].var_info.len) {
		case sizeof(uns8):
			if ((*(pData->data + offset)) == (uns8)param->info.i_int) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData->data, index + 1);
				if (!bitmap_already_set)
					pData->dirty = TRUE;	/* Need force a set back to the user, during playback */
				break;
			}

			*(pData->data + offset) = (uns8)param->info.i_int;
			pData->dirty = TRUE;
			break;

		case sizeof(uns16):
			data16 = (uns16)param->info.i_int;
			retval = memcmp(pData->data + offset, &data16, sizeof(uns16));
			if (retval == 0) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData->data, index + 1);
				if (!bitmap_already_set)
					pData->dirty = TRUE;	/* Need force a set back to the user, during playback */
				break;
			}

			memcpy(pData->data + offset, &data16, sizeof(uns16));
			pData->dirty = TRUE;
			break;

		case sizeof(uns32):
			retval = memcmp(pData->data + offset, &param->info.i_int, sizeof(uns32));
			if (retval == 0) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData->data, index + 1);
				if (!bitmap_already_set)
					pData->dirty = TRUE;	/* Need force a set back to the user, during playback */
				break;
			}

			memcpy(pData->data + offset, &param->info.i_int, sizeof(uns32));
			pData->dirty = TRUE;
			break;

		default:
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}		/* switch (param->i_length) */

		break;

	case NCSMIB_FMAT_BOOL:
		data_bool = (NCS_BOOL)param->info.i_int;
		retval = memcmp(pData->data + offset, &data_bool, sizeof(NCS_BOOL));
		if (retval == 0) {
			bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData->data, index + 1);
			if (!bitmap_already_set)
				pData->dirty = TRUE;	/* Need force a set back to the user, during playback */
			break;
		}

		memcpy(pData->data + offset, &data_bool, sizeof(NCS_BOOL));	/* Fix verification post SP03. TBD */
		pData->dirty = TRUE;
		break;

	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	pss_set_bit_for_param(pData->data, (index + 1));

	/* If the status of the row is being set, mark it appropriately */
	if (tbl_info->ptbl_info->status_field == param->i_param_id) {
		if (param->info.i_int == NCSMIB_ROWSTATUS_DESTROY)
			pData->deleted = TRUE;
		else
			pData->deleted = FALSE;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_apply_changes_to_sclr_node

  DESCRIPTION:       Apply changes in Param_Val to the patricia node

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_apply_changes_to_sclr_node(PSS_MIB_TBL_INFO *tbl_info,
				     PSS_TBL_REC *tbl_rec, NCS_BOOL created_new, NCSMIB_PARAM_VAL *param)
{
	uns32 index = tbl_info->ptbl_info->objects_local_to_tbl[param->i_param_id];
	PSS_VAR_INFO *var_info = &(tbl_info->pfields[index]);
	uns32 offset = var_info->offset;
	uns32 retval;
	uns16 len16, max_len = 0x0000;
	uns16 data16;
	NCS_BOOL data_bool, bitmap_already_set = FALSE;
	uns8 *pData = tbl_rec->info.scalar.data;

	/* Just a sanity check here */
	if (param->i_param_id != var_info->var_info.param_id) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_PARAM_ID_MISMATCH_WITH_VAR_INFO_PARAM_ID);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	retval = pss_validate_param_val(&var_info->var_info, param);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN2_SET_PARAM_VAL_FAIL_VALIDATION_TEST, param->i_param_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	switch (param->i_fmat_id) {
	case NCSMIB_FMAT_OCT:
		if (var_info->var_info.obj_type == NCSMIB_OCT_OBJ_TYPE) {
			if (var_info->var_info.obj_spec.stream_spec.max_len < param->i_length) {
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR,
						    PSS_HDLN2_SCLR_SET_OCT_OBJECT_LENGTH_MISMATCH_WITH_STREAM_SPEC_MAX_LEN);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			max_len = var_info->var_info.obj_spec.stream_spec.max_len;
		} else {	/* Discrete octet strings */

			max_len = ncsmiblib_get_max_len_of_discrete_octet_string(&var_info->var_info);
			if (max_len < param->i_length) {
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR,
						    PSS_HDLN2_SCLR_SET_OCT_OBJECT_LENGTH_MISMATCH_WITH_DISCRETE_OCT_STR_MAX_LEN);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
		if (var_info->var_length == TRUE) {
			memcpy(&len16, pData + offset, sizeof(uns16));
			if (len16 < param->i_length)
				len16 = param->i_length;

			if (param->i_length != 0) {
				/* To handle null octet strings */
				retval = memcmp(pData + offset + sizeof(uns16), param->info.i_oct, len16);
				if (retval == 0) {
					bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData, index + 1);
					if (!bitmap_already_set) {
						memset(pData + offset, 0, max_len + sizeof(uns16));
						memcpy(pData + offset, &param->i_length, sizeof(uns16));
						tbl_rec->dirty = TRUE;	/* Need force a set back to the user, during playback */
					}
					break;
				}

				memset(pData + offset, 0, max_len + sizeof(uns16));
				memcpy(pData + offset, &param->i_length, sizeof(uns16));
				memcpy(pData + offset + sizeof(uns16), param->info.i_oct, param->i_length);
				tbl_rec->dirty = TRUE;
			} else {
				memset(pData + offset, 0, max_len + sizeof(uns16));
				tbl_rec->dirty = TRUE;
			}
		} else {
			retval = memcmp(pData + offset, param->info.i_oct, param->i_length);
			if (retval == 0) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData, index + 1);
				if (!bitmap_already_set)
					tbl_rec->dirty = TRUE;	/* Need force a set back to the user, during playback */
				break;
			}

			memset(pData + offset, 0, var_info->var_info.len);
			memcpy(pData + offset, param->info.i_oct, param->i_length);
			tbl_rec->dirty = TRUE;
		}
		break;

	case NCSMIB_FMAT_INT:
		switch (tbl_info->pfields[index].var_info.len) {
		case sizeof(uns8):
			if ((*(pData + offset)) == (uns8)param->info.i_int) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData, index + 1);
				if (!bitmap_already_set)
					tbl_rec->dirty = TRUE;	/* Need force a set back to the user, during playback */
				break;
			}

			*(pData + offset) = (uns8)param->info.i_int;
			tbl_rec->dirty = TRUE;
			break;

		case sizeof(uns16):
			data16 = (uns16)param->info.i_int;
			retval = memcmp(pData + offset, &data16, sizeof(uns16));
			if (retval == 0) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData, index + 1);
				if (!bitmap_already_set)
					tbl_rec->dirty = TRUE;	/* Need force a set back to the user, during playback */
				break;
			}

			memcpy(pData + offset, &data16, sizeof(uns16));
			tbl_rec->dirty = TRUE;
			break;

		case sizeof(uns32):
			retval = memcmp(pData + offset, &param->info.i_int, sizeof(uns32));
			if (retval == 0) {
				bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData, index + 1);
				if (!bitmap_already_set)
					tbl_rec->dirty = TRUE;	/* Need force a set back to the user, during playback */
				break;
			}

			memcpy(pData + offset, &param->info.i_int, sizeof(uns32));
			tbl_rec->dirty = TRUE;
			break;

		default:
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_VAR_INFO_LEN_NOT_SUPPORTED);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}		/* switch (param->i_length) */
		break;

	case NCSMIB_FMAT_BOOL:
		data_bool = (NCS_BOOL)param->info.i_int;
		retval = memcmp(pData + offset, &data_bool, sizeof(NCS_BOOL));
		if (retval == 0) {
			bitmap_already_set = (NCS_BOOL)pss_get_bit_for_param(pData, index + 1);
			if (!bitmap_already_set)
				tbl_rec->dirty = TRUE;	/* Need force a set back to the user, during playback */
			break;
		}

		memcpy(pData + offset, &data_bool, sizeof(NCS_BOOL));
		tbl_rec->dirty = TRUE;
		break;

	default:
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_FMAT_ID_NOT_SUPPORTED);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	pss_set_bit_for_param(pData, (index + 1));

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_apply_inst_ids_to_node

  DESCRIPTION:       Populate the node data with the inst ids

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_apply_inst_ids_to_node(PSS_MIB_TBL_INFO *tbl_info, PSS_MIB_TBL_DATA *tbl_data, uns32 num_ids, uns32 *inst_ids)
{
	PSS_VAR_INFO *var_info;
	uns32 i, j;
	uns16 len16, max_len = 0x0000;
	uns8 *ptr;
	uns32 num_inst_ids = 0;
	NCS_BOOL data_bool;
	uns16 data16;

	for (i = 0; i < tbl_info->ptbl_info->num_objects; i++) {
		var_info = &tbl_info->pfields[i];
		if (var_info->var_info.is_index_id == TRUE) {
			ptr = tbl_data->data + var_info->offset;
			switch (var_info->var_info.fmat_id) {
			case NCSMIB_FMAT_OCT:
				if (var_info->var_length == TRUE) {
					len16 = (uns16)(inst_ids[num_inst_ids++]);
					memcpy(ptr, &len16, sizeof(uns16));
					ptr += sizeof(uns16);
				} else
					len16 = (uns16)var_info->var_info.obj_spec.stream_spec.max_len;

				if (var_info->var_info.obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
					max_len = ncsmiblib_get_max_len_of_discrete_octet_string(&var_info->var_info);
				else
					max_len = var_info->var_info.obj_spec.stream_spec.max_len;
				memset(ptr, 0, max_len);

				for (j = 0; j < len16; j++)
					*ptr++ = (uns8)inst_ids[num_inst_ids++];

				break;

			case NCSMIB_FMAT_INT:
				switch (var_info->var_info.len) {
				case sizeof(uns8):
					*ptr = (uns8)inst_ids[num_inst_ids];
					break;

				case sizeof(uns16):
					data16 = (uns16)inst_ids[num_inst_ids];
					memcpy(ptr, &data16, sizeof(uns16));
					break;

				case sizeof(uns32):
					memcpy(ptr, &inst_ids[num_inst_ids], sizeof(uns32));
					break;

				default:
					m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_VAR_INFO_LEN_NOT_SUPPORTED);
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}

				num_inst_ids++;
				break;

			case NCSMIB_FMAT_BOOL:
				data_bool = (NCS_BOOL)inst_ids[num_inst_ids];
				num_inst_ids++;
				memcpy(ptr, &data_bool, sizeof(NCS_BOOL));
				break;

			default:
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_FMAT_ID_NOT_SUPPORTED);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			pss_set_bit_for_param(tbl_data->data,
					      (tbl_info->ptbl_info->objects_local_to_tbl[var_info->var_info.param_id] +
					       1));
		}
	}

	if (num_inst_ids != num_ids) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_NUM_INST_IDS_MISMATCH);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_get_bit_for_param

  DESCRIPTION:       This function retrieves the corresponding bit in the bitmap.
                     The bitmap indicates whether the data corresponding to the
                     particular parameter is valid or not.

  RETURNS:           1, if bit for the param-id is set
                     0, otherwise.

*****************************************************************************/
uns8 pss_get_bit_for_param(uns8 *buffer, NCSMIB_PARAM_ID param)
{
	uns8 mask = (uns8)(1 << ((param - 1) % 8));
	uns32 index = (param - 1) / 8;

	return (uns8)(buffer[index] & mask);
}

/*****************************************************************************

  PROCEDURE NAME:    pss_get_count_of_valid_params

  DESCRIPTION:       This function gets the number of param-ids set in this
                     MIB row.

  RETURNS:           uns32 count of the MIB-param-ids set.

*****************************************************************************/
uns32 pss_get_count_of_valid_params(uns8 *buffer, PSS_MIB_TBL_INFO *tbl_info)
{
	uns32 i = 0, j = 0, num_params = tbl_info->ptbl_info->num_objects, cnt = 0;
	uns8 mask = 0x00;

	for (i = 0; i < num_params; i++) {
		if (tbl_info->pfields[i].var_info.param_id != 0) {
			mask = (uns8)(1 << (i % 8));
			j = i / 8;
			if (buffer[j] & mask)
				++cnt;
		}
	}
	return cnt;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_set_bit_for_param

  DESCRIPTION:       This function sets the corresponding bit in the bitmap for
                     the MIB row. The bitmap indicates whether the data 
                     corresponding to the particular parameter is valid or not.

  RETURNS:           void

*****************************************************************************/
void pss_set_bit_for_param(uns8 *buffer, NCSMIB_PARAM_ID param)
{
	uns32 index = (param - 1) / 8;
	uns8 mask = (uns8)(1 << ((param - 1) % 8));

	buffer[index] |= mask;
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_read_from_store

  DESCRIPTION:       This function searches for a entry indexed by inst_ids.
                     The search is made in a file on the persistent store, which
                     contains the data in sorted order. Hence binary search is used.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_read_from_store(PSS_PWE_CB *pwe_cb, uns8 *profile_name,
			  uns8 *key, char *p_pcn, uns32 tbl_id, NCS_PATRICIA_NODE *pNode, NCS_BOOL *entry_found)
{
	PSS_MIB_TBL_INFO *tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];
	long file_hdl = 0;
	uns8 *pkey;
	PSS_MIB_TBL_DATA *tbl_data = (PSS_MIB_TBL_DATA *)pNode;
	uns8 *buffer = tbl_data->data;
	uns32 buf_len;
	uns32 num_entries;
	NCS_PSSTS_ARG pssts_arg;
	uns32 retval;
	uns32 low, high, mid, file_size, rem_file_size = 0;
	NCS_BOOL file_exists;

	*entry_found = FALSE;

	/* Check if the file exists */
	m_NCS_PSSTS_FILE_EXISTS(pwe_cb->p_pss_cb->pssts_api,
				pwe_cb->p_pss_cb->pssts_hdl, retval,
				profile_name, pwe_cb->pwe_id, p_pcn, tbl_id, file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* The file does not exist, so no entry on the persistent storage */
	if (file_exists == FALSE)
		return NCSCC_RC_SUCCESS;

	/* Get the file size to determine the number of entries in it */
	m_NCS_PSSTS_FILE_SIZE(pwe_cb->p_pss_cb->pssts_api,
			      pwe_cb->p_pss_cb->pssts_hdl, retval, profile_name,
			      pwe_cb->pwe_id, p_pcn, tbl_id, file_size);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_FILE_SIZE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	/* 3.0.b addition */
	/* Considering the prefixed Table details header */
	rem_file_size = file_size - PSS_TABLE_DETAILS_HEADER_LEN;	/* To consider both header_len < or > or = max_row_length */
	if (rem_file_size % tbl_info->max_row_length != 0) {
		/* Log that the ps_file is not consistent */
		m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, tbl_id);
		m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_FILE_SIZE, file_size);
		return NCSCC_RC_FAILURE;
	}
	/* End of 3.0.b addition */

	buf_len = tbl_info->max_row_length;
	num_entries = (rem_file_size) / (buf_len);
	if (num_entries == 0)
		return NCSCC_RC_SUCCESS;

	/* Now allocate space for the index ids and data buffer */
	pkey = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_key_length);

	if (pkey == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_read_from_store()");
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	memset(pkey, '\0', tbl_info->max_key_length);

	/* Open the file for reading */
	m_NCS_PSSTS_FILE_OPEN(pwe_cb->p_pss_cb->pssts_api,
			      pwe_cb->p_pss_cb->pssts_hdl, retval, profile_name,
			      pwe_cb->pwe_id, p_pcn, tbl_id, NCS_PSSTS_FILE_PERM_READ, file_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		m_MMGR_FREE_PSS_OCT(pkey);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	pssts_arg.i_op = NCS_PSSTS_OP_READ_FILE;
	pssts_arg.ncs_hdl = pwe_cb->p_pss_cb->pssts_hdl;
	pssts_arg.info.read_file.i_handle = file_hdl;
	pssts_arg.info.read_file.io_buffer = buffer;
	pssts_arg.info.read_file.i_bytes_to_read = buf_len;

	/* Binary search algorithm for looking up the entry */
	low = 1;
	high = num_entries;
	while (low <= high) {
		int res;
		mid = (low + high) / 2;
		/* Read in the data from the file */
		pssts_arg.info.read_file.i_offset = ((mid - 1) * buf_len) + PSS_TABLE_DETAILS_HEADER_LEN;

		retval = (pwe_cb->p_pss_cb->pssts_api) (&pssts_arg);
		if ((retval != NCSCC_RC_SUCCESS) || (pssts_arg.info.read_file.o_bytes_read != buf_len)) {
			/* Close the file */
			m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval,
					       file_hdl);
			m_MMGR_FREE_PSS_OCT(pkey);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		pss_get_key_from_data(tbl_info, pkey, buffer);
		res = memcmp(key, pkey, tbl_info->max_key_length);

		if (res < 0)
			high = mid - 1;
		else if (res > 0)
			low = mid + 1;
		else if (res == 0) {
			*entry_found = TRUE;
			break;
		}
	}			/* while(low <= high) */

	/* At this point, the entry has been found or not found
	 * but it does not matter. If it was found, the data is
	 * already in buffer.
	 */

	if (*entry_found == TRUE)
		m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_ENTRY_FOUND, tbl_id);
	else
		m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_ENTRY_NOT_FOUND, tbl_id);

	m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, file_hdl);
	m_MMGR_FREE_PSS_OCT(pkey);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_read_from_sclr_store

  DESCRIPTION:       This function searches for the only entry for the scalar
                     table.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_read_from_sclr_store(PSS_PWE_CB *pwe_cb, uns8 *profile_name,
			       uns8 *data, char *p_pcn, uns32 tbl_id, NCS_BOOL *entry_found)
{
	PSS_MIB_TBL_INFO *tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];
	long file_hdl = 0;
	uns32 buf_len;
	NCS_PSSTS_ARG pssts_arg;
	uns32 retval, file_size = 0, temp_file_size = 0;
	NCS_BOOL file_exists;

	*entry_found = FALSE;

	/* Check if the file exists */
	m_NCS_PSSTS_FILE_EXISTS(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
				retval, profile_name, pwe_cb->pwe_id, p_pcn, tbl_id, file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* The file does not exist, so no entry on the persistent storage */
	if (file_exists == FALSE)
		return NCSCC_RC_SUCCESS;

	buf_len = tbl_info->max_row_length;

	/* Get the file size to determine the number of entries in it */
	m_NCS_PSSTS_FILE_SIZE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
			      retval, profile_name, pwe_cb->pwe_id, p_pcn, tbl_id, file_size);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_FILE_SIZE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (file_size != 0) {
		/* 3.0.b addition */
		/* Considering the prefixed Table details header */
		temp_file_size = file_size - PSS_TABLE_DETAILS_HEADER_LEN;	/* To consider both header_len < or > or = max_row_length */
		if (temp_file_size % tbl_info->max_row_length == 0) {
			/* End of 3.0.b addition */

			/* Open the file for reading */
			m_NCS_PSSTS_FILE_OPEN(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
					      retval, profile_name, pwe_cb->pwe_id, p_pcn, tbl_id,
					      NCS_PSSTS_FILE_PERM_READ, file_hdl);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			/* Read in the data from the file */
			memset(&pssts_arg, '\0', sizeof(pssts_arg));
			pssts_arg.i_op = NCS_PSSTS_OP_READ_FILE;
			pssts_arg.ncs_hdl = pwe_cb->p_pss_cb->pssts_hdl;
			pssts_arg.info.read_file.i_handle = file_hdl;
			pssts_arg.info.read_file.io_buffer = data;
			pssts_arg.info.read_file.i_bytes_to_read = buf_len;
			pssts_arg.info.read_file.i_offset = PSS_TABLE_DETAILS_HEADER_LEN;

			retval = (pwe_cb->p_pss_cb->pssts_api) (&pssts_arg);
			if ((retval != NCSCC_RC_SUCCESS) || (pssts_arg.info.read_file.o_bytes_read != buf_len)) {
				/* Close the file */
				m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api,
						       pwe_cb->p_pss_cb->pssts_hdl, retval, file_hdl);

				/* log the the read failure */
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);

				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
	}
	*entry_found = TRUE;

	/* At this point, the entry has been found or not found
	 * but it does not matter. If it was found, the data is
	 * already in buffer.
	 */

	if (*entry_found == TRUE)
		m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_ENTRY_FOUND, tbl_id);
	else
		m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_ENTRY_NOT_FOUND, tbl_id);

	m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, file_hdl);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_get_inst_ids_from_data

  DESCRIPTION:       This function takes in a data buffer and retrieves the
                     indexes from this buffer into the pinst_ids buffer. The
                     pinst_ids buffer is also allocated by the caller.

  RETURNS:           Number of inst ids encoded
                     0 - failure. Turn on m_MAB_DBG_SINK

*****************************************************************************/
uns32 pss_get_inst_ids_from_data(PSS_MIB_TBL_INFO *tbl_info, uns32 *pinst_ids, uns8 *buffer)
{
	PSS_VAR_INFO *var_info;
	uns8 data8;
	uns16 data16, max_len = 0x0000;
	uns32 i, j, inst_num = 0;
	uns32 offset;
	uns16 len16;
	NCS_BOOL data_bool;

	for (i = 0; i < tbl_info->ptbl_info->num_objects; i++) {
		var_info = &(tbl_info->pfields[i]);
		if (var_info->var_info.is_index_id == FALSE)
			continue;

		offset = var_info->offset;
		switch (var_info->var_info.fmat_id) {
		case NCSMIB_FMAT_INT:
			{
				/* Int can be of 3 different lengths */
				switch (var_info->var_info.len) {
				case (sizeof(uns8)):
					data8 = *(buffer + offset);
					pinst_ids[inst_num] = (uns32)data8;
					break;

				case (sizeof(uns16)):
					memcpy(&data16, buffer + offset, sizeof(uns16));
					pinst_ids[inst_num] = (uns32)data16;
					break;

				case (sizeof(uns32)):
					memcpy(&pinst_ids[inst_num], buffer + offset, sizeof(uns32));
					break;

				default:
					return m_MAB_DBG_SINK(0);
				}	/* switch (var_info->var_info.len) */

				inst_num++;

				break;
			}

		case NCSMIB_FMAT_OCT:
			{
				if (var_info->var_length == TRUE) {
					memcpy(&len16, buffer + offset, sizeof(uns16));
					pinst_ids[inst_num++] = len16;
					offset += sizeof(uns16);
				} else
					len16 = (uns16)var_info->var_info.obj_spec.stream_spec.max_len;

				if (var_info->var_info.obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
					max_len = ncsmiblib_get_max_len_of_discrete_octet_string(&var_info->var_info);
				else
					max_len = var_info->var_info.obj_spec.stream_spec.max_len;

				for (j = 0; j < len16; j++) {
					data8 = *(buffer + offset + j);
					pinst_ids[inst_num] = (uns32)data8;
					inst_num++;
				}	/* for (j = 0; j < len16; j++) */
			}
			break;

		case NCSMIB_FMAT_BOOL:
			{
				memcpy(&data_bool, buffer + offset, sizeof(NCS_BOOL));
				pinst_ids[inst_num++] = (uns32)data_bool;
			}
			break;

		default:
			return m_MAB_DBG_SINK(0);
		}		/* switch (var_info->var_info.fmat_id) */
	}			/* for (i = 0; i < tbl_info->ptbl_info->num_objects; i++) */

	/* Sanity check here */
	if (inst_num > tbl_info->num_inst_ids)
		return m_MAB_DBG_SINK(0);

	return inst_num;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_delete_inst_node_from_tree

  DESCRIPTION:       This function deletes a node from a patricia tree and
                     frees up the allocated memory

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_delete_inst_node_from_tree(NCS_PATRICIA_TREE *pTree, NCS_PATRICIA_NODE *pNode)
{
	uns32 retval;
	PSS_MIB_TBL_DATA *tbl_data = (PSS_MIB_TBL_DATA *)pNode;

	retval = ncs_patricia_tree_del(pTree, pNode);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_DEL_INST_NODE_FROM_PAT_TREE_FAIL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_MMGR_FREE_PSS_OCT(tbl_data->key);
	m_MMGR_FREE_PSS_OCT(tbl_data->data);
	m_MMGR_FREE_PSS_MIB_TBL_DATA(tbl_data);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_oac_warmboot_process_tbl

  DESCRIPTION:       This function reads in the data from the current profile
                     on the persistent store and issues SNMP SET requests for
                     a given OAC.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_oac_warmboot_process_tbl(PSS_PWE_CB *pwe_cb, char *p_pcn, PSS_TBL_REC *tbl_rec, PSS_CLIENT_ENTRY *node,
#if (NCS_PSS_RED == 1)
				   NCS_BOOL *i_continue_session,
#endif
				   NCS_BOOL is_end_of_playback)
{
	PSS_CB *inst = pwe_cb->p_pss_cb;
	uns8 *buffer = NULL, *ptr;
	uns32 *pinst_ids = NULL, *pinst_ids_first = NULL;
#if (NCS_PSS_RED == 1)
	uns32 *pinst_re_ids = NULL;
#endif
	uns32 retval = NCSCC_RC_SUCCESS, buf_size, item_size;
	uns32 bytes_read, num_items;
	long file_hdl = 0;
	uns32 max_num_rows;
	uns32 offset = 0, i, j;
	NCSMIB_PARAM_VAL pv;
	NCSMIB_IDX idx, idx_first;
	PSS_MIB_TBL_INFO *tbl_info;
	NCSROW_AID ra;
	NCSPARM_AID pa;
	USRBUF *ub = NULL;
	NCS_BOOL file_exists = FALSE, is_last_chunk = FALSE, pa_inited = FALSE;
	uns32 file_size = 0, rem_file_size = 0;

	tbl_info = inst->mib_tbl_desc[tbl_rec->tbl_id];

	memset(&ra, '\0', sizeof(ra));
	memset(&pa, '\0', sizeof(pa));
#if (NCS_PSS_RED == 1)
	if ((pwe_cb->processing_pending_active_events == TRUE) && ((*i_continue_session) == FALSE)) {
		if (pwe_cb->curr_plbck_ssn_info.tbl_id == 0)
			(*i_continue_session) = TRUE;	/* This is the first table in the playback session */
		else if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl_rec->tbl_id) &&
			 (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS)) {
			(*i_continue_session) = TRUE;	/* This table was already played-back in previous playback session */
			return NCSCC_RC_SUCCESS;
		}
	} else if (pwe_cb->processing_pending_active_events == FALSE) {
		(*i_continue_session) = TRUE;	/* This is normal mode */
	}
	if ((*i_continue_session) == TRUE) {
		if (pwe_cb->processing_pending_active_events == TRUE) {
			pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
		}
	} else {
		if ((pwe_cb->curr_plbck_ssn_info.tbl_id != 0) &&
		    (pwe_cb->curr_plbck_ssn_info.tbl_id != tbl_rec->tbl_id))
			return NCSCC_RC_SUCCESS;
	}
#endif

	/* First check if the file exists, if not, no data is to be restored */
	m_NCS_PSSTS_FILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
				inst->current_profile, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id, file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL,
				pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	if (file_exists == FALSE) {
#if (NCS_PSS_RED == 1)
		if (pwe_cb->processing_pending_active_events == TRUE) {
			if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl_rec->tbl_id) && ((*i_continue_session) == FALSE)) {
				/* Ask PSS to continue playback for the next table */
				(*i_continue_session) = TRUE;
				return NCSCC_RC_SUCCESS;
			}
		}
#endif
		return NCSCC_RC_SUCCESS;
	}

	/* Open the file from the current configuration */
	m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, inst->pssts_hdl, retval, inst->current_profile,
			      pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id, NCS_PSSTS_FILE_PERM_READ, file_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	item_size = tbl_info->max_row_length;
	max_num_rows = NCS_PSS_MAX_CHUNK_SIZE / item_size;
	if (max_num_rows == 0)	/* Should we return an error here? */
		max_num_rows = 1;

	buf_size = item_size * max_num_rows;
	buffer = m_MMGR_ALLOC_PSS_OCT(buf_size);
	if (buffer == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_oac_warmboot_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	pinst_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
	if (pinst_ids == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_INST_IDS_ALLOC_FAIL, "pss_oac_warmboot_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
#if (NCS_PSS_RED == 1)
	/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
	{
		pinst_re_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
		if (pinst_re_ids == NULL) {
			m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_INST_IDS_ALLOC_FAIL,
					  "pss_oac_warmboot_process_tbl()");
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}
	}
#endif
	pinst_ids_first = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
	if (pinst_ids_first == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_INST_IDS_ALLOC_FAIL, "pss_oac_warmboot_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(pinst_ids, '\0', tbl_info->num_inst_ids * sizeof(uns32));
	memset(pinst_ids_first, '\0', tbl_info->num_inst_ids * sizeof(uns32));
#if (NCS_PSS_RED == 1)
	memset(pinst_re_ids, '\0', tbl_info->num_inst_ids * sizeof(uns32));
#endif

	idx.i_inst_ids = pinst_ids;
	idx_first.i_inst_ids = pinst_ids_first;

	/* Get the total file size from the persistent store. 
	 * Read in the data from this file in chunks 
	 * and issue SETALLROWS requests from it,
	 * by calling into the MAC's ncsmib_timed_request function.
	 */
	m_NCS_PSSTS_FILE_SIZE(inst->pssts_api, inst->pssts_hdl, retval,
			      inst->current_profile, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id, file_size);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_FILE_SIZE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id);
		goto cleanup;
	}
	rem_file_size = file_size - PSS_TABLE_DETAILS_HEADER_LEN;
	offset = PSS_TABLE_DETAILS_HEADER_LEN;

	is_last_chunk = FALSE;
	while (rem_file_size != 0) {
		if (rem_file_size < tbl_info->max_row_length)
			break;

		m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl,
				      buf_size, offset, buffer, bytes_read);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			break;
		}

		rem_file_size -= bytes_read;
		if (rem_file_size < tbl_info->max_row_length) {
			is_last_chunk = TRUE;
		}

		num_items = bytes_read / item_size;
		if (num_items == 0)
			break;

		offset += num_items * item_size;
		/* Encode the set requests into a usrbuf */
		ptr = buffer;
		if ((inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETALLROWS) &&
		    (ra.uba.start == NULL)) {
			if ((retval = ncssetallrows_enc_init(&ra)) != NCSCC_RC_SUCCESS)
				break;
		}

		for (i = 0; i < num_items; i++) {
			uns32 lcl_param_cnt = 0, lcl_max_settable_params = 0;

			if (inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SET) {
				/* Required only for SETs */
				for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) {
					if (pss_get_bit_for_param(ptr, (j + 1)) == 0)
						continue;
					if ((tbl_info->pfields[j].var_info.is_index_id) &&
					    (!((tbl_info->pfields[j].var_info.is_readonly_persistent == TRUE) ||
					       (tbl_info->pfields[j].var_info.access == NCSMIB_ACCESS_READ_WRITE) ||
					       (tbl_info->pfields[j].var_info.access == NCSMIB_ACCESS_READ_CREATE))))
						continue;
					lcl_max_settable_params++;
				}
			}

			idx.i_inst_len = pss_get_inst_ids_from_data(tbl_info, pinst_ids, ptr);
			if (inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETALLROWS) {
				ncsrow_enc_init(&ra);
				if (i == 0) {	/* Preserve the first inst ids */
					memcpy(pinst_ids_first, pinst_ids, tbl_info->num_inst_ids * sizeof(uns32));
					idx_first.i_inst_len = idx.i_inst_len;
				}
				ncsrow_enc_inst_ids(&ra, &idx);
			}

			/* Process all the parameters in this row */
			lcl_param_cnt = lcl_max_settable_params;
			pa_inited = FALSE;
			for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) {
				uns32 param_offset = tbl_info->pfields[j].offset;

				if (pss_get_bit_for_param(ptr, (j + 1)) == 0)
					continue;
				if ((tbl_info->pfields[j].var_info.is_index_id) &&
				    (!((tbl_info->pfields[j].var_info.is_readonly_persistent == TRUE) ||
				       (tbl_info->pfields[j].var_info.access == NCSMIB_ACCESS_READ_WRITE) ||
				       (tbl_info->pfields[j].var_info.access == NCSMIB_ACCESS_READ_CREATE))))
					continue;

				if ((inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETROW) &&
				    (FALSE == pa_inited)) {
					memset(&pa, '\0', sizeof(pa));
					ncsparm_enc_init(&pa);
					pa_inited = TRUE;
				}

				switch (tbl_info->pfields[j].var_info.fmat_id) {
				case NCSMIB_FMAT_INT:
					pv.i_fmat_id = NCSMIB_FMAT_INT;
					pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
					switch (tbl_info->pfields[j].var_info.len) {
						uns16 data16;
					case sizeof(uns8):
						pv.info.i_int = (uns32)(*((uns8 *)(ptr + param_offset)));
						break;
					case sizeof(uns16):
						memcpy(&data16, ptr + param_offset, sizeof(uns16));
						pv.info.i_int = (uns32)data16;
						break;
					case sizeof(uns32):
						memcpy(&pv.info.i_int, ptr + param_offset, sizeof(uns32));
						break;
					default:
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					break;

				case NCSMIB_FMAT_BOOL:
					{
						NCS_BOOL data_bool;
						pv.i_fmat_id = NCSMIB_FMAT_INT;
						pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
						memcpy(&data_bool, ptr + param_offset, sizeof(NCS_BOOL));
						pv.info.i_int = (uns32)data_bool;
					}
					break;

				case NCSMIB_FMAT_OCT:
					{
						PSS_VAR_INFO *var_info = &(tbl_info->pfields[j]);
						if (var_info->var_length == TRUE) {
							memcpy(&pv.i_length, ptr + param_offset, sizeof(uns16));
							param_offset += sizeof(uns16);
						} else
							pv.i_length = var_info->var_info.obj_spec.stream_spec.max_len;

						pv.i_fmat_id = NCSMIB_FMAT_OCT;
						pv.i_param_id = var_info->var_info.param_id;
						pv.info.i_oct = ptr + param_offset;
					}
					break;

				default:
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}

				if (inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETALLROWS)
					ncsrow_enc_param(&ra, &pv);
				else if (inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETROW)
					ncsparm_enc_param(&pa, &pv);
#if (NCS_PSS_RED == 1)
				else if (((pwe_cb->processing_pending_active_events == TRUE) &&
					  (((*i_continue_session) == TRUE) ||
					   (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
					     ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					      (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						      idx.i_inst_len) == 0)))))) ||
					 (pwe_cb->processing_pending_active_events == FALSE))
#else
				else
#endif
				{
					/* Compose the SET request here. */
					NCSMIB_ARG mib_arg;
					NCSMEM_AID ma;
					uns8 space[2048];

#if (NCS_PSS_RED == 1)
					if ((pwe_cb->processing_pending_active_events == TRUE) &&
					    ((*i_continue_session) == FALSE)) {
						if ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
						    &&
						    (memcmp
						     (idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						      idx.i_inst_len) == 0)
						    && (pv.i_param_id == pwe_cb->curr_plbck_ssn_info.mib_obj_id)) {
							/* Found the last sent MIB object */
							(*i_continue_session) = TRUE;
							--lcl_param_cnt;
							/* Skip to the next MIB object */
							continue;	/* in the MIB-object for-loop itself. */
						}
					}
#endif
					memset(&mib_arg, 0, sizeof(mib_arg));
					ncsmib_init(&mib_arg);
					ncsmem_aid_init(&ma, space, sizeof(space));

					mib_arg.i_idx.i_inst_len = idx.i_inst_len;
					mib_arg.i_idx.i_inst_ids = pinst_ids;
					mib_arg.i_mib_key = pwe_cb->mac_key;
					mib_arg.i_op = NCSMIB_OP_REQ_SET;
					mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
					if (is_end_of_playback && is_last_chunk && (i == (num_items - 1))
					    && (lcl_param_cnt == 1)) {
						/* This is the last event of the last MIB row */
						mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
						m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SET, p_pcn,
									 pwe_cb->pwe_id, tbl_rec->tbl_id);
					}
					mib_arg.i_rsp_fnc = NULL;
					mib_arg.i_tbl_id = tbl_rec->tbl_id;
					mib_arg.i_usr_key = pwe_cb->hm_hdl;
					mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
					mib_arg.req.info.set_req.i_param_val = pv;

					m_PSS_LOG_NCSMIB_ARG(&mib_arg);

					retval = pss_sync_mib_req(&mib_arg, inst->mib_req_func, 1000, &ma);
#if (NCS_PSS_RED == 1)
					/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
					{
						/* Send Async updates to Standby */
						pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
						pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx.i_inst_len;
						memcpy(pinst_re_ids, pinst_ids, tbl_info->num_inst_ids * sizeof(uns32));
						pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids;
						pwe_cb->curr_plbck_ssn_info.mib_obj_id = pv.i_param_id;
						m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
					}
#endif
					if (retval != NCSCC_RC_SUCCESS) {
						retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
					} else if (mib_arg.i_op != NCSMIB_OP_RSP_SET) {
						retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
						m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SET);
					} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
						retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
						m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS,
								 mib_arg.rsp.i_status);
					} else {
						m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
					}
					--lcl_param_cnt;
				}
			}	/* for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) */

			if (inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETALLROWS) {
				ncsrow_enc_done(&ra);
			}
#if (NCS_PSS_RED == 1)
			else if ((inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETROW) &&
				 (((pwe_cb->processing_pending_active_events == TRUE) &&
				   (((*i_continue_session) == TRUE) ||
				    (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
				      ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
				       (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
					       idx.i_inst_len) == 0)))))) ||
				  (pwe_cb->processing_pending_active_events == FALSE)))
#else
			else if (inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETROW)
#endif
			{
				NCSMIB_ARG mib_arg;
				NCSMEM_AID ma;
				uns8 space[2048];

#if (NCS_PSS_RED == 1)
				if ((pwe_cb->processing_pending_active_events == TRUE) &&
				    ((*i_continue_session) == FALSE)) {
					if ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					    (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						    idx.i_inst_len) == 0)) {
						/* Found the last sent MIB object */
						(*i_continue_session) = TRUE;
						--lcl_param_cnt;
						/* Skip to the next MIB row */
						++i;
						ptr += item_size;
						continue;	/* continue in the MIB-row for-loop itself. */
					}
				}
#endif

				memset(&mib_arg, 0, sizeof(mib_arg));
				ncsmib_init(&mib_arg);
				ncsmem_aid_init(&ma, space, sizeof(space));

				mib_arg.i_idx.i_inst_len = idx.i_inst_len;
				mib_arg.i_idx.i_inst_ids = pinst_ids;
				mib_arg.i_mib_key = pwe_cb->mac_key;
				mib_arg.i_op = NCSMIB_OP_REQ_SETROW;
				mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
				if (is_end_of_playback && is_last_chunk && (i == (num_items - 1))) {
					mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
					m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SETROW, p_pcn,
								 pwe_cb->pwe_id, tbl_rec->tbl_id);
				}
				mib_arg.i_rsp_fnc = NULL;
				mib_arg.i_tbl_id = tbl_rec->tbl_id;
				mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
				mib_arg.i_usr_key = pwe_cb->hm_hdl;
				mib_arg.req.info.setrow_req.i_usrbuf = ncsparm_enc_done(&pa);
				if (mib_arg.req.info.setrow_req.i_usrbuf == NULL) {
					retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					goto cleanup;
				}
				m_BUFR_STUFF_OWNER(mib_arg.req.info.setrow_req.i_usrbuf);

				m_PSS_LOG_NCSMIB_ARG(&mib_arg);

				retval = pss_sync_mib_req(&mib_arg, inst->mib_req_func, 1000, &ma);
#if (NCS_PSS_RED == 1)
				/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
				{
					/* Send Async updates to Standby */
					pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
					pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx.i_inst_len;
					memcpy(pinst_re_ids, pinst_ids, tbl_info->num_inst_ids * sizeof(uns32));
					pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids;
					pwe_cb->curr_plbck_ssn_info.mib_obj_id = 0;
					m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
				}
#endif
				if (retval != NCSCC_RC_SUCCESS) {
					retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
				} else if (mib_arg.i_op != NCSMIB_OP_RSP_SETROW) {
					retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
					m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SETROW);
				} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
					retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
					m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS,
							 mib_arg.rsp.i_status);
				} else {
					m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
					m_MMGR_FREE_BUFR_LIST(mib_arg.rsp.info.setallrows_rsp.i_usrbuf);
					if (mib_arg.req.info.setrow_req.i_usrbuf != NULL)
						m_MMGR_FREE_BUFR_LIST(mib_arg.rsp.info.setrow_rsp.i_usrbuf);
				}
			}
			ptr += item_size;
		}		/* for (i = 0; i < num_items; i++) */

	}			/* while(TRUE) */

	if (inst->mib_tbl_desc[tbl_rec->tbl_id]->capability == NCSMIB_CAPABILITY_SETALLROWS) {
		NCSMIB_ARG mib_arg;
		uns8 space[2048];
		NCSMEM_AID ma;

		ub = ncssetallrows_enc_done(&ra);
		if (ub == NULL) {
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		memset(&mib_arg, 0, sizeof(mib_arg));
		ncsmem_aid_init(&ma, space, sizeof(space));
		/* Populate the mib_arg structure and send the request */
		ncsmib_init(&mib_arg);
		mib_arg.i_idx = idx_first;
		mib_arg.i_op = NCSMIB_OP_REQ_SETALLROWS;
		mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
		if (is_last_chunk && is_end_of_playback) {
			mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
			m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SETALLROWS, p_pcn, pwe_cb->pwe_id,
						 tbl_rec->tbl_id);
		}
		mib_arg.i_tbl_id = tbl_rec->tbl_id;
		m_BUFR_STUFF_OWNER(ub);
		mib_arg.req.info.setallrows_req.i_usrbuf = ub;
		ub = NULL;
		mib_arg.i_usr_key = pwe_cb->hm_hdl;
		mib_arg.i_mib_key = pwe_cb->mac_key;
		mib_arg.i_rsp_fnc = NULL;
		mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;

		m_PSS_LOG_NCSMIB_ARG(&mib_arg);

		retval = pss_sync_mib_req(&mib_arg, inst->mib_req_func, 1000, &ma);
#if (NCS_PSS_RED == 1)
		/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
		{
			/* Send Async updates to Standby */
			pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
			pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx_first.i_inst_len;
			memcpy(pinst_re_ids, idx_first.i_inst_ids, tbl_info->num_inst_ids * sizeof(uns32));
			pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids;
			pwe_cb->curr_plbck_ssn_info.mib_obj_id = 0;
			m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
		}
#endif
		if (retval != NCSCC_RC_SUCCESS) {
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
		} else if (mib_arg.i_op != NCSMIB_OP_RSP_SETALLROWS) {
			retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
			m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SETALLROWS);
		} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
			retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
			m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS, mib_arg.rsp.i_status);
		} else {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
			m_MMGR_FREE_BUFR_LIST(mib_arg.rsp.info.setallrows_rsp.i_usrbuf);
			mib_arg.rsp.info.setallrows_rsp.i_usrbuf = NULL;
		}
	}
 cleanup:

	if (ub != NULL)
		m_MMGR_FREE_BUFR_LIST(ub);

	if (buffer != NULL)
		m_MMGR_FREE_PSS_OCT(buffer);

	if (pinst_ids != NULL)
		m_MMGR_FREE_MIB_INST_IDS(pinst_ids);

	if (pinst_ids_first != NULL)
		m_MMGR_FREE_MIB_INST_IDS(pinst_ids_first);

#if (NCS_PSS_RED == 1)
	if (pinst_re_ids != NULL)
		m_MMGR_FREE_MIB_INST_IDS(pinst_re_ids);
	pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = NULL;
#endif

	if (file_hdl != 0)
		m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval, file_hdl);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_oac_warmboot_process_sclr_tbl

  DESCRIPTION:       This function issues appropriate SNMP requests for the 
                     entire elements of the only row in the particular scalar 
                     table.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_oac_warmboot_process_sclr_tbl(PSS_PWE_CB *pwe_cb, char *pcn, PSS_TBL_REC *tbl_rec, PSS_CLIENT_ENTRY *node,
#if (NCS_PSS_RED == 1)
					NCS_BOOL *i_continue_session,
#endif
					NCS_BOOL is_end_of_playback)
{
	PSS_CB *inst = pwe_cb->p_pss_cb;
	uns32 retval = NCSCC_RC_SUCCESS;
	NCS_BOOL curr_file_exists = FALSE;
	uns32 bytes_read = 0, j = 0, lcl_params_max = 0;
	long curr_file_hdl = 0;
	uns8 *curr_data = NULL;
	PSS_MIB_TBL_INFO *tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_rec->tbl_id];

#if (NCS_PSS_RED == 1)
	if ((pwe_cb->processing_pending_active_events == TRUE) &&
	    ((*i_continue_session) == FALSE) && (pwe_cb->curr_plbck_ssn_info.tbl_id == 0)) {
		(*i_continue_session) = TRUE;	/* This is the first table in the playback session */
	}
	if ((*i_continue_session) == TRUE) {
		if (pwe_cb->processing_pending_active_events == TRUE) {
			pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
		}
	} else {
		if (pwe_cb->curr_plbck_ssn_info.tbl_id != tbl_rec->tbl_id)
			return NCSCC_RC_SUCCESS;
	}
#endif

	/* First check if there is any data to be processed */
	m_NCS_PSSTS_FILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
				inst->current_profile, pwe_cb->pwe_id, pcn, tbl_rec->tbl_id, curr_file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL,
				pwe_cb->pwe_id, pcn, tbl_rec->tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	if (curr_file_exists == FALSE) {
#if (NCS_PSS_RED == 1)
		if (pwe_cb->processing_pending_active_events == TRUE) {
			if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl_rec->tbl_id) && ((*i_continue_session) == FALSE)) {
				/* Ask PSS to continue playback for the next table */
				(*i_continue_session) = TRUE;
				return NCSCC_RC_SUCCESS;
			}
		}
#endif
		return NCSCC_RC_SUCCESS;
	}

	/* Allocate memory for the buffers and keys */
	curr_data = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_row_length);
	if (curr_data == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_oac_warmboot_process_sclr_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(curr_data, '\0', tbl_info->max_row_length);

	/* Now open the files and read in data from them */
	m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, inst->pssts_hdl, retval,
			      inst->current_profile, pwe_cb->pwe_id, pcn, tbl_rec->tbl_id,
			      NCS_PSSTS_FILE_PERM_READ, curr_file_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, curr_file_hdl,
			      tbl_info->max_row_length, PSS_TABLE_DETAILS_HEADER_LEN, curr_data, bytes_read);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, pcn, tbl_rec->tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	if (bytes_read != tbl_info->max_row_length) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	lcl_params_max = pss_get_count_of_valid_params(curr_data, tbl_info);
	/* Send MIB SET requests for each of the parameters of the scalar table. */
	for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) {
		uns32 param_offset = tbl_info->pfields[j].offset;
		NCSMIB_PARAM_VAL pv;

		if (pss_get_bit_for_param(curr_data, (j + 1)) == 0)
			continue;

		memset(&pv, '\0', sizeof(pv));
		switch (tbl_info->pfields[j].var_info.fmat_id) {
		case NCSMIB_FMAT_INT:
			pv.i_fmat_id = NCSMIB_FMAT_INT;
			pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
			switch (tbl_info->pfields[j].var_info.len) {
				uns16 data16;
			case sizeof(uns8):
				pv.info.i_int = (uns32)(*((uns8 *)(curr_data + param_offset)));
				break;
			case sizeof(uns16):
				memcpy(&data16, curr_data + param_offset, sizeof(uns16));
				pv.info.i_int = (uns32)data16;
				break;
			case sizeof(uns32):
				memcpy(&pv.info.i_int, curr_data + param_offset, sizeof(uns32));
				break;
			default:
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			break;

		case NCSMIB_FMAT_BOOL:
			{
				NCS_BOOL data_bool;
				pv.i_fmat_id = NCSMIB_FMAT_INT;
				pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
				memcpy(&data_bool, curr_data + param_offset, sizeof(NCS_BOOL));
				pv.info.i_int = (uns32)data_bool;
			}
			break;

		case NCSMIB_FMAT_OCT:
			{
				PSS_VAR_INFO *var_info = &(tbl_info->pfields[j]);
				if (var_info->var_length == TRUE) {
					memcpy(&pv.i_length, curr_data + param_offset, sizeof(uns16));
					param_offset += sizeof(uns16);
				} else
					pv.i_length = var_info->var_info.obj_spec.stream_spec.max_len;

				pv.i_fmat_id = NCSMIB_FMAT_OCT;
				pv.i_param_id = var_info->var_info.param_id;
				pv.info.i_oct = curr_data + param_offset;
			}
			break;

		default:
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		/* Send the MIB SET request */
#if (NCS_PSS_RED == 1)
		if (((pwe_cb->processing_pending_active_events == TRUE) &&
		     (((*i_continue_session) == TRUE) ||
		      ((tbl_rec->tbl_id == pwe_cb->curr_plbck_ssn_info.tbl_id) &&
		       (pv.i_param_id == pwe_cb->curr_plbck_ssn_info.mib_obj_id)))) ||
		    (pwe_cb->processing_pending_active_events == FALSE))
#endif
		{
			NCSMIB_ARG mib_arg;
			NCSMEM_AID ma;
			uns8 space[2048];

#if (NCS_PSS_RED == 1)
			if ((pwe_cb->processing_pending_active_events == TRUE) &&
			    ((*i_continue_session) == FALSE) &&
			    ((tbl_rec->tbl_id == pwe_cb->curr_plbck_ssn_info.tbl_id) &&
			     (pv.i_param_id == pwe_cb->curr_plbck_ssn_info.mib_obj_id))) {
				/* Found the last sent MIB object */
				(*i_continue_session) = TRUE;
				--lcl_params_max;
				continue;	/* To the next object */
			}
#endif
			memset(&mib_arg, 0, sizeof(mib_arg));

			ncsmib_init(&mib_arg);
			ncsmem_aid_init(&ma, space, sizeof(space));

			mib_arg.i_idx.i_inst_len = 0;
			mib_arg.i_idx.i_inst_ids = NULL;
			mib_arg.i_mib_key = pwe_cb->mac_key;

			mib_arg.i_op = NCSMIB_OP_REQ_SET;
			mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
			if (is_end_of_playback && (lcl_params_max == 1)) {
				/* If it is the last event in the playback, set the bit. */
				mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
				m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SET, pcn, pwe_cb->pwe_id,
							 tbl_rec->tbl_id);
			}
			mib_arg.i_rsp_fnc = NULL;
			mib_arg.i_tbl_id = tbl_rec->tbl_id;
			mib_arg.i_usr_key = pwe_cb->hm_hdl;
			mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
			mib_arg.req.info.set_req.i_param_val = pv;

			m_PSS_LOG_NCSMIB_ARG(&mib_arg);

			retval = pss_sync_mib_req(&mib_arg, inst->mib_req_func, 1000, &ma);
#if (NCS_PSS_RED == 1)
			/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
			{
				/* Send Async updates to Standby */
				pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
				pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = 0;
				pwe_cb->curr_plbck_ssn_info.mib_obj_id = pv.i_param_id;
				m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
			}
#endif
			if (retval != NCSCC_RC_SUCCESS) {
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
			} else if (mib_arg.i_op != NCSMIB_OP_RSP_SET) {
				retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
				m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SET);
			} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
				retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
				m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS,
						 mib_arg.rsp.i_status);
			} else {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
			}
			--lcl_params_max;
		}
#if (NCS_PSS_RED == 1)
		else {
			/* These correspond to MIB objects which were already sent. So, 
			   we need count them as well. */
			--lcl_params_max;
		}
#endif
	}			/* for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) */

 cleanup:
	if (curr_data != NULL)
		m_MMGR_FREE_PSS_OCT(curr_data);

	if (curr_file_hdl != 0)
		m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval, curr_file_hdl);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_signal_start_of_playback

  DESCRIPTION:       This function broadcasts start of playback message to
                     MAC and OACs.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_signal_start_of_playback(PSS_PWE_CB *pwe_cb, USRBUF *ub)
{
	MAB_MSG msg;
	NCSMDS_INFO info;

	/* send messages to MAC and OAC about the start of the playback session */
	memset(&msg, 0, sizeof(msg));
	msg.vrid = pwe_cb->pwe_id;
	msg.op = MAB_MAC_PLAYBACK_START;
	if (ub != NULL)
		msg.data.data.plbck_start.i_ub = ub;

	memset(&info, 0, sizeof(info));
	info.i_mds_hdl = pwe_cb->mds_pwe_handle;
	info.i_op = MDS_SEND;
	info.i_svc_id = NCSMDS_SVC_ID_PSS;

	info.info.svc_send.i_msg = (NCSCONTEXT)&msg;
	info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
	info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_MAC;
	info.info.svc_send.i_sendtype = MDS_SENDTYPE_BCAST;
	info.info.svc_send.info.bcast.i_bcast_scope = NCSMDS_SCOPE_NONE;
	if (ncsmds_api(&info) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MAC_PBKS_FAILURE);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MAC_PBKS_SUCCESS);

	/* To fix the leak introduced to fix the crash in MDS broadcast processing to multiple receivers. */
	m_MMGR_FREE_BUFR_LIST(ub);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_signal_end_of_playback

  DESCRIPTION:       This function broadcasts end of playback message to
                     MAC and OACs.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_signal_end_of_playback(PSS_PWE_CB *pwe_cb)
{
	MAB_MSG msg;
	NCSMDS_INFO info;

	/* send messages to MAAs about the end of the playback session */
	memset(&msg, 0, sizeof(msg));
	msg.vrid = pwe_cb->pwe_id;
	msg.op = MAB_MAC_PLAYBACK_END;

	memset(&info, 0, sizeof(info));
	info.i_mds_hdl = pwe_cb->mds_pwe_handle;
	info.i_op = MDS_SEND;
	info.i_svc_id = NCSMDS_SVC_ID_PSS;
	info.info.svc_send.i_msg = (NCSCONTEXT)&msg;
	info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
	info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_MAC;
	info.info.svc_send.i_sendtype = MDS_SENDTYPE_BCAST;
	info.info.svc_send.info.bcast.i_bcast_scope = NCSMDS_SCOPE_NONE;
	if (ncsmds_api(&info) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_OAC_PBKE_FAILURE);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MAC_PBKE_SUCCESS);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_save_to_store

  DESCRIPTION:       This function saves the contents of a patricia tree to
                     the corresponding table file.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_save_to_store(PSS_PWE_CB *pwe_cb, NCS_PATRICIA_TREE *pTree, uns8 *profile_name, char *p_pcn, uns32 tbl_id)
{
	PSS_CB *inst = pwe_cb->p_pss_cb;
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_MIB_TBL_DATA *pData = NULL;
	PSS_MIB_TBL_INFO *tbl_info = NULL;
	uns8 *in_buf = NULL, *out_buf = NULL;
	uns8 *in_ptr, *out_ptr;
	uns8 *key = NULL;
	uns32 in_rows_left = 0, row_size, buf_size, max_num_rows;
	uns32 out_rows_filled = 0, read_offset = 0;
	long file_hdl = 0, tfile_hdl = 0;
	uns32 retval = NCSCC_RC_SUCCESS;
	NCS_BOOL file_exists;
	uns32 bytes_read;
	PSS_TABLE_PATH_RECORD ps_file_record;

	if (gl_pss_amf_attribs.ha_state != NCS_APP_AMF_HA_STATE_ACTIVE)
		return NCSCC_RC_SUCCESS;

	if ((ncs_patricia_tree_size(pTree) == 0) && (profile_name == NULL)) {
		return NCSCC_RC_SUCCESS;
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SAVE_TO_STORE_START);
	tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];
	row_size = tbl_info->max_row_length;
	max_num_rows = NCS_PSS_MAX_CHUNK_SIZE / row_size;
	if (max_num_rows == 0)	/* Or should we return an error here? */
		max_num_rows = 1;

	buf_size = max_num_rows * row_size;

	/* Allocate memory for the buffers */
	in_buf = m_MMGR_ALLOC_PSS_OCT(buf_size);
	if (in_buf == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_save_to_store()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	in_ptr = in_buf;
	memset(in_buf, '\0', buf_size);

	out_buf = m_MMGR_ALLOC_PSS_OCT(buf_size);
	if (out_buf == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_save_to_store()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(out_buf, '\0', buf_size);
	out_ptr = out_buf;

	key = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_key_length);
	if (key == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_save_to_store()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(key, '\0', tbl_info->max_key_length);

	pNode = NULL;
	if (pTree->n_nodes != 0)
		pNode = ncs_patricia_tree_getnext(pTree, NULL);
	pData = (PSS_MIB_TBL_DATA *)pNode;

	/* Open the temporary file for writing */
	m_NCS_PSSTS_FILE_OPEN_TEMP(inst->pssts_api, inst->pssts_hdl, retval, tfile_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_OPEN_TEMP_FILE_FAIL);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	/* Check if the table file exists */
	m_NCS_PSSTS_FILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
				inst->current_profile, pwe_cb->pwe_id, p_pcn, tbl_id, file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	if (file_exists == FALSE) {
		/* 3.0.b addition: Writing Table details header temp file */

		memset(&ps_file_record, 0, sizeof(PSS_TABLE_PATH_RECORD));
		/* Filling the current profile, pwe_id, pcn and table_id */
		memcpy(&ps_file_record.profile, &inst->current_profile, NCS_PSS_MAX_PROFILE_NAME);
		ps_file_record.pwe_id = pwe_cb->pwe_id;
		memcpy(&ps_file_record.pcn, &p_pcn, NCSMIB_PCN_LENGTH_MAX);
		ps_file_record.tbl_id = tbl_id;
		/* Write the table details header into temp file */
		retval = pss_tbl_details_header_write(inst, inst->pssts_hdl, tfile_hdl, &ps_file_record);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}
		/* End of 3.0.b addition */
		goto write_tree_to_file;
	}

	/* Open the table file for reading */
	m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, inst->pssts_hdl, retval,
			      inst->current_profile, pwe_cb->pwe_id, p_pcn, tbl_id, NCS_PSSTS_FILE_PERM_READ, file_hdl)
	    if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN2_SET_OPEN_MIB_FILE_FAIL, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	/* 3.0.b addition: Reading the table details header from disk-image of the table and writing it to temp file */

	m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl,
			      PSS_TABLE_DETAILS_HEADER_LEN, read_offset, in_buf, bytes_read);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval,
			       tfile_hdl, PSS_TABLE_DETAILS_HEADER_LEN, in_buf);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	read_offset = PSS_TABLE_DETAILS_HEADER_LEN;
	/* End of 3.0.b addition */

	m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl,
			      buf_size, read_offset, in_buf, bytes_read);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	in_rows_left = bytes_read / row_size;
	read_offset += (in_rows_left * row_size);

	while ((pTree->n_nodes != 0) && (pNode != NULL) && (in_rows_left > 0)) {
		int res;

		pData = (PSS_MIB_TBL_DATA *)pNode;
		pss_get_key_from_data(tbl_info, key, in_ptr);
		res = memcmp(pData->key, key, tbl_info->max_key_length);

		if (res <= 0) {
			if (pData->deleted == FALSE) {
				memcpy(out_ptr, pData->data, row_size);
				out_ptr += row_size;
				out_rows_filled++;
			}
			pss_delete_inst_node_from_tree(pTree, pNode);
			pwe_cb->p_pss_cb->mem_in_store -=
			    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);
			pNode = NULL;
			if (pTree->n_nodes != 0)
				pNode = ncs_patricia_tree_getnext(pTree, NULL);
			if (res == 0) {
				in_ptr += row_size;
				in_rows_left--;
			}
		} else if (res > 0) {
			memcpy(out_ptr, in_ptr, row_size);
			out_ptr += row_size;
			out_rows_filled++;
			in_ptr += row_size;
			in_rows_left--;
		}

		/* Read in more data from the table file if needed */
		if (in_rows_left == 0) {
			m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl,
					      buf_size, read_offset, in_buf, bytes_read);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}

			in_ptr = in_buf;
			in_rows_left = bytes_read / row_size;
			read_offset += (in_rows_left * row_size);
		}

		/* If the output buffer is full, write it to the file */
		if (out_rows_filled == max_num_rows) {
			m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval, tfile_hdl, buf_size, out_buf);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}

			out_rows_filled = 0;
			out_ptr = out_buf;
		}
	}

	/* Transfer the rest of the data from the tbl_file to the temp file */
	if (in_rows_left > 0) {
		/* If there is any data in the out_buf, write it */
		if (out_rows_filled > 0) {
			m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval,
					       tfile_hdl, (out_rows_filled * row_size), out_buf);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}

			out_rows_filled = 0;
			out_ptr = out_buf;
		}

		/* There is some data in the in_buf, write it to the temp file */
		{
			m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval,
					       tfile_hdl, (in_rows_left * row_size), in_ptr);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}

			in_rows_left = 0;
			in_ptr = in_buf;
		}

		/* Now read in data from the table file and write to the temp file */
		while (TRUE) {
			m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl,
					      buf_size, read_offset, in_buf, bytes_read);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}

			in_rows_left = bytes_read / row_size;
			read_offset += (in_rows_left * row_size);
			/* If no more data, then break out of the loop */
			if (in_rows_left == 0)
				break;

			m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval,
					       tfile_hdl, (in_rows_left * row_size), in_buf);
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}
		}		/* while (TRUE) */
	}

 write_tree_to_file:
	/* If there is still some data in the patricia tree, write it to the disk */
	if (pNode != NULL) {
		while ((pTree->n_nodes != 0) && (pNode != NULL)) {
			pData = (PSS_MIB_TBL_DATA *)pNode;
			if (pData->deleted == FALSE) {
				memcpy(out_ptr, pData->data, row_size);
				out_ptr += row_size;
				out_rows_filled++;
			}
			pss_delete_inst_node_from_tree(pTree, pNode);
			pwe_cb->p_pss_cb->mem_in_store -=
			    (sizeof(PSS_MIB_TBL_DATA) + tbl_info->max_key_length + tbl_info->max_row_length);

			pNode = NULL;
			if (pTree->n_nodes != 0)
				pNode = ncs_patricia_tree_getnext(pTree, NULL);
			pData = (PSS_MIB_TBL_DATA *)pNode;

			/* If the output buffer is full, write it to the file */
			if (out_rows_filled == max_num_rows) {
				m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval,
						       tfile_hdl, buf_size, out_buf);
				if (retval != NCSCC_RC_SUCCESS) {
					m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL,
							pwe_cb->pwe_id, p_pcn, tbl_id);
					retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					goto cleanup;
				}

				out_rows_filled = 0;
				out_ptr = out_buf;
			}
		}		/* while (pNode != NULL) */
	}

	/* If there is still some data in the out_buf, write it to the store */
	if (out_rows_filled > 0) {
		m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval,
				       tfile_hdl, (out_rows_filled * row_size), out_buf);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		out_rows_filled = 0;
		out_ptr = out_buf;
	}

 cleanup:
	if (in_buf != NULL)
		m_MMGR_FREE_PSS_OCT(in_buf);
	if (out_buf != NULL)
		m_MMGR_FREE_PSS_OCT(out_buf);
	if (key != NULL)
		m_MMGR_FREE_PSS_OCT(key);

	if (file_hdl != 0) {
		uns32 retval2;
		m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval2, file_hdl);
	}

	if (tfile_hdl != 0) {
		uns32 retval2;
		m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval2, tfile_hdl);
	}

	/* If all the operations have been successful,
	 * copy the temporary file to the table file.
	 */
	if (retval == NCSCC_RC_SUCCESS) {
		m_NCS_PSSTS_FILE_COPY_TEMP(inst->pssts_api, inst->pssts_hdl, retval,
					   inst->current_profile, pwe_cb->pwe_id, p_pcn, tbl_id);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_COPY_TEMP_FILE_FAIL);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	/* If a different profile name has been specified,
	 * then copy the temporary file to the table file
	 * for this profile also.
	 */
	if ((profile_name != NULL) && (profile_name[0] != '\0')) {
		m_NCS_PSSTS_FILE_COPY_TEMP(inst->pssts_api, inst->pssts_hdl, retval,
					   profile_name, pwe_cb->pwe_id, p_pcn, tbl_id);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_COPY_TEMP_FILE_TO_ALT_PROFILE_FAIL);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SAVE_TO_STORE_END);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_save_to_sclr_store

  DESCRIPTION:       This function saves the contents of a scalar table(one
                     row only) to the corresponding table file.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_save_to_sclr_store(PSS_PWE_CB *pwe_cb, PSS_TBL_REC *tbl_rec, uns8 *profile_name, char *p_pcn, uns32 tbl_id)
{
	PSS_CB *inst = pwe_cb->p_pss_cb;
	PSS_MIB_TBL_INFO *tbl_info;
	uns8 *pData;
	uns32 row_size, buf_size;
	long tfile_hdl = 0;
	uns32 retval = NCSCC_RC_SUCCESS;
	NCS_BOOL file_exists;
	PSS_TABLE_PATH_RECORD ps_file_record;

	if (gl_pss_amf_attribs.ha_state != NCS_APP_AMF_HA_STATE_ACTIVE)
		return NCSCC_RC_SUCCESS;

	pData = tbl_rec->info.scalar.data;
	if (pData == NULL)	/* If no data present, return */
		return NCSCC_RC_SUCCESS;

	if (tbl_rec->dirty == FALSE)
		goto cleanup;

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SAVE_TO_STORE_START);
	tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_id];
	row_size = tbl_info->max_row_length;

	buf_size = 1 * row_size;

	/* Check if the table file exists */
	m_NCS_PSSTS_FILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval,
				inst->current_profile, pwe_cb->pwe_id, p_pcn, tbl_id, file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	/* Open the temporary file for writing */
	m_NCS_PSSTS_FILE_OPEN_TEMP(inst->pssts_api, inst->pssts_hdl, retval, tfile_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_OPEN_TEMP_FILE_FAIL);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	/* 3.0.b addition: Writing Table details header temp file */

	memset(&ps_file_record, 0, sizeof(PSS_TABLE_PATH_RECORD));
	/* Filling the current profile, pwe_id, pcn and table_id */
	memcpy(&ps_file_record.profile, &inst->current_profile, NCS_PSS_MAX_PROFILE_NAME);
	ps_file_record.pwe_id = pwe_cb->pwe_id;
	memcpy(&ps_file_record.pcn, &p_pcn, NCSMIB_PCN_LENGTH_MAX);
	ps_file_record.tbl_id = tbl_id;
	/* Write the table details header into temp file */
	retval = pss_tbl_details_header_write(inst, inst->pssts_hdl, tfile_hdl, &ps_file_record);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	/* End of 3.0.b addition */

	/* Open the temp file for writing */
	m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, inst->pssts_hdl, retval, tfile_hdl, buf_size, pData);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, pwe_cb->pwe_id, p_pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

 cleanup:
	if (tfile_hdl != 0) {
		uns32 retval2;
		m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval2, tfile_hdl);
	}

	/* Update current profile first. */
	m_NCS_PSSTS_FILE_COPY_TEMP(inst->pssts_api, inst->pssts_hdl, retval,
				   inst->current_profile, pwe_cb->pwe_id, p_pcn, tbl_id);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_COPY_TEMP_FILE_FAIL);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* If a different profile name has been specified,
	 * then copy the temporary file to the table file
	 * for this profile also.
	 */
	if ((profile_name != NULL) && (profile_name[0] != '\0')) {
		m_NCS_PSSTS_FILE_COPY_TEMP(inst->pssts_api, inst->pssts_hdl, retval,
					   profile_name, pwe_cb->pwe_id, p_pcn, tbl_id);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_COPY_TEMP_FILE_TO_ALT_PROFILE_FAIL);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	}

	/* Free up the data in the scalar node pointer. Not required here. The 
	   data can be retreived whenever required. */
	m_MMGR_FREE_PSS_OCT(tbl_rec->info.scalar.data);
	tbl_rec->info.scalar.data = NULL;

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SAVE_TO_STORE_END);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_set_key_from_inst_ids

  DESCRIPTION:       This function takes in inst_ids and retrieves the
                     key into the caller-allocated buffer pkey.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn DBG_SINK

*****************************************************************************/
uns32 pss_set_key_from_inst_ids(PSS_MIB_TBL_INFO *tbl_info, uns8 *key, uns32 num_inst_ids, uns32 *inst_ids)
{
	uns32 i, j, counter;
	uns16 len16;
	uns8 *ptr = key;
	PSS_VAR_INFO *var_info;

	counter = 0;
	memset(key, 0, tbl_info->max_key_length);
	/*memset(key, '\0', NCS_PATRICIA_MAX_KEY_SIZE); */
	for (i = 0; i < tbl_info->ptbl_info->num_objects; i++) {
		var_info = &(tbl_info->pfields[i]);
		if (var_info->var_info.is_index_id == FALSE)
			continue;

		switch (var_info->var_info.fmat_id) {
		case NCSMIB_FMAT_INT:
			{
				switch (var_info->var_info.len) {
				case sizeof(uns8):
					ncs_encode_8bit(&ptr, (uns8)inst_ids[counter]);
					break;
				case sizeof(uns16):
					ncs_encode_16bit(&ptr, (uns16)inst_ids[counter]);
					break;
				case sizeof(uns32):
					ncs_encode_32bit(&ptr, inst_ids[counter]);
					break;
				default:
					m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR,
							    PSS_HDLN2_SET_KEY_FROM_INST_IDS_FMAT_INT_NOT_SUPPORTED);
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}
				counter++;
			}
			break;

		case NCSMIB_FMAT_OCT:
			{
				uns16 max_len = 0x0000;

				if (var_info->var_length == TRUE)
					len16 = (uns16)inst_ids[counter++];
				else
					len16 = var_info->var_info.obj_spec.stream_spec.max_len;

				if (var_info->var_info.obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
					max_len = ncsmiblib_get_max_len_of_discrete_octet_string(&var_info->var_info);
				else
					max_len = var_info->var_info.obj_spec.stream_spec.max_len;

				for (j = 0; j < len16; j++)
					ncs_encode_8bit(&ptr, (uns8)inst_ids[counter++]);

				for (j = len16; j < max_len; j++)
					ncs_encode_8bit(&ptr, 0);
			}
			break;

		default:
			m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_KEY_FROM_INST_IDS_FMAT_ID_NOT_SUPPORTED);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}		/* switch(var_info->var_info.fmat_id) */
	}			/* for (i = 0; i < tbl_info->ptbl_info->num_objects; i++) */

	/* Just a sanity check here */
	if (counter != num_inst_ids) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_KEY_FRM_INST_IDS_NUM_INST_IDS_MISMATCH);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_get_key_from_data

  DESCRIPTION:       This function takes in a data buffer and retrieves the
                     key into the caller-allocated buffer pkey.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn DBG_SINK

*****************************************************************************/
uns32 pss_get_key_from_data(PSS_MIB_TBL_INFO *tbl_info, uns8 *pkey, uns8 *buffer)
{
	PSS_VAR_INFO *var_info;
	uns16 data16;
	uns32 i, j;
	uns32 offset, data32;
	uns8 *ptr = pkey, data8;
	uns16 len16;

	memset(ptr, 0, tbl_info->max_key_length);
	for (i = 0; i < tbl_info->ptbl_info->num_objects; i++) {
		var_info = &(tbl_info->pfields[i]);
		if (var_info->var_info.is_index_id == FALSE)
			continue;

		offset = var_info->offset;
		switch (var_info->var_info.fmat_id) {
		case NCSMIB_FMAT_INT:
			{
				/* Int can be of 3 different lengths */
				switch (var_info->var_info.len) {
				case (sizeof(uns8)):
					ncs_encode_8bit(&ptr, *(buffer + offset));
					break;

				case (sizeof(uns16)):
					memcpy(&data16, buffer + offset, sizeof(uns16));
					ncs_encode_16bit(&ptr, data16);
					break;

				case (sizeof(uns32)):
					memcpy(&data32, buffer + offset, sizeof(uns32));
					ncs_encode_32bit(&ptr, data32);
					break;

				default:
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}	/* switch (var_info->var_info.len) */

				break;
			}

		case NCSMIB_FMAT_OCT:
			{
				uns16 max_len = 0x0000;

				if (var_info->var_length == TRUE) {
					memcpy(&len16, (buffer + offset), sizeof(uns16));
					offset += sizeof(uns16);
					if (var_info->var_info.obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
						max_len =
						    ncsmiblib_get_max_len_of_discrete_octet_string(&var_info->var_info);
					else
						max_len = var_info->var_info.obj_spec.stream_spec.max_len;
				} else
					len16 = max_len = var_info->var_info.obj_spec.stream_spec.max_len;

				for (j = 0; j < len16; j++) {
					data8 = *(buffer + offset + j);
					ncs_encode_8bit(&ptr, data8);
				}
				for (j = len16; j < max_len; j++)
					ncs_encode_8bit(&ptr, 0);
			}
			break;

		default:
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}		/* switch (var_info->var_info.fmat_id) */
	}			/* for (i = 0; i < tbl_info->ptbl_info->num_objects; i++) */

	/* Sanity check here */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_mib_request

  DESCRIPTION:       This function processes the CLI commands which are 
                     received in the form of a MIB request.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_process_mib_request(MAB_MSG *msg)
{
	PSS_CB *inst = (PSS_CB *)msg->yr_hdl;
	NCSMIB_ARG *arg = msg->data.data.snmp;
	uns32 retval = NCSCC_RC_SUCCESS;

	if (inst == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_INST);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (arg == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_RECEIVED_CLI_REQ);

	switch (arg->i_tbl_id) {
	case NCSMIB_TBL_PSR_PROFILES:
		retval = pss_process_tbl_pss_profiles(inst, arg);
		ncsmib_memfree(arg);
		break;

	case NCSMIB_SCLR_PSR_TRIGGER:
		retval = pss_process_sclr_trigger(inst, arg);
		ncsmib_memfree(arg);
		break;

	case NCSMIB_TBL_PSR_CMD:
		retval = pss_process_tbl_cmd(inst, arg);
		ncsmib_memfree(arg);
		break;

	default:
		arg->rsp.i_status = m_MAB_DBG_SINK(NCSCC_RC_NO_SUCH_TBL);
		arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
		arg->i_rsp_fnc(arg);
		ncsmib_arg_free_resources(arg, TRUE);
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_CLI_INV_TBL);
		break;
	}

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_tbl_pss_profiles

  DESCRIPTION:       This function processes the MIB requests for the table
                     NCSMIB_TBL_PSR_PROFILES.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_process_tbl_pss_profiles(PSS_CB *inst, NCSMIB_ARG *arg)
{
	uns32 retval;
	NCSMIBLIB_REQ_INFO miblib_req;

	if (!m_NCSMIB_ISIT_A_REQ(arg->i_op))
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	memset(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

	miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
	miblib_req.info.i_mib_op_info.args = arg;
	miblib_req.info.i_mib_op_info.cb = inst;

	/* call the mib routine handler */
	retval = ncsmiblib_process_req(&miblib_req);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_sclr_trigger

  DESCRIPTION:       This function processes the MIB requests on the scalar
                     table NCSMIB_SCLR_PSR_TRIGGER.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_process_sclr_trigger(PSS_CB *inst, NCSMIB_ARG *arg)
{
	uns32 retval;
	NCSMIBLIB_REQ_INFO miblib_req;
	/* Currently, the scalar value can only be set to enabled or disabled */

	if (!m_NCSMIB_ISIT_A_REQ(arg->i_op))
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	memset(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

	miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
	miblib_req.info.i_mib_op_info.args = arg;
	miblib_req.info.i_mib_op_info.cb = inst;

	/* call the mib routine handler */
	retval = ncsmiblib_process_req(&miblib_req);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_save_current_configuration

  DESCRIPTION:       This function saves the in-memory configuration to the
                     persistent store.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_save_current_configuration(PSS_CB *inst)
{
	PSS_PWE_CB *pwe_cb = NULL;
	PSS_TBL_REC *rec = NULL;
	PSS_CSI_NODE *t_csi = NULL;
	uns32 retval = NCSCC_RC_SUCCESS;

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SAVE_CURR_CONF);

	for (t_csi = gl_pss_amf_attribs.csi_list; t_csi != NULL; t_csi = t_csi->next) {
		PSS_RFRSH_TBL_LIST *tmp = NULL, *prv_tmp = NULL;

		pwe_cb = t_csi->pwe_cb;
		if (pwe_cb == NULL)
			continue;

		/* Traverse the refresh list and save the data */
		for (tmp = pwe_cb->refresh_tbl_list; tmp != NULL; prv_tmp = tmp, tmp = tmp->next) {
			if (prv_tmp != NULL) {
				m_MMGR_FREE_PSS_RFRSH_TBL_LIST(prv_tmp);
				prv_tmp = NULL;
			}
			rec = tmp->pss_tbl_rec;
			if (rec->is_scalar) {
				retval = pss_save_to_sclr_store(pwe_cb, rec, NULL,
								(char *)&rec->pss_client_key->pcn, rec->tbl_id);
			} else {
				if (rec->info.other.tree_inited == TRUE) {
					retval = pss_save_to_store(pwe_cb, &rec->info.other.data, NULL,
								   (char *)&rec->pss_client_key->pcn, rec->tbl_id);
				}
			}
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_SAVE_TBL_ERROR);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			rec->dirty = FALSE;
		}		/* for-loop (tmp != NULL) */
		if (prv_tmp != NULL) {
			m_MMGR_FREE_PSS_RFRSH_TBL_LIST(prv_tmp);
			prv_tmp = NULL;
		}
		pwe_cb->refresh_tbl_list = NULL;
	}			/* for-loop for all CSIs */

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_SAVE_CURR_CONF_DONE);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_get_profile_from_inst_ids

  DESCRIPTION:       This function retrieves a profile name from the inst ids

  RETURNS:           void

*****************************************************************************/
void pss_get_profile_from_inst_ids(uns8 *profile, uns32 num_inst_ids, const uns32 *inst_ids)
{
	uns32 i;

	for (i = 1; i < num_inst_ids; i++)	/* Skipping the length - first oid */
		profile[i - 1] = (uns8)inst_ids[i];

	profile[num_inst_ids - 1] = '\0';

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_playback_process_sclr_tbl

  DESCRIPTION:       This function issues appropriate SNMP requests for the 
                     entire elements of the only row in the particular scalar 
                     table.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_playback_process_sclr_tbl(PSS_PWE_CB *pwe_cb, uns8 *profile, PSS_TBL_REC *tbl_rec,
#if (NCS_PSS_RED == 1)
				    NCS_BOOL *i_continue_session,
#endif
				    NCS_BOOL last_plbck_evt)
{
	uns32 retval = NCSCC_RC_SUCCESS;
	NCS_BOOL alt_file_exists = FALSE, first_set_ready = FALSE;
	uns32 bytes_read = 0, j = 0, lcl_params_max = 0;
	long alt_file_hdl = 0;
	uns8 *alt_data = NULL;
	PSS_MIB_TBL_INFO *tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl_rec->tbl_id];
	char *p_pcn = (char *)&tbl_rec->pss_client_key->pcn;

#if (NCS_PSS_RED == 1)
	if ((pwe_cb->processing_pending_active_events == TRUE) &&
	    ((*i_continue_session) == FALSE) && (pwe_cb->curr_plbck_ssn_info.tbl_id == 0)) {
		(*i_continue_session) = TRUE;	/* This is the first table in the playback session */
	}
	if ((*i_continue_session) == TRUE) {
		if (pwe_cb->processing_pending_active_events == TRUE) {
			pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
		}
	} else {
		if (pwe_cb->curr_plbck_ssn_info.tbl_id != tbl_rec->tbl_id)
			return NCSCC_RC_SUCCESS;
	}
#endif

	/* First check if there is any data to be processed */
	m_NCS_PSSTS_FILE_EXISTS(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
				retval, profile, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id, alt_file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL,
				pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	if (alt_file_exists == FALSE) {
#if (NCS_PSS_RED == 1)
		if (pwe_cb->processing_pending_active_events == TRUE) {
			if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl_rec->tbl_id) && ((*i_continue_session) == FALSE)) {
				/* Ask PSS to continue playback for the next table */
				(*i_continue_session) = TRUE;
				return NCSCC_RC_SUCCESS;
			}
		}
#endif
		return NCSCC_RC_SUCCESS;
	}

	/* Allocate memory for the buffers and keys */
	alt_data = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_row_length);
	if (alt_data == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_playback_process_sclr_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(alt_data, '\0', tbl_info->max_row_length);

	/* Now open the files and read in data from them */
	m_NCS_PSSTS_FILE_OPEN(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
			      retval, profile, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id, NCS_PSSTS_FILE_PERM_READ,
			      alt_file_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	m_NCS_PSSTS_FILE_READ(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
			      retval, alt_file_hdl, tbl_info->max_row_length, PSS_TABLE_DETAILS_HEADER_LEN, alt_data,
			      bytes_read);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, tbl_rec->tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	if (bytes_read != tbl_info->max_row_length) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	lcl_params_max = pss_get_count_of_valid_params(alt_data, tbl_info);

	/* Send MIB SET requests for each of the parameters of the scalar table. */
	for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) {
		uns32 param_offset = tbl_info->pfields[j].offset;
		NCSMIB_PARAM_VAL pv;

		if (pss_get_bit_for_param(alt_data, (j + 1)) == 0)
			continue;

		memset(&pv, '\0', sizeof(pv));
		switch (tbl_info->pfields[j].var_info.fmat_id) {
		case NCSMIB_FMAT_INT:
			pv.i_fmat_id = NCSMIB_FMAT_INT;
			pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
			switch (tbl_info->pfields[j].var_info.len) {
				uns16 data16;
			case sizeof(uns8):
				pv.info.i_int = (uns32)(*((uns8 *)(alt_data + param_offset)));
				break;
			case sizeof(uns16):
				memcpy(&data16, alt_data + param_offset, sizeof(uns16));
				pv.info.i_int = (uns32)data16;
				break;
			case sizeof(uns32):
				memcpy(&pv.info.i_int, alt_data + param_offset, sizeof(uns32));
				break;
			default:
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			break;

		case NCSMIB_FMAT_BOOL:
			{
				NCS_BOOL data_bool;
				pv.i_fmat_id = NCSMIB_FMAT_INT;
				pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
				memcpy(&data_bool, alt_data + param_offset, sizeof(NCS_BOOL));
				pv.info.i_int = (uns32)data_bool;
			}
			break;

		case NCSMIB_FMAT_OCT:
			{
				PSS_VAR_INFO *var_info = &(tbl_info->pfields[j]);
				if (var_info->var_length == TRUE) {
					memcpy(&pv.i_length, alt_data + param_offset, sizeof(uns16));
					param_offset += sizeof(uns16);
				} else
					pv.i_length = var_info->var_info.obj_spec.stream_spec.max_len;

				pv.i_fmat_id = NCSMIB_FMAT_OCT;
				pv.i_param_id = var_info->var_info.param_id;
				pv.info.i_oct = alt_data + param_offset;
			}
			break;

		default:
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		/* Send the MIB SET request */
#if (NCS_PSS_RED == 1)
		if (((pwe_cb->processing_pending_active_events == TRUE) &&
		     (((*i_continue_session) == TRUE) ||
		      ((tbl_rec->tbl_id == pwe_cb->curr_plbck_ssn_info.tbl_id) &&
		       (pv.i_param_id == pwe_cb->curr_plbck_ssn_info.mib_obj_id)))) ||
		    (pwe_cb->processing_pending_active_events == FALSE))
#endif
		{
			NCSMIB_ARG mib_arg;
			NCSMEM_AID ma;
			uns8 space[2048];

#if (NCS_PSS_RED == 1)
			if ((pwe_cb->processing_pending_active_events == TRUE) &&
			    ((*i_continue_session) == FALSE) &&
			    ((tbl_rec->tbl_id == pwe_cb->curr_plbck_ssn_info.tbl_id) &&
			     (pv.i_param_id == pwe_cb->curr_plbck_ssn_info.mib_obj_id))) {
				/* Found the last sent MIB object */
				(*i_continue_session) = TRUE;
				--lcl_params_max;
				++j;
				continue;	/* To the next object */
			}
#endif
			if (first_set_ready == FALSE) {
				/* Send the REMOVEROWS for the scalar table only once. */
				memset(&mib_arg, 0, sizeof(mib_arg));
				ncsmib_init(&mib_arg);
				ncsmem_aid_init(&ma, space, sizeof(space));

				mib_arg.i_idx.i_inst_len = 0;
				mib_arg.i_idx.i_inst_ids = NULL;
				mib_arg.i_mib_key = pwe_cb->mac_key;

				mib_arg.i_op = NCSMIB_OP_REQ_REMOVEROWS;
				mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
				mib_arg.i_rsp_fnc = NULL;
				mib_arg.i_tbl_id = tbl_rec->tbl_id;
				mib_arg.i_usr_key = pwe_cb->hm_hdl;
				mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;

				m_PSS_LOG_NCSMIB_ARG(&mib_arg);

				retval = pss_sync_mib_req(&mib_arg, pwe_cb->p_pss_cb->mib_req_func, 1000, &ma);
				if (retval != NCSCC_RC_SUCCESS) {
					retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
				} else if (mib_arg.i_op != NCSMIB_OP_RSP_REMOVEROWS) {
					retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
					m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_REMOVEROWS);
				} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
					retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
					m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS);
				} else {
					m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
				}
				first_set_ready = TRUE;
			}

			memset(&mib_arg, 0, sizeof(mib_arg));

			ncsmib_init(&mib_arg);
			ncsmem_aid_init(&ma, space, sizeof(space));

			mib_arg.i_idx.i_inst_len = 0;
			mib_arg.i_idx.i_inst_ids = NULL;
			mib_arg.i_mib_key = pwe_cb->mac_key;

			mib_arg.i_op = NCSMIB_OP_REQ_SET;
			mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
			if (last_plbck_evt && (lcl_params_max == 1)) {
				mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
				m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SET, p_pcn, pwe_cb->pwe_id,
							 tbl_rec->tbl_id);
			}
			mib_arg.i_rsp_fnc = NULL;
			mib_arg.i_tbl_id = tbl_rec->tbl_id;
			mib_arg.i_usr_key = pwe_cb->hm_hdl;
			mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
			mib_arg.req.info.set_req.i_param_val = pv;

			m_PSS_LOG_NCSMIB_ARG(&mib_arg);

			retval = pss_sync_mib_req(&mib_arg, pwe_cb->p_pss_cb->mib_req_func, 1000, &ma);
#if (NCS_PSS_RED == 1)
			/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
			{
				/* Send Async updates to Standby */
				pwe_cb->curr_plbck_ssn_info.tbl_id = tbl_rec->tbl_id;
				pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = 0;
				pwe_cb->curr_plbck_ssn_info.mib_obj_id = pv.i_param_id;
				m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
			}
#endif
			if (retval != NCSCC_RC_SUCCESS) {
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
			} else if (mib_arg.i_op != NCSMIB_OP_RSP_SET) {
				retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
				m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SET);
			} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
				retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
				m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS,
						 mib_arg.rsp.i_status);
			} else {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
			}
			--lcl_params_max;
		}
#if (NCS_PSS_RED == 1)
		else {
			/* These correspond to MIB objects which were already sent. So, 
			   we need count them as well. */
			--lcl_params_max;
		}
#endif
	}			/* for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) */

 cleanup:
	if (alt_data != NULL)
		m_MMGR_FREE_PSS_OCT(alt_data);

	if (alt_file_hdl != 0)
		m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, alt_file_hdl);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_broadcast_playback_signal

  DESCRIPTION:       This function broadcasts start and end of playback
                     signals to all the MACs and OACs across all the PWEs
                     that are present in the system.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_broadcast_playback_signal(PSS_CSI_NODE *csi_list, NCS_BOOL is_start)
{
	uns32 retval = NCSCC_RC_SUCCESS;
	PSS_CSI_NODE *t_csi = NULL;

	for (t_csi = csi_list; t_csi; t_csi = t_csi->next) {
		if (is_start == TRUE)
			retval = pss_signal_start_of_playback(t_csi->pwe_cb, NULL);
		else
			retval = pss_signal_end_of_playback(t_csi->pwe_cb);

		if (retval != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_add_to_refresh_tbl_list

  DESCRIPTION:       This function adds the PSS_TBL_REC to the refresh-list.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_add_to_refresh_tbl_list(PSS_PWE_CB *pwe_cb, PSS_TBL_REC *tbl_rec)
{
	PSS_RFRSH_TBL_LIST *tmp = pwe_cb->refresh_tbl_list, *prv_tmp = NULL;

	if (pwe_cb->refresh_tbl_list != NULL) {
		while (tmp != NULL) {
			if (tmp->pss_tbl_rec == tbl_rec)
				return NCSCC_RC_SUCCESS;
			prv_tmp = tmp;
			tmp = tmp->next;
		}
		/* No matching node found. So, adding node at the end of the list */
		if ((prv_tmp->next = m_MMGR_ALLOC_PSS_RFRSH_TBL_LIST) == NULL) {
			/* Alloc failure. */
			m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_RFRSH_TBL_LIST_ALLOC_FAIL,
					  "pss_add_to_refresh_tbl_list");
			return NCSCC_RC_FAILURE;
		}
		memset(prv_tmp->next, '\0', sizeof(PSS_RFRSH_TBL_LIST));
		prv_tmp->next->pss_tbl_rec = tbl_rec;
		return NCSCC_RC_SUCCESS;
	}

	if ((tmp = m_MMGR_ALLOC_PSS_RFRSH_TBL_LIST) == NULL) {
		/* Alloc failure. */
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_RFRSH_TBL_LIST_ALLOC_FAIL, "pss_add_to_refresh_tbl_list");
		return NCSCC_RC_FAILURE;
	}
	memset(tmp, '\0', sizeof(PSS_RFRSH_TBL_LIST));
	tmp->pss_tbl_rec = tbl_rec;

	pwe_cb->refresh_tbl_list = tmp;	/* Add the first node to the list */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_free_refresh_tbl_list

  DESCRIPTION:       This function frees the refresh-list.

  RETURNS:           void

*****************************************************************************/
void pss_free_refresh_tbl_list(PSS_PWE_CB *pwe_cb)
{
	PSS_RFRSH_TBL_LIST *tmp = pwe_cb->refresh_tbl_list, *nxt_tmp = NULL;

	while (tmp != NULL) {
		nxt_tmp = tmp->next;
		m_MMGR_FREE_PSS_RFRSH_TBL_LIST(tmp);
		tmp = nxt_tmp;
	}

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_del_rec_frm_refresh_tbl_list

  DESCRIPTION:       This function deletes the entry of the "rec" from the 
                     refresh-list.

  RETURNS:           void

*****************************************************************************/
void pss_del_rec_frm_refresh_tbl_list(PSS_PWE_CB *pwe_cb, PSS_TBL_REC *rec)
{
	PSS_RFRSH_TBL_LIST *tmp = pwe_cb->refresh_tbl_list, *prv_tmp = NULL;

	for (; tmp != NULL; prv_tmp = tmp, tmp = tmp->next) {
		if (tmp->pss_tbl_rec == rec) {
			if (prv_tmp == NULL)
				pwe_cb->refresh_tbl_list = tmp->next;
			else
				prv_tmp->next = tmp->next;

			m_MMGR_FREE_PSS_RFRSH_TBL_LIST(tmp);
			break;
		}
	}

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_free_mab_msg 

  DESCRIPTION:       This function frees the MAB_MSG message envelope.

  RETURNS:           void

*****************************************************************************/
void pss_free_mab_msg(MAB_MSG *mm, NCS_BOOL free_msg)
{
	MAB_PSS_TBL_LIST *tbl = NULL, *nxt_tbl = NULL;

	if (mm == NULL)
		return;

	switch (mm->op) {
	case MAB_PSS_TBL_BIND:
		{
			MAB_PSS_TBL_BIND_EVT *bind = &mm->data.data.oac_pss_tbl_bind, *nxt_bind = NULL;

			m_MMGR_FREE_MAB_PCN_STRING(bind->pcn_list.pcn);
			tbl = bind->pcn_list.tbl_list;
			while (tbl != NULL) {
				nxt_tbl = tbl->next;
				m_MMGR_FREE_MAB_PSS_TBL_LIST(tbl);
				tbl = nxt_tbl;
			}

			bind = bind->next;	/* First element is not allocated */
			while (bind != NULL) {
				nxt_bind = bind->next;

				m_MMGR_FREE_MAB_PCN_STRING(bind->pcn_list.pcn);
				tbl = bind->pcn_list.tbl_list;
				while (tbl != NULL) {
					nxt_tbl = tbl->next;
					m_MMGR_FREE_MAB_PSS_TBL_LIST(tbl);
					tbl = nxt_tbl;
				}

				m_MMGR_FREE_MAB_PSS_TBL_BIND_EVT(bind);
				bind = nxt_bind;
			}
		}
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_TBL_UNBIND:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_SET_REQUEST:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_WARM_BOOT:
		pss_free_wbreq_list(mm);
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_MIB_REQUEST:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_SVC_MDS_EVT:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_BAM_CONF_DONE:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_AMF_INIT_RETRY:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_VDEST_ROLE_QUIESCED:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	case MAB_PSS_RE_RESUME_ACTIVE_ROLE_FUNCTIONALITY:
		if (free_msg)
			m_MMGR_FREE_MAB_MSG(mm);
		break;

	default:
		break;
	}

	return;
}

/*****************************************************************************
 * Function:    pss_validate_set_mib_obj
 * Description: PSS function for validation of the param being set 
 * Input:       tbl_obj - is the pointer to the PSS_MIB_TBL_INFO struct for
 *                 the appropriate MIB table
 *              param  - pointer to the PARAM_VAL structure
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:
 *****************************************************************************/
uns32 pss_validate_set_mib_obj(PSS_MIB_TBL_INFO *tbl_obj, NCSMIB_PARAM_VAL *param)
{
	uns32 index = 0;
	NCSMIB_VAR_INFO *var_info = NULL;
	PSS_NCSMIB_TBL_INFO *tbl_info = NULL;

	tbl_info = tbl_obj->ptbl_info;

	index = tbl_info->objects_local_to_tbl[param->i_param_id];
	var_info = &tbl_obj->pfields[index].var_info;

	/* Check the object id's range within that table */
	if (param->i_param_id > tbl_info->num_objects)
		return NCSCC_RC_NO_OBJECT;

	if (var_info->status == NCSMIB_OBJ_OBSOLETE)
		return NCSCC_RC_NO_OBJECT;

	/* Check whether the object is settable or not */
	if ((var_info->access == NCSMIB_ACCESS_NOT_ACCESSIBLE) ||
	    (var_info->access == NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY))
		return NCSCC_RC_NO_ACCESS;

	/* Now validate the param value */
	if (pss_validate_param_val(var_info, param) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN2_SET_PARAM_VAL_FAIL_VALIDATION_TEST, param->i_param_id);
		return NCSCC_RC_INV_VAL;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function:    pss_validate_param_val
 * Description: PSS function for validation of the param value
 * Input:       var_info - is the pointer to the NCSMIB_VAR_INFO struct for
 *                 the param
 *              param  - pointer to the PARAM_VAL structure
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:
 *****************************************************************************/
uns32 pss_validate_param_val(NCSMIB_VAR_INFO *var_info, NCSMIB_PARAM_VAL *param)
{
	uns32 ret_code = NCSCC_RC_SUCCESS;
	uns32 i;

	switch (var_info->obj_type) {
	case NCSMIB_INT_RANGE_OBJ_TYPE:
		if ((param->info.i_int < var_info->obj_spec.range_spec.min) ||
		    (param->info.i_int > var_info->obj_spec.range_spec.max)) {
			ret_code = NCSCC_RC_FAILURE;
		}
		break;

	case NCSMIB_INT_DISCRETE_OBJ_TYPE:
		for (i = 0; i < var_info->obj_spec.value_spec.num_values; i++) {
			if ((param->info.i_int >= var_info->obj_spec.value_spec.values[i].min) &&
			    (param->info.i_int <= var_info->obj_spec.value_spec.values[i].max)) {
				ret_code = NCSCC_RC_SUCCESS;
				break;
			}
		}

		/* We're here because the param val was not found in the array of
		 * valid param values
		 */
		if (i == var_info->obj_spec.value_spec.num_values) {
			ret_code = NCSCC_RC_FAILURE;
		}
		break;

	case NCSMIB_TRUTHVAL_OBJ_TYPE:
		if ((param->info.i_int < NCS_SNMP_TRUE) || (param->info.i_int > NCS_SNMP_FALSE))
			ret_code = NCSCC_RC_FAILURE;
		break;

	case NCSMIB_OCT_OBJ_TYPE:
		if ((param->i_length < var_info->obj_spec.stream_spec.min_len) ||
		    (param->i_length > var_info->obj_spec.stream_spec.max_len)) {
			ret_code = NCSCC_RC_FAILURE;
		}
		break;

	case NCSMIB_OCT_DISCRETE_OBJ_TYPE:
		for (i = 0; i < var_info->obj_spec.disc_stream_spec.num_values; i++) {
			if ((param->i_length >= var_info->obj_spec.disc_stream_spec.values[i].min_len) &&
			    (param->i_length <= var_info->obj_spec.disc_stream_spec.values[i].max_len)) {
				ret_code = NCSCC_RC_SUCCESS;
				break;
			}
		}

		/* We're here because the param val was not found in the array of
		 * valid param values
		 */
		if (i == var_info->obj_spec.disc_stream_spec.num_values) {
			ret_code = NCSCC_RC_FAILURE;
		}
		break;

	case NCSMIB_OTHER_INT_OBJ_TYPE:
	case NCSMIB_OTHER_OCT_OBJ_TYPE:
		break;

	default:
		ret_code = NCSCC_RC_FAILURE;
		break;
	}
	return ret_code;
}

/*****************************************************************************
 * Function:    pss_validate_index
 * Description: PSS internal function for validation of the index
 * Input:       tbl_obj - Pointer to PSS_MIB_TBL_INFO structure for the
 *                 appropriate MIB table
 *              idx  - pointer to a struct of the type NCSMIB_IDX. Contains
 *                 the number of octets in the index and a pointer to the
 *                 actual index
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:       .
 *****************************************************************************/
uns32 pss_validate_index(PSS_MIB_TBL_INFO *tbl_obj, NCSMIB_IDX *idx)
{
	uns16 i, index;
	uns32 actual_inst_len = 0;
	uns32 stream_len;
	uns16 octet_len;
	uns32 remaining_len = idx->i_inst_len;
	const uns32 *inst_ids = idx->i_inst_ids;
	PSS_VAR_INFO *var_info;
	NCSMIB_PARAM_VAL param;
	uns16 param_id;

	for (i = 0; i < (tbl_obj->ptbl_info->idx_data[0]); i++) {
		/* Get SNMP param_id of the index elenent */
		index = tbl_obj->ptbl_info->idx_data[i + 1];	/* Get local index first */
		var_info = &(tbl_obj->pfields[index]);	/* Lookup var_info */
		param_id = var_info->var_info.param_id;

		switch (var_info->var_info.obj_type) {
		case NCSMIB_INT_RANGE_OBJ_TYPE:
		case NCSMIB_INT_DISCRETE_OBJ_TYPE:
		case NCSMIB_OTHER_INT_OBJ_TYPE:
			if (remaining_len >= 1) {

				/* check whether the integer value is in the right range */
				param.i_fmat_id = NCSMIB_FMAT_INT;
				param.i_length = 1;
				param.i_param_id = param_id;
				param.info.i_int = *inst_ids;
				if (pss_validate_param_val(&var_info->var_info, &param) != NCSCC_RC_SUCCESS) {
					/* Param val not in the valid range */
					m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN2_SET_PARAM_VAL_FAIL_VALIDATION_TEST,
							  param.i_param_id);
					return NCSCC_RC_FAILURE;
				}
				actual_inst_len++;	/* Increment length by size of an INTEGER */
				remaining_len--;
				inst_ids++;	/* Move pointer by 4 bytes */
			} else {
				/* The key seems to be shorter than expected */
				return NCSCC_RC_FAILURE;
			}
			break;

		case NCSMIB_OCT_OBJ_TYPE:
		case NCSMIB_OCT_DISCRETE_OBJ_TYPE:
		case NCSMIB_OTHER_OCT_OBJ_TYPE:
			if (remaining_len >= 1) {
				/* check whether variable length octet or constant length octet */
				if (var_info->var_info.obj_spec.stream_spec.min_len ==
				    var_info->var_info.obj_spec.stream_spec.max_len) {
					/* constant length octet string */
					stream_len = var_info->var_info.obj_spec.stream_spec.max_len;
					octet_len = (uns16)stream_len;
				} else {
					/* variable length octet string */
					octet_len = (uns16)(*inst_ids);
					/* Number of bytes required for the OCTET type index */
					stream_len = (octet_len);
					actual_inst_len++;	/* Increment length by size of an INTEGER */
					remaining_len--;
					inst_ids++;	/* Move pointer by 4 bytes */
				}
			} else {
				/* The key seems to be shorter than expected */
				return NCSCC_RC_FAILURE;
			}

			if (remaining_len >= stream_len) {
				/* validate the OCTET type param */
				param.i_fmat_id = NCSMIB_FMAT_OCT;
				param.i_length = octet_len;
				param.i_param_id = param_id;
				/* For OCT types, only the length is validated. It's ok if
				 * param.info field is not set
				 */
				if (pss_validate_param_val(&var_info->var_info, &param) != NCSCC_RC_SUCCESS) {
					/* Param val not in the valid range */
					m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN2_SET_PARAM_VAL_FAIL_VALIDATION_TEST,
							  param.i_param_id);
					return NCSCC_RC_FAILURE;
				}
				actual_inst_len += stream_len;
				remaining_len -= stream_len;
				inst_ids += stream_len;
			} else {
				/* The key seems to be shorter than expected */
				return NCSCC_RC_FAILURE;
			}
			break;

		default:
			return NCSCC_RC_FAILURE;
			break;
		}
	}

	if (actual_inst_len == idx->i_inst_len) {
		return NCSCC_RC_SUCCESS;
	} else {
		/* Something wrong with the index */
		return NCSCC_RC_FAILURE;
	}
}

/*****************************************************************************
 * Function:    pss_stdby_oaa_down_list_update
 * Description: PSS internal function for updating standby oaa down buffer
 * Input:       oaa_addr - oac MDS_DEST ADDRESS 
                pwe_cb - 
 *              buffer_op  - indicates whether to add node to the buffer or delete 
 * Returns:     NCSCC_RC_SUCCESSS/FAILURE
 * Notes:       .
 *****************************************************************************/
uns32 pss_stdby_oaa_down_list_update(MDS_DEST oaa_addr, NCSCONTEXT yr_hdl, PSS_STDBY_OAA_DOWN_BUFFER_OP buffer_op)
{
	PSS_OAA_ENTRY *oaa_node = NULL;
	PSS_PWE_CB *pwe_cb = NULL;
	PSS_STDBY_OAA_DOWN_BUFFER_NODE *new_node, *curr_node, *prev_node;

	/* Validate the Handle */
	pwe_cb = (PSS_PWE_CB *)m_PSS_VALIDATE_HDL((long)yr_hdl);

	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return NCSCC_RC_FAILURE;
	}

	if (PSS_STDBY_OAA_DOWN_BUFFER_ADD == buffer_op) {
		oaa_node = pss_findadd_entry_in_oaa_tree(pwe_cb, &oaa_addr, FALSE);
		if ((oaa_node == NULL) || (oaa_node->tbl_cnt == 0)) {
			/* No valid tables from this OAA. */
			return NCSCC_RC_FAILURE;
		}
		new_node = m_MMGR_ALLOC_STDBY_PSS_BUFFER_NODE;
		memset(new_node, '\0', sizeof(PSS_STDBY_OAA_DOWN_BUFFER_NODE));
		new_node->oaa_addr = oaa_addr;
		new_node->oaa_node = oaa_node;
		new_node->next = NULL;
		if (NULL == pwe_cb->pss_stdby_oaa_down_buffer) {
			/* list is empty, so this is the first node to be added */
			pwe_cb->pss_stdby_oaa_down_buffer = new_node;
		} else {
			curr_node = pwe_cb->pss_stdby_oaa_down_buffer;
			while (curr_node->next != NULL)
				curr_node = curr_node->next;
			curr_node->next = new_node;
		}
	} else if (PSS_STDBY_OAA_DOWN_BUFFER_DELETE == buffer_op) {
		curr_node = prev_node = pwe_cb->pss_stdby_oaa_down_buffer;
		while (curr_node) {
			if (curr_node->oaa_addr == oaa_addr)
				break;

			prev_node = curr_node;
			curr_node = curr_node->next;
		}
		if (curr_node == NULL) {	/* if list NULL or no match found */
			return NCSCC_RC_FAILURE;
		}

		if (prev_node == curr_node) {	/* if match found in first node */
			pwe_cb->pss_stdby_oaa_down_buffer = pwe_cb->pss_stdby_oaa_down_buffer->next;
		} else {	/* if match found in middle or end */

			prev_node->next = curr_node->next;
		}

		m_MMGR_FREE_STDBY_PSS_BUFFER_NODE(curr_node);
	}

	/* Release the Handle  */
	ncshm_give_hdl((long)yr_hdl);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function:    pss_stdby_oaa_down_list_dump
 * Description: PSS internal function for dumping standby oaa down buffer
 * Input:       pss_stdby_oaa_down_buffer - pointer to standby pss oaa down list
                fh - pointer to file into which it dumps the data
 * Returns:     
 * Notes:       .
 *****************************************************************************/
void pss_stdby_oaa_down_list_dump(PSS_STDBY_OAA_DOWN_BUFFER_NODE *pss_stdby_oaa_down_buffer, FILE *fh)
{
	PSS_STDBY_OAA_DOWN_BUFFER_NODE *oaa_down_node = pss_stdby_oaa_down_buffer;
	if (NULL == pss_stdby_oaa_down_buffer) {
		fprintf(fh, "\t\t - pss_stdby_oaa_down_buffer is EMPTY\n");
		return;
	}
	while (oaa_down_node) {
		fprintf(fh, "\t\t - oaa mds dest: %llx\n", oaa_down_node->oaa_addr);
		oaa_down_node = oaa_down_node->next;
	}
	return;
}
#endif   /* (NCS_PSR == 1) */

#endif   /* (NCS_MAB == 1) */
