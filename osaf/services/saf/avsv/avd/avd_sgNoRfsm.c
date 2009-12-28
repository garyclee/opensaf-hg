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

  DESCRIPTION: This file is part of the SG processing module. It contains
  the SG state machine, for processing the events related to SG for No
  redundancy model.

..............................................................................

  FUNCTIONS INCLUDED in this file:

  avd_sg_nored_su_chose_asgn - Choose and assign SIs to SUs. 
  avd_sg_nored_si_func - Function to process the new complete SI in the SG.
  avd_sg_nored_su_fault_func - function is called when a SU failed and switchover
                               needs to be done.
  avd_sg_nored_su_insvc_func - function is called when a SU readiness state changes
                            to inservice from out of service
  avd_sg_nored_susi_sucss_func - processes successful SUSI assignment. 
  avd_sg_nored_susi_fail_func - processes failure of SUSI assignment.
  avd_sg_nored_realign_func - function called when SG operation is done or cluster
                           timer expires.
  avd_sg_nored_node_fail_func - function is called when the node has already failed and
                             the SIs have to be failed over.
  avd_sg_nored_su_admin_fail -  function is called when SU is LOCKED/SHUTDOWN.  
  avd_sg_nored_si_admin_down - function is called when SIs is LOCKED/SHUTDOWN.
  avd_sg_nored_sg_admin_down - function is called when SGs is LOCKED/SHUTDOWN.
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <logtrace.h>

#include <avd.h>

/*****************************************************************************
 * Function: avd_sg_nored_su_chose_asgn
 *
 * Purpose:  This function will identify unassigned SIs, search for
 * unassigned in-service SUs and assign this unassigned SIs to them by
 * Sending D2N-INFO_SU_SI_ASSIGN message for the SUs with role active for the 
 * SIs. It then adds the Assigning SUs to the SU operation list. If no
 * unassigned ready SI or unassigned in-service SU exists, it returns NULL.
 *
 * Input: cb - the AVD control block
 *        sg - The pointer to the service group.
 *        
 *
 * Returns: pointer to the first SU that is undergoing assignment. Null if
 *          no assignments need to happen.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

static AVD_SU *avd_sg_nored_su_chose_asgn(AVD_CL_CB *cb, AVD_SG *sg)
{
	AVD_SU *i_su;
	AVD_SI *i_si;
	NCS_BOOL l_flag;
	AVD_SU_SI_REL *tmp;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_su_chose_asgn");

	i_si = sg->list_of_si;
	i_su = sg->list_of_su;

	while ((i_si != AVD_SI_NULL) && (i_su != NULL)) {
		/* Screen SI sponsors state and adjust the SI-SI dep state accordingly */
		avd_screen_sponsor_si_state(cb, i_si, FALSE);

		/* verify that the SI is unassigned and ready */
		if ((i_si->saAmfSIAdminState != SA_AMF_ADMIN_UNLOCKED) ||
		    (i_si->si_dep_state == AVD_SI_SPONSOR_UNASSIGNED) ||
		    (i_si->si_dep_state == AVD_SI_UNASSIGNING_DUE_TO_DEP) ||
		    (i_si->max_num_csi != i_si->num_csi) || (i_si->list_of_sisu != AVD_SU_SI_REL_NULL)) {
			i_si = i_si->sg_list_of_si_next;
			continue;
		}

		/* identify a in-service unassigned SU so that the SI can be assigned. */
		l_flag = TRUE;
		while ((i_su != NULL) && (l_flag == TRUE)) {
			if ((i_su->saAmfSuReadinessState == SA_AMF_READINESS_IN_SERVICE) &&
			    (i_su->list_of_susi == AVD_SU_SI_REL_NULL)) {
				l_flag = FALSE;
				continue;
			}

			i_su = i_su->sg_list_su_next;
		}

		if (i_su == NULL)
			continue;

		/* if the SU is not null assign active to that SU for the SI. */
		if (avd_new_assgn_susi(cb, i_su, i_si, SA_AMF_HA_ACTIVE, FALSE, &tmp) == NCSCC_RC_SUCCESS) {
			/* Add the SU to the operation list */
			avd_sg_su_oper_list_add(cb, i_su, FALSE);

			/* since both this SI and SU have a relationship choose the next Si and
			 * SU.
			 */
			i_si = i_si->sg_list_of_si_next;
			i_su = i_su->sg_list_su_next;
		} else {
			/* log a fatal error */
			m_AVD_LOG_INVALID_VAL_ERROR(((long)i_si));
			m_AVD_LOG_INVALID_NAME_VAL_ERROR(i_si->name.value, i_si->name.length);

			/* choose the next SU */
			i_su = i_su->sg_list_su_next;
		}

	}			/* while ((i_si != AVD_SI_NULL) && (i_su != AVD_SU_NULL)) */

	return sg->su_oper_list.su;
}

