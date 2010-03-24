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

  DESCRIPTION:This module deals with the switch operations requested by
  Administator.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <immutil.h>
#include <logtrace.h>

#include <avd.h>
#include <avd_imm.h>
#include <avd_app.h>
#include <avd_cluster.h>
#include <avd_clm.h>
#include <rda_papi.h>

static uns32 avd_role_switch_qsd_actv(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_switch_qsd_stdby(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_failover(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_failover_qsd_actv(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_rde_set_role(SaAmfHAStateT role);

/****************************************************************************\
 * Function: avd_role_change
 *
 * Purpose:  AVSV function to handle AVD role change event received.
 *
 * Input: cb - AVD control block pointer.
 *
 * Returns: None.
 *
 * NOTES: 
 *
 * 
\**************************************************************************/
void avd_role_change(AVD_CL_CB *cb, AVD_EVT *evt)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_D2D_MSG *msg = evt->info.avd_msg;
	AVD_ROLE_CHG_CAUSE_T cause = msg->msg_info.d2d_chg_role_req.cause;
	SaAmfHAStateT role = msg->msg_info.d2d_chg_role_req.role;

	TRACE_ENTER2("cause=%u, role=%u", cause, role);

	if (cb->avail_state_avd == role) {
		goto done;
	}

	/* 
	 * First validate that this is correct role switch.
	 */
	if (((cb->avail_state_avd == SA_AMF_HA_ACTIVE) &&
	     (role == SA_AMF_HA_STANDBY)) ||
	    ((cb->avail_state_avd == SA_AMF_HA_STANDBY) && (role == SA_AMF_HA_QUIESCED))) {
		LOG_ER("avd_role_change Invalid role change Active -> Standby");
		status = NCSCC_RC_FAILURE;
		goto done;
	}

	/* Check what is the cause of this role set */
	if ((cause == AVD_INIT_ROLE) &&
	    (cb->role_set == FALSE) && ((role == SA_AMF_HA_ACTIVE) || (role == SA_AMF_HA_STANDBY))) {
		/* Set Initial role of AVD */
		status = avd_init_role_set(cb, role);
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (cause == AVD_SWITCH_OVER) &&
	    (cb->avail_state_avd == SA_AMF_HA_ACTIVE) && (role == SA_AMF_HA_QUIESCED)) {
		if (SA_TRUE == cb->swap_switch ) {
			/* swap resulted Switch Active -> Quiesced */
			m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_ATOQ, NCSFL_SEV_NOTICE, 0);
			amfd_switch_actv_qsd(cb);
			status = NCSCC_RC_SUCCESS;
		} 
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (cause == AVD_SWITCH_OVER) &&
	    (role == SA_AMF_HA_ACTIVE) && (cb->avail_state_avd == SA_AMF_HA_QUIESCED)) {
		if (SA_TRUE == cb->swap_switch ) {
			/* swap resulted Switch  Quiesced -> Active */
			m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_QTOA, NCSFL_SEV_NOTICE, 0);
			amfd_switch_qsd_actv(cb);
			status = NCSCC_RC_SUCCESS;
		}
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (cause == AVD_SWITCH_OVER) &&
	    (role == SA_AMF_HA_STANDBY) && (cb->avail_state_avd == SA_AMF_HA_QUIESCED)) {
		if (SA_TRUE == cb->swap_switch ) {
			/* swap resulted Switch  Quiesced -> standby */
			m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_QTOS, NCSFL_SEV_NOTICE, 0);
			amfd_switch_qsd_stdby(cb);
			status = NCSCC_RC_SUCCESS;
		}
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (cause == AVD_SWITCH_OVER) &&
	    (role == SA_AMF_HA_ACTIVE) && (cb->avail_state_avd == SA_AMF_HA_STANDBY)) {
		if (SA_TRUE == cb->swap_switch ) {
			/* swap resulted Switch  standby -> Active */
			m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_STOA, NCSFL_SEV_NOTICE, 0);
			amfd_switch_stdby_actv(cb);
			status = NCSCC_RC_SUCCESS;
			goto done;
		}
	}

	if ((cb->role_set == TRUE) &&
	    (cause == AVD_FAIL_OVER) &&
	    (cb->avail_state_avd == SA_AMF_HA_QUIESCED) && (role == SA_AMF_HA_ACTIVE)) {
		/* Fail-over Quiesced to Active */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_QTOA, NCSFL_SEV_NOTICE, 0);
		status = avd_role_failover_qsd_actv(cb, role);
		goto done;

	}

	if ((cb->role_set == TRUE) &&
	    (cause == AVD_FAIL_OVER) &&
	    (cb->avail_state_avd == SA_AMF_HA_STANDBY) && (role == SA_AMF_HA_ACTIVE)) {
		/* Fail-over Standby to Active */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_STOA, NCSFL_SEV_NOTICE, 0);
		status = avd_role_failover(cb, role);

	} else {
		LOG_ER("avd_role_change Invalid role change");
		status = NCSCC_RC_FAILURE;
	}

 done:
	if (NCSCC_RC_SUCCESS != status) {
		LOG_ER("avd_role_change role change failure");
	}

	if (msg)
		avsv_d2d_msg_free(msg);
	TRACE_LEAVE2("%u", status);
	return;
}

