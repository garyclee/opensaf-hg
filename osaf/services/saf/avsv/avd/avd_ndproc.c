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

  DESCRIPTION: This file contains the node related events processing.
  Some of these node realted events might effect the AvNDs state machine. Some
  messages might trigger the configuration module processing. It is part of
  the node submodule.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_msg_sanity_chk -  sanity check w.r.t the message.
  avd_reg_hlth_func - health check data message response handler.
  avd_reg_su_func - SU message response handler.
  avd_reg_comp_func - component message response handler.
  avd_node_down_func - utility to shut down the node.
  avd_tmr_rcv_hb_nd_func - heartbeat failure event handler.
  avd_oper_req_func - operation request message response handler.
  avd_data_update_req_func - data update message handler.

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <immutil.h>
#include <logtrace.h>

#include <avd.h>
#include <avd_imm.h>
#include <avd_cluster.h>
#include <logtrace.h>

/*****************************************************************************
 * Function: avd_msg_sanity_chk
 *
 * Purpose:  This function does a sanity check w.r.t the message received and
 *           returns the avnd structure related to the node.
 *
 * Input: cb - the AVD control block pointer.
 *        evt - The event information.
 *        node_id - The node id in the message.
 *        msg_typ - the message type for which sanity needs to be done.
 *        
 *
 * Return: avnd  - pointer to the avnd structure of the node. NULL if
 *                 failure.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/
AVD_AVND *avd_msg_sanity_chk(AVD_CL_CB *cb, AVD_EVT *evt, SaClmNodeIdT node_id, AVSV_DND_MSG_TYPE msg_typ)
{
	AVD_AVND *avnd = NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_msg_sanity_chk");

	m_AVD_LOG_MSG_DND_RCV_INFO(AVD_LOG_PROC_MSG, evt->info.avnd_msg, node_id);

	if (avd_cluster->saAmfClusterAdminState != SA_AMF_ADMIN_UNLOCKED) {
		avd_log(NCSFL_SEV_ERROR, "cluster admin state down");
		return avnd;
	}

	if (cb->init_state < AVD_CFG_DONE) {
		/* Don't initialise the AvND when the AVD is not
		 * completely initialised with the saved information
		 */
		avd_log(NCSFL_SEV_WARNING, "invalid init state (%u)", cb->init_state);
		return avnd;
	}

	if (evt->info.avnd_msg->msg_type != msg_typ) {
		avd_log(NCSFL_SEV_ERROR, "wrong message type (%u)", evt->info.avnd_msg->msg_type);
		return avnd;
	}

	if ((avnd = avd_node_find_nodeid(node_id)) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "invalid node ID (%x)", node_id);
		return avnd;
	}

	/* sanity check done return the avnd structure */
	return avnd;
}

/*****************************************************************************
 * Function: avd_reg_su_func
 *
 * Purpose:  This function is the handler for reg_su event
 * indicating the arrival of the reg_su message response. Based on the state
 * machine and the error value returned, it will processes the message
 * accordingly.
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

void avd_reg_su_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_DND_MSG *n2d_msg = evt->info.avnd_msg;
	AVD_AVND *avnd;

	TRACE_ENTER2("from %x, '%s'", n2d_msg->msg_info.n2d_reg_su.node_id, n2d_msg->msg_info.n2d_reg_su.su_name.value);

	if ((avnd = avd_msg_sanity_chk(cb, evt, n2d_msg->msg_info.n2d_reg_su.node_id, AVSV_N2D_REG_SU_MSG))
	    == NULL) {
		/* sanity failed return */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		goto done;
	}

	if ((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
	    (avnd->node_state == AVD_AVND_STATE_GO_DOWN) ||
	    ((avnd->rcv_msg_id + 1) != n2d_msg->msg_info.n2d_reg_su.msg_id)) {
		/* log information error that the node is in invalid state */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->rcv_msg_id);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		goto done;
	}

	/* Update the receive id for the node */
	m_AVD_SET_AVND_RCV_ID(cb, avnd, (n2d_msg->msg_info.n2d_reg_su.msg_id));

	/* 
	 * Send the Ack message to the node, indicationg that the message with this
	 * message ID is received successfully.
	 */
	if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
	}

	/* check if this is a operator initiated update */
	if (avnd->node_state == AVD_AVND_STATE_PRESENT) {
		if (n2d_msg->msg_info.n2d_reg_su.error != NCSCC_RC_SUCCESS) {
			/* Cannot do much, let the operator clean up the mess */
			LOG_ER("%s: '%s' node=%x FAILED to register", __FUNCTION__,
			       n2d_msg->msg_info.n2d_reg_su.su_name.value, n2d_msg->msg_info.n2d_reg_su.node_id);
		}

		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		goto done;
	}

	/* check the ack message for errors. If error stop and free all the
	 * messages in the list and issue HPI restart of the node
	 */

	if (n2d_msg->msg_info.n2d_reg_su.error == NCSCC_RC_SUCCESS) {
		/* the AVND has been successfully updated with the SU information
		 */

		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		goto done;
	}

	/* log an error since this shouldn't happen */

	m_AVD_LOG_INVALID_VAL_FATAL(n2d_msg->msg_info.n2d_reg_su.error);

	/* call the routine to failover all the effected nodes
	 * due to restarting this node
	 */

	avd_node_down_func(cb, avnd);
	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;

	/* checkpoint the AVND structure */
 done:
	TRACE_LEAVE();
}

