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

  DESCRIPTION:
   The DTSv Global Policy table.
  ..............................................................................

  FUNCTIONS INCLUDED in this module:

  dtsv_global_policy_table_register       - Register Global Policy Table.
  dtsv_policy_tbl_req                     - Policy table request.
  ncsdtsvscalars_set                      - Global policy set operation.
  ncsdtsvscalars_get                      - Global Policy get operation.
  ncsdtsvscalars_next                     - Global Policy Next operation.
  ncsdtsvscalars_setrow                   - Global Policy set-row operation.
  ncsdtsvscalars_extract                  - Global Policy extract operation.
  dtsv_global_filtering_policy_change     - Global filter policy change.

******************************************************************************/

#include "dts.h"

/* Local routines for this table */
static uns32 dtsv_global_filtering_policy_change(DTS_CB *inst, NCSMIB_PARAM_ID param_id);

/**************************************************************************
 Function: dtsv_global_policy_table_register
 Purpose:  Global Policy table register routine
 Input:    void
 Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 dtsv_global_policy_table_register(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	rc = ncsdtsvscalars_tbl_reg();

	return rc;
}

/**************************************************************************
* Function: ncsdtsvscalars_set 
* Purpose:  Global Policy table's set object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*           var_info : varinfo pointer
*           flag     : Tells the operation to be performed           
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 ncsdtsvscalars_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL flag)
{
	DTS_CB *inst = (DTS_CB *)cb;
	NCS_BOOL sameval = FALSE;
	NCSMIBLIB_REQ_INFO reqinfo;
	NCSMIB_SET_REQ *set_req = &arg->req.info.set_req;
	uns32 rc = NCSCC_RC_SUCCESS, old_buff_size = 0;
	uns8 old_log_device = 0;
	uns32 old_num_log_files = 0;
	DTS_SVC_REG_TBL *node;
	OP_DEVICE *device;
	DTS_LOG_CKPT_DATA data;

	memset(&reqinfo, 0, sizeof(reqinfo));
	strcpy(data.file_name, "");

	if (flag)
		return rc;

	/* DTS_PSSV integration - Check for BITS type MIBs.
	 *                        If yes, then don't call miblib_req, call
	 *                        dts specific function instead.
	 */
	switch (set_req->i_param_val.i_param_id) {
	case ncsDtsvGlobalLogDevice_ID:
		old_log_device = inst->g_policy.g_policy.log_dev;
		/* Check for valid value of log device */
		/* First check for null i_oct for the above-mentined check */
		if (set_req->i_param_val.info.i_oct == NULL)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_global_policy_set_obj: Incorrect value of length for SET reuest received - 0");
		else if (*(uns8 *)set_req->i_param_val.info.i_oct < DTS_LOG_DEV_VAL_MAX) {
			if ((*(uns8 *)set_req->i_param_val.info.i_oct == 0) && (old_log_device != 0))
				m_LOG_DTS_EVT(DTS_LOG_DEV_NONE, 0, 0, 0);
			else if (*(uns8 *)set_req->i_param_val.info.i_oct != 0)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_global_policy_set_obj: Log Device bit map beyond permissible values");
		}
	case ncsDtsvGlobalCategoryBitMap_ID:
	case ncsDtsvGlobalSeverityBitMap_ID:
		/* Call DTS's own SET functionality for BITS datatype */
		rc = dtsv_global_policy_set_oct(cb, arg, &inst->g_policy.g_policy);
		/* Jump to dts_handle_global_param_set, no need to call miblib_req */
		goto post_set;

	case ncsDtsvGlobalCloseOpenFiles_ID:
		dts_close_opened_files();
		return rc;

	case ncsDtsvGlobalCircularBuffSize_ID:
		old_buff_size = inst->g_policy.g_policy.cir_buff_size;
		break;
	case ncsDtsvGlobalNumOfLogFiles_ID:
		old_num_log_files = inst->g_policy.g_num_log_files;
		break;
	}

	/* DO the SET operation */
	reqinfo.req = NCSMIBLIB_REQ_SET_UTIL_OP;
	reqinfo.info.i_set_util_info.data = (NCSCONTEXT)&inst->g_policy;
	reqinfo.info.i_set_util_info.param = &set_req->i_param_val;
	reqinfo.info.i_set_util_info.same_value = &sameval;
	reqinfo.info.i_set_util_info.var_info = var_info;
	rc = ncsmiblib_process_req(&reqinfo);

 post_set:
	if (NCSCC_RC_SUCCESS != rc)
		return rc;

	/* Do not proceed further if value to be set is same as that of the 
	 * already existing value 
	 */

	/* Do processing specific to the param objects after set */
	switch (set_req->i_param_val.i_param_id) {
	case ncsDtsvGlobalMessageLogging_ID:
		if (sameval)
			return rc;
		inst->g_policy.device.new_file = TRUE;

		dts_circular_buffer_clear(&inst->g_policy.device.cir_buffer);
		m_LOG_DTS_CBOP(DTS_CBOP_CLEARED, 0, 0);

		/* Update the cli_bit_map field in DTS_CB */
		inst->cli_bit_map = set_req->i_param_val.i_param_id;
		if (NCSCC_RC_SUCCESS != dtsv_global_filtering_policy_change(inst, set_req->i_param_val.i_param_id))
			return NCSCC_RC_FAILURE;
		break;

	case ncsDtsvGlobalLogDevice_ID:
		if (sameval)
			return rc;

		rc = dts_log_device_set(&inst->g_policy.g_policy, &inst->g_policy.device, old_log_device);
		break;

	case ncsDtsvGlobalLogFileSize_ID:
		break;

	case ncsDtsvGlobalFileLogCompFormat_ID:
		if (sameval)
			return rc;
		inst->g_policy.device.file_log_fmt_change = TRUE;
		break;

	case ncsDtsvGlobalCircularBuffSize_ID:
		if (sameval)
			return rc;
		rc = dts_buff_size_set(&inst->g_policy.g_policy, &inst->g_policy.device, old_buff_size);
		break;

	case ncsDtsvGlobalCirBuffCompFormat_ID:
		if (sameval)
			return rc;
		inst->g_policy.device.buff_log_fmt_change = TRUE;
		break;

	case ncsDtsvGlobalCategoryBitMap_ID:
		inst->g_policy.g_policy.category_bit_map = ntohl(inst->g_policy.g_policy.category_bit_map);
		/* Smik - update the cli_bit_map field in DTS_CB */
		inst->cli_bit_map = set_req->i_param_val.i_param_id;
		if (NCSCC_RC_SUCCESS != dtsv_global_filtering_policy_change(inst, set_req->i_param_val.i_param_id))
			return NCSCC_RC_FAILURE;
		break;

	case ncsDtsvGlobalLoggingState_ID:
	case ncsDtsvGlobalSeverityBitMap_ID:
		/* Smik - update the cli_bit_map field in DTS_CB */
		inst->cli_bit_map = set_req->i_param_val.i_param_id;
		if (NCSCC_RC_SUCCESS != dtsv_global_filtering_policy_change(inst, set_req->i_param_val.i_param_id))
			return NCSCC_RC_FAILURE;
		break;

	case ncsDtsvGlobalNumOfLogFiles_ID:
		{
			if (sameval)
				return rc;

			if (old_num_log_files < inst->g_policy.g_num_log_files)
				break;

			/* Remove log files for global level logging */
			device = &inst->g_policy.device;
			while (m_DTS_NUM_LOG_FILES(device) > inst->g_policy.g_num_log_files) {
				m_DTS_RMV_FILE(device);
				data.key.node = 0;
				data.key.ss_svc_id = 0;
				m_DTSV_SEND_CKPT_UPDT_ASYNC(&dts_cb, NCS_MBCSV_ACT_RMV, NCS_PTR_TO_UNS64_CAST(&data),
							    DTSV_CKPT_DTS_LOG_FILE_CONFIG);
			}

			/* Remove log files for node & service level logging */
			node = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&inst->svc_tbl, NULL);
			while (node != NULL) {
				device = &node->device;
				while (m_DTS_NUM_LOG_FILES(device) > inst->g_policy.g_num_log_files) {
					m_DTS_RMV_FILE(device);
					data.key = node->my_key;
					m_DTSV_SEND_CKPT_UPDT_ASYNC(&dts_cb, NCS_MBCSV_ACT_RMV, (long)&data,
								    DTSV_CKPT_DTS_LOG_FILE_CONFIG);
				}

				node =
				    (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&inst->svc_tbl,
										 (const uns8 *)&node->ntwk_key);
			}
		}
		break;

	case ncsDtsvGlobalLogMsgSequencing_ID:
		if (sameval)
			return rc;
		if (inst->g_policy.g_enable_seq == NCS_SNMP_TRUE)
			dts_enable_sequencing(inst);
		else
			dts_disable_sequencing(inst);
		break;

	default:
		break;
	}

	if (rc != NCSCC_RC_FAILURE) {
		/* Smik - Send Async update to stby DTS */
		m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_UPDATE, (long)&inst->g_policy,
					    DTSV_CKPT_GLOBAL_POLICY_CONFIG);
	}

	/* Re-set cli_bit_map */
	inst->cli_bit_map = 0;

	return rc;
}

