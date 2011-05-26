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
  This file includes the functions require for MDS interface.

  ..............................................................................

  FUNCTIONS INCLUDED in this module:
  dts_mds_reg        - Register with MDS.
  dts_mds_change_role- MDS change role.
  dts_mds_rcv        - rcv something from MDS lower layer.
  dts_mds_evt        - subscription event entry point
  dts_mds_enc        - MDS encode routine.
  dts_mds_dec        - MDS decode routine.
  dts_mds_cpy        - MDS copy routine.
  dts_log_msg_decode - Log message decode.
  dts_log_str_decode - Decode log string.
  decode_ip_address  - Decode IP address.
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "dts.h"
#include "ncssysf_mem.h"

/****************************************************************************
  Name          : dts_mds_reg
 
  Description   : This routine registers the DTS Service with MDS with the 
                  DTSs Vaddress. 
 
  Arguments     : cb - ptr to the DTS control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uint32_t dts_mds_reg(DTS_CB *cb)
{
	NCSVDA_INFO vda_info;
	NCSMDS_INFO svc_to_mds_info;
	MDS_SVC_ID svc_ids_array[2];

	/* prepare the cb with the vaddress */
	memset(&cb->vaddr, 0, sizeof(MDS_DEST));
	cb->vaddr = DTS_VDEST_ID;

	vda_info.req = NCSVDA_VDEST_CREATE;
	vda_info.info.vdest_create.i_persistent = false;
	vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
	vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
	vda_info.info.vdest_create.info.specified.i_vdest = cb->vaddr;

	/* create Vdest address */
	if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_reg: Vdest creation failed");

	/* store the info returned by MDS */
	cb->mds_hdl = vda_info.info.vdest_create.o_mds_pwe1_hdl;
	/* Smik - New additions */
	cb->vaddr_pwe_hdl = vda_info.info.vdest_create.o_mds_pwe1_hdl;
	cb->vaddr_hdl = vda_info.info.vdest_create.o_mds_vdest_hdl;

#if (DTS_SIM_TEST_ENV == 1)
	if (true == cb->is_test) {
		/* set the role of the vdest */
		if (dts_mds_change_role(cb, cb->ha_state) != NCSCC_RC_SUCCESS) {
			dts_mds_unreg(cb, false);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_reg: DTS Role change failed");
		}
	}			/* is_test is true */
