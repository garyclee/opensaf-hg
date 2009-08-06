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
   The DTSv Circular Buffer Operation table.

  ..............................................................................

  FUNCTIONS INCLUDED in this module:
  dtsv_cirbuff_op_table_register     - Register CB operation table.
  dtsv_populate_cirbuff_op_tbl_info  - CB operation table information.
  dtsv_populate_cirbuff_op_var_info  - CB op table var info.
  dtsv_cirbuff_op_set_obj            - CB set operation.
  dtsv_cirbuff_op_get_obj            - CB get operation.
  dtsv_extract_cirbuff_op_obj        - CB extract operation.
  dtsv_next_cirbuff_op_obj           - CB next operation.
  dtsv_setrow_cirbuff_op_obj         - CB set-row operation.
  dump_global_buffer_log             - CB - dump global buffer log.
  dump_per_node_buffer_log           - CB - dump per node buffer log.
  dump_svc_per_node_buffer_log       - CB - dump service per node buffer log.
  dts_dump_log_to_op_device          - CB - dump log to output device.
  dts_circular_buffer_alloc          - Allocate circular buffer.
  dts_circular_buffer_free           - Free circular buffer.
  dts_circular_buffer_clear          - Clear circular buffer.
  dts_cir_buff_set_default           - Set circular buffer parameters to default.
  dts_dump_to_cir_buffer             - Dump circular buffer contents.
  dts_buff_size_increased            - Increase the circular buffer size.
  dts_buff_size_decreased            - Decrease circular buffer size.
  dts_dump_buffer_to_buffer          - Dump from src to dst buffer.
******************************************************************************/

#include "dts.h"

/* Static vriable declarations */
static NCSMIB_OBJ_INFO objinfo;
static NCSMIB_VAR_INFO varinfo[dtsvCirBuffOpMax_Id];
static uns16 idx[2];

/* Local routines for this table */
static uns32 dtsv_cirbuff_op_set_obj(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL flag);
static uns32 dtsv_cirbuff_op_get_obj(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

static uns32
dtsv_extract_cirbuff_op_obj(NCSMIB_PARAM_VAL *param_val, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
static uns32
dtsv_next_cirbuff_op_obj(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_len);
static uns32
dtsv_setrow_cirbuff_op_obj(NCSCONTEXT cb, NCSMIB_ARG *args,
			   NCSMIB_SETROW_PARAM_VAL *params, struct ncsmib_obj_info *objinfo, NCS_BOOL flag);

static uns32 dump_global_buffer_log(DTS_CB *inst, uns8 device);
static uns32 dump_per_node_buffer_log(DTS_CB *inst, uns32 node_id, uns8 device);
static uns32 dump_svc_per_node_buffer_log(DTS_CB *inst, SVC_KEY *key, uns8 device);

static void dtsv_populate_cirbuff_op_tbl_info(NCSMIB_TBL_INFO *pInfo);
static void dtsv_populate_cirbuff_op_var_info(NCSMIB_VAR_INFO *pInfo);
/**************************************************************************
 Function: dtsv_cirbuff_op_table_register
 Purpose:  Circular Buffer Operation table register routine
 Input:    void
 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 dtsv_cirbuff_op_table_register(void)
{
	NCSMIBLIB_REQ_INFO reqinfo;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&reqinfo, 0, sizeof(reqinfo));
	memset(&objinfo, 0, sizeof(objinfo));

	/* Populate the Table Information */
	dtsv_populate_cirbuff_op_tbl_info(&objinfo.tbl_info);

	/* Populate the Var Infoamtion */
	objinfo.var_info = varinfo;
	dtsv_populate_cirbuff_op_var_info(objinfo.var_info);

	/* Register the Group table */
	reqinfo.req = NCSMIBLIB_REQ_REGISTER;
	reqinfo.info.i_reg_info.obj_info = &objinfo;
	rc = ncsmiblib_process_req(&reqinfo);
	return rc;
}

/**************************************************************************
 Function: dtsv_populate_cirbuff_op_tbl_info
 Purpose:  This routine popultaes the Circular Buffer Operation table information
 Input:    pInfo: Pointer to NCSMIB_TBL_INFO structure
 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
static void dtsv_populate_cirbuff_op_tbl_info(NCSMIB_TBL_INFO *pInfo)
{
	/* Populate the Index Array */
	idx[0] = dtsvCirbuffOpIndexNode_Id;
	idx[1] = dtsvCirbuffOpIndexService_Id;

	/* Set Table params */
	pInfo->table_id = NCSMIB_TBL_DTSV_CIRBUFF_OP;
	pInfo->table_of_scalars = FALSE;
	pInfo->index_in_this_tbl = TRUE;
	pInfo->idx_data.index_info.num_index_ids = 2;
	pInfo->idx_data.index_info.index_param_ids = idx;
	pInfo->num_objects = dtsvCirBuffOpMax_Id - 1;
	pInfo->row_in_one_data_struct = TRUE;

	/* Set Table function handlers */
	pInfo->set = dtsv_cirbuff_op_set_obj;
	pInfo->get = dtsv_cirbuff_op_get_obj;
	pInfo->next = dtsv_next_cirbuff_op_obj;
	pInfo->setrow = dtsv_setrow_cirbuff_op_obj;
	pInfo->extract = dtsv_extract_cirbuff_op_obj;
}

