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

  DESCRIPTION:

  This file contains all Public APIs for the Object Access Client (OAC) portion
  of the MIB Acess Broker (MAB) subsystem.

*******************************************************************************/

#include "mab.h"

#if (NCS_MAB == 1)

static void oac_destroy_pcn_list(OAC_TBL *inst);
static void oac_destroy_wb_pend_list(OAC_TBL *inst);
static uns32 oac_add_node_to_wbreq_hdl_list(OAC_TBL *inst, uns32 wbreq_hdl, NCSOAC_EOP_USR_IND_FNC func);
static void oac_del_node_from_wbreq_hdl_list(OAC_TBL *inst, uns32 wbreq_hdl);
static void oac_destroy_wbreq_hdl_list(OAC_TBL *inst);

/*****************************************************************************

                     OAC LAYER MANAGEMENT IMPLEMENTAION

  PROCEDURE NAME:    ncsoac_lm

  DESCRIPTION:       Core API for all Object Access Client layer management 
                     operations used by a target system to instantiate and
                     control an OAC instance. Its operations include:

                     CREATE  an OAC instance
                     DESTROY an OAC instance
                     SET     an OAC configuration object
                     GET     an OAC configuration object

  RETURNS:           SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
                               enable m_MAB_DBG_SINK() for details.

*****************************************************************************/
uns32 ncsoac_lm(NCSOAC_LM_ARG *arg)
{
	switch (arg->i_op) {
	case NCSOAC_LM_OP_CREATE:
		return oac_svc_create(&arg->info.create);

	case NCSOAC_LM_OP_DESTROY:
		return oac_svc_destroy(&arg->info.destroy);

	case NCSOAC_LM_OP_SET:
		return oac_svc_set(&arg->info.set);

	case NCSOAC_LM_OP_GET:
		return oac_svc_get(&arg->info.get);

	default:
		break;
	}
	return NCSCC_RC_FAILURE;
}

/*****************************************************************************

                     OAC SUBSYSTEM INTERFACE IMPLEMENTATION

  PROCEDURE NAME:    ncsoac_ss

  DESCRIPTION:       Core API used by Subcomponents that wish to participate
                     in MAB services. Such subcomponents must make ownership
                     claims reguarding MIB Tables. These claims are

                     TBL_OWNED claim MIB Table ID ownership
                     ROW_OWNED claim ownership of specific MIB Table Rows
                     ROW_GONE  cancel ownership of specific MIB Table Rows
                     TBL_GONE  cancel ownership of a specific MIB Table

  RETURNS:           SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
                               enable m_MAB_DBG_SINK() for details.

*****************************************************************************/

