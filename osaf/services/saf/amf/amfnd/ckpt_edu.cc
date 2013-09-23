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

  This file contains utility routines for managing encode/decode operations
  in AvSv checkpoint messages.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avnd.h"
#include "amf.h"
#include "ncs_saf_edu.h"
#include "avnd_ckpt_edu.h"
#include "amf_d2nedu.h"
#include "amf_n2avaedu.h"

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_hlt_config

  DESCRIPTION:      EDU program handler for "AVND_HC" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_HC" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_hlt_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				   NCSCONTEXT ptr, uint32_t *ptr_data_len,
				   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_HC *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_hlt_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_hlt_config, 0, 0, 0,
		 sizeof(AVND_HC), 0, NULL},

		/* Fill here AVD HLT data structure encoding rules */
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_HC *)0)->key.comp_name, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
		 (long)&((AVND_HC *)0)->key.key_len, 4, NULL},
		{EDU_EXEC, ncs_edp_saamfhealthcheckkeyt, 0, 0, 0,
		 (long)&((AVND_HC *)0)->key.name, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_HC *)0)->period, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_HC *)0)->max_dur, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_HC *)0)->is_ext, 0, NULL},

		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_HC *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_HC **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_HC));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_HC*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_hlt_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_su

  DESCRIPTION:      EDU program handler for "AVND_SU" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_SU" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_su(EDU_HDL *hdl, EDU_TKN *edu_tkn,
			   NCSCONTEXT ptr, uint32_t *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_SU *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_su_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_su, 0, 0, 0,
		 sizeof(AVND_SU), 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_SU *)0)->name, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->flag, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->su_err_esc_level, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_SU *)0)->comp_restart_prob, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->comp_restart_max, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_SU *)0)->su_restart_prob, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->su_restart_max, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->comp_restart_cnt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->su_restart_cnt, 0, NULL},

		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_SU *)0)->su_err_esc_tmr.is_active, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->oper, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->pres, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->si_active_cnt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU *)0)->si_standby_cnt, 0, NULL},

		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_SU *)0)->su_is_external, 0, NULL},

		/* Fill here AVND SU data structure encoding rules */
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_SU *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_SU **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_SU));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_SU*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_su_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_su_si

  DESCRIPTION:      EDU program handler for "AVND_SU_SI_REC" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_SU_SI_REC" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_su_si(EDU_HDL *hdl, EDU_TKN *edu_tkn,
			      NCSCONTEXT ptr, uint32_t *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_SU_SI_REC *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_su_si_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_su_si, 0, 0, 0,
		 sizeof(AVND_SU_SI_REC), 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_SU_SI_REC *)0)->name, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU_SI_REC *)0)->curr_state, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU_SI_REC *)0)->prv_state, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU_SI_REC *)0)->curr_assign_state, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU_SI_REC *)0)->prv_assign_state, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_SU_SI_REC *)0)->su_name, 0, NULL},

		/* Fill here AVND SU_SI data structure encoding rules */
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_SU_SI_REC *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_SU_SI_REC **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_SU_SI_REC));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_SU_SI_REC*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_su_si_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_comp

  DESCRIPTION:      EDU program handler for "AVND_COMP" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_COMP" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  NOTE   :          Only relevant information is being sent to peer AvND. Some
                    information like is_am_en is not sent as ext comp doesn't
                    support Active Monitoring.

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_comp(EDU_HDL *hdl, EDU_TKN *edu_tkn,
			     NCSCONTEXT ptr, uint32_t *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_COMP *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_comp_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_comp, 0, 0, 0,
		 sizeof(AVND_COMP), 0, NULL},

		/* Fill here AVND COMP data structure encoding rules */
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->name, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->inst_level, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->flag, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->is_restart_en, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SAAMFCOMPONENTCAPABILITYMODELT, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->cap, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].len, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].cmd,
		 AVSV_MISC_STR_MAX_SIZE, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_INSTANTIATE - 1].timeout,
		 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].len, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].cmd,
		 AVSV_MISC_STR_MAX_SIZE, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.cmds[AVND_COMP_CLC_CMD_TYPE_TERMINATE - 1].timeout,
		 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.inst_retry_max, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.inst_retry_cnt, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.inst_cmd_ts, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.inst_code_rcvd, 0, NULL},

		{EDU_EXEC, m_NCS_EDP_SAAMFHANDLET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->reg_hdl, 0, NULL},
		{EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->reg_dest, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->oper, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->pres, 0, NULL},

		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->term_cbk_timeout, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->csi_set_cbk_timeout, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->quies_complete_cbk_timeout, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->csi_rmv_cbk_timeout, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->pxied_inst_cbk_timeout, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->pxied_clean_cbk_timeout, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->err_info.src, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->err_info.def_rec, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->err_info.detect_time, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->err_info.restart_cnt, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->pend_evt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->comp_type, 0, NULL},

		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->orph_tmr.is_active, 0, NULL},

		{EDU_EXEC, m_NCS_EDP_SACLMNODEIDT, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->node_id, 0, NULL},

		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->mds_ctxt.length, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
		 (long)&((AVND_COMP *)0)->mds_ctxt.data,
		 MDS_SYNC_SND_CTXT_LEN_MAX, NULL},

		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->reg_resp_pending, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.exec_cmd, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.cmd_exec_ctxt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.clc_reg_tmr.is_active, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->clc_info.clc_reg_tmr.type, 0, NULL},

		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP *)0)->proxy_comp_name, 0, NULL},
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_COMP *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_COMP **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_COMP));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_COMP*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_comp_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_csi_rec

  DESCRIPTION:      EDU program handler for "AVND_COMP_CSI_REC" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_COMP_CSI_REC" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_csi_rec(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				NCSCONTEXT ptr, uint32_t *ptr_data_len,
				EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_COMP_CSI_REC *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_csi_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_csi_rec, 0, 0, 0,
		 sizeof(AVND_COMP_CSI_REC), 0, NULL},

		/* Fill here AVND CSI data structure encoding rules */
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->name, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->rank, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->act_comp_name, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->trans_desc, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->standby_rank, 0, NULL},
		{EDU_EXEC, avsv_edp_csi_attr_info, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->attrs, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->curr_assign_state, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->prv_assign_state, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->comp_name, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->si_name, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP_CSI_REC *)0)->su_name, 0, NULL},

		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_COMP_CSI_REC *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_COMP_CSI_REC **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_COMP_CSI_REC));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_COMP_CSI_REC*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_csi_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_siq

  DESCRIPTION:      EDU program handler for "AVND_SU_SI_PARAM" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_SU_SI_PARAM" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_siq(EDU_HDL *hdl, EDU_TKN *edu_tkn,
			    NCSCONTEXT ptr, uint32_t *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_SU_SI_PARAM *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_siq_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_siq, 0, 0, 0,
		 sizeof(AVND_SU_SI_PARAM), 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->msg_id, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SACLMNODEIDT, 0, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->node_id, 0, NULL},
		{EDU_EXEC, ncs_edp_int, 0, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->msg_act, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->su_name, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->si_name, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SAAMFHASTATET, 0, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->ha_state, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->num_assigns, 0, NULL},
		{EDU_EXEC, avsv_edp_susi_asgn, EDQ_POINTER, 0, 0,
		 (long)&((AVND_SU_SI_PARAM *)0)->list, 0, NULL},

		/* Fill here AVND SU_SI data structure encoding rules */
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_SU_SI_PARAM *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_SU_SI_PARAM **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_SU_SI_PARAM));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_SU_SI_PARAM*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_siq_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_comp_hc

  DESCRIPTION:      EDU program handler for "AVND_COMP_HC_REC" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_COMP_HC_REC" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_comp_hc(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				NCSCONTEXT ptr, uint32_t *ptr_data_len,
				EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_COMP_HC_REC *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_comp_hc_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_comp_hc, 0, 0, 0,
		 sizeof(AVND_COMP_HC_REC), 0, NULL},
		{EDU_EXEC, ncs_edp_saamfhealthcheckkeyt, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->key, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SAAMFHEALTHCHECKINVOCATIONT, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->inv, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SAAMFRECOMMENDEDRECOVERYT, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->rec_rcvr, 0, NULL},

		{EDU_EXEC, m_NCS_EDP_SAAMFHANDLET, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->req_hdl, 0, NULL},
		{EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->dest, 0, NULL},

		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->period, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->max_dur, 0, NULL},

		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->status, 0, NULL},
		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->comp_name, 0, NULL},

		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_COMP_HC_REC *)0)->tmr.is_active, 0, NULL},

		/* Fill here AVND COMP_HC data structure encoding rules */
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_COMP_HC_REC *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_COMP_HC_REC **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_COMP_HC_REC));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_COMP_HC_REC*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_comp_hc_rules, struct_ptr,
				 ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_comp_cbk_rec

  DESCRIPTION:      EDU program handler for "AVND_COMP_CBK" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVND_COMP_CBK" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_comp_cbk_rec(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				     NCSCONTEXT ptr, uint32_t *ptr_data_len,
				     EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_COMP_CBK *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_comp_cbk_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_comp_cbk_rec, 0, 0, 0,
		 sizeof(AVND_COMP_CBK), 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_CBK *)0)->opq_hdl, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_COMP_CBK *)0)->orig_opq_hdl, 0, NULL},
		{EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0,
		 (long)&((AVND_COMP_CBK *)0)->dest, 0, NULL},
		{EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
		 (long)&((AVND_COMP_CBK *)0)->timeout, 0, NULL},

		{EDU_EXEC, avsv_edp_cbq_info, EDQ_POINTER, 0, 0,
		 (long)&((AVND_COMP_CBK *)0)->cbk_info, 0, NULL},

		{EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
		 (long)&((AVND_COMP_CBK *)0)->comp_name, 0, NULL},

		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((AVND_COMP_CBK *)0)->resp_tmr.is_active, 0, NULL},

		/* Fill here AVND AVND_COMP_CBK data structure encoding rules */
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_COMP_CBK *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_COMP_CBK **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_COMP_CBK));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_COMP_CBK*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_comp_cbk_rules, struct_ptr,
				 ptr_data_len, buf_env, op, o_err);

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avnd_edp_ckpt_msg_async_updt_cnt

  DESCRIPTION:      EDU program handler for "AVND_ASYNC_UPDT_CNT" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVND_ASYNC_UPDT_CNT" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t avnd_edp_ckpt_msg_async_updt_cnt(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				       NCSCONTEXT ptr, uint32_t *ptr_data_len,
				       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	AVND_ASYNC_UPDT_CNT *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET avnd_ckpt_msg_async_updt_cnt_rules[] = {
		{EDU_START, avnd_edp_ckpt_msg_async_updt_cnt, 0, 0, 0,
		 sizeof(AVND_ASYNC_UPDT_CNT), 0, NULL},

		/*
		 * Fill here Async Update data structure encoding rules
		 */
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->hlth_config_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->su_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->comp_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->su_si_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->siq_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->csi_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->comp_hlth_rec_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((AVND_ASYNC_UPDT_CNT *)0)->comp_cbk_rec_updt, 0, NULL},

		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (AVND_ASYNC_UPDT_CNT *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (AVND_ASYNC_UPDT_CNT **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(AVND_ASYNC_UPDT_CNT));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = static_cast<AVND_ASYNC_UPDT_CNT*>(ptr);
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avnd_ckpt_msg_async_updt_cnt_rules, struct_ptr,
				 ptr_data_len, buf_env, op, o_err);

	return rc;
}