/**************************************************************************
 Function: dtsv_populate_cirbuff_op_var_info
 Purpose:  This routine populates the Circular Buffer Operation param information
 Input:    pInfo: Pointer to NCSMIB_VAR_INFO structures
 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
static void dtsv_populate_cirbuff_op_var_info(NCSMIB_VAR_INFO *pInfo)
{
	/* dtsvCirbuffOpIndexNode_Id */
	pInfo[dtsvCirbuffOpIndexNode_Id - 1].offset =
	    (uns16)((long)&CIR_BUFFER_OP_TABLE_NULL->my_key.node - (long)CIR_BUFFER_OP_TABLE_NULL);
	pInfo[dtsvCirbuffOpIndexNode_Id - 1].len = sizeof(CIR_BUFFER_OP_TABLE_NULL->my_key.node);
	pInfo[dtsvCirbuffOpIndexNode_Id - 1].access = NCSMIB_ACCESS_READ_WRITE;
	pInfo[dtsvCirbuffOpIndexNode_Id - 1].status = NCSMIB_OBJ_CURRENT;
	pInfo[dtsvCirbuffOpIndexNode_Id - 1].set_when_down = FALSE;
	pInfo[dtsvCirbuffOpIndexNode_Id - 1].fmat_id = NCSMIB_FMAT_INT;
	pInfo[dtsvCirbuffOpIndexNode_Id - 1].obj_type = NCSMIB_OTHER_INT_OBJ_TYPE;

	/* dtsvCirbuffOpIndexService_Id */
	pInfo[dtsvCirbuffOpIndexService_Id - 1].offset =
	    (uns16)((long)&CIR_BUFFER_OP_TABLE_NULL->my_key.ss_svc_id - (long)CIR_BUFFER_OP_TABLE_NULL);
	pInfo[dtsvCirbuffOpIndexService_Id - 1].len = sizeof(CIR_BUFFER_OP_TABLE_NULL->my_key.ss_svc_id);
	pInfo[dtsvCirbuffOpIndexService_Id - 1].access = NCSMIB_ACCESS_READ_WRITE;
	pInfo[dtsvCirbuffOpIndexService_Id - 1].status = NCSMIB_OBJ_CURRENT;
	pInfo[dtsvCirbuffOpIndexService_Id - 1].set_when_down = FALSE;
	pInfo[dtsvCirbuffOpIndexService_Id - 1].fmat_id = NCSMIB_FMAT_INT;
	pInfo[dtsvCirbuffOpIndexService_Id - 1].obj_type = NCSMIB_OTHER_INT_OBJ_TYPE;

	/* dtsvCirbuffOpOperation */
	pInfo[dtsvCirbuffOpOperation - 1].offset =
	    (uns16)((long)&CIR_BUFFER_OP_TABLE_NULL->operation - (long)CIR_BUFFER_OP_TABLE_NULL);
	pInfo[dtsvCirbuffOpOperation - 1].len = sizeof(CIR_BUFFER_OP_TABLE_NULL->operation);
	pInfo[dtsvCirbuffOpOperation - 1].access = NCSMIB_ACCESS_READ_WRITE;
	pInfo[dtsvCirbuffOpOperation - 1].status = NCSMIB_OBJ_CURRENT;
	pInfo[dtsvCirbuffOpOperation - 1].set_when_down = FALSE;
	pInfo[dtsvCirbuffOpOperation - 1].fmat_id = NCSMIB_FMAT_INT;
	pInfo[dtsvCirbuffOpOperation - 1].obj_type = NCSMIB_OTHER_INT_OBJ_TYPE;

	/* dtsvCirbuffOpDevice */
	pInfo[dtsvCirbuffOpOperation - 1].offset =
	    (uns16)((long)&CIR_BUFFER_OP_TABLE_NULL->op_device - (long)CIR_BUFFER_OP_TABLE_NULL);
	pInfo[dtsvCirbuffOpDevice - 1].len = sizeof(CIR_BUFFER_OP_TABLE_NULL->op_device);
	pInfo[dtsvCirbuffOpDevice - 1].access = NCSMIB_ACCESS_READ_WRITE;
	pInfo[dtsvCirbuffOpDevice - 1].status = NCSMIB_OBJ_CURRENT;
	pInfo[dtsvCirbuffOpDevice - 1].set_when_down = FALSE;
	pInfo[dtsvCirbuffOpDevice - 1].fmat_id = NCSMIB_FMAT_INT;
	pInfo[dtsvCirbuffOpDevice - 1].obj_type = NCSMIB_OTHER_INT_OBJ_TYPE;
}