#endif

	/* Install mds */
	memset(&svc_to_mds_info, 0, sizeof(NCSMDS_INFO));
	svc_to_mds_info.i_mds_hdl = cb->mds_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_DTS;
	svc_to_mds_info.i_op = MDS_INSTALL;
	svc_to_mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)(long)cb;
	svc_to_mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
	svc_to_mds_info.info.svc_install.i_svc_cb = dts_mds_callback;
	svc_to_mds_info.info.svc_install.i_mds_q_ownership = false;
	/* versioning changes */
	svc_to_mds_info.info.svc_install.i_mds_svc_pvt_ver = cb->dts_mds_version;
	cb->created = true;
	if (ncsmds_api(&svc_to_mds_info) != NCSCC_RC_SUCCESS) {
		cb->created = false;
		dts_mds_unreg(cb, false);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_reg:  MDS install failed");
	}

	/* DTS is subscribing for DTA MDS service */
	memset(&svc_to_mds_info, 0, sizeof(NCSMDS_INFO));
	svc_to_mds_info.i_mds_hdl = cb->mds_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_DTS;
	svc_to_mds_info.i_op = MDS_SUBSCRIBE;
	svc_to_mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
	svc_to_mds_info.info.svc_subscribe.i_num_svcs = DTS_NUM_SVCS_TO_SUBSCRIBE;
	svc_ids_array[0] = NCSMDS_SVC_ID_DTA;
	svc_to_mds_info.info.svc_subscribe.i_svc_ids = svc_ids_array;

	if (ncsmds_api(&svc_to_mds_info) != NCSCC_RC_SUCCESS) {
		cb->created = false;
		dts_mds_unreg(cb, true);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_reg: DTA Event subscription failed!!");
	}

	/* DTS is subscribing for IMMMD MDS service */
	memset(&svc_to_mds_info, 0, sizeof(NCSMDS_INFO));
	svc_to_mds_info.i_mds_hdl = cb->mds_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_DTS;
	svc_to_mds_info.i_op = MDS_SUBSCRIBE;
	svc_to_mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_INTRANODE;
	svc_to_mds_info.info.svc_subscribe.i_num_svcs = DTS_NUM_SVCS_TO_SUBSCRIBE;
	svc_ids_array[0] = NCSMDS_SVC_ID_IMMND;
	svc_to_mds_info.info.svc_subscribe.i_svc_ids = svc_ids_array;

	if (ncsmds_api(&svc_to_mds_info) != NCSCC_RC_SUCCESS) {
		cb->created = false;
		dts_mds_unreg(cb, true);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_reg: IMMND Event subscription failed!!");
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : dts_mds_change_role
 
  Description   : This routine use for setting and changing role. 
 
  Arguments     : role - Role to be set.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uint32_t dts_mds_change_role(DTS_CB *cb, SaAmfHAStateT role)
{
	NCSVDA_INFO vda_info;

	memset(&vda_info, 0, sizeof(vda_info));

	vda_info.req = NCSVDA_VDEST_CHG_ROLE;
	vda_info.info.vdest_chg_role.i_vdest = cb->vaddr;
	/*vda_info.info.vdest_chg_role.i_anc = V_DEST_QA_1; */

	vda_info.info.vdest_chg_role.i_new_role = role;
	if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_change_role: DTS change role failed.");
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : dts_mds_unreg
 
  Description   : This routine unregisters the DTS Service from MDS on the
                  DTSs Vaddress 
 
  Arguments     : cb - ptr to the DTScontrol block
 
  Return Values : NONE
 
  Notes         : None.
******************************************************************************/
void dts_mds_unreg(DTS_CB *cb, bool un_install)
{
	NCSVDA_INFO vda_info;
	NCSMDS_INFO svc_to_mds_info;

	/* uninstall MDS */
	if (un_install) {
		svc_to_mds_info.i_mds_hdl = cb->mds_hdl;
		svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_DTS;
		svc_to_mds_info.i_op = MDS_UNINSTALL;
		ncsmds_api(&svc_to_mds_info);
	}

	/* destroy vaddress */
	vda_info.req = NCSVDA_VDEST_DESTROY;
	vda_info.info.vdest_destroy.i_vdest = cb->vaddr;
	ncsvda_api(&vda_info);

	return;
}

/****************************************************************************
  Name          : dts_mds_send_msg
 
  Description   : This routine is use for sending the message to the DTA
 
  Arguments     : cb - ptr to the DTScontrol block
 
  Return Values : NONE
 
  Notes         : None.
******************************************************************************/
uint32_t dts_mds_send_msg(DTSV_MSG *msg, MDS_DEST dta_dest, MDS_CLIENT_HDL mds_hdl)
{
	NCSMDS_INFO mds_info;

	memset(&mds_info, 0, sizeof(mds_info));

	mds_info.i_mds_hdl = mds_hdl;
	mds_info.i_svc_id = NCSMDS_SVC_ID_DTS;
	mds_info.i_op = MDS_SEND;

	mds_info.info.svc_send.i_msg = msg;
	mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_DTA;

	if (true == msg->rsp_reqd) {
		mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
		mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_RSP;
		mds_info.info.svc_send.info.rsp.i_msg_ctxt = msg->msg_ctxt;
		mds_info.info.svc_send.info.rsp.i_sender_dest = dta_dest;
	} else {
		mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_LOW;
		mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
		mds_info.info.svc_send.info.snd.i_to_dest = dta_dest;
	}

	if (ncsmds_api(&mds_info) == NCSCC_RC_SUCCESS) {
		return NCSCC_RC_SUCCESS;
	} else {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_send_msg : MDS send failed");
	}
}

/****************************************************************************
 * Name          : dts_mds_callback
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
uint32_t dts_mds_callback(NCSMDS_CALLBACK_INFO *cbinfo)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTSV_MSG *msg = NULL;

	switch (cbinfo->i_op) {
	case MDS_CALLBACK_COPY:
		status = dts_mds_cpy(cbinfo->i_yr_svc_hdl, cbinfo->info.cpy.i_msg,
				     cbinfo->info.cpy.i_to_svc_id,
				     &cbinfo->info.cpy.o_cpy,
				     cbinfo->info.cpy.i_last,
				     cbinfo->info.cpy.i_rem_svc_pvt_ver, &cbinfo->info.cpy.o_msg_fmt_ver);
		break;

	case MDS_CALLBACK_ENC:
		/* Treating both encode types as same */
		status = dts_mds_enc(cbinfo->i_yr_svc_hdl, cbinfo->info.enc.i_msg,
				     cbinfo->info.enc.i_to_svc_id,
				     cbinfo->info.enc.io_uba,
				     cbinfo->info.enc_flat.i_rem_svc_pvt_ver, &cbinfo->info.enc.o_msg_fmt_ver);
		break;

	case MDS_CALLBACK_ENC_FLAT:
		/* Treating both encode types as same */
		status = dts_mds_enc(cbinfo->i_yr_svc_hdl, cbinfo->info.enc_flat.i_msg,
				     cbinfo->info.enc_flat.i_to_svc_id,
				     cbinfo->info.enc_flat.io_uba,
				     cbinfo->info.enc_flat.i_rem_svc_pvt_ver, &cbinfo->info.enc.o_msg_fmt_ver);
		break;

	case MDS_CALLBACK_DEC:
		/* Treating both decode types as same */
		status = dts_mds_dec(cbinfo->i_yr_svc_hdl, &cbinfo->info.dec.o_msg,
				     cbinfo->info.dec.i_fr_svc_id,
				     cbinfo->info.dec.io_uba, cbinfo->info.dec.i_msg_fmt_ver);
		break;

	case MDS_CALLBACK_DEC_FLAT:
		/* Treating both decode types as same */
		status = dts_mds_dec(cbinfo->i_yr_svc_hdl, &cbinfo->info.dec_flat.o_msg,
				     cbinfo->info.dec_flat.i_fr_svc_id,
				     cbinfo->info.dec_flat.io_uba, cbinfo->info.dec_flat.i_msg_fmt_ver);
		break;

	case MDS_CALLBACK_RECEIVE:
		status = dts_mds_rcv(cbinfo);
		break;

	case MDS_CALLBACK_SVC_EVENT:
		dts_mds_evt(cbinfo->info.svc_evt, cbinfo->i_yr_svc_hdl);

		status = NCSCC_RC_SUCCESS;
		break;
		/* Case for handling mds callback for MDS_CALLBACK_QUIESCED_ACK */
	case MDS_CALLBACK_QUIESCED_ACK:
		/* Post a message to DTS mailbox and process that message in a 
		 * as in normal flow.
		 */
		msg = m_MMGR_ALLOC_DTSV_MSG;
		if (msg == NULL)
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dts_mds_callback: Failed to allocate memory for DTSV_MSG");
		memset(msg, 0, sizeof(DTSV_MSG));
		msg->msg_type = DTS_QUIESCED_CMPLT;
		if (m_DTS_SND_MSG(&gl_dts_mbx, msg, NCS_IPC_PRIORITY_NORMAL) != NCSCC_RC_SUCCESS) {
			if (0 != msg)
				m_MMGR_FREE_DTSV_MSG(msg);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "QUIESCED_COMPL MSG  IPC send failed");
		}
		break;

	default:
		status = m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					"dts_mds_callback: DTS callback is called with wrong operation type");
		break;
	}

	return status;
}

