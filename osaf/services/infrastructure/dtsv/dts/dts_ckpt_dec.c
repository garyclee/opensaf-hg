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
  DTS data structures during checkpointing. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "dts.h"

/* Declaration of async update functions */
static uint32_t dtsv_decode_ckpt_null(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t dtsv_decode_ckpt_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t dtsv_decode_ckpt_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t dtsv_decode_ckpt_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uint32_t dtsv_decode_ckpt_log_file_ckpt_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
/* Declaration of static cold sync decode functions */
static uint32_t dtsv_decode_cold_sync_null(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t dtsv_decode_cold_sync_rsp_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t dtsv_decode_cold_sync_rsp_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t dtsv_decode_cold_sync_rsp_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);
static uint32_t dtsv_decode_cold_sync_rsp_dts_async_updt_cnt(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj);

/*
 * Function list for decoding the async data.
 * We will jump into this function using the reo_type received 
 * in the decode argument.
 */

const DTSV_DECODE_CKPT_DATA_FUNC_PTR dtsv_dec_ckpt_data_func_list[DTSV_CKPT_MSG_MAX] = {
	dtsv_decode_ckpt_null,
	dtsv_decode_ckpt_dts_svc_reg_tbl_config,
	dtsv_decode_ckpt_dta_dest_list_config,
	dtsv_decode_ckpt_global_policy_config,
	dtsv_decode_ckpt_log_file_ckpt_config,
	/* 
	 * Messages to update independent fields.
	 */

};

/*
 * Function list for decoding the cold sync response data
 * We will jump into this function using the reo_type received 
 * in the cold sync repsonce argument.
 */
const DTSV_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR dtsv_dec_cold_sync_rsp_data_func_list[] = {
	dtsv_decode_cold_sync_null,
	dtsv_decode_cold_sync_rsp_dts_svc_reg_tbl_config,
	dtsv_decode_cold_sync_rsp_dta_dest_list_config,
	dtsv_decode_cold_sync_rsp_global_policy_config,
	dtsv_decode_cold_sync_rsp_dts_async_updt_cnt
};

/****************************************************************************\
 * Function: dtsv_decode_ckpt_null
 *
 * Purpose:  Error scenario. Nothing to encode.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t dtsv_decode_ckpt_null(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	return m_DTS_DBG_SINK(NCSCC_RC_SUCCESS, "dtsv_encode_ckpt_null: Received decode for reo_type of 0. Do nothing");
}

/****************************************************************************\
 * Function: dtsv_decode_cold_sync_null
 *
 * Purpose:  Error scenario. Nothing to encode.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t dtsv_decode_cold_sync_null(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	return m_DTS_DBG_SINK(NCSCC_RC_SUCCESS, "dtsv_encode_ckpt_null: Received decode for reo_type of 0. Do nothing");
}

/****************************************************************************\
 * Function: dtsv_decode_ckpt_dts_svc_reg_tbl_config
 *
 * Purpose:  Decode entire DTS_SVC_REG_TBL data.
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
static uint32_t dtsv_decode_ckpt_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;
	DTS_SVC_REG_TBL *svc_reg_ptr;
	DTS_SVC_REG_TBL dec_svc_reg;
	uint8_t *ptr = NULL;

	svc_reg_ptr = &dec_svc_reg;

	/* 
	 * For updating CB, action is always to do update. We don't have add and remove
	 * action on CB. So call EDU to decode CB data.
	 */
	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
		/* Decode data only */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTS_SVC_REG_TBL **)&svc_reg_ptr, &ederror);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Decode MDS_DEST of DTA to be removed */
		ptr = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&cb->svc_rmv_mds_dest, sizeof(uint64_t));
		if (ptr == NULL) {
			m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_ckpt_dts_svc_reg_tbl_config: Decode flatten space failed");
		}
		cb->svc_rmv_mds_dest = ncs_decode_64bit(&ptr);
		ncs_dec_skip_space(&dec->i_uba, sizeof(uint64_t));

	case NCS_MBCSV_ACT_UPDATE:
		/* Decode cli_bit_map first */
		ptr = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&cb->cli_bit_map, sizeof(uint8_t));
		if (ptr == NULL) {
			m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_ckpt_dts_svc_reg_tbl_config: Decode flatten space failed");
		}
		cb->cli_bit_map = ncs_decode_8bit(&ptr);
		ncs_dec_skip_space(&dec->i_uba, sizeof(uint8_t));
		/* Now decode svc */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTS_SVC_REG_TBL **)&svc_reg_ptr, &ederror);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Decode failed!!! */
		memset(&cb->svc_rmv_mds_dest, '\0', sizeof(MDS_DEST));
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_ckpt_dts_svc_reg_tbl_config: Decode failed");
	}
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_ASYNC_UPDT, DTS_FC_UPDT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILLLL",
		   DTS_ASYNC_SVC_REG, dec->i_action, svc_reg_ptr->my_key.node, svc_reg_ptr->my_key.ss_svc_id,
		   (uint32_t)cb->svc_rmv_mds_dest);
	status = dtsv_ckpt_add_rmv_updt_svc_reg(cb, svc_reg_ptr, dec->i_action);

	memset(&cb->svc_rmv_mds_dest, '\0', sizeof(MDS_DEST));

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->async_updt_cnt.dts_svc_reg_tbl_updt++;
	else
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);

	return status;
}
/****************************************************************************\
 * Function: dtsv_decode_ckpt_dta_dest_list_config
 *
 * Purpose:  Decode entire DTA_DEST_LIST data..
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
static uint32_t dtsv_decode_ckpt_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTA_DEST_LIST *dta_dest_ptr;
	DTA_DEST_LIST dec_dta_dest;
	EDU_ERR ederror = 0;
	uint8_t *dec_svc = NULL;	/*Added to decode svc_id crspndg to dta */
	uint8_t *dec_spec = NULL;
	SVC_KEY svc_key;

	dta_dest_ptr = &dec_dta_dest;

	dec_svc = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&svc_key.ss_svc_id, sizeof(uint32_t));
	if (dec_svc == NULL) {
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		memset(&cb->last_spec_loaded, '\0', sizeof(SPEC_CKPT));
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_decode_ckpt_dta_dest_list_config: Decode flatten space failed");
	}
	svc_key.ss_svc_id = ncs_decode_32bit(&dec_svc);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));

	/* Versioning support: Now decode the SPEC_CKPT struct */
	if (ncs_decode_n_octets_from_uba(&dec->i_uba, (uint8_t *)cb->last_spec_loaded.svc_name, DTSV_SVC_NAME_MAX) !=
	    NCSCC_RC_SUCCESS) {
		memset(&cb->last_spec_loaded, '\0', sizeof(SPEC_CKPT));
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_ckpt_dta_dest_list_config: Decode octets failed");
	}
	dec_spec = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&cb->last_spec_loaded.version, sizeof(uint16_t));
	if (dec_spec == NULL) {
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		memset(&cb->last_spec_loaded, '\0', sizeof(SPEC_CKPT));
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_decode_ckpt_dta_dest_list_config: Decode flatten space failed");
	}
	cb->last_spec_loaded.version = ncs_decode_16bit(&dec_spec);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint16_t));

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
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dta_dest_list_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTA_DEST_LIST **)&dta_dest_ptr, &ederror);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dta_dest_list_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTA_DEST_LIST **)&dta_dest_ptr, &ederror);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Decode failed!!! */
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		memset(&cb->last_spec_loaded, '\0', sizeof(SPEC_CKPT));
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_ckpt_dta_dest_list_config: Decode failed");
	}

	svc_key.node = m_NCS_NODE_ID_FROM_MDS_DEST(dta_dest_ptr->dta_addr);
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_ASYNC_UPDT, DTS_FC_UPDT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILLLL",
		   DTS_ASYNC_DTA_DEST, dec->i_action, svc_key.node, svc_key.ss_svc_id, (uint32_t)dta_dest_ptr->dta_addr);
	status = dtsv_ckpt_add_rmv_updt_dta_dest(cb, dta_dest_ptr, dec->i_action, svc_key);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->async_updt_cnt.dta_dest_list_updt++;
	else
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);

	/* Versioning support : reset the DTS CB's spec_ckpt structure */
	memset(&cb->last_spec_loaded, '\0', sizeof(SPEC_CKPT));

	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_ckpt_global_policy_config
 *
 * Purpose:  Decode entire GLOBAL_POLICY data..
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
static uint32_t dtsv_decode_ckpt_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	GLOBAL_POLICY *gp_ptr;
	GLOBAL_POLICY dec_gp;
	EDU_ERR ederror = 0;
	uint8_t *ptr = NULL;

	/* Smik - Decode DTS_CB->cli_bit_map before decoding the GLOBAL POLICY */
	ptr = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&cb->cli_bit_map, sizeof(uint8_t));
	if (ptr == NULL) {
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_decode_ckpt_global_policy_config: Decode flatten space failed");
	}
	cb->cli_bit_map = ncs_decode_8bit(&ptr);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint8_t));

	gp_ptr = &dec_gp;
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
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_global_policy_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (GLOBAL_POLICY **)&gp_ptr, &ederror);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_global_policy_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (GLOBAL_POLICY **)&gp_ptr, &ederror);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Decode failed!!! */
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_ckpt_global_policy_config: Decode failed");
	}

	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_ASYNC_UPDT, DTS_FC_UPDT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILLLL",
		   DTS_ASYNC_GLOBAL_POLICY, dec->i_action, 0, 0, 0);
	status = dtsv_ckpt_add_rmv_updt_global_policy(cb, gp_ptr, NULL, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->async_updt_cnt.global_policy_updt++;
	else
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);

	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_ckpt_dts_log_file_ckpt_config
 *
 * Purpose:  Decode entire DTS_LOG_CKPT_DATA data..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
