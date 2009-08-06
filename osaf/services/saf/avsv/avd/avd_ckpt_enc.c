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
  AVD data structures during checkpointing. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"
/* Declaration of async update functions */
static uns32 avsv_encode_ckpt_avd_cb_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_avnd_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_sg_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_su_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_si_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_sg_admin_si(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_su_si_rel(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_comp_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_csi_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_hlt_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_oper_su(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_cb_cl_view_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avnd_node_up_info(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avnd_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avnd_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avnd_node_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avnd_rcv_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avnd_snd_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avnd_avm_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_sg_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_sg_adjust_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_sg_su_assigned_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_sg_su_spare_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_sg_su_uninst_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_sg_fsm_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_si_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_si_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_term_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_rediness_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_act_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_su_preinstan(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_si_su_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_si_su_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_si_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_si_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_comp_proxy_comp_name_net(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_comp_curr_num_csi_actv(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_comp_curr_num_csi_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_comp_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_comp_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_comp_restart_count(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_sus_per_si_rank_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_comp_cs_type_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_cs_type_param_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_ckpt_avd_si_dep_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uns32 avd_entire_data_update(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, NCS_BOOL c_sync);

/* Declaration of static cold sync encode functions */
static uns32 avsv_encode_cold_sync_rsp_avd_cb_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_avnd_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_sg_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_su_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_si_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_sg_su_oper_list(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_sg_admin_si(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_comp_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_csi_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_su_si_rel(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_hlt_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_async_updt_cnt(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_sus_per_si_rank_config(AVD_CL_CB *cb,
								  NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_comp_cs_type_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
static uns32 avsv_encode_cold_sync_rsp_avd_cs_type_param_config(AVD_CL_CB *cb,
								NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);

static uns32 avsv_encode_su_oper_list(AVD_CL_CB *cb, AVD_SG *sg, NCS_MBCSV_CB_ENC *enc);
static uns32 avsv_encode_cold_sync_rsp_avd_si_dep_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);
/*
 * Function list for encoding the async data.
 * We will jump into this function using the reo_type received 
 * in the encode argument.
 */
const AVSV_ENCODE_CKPT_DATA_FUNC_PTR avsv_enc_ckpt_data_func_list[AVSV_CKPT_MSG_MAX] = {
	avsv_encode_ckpt_avd_cb_config,
	avsv_encode_ckpt_avd_avnd_config,
	avsv_encode_ckpt_avd_sg_config,
	avsv_encode_ckpt_avd_su_config,
	avsv_encode_ckpt_avd_si_config,
	avsv_encode_ckpt_avd_oper_su,
	avsv_encode_ckpt_avd_sg_admin_si,
	avsv_encode_ckpt_avd_comp_config,
	avsv_encode_ckpt_avd_cs_type_param_config,
	avsv_encode_ckpt_avd_csi_config,
	avsv_encode_ckpt_avd_comp_cs_type_config,
	avsv_encode_ckpt_avd_su_si_rel,
	avsv_encode_ckpt_avd_hlt_config,
	avsv_encode_ckpt_avd_sus_per_si_rank_config,

	/* 
	 * Messages to update independent fields.
	 */

	/* CB Async Update messages */
	avsv_encode_ckpt_cb_cl_view_num,

	/* AVND Async Update messages */
	avsv_encode_ckpt_avnd_node_up_info,
	avsv_encode_ckpt_avnd_su_admin_state,
	avsv_encode_ckpt_avnd_oper_state,
	avsv_encode_ckpt_avnd_node_state,
	avsv_encode_ckpt_avnd_rcv_msg_id,
	avsv_encode_ckpt_avnd_snd_msg_id,
	avsv_encode_ckpt_avnd_avm_oper_state,

	/* SG Async Update messages */
	avsv_encode_ckpt_sg_admin_state,
	avsv_encode_ckpt_sg_adjust_state,
	avsv_encode_ckpt_sg_su_assigned_num,
	avsv_encode_ckpt_sg_su_spare_num,
	avsv_encode_ckpt_sg_su_uninst_num,
	avsv_encode_ckpt_sg_fsm_state,

	/* SU Async Update messages */
	avsv_encode_ckpt_su_si_curr_active,
	avsv_encode_ckpt_su_si_curr_stby,
	avsv_encode_ckpt_su_admin_state,
	avsv_encode_ckpt_su_term_state,
	avsv_encode_ckpt_su_switch,
	avsv_encode_ckpt_su_oper_state,
	avsv_encode_ckpt_su_pres_state,
	avsv_encode_ckpt_su_rediness_state,
	avsv_encode_ckpt_su_act_state,
	avsv_encode_ckpt_su_preinstan,

	/* SI Async Update messages */
	avsv_encode_ckpt_si_su_curr_active,
	avsv_encode_ckpt_si_su_curr_stby,
	avsv_encode_ckpt_si_switch,
	avsv_encode_ckpt_si_admin_state,

	/* COMP Async Update messages */
	avsv_encode_ckpt_comp_proxy_comp_name_net,
	avsv_encode_ckpt_comp_curr_num_csi_actv,
	avsv_encode_ckpt_comp_curr_num_csi_stby,
	avsv_encode_ckpt_comp_oper_state,
	avsv_encode_ckpt_comp_pres_state,
	avsv_encode_ckpt_comp_restart_count,
	NULL,			/* AVSV_SYNC_COMMIT */
	avsv_encode_ckpt_avd_si_dep_config
};

/*
 * Function list for encoding the cold sync response data
 * We will jump into this function using the reo_type received 
 * in the cold sync repsonce argument.
 */
const AVSV_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR avsv_enc_cold_sync_rsp_data_func_list[] = {
	avsv_encode_cold_sync_rsp_avd_cb_config,
	avsv_encode_cold_sync_rsp_avd_avnd_config,
	avsv_encode_cold_sync_rsp_avd_sg_config,
	avsv_encode_cold_sync_rsp_avd_su_config,
	avsv_encode_cold_sync_rsp_avd_si_config,
	avsv_encode_cold_sync_rsp_avd_sg_su_oper_list,
	avsv_encode_cold_sync_rsp_avd_sg_admin_si,
	avsv_encode_cold_sync_rsp_avd_comp_config,
	avsv_encode_cold_sync_rsp_avd_cs_type_param_config,
	avsv_encode_cold_sync_rsp_avd_csi_config,
	avsv_encode_cold_sync_rsp_avd_comp_cs_type_config,
	avsv_encode_cold_sync_rsp_avd_su_si_rel,
	avsv_encode_cold_sync_rsp_avd_hlt_config,
	avsv_encode_cold_sync_rsp_avd_sus_per_si_rank_config,
	avsv_encode_cold_sync_rsp_avd_async_updt_cnt,
	avsv_encode_cold_sync_rsp_avd_si_dep_config
};

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_cb_config
 *
 * Purpose:  Encode entire CB data..
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
static uns32 avsv_encode_ckpt_avd_cb_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_cb_config");

	/* 
	 * For updating CB, action is always to do update. We don't have add and remove
	 * action on CB. So call EDU to encode CB data.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cb, &enc->io_uba,
				    EDP_OP_TYPE_ENC, cb, &ederror, enc->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		return status;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_avnd_config
 *
 * Purpose:  Encode entire AVD_AVND data.
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
static uns32 avsv_encode_ckpt_avd_avnd_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_avnd_config");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		return status;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_sg_config
 *
 * Purpose:  Encode entire AVD_SG data..
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
static uns32 avsv_encode_ckpt_avd_sg_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_sg_config");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_su_config
 *
 * Purpose:  Encode entire AVD_SU data..
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
static uns32 avsv_encode_ckpt_avd_su_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_su_config");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_si_dep_config
 *
 * Purpose:  Encode entire AVD_SI_SI_DEP data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
static uns32 avsv_encode_ckpt_avd_si_dep_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_si_dep_config");

	/* Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si_dep, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_SI_SI_DEP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si_dep, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVD_SI_SI_DEP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_si_config
 *
 * Purpose:  Encode entire AVD_SI data..
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
static uns32 avsv_encode_ckpt_avd_si_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_si_config");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_SI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_sg_admin_si
 *
 * Purpose:  Encode entire AVD_SG_ADMIN_SI data..
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
static uns32 avsv_encode_ckpt_avd_sg_admin_si(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_sg_admin_si");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. 
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_RMV:
		/*
		 * Send SI key.
		 */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVD_SI *)((AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)))->
						admin_si, &ederror, enc->i_peer_version, 1, 1);
		break;

	case NCS_MBCSV_ACT_UPDATE:
	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_su_si_rel
 *
 * Purpose:  Encode entire AVD_SU_SI_REL data..
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
static uns32 avsv_encode_ckpt_avd_su_si_rel(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVSV_SU_SI_REL_CKPT_MSG su_si_ckpt;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_su_si_rel");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only. In this case key is SU and SI key.
	 */
	su_si_ckpt.su_name_net = ((AVD_SU_SI_REL *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)))->su->name_net;
	su_si_ckpt.si_name_net = ((AVD_SU_SI_REL *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)))->si->name_net;

	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		su_si_ckpt.fsm = ((AVD_SU_SI_REL *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)))->fsm;
		su_si_ckpt.state = ((AVD_SU_SI_REL *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)))->state;

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su_si_rel,
					    &enc->io_uba, EDP_OP_TYPE_ENC, &su_si_ckpt, &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su_si_rel, &enc->io_uba,
						EDP_OP_TYPE_ENC, &su_si_ckpt, &ederror, enc->i_peer_version, 2, 1, 2);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_comp_config
 *
 * Purpose:  Encode entire AVD_COMP data..
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
static uns32 avsv_encode_ckpt_avd_comp_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_comp_config");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_csi_config
 *
 * Purpose:  Encode entire AVD_CSI data..
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
static uns32 avsv_encode_ckpt_avd_csi_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_csi_config");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_csi, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_CSI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_csi, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_CSI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_hlt_config
 *
 * Purpose:  Encode entire AVD_HLT data..
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
static uns32 avsv_encode_ckpt_avd_hlt_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_hlt_config");

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_UPDATE:
	case NCS_MBCSV_ACT_ADD:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avd_hlt, &enc->io_uba,
					    EDP_OP_TYPE_ENC, (AVD_HLT *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
					    &ederror, enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avd_hlt, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_HLT *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 3, 1, 2, 3);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_oper_su
 *
 * Purpose:  Encode Operation SU name.
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
static uns32 avsv_encode_ckpt_avd_oper_su(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_oper_su");

	/* 
	 * In case of both Add and remove request send the operation SU name. 
	 * We don't have update for this reo_type.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_RMV:
		/* Send entire data */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 1, 1);
		break;

	case NCS_MBCSV_ACT_UPDATE:
	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}
	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_cb_cl_view_num
 *
 * Purpose:  Encode cluster view number.
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
static uns32 avsv_encode_ckpt_cb_cl_view_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_cb_cl_view_num");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cb, &enc->io_uba,
						EDP_OP_TYPE_ENC, cb, &ederror, enc->i_peer_version, 1, 3);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avnd_node_up_info
 *
 * Purpose:  Encode avnd node up info.
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
static uns32 avsv_encode_ckpt_avnd_node_up_info(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avnd_node_up_info");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 6, 1, 2, 4, 5, 6, 7);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avnd_su_admin_state
 *
 * Purpose:  Encode avnd su admin state info.
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
static uns32 avsv_encode_ckpt_avnd_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avnd_su_admin_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 8);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avnd_oper_state
 *
 * Purpose:  Encode avnd oper state info.
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
static uns32 avsv_encode_ckpt_avnd_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avnd_oper_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avnd_node_state
 *
 * Purpose:  Encode avnd node state info.
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
static uns32 avsv_encode_ckpt_avnd_node_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avnd_node_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 11);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avnd_rcv_msg_id
 *
 * Purpose:  Encode avnd receive message ID.
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
static uns32 avsv_encode_ckpt_avnd_rcv_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avnd_rcv_msg_id");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 15);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avnd_snd_msg_id
 *
 * Purpose:  Encode avnd Send message ID.
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
static uns32 avsv_encode_ckpt_avnd_snd_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avnd_snd_msg_id");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 16);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avnd_avm_oper_state
 *
 * Purpose:  Encode avnd avm oper state info.
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
static uns32 avsv_encode_ckpt_avnd_avm_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avnd_avm_oper_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_AVND *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 17);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_sg_admin_state
 *
 * Purpose:  Encode SG admin state.
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
static uns32 avsv_encode_ckpt_sg_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_sg_admin_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 10);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_sg_adjust_state
 *
 * Purpose:  Encode SG adjust state.
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
static uns32 avsv_encode_ckpt_sg_adjust_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_sg_adjust_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 11);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_sg_su_assigned_num
 *
 * Purpose:  Encode SG su assign number.
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
static uns32 avsv_encode_ckpt_sg_su_assigned_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_sg_su_assigned_num");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 18);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_sg_su_spare_num
 *
 * Purpose:  Encode SG su spare number.
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
static uns32 avsv_encode_ckpt_sg_su_spare_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_sg_su_spare_num");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 19);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_sg_su_uninst_num
 *
 * Purpose:  Encode SG su uninst number.
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
static uns32 avsv_encode_ckpt_sg_su_uninst_num(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_sg_su_uninst_num");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 20);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_sg_fsm_state
 *
 * Purpose:  Encode SG FSM state.
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
static uns32 avsv_encode_ckpt_sg_fsm_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_sg_fsm_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SG *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 21);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_si_curr_active
 *
 * Purpose:  Encode SU Current number of Active SI.
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
static uns32 avsv_encode_ckpt_su_si_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_si_curr_active");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 6);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_si_curr_stby
 *
 * Purpose:  Encode SU Current number of Standby SI.
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
static uns32 avsv_encode_ckpt_su_si_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_si_curr_stby");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 7);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_admin_state
 *
 * Purpose:  Encode SU Admin state.
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
static uns32 avsv_encode_ckpt_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_admin_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_term_state
 *
 * Purpose:  Encode SU Admin state to terminate service.
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
static uns32 avsv_encode_ckpt_su_term_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_term_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 10);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_switch
 *
 * Purpose:  Encode SU toggle SI.
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
static uns32 avsv_encode_ckpt_su_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_switch");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 11);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_oper_state
 *
 * Purpose:  Encode SU Operation state.
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
static uns32 avsv_encode_ckpt_su_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_oper_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 12);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_pres_state
 *
 * Purpose:  Encode SU Presence state.
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
static uns32 avsv_encode_ckpt_su_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_pres_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 13);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_rediness_state
 *
 * Purpose:  Encode SU Rediness state.
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
static uns32 avsv_encode_ckpt_su_rediness_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_rediness_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 15);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_act_state
 *
 * Purpose:  Encode SU action state.
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
static uns32 avsv_encode_ckpt_su_act_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_act_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 17);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_su_preinstan
 *
 * Purpose:  Encode SU preinstatible object.
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
static uns32 avsv_encode_ckpt_su_preinstan(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_su_preinstan");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SU *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 18);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_si_su_curr_active
 *
 * Purpose:  Encode number of active SU assignment for this SI.
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
static uns32 avsv_encode_ckpt_si_su_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_si_su_curr_active");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 4);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_si_su_curr_stby
 *
 * Purpose:  Encode number of standby SU assignment for this SI.
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
static uns32 avsv_encode_ckpt_si_su_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_si_su_curr_stby");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 5);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}
	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_si_switch
 *
 * Purpose:  Encode SI switch.
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
static uns32 avsv_encode_ckpt_si_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_si_switch");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 8);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_si_admin_state
 *
 * Purpose:  Encode SI admin state.
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
static uns32 avsv_encode_ckpt_si_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_si_admin_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_SI *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 9);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_comp_proxy_comp_name_net
 *
 * Purpose:  Encode COMP proxy compnent name.
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
static uns32 avsv_encode_ckpt_comp_proxy_comp_name_net(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_comp_proxy_comp_name_net");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 23);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_comp_curr_num_csi_actv
 *
 * Purpose:  Encode COMP Current number of CSI active.
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
static uns32 avsv_encode_ckpt_comp_curr_num_csi_actv(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_comp_curr_num_csi_actv");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 32);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_comp_curr_num_csi_stby
 *
 * Purpose:  Encode COMP Current number of CSI standby.
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
static uns32 avsv_encode_ckpt_comp_curr_num_csi_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_comp_curr_num_csi_stby");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 33);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_comp_oper_state
 *
 * Purpose:  Encode COMP Operation State.
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
static uns32 avsv_encode_ckpt_comp_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_comp_oper_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 34);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_comp_pres_state
 *
 * Purpose:  Encode COMP Presence State.
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
static uns32 avsv_encode_ckpt_comp_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_comp_pres_state");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 35);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_comp_restart_count
 *
 * Purpose:  Encode COMP Restart count.
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
static uns32 avsv_encode_ckpt_comp_restart_count(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_comp_restart_count");

	/* 
	 * Action in this case is just to update. If action passed is add/rmv then log
	 * error. Call EDU encode to encode this field.
	 */
	if (NCS_MBCSV_ACT_UPDATE == enc->io_action) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
						EDP_OP_TYPE_ENC, (AVD_COMP *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 36);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
	} else {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp
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
uns32 avsv_encode_cold_sync_rsp(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp");

	return avd_entire_data_update(cb, enc, TRUE);
}

/****************************************************************************\
 * Function: avd_entire_data_update
 *
 * Purpose:  Encode entire data to be sent during cold sync or warm sync.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *        c_sync - TRUE - Called while in cold sync.
 *                 FALSE - Called while in warm sync.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avd_entire_data_update(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, NCS_BOOL c_sync)
{
	uns32 status = NCSCC_RC_SUCCESS;
	uns32 num_of_obj = 0;
	uns8 *encoded_cnt_loc;
	uns8 logbuff[SA_MAX_NAME_LENGTH];

	/* 
	 * Since at decode we need to find out how many objects of particular data
	 * type are being sent, encode that information at the begining of the message.
	 */
	encoded_cnt_loc = ncs_enc_reserve_space(&enc->io_uba, sizeof(uns32));
	if (!encoded_cnt_loc) {
		/* Handle error */
	}
	ncs_enc_claim_space(&enc->io_uba, sizeof(uns32));

	/* 
	 * If reo_handle and reo_type is NULL then this the first time mbcsv is calling
	 * the cold sync response for the standby. So start from first data structure 
	 * which is CB. Next time onwards depending on the value of reo_type and reo_handle
	 * send the next data structures.
	 */
	if (enc->io_reo_type == AVSV_CKPT_AVD_SI_DEP_CONFIG) {
		status = avsv_encode_cold_sync_rsp_avd_si_dep_config(cb, enc, &num_of_obj);
	} else {
		status = avsv_enc_cold_sync_rsp_data_func_list[enc->io_reo_type] (cb, enc, &num_of_obj);
	}

	memset(logbuff, '\0', SA_MAX_NAME_LENGTH);
	snprintf(logbuff, SA_MAX_NAME_LENGTH - 1,
		 "avd_entire_data_update\n\nSent reotype = %d num_obj = %d -------------\n", enc->io_reo_type,
		 num_of_obj);
	m_AVD_LOG_FUNC_ENTRY(logbuff);

	/* Now encode the number of objects actually in the UBA. */

	if (encoded_cnt_loc != NULL) {
		ncs_encode_32bit(&encoded_cnt_loc, num_of_obj);
	}

	/*
	 * Check if we have reached to last message required to be sent in cold sync 
	 * response, if yes then send cold sync complete. Else ask MBCSv to call you 
	 * back with the next reo_type.
	 */
	if ((AVSV_CKPT_AVD_SI_DEP_CONFIG == enc->io_reo_type) ||
	    ((AVSV_COLD_SYNC_RSP_ASYNC_UPDT_CNT == enc->io_reo_type) && (cb->avd_peer_ver < 2))) {
		if (c_sync) {
			enc->io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;

			/* Start Heart Beating with the peer */
			avd_init_heartbeat(cb);
		} else
			enc->io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE;
	} else {
		if ((AVSV_COLD_SYNC_RSP_ASYNC_UPDT_CNT == enc->io_reo_type) && (cb->avd_peer_ver >= 2)) {
			/* Jump to next cold sync updt reo_type - AVSV_CKPT_AVD_SI_DEP_CONFIG */
			enc->io_reo_type = AVSV_CKPT_AVD_SI_DEP_CONFIG;
		} else
			enc->io_reo_type++;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_cb_config
 *
 * Purpose:  Encode entire CB data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_cb_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_cb_config");

	/* 
	 * Send the CB data.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cb, &enc->io_uba,
				    EDP_OP_TYPE_ENC, cb, &ederror, enc->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		return NCSCC_RC_FAILURE;
	}

	*num_of_obj = 1;

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_avnd_config
 *
 * Purpose:  Encode entire AVD_AVND data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_avnd_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	SaClmNodeIdT node_id = 0;
	AVD_AVND *avnd;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_avnd_config");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	while (NULL != (avnd = (AVD_AVND *)ncs_patricia_tree_getnext(&cb->avnd_anchor, (uns8 *)&node_id))) {
		if (NCS_ROW_ACTIVE != avnd->row_status) {
			node_id = avnd->node_info.nodeId;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
					    &enc->io_uba, EDP_OP_TYPE_ENC, avnd, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		(*num_of_obj)++;
		node_id = avnd->node_info.nodeId;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_sg_config
 *
 * Purpose:  Encode entire AVD_SG data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_sg_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	SaNameT sg_name_net;
	AVD_SG *sg;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_sg_config");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	sg_name_net.length = 0;
	for (sg = avd_sg_struc_find_next(cb, sg_name_net, FALSE);
	     sg != AVD_SG_NULL; sg = avd_sg_struc_find_next(cb, sg_name_net, FALSE)) {
		if (NCS_ROW_ACTIVE != sg->row_status) {
			sg_name_net = sg->name_net;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &enc->io_uba,
					    EDP_OP_TYPE_ENC, sg, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		sg_name_net = sg->name_net;
		(*num_of_obj)++;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_su_config
 *
 * Purpose:  Encode entire AVD_SU data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_su_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_SU *su;
	SaNameT su_name;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_su_config");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	su_name.length = 0;
	for (su = avd_su_struc_find_next(cb, su_name, FALSE); su != NULL;
	     su = avd_su_struc_find_next(cb, su_name, FALSE)) {
		if (NCS_ROW_ACTIVE != su->row_status) {
			su_name = su->name_net;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
					    EDP_OP_TYPE_ENC, su, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		su_name = su->name_net;
		(*num_of_obj)++;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_si_dep_config
 *
 * Purpose:  Encode entire AVD_SI_SI_DEP data..
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
static uns32 avsv_encode_cold_sync_rsp_avd_si_dep_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_SI_SI_DEP_INDX indx;
	EDU_ERR ederror = 0;
	AVD_SI_SI_DEP *si_dep;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_si_dep_config");

	/* Walk through the entire list and send the entire list data. */
	memset((char *)&indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));

	si_dep = avd_si_si_dep_find_next(cb, &indx, TRUE);
	while (si_dep) {
		if (NCS_ROW_ACTIVE != si_dep->row_status) {
			si_dep = avd_si_si_dep_find_next(cb, &si_dep->indx_mib, TRUE);
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si_dep, &enc->io_uba,
					    EDP_OP_TYPE_ENC, si_dep, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		(*num_of_obj)++;
		si_dep = avd_si_si_dep_find_next(cb, &si_dep->indx_mib, TRUE);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_si_config
 *
 * Purpose:  Encode entire AVD_SI data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_si_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_SI *si;
	SaNameT si_name;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_si_config");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	si_name.length = 0;
	for (si = avd_si_struc_find_next(cb, si_name, FALSE); si != NULL;
	     si = avd_si_struc_find_next(cb, si_name, FALSE)) {
		if (NCS_ROW_ACTIVE != si->row_status) {
			si_name = si->name_net;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
					    EDP_OP_TYPE_ENC, si, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		si_name = si->name_net;
		(*num_of_obj)++;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_sg_su_oper_list
 *
 * Purpose:  Encode entire AVD_SG_SU_OPER_LIST data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_sg_su_oper_list(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	SaNameT sg_name_net;
	AVD_SG *sg;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_sg_su_oper_list");

	/* 
	 * Walk through the entire SG list and send the SU operation list
	 * for that SG and then move to next SG.
	 */
	sg_name_net.length = 0;
	for (sg = avd_sg_struc_find_next(cb, sg_name_net, FALSE);
	     sg != AVD_SG_NULL; sg = avd_sg_struc_find_next(cb, sg_name_net, FALSE)) {
		if (NCS_ROW_ACTIVE != sg->row_status) {
			sg_name_net = sg->name_net;
			continue;
		}

		status = avsv_encode_su_oper_list(cb, sg, enc);

		if (status != NCSCC_RC_SUCCESS) {
			/* Log Error */
			m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
			return NCSCC_RC_FAILURE;
		}

		sg_name_net = sg->name_net;
		(*num_of_obj)++;

	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_sg_admin_si
 *
 * Purpose:  Encode entire AVD_SG_ADMIN_SI data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_sg_admin_si(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	SaNameT sg_name_net;
	AVD_SG *sg;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_sg_admin_si");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	sg_name_net.length = 0;
	for (sg = avd_sg_struc_find_next(cb, sg_name_net, FALSE);
	     sg != AVD_SG_NULL; sg = avd_sg_struc_find_next(cb, sg_name_net, FALSE)) {
		if ((NCS_ROW_ACTIVE != sg->row_status) || (NULL == sg->admin_si)) {
			sg_name_net = sg->name_net;
			continue;
		}

		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &enc->io_uba,
						EDP_OP_TYPE_ENC, sg->admin_si, &ederror, enc->i_peer_version, 1, 1);

		if (status != NCSCC_RC_SUCCESS) {
			/* Log Error */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		sg_name_net = sg->name_net;
		(*num_of_obj)++;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_su_si_rel
 *
 * Purpose:  Encode entire AVD_SU_SI_REL data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_su_si_rel(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_SU *su;
	SaNameT su_name;
	AVSV_SU_SI_REL_CKPT_MSG su_si_ckpt;
	AVD_SU_SI_REL *rel;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_su_si_rel");

	/* 
	 * Walk through the entire list and send the entire list data.
	 * We will walk the SU tree and all the SU_SI relationship for that SU
	 * are sent.We will send the corresponding COMP_CSI relationship for that SU_SI
	 * in the same update.
	 */
	su_name.length = 0;
	for (su = avd_su_struc_find_next(cb, su_name, FALSE); su != NULL;
	     su = avd_su_struc_find_next(cb, su_name, FALSE)) {
		if (NCS_ROW_ACTIVE != su->row_status) {
			su_name = su->name_net;
			continue;
		}

		su_si_ckpt.su_name_net = su->name_net;

		for (rel = su->list_of_susi; rel != NULL; rel = rel->su_next) {
			su_si_ckpt.si_name_net = rel->si->name_net;
			su_si_ckpt.fsm = rel->fsm;
			su_si_ckpt.state = rel->state;

			status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su_si_rel,
						    &enc->io_uba, EDP_OP_TYPE_ENC, &su_si_ckpt, &ederror,
						    enc->i_peer_version);

			if (status != NCSCC_RC_SUCCESS) {
				/* Encode failed!!! */
				m_AVD_LOG_INVALID_VAL_FATAL(ederror);
				return status;
			}

			(*num_of_obj)++;
		}
		su_name = su->name_net;
	}
	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_comp_config
 *
 * Purpose:  Encode entire AVD_COMP data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_comp_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_COMP *comp;
	SaNameT comp_name;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_comp_config");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	comp_name.length = 0;
	for (comp = avd_comp_struc_find_next(cb, comp_name, FALSE); comp != NULL;
	     comp = avd_comp_struc_find_next(cb, comp_name, FALSE)) {
		if (NCS_ROW_ACTIVE != comp->row_status) {
			comp_name = comp->comp_info.name_net;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &enc->io_uba,
					    EDP_OP_TYPE_ENC, comp, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Log Error */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		comp_name = comp->comp_info.name_net;
		(*num_of_obj)++;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_csi_config
 *
 * Purpose:  Encode entire AVD_CSI data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_csi_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_CSI *csi;
	SaNameT csi_name;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_csi_config");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	csi_name.length = 0;
	for (csi = avd_csi_struc_find_next(cb, csi_name, FALSE); csi != NULL;
	     csi = avd_csi_struc_find_next(cb, csi_name, FALSE)) {
		if (NCS_ROW_ACTIVE != csi->row_status) {
			csi_name = csi->name_net;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_csi, &enc->io_uba,
					    EDP_OP_TYPE_ENC, csi, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Log Error */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			return NCSCC_RC_FAILURE;
		}

		csi_name = csi->name_net;
		(*num_of_obj)++;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_hlt_config
 *
 * Purpose:  Encode entire AVD_HLT data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_hlt_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_HLT *hlt_chk = AVD_HLT_NULL;
	AVSV_HLT_KEY hlt_name;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_hlt_config");

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	memset(&hlt_name, '\0', sizeof(AVSV_HLT_KEY));
	for (hlt_chk = avd_hlt_struc_find_next(cb, hlt_name); hlt_chk != NULL;
	     hlt_chk = avd_hlt_struc_find_next(cb, hlt_name)) {
		if (NCS_ROW_ACTIVE != hlt_chk->row_status) {
			hlt_name = hlt_chk->key_name;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avd_hlt, &enc->io_uba,
					    EDP_OP_TYPE_ENC, hlt_chk, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			break;
		}

		hlt_name = hlt_chk->key_name;
		(*num_of_obj)++;
	}
	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_async_updt_cnt
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
static uns32 avsv_encode_cold_sync_rsp_avd_async_updt_cnt(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_async_updt_cnt");

	/* 
	 * Encode and send async update counts for all the data structures.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_async_updt_cnt,
				    &enc->io_uba, EDP_OP_TYPE_ENC, &cb->async_updt_cnt, &ederror, enc->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_warm_sync_rsp
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
uns32 avsv_encode_warm_sync_rsp(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;
	uns8 logbuff[SA_MAX_NAME_LENGTH];

	memset(logbuff, '\0', SA_MAX_NAME_LENGTH);
	snprintf(logbuff, SA_MAX_NAME_LENGTH - 1,
		 "avsv_encode_warm_sync_rsp \n UPDATE CNTS AT ACTIVE(WarmSink) : cb=%d,avnd=%d,sg=%d,su=%d,si=%d,ol=%d,\nas=%d,rel=%d,co=%d,cs=%d,pl=%d,hlt=%d, sus=%d, ccstype=%d, cspar=%d \n",
		 cb->async_updt_cnt.cb_updt, cb->async_updt_cnt.avnd_updt, cb->async_updt_cnt.sg_updt,
		 cb->async_updt_cnt.su_updt, cb->async_updt_cnt.si_updt, cb->async_updt_cnt.sg_su_oprlist_updt,
		 cb->async_updt_cnt.sg_admin_si_updt, cb->async_updt_cnt.su_si_rel_updt, cb->async_updt_cnt.comp_updt,
		 cb->async_updt_cnt.csi_updt, cb->async_updt_cnt.csi_parm_list_updt, cb->async_updt_cnt.hlt_updt,
		 cb->async_updt_cnt.sus_per_si_rank_updt, cb->async_updt_cnt.comp_cs_type_sup_updt,
		 cb->async_updt_cnt.cs_type_param_updt);
	m_AVD_LOG_FUNC_ENTRY(logbuff);

	/* 
	 * Encode and send latest async update counts. (In the same manner we sent
	 * in the last message of the cold sync response.
	 */
	status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_async_updt_cnt,
				    &enc->io_uba, EDP_OP_TYPE_ENC, &cb->async_updt_cnt, &ederror, enc->i_peer_version);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_data_sync_rsp
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
uns32 avsv_encode_data_sync_rsp(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	m_AVD_LOG_FUNC_ENTRY("avsv_encode_data_sync_rsp");

	return avd_entire_data_update(cb, enc, FALSE);
}

/****************************************************************************\
 * Function: avsv_encode_su_oper_list
 *
 * Purpose:  Encode entire AVD_SG_SU_OPER_LIST data..
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
static uns32 avsv_encode_su_oper_list(AVD_CL_CB *cb, AVD_SG *sg, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	uns32 num_of_opr_su = 0;
	uns8 *encoded_cnt_loc;
	AVD_SG_OPER *oper_list_ptr = NULL;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_su_oper_list");

	/* Reserve space for the number of operation SU to be encoded */
	encoded_cnt_loc = ncs_enc_reserve_space(&enc->io_uba, sizeof(uns32));
	if (!encoded_cnt_loc) {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
	}
	ncs_enc_claim_space(&enc->io_uba, sizeof(uns32));

	/*
	 * Now walk through the entire SU operation list and encode it.
	 */
	for (oper_list_ptr = &sg->su_oper_list;
	     ((oper_list_ptr != NULL) && (oper_list_ptr->su != NULL)); oper_list_ptr = oper_list_ptr->next) {
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &enc->io_uba,
						EDP_OP_TYPE_ENC, oper_list_ptr->su, &ederror, enc->i_peer_version, 1,
						1);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
		}
		num_of_opr_su++;
	}

	/* 
	 * Encode the number of operation SU's we are sending in this message.
	 */
	if (encoded_cnt_loc != NULL) {
		ncs_encode_32bit(&encoded_cnt_loc, num_of_opr_su);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_sus_per_si_rank_config
 *
 * Purpose:  Encode entire AVD_SUS_PER_SI_RANK data..
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
static uns32 avsv_encode_ckpt_avd_sus_per_si_rank_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_sus_per_si_rank_config");

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_UPDATE:
	case NCS_MBCSV_ACT_ADD:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sus_per_si_rank, &enc->io_uba,
					    EDP_OP_TYPE_ENC,
					    (AVD_SUS_PER_SI_RANK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
					    enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sus_per_si_rank, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVD_SUS_PER_SI_RANK *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)),
						&ederror, enc->i_peer_version, 2, 1, 2);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_comp_cs_type_config
 *
 * Purpose:  Encode entire AVD_COMP_CS_TYPE data..
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
static uns32 avsv_encode_ckpt_avd_comp_cs_type_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_comp_cs_type_config");

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_UPDATE:
	case NCS_MBCSV_ACT_ADD:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp_cs_type, &enc->io_uba,
					    EDP_OP_TYPE_ENC,
					    (AVD_COMP_CS_TYPE *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
					    enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp_cs_type, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVD_COMP_CS_TYPE *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 2, 1, 2);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_ckpt_avd_cs_type_param_config
 *
 * Purpose:  Encode entire AVD_CS_TYPE_PARAM data..
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
static uns32 avsv_encode_ckpt_avd_cs_type_param_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uns32 status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_ckpt_avd_cs_type_param_config");

	/*
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_UPDATE:
	case NCS_MBCSV_ACT_ADD:
		/* Send entire data */
		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cs_type_param, &enc->io_uba,
					    EDP_OP_TYPE_ENC,
					    (AVD_CS_TYPE_PARAM *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
					    enc->i_peer_version);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cs_type_param, &enc->io_uba,
						EDP_OP_TYPE_ENC,
						(AVD_CS_TYPE_PARAM *)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror,
						enc->i_peer_version, 2, 1, 2);
		break;

	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_AVD_LOG_INVALID_VAL_FATAL(ederror);
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_sus_per_si_rank_config
 *
 * Purpose:  Encode entire AVD_SUS_PER_SI_RANK data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_sus_per_si_rank_config(AVD_CL_CB *cb,
								  NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_SUS_PER_SI_RANK *su_si = AVD_SU_PER_SI_RANK_NULL;
	AVD_SUS_PER_SI_RANK_INDX indx;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_sus_per_si_rank_config");

	/*
	 * Walk through the entire list and send the entire list data.
	 */
	memset(&indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
	for (su_si = avd_sus_per_si_rank_struc_find_next(cb, indx); su_si != NULL;
	     su_si = avd_sus_per_si_rank_struc_find_next(cb, indx)) {
		if (NCS_ROW_ACTIVE != su_si->row_status) {
			indx = su_si->indx;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sus_per_si_rank, &enc->io_uba,
					    EDP_OP_TYPE_ENC, su_si, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			break;
		}

		indx = su_si->indx;
		(*num_of_obj)++;
	}
	return status;
}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_comp_cs_type_config
 *
 * Purpose:  Encode entire AVD_COMP_CS_TYPE data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_comp_cs_type_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_COMP_CS_TYPE *cs_type = AVD_COMP_CS_TYPE_NULL;
	AVD_COMP_CS_TYPE_INDX indx;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_comp_cs_type_config");

	/*
	 * Walk through the entire list and send the entire list data.
	 */
	memset(&indx, '\0', sizeof(AVD_COMP_CS_TYPE_INDX));
	for (cs_type = avd_comp_cs_type_struc_find_next(cb, indx); cs_type != NULL;
	     cs_type = avd_comp_cs_type_struc_find_next(cb, indx)) {
		if (NCS_ROW_ACTIVE != cs_type->row_status) {
			indx = cs_type->indx;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp_cs_type, &enc->io_uba,
					    EDP_OP_TYPE_ENC, cs_type, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			break;
		}

		indx = cs_type->indx;
		(*num_of_obj)++;
	}
	return status;

}

/****************************************************************************\
 * Function: avsv_encode_cold_sync_rsp_avd_cs_type_param_config
 *
 * Purpose:  Encode entire AVD_CS_TYPE_PARAM data..
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
static uns32 avsv_encode_cold_sync_rsp_avd_cs_type_param_config(AVD_CL_CB *cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_CS_TYPE_PARAM *cs_param = AVD_CS_TYPE_PARAM_NULL;
	AVD_CS_TYPE_PARAM_INDX indx;
	EDU_ERR ederror = 0;

	m_AVD_LOG_FUNC_ENTRY("avsv_encode_cold_sync_rsp_avd_cs_type_param_config");

	/*
	 * Walk through the entire list and send the entire list data.
	 */
	memset(&indx, '\0', sizeof(AVD_CS_TYPE_PARAM_INDX));
	for (cs_param = avd_cs_type_param_struc_find_next(cb, indx); cs_param != NULL;
	     cs_param = avd_cs_type_param_struc_find_next(cb, indx)) {
		if (NCS_ROW_ACTIVE != cs_param->row_status) {
			indx = cs_param->indx;
			continue;
		}

		status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cs_type_param, &enc->io_uba,
					    EDP_OP_TYPE_ENC, cs_param, &ederror, enc->i_peer_version);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_AVD_LOG_INVALID_VAL_FATAL(ederror);
			break;
		}

		indx = cs_param->indx;
		(*num_of_obj)++;
	}
	return status;

}