uns32 ncsoac_ss(NCSOAC_SS_ARG *arg)
{
	if (arg == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	switch (arg->i_op) {
	case NCSOAC_SS_OP_TBL_OWNED:
		return oac_ss_tbl_reg(&arg->info.tbl_owned, arg->i_tbl_id, arg->i_oac_hdl);

	case NCSOAC_SS_OP_ROW_OWNED:
		return oac_ss_row_reg(&arg->info.row_owned, arg->i_tbl_id, arg->i_oac_hdl);

	case NCSOAC_SS_OP_ROW_GONE:
		return oac_ss_row_unreg(&arg->info.row_gone, arg->i_tbl_id, arg->i_oac_hdl);

	case NCSOAC_SS_OP_TBL_GONE:
		return oac_ss_tbl_unreg(&arg->info.tbl_gone, arg->i_tbl_id, arg->i_oac_hdl);

	case NCSOAC_SS_OP_ROW_MOVE:
		return oac_ss_row_move(&arg->info.row_move, arg->i_oac_hdl);

	case NCSOAC_SS_OP_WARMBOOT_REQ_TO_PSSV:
		return oac_ss_warmboot_req_to_pssv(arg);

	case NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV:
		return oac_ss_push_mibarg_data_to_pssv(arg);

	default:
		break;
	}

	return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
}

/*****************************************************************************
 *****************************************************************************

     P R I V A T E  Support Functions for OAC APIs are below 
   
                     (should be in oac_pvt.c ?)

*****************************************************************************
*****************************************************************************/

/*#############################################################################
 *
 *                 PRIVATE OAC SUBSYSTEM INTERFACE IMPLEMENTATION
 *
 *############################################################################*/

/*****************************************************************************
   oac_mib_local_response
*****************************************************************************/

static uns32 oac_mib_local_response(NCSMIB_ARG *rsp)
{
	OAC_TBL *inst;
	uns32 hm_hdl;

	m_OAC_LK_INIT;

	if (rsp == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	m_MAB_DBG_TRACE("\noac_mib_local_response():entered.");

	hm_hdl = (uns32)rsp->i_usr_key;
	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(hm_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED, &inst->lock);

	if (!m_NCSMIB_ISIT_A_RSP(rsp->i_op)) {
		ncshm_give_hdl(hm_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* put good values for usr_key, xchg_id and rsp_fnc in the response */

	/* recover the information about the original MIB request sender */
	{
		NCS_SE *se;
		uns8 *stream;

		if ((se = ncsstack_pop(&rsp->stack)) == NULL) {
			ncshm_give_hdl(hm_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		stream = m_NCSSTACK_SPACE(se);

		if (se->type != NCS_SE_TYPE_MIB_ORIG) {
			ncshm_give_hdl(hm_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		rsp->i_usr_key = ncs_decode_64bit(&stream);
		if (4 == sizeof(void *))
			rsp->i_rsp_fnc = (NCSMIB_FNC)(long)ncs_decode_32bit(&stream);
		else if (8 == sizeof(void *))
			rsp->i_rsp_fnc = (NCSMIB_FNC)(long)ncs_decode_64bit(&stream);

	}

	ncshm_give_hdl(hm_hdl);
	m_OAC_UNLK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);

	if ((rsp->i_tbl_id >= NCSMIB_TBL_PSR_START) && (rsp->i_tbl_id <= NCSMIB_TBL_PSR_END))
		goto end;
	/* Forward the response to PSS if the request succeeded */
	if ((rsp->rsp.i_status == NCSCC_RC_SUCCESS) && (inst->playback_session == FALSE)) {
		MAB_MSG msg;
		uns32 code;

		/* Forward this message to the PSS */
		memset(&msg, 0, sizeof(msg));
		msg.op = MAB_PSS_SET_REQUEST;
		msg.data.data.snmp = rsp;
		msg.fr_anc = inst->my_anc;

		msg.data.seq_num = inst->seq_num;
		++inst->seq_num;
		code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS, inst->psr_vcard);
		if (code != NCSCC_RC_SUCCESS) {
			m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_SND_MSG_FAILED, NCSMDS_SVC_ID_OAC, code);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		/* Add this message to the buffer zone. */
		/* Alloc msg_ptr, and duplicate arg->info.push_mibarg_data.arg here. */
		oac_psr_add_to_buffer_zone(inst, &msg);

		if ((inst->psr_here == TRUE) && (inst->is_active == TRUE)) {
			code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS, inst->psr_vcard);
			if (code != NCSCC_RC_SUCCESS) {
				m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_OAC_SENDTO_PSR_FAILED, msg.data.seq_num);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_SENDTO_PSR_SUCCESS, msg.data.seq_num);
		}
	}
 end:

	rsp->i_rsp_fnc(rsp);

	rsp->i_usr_key = 0;

	m_MAB_DBG_TRACE("\noac_mib_local_response():left.");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_ss_row_move

  DESCRIPTION:       Moves MIB Row Ownership

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/

uns32 oac_ss_row_move(NCSOAC_ROW_MOVE *row_move, uns32 oac_hdl)
{
	OAC_TBL *inst;
	NCS_SE *se;
	uns8 *stream;
	MAB_MSG msg;
	uns32 code;

	m_OAC_LK_INIT;

	m_MAB_DBG_TRACE("\noac_ss_row_move():entered.");

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(oac_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED, &inst->lock);

	/* Save information about the original MIB request sender */
	if ((se = ncsstack_push(&row_move->i_move_req->stack,
				NCS_SE_TYPE_MIB_ORIG, (uns8)sizeof(NCS_SE_MIB_ORIG))) == NULL) {
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	stream = m_NCSSTACK_SPACE(se);

	ncs_encode_64bit(&stream, row_move->i_move_req->i_usr_key);
	if (4 == sizeof(void *))
		ncs_encode_32bit(&stream, (uns32)(long)row_move->i_move_req->i_rsp_fnc);
	else if (8 == sizeof(void *))
		ncs_encode_64bit(&stream, (long)row_move->i_move_req->i_rsp_fnc);

	if (memcmp(&inst->my_vcard, &row_move->i_move_req->req.info.moverow_req.i_move_to, sizeof(inst->my_vcard)) == 0) {
		/* this is a local moverow request */
		NCSMIB_ARG *mib_req = row_move->i_move_req;
		OAC_TBL_REC *tbl_rec;

		if ((tbl_rec = oac_tbl_rec_find(inst, mib_req->i_tbl_id)) == NULL) {
			ncshm_give_hdl(oac_hdl);
			m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
					      MAB_OAC_FLTR_REG_NO_TBL, inst->vrid, mib_req->i_tbl_id);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			m_MMGR_FREE_BUFR_LIST(row_move->i_move_req->req.info.moverow_req.i_usrbuf);
			row_move->i_move_req->req.info.moverow_req.i_usrbuf = NULL;
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		mib_req->i_mib_key = tbl_rec->handle;
		mib_req->i_usr_key = oac_hdl;
		mib_req->i_rsp_fnc = oac_mib_local_response;

		ncshm_give_hdl(oac_hdl);

		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);

		if ((tbl_rec->req_fnc) (mib_req) != NCSCC_RC_SUCCESS) {
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_MIB_REQ_FAILED);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_MIB_REQ_SUCCESS);

		mib_req->i_mib_key = 0;
		mib_req->i_usr_key = 0;

	} else {		/* remote moverow request */

		/* push BACKTO stack element */
		if ((se = ncsstack_push(&row_move->i_move_req->stack,
					NCS_SE_TYPE_BACKTO, (uns8)sizeof(NCS_SE_BACKTO))) == NULL) {
			ncshm_give_hdl(oac_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}

		stream = m_NCSSTACK_SPACE(se);

		mds_st_encode_mds_dest(&stream, &inst->my_vcard);
		ncs_encode_16bit(&stream, NCSMDS_SVC_ID_OAC);
		ncs_encode_16bit(&stream, inst->vrid);

		msg.vrid = inst->vrid;
		memset(&msg.fr_card, 0, sizeof(msg.fr_card));
		msg.fr_svc = 0;
		msg.op = MAB_MAS_REQ_HDLR;
		msg.data.data.snmp = row_move->i_move_req;

		if (inst->mas_here == FALSE) {
			ncshm_give_hdl(oac_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return NCSCC_RC_FAILURE;
		}

		code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_MAS, inst->mas_vcard);
		m_MMGR_FREE_BUFR_LIST(msg.data.data.snmp->req.info.moverow_req.i_usrbuf);
		msg.data.data.snmp->req.info.moverow_req.i_usrbuf = NULL;

		if (code != NCSCC_RC_SUCCESS) {
			ncshm_give_hdl(oac_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_SND_MSG_FAILED, NCSMDS_SVC_ID_OAC, code);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
	}

	m_MAB_DBG_TRACE("\noac_ss_row_move():left.");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_ss_push_mibarg_data_to_pssv

  DESCRIPTION:       Sends Dynamic data to PSSv

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/

uns32 oac_ss_push_mibarg_data_to_pssv(NCSOAC_SS_ARG *arg)
{
	OAC_TBL *inst;
	uns32 oac_hdl;
	OAC_TBL_REC *tbl_rec = NULL;

	m_MAB_DBG_TRACE("\noac_ss_push_mibarg_data_to_pssv():entered.");

	if (arg->info.push_mibarg_data.arg == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	oac_hdl = arg->i_oac_hdl;
	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(oac_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK_INIT;
	m_OAC_LK(&inst->lock);

	if (((tbl_rec = oac_tbl_rec_find(inst, arg->i_tbl_id)) == NULL) ||
	    (tbl_rec->is_persistent == FALSE) || (arg->info.push_mibarg_data.arg->i_tbl_id != arg->i_tbl_id)) {
		m_OAC_UNLK(&inst->lock);
		ncshm_give_hdl(oac_hdl);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if ((inst->psr_here == TRUE) && (inst->playback_session == FALSE) && (inst->is_active == TRUE)) {
		MAB_MSG msg;
		uns32 code;

		/* Forward this message to the PSS */
		memset(&msg, 0, sizeof(msg));
		msg.op = MAB_PSS_SET_REQUEST;
		msg.data.data.snmp = arg->info.push_mibarg_data.arg;
		msg.fr_anc = inst->my_anc;

		msg.data.seq_num = inst->seq_num;
		++inst->seq_num;
		code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS, inst->psr_vcard);
		if (code != NCSCC_RC_SUCCESS) {
			m_OAC_UNLK(&inst->lock);
			ncshm_give_hdl(oac_hdl);
			m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_SND_MSG_FAILED, NCSMDS_SVC_ID_OAC, code);
			m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_OAC_SENDTO_PSR_FAILED, msg.data.seq_num);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		/* Add this message to the buffer zone. */
		/* Alloc msg_ptr, and duplicate arg->info.push_mibarg_data.arg here. */
		oac_psr_add_to_buffer_zone(inst, &msg);

		m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_SENDTO_PSR_SUCCESS, msg.data.seq_num);

	} else {
		m_OAC_UNLK(&inst->lock);
		ncshm_give_hdl(oac_hdl);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_UNLK(&inst->lock);
	ncshm_give_hdl(oac_hdl);

	m_MAB_DBG_TRACE("\noac_ss_push_mibarg_data_to_pssv():left.");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_ss_warmboot_req_to_pssv

  DESCRIPTION:       Sends warmboot request to PSSv

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/

uns32 oac_ss_warmboot_req_to_pssv(NCSOAC_SS_ARG *arg)
{
	OAC_TBL *inst;
	MAB_MSG msg;
	uns32 oac_hdl, ret_val, code = NCSCC_RC_FAILURE;
	OAA_WBREQ_PEND_LIST *p_wbreq_node = NULL;
	NCS_BOOL send_req = FALSE;

	m_MAB_DBG_TRACE("\noac_ss_warmboot_req_to_pssv():entered.");

	if (arg->info.warmboot_req.i_pcn == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	oac_hdl = arg->i_oac_hdl;
	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(oac_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK_INIT;
	m_OAC_LK(&inst->lock);

	/* Lookup the PCN in the inst->pcn_list */
	if (oac_validate_pcn(inst, arg->info.warmboot_req.i_pcn) == FALSE) {
		/* Invalid PCN */
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_LOG_OAA_WARMBOOTREQ_INFO_I(NCSFL_SEV_NOTICE, MAB_OAA_WARMREQ_INFO_I_PSS_MAS_STATUS, inst->psr_here,
				     inst->mas_here);
	{
		NCSOAC_PSS_TBL_LIST *tmp = arg->info.warmboot_req.i_tbl_list;
		if (tmp != NULL) {
			while (tmp != NULL) {
				m_LOG_OAA_WARMBOOTREQ_INFO_II(NCSFL_SEV_NOTICE, MAB_OAA_WARMREQ_INFO_II_RCVD_REQ,
							      arg->info.warmboot_req.i_pcn,
							      arg->info.warmboot_req.is_system_client, tmp->tbl_id);
				tmp = tmp->next;
			}
		} else {
			m_LOG_OAA_WARMBOOTREQ_INFO_II(NCSFL_SEV_NOTICE, MAB_OAA_WARMREQ_INFO_II_RCVD_REQ,
						      arg->info.warmboot_req.i_pcn,
						      arg->info.warmboot_req.is_system_client, 0);
		}
	}

	if (arg->info.warmboot_req.i_tbl_list == NULL) {
		/* This is a playback request for the entire PCN */
		if ((inst->psr_here == FALSE) || (inst->mas_here == FALSE) || (inst->is_active == FALSE)) {
			/* Since PSS is not UP, this request need be queued at the OAC.
			   This request is for all the MIBs of the particular PCN */
			code = oac_add_warmboot_req_in_wbreq_list(inst, &arg->info.warmboot_req, &p_wbreq_node);
			if (code != NCSCC_RC_SUCCESS) {
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_ADD_WBREQ_TO_PEND_LIST_FAIL);
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
						   MAB_HDLN_OAC_TERMINATING_WARMBOOT_REQ_API_PROCESSING);
				ncshm_give_hdl(oac_hdl);
				m_OAC_UNLK(&inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			if (p_wbreq_node == NULL) {
				/* Node allocation failed. Error to be handled. TBD. */
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
						   MAB_HDLN_OAC_TERMINATING_WARMBOOT_REQ_API_PROCESSING);
				ncshm_give_hdl(oac_hdl);
				m_OAC_UNLK(&inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			arg->info.warmboot_req.o_wbreq_hdl = p_wbreq_node->wbreq_hdl;
		} else {
			/* Send the request to PSSv */
			send_req = TRUE;
		}
	} else {
		if ((inst->psr_here == FALSE) || (inst->mas_here == FALSE) || (inst->is_active == FALSE)) {
			NCSOAC_PSS_TBL_LIST *l_ptr = arg->info.warmboot_req.i_tbl_list;
			OAA_PCN_LIST *p_pcn = NULL;

			/* Run loop for all MIB tables of this PCN */
			while (l_ptr != NULL) {
				OAC_TBL_REC *tbl_rec = oac_tbl_rec_find(inst, l_ptr->tbl_id);

				if (tbl_rec != NULL) {
					if (!tbl_rec->is_persistent) {
						m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_ERROR,
								     MAB_OAA_PCN_INFO_TBL_IN_WARMBOOT_REQ_NOT_PERSISTENT,
								     l_ptr->tbl_id);
						m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
								   MAB_HDLN_OAC_TERMINATING_WARMBOOT_REQ_API_PROCESSING);
						ncshm_give_hdl(oac_hdl);
						m_OAC_UNLK(&inst->lock);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}

					p_pcn = oac_findadd_pcn_in_list(inst, arg->info.warmboot_req.i_pcn,
									arg->info.warmboot_req.is_system_client,
									l_ptr->tbl_id, FALSE);
					if (p_pcn == NULL) {
						m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_ERROR,
								     MAB_OAA_PCN_INFO_WARMBOOT_REQ_TBL_NOT_FND_IN_PCN_LIST,
								     l_ptr->tbl_id);
						m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
								   MAB_HDLN_OAC_TERMINATING_WARMBOOT_REQ_API_PROCESSING);
						ncshm_give_hdl(oac_hdl);
						m_OAC_UNLK(&inst->lock);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					code = oac_add_warmboot_req_in_wbreq_list(inst,
										  &arg->info.warmboot_req,
										  &p_wbreq_node);
					if (code != NCSCC_RC_SUCCESS) {
						m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
								   MAB_HDLN_OAC_ADD_WBREQ_TO_PEND_LIST_FAIL);
						m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
								   MAB_HDLN_OAC_TERMINATING_WARMBOOT_REQ_API_PROCESSING);
						ncshm_give_hdl(oac_hdl);
						m_OAC_UNLK(&inst->lock);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					if (p_wbreq_node == NULL) {
						/* Node allocation failed. Error to be handled. */
						m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
								   MAB_HDLN_OAC_TERMINATING_WARMBOOT_REQ_API_PROCESSING);
						ncshm_give_hdl(oac_hdl);
						m_OAC_UNLK(&inst->lock);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					arg->info.warmboot_req.o_wbreq_hdl = p_wbreq_node->wbreq_hdl;
				}
				l_ptr = l_ptr->next;
			}
		} else {
			/* Send the request to PSSv */
			send_req = TRUE;
		}
	}
	if (send_req == TRUE) {
		MAB_PSS_WARMBOOT_REQ wbreq;

		memset(&msg, 0, sizeof(msg));
		msg.vrid = inst->vrid;
		msg.op = MAB_PSS_WARM_BOOT;
		msg.fr_anc = inst->my_anc;

		m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_INVOKING_SEND_WARMBOOT_REQ_TO_PSS);

		memset(&wbreq, '\0', sizeof(wbreq));
		ret_val = oac_convert_input_wbreq_to_mab_request(inst, &arg->info.warmboot_req, &wbreq);
		if (ret_val != NCSCC_RC_SUCCESS) {
			oac_del_node_from_wbreq_hdl_list(inst, arg->info.warmboot_req.o_wbreq_hdl);
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_CONVRT_WBREQ_TO_MAB_MSG_FAIL);
			m_OAC_UNLK(&inst->lock);
			ncshm_give_hdl(oac_hdl);
			return NCSCC_RC_SUCCESS;
		}

		/* Any invalid PCN/table-id in the request would be dropped here. Remaining
		   entries in the request would be processed. */
		msg.data.data.oac_pss_warmboot_req = wbreq;
		{
			MAB_PSS_TBL_LIST *tmp = wbreq.pcn_list.tbl_list;
			if (tmp != NULL) {
				while (tmp != NULL) {
					m_LOG_OAA_WARMBOOTREQ_INFO_III(NCSFL_SEV_NOTICE,
								       MAB_OAA_WARMREQ_INFO_III_REQ_MDS_EVT_TO_PSS_COMPOSED,
								       wbreq.pcn_list.pcn, wbreq.is_system_client,
								       tmp->tbl_id, wbreq.wbreq_hdl);
					tmp = tmp->next;
				}
			} else {
				m_LOG_OAA_WARMBOOTREQ_INFO_III(NCSFL_SEV_NOTICE,
							       MAB_OAA_WARMREQ_INFO_III_REQ_MDS_EVT_TO_PSS_COMPOSED,
							       wbreq.pcn_list.pcn, wbreq.is_system_client, 0, 0);
			}
		}

		/* Assign the wbreq_hdl here. */
		msg.data.data.oac_pss_warmboot_req.wbreq_hdl = ++inst->wbreq_hdl_index;

		/* Add a node for the new request to the list "inst->wbreq_hdl_list" */
		if (oac_add_node_to_wbreq_hdl_list(inst, msg.data.data.oac_pss_warmboot_req.wbreq_hdl,
						   arg->info.warmboot_req.i_eop_usr_ind_fnc) != NCSCC_RC_SUCCESS) {
			/* Alloc failed. */
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_WBREQ_HDL_LIST_ALLOC_FAIL,
					  "oac_ss_warmboot_req_to_pssv()");
			if (wbreq.pcn_list.tbl_list != NULL)
				oac_free_pss_tbl_list(wbreq.pcn_list.tbl_list);
			if (wbreq.pcn_list.pcn != NULL)
				m_MMGR_FREE_MAB_PCN_STRING(wbreq.pcn_list.pcn);
			m_OAC_UNLK(&inst->lock);
			ncshm_give_hdl(oac_hdl);
			return NCSCC_RC_SUCCESS;
		}

		msg.data.seq_num = inst->seq_num;
		++inst->seq_num;
		if (inst->is_active == TRUE) {
			ret_val = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS,
					      inst->psr_vcard);
			if (ret_val != NCSCC_RC_SUCCESS) {
				oac_del_node_from_wbreq_hdl_list(inst, arg->info.warmboot_req.o_wbreq_hdl);
				if (wbreq.pcn_list.tbl_list != NULL)
					oac_free_pss_tbl_list(wbreq.pcn_list.tbl_list);
				if (wbreq.pcn_list.pcn != NULL)
					m_MMGR_FREE_MAB_PCN_STRING(wbreq.pcn_list.pcn);
				m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_SND_MSG_FAILED,
						    NCSMDS_SVC_ID_OAC, code);
				m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_OAC_WARMBOOT_REQ_TO_PSS_MDS_SEND_FAIL,
						 msg.data.seq_num);
				m_OAC_UNLK(&inst->lock);
				ncshm_give_hdl(oac_hdl);
				return NCSCC_RC_SUCCESS;
			}
			m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_WARMBOOT_REQ_SEND_SUCCESS, msg.data.seq_num);
		}

		/* Add this message to the buffer zone. */
		/* Alloc msg_ptr here */
		oac_psr_add_to_buffer_zone(inst, &msg);
	}

	m_OAC_UNLK(&inst->lock);
	ncshm_give_hdl(oac_hdl);

	m_MAB_DBG_TRACE("\noac_ss_warmboot_req_to_pssv():left.");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_validate_pcn

  DESCRIPTION:       Validate the PCN with the list present with OAA.

  RETURNS:           TRUE, if valid PCN.
                     FALSE, otherwise.

*****************************************************************************/
NCS_BOOL oac_validate_pcn(OAC_TBL *inst, char *pcn)
{
	OAA_PCN_LIST *pcn_list = inst->pcn_list;
	uns32 str_len = 0;

	if ((pcn == NULL) || (pcn[0] == '\0') || ((str_len = strlen(pcn)) == 0)) {
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_INVLD_PCN_IN_WARMBOOT_REQ);
		return FALSE;
	}

	for (; pcn_list != NULL; pcn_list = pcn_list->next) {
		if ((str_len == strlen(pcn_list->pcn)) && (strcmp(pcn_list->pcn, pcn) == 0))
			return TRUE;
	}

	m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_PCN_OF_WARMBOOT_REQ_NOT_IN_PCN_LIST);
	return FALSE;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_findadd_pcn_in_list

  DESCRIPTION:       Find/add the PCN node in the "pcn_list" linked list.

  RETURNS:           Pointer to the node in the "pcn_list" linked list.

*****************************************************************************/
OAA_PCN_LIST *oac_findadd_pcn_in_list(OAC_TBL *inst, char *pcn, NCS_BOOL is_system_client, uns32 tbl_id, NCS_BOOL add)
{
	OAA_PCN_LIST *tmp = inst->pcn_list, *prv_tmp = NULL;
	uns32 lcl_len = 0;

	if ((pcn == NULL) || ((lcl_len = strlen(pcn)) == 0)) {
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAA_PCN_NULL_IN_OAC_FINDADD_PCN_IN_LIST_FUNC);
		return NULL;
	}

	while (tmp != NULL) {
		if ((lcl_len == strlen(tmp->pcn)) && (memcmp(pcn, tmp->pcn, lcl_len) == 0) && (tbl_id == tmp->tbl_id)) {
			m_LOG_OAA_PCN_INFO_II(NCSFL_SEV_INFO,
					      MAB_OAA_PCN_INFO_PCN_FND_IN_PCN_LIST_OAC_FINDADD_PCN_IN_LIST_FUNC, pcn,
					      tbl_id);
			return tmp;
		}
		prv_tmp = tmp;
		tmp = tmp->next;
	}

	if (!add)
		return NULL;

	if ((tmp = m_MMGR_ALLOC_MAB_OAA_PCN_LIST) == NULL) {
		m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAC_PCN_LIST_ALLOC_FAIL, "oac_findadd_pcn_in_list()");
		return NULL;
	}
	memset(tmp, '\0', sizeof(OAA_PCN_LIST));
	if ((tmp->pcn = m_MMGR_ALLOC_MAB_PCN_STRING(strlen(pcn) + 1)) == NULL) {
		m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PCN_STRING_ALLOC_FAIL, "oac_findadd_pcn_in_list()");
		m_MMGR_FREE_MAB_OAA_PCN_LIST(tmp);
		return NULL;
	}
	memset(tmp->pcn, '\0', (strlen(pcn) + 1));
	strcpy(tmp->pcn, pcn);
	tmp->tbl_id = tbl_id;
	if (prv_tmp != NULL)
		prv_tmp->next = tmp;
	else
		inst->pcn_list = tmp;

	return tmp;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_add_warmboot_req_in_wbreq_list

  DESCRIPTION:       Add the warmboot request to the pending list "wbreq_list".

  RETURNS:           SUCCESS or FAILURE. Also, returns the pointer to the 
                     OAC_WBREQ_PEND_LIST element in the linked list.