/*****************************************************************************
 * Function: avd_reg_comp_func
 *
 * Purpose:  This function is the handler for reg of component
 * event indicating the arrival of the reg_comp message response.
 * Based on the state machine and the error value returned, it will process the
 * message accordingly.
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

void avd_reg_comp_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_DND_MSG *n2d_msg;
	AVD_AVND *avnd;

	m_AVD_LOG_FUNC_ENTRY("avd_reg_comp_func");

	if (evt->info.avnd_msg == NULL) {
		/* log error that a message contents is missing */
		m_AVD_LOG_INVALID_VAL_ERROR(0);
		return;
	}

	n2d_msg = evt->info.avnd_msg;

	m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG, n2d_msg, sizeof(AVD_DND_MSG), n2d_msg);

	if ((avnd = avd_msg_sanity_chk(cb, evt, n2d_msg->msg_info.n2d_reg_comp.node_id, AVSV_N2D_REG_COMP_MSG))
	    == NULL) {
		/* sanity failed return */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	if ((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
	    (avnd->node_state == AVD_AVND_STATE_GO_DOWN) ||
	    ((avnd->rcv_msg_id + 1) != n2d_msg->msg_info.n2d_reg_comp.msg_id)) {
		/* log information error that the node is in invalid state */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->rcv_msg_id);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/* Update the receive id for the node */
	m_AVD_SET_AVND_RCV_ID(cb, avnd, (n2d_msg->msg_info.n2d_reg_comp.msg_id));

	/* 
	 * Send the Ack message to the node, indicationg that the message with this
	 * message ID is received successfully.
	 */
	if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
	}

	/* check if this is a operator initiated update */
	if (avnd->node_state == AVD_AVND_STATE_PRESENT) {
		avd_comp_ack_msg(cb, n2d_msg);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/* check the ack message for errors. If error stop and free all the
	 * messages in the list and issue HPI restart of the node
	 */

	if (n2d_msg->msg_info.n2d_reg_comp.error == NCSCC_RC_SUCCESS) {
		/* the AVND has been successfully updated with the 
		 * component information.
		 */

		/* Trigger the node FSM handler for this event. */
		avd_nd_reg_comp_evt_hdl(cb, avnd);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/* log an error since this shouldn't happen */
	m_AVD_LOG_INVALID_VAL_FATAL(n2d_msg->msg_info.n2d_reg_comp.error);

	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;

	/* call the routine to failover all the effected nodes
	 * due to restarting this node
	 */

	avd_node_down_func(cb, avnd);
	return;
}

/*****************************************************************************
 * Function: avd_node_down_func
 *
 * Purpose:  This function is called to abruptly reset a node. It issues the
 * AVM HPI request to reset the node, stop the timers if any.
 *
 * Input: cb - the AVD control block
 *        avnd - The AVND pointer of the node that needs to be shutdown.
 *
 * Returns: None.
 *
 * NOTES: None. 
 **************************************************************************/

void avd_node_down_func(AVD_CL_CB *cb, AVD_AVND *avnd)
{

	m_AVD_LOG_FUNC_ENTRY("avd_node_down_func");

	/* clean up the heartbeat timer for this node. */
	avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));

	/* call HPI restart */
	if (avd_avm_send_reset_req(cb, &avnd->node_info.nodeName) == NCSCC_RC_SUCCESS) {

		/* the node is going down the operation state should be made disabled.
		 * make the node status as going down
		 */

		/* if we are in shutting down state, dont change the state;we have to
		 * send shutdown responce to avm
		 */
		if (avnd->node_state != AVD_AVND_STATE_SHUTTING_DOWN) {
			avd_node_state_set(avnd, AVD_AVND_STATE_GO_DOWN);
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
		}
	}
}