static const char *role_to_str(SaAmfHAStateT role)
{
	switch (role) {
	case SA_AMF_HA_ACTIVE:
		return "ACTIVE";
		break;
	case SA_AMF_HA_STANDBY:
		return "STANDBY";
		break;
	default:
		return "unknown";
		break;
	}
}

/****************************************************************************\
 * Function: avd_init_role_set
 *
 * Purpose:  AVSV function to handle AVD's initial role setting. 
 *
 * Input: cb - AVD control block pointer.
 *        role - Role to be set.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
\**************************************************************************/
uns32 avd_init_role_set(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	uns32 status = NCSCC_RC_FAILURE;

	LOG_NO("I am %s", role_to_str(role));

	/*
	 * Set mds VDEST initial role. We need not send the role to AvND
	 * as when AvND will come up, we will send.
	 */
	if (NCSCC_RC_SUCCESS != (status = avd_mds_set_vdest_role(cb, role))) {
		/* Log error */
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_VDEST_ROL);
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		goto done;
	}

	/*
	 * Set mbcsv initial role.
	 */
	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, role))) {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		goto done;
	}

	cb->avail_state_avd = role;
	cb->role_set = TRUE;

	if (role == SA_AMF_HA_ACTIVE) {
		/* start the cluster track now */
		if(avd_clm_track_start() != SA_AIS_OK) {
			LOG_ER("avd_clm_track_start FAILED");
			goto done;
		}
	}

	if ((role == SA_AMF_HA_ACTIVE) &&
	    ((status = avd_imm_config_get()) == NCSCC_RC_SUCCESS) &&
	    (avd_imm_impl_set() == SA_AIS_OK)) {
		cb->init_state = AVD_CFG_DONE;
		{
			SaNameT su_name;
                        AVD_SU  *avd_su = NULL;
			memset(&su_name, '\0', sizeof(SaNameT));
			/* Set runtime cached attributes. */
			/* saAmfSUPreInstantiable and saAmfSUHostedByNode. */
			avd_su = avd_su_getnext(&su_name);
			while (avd_su != NULL) {
				avd_saImmOiRtObjectUpdate(&avd_su->name,
						"saAmfSUPreInstantiable", SA_IMM_ATTR_SAUINT32T, 
						&avd_su->saAmfSUPreInstantiable);

				avd_saImmOiRtObjectUpdate(&avd_su->name,
						"saAmfSUHostedByNode", SA_IMM_ATTR_SANAMET, 
						&avd_su->saAmfSUHostedByNode);

				avd_saImmOiRtObjectUpdate(&avd_su->name,
						"saAmfSUPresenceState", SA_IMM_ATTR_SAUINT32T, 
						&avd_su->saAmfSUPresenceState);

				avd_saImmOiRtObjectUpdate(&avd_su->name,
						"saAmfSUOperState", SA_IMM_ATTR_SAUINT32T, 
						&avd_su->saAmfSUOperState);

				avd_saImmOiRtObjectUpdate(&avd_su->name,
						"saAmfSUReadinessState", SA_IMM_ATTR_SAUINT32T, 
						&avd_su->saAmfSuReadinessState);

				avd_su = avd_su_getnext(&avd_su->name);
			}
		}

		{
			SaNameT comp_name;
                        AVD_COMP  *avd_comp = NULL;
			memset(&comp_name, '\0', sizeof(SaNameT));
			/* Set Component Class runtime cached attributes. */
			avd_comp = avd_comp_getnext(&comp_name);
			while (avd_comp != NULL) {
				avd_saImmOiRtObjectUpdate(&avd_comp->comp_info.name,
						"saAmfCompReadinessState", SA_IMM_ATTR_SAUINT32T, &avd_comp->saAmfCompReadinessState);

				avd_saImmOiRtObjectUpdate(&avd_comp->comp_info.name,
						"saAmfCompOperState", SA_IMM_ATTR_SAUINT32T, &avd_comp->saAmfCompOperState);

				avd_saImmOiRtObjectUpdate(&avd_comp->comp_info.name,
						"saAmfCompPresenceState", SA_IMM_ATTR_SAUINT32T, &avd_comp->saAmfCompPresenceState);

				avd_comp = avd_comp_getnext(&avd_comp->comp_info.name);
			}

		}

		{
			SaNameT node_name;
                        AVD_AVND  *avd_avnd = NULL;
			memset(&node_name, '\0', sizeof(SaNameT));
			/* Set Node Class runtime cached attributes. */
			avd_avnd = avd_node_getnext(&node_name);
			while (avd_avnd != NULL) {
				avd_saImmOiRtObjectUpdate(&avd_avnd->name, "saAmfNodeOperState",
						SA_IMM_ATTR_SAUINT32T, &avd_avnd->saAmfNodeOperState);

				avd_avnd = avd_node_getnext(&avd_avnd->name);
			}
		}

		{
			SaNameT si_name;
                        AVD_SI  *avd_si = NULL;
			memset(&si_name, '\0', sizeof(SaNameT));
			/* Set Node Class runtime cached attributes. */
			avd_si = avd_si_getnext(&si_name);
			while (avd_si != NULL) {
				avd_saImmOiRtObjectUpdate(&avd_si->name, "saAmfSIAssignmentState",
						SA_IMM_ATTR_SAUINT32T, &avd_si->saAmfSIAssignmentState);

				avd_si = avd_si_getnext(&avd_si->name);
			}
		}

	}

