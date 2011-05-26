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
  in DTSv checkpoint messages.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "dts.h"

/*****************************************************************************

  PROCEDURE NAME:   dts_compile_ckpt_edp

  DESCRIPTION:      This function registers all the EDP's require 
                    for checkpointing.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dts_compile_ckpt_edp(DTS_CB *cb)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	EDU_ERR err = EDU_NORMAL;

	/* Initialize the EDU handle */
	m_NCS_EDU_HDL_INIT(&cb->edu_hdl);

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_log_ckpt_data_config, &err);
	if (rc != NCSCC_RC_SUCCESS)
		goto error;

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_ll_file_config, &err);
	if (rc != NCSCC_RC_SUCCESS)
		goto error;

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_file_list_config, &err);
	if (rc != NCSCC_RC_SUCCESS)
		goto error;

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, &err);
	if (rc != NCSCC_RC_SUCCESS)
		goto error;

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, dtsv_edp_ckpt_msg_dta_dest_list_config, &err);
	if (rc != NCSCC_RC_SUCCESS)
		goto error;

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, dtsv_edp_ckpt_msg_global_policy_config, &err);
	if (rc != NCSCC_RC_SUCCESS)
		goto error;

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, dtsv_edp_ckpt_msg_async_updt_cnt, &err);
	if (rc != NCSCC_RC_SUCCESS)
		goto error;

	return rc;

 error:
	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
	return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_compile_ckpt_edp: Unable to register EDP for DTS checkpointing");

}