/****************************************************************************
 * Function Name: dts_mds_rcv
 * Purpose:       MDS receive function.
 ****************************************************************************/

uint32_t dts_mds_rcv(NCSMDS_CALLBACK_INFO *cbinfo)
{
	DTS_CB *inst = &dts_cb;
	DTSV_MSG *msg = (DTSV_MSG *)cbinfo->info.receive.i_msg;
	uint32_t send_pri;

	if ((cbinfo->info.receive.i_msg_fmt_ver < DTS_MDS_MIN_MSG_FMAT_VER_SUPPORT) ||
	    (cbinfo->info.receive.i_msg_fmt_ver > DTS_MDS_MAX_MSG_FMAT_VER_SUPPORT))
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_mds_rcv: Message format version is not withing acceptable limits.");

	if (inst->created == false) {
		if (0 != msg)
			m_MMGR_FREE_DTSV_MSG(msg);

		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_rcv: DTS does not exist. First create DTS");
	}

	if (NULL == msg)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_rcv: NULL message received.");

	msg->seq_msg = false;

	/* plant DTS subcomponent's control block in DTSV_MSG */

	msg->dest_addr = cbinfo->info.receive.i_fr_dest;	/* Sending DTA's address */
	msg->node = cbinfo->info.receive.i_node_id;	/* Replace it by MDS provided value */
	msg->rsp_reqd = cbinfo->info.receive.i_rsp_reqd;
	msg->msg_ctxt = cbinfo->info.receive.i_msg_ctxt;

#if (DTS_FLOW == 1)
	/* Check for DTS message threshold value.DTS defines MAX threshold of 30000
	 * messages after which all messages will be dropped.
	 */
	if (msg->msg_type == DTA_LOG_DATA) {
		if (inst->msg_count > DTS_MAX_THRESHOLD) {
			m_MMGR_FREE_OCT(msg->data.data.msg.log_msg.hdr.fmat_type);
			if (msg->data.data.msg.log_msg.uba.ub != NULL)
				m_MMGR_FREE_BUFR_LIST(msg->data.data.msg.log_msg.uba.ub);
			if (0 != msg)
				m_MMGR_FREE_DTSV_MSG(msg);
			return NCSCC_RC_SUCCESS;
		}

		else if (inst->msg_count > DTS_AVG_THRESHOLD) {
			if (msg->data.data.msg.log_msg.hdr.severity <= NCSFL_SEV_ERROR) {
				m_MMGR_FREE_OCT(msg->data.data.msg.log_msg.hdr.fmat_type);
				if (msg->data.data.msg.log_msg.uba.ub != NULL)
					m_MMGR_FREE_BUFR_LIST(msg->data.data.msg.log_msg.uba.ub);
				if (0 != msg)
					m_MMGR_FREE_DTSV_MSG(msg);
				return NCSCC_RC_SUCCESS;
			}
		}

		else if (inst->msg_count > DTS_MIN_THRESHOLD) {
			if (msg->data.data.msg.log_msg.hdr.severity <= NCSFL_SEV_NOTICE) {
				m_MMGR_FREE_OCT(msg->data.data.msg.log_msg.hdr.fmat_type);
				if (msg->data.data.msg.log_msg.uba.ub != NULL)
					m_MMGR_FREE_BUFR_LIST(msg->data.data.msg.log_msg.uba.ub);
				if (0 != msg)
					m_MMGR_FREE_DTSV_MSG(msg);
				return NCSCC_RC_SUCCESS;
			}
		}
	} else if (msg->msg_type == DTA_FLOW_CONTROL) {
		if (inst->msg_count > DTS_MIN_THRESHOLD) {
			if (0 != msg)
				m_MMGR_FREE_DTSV_MSG(msg);
			return NCSCC_RC_SUCCESS;
		}
	}
#endif

	/* 
	 * Put it in DTS's work queue; If message is registration Message then send at Normal
	 * priority, of de-registration and log messages Low priority is Okay. 
	 */

	if (msg->msg_type == DTA_LOG_DATA)
		send_pri = NCS_IPC_PRIORITY_LOW;
	else
		send_pri = NCS_IPC_PRIORITY_NORMAL;

	if (m_DTS_SND_MSG(&gl_dts_mbx, cbinfo->info.receive.i_msg, send_pri) != NCSCC_RC_SUCCESS) {
		if (0 != msg) {
			if (msg->msg_type == DTA_LOG_DATA)
				dts_free_msg_content(&msg->data.data.msg.log_msg);

			m_MMGR_FREE_DTSV_MSG(msg);
		}

		/*Changed due to possibility of an infinite loop, with this log message
		 *also failing due to IPC send failure.
		 */
		/*m_LOG_DTS_SVC_PRVDR(DTS_SP_MDS_SND_MSG_FAILED); */
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_rcv: DTS: IPC send failed");
	}

	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Function Name: dts_mds_evt
 * Purpose:        DTS is informed when MDS events occurr that he has 
 *                subscribed to.
 ****************************************************************************/

void dts_mds_evt(MDS_CALLBACK_SVC_EVENT_INFO svc_info, MDS_CLIENT_HDL yr_svc_hdl)
{

	DTS_CB *inst = &dts_cb;
	DTSV_MSG *msg;

	if (inst->created == false) {
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_evt: DTS does not exist");
		return;
	}

	msg = m_MMGR_ALLOC_DTSV_MSG;
	if (msg == NULL) {
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_evt: Failed to allocate DTSV message");
		return;
	}
	memset(msg, '\0', sizeof(DTSV_MSG));

        if(svc_info.i_svc_id == NCSMDS_SVC_ID_IMMND)
                msg->msg_type = DTS_IMMND_EVT_RCV;
        else
                msg->msg_type = DTS_DTA_EVT_RCV;

	msg->node = svc_info.i_node_id;
	msg->data.data.evt.change = svc_info.i_change;
	msg->dest_addr = svc_info.i_dest;

	if (m_DTS_SND_MSG(&gl_dts_mbx, msg, NCS_IPC_PRIORITY_NORMAL) != NCSCC_RC_SUCCESS) {
		if (0 != msg) {
			if (msg->msg_type == DTA_LOG_DATA)
				dts_free_msg_content(&msg->data.data.msg.log_msg);

			m_MMGR_FREE_DTSV_MSG(msg);
		}
		/*Changed due to possibility of an infinite loop, with this log message
		 *also failing due to IPC send failure.
		 */
		/*m_LOG_DTS_SVC_PRVDR(DTS_SP_MDS_SND_MSG_FAILED); */

		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_evt: DTS: IPC send failed");

		return;
	}

	return;

}

/****************************************************************************
 * Name          : dts_set_dta_up_down
 *
 * Description   : Function marks DTA as up or down on receiving the up down
 *                 event from the DTA.
 *
 * Arguments     : node_id - Node ID of the node where DTA resides.
 *                 adest   - DTA destination address.
 *                 up_down - Flag indicating whether DTA is up or down.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
void dts_set_dta_up_down(NODE_ID node_id, MDS_DEST adest, bool up_down)
{
	DTS_CB *inst = &dts_cb;
	SVC_KEY key, nt_key;
	DTS_SVC_REG_TBL *svc;
	DTA_DEST_LIST *dta, *dta_ptr;
	SVC_ENTRY *svc_entry;
	/*MDS_DEST              dta_key; */
	OP_DEVICE *dev;

	/*  Network order key added */
	nt_key.node = m_NCS_OS_HTONL(node_id);
	nt_key.ss_svc_id = 0;

	/* Check if node with node_id already exists. If it doesn't, return */
	if ((svc = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&inst->svc_tbl, (const uint8_t *)&nt_key)) == NULL) {
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_set_dta_up_down: No svc node exists in patricia tree");
		return;
	}

	if ((dta = (DTA_DEST_LIST *)ncs_patricia_tree_get(&inst->dta_list, (const uint8_t *)&adest)) != NULL) {
		/* Adjust the pointer to DTA with the offset */
		dta = (DTA_DEST_LIST *)((long)dta - DTA_DEST_LIST_OFFSET);

		dta->dta_up = up_down;

		if (up_down == false) {
			m_LOG_DTS_EVT(DTS_EV_DTA_DOWN, 0, m_NCS_NODE_ID_FROM_MDS_DEST(dta->dta_addr),
				      (uint32_t)(dta->dta_addr));
			/* go through the svc_list of DTA removing DTA frm all those svcs */
			svc_entry = dta->svc_list;
			while (svc_entry != NULL) {
				svc = svc_entry->svc;
				if (svc == NULL) {
					svc_entry = svc_entry->next_in_dta_entry;
					continue;
				}
				key = svc->my_key;

				if ((dta_ptr = (DTA_DEST_LIST *)dts_find_dta(svc, &dta->dta_addr)) != NULL) {
					/* Remove dta entry frm svc->v_cd_list */
					if ((dta_ptr = (DTA_DEST_LIST *)dts_dequeue_dta(svc, dta_ptr)) == NULL) {
						m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							       "dts_set_dta_up_down: Unable to remove dta entry frm svc->v_cd_list");
						break;
					}
					m_LOG_DTS_EVT(DTS_EV_SVC_DTA_RMV, svc->my_key.ss_svc_id, svc->my_key.node,
						      (uint32_t)(dta_ptr->dta_addr));

					/* Versioning support - Remove spec entry corresponding to the 
					 * DTA from svc's spec_list. 
					 * Note: Even when spec's use_count becomes 0 don't call unload
					 * of library here. Unload will be called only in case of 
					 * service unregister. 
					 */
					if (dts_del_spec_frm_svc(svc, adest, NULL) != NCSCC_RC_SUCCESS)
						m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							       "dts_set_dta_up_down: Unable to remove spec entry");

					/* Point to the next svc in DTA svc_list */
					svc_entry = svc_entry->next_in_dta_entry;

					/*Remove svc entry frm dta->svc_list */
					if ((svc = (DTS_SVC_REG_TBL *)dts_del_svc_frm_dta(dta_ptr, svc)) == NULL) {
						m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							       "dts_set_dta_up_down: Unable to remove svc from dta->svc_list");
						break;
					}
					m_LOG_DTS_EVT(DTS_EV_DTA_SVC_RMV, svc->my_key.ss_svc_id, svc->my_key.node,
						      (uint32_t)(dta_ptr->dta_addr));

					if (svc->dta_count == 0) {
						dts_circular_buffer_free(&svc->device.cir_buffer);
						dev = &svc->device;
						/* Cleanup the DTS_FILE_LIST datastructure for svc */
						m_DTS_FREE_FILE_LIST(dev);
						if ((true == svc->device.file_open) && (svc->device.svc_fh != NULL)) {
							fclose(svc->device.svc_fh);
							svc->device.svc_fh = NULL;
							svc->device.file_open = false;
							svc->device.new_file = true;
							svc->device.cur_file_size = 0;
						}
						/* Cleanup the console devices associated with the node */
						m_DTS_RMV_ALL_CONS(dev);
						ncs_patricia_tree_del(&inst->svc_tbl, (NCS_PATRICIA_NODE *)svc);
						m_LOG_DTS_EVT(DTS_EV_SVC_DEREG_SUCCESSFUL, key.ss_svc_id, key.node,
							      (uint32_t)(dta_ptr->dta_addr));
						if (NULL != svc)
							m_MMGR_FREE_SVC_REG_TBL(svc);
					}
				} /*end of if */
				else {
					m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						       "dts_set_dta_up_down: dta entry missing in svc->v_cd_list");
					/* hop to the next service */
					svc_entry = svc_entry->next_in_dta_entry;
				}

			}	/* end of while */

			/*At the end of while loop svc_list shud point to NULL */
			dta->svc_list = NULL;

			if (ncs_patricia_tree_del(&inst->dta_list, (NCS_PATRICIA_NODE *)&dta->node) != NCSCC_RC_SUCCESS) {
				m_LOG_DTS_EVT(DTS_EV_DTA_DEST_RMV_FAIL, 0, m_NCS_NODE_ID_FROM_MDS_DEST(dta->dta_addr),
					      (uint32_t)(dta->dta_addr));
				m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					       "dts_set_dta_up_down: Unable to remove DTA entry frm patricia tree");
				return;
			}
			m_LOG_DTS_EVT(DTS_EV_DTA_DEST_RMV_SUCC, 0, m_NCS_NODE_ID_FROM_MDS_DEST(dta->dta_addr),
				      (uint32_t)(dta->dta_addr));
			/* Send Async update (REMOVE) for DTA_DEST_LIST */
			m_DTSV_SEND_CKPT_UPDT_ASYNC(inst, NCS_MBCSV_ACT_RMV, (MBCSV_REO_HDL)(long)dta,
						    DTSV_CKPT_DTA_DEST_LIST_CONFIG);

			if (NULL != dta)
				m_MMGR_FREE_VCARD_TBL(dta);
		}
		/* end of up_down == false */
		else if ((true == up_down) && (true == dta->updt_req)) {
			/* send filter config msg for all svcs for this dta */
			m_LOG_DTS_EVT(DTS_EV_DTA_UP, 0, m_NCS_NODE_ID_FROM_MDS_DEST(dta->dta_addr),
				      (uint32_t)(dta->dta_addr));
			svc_entry = dta->svc_list;
			while (svc_entry != NULL) {
				dts_send_filter_config_msg(inst, svc_entry->svc, dta);
				dta->updt_req = false;
				svc_entry = svc_entry->next_in_dta_entry;
			}
		}
	}
	/* end of if dta = ncs_patricia_tree_get() */
	else {
		/*Nothing to do */
	}
}