/**************************************************************************
* Function: dtsv_cirbuff_op_set_obj
* Purpose:  Circular Buffer Operation table's set object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer
*           var_info : varinfo pointer
*           flag     : Tells the operation to be performed           
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
static uns32 dtsv_cirbuff_op_set_obj(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL flag)
{
	/* Operation not supported for this table. Only SetRow Operation */
	return NCSCC_RC_FAILURE;
}

/**************************************************************************
* Function: dtsv_cirbuff_op_get_obj
* Purpose:  Circular Buffer Operation table's get object routine
* Input:    cb       : DTS_CB pointer
*           arg      : MIB arg pointer*        
*           data     : Pointer of the data structure   
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
static uns32 dtsv_cirbuff_op_get_obj(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	/* Operation not supported for this table. Only SetRow Operation */
	return NCSCC_RC_FAILURE;
}

/**************************************************************************
* Function: dtsv_extract_cirbuff_op_obj
* Purpose:  Circular Buffer Operation table's extract object value routine
* Input:    param_val: Param pointer
*           var_info : var_info pointer
*           data     : Pointer to the data structure
*           buffer   : Buffer to get the octect data           
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
static uns32
dtsv_extract_cirbuff_op_obj(NCSMIB_PARAM_VAL *param_val, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	/* Operation not supported for this table. Only SetRow Operation */
	return NCSCC_RC_FAILURE;
}

/**************************************************************************
* Function: dtsv_next_cirbuff_op_obj
* Purpose:  Circular Buffer Operation table's next object routine
* Input:    cb    : DTS_CB pointer
*           arg   : Mib arg
*           data  : Pointer to the data structure
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
static uns32
dtsv_next_cirbuff_op_obj(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_len)
{
/* Operation not supported for this table. Only SetRow Operation */
	return NCSCC_RC_FAILURE;
}