/**************************************************************************
* Function: ncsdtsvscalars_get 
* Purpose:  Global Policy table's get object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer*        
*           data     : Pointer of the data structure   
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 ncsdtsvscalars_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	DTS_CB *inst = (DTS_CB *)cb;

	/* Set the GET data pointer */
	*data = (NCSCONTEXT)&inst->g_policy;
	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ncsdtsvscalars_next 
* Purpose:  Global Policy table's next object routine
* Input:    cb    : DTS_CB pointer
*           arg   : Mib arg
*           data  : Pointer to the data structure
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 ncsdtsvscalars_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_len)
{
	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: ncsdtsvscalars_setrow 
 Purpose:  Global Policy table's setrow routine
 Input:    cb       : DTS_CB pointer
           arg      : Mib arg pointer
           params   : Poinet to param structure
           objinfo  : Pointer to object info structure
           flag     : Flase to perform setrow operation other wise testrow
 Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32
ncsdtsvscalars_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
		      NCSMIB_SETROW_PARAM_VAL *params, struct ncsmib_obj_info *objinfo, NCS_BOOL flag)
{
	return NCSCC_RC_FAILURE;
}

/**************************************************************************
 Function: dtsv_global_filtering_policy_change

 Purpose:  Change in the Global Filtering policy will affect the entire system
           This function will set the filtering policies of all the nodes and 
           all the services which are currently present in the node and the 
           service registration table respectively. Also, policy change will
           be sent to all the services.

 Input:    cb       : DTS_CB pointer
           param_id : Parameter for which change is done.

 Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

 Notes:  
**************************************************************************/
static uns32 dtsv_global_filtering_policy_change(DTS_CB *inst, NCSMIB_PARAM_ID param_id)
{
	SVC_KEY nt_key;
	DTS_SVC_REG_TBL *service;
	DTA_DEST_LIST *dta;
	DTA_ENTRY *dta_entry;

	/* Search through registration table, Set all the policies,
	 * configure all the DTA's using this policy. 
	 */
	service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&inst->svc_tbl, NULL);
	while (service != NULL) {
		/* Setup key for new search */
		/*  Network order key added */
		nt_key = service->ntwk_key;

		/* Set the Node Policy as per the Global Policy */
		if (param_id == ncsDtsvGlobalLoggingState_ID)
			service->svc_policy.enable = inst->g_policy.g_policy.enable;
		else if (param_id == ncsDtsvGlobalCategoryBitMap_ID)
			service->svc_policy.category_bit_map = inst->g_policy.g_policy.category_bit_map;
		else if (param_id == ncsDtsvGlobalSeverityBitMap_ID)
			service->svc_policy.severity_bit_map = inst->g_policy.g_policy.severity_bit_map;
		else if (param_id == ncsDtsvGlobalMessageLogging_ID) {
			service->device.new_file = TRUE;
			dts_circular_buffer_clear(&service->device.cir_buffer);
			m_LOG_DTS_CBOP(DTS_CBOP_CLEARED, service->my_key.ss_svc_id, service->my_key.node);
		} else
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_global_filtering_policy_change: Wrong param id passed to this function. Need to debug flow.");

		/* Configure services for this change */
		if ((service->my_key.ss_svc_id != 0) && (param_id != ncsDtsvGlobalMessageLogging_ID)) {
			dta_entry = service->v_cd_list;
			while (dta_entry != NULL) {
				dta = dta_entry->dta;
				dts_send_filter_config_msg(inst, service, dta);
				dta_entry = dta_entry->next_in_svc_entry;
			}
		}

		service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&inst->svc_tbl, (const uns8 *)&nt_key);
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: dtsv_global_conf_console
* Purpose:  Global Policy's console configuration
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*           flag     : Tells the operation to be performed.
*                      TRUE is add, FALSE is remove.           
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 dtsv_global_conf_console(DTS_CB *cb, NCSMIB_ARG *arg, NCS_BOOL flag)
{
	OP_DEVICE *dev = &cb->g_policy.device;
	uns8 bit_map = 0, str_len;
	NCS_UBAID uba;
	USRBUF *buf = arg->req.info.cli_req.i_usrbuf;
	uns8 data_buff[DTSV_CLI_MAX_SIZE] = "", *buf_ptr = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&uba, '\0', sizeof(uba));

	ncs_dec_init_space(&uba, buf);
	if (flag == TRUE)
		buf_ptr = ncs_dec_flatten_space(&uba, data_buff, DTSV_ADD_GBL_CONS_SIZE);
	else
		buf_ptr = ncs_dec_flatten_space(&uba, data_buff, DTSV_RMV_GBL_CONS_SIZE);

	if (buf_ptr == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: ncs_dec_flatten_space returns NULL");

	/* Decode the severity bit_map and strlen passed */
	if (flag == TRUE) {
		bit_map = ncs_decode_8bit(&buf_ptr);
		ncs_dec_skip_space(&uba, sizeof(uns8));
	}
	str_len = ncs_decode_8bit(&buf_ptr);
	ncs_dec_skip_space(&uba, sizeof(uns8));

	/* Now decode console device, to be kept in data_buff array */
	if (ncs_decode_n_octets_from_uba(&uba, (char *)&data_buff, str_len) != NCSCC_RC_SUCCESS) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dtsv_global_conf_console: ncs_decode_n_octets_from_uba failed");
	}

	if (flag == TRUE) {
		m_DTS_ADD_CONS(cb, dev, (char *)&data_buff, bit_map);
	} else {
		if (strcmp((char *)&data_buff, "all") == 0) {
			m_DTS_RMV_ALL_CONS(dev);
		} else {
			m_DTS_RMV_CONS(cb, dev, (char *)&data_buff);
		}
	}

	return rc;
}