static uint32_t dtsv_decode_ckpt_log_file_ckpt_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTS_LOG_CKPT_DATA *data_ptr;
	DTS_LOG_CKPT_DATA dec_data;
	EDU_ERR ederror = 0;

	data_ptr = &dec_data;

	switch (dec->i_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
	case NCS_MBCSV_ACT_RMV:
		break;
	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}
	status =
	    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_log_ckpt_data_config, &dec->i_uba, EDP_OP_TYPE_DEC,
			   (DTS_LOG_CKPT_DATA **)&data_ptr, &ederror);

	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_ASYNC_UPDT, DTS_FC_UPDT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TILLLL",
		   DTS_ASYNC_LOG_FILE, dec->i_action, data_ptr->key.node, data_ptr->key.ss_svc_id, 0);
	status = dtsv_ckpt_add_rmv_updt_dts_log(cb, data_ptr, dec->i_action);

	/* If update is successful, update async update count */
	if (NCSCC_RC_SUCCESS == status)
		cb->async_updt_cnt.dts_log_updt++;
	else
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);

	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_cold_sync_rsp
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
uint32_t dtsv_decode_cold_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t num_of_obj;
	uint8_t *ptr = NULL;

	m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_START);
	/* 
	 * Since at decode we need to find out how many objects of particular data
	 * type are sent, decode that information at the begining of the message.
	 */
	ptr = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&num_of_obj, sizeof(uint32_t));
	if (ptr == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_cold_sync_rsp: Decode flatten space failed");
	}

	num_of_obj = ncs_decode_32bit(&ptr);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));

	/* 
	 * Decode data received in the message.
	 */
	dec->i_action = NCS_MBCSV_ACT_ADD;
	status = dtsv_dec_cold_sync_rsp_data_func_list[dec->i_reo_type] (cb, dec, num_of_obj);

	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_cold_sync_rsp_dts_svc_reg_tbl_config
 *
 * Purpose:  Decode entire DTS_SVC_REG_TBL data..
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
static uint32_t dtsv_decode_cold_sync_rsp_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;
	DTS_SVC_REG_TBL *svc_reg_ptr;
	DTS_SVC_REG_TBL dec_svc_reg;
	DTS_FILE_LIST *file_list;
	uint32_t count;
	OP_DEVICE *dev = NULL;

	m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_SVC_REG);
	svc_reg_ptr = &dec_svc_reg;
	file_list = &dec_svc_reg.device.log_file_list;

	/* 
	 * Send the CB data.
	 */
	for (count = 0; count < num_of_obj; count++) {
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTS_SVC_REG_TBL **)&svc_reg_ptr, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Decode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_cold_sync_rsp_dts_svc_reg_tbl_config: Decode failed");
		}

		memset(file_list, '\0', sizeof(DTS_FILE_LIST));
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_file_list_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTS_FILE_LIST **)&file_list, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Decode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
			/* Removing probable mem leak */
			dev = &dec_svc_reg.device;
			m_DTS_FREE_FILE_LIST(dev);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_cold_sync_rsp_dts_svc_reg_tbl_config: Decode DTS_FILE_LIST failed");
		}

		status = dtsv_ckpt_add_rmv_updt_svc_reg(cb, svc_reg_ptr, NCS_MBCSV_ACT_ADD);

		/* Removing probable mem leak */
		dev = &dec_svc_reg.device;
		m_DTS_FREE_FILE_LIST(dev);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_cold_sync_rsp_dta_dest_list_config
 *
 * Purpose:  Decode entire DTA_DEST_LIST data..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: Since this restores the mapping of SVC<->DTAs, in case there were 
 *        unregister updates between cold-sync of SVC list and DTA list, this
 *        function will go through the list of services to remove all services
 *        with dta_count = 0 i.e. no corresponding DTA entry.
 * 