done:
	return status;
}

/****************************************************************************\
 * Function: avd_role_failover
 *
 * Purpose:  AVSV function to handle AVD's fail-over. 
 *
 * Input: cb - AVD control block pointer.
 *        role - Role to be set.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
\**************************************************************************/
static uns32 avd_role_failover(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_AVND *avnd = NULL;

	TRACE_ENTER();
	LOG_NO("FAILOVER StandBy --> Active");

	/* If we are in the middle of admin switch, ignore it */
	if (cb->swap_switch == SA_TRUE) {
		cb->swap_switch = SA_FALSE;
	}

	/*
	 * Check whether Standby is in sync with Active. If yes then
	 * proceed further. Else return failure.
	 */
	if (AVD_STBY_OUT_OF_SYNC == cb->stby_sync_state) {
		LOG_ER("FAILOVER StandBy --> Active FAILED, Stanby OUT OF SYNC");
		return NCSCC_RC_FAILURE;
	}

	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		LOG_ER("FAILOVER StandBy --> Active FAILED, DB not found");
		return NCSCC_RC_FAILURE;
	}

	/* check the node state */
	if (avnd->node_state != AVD_AVND_STATE_PRESENT) {
		LOG_ER("FAILOVER StandBy --> Active FAILED, stdby not in good state");
		return NCSCC_RC_FAILURE;
	}

	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_ACTIVE))) {
		/* log error that the node id is invalid */
		LOG_ER("FAILOVER StandBy --> Active, ckpt set role failed");
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		LOG_ER("FAILOVER StandBy --> Active FAILED, MBCSV DISPATCH FAILED");
		return NCSCC_RC_FAILURE;
	}

	/*
	 * We might be having some async update messages in the
	 * Queue to be processed, now drop all of them.
	 */
	avsv_dequeue_async_update_msgs(cb, FALSE);

	cb->avail_state_avd = role;

	/* Declare this standby as Active. Set Vdest role and MBCSv role */
	if (NCSCC_RC_SUCCESS != (status = avd_mds_set_vdest_role(cb, role))) {
		LOG_ER("FAILOVER StandBy --> Active, VDEST Change role failed ");
	}

	/* Time to send fail-over messages to all the AVND's */
	avd_fail_over_event(cb);

	/* We need to send the role to AvND. */
	status = avd_avnd_send_role_change(cb, cb->node_id_avd, cb->avail_state_avd);
	if (NCSCC_RC_SUCCESS != status) {
		m_AVD_PXY_PXD_ERR_LOG("avd_role_failover: role sent failed. Node Id and role are",
				      NULL, cb->node_id_avd, cb->avail_state_avd, 0, 0);
	} else {
		avd_d2n_msg_dequeue(cb);
	}

	/* We are successfully changed role to Active. Now reset 
	 * the old Active. TODO */

	cb->node_id_avd_other = 0;

	if (avd_imm_config_get() != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	if (avd_imm_impl_set() != SA_AIS_OK)
		return NCSCC_RC_FAILURE;

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avd_role_failover_qsd_actv
 *
 * Purpose:  AVSV function to handle AVD's fail-over. 
 *
 * Input: cb - AVD control block pointer.
 *        role - Role to be set.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
\**************************************************************************/
static uns32 avd_role_failover_qsd_actv(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVD_AVND *avnd = NULL;
	AVD_AVND *avnd_other = NULL;
	AVD_EVT *evt = AVD_EVT_NULL;
	NCSMDS_INFO svc_to_mds_info;

	TRACE_ENTER();
	LOG_NO("FAILOVER Quiesced --> Active");

	/* If we are in the middle of admin switch, ignore it */
	if (cb->swap_switch == SA_TRUE) {
		cb->swap_switch = SA_FALSE;
	}

	/*
	 * Check whether Standby is in sync with Active. If yes then
	 * proceed further. Else return failure.
	 */
	if (AVD_STBY_OUT_OF_SYNC == cb->stby_sync_state) {
		LOG_ER("FAILOVER Quiesced --> Active FAILED, Stanby OUT OF SYNC");
		return NCSCC_RC_FAILURE;
	}

	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		LOG_ER("FAILOVER Quiesced --> Active FAILED, DB not found");
		return NCSCC_RC_FAILURE;
	}

	/* check the node state */
	if (avnd->node_state != AVD_AVND_STATE_PRESENT) {
		LOG_ER("FAILOVER Quiesced --> Active FAILED, stdby not in good state");
		return NCSCC_RC_FAILURE;
	}

	/* Section to check whether AvD was doing a  role change before getting into failover */
	svc_to_mds_info.i_mds_hdl = cb->vaddr_pwe_hdl;
	svc_to_mds_info.i_svc_id = NCSMDS_SVC_ID_AVD;
	svc_to_mds_info.i_op = MDS_QUERY_DEST;
	svc_to_mds_info.info.query_dest.i_dest = cb->vaddr;
	svc_to_mds_info.info.query_dest.i_svc_id = NCSMDS_SVC_ID_AVD;
	svc_to_mds_info.info.query_dest.i_query_for_role = TRUE;
	svc_to_mds_info.info.query_dest.info.query_for_role.i_anc = cb->avm_mds_dest;

	if (ncsmds_api(&svc_to_mds_info) == NCSCC_RC_SUCCESS) {
		if (svc_to_mds_info.info.query_dest.info.query_for_role.o_vdest_rl == SA_AMF_HA_ACTIVE) {
			/* We were in middle of switch, but we had not progresses much with role switch functionality.
			 * its ok to just change the NCS SU's who are already quiesced, back to Active. 
			 * Post an evt on mailbox to set active role to all NCS SU 
			 * 
			 */
			AVD_EVT evt;
			memset(&evt, '\0', sizeof(AVD_EVT));
			evt.rcv_evt = AVD_EVT_SWITCH_NCS_SU;

			/* set cb state to active */
			cb->avail_state_avd = role;
			avd_role_switch_ncs_su(cb, &evt);

			if (NULL != (avnd_other = avd_node_find_nodeid(cb->node_id_avd_other))) {
				/* We are successfully changed role to Active.
				   do node down processing for other node */
				avd_avm_mark_nd_absent(cb, avnd_other);
			} else {
				m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
			}

			return NCSCC_RC_SUCCESS;
			/* END OF THIS FLOW */
		}
	}

	/* We are not in middle of role switch functionality, carry on with normal failover flow */
	avsv_set_ckpt_role(cb, SA_AMF_HA_ACTIVE);

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		LOG_ER("NCS_AvSv: FAILOVER Quiesced --> Active FAILED, MBCSV DISPATCH FAILED");
		return NCSCC_RC_FAILURE;
	}

	/*
	 * We might be having some async update messages in the
	 * Queue to be processed, now drop all of them.
	 */
	avsv_dequeue_async_update_msgs(cb, FALSE);

	cb->avail_state_avd = role;

	/* Declare this standby as Active. Set Vdest role and MBCSv role */
	if (NCSCC_RC_SUCCESS != (status = avd_mds_set_vdest_role(cb, role))) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_VDEST_ROL);
		m_AVD_LOG_INVALID_VAL_ERROR(role);
		m_AVD_LOG_INVALID_VAL_ERROR(status);
	}

	/* Time to send fail-over messages to all the AVND's */
	avd_fail_over_event(cb);

	/* We need to send the role to AvND. */
	status = avd_avnd_send_role_change(cb, cb->node_id_avd, cb->avail_state_avd);
	if (NCSCC_RC_SUCCESS != status) {
		m_AVD_PXY_PXD_ERR_LOG("avd_role_failover_qsd_actv: role sent failed. Node Id and role are",
				      NULL, cb->node_id_avd, cb->avail_state_avd, 0, 0);
	} else {
		avd_d2n_msg_dequeue(cb);
	}

	/* Post an evt on mailbox to set active role to all NCS SU */
	/* create the message event */
	evt = calloc(1, sizeof(AVD_EVT));
	if (evt == AVD_EVT_NULL) {
		/* log error */
		LOG_ER("FAILOVER Quiesced --> Active FAILED, MEMALLOC FAILED");
		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_RCVD_VAL(((long)evt));

	evt->rcv_evt = AVD_EVT_SWITCH_NCS_SU;

	m_AVD_LOG_EVT_INFO(AVD_SND_AVND_MSG_EVENT, evt->rcv_evt);

	if (m_NCS_IPC_SEND(&cb->avd_mbx, evt, NCS_IPC_PRIORITY_HIGH)
	    != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
		/* log error */
		/* free the event and return */
		free(evt);
		LOG_ER("FAILOVER Quiesced --> Active FAILED, IPC SEND FAILED");

		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);

	/* We are successfully changed role to Active. Gen a reset 
	 * responce for the other card. TODO */

	cb->node_id_avd_other = 0;

	if (avd_imm_config_get() != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	avd_imm_impl_set_task_create();

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avd_role_switch_ncs_su 
 *
 * Purpose:  AVSV function to handle the situation where the AvD has succesfully transitioned
 *           to the active role. This function will make all the NCS SUs as active.
 * 
 * Input: cb - AVD control block pointer.
 *        role - Role to be set.
 *
 * Returns: None.
 *
 * NOTES:   None.
 *
 * 
\**************************************************************************/
void avd_role_switch_ncs_su(AVD_CL_CB *cb, AVD_EVT *evt)
{
	AVD_AVND *avnd = NULL, *other_avnd = NULL;
	AVD_SU *i_su = NULL;
	AVSV_N2D_INFO_SU_SI_ASSIGN_MSG_INFO assign;

	TRACE_ENTER();

	/* get the avnd from node_id */
	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd);
		return;
	}
	other_avnd = avd_node_find_nodeid(cb->node_id_avd_other);

	/* if we are not having any NCS SU's just jump to next level */
	if ((avnd->list_of_ncs_su == NULL) && (NULL != other_avnd)) {
		memset(&assign, 0, sizeof(AVSV_N2D_INFO_SU_SI_ASSIGN_MSG_INFO));
		assign.ha_state = SA_AMF_HA_ACTIVE;
		assign.error = NCSCC_RC_SUCCESS;
		avd_ncs_su_mod_rsp(cb, avnd, &assign);
		return;
	}

	for (i_su = avnd->list_of_ncs_su; i_su != NULL; i_su = i_su->avnd_list_su_next) {
		if ((i_su->list_of_susi != 0) &&
		    (i_su->sg_of_su->sg_redundancy_model == SA_AMF_2N_REDUNDANCY_MODEL) &&
		    (i_su->list_of_susi->state != SA_AMF_HA_ACTIVE)) {
			if (NULL != other_avnd ) {
				avd_sg_su_si_mod_snd(cb, i_su, SA_AMF_HA_ACTIVE);
			} else {
				avd_sg_su_oper_list_add(cb, i_su, FALSE);
				m_AVD_SET_SU_SWITCH(cb, i_su, AVSV_SI_TOGGLE_SWITCH);
				m_AVD_SET_SG_FSM(cb, (i_su->sg_of_su), AVD_SG_FSM_SU_OPER);
				m_AVD_LOG_RCVD_VAL(i_su->sg_of_su->sg_fsm_state);
			}
		}
	}
	
	TRACE_LEAVE();

	return;
}