/****************************************************************************
 * Function Name: dts_mds_enc
 * Purpose:        encode a DTS message headed out
 ****************************************************************************/

uint32_t dts_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
		  SS_SVC_ID to_svc, NCS_UBAID *uba,
		  MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver)
{
	uint8_t *data;
	DTSV_MSG *mm;
	NCS_UBAID *payload_uba;

	if (uba == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_enc: User buff is NULL");

	/*Versioning changes - Set message format version same as remote version */
	*msg_fmat_ver = 1;

	mm = (DTSV_MSG *)msg;

	if (mm == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_enc: Message to be encoded is NULL");

	data = ncs_enc_reserve_space(uba, DTSV_DTS_DTA_MSG_HDR_SIZE);
	if (data == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_enc: ncs_enc_reserve_space returns NULL");

	ncs_encode_8bit(&data, mm->msg_type);

	ncs_enc_claim_space(uba, DTSV_DTS_DTA_MSG_HDR_SIZE);

	switch (mm->msg_type) {
	case DTS_SVC_REG_CONF:
		{
			data = ncs_enc_reserve_space(uba, DTSV_REG_CONF_MSG_SIZE);

			if (data == NULL)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_enc: ncs_enc_reserve_space returns NULL");

			ncs_encode_32bit(&data, mm->data.data.reg_conf.msg_fltr.svc_id);
			ncs_encode_32bit(&data, mm->data.data.reg_conf.msg_fltr.enable_log);
			ncs_encode_32bit(&data, mm->data.data.reg_conf.msg_fltr.category_bit_map);
			ncs_encode_8bit(&data, mm->data.data.reg_conf.msg_fltr.severity_bit_map);
			/* No need of policy handles */
			/*ncs_encode_32bit(&data, mm->data.data.reg_conf.msg_fltr.policy_hdl); */

			ncs_enc_claim_space(uba, DTSV_REG_CONF_MSG_SIZE);

			break;
		}
	case DTS_SVC_MSG_FLTR:
		{
			data = ncs_enc_reserve_space(uba, DTSV_FLTR_MSG_SIZE);

			if (data == NULL)
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_enc: ncs_enc_reserve_space returns NULL");

			ncs_encode_32bit(&data, mm->data.data.msg_fltr.svc_id);
			ncs_encode_32bit(&data, mm->data.data.msg_fltr.enable_log);
			ncs_encode_32bit(&data, mm->data.data.msg_fltr.category_bit_map);
			ncs_encode_8bit(&data, mm->data.data.msg_fltr.severity_bit_map);

			ncs_enc_claim_space(uba, DTSV_FLTR_MSG_SIZE);

			break;
		}
	case DTS_FAIL_OVER:
		{
			payload_uba = &mm->data.data.msg.log_msg.uba;
			ncs_enc_append_usrbuf(uba, payload_uba->start);
			break;
		}
#if (DTA_FLOW == 1)
	case DTA_FLOW_CONTROL:
	case DTS_CONGESTION_HIT:
	case DTS_CONGESTION_CLEAR:
		/* Do nothing */
		break;
#endif

	default:
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: dts_mds_dec
 * Purpose:        decode a DTS message coming in
 ****************************************************************************/
uint32_t dts_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT *msg,
		  SS_SVC_ID to_svc, NCS_UBAID *uba, MDS_CLIENT_MSG_FORMAT_VER msg_fmat_ver)
{
	uint8_t *data = NULL;
	DTSV_MSG *mm;
	uint8_t data_buff[DTSV_DTA_DTS_HDR_SIZE];
	uint32_t lenn = 0;

	if ((msg_fmat_ver < DTS_MDS_MIN_MSG_FMAT_VER_SUPPORT) || (msg_fmat_ver > DTS_MDS_MAX_MSG_FMAT_VER_SUPPORT))
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_mds_dec: Message format version is not within acceptable range");

	if (uba == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_dec: DTS decode: user buffer is NULL");

	if (msg == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_dec: DTS decode: Message is NULL");

	mm = m_MMGR_ALLOC_DTSV_MSG;

	if (mm == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_dec: DTS decode: Failed to allocate DTSV message");
	}

	memset(mm, '\0', sizeof(DTSV_MSG));

	*msg = mm;

	data = ncs_dec_flatten_space(uba, data_buff, DTSV_DTA_DTS_HDR_SIZE);
	if (data == NULL) {
		m_MMGR_FREE_DTSV_MSG(mm);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_dec: DTS decode: ncs_dec_flatten_space returns NULL");
	}

	mm->msg_type = ncs_decode_8bit(&data);

	ncs_dec_skip_space(uba, DTSV_DTA_DTS_HDR_SIZE);

	switch (mm->msg_type) {
	case DTA_REGISTER_SVC:
		{
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(SS_SVC_ID));
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: DTS decode: ncs_dec_flatten_space returns NULL");
			}
			mm->data.data.reg.svc_id = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(SS_SVC_ID));

			/* Decode the version no. */
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint16_t));
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: DTS decode: ncs_dec_flatten_space returns NULL");
			}
			mm->data.data.reg.version = ncs_decode_16bit(&data);
			ncs_dec_skip_space(uba, sizeof(uint16_t));

			/* Decode the service name */
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: DTS decode: ncs_dec_flatten_space returns NULL");
			}
			lenn = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(uint32_t));
			if (lenn == 0) {
				/* No need to decode any further. no service name specified */
			} else if (lenn < DTSV_SVC_NAME_MAX) {	/* Check valid len of svc_name */
				ncs_decode_n_octets_from_uba(uba, (uint8_t *)mm->data.data.reg.svc_name, lenn);
			} else {	/* Discard this message */

				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: Length of service name decoded in register message exceeds limits");
			}
			break;
		}
	case DTA_UNREGISTER_SVC:
		{
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(SS_SVC_ID));
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: DTS decode: ncs_dec_flatten_space returns NULL");
			}
			mm->data.data.unreg.svc_id = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(SS_SVC_ID));

			/* Decode the version no. */
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint16_t));
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: DTS decode: ncs_dec_flatten_space returns NULL");
			}
			mm->data.data.unreg.version = ncs_decode_16bit(&data);
			ncs_dec_skip_space(uba, sizeof(uint16_t));

			/* Decode the service name */
			data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
			if (data == NULL) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: DTS decode: ncs_dec_flatten_space returns NULL");
			}
			lenn = ncs_decode_32bit(&data);
			ncs_dec_skip_space(uba, sizeof(uint32_t));
			if (lenn == 0) {
				/* No need to decode any further. no service name specified */
			}
			if (lenn < DTSV_SVC_NAME_MAX) {	/* Check valid len of svc_name */
				ncs_decode_n_octets_from_uba(uba, (uint8_t *)mm->data.data.unreg.svc_name, lenn);
			} else {	/* Discard this message */

				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: Length of service name decoded in unregister msg exceeds limits");
			}
			break;
		}
	case DTA_LOG_DATA:
		{
			/* Versioning changes : Set the msg_fmat_ver field of DTA_LOG_MSG 
			 * according to the message format version from MDS callback. 
			 */
			mm->data.data.msg.msg_fmat_ver = msg_fmat_ver;

			/* Check for any mem failure in dts_log_str_decode */
			if (dts_log_msg_decode(&mm->data.data.msg.log_msg, uba) == NCSCC_RC_FAILURE) {
				m_MMGR_FREE_DTSV_MSG(mm);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dts_mds_dec: dts_log_msg_decode returned failure");
			}
			break;
		}

