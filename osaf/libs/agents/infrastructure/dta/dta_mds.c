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

  This file contains DTA specific MDS routines.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  dta_mds_grcv   - rcv something from MDS lower layer.
  dta_mds_rcv    - rcv something from MDS lower layer.
  dta_mds_subevt - subscription event entry point
  dta_mds_enc    - MDS encode function
  dta_mds_dec    - MDS decode function
  dta_mds_cpy    - MDS copy function
  dta_copy_octets- Copy octets.
  dta_log_msg_encode - Encodes message to be logged.
  encode_ip_address - Function encodes IP address
  dta_svc_reg_check - Function to check for entry in SVC_REG_TBL.
  dta_svc_reg_check - Function to update SVC_REG_TBL.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "dta.h"
#include "ncssysf_mem.h"

DTA_CB dta_cb;

/****************************************************************************
 * Name          : dta_get_ada_hdl
 *
 * Description   : This function Gets the Handles of local MDS destination
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dta_get_ada_hdl(void)
{
	NCSADA_INFO ada_info;

	memset(&ada_info, 0, sizeof(ada_info));

	ada_info.req = NCSADA_GET_HDLS;

	if (ncsada_api(&ada_info) != NCSCC_RC_SUCCESS) {
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_get_ada_hdl: DTA get ADEST handle failed.");
	}

	/* Store values returned by ADA */
	dta_cb.mds_hdl = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
	dta_cb.dta_dest = ada_info.info.adest_get_hdls.o_adest;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : dta_mds_install_and_subscribe
 *
 * Description   : This function registers the DTA Service with MDS.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dta_mds_install_and_subscribe(void)
{
	NCSMDS_INFO mds_info;
	MDS_SVC_ID svc_ids_array[2];

	if (dta_cb.mds_hdl == 0) {
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
				      "dta_mds_install_and_subscribe: MDS handle is NULL. Need to debug.");
	}

	memset(&mds_info, 0, sizeof(mds_info));

	/* Do common stuff */
	mds_info.i_mds_hdl = dta_cb.mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_DTA;

   /*****************************************************\
                   STEP: Install with MDS 
   \*****************************************************/
	mds_info.i_op = MDS_INSTALL;
	mds_info.info.svc_install.i_mds_q_ownership = FALSE;
	mds_info.info.svc_install.i_svc_cb = dta_mds_callback;
	mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)(long)&dta_cb;
	mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;	/* Total PWE scope */

	/* versioning changes - set version no. with which DTA is installing */
	mds_info.info.svc_install.i_mds_svc_pvt_ver = dta_cb.dta_mds_version;

	if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS) {
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
				      "dta_mds_install_and_subscribe: DTA : MDS install and subscribe failed.");
	}

   /*****************************************************\
                   STEP: Subscribe to service(s)
   \*****************************************************/
	mds_info.i_op = MDS_SUBSCRIBE;
	mds_info.info.svc_subscribe.i_num_svcs = 1;
	mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	svc_ids_array[0] = NCSMDS_SVC_ID_DTS;
	mds_info.info.svc_subscribe.i_svc_ids = svc_ids_array;

	if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS) {
		dta_mds_uninstall();
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
				      "dta_mds_install_and_subscribe: MDS service subscription failed.");
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : dta_mds_uninstall
 *
 * Description   : This function un-registers the DTA Service with MDS.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dta_mds_uninstall(void)
{
	NCSMDS_INFO arg;
	uns32 rc;

	/* Un-install your service into MDS. */
	memset(&arg, 0, sizeof(NCSMDS_INFO));

	arg.i_mds_hdl = dta_cb.mds_hdl;
	arg.i_svc_id = NCSMDS_SVC_ID_DTA;
	arg.i_op = MDS_UNINSTALL;

	rc = ncsmds_api(&arg);

	if (rc != NCSCC_RC_SUCCESS) {
		return rc;
	}

	return rc;
}

