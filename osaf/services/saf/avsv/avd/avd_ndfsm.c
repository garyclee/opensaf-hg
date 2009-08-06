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

  DESCRIPTION: This file contains the node state machine related functional
  routines. It is part of the Node submodule.

..............................................................................

  FUNCTIONS INCLUDED in this file:

  avd_node_up_func - node up message handler.
  avd_nd_heartbeat_msg_func - heartbeat message handler.  
  avd_tmr_rcv_hb_nd_func - heartbeat timer expiry handler.
  avd_nd_reg_comp_evt_hdl - Component database update on AvND success 
                            event handler.
  avd_nd_ncs_su_assigned - NCS SU getting successfully assigned with SI event
                           handler.
  avd_nd_ncs_su_failed - NCS SU failure event handler.
  avd_mds_avnd_up_func - MDS node up call back.
  avd_mds_avnd_down_func - MDS node down call back.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

/*****************************************************************************
 * Macro: m_AVD_UNDO_UP_CHNG
 *
 * Purpose:  This macro will undo the changes done for the node when
 *           node update message couldnt be sent. It also 
 *           Decrements the global view number.
 *
 * Input: cb - the AVD control block pointer.
 *        avnd - pointer to the avnd structure of the node.
 *        
 *
 * Return: none.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/
#define m_AVD_UNDO_UP_CHNG(cb,avnd) \
{\
   avnd->node_info.bootTimestamp = 0; \
   memset(&(avnd->adest),'\0',sizeof(MDS_DEST)); \
   memset(&(avnd->node_info.nodeAddress),'\0',sizeof(SaClmNodeAddressT)); \
   avnd->rcv_msg_id = 0; \
   cb->cluster_view_number --; \
   avnd->node_info.initialViewNumber = 0; \
   avnd->node_info.member = SA_FALSE; \
}

/*****************************************************************************
 * Function: avd_node_up_func
 *
 * Purpose:  This function is the handler for node up event indicating
 * the arrival of the node_up message. Based on the state machine either
 * It will ignore the message or send all the reg messages to the node. 
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_node_up_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_AVND *avnd = AVD_AVND_NULL;
	AVD_DND_MSG *n2d_msg;
	NCS_BOOL comp_sent;
	uns32 rc = NCSCC_RC_SUCCESS;

	m_AVD_LOG_FUNC_ENTRY("avd_node_up_func");

	if (evt->info.avnd_msg == NULL) {
		/* log error that a message contents is missing */
		m_AVD_LOG_INVALID_VAL_ERROR(0);
		return;
	}

	n2d_msg = evt->info.avnd_msg;

	m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG, n2d_msg, sizeof(AVD_DND_MSG), n2d_msg);

	if ((avnd = avd_msg_sanity_chk(cb, evt, n2d_msg->msg_info.n2d_clm_node_up.node_id, AVSV_N2D_CLM_NODE_UP_MSG))
	    == AVD_AVND_NULL) {
		/* sanity failed return */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/* Check the AvD FSM state process node up only if AvD is in init done or
	 * APP init state for all nodes except the primary system controller
	 * whose node up is accepted in config done state.
	 */

	if ((n2d_msg->msg_info.n2d_clm_node_up.node_id != cb->node_id_avd) && (cb->init_state < AVD_INIT_DONE)) {
		avd_log(NCSFL_SEV_WARNING, "invalid init state (%u), node %x",
			cb->init_state, n2d_msg->msg_info.n2d_clm_node_up.node_id);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	if (avnd->node_state != AVD_AVND_STATE_ABSENT) {
		avd_log(NCSFL_SEV_WARNING, "invalid node state %u for node %x",
			avnd->node_state, n2d_msg->msg_info.n2d_clm_node_up.node_id);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/* Retrive the information from the message */
	avnd->node_info.bootTimestamp = n2d_msg->msg_info.n2d_clm_node_up.boot_timestamp;
	memcpy(&avnd->node_info.nodeAddress, &n2d_msg->msg_info.n2d_clm_node_up.node_address,
	       sizeof(SaClmNodeAddressT));
	avnd->adest = n2d_msg->msg_info.n2d_clm_node_up.adest_address;
	avnd->rcv_msg_id = n2d_msg->msg_info.n2d_clm_node_up.msg_id;

	/* Increment the view number and treat that as the
	 * new view number as a new node has joined the
	 * cluster.
	 */

	avnd->node_info.initialViewNumber = ++(cb->cluster_view_number);
	avnd->node_info.member = SA_TRUE;

	/* free the received message */
	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;

	/* Broadcast the node update to all the node directors only
	 * those that are up will use the update.
	 */

	if (avd_snd_node_update_msg(cb, avnd) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to broad cast */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
		/* undo all the changes done so that the node is still considered
		 * down.
		 */
		m_AVD_UNDO_UP_CHNG(cb, avnd);

		return;
	}

	/* Identify if this AVND is running on the same node as AVD */
	if ((avnd->node_info.nodeId == cb->node_id_avd) || (avnd->node_info.nodeId == cb->node_id_avd_other)) {
		avnd->type = AVSV_AVND_CARD_SYS_CON;

	}

	/*Intialize the heart beat receive indication value  */
	avnd->hrt_beat_rcvd = FALSE;

	/* send the node up message to the node.
	 */
	if (avd_snd_node_up_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
		/* free the node up message */

		/* call the routine to failover all the effected nodes
		 * due to restarting this node
		 */

		avd_node_down_func(cb, avnd);
		return;
	}

	/* send the Ack message to the node.
	 */
	if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);

		/* call the routine to failover all the effected nodes
		 * due to restarting this node
		 */
		avd_node_down_func(cb, avnd);
		return;
	}

	/* start the heartbeat timer for node director of non sys
	 * controller.
	 */
	if (avnd->type != AVSV_AVND_CARD_SYS_CON) {
		m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);
		m_AVD_HB_TMR_START(cb, avnd);
		m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);
	}

	/* Send role change to this controller AvND */
	if (avnd->node_info.nodeId == cb->node_id_avd) {
		/* Here obviously the role will be ACT. */
		rc = avd_avnd_send_role_change(cb, cb->node_id_avd, cb->avail_state_avd);
		if (NCSCC_RC_SUCCESS != rc) {
			m_AVD_PXY_PXD_ERR_LOG("avd_avnd_send_role_change failed. Node Id is",
					      NULL, cb->node_id_avd, 0, 0, 0);
		}
	}
	/* Send role change to this controller AvND */
	if (avnd->node_info.nodeId == cb->node_id_avd_other) {
		/* Here obviously the role will be STDBY. Here we are not picking the
		   state from "avail_state_avd_other" as it may happen that the cold 
		   sync would be in progress and there has been no heart beat exchange
		   till now, so "avail_state_avd_other" might not have been updated. */
		rc = avd_avnd_send_role_change(cb, cb->node_id_avd_other, SA_AMF_HA_STANDBY);
		if (NCSCC_RC_SUCCESS != rc) {
			m_AVD_PXY_PXD_ERR_LOG("avd_avnd_send_role_change failed. Peer AvND Node Id is",
					      NULL, cb->node_id_avd, 0, 0, 0);
		}
	}

	/* upload all the data from director to node director. By sending all the
	 * information messages.
	 */

	if (avd_snd_hlt_msg(cb, avnd, AVD_HLT_NULL, FALSE, FALSE) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
		/* we are in a bad shape. Restart the node for recovery */

		/* call the routine to failover all the effected nodes
		 * due to restarting this node
		 */

		avd_node_down_func(cb, avnd);
		return;
	}

	if (avd_snd_su_comp_msg(cb, avnd, &comp_sent, FALSE) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
		/* we are in a bad shape. Restart the node for recovery */

		/* call the routine to failover all the effected nodes
		 * due to restarting this node
		 */

		avd_node_down_func(cb, avnd);
		return;
	}

	/* If component message is sent change state to no config or else skip
	 * all the comming up states and jump to present state. This is because
	 * the node doesnt have any components meaning that even the NCS component
	 * is not present.
	 */
	if (comp_sent == TRUE) {
		avnd->node_state = AVD_AVND_STATE_NO_CONFIG;
		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
	} else
		avd_nd_reg_comp_evt_hdl(cb, avnd);

	/* checkpoint the node. */
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_UP_INFO);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, cb, AVSV_CKPT_CB_CL_VIEW_NUM);

	return;
}