/*****************************************************************************
 * Function: avd_sg_nored_si_func
 *
 * Purpose:  This function is called when a new SI is added to a SG. The SG is
 * of type No redundancy model. This function will perform the functionality
 * described in the SG FSM design. 
 *
 * Input: cb - the AVD control block
 *        si - The pointer to the service instance.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a No redundancy model specific function. If there are
 * any SIs being transitioned due to operator, this call will return
 * failure.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_si_func(AVD_CL_CB *cb, AVD_SI *si)
{
	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_si_func");
	m_AVD_LOG_RCVD_VAL(((long)si));

	m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);

	/* If the SG FSM state is not stable just return success. */
	if (si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE) {
		return NCSCC_RC_SUCCESS;
	}

	if ((cb->init_state != AVD_APP_STATE) && (si->sg_of_si->sg_ncs_spec == SA_FALSE)) {
		m_AVD_LOG_INVALID_VAL_ERROR(si->sg_of_si->sg_ncs_spec);
		return NCSCC_RC_SUCCESS;
	}

	if (avd_sg_nored_su_chose_asgn(cb, si->sg_of_si) == NULL) {
		/* all the assignments have already been done in the SG. */
		return NCSCC_RC_SUCCESS;
	}

	/* change the FSM state */
	m_AVD_SET_SG_FSM(cb, (si->sg_of_si), AVD_SG_FSM_SG_REALIGN);

	m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);

	return NCSCC_RC_SUCCESS;
}

 /*****************************************************************************
 * Function: avd_sg_nored_su_fault_func
 *
 * Purpose:  This function is called when a SU readiness state changes to
 * OOS due to a fault. It will do the functionality specified in
 * SG FSM.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the service unit.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_su_fault_func(AVD_CL_CB *cb, AVD_SU *su)
{

	SaAmfHAStateT old_state;
	AVD_SU_SI_STATE old_fsm_state;
	NCS_BOOL flag;
	AVD_AVND *su_node_ptr = NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_su_fault_func");
	m_AVD_LOG_RCVD_VAL(((long)su));

	m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

	if (su->list_of_susi == AVD_SU_SI_REL_NULL)
		return NCSCC_RC_SUCCESS;

	/* Do the functionality based on the current state. */
	switch (su->sg_of_su->sg_fsm_state) {
	case AVD_SG_FSM_STABLE:

		/* change the state of the SI to quiesced */
		old_state = su->list_of_susi->state;
		old_fsm_state = su->list_of_susi->fsm;
		su->list_of_susi->state = SA_AMF_HA_QUIESCED;
		su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
		avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
		if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
			/* log a fatal error */
			m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
			m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
			m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
							 su->list_of_susi->si->name.length);
			su->list_of_susi->state = old_state;
			su->list_of_susi->fsm = old_fsm_state;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
			return NCSCC_RC_FAILURE;
		}

		/* add the SU to the operation list and change the SG FSM to SU operation. */
		avd_sg_su_oper_list_add(cb, su, FALSE);
		m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SU_OPER);

		m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

		break;		/* case AVD_SG_FSM_STABLE: */
	case AVD_SG_FSM_SG_REALIGN:

		if (su->list_of_susi->si == su->sg_of_su->admin_si) {
			/* The SUs SI is in the SI admin pointer. If this SI admin is
			 * shutdown change to LOCK and send D2N-INFO_SU_SI_ASSIGN with
			 * quiesced to the SU. Remove the SI from SI admin pointer.
			 * Add the SU to operation list and stay in the same state.
			 */
			if (su->list_of_susi->si->saAmfSIAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				old_state = su->list_of_susi->state;
				old_fsm_state = su->list_of_susi->fsm;
				su->list_of_susi->state = SA_AMF_HA_QUIESCED;
				su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
					/* log a fatal error */
					m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
									 su->list_of_susi->si->name.length);
					su->list_of_susi->state = old_state;
					su->list_of_susi->fsm = old_fsm_state;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
					return NCSCC_RC_FAILURE;
				}

				m_AVD_SET_SI_ADMIN(cb, (su->list_of_susi->si), SA_AMF_ADMIN_LOCKED);
			}

			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
		} else {	/* if (su->list_of_susi->si == su->sg_of_su->admin_si) */

			m_AVD_CHK_OPLIST(su, flag);
			if (flag == TRUE) {
				/* The SU is same as the SU in the list. If the HA state assignment
				 * for the SU is active, Send D2N-INFO_SU_SI_ASSIGN with quiesced
				 * to the SU. Stay in the same state. If the HA state assignment
				 * for the SU is quiescing. Send D2N-INFO_SU_SI_ASSIGN with
				 * quiesced to the SU. Stay in the same state. Change admin state
				 * of SU to LOCK. For other HA states, Stay in same state. 
				 */

				if ((su->list_of_susi->state == SA_AMF_HA_ACTIVE) ||
				    (su->list_of_susi->state == SA_AMF_HA_QUIESCING)) {
					old_state = su->list_of_susi->state;
					old_fsm_state = su->list_of_susi->fsm;
					su->list_of_susi->state = SA_AMF_HA_QUIESCED;
					su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
					if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) ==
					    NCSCC_RC_FAILURE) {
						/* log a fatal error */
						m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
						m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
						m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
										 su->list_of_susi->si->name.length);
						su->list_of_susi->state = old_state;
						su->list_of_susi->fsm = old_fsm_state;
						m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi),
										 AVSV_CKPT_AVD_SI_ASS);
						avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
						return NCSCC_RC_FAILURE;
					}

					m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);
					if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
						avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
					} else if (su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
						m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
						if (flag == TRUE) {
							node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
						}
					}
				}

			} else {	/* if (flag == TRUE) */

				/* The SU is not part of the SU operation list and the SUs SI is
				 * not part of the SI admin pointer. Send D2N-INFO_SU_SI_ASSIGN with
				 * quiesced to the SU.  Add the SU to the SU operation list. 
				 * Stay in the same state.
				 */
				old_state = su->list_of_susi->state;
				old_fsm_state = su->list_of_susi->fsm;
				su->list_of_susi->state = SA_AMF_HA_QUIESCED;
				su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
					/* log a fatal error */
					m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
									 su->list_of_susi->si->name.length);
					su->list_of_susi->state = old_state;
					su->list_of_susi->fsm = old_fsm_state;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
					return NCSCC_RC_FAILURE;
				}

				avd_sg_su_oper_list_add(cb, su, FALSE);

			}	/* else (flag == TRUE) */

		}		/* else (su->list_of_susi->si == su->sg_of_su->admin_si) */

		break;		/* case AVD_SG_FSM_SG_REALIGN: */
	case AVD_SG_FSM_SU_OPER:

		if (su->sg_of_su->su_oper_list.su == su) {
			/* The SU is same as the SU in the list. If the SI relationships to
			 * the SU is quiescing change the SU admin to LOCK. 
			 * Send D2N-INFO_SU_SI_ASSIGN modify quiesced.
			 */
			if (su->list_of_susi->state == SA_AMF_HA_QUIESCING) {
				old_state = su->list_of_susi->state;
				old_fsm_state = su->list_of_susi->fsm;
				su->list_of_susi->state = SA_AMF_HA_QUIESCED;
				su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
					/* log a fatal error */
					m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
									 su->list_of_susi->si->name.length);
					su->list_of_susi->state = old_state;
					su->list_of_susi->fsm = old_fsm_state;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
					return NCSCC_RC_FAILURE;
				}

				m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);
				if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
					avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
				} else if (su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
					m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
					if (flag == TRUE) {
						node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
					}
				}
			}	/* if (su->list_of_susi->state == SA_AMF_HA_QUIESCING) */
		} else {	/* if(su->sg_of_su->su_oper_list.su == su) */

			/* The SU is not the same as the SU in the list.
			 * Send D2N-INFO_SU_SI_ASSIGN modify quiesced to the SU. Add the SU to
			 * the operation list and change state to SG_realign state.the SU is
			 * not the same as the SU in the list.
			 */

			old_state = su->list_of_susi->state;
			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->state = SA_AMF_HA_QUIESCED;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->state = old_state;
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				return NCSCC_RC_FAILURE;
			}

			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);

			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

		}		/* else (su->sg_of_su->su_oper_list.su == su) */

		break;		/* case AVD_SG_FSM_SU_OPER: */
	case AVD_SG_FSM_SI_OPER:

		if (su->sg_of_su->admin_si == su->list_of_susi->si) {
			/* The SI on this nodes SU is in the SI admin pointer. If this 
			 * SI admin is shutdown change to LOCK and send D2N-INFO_SU_SI_ASSIGN
			 * with quiesced to the SU. Remove the SI from SI admin pointer.
			 * Add the SU to operation list and change state to SU_operation. 
			 */
			if (su->sg_of_su->admin_si->saAmfSIAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				old_state = su->list_of_susi->state;
				old_fsm_state = su->list_of_susi->fsm;
				su->list_of_susi->state = SA_AMF_HA_QUIESCED;
				su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
					/* log a fatal error */
					m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
									 su->list_of_susi->si->name.length);
					su->list_of_susi->state = old_state;
					su->list_of_susi->fsm = old_fsm_state;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
					return NCSCC_RC_FAILURE;
				}
			}

			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SU_OPER);

			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
		} else {	/* if (su->sg_of_su->admin_si == su->list_of_susi->si) */

			/* Send D2N-INFO_SU_SI_ASSIGN with quiesced to the SU. Add the SU to
			 * the SU operation list. Change the state to SG_realign state.
			 */
			old_state = su->list_of_susi->state;
			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->state = SA_AMF_HA_QUIESCED;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->state = old_state;
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				return NCSCC_RC_FAILURE;
			}

			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);

			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

		}		/* else (su->sg_of_su->admin_si == su->list_of_susi->si) */

		break;		/* case AVD_SG_FSM_SI_OPER: */
	case AVD_SG_FSM_SG_ADMIN:

		if (su->sg_of_su->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED) {
			/* the SG is lock no action. */
			return NCSCC_RC_SUCCESS;
		} else if (su->sg_of_su->saAmfSGAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
			/* The SG is shutdown. if the SI relationships to the SU is quiescing.
			 * Send D2N-INFO_SU_SI_ASSIGN modify quiesced to the SU. No need to
			 * change admin to LOCK here itself. We can do it just before
			 * becoming stable.
			 */
			if ((su->list_of_susi->state == SA_AMF_HA_QUIESCING) &&
			    (su->list_of_susi->fsm == AVD_SU_SI_STATE_MODIFY)) {
				old_state = su->list_of_susi->state;
				su->list_of_susi->state = SA_AMF_HA_QUIESCED;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
					/* log a fatal error */
					m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
					m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
									 su->list_of_susi->si->name.length);
					su->list_of_susi->state = old_state;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
					return NCSCC_RC_FAILURE;
				}
			}
			/* if (su->list_of_susi->state == SA_AMF_HA_QUIESCING) */
		} else {	/* if (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN) */

			/* log fatal error */
			m_AVD_LOG_INVALID_VAL_ERROR(su->sg_of_su->saAmfSGAdminState);
			return NCSCC_RC_FAILURE;
		}

		break;		/* case AVD_SG_FSM_SG_ADMIN: */
	default:
		/* log fatal error about the invalid value */
		m_AVD_LOG_INVALID_VAL_FATAL(su->sg_of_su->sg_fsm_state);
		return NCSCC_RC_FAILURE;
		break;
	}			/* switch(su->sg_of_su->sg_fsm_state) */

	return NCSCC_RC_SUCCESS;
}

 /*****************************************************************************
 * Function: avd_sg_nored_su_insvc_func
 *
 * Purpose:  This function is called when a SU readiness state changes
 * to inservice from out of service. The SG is of type No redundancy
 * model. It will do the functionality specified in the SG FSM.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the service unit.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_su_insvc_func(AVD_CL_CB *cb, AVD_SU *su)
{

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_su_insvc_func");
	m_AVD_LOG_RCVD_VAL(((long)su));

	m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

	/* An SU will not become in service when the SG is being locked or shutdown.
	 */
	if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SG_ADMIN) {
		m_AVD_LOG_INVALID_VAL_FATAL(su->sg_of_su->sg_fsm_state);
		return NCSCC_RC_FAILURE;
	}

	/* If the SG FSM state is not stable just return success. */
	if (su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE) {
		return NCSCC_RC_SUCCESS;
	}

	if ((cb->init_state != AVD_APP_STATE) && (su->sg_of_su->sg_ncs_spec == SA_FALSE)) {
		return NCSCC_RC_SUCCESS;
	}

	if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) == NULL) {
		avd_sg_app_su_inst_func(cb, su->sg_of_su);

		/* all the assignments have already been done in the SG. */
		return NCSCC_RC_SUCCESS;
	}

	/* change the FSM state */
	m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
	m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
 * Function: avd_sg_nored_susi_sucss_func
 *
 * Purpose:  This function is called when a SU SI ack function is
 * received from the AVND with success value. The SG FSM for No redundancy
 * model will be run. The SUSI fsm state will
 * be changed to assigned or it will freed for the SU SI. 
 * 
 *
 * Input: cb - the AVD control block
 *        su - In case of entire SU related operation the SU for
 *               which the ack is received.
 *        susi - The pointer to the service unit service instance relationship.
 *        act  - The action received in the ack message.
 *        state - The HA state in the message.
 * 
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a No redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_susi_sucss_func(AVD_CL_CB *cb, AVD_SU *su, AVD_SU_SI_REL *susi,
				   AVSV_SUSI_ACT act, SaAmfHAStateT state)
{

	AVD_SI *l_si;
	NCS_BOOL flag;
	AVD_SU_SI_STATE old_fsm_state;
	AVD_AVND *su_node_ptr = NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_susi_sucss_func");
	m_AVD_LOG_RCVD_VAL(((long)su));
	m_AVD_LOG_RCVD_VAL(((long)susi));
	m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

	switch (su->sg_of_su->sg_fsm_state) {
	case AVD_SG_FSM_STABLE:
		/* Do the action specified in the message if delete else no action. */
		if (act == AVSV_SUSI_ACT_DEL) {
			/* free all the CSI assignments  */
			avd_compcsi_delete(cb, su->list_of_susi, FALSE);
			/* Unassign the SUSI */
			m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);
		}
		/* log informational error. */
		m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
		break;		/* case AVD_SG_FSM_STABLE: */
	case AVD_SG_FSM_SG_REALIGN:

		if (act == AVSV_SUSI_ACT_DEL) {
			/* Action is remove. Remove the SI relationship to this SU. If this SI  
			 * is in the SI admin pointer. Change SI admin to LOCK. Remove the SI
			 * from the SI admin pointer. If this SU is in the operation list,
			 * Remove the SU from the SU operation list. If the SU operation list 
			 * and the SI admin pointer are empty. Choose and assign 
			 * unassigned SIs to unassigned in-service SUs .
			 */

			l_si = su->list_of_susi->si;

			/* free all the CSI assignments  */
			avd_compcsi_delete(cb, su->list_of_susi, FALSE);
			/* Unassign the SUSI */
			m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

			if (su->sg_of_su->admin_si == l_si) {
				m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
			} else {
				avd_sg_su_oper_list_del(cb, su, FALSE);
			}

			if ((su->sg_of_su->admin_si == AVD_SI_NULL) && (su->sg_of_su->su_oper_list.su == NULL)) {
				if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) == NULL) {
					/* No New assignments are been done in the SG. change the FSM state */
					m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
					m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
					avd_sg_app_su_inst_func(cb, su->sg_of_su);
				}

			}

		} /* if (act == AVSV_SUSI_ACT_DEL) */
		else if (state == SA_AMF_HA_QUIESCED) {
			/* The state is quiesced. Send a D2N-INFO_SU_SI_ASSIGN with removal
			 * for this SU. If the SI is in the SI admin pointer.
			 * Change SI admin to LOCK. Remove the SI from the SI admin pointer. 
			 * Add the SU to the operation list. If the SU is in the operation list
			 * and SU admin state is shutdown, change SU admin to LOCK.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}

			m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);

			if (su->sg_of_su->admin_si == su->list_of_susi->si) {
				m_AVD_SET_SI_ADMIN(cb, (su->sg_of_su->admin_si), SA_AMF_ADMIN_LOCKED);
				m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
				avd_sg_su_oper_list_add(cb, su, FALSE);
			} else if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
			} else if (su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
				if (flag == TRUE) {
					node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
				}
			}
		} /* if (state == SA_AMF_HA_QUIESCED) */
		else if (state == SA_AMF_HA_ACTIVE) {
			/* the HA state is active. Remove the SU from the SU operation list.
			 * If the SU operation list and the SI admin pointer are empty,
			 * Choose and assign unassigned SIs to unassigned in-service SUs.
			 */

			avd_sg_su_oper_list_del(cb, su, FALSE);

			if (su->sg_of_su->su_oper_list.su == NULL) {
				if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) == NULL) {
					/* No New assignments are been done in the SG. change the FSM state */
					m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
					m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
					avd_sg_app_su_inst_func(cb, su->sg_of_su);
				}
			}
		}

		break;		/* case AVD_SG_FSM_SG_REALIGN: */
	case AVD_SG_FSM_SU_OPER:
		if (act == AVSV_SUSI_ACT_DEL) {
			/* log error. Action is remove. Remove the SI relationship to this SU. 
			 * If this SU is in the operation list,
			 * Remove the SU from the SU operation list. If the SU operation list 
			 * is empty. Choose and assign unassigned SIs to unassigned in-service SUs .
			 */

			m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
			m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
			m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
							 su->list_of_susi->si->name.length);

			/* free all the CSI assignments  */
			avd_compcsi_delete(cb, su->list_of_susi, FALSE);
			/* Unassign the SUSI */
			m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

			avd_sg_su_oper_list_del(cb, su, FALSE);

			if (su->sg_of_su->su_oper_list.su == NULL) {
				if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) != NULL) {
					/* New assignments are been done in the SG. */
					/* change the FSM state */
					m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
					m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
					return NCSCC_RC_SUCCESS;
				}

				/* change the FSM state */
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				avd_sg_app_su_inst_func(cb, su->sg_of_su);
			}

		} /* if (act == AVSV_SUSI_ACT_DEL) */
		else if (state == SA_AMF_HA_QUIESCED) {
			/* The HA state is quiesced and SU is in the operation list.
			 * If the admin state of the SU is shutdown change it to lock.
			 * Send a D2N-INFO_SU_SI_ASSIGN with remove to the SU. Change to 
			 * SG_realign state.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}

			if (su->sg_of_su->su_oper_list.su != su) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_FATAL(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);

				avd_sg_su_oper_list_add(cb, su, FALSE);
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				return NCSCC_RC_SUCCESS;
			}

			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

			m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);

			if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
			} else if (su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
				if (flag == TRUE) {
					node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
				}
			}
		}
		/* if (state == SA_AMF_HA_QUIESCED) */
		break;		/* case AVD_SG_FSM_SU_OPER: */
	case AVD_SG_FSM_SI_OPER:

		if (act == AVSV_SUSI_ACT_DEL) {
			/* log error. Action is remove. Remove the SI relationship to this SU. 
			 * If this SI is in the admin pointer,
			 * Remove the SI from the SI admin pointer. If the SI admin pointer 
			 * is empty. Choose and assign unassigned SIs to unassigned in-service SUs .
			 */

			m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
			m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
			m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
							 su->list_of_susi->si->name.length);

			l_si = su->list_of_susi->si;

			/* free all the CSI assignments  */
			avd_compcsi_delete(cb, su->list_of_susi, FALSE);
			/* Unassign the SUSI */
			m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

			if (su->sg_of_su->admin_si == l_si) {
				m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
			}

			if (su->sg_of_su->admin_si == AVD_SI_NULL) {
				if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) != NULL) {
					/* New assignments are been done in the SG. */
					/* change the FSM state */
					m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
					m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
					return NCSCC_RC_SUCCESS;
				}

				/* change the FSM state */
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				avd_sg_app_su_inst_func(cb, su->sg_of_su);
			}

		} /* if (act == AVSV_SUSI_ACT_DEL) */
		else if (state == SA_AMF_HA_QUIESCED) {
			/* message with modified quiesced for a SI in the admin pointer.
			 * If the SI admin state is shutdown change it to LOCK.
			 * Send a D2N-INFO_SU_SI_ASSIGN with remove for this SI to this SU. 
			 * Remove the SI from the admin pointer. Add the SU to operation list. 
			 * Change state to SG_realign.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}

			if (su->sg_of_su->admin_si != su->list_of_susi->si) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_FATAL(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);

				avd_sg_su_oper_list_add(cb, su, FALSE);
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				return NCSCC_RC_SUCCESS;
			}

			m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_SET_SI_ADMIN(cb, (su->list_of_susi->si), SA_AMF_ADMIN_LOCKED);
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

		}
		/* if (state == SA_AMF_HA_QUIESCED) */
		break;		/* case AVD_SG_FSM_SI_OPER: */
	case AVD_SG_FSM_SG_ADMIN:

		if (act == AVSV_SUSI_ACT_DEL) {
			/* The action is remove. Remove the SI relationship to this SU.
			 * Remove the SU from the SU operation list. If the SU operation list
			 * is empty. If this SG admin is shutdown change to LOCK. Change 
			 * the SG FSM state to stable.
			 */

			/* free all the CSI assignments  */
			avd_compcsi_delete(cb, su->list_of_susi, FALSE);
			/* Unassign the SUSI */
			m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

			avd_sg_su_oper_list_del(cb, su, FALSE);

			if (su->sg_of_su->su_oper_list.su == NULL) {
				m_AVD_SET_SG_ADMIN(cb, (su->sg_of_su), SA_AMF_ADMIN_LOCKED);

				/* change the FSM state */
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				avd_sg_app_su_inst_func(cb, su->sg_of_su);
			}

		} /* if (act == AVSV_SUSI_ACT_DEL) */
		else if (state == SA_AMF_HA_QUIESCED) {
			/* If HA state is quiesced. Send a D2N-INFO_SU_SI_ASSIGN with removal
			 * for this SU. Since when the removal is done for all the SUs
			 * we change the state to LOCK no need to scan the list to make the SG
			 * state to LOCK here.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}
		}
		/* if (state == SA_AMF_HA_QUIESCED) */
		break;		/* case AVD_SG_FSM_SG_ADMIN: */
	default:
		/* log fatal error about the invalid value */
		m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
		m_AVD_LOG_INVALID_VAL_FATAL(((uns32)su->sg_of_su->sg_fsm_state));
		m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
		return NCSCC_RC_FAILURE;
		break;
	}			/* switch(su->sg_of_su->sg_fsm_state) */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sg_nored_susi_fail_func
 *
 * Purpose:  This function is called when a SU SI ack function is
 * received from the AVND with some error value. The message may be an
 * ack for a particular SU SI or for the entire SU. It will log an event
 * about the failure. Since if a CSI set callback returns error it is
 * considered as failure of the component, AvND would have updated that
 * info for each of the components that failed and also for the SU an
 * operation state message would be sent the processing will be done in that
 * event context. For faulted SU this event would be considered as
 * completion of action, for healthy SU no SUSI state change will be done. 
 * 
 *
 * Input: cb - the AVD control block
 *        su - In case of entire SU related operation the SU for
 *               which the ack is received.
 *        susi - The pointer to the service unit service instance relationship.
 *        act  - The action received in the ack message.
 *        state - The HA state in the message.
 *        
 *
 * Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a No redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_susi_fail_func(AVD_CL_CB *cb, AVD_SU *su, AVD_SU_SI_REL *susi,
				  AVSV_SUSI_ACT act, SaAmfHAStateT state)
{
	AVD_SU_SI_STATE old_fsm_state;
	NCS_BOOL flag;
	AVD_AVND *su_node_ptr = NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_susi_fail_func");
	m_AVD_LOG_RCVD_VAL(((long)su));
	m_AVD_LOG_RCVD_VAL(((long)susi));
	m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

	switch (su->sg_of_su->sg_fsm_state) {
	case AVD_SG_FSM_STABLE:

		/* log fatal error. */
		m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
		break;		/* case AVD_SG_FSM_STABLE: */
	case AVD_SG_FSM_SG_REALIGN:

		if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING))) {
			/* The HA state is quiesced/quiescing. Send a 
			 * D2N-INFO_SU_SI_ASSIGN with removal for this SU. If SI in the 
			 * SI admin pointer, Change SI admin to LOCK. Remove the SI from
			 * the SI admin pointer. Add the SU to the operation list. 
			 * If the SU is in the operation list then. If SU admin state is 
			 * shutdown, change SU admin to LOCK.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}

			m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);

			if (su->sg_of_su->admin_si == su->list_of_susi->si) {
				m_AVD_SET_SI_ADMIN(cb, (su->sg_of_su->admin_si), SA_AMF_ADMIN_LOCKED);
				m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
				avd_sg_su_oper_list_add(cb, su, FALSE);
			} else if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
			} else if (su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				su->list_of_susi->state = SA_AMF_HA_QUIESCED;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
				if (flag == TRUE) {
					node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
				}
			}
		}		/* if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) ||
				   (state == SA_AMF_HA_QUIESCING))) */
		else {
			/* No action as other call back failure will cause operation disable 
			 * event to be sent by AvND.
			 */
			m_AVD_LOG_RCVD_VAL(((long)su));
		}

		break;		/* case AVD_SG_FSM_SG_REALIGN: */
	case AVD_SG_FSM_SU_OPER:

		if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING))) {
			/* If HA state is quiesced/quiescing and SU is in the operation list.
			 * If the admin state of the SU is shutdown change it to lock.
			 * Send a D2N-INFO_SU_SI_ASSIGN with remove to the SU. 
			 * Change to SG_realign state.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}

			m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);
			if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
			} else if (su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				su->list_of_susi->state = SA_AMF_HA_QUIESCED;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
				if (flag == TRUE) {
					node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
				}
			}
		}		/* if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) ||
				   (state == SA_AMF_HA_QUIESCING))) */
		else {
			/* No action as other call back failure will cause operation disable 
			 * event to be sent by AvND.
			 */
			m_AVD_LOG_RCVD_VAL(((long)su));
		}

		break;		/* case AVD_SG_FSM_SU_OPER: */
	case AVD_SG_FSM_SI_OPER:

		if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING))) {
			/* If message with modified quiesced/quiescing for a SI in the
			 * admin pointer. If the SI admin state is shutdown change it to LOCK.
			 * Send a D2N-INFO_SU_SI_ASSIGN with remove for this SI to this SU. 
			 * Remove the SI from the admin pointer. Add the SU to operation list. 
			 * Change state to SG_realign.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}

			if (su->sg_of_su->admin_si != su->list_of_susi->si) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_FATAL(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);

				avd_sg_su_oper_list_add(cb, su, FALSE);
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				return NCSCC_RC_SUCCESS;
			}

			m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_SET_SI_ADMIN(cb, (su->list_of_susi->si), SA_AMF_ADMIN_LOCKED);
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

		}
		/* if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) ||
		   (state == SA_AMF_HA_QUIESCING))) */
		break;		/* case AVD_SG_FSM_SI_OPER: */
	case AVD_SG_FSM_SG_ADMIN:

		if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING))) {
			/* If HA state is quiesced/quiescing. Send a D2N-INFO_SU_SI_ASSIGN 
			 * with removal for this SU. The SG admin state can be changed to LOCK
			 * when all the SUs are unassigned.
			 */

			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_UNASGN;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_DEL) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				return NCSCC_RC_FAILURE;
			}
		}
		/* if ((act == AVSV_SUSI_ACT_MOD) && ((state == SA_AMF_HA_QUIESCED) ||
		   (state == SA_AMF_HA_QUIESCING))) */
		break;		/* case AVD_SG_FSM_SG_ADMIN: */
	default:
		/* log fatal error about the invalid value */
		m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
		m_AVD_LOG_INVALID_VAL_FATAL(((uns32)su->sg_of_su->sg_fsm_state));
		m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
		return NCSCC_RC_FAILURE;
		break;
	}			/* switch(su->sg_of_su->sg_fsm_state) */

	return NCSCC_RC_SUCCESS;

}

 /*****************************************************************************
 * Function: avd_sg_nored_realign_func
 *
 * Purpose:  This function will call the chose assign function to check and
 * assign SIs. If any assigning is being done it adds the SUs to the operation
 * list and sets the SG FSM state to SG realign. It resets the ncsSGAdjustState.
 * If everything is 
 * fine, it calls the routine to bring the preffered number of SUs to 
 * inservice state and change the SG state to stable. The functionality is
 * described in the SG FSM. The same function is used for both cluster_timer and
 * and sg_operator events as described in the SG FSM.
 *
 * Input: cb - the AVD control block
 *        sg - The pointer to the service group.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_realign_func(AVD_CL_CB *cb, AVD_SG *sg)
{
	TRACE_ENTER2("'%s'", sg->name.value);

	/* If the SG FSM state is not stable just return success. */
	if ((cb->init_state != AVD_APP_STATE) && (sg->sg_ncs_spec == SA_FALSE)) {
		goto done;
	}

	if (sg->sg_fsm_state != AVD_SG_FSM_STABLE) {
		m_AVD_SET_SG_ADJUST(cb, sg, AVSV_SG_STABLE);
		avd_sg_app_su_inst_func(cb, sg);
		goto done;
	}

	if (avd_sg_nored_su_chose_asgn(cb, sg) == NULL) {
		/* all the assignments have already been done in the SG. */
		m_AVD_SET_SG_ADJUST(cb, sg, AVSV_SG_STABLE);
		avd_sg_app_su_inst_func(cb, sg);
		goto done;
	}

	/*  change the FSM state */

	m_AVD_SET_SG_ADJUST(cb, sg, AVSV_SG_STABLE);

	m_AVD_SET_SG_FSM(cb, sg, AVD_SG_FSM_SG_REALIGN);
	m_AVD_LOG_RCVD_VAL(sg->sg_fsm_state);

 done:
	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sg_nored_node_fail_func
 *
 * Purpose:  This function is called when the node has already failed and
 *           the SIs have to be failed over. It does the functionality 
 *           specified in the design.
 *
 * Input: cb - the AVD control block
 *        su - The SU that has faulted because of the node failure.
 *        
 *
 * Returns: None.
 *
 * NOTES: This is a No redundancy model specific function.
 *
 * 
 **************************************************************************/