/****************************************************************************
 * Name          : dta_mds_callback
 *
 * Description   : Call back function provided to MDS. MDS will call this
 *                 function for enc/dec/cpy/rcv/svc_evt operations.
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dta_mds_callback(NCSMDS_CALLBACK_INFO *cbinfo)
{
	uns32 status = NCSCC_RC_SUCCESS;

	switch (cbinfo->i_op) {
	case MDS_CALLBACK_COPY:
		status = dta_mds_cpy(cbinfo->i_yr_svc_hdl, cbinfo->info.cpy.i_msg,
				     cbinfo->info.cpy.i_to_svc_id,
				     &cbinfo->info.cpy.o_cpy,
				     cbinfo->info.cpy.i_last,
				     cbinfo->info.cpy.i_rem_svc_pvt_ver, &cbinfo->info.cpy.o_msg_fmt_ver);
		break;

	case MDS_CALLBACK_ENC:
		/* Treating both encode types as same */
		status = dta_mds_enc(cbinfo->i_yr_svc_hdl, cbinfo->info.enc.i_msg,
				     cbinfo->info.enc.i_to_svc_id,
				     cbinfo->info.enc.io_uba,
				     cbinfo->info.enc.i_rem_svc_pvt_ver, &cbinfo->info.enc.o_msg_fmt_ver);
		break;

	case MDS_CALLBACK_ENC_FLAT:
		/* Treating both encode types as same */
		status = dta_mds_enc(cbinfo->i_yr_svc_hdl, cbinfo->info.enc_flat.i_msg,
				     cbinfo->info.enc_flat.i_to_svc_id,
				     cbinfo->info.enc_flat.io_uba,
				     cbinfo->info.enc_flat.i_rem_svc_pvt_ver, &cbinfo->info.enc_flat.o_msg_fmt_ver);
		break;

	case MDS_CALLBACK_DEC:
		/* Treating both decode types as same */
		status = dta_mds_dec(cbinfo->i_yr_svc_hdl, &cbinfo->info.dec.o_msg,
				     cbinfo->info.dec.i_fr_svc_id,
				     cbinfo->info.dec.io_uba, cbinfo->info.dec.i_msg_fmt_ver);
		break;

	case MDS_CALLBACK_DEC_FLAT:
		/* Treating both decode types as same */
		status = dta_mds_dec(cbinfo->i_yr_svc_hdl, &cbinfo->info.dec_flat.o_msg,
				     cbinfo->info.dec_flat.i_fr_svc_id,
				     cbinfo->info.dec_flat.io_uba, cbinfo->info.dec_flat.i_msg_fmt_ver);
		break;

	case MDS_CALLBACK_RECEIVE:
		status = dta_mds_rcv(cbinfo->i_yr_svc_hdl, cbinfo->info.receive.i_msg);
		break;

	case MDS_CALLBACK_SVC_EVENT:
		dta_mds_evt(cbinfo->info.svc_evt, cbinfo->i_yr_svc_hdl);

		status = NCSCC_RC_SUCCESS;
		break;

	default:
		status = m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
					"dta_mds_callback: MDS call back is called with the wrong operation type.");
		break;
	}

	return status;
}