/*****************************************************************************
 * Function: avd_nd_heartbeat_msg_func
 *
 * Purpose:  This function is the handler for node director heartbeat event
 * indicating the arrival of the heartbeat message from node director. Based
 * on the state machine it will process the message accordingly.
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void avd_nd_heartbeat_msg_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_DND_MSG *n2d_msg;
	AVD_AVND *avnd;

	m_AVD_LOG_FUNC_ENTRY("avd_nd_heartbeat_msg_func");

	if (evt->info.avnd_msg == NULL) {
		/* log error that a message contents is missing */
		m_AVD_LOG_INVALID_VAL_ERROR(0);
		return;
	}

	n2d_msg = evt->info.avnd_msg;

	m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

	if ((avnd = avd_msg_sanity_chk(cb, evt, n2d_msg->msg_info.n2d_hrt_bt.node_id, AVSV_N2D_HEARTBEAT_MSG))
	    == AVD_AVND_NULL) {
		/* sanity failed return */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);
		return;
	}

	if ((avnd->node_state == AVD_AVND_STATE_ABSENT) || (avnd->node_state == AVD_AVND_STATE_GO_DOWN)) {
		/* log information error that the node is in invalid state */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);
		return;
	}

	/* stop the timer if it exists */
	avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));

	/* restart the heart beat timer for payload card only */
	if ((avnd->type != AVSV_AVND_CARD_SYS_CON) && (cb->avail_state_avd == SA_AMF_HA_ACTIVE)) {
		m_AVD_HB_TMR_START(cb, avnd);
	}

	/* check that this is the first heart Beat message after
	   system start or after a heart beat timer expiry .If 
	   yes than inform AVM 
	 */
	if (FALSE == avnd->hrt_beat_rcvd) {
		if (NCSCC_RC_SUCCESS != avd_avm_nd_hb_restore_msg(cb, avnd->node_info.nodeId)) {
			/* log error that the node id is invalid */
			m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
			m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

			avsv_dnd_msg_free(n2d_msg);
			evt->info.avnd_msg = NULL;
			return;
		}
		/*Message Successfully sent: RDE informed */
		/*Set the avnd heart beat received value to True  */
		avnd->hrt_beat_rcvd = TRUE;
	}

	m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

	/* free the received message */
	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;

	return;
}