/****************************************************************************\
 * Function: avd_role_switch_qsd_actv
 *
 * Purpose:  AVSV function to handle AVD's role switch from Quiesced to 
 *           Active.  
 *
 * Input: cb - AVD control block pointer.
 *        role - Role to be set.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
\**************************************************************************/
static uns32 avd_role_switch_qsd_actv(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	LOG_NO("AMFD ROLE SWITCH Quiesced --> Active");
	return amfd_switch_stdby_actv(cb);
}


/*****************************************************************************
 * Function: avd_mds_qsd_role_func
 *
 * Purpose:  This function is the handler for the MDS event indicating that the
 * the role  of the AvD is now quiesced.
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

void avd_mds_qsd_role_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
	uns32 status = NCSCC_RC_SUCCESS;
	uns32 rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER();
	/* Now set the MBCSv role to quiesced, */
	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_QUIESCED))) {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(status);
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (rc = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DISPATCH_FAILURE, NCSFL_SEV_NOTICE, rc);
		cb->swap_switch = SA_FALSE;
		return;
	}

	/* Send the message to the other AVD to go active */
	if (NCSCC_RC_SUCCESS != (rc= avd_d2d_chg_role_req(cb, AVD_SWITCH_OVER ,SA_AMF_HA_ACTIVE))) {
		/* Log error */
		LOG_ER("Unable to send the AMFD change role request from Standby to Active");
		cb->swap_switch = SA_FALSE;
		amfd_switch_qsd_actv(cb);
	}

	TRACE_LEAVE();
	return;
}