/*****************************************************************************

  PROCEDURE NAME:   dtsv_edp_ckpt_msg_dts_log_ckpt_config

  DESCRIPTION:      EDU program handler for "DTS_LOG_CKPT" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "DTS_LOG_CKPT" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dtsv_edp_ckpt_msg_dts_log_ckpt_data_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
						 NCSCONTEXT ptr, uint32_t *ptr_data_len,
						 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	DTS_LOG_CKPT_DATA *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET dtsv_ckpt_msg_dts_log_ckpt_data_rules[] = {
		{EDU_START, dtsv_edp_ckpt_msg_dts_log_ckpt_data_config, 0, 0, 0,
		 sizeof(DTS_LOG_CKPT_DATA), 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
		 (long)&((DTS_LOG_CKPT_DATA *)0)->file_name, 250, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_LOG_CKPT_DATA *)0)->key.node, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_LOG_CKPT_DATA *)0)->key.ss_svc_id, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_LOG_CKPT_DATA *)0)->new_file, 0, NULL},
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (DTS_LOG_CKPT_DATA *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (DTS_LOG_CKPT_DATA **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(DTS_LOG_CKPT_DATA));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, dtsv_ckpt_msg_dts_log_ckpt_data_rules, struct_ptr, ptr_data_len, buf_env,
				 op, o_err);
	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   dtsv_edp_ckpt_msg_dts_ll_file_config

  DESCRIPTION:      EDU program handler for "DTS_LL_FILE" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "DTS_LL_FILE" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dtsv_edp_ckpt_msg_dts_ll_file_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					   NCSCONTEXT ptr, uint32_t *ptr_data_len,
					   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	DTS_LL_FILE *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET dtsv_ckpt_msg_dts_ll_file_rules[] = {
		{EDU_START, dtsv_edp_ckpt_msg_dts_ll_file_config, EDQ_LNKLIST, 0, 0,
		 sizeof(DTS_LL_FILE), 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
		 (long)&((DTS_LL_FILE *)0)->file_name, 250, NULL},
		{EDU_TEST_LL_PTR, dtsv_edp_ckpt_msg_dts_ll_file_config, 0, 0, 0,
		 (long)&((DTS_LL_FILE *)0)->next, 0, NULL},
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (DTS_LL_FILE *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (DTS_LL_FILE **)ptr;
		if (*d_ptr == NULL) {
			*d_ptr = m_MMGR_ALLOC_DTS_LOG_FILE;
			if (*d_ptr == NULL) {
				*o_err = EDU_ERR_MEM_FAIL;
				return NCSCC_RC_FAILURE;
			}
		}
		memset(*d_ptr, '\0', sizeof(DTS_LL_FILE));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, dtsv_ckpt_msg_dts_ll_file_rules, struct_ptr, ptr_data_len, buf_env, op,
				 o_err);
	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   dtsv_edp_ckpt_msg_dts_file_list_config

  DESCRIPTION:      EDU program handler for "DTS_FILE_LIST" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "DTS_FILE_LIST" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dtsv_edp_ckpt_msg_dts_file_list_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					     NCSCONTEXT ptr, uint32_t *ptr_data_len,
					     EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	DTS_FILE_LIST *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET dtsv_ckpt_msg_dts_file_list_rules[] = {
		{EDU_START, dtsv_edp_ckpt_msg_dts_file_list_config, 0, 0, 0,
		 sizeof(DTS_FILE_LIST), 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_FILE_LIST *)0)->num_of_files, 0, NULL},
		{EDU_EXEC, dtsv_edp_ckpt_msg_dts_ll_file_config, EDQ_POINTER, 0, 0,
		 (long)&((DTS_FILE_LIST *)0)->head, 0, NULL},
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (DTS_FILE_LIST *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (DTS_FILE_LIST **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(DTS_FILE_LIST));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, dtsv_ckpt_msg_dts_file_list_rules, struct_ptr, ptr_data_len, buf_env, op,
				 o_err);
	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config

  DESCRIPTION:      EDU program handler for "DTS_SVC_REG_TBL" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "DTS_SVC_REG_TBL" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					       NCSCONTEXT ptr, uint32_t *ptr_data_len,
					       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	/*Smik - Implementation needed - making changes */
	DTS_SVC_REG_TBL *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET dtsv_ckpt_msg_dts_svc_reg_tbl_rules[] = {
		{EDU_START, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, 0, 0, 0,
		 sizeof(DTS_SVC_REG_TBL), 0, NULL},

		/* DTS Service Registration table information */
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->per_node_logging, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->my_key.node, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->my_key.ss_svc_id, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.log_dev, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.log_file_size, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.file_log_fmt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.cir_buff_size, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.buff_log_fmt, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.enable, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.category_bit_map, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->svc_policy.severity_bit_map, 0, NULL},
		/* Log file name would be checkpointed by another msg */
		/*{EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
		   (uint32_t)&((DTS_SVC_REG_TBL*)0)->device.log_file, 250, NULL}, */
		/* Smik - Omitting encoding for FILE* for recreating later */
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->device.file_open, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->device.cur_file_size, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->device.new_file, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->device.last_rec_id, 0, NULL},
		/* Smik - Omitting circular buffer */
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->device.file_log_fmt_change, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTS_SVC_REG_TBL *)0)->device.buff_log_fmt_change, 0, NULL},
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (DTS_SVC_REG_TBL *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (DTS_SVC_REG_TBL **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(DTS_SVC_REG_TBL));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, dtsv_ckpt_msg_dts_svc_reg_tbl_rules, struct_ptr, ptr_data_len, buf_env,
				 op, o_err);
	/* Smik - implementation ends here */
	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   dtsv_edp_ckpt_msg_dta_dest_list_config

  DESCRIPTION:      EDU program handler for "DTA_DEST_LIST" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "DTA_DEST_LIST" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dtsv_edp_ckpt_msg_dta_dest_list_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					     NCSCONTEXT ptr, uint32_t *ptr_data_len,
					     EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	DTA_DEST_LIST *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET dtsv_ckpt_msg_dta_dest_list_rules[] = {
		{EDU_START, dtsv_edp_ckpt_msg_dta_dest_list_config, 0, 0, 0,
		 sizeof(DTA_DEST_LIST), 0, NULL},

		/* Fill here DTA_DEST_LIST data structure encoding rules */
		/*{EDU_TEST_LL_PTR, dtsv_edp_ckpt_msg_dta_dest_list_config, 0, 0, 0,
		   (uint32_t)&((DTA_DEST_LIST*)0)->qel, 0, NULL}, */
		{EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0,
		 (long)&((DTA_DEST_LIST *)0)->dta_addr, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTA_DEST_LIST *)0)->dta_up, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((DTA_DEST_LIST *)0)->updt_req, 0, NULL},
		/* Smik - Need to add rule for NCS_QUEUE svc_list */
		/*{EDU_EXEC, dtsv_edp_ckpt_msg_dta_dest_list_config, 0, 0, 0,
		   (uint32_t)&((DTA_DEST_LIST*)0)->svc_list.head, 0, NULL}, */
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (DTA_DEST_LIST *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (DTA_DEST_LIST **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(DTA_DEST_LIST));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, dtsv_ckpt_msg_dta_dest_list_rules, struct_ptr, ptr_data_len, buf_env, op,
				 o_err);
	/* Smik - implementation ends here */
	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   dtsv_edp_ckpt_msg_global_policy_config

  DESCRIPTION:      EDU program handler for "GLOBAL_POLICY" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "GLOBAL_POLICY" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dtsv_edp_ckpt_msg_global_policy_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					     NCSCONTEXT ptr, uint32_t *ptr_data_len,
					     EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	/* Smik - implementation needed */
	GLOBAL_POLICY *struct_ptr = NULL, **d_ptr = NULL;

	EDU_INST_SET dtsv_ckpt_msg_global_policy_rules[] = {
		{EDU_START, dtsv_edp_ckpt_msg_global_policy_config, 0, 0, 0,
		 sizeof(GLOBAL_POLICY), 0, NULL},

		/* Fill here GLOBAL_POLICY data structure encoding rules */
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->global_logging, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.log_dev, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.log_file_size, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.file_log_fmt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.cir_buff_size, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.buff_log_fmt, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.enable, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.category_bit_map, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_policy.severity_bit_map, 0, NULL},
		/* Log file name would be ckpted by another msg */
		/*{EDU_EXEC, ncs_edp_char, EDQ_ARRAY, 0, 0,
		   (uint32_t)&((GLOBAL_POLICY*)0)->device.log_file, 250, NULL}, */
		/* Smik - Omitting encoding for FILE* for recreating later */
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->device.file_open, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->device.cur_file_size, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->device.new_file, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->device.last_rec_id, 0, NULL},
		/* Smik - Omitting circular buffer */
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->device.file_log_fmt_change, 0, NULL},
		{EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->device.buff_log_fmt_change, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->dflt_logging, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_num_log_files, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_enable_seq, 0, NULL},
		{EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0,
		 (long)&((GLOBAL_POLICY *)0)->g_close_files, 0, NULL},
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (GLOBAL_POLICY *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (GLOBAL_POLICY **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(GLOBAL_POLICY));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, dtsv_ckpt_msg_global_policy_rules, struct_ptr, ptr_data_len, buf_env, op,
				 o_err);
	/* Smik - implementation ends here */
	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   dtsv_edp_ckpt_msg_async_updt_cnt

  DESCRIPTION:      EDU program handler for "DTSV_ASYNC_UPDT_CNT" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "DTSV_ASYNC_UPDT_CNT" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uint32_t dtsv_edp_ckpt_msg_async_updt_cnt(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				       NCSCONTEXT ptr, uint32_t *ptr_data_len,
				       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	DTSV_ASYNC_UPDT_CNT *struct_ptr = NULL, **d_ptr = NULL;
	/* Smik - implementation needed here */
	EDU_INST_SET dtsv_ckpt_msg_async_updt_cnt_rules[] = {
		{EDU_START, dtsv_edp_ckpt_msg_async_updt_cnt, 0, 0, 0,
		 sizeof(DTSV_ASYNC_UPDT_CNT), 0, NULL},

		/*
		 * Fill here Async Update data structure encoding rules 
		 */
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTSV_ASYNC_UPDT_CNT *)0)->dts_svc_reg_tbl_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTSV_ASYNC_UPDT_CNT *)0)->dta_dest_list_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTSV_ASYNC_UPDT_CNT *)0)->global_policy_updt, 0, NULL},
		{EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
		 (long)&((DTSV_ASYNC_UPDT_CNT *)0)->dts_log_updt, 0, NULL},
		{EDU_END, 0, 0, 0, 0, 0, 0, NULL},
	};

	if (op == EDP_OP_TYPE_ENC) {
		struct_ptr = (DTSV_ASYNC_UPDT_CNT *)ptr;
	} else if (op == EDP_OP_TYPE_DEC) {
		d_ptr = (DTSV_ASYNC_UPDT_CNT **)ptr;
		if (*d_ptr == NULL) {
			*o_err = EDU_ERR_MEM_FAIL;
			return NCSCC_RC_FAILURE;
		}
		memset(*d_ptr, '\0', sizeof(DTSV_ASYNC_UPDT_CNT));
		struct_ptr = *d_ptr;
	} else {
		struct_ptr = ptr;
	}

	rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, dtsv_ckpt_msg_async_updt_cnt_rules, struct_ptr, ptr_data_len, buf_env,
				 op, o_err);
	/* Smik - implementation ends here */
	return rc;
}