/*****************************************************************************
 * Function: avd_oper_req_func
 *
 * Purpose:  This function is the handler for operation requests response  
 * event indicating the arrival of the response message to the operation . 
 * request message. This function will log and ignore the operation based 
 * on the response in case of error. If success no action.
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_oper_req_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_DND_MSG *n2d_msg = evt->info.avnd_msg;
	AVD_AVND *avnd;

	TRACE_ENTER2("from %x", n2d_msg->msg_info.n2d_op_req.node_id);

	if ((avnd = avd_msg_sanity_chk(cb, evt, n2d_msg->msg_info.n2d_op_req.node_id, AVSV_N2D_OPERATION_REQUEST_MSG))
	    == NULL) {
		/* sanity failed return */
		goto done;
	}

	if ((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
	    (avnd->node_state == AVD_AVND_STATE_GO_DOWN) ||
	    ((avnd->rcv_msg_id + 1) != n2d_msg->msg_info.n2d_op_req.msg_id)) {
		/* log information error that the node is in invalid state */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->rcv_msg_id);
		goto done;
	}

	/* Update the receive id for the node */
	m_AVD_SET_AVND_RCV_ID(cb, avnd, (n2d_msg->msg_info.n2d_op_req.msg_id));

	/* 
	 * Send the Ack message to the node, indicationg that the message with this
	 * message ID is received successfully.
	 */
	if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
	}

	/* check the ack message for errors. If error stop and free all the
	 * messages in the list and issue HPI restart of the node
	 */

	if (n2d_msg->msg_info.n2d_op_req.error == NCSCC_RC_SUCCESS) {
		/* the AVND has been successfully updated with the operation
		 */
		goto done;
	}

	LOG_ER("Operation request FAILED, sender %x, '%s'",
		n2d_msg->msg_info.n2d_op_req.node_id, n2d_msg->msg_info.n2d_op_req.param_info.name.value);

 done:
	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;
	TRACE_LEAVE();
}

/**
 * handler to report error response to imm for any pending admin operation on su 
 *
 * @param su
 * @param pres
 */
static void su_admin_op_report_to_imm(AVD_SU *su, SaAmfPresenceStateT pres)
{
	AVD_CL_CB *cb = (AVD_CL_CB *)avd_cb;

	TRACE_ENTER2("(%u) (%u)", pres, su->pend_cbk.invocation);

	switch (su->pend_cbk.admin_oper) {
	case SA_AMF_ADMIN_LOCK_INSTANTIATION:
		if (pres == SA_AMF_PRESENCE_UNINSTANTIATED) {
			immutil_saImmOiAdminOperationResult(cb->immOiHandle, su->pend_cbk.invocation, SA_AIS_OK);
			su->pend_cbk.invocation = 0;
			su->pend_cbk.admin_oper = 0;
		} else if (pres == SA_AMF_PRESENCE_TERMINATION_FAILED) {
			immutil_saImmOiAdminOperationResult(cb->immOiHandle, su->pend_cbk.invocation,
							    SA_AIS_ERR_REPAIR_PENDING);
			su->pend_cbk.invocation = 0;
			su->pend_cbk.admin_oper = 0;
		}
		break;
	case SA_AMF_ADMIN_UNLOCK_INSTANTIATION:
		if (pres == SA_AMF_PRESENCE_INSTANTIATED) {
			immutil_saImmOiAdminOperationResult(cb->immOiHandle, su->pend_cbk.invocation, SA_AIS_OK);
			su->pend_cbk.invocation = 0;
			su->pend_cbk.admin_oper = 0;
		} else if ((pres == SA_AMF_PRESENCE_INSTANTIATION_FAILED) ||
			   (pres == SA_AMF_PRESENCE_TERMINATION_FAILED)) {
			immutil_saImmOiAdminOperationResult(cb->immOiHandle, su->pend_cbk.invocation,
							    SA_AIS_ERR_REPAIR_PENDING);
			su->pend_cbk.invocation = 0;
			su->pend_cbk.admin_oper = 0;
		}
		break;
	default:
		break;
	}

	TRACE_LEAVE2("(%u)", su->pend_cbk.invocation);
}