/*****************************************************************************
 * Function: avd_tmr_rcv_hb_nd_func
 *
 * Purpose:  This function is the handler for node director receive heartbeat 
 * timeout event indicating the timer expiry of receive heartbeat timer for
 *  node director. Based on the state machine it will process the 
 * timeout accordingly.
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void avd_tmr_rcv_hb_nd_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_AVND *avnd = AVD_AVND_NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_tmr_rcv_hb_nd_func");

	if (evt->info.tmr.type != AVD_TMR_RCV_HB_ND) {
		/* log error that a wrong timer type value */
		m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.type);
		return;
	}

	if (cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* log error that a cluster is admin down */
		m_AVD_LOG_INVALID_VAL_ERROR(cb->cluster_admin_state);
		return;
	}

	if (cb->init_state < AVD_CFG_DONE) {
		/* This is a error situation. Without completing
		 * initialization the AVD will not respond to
		 * AVND.
		 */

		/* log error */
		m_AVD_LOG_INVALID_VAL_FATAL(cb->init_state);
		return;
	}

	syslog(LOG_WARNING, "AVD: Heart Beat missed with node director on %x", evt->info.tmr.node_id);

	/* get avnd ptr to call avd_avm_mark_nd_absent */
	if ((avnd = avd_avnd_struc_find_nodeid(cb, evt->info.tmr.node_id)) == AVD_AVND_NULL) {
		/* we can't do anything without getting avnd ptr. just return */
		m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.node_id);
		return;
	}

	/*Reset the first heart beat receive boolean variable
	   present in avnd structure */

	avnd->hrt_beat_rcvd = FALSE;

	/* Inform LFM about the heartbeat loss. All the other processing
	 * is delayed until a response is received from LFM
	 *
	 * Mark the node status as ???
	 */
	if (NCSCC_RC_SUCCESS != avd_avm_nd_hb_lost_msg(cb, evt->info.tmr.node_id)) {
		/* log error that the node id is invalid */
		m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.node_id);
		return;
	}

	/* check if the node was undergoing shutdown, if so send shutdown response */
	if (avnd->node_state == AVD_AVND_STATE_SHUTTING_DOWN)
		avd_avm_send_reset_req(cb, &avnd->node_info.nodeName);

	return;
}