*****************************************************************************/
uns32 oac_add_warmboot_req_in_wbreq_list(OAC_TBL *inst, NCSOAC_PSS_WARMBOOT_REQ *req,
					 OAA_WBREQ_PEND_LIST **o_wbreq_node)
{
	NCS_BOOL lcl_pcn_fnd = FALSE;
	OAA_WBREQ_PEND_LIST *list = NULL, *prv_list = NULL, *tmp_list = NULL;
	NCSOAC_PSS_WARMBOOT_REQ *wbr = req;

	if ((wbr == NULL) || (wbr->i_pcn == NULL) || (wbr->i_pcn[0] == '\0')) {
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_INVLD_PCN_IN_WARMBOOT_REQ);
		return NCSCC_RC_FAILURE;
	}

	/* m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_ERROR, MAB_OAA_PCN_INFO_TBL_NOT_FND_IN_PCN_LIST_DURING_UNBIND_OP, tbl_id); */

	*o_wbreq_node = NULL;
	/* Look if this is already present in the wbreq_list */
	list = inst->wbreq_list;
	while (list != NULL) {
		uns32 lcl_len = strlen(wbr->i_pcn);

		if ((lcl_len == strlen(list->pcn)) && (memcmp(wbr->i_pcn, list->pcn, lcl_len) == 0)) {
			/* Found the PCN here. Now, look for the specific table. */
			NCSOAC_PSS_TBL_LIST *wbr_tbl_elem = wbr->i_tbl_list;
			NCSOAC_PSS_TBL_LIST *oac_tbl_elem = NULL, *prv_oac_tbl_elem = NULL, *tmp_elem = NULL;

			lcl_pcn_fnd = TRUE;
			/* For each table in the warmboot request */
			while (wbr_tbl_elem != NULL) {
				NCS_BOOL lcl_tbl_fnd = FALSE;

				oac_tbl_elem = list->tbl_list;
				while (oac_tbl_elem != NULL) {
					if (oac_tbl_elem->tbl_id == wbr_tbl_elem->tbl_id) {
						/* Found the entry. Continue to next table 
						   in the incoming request list. */
						lcl_tbl_fnd = TRUE;
						prv_oac_tbl_elem = oac_tbl_elem;
						oac_tbl_elem = NULL;
						break;
					}
					prv_oac_tbl_elem = oac_tbl_elem;
					oac_tbl_elem = oac_tbl_elem->next;
				}
				if (lcl_tbl_fnd == FALSE) {
					/* Insert the table now */
					if ((tmp_elem = m_MMGR_ALLOC_NCSOAC_PSS_TBL_LIST) == NULL) {
						m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL,
								  MAB_MF_OAC_NCSOAC_PSS_TBL_LIST_ALLOC_FAIL,
								  "oac_add_warmboot_req_in_wbreq_list()");
						return NCSCC_RC_FAILURE;
					}
					memset(tmp_elem, '\0', sizeof(NCSOAC_PSS_TBL_LIST));
					tmp_elem->tbl_id = wbr_tbl_elem->tbl_id;

					if (prv_oac_tbl_elem == NULL) {
						list->tbl_list = tmp_elem;
					} else {
						prv_oac_tbl_elem->next = tmp_elem;
					}
				}
				wbr_tbl_elem = wbr_tbl_elem->next;
			}
			*o_wbreq_node = list;
			return NCSCC_RC_SUCCESS;
		}
		prv_list = list;
		list = list->next;
	}
	if (lcl_pcn_fnd == FALSE) {
		uns32 lcl_strlen = strlen(wbr->i_pcn);

		/* Insert the PCN into the list. */
		if ((tmp_list = m_MMGR_ALLOC_MAB_OAA_WBREQ_PEND_LIST) == NULL) {
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_WBREQ_PEND_LIST_ALLOC_FAIL,
					  "oac_add_warmboot_req_in_wbreq_list()");
			return NCSCC_RC_FAILURE;
		}
		memset(tmp_list, '\0', sizeof(OAA_WBREQ_PEND_LIST));
		tmp_list->pcn = m_MMGR_ALLOC_MAB_PCN_STRING(lcl_strlen + 1);
		if (tmp_list->pcn == NULL) {
			/* Alloc failed. */
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PCN_STRING_ALLOC_FAIL,
					  "oac_add_warmboot_req_in_wbreq_list()");
			m_MMGR_FREE_MAB_OAA_WBREQ_PEND_LIST(tmp_list);
			return NCSCC_RC_FAILURE;
		}
		memset(tmp_list->pcn, '\0', lcl_strlen + 1);
		strcpy(tmp_list->pcn, wbr->i_pcn);
		tmp_list->is_system_client = wbr->is_system_client;

		/* Assign the wbreq_hdl here. */
		tmp_list->wbreq_hdl = ++inst->wbreq_hdl_index;
		tmp_list->eop_usr_ind_fnc = wbr->i_eop_usr_ind_fnc;

		if (prv_list != NULL)
			prv_list->next = tmp_list;
		else
			inst->wbreq_list = tmp_list;

		/* Add a node for the new request to the list "inst->wbreq_hdl_list" */
		if (oac_add_node_to_wbreq_hdl_list(inst, tmp_list->wbreq_hdl,
						   tmp_list->eop_usr_ind_fnc) != NCSCC_RC_SUCCESS) {
			/* Alloc failed. */
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_WBREQ_HDL_LIST_ALLOC_FAIL,
					  "oac_add_warmboot_req_in_wbreq_list()");
			m_MMGR_FREE_MAB_OAA_WBREQ_PEND_LIST(tmp_list);
			return NCSCC_RC_FAILURE;
		}

		/* Insert the tables list into the new PCN node. */
		{
			NCSOAC_PSS_TBL_LIST *prv_p_tbl = NULL, *lcl_tbl = NULL;
			NCSOAC_PSS_TBL_LIST *tbl_elem = wbr->i_tbl_list;

			while (tbl_elem != NULL) {
				if ((lcl_tbl = m_MMGR_ALLOC_NCSOAC_PSS_TBL_LIST) == NULL) {
					/* Alloc failed. */
					m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAC_NCSOAC_PSS_TBL_LIST_ALLOC_FAIL,
							  "oac_add_warmboot_req_in_wbreq_list()");
					/* Free unnecessary pointers. TBD. */
					oac_del_node_from_wbreq_hdl_list(inst, tmp_list->wbreq_hdl);
					m_MMGR_FREE_MAB_OAA_WBREQ_PEND_LIST(tmp_list);
					return NCSCC_RC_FAILURE;
				}
				memset(lcl_tbl, '\0', sizeof(NCSOAC_PSS_TBL_LIST));
				lcl_tbl->tbl_id = tbl_elem->tbl_id;

				if (prv_p_tbl == NULL) {
					tmp_list->tbl_list = prv_p_tbl = lcl_tbl;
				} else
					prv_p_tbl->next = lcl_tbl;

				tbl_elem = tbl_elem->next;
			}
		}
		*o_wbreq_node = tmp_list;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_add_node_to_wbreq_hdl_list

  DESCRIPTION:       Add a node to inst->wbreq_hdl_list 

  RETURNS:           SUCCESS or FAILURE.