#if (DTA_FLOW == 1)
	case DTA_FLOW_CONTROL:
	case DTS_CONGESTION_HIT:
	case DTS_CONGESTION_CLEAR:
		/* Do Nothing */
		break;
#endif

	default:
		m_MMGR_FREE_DTSV_MSG(mm);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_dec: Wrong message type is received");
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: dts_mds_cpy
 * Purpose:        copy a DTS message going to somebody in this memory space
 ****************************************************************************/

uint32_t dts_mds_cpy(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg,
		  SS_SVC_ID to_svc, NCSCONTEXT *cpy,
		  bool last, MDS_SVC_PVT_SUB_PART_VER remote_ver, MDS_CLIENT_MSG_FORMAT_VER *msg_fmat_ver)
{
	DTSV_MSG *mm;

	if (msg == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_cpy: msg received is null");

	/* Versioning changes - Set the msg fmat version */
	*msg_fmat_ver = 1;

	mm = m_MMGR_ALLOC_DTSV_MSG;
	if (mm == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_cpy: DTSV message allocation failed");
	}
	memset(mm, '\0', sizeof(DTSV_MSG));

	*cpy = mm;

	memcpy(mm, msg, sizeof(DTSV_MSG));

	switch (mm->msg_type) {
	case DTS_SVC_REG_CONF:
	case DTS_SVC_MSG_FLTR:
		/* nothing to do here... */
		break;
	case DTS_FAIL_OVER:
		{
			break;
		}
#if (DTA_FLOW == 1)
	case DTA_FLOW_CONTROL:
	case DTS_CONGESTION_HIT:
	case DTS_CONGESTION_CLEAR:
		/* Do Nothing */
		break;
#endif

	default:
		if (0 != mm)
			m_MMGR_FREE_DTSV_MSG(mm);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_mds_cpy: Wrong message type");
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function Name: dts_log_msg_decode
 * Purpose:       decodes a NCSDTS_FLTR from a ubaid
 ****************************************************************************/
uint32_t dts_log_msg_decode(NCSFL_NORMAL *logmsg, NCS_UBAID *uba)
{
	uint8_t *data = NULL;
	uint8_t data_buff[DTS_MAX_SIZE_DATA];

	if (uba == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_log_msg_decode: User buffer is NULL");

	if (logmsg == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_log_msg_decode: Message to be decoded is NULL");

	data = ncs_dec_flatten_space(uba, data_buff, DTS_LOG_MSG_HDR_SIZE);
	if (data == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_log_msg_decode: ncs_dec_flatten_space returns NULL");

	logmsg->hdr.time.seconds = ncs_decode_32bit(&data);
	logmsg->hdr.time.millisecs = ncs_decode_32bit(&data);

	logmsg->hdr.ss_id = ncs_decode_32bit(&data);
	logmsg->hdr.inst_id = ncs_decode_32bit(&data);
	logmsg->hdr.severity = ncs_decode_8bit(&data);
	logmsg->hdr.category = ncs_decode_32bit(&data);

	logmsg->hdr.fmat_id = ncs_decode_8bit(&data);

	ncs_dec_skip_space(uba, DTS_LOG_MSG_HDR_SIZE);

	/* Check for any mem failure in dts_log_str_decode */
	if (dts_log_str_decode(uba, &logmsg->hdr.fmat_type) == NCSCC_RC_FAILURE)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				      "dts_log_msg_decode: dts_log_str_decode returned memory alloc failure");

	memcpy(&logmsg->uba, uba, sizeof(NCS_UBAID));

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: dts_log_str_decode
 * Purpose:        Decodes the flexlog string.
 *****************************************************************************/
uint32_t dts_log_str_decode(NCS_UBAID *uba, char **str)
{
	uint32_t length = 0;
	uint8_t *data = NULL;
	uint8_t data_buff[DTS_MAX_SIZE_DATA];

	data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
	if (data == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_log_str_decode: Decode flatten space failed");
	}
	length = ncs_decode_32bit(&data);
	ncs_dec_skip_space(uba, sizeof(uint32_t));

	*str = m_MMGR_ALLOC_OCT(length);
	if (*str == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_log_str_decode: Memory allocation failed");

	memset(*str, '\0', length);

	ncs_decode_n_octets_from_uba(uba, (uint8_t *)*str, length);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: dts_ipaddr_decode
 * Purpose:        Decodes the IP address.
 *****************************************************************************/
uint32_t decode_ip_address(NCS_UBAID *uba, NCS_IP_ADDR *ipa)
{
	uint8_t *data = NULL;
	uint8_t data_buff[sizeof(NCS_IP_ADDR)];

	data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint8_t));
	if (data == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "decode_ip_address: Decode flatten space failed");
	}
	ipa->type = ncs_decode_8bit(&data);
	ncs_dec_skip_space(uba, sizeof(uint8_t));

	if (ipa->type == NCS_IP_ADDR_TYPE_IPV4) {
		data = ncs_dec_flatten_space(uba, data_buff, sizeof(uint32_t));
		ipa->info.v4 = ncs_decode_32bit(&data);
		ncs_dec_skip_space(uba, sizeof(uint32_t));
	}
	else {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}