/*****************************************************************************
 * Function: avd_nd_reg_comp_evt_hdl
 *
 * Purpose:  This function is the handler for node director event when the comp
 * message acknowledgement is received indicating that all the configuration
 * on the node is done. This function will instantiate all the NCS SUs on
 * the node. If none exists it will change the node FSM state to present and
 * call the AvD state machine.
 *
 * Input: cb - the AVD control block
 *        avnd - The AvND which has sent the ack for all the component additions.
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void avd_nd_reg_comp_evt_hdl(AVD_CL_CB *cb, AVD_AVND *avnd)
{
	AVD_SU *ncs_su;
	struct {
		uns32 seconds;
		uns32 millisecs;
	} boot_timestamp;

	m_AVD_LOG_FUNC_ENTRY("avd_nd_reg_comp_evt_hdl");

	/* Check the AvND structure to see if any NCS SUs exist on the node.
	 * If none exist change the FSM state present*/
	if ((ncs_su = avnd->list_of_ncs_su) == AVD_SU_NULL) {
		/* now change the state to present */
		avnd->node_state = AVD_AVND_STATE_PRESENT;
		avnd->oper_state = NCS_OPER_STATE_ENABLE;

		cb->cluster_num_nodes++;

		/* We can now set the LEDS */
		avd_snd_set_leds_msg(cb, avnd);

		/* check if this is the primary system controller if yes change the
		 * state of the AvD to init done state.
		 */
		if (avnd->node_info.nodeId == cb->node_id_avd) {
			cb->init_state = AVD_INIT_DONE;
			/* Note the current time for cluster boottime */
			m_GET_MSEC_TIME_STAMP(&boot_timestamp.seconds, &boot_timestamp.millisecs);
			cb->cluster_init_time = ((uns64)boot_timestamp.seconds *
						 (uns64)(AVSV_NANOSEC_TO_LEAPTM) * (uns64)100)
			    + ((uns64)boot_timestamp.millisecs * (uns64)(AVSV_NANOSEC_TO_LEAPTM / 10));
			/* start the cluster init timer. */
			m_AVD_CLINIT_TMR_START(cb);
		}

		/* Instantiate the application SUs */
		avd_sg_app_node_su_inst_func(cb, avnd);

		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_OPER_STATE);
		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, cb, AVSV_CKPT_AVD_CB_CONFIG);

		return;
	}

	avnd->node_state = AVD_AVND_STATE_NCS_INIT;
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);

	/* Instantiate all the NCS SUs on this node by sending the presence state
	 * message for each of the SUs whose components have all been configured.
	 */
	while (ncs_su != AVD_SU_NULL) {
		if ((ncs_su->row_status != NCS_ROW_ACTIVE) || (ncs_su->num_of_comp != ncs_su->curr_num_comp)) {
			/* skip these incomplete SUs. */
			ncs_su = ncs_su->avnd_list_su_next;
			continue;
		}

		/* Here an assumption is made that all NCS SUs are preinstantiable */
		avd_snd_presence_msg(cb, ncs_su, FALSE);
		ncs_su = ncs_su->avnd_list_su_next;
	}

	return;
}

