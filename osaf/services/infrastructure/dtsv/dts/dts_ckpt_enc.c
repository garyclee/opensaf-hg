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
static uint32_t dtsv_encode_ckpt_null(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t dtsv_encode_ckpt_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t dtsv_encode_ckpt_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t dtsv_encode_ckpt_dts_log_file_ckpt_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc);
static uint32_t dtsv_encode_ckpt_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc);

/* Declaration of static cold sync encode functions */
static uint32_t dtsv_encode_cold_sync_null(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t dtsv_encode_cold_sync_rsp_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t dtsv_encode_cold_sync_rsp_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t dtsv_encode_cold_sync_rsp_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);
static uint32_t dtsv_encode_cold_sync_rsp_dts_async_updt_cnt(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);

/*
 * Function list for encoding the async data.
 * We will jump into this function using the reo_type received 
 * in the encode argument.
 */
const DTSV_ENCODE_CKPT_DATA_FUNC_PTR dtsv_enc_ckpt_data_func_list[DTSV_CKPT_MSG_MAX] = {
	dtsv_encode_ckpt_null,	/*Function to handle enc with reo_type set to 0 */
	dtsv_encode_ckpt_dts_svc_reg_tbl_config,
	dtsv_encode_ckpt_dta_dest_list_config,
	dtsv_encode_ckpt_global_policy_config,
	dtsv_encode_ckpt_dts_log_file_ckpt_config,
	/* 
	 * Messages to update independent fields.
	 */

};

/*
 * Function list for encoding the cold sync response data
 * We will jump into this function using the reo_type received 
 * in the cold sync repsonce argument.
 */
const DTSV_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR dtsv_enc_cold_sync_rsp_data_func_list[] = {
	dtsv_encode_cold_sync_null,
	dtsv_encode_cold_sync_rsp_dts_svc_reg_tbl_config,
	dtsv_encode_cold_sync_rsp_dta_dest_list_config,
	dtsv_encode_cold_sync_rsp_global_policy_config,
	dtsv_encode_cold_sync_rsp_dts_async_updt_cnt
};