void avd_sg_nored_node_fail_func(AVD_CL_CB *cb, AVD_SU *su)
{

	NCS_BOOL flag;
	AVD_SI *l_si;
	SaAmfHAStateT old_state;
	AVD_AVND *su_node_ptr = NULL;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_node_fail_func");
	m_AVD_LOG_RCVD_VAL(((long)su));

	m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

	if (su->list_of_susi == AVD_SU_SI_REL_NULL)
		return;

	switch (su->sg_of_su->sg_fsm_state) {
	case AVD_SG_FSM_STABLE:

		/* free all the CSI assignments  */
		avd_compcsi_delete(cb, su->list_of_susi, FALSE);
		/* Unassign the SUSI */
		m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

		if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) != NULL) {
			/* new assignments are been done in the SG. change the FSM state */
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

		}

		break;		/* case AVD_SG_FSM_STABLE: */
	case AVD_SG_FSM_SG_REALIGN:

		/* If the SU is same as the SU in the list,Remove the SI relationship
		 * to this SU. Remove the SU from operation list. If 
		 * the SUs SI assignment is the same as that in the SI admin pointer,
		 * Remove the SI relationship to this SU. Remove the SI from
		 * SI admin pointer. If the SI admin state is shutdown change to LOCK.
		 * If SU operation list and SI admin list is empty, Choose and assign
		 * unassigned SIs to unassigned in-service SUs. If the SU is not part of
		 * the SU operation list or the SUs SI is not part of the SI admin pointer.
		 * Remove the SI relationship to this SU.
		 */
		l_si = su->list_of_susi->si;
		old_state = su->list_of_susi->state;

		/* free all the CSI assignments  */
		avd_compcsi_delete(cb, su->list_of_susi, FALSE);
		/* Unassign the SUSI */
		m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

		if (su->sg_of_su->admin_si == l_si) {
			m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
			m_AVD_SET_SI_ADMIN(cb, l_si, SA_AMF_ADMIN_LOCKED);
		} else {

			avd_sg_su_oper_list_del(cb, su, FALSE);

			m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);

			if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
			} else if ((su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) &&
				   (old_state == SA_AMF_HA_QUIESCING)) {
				m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
				if (flag == TRUE) {
					node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
				}
			}
		}

		if ((su->sg_of_su->admin_si == AVD_SI_NULL) && (su->sg_of_su->su_oper_list.su == NULL)) {
			if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) == NULL) {
				/* No New assignments are been done in the SG. change the FSM state */
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				avd_sg_app_su_inst_func(cb, su->sg_of_su);
			}

		}
		break;		/* case AVD_SG_FSM_SG_REALIGN: */
	case AVD_SG_FSM_SU_OPER:

		/* If the SU is same as the SU in the list, Remove the SI relationship
		 * to this SU. If this SU admin is shutdown change to LOCK.
		 * Remove the SU from operation list. Choose and 
		 * assign unassigned SIs to unassigned in-service SUs. If the SU is not
		 * the same as the SU in the list, Remove the SI relationship to this SU.
		 * Stay in the same state.
		 */
		old_state = su->list_of_susi->state;

		/* free all the CSI assignments  */
		avd_compcsi_delete(cb, su->list_of_susi, FALSE);
		/* Unassign the SUSI */
		m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

		if (su->sg_of_su->su_oper_list.su == su) {
			avd_sg_su_oper_list_del(cb, su, FALSE);

			m_AVD_GET_SU_NODE_PTR(cb, su, su_node_ptr);

			if (su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
				avd_su_admin_state_set(su, SA_AMF_ADMIN_LOCKED);
			} else if ((su_node_ptr->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) &&
				   (old_state == SA_AMF_HA_QUIESCING)) {
				m_AVD_IS_NODE_LOCK((su_node_ptr), flag);
				if (flag == TRUE) {
					node_admin_state_set(su_node_ptr, SA_AMF_ADMIN_LOCKED);
				}
			}

			if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) != NULL) {
				/* New assignments are been done in the SG, change state to SG realign. */
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				return;
			}

			/* change the FSM state */
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
			avd_sg_app_su_inst_func(cb, su->sg_of_su);
		}

		break;		/* case AVD_SG_FSM_SU_OPER: */
	case AVD_SG_FSM_SI_OPER:

		/* If the SI on this nodes SU is in the SI admin pointer.  Remove the
		 * SI relationship to this SU. If this SI admin is shutdown change
		 * to LOCK. Remove the SI from SI admin pointer. Choose and assign
		 * unassigned SIs to unassigned in-service SUs. If not the same SI,
		 * Remove the SI relationship to this SU. Stay in the same state.
		 */

		l_si = su->list_of_susi->si;
		old_state = su->list_of_susi->state;

		/* free all the CSI assignments  */
		avd_compcsi_delete(cb, su->list_of_susi, FALSE);
		/* Unassign the SUSI */
		m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

		if (su->sg_of_su->admin_si == l_si) {
			m_AVD_CLEAR_SG_ADMIN_SI(cb, (su->sg_of_su));
			m_AVD_SET_SI_ADMIN(cb, l_si, SA_AMF_ADMIN_LOCKED);

			if (avd_sg_nored_su_chose_asgn(cb, su->sg_of_su) != NULL) {
				/* New assignments are been done in the SG, change state to SG realign. */
				m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SG_REALIGN);
				m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
				return;
			}

			/* change the FSM state */
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
			avd_sg_app_su_inst_func(cb, su->sg_of_su);
		}

		break;		/* case AVD_SG_FSM_SI_OPER: */
	case AVD_SG_FSM_SG_ADMIN:

		/* Remove the SI relationship to this SU. Remove the SU from
		 * the SU operation list. If the SU operation list is empty,
		 * If this SG admin is shutdown change to LOCK. Change the
		 * SG FSM state to stable.
		 */

		/* free all the CSI assignments  */
		avd_compcsi_delete(cb, su->list_of_susi, FALSE);
		/* Unassign the SUSI */
		m_AVD_SU_SI_TRG_DEL(cb, su->list_of_susi);

		avd_sg_su_oper_list_del(cb, su, FALSE);

		if (su->sg_of_su->su_oper_list.su == NULL) {
			m_AVD_SET_SG_ADMIN(cb, (su->sg_of_su), SA_AMF_ADMIN_LOCKED);

			/* change the FSM state */
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_STABLE);
			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
			avd_sg_app_su_inst_func(cb, su->sg_of_su);
		}

		break;		/* case AVD_SG_FSM_SG_ADMIN: */
	default:
		/* log fatal error about the invalid value */
		m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
		m_AVD_LOG_INVALID_VAL_FATAL(((uns32)su->sg_of_su->sg_fsm_state));
		m_AVD_LOG_INVALID_NAME_VAL_FATAL(su->name.value, su->name.length);
		return;
		break;
	}			/* switch(su->sg_of_su->sg_fsm_state) */

	return;
}