/*****************************************************************************
 * Function: admin_switch
 *
 * Purpose: 
 *
 * Input: cb - the AVD control block
 *
 * Returns: 
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void amfd_switch(AVD_CL_CB *cb)
{
	AVD_AVND *avnd = NULL;  
	AVD_SU *i_su = NULL;

	TRACE_ENTER();

	/* First check if there are any other SI's that are any other active */
	/* get the avnd from node_id */
	avnd = avd_node_find_nodeid (cb->node_id_avd);

	for (i_su = avnd->list_of_ncs_su; i_su != NULL; i_su = i_su->avnd_list_su_next) {
		if ((i_su->list_of_susi != 0) &&
			(i_su->sg_of_su->sg_redundancy_model == SA_AMF_2N_REDUNDANCY_MODEL) &&
				(i_su->list_of_susi->state == SA_AMF_HA_ACTIVE)) {
				/* Still some SI on the local SU are ACTIVE, Dont switch the AMFD */
				return;
			}
	}

	cb->swap_switch = SA_TRUE;
	/* Post a evt to the AVD mailbox for the admin switch role change */
	if (NCSCC_RC_SUCCESS != avd_post_amfd_switch_role_change_evt(cb, SA_AMF_HA_QUIESCED)) {
		LOG_ER("unable to post to mailbox for amfd switch");
		cb->swap_switch= SA_FALSE;
		return ;
	}
	TRACE_LEAVE();
}