/*****************************************************************************
 * Function: avd_nd_ncs_su_assigned
 *
 * Purpose:  This function is the handler for node director event when a
 *           NCS SU is assigned with a SI. It verifies that all the
 *           NCS SUs are assigned and calls the SG module instantiation
 *           function for each of the SUs on the node. It will also change the 
 *           node FSM state to present and call the AvD state machine.
 *
 * Input: cb - the AVD control block
 *        avnd - The AvND which has sent the ack for all the component additions.
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void avd_nd_ncs_su_assigned(AVD_CL_CB *cb, AVD_AVND *avnd)
{
	AVD_SU *ncs_su;
	struct {
		uns32 seconds;
		uns32 millisecs;
	} boot_timestamp;

	m_AVD_LOG_FUNC_ENTRY("avd_nd_ncs_su_assigned");

	ncs_su = avnd->list_of_ncs_su;

	while (ncs_su != AVD_SU_NULL) {
		if ((ncs_su->row_status != NCS_ROW_ACTIVE) || (ncs_su->num_of_comp != ncs_su->curr_num_comp)) {
			/* skip these incomplete SUs. */
			ncs_su = ncs_su->avnd_list_su_next;
			continue;
		}

		if ((ncs_su->list_of_susi == AVD_SU_SI_REL_NULL) ||
		    (ncs_su->list_of_susi->fsm != AVD_SU_SI_STATE_ASGND)) {
			/* this is an unassigned SU so no need to scan further return here. */
			return;
		}

		ncs_su = ncs_su->avnd_list_su_next;
	}

	/* All the NCS SUs are assigned now change the state to present */
	if (avnd->node_state != AVD_AVND_STATE_PRESENT) {
		avnd->node_state = AVD_AVND_STATE_PRESENT;
		avnd->oper_state = NCS_OPER_STATE_ENABLE;

		cb->cluster_num_nodes++;

		/* We can now set the LEDS */
		avd_snd_set_leds_msg(cb, avnd);

		/* check if this is the primary system controller if yes change the
		 * state of the AvD to init done state.
		 */
		if (avnd->node_info.nodeId == cb->node_id_avd) {
			cb->init_state = AVD_INIT_DONE;
			/* Note the current time for cluster boottime */
			m_GET_MSEC_TIME_STAMP(&boot_timestamp.seconds, &boot_timestamp.millisecs);
			cb->cluster_init_time = ((uns64)boot_timestamp.seconds *
						 (uns64)(AVSV_NANOSEC_TO_LEAPTM) * (uns64)100)
			    + ((uns64)boot_timestamp.millisecs * (uns64)(AVSV_NANOSEC_TO_LEAPTM / 10));

			/* start the cluster init timer. */
			m_AVD_CLINIT_TMR_START(cb);
		}

		/* Instantiate the application SUs. */
		avd_sg_app_node_su_inst_func(cb, avnd);

	}

	/* if (avnd->node_state != AVD_AVND_STATE_PRESENT) */
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_OPER_STATE);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, cb, AVSV_CKPT_AVD_CB_CONFIG);

	return;
}

/*****************************************************************************
 * Function: avd_nd_ncs_su_failed
 *
 * Purpose:  This function is the handler for node director event when a
 *           NCS SUs operation state goes to disabled. The functionality as
 *           for heartbeat failure is executed w.r.t to the node.
 *
 * Input: cb - the AVD control block
 *        avnd - The AvND which has sent the ack for all the component additions.
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void avd_nd_ncs_su_failed(AVD_CL_CB *cb, AVD_AVND *avnd)
{

	m_AVD_LOG_FUNC_ENTRY("avd_nd_ncs_su_failed");

	/* call the function to down this node */
	avd_node_down_func(cb, avnd);

	/* reboot the AvD if the AvND matches self and AVD is quiesced. */
	if ((avnd->node_info.nodeId == cb->node_id_avd) && (cb->avail_state_avd == SA_AMF_HA_QUIESCED)) {
		ncs_reboot("Node failed in qsd state");
	}

	return;

}