/**************************************************************************
 Function: dtsv_setrow_cirbuff_op_obj

 Purpose:  Circular Buffer Operation table's setrow routine

 Input:    cb       : DTS_CB pointer
           arg      : Mib arg pointer
           params   : Poinet to param structure
           objinfo  : Pointer to object info structure
           flag     : Flase to perform setrow operation other wise testrow

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
static uns32
dtsv_setrow_cirbuff_op_obj(NCSCONTEXT cb, NCSMIB_ARG *args,
			   NCSMIB_SETROW_PARAM_VAL *params, struct ncsmib_obj_info *objinfo, NCS_BOOL flag)
{
	DTS_CB *inst = (DTS_CB *)cb;

	SVC_KEY key;
	uns8 operation, op_device = 0;
	uns32 paramid = 0;

	memset(&key, 0, sizeof(SVC_KEY));

	for (paramid = dtsvCirbuffOpIndexNode_Id; paramid < dtsvCirBuffOpMax_Id; paramid++) {
		if (!(params[paramid - 1].set_flag))
			return NCSCC_RC_FAILURE;

		if (paramid == dtsvCirbuffOpIndexNode_Id)
			key.node = params[paramid - 1].param.info.i_int;
		else if (paramid == dtsvCirbuffOpIndexService_Id)
			key.ss_svc_id = params[paramid - 1].param.info.i_int;
		else if (paramid == dtsvCirbuffOpOperation)
			operation = (uns8)params[paramid - 1].param.info.i_int;
		else if (paramid == dtsvCirbuffOpDevice)
			op_device = (uns8)params[paramid - 1].param.info.i_int;
		else
			return NCSCC_RC_FAILURE;
	}

	if ((key.node == 0) && (key.ss_svc_id == 0)) {
		return (dump_global_buffer_log(inst, op_device));
	} else if (key.ss_svc_id == 0) {
		return (dump_per_node_buffer_log(inst, key.node, op_device));
	} else {
		return (dump_svc_per_node_buffer_log(inst, &key, op_device));
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: dump_global_buffer_log

 Purpose:  Circular Buffer Operation. Dump log of global buffer to output
           device (can be file or a console).

 Input:    inst     : DTS_CB pointer
           device   : Output device where log to be dumped.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
static uns32 dump_global_buffer_log(DTS_CB *inst, uns8 device)
{
	char file[250];

	if (((inst->g_policy.g_policy.log_dev & CIRCULAR_BUFFER) != CIRCULAR_BUFFER) ||
	    (inst->g_policy.device.cir_buffer.inuse == FALSE)) {
		return NCSCC_RC_FAILURE;
	}

	if (device == LOG_FILE) {
		if (dts_new_log_file_create(file, NULL, GLOBAL_FILE) == 0)
			return NCSCC_RC_FAILURE;

		if (dts_dump_log_to_op_device(&inst->g_policy.device.cir_buffer, device, file) == NCSCC_RC_SUCCESS) {
			m_LOG_DTS_CBOP(DTS_CBOP_DUMPED, 0, 0);
			return NCSCC_RC_SUCCESS;
		}
	} else {
		if (dts_dump_log_to_op_device(&inst->g_policy.device.cir_buffer, device, NULL) == NCSCC_RC_SUCCESS) {
			m_LOG_DTS_CBOP(DTS_CBOP_DUMPED, 0, 0);
			return NCSCC_RC_SUCCESS;
		}
	}

	return NCSCC_RC_SUCCESS;

}

/**************************************************************************
 Function: dump_per_node_buffer_log
 Purpose:  Circular Buffer Operation. Dump log of per node buffer to output
           device (can be file or a console).

 Input:    inst     : DTS_CB pointer
           node_id  : Node id of the node for which log to be dumped.
         device   : Log output device.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
static uns32 dump_per_node_buffer_log(DTS_CB *inst, uns32 node_id, uns8 device)
{
	DTS_SVC_REG_TBL *node;
	char file[250];
	SVC_KEY key, nt_key;

	key.node = node_id;
	key.ss_svc_id = 0;

	/*  Network order key added */
	nt_key.node = m_NCS_OS_HTONL(node_id);
	nt_key.ss_svc_id = 0;

	if (((node = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl,
							      (const uns8 *)&nt_key)) == NULL) ||
	    ((node->svc_policy.log_dev & CIRCULAR_BUFFER) != CIRCULAR_BUFFER) ||
	    (node->device.cir_buffer.inuse == FALSE)) {
		return NCSCC_RC_FAILURE;
	}

	if (device == LOG_FILE) {
		if (dts_new_log_file_create(file, &key, PER_NODE_FILE) == 0)
			return NCSCC_RC_FAILURE;

		if (dts_dump_log_to_op_device(&node->device.cir_buffer, device, file) == NCSCC_RC_SUCCESS) {
			m_LOG_DTS_CBOP(DTS_CBOP_DUMPED, 0, node_id);
			return NCSCC_RC_SUCCESS;
		}
	} else {
		if (dts_dump_log_to_op_device(&node->device.cir_buffer, device, NULL) == NCSCC_RC_SUCCESS) {
			m_LOG_DTS_CBOP(DTS_CBOP_DUMPED, 0, node_id);
			return NCSCC_RC_SUCCESS;
		}
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: dump_svc_per_node_buffer_log
 Purpose:  Circular Buffer Operation. Dump log of per node buffer to output
           device (can be file or a console).

 Input:    inst     : DTS_CB pointer
           key      : Service key.
         device   : Log output device.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
static uns32 dump_svc_per_node_buffer_log(DTS_CB *inst, SVC_KEY *key, uns8 device)
{
	DTS_SVC_REG_TBL *service;
	char file[250];
	SVC_KEY nt_key;

	/*  Network order key added */
	nt_key.node = m_NCS_OS_HTONL(key->node);
	nt_key.ss_svc_id = m_NCS_OS_HTONL(key->ss_svc_id);

	if (((service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl,
								 (const uns8 *)&nt_key)) == NULL) ||
	    ((service->svc_policy.log_dev & CIRCULAR_BUFFER) != CIRCULAR_BUFFER) ||
	    (service->device.cir_buffer.inuse == FALSE)) {
		return NCSCC_RC_FAILURE;
	}

	if (device == LOG_FILE) {
		if (dts_new_log_file_create(file, key, PER_SVC_FILE) == 0)
			return NCSCC_RC_FAILURE;

		if (dts_dump_log_to_op_device(&service->device.cir_buffer, device, file) == NCSCC_RC_SUCCESS) {
			m_LOG_DTS_CBOP(DTS_CBOP_DUMPED, key->ss_svc_id, key->node);
			return NCSCC_RC_SUCCESS;
		}
	} else {
		if (dts_dump_log_to_op_device(&service->device.cir_buffer, device, NULL) == NCSCC_RC_SUCCESS) {
			m_LOG_DTS_CBOP(DTS_CBOP_DUMPED, key->ss_svc_id, key->node);
			return NCSCC_RC_SUCCESS;
		}
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: dts_dump_log_to_op_device

 Purpose:  Give me pointer to your circular buffer and the device name where 
           you wanna dump, I will dump it for you if everything goes fine.

 Input:    cir_buff : Circular buffer.
           device   : specify the output device where log to be dumped.
           file     : File where to dump.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
**************************************************************************/
uns32 dts_dump_log_to_op_device(CIR_BUFFER *cir_buff, uns8 device, char *file)
{
	uns8 i, num;
	uns32 j;
	FILE *fh;
	uns8 *str = dts_cb.cb_log_str;
	uns8 *ptr;
	uns8 inuse_buff = 0;
	NCS_BOOL found = FALSE;

	if (cir_buff->buff_allocated == FALSE)
		return NCSCC_RC_FAILURE;

	/* First Step : Find the INUSE buffer and then start reading from the next */
	for (i = 0; i < NUM_BUFFS; i++) {
		if (cir_buff->buff_part[i].status == INUSE) {
			found = TRUE;
			inuse_buff = i;
			break;
		}
	}

	/* No buffer found INUSE. So something looks wrong */
	if (found == FALSE)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_dump_log_to_op_device: You should not come to this place. SERIOUS issue. No buffer found to be in use");

	/* Take care of the buffer wrap around case. Start from the next buff. */
	if (++inuse_buff >= NUM_BUFFS)
		inuse_buff = 0;

	num = inuse_buff;
	for (i = 0; i < NUM_BUFFS; i++) {
		/* "num" should be between 0 to --NUM_BUFFS */
		if (num >= NUM_BUFFS)
			num = 0;

		if (cir_buff->buff_part[num].status == CLEAR) {
			num++;
			continue;
		}

		ptr = cir_buff->buff_part[num].cir_buff_ptr;

		if (device == LOG_FILE) {
			if ((fh = sysf_fopen(file, "a+")) != NULL) {
				fprintf(fh, "\n NEW PART \n");

				for (j = 0; j < cir_buff->buff_part[num].num_of_elements; j++) {
					strcpy(str, ptr);
					fprintf(fh, (const char *)str);
					ptr += (strlen(str) + 1);
				}

				fclose(fh);
			} else
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "URGENT: dts_dump_log_to_op_device: Unable to Open FILE. Something is going wrong.");
		} else if (device == OUTPUT_CONSOLE) {
			for (j = 0; j < cir_buff->buff_part[num].num_of_elements; j++) {
				strcpy(str, ptr);
				printf("%s", str);
				ptr += (strlen(str) + 1);
			}
		} else
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dts_dump_log_to_op_device: Device type received is not correct");

		if (cir_buff->buff_part[num].status == INUSE)
			return NCSCC_RC_SUCCESS;

		num++;
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_circular_buffer_alloc

 Purpose:  This function is used for allocating the circular buffer depending
           on its size, if the circular buffer logging is enabled.

 Input:    cir_buff     : Pointer to the circular buffer array.
           buffer_size  : Total buffer size to be used.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_circular_buffer_alloc(CIR_BUFFER *cir_buff, uns32 buffer_size)
{
	uns32 size = ((buffer_size * 1024) / NUM_BUFFS);
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	if (cir_buff->buff_allocated == TRUE)
		return NCSCC_RC_FAILURE;

	/* Allocate a single chunk of memory of size == buffer_size */
	cir_buff->buff_part[0].cir_buff_ptr = m_MMGR_ALLOC_CIR_BUFF(buffer_size * 1024);

	if (cir_buff->buff_part[0].cir_buff_ptr == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_circular_buffer_alloc: Failed to allocate circular buffer");
	memset(cir_buff->buff_part[0].cir_buff_ptr, '\0', (buffer_size * 1024));

	/* 
	 * Now assign all the buffer partitions to default value.
	 * Also, assign addressed to each partition poniter.
	 */
	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].cir_buff_ptr = (cir_buff->buff_part[0].cir_buff_ptr + (size * i));

		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;
	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->inuse = FALSE;
	cir_buff->part_size = size;
	cir_buff->cur_location = cir_buff->buff_part[0].cir_buff_ptr;
	cir_buff->buff_part[0].status = INUSE;
	cir_buff->buff_allocated = TRUE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_circular_buffer_free

 Purpose:  This function is used for freeing the circular buffer depending
           on its size.

 Input:    cir_buff     : Pointer to the circular buffer array.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_circular_buffer_free(CIR_BUFFER *cir_buff)
{
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	if (cir_buff->buff_allocated == FALSE)
		return NCSCC_RC_FAILURE;

	if (0 != cir_buff->buff_part[0].cir_buff_ptr)
		m_MMGR_FREE_CIR_BUFF(cir_buff->buff_part[0].cir_buff_ptr);

	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].cir_buff_ptr = NULL;
		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;

	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->part_size = 0;
	cir_buff->inuse = FALSE;
	cir_buff->cur_location = NULL;
	cir_buff->buff_allocated = FALSE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_circular_buffer_clear

 Purpose:  This function is used for clearing the circular buffer so that
           the next dumped message will start logging a fresh.

 Input:    cir_buff     : Pointer to the circular buffer array.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_circular_buffer_clear(CIR_BUFFER *cir_buff)
{
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	if (cir_buff->buff_allocated == FALSE)
		return NCSCC_RC_FAILURE;

	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;
	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->inuse = FALSE;
	cir_buff->cur_location = cir_buff->buff_part[0].cir_buff_ptr;
	cir_buff->buff_part[0].status = INUSE;
	cir_buff->buff_allocated = TRUE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_cir_buff_set_default

 Purpose:  This function is used for setting to default all the circular buffer
           parameters.

 Input:    cir_buff     : Pointer to the circular buffer array.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_cir_buff_set_default(CIR_BUFFER *cir_buff)
{
	uns32 i;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	for (i = 0; i < NUM_BUFFS; i++) {
		cir_buff->buff_part[i].cir_buff_ptr = NULL;
		cir_buff->buff_part[i].num_of_elements = 0;
		cir_buff->buff_part[i].status = CLEAR;
	}

	cir_buff->cur_buff_num = 0;
	cir_buff->cur_buff_offset = 0;
	cir_buff->inuse = FALSE;
	cir_buff->cur_location = NULL;
	cir_buff->buff_allocated = FALSE;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_dump_to_cir_buffer

 Purpose:  This function is used for allocating the circular buffer depending
           on its size, if the circular buffer logging is enabled.

 Input:    cir_buff : Pointer to the circular buffer array.
           str      : String to be copied into the buffer.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_dump_to_cir_buffer(CIR_BUFFER *cir_buff, char *str)
{
	uns32 str_len = strlen(str) + 1;

	if (TRUE != cir_buff->buff_allocated)
		return NCSCC_RC_FAILURE;

	cir_buff->inuse = TRUE;

	/*
	 * Check whether we are crossing the partition size limit. If yes then 
	 * start dumping on the new partiotion.
	 */
	if ((cir_buff->cur_buff_offset + str_len) > cir_buff->part_size) {
		cir_buff->buff_part[cir_buff->cur_buff_num].status = FULL;

		if (++cir_buff->cur_buff_num >= NUM_BUFFS) {
			cir_buff->cur_buff_num = 0;
		}

		/* We want to dump on new partition so do the basic initialization */
		cir_buff->cur_location = cir_buff->buff_part[cir_buff->cur_buff_num].cir_buff_ptr;
		cir_buff->cur_buff_offset = 0;

		cir_buff->buff_part[cir_buff->cur_buff_num].status = INUSE;
		cir_buff->buff_part[cir_buff->cur_buff_num].num_of_elements = 0;
	}

	/*
	 * So either we are within the range of buffer partion or we are 
	 * dumping on the new partition. So dump the message.
	 * Here if we are still exceeding the partition size, then there is some
	 * problem and we are not able to log this message into circular buffer.
	 * Need to look at this problem so return failure.
	 */
	if ((cir_buff->cur_buff_offset + str_len) > cir_buff->part_size)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_dump_to_cir_buffer: Hmmm!! Looks like your message does not fit into the buffer part. Increase buffer size.");

	/* So everything is set to dump your message in buffer. Good Luck!! */
	strcpy(cir_buff->cur_location, str);
	cir_buff->cur_location += str_len;

	cir_buff->buff_part[cir_buff->cur_buff_num].num_of_elements++;
	cir_buff->cur_buff_offset += str_len;

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************\
 Function: dts_buff_size_increased

 Purpose:  This function is called when circular buffer size is increased.

 Input:    cir_buff : Pointer to the circular buffer array.
           new_size : New size of the buffer.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_buff_size_increased(CIR_BUFFER *cir_buff, uns32 new_size)
{
	CIR_BUFFER tmp_buff = *cir_buff;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	/* Changes to allow SET on buffer size before having to 
	 *            set log device to buffer first.
	 */
	if (dts_cir_buff_set_default(cir_buff) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	if (dts_circular_buffer_alloc(cir_buff, new_size) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*
	 * Check whether the buffer is in use. If yes then only copy from 
	 * old to new buffer. Otherwise there is no point is copying since we won't 
	 * have any messages logged into the buffer.
	 */
	if (tmp_buff.inuse == TRUE) {
		if (dts_dump_buffer_to_buffer(&tmp_buff, cir_buff, NUM_BUFFS) != NCSCC_RC_SUCCESS)
			return NCSCC_RC_FAILURE;
	}

	cir_buff->inuse = tmp_buff.inuse;

	/* Free memory allocated for the old buffer. */
	dts_circular_buffer_free(&tmp_buff);

	return NCSCC_RC_SUCCESS;

}

/**************************************************************************\
 Function: dts_buff_size_decreased

 Purpose:  This function is called when circular buffer size is increased.

 Input:    cir_buff : Pointer to the circular buffer array.
           new_size : New size of the buffer.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
\**************************************************************************/
uns32 dts_buff_size_decreased(CIR_BUFFER *cir_buff, uns32 new_size)
{
	CIR_BUFFER tmp_buff = *cir_buff;
	uns32 i = 0;

	if (cir_buff == NULL)
		return NCSCC_RC_FAILURE;

	/* Changes to allow SET on buffer size before having to 
	 *            set log device to buffer first.
	 */
	if (dts_cir_buff_set_default(cir_buff) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	if (dts_circular_buffer_alloc(cir_buff, new_size) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*
	 * Check whether the buffer is in use. If yes then only copy from 
	 * old to new buffer. Otherwise there is no point is copying since we won't 
	 * have any messages logged into the buffer.
	 */
	if (tmp_buff.inuse == TRUE) {
		/* 
		 * Since buffer size is decreased we are now going to find how many
		 * partitions of old buffer we can copy into the new buffer.
		 * If user decreases the size by less that 1/NUM_BUFFS size then 
		 * we will not able to dump a single message.
		 */
		for (i = 0; i < NUM_BUFFS; i++) {
			if ((tmp_buff.part_size * (i + 1)) > new_size)
				break;
		}

		if (dts_dump_buffer_to_buffer(&tmp_buff, cir_buff, i) != NCSCC_RC_SUCCESS)
			return NCSCC_RC_FAILURE;
	}

	cir_buff->inuse = tmp_buff.inuse;

	/* Free memory allocated for the old buffer. */
	dts_circular_buffer_free(&tmp_buff);

	return NCSCC_RC_SUCCESS;

}

/**************************************************************************\
 Function: dts_dump_buffer_to_buffer

 Purpose:  Give me pointer to your src and dst circular buffer  
           you wanna dump, I will dump it for you if everything goes fine.

 Input:    src_cir_buff : Source Circular buffer.
           dst_cir_buff : Destination circular buffer.
           number       : Number of buffer partitions to be dumped.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 Notes:  
\**************************************************************************/
uns32 dts_dump_buffer_to_buffer(CIR_BUFFER *src_cir_buff, CIR_BUFFER *dst_cir_buff, uns32 number)
{
	uns8 i = 0, num = 0;
	uns32 j = 0;
	uns8 *ptr = NULL;
	uns8 inuse_buff = 0;
	NCS_BOOL found = FALSE;

	if ((src_cir_buff->buff_allocated == FALSE) || (dst_cir_buff->buff_allocated == FALSE))
		return NCSCC_RC_FAILURE;

	if (number == 0)
		return NCSCC_RC_SUCCESS;

	/* First Step : Find the INUSE buffer and then start reading from the next */
	for (i = 0; i < NUM_BUFFS; i++) {
		if (src_cir_buff->buff_part[i].status == INUSE) {
			found = TRUE;
			inuse_buff = i;
			break;
		}
	}

	/* No buffer found INUSE. So something looks wrong? -- You should never hit this case. */
	if (found == FALSE)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "URGENT: dts_dump_to_cir_buffer: No buffer found INUSE. So something looks wrong?");

	/* Take care of the buffer wrap around case. Start from the next buff. */
	if (++inuse_buff >= NUM_BUFFS)
		inuse_buff = 0;

	num = inuse_buff;

	/* 
	 * Since we have to dump only "number" of buffers, we should
	 * skip "(NUM_BUFFS - number)" buffers and start copying 
	 * from the next 
	 */
	for (i = 0; i < (NUM_BUFFS - number); i++) {
		if (++num >= NUM_BUFFS)
			num = 0;
	}

	/* Loop for "NUM_BUFFS" times */
	for (i = 0; i < NUM_BUFFS; i++) {
		/* "num" should be between 0 to --NUM_BUFFS */
		if (num >= NUM_BUFFS)
			num = 0;

		/* If buffer status is clear then no need to dump */
		if (src_cir_buff->buff_part[num].status == CLEAR) {
			num++;
			continue;
		}

		ptr = src_cir_buff->buff_part[num].cir_buff_ptr;

		/* Copy all the elemetns from source to destination buffer. */
		for (j = 0; j < src_cir_buff->buff_part[num].num_of_elements; j++) {
			if (dts_dump_to_cir_buffer(dst_cir_buff, (char *)ptr) != NCSCC_RC_SUCCESS)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "URGENT: dts_dump_to_cir_buffer: Failed to copy to new buffer");

			ptr += (strlen(ptr) + 1);
		}

		if (src_cir_buff->buff_part[num].status == INUSE)
			return NCSCC_RC_SUCCESS;

		num++;
	}

	return NCSCC_RC_SUCCESS;
}