/*****************************************************************************
 * Function: avd_post_amfd_switch_role_change_evt
 *
 * Purpose: 
 *
 * Input: cb - the AVD control block
 *
 * Returns: 
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_post_amfd_switch_role_change_evt(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	AVD_EVT *evt;

	evt = malloc(sizeof(AVD_EVT));
	assert(evt);
	evt->rcv_evt = AVD_EVT_ROLE_CHANGE;
	evt->info.avd_msg = malloc(sizeof(AVD_D2D_MSG));
	evt->info.avd_msg->msg_type = AVD_D2D_CHANGE_ROLE_REQ;
	evt->info.avd_msg->msg_info.d2d_chg_role_req.cause = AVD_SWITCH_OVER;
	evt->info.avd_msg->msg_info.d2d_chg_role_req.role =  role; 

	rc = ncs_ipc_send(&avd_cb->avd_mbx, (NCS_IPC_MSG *)evt, MDS_SEND_PRIORITY_HIGH);
	assert(rc == NCSCC_RC_SUCCESS);
	return rc;
}

/*****************************************************************************
 * Function: avd_d2d_chg_role_req
 *
 * Purpose: 
 *
 * Input: cb - the AVD control block
 *
 * Returns: 
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_d2d_chg_role_req(AVD_CL_CB *cb, AVD_ROLE_CHG_CAUSE_T cause, SaAmfHAStateT role)
{
	AVD_D2D_MSG d2d_msg;
	TRACE_ENTER();

	if((cb->node_id_avd_other == 0) || (cb->other_avd_adest == 0)) {
		return NCSCC_RC_FAILURE;
	}
 
	/* prepare the message. */

	d2d_msg.msg_type = AVD_D2D_CHANGE_ROLE_REQ;
	d2d_msg.msg_info.d2d_chg_role_req.role = role;
	d2d_msg.msg_info.d2d_chg_role_req.cause = cause;

	/* send the message */
	if (avd_d2d_msg_snd(cb,  &d2d_msg) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		LOG_ER("MDS Send failed for AMFD Role Change REQ");
		return NCSCC_RC_FAILURE;
	}

	TRACE_LEAVE();
  
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_d2d_chg_role_rsp
 *
 * Purpose: 
 *
 * Input: cb - the AVD control block
 *
 * Returns: 
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_d2d_chg_role_rsp(AVD_CL_CB *cb, uns32 status, SaAmfHAStateT role)
{
	AVD_D2D_MSG d2d_msg;

	TRACE_ENTER();
	LOG_NO("amfd sending response to change role req status=%d role=%d",status,role);


	if((cb->node_id_avd_other == 0) || (cb->other_avd_adest == 0)) {
		return NCSCC_RC_FAILURE;
	}
 
	/* prepare the message. */
	d2d_msg.msg_type = AVD_D2D_CHANGE_ROLE_RSP;
	d2d_msg.msg_info.d2d_chg_role_rsp.role = role;
	d2d_msg.msg_info.d2d_chg_role_rsp.status = status;

	/* send the message */
	if (avd_d2d_msg_snd(cb,  &d2d_msg) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		LOG_ER("MDS Send failed for AFMD Role Change RSP");
		return NCSCC_RC_FAILURE;
	}
  
	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}


