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

static uns32 avd_role_switch_actv_qsd(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_switch_qsd_actv(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_switch_qsd_stdby(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_switch_stdby_actv(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_failover(AVD_CL_CB *cb, SaAmfHAStateT role);
static uns32 avd_role_failover_qsd_actv(AVD_CL_CB *cb, SaAmfHAStateT role);

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
	AVM_AVD_SYS_CON_ROLE_T *msg = &evt->info.avm_msg->avm_avd_msg.role;
	uns32 status = NCSCC_RC_SUCCESS;

	TRACE_ENTER2("cause=%u, role=%u", msg->cause, msg->role);

	if ((cb->role_switch == SA_TRUE) && (msg->cause == AVM_ADMIN_SWITCH_OVER)) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->role_switch);
		m_AVD_LOG_INVALID_VAL_FATAL(msg->cause);
		avm_avd_free_msg(&evt->info.avm_msg);
		goto done;
	}

	/* 
	 * First validate that this is correct role switch.
	 */
	if (((cb->avail_state_avd == SA_AMF_HA_ACTIVE) &&
	     (msg->role == SA_AMF_HA_STANDBY)) ||
	    ((cb->avail_state_avd == SA_AMF_HA_STANDBY) && (msg->role == SA_AMF_HA_QUIESCED))) {
		m_AVD_LOG_INVALID_VAL_ERROR(cb->avail_state_avd);
		m_AVD_LOG_INVALID_VAL_ERROR(msg->role);
		status = NCSCC_RC_FAILURE;
		goto done;
	}

	/* Check what is the cause of this role set */
	if ((msg->cause == AVM_INIT_ROLE) &&
	    (cb->role_set == FALSE) && ((msg->role == SA_AMF_HA_ACTIVE) || (msg->role == SA_AMF_HA_STANDBY))) {
		/* Set Initial role of AVD */
		status = avd_init_role_set(cb, msg->role);
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (msg->cause == AVM_ADMIN_SWITCH_OVER) &&
	    (cb->avail_state_avd == SA_AMF_HA_ACTIVE) && (msg->role == SA_AMF_HA_QUIESCED)) {
		/* Admn Switch Active -> Quiesced */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_ATOQ, NCSFL_SEV_NOTICE, 0);
		cb->role_switch = SA_TRUE;
		status = avd_role_switch_actv_qsd(cb, msg->role);
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (msg->cause == AVM_ADMIN_SWITCH_OVER) &&
	    (msg->role == SA_AMF_HA_ACTIVE) && (cb->avail_state_avd == SA_AMF_HA_QUIESCED)) {
		/* Admn Switch Quiesced -> Active */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_QTOA, NCSFL_SEV_NOTICE, 0);
		cb->role_switch = SA_TRUE;
		status = avd_role_switch_qsd_actv(cb, msg->role);
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (msg->cause == AVM_ADMIN_SWITCH_OVER) &&
	    (msg->role == SA_AMF_HA_STANDBY) && (cb->avail_state_avd == SA_AMF_HA_QUIESCED)) {
		/* Admn Switch Quiesced -> Standby */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_QTOS, NCSFL_SEV_NOTICE, 0);
		cb->role_switch = SA_TRUE;
		status = avd_role_switch_qsd_stdby(cb, msg->role);
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (msg->cause == AVM_ADMIN_SWITCH_OVER) &&
	    (msg->role == SA_AMF_HA_ACTIVE) && (cb->avail_state_avd == SA_AMF_HA_STANDBY)) {
		/* Admn Switch Standby -> Active */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_STOA, NCSFL_SEV_NOTICE, 0);
		cb->role_switch = SA_TRUE;
		status = avd_role_switch_stdby_actv(cb, msg->role);
		goto done;
	}

	if ((cb->role_set == TRUE) &&
	    (msg->cause == AVM_FAIL_OVER) &&
	    (cb->avail_state_avd == SA_AMF_HA_QUIESCED) && (msg->role == SA_AMF_HA_ACTIVE)) {
		/* Fail-over Quiesced to Active */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_QTOA, NCSFL_SEV_NOTICE, 0);
		status = avd_role_failover_qsd_actv(cb, msg->role);
		goto done;

	}

	if ((cb->role_set == TRUE) &&
	    (msg->cause == AVM_FAIL_OVER) &&
	    (cb->avail_state_avd == SA_AMF_HA_STANDBY) && (msg->role == SA_AMF_HA_ACTIVE)) {
		/* Fail-over Standby to Active */
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_STOA, NCSFL_SEV_NOTICE, 0);
		status = avd_role_failover(cb, msg->role);

	} else {
		/* AVM is screwing up something. Log error */
		m_AVD_LOG_INVALID_VAL_ERROR(cb->role_set);
		m_AVD_LOG_INVALID_VAL_ERROR(msg->cause);
		m_AVD_LOG_INVALID_VAL_ERROR(cb->avail_state_avd);
		m_AVD_LOG_INVALID_VAL_ERROR(msg->role);
		status = NCSCC_RC_FAILURE;
	}

	/* Send this status to AVM in the role response message */
 done:
	if (NCSCC_RC_SUCCESS != status) {
		cb->role_switch = SA_FALSE;
		m_AVD_LOG_CKPT_EVT(AVD_ROLE_CHANGE_FAILURE, NCSFL_SEV_NOTICE, 0);
	}

	avm_avd_free_msg(&evt->info.avm_msg);
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
		} else
			LOG_NO("avd_clm_track_start success");
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
				avd_saImmOiRtObjectUpdate(&avd_avnd->node_info.nodeName, "saAmfNodeOperState",
						SA_IMM_ATTR_SAUINT32T, &avd_avnd->saAmfNodeOperState);

				avd_avnd = avd_node_getnext(&avd_avnd->node_info.nodeName);
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
 * Function: avd_role_switch_actv_qsd
 *
 * Purpose:  AVSV function to handle AVD's role switch from AvM requesting
 *           the role change from Active to Quiesced in liue of Adminstartive
 *           action. 
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
static uns32 avd_role_switch_actv_qsd(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	AVD_SU *i_su = NULL;
	AVD_AVND *avnd = NULL;
	AVD_AVND *avnd_other = NULL;
	SaClmNodeIdT node_id = 0;

	m_AVD_LOG_FUNC_ENTRY("avd_role_switch_actv_qsd");
	m_NCS_DBG_PRINTF("\nROLE SWITCH Active --> Quiesced");
	syslog(LOG_NOTICE, "ROLE SWITCH Active --> Quiesced");

	if (cb->init_state != AVD_APP_STATE) {
		m_AVD_LOG_INVALID_VAL_ERROR(cb->init_state);
		return NCSCC_RC_FAILURE;
	}

	/* get the avnd from node_id */
	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd);
		return NCSCC_RC_FAILURE;
	}

	/* get the avnd from node_id */
	if (NULL == (avnd_other = avd_node_find_nodeid(cb->node_id_avd_other))) {
		m_AVD_LOG_INVALID_VAL_ERROR(cb->node_id_avd_other);
		return NCSCC_RC_FAILURE;
	}

	/* check if the other AVD is up and has standby role. verify that
	 * the AvND on the node is in present state. if failure send a
	 * message to AvM indicating failure response. 
	 */
	if ((cb->node_id_avd_other == 0) ||
	    (cb->avail_state_avd_other != SA_AMF_HA_STANDBY) ||
	    (avnd->node_state != AVD_AVND_STATE_PRESENT) || (avnd_other->node_state != AVD_AVND_STATE_PRESENT)) {
		m_AVD_LOG_INVALID_VAL_ERROR(cb->node_id_avd_other);
		m_AVD_LOG_INVALID_VAL_ERROR(cb->avail_state_avd_other);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd_other->node_state);
		m_AVD_LOG_CKPT_EVT(AVD_STBY_UNAVAIL_FOR_RCHG, NCSFL_SEV_NOTICE, 0);
		return NCSCC_RC_FAILURE;
	}

	/*  Mark AVD as Quiesced. */
	cb->avail_state_avd = SA_AMF_HA_QUIESCED;

	if (avd_clm_track_stop() != SA_AIS_OK) {
		LOG_ER("ClmTrack stop failed");
	}

	/* if no NCS Su's are present, go ahead and set mds role */
	if (avnd->list_of_ncs_su == NULL) {
		uns32 rc = NCSCC_RC_SUCCESS;
		if (NCSCC_RC_SUCCESS != (rc = avd_mds_set_vdest_role(cb, SA_AMF_HA_QUIESCED))) {
			m_AVD_LOG_INVALID_VAL_FATAL(rc);
			return NCSCC_RC_FAILURE;
		}
		/* We need to send the role to AvND. */
		rc = avd_avnd_send_role_change(cb, cb->node_id_avd, cb->avail_state_avd);
		if (NCSCC_RC_SUCCESS != rc) {
			m_AVD_PXY_PXD_ERR_LOG("avd_role_switch_actv_qsd: role sent failed. Node Id and role are",
					      NULL, cb->node_id_avd, cb->avail_state_avd, 0, 0);
		} else {
			avd_d2n_msg_dequeue(cb);
		}

	}

	/*  Send Quiesced to all Active NCS Su's having 2N redun model, present in this node */
	for (i_su = avnd->list_of_ncs_su; i_su != NULL; i_su = i_su->avnd_list_su_next) {
		if ((i_su->list_of_susi != 0) && (i_su->list_of_susi->state == SA_AMF_HA_ACTIVE) &&
		    (i_su->sg_of_su->sg_redundancy_model == SA_AMF_2N_REDUNDANCY_MODEL))
			avd_sg_su_si_mod_snd(cb, i_su, SA_AMF_HA_QUIESCED);
	}

	/* Give up our IMM OI implementer role */
	(void)immutil_saImmOiImplementerClear(cb->immOiHandle);
	cb->impl_set = FALSE;


	/* Walk through all the nodes and stop AvND rcv HeartBeat. */
	while (NULL != (avnd = avd_node_getnext_nodeid(node_id))) {
		node_id = avnd->node_info.nodeId;

		m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);
		/* stop the timer if it exists */
		avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));
		m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);
	}

	return NCSCC_RC_SUCCESS;
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
	if (cb->role_switch == SA_TRUE) {
		cb->role_switch = SA_FALSE;
	}

	/*
	 * Check whether Standby is in sync with Active. If yes then
	 * proceed further. Else return failure.
	 */
	if (AVD_STBY_OUT_OF_SYNC == cb->stby_sync_state) {
		m_AVD_LOG_CKPT_EVT(AVD_STBY_UNAVAIL_FOR_RCHG, NCSFL_SEV_NOTICE, 0);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER StandBy --> Active FAILED, Stanby OUT OF SYNC\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER StandBy --> Active FAILED, Stanby OUT OF SYNC");
		return NCSCC_RC_FAILURE;
	}

	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER StandBy --> Active FAILED, DB not found\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER StandBy --> Active FAILED, DB not found");
		return NCSCC_RC_FAILURE;
	}

	/* check the node state */
	if (avnd->node_state != AVD_AVND_STATE_PRESENT) {
		m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_state);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER StandBy --> Active FAILED, stdby not in good state\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER StandBy --> Active FAILED, stdby not in good state");
		return NCSCC_RC_FAILURE;
	}

	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_ACTIVE))) {
		/* log error that the node id is invalid */
		m_AVD_LOG_INVALID_VAL_ERROR(cb->node_id_avd_other);
		m_AVD_LOG_INVALID_VAL_ERROR(status);
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DISPATCH_FAILURE, NCSFL_SEV_NOTICE, status);
		m_AVD_LOG_INVALID_VAL_ERROR(status);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER StandBy --> Active FAILED, MBCSV DISPATCH FAILED\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER StandBy --> Active FAILED, MBCSV DISPATCH FAILED");
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
		/* log error that the node id is invalid */
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_VDEST_ROL);
		m_AVD_LOG_INVALID_VAL_ERROR(cb->node_id_avd_other);
		m_AVD_LOG_INVALID_VAL_ERROR(status);
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

	m_AVD_LOG_FUNC_ENTRY("avd_role_failover_qsd_actv");
	m_NCS_DBG_PRINTF("\nFAILOVER Quiesced --> Active");
	syslog(LOG_NOTICE, "FAILOVER Quiesced --> Active");

	/* If we are in the middle of admin switch, ignore it */
	if (cb->role_switch == SA_TRUE) {
		cb->role_switch = SA_FALSE;
	}

	/*
	 * Check whether Standby is in sync with Active. If yes then
	 * proceed further. Else return failure.
	 */
	if (AVD_STBY_OUT_OF_SYNC == cb->stby_sync_state) {
		m_AVD_LOG_CKPT_EVT(AVD_STBY_UNAVAIL_FOR_RCHG, NCSFL_SEV_NOTICE, 0);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER Quiesced --> Active FAILED, Stanby OUT OF SYNC\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER Quiesced --> Active FAILED, Stanby OUT OF SYNC");
		return NCSCC_RC_FAILURE;
	}

	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER Quiesced --> Active FAILED, DB not found\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER Quiesced --> Active FAILED, DB not found");
		return NCSCC_RC_FAILURE;
	}

	/* check the node state */
	if (avnd->node_state != AVD_AVND_STATE_PRESENT) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER Quiesced --> Active FAILED, stdby not in good state\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER Quiesced --> Active FAILED, stdby not in good state");
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
		m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DISPATCH_FAILURE, NCSFL_SEV_NOTICE, status);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER Quiesced --> Active FAILED, MBCSV DISPATCH FAILED\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER Quiesced --> Active FAILED, MBCSV DISPATCH FAILED");
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
		m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER Quiesced --> Active FAILED, MEMALLOC FAILED\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER Quiesced --> Active FAILED, MEMALLOC FAILED");
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
		m_NCS_DBG_PRINTF("\nAvSv: FAILOVER Quiesced --> Active FAILED, IPC SEND FAILED\n");
		syslog(LOG_ERR, "NCS_AvSv: FAILOVER Quiesced --> Active FAILED, IPC SEND FAILED");

		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);

	/* We are successfully changed role to Active. Gen a reset 
	 * responce for the other card. TODO */

	cb->node_id_avd_other = 0;

	if (avd_imm_config_get() != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	avd_imm_impl_set_task_create();

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
	AVD_AVND *avnd = NULL;
	AVD_SU *i_su = NULL;
	AVSV_N2D_INFO_SU_SI_ASSIGN_MSG_INFO assign;

	m_AVD_LOG_FUNC_ENTRY("avd_role_switch_ncs_su");

	/* get the avnd from node_id */
	if (NULL == (avnd = avd_node_find_nodeid(cb->node_id_avd))) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd);
		return;
	}

	/* if we are not having any NCS SU's just jump to next level */
	if ((avnd->list_of_ncs_su == NULL) && (cb->role_switch == SA_TRUE)) {
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
			if (cb->role_switch == SA_TRUE) {
				avd_sg_su_si_mod_snd(cb, i_su, SA_AMF_HA_ACTIVE);
			} else {
				avd_sg_su_oper_list_add(cb, i_su, FALSE);
				m_AVD_SET_SU_SWITCH(cb, i_su, AVSV_SI_TOGGLE_SWITCH);
				m_AVD_SET_SG_FSM(cb, (i_su->sg_of_su), AVD_SG_FSM_SU_OPER);
				m_AVD_LOG_RCVD_VAL(i_su->sg_of_su->sg_fsm_state);
			}
		}
	}

	return;
}

