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

  DESCRIPTION: This module contain all the decode routines require for decoding
  AVND data structures during checkpointing. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <ncsencdec_pub.h>
#include "avnd.h"
#include "avnd_ckpt_edu.h"

/* Declaration of async update functions */
static uint32_t avnd_decode_ckpt_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

static uint32_t avnd_decode_ckpt_hc_period(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_hc_max_dur(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

static uint32_t avnd_decode_ckpt_su_flag_change(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_err_esc_level(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_comp_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_comp_restart_max(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_restart_max(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_comp_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_err_esc_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_oper_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_pres_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

static uint32_t avnd_decode_ckpt_comp_flag_change(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_reg_hdl(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_reg_dest(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_oper_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_pres_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_term_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_csi_set_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_quies_cmplt_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_csi_rmv_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_pxied_inst_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_pxied_clean_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_err_info(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_def_recovery(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_pend_evt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_orph_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_node_id(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_type(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_mds_ctxt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_reg_resp_pending(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_inst_cmd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_term_cmd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_inst_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_term_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_inst_retry_max(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_inst_retry_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_exec_cmd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_cmd_exec_ctxt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_inst_cmd_ts(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_clc_reg_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_inst_code_rcvd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_proxy_proxied_add(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_proxy_proxied_del(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

static uint32_t avnd_decode_ckpt_su_si_rec_curr_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_si_rec_prv_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_si_rec_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_su_si_rec_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

static uint32_t avnd_decode_ckpt_comp_csi_act_comp_name(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_csi_trans_desc(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_csi_standby_rank(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_csi_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_csi_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

static uint32_t avnd_decode_ckpt_comp_hc_rec_status(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_hc_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

static uint32_t avnd_decode_ckpt_comp_cbk_rec_amf_hdl(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_cbk_rec_mds(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_cbk_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t avnd_decode_ckpt_comp_cbk_rec_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec);

/* Declaration of static cold sync decode functions */
static uint32_t avnd_decode_cold_sync_rsp_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t avnd_decode_cold_sync_rsp_su_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t avnd_decode_cold_sync_rsp_comp_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t avnd_decode_cold_sync_rsp_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t avnd_decode_cold_sync_rsp_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t avnd_decode_cold_sync_rsp_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t avnd_decode_cold_sync_rsp_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t avnd_decode_cold_sync_rsp_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);

static uint32_t avnd_decode_cold_sync_rsp_async_updt_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);

/*
 * Function list for decoding the async data.
 * We will jump into this function using the reo_type received 
 * in the decode argument.
 */

extern const AVND_DECODE_CKPT_DATA_FUNC_PTR avnd_dec_ckpt_data_func_list[AVND_CKPT_MSG_MAX] = {
	avnd_decode_ckpt_hlt_config,
	avnd_decode_ckpt_su_config,
	avnd_decode_ckpt_comp_config,
	avnd_decode_ckpt_su_si_rec,
	avnd_decode_ckpt_siq_rec,
	avnd_decode_ckpt_csi_rec,
	avnd_decode_ckpt_comp_hlt_rec,
	avnd_decode_ckpt_comp_cbk_rec,

	/* Messages to update independent fields. */

	/* Health Check Config Async Update messages */
	avnd_decode_ckpt_hc_period,
	avnd_decode_ckpt_hc_max_dur,

	/* SU Async Update messages */
	avnd_decode_ckpt_su_flag_change,
	avnd_decode_ckpt_su_err_esc_level,
	avnd_decode_ckpt_su_comp_restart_prob,
	avnd_decode_ckpt_su_comp_restart_max,
	avnd_decode_ckpt_su_restart_prob,
	avnd_decode_ckpt_su_restart_max,
	avnd_decode_ckpt_su_comp_restart_cnt,
	avnd_decode_ckpt_su_restart_cnt,
	avnd_decode_ckpt_su_err_esc_tmr,
	avnd_decode_ckpt_su_oper_state,
	avnd_decode_ckpt_su_pres_state,

	/* Component Async Update messages */
	avnd_decode_ckpt_comp_flag_change,
	avnd_decode_ckpt_comp_reg_hdl,
	avnd_decode_ckpt_comp_reg_dest,
	avnd_decode_ckpt_comp_oper_state,
	avnd_decode_ckpt_comp_pres_state,
	avnd_decode_ckpt_comp_term_cbk_timeout,
	avnd_decode_ckpt_comp_csi_set_cbk_timeout,
	avnd_decode_ckpt_comp_quies_cmplt_cbk_timeout,
	avnd_decode_ckpt_comp_csi_rmv_cbk_timeout,
	avnd_decode_ckpt_comp_pxied_inst_cbk_timeout,
	avnd_decode_ckpt_comp_pxied_clean_cbk_timeout,
	avnd_decode_ckpt_comp_err_info,
	avnd_decode_ckpt_comp_def_recovery,
	avnd_decode_ckpt_comp_pend_evt,
	avnd_decode_ckpt_comp_orph_tmr,
	avnd_decode_ckpt_comp_node_id,
	avnd_decode_ckpt_comp_type,
	avnd_decode_ckpt_comp_mds_ctxt,
	avnd_decode_ckpt_comp_reg_resp_pending,
	avnd_decode_ckpt_comp_inst_cmd,
	avnd_decode_ckpt_comp_term_cmd,
	avnd_decode_ckpt_comp_inst_timeout,
	avnd_decode_ckpt_comp_term_timeout,
	avnd_decode_ckpt_comp_inst_retry_max,
	avnd_decode_ckpt_comp_inst_retry_cnt,
	avnd_decode_ckpt_comp_exec_cmd,
	avnd_decode_ckpt_comp_cmd_exec_ctxt,
	avnd_decode_ckpt_comp_inst_cmd_ts,
	avnd_decode_ckpt_comp_clc_reg_tmr,
	avnd_decode_ckpt_comp_inst_code_rcvd,
	avnd_decode_ckpt_comp_proxy_proxied_add,
	avnd_decode_ckpt_comp_proxy_proxied_del,

	/* SU SI RECORD Async Update messages */
	avnd_decode_ckpt_su_si_rec_curr_state,
	avnd_decode_ckpt_su_si_rec_prv_state,
	avnd_decode_ckpt_su_si_rec_curr_assign_state,
	avnd_decode_ckpt_su_si_rec_prv_assign_state,

	/* CSI REC Async Update messages */
	avnd_decode_ckpt_comp_csi_act_comp_name,
	avnd_decode_ckpt_comp_csi_trans_desc,
	avnd_decode_ckpt_comp_csi_standby_rank,
	avnd_decode_ckpt_comp_csi_curr_assign_state,
	avnd_decode_ckpt_comp_csi_prv_assign_state,

	/* Comp Health Check Async Update messages */
	avnd_decode_ckpt_comp_hc_rec_status,
	avnd_decode_ckpt_comp_hc_rec_tmr,

	/* Comp Callback Record Async Update messages */
	avnd_decode_ckpt_comp_cbk_rec_amf_hdl,
	avnd_decode_ckpt_comp_cbk_rec_mds,
	avnd_decode_ckpt_comp_cbk_rec_tmr,
	avnd_decode_ckpt_comp_cbk_rec_timeout
};

/*
 * Function list for decoding the cold sync response data
 * We will jump into this function using the reo_type received 
 * in the cold sync repsonce argument.
 */
const AVND_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR avnd_dec_cold_sync_rsp_data_func_list[] = {
	avnd_decode_cold_sync_rsp_hlt_config,
	avnd_decode_cold_sync_rsp_su_config,
	avnd_decode_cold_sync_rsp_comp_config,
	avnd_decode_cold_sync_rsp_su_si_rec,
	avnd_decode_cold_sync_rsp_siq_rec,
	avnd_decode_cold_sync_rsp_csi_rec,
	avnd_decode_cold_sync_rsp_comp_hlt_rec,
	avnd_decode_cold_sync_rsp_comp_cbk_rec,
	avnd_decode_cold_sync_rsp_async_updt_cnt
};

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp
 *
 * Purpose:  Decode cold sync response message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uint32_t avnd_decode_cold_sync_rsp(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t num_of_obj;
	uint8_t *ptr;


	/*
	 * Since at decode we need to find out how many objects of particular data
	 * type are sent, decode that information at the begining of the message.
	 */
	ptr = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&num_of_obj, sizeof(uint32_t));
	num_of_obj = ncs_decode_32bit(&ptr);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));

	/*
	 * Decode data received in the message.
	 */
	dec->i_action = NCS_MBCSV_ACT_ADD;
	status = avnd_dec_cold_sync_rsp_data_func_list[dec->i_reo_type] (cb, dec, num_of_obj);

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_data_sync_rsp
 *
 * Purpose:  Decode Data sync response message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uint32_t avnd_decode_data_sync_rsp(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t num_of_obj;
	uint8_t *ptr;


	/*
	 * Since at decode we need to find out how many objects of particular data
	 * type are sent, decode that information at the begining of the message.
	 */
	ptr = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&num_of_obj, sizeof(uint32_t));
	num_of_obj = ncs_decode_32bit(&ptr);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));

	/*
	 * Decode data received in the message.
	 */
	dec->i_action = NCS_MBCSV_ACT_ADD;
	status = avnd_dec_cold_sync_rsp_data_func_list[dec->i_reo_type] (cb, dec, num_of_obj);

	return status;

}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_su_config
 *
 * Purpose:  Decode entire AVND_SU data..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_su_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_ptr = NULL;
	AVND_SU dec_su;


	su_ptr = &dec_su;

	for (count = 0; count < num_of_obj; count++) {
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU **)&su_ptr, &ederror,
					    dec->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_su_data(cb, su_ptr, dec->i_action);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}

	}			/* for loop */

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_su_si_rec
 *
 * Purpose:  Decode AVND_SU_SI_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU_SI_REC *su_si_ckpt;
	AVND_SU_SI_REC dec_su_si_ckpt;

	su_si_ckpt = &dec_su_si_ckpt;

	for (count = 0; count < num_of_obj; count++) {
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU_SI_REC **)&su_si_ckpt,
					    &ederror, dec->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_su_si_rec(cb, su_si_ckpt, dec->i_action);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}			/* for loop */

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_siq_rec
 *
 * Purpose:  Decode AVND_SU_SIQ_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU_SIQ_REC *su_siq_ckpt;
	AVND_SU_SIQ_REC dec_su_siq_ckpt;
	AVND_SU_SI_PARAM *su_si_param_ptr = NULL;

	su_si_param_ptr = &(dec_su_siq_ckpt.info);
	su_siq_ckpt = &dec_su_siq_ckpt;

	for (count = 0; count < num_of_obj; count++) {

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_siq,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU_SI_PARAM **)&su_si_param_ptr,
					    &ederror, dec->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_su_siq_rec(cb, su_siq_ckpt, dec->i_action);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}			/* for loop */

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_comp_config
 *
 * Purpose:  Decode entire AVND_COMP data..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_comp_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_ptr;
	AVND_COMP dec_comp;

	comp_ptr = &dec_comp;

	for (count = 0; count < num_of_obj; count++) {
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_ptr, &ederror,
					    dec->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_comp_data(cb, comp_ptr, dec->i_action);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_csi_rec
 *
 * Purpose:  Decode entire AVND_COMP_CSI_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CSI_REC *csi_ptr_dec;
	AVND_COMP_CSI_REC dec_csi;


	csi_ptr_dec = &dec_csi;

	for (count = 0; count < num_of_obj; count++) {
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_ptr_dec, &ederror,
					    dec->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_csi_data(cb, csi_ptr_dec, dec->i_action);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}

	}			/* for loop */

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_comp_hlt_rec
 *
 * Purpose:  Decode entire AVND_COMP_HC_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_HC_REC *hlt_ptr;
	AVND_COMP_HC_REC dec_hlt;

	hlt_ptr = &dec_hlt;

	for (count = 0; count < num_of_obj; count++) {
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_HC_REC **)&hlt_ptr, &ederror,
					    dec->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_comp_hlt_rec(cb, hlt_ptr, dec->i_action);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}			/* for loop */

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_comp_cbk_rec
 *
 * Purpose:  Decode entire AVND_COMP_CBK data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CBK *cbk_ptr;
	AVND_COMP_CBK cbk_info;


	cbk_ptr = &cbk_info;

	for (count = 0; count < num_of_obj; count++) {
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_CBK **)&cbk_ptr, &ederror,
					    dec->i_peer_version);
		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_comp_cbk_rec(cb, cbk_ptr, dec->i_action);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}			/* for loop */

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_hlt_config
 *
 * Purpose:  Decode entire AVND_HC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_HC *hlt_ptr;
	AVND_HC dec_hlt;
	uint32_t count = 0;


	hlt_ptr = &dec_hlt;

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	for (count = 0; count < num_of_obj; count++) {
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_HC **)&hlt_ptr, &ederror,
					    dec->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			return status;
		}

		status = avnd_ckpt_add_rmv_updt_hlt_data(cb, hlt_ptr, dec->i_action);
		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}

	}

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_warm_sync_rsp
 *
 * Purpose:  Decode Warm sync response message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uint32_t avnd_decode_warm_sync_rsp(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_ASYNC_UPDT_CNT *updt_cnt;
	AVND_ASYNC_UPDT_CNT dec_updt_cnt;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	char logbuff[SA_MAX_NAME_LENGTH];


	updt_cnt = &dec_updt_cnt;

	/*
	 * Decode latest async update counts. (In the same manner we received
	 * in the last message of the cold sync response.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_async_updt_cnt,
				    &dec->i_uba, EDP_OP_TYPE_DEC, &updt_cnt, &ederror, dec->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	memset(logbuff, '\0', SA_MAX_NAME_LENGTH);
	snprintf(logbuff, SA_MAX_NAME_LENGTH - 1,
		 "\n UPDATE CNTS RCVD(Active -> Standby): hc=%d,su=%d,comp=%d,su_si=%d,siq=%d,csi=%d,\ncomp_hc=%d,comp_cbk=%d\n",
		 updt_cnt->hlth_config_updt, updt_cnt->su_updt, updt_cnt->comp_updt, updt_cnt->su_si_updt,
		 updt_cnt->siq_updt, updt_cnt->csi_updt, updt_cnt->comp_hlth_rec_updt, updt_cnt->comp_cbk_rec_updt);


	memset(logbuff, '\0', SA_MAX_NAME_LENGTH);
	snprintf(logbuff, SA_MAX_NAME_LENGTH - 1,
		 "\n UPDATE CNTS AT STANDBY: hc=%d,su=%d,comp=%d,su_si=%d,siq=%d,csi=%d,\ncomp_hc=%d,comp_cbk=%d\n",
		 cb->avnd_async_updt_cnt.hlth_config_updt, cb->avnd_async_updt_cnt.su_updt,
		 cb->avnd_async_updt_cnt.comp_updt, cb->avnd_async_updt_cnt.su_si_updt,
		 cb->avnd_async_updt_cnt.siq_updt, cb->avnd_async_updt_cnt.csi_updt,
		 cb->avnd_async_updt_cnt.comp_hlth_rec_updt, cb->avnd_async_updt_cnt.comp_cbk_rec_updt);


	/*
	 * Compare the update counts of the Standby with Active.
	 * if they matches return success. If it fails then
	 * send a data request.
	 */
	if (0 != memcmp(updt_cnt, &cb->avnd_async_updt_cnt, sizeof(AVND_ASYNC_UPDT_CNT))) {
		/* Log this as a Notify kind of message....this means we
		 * have mismatch in warm sync and we need to resync.
		 * Stanby AVND is unavailable for failure */

		cb->stby_sync_state = AVND_STBY_OUT_OF_SYNC;

		/*
		 * Remove All data structures from the standby. We will get them again
		 * during our data responce sync-up.
		 */
		avnd_ext_comp_data_clean_up(cb, false);

		/*
		 * Now send data request, which will sync Standby with Active.
		 */
		if (NCSCC_RC_SUCCESS != avnd_send_data_req(cb)) {
			/* Log error */
		}

		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_data_req
 *
 * Purpose:  Decode Data request message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uint32_t avnd_decode_data_req(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	/*
	 * Don't decode anything...just return success.
	 */
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avnd_decode_cold_sync_rsp_async_updt_cnt
 *
 * Purpose:  Send the latest async update count. This count will be used
 *           during warm sync for verifying the data at stnadby.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_cold_sync_rsp_async_updt_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_ASYNC_UPDT_CNT *updt_cnt;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	updt_cnt = &cb->avnd_async_updt_cnt;
	/*
	 * Decode and send async update counts for all the data structures.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_async_updt_cnt,
				    &dec->i_uba, EDP_OP_TYPE_DEC, &updt_cnt, &ederror, dec->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
	}

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_hlt_config 
 *
 * Purpose:  Decode entire AVND_HC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_hlt_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_HC *hlt_config_ptr;
	AVND_HC dec_hlt_config;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	hlt_config_ptr = &dec_hlt_config;

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_HC **)&hlt_config_ptr, &ederror,
					    dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_HC **)&hlt_config_ptr, &ederror, 3, 1, 2, 3);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_hlt_data(cb, hlt_config_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.hlth_config_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_config 
 *
 * Purpose:  Decode entire AVND_SU data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_SU *su_ptr;
	AVND_SU dec_su;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	su_ptr = &dec_su;

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU **)&su_ptr, &ederror,
					    dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU **)&su_ptr, &ederror, 1, 1);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_su_data(cb, su_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_config 
 *
 * Purpose:  Decode entire AVND_COMP data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_config(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_COMP *comp_ptr;
	AVND_COMP dec_comp;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	comp_ptr = &dec_comp;

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_ptr, &ederror,
					    dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_ptr, &ederror, 1, 1);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_comp_data(cb, comp_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_si_rec 
 *
 * Purpose:  Decode entire AVND_SU_SI_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_si_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_SU_SI_REC *su_si_ptr;
	AVND_SU_SI_REC dec_su_si;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	su_si_ptr = &dec_su_si;

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU_SI_REC **)&su_si_ptr, &ederror,
					    dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU_SI_REC **)&su_si_ptr, &ederror, 2, 1, 6);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_su_si_rec(cb, su_si_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.su_si_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_siq_rec 
 *
 * Purpose:  Decode entire AVND_SU_SIQ_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_siq_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_SU_SIQ_REC *siq_ptr;
	AVND_SU_SIQ_REC dec_siq;
	AVND_SU_SI_PARAM *su_si_param_ptr = NULL;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	siq_ptr = &dec_siq;
	su_si_param_ptr = &(dec_siq.info);

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_siq,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU_SI_PARAM **)&su_si_param_ptr,
					    &ederror, dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_siq,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_SU_SI_PARAM **)&su_si_param_ptr, &ederror, 1,
				      4);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_su_siq_rec(cb, siq_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.siq_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_csi_rec 
 *
 * Purpose:  Decode entire AVND_COMP_CSI_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_csi_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_COMP_CSI_REC *csi_ptr;
	AVND_COMP_CSI_REC dec_csi;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	csi_ptr = &dec_csi;

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_ptr, &ederror,
					    dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_ptr, &ederror, 4, 1, 9,
				      10, 11);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_csi_data(cb, csi_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.csi_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_hlt_rec 
 *
 * Purpose:  Decode entire AVND_COMP_HC_REC data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_hlt_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_COMP_HC_REC *comp_hc_ptr;
	AVND_COMP_HC_REC dec_comp_hc;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	comp_hc_ptr = &dec_comp_hc;

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_HC_REC **)&comp_hc_ptr, &ederror,
					    dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_HC_REC **)&comp_hc_ptr, &ederror, 3, 1,
				      4, 9);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_comp_hlt_rec(cb, comp_hc_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.comp_hlth_rec_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_cbk_rec 
 *
 * Purpose:  Decode entire AVND_COMP_CBK data.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_cbk_rec(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	AVND_COMP_CBK *comp_cbk_ptr;
	AVND_COMP_CBK dec_comp_cbk;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);


	comp_cbk_ptr = &dec_comp_cbk;

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is add then create new element, if it is update
	 * request then just update data structure, and if it is remove then
	 * remove entry from the list.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec,
					    &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_CBK **)&comp_cbk_ptr, &ederror,
					    dec->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec,
				      &dec->i_uba, EDP_OP_TYPE_DEC, (AVND_COMP_CBK **)&comp_cbk_ptr, &ederror, 2, 1, 6);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	status = avnd_ckpt_add_rmv_updt_comp_cbk_rec(cb, comp_cbk_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->avnd_async_updt_cnt.comp_cbk_rec_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_hc_period
 *
 * Purpose:  Decode avnd health check period.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_hc_period(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_HC *hc_ptr = NULL;
	AVND_HC dec_hc;
	AVND_HC *hc_rec = NULL;


	hc_ptr = &dec_hc;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_HC **)&hc_ptr, &ederror, 4, 1, 2, 3, 4);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	hc_rec = avnd_hcdb_rec_get(cb, &hc_ptr->key);
	if (NULL == hc_rec) {
		return NCSCC_RC_FAILURE;
	}

	hc_rec->period = hc_ptr->period;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.hlth_config_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_hc_max_dur
 *
 * Purpose:  Decode avnd max health check duration.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_hc_max_dur(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_HC *hc_ptr = NULL;
	AVND_HC dec_hc;
	AVND_HC *hc_rec = NULL;


	hc_ptr = &dec_hc;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_hlt_config, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_HC **)&hc_ptr, &ederror, 4, 1, 2, 3, 5);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	hc_rec = avnd_hcdb_rec_get(cb, &hc_ptr->key);
	if (NULL == hc_rec) {
		return NCSCC_RC_FAILURE;
	}

	hc_rec->max_dur = hc_ptr->max_dur;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.hlth_config_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_flag_change
 *
 * Purpose:  Decode avnd SU flag info.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_flag_change(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 2);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->flag = su_dec_ptr->flag;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_err_esc_level
 *
 * Purpose:  Decode avnd SU Err Esc Level info.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_err_esc_level(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 3);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->su_err_esc_level = su_dec_ptr->su_err_esc_level;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_comp_restart_prob
 *
 * Purpose:  Decode avnd_su component restart probation info.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_comp_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 4);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->comp_restart_prob = su_dec_ptr->comp_restart_prob;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_comp_restart_max
 *
 * Purpose:  Decode avnd_su max component restart count.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_comp_restart_max(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 5);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->comp_restart_max = su_dec_ptr->comp_restart_max;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_restart_prob
 *
 * Purpose:  Decode avnd_su su restart probation.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_restart_prob(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->su_restart_prob = su_dec_ptr->su_restart_prob;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_restart_max
 *
 * Purpose:  Decode avnd_su max su restart count.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_restart_max(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 7);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->su_restart_max = su_dec_ptr->su_restart_max;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_comp_restart_cnt
 *
 * Purpose:  Decode avnd_su comp restart count.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_comp_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 8);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->comp_restart_cnt = su_dec_ptr->comp_restart_cnt;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_restart_cnt
 *
 * Purpose:  Decode avnd_su su restart count.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_restart_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 9);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->su_restart_cnt = su_dec_ptr->su_restart_cnt;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_err_esc_tmr
 *
 * Purpose:  Decode avnd_su su error escalation timer info.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_err_esc_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 3, 1, 3, 10);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->su_err_esc_level = su_dec_ptr->su_err_esc_level;
	su_rec->su_err_esc_tmr.is_active = su_dec_ptr->su_err_esc_tmr.is_active;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_oper_state
 *
 * Purpose:  Decode avnd su operational state.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_oper_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 11);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->oper = su_dec_ptr->oper;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_pres_state
 *
 * Purpose:  Decode avnd su presence state.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_pres_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU *su_dec_ptr = NULL;
	AVND_SU dec_su;
	AVND_SU *su_rec = NULL;


	su_dec_ptr = &dec_su;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU **)&su_dec_ptr, &ederror, 2, 1, 12);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_rec = m_AVND_SUDB_REC_GET(cb->sudb, su_dec_ptr->name);
	if (NULL == su_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_rec->pres = su_dec_ptr->pres;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_flag_change
 *
 * Purpose:  Decode avnd component flag change info.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_flag_change(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 3);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->flag = comp_dec_ptr->flag;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_reg_hdl
 *
 * Purpose:  Decode avnd component registring process amf handle info.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_reg_hdl(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 16);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->reg_hdl = comp_dec_ptr->reg_hdl;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_reg_dest
 *
 * Purpose:  Decode avnd component registring process mds info.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_reg_dest(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 17);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->reg_dest = comp_dec_ptr->reg_dest;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_oper_state
 *
 * Purpose:  Decode avnd component operational state.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_oper_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 18);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->oper = comp_dec_ptr->oper;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_pres_state
 *
 * Purpose:  Decode avnd component presence state.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_pres_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 19);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->pres = comp_dec_ptr->pres;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_term_cbk_timeout
 *
 * Purpose:  Decode avnd component termination callback timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_term_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 20);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->term_cbk_timeout = comp_dec_ptr->term_cbk_timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_csi_set_cbk_timeout
 *
 * Purpose:  Decode avnd component CSI set callback timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_csi_set_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 21);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->csi_set_cbk_timeout = comp_dec_ptr->csi_set_cbk_timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_quies_cmplt_cbk_timeout
 *
 * Purpose:  Decode avnd component quiescing complete callback timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_quies_cmplt_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 22);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->quies_complete_cbk_timeout = comp_dec_ptr->quies_complete_cbk_timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_csi_rmv_cbk_timeout
 *
 * Purpose:  Decode avnd component CSI remove callback timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_csi_rmv_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 23);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->csi_rmv_cbk_timeout = comp_dec_ptr->csi_rmv_cbk_timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_pxied_inst_cbk_timeout
 *
 * Purpose:  Decode avnd component proxied instantiation callback timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_pxied_inst_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 24);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->pxied_inst_cbk_timeout = comp_dec_ptr->pxied_inst_cbk_timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_pxied_clean_cbk_timeout
 *
 * Purpose:  Decode avnd component proxied cleanup callback timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_pxied_clean_cbk_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 25);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->pxied_clean_cbk_timeout = comp_dec_ptr->pxied_clean_cbk_timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_err_info
 *
 * Purpose:  Decode avnd component error information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_err_info(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 5, 1, 26, 27, 28, 29);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->err_info.src = comp_dec_ptr->err_info.src;
	comp_rec->err_info.def_rec = comp_dec_ptr->err_info.def_rec;
	comp_rec->err_info.detect_time = comp_dec_ptr->err_info.detect_time;
	comp_rec->err_info.restart_cnt = comp_dec_ptr->err_info.restart_cnt;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_def_recovery
 *
 * Purpose:  Decode avnd component default recovery information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_def_recovery(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 27);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->err_info.def_rec = comp_dec_ptr->err_info.def_rec;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_pend_evt
 *
 * Purpose:  Decode avnd component pending event information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_pend_evt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 30);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->pend_evt = comp_dec_ptr->pend_evt;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_orph_tmr
 *
 * Purpose:  Decode avnd component orphan timer information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_orph_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 32);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->orph_tmr.is_active = comp_dec_ptr->orph_tmr.is_active;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_node_id
 *
 * Purpose:  Decode avnd component node id information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_node_id(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 33);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->node_id = comp_dec_ptr->node_id;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_type
 *
 * Purpose:  Decode avnd component type information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_type(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 31);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->comp_type = comp_dec_ptr->comp_type;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_mds_ctxt
 *
 * Purpose:  Decode avnd component mds context information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_mds_ctxt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 3, 1, 34, 35);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->mds_ctxt.length = comp_dec_ptr->mds_ctxt.length;
	memcpy(&comp_rec->mds_ctxt.data, &comp_dec_ptr->mds_ctxt.data, sizeof(MDS_SYNC_SND_CTXT_LEN_MAX));

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_reg_resp_pending
 *
 * Purpose:  Decode avnd component registration pending information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_reg_resp_pending(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 36);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->reg_resp_pending = comp_dec_ptr->reg_resp_pending;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_inst_cmd
 *
 * Purpose:  Decode avnd component instantiate command information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_inst_cmd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 3, 1, 6, 7);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len =
	    comp_dec_ptr->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len;

	memcpy(comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1]
	       .cmd,
	       comp_dec_ptr->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd,
	       comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len);

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_term_cmd
 *
 * Purpose:  Decode avnd component terminate command information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_term_cmd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 3, 1, 9, 10);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len =
	    comp_dec_ptr->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len;

	memcpy(comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1]
	       .cmd,
	       comp_dec_ptr->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd,
	       comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len);

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_inst_timeout
 *
 * Purpose:  Decode avnd component instantiation timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_inst_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 8);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].timeout =
	    comp_dec_ptr->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_term_timeout
 *
 * Purpose:  Decode avnd component termination timeout value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_term_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 11);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].timeout =
	    comp_dec_ptr->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_inst_retry_max
 *
 * Purpose:  Decode avnd component instantiation retry max value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_inst_retry_max(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 12);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.inst_retry_max = comp_dec_ptr->clc_info.inst_retry_max;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_inst_retry_cnt
 *
 * Purpose:  Decode avnd component instantiation retry count value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_inst_retry_cnt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 13);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.inst_retry_cnt = comp_dec_ptr->clc_info.inst_retry_cnt;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_exec_cmd
 *
 * Purpose:  Decode avnd component execution command value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_exec_cmd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 37);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.exec_cmd = comp_dec_ptr->clc_info.exec_cmd;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_cmd_exec_ctxt
 *
 * Purpose:  Decode avnd component execution context.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_cmd_exec_ctxt(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 38);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.cmd_exec_ctxt = comp_dec_ptr->clc_info.cmd_exec_ctxt;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_inst_cmd_ts
 *
 * Purpose:  Decode avnd component instantiation time stamp value.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_inst_cmd_ts(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 14);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.inst_cmd_ts = comp_dec_ptr->clc_info.inst_cmd_ts;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_clc_reg_tmr
 *
 * Purpose:  Decode avnd component registration timer inforamtion.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_clc_reg_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 3, 1, 39, 40);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.clc_reg_tmr.is_active = comp_dec_ptr->clc_info.clc_reg_tmr.is_active;
	comp_rec->clc_info.clc_reg_tmr.type = comp_dec_ptr->clc_info.clc_reg_tmr.type;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_inst_code_rcvd
 *
 * Purpose:  Decode avnd component instantiation received code inforamtion.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_inst_code_rcvd(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 2, 1, 15);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
	if (NULL == comp_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_rec->clc_info.inst_code_rcvd = comp_dec_ptr->clc_info.inst_code_rcvd;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_proxy_proxied_add
 *
 * Purpose:  Decode avnd component proxy-proxied inforamtion.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_proxy_proxied_add(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;
	AVND_COMP *pxy_comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 3, 1, 31, 41);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	/* This component might be any one of the following:
	   1. an internode, which is a proxy for an external component.
	   2. An external component, local to Ctrl.

	   In case 1. => Search comp in EXT_COMPDB_REC and proxy in COMPDB_REC.
	   In case 2. => Search comp in COMPDB_REC and proxy in EXT_COMPDB_REC.
	 */

	if (m_AVND_COMP_TYPE_IS_INTER_NODE(comp_dec_ptr)) {
		comp_rec = m_AVND_INT_EXT_COMPDB_REC_GET(cb->internode_avail_comp_db, comp_dec_ptr->name);
		pxy_comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->proxy_comp_name);
	} else {
		comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
		pxy_comp_rec = m_AVND_INT_EXT_COMPDB_REC_GET(cb->internode_avail_comp_db,
							     comp_dec_ptr->proxy_comp_name);
	}

	if ((NULL == comp_rec) || (NULL == pxy_comp_rec)) {
		return NCSCC_RC_FAILURE;
	}

	status = avnd_comp_proxied_add(cb, comp_rec, pxy_comp_rec, false);
	if (NCSCC_RC_SUCCESS != status) {
		return NCSCC_RC_FAILURE;
	}

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_proxy_proxied_del
 *
 * Purpose:  Decode avnd component proxy-proxied inforamtion.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_proxy_proxied_del(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP *comp_dec_ptr = NULL;
	AVND_COMP dec_comp;
	AVND_COMP *comp_rec = NULL;
	AVND_COMP *pxy_comp_rec = NULL;


	comp_dec_ptr = &dec_comp;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP **)&comp_dec_ptr, &ederror, 3, 1, 31, 41);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	/* This component might be any one of the following:
	   1. an internode, which is a proxy for an external component.
	   2. An external component, local to Ctrl.

	   In case 1. => Search comp in EXT_COMPDB_REC and proxy in COMPDB_REC.
	   In case 2. => Search comp in COMPDB_REC and proxy in EXT_COMPDB_REC.
	 */

	if (m_AVND_COMP_TYPE_IS_INTER_NODE(comp_dec_ptr)) {
		comp_rec = m_AVND_INT_EXT_COMPDB_REC_GET(cb->internode_avail_comp_db, comp_dec_ptr->name);
		pxy_comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->proxy_comp_name);
	} else {
		comp_rec = m_AVND_COMPDB_REC_GET(cb->compdb, comp_dec_ptr->name);
		pxy_comp_rec = m_AVND_INT_EXT_COMPDB_REC_GET(cb->internode_avail_comp_db,
							     comp_dec_ptr->proxy_comp_name);
	}

	if ((NULL == comp_rec) || (NULL == pxy_comp_rec)) {
		return NCSCC_RC_FAILURE;
	}

	status = avnd_comp_proxied_del(cb, comp_rec, pxy_comp_rec, false, NULL);
	if (NCSCC_RC_SUCCESS != status) {
		return NCSCC_RC_FAILURE;
	}

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_si_rec_curr_state
 *
 * Purpose:  Decode avnd SU_SI_REC current state information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_si_rec_curr_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU_SI_REC *su_si_dec_ptr = NULL;
	AVND_SU_SI_REC dec_su_si;
	AVND_SU_SI_REC *su_si_rec = NULL;


	su_si_dec_ptr = &dec_su_si;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU_SI_REC **)&su_si_dec_ptr, &ederror, 3, 1, 2, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_si_rec = avnd_su_si_rec_get(cb, &su_si_dec_ptr->su_name, &su_si_dec_ptr->name);
	if (NULL == su_si_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_si_rec->curr_state = su_si_dec_ptr->curr_state;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_si_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_si_rec_prv_state
 *
 * Purpose:  Decode avnd SU_SI_REC previous state information.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_si_rec_prv_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU_SI_REC *su_si_dec_ptr = NULL;
	AVND_SU_SI_REC dec_su_si;
	AVND_SU_SI_REC *su_si_rec = NULL;


	su_si_dec_ptr = &dec_su_si;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU_SI_REC **)&su_si_dec_ptr, &ederror, 3, 1, 3, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_si_rec = avnd_su_si_rec_get(cb, &su_si_dec_ptr->su_name, &su_si_dec_ptr->name);
	if (NULL == su_si_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_si_rec->prv_state = su_si_dec_ptr->prv_state;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_si_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_si_rec_curr_assign_state
 *
 * Purpose:  Decode avnd SU_SI_REC current assign state information
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_si_rec_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU_SI_REC *su_si_dec_ptr = NULL;
	AVND_SU_SI_REC dec_su_si;
	AVND_SU_SI_REC *su_si_rec = NULL;


	su_si_dec_ptr = &dec_su_si;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU_SI_REC **)&su_si_dec_ptr, &ederror, 3, 1, 4, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_si_rec = avnd_su_si_rec_get(cb, &su_si_dec_ptr->su_name, &su_si_dec_ptr->name);
	if (NULL == su_si_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_si_rec->curr_assign_state = su_si_dec_ptr->curr_assign_state;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_si_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_su_si_rec_prv_assign_state
 *
 * Purpose:  Decode avnd SU_SI_REC previous assign state information
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_su_si_rec_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_SU_SI_REC *su_si_dec_ptr = NULL;
	AVND_SU_SI_REC dec_su_si;
	AVND_SU_SI_REC *su_si_rec = NULL;


	su_si_dec_ptr = &dec_su_si;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_su_si, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_SU_SI_REC **)&su_si_dec_ptr, &ederror, 3, 1, 5, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	su_si_rec = avnd_su_si_rec_get(cb, &su_si_dec_ptr->su_name, &su_si_dec_ptr->name);
	if (NULL == su_si_rec) {
		return NCSCC_RC_FAILURE;
	}

	su_si_rec->prv_assign_state = su_si_dec_ptr->prv_assign_state;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.su_si_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_csi_act_comp_name
 *
 * Purpose:  Decode AVND_COMP_CSI_REC active component name info. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_csi_act_comp_name(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CSI_REC *csi_dec_ptr = NULL;
	AVND_COMP_CSI_REC dec_csi;
	AVND_COMP_CSI_REC *csi_rec = NULL;


	csi_dec_ptr = &dec_csi;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_dec_ptr, &ederror, 3, 1, 3, 9);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	csi_rec = avnd_compdb_csi_rec_get(cb, &csi_dec_ptr->comp_name, &csi_dec_ptr->name);
	if (NULL == csi_rec) {
		return NCSCC_RC_FAILURE;
	}

	csi_rec->act_comp_name = csi_dec_ptr->act_comp_name;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.csi_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_csi_trans_desc
 *
 * Purpose:  Decode AVND_COMP_CSI_REC transition descriptor. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_csi_trans_desc(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CSI_REC *csi_dec_ptr = NULL;
	AVND_COMP_CSI_REC dec_csi;
	AVND_COMP_CSI_REC *csi_rec = NULL;


	csi_dec_ptr = &dec_csi;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_dec_ptr, &ederror, 3, 1, 4, 9);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	csi_rec = avnd_compdb_csi_rec_get(cb, &csi_dec_ptr->comp_name, &csi_dec_ptr->name);
	if (NULL == csi_rec) {
		return NCSCC_RC_FAILURE;
	}

	csi_rec->trans_desc = csi_dec_ptr->trans_desc;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.csi_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_csi_standby_rank
 *
 * Purpose:  Decode AVND_COMP_CSI_REC Standby Rank. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_csi_standby_rank(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CSI_REC *csi_dec_ptr = NULL;
	AVND_COMP_CSI_REC dec_csi;
	AVND_COMP_CSI_REC *csi_rec = NULL;


	csi_dec_ptr = &dec_csi;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_dec_ptr, &ederror, 3, 1, 5, 9);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	csi_rec = avnd_compdb_csi_rec_get(cb, &csi_dec_ptr->comp_name, &csi_dec_ptr->name);
	if (NULL == csi_rec) {
		return NCSCC_RC_FAILURE;
	}

	csi_rec->standby_rank = csi_dec_ptr->standby_rank;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.csi_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_csi_curr_assign_state
 *
 * Purpose:  Decode AVND_COMP_CSI_REC current assign state. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_csi_curr_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CSI_REC *csi_dec_ptr = NULL;
	AVND_COMP_CSI_REC dec_csi;
	AVND_COMP_CSI_REC *csi_rec = NULL;


	csi_dec_ptr = &dec_csi;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_dec_ptr, &ederror, 3, 1, 7, 9);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	csi_rec = avnd_compdb_csi_rec_get(cb, &csi_dec_ptr->comp_name, &csi_dec_ptr->name);
	if (NULL == csi_rec) {
		return NCSCC_RC_FAILURE;
	}

	csi_rec->curr_assign_state = csi_dec_ptr->curr_assign_state;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.csi_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_csi_prv_assign_state
 *
 * Purpose:  Decode AVND_COMP_CSI_REC previous assign state. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_csi_prv_assign_state(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CSI_REC *csi_dec_ptr = NULL;
	AVND_COMP_CSI_REC dec_csi;
	AVND_COMP_CSI_REC *csi_rec = NULL;


	csi_dec_ptr = &dec_csi;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_csi_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CSI_REC **)&csi_dec_ptr, &ederror, 3, 1, 8, 9);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	csi_rec = avnd_compdb_csi_rec_get(cb, &csi_dec_ptr->comp_name, &csi_dec_ptr->name);
	if (NULL == csi_rec) {
		return NCSCC_RC_FAILURE;
	}

	csi_rec->prv_assign_state = csi_dec_ptr->prv_assign_state;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.csi_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_hc_rec_status
 *
 * Purpose:  Decode AVND_COMP_HC_REC status info. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_hc_rec_status(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_HC_REC *comp_hc_dec_ptr = NULL;
	AVND_COMP_HC_REC *comp_hc_rec = NULL;
	AVND_COMP *comp = NULL;
	AVND_COMP_HC_REC tmp_hc_rec;
	AVND_COMP_HC_REC dec_comp_hc;


	comp_hc_dec_ptr = &dec_comp_hc;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_HC_REC **)&comp_hc_dec_ptr, &ederror, 4, 1, 4, 8, 9);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp = m_AVND_COMPDB_REC_GET(cb->compdb, comp_hc_dec_ptr->comp_name);
	if (NULL == comp) {
		return NCSCC_RC_FAILURE;
	}

	memset(&tmp_hc_rec, '\0', sizeof(AVND_COMP_HC_REC));
	tmp_hc_rec.key = comp_hc_dec_ptr->key;
	tmp_hc_rec.req_hdl = comp_hc_dec_ptr->req_hdl;

	comp_hc_rec = m_AVND_COMPDB_REC_HC_GET(*comp, tmp_hc_rec);

	if (NULL == comp_hc_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_hc_rec->status = comp_hc_dec_ptr->status;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_hlth_rec_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_hc_rec_tmr
 *
 * Purpose:  Decode AVND_COMP_HC_REC tmr info. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_hc_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_HC_REC *comp_hc_dec_ptr = NULL;
	AVND_COMP_HC_REC *comp_hc_rec = NULL;
	AVND_COMP *comp = NULL;
	AVND_COMP_HC_REC tmp_hc_rec;
	AVND_COMP_HC_REC dec_comp_hc;


	comp_hc_dec_ptr = &dec_comp_hc;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_hc, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_HC_REC **)&comp_hc_dec_ptr, &ederror, 4, 1, 4, 9, 10);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp = m_AVND_COMPDB_REC_GET(cb->compdb, comp_hc_dec_ptr->comp_name);
	if (NULL == comp) {
		return NCSCC_RC_FAILURE;
	}

	memset(&tmp_hc_rec, '\0', sizeof(AVND_COMP_HC_REC));
	tmp_hc_rec.key = comp_hc_dec_ptr->key;
	tmp_hc_rec.req_hdl = comp_hc_dec_ptr->req_hdl;

	comp_hc_rec = m_AVND_COMPDB_REC_HC_GET(*comp, tmp_hc_rec);

	if (NULL == comp_hc_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_hc_rec->tmr.is_active = comp_hc_dec_ptr->tmr.is_active;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_hlth_rec_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_cbk_rec_amf_hdl
 *
 * Purpose:  Decode AVND_COMP_CBK amf handle info. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_cbk_rec_amf_hdl(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CBK *comp_cbk_dec_ptr = NULL;
	AVND_COMP_CBK *comp_cbk_rec = NULL;
	AVND_COMP *comp = NULL;
	AVND_COMP_CBK dec_comp_cbk;


	comp_cbk_dec_ptr = &dec_comp_cbk;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CBK **)&comp_cbk_dec_ptr, &ederror, 3, 1, 5, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp = m_AVND_COMPDB_REC_GET(cb->compdb, comp_cbk_dec_ptr->comp_name);
	if (NULL == comp) {
		return NCSCC_RC_FAILURE;
	}

	/* get the matching entry from the cbk list */
	m_AVND_COMP_CBQ_INV_GET(comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);

	/* it can be a responce from a proxied, see any proxied comp are there */
	if (!comp_cbk_rec && comp->pxied_list.n_nodes != 0) {
		AVND_COMP_PXIED_REC *rec = 0;

		/* parse thru all proxied comp, look in the list of inv handle of each of them */
		for (rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pxied_list); rec;
		     rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->comp_dll_node)) {
			m_AVND_COMP_CBQ_INV_GET(rec->pxied_comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);
			if (comp_cbk_rec)
				break;
		}
	}

	if (NULL == comp_cbk_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_cbk_rec->cbk_info->hdl = comp_cbk_dec_ptr->cbk_info->hdl;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_cbk_rec_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_cbk_rec_mds
 *
 * Purpose:  Decode AVND_COMP_CBK mds info. 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_cbk_rec_mds(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CBK *comp_cbk_dec_ptr = NULL;
	AVND_COMP_CBK *comp_cbk_rec = NULL;
	AVND_COMP *comp = NULL;
	AVND_COMP_CBK dec_comp_cbk;


	comp_cbk_dec_ptr = &dec_comp_cbk;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CBK **)&comp_cbk_dec_ptr, &ederror, 3, 1, 3, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp = m_AVND_COMPDB_REC_GET(cb->compdb, comp_cbk_dec_ptr->comp_name);
	if (NULL == comp) {
		return NCSCC_RC_FAILURE;
	}

	/* get the matching entry from the cbk list */
	m_AVND_COMP_CBQ_INV_GET(comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);

	/* it can be a responce from a proxied, see any proxied comp are there */
	if (!comp_cbk_rec && comp->pxied_list.n_nodes != 0) {
		AVND_COMP_PXIED_REC *rec = 0;

		/* parse thru all proxied comp, look in the list of inv handle of each of them */
		for (rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pxied_list); rec;
		     rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->comp_dll_node)) {
			m_AVND_COMP_CBQ_INV_GET(rec->pxied_comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);
			if (comp_cbk_rec)
				break;
		}
	}

	if (NULL == comp_cbk_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_cbk_rec->dest = comp_cbk_dec_ptr->dest;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_cbk_rec_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_cbk_rec_tmr
 *
 * Purpose:  Decode AVND_COMP_CBK timer info.  
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_cbk_rec_tmr(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CBK *comp_cbk_dec_ptr = NULL;
	AVND_COMP_CBK *comp_cbk_rec = NULL;
	AVND_COMP *comp = NULL;
	AVND_COMP_CBK dec_comp_cbk;


	comp_cbk_dec_ptr = &dec_comp_cbk;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CBK **)&comp_cbk_dec_ptr, &ederror, 3, 1, 6, 7);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp = m_AVND_COMPDB_REC_GET(cb->compdb, comp_cbk_dec_ptr->comp_name);
	if (NULL == comp) {
		return NCSCC_RC_FAILURE;
	}

	/* get the matching entry from the cbk list */
	m_AVND_COMP_CBQ_INV_GET(comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);

	/* it can be a responce from a proxied, see any proxied comp are there */
	if (!comp_cbk_rec && comp->pxied_list.n_nodes != 0) {
		AVND_COMP_PXIED_REC *rec = 0;

		/* parse thru all proxied comp, look in the list of inv handle of each of them */
		for (rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pxied_list); rec;
		     rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->comp_dll_node)) {
			m_AVND_COMP_CBQ_INV_GET(rec->pxied_comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);
			if (comp_cbk_rec)
				break;
		}
	}

	if (NULL == comp_cbk_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_cbk_rec->resp_tmr.is_active = comp_cbk_dec_ptr->resp_tmr.is_active;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_cbk_rec_updt++;

	return status;
}