/****************************************************************************\
 * Function: dtsv_encode_ckpt_null
 *
 * Purpose:  Error scenario. Nothing to encode. 
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t dtsv_encode_ckpt_null(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	return m_DTS_DBG_SINK(NCSCC_RC_SUCCESS, "dtsv_encode_ckpt_null: Received encode for reo_type of 0. Do nothing");
}

/****************************************************************************\
 * Function: dtsv_encode_cold_sync_null
 *
 * Purpose:  Error scenario. Nothing to encode. 
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uint32_t dtsv_encode_cold_sync_null(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	return m_DTS_DBG_SINK(NCSCC_RC_SUCCESS,
			      "dtsv_encode_cold_sync_null: Received encode for reo_type of 0. Do nothing");
}

/****************************************************************************\
 * Function: dtsv_encode_ckpt_dts_svc_reg_tbl_config
 *
 * Purpose:  Encode entire DTS_SVC_REG_TBL data..
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
static uint32_t dtsv_encode_ckpt_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;
	uint8_t *encoded_cli_map, *encoded_dta_dest;

	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
		/* Send data only */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, &enc->io_uba,
				   EDP_OP_TYPE_ENC, (NCSCONTEXT *)(long)enc->io_reo_hdl, &ederror);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Encode MDS_DEST of DTA to be removed */
		encoded_dta_dest = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint64_t));
		if (!encoded_dta_dest) {
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_encode_ckpt_fls_svc_reg_tbl_config: Encode(DTA MDS_DEST) failed");
		}
		ncs_enc_claim_space(&enc->io_uba, sizeof(uint64_t));
		if (encoded_dta_dest != NULL) {
			ncs_encode_64bit(&encoded_dta_dest, cb->svc_rmv_mds_dest);
		}

	case NCS_MBCSV_ACT_UPDATE:
		/* Send encoded cli_bit_map information */
		encoded_cli_map = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint8_t));
		if (!encoded_cli_map) {
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_encode_ckpt_dts_svc_reg_tbl_config: Encode(cli_bit_map) failed");
		}
		ncs_enc_claim_space(&enc->io_uba, sizeof(uint8_t));

		if (encoded_cli_map != NULL) {
			ncs_encode_8bit(&encoded_cli_map, cb->cli_bit_map);
		}
		/* Now encode the svc entry */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, &enc->io_uba,
				   EDP_OP_TYPE_ENC, (NCSCONTEXT *)(long)enc->io_reo_hdl, &ederror);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}
	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_encode_ckpt_dts_svc_reg_tbl_config: Encode failed");
		/*return status; */
	}

	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_ckpt_dta_dest_list_config
 *
 * Purpose:  Encode entire DTA_DEST_LIST data.
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
static uint32_t dtsv_encode_ckpt_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;
	uint8_t *enc_svc;		/*To encode the svc corspdng to dta being added/deleted */
	uint8_t *enc_spec;		/* To encode the corr. ASCII_SPEC name and version */
	DTA_DEST_LIST *dta;
	SVC_KEY svc_key;

	enc_svc = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint32_t));

	if (enc_svc == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_encode_ckpt_dta_dest_list_config: ncs_enc_reserve_space returns NULL");

	ncs_enc_claim_space(&enc->io_uba, sizeof(uint32_t));

	/* Get the pointer to dta */
	dta = (DTA_DEST_LIST *)(long)enc->io_reo_hdl;

	/*svc added to dta->svc_list is always at the head. So new svc is always 
	   pointed to by dta->svc_list */
	if (dta->svc_list != NULL)
		svc_key = dta->svc_list->svc->my_key;
	else {
		/*In this case, this better be RMV event or you're screwed
		   Put ss_svc_id to 0 */
		svc_key.node = m_NCS_NODE_ID_FROM_MDS_DEST(dta->dta_addr);
		svc_key.ss_svc_id = 0;
	}

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		break;

	case NCS_MBCSV_ACT_RMV:
		/* For remove we don't need svc_key.ss_svc_id, so better have it as 0 */
		svc_key.ss_svc_id = 0;
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	/*Now encode the svc id of the svc associated with DTA to be added/deleted */
	ncs_encode_32bit(&enc_svc, svc_key.ss_svc_id);

	/* Versioning support : Encode the SPEC_CKPT structure to ckpt the spec 
	 * table correspondind to the DTA being added. For removal this doesn't
	 * matter.
	 */
	if (ncs_encode_n_octets_in_uba(&enc->io_uba, (uint8_t *)cb->last_spec_loaded.svc_name, DTSV_SVC_NAME_MAX) !=
	    NCSCC_RC_SUCCESS)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_encode_ckpt_dta_dest_list_config: ncs_encode_n_octets_in_uba returns NULL");

	enc_spec = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint16_t));

	if (enc_spec == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_encode_ckpt_dta_dest_list_config: ncs_enc_reserve_space returns NULL");

	ncs_encode_16bit(&enc_spec, cb->last_spec_loaded.version);

	ncs_enc_claim_space(&enc->io_uba, sizeof(uint16_t));

	/* Now encode the DTA_DEST_LIST using edp */
	status =
	    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dta_dest_list_config, &enc->io_uba, EDP_OP_TYPE_ENC,
			   (NCSCONTEXT *)(long)enc->io_reo_hdl, &ederror);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_encode_ckpt_dta_dest_list_config: Encode failed");
	}

	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_ckpt_dts_file_list_config
 *
 * Purpose:  Encode a particular DTS_FILE_LIST data..
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
static uint32_t dtsv_encode_ckpt_dts_log_file_ckpt_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
	case NCS_MBCSV_ACT_RMV:
		break;
	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	status =
	    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_log_ckpt_data_config, &enc->io_uba, EDP_OP_TYPE_ENC,
			   (NCSCONTEXT *)(long)enc->io_reo_hdl, &ederror);
	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_encode_ckpt_dts_log_file_ckpt_config: Encode failed");
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_ckpt_global_policy_config
 *
 * Purpose:  Encode entire GLOBAL_POLICY data..
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
static uint32_t dtsv_encode_ckpt_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;
	uint8_t *encoded_cli_map;

	/* Smik - Encode DTS_CB->cli_bit__map before encoding global policy */
	encoded_cli_map = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint8_t));
	if (!encoded_cli_map) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_encode_ckpt_global_policy_config: Encode(cli_bit_map) failed");
	}
	ncs_enc_claim_space(&enc->io_uba, sizeof(uint8_t));

	if (encoded_cli_map != NULL) {
		ncs_encode_8bit(&encoded_cli_map, cb->cli_bit_map);
	}

	/* 
	 * Check for the action type (whether it is add, rmv or update) and act
	 * accordingly. If it is update or add, encode entire data. If it is rmv
	 * send key information only.
	 */
	switch (enc->io_action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_UPDATE:
		/* Send entire data */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_global_policy_config, &enc->io_uba, EDP_OP_TYPE_ENC,
				   (NCSCONTEXT *)(long)enc->io_reo_hdl, &ederror);
		break;

	case NCS_MBCSV_ACT_RMV:
		/* Send only key information */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_global_policy_config, &enc->io_uba, EDP_OP_TYPE_ENC,
				   (NCSCONTEXT *)(long)enc->io_reo_hdl, &ederror);
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_ASYNC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_encode_ckpt_global_policy_config: Encode failed");
	}

	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_cold_sync_rsp
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
uint32_t dtsv_encode_cold_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_START);

	return dtsv_encode_all(cb, enc, TRUE);
}