/****************************************************************************\
 * Function: avd_role_switch_stdby_actv
 *
 * Purpose:  AVSV function to handle AVD's role switch from AvM requesting
 *           the role change from standby to active in liue of Adminstartive
 *           action. 
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
static uns32 avd_role_switch_stdby_actv(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	AVD_EVT *evt = AVD_EVT_NULL;
	uns32 status = NCSCC_RC_SUCCESS;

	m_AVD_LOG_FUNC_ENTRY("avd_role_switch_stdby_actv");
	m_NCS_DBG_PRINTF("\nROLE SWITCH StandBy --> Active");
	syslog(LOG_NOTICE, "ROLE SWITCH StandBy --> Active");

	/*
	 * Check whether Standby is in sync with Active. If yes then
	 * proceed further. Else return failure.
	 */
	if (AVD_STBY_OUT_OF_SYNC == cb->stby_sync_state) {
		m_AVD_LOG_CKPT_EVT(AVD_STBY_UNAVAIL_FOR_RCHG, NCSFL_SEV_NOTICE, role);
		m_NCS_DBG_PRINTF("\nAvSv: Switchover Standby --> Active FAILED, Standby OUT OF SYNC\n");
		syslog(LOG_ERR, "NCS_AvSv: Switchover Standby --> Active FAILED, Standby OUT OF SYNC");
		return NCSCC_RC_FAILURE;
	}

	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_ACTIVE))) {
		/* log error that the node id is invalid */
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		m_NCS_DBG_PRINTF("\nAvSv: Switchover Standby --> Active FAILED, CKPT role set failed\n");
		syslog(LOG_ERR, "NCS_AvSv: Switchover Standby --> Active FAILED, CKPT role set failed");
		return NCSCC_RC_FAILURE;
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DISPATCH_FAILURE, NCSFL_SEV_NOTICE, status);
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		m_NCS_DBG_PRINTF("\nAvSv: Switchover Standby --> Active FAILED, Dispatch failed\n");
		syslog(LOG_ERR, "NCS_AvSv: Switchover Standby --> Active FAILED, Dispatch failed");
		return NCSCC_RC_FAILURE;
	}

	/*
	 * We might be having some async update messages in the
	 * Queue to be processed, now drop all of them.
	 */
	avsv_dequeue_async_update_msgs(cb, FALSE);

	cb->avail_state_avd = role;

	/* Declare this standby as Active. Set Vdest role role */
	if (NCSCC_RC_SUCCESS != (status = avd_mds_set_vdest_role(cb, role))) {
		/* log error that the node id is invalid */
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_VDEST_ROL);
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		m_NCS_DBG_PRINTF("\nAvSv: Switchover Standby --> Active FAILED, MDS role set failed\n");
		syslog(LOG_ERR, "NCS_AvSv: Switchover Standby --> Active FAILED, MDS role set failed");
		return NCSCC_RC_FAILURE;
	}

	/* Time to send fail-over messages to all the AVND's */
	avd_fail_over_event(cb);

	/* We need to send the role to AvND. */
	status = avd_avnd_send_role_change(cb, cb->node_id_avd, cb->avail_state_avd);
	if (NCSCC_RC_SUCCESS != status) {
		m_AVD_PXY_PXD_ERR_LOG("avd_role_switch_stdby_actv: role sent failed. Node Id and role are",
				      NULL, cb->node_id_avd, cb->avail_state_avd, 0, 0);
	} else {
		avd_d2n_msg_dequeue(cb);
	}

	/* Post an evt on mailbox to set active role to all NCS SU */
	/* create the message event */
	evt = calloc(1, sizeof(AVD_EVT));
	if (evt == AVD_EVT_NULL) {
		/* log error */
		m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
		m_NCS_DBG_PRINTF("\nAvSv: Switchover Standby --> Active FAILED, MALLOC failed\n");
		syslog(LOG_ERR, "NCS_AvSv: Switchover Standby --> Active FAILED, MALLOC failed");
		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_RCVD_VAL(((long)evt));

	evt->rcv_evt = AVD_EVT_SWITCH_NCS_SU;

	m_AVD_LOG_EVT_INFO(AVD_SND_AVND_MSG_EVENT, evt->rcv_evt);

	if (m_NCS_IPC_SEND(&cb->avd_mbx, evt, NCS_IPC_PRIORITY_LOW)
	    != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
		/* log error */
		/* free the event and return */
		free(evt);

		m_NCS_DBG_PRINTF("\nAvSv: Switchover Standby --> Active FAILED, IPC send failed\n");
		syslog(LOG_ERR, "NCS_AvSv: Switchover Standby --> Active FAILED, IPC send failed");
		return NCSCC_RC_FAILURE;
	}

	m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);

	return NCSCC_RC_SUCCESS;
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
	m_AVD_LOG_FUNC_ENTRY("avd_role_switch_qsd_actv");
	m_NCS_DBG_PRINTF("\nROLE SWITCH Quiesced --> Active");
	syslog(LOG_NOTICE, "ROLE SWITCH Quiesced --> Active");

	return avd_role_switch_stdby_actv(cb, role);
}