/**
 * handler to report error response to imm for any pending admin operation on node 
 * depending on the SU presence state updates received
 *
 * @param su
 * @param pres
 */
static void node_admin_op_report_to_imm(AVD_SU *su, SaAmfPresenceStateT pres)
{
	AVD_CL_CB *cb = (AVD_CL_CB *)avd_cb;

	TRACE_ENTER2("(%u) (%u)", pres, su->su_on_node->admin_node_pend_cbk.invocation);

	switch (su->su_on_node->admin_node_pend_cbk.admin_oper) {
	case SA_AMF_ADMIN_LOCK_INSTANTIATION:
		if (pres == SA_AMF_PRESENCE_UNINSTANTIATED) {
			su->su_on_node->su_cnt_admin_oper--;
			if (su->su_on_node->su_cnt_admin_oper == 0) {
				/* if this is the last SU then send out the pending callback */
				immutil_saImmOiAdminOperationResult(cb->immOiHandle,
								    su->su_on_node->admin_node_pend_cbk.invocation,
								    SA_AIS_OK);
				su->su_on_node->admin_node_pend_cbk.invocation = 0;
				su->su_on_node->admin_node_pend_cbk.admin_oper = 0;
			}
		} else if (pres == SA_AMF_PRESENCE_TERMINATION_FAILED) {
			immutil_saImmOiAdminOperationResult(cb->immOiHandle,
							    su->su_on_node->admin_node_pend_cbk.invocation,
							    SA_AIS_ERR_REPAIR_PENDING);
			su->su_on_node->admin_node_pend_cbk.invocation = 0;
			su->su_on_node->admin_node_pend_cbk.admin_oper = 0;
			su->su_on_node->su_cnt_admin_oper = 0;
		}		/* else do nothing ::SA_AMF_PRESENCE_TERMINATING update is valid */
		break;

	case SA_AMF_ADMIN_UNLOCK_INSTANTIATION:
		if (pres == SA_AMF_PRESENCE_INSTANTIATED) {
			su->su_on_node->su_cnt_admin_oper--;
			if (su->su_on_node->su_cnt_admin_oper == 0) {
				/* if this is the last SU then send out the pending callback */
				immutil_saImmOiAdminOperationResult(cb->immOiHandle,
								    su->su_on_node->admin_node_pend_cbk.invocation,
								    SA_AIS_OK);
				su->su_on_node->admin_node_pend_cbk.invocation = 0;
				su->su_on_node->admin_node_pend_cbk.admin_oper = 0;
			}
		} else if ((pres == SA_AMF_PRESENCE_TERMINATION_FAILED) ||
			   (pres == SA_AMF_PRESENCE_INSTANTIATION_FAILED)) {
			immutil_saImmOiAdminOperationResult(cb->immOiHandle,
							    su->su_on_node->admin_node_pend_cbk.invocation,
							    SA_AIS_ERR_REPAIR_PENDING);
			su->su_on_node->admin_node_pend_cbk.invocation = 0;
			su->su_on_node->admin_node_pend_cbk.admin_oper = 0;
			su->su_on_node->su_cnt_admin_oper = 0;
		}		/* else do nothing :: SA_AMF_PRESENCE_INSTANTIATING update is valid */
		break;

	default:
		/* present state updates may be received as part of other admin ops also */
		TRACE_LEAVE2("Present state update admin op %d", su->su_on_node->admin_node_pend_cbk.admin_oper);
		break;
	}

	TRACE_LEAVE2("(%u)(%u)", su->su_on_node->admin_node_pend_cbk.invocation, su->su_on_node->su_cnt_admin_oper);
}