/****************************************************************************\
 * Function: dtsv_encode_all
 *
 * Purpose:  Encode response message for all datastructures.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *        c_sync - TRUE - Called while in cold sync.
 *                 FALSE - Called while in data response.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uint32_t dtsv_encode_all(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, NCS_BOOL csync)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	uint32_t num_of_obj = 0;
	uint8_t *encoded_cnt_loc;

	/* 
	 * Since at decode we need to find out how many objects of particular data
	 * type are being sent, encode the information at the begining of message.
	 */
	encoded_cnt_loc = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint32_t));
	if (!encoded_cnt_loc) {
		/* Handle error */
		m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_encode_cold_sync_rsp: Encode reserve space failed");
	}
	ncs_enc_claim_space(&enc->io_uba, sizeof(uint32_t));

	/* 
	 * If reo_handle and reo_type is NULL then this the first time mbcsv is calling
	 * the cold sync response for the standby. So start from first data structure 
	 * Next time onwards depending on the value of reo_type and reo_handle
	 * send the next data structures.
	 */
	status = dtsv_enc_cold_sync_rsp_data_func_list[enc->io_reo_type] (cb, enc, &num_of_obj);

	/* Now encode the number of objects actually in the UBA. */
	if (encoded_cnt_loc != NULL) {
		ncs_encode_32bit(&encoded_cnt_loc, num_of_obj);
	}

	/*
	 * Check if we have reached to last message required to be sent in cold sync 
	 * response, if yes then send cold sync complete. Else ask MBCSv to call you 
	 * back with the next reo_type.
	 */
	if (DTSV_COLD_SYNC_RSP_ASYNC_UPDT_CNT == enc->io_reo_type) {
		if (csync)
			enc->io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
		else
			enc->io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE;
		m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_COMPLETE);
	} else
		enc->io_reo_type++;

	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_cold_sync_rsp_dts_svc_reg_tbl_config
 *
 * Purpose:  Encode entire DTS_SVC_REG_TBL data..
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
static uint32_t dtsv_encode_cold_sync_rsp_dts_svc_reg_tbl_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTS_SVC_REG_TBL *dts_svc_reg_ptr;
	EDU_ERR ederror = 0;
	SVC_KEY nt_key;
	DTS_FILE_LIST *file_list;

	m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_SVC_REG);
	/* 
	 * Encode the DTS_SVC_REG_TBLs for all services registered.
	 */
	dts_svc_reg_ptr = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&cb->svc_tbl, NULL);
	/* Encode the DTS_SVC_REG_TBL for all the services in the patricia tree */
	while (dts_svc_reg_ptr != NULL) {
		/*if(dts_svc_reg_ptr->my_key.ss_svc_id == 0)
		   {
		   key = dts_svc_reg_ptr->my_key; 
		   dts_svc_reg_ptr =(DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&cb->svc_tbl, (const uint8_t*)&key);
		   } */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config, &enc->io_uba,
				   EDP_OP_TYPE_ENC, dts_svc_reg_ptr, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_encode_cold_sync_rsp_dts_svc_reg_tbl_config: Encode Failed");
		}
		file_list = &dts_svc_reg_ptr->device.log_file_list;
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_file_list_config, &enc->io_uba, EDP_OP_TYPE_ENC,
				   file_list, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_encode_cold_sync_rsp_dts_svc_reg_tbl_config: Encode DTS_FILE_LIST Failed");
		}

		/*  Network order key added */
		nt_key = dts_svc_reg_ptr->ntwk_key;
		dts_svc_reg_ptr = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&cb->svc_tbl, (const uint8_t *)&nt_key);
		(*num_of_obj)++;
		/* Point to the next service entry */
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_cold_sync_rsp_dta_dest_list_config
 *
 * Purpose:  Encode entire DTA_DEST_LIST data..
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
static uint32_t dtsv_encode_cold_sync_rsp_dta_dest_list_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTA_DEST_LIST *dta_dest;
	MDS_DEST dta_key;
	EDU_ERR ederror = 0;
	uint8_t *enc_data;		/*Introduced to send num of svc with a DTA */
	SVC_ENTRY *svc_entry;
	SPEC_ENTRY *spec_entry = NULL;

	m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_DTA_DEST);

	/* Set the dta_key to NULL at the start */
	memset(&dta_key, 0, sizeof(MDS_DEST));
	/* 
	 * Walk through the entire list and send the entire list data.
	 */
	dta_dest = (DTA_DEST_LIST *)ncs_patricia_tree_getnext(&cb->dta_list, NULL);
	while (dta_dest != NULL) {
		dta_dest = (DTA_DEST_LIST *)((long)dta_dest - DTA_DEST_LIST_OFFSET);

		/* Point to the next dta entry */
		dta_key = dta_dest->dta_addr;
		/* First encode the DTA_DEST_LIST with edu */
		status =
		    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dta_dest_list_config, &enc->io_uba, EDP_OP_TYPE_ENC,
				   dta_dest, &ederror);

		if (status != NCSCC_RC_SUCCESS) {
			/* Encode failed!!! */
			m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_FAILED);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_encode_cold_sync_rsp_dta_dest_list_config: Encode failed");
		}
		/* Now encode the num of svcs associated with the DTA */
		enc_data = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint32_t));
		if (enc_data == NULL)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_encode_cold_sync_rsp_dta_dest_list_config: ncs_enc_reserve_space returns NULL");

		ncs_enc_claim_space(&enc->io_uba, sizeof(uint32_t));
		/* Encode dta_dest->num_svcs first */
		ncs_encode_32bit(&enc_data, dta_dest->dta_num_svcs);

		svc_entry = dta_dest->svc_list;
		/* Encode the SS_SVC_IDs here */
		/* Versioning support changes : Also checkpoint the service name &version 
		 * for each of these DTA.
		 */
		while (svc_entry != NULL) {
			enc_data = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint32_t));
			if (enc_data == NULL)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_encode_cold_sync_rsp_dta_dest_list_config: ncs_enc_reserve_space returns NULL");

			ncs_encode_32bit(&enc_data, svc_entry->svc->my_key.ss_svc_id);
			ncs_enc_claim_space(&enc->io_uba, sizeof(uint32_t));

			/* get the corr. SPEC_ENTRY in the svc_reg_tbl and encode it */
			spec_entry = (SPEC_ENTRY *)dts_find_spec_entry(svc_entry->svc, &dta_key);
			if (spec_entry != NULL) {
				enc_data = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint32_t));
				if (enc_data == NULL)
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_encode_cold_sync_rsp_dta_dest_list_config: ncs_enc_reserve_space returns NULL");

				/*Encode 1 here to indicate spec_entry for this DTA to decode side */
				ncs_encode_16bit(&enc_data, 1);
				ncs_encode_16bit(&enc_data, spec_entry->spec_struct->ss_spec->ss_ver);
				ncs_enc_claim_space(&enc->io_uba, sizeof(uint32_t));
				/* Now encode the service name */
				if (ncs_encode_n_octets_in_uba(&enc->io_uba, (uint8_t *)spec_entry->svc_name, DTSV_SVC_NAME_MAX) !=
				    NCSCC_RC_SUCCESS)
					m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						       "dtsv_encode_cold_sync_rsp_dta_dest_list_config: ncs_encode_n_octets_in_uba returns failure");
			} else {
				enc_data = ncs_enc_reserve_space(&enc->io_uba, sizeof(uint16_t));
				if (enc_data == NULL)
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_encode_cold_sync_rsp_dta_dest_list_config: ncs_enc_reserve_space returns NULL");

				/*Encode 1 here to indicate spec_entry for this DTA to decode side */
				ncs_encode_16bit(&enc_data, 0);
				ncs_enc_claim_space(&enc->io_uba, sizeof(uint16_t));
			}
			svc_entry = svc_entry->next_in_dta_entry;
		}

		(*num_of_obj)++;
		dta_dest = (DTA_DEST_LIST *)ncs_patricia_tree_getnext(&cb->dta_list, (const uint8_t *)&dta_key);
	}			/*end of while */
	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_cold_sync_rsp_global_policy_config
 *
 * Purpose:  Encode entire GLOBAL_POLICY data..
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
static uint32_t dtsv_encode_cold_sync_rsp_global_policy_config(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	GLOBAL_POLICY *gp;
	EDU_ERR ederror = 0;
	uint8_t *enc_data;		/*Introduced to encode log directory */
	uint32_t len = 0, i;
	DTS_FILE_LIST *file_list;

	m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_GLOBAL_POLICY);
	len = strlen(cb->log_path);
	/* Start encoding the log directory path */
	enc_data = ncs_enc_reserve_space(&enc->io_uba, (len + 4) * (sizeof(char)));
	if (enc_data == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_encode_cold_sync_rsp_global_policy_config: ncs_enc_reserve_space returns NULL");

	ncs_enc_claim_space(&enc->io_uba, (len + 4) * (sizeof(char)));
	/* Encode len first */
	ncs_encode_32bit(&enc_data, len);
	/* Now encode the chars for log_path */
	for (i = 0; i < len; i++)
		ncs_encode_8bit(&enc_data, cb->log_path[i]);
	/* 
	 * Encode the entire global_policy structure in DTS_CB.
	 */
	gp = &cb->g_policy;

	status =
	    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_global_policy_config, &enc->io_uba, EDP_OP_TYPE_ENC, gp,
			   &ederror);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_encode_cold_sync_rsp_global_policy_config: Encode failed");
	}
	file_list = &cb->g_policy.device.log_file_list;
	status =
	    m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_dts_file_list_config, &enc->io_uba, EDP_OP_TYPE_ENC,
			   file_list, &ederror);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_encode_cold_sync_rsp_global_policy_config: Encode DTS_FILE_LIST failed");
	}

	(*num_of_obj)++;
	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_cold_sync_rsp_dts_async_updt_cnt
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
static uint32_t dtsv_encode_cold_sync_rsp_dts_async_updt_cnt(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_UPDT_COUNT);
	/* 
	 * Encode and send async update counts for all the data structures.
	 */
	status = m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_async_updt_cnt,
				&enc->io_uba, EDP_OP_TYPE_ENC, &cb->async_updt_cnt, &ederror);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_CSYNC_ENC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_encode_cold_sync_rsp_dts_async_updt_cnt: Encode failed");
	}
	/* No need to update num_of_obj here */
	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_warm_sync_rsp
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
uint32_t dtsv_encode_warm_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	EDU_ERR ederror = 0;

	m_LOG_DTS_CHKOP(DTS_WSYNC_ENC_START);
	/* 
	 * Encode and send latest async update counts. (In the same manner we sent
	 * in the last message of the cold sync response.
	 */
	status = m_NCS_EDU_EXEC(&cb->edu_hdl, dtsv_edp_ckpt_msg_async_updt_cnt,
				&enc->io_uba, EDP_OP_TYPE_ENC, &cb->async_updt_cnt, &ederror);

	if (status != NCSCC_RC_SUCCESS) {
		/* Encode failed!!! */
		m_LOG_DTS_CHKOP(DTS_WSYNC_ENC_FAILED);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_encode_warm_sync_rsp: Encode failed");
	}
	m_LOG_DTS_CHKOP(DTS_WSYNC_ENC_COMPLETE);
	return status;
}

/****************************************************************************\
 * Function: dtsv_encode_data_sync_rsp
 *
 * Purpose:  Encode Data sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This function has similar functionality as cold-sync.
 * 
\**************************************************************************/
uint32_t dtsv_encode_data_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc)
{
	m_LOG_DTS_CHKOP(DTS_ENC_DATA_RESP);
	return dtsv_encode_all(cb, enc, FALSE);
}