/**************************************************************************
* Function: dtsv_global_disp_conf_console
* Purpose:  Displays global policy's currently configured consoles
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*
* Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 dtsv_global_disp_conf_console(DTS_CB *cb, NCSMIB_ARG *arg)
{
	OP_DEVICE *dev = &cb->g_policy.device;
	uns8 str_len, bit_map;
	NCS_UBAID rsp_uba;
	DTS_CONS_LIST *cons_ptr = dev->cons_list_ptr;
	uns8 *buff_ptr = NULL, def_num_cons = 1;
	uns32 count;

	memset(&rsp_uba, '\0', sizeof(rsp_uba));

	/* Set parameters for response */
	arg->rsp.info.cli_rsp.i_cmnd_id = dtsvDispGlobalConsole;
	arg->rsp.info.cli_rsp.o_partial = FALSE;

	if (ncs_enc_init_space(&rsp_uba) != NCSCC_RC_SUCCESS)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_global_conf_console: userbuf init failed");

	if (cons_ptr == NULL) {
		/* encode number of devices(which is 1 in this case) */
		buff_ptr = ncs_enc_reserve_space(&rsp_uba, sizeof(uns8));
		if (buff_ptr == NULL)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_global_conf_console: reserve space for encoding failed");

		ncs_encode_8bit(&buff_ptr, def_num_cons);
		ncs_enc_claim_space(&rsp_uba, sizeof(uns8));

		/* Now encode the serial device associated with DTS_CB */
		str_len = strlen((char *)cb->cons_dev);
		bit_map = cb->g_policy.g_policy.severity_bit_map;

		buff_ptr = ncs_enc_reserve_space(&rsp_uba, DTSV_RSP_CONS_SIZE);
		if (buff_ptr == NULL)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_global_conf_console: reserve space for encoding failed");

		ncs_encode_8bit(&buff_ptr, bit_map);
		ncs_encode_8bit(&buff_ptr, str_len);
		ncs_enc_claim_space(&rsp_uba, DTSV_RSP_CONS_SIZE);

		if (ncs_encode_n_octets_in_uba(&rsp_uba, (char *)cb->cons_dev, str_len) != NCSCC_RC_SUCCESS)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_global_conf_console: encode_n_octets_in_uba for serial console failed");

		/* Add uba to rsp's usrbuf */
		arg->rsp.info.cli_rsp.o_answer = rsp_uba.start;
	} /*end of if cons_ptr==NULL */
	else {
		/* Encode number of console devices */
		buff_ptr = ncs_enc_reserve_space(&rsp_uba, sizeof(uns8));
		if (buff_ptr == NULL)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_global_conf_console: reserve space for encoding failed");

		ncs_encode_8bit(&buff_ptr, dev->num_of_cons_conf);
		ncs_enc_claim_space(&rsp_uba, sizeof(uns8));

		/* Now encode individual console devices */
		for (count = 0; count < dev->num_of_cons_conf; count++) {
			/* Encode the serial device associated with global policy */
			str_len = strlen(cons_ptr->cons_dev);
			bit_map = cons_ptr->cons_sev_filter;
			buff_ptr = ncs_enc_reserve_space(&rsp_uba, DTSV_RSP_CONS_SIZE);
			if (buff_ptr == NULL)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_global_conf_console: reserve space for encoding failed");

			ncs_encode_8bit(&buff_ptr, bit_map);
			ncs_encode_8bit(&buff_ptr, str_len);
			ncs_enc_claim_space(&rsp_uba, DTSV_RSP_CONS_SIZE);

			if (ncs_encode_n_octets_in_uba(&rsp_uba, cons_ptr->cons_dev, str_len) != NCSCC_RC_SUCCESS)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_global_conf_console: encode_n_octets_in_uba for console failed");

			/* Point to next device */
			cons_ptr = cons_ptr->next;
		}		/*end of for */

		/* Add uba to rsp's usrbuf */
		arg->rsp.info.cli_rsp.o_answer = rsp_uba.start;
	}			/*end of else */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: ncsdtsvscalars_rmvrow
 *
 * Purpose:  This function is one of the RMVROW processing routines for objects
 * in AVSV_SCALARS_ID table.
 *
 * Input:  cb        - DTS control block.
 *         idx       - pointer to NCSMIB_IDX
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 **************************************************************************/
uns32 ncsdtsvscalars_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
	/* Assign defaults for your globals here. */
	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ncsdtsvscalars_extract 