*****************************************************************************/
static uns32 oac_add_node_to_wbreq_hdl_list(OAC_TBL *inst, uns32 wbreq_hdl, NCSOAC_EOP_USR_IND_FNC func)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	OAA_WBREQ_HDL_LIST *list = NULL, *prv_list = NULL;

	if ((func == NULL) || (wbreq_hdl == 0))
		return NCSCC_RC_SUCCESS;	/* No need to support EOP for this client */

	for (list = inst->wbreq_hdl_list;
	     ((list != NULL) && (list->wbreq_hdl != wbreq_hdl)); prv_list = list, list = list->next) {
		;
	}
	if (list == NULL) {
		/* Need add a new node. */
		if ((list = m_MMGR_ALLOC_OAA_WBREQ_HDL_LIST) == NULL)
			return NCSCC_RC_FAILURE;

		memset(list, '\0', sizeof(OAA_WBREQ_HDL_LIST));
		list->wbreq_hdl = wbreq_hdl;
		list->eop_usr_ind_fnc = func;
		list->next = NULL;

		if (prv_list != NULL)
			prv_list->next = list;
		else
			inst->wbreq_hdl_list = list;
	}

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_del_node_from_wbreq_hdl_list

  DESCRIPTION:       Delete a node from inst->wbreq_hdl_list 

  RETURNS:           void

*****************************************************************************/
static void oac_del_node_from_wbreq_hdl_list(OAC_TBL *inst, uns32 wbreq_hdl)
{
	OAA_WBREQ_HDL_LIST *list = NULL, *prv_list = NULL;

	for (list = inst->wbreq_hdl_list;
	     ((list != NULL) && (list->wbreq_hdl != wbreq_hdl)); prv_list = list, list = list->next) {
		;
	}
	if (list != NULL) {
		if (prv_list != NULL)
			prv_list->next = list->next;
		else
			inst->wbreq_hdl_list = list->next;

		m_MMGR_FREE_OAA_WBREQ_HDL_LIST(list);
	}

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_destroy_wbreq_hdl_list

  DESCRIPTION:       Destroy inst->wbreq_hdl_list 

  RETURNS:           void

*****************************************************************************/
static void oac_destroy_wbreq_hdl_list(OAC_TBL *inst)
{
	OAA_WBREQ_HDL_LIST *list = NULL, *nxt_list = NULL;

	list = inst->wbreq_hdl_list;
	while (list != NULL) {
		nxt_list = list->next;
		m_MMGR_FREE_OAA_WBREQ_HDL_LIST(list);
		list = nxt_list;
	}
	inst->wbreq_hdl_list = NULL;

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_refresh_table_bind_to_pssv

  DESCRIPTION:       Refresh the Table-bind list to PSS.

  RETURNS:           void.

*****************************************************************************/
void oac_refresh_table_bind_to_pssv(OAC_TBL *inst)
{
	MAB_PSS_TBL_BIND_EVT *bind_req = NULL;

	/* Send Table-Bind events of all the persistent MIBs in a single buffer to PSS. */
	if (oac_gen_tbl_bind(inst->pcn_list, &bind_req) != NCSCC_RC_SUCCESS)
		return;

	if (bind_req != NULL) {
		(void)oac_psr_send_bind_req(inst, bind_req, TRUE);
	}

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_send_pending_warmboot_reqs_to_pssv

  DESCRIPTION:       Sends pending warmboot-requests to PSS.

  RETURNS:           SUCCESS or FAILURE.

*****************************************************************************/
uns32 oac_send_pending_warmboot_reqs_to_pssv(OAC_TBL *inst)
{
	MAB_MSG msg;
	uns32 retval = NCSCC_RC_SUCCESS;
	MAB_PSS_WARMBOOT_REQ *req = NULL, *req_head = NULL, *p_req = NULL;
	MAB_PSS_TBL_LIST *olist = NULL, *p_tlist = NULL;
	NCSOAC_PSS_TBL_LIST *tlist = NULL;
	OAA_WBREQ_PEND_LIST *list = inst->wbreq_list;

	while (list != NULL) {
		if ((req = m_MMGR_ALLOC_MAB_PSS_WARMBOOT_REQ) == NULL) {
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PSS_WARMBOOT_REQ,
					  "oac_send_pending_warmboot_reqs_to_pssv()");
			oac_free_wbreq(req_head);
			return NCSCC_RC_FAILURE;
		}
		memset(req, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
		req->is_system_client = list->is_system_client;
		if ((req->pcn_list.pcn = m_MMGR_ALLOC_MAB_PCN_STRING(strlen(list->pcn) + 1)) == NULL) {
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PCN_STRING_ALLOC_FAIL,
					  "oac_send_pending_warmboot_reqs_to_pssv()");
			m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(req);
			oac_free_wbreq(req_head);
			return NCSCC_RC_FAILURE;
		}
		memset(req->pcn_list.pcn, '\0', (strlen(list->pcn)) + 1);
		strcpy(req->pcn_list.pcn, list->pcn);
		req->wbreq_hdl = list->wbreq_hdl;

		/* Duplicate the table-list */
		tlist = list->tbl_list;
		p_tlist = NULL;
		while (tlist != NULL) {
			if ((olist = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL) {

				m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
						  "oac_send_pending_warmboot_reqs_to_pssv()");
				oac_free_pss_tbl_list(req->pcn_list.tbl_list);
				m_MMGR_FREE_MAB_PCN_STRING(req->pcn_list.pcn);
				m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(req);
				oac_free_wbreq(req_head);
				return NCSCC_RC_FAILURE;
			}
			memset(olist, '\0', sizeof(MAB_PSS_TBL_LIST));
			olist->tbl_id = tlist->tbl_id;

			/* Add the element to the list */
			if (p_tlist == NULL) {
				p_tlist = req->pcn_list.tbl_list = olist;
			} else {
				p_tlist->next = olist;
				p_tlist = olist;
			}

			tlist = tlist->next;
		}

		if (req_head == NULL)
			p_req = req_head = req;
		else {
			p_req->next = req;
			p_req = req;
		}

		list = list->next;
	}

	if (req_head != NULL) {
		/* Send the message now to PSS */
		memset(&msg, '\0', sizeof(msg));
		msg.vrid = inst->vrid;
		msg.op = MAB_PSS_WARM_BOOT;
		msg.fr_anc = inst->my_anc;
		msg.data.data.oac_pss_warmboot_req = *req_head;
		for (req = req_head; req != NULL; req = req->next) {
			MAB_PSS_TBL_LIST *tmp = req->pcn_list.tbl_list;
			if (tmp != NULL) {
				while (tmp != NULL) {
					m_LOG_OAA_WARMBOOTREQ_INFO_III(NCSFL_SEV_NOTICE,
								       MAB_OAA_WARMREQ_INFO_III_REQ_MDS_EVT_TO_PSS_COMPOSED,
								       req->pcn_list.pcn, req->is_system_client,
								       tmp->tbl_id, req->wbreq_hdl);
					tmp = tmp->next;
				}
			} else {
				m_LOG_OAA_WARMBOOTREQ_INFO_III(NCSFL_SEV_NOTICE,
							       MAB_OAA_WARMREQ_INFO_III_REQ_MDS_EVT_TO_PSS_COMPOSED,
							       req->pcn_list.pcn, req->is_system_client, 0, 0);
			}
		}

		msg.data.seq_num = inst->seq_num;
		++inst->seq_num;
		retval = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS, inst->psr_vcard);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_SND_MSG_FAILED, NCSMDS_SVC_ID_OAC, retval);
			m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_OAC_WARMBOOT_REQ_TO_PSS_MDS_SEND_FAIL,
					 msg.data.seq_num);
			oac_free_wbreq(req_head);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_WARMBOOT_REQ_SEND_SUCCESS, msg.data.seq_num);

		/* Add this message to the buffer zone. */
		oac_psr_add_to_buffer_zone(inst, &msg);

		m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(req_head);
		oac_destroy_wb_pend_list(inst);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_free_wbreq

  DESCRIPTION:       Frees the MAB_PSS_WARMBOOT_REQ list.

  RETURNS:           void