/****************************************************************************\
 * Function: avd_role_switch_qsd_stdby
 *
 * Purpose:  AVSV function to handle AVD's role switch from Quiesced to 
 *           Standby.  
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
static uns32 avd_role_switch_qsd_stdby(AVD_CL_CB *cb, SaAmfHAStateT role)
{
	AVD_AVND *avnd = NULL;
	uns32 node_id = 0;
	uns32 status = NCSCC_RC_SUCCESS;

	m_AVD_LOG_FUNC_ENTRY("avd_role_switch_qsd_stdby");
	m_NCS_DBG_PRINTF("\nROLE SWITCH Quiesced --> StandBy");
	syslog(LOG_NOTICE, "ROLE SWITCH Quiesced --> StandBy");
	if (NCSCC_RC_SUCCESS != (status = avd_mds_set_vdest_role(cb, SA_AMF_HA_STANDBY))) {
		/* log error that the node id is invalid */
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_VDEST_ROL);
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		return NCSCC_RC_FAILURE;
	}

	/* Change MBCSv role to Standby */
	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_STANDBY))) {
		/* log error that the node id is invalid */
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		return NCSCC_RC_FAILURE;
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DISPATCH_FAILURE, NCSFL_SEV_NOTICE, status);
		m_AVD_LOG_INVALID_VAL_FATAL(status);
		return NCSCC_RC_FAILURE;
	}

	node_id = 0;

	/* Walk through all the nodes and  free PG records. */
	while (NULL != (avnd = avd_node_getnext_nodeid(node_id))) {
		node_id = avnd->node_info.nodeId;
		avd_pg_node_csi_del_all(cb, avnd);
	}

	/* Send responce to AvM */
	cb->avail_state_avd = role;
	cb->role_switch = SA_FALSE;

	return NCSCC_RC_SUCCESS;
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

	m_AVD_LOG_FUNC_ENTRY("avd_mds_qsd_role_func");
	m_NCS_DBG_PRINTF("\nAVD: MDS became Quiesced");

	if (cb->role_switch == SA_FALSE)
		return;

	/* Now set the MBCSv role to quiesced, then inform AvM about a successful switch. */
	if (NCSCC_RC_SUCCESS != (status = avsv_set_ckpt_role(cb, SA_AMF_HA_QUIESCED))) {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(status);
	}

	/* Now Dispatch all the messages from the MBCSv mail-box */
	if (NCSCC_RC_SUCCESS != (rc = avsv_mbcsv_dispatch(cb, SA_DISPATCH_ALL))) {
		m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
		m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DISPATCH_FAILURE, NCSFL_SEV_NOTICE, rc);
		return;
	}

	/* reset the role switch flag */
	cb->role_switch = SA_FALSE;

	return;
}