/*****************************************************************************
 * Function: avd_mds_avnd_up_func
 *
 * Purpose:  This function is the handler for the AvND up event from
 * mds. The function right now is just a place holder.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_mds_avnd_up_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	m_AVD_LOG_FUNC_ENTRY("avd_mds_avnd_up_func");
	return;
}

/*****************************************************************************
 * Function: avd_mds_avnd_down_func
 *
 * Purpose:  This function is the handler for the AvND down event from
 * mds. The function right now is just a place holder.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_mds_avnd_down_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	m_AVD_LOG_FUNC_ENTRY("avd_mds_avnd_down_func");
	return;
}

/*****************************************************************************
 * Function: avd_fail_over_event
 *
 * Purpose:  This function is the handler for the AVD fail-over event. This 
 *           function will be called on receiving fail-over callback at standby.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/
void avd_fail_over_event(AVD_CL_CB *cb)
{
	AVD_AVND *avnd;
	SaClmNodeIdT node_id = 0;
	AVD_FAIL_OVER_NODE *node_to_add;

	/* Mark this AVD as one recovering from fail-over */
	cb->avd_fover_state = TRUE;

	/* reset the count - number of nodes in cluster */
	cb->cluster_num_nodes = 0;

	/* Walk through all the nodes and send verify message to them. */
	while (NULL != (avnd = (AVD_AVND *)ncs_patricia_tree_getnext(&cb->avnd_anchor, (uns8 *)&node_id))) {
		node_id = avnd->node_info.nodeId;

		/* lets calculate the number of nodes in cluster */
		if (AVD_AVND_STATE_ABSENT != avnd->node_state) {
			cb->cluster_num_nodes++;
		}

		/*
		 * If AVND state machine is in Absent state then just return.
		 */
		if ((AVD_AVND_STATE_ABSENT == avnd->node_state) ||
		    ((node_id == cb->node_id_avd_other) && cb->role_switch == FALSE)) {
			continue;
		}

		/*
		 * Check if we are in present state. If yes then send DATA verify 
		 * message to all the AVND's.
		 */
		if (AVD_AVND_STATE_PRESENT == avnd->node_state) {
			/*
			 * Send verify message to this node.
			 */
			if (NCSCC_RC_SUCCESS != avd_snd_node_data_verify_msg(cb, avnd)) {
				/* Log Error */
				return;
			}

			/* we should send the above data verify msg right now */
			avd_d2n_msg_dequeue(cb);

			/* remove the PG record */
			avd_pg_node_csi_del_all(cb, avnd);

			/* 
			 * Start the heartbeat timer for node director of non sys
			 * controller.
			 */
			if (avnd->type != AVSV_AVND_CARD_SYS_CON) {
				m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);
				avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));
				m_AVD_HB_TMR_START(cb, avnd);
				m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);
			}

			/* Add this node in our fail-over node list */
			if (NULL == (node_to_add = m_MMGR_ALLOC_CLM_NODE_ID)) {
				/* Log Error */
				return;
			}

			memset((char *)node_to_add, '\0', sizeof(AVD_FAIL_OVER_NODE));

			node_to_add->node_id = avnd->node_info.nodeId;
			node_to_add->tree_node_id_node.key_info = (uns8 *)&(node_to_add->node_id);
			node_to_add->tree_node_id_node.bit = 0;
			node_to_add->tree_node_id_node.left = NCS_PATRICIA_NODE_NULL;
			node_to_add->tree_node_id_node.right = NCS_PATRICIA_NODE_NULL;

			if (ncs_patricia_tree_add(&cb->node_list, &node_to_add->tree_node_id_node) != NCSCC_RC_SUCCESS) {
				/* log an error */
				m_MMGR_FREE_CLM_NODE_ID(node_to_add);
				return;
			}
		} else {
			/*
			 * In case of all other states, send HPI event to restart the node.
			 */
			avd_node_down_func(cb, avnd);
		}

		/*
		 * Since we are sending verify message, this is the time to reset
		 * our send ID counter so that next time onwards messages will 
		 * be sent with new send ID.
		 */
		avnd->snd_msg_id = 0;
	}
}