*****************************************************************************/
void oac_free_wbreq(MAB_PSS_WARMBOOT_REQ *head)
{
	MAB_PSS_WARMBOOT_REQ *req = NULL;
	MAB_PSS_TBL_LIST *olist = NULL, *tmp_olist = NULL;

	while (head != NULL) {
		req = head->next;

		m_MMGR_FREE_MAB_PCN_STRING(head->pcn_list.pcn);
		olist = head->pcn_list.tbl_list;
		while (olist != NULL) {
			tmp_olist = olist->next;
			m_MMGR_FREE_MAB_PSS_TBL_LIST(olist);
			olist = tmp_olist;
		}
		m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(head);

		head = req;
	}

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_convert_input_wbreq_to_mab_request

  DESCRIPTION:       Validate the warmboot request parameters

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 oac_convert_input_wbreq_to_mab_request(OAC_TBL *inst,
					     NCSOAC_PSS_WARMBOOT_REQ *in_wbreq, MAB_PSS_WARMBOOT_REQ *req)
{
	OAC_TBL_REC *tbl_rec;
	NCS_BOOL is_wrong_tbl = FALSE;
	NCSOAC_PSS_TBL_LIST *tlist = NULL;
	MAB_PSS_TBL_LIST *tmp_olist = NULL;
	uns32 invalid_tbl_cnt = 0, total_tbl_cnt = 0;

	if (in_wbreq->i_pcn != NULL) {
		tlist = in_wbreq->i_tbl_list;
		while (tlist != NULL) {
			is_wrong_tbl = FALSE;
			if ((tbl_rec = oac_tbl_rec_find(inst, tlist->tbl_id)) != NULL) {
				if (!tbl_rec->is_persistent) {
					m_LOG_OAA_PCN_INFO_II(NCSFL_SEV_ERROR,
							      MAB_OAA_PCN_INFO_WARMBOOT_REQ_FOR_NON_PERSISTENT_TABLE_RCVD,
							      in_wbreq->i_pcn, tlist->tbl_id);
					is_wrong_tbl = TRUE;
				}
			} else {
				m_LOG_OAA_PCN_INFO_II(NCSFL_SEV_ERROR,
						      MAB_OAA_PCN_INFO_WARMBOOT_REQ_FOR_NON_BOUND_TABLE_RCVD,
						      in_wbreq->i_pcn, tlist->tbl_id);
				is_wrong_tbl = TRUE;
			}
			if (is_wrong_tbl) {
				/* Warmboot request for this table is not valid. So, deleting the 
				   node */
				invalid_tbl_cnt++;
				total_tbl_cnt++;
				tlist = tlist->next;
				continue;
			} else {
				/* Create to the node */
				if ((tmp_olist = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL) {
					m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
							  "oac_convert_input_wbreq_to_mab_request()");
					oac_free_pss_tbl_list(req->pcn_list.tbl_list);
					return NCSCC_RC_FAILURE;
				}
				memset(tmp_olist, '\0', sizeof(MAB_PSS_TBL_LIST));
				tmp_olist->tbl_id = tlist->tbl_id;

				/* Add to the list */
				tmp_olist->next = req->pcn_list.tbl_list;
				req->pcn_list.tbl_list = tmp_olist;
			}
			total_tbl_cnt++;
			tlist = tlist->next;
		}
		if ((in_wbreq->i_tbl_list != NULL) && (invalid_tbl_cnt == total_tbl_cnt)) {
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,
					   MAB_HDLN_OAC_WARMBOOT_REQ_REJECTED_DUE_TO_INVALID_TBLS_SPCFD);
			return NCSCC_RC_FAILURE;
		}
		req->pcn_list.pcn = m_MMGR_ALLOC_MAB_PCN_STRING(strlen(in_wbreq->i_pcn) + 1);
		if (req->pcn_list.pcn == NULL) {
			m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PCN_STRING_ALLOC_FAIL,
					  "oac_convert_input_wbreq_to_mab_request()");
			oac_free_pss_tbl_list(req->pcn_list.tbl_list);
			return NCSCC_RC_FAILURE;
		}
		memset(req->pcn_list.pcn, '\0', (strlen(in_wbreq->i_pcn) + 1));
		strcpy(req->pcn_list.pcn, in_wbreq->i_pcn);
		req->is_system_client = in_wbreq->is_system_client;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_free_pss_tbl_list

  DESCRIPTION:       Frees the MAB_PSS_TBL_LIST list.

  RETURNS:           void

*****************************************************************************/
void oac_free_pss_tbl_list(MAB_PSS_TBL_LIST *list)
{
	MAB_PSS_TBL_LIST *tmp = NULL;

	while (list != NULL) {
		tmp = list->next;
		m_MMGR_FREE_MAB_PSS_TBL_LIST(list);
		list = tmp;
	}
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_ss_tbl_reg

  DESCRIPTION:       Register MIB Table Ownership

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 oac_ss_tbl_reg(NCSOAC_TBL_OWNED *tbl_owned, uns32 tbl_id, uns32 oac_hdl)
{
	OAC_TBL *inst;
	OAC_TBL_REC *tbl_rec;

	m_OAC_LK_INIT;

	m_MAB_DBG_TRACE("\noac_ss_tbl_reg():entered.");

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(oac_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED, &inst->lock);

	m_LOG_MAB_API(MAB_API_OAC_TBL_REG);

	if ((tbl_rec = oac_tbl_rec_find(inst, tbl_id)) != NULL) {
		MAB_LM_EVT mle;
		mle.i_args = (NCSCONTEXT)(long)tbl_id;
		mle.i_event = OAC_TBL_REG_EXIST;
		mle.i_usr_key = inst->hm_hdl;
		mle.i_vrid = inst->vrid;
		mle.i_which_mab = NCSMAB_OAC;
		m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_TBL_REG_EXIST);
		inst->lm_cbfnc(&mle);

		m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_TBL_EXISTS);
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if ((tbl_owned->i_mib_req == NULL) || (tbl_owned->i_ss_id == 0)) {
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	tbl_rec = m_MMGR_ALLOC_OAC_TBL_REC;

	if (tbl_rec == NULL) {
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	memset(tbl_rec, 0, sizeof(OAC_TBL_REC));

	tbl_rec->dfltr_regd = FALSE;
	tbl_rec->tbl_id = tbl_id;
	tbl_rec->ss_id = tbl_owned->i_ss_id;
	tbl_rec->handle = tbl_owned->i_mib_key;
	tbl_rec->req_fnc = tbl_owned->i_mib_req;
	if (tbl_owned->is_persistent == TRUE) {
		tbl_rec->is_persistent = tbl_owned->is_persistent;
		if ((tbl_owned->i_pcn == NULL) || ((char)tbl_owned->i_pcn[0] == '\0')) {
			/* Invalid PCN name. Don't make this MIB persistent. */
			tbl_rec->is_persistent = FALSE;
			m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_INVLD_PCN_IN_TBL_REG, tbl_id);
			/* Throw up a notification to the user also. ???? TBD. */
			m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_NOTICE, MAB_OAA_PCN_INFO_TBL_MADE_NON_PERSISTENT, tbl_id);
		} else {
			OAA_PCN_LIST *p_pcn = NULL;
			p_pcn = oac_findadd_pcn_in_list(inst, tbl_owned->i_pcn, FALSE,	/* Don't bother about this */
							tbl_id, TRUE);
			if (p_pcn == NULL) {
				/* Add to the PCN list failed. Appropriate action to be taken. */
				m_MMGR_FREE_OAC_TBL_REC(tbl_rec);
				m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_OAC_PCN_LIST_ADD_FAIL, tbl_id);
				ncshm_give_hdl(oac_hdl);
				m_OAC_UNLK(&inst->lock);
				m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			if ((inst->psr_here) && (inst->is_active == TRUE)) {
				MAB_PSS_TBL_BIND_EVT bind_evt;

				m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_INVOKING_SEND_BIND_REQ_TO_PSS);

				memset(&bind_evt, '\0', sizeof(MAB_PSS_TBL_BIND_EVT));

				if ((bind_evt.pcn_list.pcn =
				     m_MMGR_ALLOC_MAB_PCN_STRING(strlen(p_pcn->pcn) + 1)) == NULL) {
					m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PCN_STRING_ALLOC_FAIL,
							  "oac_ss_tbl_reg()");
					return NCSCC_RC_FAILURE;
				}
				memset(bind_evt.pcn_list.pcn, '\0', (strlen(p_pcn->pcn) + 1));
				strcpy(bind_evt.pcn_list.pcn, p_pcn->pcn);

				if ((bind_evt.pcn_list.tbl_list = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL) {
					m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
							  "oac_ss_tbl_reg()");
					m_MMGR_FREE_MAB_PCN_STRING(bind_evt.pcn_list.pcn);
					return NCSCC_RC_FAILURE;
				}
				memset(bind_evt.pcn_list.tbl_list, '\0', sizeof(MAB_PSS_TBL_LIST));
				bind_evt.pcn_list.tbl_list->tbl_id = tbl_id;

				(void)oac_psr_send_bind_req(inst, &bind_evt, FALSE);
			} else {
				m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_INFO, MAB_OAA_PCN_INFO_PSS_NOT_PRESENT_DURING_TBL_REG,
						     tbl_id);
			}
		}
	}

	oac_tbl_rec_put(inst, tbl_rec);
	m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_NEW_TBL_SUCCESS);

	ncshm_give_hdl(oac_hdl);
	m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_REGD_TBL);
	m_OAC_UNLK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);

	m_MAB_DBG_TRACE("\noac_ss_tbl_reg():left.");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_psr_send_bind_req

  DESCRIPTION:       Send Table-BIND request to PSS.

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 oac_psr_send_bind_req(OAC_TBL *inst, MAB_PSS_TBL_BIND_EVT *bind_req, NCS_BOOL free_head)
{
	MAB_MSG msg;
	uns32 code;

	/* Forward this message to the PSS */
	memset(&msg, 0, sizeof(msg));
	msg.op = MAB_PSS_TBL_BIND;
	msg.data.data.oac_pss_tbl_bind = *bind_req;
	msg.fr_anc = inst->my_anc;

	msg.data.seq_num = inst->seq_num;
	code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS, inst->psr_vcard);
	if (code != NCSCC_RC_SUCCESS) {
		MAB_PSS_TBL_LIST *list = bind_req->pcn_list.tbl_list;

		m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_SND_MSG_FAILED, NCSMDS_SVC_ID_OAC, code);

		while (list != NULL) {
			m_LOG_OAA_PCN_INFO_II(NCSFL_SEV_ERROR, MAB_OAA_PCN_INFO_BIND_REQ_TO_PSS_MDS_SND_FAIL,
					      bind_req->pcn_list.pcn, list->tbl_id);
			list = list->next;
		}
		m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_OAC_BIND_REQ_TO_PSS_SND_FAIL, msg.data.seq_num);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
/* #if (MAB_LOG == 1) */
	{
		MAB_PSS_TBL_LIST *list = bind_req->pcn_list.tbl_list;
		while (list != NULL) {
			m_LOG_OAA_PCN_INFO_II(NCSFL_SEV_NOTICE, MAB_OAA_PCN_INFO_BIND_REQ_TO_PSS_MDS_SND_SUCCESS,
					      bind_req->pcn_list.pcn, list->tbl_id);
			/* m_LOG_OAA_BIND_EVT(bind_req->pcn_list.pcn, list->tbl_id); */
			list = list->next;
		}
		m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_BIND_REQ_TO_PSS_SND_SUCCESS, msg.data.seq_num);
	}