/*****************************************************************************
 * Function: avd_data_update_req_func
 *
 * Purpose:  This function is the handler for update request message
 * event indicating the arrival of the data update request message.
 * This function will update the local structures with the information
 * provided by this message and send response to any admin callbacks from IMM
 * for which response was not sent.
 * It is used for supporting some of the objects whose values are
 * are managed by AvND.
 *
 * Input: cb - the AVD control block
 *        evt - The event information.
 *
 * Returns: None.
 *
 *
 * NOTES: The table id object id list handled in the routine from the AvND are
 *
 *     NCSMIB_TBL_AVSV_AMF_SU   : saAmfSUPresenceState_ID
 *     NCSMIB_TBL_AVSV_AMF_COMP : saAmfCompRestartCount_ID
 *                                saAmfCompOperState_ID
 *                                saAmfCompPresenceState_ID
 *
 * 
 **************************************************************************/

void avd_data_update_req_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_DND_MSG *n2d_msg = evt->info.avnd_msg;
	AVD_AVND *avnd;
	AVD_COMP *comp;
	AVD_SU *su;
	uns32 l_val = 0;

	TRACE_ENTER2("from %x", n2d_msg->msg_info.n2d_data_req.node_id);

	if ((avnd = avd_msg_sanity_chk(cb, evt, n2d_msg->msg_info.n2d_data_req.node_id, AVSV_N2D_DATA_REQUEST_MSG))
	    == NULL) {
		/* sanity failed return */
		goto done;
	}

	if ((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
	    (avnd->node_state == AVD_AVND_STATE_GO_DOWN) ||
	    ((avnd->rcv_msg_id + 1) != n2d_msg->msg_info.n2d_data_req.msg_id)) {
		/* log information error that the node is in invalid state */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->rcv_msg_id);
		m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_data_req.msg_id);
		goto done;
	}

	/* Update the receive message id. */
	m_AVD_SET_AVND_RCV_ID(cb, avnd, (n2d_msg->msg_info.n2d_data_req.msg_id));

	/* 
	 * Send the Ack message to the node, indicationg that the message with this
	 * message ID is received successfully.
	 */
	if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
	}

	/* Verify that operation is only modification. */
	if (n2d_msg->msg_info.n2d_data_req.param_info.act != AVSV_OBJ_OPR_MOD) {
		/* log error that a the table value is invalid */
		m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_data_req.param_info.act);
		goto done;
	}

	switch (n2d_msg->msg_info.n2d_data_req.param_info.class_id) {
	case AVSV_SA_AMF_COMP:{
			/* Find the component record in the database, specified in the message. */
			if ((comp = avd_comp_get(&n2d_msg->msg_info.n2d_data_req.param_info.name)) == NULL) {
				avd_log(NCSFL_SEV_ERROR, "Invalid Comp '%s' (%u)",
					n2d_msg->msg_info.n2d_data_req.param_info.name.value,
					n2d_msg->msg_info.n2d_data_req.param_info.name.length);
				goto done;
			}

			switch (n2d_msg->msg_info.n2d_data_req.param_info.attr_id) {
			case saAmfCompOperState_ID:
				TRACE("comp oper state");
				if (n2d_msg->msg_info.n2d_data_req.param_info.value_len == sizeof(uns32)) {
					SaAmfReadinessStateT saAmfCompReadinessState;

					l_val = *((uns32 *)&n2d_msg->msg_info.n2d_data_req.param_info.value[0]);
					avd_comp_oper_state_set(comp, ntohl(l_val));

					/* We need to update saAmfCompReadinessState */
					if ((comp->su->saAmfSuReadinessState == SA_AMF_READINESS_IN_SERVICE) &&
					    (comp->saAmfCompOperState == SA_AMF_OPERATIONAL_ENABLED)) {
						saAmfCompReadinessState = SA_AMF_READINESS_IN_SERVICE;
					} else if ((comp->su->saAmfSuReadinessState == SA_AMF_READINESS_STOPPING) &&
						   (comp->saAmfCompOperState == SA_AMF_OPERATIONAL_ENABLED)) {
						saAmfCompReadinessState = SA_AMF_READINESS_STOPPING;
					} else
						saAmfCompReadinessState = SA_AMF_READINESS_OUT_OF_SERVICE;

					avd_comp_readiness_state_set(comp, saAmfCompReadinessState);
				} else {
					/* log error that a the  value len is invalid */
					m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_data_req.param_info.
								    value_len);
				}
				break;
			case saAmfCompPresenceState_ID:
				TRACE("comp pres state");
				if (n2d_msg->msg_info.n2d_data_req.param_info.value_len == sizeof(uns32)) {
					l_val = *((uns32 *)&n2d_msg->msg_info.n2d_data_req.param_info.value[0]);
					avd_comp_pres_state_set(comp, ntohl(l_val));
				} else {
					/* log error that a the  value len is invalid */
					m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_data_req.param_info.
								    value_len);
				}
				break;
			case saAmfCompRestartCount_ID:
				TRACE("comp restart count");
				if (n2d_msg->msg_info.n2d_data_req.param_info.value_len == sizeof(uns32)) {
					l_val = *((uns32 *)&n2d_msg->msg_info.n2d_data_req.param_info.value[0]);
					comp->saAmfCompRestartCount = ntohl(l_val);
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVSV_CKPT_COMP_RESTART_COUNT);
					avd_saImmOiRtObjectUpdate(&comp->comp_info.name,
								  "saAmfCompRestartCount", SA_IMM_ATTR_SAUINT32T,
								  &comp->saAmfCompRestartCount);
				} else {
					/* log error that a the  value len is invalid */
					m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_data_req.param_info.
								    value_len);
				}
				break;
			case saAmfCompCurrProxyName_ID:
				TRACE("comp curr proxy name");
				if (n2d_msg->msg_info.n2d_data_req.param_info.value_len ==
				    strlen((char *)n2d_msg->msg_info.n2d_data_req.param_info.value)) {
					l_val = n2d_msg->msg_info.n2d_data_req.param_info.value_len;
					comp->saAmfCompCurrProxyName.length = l_val;

					strncpy((char *)comp->saAmfCompCurrProxyName.value,
						(char *)n2d_msg->msg_info.n2d_data_req.param_info.value,
						SA_MAX_NAME_LENGTH - 1);
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVSV_CKPT_COMP_CURR_PROXY_NAME);
					avd_saImmOiRtObjectUpdate(&comp->comp_info.name,
								  "saAmfCompCurrProxyName", SA_IMM_ATTR_SAUINT32T,
								  &comp->saAmfCompCurrProxyName);
				} else {
					/* log error that a the  value len is invalid */
					m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_data_req.param_info.
								    value_len);
				}
				break;
			default:
				/* log error that a the object value is invalid */
				m_AVD_LOG_INVALID_VAL_FATAL(n2d_msg->msg_info.n2d_data_req.param_info.attr_id);
				break;
			}	/* switch(n2d_msg->msg_info.n2d_data_req.param_info.obj_id) */

			break;
		}
	case AVSV_SA_AMF_SU:{
			/* Find the component record in the database, specified in the message. */
			if ((su = avd_su_get(&n2d_msg->msg_info.n2d_data_req.param_info.name)) == NULL) {
				avd_log(NCSFL_SEV_ERROR, "Invalid SU '%s' (%u)",
					n2d_msg->msg_info.n2d_data_req.param_info.name.value,
					n2d_msg->msg_info.n2d_data_req.param_info.name.length);
				goto done;
			}

			switch (n2d_msg->msg_info.n2d_data_req.param_info.attr_id) {
			case saAmfSUPresenceState_ID:
				TRACE("su pres state");
				if (n2d_msg->msg_info.n2d_data_req.param_info.value_len == sizeof(uns32)) {
					l_val = ntohl(*((uns32 *)&n2d_msg->msg_info.n2d_data_req.param_info.value[0]));
					avd_su_pres_state_set(su, l_val);

					/* Send response to any admin callbacks delivered by IMM if not sent already. */
					if (su->su_on_node->admin_node_pend_cbk.invocation != 0) {
						node_admin_op_report_to_imm(su, l_val);
					} else if (su->pend_cbk.invocation != 0) {
						su_admin_op_report_to_imm(su, l_val);
					}
				} else {
					/* log error that a the  value len is invalid */
					m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_data_req.param_info.
								    value_len);
				}
				break;
			default:
				/* log error that a the object value is invalid */
				m_AVD_LOG_INVALID_VAL_FATAL(n2d_msg->msg_info.n2d_data_req.param_info.attr_id);
				break;
			}	/* switch(n2d_msg->msg_info.n2d_data_req.param_info.obj_id) */

			break;	/* case NCSMIB_TBL_AVSV_AMF_SU */
		}
	default:
		/* log error that a the table value is invalid */
		m_AVD_LOG_INVALID_VAL_FATAL(n2d_msg->msg_info.n2d_data_req.param_info.class_id);
		goto done;
		break;
	}			/* switch(n2d_msg->msg_info.n2d_data_req.param_info.table_id) */

 done:
	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;
	TRACE_LEAVE();
}