/*****************************************************************************
 * Function: avd_sg_nored_su_admin_fail
 *
 * Purpose:  This function is called when SU become OOS because of the
 * LOCK or shutdown of the SU or node.The functionality will be as described in
 * the SG design FSM. 
 *
 * Input: cb - the AVD control block
 *        su - The SU that has failed because of the admin operation.
 *        avnd - The AvND structure of the node that is being operated upon.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a No redundancy model specific function. The avnd pointer
 * value is valid only if this is a SU operation being done because of the node
 * admin change.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_su_admin_fail(AVD_CL_CB *cb, AVD_SU *su, AVD_AVND *avnd)
{

	SaAmfHAStateT old_state;
	AVD_SU_SI_STATE old_fsm_state;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_su_admin_fail");
	m_AVD_LOG_RCVD_VAL(((long)su));
	m_AVD_LOG_RCVD_VAL(((long)avnd));

	m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
	if ((cb->init_state != AVD_APP_STATE) && (su->sg_of_su->sg_ncs_spec == SA_FALSE)) {
		return NCSCC_RC_FAILURE;
	}

	switch (su->sg_of_su->sg_fsm_state) {
	case AVD_SG_FSM_STABLE:
		if ((su->saAmfSUAdminState == SA_AMF_ADMIN_LOCKED) ||
		    ((avnd != NULL) && (avnd->saAmfNodeAdminState == SA_AMF_ADMIN_LOCKED))) {

			/* change the  assignment to quiesced. */
			old_state = su->list_of_susi->state;
			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->state = SA_AMF_HA_QUIESCED;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->state = old_state;
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				return NCSCC_RC_FAILURE;
			}

			/* add the SU to the operation list and change the SG FSM to SU operation. */
			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SU_OPER);

			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);

		}		/* if ((su->admin_state == NCS_ADMIN_STATE_LOCK) ||
				   ((avnd != AVD_AVND_NULL) && (avnd->su_admin_state == NCS_ADMIN_STATE_LOCK))) */
		else if ((su->saAmfSUAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) ||
			 ((avnd != NULL) && (avnd->saAmfNodeAdminState == SA_AMF_ADMIN_SHUTTING_DOWN))) {
			/* change the  assignment to quiesced. */
			old_state = su->list_of_susi->state;
			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->state = SA_AMF_HA_QUIESCING;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->state = old_state;
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				return NCSCC_RC_FAILURE;
			}

			/* add the SU to the operation list and change the SG FSM to SU operation. */
			avd_sg_su_oper_list_add(cb, su, FALSE);
			m_AVD_SET_SG_FSM(cb, (su->sg_of_su), AVD_SG_FSM_SU_OPER);

			m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
		}		/* if ((su->admin_state == NCS_ADMIN_STATE_SHUTDOWN) ||
				   ((avnd != AVD_AVND_NULL) && (avnd->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN))) */
		break;		/* case AVD_SG_FSM_STABLE: */
	case AVD_SG_FSM_SU_OPER:
		if ((su->sg_of_su->su_oper_list.su == su) &&
		    (su->list_of_susi->state == SA_AMF_HA_QUIESCING) &&
		    ((su->saAmfSUAdminState == SA_AMF_ADMIN_LOCKED) ||
		     ((avnd != NULL) && (avnd->saAmfNodeAdminState == SA_AMF_ADMIN_LOCKED)))) {
			/* If the SU is in the operation list and the SU admin state is lock.
			 * send D2N-INFO_SU_SI_ASSIGN modify quiesced message to the SU. 
			 */
			/* change the  assignment to quiesced. */
			old_state = su->list_of_susi->state;
			old_fsm_state = su->list_of_susi->fsm;
			su->list_of_susi->state = SA_AMF_HA_QUIESCED;
			su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
			if (avd_snd_susi_msg(cb, su, su->list_of_susi, AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)su->list_of_susi));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->list_of_susi->si->name.value,
								 su->list_of_susi->si->name.length);
				su->list_of_susi->state = old_state;
				su->list_of_susi->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (su->list_of_susi), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, su->list_of_susi);
				return NCSCC_RC_FAILURE;
			}
		}
		break;		/* case AVD_SG_FSM_SU_OPER: */
	default:
		/* Log a error */
		m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
		m_AVD_LOG_INVALID_VAL_ERROR(((uns32)su->sg_of_su->sg_fsm_state));
		m_AVD_LOG_INVALID_NAME_VAL_ERROR(su->name.value, su->name.length);
		return NCSCC_RC_FAILURE;
		break;
	}			/* switch (su->sg_of_su->sg_fsm_state) */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sg_nored_si_admin_down
 *
 * Purpose:  This function is called when SIs admin state is changed to
 * LOCK or shutdown. The functionality will be as described in
 * the SG design FSM. 
 *
 * Input: cb - the AVD control block
 *        si - The SI pointer.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a No redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_si_admin_down(AVD_CL_CB *cb, AVD_SI *si)
{
	AVD_SU_SI_STATE old_fsm_state;
	SaAmfHAStateT old_state;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_si_admin_down");
	m_AVD_LOG_RCVD_VAL(((long)si));

	m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);

	if ((cb->init_state != AVD_APP_STATE) && (si->sg_of_si->sg_ncs_spec == SA_FALSE)) {
		return NCSCC_RC_FAILURE;
	}
	if (si->list_of_sisu == AVD_SU_SI_REL_NULL) {
		return NCSCC_RC_SUCCESS;
	}
	switch (si->sg_of_si->sg_fsm_state) {
	case AVD_SG_FSM_STABLE:
		if (si->saAmfSIAdminState == SA_AMF_ADMIN_LOCKED) {
			/* SI lock. Send D2N-INFO_SU_SI_ASSIGN modify quiesced for this SI. 
			 * Change state to SI_operation state. Add it to admin SI pointer.
			 */
			/* change the  assignment to quiesced. */
			old_state = si->list_of_sisu->state;
			old_fsm_state = si->list_of_sisu->fsm;
			si->list_of_sisu->state = SA_AMF_HA_QUIESCED;
			si->list_of_sisu->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (si->list_of_sisu), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, si->list_of_sisu);
			if (avd_snd_susi_msg(cb, si->list_of_sisu->su, si->list_of_sisu, AVSV_SUSI_ACT_MOD) ==
			    NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)si->list_of_sisu));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(si->list_of_sisu->su->name.value,
								 si->list_of_sisu->su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(si->name.value, si->name.length);
				si->list_of_sisu->state = old_state;
				si->list_of_sisu->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (si->list_of_sisu), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, si->list_of_sisu);
				return NCSCC_RC_FAILURE;
			}

			/* add the SI to the admin list and change the SG FSM to SI operation. */
			m_AVD_SET_SG_ADMIN_SI(cb, si);
			m_AVD_SET_SG_FSM(cb, (si->sg_of_si), AVD_SG_FSM_SI_OPER);
			m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
		} /* if (si->admin_state == NCS_ADMIN_STATE_LOCK) */
		else if (si->saAmfSIAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
			/* SI shutdown. Send D2N-INFO_SU_SI_ASSIGN modify quiescing for this SI.
			 * Change state to SI_operation state. Add it to admin SI pointer.
			 */

			/* change the state of the SI to quiescing */

			old_state = si->list_of_sisu->state;
			old_fsm_state = si->list_of_sisu->fsm;
			si->list_of_sisu->state = SA_AMF_HA_QUIESCING;
			si->list_of_sisu->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (si->list_of_sisu), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, si->list_of_sisu);
			if (avd_snd_susi_msg(cb, si->list_of_sisu->su, si->list_of_sisu, AVSV_SUSI_ACT_MOD) ==
			    NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)si->list_of_sisu));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(si->list_of_sisu->su->name.value,
								 si->list_of_sisu->su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(si->name.value, si->name.length);
				si->list_of_sisu->state = old_state;
				si->list_of_sisu->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (si->list_of_sisu), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, si->list_of_sisu);
				return NCSCC_RC_FAILURE;
			}

			/* add the SI to the admin list and change the SG FSM to SI operation. */
			m_AVD_SET_SG_ADMIN_SI(cb, si);
			m_AVD_SET_SG_FSM(cb, (si->sg_of_si), AVD_SG_FSM_SI_OPER);
			m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
		}		/* if (si->admin_state == NCS_ADMIN_STATE_SHUTDOWN) */
		break;		/* case AVD_SG_FSM_STABLE: */
	case AVD_SG_FSM_SI_OPER:
		if ((si->sg_of_si->admin_si == si) && (si->saAmfSIAdminState == SA_AMF_ADMIN_LOCKED)) {
			/* If the SI is in the admin pointer and the SI admin state is shutdown,
			 * change the admin state of the SI to lock and 
			 * send D2N-INFO_SU_SI_ASSIGN modify quiesced message to the SU that
			 * is being assigned quiescing state.
			 */

			/* change the state of the SI to quiesced */

			old_state = si->list_of_sisu->state;
			old_fsm_state = si->list_of_sisu->fsm;
			si->list_of_sisu->state = SA_AMF_HA_QUIESCED;
			si->list_of_sisu->fsm = AVD_SU_SI_STATE_MODIFY;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (si->list_of_sisu), AVSV_CKPT_AVD_SI_ASS);
			avd_gen_su_ha_state_changed_ntf(cb, si->list_of_sisu);
			if (avd_snd_susi_msg(cb, si->list_of_sisu->su, si->list_of_sisu, AVSV_SUSI_ACT_MOD) ==
			    NCSCC_RC_FAILURE) {
				/* log a fatal error */
				m_AVD_LOG_INVALID_VAL_ERROR(((long)si->list_of_sisu));
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(si->list_of_sisu->su->name.value,
								 si->list_of_sisu->su->name.length);
				m_AVD_LOG_INVALID_NAME_VAL_ERROR(si->name.value, si->name.length);
				si->list_of_sisu->state = old_state;
				si->list_of_sisu->fsm = old_fsm_state;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (si->list_of_sisu), AVSV_CKPT_AVD_SI_ASS);
				avd_gen_su_ha_state_changed_ntf(cb, si->list_of_sisu);
				return NCSCC_RC_FAILURE;
			}
		}
		break;		/* case AVD_SG_FSM_SI_OPER: */
	default:
		/* Log a error */
		m_AVD_LOG_INVALID_VAL_ERROR(((long)si));
		m_AVD_LOG_INVALID_VAL_ERROR(((uns32)si->sg_of_si->sg_fsm_state));
		m_AVD_LOG_INVALID_NAME_VAL_ERROR(si->name.value, si->name.length);
		return NCSCC_RC_FAILURE;
		break;
	}			/* switch (si->sg_of_si->sg_fsm_state) */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sg_nored_sg_admin_down
 *
 * Purpose:  This function is called when SGs admin state is changed to
 * LOCK or shutdown. The functionality will be as described in
 * the SG design FSM. 
 *
 * Input: cb - the AVD control block
 *        sg - The SG pointer.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a No redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_nored_sg_admin_down(AVD_CL_CB *cb, AVD_SG *sg)
{
	AVD_SU *i_su;
	AVD_SG_OPER *l_suopr;

	m_AVD_LOG_FUNC_ENTRY("avd_sg_nored_sg_admin_down");
	m_AVD_LOG_RCVD_VAL(((long)sg));

	m_AVD_LOG_RCVD_VAL(sg->sg_fsm_state);

	if ((cb->init_state != AVD_APP_STATE) && (sg->sg_ncs_spec == SA_FALSE)) {
		return NCSCC_RC_FAILURE;
	}

	switch (sg->sg_fsm_state) {
	case AVD_SG_FSM_STABLE:

		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED) {
			/* SG lock. Identify all the assigned SUs, send D2N-INFO_SU_SI_ASSIGN 
			 * modify quiesced for each of the SU w.r.t the corresponding SI. 
			 * Add them to the SU operation list. Change state to SG_admin. 
			 * If no assigned SU exist, no action, stay in stable state.
			 */

			i_su = sg->list_of_su;
			while (i_su != NULL) {
				if (i_su->list_of_susi != AVD_SU_SI_REL_NULL) {
					i_su->list_of_susi->state = SA_AMF_HA_QUIESCED;
					i_su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (i_su->list_of_susi),
									 AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, i_su->list_of_susi);
					avd_snd_susi_msg(cb, i_su, i_su->list_of_susi, AVSV_SUSI_ACT_MOD);

					/* add the SU to the operation list */
					avd_sg_su_oper_list_add(cb, i_su, FALSE);
				}

				i_su = i_su->sg_list_su_next;
			}

		} /* if (sg->admin_state == NCS_ADMIN_STATE_LOCK) */
		else if (sg->saAmfSGAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
			/* SG shutdown. Identify all the assigned SUs, 
			 * send D2N-INFO_SU_SI_ASSIGN modify quiescing for each of the SU w.r.t
			 * the corresponding SI. Add the SUs to the SU operation list. 
			 * Change state to SG_admin. If no assigned SU exist, change the 
			 * SG admin state to LOCK, stay in stable state.
			 */
			i_su = sg->list_of_su;
			while (i_su != NULL) {
				if (i_su->list_of_susi != AVD_SU_SI_REL_NULL) {
					i_su->list_of_susi->state = SA_AMF_HA_QUIESCING;
					i_su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (i_su->list_of_susi),
									 AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, i_su->list_of_susi);
					avd_snd_susi_msg(cb, i_su, i_su->list_of_susi, AVSV_SUSI_ACT_MOD);

					/* add the SU to the operation list */
					avd_sg_su_oper_list_add(cb, i_su, FALSE);
				}

				i_su = i_su->sg_list_su_next;
			}

		} /* if (sg->admin_state == NCS_ADMIN_STATE_SHUTDOWN) */
		else {
			return NCSCC_RC_FAILURE;
		}

		if (sg->su_oper_list.su != NULL) {
			m_AVD_SET_SG_FSM(cb, sg, AVD_SG_FSM_SG_ADMIN);
			m_AVD_LOG_RCVD_VAL(sg->sg_fsm_state);
		}

		break;		/* case AVD_SG_FSM_STABLE: */
	case AVD_SG_FSM_SG_ADMIN:
		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED) {
			/* If the SG admin state is shutdown, change the admin state of the 
			 * SG to lock and send D2N-INFO_SU_SI_ASSIGN modify quiesced message
			 * to all the SUs in the SU operation list with quiescing assignment.
			 */
			if (sg->su_oper_list.su != NULL) {
				i_su = sg->su_oper_list.su;
				if ((i_su->list_of_susi->state == SA_AMF_HA_QUIESCING) &&
				    (i_su->list_of_susi->fsm == AVD_SU_SI_STATE_MODIFY)) {
					i_su->list_of_susi->state = SA_AMF_HA_QUIESCED;
					m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (i_su->list_of_susi),
									 AVSV_CKPT_AVD_SI_ASS);
					avd_gen_su_ha_state_changed_ntf(cb, i_su->list_of_susi);
					avd_snd_susi_msg(cb, i_su, i_su->list_of_susi, AVSV_SUSI_ACT_MOD);
				}

				l_suopr = i_su->sg_of_su->su_oper_list.next;
				while (l_suopr != NULL) {
					if ((l_suopr->su->list_of_susi->state == SA_AMF_HA_QUIESCING) &&
					    (l_suopr->su->list_of_susi->fsm == AVD_SU_SI_STATE_MODIFY)) {
						l_suopr->su->list_of_susi->state = SA_AMF_HA_QUIESCED;
						m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (l_suopr->su->list_of_susi),
										 AVSV_CKPT_AVD_SI_ASS);
						avd_gen_su_ha_state_changed_ntf(cb, l_suopr->su->list_of_susi);
						avd_snd_susi_msg(cb, l_suopr->su, l_suopr->su->list_of_susi,
								 AVSV_SUSI_ACT_MOD);
					}

					l_suopr = l_suopr->next;
				}
			}

		}		/* if (sg->admin_state == NCS_ADMIN_STATE_LOCK) */
		break;		/* case AVD_SG_FSM_SG_ADMIN: */
	default:
		/* Log a error */
		m_AVD_LOG_INVALID_VAL_FATAL(((long)sg));
		m_AVD_LOG_INVALID_VAL_FATAL(((uns32)sg->sg_fsm_state));
		m_AVD_LOG_INVALID_NAME_VAL_FATAL(sg->name.value, sg->name.length);
		return NCSCC_RC_FAILURE;
		break;
	}			/* switch (sg->sg_fsm_state) */

	return NCSCC_RC_SUCCESS;
}