/* #endif */

	if (free_head == TRUE) {
		oac_free_bind_req_list(bind_req);
	} else {
		oac_free_bind_req_list(bind_req->next);
		bind_req->next = NULL;
		m_MMGR_FREE_MAB_PCN_STRING(bind_req->pcn_list.pcn);
		oac_free_pss_tbl_list(bind_req->pcn_list.tbl_list);
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_ss_tbl_unreg

  DESCRIPTION:       Unregister Table Ownerhsip. This causes any lingering
                     Filters to be unregistered as well.

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 oac_ss_tbl_unreg(NCSOAC_TBL_GONE *tbl_gone, uns32 tbl_id, uns32 oac_hdl)
{
	OAC_TBL *inst;
	OAC_TBL_REC *tbl_rec;
	OAC_FLTR *fltr;
	OAC_FLTR *next_fltr;

	m_OAC_LK_INIT;

	m_MAB_DBG_TRACE("\noac_ss_tbl_unreg():entered.");

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(oac_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED, &inst->lock);

	m_LOG_MAB_API(MAB_API_OAC_TBL_UNREG);

	if ((tbl_rec = oac_tbl_rec_find(inst, tbl_id)) == NULL) {
		MAB_LM_EVT mle;
		mle.i_args = (NCSCONTEXT)(long)tbl_id;
		mle.i_event = OAC_TBL_UNREG_NONEXIST;
		mle.i_usr_key = inst->hm_hdl;
		mle.i_vrid = inst->vrid;
		mle.i_which_mab = NCSMAB_OAC;
		m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_TBL_UNREG_NONEXIST);
		inst->lm_cbfnc(&mle);

		ncshm_give_hdl(oac_hdl);
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NO_TBL_TO_UNREG);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* now we need to unregister all the filters in the filter list */
	if (tbl_rec->fltr_list != NULL) {
		for (fltr = tbl_rec->fltr_list; fltr != NULL; fltr = next_fltr) {
			next_fltr = fltr->next;
			if (oac_fltr_unreg_xmit(inst, fltr->fltr_id, tbl_rec->tbl_id) == NCSCC_RC_FAILURE) {
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_SCHD_UNREG_FLTR_NOT);
				ncshm_give_hdl(oac_hdl);
				m_OAC_UNLK(&inst->lock);
				m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_SCHD_UNREG_FLTR_OK);
		}

		if (tbl_rec->fltr_list == NULL) {
			if (tbl_rec->dfltr_regd == TRUE) {
				tbl_rec->dfltr_regd = FALSE;
				if (oac_fltr_unreg_xmit(inst, 1, tbl_rec->tbl_id) == NCSCC_RC_FAILURE) {
					m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_SCHD_UNREG_FLTR_NOT);
					ncshm_give_hdl(oac_hdl);
					m_OAC_UNLK(&inst->lock);
					m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}
				m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_SCHD_UNREG_FLTR_OK);
			}

			if ((tbl_rec = oac_tbl_rec_rmv(inst, tbl_id)) == NULL) {
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_DEL_TBL_REC_FAILED);
				ncshm_give_hdl(oac_hdl);
				m_OAC_UNLK(&inst->lock);
				m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			if ((tbl_rec->is_persistent) && (inst->is_active == TRUE)) {
				(void)oac_psr_send_unbind_req(inst, tbl_id);
			}

			oac_del_pcn_from_list(inst, tbl_id);
			m_MMGR_FREE_OAC_TBL_REC(tbl_rec);
			m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_DEL_TBL_REC_SUCCESS);
		} else {
			/* schedule this row for deletion...
			   once OAC sends all MAS_UNREG msgs it will delete it...
			 */

			/* KCQ: we shouldn't be here */
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_SCHD_DEL_TBL_REC);
		}
	} else {
		if (tbl_rec->dfltr_regd == TRUE) {
			tbl_rec->dfltr_regd = FALSE;
			if (oac_fltr_unreg_xmit(inst, 1, tbl_rec->tbl_id) == NCSCC_RC_FAILURE) {
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_SCHD_UNREG_FLTR_NOT);
				ncshm_give_hdl(oac_hdl);
				m_OAC_UNLK(&inst->lock);
				m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_SCHD_UNREG_FLTR_OK);
		}
		if ((tbl_rec = oac_tbl_rec_rmv(inst, tbl_id)) == NULL) {
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_DEL_TBL_REC_FAILED);
			ncshm_give_hdl(oac_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		oac_del_pcn_from_list(inst, tbl_id);
		if ((tbl_rec->is_persistent) && (inst->is_active == TRUE)) {
			(void)oac_psr_send_unbind_req(inst, tbl_id);	/* Send to PSS anyway */
		}
		m_MMGR_FREE_OAC_TBL_REC(tbl_rec);
		m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_DEL_TBL_REC_SUCCESS);
	}

	ncshm_give_hdl(oac_hdl);
	m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_UNREGD_TBL);
	m_OAC_UNLK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);

	m_MAB_DBG_TRACE("\noac_ss_tbl_unreg():left.");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_psr_send_unbind_req

  DESCRIPTION:       Send Table-UNBIND request to PSS.

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/
uns32 oac_psr_send_unbind_req(OAC_TBL *inst, uns32 tbl_id)
{
	MAB_MSG msg;
	uns32 code;

	/* Forward this message to the PSS */
	memset(&msg, 0, sizeof(msg));
	msg.op = MAB_PSS_TBL_UNBIND;
	msg.data.data.oac_pss_tbl_unbind.tbl_id = tbl_id;
	msg.fr_anc = inst->my_anc;

	msg.data.seq_num = inst->seq_num;
	code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS, inst->psr_vcard);
	if (code != NCSCC_RC_SUCCESS) {
		m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_OAC_MDS_SND_MSG_FAILED, NCSMDS_SVC_ID_OAC, code);
		m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_ERROR, MAB_OAA_PCN_INFO_UNBIND_REQ_TO_PSS_MDS_SND_FAIL, tbl_id);
		m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_OAC_UNBIND_REQ_TO_PSS_SND_FAIL, msg.data.seq_num);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* m_LOG_OAA_UNBIND_EVT(tbl_id); */
	m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_NOTICE, MAB_OAA_PCN_INFO_UNBIND_REQ_TO_PSS_MDS_SND_SUCCESS, tbl_id);
	m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_UNBIND_REQ_TO_PSS_SND_SUCCESS, msg.data.seq_num);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_del_pcn_from_list

  DESCRIPTION:       Delete PCN from the pcn_list.

  RETURNS:           void

*****************************************************************************/
void oac_del_pcn_from_list(OAC_TBL *inst, uns32 tbl_id)
{
	OAA_PCN_LIST *tmp = inst->pcn_list, *prv_tmp = NULL;

	for (tmp = inst->pcn_list; tmp != NULL; prv_tmp = tmp, tmp = tmp->next) {
		if (tmp->tbl_id == tbl_id) {
			m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_INFO, MAB_OAA_PCN_INFO_TBL_FND_IN_PCN_LIST_DURING_UNBIND_OP,
					     tbl_id);

			if (prv_tmp == NULL)
				inst->pcn_list = tmp->next;
			else
				prv_tmp->next = tmp->next;
			m_MMGR_FREE_MAB_PCN_STRING(tmp->pcn);
			m_MMGR_FREE_MAB_OAA_PCN_LIST(tmp);
			return;
		}
	}

	m_LOG_OAA_PCN_INFO_I(NCSFL_SEV_ERROR, MAB_OAA_PCN_INFO_TBL_NOT_FND_IN_PCN_LIST_DURING_UNBIND_OP, tbl_id);

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_ss_row_reg

  DESCRIPTION:       Declare MIB Table Row ownership, as described in a 
                     MIB Table Row Filter.

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/

uns32 oac_ss_row_reg(NCSOAC_ROW_OWNED *row_owned, uns32 tbl_id, uns32 oac_hdl)
{
	OAC_TBL *inst;
	OAC_TBL_REC *tbl_rec;
	OAC_FLTR *fltr = NULL;
	OAC_FLTR *new_fltr = NULL;
	NCS_BOOL match_found = FALSE;

	m_OAC_LK_INIT;

	m_MAB_DBG_TRACE("\noac_ss_row_reg():entered.");

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(oac_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED, &inst->lock);

	m_LOG_MAB_API(MAB_API_OAC_ROW_REG);

	if ((tbl_rec = oac_tbl_rec_find(inst, tbl_id)) == NULL) {
		MAB_LM_EVT mle;
		mle.i_args = (NCSCONTEXT)(long)tbl_id;
		mle.i_event = OAC_FLTR_REG_NO_TBL;
		mle.i_usr_key = inst->hm_hdl;
		mle.i_vrid = inst->vrid;
		mle.i_which_mab = NCSMAB_OAC;
		m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_FLTR_REG_NO_TBL);
		inst->lm_cbfnc(&mle);

		ncshm_give_hdl(oac_hdl);
		m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_OAC_FLTR_REG_NO_TBL, inst->vrid, tbl_id);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	fltr = tbl_rec->fltr_list;	/* OK, set the OAC Filter ptr SMM */

	if (tbl_rec->fltr_list != NULL) {
		if (row_owned->i_fltr.type == NCSMAB_FLTR_DEFAULT) {
			if ((fltr->fltr.type != NCSMAB_FLTR_RANGE) && (fltr->fltr.type != NCSMAB_FLTR_EXACT)) {
				MAB_LM_EVT mle;
				MAB_LM_FLTR_TYPE_MM ftm;
				ftm.i_xst_type = fltr->fltr.type;
				ftm.i_new_type = row_owned->i_fltr.type;
				mle.i_args = (NCSCONTEXT)&ftm;
				mle.i_event = OAC_FLTR_TYPE_MISMATCH;
				mle.i_usr_key = inst->hm_hdl;
				mle.i_vrid = inst->vrid;
				mle.i_which_mab = NCSMAB_OAC;
				m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_FLTR_TYPE_MM);
				inst->lm_cbfnc(&mle);

				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_REG_WRONG_FLTR_TYPE);
				ncshm_give_hdl(oac_hdl);
				m_OAC_UNLK(&inst->lock);
				m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}

		/* filters must be of the same type */
		if ((fltr->fltr.type != row_owned->i_fltr.type) && (row_owned->i_fltr.type != NCSMAB_FLTR_DEFAULT)) {
			MAB_LM_EVT mle;
			MAB_LM_FLTR_TYPE_MM ftm;
			ftm.i_xst_type = fltr->fltr.type;
			ftm.i_new_type = row_owned->i_fltr.type;
			mle.i_args = (NCSCONTEXT)&ftm;
			mle.i_event = OAC_FLTR_TYPE_MISMATCH;
			mle.i_usr_key = inst->hm_hdl;
			mle.i_vrid = inst->vrid;
			mle.i_which_mab = NCSMAB_OAC;
			m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_FLTR_TYPE_MM);
			inst->lm_cbfnc(&mle);

			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_REG_WRONG_FLTR_TYPE);
			ncshm_give_hdl(oac_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	} else if ((row_owned->i_fltr.type != NCSMAB_FLTR_DEFAULT) &&
		   (tbl_rec->dfltr_regd == TRUE) &&
		   ((row_owned->i_fltr.type != NCSMAB_FLTR_RANGE) && (row_owned->i_fltr.type != NCSMAB_FLTR_EXACT))) {
		MAB_LM_EVT mle;
		MAB_LM_FLTR_TYPE_MM ftm;
		ftm.i_xst_type = fltr->fltr.type;
		ftm.i_new_type = row_owned->i_fltr.type;
		mle.i_args = (NCSCONTEXT)&ftm;
		mle.i_event = OAC_FLTR_TYPE_MISMATCH;
		mle.i_usr_key = inst->hm_hdl;
		mle.i_vrid = inst->vrid;
		mle.i_which_mab = NCSMAB_OAC;
		m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_FLTR_TYPE_MM);
		inst->lm_cbfnc(&mle);

		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_REG_WRONG_FLTR_TYPE);
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	if (row_owned->i_fltr.type == NCSMAB_FLTR_DEFAULT) {
		OAC_FLTR fltr;
		memset(&fltr, 0, sizeof(fltr));
		fltr.fltr.type = NCSMAB_FLTR_DEFAULT;
		tbl_rec->dfltr_regd = TRUE;
		tbl_rec->ss_cb_fnc = row_owned->i_ss_cb;
		tbl_rec->ss_cb_hdl = row_owned->i_ss_hdl;
		if ((tbl_rec->ss_cb_fnc == NULL) || (tbl_rec->ss_cb_hdl == 0)) {
			ncshm_give_hdl(oac_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		/* KCQ: default filter id is always 1 */
		row_owned->o_row_hdl = 1;

		if (oac_fltr_reg_xmit(inst, &fltr, tbl_rec->tbl_id) == NCSCC_RC_FAILURE) {
			ncshm_give_hdl(oac_hdl);
			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NO_FRWD_REG_TO_MAS);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
	} else {
		if ((row_owned->i_fltr.is_move_row_fltr == TRUE) && (row_owned->i_fltr.type == NCSMAB_FLTR_RANGE)) {
			OAC_FLTR *fltr;

			for (fltr = tbl_rec->fltr_list; fltr != NULL; fltr = fltr->next) {
				if (fltr->fltr.is_move_row_fltr == TRUE) {
					if ((row_owned->i_fltr.fltr.range.i_bgn_idx == fltr->fltr.fltr.range.i_bgn_idx)
					    && (row_owned->i_fltr.fltr.range.i_idx_len ==
						fltr->fltr.fltr.range.i_idx_len)
					    &&
					    (memcmp
					     (row_owned->i_fltr.fltr.range.i_min_idx_fltr,
					      fltr->fltr.fltr.range.i_min_idx_fltr,
					      (fltr->fltr.fltr.range.i_idx_len) * sizeof(uns32)) == 0)
					    ) {
						match_found = TRUE;
						break;
					}
				}
			}

			if (match_found == TRUE) {
				/* reuse the OAC filter */
				row_owned->o_row_hdl = fltr->fltr_id;
				m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_NEW_FLTR_SUCCESS);
			}
		}		/* range filter */
		if (row_owned->i_fltr.type == NCSMAB_FLTR_EXACT) {
			fltr = oac_fltrs_exact_fltr_locate(tbl_rec->fltr_list, &row_owned->i_fltr.fltr.exact);
			if (fltr != NULL) {
				row_owned->o_row_hdl = fltr->fltr_id;
				m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_NEW_FLTR_SUCCESS);
			}
		}
		/* if (row_owned->i_fltr.type == NCSMAB_FLTR_EXACT) */
		if (match_found == FALSE) {
			if ((new_fltr = oac_fltr_create(inst, &(row_owned->i_fltr))) == NULL) {
				ncshm_give_hdl(oac_hdl);
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NEW_FLTR_FAILED);
				m_OAC_UNLK(&inst->lock);
				m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			row_owned->o_row_hdl = new_fltr->fltr_id;

			oac_fltr_put(&(tbl_rec->fltr_list), new_fltr);

			m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_NEW_FLTR_SUCCESS);

			if (oac_fltr_reg_xmit(inst, new_fltr, tbl_rec->tbl_id) == NCSCC_RC_FAILURE) {
				ncshm_give_hdl(oac_hdl);
				m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NO_FRWD_REG_TO_MAS);
				m_OAC_UNLK(&inst->lock);
				m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
		}
	}

	m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_FRWD_FLTR_REG_TO_MAS);

	ncshm_give_hdl(oac_hdl);
	m_OAC_UNLK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);

	m_MAB_DBG_TRACE("\noac_ss_row_reg():left.");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_ss_row_unreg

  DESCRIPTION:       withdraw ownership claims of MIB Table Rows by handing
                     back the handle that maps to the MIB Table Row Filter 
                     in question.

  RETURNS:           SUCCESS or FAILURE