/****************************************************************************
 * Name          : dta_svc_reg_config
 *
 * Description   : Function used for configuring the service after receiving 
 *                 the registration confirmation message from the DTS.
 *
 * Arguments     : inst : DTA control block.
 *                 msg  : Registration confirmation message received from the DTS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dta_svc_reg_config(DTA_CB *inst, DTSV_MSG *msg)
{
	REG_TBL_ENTRY *svc;

	m_DTA_LK(&inst->lock);
	/* 
	 * Check whether the service is already registered with the DTA
	 * If NO, then return failre.
	 * If Yes,then configure the service the the received filtering policies.
	 */
	if ((svc = (REG_TBL_ENTRY *)ncs_patricia_tree_get(&inst->reg_tbl, (const
									   uns8 *)&((DTSV_MSG *)msg)->data.data.
							  reg_conf.msg_fltr.svc_id)) == NULL) {
		m_DTA_UNLK(&inst->lock);
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
				      "dta_svc_reg_config: Failed to configure service. Service does not exist in registration table");
	}

	/* No need of policy handles */
	/*svc->policy_hdl = ((DTSV_MSG*)msg)->data.data.reg_conf.msg_fltr.policy_hdl; */
	svc->category_bit_map = ((DTSV_MSG *)msg)->data.data.reg_conf.msg_fltr.category_bit_map;
	svc->severity_bit_map = ((DTSV_MSG *)msg)->data.data.reg_conf.msg_fltr.severity_bit_map;
	svc->enable_log = ((DTSV_MSG *)msg)->data.data.reg_conf.msg_fltr.enable_log;

	svc->log_msg = TRUE;

	/* Smik - Set the service flag to FALSE */
	svc->svc_flag = FALSE;

	m_DTA_UNLK(&inst->lock);
	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : dta_svc_reg_updt
 *
 * Description   : Function used for updating the service after receiving 
 *                 the decoded DTS_FAIL_OVER message from the DTS.
 *
 * Arguments     : inst : DTA control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dta_svc_reg_updt(DTA_CB *inst, uns32 svc_id, uns32 enable_log, uns32 category_bit_map, uns8 severity_bit_map)
{
	REG_TBL_ENTRY *svc;

	m_DTA_LK(&inst->lock);
	/* Check whether the service is already registered with the DTA
	 * If NO, then do nothing.
	 * If Yes,then update the service with the received filtering policies and
	 * service handle*/
	if ((svc = (REG_TBL_ENTRY *)ncs_patricia_tree_get(&inst->reg_tbl, (const uns8 *)&svc_id)) == NULL) {
		/*Smik - Service not found in DTA */
		m_DTA_UNLK(&inst->lock);
		return NCSCC_RC_SUCCESS;
	}
	/*svc->policy_hdl = policy_hdl; */
	svc->enable_log = enable_log;
	svc->category_bit_map = category_bit_map;
	svc->severity_bit_map = severity_bit_map;
	/* Smik - mark the svc entry by setting the flag to true */
	svc->svc_flag = TRUE;
	/* Set log_msg to TRUE to indicate new Act DTS has the svc_reg entry */
	svc->log_msg = TRUE;
	m_DTA_UNLK(&inst->lock);

	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : dta_svc_reg_check
 *
 * Description   : Function used for checking whether all the services 
 *                 after receiving the decoded DTS_FAIL_OVER message from 
 *                 the DTS have new service handles or not.
 *
 * Arguments     : inst : DTA control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 dta_svc_reg_check(DTA_CB *inst)
{
	REG_TBL_ENTRY *svc;
	DTSV_MSG msg;

	m_DTA_LK(&inst->lock);

	/*Traverse the patricia tree for each service reg entry */
	svc = (REG_TBL_ENTRY *)ncs_patricia_tree_getnext(&inst->reg_tbl, NULL);
	while (svc != NULL) {
		/* Smik - if flag is not TRUE, send reg req to DTS. Else set to FALSE */
		if (svc->svc_flag != TRUE) {
			svc->category_bit_map = 0;
			svc->severity_bit_map = 0;
			svc->enable_log = FALSE;
			/*svc->policy_hdl = 0; */

			/* 
			 * Check whether DTS exist. If DTS does not exist then we will send 
			 * registration information to DTS at later time when DTS comes up. 
			 * For now we are done. So return success.
			 */
			if (inst->dts_exist == FALSE) {
				m_DTA_UNLK(&inst->lock);
				return NCSCC_RC_SUCCESS;
			}

			/* 
			 * DTS is up ... Send registration message to DTS. 
			 */
			dta_fill_reg_msg(&msg, svc->svc_id, svc->version, svc->svc_name, DTA_REGISTER_SVC);

#if (DTA_FLOW == 1)
			if (dta_mds_sync_send(&msg, inst, DTA_MDS_SEND_TIMEOUT, TRUE) != NCSCC_RC_SUCCESS)
#else
			if (dta_mds_sync_send(&msg, inst, DTA_MDS_SEND_TIMEOUT) != NCSCC_RC_SUCCESS)
#endif
			{
				m_DTA_UNLK(&inst->lock);
				return m_DTA_DBG_SINK_SVC(NCSCC_RC_FAILURE,
							  "dta_reg_svc: MDS sync send failed", svc->svc_id);
			}
		}
		/* Smik - Set the flag to false for future receipt of DTS_FAIL_OVER msg */
		svc->svc_flag = FALSE;
		svc = (REG_TBL_ENTRY *)ncs_patricia_tree_getnext(&inst->reg_tbl, (const uns8 *)&svc->svc_id);
	}			/*end of while */

	m_DTA_UNLK(&inst->lock);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Name          : dta_mds_rcv
 *
 * Description   : DTA MDS receive call back function.
 *
 * Arguments     : yr_svc_hdl : DTA service handle.
 *                 msg        : Message received from the DTS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 dta_mds_rcv(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg)
{
	DTA_CB *inst = (DTA_CB *)(long)yr_svc_hdl;
	REG_TBL_ENTRY *svc;
	uns32 status = NCSCC_RC_SUCCESS;
	NCS_UBAID *uba;
	uns8 severity_bit_map, data_buff[DTSV_DTS_DTA_MSG_HDR_SIZE];
	uns8 *data;
	uns32 svc_count, count, svc_id, enable_log, category_bit_map;
	int warning_rmval = 0;

	if (inst->created == FALSE) {
		if (((DTSV_MSG *)msg)->data.data.msg.log_msg.uba.ub != NULL)
			m_MMGR_FREE_BUFR_LIST(((DTSV_MSG *)msg)->data.data.msg.log_msg.uba.ub);
		m_MMGR_FREE_DTSV_MSG((DTSV_MSG *)msg);
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_rcv: DTA does not exist. First create DTA.");
	}

	m_DTA_LK(&inst->lock);

	switch (((DTSV_MSG *)msg)->msg_type) {
	case DTS_SVC_REG_CONF:
		/* Send indication to DTA mbx to send buffered logs to DTS now */
		if(m_DTA_SND_MSG(&gl_dta_mbx, msg, NCS_IPC_PRIORITY_HIGH) != NCSCC_RC_SUCCESS) {
			m_DTA_UNLK(&inst->lock);
			m_MMGR_FREE_DTSV_MSG(msg);
			fprintf(stderr, "dta_mds_evt: Send to DTA msgbox failed");
			break;
		} 
		m_DTA_UNLK(&inst->lock);
		return NCSCC_RC_SUCCESS;;
		break;

	case DTS_SVC_MSG_FLTR:
		{
			/* Check whether the service is registered with the DTA
			 * If yes,configure the service. 
			 * If No, then return failre since the service is not registered
			 */
			/*if ((svc = (REG_TBL_ENTRY *) ncs_find_item(&inst->reg_tbl, 
			   (NCSCONTEXT)&((DTSV_MSG*)msg)->data.data.msg_fltr.svc_id, 
			   dta_match_service)) == NULL) */
			if ((svc = (REG_TBL_ENTRY *)ncs_patricia_tree_get(&inst->reg_tbl,
									  (const uns8 *)&((DTSV_MSG *)msg)->data.data.
									  msg_fltr.svc_id)) == NULL) {
				status =
				    m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
						   "dta_mds_rcv: Failed to configure service. Service does not exist in registration table");
				break;
			}

			svc->category_bit_map = ((DTSV_MSG *)msg)->data.data.msg_fltr.category_bit_map;
			svc->severity_bit_map = ((DTSV_MSG *)msg)->data.data.msg_fltr.severity_bit_map;
			svc->enable_log = ((DTSV_MSG *)msg)->data.data.msg_fltr.enable_log;

			break;
		}
	case DTS_FAIL_OVER:
		{
			warning_rmval =
			    m_DTA_DBG_SINK(NCSCC_RC_SUCCESS, "dta_mds_rcv : Message received from newly Act DTS.");

			uba = &((DTSV_MSG *)msg)->data.data.msg.log_msg.uba;
			ncs_dec_init_space(uba, uba->start);
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
			if (data == NULL) {
				if (((DTSV_MSG *)msg)->data.data.msg.log_msg.uba.ub != NULL)
					m_MMGR_FREE_BUFR_LIST(((DTSV_MSG *)msg)->data.data.msg.log_msg.uba.ub);
				m_MMGR_FREE_DTSV_MSG((DTSV_MSG *)msg);
				m_DTA_UNLK(&inst->lock);
				return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
						      "dta_mds_rcv :ncs_dec_flatten_space returns NULL");
			}
			/* Smik - get the count of services */
			svc_count = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(uns32));

			/* Smik - Now reserve the required space */
			data = ncs_dec_flatten_space(uba, data_buff, svc_count * DTSV_REG_CONF_MSG_SIZE);
			if (data == NULL) {
				if (((DTSV_MSG *)msg)->data.data.msg.log_msg.uba.ub != NULL)
					m_MMGR_FREE_BUFR_LIST(((DTSV_MSG *)msg)->data.data.msg.log_msg.uba.ub);
				m_MMGR_FREE_DTSV_MSG((DTSV_MSG *)msg);
				m_DTA_UNLK(&inst->lock);
				return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
						      "dta_mds_rcv :ncs_dec_flatten_space returns NULL");
			}
			for (count = 0; count < svc_count; count++) {
				svc_id = ncs_decode_32bit(&data);
				enable_log = ncs_decode_32bit(&data);
				category_bit_map = ncs_decode_32bit(&data);
				severity_bit_map = ncs_decode_8bit(&data);
				/* No need of policy handles */
				/*policy_hdl = ncs_decode_32bit(&data); */
				/*Update the DTA's svc_reg table */
				status = dta_svc_reg_updt(inst, svc_id, enable_log, category_bit_map, severity_bit_map);
				ncs_dec_skip_space(uba, DTSV_REG_CONF_MSG_SIZE);
			}
			ncs_dec_skip_space(uba, svc_count * DTSV_REG_CONF_MSG_SIZE);

			/* Smik - Check for services without new service handlers */
			status = dta_svc_reg_check(inst);
			break;
		}
	default:
		status = m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
					"dta_mds_rcv: Wrong message type received in MDS receive callback.");
	}

	m_MMGR_FREE_DTSV_MSG((DTSV_MSG *)msg);

	m_DTA_UNLK(&inst->lock);

	return NCSCC_RC_SUCCESS;;

}