\**************************************************************************/
static uint32_t dtsv_decode_cold_sync_rsp_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0, svc_count = 0, i;
	uint16_t spec_present = 0;
	EDU_ERR ederror = 0;
	DTA_DEST_LIST *dta_dest_ptr;
	DTA_DEST_LIST dec_dta_dest;
	uint8_t *dec_svc = NULL;	/*Added to decode svc_id crspndg to dta */
	SVC_KEY svc_key, nt_key;
	DTS_SVC_REG_TBL *svc = NULL;
	OP_DEVICE *device = NULL;

	dta_dest_ptr = &dec_dta_dest;

	m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_DTA_DEST);
	/* 
	 * Walk through the entire list and decode the entire data received.
	 */
	for (count = 0; count < num_of_obj; count++) {
		/* First decode the DTA_DEST_LIST with edu */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dta_dest_list_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTA_DEST_LIST **)&dta_dest_ptr, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Decode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_cold_sync_rsp_dta_dest_list_config: Decode failed");
		}

		/* Now decode the num of svcs and svc_ids associated with DTA */
		dec_svc = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&svc_count, sizeof(uint32_t));
		if (dec_svc == NULL) {
			m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_cold_sync_rsp_dta_dest_list_config: Decode flatten space failed");
		}
		svc_count = ncs_decode_32bit(&dec_svc);
		ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));

		for (i = 0; i < svc_count; i++) {
			dec_svc = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&svc_key.ss_svc_id, sizeof(uint32_t));
			if (dec_svc == NULL) {
				m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_decode_cold_sync_rsp_dta_dest_list_config: Decode flatten space failed");
			}
			svc_key.ss_svc_id = ncs_decode_32bit(&dec_svc);
			ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));
			svc_key.node = m_NCS_NODE_ID_FROM_MDS_DEST(dta_dest_ptr->dta_addr);

			/* Check whether any spec_entry was sent by the sender */
			dec_svc = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&spec_present, sizeof(uint16_t));
			if (dec_svc == NULL) {
				m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_decode_cold_sync_rsp_dta_dest_list_config: Decode flatten space failed");
			}
			spec_present = ncs_decode_16bit(&dec_svc);
			ncs_dec_skip_space(&dec->i_uba, sizeof(uint16_t));
			if (spec_present != 0) {
				/*Now decode the version and service no. asscociated with this DTA */
				dec_svc =
				    ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&cb->last_spec_loaded.version,
							  sizeof(uint16_t));
				if (dec_svc == NULL) {
					m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_decode_cold_sync_rsp_dta_dest_list_config: Decode flatten space failed");
				}
				cb->last_spec_loaded.version = ncs_decode_16bit(&dec_svc);
				ncs_dec_skip_space(&dec->i_uba, sizeof(uint16_t));

				if (ncs_decode_n_octets_from_uba
				    (&dec->i_uba, (uint8_t *)&cb->last_spec_loaded.svc_name,
				     DTSV_SVC_NAME_MAX) != NCSCC_RC_SUCCESS)
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_decode_cold_sync_rsp_dta_dest_list_config: ncs_decode_n_octets_from_uba failed");
			}
			status = dtsv_ckpt_add_rmv_updt_dta_dest(cb, dta_dest_ptr, NCS_MBCSV_ACT_ADD, svc_key);

			/* Reset the CB last_spec_loaded struct */
			memset(&cb->last_spec_loaded, '\0', sizeof(SPEC_CKPT));

			if (status != NCSCC_RC_SUCCESS)
				return NCSCC_RC_FAILURE;
		}		/*end of for i */

		/*Adjust the num of svc for dta - inconsequential assignment */
		dta_dest_ptr->dta_num_svcs = svc_count;
	}			/*end of for count */

	/* Now since the DTA list sync is complete check if there are any
	 * services with dta_count = 0 delete those entries.
	 */
	svc = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&cb->svc_tbl, (const uint8_t *)NULL);
	while (svc != NULL) {
		nt_key = svc->ntwk_key;
		if ((svc->dta_count == 0) && (svc->my_key.ss_svc_id != 0)) {
			/* Remove the service entry now */
			if (&svc->device.cir_buffer != NULL)
				dts_circular_buffer_free(&svc->device.cir_buffer);
			device = &svc->device;
			/* Cleanup the DTS_FILE_LIST datastructure for svc */
			m_DTS_FREE_FILE_LIST(device);

			ncs_patricia_tree_del(&cb->svc_tbl, (NCS_PATRICIA_NODE *)svc);
			m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_RMVD, svc->my_key.ss_svc_id, svc->my_key.node, 0);
			if (NULL != svc) {
				m_MMGR_FREE_SVC_REG_TBL(svc);
				svc = NULL;
			}
		}
		svc = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&cb->svc_tbl, (const uint8_t *)&nt_key);
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_cold_sync_rsp_global_policy_config
 *
 * Purpose:  Decode entire GLOBAL_POLICY data..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: Decoding for cb->log_path added too.
 *
 * 