/*****************************************************************************
 * Function: avd_comp_validation_func
 *
 * Purpose:  This function responds to componenet validation query from AvND. 
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

void avd_comp_validation_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_COMP *comp_ptr = NULL;
	AVD_DND_MSG *n2d_msg = NULL;
	uns32 res = NCSCC_RC_SUCCESS;
	AVD_AVND *avnd = NULL;
	AVSV_N2D_COMP_VALIDATION_INFO *valid_info = NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_comp_validation_func");

	if (evt->info.avnd_msg == NULL) {
		/* log error that a message contents is missing */
		m_AVD_LOG_INVALID_VAL_ERROR(0);
		return;
	}

	n2d_msg = evt->info.avnd_msg;

	m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG, n2d_msg, sizeof(AVD_DND_MSG), n2d_msg);

	valid_info = &n2d_msg->msg_info.n2d_comp_valid_info;

	if ((avnd = avd_msg_sanity_chk(cb, evt, valid_info->node_id, AVSV_N2D_COMP_VALIDATION_MSG))
	    == NULL) {
		/* sanity failed return */
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	if ((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
	    (avnd->node_state == AVD_AVND_STATE_GO_DOWN) || ((avnd->rcv_msg_id + 1) != valid_info->msg_id)) {
		/* log information error that the node is in invalid state */
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->rcv_msg_id);
		avsv_dnd_msg_free(n2d_msg);
		evt->info.avnd_msg = NULL;
		return;
	}

	/* Update the receive id for the node */
	m_AVD_SET_AVND_RCV_ID(cb, avnd, (valid_info->msg_id));

	comp_ptr = avd_comp_get(&valid_info->comp_name);

	if (NULL != comp_ptr) {
		/* We found the component, reply to AvND. */
		res = avd_snd_comp_validation_resp(cb, avnd, comp_ptr, n2d_msg);

	} else {
		/* We couldn't find the component, this is not a configured component. 
		   Reply to AvND. */
		res = avd_snd_comp_validation_resp(cb, avnd, NULL, n2d_msg);
	}

	if (NCSCC_RC_FAILURE == res) {
		m_AVD_PXY_PXD_ERR_LOG("avd_comp_validation_func:failure:Comp,MsgId,NodeId,hdl and mds_dest are",
				      &valid_info->comp_name, valid_info->msg_id, valid_info->node_id,
				      valid_info->hdl, valid_info->mds_dest);
	}

	avsv_dnd_msg_free(n2d_msg);
	evt->info.avnd_msg = NULL;
	return;

}