/****************************************************************************\
 * Name          : dta_mds_evt
 *
 * Description   : DTA is informed when MDS events occurr that he has subscribed to.
 *
 * Arguments     : svc_info   : Service event Info.
 *                 yr_svc_hdl : DTA service handle.
 *
 * Return Values : None.
 *
 * Notes         : None.
\*****************************************************************************/
void dta_mds_evt(MDS_CALLBACK_SVC_EVENT_INFO svc_info, MDS_CLIENT_HDL yr_svc_hdl)
{
	DTA_CB *inst = (DTA_CB *)(long)yr_svc_hdl;
	uns32 retval;
	int warning_rmval = 0;

	if (inst->created == FALSE) {
		retval = m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_evt: DTA instance does not exist");
		return;
	}

	m_DTA_LK(&inst->lock);

	switch (svc_info.i_change) {	/* Review change type */
	case NCSMDS_NO_ACTIVE:
		/* Do nothing. Wait for MDS_DOWN */
		break;

	case NCSMDS_DOWN:
		/* Make changes to the MAS list and corresponding MDS handles */
		if (svc_info.i_svc_id == NCSMDS_SVC_ID_DTS) {
			/* Stop sending the log messages to DTS */
			inst->dts_exist = FALSE;
		}

		break;

	case NCSMDS_NEW_ACTIVE:
		{
			/* Send registration information, start sending the log messages. */
			inst->dts_exist = TRUE;
			inst->dts_dest = svc_info.i_dest;
			inst->dts_node_id = svc_info.i_node_id;
			inst->dts_pwe_id = svc_info.i_pwe_id;
			inst->act_dts_ver = svc_info.i_rem_svc_pvt_ver;
		}
		break;

	case NCSMDS_UP:
		{
			if (svc_info.i_svc_id == NCSMDS_SVC_ID_DTS) {
				/*NCS_Q_ITR     itr; */
				REG_TBL_ENTRY *svc;
				DTSV_MSG msg;

				/* Send registration information again.....start sending the log messages. */
				inst->dts_exist = TRUE;
				inst->dts_dest = svc_info.i_dest;
				inst->dts_node_id = svc_info.i_node_id;
				inst->dts_pwe_id = svc_info.i_pwe_id;
				inst->act_dts_ver = svc_info.i_rem_svc_pvt_ver;

				m_DTA_UNLK(&inst->lock);
				/* We want to register all the services with DTS. So unlock to 
				 * avoid dead lock situation and then do the MDS send for all the 
				 * services 
				 */
				svc = (REG_TBL_ENTRY *)ncs_patricia_tree_getnext(&inst->reg_tbl, NULL);
				while (svc != NULL) {
					dta_fill_reg_msg(&msg, svc->svc_id, svc->version, svc->svc_name,
							 DTA_REGISTER_SVC);

					if (dta_mds_async_send(&msg, inst) != NCSCC_RC_SUCCESS) {
						warning_rmval =
						    m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
								   "dta_mds_evt: MDS async send failed.");
					}
					svc =
					    (REG_TBL_ENTRY *)ncs_patricia_tree_getnext(&inst->reg_tbl,
										       (const uns8 *)&svc->svc_id);
				}

				m_DTA_LK(&inst->lock);

				/* Send indication to DTA mbx to send buffered logs to DTS now */
				{
					DTSV_MSG *msg = NULL;
					msg = m_MMGR_ALLOC_DTSV_MSG;
					if (msg == NULL) {
						m_DTA_UNLK(&inst->lock);
						warning_rmval =
						    m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
								   "ncs_logmsg: Memory allocation failed for DTSV_MSG for sending buffered messages");
						return;
					}
					memset(msg, '\0', sizeof(DTSV_MSG));
					msg->msg_type = DTS_UP_EVT;
					/* Now post this msg on the DTA mbx */
					if (m_DTA_SND_MSG(&gl_dta_mbx, msg, NCS_IPC_PRIORITY_HIGH) != NCSCC_RC_SUCCESS) {
						m_DTA_UNLK(&inst->lock);
						m_MMGR_FREE_DTSV_MSG(msg);
						warning_rmval =
						    m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
								   "dta_mds_evt: Send to DTA msgbox failed");
						return;
					}
				}

				m_DTA_UNLK(&inst->lock);
				return;
			}
		}
		break;

	default:
		break;
	}

	m_DTA_UNLK(&inst->lock);
}

