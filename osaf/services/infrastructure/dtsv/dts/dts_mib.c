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

    This has the mib specific functions of the Flex Log Service (DTS).
    This file contains these groups of private functions

  - Table registration function for registering mib table and filters with OAC
  - OAC callback function for processing MIB requests

  ..............................................................................

  FUNCTIONS INCLUDED in this module:
  ncs_dtsv_mib_request              - High levl entry point
  dts_register_tables               - Registers DTSV tables.
  dts_unregister_table              - Un-registers DLS tables.
  dts_register_table                - Register table.
  dts_unregister_table              - Un-register table.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mab.h"
#include "dts.h"

static uns32 dts_register_table(DTS_CB *inst, uns32 table_id, uns32 (*reg_table) (), uns32 *tbl_hdl);
static uns32 dts_unregister_table(DTS_CB *inst, uns32 table_id, uns32 *tbl_hdl);

/****************************************************************************
  PROCEDURE NAME  : ncs_dtsv_mib_request
  DESCRIPTION     : Hign level entry point into DTSV for MIB requests used if
                    if MIB Access Broker (MAB) is not in use

  ARGUMENTS       : mib_args:  Arguments for request
      
  RETURNS         : NCSCC_RC_SUCCESS
                    NCSCC_RC_FAILURE
  NOTES:
*****************************************************************************/
uns32 ncs_dtsv_mib_request(NCSMIB_ARG *mib_args)
{
	DTS_CB *inst = &dts_cb;
	uns32 rc = NCSCC_RC_SUCCESS;

	if ((mib_args->i_tbl_id >= NCSMIB_TBL_DTSV_BASE) && (mib_args->i_tbl_id <= NCSMIB_TBL_DTSV_END)) {
		if (inst->created == FALSE) {
			mib_args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
			mib_args->i_rsp_fnc(mib_args);
			return NCSCC_RC_FAILURE;
		} else {
			rc = dtsv_policy_tbl_req(mib_args);
		}
	} else
		return NCSCC_RC_FAILURE;

	return rc;
}