/****************************************************************************\
 * Function: amfd_switch_actv_qsd
 *
 * Purpose:  function to handle AVD's role 
 *           change from Active to Quiesced in liue of SI SWAP Function
 * 
 * Input: cb - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/

uns32 amfd_switch_actv_qsd(AVD_CL_CB *cb)
{
	AVD_AVND *avnd = NULL;
	AVD_AVND *avnd_other = NULL;
	SaClmNodeIdT node_id = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER();
	LOG_NO("AMFD ROLE SWITCH Active --> Quiesced");

	if (cb->init_state != AVD_APP_STATE) {
		LOG_ER("AMFD ROLE SWITCH Active --> Quiesced, AMFD not in APP State");
		cb->swap_switch = SA_FALSE;
		return NCSCC_RC_FAILURE;
	}

	/* get the avnd from node_id */
	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		LOG_ER("AMFD ROLE SWITCH Active --> Quiesced, Local Node not found");
		cb->swap_switch = SA_FALSE;
		return NCSCC_RC_FAILURE;
	}

	/* get the avnd from node_id */
	if (NULL == (avnd_other = avd_node_find_nodeid(cb->node_id_avd_other))) {
		LOG_ER("AMFD ROLE SWITCH Active --> Quiesced, remote Node not found");
		cb->swap_switch = SA_FALSE;
		return NCSCC_RC_FAILURE;
	}

	/* check if the other AVD is up and has standby role. verify that
	 * the AvND on the node is in present state. 
	 */
	if ((cb->node_id_avd_other == 0) ||
	    (cb->avail_state_avd_other != SA_AMF_HA_STANDBY) ||
	    (avnd->node_state != AVD_AVND_STATE_PRESENT) || (avnd_other->node_state != AVD_AVND_STATE_PRESENT)) {
		cb->swap_switch = SA_FALSE;
		return NCSCC_RC_FAILURE;
	}

	/*  Mark AVD as Quiesced. */
	cb->avail_state_avd = SA_AMF_HA_QUIESCED;
	
	if (avd_clm_track_stop() != SA_AIS_OK) {
		LOG_ER("ClmTrack stop failed");
	}

	/* Go ahead and set mds role as already the NCS SU has been switched */
	if (NCSCC_RC_SUCCESS != (rc = avd_mds_set_vdest_role(cb, SA_AMF_HA_QUIESCED))) {
		LOG_ER("AMFD ROLE SWITCH Active --> Quiesced, vdest role set failed");
		cb->swap_switch = SA_FALSE;
		return NCSCC_RC_FAILURE;
	}

	if (NCSCC_RC_SUCCESS != avd_rde_set_role(SA_AMF_HA_QUIESCED)) {
		LOG_ER("rde role change failed from actv -> qsd");
	}

	/* We need to send the role to AvND. */
	rc = avd_avnd_send_role_change(cb, cb->node_id_avd, cb->avail_state_avd);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVD_PXY_PXD_ERR_LOG("amfd_switch_actv_qsd: role sent failed. Node Id and role are",
			      NULL, cb->node_id_avd, cb->avail_state_avd, 0, 0);
	} else {
		avd_d2n_msg_dequeue(cb);
	}

	/* Give up our IMM OI implementer role */
	(void)immutil_saImmOiImplementerClear(cb->immOiHandle);
	cb->is_implementer = FALSE;

	/* Walk through all the nodes and stop AvND rcv HeartBeat. */
	while (NULL != (avnd = avd_node_getnext_nodeid(node_id))) {
		node_id = avnd->node_info.nodeId;
	}
	TRACE_LEAVE();

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: amfd_switch_qsd_stdby
 *
 * Purpose:  function to handle AVD's role switch from Quiesced to 
 *           Standby.  
 *
 * Input: cb - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/

uns32 amfd_switch_qsd_stdby(AVD_CL_CB *cb)
{
	AVD_AVND *avnd = NULL;
	uns32 node_id = 0;
	uns32 status = NCSCC_RC_SUCCESS;

	TRACE_ENTER();
	LOG_NO("AMFD Switching Quiesced --> StandBy");

	cb->swap_switch = SA_FALSE;

	if (NCSCC_RC_SUCCESS != (status = avd_mds_set_vdest_role(cb, SA_AMF_HA_STANDBY))) {
		LOG_ER("AMFD Switch Quiesced --> StandBy, Vdest set role failed");
		return NCSCC_RC_FAILURE;
	}

	/* Change MBCSv role to Standby */
	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_STANDBY))) {
		LOG_ER("AMFD Switch Quiesced --> StandBy, MBCSv set role failed");
		return NCSCC_RC_FAILURE;
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		LOG_ER("AMFD Switch Quiesced --> StandBy, MBCSv dispatch failed");
		return NCSCC_RC_FAILURE;
	}

	if (NCSCC_RC_SUCCESS != avd_rde_set_role(SA_AMF_HA_STANDBY)) {
		LOG_ER("rde role change failed from qsd -> stdby");
	}

	node_id = 0;

	/* Walk through all the nodes and  free PG records. */
	while (NULL != (avnd = avd_node_getnext_nodeid(node_id))) {
		node_id = avnd->node_info.nodeId;
		avd_pg_node_csi_del_all(cb, avnd);
	}

	cb->avail_state_avd = SA_AMF_HA_STANDBY;

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}