/****************************************************************************\
 * Name          : dta_mds_enc
 *
 * Description   : Encode a DTA message headed out
 *
 * Arguments     : yr_svc_hdl : DTA service handle.
 *                 msg        : Message received from the DTS.
 *                 to_svc     : Service ID to send.
 *                 uba        : User Buffer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 dta_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
		  SS_SVC_ID to_svc, NCS_UBAID *uba,
		  MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver)
{
	uns8 *data;
	DTSV_MSG *mm;
	uns32 lenn = 0;

	if (uba == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_enc : user buffer is NULL");

	mm = (DTSV_MSG *)msg;

	if (mm == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_enc : Message received is NULL");

	/* Versioning changes - Set message version to 1. Message format version is 
	 * set to 2 only when fmat type 'D' is encoded. Check is done later under 
	 * the switch case for DTA_LOG_MSG 
	 */
	*msg_fmat_ver = 1;

	data = ncs_enc_reserve_space(uba, DTSV_DTA_DTS_HDR_SIZE);
	if (data == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_enc: ncs_enc_reserve_space returns NULL");

	ncs_encode_8bit(&data, mm->msg_type);

	ncs_enc_claim_space(uba, DTSV_DTA_DTS_HDR_SIZE);

	switch (mm->msg_type) {
	case DTA_REGISTER_SVC:
		{
			data = ncs_enc_reserve_space(uba, sizeof(SS_SVC_ID));
			ncs_encode_32bit(&data, mm->data.data.reg.svc_id);
			ncs_enc_claim_space(uba, sizeof(SS_SVC_ID));

			/* Now encode the version for the service */
			data = ncs_enc_reserve_space(uba, sizeof(uns16));
			ncs_encode_16bit(&data, mm->data.data.reg.version);
			ncs_enc_claim_space(uba, sizeof(uns16));

			/* Now encode the service name for the service */
			if (*mm->data.data.reg.svc_name != '\0') {
				lenn = strlen(mm->data.data.reg.svc_name) + 1;
				data = ncs_enc_reserve_space(uba, sizeof(uns32));
				if (data == NULL) {
					return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
							      "dta_mds_enc: Reserve space failed while encoding service registration message");
				}
				ncs_encode_32bit(&data, lenn);
				ncs_enc_claim_space(uba, sizeof(uns32));
				ncs_encode_n_octets_in_uba(uba, (uns8 *)mm->data.data.reg.svc_name, lenn);
			} else {
				data = ncs_enc_reserve_space(uba, sizeof(uns32));
				if (data == NULL) {
					return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
							      "dta_mds_enc: Reserve space failed while encoding service registration message");
				}
				ncs_encode_32bit(&data, lenn);
				ncs_enc_claim_space(uba, sizeof(uns32));
			}

			return NCSCC_RC_SUCCESS;
		}
	case DTA_UNREGISTER_SVC:
		{
			data = ncs_enc_reserve_space(uba, sizeof(SS_SVC_ID));
			ncs_encode_32bit(&data, mm->data.data.unreg.svc_id);
			ncs_enc_claim_space(uba, sizeof(SS_SVC_ID));

			/* Now encode the version for the service */
			data = ncs_enc_reserve_space(uba, sizeof(uns16));
			ncs_encode_16bit(&data, mm->data.data.unreg.version);
			ncs_enc_claim_space(uba, sizeof(uns16));

			/* Now encode the service name for the service */
			if (*mm->data.data.unreg.svc_name != '\0') {
				lenn = strlen(mm->data.data.unreg.svc_name) + 1;
				data = ncs_enc_reserve_space(uba, sizeof(uns32));
				if (data == NULL) {
					return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
							      "dta_mds_enc: Reserve space failed while encoding service registration message");
				}
				ncs_encode_32bit(&data, lenn);
				ncs_enc_claim_space(uba, sizeof(uns32));
				ncs_encode_n_octets_in_uba(uba, (uns8 *)mm->data.data.unreg.svc_name, lenn);
			} else {
				data = ncs_enc_reserve_space(uba, sizeof(uns32));
				if (data == NULL) {
					return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
							      "dta_mds_enc: Reserve space failed while encoding service registration message");
				}
				ncs_encode_32bit(&data, lenn);
				ncs_enc_claim_space(uba, sizeof(uns32));
			}

			return NCSCC_RC_SUCCESS;
		}
	case DTA_LOG_DATA:
		{

			/* Support for old code */
			/* Check whether the DTS version is compatible with the message
			 * format encoded for. If DTS for which 'D' was encoded for was
			 * 2 and now DTS for which msg is being sent is 1, drop the 
			 * message.
			 */
			if ((remote_ver == 1) && (mm->data.data.msg.msg_fmat_ver == 2)) {
				return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
						      "dta_mds_enc_v2: Log msg with format type 'D/N/U/X' dropped, because Active DTS version has changed to 1.");
			} else {
	     /*****************************************************\
             CASE 1: remove_ver = 1, msg_fmat_ver = 1 - no problem

             CASE 2: remove_ver = 2, msg_fmat_ver = 1 - no problem
                     (New DTS will accept a old format message
                      irrespective of whether it comes from an 
                      old or new DTA. In particular, the "D" format
                      will also go through fine)

             CASE 3: remove_ver = 2, msg_fmat_ver = 2 - no problem
             \*****************************************************/

				*msg_fmat_ver = mm->data.data.msg.msg_fmat_ver;
				assert((mm->data.data.msg.msg_fmat_ver == 1) || (mm->data.data.msg.msg_fmat_ver == 2));
				return dta_log_msg_encode(&mm->data.data.msg.log_msg, uba);
			}
		}