\**************************************************************************/
static uint32_t dtsv_decode_cold_sync_rsp_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t count = 0;
	EDU_ERR ederror = 0;
	GLOBAL_POLICY *gp_ptr;
	GLOBAL_POLICY dec_gp;
	DTS_FILE_LIST *file_list;
	uint8_t tmp, *dec_path = NULL;	/*Added to decode log directory path */
	uint32_t len, i;
	char str[200] = "", *c = NULL;
	OP_DEVICE *dev = NULL;

	gp_ptr = &dec_gp;
	file_list = &dec_gp.device.log_file_list;

	m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_GLOBAL_POLICY);
	/* Start decoding the log directory path */
	dec_path = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&len, sizeof(uint32_t));
	if (dec_path == NULL) {
		m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_decode_cold_sync_rsp_global_policy_config: Decode flatten space failed");
	}

	/* Decode the length of the string */
	len = ncs_decode_32bit(&dec_path);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));

	for (i = 0; i < len; i++) {
		dec_path = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&tmp, sizeof(uint8_t));
		if (dec_path == NULL) {
			m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_cold_sync_rsp_global_policy_config: Decode flatten space failed");
		}
		tmp = ncs_decode_8bit(&dec_path);
		c = (char *)&tmp;
		ncs_dec_skip_space(&dec->i_uba, sizeof(uint8_t));
		str[i] = *c;
		/* Now append this char to str */
		/*strcat(str, c); */
	}
	/* If decoded log_path is not the same, copy it as new log_path */
	if (strcmp(str, cb->log_path) != 0)
		strcpy(cb->log_path, str);

	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	for (count = 0; count < num_of_obj; count++) {
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_global_policy_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (GLOBAL_POLICY **)&gp_ptr, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Decode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_cold_sync_rsp_global_policy_config: Decode failed");
		}
		memset(file_list, '\0', sizeof(DTS_FILE_LIST));
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_file_list_config, &dec->i_uba, EDP_OP_TYPE_DEC,
				   (DTS_FILE_LIST **)&file_list, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Decode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
			/* Removing probable mem leak */
			dev = &dec_gp.device;
			m_DTS_FREE_FILE_LIST(dev);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_decode_cold_sync_rsp_global_policy_config: Decode failed");
		}

		status = dtsv_ckpt_add_rmv_updt_global_policy(cb, gp_ptr, file_list, dec->i_action);

		/* Removing probable mem leak */
		dev = &dec_gp.device;
		m_DTS_FREE_FILE_LIST(dev);

		if (status != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_cold_sync_rsp_dts_async_updt_cnt
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
static uint32_t dtsv_decode_cold_sync_rsp_dts_async_updt_cnt(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec, uint32_t num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTSV_ASYNC_UPDT_CNT *updt_cnt;
	EDU_ERR ederror = 0;

	updt_cnt = &cb->async_updt_cnt;

	m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_UPDT_COUNT);
	/* 
	 * Decode and send async update counts for all the data structures.
	 */
	status = m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_async_updt_cnt,
				&dec->i_uba, EDP_OP_TYPE_DEC, (DTSV_ASYNC_UPDT_CNT **)&updt_cnt, &ederror);

	if (status != NCSCC_RC_SUCCESS) {
		/* Decode failed!!! */
		m_LOG_DTS_CHKOP(DTS_CSYNC_DEC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_cold_sync_rsp_dts_async_updt_cnt: Decode failed");
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_warm_sync_rsp
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
uint32_t dtsv_decode_warm_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTSV_ASYNC_UPDT_CNT *updt_cnt;
	DTSV_ASYNC_UPDT_CNT dec_updt_cnt;
	EDU_ERR ederror = 0;

	updt_cnt = &dec_updt_cnt;
	m_LOG_DTS_CHKOP(DTS_WSYNC_DEC_START);
	/* 
	 * Decode latest async update counts. (In the same manner we received
	 * in the last message of the cold sync response.
	 */
	status = m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_async_updt_cnt,
				&dec->i_uba, EDP_OP_TYPE_DEC, (DTSV_ASYNC_UPDT_CNT **)&updt_cnt, &ederror);

	if (status != NCSCC_RC_SUCCESS) {
		/* Decode failed!!! */
		m_LOG_DTS_CHKOP(DTS_WSYNC_DEC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_warm_sync_rsp: Decode failed");
	}

	m_LOG_DTS_CHKOP(DTS_WSYNC_DEC_COMPLETE);
	/*
	 * Compare the update counts of the Standby with Active.
	 * if they matches return success. If it fails then 
	 * send a data request.
	 */
	if (0 != memcmp(updt_cnt, &cb->async_updt_cnt, sizeof(DTSV_ASYNC_UPDT_CNT))) {
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_WSYNC_ERR, DTS_FC_UPDT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR,
			   "TILLLLLLLL", DTS_WSYNC_DATA_MISMATCH, updt_cnt->dts_svc_reg_tbl_updt,
			   updt_cnt->dta_dest_list_updt, updt_cnt->global_policy_updt, updt_cnt->dts_log_updt,
			   cb->async_updt_cnt.dts_svc_reg_tbl_updt, cb->async_updt_cnt.dta_dest_list_updt,
			   cb->async_updt_cnt.global_policy_updt, cb->async_updt_cnt.dts_log_updt);
		/* Clear service registration tables & dta_dest too */
		m_DTS_LK(&cb->lock);
		dtsv_clear_registration_table(cb);
		m_DTS_UNLK(&cb->lock);
		m_DTS_DBG_SINK(NCSCC_RC_SUCCESS, "dtsv_decode_warm_sync_rsp: Cleaned-up service registration table");

		if (NCSCC_RC_SUCCESS != dtsv_send_data_req(cb)) {
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_warm_sync_rsp: Send Data request failed");
		}

		/* Data response should be identical to cold-sync */
		cb->in_sync = false;
		cb->cold_sync_in_progress = true;
		cb->cold_sync_done = 0;

		status = NCSCC_RC_FAILURE;
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_data_sync_rsp
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
uint32_t dtsv_decode_data_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t num_of_obj;
	uint8_t *ptr = NULL;

	m_LOG_DTS_CHKOP(DTS_DEC_DATA_RESP);
	/*
	 * Since at decode we need to find out how many objects of particular data
	 * type are sent, decode that information at the begining of the message.
	 */
	ptr = ncs_dec_flatten_space(&dec->i_uba, (uint8_t *)&num_of_obj, sizeof(uint32_t));
	if (ptr == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_decode_data_sync_rsp: Decode flatten space failed");
	}
	num_of_obj = ncs_decode_32bit(&ptr);
	ncs_dec_skip_space(&dec->i_uba, sizeof(uint32_t));

	/*
	 * Decode data received in the message.
	 */
	dec->i_action = NCS_MBCSV_ACT_ADD;
	status = dtsv_dec_cold_sync_rsp_data_func_list[dec->i_reo_type] (cb, dec, num_of_obj);
	return status;
}

/****************************************************************************\
 * Function: dtsv_decode_data_req
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
uint32_t dtsv_decode_data_req(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	uint32_t status = NCSCC_RC_SUCCESS;

	/* 
	 * Do nothing here. Any data req will be treated as initiation for 
	 * cold sync.
	 */
	m_LOG_DTS_CHKOP(DTS_DEC_DATA_REQ);
	return status;
}
