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

  DESCRIPTION: This module contain all the encode routines require for encoding
  AVND data structures during checkpointing.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "ncsencdec_pub.h"

#include "avnd.h"
#include "avnd_ckpt_edu.h"

/* Declaration of async update functions */
static uint32_t avnd_encode_ckpt_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

static uint32_t avnd_encode_ckpt_hc_period(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_hc_max_dur(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

static uint32_t avnd_encode_ckpt_su_flag_change(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_err_esc_level(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_comp_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_comp_restart_max(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_restart_max(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_comp_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_err_esc_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_oper_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_pres_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

static uint32_t avnd_encode_ckpt_comp_flag_change(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_reg_hdl(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_reg_dest(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_oper_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_pres_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_term_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_csi_set_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_quies_cmplt_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_csi_rmv_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_pxied_inst_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_pxied_clean_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_err_info(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_def_recovery(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_pend_evt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_orph_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_node_id(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_type(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_mds_ctxt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_reg_resp_pending(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_inst_cmd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_term_cmd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_inst_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_term_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_inst_retry_max(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_inst_retry_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_exec_cmd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_cmd_exec_ctxt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_inst_cmd_ts(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_clc_reg_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_inst_code_rcvd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_proxy_proxied_add(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_proxy_proxied_del(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

static uint32_t avnd_encode_ckpt_su_si_rec_curr_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_si_rec_prv_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_si_rec_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_su_si_rec_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

static uint32_t avnd_encode_ckpt_comp_csi_act_comp_name(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_csi_trans_desc(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_csi_standby_rank(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_csi_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_csi_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

static uint32_t avnd_encode_ckpt_comp_hc_rec_status(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_hc_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

static uint32_t avnd_encode_ckpt_comp_cbk_rec_amf_hdl(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_cbk_rec_mds(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_cbk_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t avnd_encode_ckpt_comp_cbk_rec_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc);

/* Declaration of static cold sync encode functions */
static uint32_t avnd_entire_data_update(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, bool c_sync);
static uint32_t avnd_encode_cold_sync_rsp_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_su_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_comp_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t avnd_encode_cold_sync_rsp_async_updt_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);

/*
 * Function list for encoding the async data.
 * We will jump into this function using the reo_type received
 * in the encode argument.
 */
const AVND_ENCODE_CKPT_DATA_FUNC_PTR avnd_enc_ckpt_data_func_list[AVND_CKPT_MSG_MAX] = {
	avnd_encode_ckpt_hlt_config,
	avnd_encode_ckpt_su_config,
	avnd_encode_ckpt_comp_config,
	avnd_encode_ckpt_su_si_rec,
	avnd_encode_ckpt_siq_rec,
	avnd_encode_ckpt_csi_rec,
	avnd_encode_ckpt_comp_hlt_rec,
	avnd_encode_ckpt_comp_cbk_rec,

	/* Messages to update independent fields. */

	/* Health Check Config Async Update messages */
	avnd_encode_ckpt_hc_period,
	avnd_encode_ckpt_hc_max_dur,

	/* SU Async Update messages */
	avnd_encode_ckpt_su_flag_change,
	avnd_encode_ckpt_su_err_esc_level,
	avnd_encode_ckpt_su_comp_restart_prob,
	avnd_encode_ckpt_su_comp_restart_max,
	avnd_encode_ckpt_su_restart_prob,
	avnd_encode_ckpt_su_restart_max,
	avnd_encode_ckpt_su_comp_restart_cnt,
	avnd_encode_ckpt_su_restart_cnt,
	avnd_encode_ckpt_su_err_esc_tmr,
	avnd_encode_ckpt_su_oper_state,
	avnd_encode_ckpt_su_pres_state,

	/* Component Async Update messages */
	avnd_encode_ckpt_comp_flag_change,
	avnd_encode_ckpt_comp_reg_hdl,
	avnd_encode_ckpt_comp_reg_dest,
	avnd_encode_ckpt_comp_oper_state,
	avnd_encode_ckpt_comp_pres_state,
	avnd_encode_ckpt_comp_term_cbk_timeout,
	avnd_encode_ckpt_comp_csi_set_cbk_timeout,
	avnd_encode_ckpt_comp_quies_cmplt_cbk_timeout,
	avnd_encode_ckpt_comp_csi_rmv_cbk_timeout,
	avnd_encode_ckpt_comp_pxied_inst_cbk_timeout,
	avnd_encode_ckpt_comp_pxied_clean_cbk_timeout,
	avnd_encode_ckpt_comp_err_info,
	avnd_encode_ckpt_comp_def_recovery,
	avnd_encode_ckpt_comp_pend_evt,
	avnd_encode_ckpt_comp_orph_tmr,
	avnd_encode_ckpt_comp_node_id,
	avnd_encode_ckpt_comp_type,
	avnd_encode_ckpt_comp_mds_ctxt,
	avnd_encode_ckpt_comp_reg_resp_pending,
	avnd_encode_ckpt_comp_inst_cmd,
	avnd_encode_ckpt_comp_term_cmd,
	avnd_encode_ckpt_comp_inst_timeout,
	avnd_encode_ckpt_comp_term_timeout,
	avnd_encode_ckpt_comp_inst_retry_max,
	avnd_encode_ckpt_comp_inst_retry_cnt,
	avnd_encode_ckpt_comp_exec_cmd,
	avnd_encode_ckpt_comp_cmd_exec_ctxt,
	avnd_encode_ckpt_comp_inst_cmd_ts,
	avnd_encode_ckpt_comp_clc_reg_tmr,
	avnd_encode_ckpt_comp_inst_code_rcvd,
	avnd_encode_ckpt_comp_proxy_proxied_add,
	avnd_encode_ckpt_comp_proxy_proxied_del,

	/* SU SI RECORD Async Update messages */
	avnd_encode_ckpt_su_si_rec_curr_state,
	avnd_encode_ckpt_su_si_rec_prv_state,
	avnd_encode_ckpt_su_si_rec_curr_assign_state,
	avnd_encode_ckpt_su_si_rec_prv_assign_state,

	/* CSI REC Async Update messages */
	avnd_encode_ckpt_comp_csi_act_comp_name,
	avnd_encode_ckpt_comp_csi_trans_desc,
	avnd_encode_ckpt_comp_csi_standby_rank,
	avnd_encode_ckpt_comp_csi_curr_assign_state,
	avnd_encode_ckpt_comp_csi_prv_assign_state,

	/* Comp Health Check Async Update messages */
	avnd_encode_ckpt_comp_hc_rec_status,
	avnd_encode_ckpt_comp_hc_rec_tmr,

	/* Comp Callback Record Async Update messages */
	avnd_encode_ckpt_comp_cbk_rec_amf_hdl,
	avnd_encode_ckpt_comp_cbk_rec_mds,
	avnd_encode_ckpt_comp_cbk_rec_tmr,
	avnd_encode_ckpt_comp_cbk_rec_timeout
};

/*
 * Function list for encoding the cold sync response data
 * We will jump into this function using the reo_type received
 * in the cold sync repsonce argument.
 */
const AVND_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR avnd_enc_cold_sync_rsp_data_func_list[] = {
	avnd_encode_cold_sync_rsp_hlt_config,
	avnd_encode_cold_sync_rsp_su_config,
	avnd_encode_cold_sync_rsp_comp_config,
	avnd_encode_cold_sync_rsp_su_si_rec,
	avnd_encode_cold_sync_rsp_siq_rec,
	avnd_encode_cold_sync_rsp_csi_rec,
	avnd_encode_cold_sync_rsp_comp_hlt_rec,
	avnd_encode_cold_sync_rsp_comp_cbk_rec,
	avnd_encode_cold_sync_rsp_async_updt_cnt
};

/****************************************************************************\
 * Function: avnd_encode_data_sync_rsp
 *
 * Purpose:  Encode Data sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uint32_t avnd_encode_data_sync_rsp(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	return avnd_entire_data_update(cb, enc, false);
}

/****************************************************************************\
 * Function: avnd_entire_data_update
 *
 * Purpose:  Encode entire data to be sent during cold sync or warm sync.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *        c_sync - true - Called while in cold sync.
 *                 false - Called while in warm sync.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_entire_data_update(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, bool c_sync)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t num_of_obj = 0;
	uint8_t *encoded_cnt_loc;

	/* 
	 * Since at decode we need to find out how many objects of particular data
	 * type are being sent, encode that information at the begining of the message.
	 */
	encoded_cnt_loc = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint32_t));
	if (!encoded_cnt_loc) {
		/* Handle error */
	}
	ncs_enc_claim_space(&enc->io_uba, sizeof(uint32_t));

	/* 
	 * If reo_handle and reo_type is NULL then this the first time mbcsv is calling
	 * the cold sync response for the standby. So start from first data structure 
	 * which is CB. Next time onwards depending on the value of reo_type and reo_handle
	 * send the next data structures.
	 */
	status = avnd_enc_cold_sync_rsp_data_func_list[enc->io_reo_type] (cb, enc, &num_of_obj);

	/* Now encode the number of objects actually in the UBA. */

	if (encoded_cnt_loc != NULL) {
		ncs_encode_32bit(&encoded_cnt_loc, num_of_obj);
	}

	/*
	 * Check if we have reached to last message required to be sent in cold sync 
	 * response, if yes then send cold sync complete. Else ask MBCSv to call you 
	 * back with the next reo_type.
	 */
	if (AVND_COLD_SYNC_RSP_ASYNC_UPDT_CNT == enc->io_reo_type) {
		if (c_sync)
			enc->io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
		else
			enc->io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE;
	} else
		enc->io_reo_type++;

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_su_config
 *
 * Purpose:  Encode entire AVND_SU data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_su_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_SU *su;
	EDU_ERR ederror = 0;


	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)0);
	while (su != 0) {
		if (true == su->su_is_external) {
			status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						    EDP_OP_TYPE_ENC, su, &ederror, enc->i_peer_version);

			if (status != NCSCC_RC_SUCCESS) {
				/* Encode failed!!! */
				return NCSCC_RC_FAILURE;
			}

			(*num_of_obj)++;
		}		/* if(true == su->su_is_external) */
		su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)&su->name);
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_comp_config
 *
 * Purpose:  Encode entire AVND_COMP data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_comp_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_COMP *comp;
	EDU_ERR ederror = 0;


	/* NOTE    :    1. All external components are proxied components.
	   2. All Components in internode_avail_comp_db are proxy 
	   components(internode). But we need to send only those
	   which are proxy to external components.

	   We need to send internode component(proxy) first and after that external
	   component (proxied), bz while decoding, first we will add all proxy 
	   components in DB and then one by one proxied component. After decoding
	   proxied component, we can check comp->proxy_comp_name, if it NULL,
	   then comp will be in ORPH state and no proxy is attached to it. If 
	   comp->proxy_comp_name is not NULL, then it is name of proxy for the
	   proxied component and use avnd_comp_proxied_add to add it in proxy
	   component pxied_list. For adding proxied component to the pxied_list
	   of proxy component, we should have proxy component already in DB,
	   that's why we need to send proxy component first. */

	/* Encode internode components (proxy) first, but only for those, which are
	   proxy for external components. */
	comp = (AVND_COMP *)ncs_patricia_tree_getnext(&cb->internode_avail_comp_db, (uint8_t *)0);

	while (comp != 0) {
		if (m_AVND_PROXY_IS_FOR_EXT_COMP(comp)) {
			status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						    EDP_OP_TYPE_ENC, comp, &ederror, enc->i_peer_version);

			if (status != NCSCC_RC_SUCCESS) {
				/* Log Error */
				return NCSCC_RC_FAILURE;
			}

			(*num_of_obj)++;
		}
		comp = (AVND_COMP *)ncs_patricia_tree_getnext(&cb->internode_avail_comp_db, (uint8_t *)&comp->name);
	}
	/*
	 * Now send proxied components (external component).
	 */
	comp = (AVND_COMP *)ncs_patricia_tree_getnext(&cb->compdb, (uint8_t *)0);

	while (comp != 0) {
		if (true == comp->su->su_is_external) {
			status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						    EDP_OP_TYPE_ENC, comp, &ederror, enc->i_peer_version);

			if (status != NCSCC_RC_SUCCESS) {
				/* Log Error */
				return NCSCC_RC_FAILURE;
			}

			(*num_of_obj)++;
		}		/* if(true == comp->su->su_is_external) */
		comp = (AVND_COMP *)
		    ncs_patricia_tree_getnext(&cb->compdb, (uint8_t *)&comp->name);
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_su_si_rec
 *
 * Purpose:  Encode entire AVND_SU_SI_REC data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_SU *su;
	AVND_SU_SI_REC *rel;
	EDU_ERR ederror = 0;


	su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)0);
	while (su != 0) {
		if (true == su->su_is_external) {
			for (rel = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_FIRST(&su->si_list);
			     rel; rel = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_NEXT(&rel->su_dll_node)) {

				status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &enc->io_uba,
							    EDP_OP_TYPE_ENC, rel, &ederror, enc->i_peer_version);

				if (status != NCSCC_RC_SUCCESS) {
					/* Encode failed!!! */
					return NCSCC_RC_FAILURE;
				}

				(*num_of_obj)++;
			}
		}
		/* if(true == su->su_is_external) */
		su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)&su->name);
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_siq_rec
 *
 * Purpose:  Encode entire AVND_SU_SI_PARAM data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_SU *su = NULL;
	AVND_SU_SIQ_REC *siq;
	EDU_ERR ederror = 0;


	su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)0);
	while (su != 0) {
		if (true == su->su_is_external) {
			for (siq = (AVND_SU_SIQ_REC *)m_NCS_DBLIST_FIND_LAST(&su->siq);
			     siq; siq = (AVND_SU_SIQ_REC *)m_NCS_DBLIST_FIND_PREV(&siq->su_dll_node)) {

				status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_siq, &enc->io_uba,
							    EDP_OP_TYPE_ENC, &siq->info, &ederror, enc->i_peer_version);

				if (status != NCSCC_RC_SUCCESS) {
					/* Encode failed!!! */
					return NCSCC_RC_FAILURE;
				}

				(*num_of_obj)++;
			}
		}		/* if(true == su->su_is_external) */
		su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)&su->name);
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_csi_rec
 *
 * Purpose:  Encode entire AVND_COMP_CSI_REC data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_SU *su;
	AVND_SU_SI_REC *curr_su_si = NULL;
	AVND_COMP_CSI_REC *csi = NULL;
	EDU_ERR ederror = 0;


	su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)0);
	while (su != 0) {
		if (true == su->su_is_external) {
			for (curr_su_si = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_FIRST(&su->si_list);
			     curr_su_si;
			     curr_su_si = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_NEXT(&curr_su_si->su_dll_node)) {

				for (csi = (AVND_COMP_CSI_REC *)m_NCS_DBLIST_FIND_FIRST(&curr_su_si->csi_list);
				     csi; csi = (AVND_COMP_CSI_REC *)m_NCS_DBLIST_FIND_NEXT(&csi->si_dll_node)) {
					/* Before calling EDU, fill comp_name and si_name */
					csi->comp_name = csi->comp->name;
					csi->si_name = csi->si->name;
					csi->su_name = su->name;

					status =
					    m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
							       EDP_OP_TYPE_ENC, csi, &ederror, enc->i_peer_version);

					if (status != NCSCC_RC_SUCCESS) {
						/* Encode failed!!! */
						return NCSCC_RC_FAILURE;
					}

					(*num_of_obj)++;
				}
			}
		}		/* if(true == su->su_is_external) */
		su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uint8_t *)&su->name);
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_comp_hlt_rec
 *
 * Purpose:  Encode entire AVND_COMP_HC_REC data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_COMP *comp = NULL;
	AVND_COMP_HC_REC *comp_hc = NULL;
	EDU_ERR ederror = 0;


	/*
	 * Walk through the entire list and send the entire list data.
	 */
	comp = (AVND_COMP *)ncs_patricia_tree_getnext(&cb->compdb, (uint8_t *)0);

	while (comp != 0) {
		if (true == comp->su->su_is_external) {
			for (comp_hc = (AVND_COMP_HC_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->hc_list);
			     comp_hc; comp_hc = (AVND_COMP_HC_REC *)m_NCS_DBLIST_FIND_NEXT(&comp_hc->comp_dll_node)) {
				/* Before calling EDU, fill comp_name and si_name */
				comp_hc->comp_name = comp_hc->comp->name;

				status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc, &enc->io_uba,
							    EDP_OP_TYPE_ENC, comp_hc, &ederror, enc->i_peer_version);

				if (status != NCSCC_RC_SUCCESS) {
					/* Log Error */
					return NCSCC_RC_FAILURE;
				}

				(*num_of_obj)++;
			}
		}		/* if(true == comp->su->su_is_external) */
		comp = (AVND_COMP *)
		    ncs_patricia_tree_getnext(&cb->compdb, (uint8_t *)&comp->name);
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_comp_cbk_rec
 *
 * Purpose:  Encode entire AVND_COMP_CBK data.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_COMP *comp = NULL;
	AVND_COMP_CBK *comp_cbk = NULL;
	EDU_ERR ederror = 0;


	/*
	 * Walk through the entire list and send the entire list data.
	 */
	comp = (AVND_COMP *)ncs_patricia_tree_getnext(&cb->compdb, (uint8_t *)0);

	while (comp != 0) {
		if (true == comp->su->su_is_external) {
			for (comp_cbk = comp->cbk_list; comp_cbk; comp_cbk = comp_cbk->next) {
				/* Before calling EDU, fill comp_name */
				comp_cbk->comp_name = comp_cbk->comp->name;

				status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &enc->io_uba,
							    EDP_OP_TYPE_ENC, comp_cbk, &ederror, enc->i_peer_version);

				if (status != NCSCC_RC_SUCCESS) {
					/* Log Error */
					return NCSCC_RC_FAILURE;
				}

				(*num_of_obj)++;
			}
		}		/* if(true == comp->su->su_is_external) */
		comp = (AVND_COMP *)
		    ncs_patricia_tree_getnext(&cb->compdb, (uint8_t *)&comp->name);
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_hlt_config
 *
 * Purpose:  Encode entire AVND_HC data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_HC *hc_config;
	EDU_ERR ederror = 0;


	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	hc_config = (AVND_HC *)ncs_patricia_tree_getnext(&cb->hcdb, (uint8_t *)0);
	while (hc_config != 0) {
		if (true == hc_config->is_ext) {
			status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config, &enc->io_uba,
						    EDP_OP_TYPE_ENC, hc_config, &ederror, enc->i_peer_version);

			if (status != NCSCC_RC_SUCCESS) {
				/* Encode failed!!! */
				return NCSCC_RC_FAILURE;
			}

			(*num_of_obj)++;
		}		/* if(true == hc_config->is_ext) */
		hc_config = (AVND_HC *)ncs_patricia_tree_getnext(&cb->hcdb, (uint8_t *)&hc_config->key);
	}			/* while(hc_config != 0) */

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp
 *
 * Purpose:  Encode cold sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uint32_t avnd_encode_cold_sync_rsp(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{

	return avnd_entire_data_update(cb, enc, true);
}

/****************************************************************************\
 * Function: avnd_encode_warm_sync_rsp
 *
 * Purpose:  Encode Warm sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uint32_t avnd_encode_warm_sync_rsp(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;
	char logbuff[SA_MAX_NAME_LENGTH];

	memset(logbuff, '\0', SA_MAX_NAME_LENGTH);
	snprintf(logbuff, SA_MAX_NAME_LENGTH - 1,
		 "avnd_encode_warm_sync_rsp \n UPDATE CNTS AT ACTIVE(WarmSink) : hc=%d,su=%d,comp=%d,su_si=%d,siq=%d,csi=%d,\ncomp_hc=%d,comp_cbk=%d\n",
		 cb->avnd_async_updt_cnt.hlth_config_updt, cb->avnd_async_updt_cnt.su_updt,
		 cb->avnd_async_updt_cnt.comp_updt, cb->avnd_async_updt_cnt.su_si_updt,
		 cb->avnd_async_updt_cnt.siq_updt, cb->avnd_async_updt_cnt.csi_updt,
		 cb->avnd_async_updt_cnt.comp_hlth_rec_updt, cb->avnd_async_updt_cnt.comp_cbk_rec_updt);


	/*
	 * Encode and send latest async update counts. (In the same manner we sent
	 * in the last message of the cold sync response.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_async_updt_cnt,
				    &enc->io_uba, EDP_OP_TYPE_ENC, &cb->avnd_async_updt_cnt, &ederror,
				    enc->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_cold_sync_rsp_async_updt_cnt
 *
 * Purpose:  Send the latest async update count. This count will be used
 *           during warm sync for verifying the data at stnadby.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_cold_sync_rsp_async_updt_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Encode and send async update counts for all the data structures.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_async_updt_cnt,
				    &enc->io_uba, EDP_OP_TYPE_ENC, &cb->avnd_async_updt_cnt, &ederror,
				    enc->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_hlt_config
 *
 * Purpose:  Encode entire AVND_HC data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVND_HC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_HC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 2, 3);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_su_config
 *
 * Purpose:  Encode entire AVND_SU data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_config
 *
 * Purpose:  Encode entire AVND_COMP data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_config(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_si_rec
 *
 * Purpose:  Encode entire AVND_SU_SI_REC data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVND_SU_SI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_SU_SI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 2, 1, 6);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_siq_rec
 *
 * Purpose:  Encode entire AVND_SU_SIQ_REC data.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_siq, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVND_SU_SI_PARAM *)
					    &(((AVND_SU_SIQ_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)))->info),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_siq, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU_SI_PARAM *)
						&(((AVND_SU_SIQ_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)))->info),
						&ederror, enc->i_peer_version, 1, 4);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_csi_rec
 *
 * Purpose:  Encode entire AVND_COMP_CSI_REC data.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
					    EDP_OP_TYPE_ENC,
					    (AVND_COMP_CSI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
					    enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CSI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 4, 1, 9, 10, 11);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_hlt_rec
 *
 * Purpose:  Encode entire AVND_COMP_HC_REC data.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc, &enc->io_uba,
					    EDP_OP_TYPE_ENC,
					    (AVND_COMP_HC_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
					    enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_HC_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 4, 9);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_cbk_rec
 *
 * Purpose:  Encode entire AVND_COMP_CBK data.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVND_COMP_CBK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec,
						&enc->io_uba, EDP_OP_TYPE_ENC,
						(AVND_COMP_CBK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 6);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_hc_period
 *
 * Purpose:  Encode avnd health check period info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_hc_period(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_HC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 4, 1, 2, 3, 4);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_hc_max_dur
 *
 * Purpose:  Encode avnd health check max duration info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_hc_max_dur(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_HC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 4, 1, 2, 3, 5);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_flag_change
 *
 * Purpose:  Encode avnd SU flag info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_flag_change(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 2);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_err_esc_level
 *
 * Purpose:  Encode avnd SU Err Esc Level info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_err_esc_level(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 3);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_comp_restart_prob
 *
 * Purpose:  Encode avnd_su component restart probation info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_comp_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 4);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_comp_restart_max
 *
 * Purpose:  Encode avnd_su max component restart count.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_comp_restart_max(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 5);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_restart_prob
 *
 * Purpose:  Encode avnd_su su restart probation.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_restart_max
 *
 * Purpose:  Encode avnd_su max su restart count.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_restart_max(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 7);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_comp_restart_cnt
 *
 * Purpose:  Encode avnd_su comp restart count.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_comp_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 8);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_restart_cnt
 *
 * Purpose:  Encode avnd_su su restart count.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_err_esc_tmr
 *
 * Purpose:  Encode avnd_su error escalation timer info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_err_esc_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 3, 10);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_oper_state
 *
 * Purpose:  Encode avnd su operational state
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_oper_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 11);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_pres_state
 *
 * Purpose:  Encode avnd su presence state
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_pres_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 12);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_flag_change
 *
 * Purpose:  Encode avnd component flag change info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_flag_change(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 3);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_reg_hdl
 *
 * Purpose:  Encode avnd component registring process amf handle info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_reg_hdl(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 16);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_reg_dest
 *
 * Purpose:  Encode avnd registring process mds info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_reg_dest(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 17);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_oper_state
 *
 * Purpose:  Encode avnd component operational state.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_oper_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 18);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_pres_state
 *
 * Purpose:  Encode avnd component presence state.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_pres_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 19);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_term_cbk_timeout
 *
 * Purpose:  Encode avnd component termination callback timeout value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_term_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 20);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_csi_set_cbk_timeout
 *
 * Purpose:  Encode avnd component CSI set callback timeout value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_csi_set_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 21);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_quies_cmplt_cbk_timeout
 *
 * Purpose:  Encode avnd component quiescing complete callback timeout value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_quies_cmplt_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 22);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_csi_rmv_cbk_timeout
 *
 * Purpose:  Encode avnd component CSI remove callback timeout value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_csi_rmv_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 23);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_pxied_inst_cbk_timeout
 *
 * Purpose:  Encode avnd component proxied instantiation callback timeout value
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_pxied_inst_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 24);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_pxied_clean_cbk_timeout
 *
 * Purpose:  Encode avnd component proxied cleanup callback timeout value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_pxied_clean_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 25);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_err_info
 *
 * Purpose:  Encode avnd component error information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_err_info(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 5, 1, 26, 27, 28, 29);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_def_recovery
 *
 * Purpose:  Encode avnd component default recovery information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_def_recovery(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 27);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_pend_evt
 *
 * Purpose:  Encode avnd component pending event information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_pend_evt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 30);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_orph_tmr
 *
 * Purpose:  Encode avnd component orphan timer information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_orph_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 32);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_node_id
 *
 * Purpose:  Encode avnd component node id information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_node_id(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 33);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_type
 *
 * Purpose:  Encode avnd component type information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_type(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 31);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_mds_ctxt
 *
 * Purpose:  Encode avnd component mds context information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_mds_ctxt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 34, 35);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_reg_resp_pending
 *
 * Purpose:  Encode avnd component registration pending information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_reg_resp_pending(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 36);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_inst_cmd
 *
 * Purpose:  Encode avnd component instantiate command information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_inst_cmd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 6, 7);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_term_cmd
 *
 * Purpose:  Encode avnd component terminate command information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_term_cmd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 9, 10);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_inst_timeout
 *
 * Purpose:  Encode avnd component instantiation timeout value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_inst_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 8);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_term_timeout
 *
 * Purpose:  Encode avnd component termination timeout value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_term_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 11);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_inst_retry_max
 *
 * Purpose:  Encode avnd component instantiation retry max value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_inst_retry_max(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 12);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_inst_retry_cnt
 *
 * Purpose:  Encode avnd component instantiation retry count value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_inst_retry_cnt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 13);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_exec_cmd
 *
 * Purpose:  Encode avnd component execution command value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_exec_cmd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 37);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_cmd_exec_ctxt
 *
 * Purpose:  Encode avnd component execution context.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_cmd_exec_ctxt(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 38);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_inst_cmd_ts
 *
 * Purpose:  Encode avnd component instantiation time stamp value.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_inst_cmd_ts(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 14);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_clc_reg_tmr
 *
 * Purpose:  Encode avnd component registration timer inforamtion.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_clc_reg_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 39, 40);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_inst_code_rcvd
 *
 * Purpose:  Encode avnd component instantiation received code inforamtion.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_inst_code_rcvd(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 15);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_proxy_proxied_add
 *
 * Purpose:  Encode avnd component proxy-proxied add information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_proxy_proxied_add(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 31, 41);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_encode_ckpt_comp_proxy_proxied_del
 *
 * Purpose:  Encode avnd component proxy-proxied delete information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_proxy_proxied_del(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVND_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 31, 41);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_si_rec_curr_state
 *
 * Purpose:  Encode avnd SU_SI_REC current state information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_si_rec_curr_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_SU_SI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 2, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_si_rec_prv_state
 *
 * Purpose:  Encode avnd SU_SI_REC previous state information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_si_rec_prv_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_SU_SI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 3, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_si_rec_curr_assign_state
 *
 * Purpose:  Encode avnd SU_SI_REC current assign state information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_si_rec_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_SU_SI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 4, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_su_si_rec_prv_assign_state
 *
 * Purpose:  Encode avnd SU_SI_REC previous assign state information.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_su_si_rec_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_SU_SI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 5, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_csi_act_comp_name
 *
 * Purpose:  Encode AVND_COMP_CSI_REC active component name info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_csi_act_comp_name(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CSI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 3, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_csi_trans_desc
 *
 * Purpose:  Encode AVND_COMP_CSI_REC transition descriptor.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_csi_trans_desc(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CSI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 4, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_csi_standby_rank
 *
 * Purpose:  Encode AVND_COMP_CSI_REC Standby Rank.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_csi_standby_rank(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CSI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 5, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_csi_curr_assign_state
 *
 * Purpose:  Encode AVND_COMP_CSI_REC current assign state.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_csi_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CSI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 7, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_csi_prv_assign_state
 *
 * Purpose:  Encode AVND_COMP_CSI_REC previous assign state.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_csi_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CSI_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 8, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_hc_rec_status
 *
 * Purpose:  Encode AVND_COMP_HC_REC status info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_hc_rec_status(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_HC_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 4, 1, 4, 8, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_hc_rec_tmr
 *
 * Purpose:  Encode AVND_COMP_HC_REC tmr info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_hc_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_HC_REC *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 4, 1, 4, 9, 10);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_cbk_rec_amf_hdl
 *
 * Purpose:  Encode AVND_COMP_CBK amf handle info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_cbk_rec_amf_hdl(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CBK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 5, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_cbk_rec_mds
 *
 * Purpose:  Encode AVND_COMP_CBK mds info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_cbk_rec_mds(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CBK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 3, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_cbk_rec_tmr
 *
 * Purpose:  Encode AVND_COMP_CBK timer info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_cbk_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CBK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 6, 7);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function:  avnd_encode_ckpt_comp_cbk_rec_timeout
 *
 * Purpose:  Encode AVND_COMP_CBK timeout info.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_encode_ckpt_comp_cbk_rec_timeout(AVND_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;


	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVND_COMP_CBK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 3, 1, 4, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
		}
	} else {
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}

	return status;
}