#if (DTA_FLOW == 1)
	case DTA_FLOW_CONTROL:
	case DTS_CONGESTION_HIT:
	case DTS_CONGESTION_CLEAR:
		{
			/* Do nothing */
			break;
		}
#endif

	default:
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_enc: Wrong message type received");
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Name          : dta_mds_dec
 *
 * Description   : Decode a DTA message coming in.
 *
 * Arguments     : yr_svc_hdl : DTA service handle.
 *                 msg        : Message received from the DTS.
 *                 to_svc     : Service ID to send.
 *                 uba        : User Buffer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 dta_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT *msg,
		  SS_SVC_ID to_svc, NCS_UBAID *uba, MDS_CLIENT_MSG_FORMAT_VER msg_fmat_ver)
{
	uns8 *data;
	DTSV_MSG *mm;
	uns8 data_buff[DTSV_DTS_DTA_MSG_HDR_SIZE];
	uns32 status = NCSCC_RC_SUCCESS;
	NCS_UBAID *payload_uba;

	if (uba == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_dec : User buffer is NULL");

	if ((msg_fmat_ver < DTA_MDS_MIN_MSG_FMAT_VER_SUPPORT) || (msg_fmat_ver > DTA_MDS_MAX_MSG_FMAT_VER_SUPPORT))
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_dec : Message format not acceptable");

	if (msg == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_dec : Message is NULL");

	mm = m_MMGR_ALLOC_DTSV_MSG;

	if (mm == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_dec : Memory allocation failed.");
	memset(mm, '\0', sizeof(DTSV_MSG));

	*msg = mm;

	data = ncs_dec_flatten_space(uba, data_buff, DTSV_DTS_DTA_MSG_HDR_SIZE);
	if (data == NULL) {
		m_MMGR_FREE_DTSV_MSG(mm);
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_dec :ncs_dec_flatten_space returns NULL");
	}

	mm->msg_type = ncs_decode_8bit(&data);

	ncs_dec_skip_space(uba, DTSV_DTS_DTA_MSG_HDR_SIZE);

	switch (mm->msg_type) {
	case DTS_SVC_REG_CONF:
		{
			data = ncs_dec_flatten_space(uba, data_buff, DTSV_REG_CONF_MSG_SIZE);
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
						      "dta_mds_dec :ncs_dec_flatten_space returns NULL");
			}

			mm->data.data.reg_conf.msg_fltr.svc_id = ncs_decode_32bit(&data);
			mm->data.data.reg_conf.msg_fltr.enable_log = ncs_decode_32bit(&data);
			mm->data.data.reg_conf.msg_fltr.category_bit_map = ncs_decode_32bit(&data);
			mm->data.data.reg_conf.msg_fltr.severity_bit_map = ncs_decode_8bit(&data);
			/* No need of policy handles */
			/*mm->data.data.reg_conf.msg_fltr.policy_hdl = ncs_decode_32bit(&data); */
			ncs_dec_skip_space(uba, DTSV_REG_CONF_MSG_SIZE);

			break;
		}
	case DTS_SVC_MSG_FLTR:
		{
			data = ncs_dec_flatten_space(uba, data_buff, DTSV_FLTR_MSG_SIZE);
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
						      "dta_mds_dec :ncs_dec_flatten_space returns NULL");
			}

			mm->data.data.msg_fltr.svc_id = ncs_decode_32bit(&data);
			mm->data.data.msg_fltr.enable_log = ncs_decode_32bit(&data);
			mm->data.data.msg_fltr.category_bit_map = ncs_decode_32bit(&data);
			mm->data.data.msg_fltr.severity_bit_map = ncs_decode_8bit(&data);
			ncs_dec_skip_space(uba, DTSV_FLTR_MSG_SIZE);

			break;
		}
	case DTS_FAIL_OVER:
		{
			payload_uba = &mm->data.data.msg.log_msg.uba;
			if (ncs_enc_init_space(payload_uba) != NCSCC_RC_SUCCESS) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_dec: Encode init space failed");
			}
			ncs_enc_append_usrbuf(payload_uba, uba->ub);
			/* Point the uba->ub pointer to NULL after its copied */
			uba->ub = NULL;
			break;
		}