*****************************************************************************/

uns32 oac_ss_row_unreg(NCSOAC_ROW_GONE *row_gone, uns32 tbl_id, uns32 oac_hdl)
{
	OAC_TBL *inst;
	OAC_TBL_REC *tbl_rec;
	OAC_FLTR *fltr;

	m_OAC_LK_INIT;

	m_MAB_DBG_TRACE("\noac_ss_row_unreg():entered.");

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(oac_hdl)) == NULL) {
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_OAC_LK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED, &inst->lock);

	m_LOG_MAB_API(MAB_API_OAC_ROW_UNREG);

	if ((tbl_rec = oac_tbl_rec_find(inst, tbl_id)) == NULL) {
		MAB_LM_EVT mle;
		mle.i_args = (NCSCONTEXT)(long)tbl_id;
		mle.i_event = OAC_FLTR_UNREG_NO_TBL;
		mle.i_usr_key = inst->hm_hdl;
		mle.i_vrid = inst->vrid;
		mle.i_which_mab = NCSMAB_OAC;
		m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_FLTR_UNREG_NO_TBL);
		inst->lm_cbfnc(&mle);

		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NO_TBL_TO_UNREG);
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if ((tbl_rec->fltr_list == NULL) && (tbl_rec->dfltr_regd != TRUE)) {
		MAB_LM_EVT mle;
		MAB_LM_FLTR_NULL fn;
		fn.i_fltr_id = row_gone->i_row_hdl;
		fn.i_vcard = inst->my_vcard;
		mle.i_args = (NCSCONTEXT)&fn;
		mle.i_event = OAC_FLTR_UNREG_NO_FLTR;
		mle.i_usr_key = inst->hm_hdl;
		mle.i_vrid = inst->vrid;
		mle.i_which_mab = NCSMAB_OAC;
		m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_FLTR_UNREG_NO_FLTR);
		inst->lm_cbfnc(&mle);

		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NO_FLTRS_TO_UNREG);
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	if ((uns32)row_gone->i_row_hdl > 1) {
		if ((fltr = oac_fltr_find(&(tbl_rec->fltr_list), row_gone->i_row_hdl)) == NULL) {
			MAB_LM_EVT mle;
			MAB_LM_FLTR_NULL fn;
			fn.i_fltr_id = row_gone->i_row_hdl;
			fn.i_vcard = inst->my_vcard;
			mle.i_args = (NCSCONTEXT)&fn;
			mle.i_event = OAC_FLTR_UNREG_NO_FLTR;
			mle.i_usr_key = inst->hm_hdl;
			mle.i_vrid = inst->vrid;
			mle.i_which_mab = NCSMAB_OAC;
			m_LOG_MAB_EVT(NCSFL_SEV_ERROR, MAB_EV_LM_OAC_FLTR_UNREG_NO_FLTR);
			inst->lm_cbfnc(&mle);

			m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_FIND_FLTR_FAILED);
			ncshm_give_hdl(oac_hdl);
			m_OAC_UNLK(&inst->lock);
			m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		}
		m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_FIND_FLTR_SUCCESS);
	}
	if (oac_fltr_unreg_xmit(inst, row_gone->i_row_hdl, tbl_rec->tbl_id) == NCSCC_RC_FAILURE) {
		m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NO_FRWD_UNREG);
		ncshm_give_hdl(oac_hdl);
		m_OAC_UNLK(&inst->lock);
		m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_UNREGD_MAB_FLTR);

	ncshm_give_hdl(oac_hdl);
	m_OAC_UNLK(&inst->lock);
	m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED, &inst->lock);

	m_MAB_DBG_TRACE("\noac_ss_row_unreg():left.");
	return NCSCC_RC_SUCCESS;
}

/*#############################################################################
 *
 *                   PRIVATE OAC LAYER MANAGEMENT IMPLEMENTAION
 *
 *############################################################################*/