/*****************************************************************************
 * Function: avd_ack_nack_event
 *
 * Purpose:  This function is the handler for the AVD ACK_NACK event message 
 *           received from the AVND in response to DATA_VERIFY message.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/
void avd_ack_nack_event(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_AVND *avnd;
	AVD_SU *su_ptr;
	AVD_SU_SI_REL *rel_ptr;
	NCS_BOOL comp_sent;
	AVD_FAIL_OVER_NODE *node_fovr;
	AVD_DND_MSG *n2d_msg;

	n2d_msg = evt->info.avnd_msg;
	/* Find if node is there in the f-over node list. If yes then remove entry
	 * and process the message. Else just return.
	 */
	if (NULL != (node_fovr =
		     (AVD_FAIL_OVER_NODE *)ncs_patricia_tree_get(&cb->node_list,
								 (uns8 *)&evt->info.avnd_msg->msg_info.
								 n2d_ack_nack_info.node_id))) {
		ncs_patricia_tree_del(&cb->node_list, &node_fovr->tree_node_id_node);
		m_MMGR_FREE_CLM_NODE_ID(node_fovr);
	} else {
		/* do i need to log an error */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/*
	 * Check if AVND is in the present state. If No then drop this event.
	 */
	if (NULL == (avnd = avd_avnd_struc_find_nodeid(cb, evt->info.avnd_msg->msg_info.n2d_ack_nack_info.node_id))) {
		/* Not an error? Log information will be helpful */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	if (AVD_AVND_STATE_PRESENT != avnd->node_state) {
		/* Not an error? Log information will be helpful */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	if ((TRUE == evt->info.avnd_msg->msg_info.n2d_ack_nack_info.ack) &&
	    (TRUE == evt->info.avnd_msg->msg_info.n2d_ack_nack_info.v_num_ack)) {
		/* Wow great!! We are in sync with this node...Log inforamtion */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/*
	 * In case of view number nack, send information of all the cluster
	 * nodes to this AVND.
	 */
	if (evt->info.avnd_msg->msg_info.n2d_ack_nack_info.v_num_ack == FALSE) {
		/* Log inormation */
		if (NCSCC_RC_SUCCESS != avd_snd_node_info_on_fover_msg(cb, avnd)) {
			m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
			/* we are in a bad shape. Restart the node for recovery */

			/* call the routine to failover all the effected nodes
			 * due to restarting this node
			 */
			avd_node_down_func(cb, avnd);
			avsv_dnd_msg_free(n2d_msg);
			evt->info.avnd_msg = NULL;
			return;
		}
	}

	/*
	 * In case AVND is in present state and if NACK is received then 
	 * Send entire configuration to AVND. Seems like we received NACK :(,
	 * Log information that we received NACK.
	 */
	if (FALSE == evt->info.avnd_msg->msg_info.n2d_ack_nack_info.ack) {
		/* Log Information */
		if (avd_snd_hlt_msg(cb, avnd, AVD_HLT_NULL, TRUE, FALSE) != NCSCC_RC_SUCCESS) {
			m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
			/* we are in a bad shape. Restart the node for recovery */

			/* call the routine to failover all the effected nodes
			 * due to restarting this node
			 */

			avd_node_down_func(cb, avnd);
			avsv_dnd_msg_free(n2d_msg);
			evt->info.avnd_msg = NULL;
			return;
		}

		if (avd_snd_su_comp_msg(cb, avnd, &comp_sent, TRUE) != NCSCC_RC_SUCCESS) {
			m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
			/* we are in a bad shape. Restart the node for recovery */

			/* call the routine to failover all the effected nodes
			 * due to restarting this node
			 */

			avd_node_down_func(cb, avnd);
			avsv_dnd_msg_free(n2d_msg);
			evt->info.avnd_msg = NULL;
			return;
		}

		/*
		 * Send SU_SI relationship which are in ASSIGN, MODIFY and  
		 * UNASSIGN state.for this node.
		 */
		for (su_ptr = avnd->list_of_ncs_su; su_ptr != NULL; su_ptr = su_ptr->avnd_list_su_next) {
			for (rel_ptr = su_ptr->list_of_susi; rel_ptr != NULL; rel_ptr = rel_ptr->su_next) {
				if ((AVD_SU_SI_STATE_ASGND == rel_ptr->fsm) || (AVD_SU_SI_STATE_ABSENT == rel_ptr->fsm))
					continue;

				if (avd_snd_susi_msg(cb, su_ptr, rel_ptr, rel_ptr->fsm) != NCSCC_RC_SUCCESS) {
					/* log a fatal error that a message couldn't be sent */
					m_AVD_LOG_INVALID_VAL_ERROR(((long)su_ptr));
					m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su_ptr->name_net.value,
									     su_ptr->name_net.length);
				}
			}
		}

		/* check the LED status and send a msg if required */
		if (avnd->node_state == AVD_AVND_STATE_PRESENT && avnd->oper_state == NCS_OPER_STATE_ENABLE) {
			/* We can now set the LEDS */
			avd_snd_set_leds_msg(cb, avnd);
		}

		/*
		 * We have take care of NCS SU's, now do the same for normal SU's.
		 */
		for (su_ptr = avnd->list_of_su; su_ptr != NULL; su_ptr = su_ptr->avnd_list_su_next) {
			/* check if susi. If not continue */
			if (!su_ptr->list_of_susi)
				continue;

			/* if SG is 2n,N+M,N-way active and susi is modifying send susi relation */
			if ((su_ptr->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) ||
			    (su_ptr->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_NPM) ||
			    (su_ptr->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_NWAYACTV)) {
				if (AVD_SU_SI_STATE_MODIFY == su_ptr->list_of_susi->fsm) {
					avd_snd_susi_msg(cb, su_ptr, AVD_SU_SI_REL_NULL, AVSV_SUSI_ACT_MOD);
					continue;
				}
			}

			for (rel_ptr = su_ptr->list_of_susi; rel_ptr != NULL; rel_ptr = rel_ptr->su_next) {
				if ((AVD_SU_SI_STATE_ASGND == rel_ptr->fsm) || (AVD_SU_SI_STATE_ABSENT == rel_ptr->fsm))
					continue;

				if (avd_snd_susi_msg(cb, su_ptr, rel_ptr, rel_ptr->fsm) != NCSCC_RC_SUCCESS) {
					/* log a fatal error that a message couldn't be sent */
					m_AVD_LOG_INVALID_VAL_ERROR(((long)su_ptr));
					m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su_ptr->name_net.value,
									     su_ptr->name_net.length);
				}
			}
		}
	}

	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;
}

/*****************************************************************************
 * Function: avd_node_down
 *
 * Purpose:  This function is set this node to down.
 *
 * Input: cb - the AVD control block
 *        node_id - Node ID.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/
uns32 avd_node_down(AVD_CL_CB *cb, SaClmNodeIdT node_id)
{

	AVD_AVND *avnd;

	m_AVD_LOG_FUNC_ENTRY("avd_node_down");

	if ((avnd = avd_avnd_struc_find_nodeid(cb, node_id)
	    ) == AVD_AVND_NULL) {
		/* log error that the node id is invalid */
		m_AVD_LOG_INVALID_VAL_FATAL(node_id);
		return NCSCC_RC_FAILURE;
	}

	if (avnd->row_status != NCS_ROW_ACTIVE) {
		/* log error that the node is not valid */
		m_AVD_LOG_INVALID_VAL_FATAL(node_id);
		return NCSCC_RC_FAILURE;
	}

	if ((avnd->node_state == AVD_AVND_STATE_ABSENT) || (avnd->node_state == AVD_AVND_STATE_GO_DOWN)) {
		/* log information error that the node is in invalid state */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		/* ignore the heartbeat timeout */
		return NCSCC_RC_FAILURE;
	}

	/* log an error that the heartbeat failed so the node is down */

	/* call the routine to failover all the effected nodes
	 * due to restarting this node
	 */

	/* call the function to down this node */
	avd_node_down_func(cb, avnd);

	return NCSCC_RC_SUCCESS;
}