#if (DTA_FLOW == 1)
	case DTA_FLOW_CONTROL:
		{
			/* Free the allocated memory here */
			m_MMGR_FREE_DTSV_MSG(mm);
			return NCSCC_RC_SUCCESS;
		}
#endif

	default:
		m_MMGR_FREE_DTSV_MSG(mm);
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_dec :Wrong message type received");
	}
	return status;
}

/****************************************************************************\
 * Name          : dta_mds_cpy
 *
 * Description   : Copy a DTA message.
 *
 * Arguments     : yr_svc_hdl : DTA service handle.
 *                 msg        : Message received from the DTS.
 *                 to_svc     : Service ID to send.
 *                 cpy        : Copy to.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 dta_mds_cpy(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
		  SS_SVC_ID to_svc, NCSCONTEXT *cpy,
		  NCS_BOOL last, MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver)
{

	DTSV_MSG *mm;
	int warning_rmval = 0;

	if (msg == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_cpy : Message is NULL");

	/* Versioning changes - Set msg format version same as the remote version */
	*msg_fmat_ver = 1;

	mm = m_MMGR_ALLOC_DTSV_MSG;

	if (mm == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_cpy: Memory allocation failed");

	memset(mm, '\0', sizeof(DTSV_MSG));

	*cpy = mm;

	memcpy(mm, msg, sizeof(DTSV_MSG));

	if (mm->msg_type == DTA_LOG_DATA) {
		/* Set the message format appropriately. Same logic as dta_mds_enc. */
		/* Check whether the DTS version is compatible with the message
		 * format encoded for. If DTS for which 'D' was encoded for was
		 * 2 and now DTS for which msg is being sent is 1, drop the
		 * message.
		 */
		if ((remote_ver == 1) && (mm->data.data.msg.msg_fmat_ver == 2)) {
			return m_DTA_DBG_SINK(NCSCC_RC_FAILURE,
					      "dta_mds_cpy: Log msg with format type 'D' dropped, because Active DTS version has changed to 1.");
		} else {
	  /*****************************************************\
          CASE 1: remove_ver = 1, msg_fmat_ver = 1 - no problem

          CASE 2: remove_ver = 2, msg_fmat_ver = 1 - no problem
                  (New DTS will accept a old format message
                   irrespective of whether it comes from an
                   old or new DTA. In particular, the "D" format
                   will also go through fine)

          CASE 3: remove_ver = 2, msg_fmat_ver = 2 - no problem
          \*****************************************************/

			*msg_fmat_ver = mm->data.data.msg.msg_fmat_ver;
			assert((mm->data.data.msg.msg_fmat_ver == 1) || (mm->data.data.msg.msg_fmat_ver == 2));
		}

		if (NCSCC_RC_SUCCESS != dta_copy_octets(&mm->data.data.msg.log_msg.hdr.fmat_type,
							((DTSV_MSG *)msg)->data.data.msg.log_msg.hdr.fmat_type,
							(uns16)(1 +
								strlen(((DTSV_MSG *)msg)->data.data.msg.log_msg.hdr.
								       fmat_type)))) {
			if (mm != NULL)
				m_MMGR_FREE_DTSV_MSG(mm);
			return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_cpy: Copy octet failed.");
		}

		ncs_dec_init_space(&mm->data.data.msg.log_msg.uba, mm->data.data.msg.log_msg.uba.start);
	}

	if (mm->msg_type == DTS_FAIL_OVER)
		warning_rmval =
		    m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_cpy: Received copy callback for DTS_FAIL_OVER msg");

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Name          : dta_copy_octates
 *
 * Description   : This finction is used for copying the octet strings from 
 *                 source to destination.
 *
 * Arguments     : dest : Copy to.
 *                 src  : Copy from.
 *                 length: Number of bytes.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 dta_copy_octets(char **dest, char *src, uns16 length)
{
	if ((dest == NULL) || (src == NULL) || (length == 0)) {
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_copy_octets : failed to copy.");
	}

	*dest = m_MMGR_ALLOC_OCT(length);

	if (*dest == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_copy_octets: Memory allocation failed.");

	memset(*dest, '\0', length);

	memcpy(*dest, src, (length * sizeof(uns8)));

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Name          : dta_log_msg_encode
 *
 * Description   : This finction is used for encoding a NCSFL_NORMAL into a ubaid.
 *
 * Arguments     : logmsg  : Normalized structure to be encoded.
 *                 uba     : User buffer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
\*****************************************************************************/
uns32 dta_log_msg_encode(NCSFL_NORMAL *logmsg, NCS_UBAID *uba)
{
	uns8 *data;
	uns32 length;

	if (uba == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_log_msg_encode: User buffer is NULL");

	if (logmsg == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_log_msg_encode: log message is NULL");

	data = ncs_enc_reserve_space(uba, DTS_LOG_MSG_HDR_SIZE);

	if (data == NULL)
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_log_msg_encode: ncs_enc_reserve_space returns NULL");

	ncs_encode_32bit(&data, logmsg->hdr.time.seconds);
	ncs_encode_32bit(&data, logmsg->hdr.time.millisecs);

	ncs_encode_32bit(&data, logmsg->hdr.ss_id);
	ncs_encode_32bit(&data, logmsg->hdr.inst_id);
	ncs_encode_8bit(&data, logmsg->hdr.severity);
	ncs_encode_32bit(&data, logmsg->hdr.category);

	ncs_encode_8bit(&data, logmsg->hdr.fmat_id);

	ncs_enc_claim_space(uba, DTS_LOG_MSG_HDR_SIZE);

	length = strlen(logmsg->hdr.fmat_type) + 1;

	data = ncs_enc_reserve_space(uba, sizeof(uns32));
	ncs_encode_32bit(&data, length);
	ncs_enc_claim_space(uba, sizeof(uns32));

	ncs_encode_n_octets_in_uba(uba, (uns8 *)logmsg->hdr.fmat_type, length);

	/*m_MMGR_FREE_OCT((uns8*)logmsg->hdr.fmat_type); */

	/* Append user buffer */
	ncs_enc_append_usrbuf(uba, logmsg->uba.start);

	/* Now we have handed the uba to MDS, so make our uba ptr to NULL */
	logmsg->uba.start = NULL;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : encode_ip_address
 
  Description   : Function for encoding the IP address
 
  Arguments     : uba  - User Buffer.
                  ipa  - IP address to be encoded.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 encode_ip_address(NCS_UBAID *uba, NCS_IP_ADDR ipa)
{
	uns8 *data;

	data = ncs_enc_reserve_space(uba, sizeof(uns8));
	ncs_encode_8bit(&data, ipa.type);
	ncs_enc_claim_space(uba, sizeof(uns8));

	if (ipa.type == NCS_IP_ADDR_TYPE_IPV4) {
		data = ncs_enc_reserve_space(uba, sizeof(uns32));
		ncs_encode_32bit(&data, ipa.info.v4);
		ncs_enc_claim_space(uba, sizeof(uns32));
	}
#if (NCS_IPV6 == 1)
	else if (ipa.type == NCS_IP_ADDR_TYPE_IPV6) {
		data = ncs_enc_reserve_space(uba, (NCS_IPV6_ADDR_UNS8_CNT * sizeof(uns8)));
		ncs_encode_octets(&data, ipa.info.v6, NCS_IPV6_ADDR_UNS8_CNT);
		ncs_enc_claim_space(uba, (NCS_IPV6_ADDR_UNS8_CNT * sizeof(uns8)));
	}
#endif
	else {
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "encode_ip_address: Incorrect IP address type passed.");
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : dta_mds_sync_send
 
  Description   : This routine sends the DTA message to DTS and the responce
                  is expected from the DTS.
 
  Arguments     : msg  - Message to be sent
                  inst - DTA control blovk
                  timeout - timeout value in 10 ms 
                  svc_reg - to check whether sync send is for service 
                            registration or not
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
#if (DTA_FLOW == 1)
uns32 dta_mds_sync_send(DTSV_MSG *msg, DTA_CB *inst, uns32 timeout, NCS_BOOL svc_reg)
#else
uns32 dta_mds_sync_send(DTSV_MSG *msg, DTA_CB *inst, uns32 timeout)
#endif
{
	NCSMDS_INFO mds_info;
	uns32 status = NCSCC_RC_SUCCESS;

	memset(&mds_info, 0, sizeof(NCSMDS_INFO));
	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_DTA;
	mds_info.i_op = MDS_SEND;

	/* fill the send structure */
	mds_info.info.svc_send.i_msg = (NCSCONTEXT)msg;
	mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;
	mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_DTS;
	mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SNDRSP;

	/* fill the send rsp strcuture */
	mds_info.info.svc_send.info.sndrsp.i_time_to_wait = timeout;	/* timeto wait in 10ms */
	mds_info.info.svc_send.info.sndrsp.i_to_dest = inst->dts_dest;

	/* send the message */
	if (ncsmds_api(&mds_info) == NCSCC_RC_SUCCESS) {
#if (DTA_FLOW == 1)
		if (svc_reg == TRUE) {
#endif
			m_DTA_LK(&dta_cb.lock);

			status = dta_svc_reg_config(&dta_cb, (DTSV_MSG *)mds_info.info.svc_send.info.sndrsp.o_rsp);

			m_MMGR_FREE_DTSV_MSG((DTSV_MSG *)mds_info.info.svc_send.info.sndrsp.o_rsp);
			m_DTA_UNLK(&dta_cb.lock);
#if (DTA_FLOW == 1)
		}
#endif
	}
#if (DTA_FLOW == 1)
	else if (svc_reg == TRUE)
#else
	else
#endif
	{
		status = m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_sync_send: Sync send failed.");
	}

	return status;
}

/****************************************************************************
  Name          : dta_mds_async_send
 
  Description   : This routine sends the DTA message to DTS.
 
  Arguments     : msg  - Message to be sent
                  inst - DTA control blovk
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 dta_mds_async_send(DTSV_MSG *msg, DTA_CB *inst)
{
	NCSMDS_INFO mds_info;

	memset(&mds_info, 0, sizeof(mds_info));

	mds_info.i_mds_hdl = inst->mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_DTA;
	mds_info.i_op = MDS_SEND;

	mds_info.info.svc_send.i_msg = msg;
	mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_LOW;
	mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
	mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_DTS;

	mds_info.info.svc_send.info.snd.i_to_dest = inst->dts_dest;

	if (ncsmds_api(&mds_info) == NCSCC_RC_SUCCESS) {
		return NCSCC_RC_SUCCESS;
	} else {
		return m_DTA_DBG_SINK(NCSCC_RC_FAILURE, "dta_mds_async_send: MDS async send failed.");
	}
}