* Purpose:  Policy table's extract object value routine
* Input:    param_val: Param pointer
*           var_info : var_info pointer
*           data     : Pointer to the data structure
*           buffer   : Buffer to get the octect data
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 ncsdtsvscalars_extract(NCSMIB_PARAM_VAL *param_val, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	if ((param_val->i_param_id == ncsDtsvGlobalLogDevice_ID)
	    || (param_val->i_param_id == ncsDtsvGlobalCategoryBitMap_ID)
	    || (param_val->i_param_id == ncsDtsvGlobalSeverityBitMap_ID)) {
		return dtsv_global_extract_oct(param_val, var_info, data, buffer);
	} else {
		return dtsv_extract_policy_obj(param_val, var_info, data, buffer);
	}
}

/**************************************************************************
* Function: dtsv_global_policy_set_oct 
* Purpose:  SET operation for MIBS of type BITS for global policies.
*           
* Input:    cb       : DTS_CB pointer
*           arg      : NCSMIB_ARG pointer
*           policy   : POLICY pointer
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_global_policy_set_oct(DTS_CB *cb, NCSMIB_ARG *arg, POLICY *policy)
{
	NCSMIB_PARAM_VAL *param_val = &arg->req.info.set_req.i_param_val;
	NCSCONTEXT data = NULL;

	if (param_val->i_param_id == ncsDtsvGlobalCategoryBitMap_ID)
		data = &policy->category_bit_map;
	else if (param_val->i_param_id == ncsDtsvGlobalSeverityBitMap_ID)
		data = &policy->severity_bit_map;
	else if (param_val->i_param_id == ncsDtsvGlobalLogDevice_ID)
		data = &policy->log_dev;

	return dtsv_policy_set_oct(arg, data);
}

/**************************************************************************
* Function: dtsv_global_extract_oct 
* Purpose:  EXTRACT operation for MIBS of type BITS for global policies.
*           
* Input:    param_val    : NCSMIB_PARAM_VAL pointer
*           data         : DTS_SVC_REG_TBL pointer
*           buffer       : Buffer pointer
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_global_extract_oct(NCSMIB_PARAM_VAL *param_val,
			      NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	GLOBAL_POLICY *gp = (GLOBAL_POLICY *)data;
	uns32 tmp = 0, nworder = 0;

	param_val->i_fmat_id = var_info->fmat_id;
	param_val->i_length = var_info->len;

	switch (param_val->i_param_id) {
	case ncsDtsvGlobalLogDevice_ID:
		param_val->i_length = sizeof(uns8);
		/*memcpy((uns8*)&tmp, (uns8*)&gp->g_policy.log_dev, param_val->i_length); */
		param_val->info.i_oct = (uns8 *)&gp->g_policy.log_dev;
		break;

	case ncsDtsvGlobalCategoryBitMap_ID:
		param_val->i_length = sizeof(uns32);
		memcpy((uns8 *)&tmp, (uns8 *)&gp->g_policy.category_bit_map, param_val->i_length);
		nworder = htonl(tmp);
		memcpy((uns8 *)buffer, (uns8 *)&nworder, param_val->i_length);
		param_val->info.i_oct = (uns8 *)buffer;
		break;

	case ncsDtsvGlobalSeverityBitMap_ID:
		param_val->i_length = sizeof(uns8);
		/*memcpy((uns8*)&tmp, (uns8*)&gp->g_policy.severity_bit_map, param_val->i_length); */
		param_val->info.i_oct = (uns8 *)&gp->g_policy.severity_bit_map;
		break;

	default:
		return NCSCC_RC_FAILURE;
	}

	/*nworder = htonl(tmp);
	   memcpy((uns8*)buffer, (uns8*)&nworder, param_val->i_length); */

	/*param_val->info.i_oct = (uns8*)buffer; */

	return rc;
}