/****************************************************************************
  PROCEDURE NAME  : dts_register_tables
  DESCRIPTION     : Registers DTSV MIB table with OAC.

  ARGUMENTS       : inst:  DTS control block
      
  RETURNS         : NCSCC_RC_SUCCESS
                    NCSCC_RC_FAILURE
  NOTES:
*****************************************************************************/
uns32 dts_register_tables(DTS_CB *inst)
{
	/*  REGISTER GLOBAL POLICY TABLE */
	if (dts_register_table(inst, NCSMIB_TBL_DTSV_SCLR_GLOBAL,
			       dtsv_global_policy_table_register, &inst->global_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  REGISTER NODE POLICY TABLE    */
	if (dts_register_table(inst, NCSMIB_TBL_DTSV_NODE,
			       dtsv_node_policy_table_register, &inst->node_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  REGISTER SERVICE PER NODE POLICY TABLE    */
	if (dts_register_table(inst, NCSMIB_TBL_DTSV_SVC_PER_NODE,
			       dtsv_svc_policy_table_register, &inst->svc_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  REGISTER CIRCULAR BUFFER OPERATION TABLE    */
	if (dts_register_table(inst, NCSMIB_TBL_DTSV_CIRBUFF_OP,
			       dtsv_cirbuff_op_table_register, &inst->cirbuff_op_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  REGISTER CLI OP REQ COMMAND TABLE   */
	if (dts_register_table(inst, NCSMIB_TBL_DTSV_CMD, NULL,	/*Don't have any MIBLIB to be registered */
			       &inst->cmd_op_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/* Send MAB_PSR_WARM_BOOT request to PSR, since DTS is a running on
	   a VDEST of its own and it is the only client to the OAC that got
	   started on this VDEST. */
	/* New PSSv Client Name to be passed in a Warmboot-request
	   public API provided by OAA.
	   if(oac_svc_init_done(inst->oac_hdl) != NCSCC_RC_SUCCESS)
	   return NCSCC_RC_FAILURE;
	 */
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME  : dts_unregister_tables
  DESCRIPTION     : Un-Registers DTSV MIB table with OAC.

  ARGUMENTS       : inst:  DTS control block
      
  RETURNS         : NCSCC_RC_SUCCESS
                    NCSCC_RC_FAILURE
  NOTES:
*****************************************************************************/
uns32 dts_unregister_tables(DTS_CB *inst)
{
	/*  UN-REGISTER GLOBAL POLICY TABLE */
	if (dts_unregister_table(inst, NCSMIB_TBL_DTSV_SCLR_GLOBAL, &inst->global_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  UN-REGISTER NODE POLICY TABLE    */
	if (dts_unregister_table(inst, NCSMIB_TBL_DTSV_NODE, &inst->node_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  UN-REGISTER SERVICE PER NODE POLICY TABLE    */
	if (dts_unregister_table(inst, NCSMIB_TBL_DTSV_SVC_PER_NODE, &inst->svc_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  UN-REGISTER CIRCULAR BUFFER OPERATION TABLE    */
	if (dts_unregister_table(inst, NCSMIB_TBL_DTSV_CIRBUFF_OP, &inst->cirbuff_op_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	/*  UN-REGISTER CLI OP REQ COMMAND TABLE   */
	if (dts_unregister_table(inst, NCSMIB_TBL_DTSV_CMD, &inst->cmd_op_tbl_row_hdl) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME  : dts_register_table
  DESCRIPTION     : Registers individual DTSV MIB table with OAC.

  ARGUMENTS       : inst:  DTS control block
      
  RETURNS         : NCSCC_RC_SUCCESS
                    NCSCC_RC_FAILURE
  NOTES:
*****************************************************************************/
uns32 dts_register_table(DTS_CB *inst, uns32 table_id, uns32 (*reg_table) (), uns32 *tbl_hdl)
{
	NCSOAC_SS_ARG dts_arg;

	memset(&dts_arg, 0, sizeof(dts_arg));

	dts_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
	dts_arg.i_tbl_id = table_id;
	dts_arg.i_oac_hdl = inst->oac_hdl;
	dts_arg.info.tbl_owned.i_mib_req = dtsv_policy_tbl_req;
	dts_arg.info.tbl_owned.i_mib_key = (long)inst;

	dts_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_DTSV;

	/* Changes for PSSv integration 
	   Make only GLOBAL/NODE/SERVICE Policy tables persistent
	 */
	if ((table_id >= NCSMIB_TBL_DTSV_SCLR_GLOBAL) && (table_id <= NCSMIB_TBL_DTSV_SVC_PER_NODE)) {
		dts_arg.info.tbl_owned.is_persistent = TRUE;
		dts_arg.info.tbl_owned.i_pcn = "DTS";
	}

	if (ncsoac_ss(&dts_arg) != NCSCC_RC_SUCCESS)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_register_table: Failed to register dtsv policy table");

	if (reg_table != NULL)
		if (reg_table() != NCSCC_RC_SUCCESS)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dts_register_table: Failed to register dtsv policy table");

	dts_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
	dts_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
	dts_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

	if (ncsoac_ss(&dts_arg) != NCSCC_RC_SUCCESS)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_register_table: Failed NCSOAC_SS_OP_ROW_OWNED for dtsv policy table");

	*tbl_hdl = dts_arg.info.row_owned.o_row_hdl;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME  : dts_unregister_table
  DESCRIPTION     : Un-Registers individual DTSV MIB table with OAC.

  ARGUMENTS       : inst:  DTS control block
      
  RETURNS         : NCSCC_RC_SUCCESS
                    NCSCC_RC_FAILURE
  NOTES:
*****************************************************************************/
uns32 dts_unregister_table(DTS_CB *inst, uns32 table_id, uns32 *tbl_hdl)
{
	NCSOAC_SS_ARG dts_arg;

	memset(&dts_arg, 0, sizeof(dts_arg));

	if (*tbl_hdl != 0) {
		dts_arg.i_op = NCSOAC_SS_OP_ROW_GONE;
		dts_arg.i_tbl_id = table_id;
		dts_arg.i_oac_hdl = inst->oac_hdl;
		dts_arg.info.row_gone.i_row_hdl = *tbl_hdl;

		if (ncsoac_ss(&dts_arg) != NCSCC_RC_SUCCESS)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dts_unregister_table: Failed to deregister dtsv policy tables");

		*tbl_hdl = 0;

		dts_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
		dts_arg.info.tbl_gone.i_meaningless = (NCSCONTEXT)0x1234;

		if (ncsoac_ss(&dts_arg) != NCSCC_RC_SUCCESS)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dts_unregister_table: Failed to deregister dtsv policy tables");
	} else
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_unregister_table: Table handle is NULL");

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: dts_mab_snd_warmboot_req
* Purpose:  Sends playback request to PSR
* Input: DTS_CB *
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 dts_mab_snd_warmboot_req(DTS_CB *cb)
{
	NCSOAC_SS_ARG dts_arg;

	memset(&dts_arg, 0, sizeof(NCSOAC_SS_ARG));

	dts_arg.i_oac_hdl = cb->oac_hdl;
	dts_arg.i_op = NCSOAC_SS_OP_WARMBOOT_REQ_TO_PSSV;
	dts_arg.info.warmboot_req.i_pcn = "DTS";
	dts_arg.info.warmboot_req.is_system_client = FALSE;

	if (ncsoac_ss(&dts_arg) != NCSCC_RC_SUCCESS) {
		/* Log error */
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mab_snd_warmboot_req: Playback request failed");
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: dtsv_policy_tbl_req
* Purpose:  High Level MIB Access Routine
* Input: NCSMIB_ARG *
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32 dtsv_policy_tbl_req(NCSMIB_ARG *args)
{
	NCSMIBLIB_REQ_INFO miblib_req;
	NCSMIB_CLI_REQ *cli_req;
	DTS_CB *inst = &dts_cb;
	uns32 status = NCSCC_RC_SUCCESS;

	if (inst->created == FALSE) {
		args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
		args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
		args->i_rsp_fnc(args);
		return NCSCC_RC_FAILURE;
	}

	/* Changes for CLI console configuration */
	if (args->i_op == NCSMIB_OP_REQ_CLI) {
		cli_req = &args->req.info.cli_req;
		/*status = dtsv_process_cli_cmd(inst, args); */
		if (args->i_tbl_id != NCSMIB_TBL_DTSV_CMD) {
			args->rsp.i_status = NCSCC_RC_NO_SUCH_TBL;
			args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
			args->i_rsp_fnc(args);
			return NCSCC_RC_FAILURE;
		} else {
			m_DTS_LK(&inst->lock);
			m_LOG_DTS_LOCK(DTS_LK_LOCKED, &inst->lock);
			switch (cli_req->i_cmnd_id) {
			case dtsvAddGlobalConsole:
				status = dtsv_global_conf_console(inst, args, TRUE);
				break;
			case dtsvRmvGlobalConsole:
				status = dtsv_global_conf_console(inst, args, FALSE);
				break;
			case dtsvDispGlobalConsole:
				status = dtsv_global_disp_conf_console(inst, args);
				break;
			case dtsvAddNodeConsole:
				status = dtsv_node_conf_console(inst, args, TRUE);
				break;
			case dtsvRmvNodeConsole:
				status = dtsv_node_conf_console(inst, args, FALSE);
				break;
			case dtsvDispNodeConsole:
				status = dtsv_node_disp_conf_console(inst, args);
				break;
			case dtsvAddSvcConsole:
				status = dtsv_service_conf_console(inst, args, TRUE);
				break;
			case dtsvRmvSvcConsole:
				status = dtsv_service_conf_console(inst, args, FALSE);
				break;
			case dtsvDispSvcConsole:
				status = dtsv_service_disp_conf_console(inst, args);
				break;

			case dtsvAsciiSpecReload:
				{
					/* Instead of reloading spec in oac cb, post msg to DTS mbx */
					DTSV_MSG *msg;
					msg = m_MMGR_ALLOC_DTSV_MSG;
					if (msg == NULL) {
						m_DTS_UNLK(&inst->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_policy_tbl_req: Unable to allocate memory for reload spec CLI cmd");
					}
					memset(msg, '\0', sizeof(DTSV_MSG));
					msg->msg_type = DTS_SPEC_RELOAD;

					/* Post this message to DTS mailbox */
					if (m_DTS_SND_MSG(&gl_dts_mbx, msg, NCS_IPC_PRIORITY_LOW) != NCSCC_RC_SUCCESS) {
						m_DTS_UNLK(&inst->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
						if (msg != NULL)
							m_MMGR_FREE_DTSV_MSG(msg);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_policy_tbl_req: IPC send failed for spec reload");
					}
					status = NCSCC_RC_SUCCESS;
				}
				break;

			case dtsvPrintCurrConfig:
				{
					/* Instead of displaying current config in oac cb, post msg to DTS mbx */
					DTSV_MSG *msg;
					msg = m_MMGR_ALLOC_DTSV_MSG;
					if (msg == NULL) {
						m_DTS_UNLK(&inst->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_policy_tbl_req: Unable to allocate memory for display config CLI cmd");
					}
					memset(msg, '\0', sizeof(DTSV_MSG));
					msg->msg_type = DTS_PRINT_CONFIG;

					/* Post this message to DTS mailbox */
					if (m_DTS_SND_MSG(&gl_dts_mbx, msg, NCS_IPC_PRIORITY_LOW) != NCSCC_RC_SUCCESS) {
						m_DTS_UNLK(&inst->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
						if (msg != NULL)
							m_MMGR_FREE_DTSV_MSG(msg);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_policy_tbl_req: IPC send failed for display config");
					}
					status = NCSCC_RC_SUCCESS;
				}
				break;

			default:
				/* clear req userbuf if not consumed */
				if (args->req.info.cli_req.i_usrbuf != NULL) {
					m_MMGR_FREE_BUFR_LIST(args->req.info.cli_req.i_usrbuf);
					args->req.info.cli_req.i_usrbuf = NULL;
				}
				m_DTS_UNLK(&inst->lock);
				m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
				return NCSCC_RC_FAILURE;
			}	/*end of case */

			args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
			args->rsp.i_status = status;
			args->rsp.info.cli_rsp.i_cmnd_id = cli_req->i_cmnd_id;

			if (args->i_rsp_fnc(args)) {
				if (args->rsp.info.cli_rsp.o_answer != NULL) {
					m_MMGR_FREE_BUFR_LIST(args->rsp.info.cli_rsp.o_answer);
					args->rsp.info.cli_rsp.o_answer = NULL;
				}
			}
		}		/*end of else */
		m_DTS_UNLK(&inst->lock);
		m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
		return status;
	} /*end of changes for processing NCSMIB_OP_REQ_CLI */
	else {
		memset(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));
		miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
		miblib_req.info.i_mib_op_info.args = args;
		miblib_req.info.i_mib_op_info.cb = inst;

		m_DTS_LK(&inst->lock);
		m_LOG_DTS_LOCK(DTS_LK_LOCKED, &inst->lock);

		/* call the mib routine handler */
		status = ncsmiblib_process_req(&miblib_req);

		m_DTS_UNLK(&inst->lock);
		m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &inst->lock);
	}
	return status;
}

/************************************************************************
*** COMMON MIB FUNCTIONS
*************************************************************************/

/**************************************************************************
 Function: dts_log_policy_change

 Purpose:  Function used for handeling the policy changes. 

 Input:    node      : Registration table entry
           old_plcy  : Old Policy.
           new_plcy  : New policy to be used..

 Returns:  None.

 Notes:  
**************************************************************************/
void dts_log_policy_change(DTS_SVC_REG_TBL *node, POLICY *old_plcy, POLICY *new_plcy)
{
	if (old_plcy->buff_log_fmt != new_plcy->buff_log_fmt)
		node->device.buff_log_fmt_change = TRUE;

	if (old_plcy->file_log_fmt != new_plcy->file_log_fmt)
		node->device.file_log_fmt_change = TRUE;

	if (old_plcy->log_dev != new_plcy->log_dev) {
		dts_log_device_set(new_plcy, &node->device, old_plcy->log_dev);
	}

	if (old_plcy->cir_buff_size != new_plcy->cir_buff_size) {
		dts_buff_size_set(new_plcy, &node->device, old_plcy->cir_buff_size);
	}
}

/**************************************************************************
 Function: dts_filter_policy_change

 Purpose:  Function used for handling the filtering policy changes. 

 Input:    node      : Registration table entry
           old_plcy  : Old Policy.
           new_plcy  : New policy to be used..

 Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.

 Notes:  
**************************************************************************/
void dts_filter_policy_change(DTS_SVC_REG_TBL *node, POLICY *old_plcy, POLICY *new_plcy)
{
	DTS_CB *inst = &dts_cb;
	SVC_KEY nt_key;
	DTS_SVC_REG_TBL *service;

	node->svc_policy.enable = new_plcy->enable;
	node->svc_policy.category_bit_map = new_plcy->category_bit_map;
	node->svc_policy.severity_bit_map = new_plcy->severity_bit_map;

	/* Node filter policy change */
	if (node->my_key.ss_svc_id == 0) {
		nt_key.node = node->ntwk_key.node;
		nt_key.ss_svc_id = 0;

		while (((service = (DTS_SVC_REG_TBL *)dts_get_next_svc_entry(&inst->svc_tbl, &nt_key)) != NULL)
		       && (service->my_key.node == node->my_key.node)) {
			nt_key.ss_svc_id = service->ntwk_key.ss_svc_id;
			service->svc_policy.enable = new_plcy->enable;
			service->svc_policy.category_bit_map = new_plcy->category_bit_map;
			service->svc_policy.severity_bit_map = new_plcy->severity_bit_map;

			dtsv_svc_filtering_policy_change(inst, service, 0, 0, 0);
		}
	}
	/* Service filter change */
	else if (node->dta_count != 0) {
		dtsv_svc_filtering_policy_change(inst, node, 0, 0, 0);
	}

	return;
}

/**************************************************************************
 Function: dts_log_device_set

 Purpose:  Function used for handeling the log device policy changes. 

 Input:    node      : Registration table entry
           old_value : Old value .

 Returns:  None.

 Notes:  
**************************************************************************/
uns32 dts_log_device_set(POLICY *policy, OP_DEVICE *device, uns8 old_value)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* 
	 * Check whether our log device has changed to circular buffer.
	 * If yes then allocate the buffer, if no then free the buffer.
	 */
	if ((policy->log_dev & CIRCULAR_BUFFER) && (!(old_value & CIRCULAR_BUFFER))) {
		rc = dts_circular_buffer_alloc(&device->cir_buffer, policy->cir_buff_size);
	} else if ((old_value & CIRCULAR_BUFFER) && (!(policy->log_dev & CIRCULAR_BUFFER))) {
		rc = dts_circular_buffer_free(&device->cir_buffer);
	}

	/* 
	 * Check whether our log device has changed to Log File.
	 * If yes then set the new log file to true.
	 */

	if ((policy->log_dev & LOG_FILE) && (!(old_value & LOG_FILE))) {
		device->new_file = TRUE;

		/*if (TRUE == device->file_open)
		   {
		   fclose(device->svc_fh);
		   device->file_open = FALSE;
		   } */
	} else if ((!(policy->log_dev & LOG_FILE)) && (old_value & LOG_FILE)) {
		device->new_file = TRUE;
		/*if (TRUE == device->file_open)
		   {
		   fclose(device->svc_fh);
		   device->file_open = FALSE;
		   } */
	}

	return rc;
}

/**************************************************************************
 Function: dts_buff_size_set

 Purpose:  Function used for handeling the changes in the buffer size. 

 Input:    node      : Registration table entry
           old_value : Old value .

 Returns:  None.

 Notes:  
**************************************************************************/
uns32 dts_buff_size_set(POLICY *policy, OP_DEVICE *device, uns32 old_value)
{
	/* Check whether log device is already set to buffer or not.
	 *            If not then return success, no need to call the buff_size 
	 *            increase/decrease functions.
	 */
	if (device->cir_buffer.buff_allocated == FALSE)
		return NCSCC_RC_SUCCESS;
	else {
		if (old_value < policy->cir_buff_size) {
			if (dts_buff_size_increased(&device->cir_buffer, policy->cir_buff_size) != NCSCC_RC_SUCCESS)
				return NCSCC_RC_FAILURE;
		} else {
			if (dts_buff_size_decreased(&device->cir_buffer, policy->cir_buff_size) != NCSCC_RC_SUCCESS)
				return NCSCC_RC_FAILURE;
		}
	}
	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: dtsv_extract_policy_obj
* Purpose:  Policy table's extract object value routine
* Input:    param_val: Param pointer
*           var_info : var_info pointer
*           data     : Pointer to the data structure
*           buffer   : Buffer to get the octect data           
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:  
**************************************************************************/
uns32
dtsv_extract_policy_obj(NCSMIB_PARAM_VAL *param_val, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	uns32 nworder;
	param_val->i_fmat_id = var_info->fmat_id;
	param_val->i_length = var_info->len;

	switch (var_info->obj_type) {
	case NCSMIB_INT_RANGE_OBJ_TYPE:
	case NCSMIB_INT_DISCRETE_OBJ_TYPE:
	case NCSMIB_OTHER_INT_OBJ_TYPE:
	case NCSMIB_TRUTHVAL_OBJ_TYPE:
		switch (var_info->len) {
		case sizeof(uns8):
			param_val->info.i_int = (uns32)(*((uns8 *)((uns8 *)data + var_info->offset)));
			break;
		case sizeof(uns16):
			param_val->info.i_int = (uns32)(*((uns16 *)((uns8 *)data + var_info->offset)));
			break;
		case sizeof(uns32):
			param_val->info.i_int = (uns32)(*((uns32 *)((uns8 *)data + var_info->offset)));
			break;
		default:
			break;
		}
		break;

	case NCSMIB_OCT_OBJ_TYPE:
		/*case NCSMIB_OCT_DISCRETE_OBJ_TYPE: */
	case NCSMIB_OTHER_OCT_OBJ_TYPE:
		if (var_info->obj_spec.stream_spec.min_len == var_info->obj_spec.stream_spec.max_len) {
			memcpy((uns8 *)buffer, ((uns8 *)data + var_info->offset), var_info->len);
		} else {
			uns32 tmp;
			memcpy((uns8 *)&tmp, ((uns8 *)data + var_info->offset), var_info->len);
			nworder = htonl(tmp);
			memcpy((uns8 *)buffer, (uns8 *)&nworder, var_info->len);
		}

		param_val->info.i_oct = (uns8 *)buffer;

		break;

	default:
		return NCSCC_RC_FAILURE;
	}			/* switch */

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: dtsv_policy_set_oct 
* Purpose:  SET operation for MIBS of type BITS for all policies.
*           
* Input     arg      : NCSMIB_ARG pointer
*           data     : NCSCONTEXT, pointer to data to be SET.
*
* Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
* Notes:
**************************************************************************/
uns32 dtsv_policy_set_oct(NCSMIB_ARG *arg, NCSCONTEXT data)
{
	NCSMIB_PARAM_VAL *param_val = &arg->req.info.set_req.i_param_val;
	uns32 rc = NCSCC_RC_SUCCESS;

	switch (param_val->i_length) {
	case sizeof(uns8):
		*(uns8 *)data = *(uns8 *)param_val->info.i_oct;
		break;

	case sizeof(uns16):
		*(uns16 *)data = *(uns16 *)param_val->info.i_oct;
		break;

	case sizeof(uns32):
		*(uns32 *)data = *(uns32 *)param_val->info.i_oct;
		break;

	default:
		rc = NCSCC_RC_FAILURE;
	}
	return rc;
}