/*****************************************************************************

  PROCEDURE NAME:    oac_svc_create

  DESCRIPTION:       Create an instance of OAC, set configuration profile to
                     default, install this OAC with MDS and subscribe to OAC 
                     events.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 oac_svc_create(NCSOAC_CREATE *create)
{
	OAC_TBL *inst;
	NCSMDS_INFO mds_info;
	MDS_SVC_ID subsvc[2];
	NCS_SPIR_REQ_INFO spir_info;
	uns32 status;

	/* log the function entry */
	m_LOG_MAB_API(MAB_API_OAC_SVC_CREATE);

	/* initialize the lock */
	m_OAC_LK_INIT;

	/* if no callback, no work will be done */
	if (create->i_lm_cbfnc == NULL) {
		/* log the failure */
		m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_SVC_PRVDR, MAB_OAC_ERR_NO_LM_CALLBACK);
		return NCSCC_RC_FAILURE;
	}

	/* allocate memory for this instance of OAA */
	if ((inst = m_MMGR_ALLOC_OAC_TBL) == NULL) {
		/* log the memory allocation failure */
		m_LOG_MAB_MEMFAIL(NCSFL_SEV_ERROR, MAB_MF_OAC_INST_ALLOC_FAILED, "oac_svc_create()");
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_OAC_ERR_CB_ALLOC_FAILED_ENVID, create->i_vrid);
		return NCSCC_RC_FAILURE;
	}

	/* initialize the OAA control block */
	memset(inst, 0, sizeof(OAC_TBL));

	/* initialize locks */
	m_OAC_LK_CREATE(&inst->lock);
	m_OAC_LK(&inst->lock);

	/* fill in the other details */
	inst->vrid = create->i_vrid;

	/* do I need to do this again, verify in the unit testing.  TBD Mahesh */
	memset(&inst->mas_vcard, 0, sizeof(inst->mas_vcard));

	/* store mailbox ptr and callback */
	inst->mbx = create->i_mbx;
	inst->lm_cbfnc = create->i_lm_cbfnc;

	/* initailize the hash table */
	oac_tbl_rec_init(inst);

	/* initialize the filter count */
	inst->nxt_fltr_id = 1;

	/* initialize the PSR related variables */
	inst->playback_session = FALSE;
	inst->psr_here = FALSE;

	/* give the handle back to the caller */
	inst->hm_hdl = ncshm_create_hdl(create->i_hm_poolid, NCS_SERVICE_ID_MAB, inst);
	create->o_oac_hdl = inst->hm_hdl;

	/* get the PWE handle of this OAA */
	memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
	spir_info.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
	spir_info.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
	memcpy(&spir_info.i_instance_name, &create->i_inst_name, sizeof(SaNameT));
	spir_info.i_environment_id = create->i_vrid;
	status = ncs_spir_api(&spir_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_OAC_ERR_SPIR_LOOKUP_CREATE_FAILED, status, create->i_vrid);
		/* free the allocated OAA instance */
		m_OAC_UNLK(&inst->lock);
		m_MMGR_FREE_OAC_TBL(inst);
		return status;
	}

	/* store the PWE handle of OAA */
	inst->mds_hdl = (MDS_HDL)spir_info.info.lookup_create_inst.o_handle;

	/* OAC joins the MDS crowd... advertises its presence */
	memset(&mds_info, 0, sizeof(mds_info));
	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_op = MDS_INSTALL;
	mds_info.i_svc_id = NCSMDS_SVC_ID_OAC;
	mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
	mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	mds_info.info.svc_install.i_svc_cb = oac_mds_cb;
	mds_info.info.svc_install.i_yr_svc_hdl = inst->hm_hdl;
	mds_info.info.svc_install.i_mds_svc_pvt_ver = (uns8)OAC_MDS_SUB_PART_VERSION;
	status = ncsmds_api(&mds_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_OAC_ERR_MDS_INSTALL_FAILED, status, create->i_vrid);
		/* free the allocated OAA instance */
		m_OAC_UNLK(&inst->lock);
		m_MMGR_FREE_OAC_TBL(inst);
		return NCSCC_RC_FAILURE;
	}
	/* get my anchor value and my destination address */
	inst->my_vcard = mds_info.info.svc_install.o_dest;
	inst->my_anc = mds_info.info.svc_install.o_anc;
	if (m_MDS_DEST_IS_AN_ADEST(inst->my_vcard)) {
		/* This is ADEST. This is no redundancy model associated with ADEST. */
		inst->is_active = TRUE;
	} else {
		NCSMDS_INFO info;

		/* This is VDEST. So, get the initial role from MDS */
		memset(&info, 0, sizeof(NCSMDS_INFO));
		info.i_op = MDS_QUERY_PWE;
		info.i_svc_id = NCSMDS_SVC_ID_OAC;
		info.i_mds_hdl = inst->mds_hdl;
		status = ncsmds_api(&info);
		if (status != NCSCC_RC_SUCCESS) {
			/* log the failure */
			m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
					   MAB_OAC_ERR_PWE_QUERY_FAILED, status, create->i_vrid);
			/* free the allocated OAA instance */
			m_OAC_UNLK(&inst->lock);
			m_MMGR_FREE_OAC_TBL(inst);
			return status;
		}
		if (info.info.query_pwe.info.virt_info.o_role == V_DEST_RL_ACTIVE)
			inst->is_active = TRUE;
	}

	/* initialize PSS interface related data structs */
	/* inst->warmboot_start = FALSE; */
	inst->pcn_list = NULL;
	inst->wbreq_list = NULL;

	/* subscribe to MAS and PSR */
	memset(&mds_info, 0, sizeof(mds_info));
	subsvc[0] = NCSMDS_SVC_ID_MAS;
	subsvc[1] = NCSMDS_SVC_ID_PSS;
	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_op = MDS_SUBSCRIBE;
	mds_info.i_svc_id = NCSMDS_SVC_ID_OAC;
	mds_info.info.svc_subscribe.i_num_svcs = 2;
	mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	mds_info.info.svc_subscribe.i_svc_ids = subsvc;
	status = ncsmds_api(&mds_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_OAC_ERR_MDS_SUBSCRB_FAILED, status, create->i_vrid);
		/* free the allocated OAA instance */
		m_OAC_UNLK(&inst->lock);
		m_MMGR_FREE_OAC_TBL(inst);
		return NCSCC_RC_FAILURE;
	}

	/* subscribe to OAA(self) SVC-events */
	memset(&mds_info, 0, sizeof(mds_info));
	subsvc[0] = NCSMDS_SVC_ID_OAC;
	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_op = MDS_RED_SUBSCRIBE;
	mds_info.i_svc_id = NCSMDS_SVC_ID_OAC;
	mds_info.info.svc_subscribe.i_num_svcs = 1;
	mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_INTRANODE;	/* NCSMDS_SCOPE_INTRACHASSIS,NCSMDS_SCOPE_INTRANODE */
	mds_info.info.svc_subscribe.i_svc_ids = subsvc;
	status = ncsmds_api(&mds_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_OAC_ERR_MDS_OAA_SUBSCRB_FAILED, status, create->i_vrid);
		/* free the allocated OAA instance */
		m_OAC_UNLK(&inst->lock);
		m_MMGR_FREE_OAC_TBL(inst);
		return NCSCC_RC_FAILURE;
	}

	m_OAC_UNLK(&inst->lock);
	return status;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_svc_destroy

  DESCRIPTION:       Destroy an instance of OAC. Withdraw from MDS and free
                     this OAC_TBL and tend to other resource recovery issues.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 oac_svc_destroy(NCSOAC_DESTROY *destroy)
{
	OAC_TBL *inst;
	uns32 i;
	OAC_TBL_REC *tbl_rec;
	OAC_TBL_REC *del_tbl_rec;
	NCS_BOOL is_next_tbl_set = FALSE;
	OAC_FLTR *fltr;
	OAC_FLTR *del_fltr;
	NCSMDS_INFO info;
	uns32 status;
	NCS_SPIR_REQ_INFO spir_info;

	/* log the function entry */
	m_LOG_MAB_API(MAB_API_OAC_SVC_DESTROY);

	/* TBD , what is this for? */
	m_OAC_LK_INIT;

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(destroy->i_oac_hdl)) == NULL) {
		/* log the failure */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_OAC_ERR_CB_NULL, destroy->i_oac_hdl);
		return NCSCC_RC_FAILURE;
	}

	/* take the lock */
	m_OAC_LK(&inst->lock);

	/* Tear down the services                                     */
	/* Unsubscribe from service events is implicit with Uninstall */
	memset(&info, 0, sizeof(info));
	info.i_mds_hdl = inst->mds_hdl;
	info.i_svc_id = NCSMDS_SVC_ID_OAC;
	info.i_op = MDS_UNINSTALL;
	status = ncsmds_api(&info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_OAC_ERR_MDS_UNINSTALL_FAILED, status, destroy->i_env_id);
		m_MAB_DBG_VOID;
		/* do not return, continue */
		m_MAB_DBG_VOID;
	}

	/* release the PWE instance from SPRR */
	memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
	spir_info.type = NCS_SPIR_REQ_REL_INST;
	spir_info.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
	memcpy(&spir_info.i_instance_name, &destroy->i_inst_name, sizeof(SaNameT));
	spir_info.i_environment_id = destroy->i_env_id;
	status = ncs_spir_api(&spir_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_OAC_ERR_SPIR_REL_INST_FAILED, status, destroy->i_env_id);
	}

	/* clean the table details */
	for (i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++) {
		for (tbl_rec = inst->hash[i]; tbl_rec != NULL; is_next_tbl_set = FALSE) {
			for (fltr = tbl_rec->fltr_list; fltr != NULL;) {
				del_fltr = fltr;
				fltr = fltr->next;
				m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG,
						       MAB_OAC_FLTR_RMV_SUCCESS, tbl_rec->tbl_id,
						       0, del_fltr->fltr.type);

				/* clean the filter details (indices) */
				mas_mab_fltr_indices_cleanup(&del_fltr->fltr);
				/*m_MMGR_FREE_MAS_FLTR(del_fltr); */
				m_MMGR_FREE_OAC_FLTR(del_fltr);

				if (fltr == NULL) {
					del_tbl_rec = tbl_rec;
					tbl_rec = tbl_rec->next;
					is_next_tbl_set = TRUE;
					m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG,
							      MAB_OAC_RMV_ROW_REC_SUCCESS, inst->vrid,
							      del_tbl_rec->tbl_id);
					/*m_MMGR_FREE_MAS_ROW_REC(del_tbl_rec); */
					m_MMGR_FREE_OAC_TBL_REC(del_tbl_rec);
				}
			}	/* for(fltr = tbl_rec->fltr_list;fltr != NULL;) */

			if (is_next_tbl_set == FALSE) {
				tbl_rec = tbl_rec->next;
			}
		}		/* for(tbl_rec = inst->hash[i];tbl_rec != NULL;is_next_tbl_set = FALSE) */
		inst->hash[i] = tbl_rec;
	}			/* for(i = 0;i < MAB_MIB_ID_HASH_TBL_SIZE;i++) */

	oac_destroy_pcn_list(inst);
	oac_destroy_wb_pend_list(inst);
	oac_destroy_wbreq_hdl_list(inst);

	/* done with the handles */
	ncshm_give_hdl(destroy->i_oac_hdl);
	ncshm_destroy_hdl(NCS_SERVICE_ID_MAB, destroy->i_oac_hdl);

	/* done with locks */
	m_OAC_UNLK(&inst->lock);
	m_OAC_LK_DLT(&inst->lock);
	m_MMGR_FREE_OAC_TBL(inst);

	return status;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_destroy_pcn_list

  DESCRIPTION:       Destroy pcn_list

  RETURNS:           void

*****************************************************************************/
static void oac_destroy_pcn_list(OAC_TBL *inst)
{
	OAA_PCN_LIST *tmp = inst->pcn_list, *nxt_tmp = NULL;

	while (tmp != NULL) {
		nxt_tmp = tmp->next;
		m_MMGR_FREE_MAB_PCN_STRING(tmp->pcn);
		m_MMGR_FREE_MAB_OAA_PCN_LIST(tmp);
		tmp = nxt_tmp;
	}
	inst->pcn_list = NULL;
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_destroy_wb_pend_list

  DESCRIPTION:       Destroy wbreq_list

  RETURNS:           void

*****************************************************************************/
static void oac_destroy_wb_pend_list(OAC_TBL *inst)
{
	OAA_WBREQ_PEND_LIST *tmp = inst->wbreq_list, *nxt_tmp = NULL;

	while (tmp != NULL) {
		NCSOAC_PSS_TBL_LIST *ttmp = tmp->tbl_list, *nxt_ttmp = NULL;

		nxt_tmp = tmp->next;
		while (ttmp != NULL) {
			nxt_ttmp = ttmp->next;
			m_MMGR_FREE_NCSOAC_PSS_TBL_LIST(ttmp);
			ttmp = nxt_ttmp;
		}
		m_MMGR_FREE_MAB_PCN_STRING(tmp->pcn);
		m_MMGR_FREE_MAB_OAA_WBREQ_PEND_LIST(tmp);
		tmp = nxt_tmp;
	}
	inst->wbreq_list = NULL;
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_svc_get

  DESCRIPTION:       Fetch the value of an OAC object, which are:
                       NCSOAC_OBJID_LOG_VAL
                       NCSOAC_OBJID_LOG_ON_OFF
                       NCSOAC_OBJID_SUBSYS_ON

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/

uns32 oac_svc_get(NCSOAC_GET *get)
{
	OAC_TBL *inst;
	uns32 *pval = &get->o_obj_val;

	m_OAC_LK_INIT;

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(get->i_oac_hdl)) == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	m_OAC_LK(&inst->lock);

	switch (get->i_obj_id) {
	case NCSOAC_OBJID_LOG_VAL:
		*pval = inst->log_bits;
		break;
	case NCSOAC_OBJID_LOG_ON_OFF:
		*pval = inst->log_enbl;
		break;
	case NCSOAC_OBJID_SUBSYS_ON:
		*pval = inst->oac_enbl;
		break;

	default:
		ncshm_give_hdl(get->i_oac_hdl);
		m_LOG_MAB_API(MAB_API_OAC_SVC_GET);
		m_OAC_UNLK(&inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	ncshm_give_hdl(get->i_oac_hdl);
	m_LOG_MAB_API(MAB_API_OAC_SVC_GET);
	m_OAC_UNLK(&inst->lock);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_svc_set

  DESCRIPTION:       Set the value of a OAC object, which are:
                       NCSMAC_OBJID_LOG_VAL
                       NCSMAC_OBJID_LOG_ON_OFF
                       NCSMAC_OBJID_SUBSYS_ON

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/

uns32 oac_svc_set(NCSOAC_SET *set)
{
	OAC_TBL *inst;

	m_OAC_LK_INIT;

	if ((inst = (OAC_TBL *)m_OAC_VALIDATE_HDL(set->i_oac_hdl)) == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	m_OAC_LK(&inst->lock);

	switch (set->i_obj_id) {
	case NCSOAC_OBJID_LOG_VAL:
		inst->log_bits = set->i_obj_val;
		break;
	case NCSOAC_OBJID_LOG_ON_OFF:
		inst->log_enbl = set->i_obj_val;
		break;
	case NCSOAC_OBJID_SUBSYS_ON:
		inst->oac_enbl = set->i_obj_val;
		break;

	default:
		ncshm_give_hdl(set->i_oac_hdl);
		m_LOG_MAB_API(MAB_API_OAC_SVC_SET);
		m_OAC_UNLK(&inst->lock);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	ncshm_give_hdl(set->i_oac_hdl);
	m_LOG_MAB_API(MAB_API_OAC_SVC_SET);
	m_OAC_UNLK(&inst->lock);

	return NCSCC_RC_SUCCESS;
}

#endif   /* (NCS_MAB == 1) */