/****************************************************************************\
 * Function: avnd_decode_ckpt_comp_cbk_rec_timeout
 *
 * Purpose:  Decode AVND_COMP_CBK timeout info.  
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uint32_t avnd_decode_ckpt_comp_cbk_rec_timeout(AVND_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = static_cast<EDU_ERR>(0);
	AVND_COMP_CBK *comp_cbk_dec_ptr = NULL;
	AVND_COMP_CBK *comp_cbk_rec = NULL;
	AVND_COMP *comp = NULL;
	AVND_COMP_CBK dec_comp_cbk;


	comp_cbk_dec_ptr = &dec_comp_cbk;

	/*
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU decode to decode this field.
	 */
	status = ncs_edu_exec(&cb->edu_hdl, avnd_edp_ckpt_msg_comp_cbk_rec, &dec->i_uba,
			      EDP_OP_TYPE_DEC, (AVND_COMP_CBK **)&comp_cbk_dec_ptr, &ederror, 3, 1, 4, 6);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		return status;
	}

	comp = m_AVND_COMPDB_REC_GET(cb->compdb, comp_cbk_dec_ptr->comp_name);
	if (NULL == comp) {
		return NCSCC_RC_FAILURE;
	}

	/* get the matching entry from the cbk list */
	m_AVND_COMP_CBQ_INV_GET(comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);

	/* it can be a responce from a proxied, see any proxied comp are there */
	if (!comp_cbk_rec && comp->pxied_list.n_nodes != 0) {
		AVND_COMP_PXIED_REC *rec = 0;

		/* parse thru all proxied comp, look in the list of inv handle of each of them */
		for (rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pxied_list); rec;
		     rec = (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->comp_dll_node)) {
			m_AVND_COMP_CBQ_INV_GET(rec->pxied_comp, comp_cbk_dec_ptr->opq_hdl, comp_cbk_rec);
			if (comp_cbk_rec)
				break;
		}
	}

	if (NULL == comp_cbk_rec) {
		return NCSCC_RC_FAILURE;
	}

	comp_cbk_rec->timeout = comp_cbk_dec_ptr->timeout;

	/* If update is successful, update async update count */
	cb->avnd_async_updt_cnt.comp_cbk_rec_updt++;

	return status;
}