/****************************************************************************\
 * Function: amfd_switch_stdby_actv
 *
 * Purpose:  function to handle AVD's role 
 *           change from standby to active in liue of SI SWAP Action
 * 
 * Input: cb - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/

uns32 amfd_switch_stdby_actv(AVD_CL_CB *cb)
{
	uns32 status = NCSCC_RC_SUCCESS;
	
	TRACE_ENTER();

	LOG_NO("AMFD Switching StandBy --> Active State");

	/*
	 * Check whether Standby is in sync with Active. If yes then
	 * proceed further. Else return failure.
	 */
	if (AVD_STBY_OUT_OF_SYNC == cb->stby_sync_state) {
		LOG_ER("AMFD: Switch Standby --> Active FAILED, Standby OUT OF SYNC");
		cb->swap_switch = SA_FALSE;
		avd_d2d_chg_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_ACTIVE);
		return NCSCC_RC_FAILURE;
	}

	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_ACTIVE))) {
		LOG_ER("AMFD: Switch Standby --> Active FAILED, CKPT role set failed");
		cb->swap_switch = SA_FALSE;
		avd_d2d_chg_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_ACTIVE);
		return NCSCC_RC_FAILURE;
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		LOG_ER("AMFD: Switch Standby --> Active FAILED, Mbcsv Dispatch failed");
		cb->swap_switch = SA_FALSE;
		avd_d2d_chg_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_ACTIVE);
		return NCSCC_RC_FAILURE;
	}

	/*
	 * We might be having some async update messages in the
	 * Queue to be processed, now drop all of them.
	 */
	avsv_dequeue_async_update_msgs(cb, FALSE);

	cb->avail_state_avd = SA_AMF_HA_ACTIVE;

	/* Declare this standby as Active. Set Vdest role role */
	if (NCSCC_RC_SUCCESS != (status = avd_mds_set_vdest_role(cb, SA_AMF_HA_ACTIVE))) {
		LOG_ER("AMFD: Switch Standby --> Active FAILED, MDS role set failed");
		cb->swap_switch = SA_FALSE;
		avd_d2d_chg_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_ACTIVE);
		return NCSCC_RC_FAILURE;
	}
	
	if (NCSCC_RC_SUCCESS != avd_rde_set_role(SA_AMF_HA_ACTIVE)) {
		LOG_ER("rde role change failed from stdy -> Active");
	}

	/* Time to send fail-over messages to all the AVND's */
	avd_fail_over_event(cb);

	/* We need to send the role to AvND. */
	status = avd_avnd_send_role_change(cb, cb->node_id_avd, cb->avail_state_avd);
	if (NCSCC_RC_SUCCESS != status) {
		m_AVD_PXY_PXD_ERR_LOG("amfd_switch_stdby_actv: role sent failed. Node Id and role are",
				NULL, cb->node_id_avd, cb->avail_state_avd, 0, 0);
	} else {
		avd_d2n_msg_dequeue(cb);
	}
	cb->swap_switch = SA_FALSE;

	if (avd_imm_config_get() != NCSCC_RC_SUCCESS) {
		LOG_ER("AMFD: Switch Standby --> Active, imm Config failed");
		avd_d2d_chg_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_ACTIVE);
		return NCSCC_RC_FAILURE;
	}

	if (avd_imm_impl_set() != SA_AIS_OK) {
		LOG_ER("AMFD: Switch Standby --> Active, imm implement failed");
		avd_d2d_chg_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_ACTIVE);
		return NCSCC_RC_FAILURE;
	}

	if(avd_clm_track_start() != SA_AIS_OK) {
		LOG_ER("AMFD: Switch Standby --> Active, clm track start failed");
		avd_d2d_chg_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_ACTIVE);
		return NCSCC_RC_FAILURE;
	}

	/* Send the message to other avd for role change rsp as success */
	avd_d2d_chg_role_rsp(cb, NCSCC_RC_SUCCESS, SA_AMF_HA_ACTIVE);

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: amfd_switch_qsd_actv
 *
 * Purpose:  function to handle AVD's role switch from Quiesced to 
 *           Active.  
 *
 * Input: cb - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/

uns32 amfd_switch_qsd_actv (AVD_CL_CB *cb)
{
	cb->swap_switch = SA_FALSE;
	if (NCSCC_RC_SUCCESS != avd_rde_set_role(SA_AMF_HA_ACTIVE)) {
		LOG_ER("rde role change failed from qsd -> actv");
	}
	return amfd_switch_stdby_actv(cb);
}


/****************************************************************************\
 * Function: avd_rde_set_role
 *
 * Purpose:  set the role of RDE
 *
 * Input: role - Role to be set of RDE
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/

uns32 avd_rde_set_role (SaAmfHAStateT role)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	PCS_RDA_REQ pcs_rda_req;

	pcs_rda_req.req_type = PCS_RDA_SET_ROLE;

	pcs_rda_req.info.io_role = role;

	if(PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req)) {
		return NCSCC_RC_FAILURE;
	}
	return rc;
}

