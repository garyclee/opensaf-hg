/*      -*- OpenSAF  -*-
*
* (C) Copyright 2010 The OpenSAF Foundation
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
*                                                                            *
*  MODULE NAME:  plms_plmc.c                                                 *
*                                                                            *
*                                                                            *
*  DESCRIPTION: Processing PLMC events.				  	     *
*                                                                            *
*****************************************************************************/

#include "plms.h"
#include "plms_utils.h"
#include "plmc_lib.h"
#include "plmc_lib_internal.h"
#include "plmc.h"

static int32  plms_plmc_tcp_cbk(tcp_msg *);
static SaUint32T plms_ee_verify(PLMS_ENTITY *, PLMS_PLMC_EE_OS_INFO *);
static SaUint32T plms_os_info_parse(SaInt8T *, PLMS_PLMC_EE_OS_INFO *);
static SaUint32T plms_plmc_unlck_insvc(PLMS_ENTITY *, PLMS_TRACK_INFO *);
void plms_os_info_free(PLMS_PLMC_EE_OS_INFO *);

static SaUint32T plms_ee_terminated_mngt_flag_clear(PLMS_ENTITY *);
static SaUint32T plms_tcp_discon_mngt_flag_clear(PLMS_ENTITY *);
static SaUint32T plms_tcp_connect_mngt_flag_clear(PLMS_ENTITY *);
static SaUint32T plms_restart_resp_mngt_flag_clear(PLMS_ENTITY *);
static SaUint32T plms_lck_resp_mngt_flag_clear(PLMS_ENTITY *);
static SaUint32T plms_unlck_resp_mngt_flag_clear(PLMS_ENTITY *);
static SaUint32T plms_lckinst_resp_mngt_flag_clear(PLMS_ENTITY *);
static SaUint32T plms_os_info_resp_mngt_flag_clear(PLMS_ENTITY *);
/******************************************************************************
Name            :
Description     : Process instantiating event from PLMC.
		  1. Do the OS verification irrespective of previous state.
		  2. If successful then, mark the presence state to 
		  instantiating.
		  3. Start instantiating timer if not running. 
Arguments       : 
                  
Return Values   : 
Notes           : TODO: If the EE is not in OOS, then should we take care 
		of moving the EE to OOS. But will I ever get instantiating
		for an EE which is not in OOS?
		
		instantiating->instantiated->tcp_connection.
******************************************************************************/
SaUint32T plms_plmc_instantiating_process(PLMS_ENTITY *ent,
					PLMS_PLMC_EE_OS_INFO *os_info)
{
	SaUint32T ret_err;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	/* If the admin state of the ent is lckinst, then ignore the 
	state as we cannot terminate the entity at this point of time.*/
	if (SA_PLM_EE_ADMIN_LOCKED_INSTANTIATION == 
		ent->entity.ee_entity.saPlmEEAdminState){
		
		LOG_ER("EE insting ignored, as the admin state is lckinst.%s",
		ent->dn_name_str);
		TRACE_LEAVE2("Return Val: %d",NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE; 	
	}
	
	/* Verify the EE Os.*/
	ret_err = plms_ee_verify(ent,os_info);
	plms_os_info_free(os_info);
	/* If verification failed then ignore the EE.*/
	if (NCSCC_RC_SUCCESS != ret_err){
		LOG_ER("EE Verification FAILED for ent: %s",ent->dn_name_str);	
		return ret_err;	
	}
	
	TRACE("EE verification is SUCCESSFUL for ent: %s",ent->dn_name_str);
	/* Verification is successful. Update the presence state.*/
	if ( SA_PLM_EE_PRESENCE_INSTANTIATING != 
		ent->entity.ee_entity.saPlmEEPresenceState){
		plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_INSTANTIATING,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);
	}
	
	/* Start istantiation-failed timer if it is not in progress.*/
	if (!(ent->tmr.timer_id)){
		ent->tmr.tmr_type = PLMS_TMR_EE_INSTANTIATING;
		ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
		ent->entity.ee_entity.saPlmEEInstantiateTimeout);
		if (NCSCC_RC_SUCCESS != ret_err){
			ent->tmr.timer_id = 0;
			ent->tmr.tmr_type = PLMS_TMR_NONE;
			ent->tmr.context_info = NULL;
		}
	}else if (PLMS_TMR_EE_TERMINATING == ent->tmr.tmr_type){
		/* Stop the term timer and start the inst timer.*/
		plms_timer_stop(ent);
		ent->tmr.tmr_type = PLMS_TMR_EE_INSTANTIATING;
		ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
		ent->entity.ee_entity.saPlmEEInstantiateTimeout);
		if (NCSCC_RC_SUCCESS != ret_err){
			ent->tmr.timer_id = 0;
			ent->tmr.tmr_type = PLMS_TMR_NONE;
			ent->tmr.context_info = NULL;
		}
	} else {
		/* The timer is already running.*/
	}

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err; 	
}	

/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : intantiating->instantiated->tcp_connect. So if the EE is in
		unlocked state, then we should start the required services only
		after the TCP connection is established. So this PLMC event is
		dummy for PLMS.
******************************************************************************/
SaUint32T plms_plmc_instantiated_process(PLMS_ENTITY *ent,
				PLMS_PLMC_EE_OS_INFO *os_info)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	
	/* If the admin state of the ent is lckinst, then ignore the 
	state as we cannot terminate the entity at this point of time.*/
	if (SA_PLM_EE_ADMIN_LOCKED_INSTANTIATION == 
		ent->entity.ee_entity.saPlmEEAdminState){
		
		LOG_ER("EE insting ignored, as the admin state is lckinst.%s",
		ent->dn_name_str);
		TRACE_LEAVE2("Return Val: %d",NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE; 	
	}

	/* Verify the EE if the presence state is not instantiating.*/
	if ( (SA_PLM_EE_PRESENCE_INSTANTIATING != 
		ent->entity.ee_entity.saPlmEEPresenceState) &&
		(SA_PLM_EE_PRESENCE_INSTANTIATED != 
		ent->entity.ee_entity.saPlmEEPresenceState)){

		TRACE("EE is not yet verified, verify the ent: %s",
		ent->dn_name_str);
		/* Verify the EE Os.*/
		ret_err = plms_ee_verify(ent,os_info);
		plms_os_info_free(os_info);
		/* If verification failed then ignore the EE.*/
		if (NCSCC_RC_SUCCESS != ret_err){
			LOG_ER("EE Verification FAILED for ent: %s",
							ent->dn_name_str);	
			return ret_err;	
		}
		TRACE("EE verification is SUCCESSFUL for ent: %s",
		ent->dn_name_str);
	}else{
		TRACE("EE is already verified. Ent: %s",ent->dn_name_str);
	}
	
	/* Verification is successful. Update the presence state.*/
	plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_INSTANTIATED,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/*Although this is the instantiated message, but we are not
	depending upon it. We are stopping the instantiation-failed timer
	only after getting the TCP connection.*/

	/*TODO: If the entity is in disable state and the isolation reason is
	terminated, then should I take the entity to enable state?*/

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
		
******************************************************************************/
SaUint32T plms_plmc_terminating_process(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	
	if ( SA_PLM_EE_PRESENCE_UNINSTANTIATED == 
		ent->entity.ee_entity.saPlmEEPresenceState){
		
		TRACE("Ent %s is already in uninstantiated state.",
		ent->dn_name_str);
		
		TRACE_LEAVE2("Return Val: %d",ret_err);
		return ret_err;
	}
	
	if ( SA_PLM_EE_PRESENCE_TERMINATING != 
		ent->entity.ee_entity.saPlmEEPresenceState){		
		plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_TERMINATING,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);
	}
	
	/* Start the termination-failed timer if not started.*/
	if (!(ent->tmr.timer_id)){
		ent->tmr.tmr_type = PLMS_TMR_EE_TERMINATING;
		ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
		ent->entity.ee_entity.saPlmEETerminateTimeout);
		if (NCSCC_RC_SUCCESS != ret_err){
			ent->tmr.timer_id = 0;
			ent->tmr.tmr_type = PLMS_TMR_NONE;
			ent->tmr.context_info = NULL;
		}
	}else if (PLMS_TMR_EE_INSTANTIATING == ent->tmr.tmr_type){
		/* Stop the inst timer and start the term timer.*/
		plms_timer_stop(ent);
		ent->tmr.tmr_type = PLMS_TMR_EE_TERMINATING;
		ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
		ent->entity.ee_entity.saPlmEETerminateTimeout);
		if (NCSCC_RC_SUCCESS != ret_err){
			ent->tmr.timer_id = 0;
			ent->tmr.tmr_type = PLMS_TMR_NONE;
			ent->tmr.context_info = NULL;
		}
	} else {
		/* The timer is already running.*/
	}

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : I can not depend on this PLMC message as for abrupt
		termination, I will not get this. Anyway this is UDP.
		So do the corresponding processing on getting tcp_term
		message for PLMC.
		
******************************************************************************/
SaUint32T plms_plmc_terminated_process(PLMS_ENTITY *ent)
{
	SaUint8T ret_err = NCSCC_RC_SUCCESS;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	
	if (SA_PLM_EE_PRESENCE_UNINSTANTIATED != 
		ent->entity.ee_entity.saPlmEEPresenceState){
		plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_UNINSTANTIATED,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);
	}
	
	/* Stop the termination-failed timer if running.*/
	if (ent->tmr.timer_id && 
		(PLMS_TMR_EE_TERMINATING == ent->tmr.tmr_type)) {
		plms_timer_stop(ent);
	}
	
	/* Take care of clearing management lost flag if required.*/
	if (plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST))
		plms_ee_terminated_mngt_flag_clear(ent);

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err; 
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : instantiating->instantiated->tcp_connect 
******************************************************************************/
SaUint32T plms_plmc_tcp_connect_process(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	SaUint32T prev_adm_op = 0;
	PLMS_TRACK_INFO *trk_info = NULL;
	PLMS_CB *cb = plms_cb; 

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	/* Stop the instantiate-failed timer if running.*/
	if (ent->tmr.timer_id && 
		(PLMS_TMR_EE_INSTANTIATING == ent->tmr.tmr_type)) {
		plms_timer_stop(ent);
	}
	
	/* Clear management lost flag.*/
	if(plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){ 
		if( NCSCC_RC_SUCCESS != plms_tcp_connect_mngt_flag_clear(ent))
			return NCSCC_RC_FAILURE;
	}
	
	/* If the admin state of the ent is lckinst, terminate the EE. 
	OH, if my parent is in OOS, then also terminate the EE.*/ 
	if ( (SA_PLM_EE_ADMIN_LOCKED_INSTANTIATION == ent->entity.ee_entity.saPlmEEAdminState) || 
	((NULL != ent->parent) && (plms_is_rdness_state_set(ent->parent,SA_PLM_READINESS_OUT_OF_SERVICE)))){
		
		LOG_ER("Term the entity, as the admin state is lckinst or parent is in OOS. Ent: %s",
		ent->dn_name_str);
		ret_err = plmc_sa_plm_admin_lock_instantiation(ent->dn_name_str,
		plms_plmc_tcp_cbk);
		
		if(ret_err){
			if (!plms_rdness_flag_is_set(ent,
				SA_PLM_RF_MANAGEMENT_LOST)){
				
				/* Set management lost flag.*/
				plms_readiness_flag_mark_unmark(ent,
				SA_PLM_RF_MANAGEMENT_LOST,1/*mark*/,
				NULL,SA_NTF_MANAGEMENT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

				/* Call managemnet lost callback.*/
				plms_mngt_lost_clear_cbk_call(ent,1/*mark*/);
			}
			ent->mngt_lost_tri = PLMS_MNGT_EE_TERM;
		}
		TRACE_LEAVE2("Return Val: %d",NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE; 	
	}

	if (plms_is_rdness_state_set(ent,SA_PLM_READINESS_IN_SERVICE)){
		TRACE("Ent %s is already in insvc.",ent->dn_name_str);
		return NCSCC_RC_SUCCESS;
	}
	/*If previous state is not instantiating/intantiated, then get os info
	and verify. If verification failed, then return.*/
	if ((SA_PLM_EE_PRESENCE_INSTANTIATED != 
		ent->entity.ee_entity.saPlmEEPresenceState) && 
		(SA_PLM_EE_PRESENCE_INSTANTIATING != 
		ent->entity.ee_entity.saPlmEEPresenceState)){

		TRACE("Get the OS info to verify the entity: %s",
							ent->dn_name_str);
		
#if 0		
		/* I have missed instantiated as well as intantiating
		but as tcp connection is established, the EE must be in
		instantiated state.*/
		plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_INSTANTIATED,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);
#endif			
		ret_err = plmc_get_os_info(ent->dn_name_str,plms_plmc_tcp_cbk);
		if (ret_err){
			LOG_ER("Request to get OS info FAILED for ent: %s",
							ent->dn_name_str);
			plms_readiness_flag_mark_unmark(ent,
			SA_PLM_RF_MANAGEMENT_LOST,1/*mark*/,NULL,
			SA_NTF_OBJECT_OPERATION,
			SA_PLM_NTFID_STATE_CHANGE_ROOT);

			ent->mngt_lost_tri = PLMS_MNGT_EE_GET_OS_INFO;
			plms_mngt_lost_clear_cbk_call(ent,1/*mark*/);

			TRACE_LEAVE2("Return Val: %d",NCSCC_RC_FAILURE);
			return NCSCC_RC_FAILURE;

		}
		/* On getting the OS info, verify the EE. If the verification
		is successful, start the services if the EE is in unlocked
		state and make an attempt to take the entity to insvc.*/
		TRACE_LEAVE2("Return Val: %d",ret_err);
		return ret_err;
	}
	
	/* I missed INSTANTIATED.*/
	if (SA_PLM_EE_PRESENCE_INSTANTIATED != 
		ent->entity.ee_entity.saPlmEEPresenceState){
		
		TRACE("PLMS missed instantiated for the entity. Set the \
		Presence state to instantiated for ent: %s",ent->dn_name_str);
		plms_presence_state_set(ent,
					SA_PLM_EE_PRESENCE_INSTANTIATED,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
	}

	/* Handle the admin operation for which this ent is restarted.*/
	if (NULL != ent->trk_info /* && ent->am_i_aff_ent*/){
		
		prev_adm_op = 1;
		trk_info = ent->trk_info;
		ent->trk_info = NULL;
		trk_info->track_count--;
		
		TRACE("Perform the in progress admin op for entity: %s,\
		adm-op: %d",ent->dn_name_str,
		trk_info->root_entity->adm_op_in_progress);
		
		if (!(trk_info->track_count)){
			/* Clean up the trk_info.*/
			trk_info->root_entity->adm_op_in_progress = FALSE;
			trk_info->root_entity->am_i_aff_ent = FALSE;
			plms_aff_ent_flag_mark_unmark(
					trk_info->aff_ent_list,FALSE);
			
			plms_ent_list_free(trk_info->aff_ent_list);	
			trk_info->aff_ent_list = NULL;
			
			/* Send response to IMM.
			TODO: Am I sending this a bit earlier?
			*/
			ret_err = saImmOiAdminOperationResult( cb->oi_hdl,
					trk_info->inv_id, SA_AIS_OK);
			if (NCSCC_RC_SUCCESS != ret_err){
				LOG_ER("Adm operation result sending to IMM \
				failed for ent: %s,imm-ret-val: %d",
				trk_info->root_entity->dn_name_str,ret_err);
			}
			TRACE("Adm operation result sending to IMM successful \
			for ent: %s",trk_info->root_entity->dn_name_str);
		}
	}
		
	if (SA_PLM_EE_ADMIN_UNLOCKED != 
		ent->entity.ee_entity.saPlmEEAdminState){
	
		TRACE("Admin state of the entity %s is not unlocked, not \
		starting the required services.",ent->dn_name_str);
		/* As the admin state is not unlocked, this entity
		can not be moved to insvc. Return.*/
		
		if ( prev_adm_op && !(trk_info->track_count))
			plms_trk_info_free(trk_info);

		TRACE_LEAVE2("Return Val: %d",ret_err);
		return NCSCC_RC_SUCCESS;
	}else{
		ret_err = plms_ee_unlock(ent,FALSE,TRUE/*mngt_cbk*/);
		if (NCSCC_RC_SUCCESS != ret_err){
			LOG_ER("Request to unlock the entity %s FAILED",
							ent->dn_name_str);
			TRACE_LEAVE2("Return Val: %d",ret_err);
			return ret_err;
		}
		TRACE("Request to unlock the EE: %s SUCCESSFUL",
							ent->dn_name_str);

		/* Make an attempt to make the readiness state
		to insvc.*/
		ret_err = plms_plmc_unlck_insvc(ent,trk_info);
	}
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_tcp_disconnect_process(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	PLMS_GROUP_ENTITY *aff_ent_list = NULL;
	PLMS_GROUP_ENTITY *head;
	PLMS_TRACK_INFO trk_info;
	PLMS_ENTITY_GROUP_INFO_LIST *log_head_grp;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	/* If the termination-failed timer is running then stop 
	the timer.*/
	if (ent->tmr.timer_id && 
		(PLMS_TMR_EE_TERMINATING == ent->tmr.tmr_type)) {
		plms_timer_stop(ent);
	}

	if ( SA_PLM_EE_PRESENCE_UNINSTANTIATED != 
			ent->entity.ee_entity.saPlmEEPresenceState){
		TRACE("PLMS has not yet received uninstantiated for the entity.\
		 Set the Presence state to uninstantiated for ent: %s",
		ent->dn_name_str);
		plms_presence_state_set(ent,
					SA_PLM_EE_PRESENCE_UNINSTANTIATED,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
	}
	
	/* Clear the management lost flag.*/
	if (plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST))
		plms_tcp_discon_mngt_flag_clear(ent);
	
	/* If the EE is already in OOS, then nothing to do.*/
	if (SA_PLM_READINESS_OUT_OF_SERVICE == 
		ent->entity.ee_entity.saPlmEEReadinessState){
		TRACE("Entity %s is already in OOS.",ent->dn_name_str);
	
	}else{ /* If the entity is in insvc, then got to make the entity 
		to move to OOS.*/
		/* Get all the affected entities.*/ 
		plms_affected_ent_list_get(ent,&aff_ent_list,0);
		
		plms_readiness_state_set(ent,SA_PLM_READINESS_OUT_OF_SERVICE,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
		
		/* Mark the readiness state and expected readiness state  
		of all the affected entities to OOS and set the dependency 
		flag.*/
		head = aff_ent_list;
		while (head){
			plms_readiness_state_set(head->plm_entity,
					SA_PLM_READINESS_OUT_OF_SERVICE,
					ent,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_DEP);
			plms_readiness_flag_mark_unmark(head->plm_entity,
					SA_PLM_RF_DEPENDENCY,TRUE,
					ent,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_DEP);

			head = head->next;
		}

		
		plms_aff_ent_exp_rdness_state_ow(aff_ent_list);
		plms_ent_exp_rdness_state_ow(ent);

		head = aff_ent_list;
		while(head){
			
			if ((NULL != head->plm_entity->trk_info) && 
			(SA_PLM_CAUSE_EE_RESTART == 
			head->plm_entity->trk_info->track_cause)){
				head = head->next;
				continue;
			}
			
			/* Terminate all the dependent EEs.*/
			if (NCSCC_RC_SUCCESS != 
					plms_is_chld(ent,head->plm_entity)){
				ret_err = plms_ee_term(head->plm_entity,
							FALSE,TRUE/*mngt_cbk*/);
				if (NCSCC_RC_SUCCESS != ret_err){
					LOG_ER("Request for EE %s termination \
					FAILED",head->plm_entity->dn_name_str);
				}
			}	
			head = head->next;
		}
		
		memset(&trk_info,0,sizeof(PLMS_TRACK_INFO));
		trk_info.aff_ent_list = aff_ent_list;
		trk_info.group_info_list = NULL;
		/* Add the groups, root entity(ent) belong to.*/
		plms_ent_grp_list_add(ent, &(trk_info.group_info_list));
	
		/* Find out all the groups, all affected entities 
		 * belong to and add the groups to trk_info->group_info_list.*/ 
		plms_ent_list_grp_list_add(trk_info.aff_ent_list,
					&(trk_info.group_info_list));	

		TRACE("Affected groups for ent %s: ",ent->dn_name_str);
		log_head_grp = trk_info.group_info_list;
		while(log_head_grp){
			TRACE("%llu,",log_head_grp->ent_grp_inf->entity_grp_hdl);
			log_head_grp = log_head_grp->next;
		}

		trk_info.change_step = SA_PLM_CHANGE_COMPLETED;
		trk_info.root_correlation_id = SA_NTF_IDENTIFIER_UNUSED;
		trk_info.track_cause = SA_PLM_CAUSE_EE_UNINSTANTIATED;
		trk_info.grp_op = SA_PLM_GROUP_MEMBER_READINESS_CHANGE;
		
		if ((NULL != ent->trk_info) && (SA_PLM_CAUSE_EE_RESTART
			                == ent->trk_info->track_cause)){
			plms_ent_to_ent_list_add(ent,&(trk_info.aff_ent_list));
			trk_info.root_entity = ent->trk_info->root_entity;
			trk_info.track_cause = ent->trk_info->track_cause;
			plms_cbk_call(&trk_info,0);
		}else{
			trk_info.root_entity = ent;
			trk_info.track_cause = SA_PLM_CAUSE_EE_UNINSTANTIATED;
			plms_cbk_call(&trk_info,1);
		}
	
		plms_ent_exp_rdness_status_clear(ent);
		plms_aff_ent_exp_rdness_status_clear(aff_ent_list);
		
		plms_ent_list_free(trk_info.aff_ent_list);
		trk_info.aff_ent_list = NULL;
		plms_ent_grp_list_free(trk_info.group_info_list);
		trk_info.group_info_list = NULL;
	}

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
		
******************************************************************************/
SaUint32T plms_plmc_unlock_response(PLMS_ENTITY *ent,SaUint32T unlock_rc)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	if (PLMS_PLMC_SUCCESS == unlock_rc){
		/* I am happy as I have already made an attempt to
		take this entity to insvc*/
		TRACE("Unlock SUCCESSFUL for ent %s",ent->dn_name_str);
		
		/* Clear the management lost flag only if set for unlock.*/
		if (plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			plms_unlck_resp_mngt_flag_clear(ent);
		}
	}else{
		/* TODO: Set management lost flag. I do not know why the
		EE was unlocked. So I am not setting admin-op-pending
		flag.*/
		plms_readiness_flag_mark_unmark(ent,SA_PLM_RF_MANAGEMENT_LOST,
						TRUE,NULL,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_PLM_NTFID_STATE_CHANGE_ROOT);

		ent->mngt_lost_tri = PLMS_MNGT_EE_UNLOCK;

		LOG_ER("Unlock FAILED for ent %s",ent->dn_name_str);
		/* Call the callback.*/
		ret_err = plms_mngt_lost_clear_cbk_call(ent,1/*lost*/);

		if (ret_err != NCSCC_RC_SUCCESS){
			LOG_ER("PLMS to PLMA track callback event sending\
				FAILED, Operation: mngt_lost(unlock-failed),\
				ent: %s.",ent->dn_name_str);
		}else{
			TRACE("PLMS to PLMA track callback event sending \
				SUCCEEDED, Operation: mngt_lost(unlock-failed),\
				ent: %s.",ent->dn_name_str);
		}
	}
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}

/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
		
******************************************************************************/
SaUint32T plms_plmc_lock_response(PLMS_ENTITY *ent,SaUint32T lock_rc)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	if (PLMS_PLMC_SUCCESS == lock_rc){
		/* Nothing to do.*/
		TRACE("Lock SUCCESSFUL for ent %s",ent->dn_name_str);
		/* Clear the management lost flag only if set for unlock.*/
		if (plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			plms_lck_resp_mngt_flag_clear(ent);
		}
			
	}else{
		/* TODO: Set management lost flag. I do not know why the
		EE was locked. So I am not setting admin-op-pending
		flag.*/
		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			plms_readiness_flag_mark_unmark(ent,
						SA_PLM_RF_MANAGEMENT_LOST,
						TRUE,NULL,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_PLM_NTFID_STATE_CHANGE_ROOT);
		

			LOG_ER("Lock FAILED for ent %s",ent->dn_name_str);
			/* Call callback.*/
			plms_mngt_lost_clear_cbk_call(ent,1/*lost*/);
		}
		
		ent->mngt_lost_tri = PLMS_MNGT_EE_LOCK;
	
	}
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : Termination-failed timer is not stopped here. If the
		termination is successful, then it will be taken care on
		getting EE presence state events. If the termination fails,
		then it will be taken care on getting timer expiry event.
******************************************************************************/
SaUint32T plms_plmc_lckinst_response(PLMS_ENTITY *ent,SaUint32T lckinst_rc)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	if (PLMS_PLMC_SUCCESS == lckinst_rc){
		/*Nothing to do.*/
		TRACE("Lock-inst SUCCESSFUL for ent %s",ent->dn_name_str);
		/* Clear the management lost flag only if set for unlock.*/
		if (plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			plms_lckinst_resp_mngt_flag_clear(ent);
		}
	}else{
		/* TODO: Set management lost flag. I do not know why the
		EE was terminated. So I am not setting admin-op-pending
		flag.*/
		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			plms_readiness_flag_mark_unmark(ent,
						SA_PLM_RF_MANAGEMENT_LOST,
						TRUE,NULL,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_PLM_NTFID_STATE_CHANGE_ROOT);

		
			LOG_ER("Lock-inst FAILED for ent %s",ent->dn_name_str);
			/* Call the callback.*/
			plms_mngt_lost_clear_cbk_call(ent,1/*lost*/);
		}

		ent->mngt_lost_tri = PLMS_MNGT_EE_TERM;
	}

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_restart_response(PLMS_ENTITY *ent,SaUint32T restart_rc)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	if (PLMS_PLMC_SUCCESS == restart_rc){
		/*Nothing to do.*/
		LOG_ER("Restart SUCCESSFUL for ent %s",ent->dn_name_str);
		
		/* Clear the management lost flag only if set for unlock.*/
		if (plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			plms_restart_resp_mngt_flag_clear(ent);
		}
	}else{
		/* TODO: Set management lost flag. I do not know why the
		EE was restarted. So I am not setting admin-op-pending
		flag.*/
		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			plms_readiness_flag_mark_unmark(ent,
						SA_PLM_RF_MANAGEMENT_LOST,
						TRUE,NULL,
						SA_NTF_MANAGEMENT_OPERATION,
						SA_PLM_NTFID_STATE_CHANGE_ROOT);

		
			LOG_ER("Restart FAILED for ent %s",ent->dn_name_str);
			/* Call the callback.*/
			plms_mngt_lost_clear_cbk_call(ent,1/*lost*/);
		}
		
		ent->mngt_lost_tri = PLMS_MNGT_EE_RESTART;

	}

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : TODO: Not implemented for release-1. 
Arguments       : 
                  
Return Values   : 
Notes           : Make sure the prot_ver is freed.
******************************************************************************/
SaUint32T plms_plmc_get_prot_ver_response(PLMS_ENTITY *ent,
					SaUint8T *prot_ver)
{
	TRACE_ENTER2("Not implemented, Entity: %s",ent->dn_name_str);
	TRACE_LEAVE2("Not implemented, Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
		
******************************************************************************/
SaUint32T plms_plmc_get_os_info_response(PLMS_ENTITY *ent,
			PLMS_PLMC_EE_OS_INFO *os_info)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	SaUint32T prev_adm_op = 0;
	PLMS_TRACK_INFO *trk_info = NULL;
	PLMS_CB *cb = plms_cb;	
	
	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	if (NULL != os_info){
	
		/* Clear management lost flag.*/
		if(plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){ 
			if(NCSCC_RC_SUCCESS != plms_os_info_resp_mngt_flag_clear(ent)){
				return NCSCC_RC_FAILURE;
			}
		}
		ret_err = plms_ee_verify(ent,os_info);
		plms_os_info_free(os_info);
		if (NCSCC_RC_SUCCESS != ret_err){
			/* Verification failed. Ignore and return from here.*/
			LOG_ER("OS verification FAILED for EE: %s",ent->dn_name_str);
			TRACE_LEAVE2("Return Val: %d",ret_err);
			return ret_err;
		}else{
			TRACE("OS verification SUCCESSFUL for EE: %s",
						ent->dn_name_str);
			
			plms_presence_state_set(ent,
					SA_PLM_EE_PRESENCE_INSTANTIATED,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);

			/* Handle the admin operation for which this ent is 
			restarted.*/
			if (NULL != ent->trk_info /* && ent->am_i_aff_ent*/){
				
				prev_adm_op = 1;
				trk_info = ent->trk_info;
				ent->trk_info = NULL;
				trk_info->track_count--;
				TRACE("Perform the in progress admin op for \
				entity: %s, adm-op: %d",ent->dn_name_str, \
				trk_info->root_entity->adm_op_in_progress);
				
				if (!(trk_info->track_count)){
					/* Clean up the trk_info.*/
					trk_info->root_entity->
						adm_op_in_progress = FALSE;
					trk_info->root_entity->am_i_aff_ent 
								= FALSE;
					plms_aff_ent_flag_mark_unmark(
					trk_info->aff_ent_list,FALSE);
					
					plms_ent_list_free(
						trk_info->aff_ent_list);	
					trk_info->aff_ent_list = NULL;
					
					/* Send response to IMM.
					TODO: Am I sending this a bit earlier?
					*/
					ret_err = saImmOiAdminOperationResult( 
						cb->oi_hdl, trk_info->inv_id, 
						SA_AIS_OK);
					if (NCSCC_RC_SUCCESS != ret_err){
						LOG_ER("Adm operation result \
						sending to IMM failed for ent: \
						%s,imm-ret-val: %d",
						ent->dn_name_str,ret_err);
					}
					TRACE("Adm operation result sending to\
					IMM successful for ent: %s",
					ent->dn_name_str);
				}
			}
			
			if (SA_PLM_HE_ADMIN_UNLOCKED != 
				ent->entity.ee_entity.saPlmEEAdminState){

				TRACE("Admin state of the entity %s is not \
				unlocked, not starting the required services.",
				ent->dn_name_str);
				/* As the admin state is not unlocked, 
				this entity can not be moved to insvc. Return.*/
				
				if ( prev_adm_op && !(trk_info->track_count))
					plms_trk_info_free(trk_info);
				TRACE_LEAVE2("Return Val: %d",ret_err);
				return NCSCC_RC_SUCCESS;
			}else{
				
				ret_err = plms_ee_unlock(ent,FALSE,TRUE);
				if (NCSCC_RC_SUCCESS != ret_err){
					TRACE("Sending unlock request for ent: \
						%s FAILED", ent->dn_name_str);
					return ret_err;
				}
				TRACE("Sending unlock request for ent: \
					%s SUCCESSFUL", ent->dn_name_str);
				/* Make an attempt to make the readiness state
				to insvc.*/
				ret_err = plms_plmc_unlck_insvc(ent,trk_info);
			}

		}
	}else{
		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			/*Set management lost flag. */
			plms_readiness_flag_mark_unmark(ent,
						SA_PLM_RF_MANAGEMENT_LOST,
						TRUE,NULL,
						SA_NTF_OBJECT_OPERATION,
						SA_PLM_NTFID_STATE_CHANGE_ROOT);
		
			/* Call the callback.*/
			plms_mngt_lost_clear_cbk_call(ent,1/*lost*/);
		}
		
		ent->mngt_lost_tri = PLMS_MNGT_EE_GET_OS_INFO;
	}

	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            :
Description     : TODO: Not implemented for release-1. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_osaf_start_response(PLMS_ENTITY *ent,
			SaUint32T osaf_start_rc)
{
	TRACE_ENTER2("Not implemented, Entity: %s",ent->dn_name_str);
	TRACE_LEAVE2("Not implemented, Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : TODO: Not implemented for release-1. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_osaf_stop_response(PLMS_ENTITY *ent,
			SaUint32T osaf_stop_rc)
{
	TRACE_ENTER2("Not implemented, Entity: %s",ent->dn_name_str);
	TRACE_LEAVE2("Not implemented, Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : TODO: Not implemented for release-1. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_svc_start_response(PLMS_ENTITY *ent,
			SaUint32T svc_start_rc)
{
	TRACE_ENTER2("Not implemented, Entity: %s",ent->dn_name_str);
	TRACE_LEAVE2("Not implemented, Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : TODO: Not implemented for release-1. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_svc_stop_response(PLMS_ENTITY *ent,
			SaUint32T svc_stop_rc)
{
	TRACE_ENTER2("Not implemented, Entity: %s",ent->dn_name_str);
	TRACE_LEAVE2("Not implemented, Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : TODO: Not implemented for release-1. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_plmd_restart_response(PLMS_ENTITY *ent,
			SaUint32T plmd_rest_rc)
{
	TRACE_ENTER2("Not implemented, Entity: %s",ent->dn_name_str);
	TRACE_LEAVE2("Not implemented, Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : TODO: Not implemented for release-1. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_plmc_ee_id_response(PLMS_ENTITY *ent,
			SaUint32T plmd_rest_rc)
{
	TRACE_ENTER2("Not implemented, Entity: %s",ent->dn_name_str);
	TRACE_LEAVE2("Not implemented, Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}

/******************************************************************************
Name            :
Description     : Connect cbk. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
int32 plms_plmc_connect_cbk(SaInt8T *ee_id,SaInt8T *msg)
{
	PLMS_EVT *evt;
	SaUint8T *tmp;
	SaUint32T len = 0;
	PLMS_CB *cb = plms_cb;

	TRACE_ENTER2("Entity: %s, msg: %s",ee_id,msg);

	/* TODO: Free this event after reading the msg from MBX and 
	 * processing.
	 * */	
	evt = (PLMS_EVT *)calloc(1,sizeof(PLMS_EVT));
	if(NULL == evt){
		LOG_CR("connect_cbk, calloc FAILED. Err no: %s",
						strerror(errno));
		assert(1);
	}
		
	memset(&(evt->req_evt.plms_plmc_evt.ee_id),0,SA_MAX_NAME_LENGTH+3);
	
	evt->req_res = PLMS_REQ;
	evt->req_evt.req_type = PLMS_PLMC_EVT_T;
	/* Fill ee-id */
	tmp = (SaUint8T *)ee_id;
	while( '\0' != tmp[len] )
		len++;
	evt->req_evt.plms_plmc_evt.ee_id.length = len;
	memcpy(evt->req_evt.plms_plmc_evt.ee_id.value,ee_id,len);
	
	/* Fill the msg type. */
	if (  0 == strcmp(PLMC_CONN_CB_MSG,msg)){
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_TCP_CONCTED; 
	}else if ( 0 == strcmp(PLMC_DISCONN_CB_MSG,msg)){
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_TCP_DISCONCTED;
	}else{
		LOG_ER("Connect cbk with invalid msg, ent: %s, msg: %s",
							ee_id,msg);
	}	

	/* Post the evt to the PLMSv MBX. */
	if (NCSCC_RC_SUCCESS != m_NCS_IPC_SEND(&cb->mbx,evt,
					NCS_IPC_PRIORITY_HIGH))	{
		LOG_ER("Posting to MBX FAILED, ent: %s, msg: %s",ee_id,msg);
		TRACE_LEAVE2("Return Val: %d",FALSE);
		return 1;
	}
	TRACE("Posted to MBX, ent: %s, msg: %s",ee_id,msg);
	TRACE_LEAVE2("Return Val: %d",TRUE);
	return 0;
}
/******************************************************************************
Name            :
Description     : UDP callback. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
int32 plms_plmc_udp_cbk(udp_msg *msg)
{
	PLMS_EVT *evt;
	SaInt8T *os_info;
	SaUint32T len = 0;
	PLMS_CB *cb = plms_cb;
	
	TRACE_ENTER2("Entity: %s, msg: %s",msg->ee_id,msg->msg);
	
	/* TODO: Free this event after reading the msg from MBX and 
	 * processing.*/	
	evt = (PLMS_EVT *)calloc(1,sizeof(PLMS_EVT));
	if(NULL == evt){
		LOG_CR("udp_cbk, calloc FAILED. Err no: %s",
						strerror(errno));
		assert(0);
	}

	memset(&(evt->req_evt.plms_plmc_evt.ee_id),0,SA_MAX_NAME_LENGTH+3);
	
	evt->req_res = PLMS_REQ;
	evt->req_evt.req_type = PLMS_PLMC_EVT_T;
	/* Fill ee-id */
	len = strlen(msg->ee_id);
	evt->req_evt.plms_plmc_evt.ee_id.length = len;
	memcpy(evt->req_evt.plms_plmc_evt.ee_id.value,msg->ee_id,len);

	switch(msg->msg_idx){
		
	case EE_INSTANTIATING:
		evt->req_evt.plms_plmc_evt.plmc_evt_type =
					PLMS_PLMC_EE_INSTING;
		break;
	
	case EE_TERMINATED:
		evt->req_evt.plms_plmc_evt.plmc_evt_type =
					PLMS_PLMC_EE_TRMTED;
		break;

	case EE_INSTANTIATED:
		evt->req_evt.plms_plmc_evt.plmc_evt_type =
					PLMS_PLMC_EE_INSTED;
		break;
	
	case EE_TERMINATING:
		evt->req_evt.plms_plmc_evt.plmc_evt_type =
					PLMS_PLMC_EE_TRMTING;
		break;
	
	default:
		LOG_ER("UDP cbk with invalid msg. ent: %s, msg %s",
						msg->ee_id,msg->msg);
		free(evt);
		return 1;
		
	}

	len = strlen(msg->os_info);
	os_info = (SaInt8T *)calloc(1,sizeof(SaInt8T)*(len+1));
	memcpy(os_info,msg->os_info,len+1);
	plms_os_info_parse(os_info,
			&(evt->req_evt.plms_plmc_evt.ee_os_info));
	free(os_info);
	os_info = NULL;
	
	/* Post the evt to the PLMSv MBX. */
	if (NCSCC_RC_SUCCESS != m_NCS_IPC_SEND(&cb->mbx,evt,
					NCS_IPC_PRIORITY_HIGH))	{
		LOG_ER("Posting to MBX FAILED, ent: %s, msg: %s",
						msg->ee_id,msg->msg);
		TRACE_LEAVE2("Return Val: %d",FALSE);
		return 1;
	}
	TRACE("Posted to MBX SUCCESSFUL, ent: %s, msg: %s",
						msg->ee_id,msg->msg);

	TRACE_LEAVE2("Return Val: %d",TRUE);
	return 0;	
}
/******************************************************************************
Name            :
Description     : TCP callback. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
static int32 plms_plmc_tcp_cbk(tcp_msg *msg)
{
	PLMS_EVT *evt;
	SaInt8T *os_info;
	SaUint32T len;
	PLMS_CB *cb = plms_cb;
	
	TRACE_ENTER2("Entity: %s, msg: %s",msg->ee_id,msg->cmd);
	
	/* TODO: Free this event after reading the msg from MBX and 
	 * processing.*/	
	evt = (PLMS_EVT *)calloc(1,sizeof(PLMS_EVT));
	if(NULL == evt){
		LOG_CR("tcp_cbk, calloc FAILED. Err no: %s",
						strerror(errno));
		assert(0);
	}
	
	evt->req_res = PLMS_REQ;
	evt->req_evt.req_type = PLMS_PLMC_EVT_T;
	/* Fill the EE-id.*/			
	len = strlen(msg->ee_id);
	
	memset(&(evt->req_evt.plms_plmc_evt.ee_id),0,SA_MAX_NAME_LENGTH+3);
	evt->req_evt.plms_plmc_evt.ee_id.length = len;
	memcpy(evt->req_evt.plms_plmc_evt.ee_id.value,msg->ee_id,len);
	
	switch(msg->cmd_enum){
	case PLMC_GET_ID_CMD:	
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_ID_RESP;
		break;
		
	case PLMC_GET_PROTOCOL_VER_CMD:	
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_VER_RESP;

		/* Get protocol ver. TODO: Free the memory allocated.*/	
		len = strlen(msg->result);
		evt->req_evt.plms_plmc_evt.ee_ver = (SaUint8T *)calloc(1,
						sizeof(SaUint8T)*(len+1)); 
		memcpy(evt->req_evt.plms_plmc_evt.ee_ver,msg->result,len+1);
		break;

	case PLMC_GET_OSINFO_CMD:	
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_OS_RESP;
					
		/* TODO: Parse the OS info and fill the evt structure. */	
		len = strlen(msg->result);
		os_info = (SaInt8T *)calloc(1,sizeof(SaInt8T)*(len+1));
		memcpy(os_info,msg->result,len+1);
		plms_os_info_parse(os_info,
				&(evt->req_evt.plms_plmc_evt.ee_os_info));
		free(os_info);
		os_info = NULL;
		break;
	case PLMC_SA_PLM_ADMIN_LOCK_INSTANTIATION_CMD:	
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_LOCK_INST_RESP;
		if (0 == strcmp(msg->result,PLMC_ACK_MSG_RECD))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;			
	case PLMC_SA_PLM_ADMIN_RESTART_CMD:	
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_RESTART_RESP;
		if (0 == strcmp(msg->result,PLMC_ACK_MSG_RECD))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;
		
	case PLMC_PLMCD_RESTART_CMD:
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_PLMD_RESTART_RESP;
		if (0 == strcmp(msg->result,PLMC_ACK_MSG_RECD))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;
	
	case PLMC_SA_PLM_ADMIN_UNLOCK_CMD:
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_EE_UNLOCK_RESP;
		if ( 0 == (strcmp(msg->result,PLMC_ACK_SUCCESS)))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			/* TODO:FAILURE and TIMEOUT are considered as FAILURE.*/
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;

	case PLMC_SA_PLM_ADMIN_LOCK_CMD:
		evt->req_evt.plms_plmc_evt.plmc_evt_type =
					PLMS_PLMC_EE_LOCK_RESP;
		if ( 0 == (strcmp(msg->result,PLMC_ACK_SUCCESS)))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			/* TODO:FAILURE and TIMEOUT are considered as FAILURE.*/
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;
	case PLMC_OSAF_START_CMD:
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_OSAF_START_RESP;
		if ( 0 == (strcmp(msg->result,PLMC_ACK_SUCCESS)))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			/* TODO:FAILURE and TIMEOUT are considered as FAILURE.*/
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;
		
	case PLMC_OSAF_STOP_CMD:
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_OSAF_STOP_RESP;
		if ( 0 == (strcmp(msg->result,PLMC_ACK_SUCCESS)))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			/* TODO:FAILURE and TIMEOUT are considered as FAILURE.*/
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;

	case PLMC_OSAF_SERVICES_START_CMD:
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_SVC_START_RESP;
		if ( 0 == (strcmp(msg->result,PLMC_ACK_SUCCESS)))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			/* TODO:FAILURE and TIMEOUT are considered as FAILURE.*/
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;
	case PLMC_OSAF_SERVICES_STOP_CMD:
		evt->req_evt.plms_plmc_evt.plmc_evt_type = 
					PLMS_PLMC_SVC_STOP_RESP;
		if ( 0 == (strcmp(msg->result,PLMC_ACK_SUCCESS)))
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_SUCCESS;
		else
			/* TODO:FAILURE and TIMEOUT are considered as FAILURE.*/
			evt->req_evt.plms_plmc_evt.resp = PLMS_PLMC_FAILURE;
		break;
	
	default:
		LOG_ER("tcp_cbk with invalid msg, ent: %s, msg: %s",
						msg->ee_id,msg->cmd);
		free(evt);
		TRACE_LEAVE2("Return Val: %d",FALSE);
		return 1;
	}
	
	/* Post the evt to the PLMSv MBX. */
	if (NCSCC_RC_SUCCESS != m_NCS_IPC_SEND(&cb->mbx,evt,
					NCS_IPC_PRIORITY_HIGH))	{
		LOG_ER("Posting to MBX FAILED, ent: %s, msg: %s",
						msg->ee_id,msg->cmd);
		TRACE_LEAVE2("Return Val: %d",FALSE);
		return 1;
	}
	TRACE("Posted to MBX SUCCESSFUL, ent: %s, msg: %s",
						msg->ee_id,msg->cmd);

	TRACE_LEAVE2("Return Val: %d",TRUE);
	return 0;
	
}
/******************************************************************************
Name            :
Description     : TODO: Error callback. Not implemented. Blocked on HP. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
int32 plms_plmc_error_cbk(plmc_lib_error *msg)
{
	TRACE_ENTER2("Not implemented");

	LOG_NO("Error cbk received. cmd: %d, ee_id: %s, err_msg: %s, err_act: %s",
	msg->cmd_enum,msg->ee_id,msg->errormsg,msg->action);	

	TRACE_LEAVE2("Not implemented");
	return 0;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
		
******************************************************************************/
SaUint32T plms_plmc_mbx_evt_process(PLMS_EVT *evt)
{
	SaUint32T ret_err = NCSCC_RC_FAILURE;
	PLMS_ENTITY *ent = NULL;
	SaUint8T tmp[SA_MAX_NAME_LENGTH+1];
	PLMS_CB *cb = plms_cb;

	if (evt->req_evt.plms_plmc_evt.ee_id.length){
		ent = (PLMS_ENTITY *)ncs_patricia_tree_get(&(cb->entity_info),
			(SaUint8T *)&(evt->req_evt.plms_plmc_evt.ee_id));
		if (NULL == ent){
			memcpy(tmp,evt->req_evt.plms_plmc_evt.ee_id.value,
				 evt->req_evt.plms_plmc_evt.ee_id.length);
			tmp[evt->req_evt.plms_plmc_evt.ee_id.length] = '\0';

			LOG_ER (" Entity not found for PLMC event. ee_id: %s \
			,evt_type: %d",
			tmp,evt->req_evt.plms_plmc_evt.plmc_evt_type);
			
			plms_os_info_free(&(evt->req_evt.plms_plmc_evt.ee_os_info));

			return ret_err;
		}
	}


	switch(evt->req_evt.plms_plmc_evt.plmc_evt_type){
	case PLMS_PLMC_EE_INSTING:
		ret_err = plms_plmc_instantiating_process(ent,
		&(evt->req_evt.plms_plmc_evt.ee_os_info));
			
		break;
	case PLMS_PLMC_EE_INSTED:
		ret_err = plms_plmc_instantiated_process(ent,
		&(evt->req_evt.plms_plmc_evt.ee_os_info));
		break;
	case PLMS_PLMC_EE_TRMTING:
		ret_err = plms_plmc_terminating_process(ent);
		plms_os_info_free(&(evt->req_evt.plms_plmc_evt.ee_os_info));
		break;
	case PLMS_PLMC_EE_TRMTED:
		ret_err = plms_plmc_terminated_process(ent);
		plms_os_info_free(&(evt->req_evt.plms_plmc_evt.ee_os_info));
		break;
	case PLMS_PLMC_EE_TCP_CONCTED:
		ret_err = plms_plmc_tcp_connect_process(ent);
		break;
	case PLMS_PLMC_EE_TCP_DISCONCTED:
		ret_err = plms_plmc_tcp_disconnect_process(ent);
		break;
	case PLMS_PLMC_EE_VER_RESP:
		ret_err = plms_plmc_get_prot_ver_response(ent,
		evt->req_evt.plms_plmc_evt.ee_ver);
		break;
	case PLMS_PLMC_EE_OS_RESP:
		ret_err = plms_plmc_get_os_info_response(ent,
		&evt->req_evt.plms_plmc_evt.ee_os_info);
		break;
	case PLMS_PLMC_EE_LOCK_RESP:
		ret_err = plms_plmc_lock_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_EE_UNLOCK_RESP:
		ret_err = plms_plmc_unlock_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_EE_LOCK_INST_RESP:
		ret_err = plms_plmc_lckinst_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_SVC_START_RESP:
		ret_err = plms_plmc_svc_start_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_SVC_STOP_RESP:
		ret_err = plms_plmc_svc_stop_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_OSAF_START_RESP:
		ret_err = plms_plmc_osaf_start_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_OSAF_STOP_RESP:
		ret_err = plms_plmc_osaf_stop_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_PLMD_RESTART_RESP:
		ret_err = plms_plmc_plmd_restart_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_EE_ID_RESP:
		ret_err = plms_plmc_ee_id_response(ent,
		 evt->req_evt.plms_plmc_evt.resp);
		break;
	case PLMS_PLMC_EE_RESTART_RESP:
		ret_err = plms_plmc_restart_response(ent,
		evt->req_evt.plms_plmc_evt.resp);
		break;
	default:
		break;
	}

	return ret_err;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
		
******************************************************************************/
static SaUint32T plms_ee_verify(PLMS_ENTITY *ent, PLMS_PLMC_EE_OS_INFO *os_info)
{
	SaInt8T  *dn_ee_type,*dn_ee_type_ver,*dn_ee_base_type;
	SaNameT dn_ee_base_type_key;
	PLMS_EE_BASE_INFO *ee_base_type_node = NULL;
	SaStringT tmp_os_ver;
	PLMS_CB *cb = plms_cb;
	
	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	memset(&dn_ee_base_type_key,0,sizeof dn_ee_base_type_key);
	
	dn_ee_type = (SaInt8T *)calloc(1,sizeof(SaInt8T)*
			(ent->entity.ee_entity.saPlmEEType.length + 1));
	if (NULL == dn_ee_type){
		LOG_CR("ee_verify, calloc FAILED.");
		TRACE_LEAVE2();
		assert(0);
	}
	
	dn_ee_type_ver= (SaInt8T *)calloc(1,sizeof(SaInt8T)*
			(ent->entity.ee_entity.saPlmEEType.length + 1));
	if (NULL == dn_ee_type_ver){
		LOG_CR("ee_verify, calloc FAILED.");
		TRACE_LEAVE2();
		assert(0);
	}

	memcpy(dn_ee_type,ent->entity.ee_entity.saPlmEEType.value,
			(ent->entity.ee_entity.saPlmEEType.length));
	dn_ee_type[ent->entity.ee_entity.saPlmEEType.length] = '\0';

	memcpy(dn_ee_type_ver,dn_ee_type,
			(ent->entity.ee_entity.saPlmEEType.length) +1 );

	/* Get the version.*/
	tmp_os_ver = strtok(dn_ee_type_ver,",");
	tmp_os_ver = strtok(tmp_os_ver,"=");
	tmp_os_ver = strtok(NULL,"=");
	if ( 0 != strcmp(tmp_os_ver,os_info->version)){
		LOG_ER("Version did not match, configured:%s,\
		PLMC-provided:%s", tmp_os_ver,os_info->version);
		free(dn_ee_type);
		free(dn_ee_type_ver);
		TRACE_LEAVE2();
		return NCSCC_RC_FAILURE;
	}
	
	/* Strip off the RDN of SaPlmEEType and get the DN of
	SaPlmEEBaseType.*/
	dn_ee_base_type = strstr(dn_ee_type,"safEEType");
	if (NULL == dn_ee_base_type){
		free(dn_ee_type);
		free(dn_ee_type_ver);
		TRACE_LEAVE2("EE Base Type is NULL.");
		return NCSCC_RC_FAILURE;
	}


	/* As the key for the EE base type patricia tree is of 
	SaNameT type, convert the string to the ksy type.*/
	dn_ee_base_type_key.length = strlen(dn_ee_base_type);
	memcpy(dn_ee_base_type_key.value,dn_ee_base_type,
				dn_ee_base_type_key.length); 

	/* Get the EEBaseType.*/
	ee_base_type_node = (PLMS_EE_BASE_INFO *)ncs_patricia_tree_get(
				&(cb->base_ee_info),
				(SaUint8T *)&(dn_ee_base_type_key));
	if (NULL == ee_base_type_node) {
		LOG_ER("EE Base Type node not found in patricia tree.");
		free(dn_ee_type);
		free(dn_ee_type_ver);
		TRACE_LEAVE2();
		return NCSCC_RC_FAILURE;
	}
	/* Match the product id.*/
	if (ee_base_type_node->ee_base_type.saPlmEetProduct){
		if (0 != strcmp(ee_base_type_node->ee_base_type.saPlmEetProduct,
							os_info->product)){
			LOG_ER("Product Id did not match, configured: %s,\
			PLMC-provided: %s",ee_base_type_node->ee_base_type.\
			saPlmEetProduct,os_info->product);
			free(dn_ee_type);
			free(dn_ee_type_ver);
			TRACE_LEAVE2();
			return NCSCC_RC_FAILURE;
		}
	}
	/* Match the vendor id.*/
	if(ee_base_type_node->ee_base_type.saPlmEetVendor){
		if (0 != strcmp(ee_base_type_node->ee_base_type.saPlmEetVendor,
							os_info->vendor)){
			LOG_ER("Vendor Id did not match, configured: %s,\
			PLMC-provided: %s",ee_base_type_node->ee_base_type.\
			saPlmEetVendor,os_info->vendor);
			free(dn_ee_type);
			free(dn_ee_type_ver);
			TRACE_LEAVE2();
			return NCSCC_RC_FAILURE;
		}
	}
	/* Match the release info.*/
	if (ee_base_type_node->ee_base_type.saPlmEetRelease){
		if (0 != strcmp(ee_base_type_node->ee_base_type.saPlmEetRelease,
							os_info->release)){
			LOG_ER("Release info did not match, configured:\
			%s, PLMC-provided: %s",ee_base_type_node->ee_base_type.\
			saPlmEetRelease,os_info->release);
			free(dn_ee_type);
			free(dn_ee_type_ver);
			TRACE_LEAVE2();
			return NCSCC_RC_FAILURE;
		}
	}
	/* Free.*/
	free(dn_ee_type);
	free(dn_ee_type_ver);
		
	TRACE("OS Verification successful for ent: %s", ent->dn_name_str);

	TRACE_LEAVE2("Return Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : Parse the string os_info and fill evt_os_info. 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_os_info_parse(SaInt8T *os_info, 
					PLMS_PLMC_EE_OS_INFO *evt_os_info)
{

	SaUint32T order=1,count;
	SaInt8T *token;
	SaUint8T cpy_cnt;
	SaInt8T *temp_os_info,*temp_os_info_free;

	temp_os_info = (SaInt8T *)calloc(1,sizeof(SaInt8T)*(strlen(os_info)+1));
	if (NULL == temp_os_info){
		LOG_CR("os_info_parse, calloc FAILED.");
		assert(0);
	}

	temp_os_info_free = temp_os_info;

	memcpy(temp_os_info,os_info,(strlen(os_info)+1));

	while(4 >= order){

		token = NULL;
		
		if ((temp_os_info[0] == ';') || ('\0' == temp_os_info[0])){
			token = (SaInt8T *)calloc(1,sizeof(SaInt8T));
			if (NULL == token){
				free(temp_os_info);
				plms_os_info_free(evt_os_info);
				LOG_CR("os_parse, calloc FAILED. Error: %s",
				strerror(errno));
				assert(0);
			}
			(token)[0] = '\0';

			if ( temp_os_info[0] == ';')
				temp_os_info = &temp_os_info[1];
		}else{
			count = 0;
			while( 1){
				if((';' == temp_os_info[count]) || 
				('\0' == temp_os_info[count])){
					break;
					}
				 count++;
			}
			token = (SaInt8T *)calloc(1,sizeof(SaInt8T)*
			(count + 1));

			if (NULL == token){
				free(temp_os_info);
				plms_os_info_free(evt_os_info);
				LOG_CR("os_parse, calloc FAILED. \
				Error: %s", strerror(errno));
				assert(0);
			}
			cpy_cnt = 0;
			while(cpy_cnt <= count-1){
				(token)[cpy_cnt] = temp_os_info[cpy_cnt];
				cpy_cnt++;
			}	
				
			(token)[count] = '\0';
			if ( '\0' != temp_os_info[count])
				temp_os_info = &temp_os_info[count+1];
			else
				temp_os_info = &temp_os_info[count];
		}

		switch(order){
		case 1:
			evt_os_info->version = token;
			break;
		case 2:
			evt_os_info->product = token;
			break;
		case 3:
			evt_os_info->vendor = token;
			break;
		case 4:
			evt_os_info->release = token;
			break;
		}
		order++;
	}
	/* Parsing done. Free the temp allocated buffer.*/
	free(temp_os_info_free);

return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
void plms_os_info_free(PLMS_PLMC_EE_OS_INFO *os_info)
{
	TRACE("Free OS info.");
	/* Free the OS info.*/
	if (NULL != os_info->version){
		free(os_info->version);
		os_info->version = NULL;
	}
	if (NULL != os_info->product){
		free(os_info->product);
		os_info->product = NULL;
	}
	if (NULL != os_info->vendor){
		free(os_info->vendor);
		os_info->vendor = NULL;
	}
	if (NULL != os_info->release){
		free(os_info->release);
		os_info->release = NULL;
	}
	return;	
}
/******************************************************************************
Name            :
Description     : 
Arguments       : 
                  
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_plmc_unlck_insvc(PLMS_ENTITY *ent,
				PLMS_TRACK_INFO *trk_info)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	PLMS_GROUP_ENTITY *aff_ent_list = NULL;
	PLMS_TRACK_INFO new_trk_info;
	PLMS_GROUP_ENTITY *log_head;
	PLMS_ENTITY_GROUP_INFO_LIST *log_head_grp;
	SaUint8T is_flag_aff = 0;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	/* Move the EE to InSvc is other conditions are matched.*/
	if (NCSCC_RC_SUCCESS == plms_move_ent_to_insvc(ent,&is_flag_aff)){
		TRACE("Make an attempt to make the children to insvc.");
		/* Move the children to insvc and clear the dependency flag.*/
		plms_move_chld_ent_to_insvc(ent->leftmost_child,
		&aff_ent_list,1,1);
		TRACE("Make an attempt to make the dependents to insvc.");
		/* Move the dependent entities to InSvc and clear the
		dependency flag.*/
		plms_move_dep_ent_to_insvc(ent->rev_dep_list,
		&aff_ent_list,1);

		TRACE("Affected entities for ent %s: ",ent->dn_name_str);
		log_head = aff_ent_list;
		while(log_head){
			TRACE("%s,",log_head->plm_entity->dn_name_str);
			log_head = log_head->next;
		}
		/* Overwrite the expected readiness status with the current.*/
		plms_aff_ent_exp_rdness_state_ow(aff_ent_list);
	
		/*Fill the expected readiness state of the root ent.*/
		plms_ent_exp_rdness_state_ow(ent);
		
		memset(&new_trk_info,0,sizeof(PLMS_TRACK_INFO));
		new_trk_info.aff_ent_list = aff_ent_list;
		new_trk_info.group_info_list = NULL;
	
		/* Add the groups, root entity(ent) belong to.*/
		plms_ent_grp_list_add(ent, &(new_trk_info.group_info_list));
	
		/* Find out all the groups, all affected entities belong to 
		and add the groups to trk_info->group_info_list. */
		plms_ent_list_grp_list_add(aff_ent_list,
					&(new_trk_info.group_info_list));	

		TRACE("Affected groups for ent %s: ",ent->dn_name_str);
		log_head_grp = new_trk_info.group_info_list;
		while(log_head_grp){
			TRACE("%llu,",log_head_grp->ent_grp_inf->entity_grp_hdl);
			log_head_grp = log_head_grp->next;
		}
		new_trk_info.change_step = SA_PLM_CHANGE_COMPLETED;
		new_trk_info.root_correlation_id = SA_NTF_IDENTIFIER_UNUSED;
		new_trk_info.grp_op = SA_PLM_GROUP_MEMBER_READINESS_CHANGE;
		if ((NULL != trk_info) && (SA_PLM_CAUSE_HE_RESET == 
		trk_info->track_cause)){
			plms_ent_to_ent_list_add(ent,&(new_trk_info.aff_ent_list));
			new_trk_info.track_cause = trk_info->track_cause;
			new_trk_info.root_entity = trk_info->root_entity;
			plms_cbk_call(&new_trk_info,0/*do not add root */);
		}else{
			new_trk_info.track_cause = SA_PLM_CAUSE_EE_INSTANTIATED;
			new_trk_info.root_entity = ent;
			plms_cbk_call(&new_trk_info,1);
		}

		plms_ent_exp_rdness_status_clear(ent);
		plms_aff_ent_exp_rdness_status_clear(aff_ent_list);
		
		plms_ent_list_free(new_trk_info.aff_ent_list);
		new_trk_info.aff_ent_list = NULL;
		
		plms_ent_grp_list_free(new_trk_info.group_info_list);
		new_trk_info.group_info_list = NULL;
	}else if(is_flag_aff){
		/*Fill the expected readiness state of the root ent.*/
		plms_ent_exp_rdness_state_ow(ent);
		
		memset(&new_trk_info,0,sizeof(PLMS_TRACK_INFO));
		new_trk_info.group_info_list = NULL;
	
		/* Add the groups, root entity(ent) belong to.*/
		plms_ent_grp_list_add(ent, &(new_trk_info.group_info_list));
	
		TRACE("Affected groups for ent %s: ",ent->dn_name_str);
		log_head_grp = new_trk_info.group_info_list;
		while(log_head_grp){
			TRACE("%llu,",log_head_grp->ent_grp_inf->entity_grp_hdl);
			log_head_grp = log_head_grp->next;
		}
		new_trk_info.change_step = SA_PLM_CHANGE_COMPLETED;
		new_trk_info.root_correlation_id = SA_NTF_IDENTIFIER_UNUSED;
		new_trk_info.grp_op = SA_PLM_GROUP_MEMBER_READINESS_CHANGE;
	
		/* For */
		if ((NULL != trk_info) && (SA_PLM_CAUSE_HE_RESET == 
		trk_info->track_cause)){
			plms_ent_to_ent_list_add(ent,&(new_trk_info.aff_ent_list));
			new_trk_info.track_cause = trk_info->track_cause;
			new_trk_info.root_entity = trk_info->root_entity;
			plms_cbk_call(&new_trk_info,0/*do not add root */);
		}else{
			new_trk_info.track_cause = SA_PLM_CAUSE_EE_INSTANTIATED;
			new_trk_info.root_entity = ent;
			plms_cbk_call(&new_trk_info,1);
		}

		plms_ent_exp_rdness_status_clear(ent);
		
		plms_ent_grp_list_free(new_trk_info.group_info_list);
		new_trk_info.group_info_list = NULL;
		
	}
	/* Free the trk_info corresponding to the last admin operation
	if count is zero.*/
	if ((NULL != trk_info) && !(trk_info->track_count))
		plms_trk_info_free(trk_info);
	
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}

/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : This routine takes care of marking management lost flag and 
		admin-operation-pending readiness flag if required. Also calls
		management lost flag if mngt_cbk flag is set.

******************************************************************************/
SaUint32T plms_ee_unlock(PLMS_ENTITY *ent,SaUint32T is_adm_op,SaUint32T mngt_cbk)
{
	SaUint32T ret_err;
	SaUint32T cbk = 0;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	
	/* Get the ee_id in string format.*/	
	ret_err = plmc_sa_plm_admin_unlock(ent->dn_name_str,plms_plmc_tcp_cbk);
	
	/* Unlock operation failed.*/
	if(ret_err){
		if(is_adm_op){
			if (!plms_rdness_flag_is_set(ent,
			SA_PLM_RF_ADMIN_OPERATION_PENDING)){
				plms_readiness_flag_mark_unmark(ent,
					SA_PLM_RF_ADMIN_OPERATION_PENDING, 
					1/*mark*/,NULL,
					SA_NTF_MANAGEMENT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
				cbk = 1;	
			}
		}

		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			/* Set management lost flag.*/
			plms_readiness_flag_mark_unmark(ent,
				SA_PLM_RF_MANAGEMENT_LOST,1/*mark*/,
				NULL,SA_NTF_MANAGEMENT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

			cbk = 1;
		}
		/* Call managemnet lost callback.*/
		if (mngt_cbk && cbk){
			plms_mngt_lost_clear_cbk_call(ent,1/*mark*/);
		}

		ent->mngt_lost_tri = PLMS_MNGT_EE_UNLOCK;
		ret_err = NCSCC_RC_FAILURE;
	}else{
		/* Unlock successful.*/
		ret_err = NCSCC_RC_SUCCESS;
	}	
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            : 
Description     : Call plmc_sa_plm_admin_lock_instantiation to terminate the EE.
		
Arguments       : 
Return Values   : 
Notes           : This routine takes care of marking management lost and
		admin-operation-pending readiness flag if required.

		Mark the presence state of the EE to terminating and also Start
		termination-failed timer if the ee termination returns
		success.
******************************************************************************/
SaUint32T plms_ee_term(PLMS_ENTITY *ent,SaUint32T is_adm_op,SaUint32T mngt_cbk)
{
	SaUint32T ret_err,cbk = 0;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	if (PLMS_EE_ENTITY != ent->entity_type)
		return NCSCC_RC_SUCCESS;

	if ( SA_PLM_EE_PRESENCE_UNINSTANTIATED == 
			ent->entity.ee_entity.saPlmEEPresenceState){
		TRACE("EE %s is already in uninstantiated state.",
		ent->dn_name_str);
		return NCSCC_RC_SUCCESS;
	}

	ret_err = plmc_sa_plm_admin_lock_instantiation(ent->dn_name_str,
	plms_plmc_tcp_cbk);
	
	/* EE termination failed operation failed.*/
	if(ret_err){
		if(is_adm_op){
			if (!plms_rdness_flag_is_set(ent,
			SA_PLM_RF_ADMIN_OPERATION_PENDING)){
				plms_readiness_flag_mark_unmark(ent,
					SA_PLM_RF_ADMIN_OPERATION_PENDING, 
					1/*mark*/,NULL,
					SA_NTF_MANAGEMENT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
				cbk = 1;	
			}
		}

		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			/* Set management lost flag.*/
			plms_readiness_flag_mark_unmark(ent,
				SA_PLM_RF_MANAGEMENT_LOST,1/*mark*/,
				NULL,SA_NTF_MANAGEMENT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

			cbk = 1;
		}
		/* Call managemnet lost callback.*/
		if (mngt_cbk && cbk){
			plms_mngt_lost_clear_cbk_call(ent,1/*mark*/);
		}
		
		ent->mngt_lost_tri = PLMS_MNGT_EE_TERM;
		ret_err = NCSCC_RC_FAILURE;
	}else{
		/* EE termination successful.*/
		/* Mark the presence state of the EE to terminating.*/
		plms_presence_state_set(ent,
					SA_PLM_EE_PRESENCE_TERMINATING,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
		/* Start the termination failed timer.*/
		if (!(ent->tmr.timer_id)){
			ent->tmr.tmr_type = PLMS_TMR_EE_TERMINATING;
			ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
			ent->entity.ee_entity.saPlmEETerminateTimeout);
			if (NCSCC_RC_SUCCESS != ret_err){
				ent->tmr.timer_id = 0;
				ent->tmr.tmr_type = PLMS_TMR_NONE;
				ent->tmr.context_info = NULL;
			}
		}else if (PLMS_TMR_EE_INSTANTIATING == ent->tmr.tmr_type){
			/* Stop the inst timer and start the term timer.*/
			plms_timer_stop(ent);
			ent->tmr.tmr_type = PLMS_TMR_EE_TERMINATING;
			ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
			ent->entity.ee_entity.saPlmEETerminateTimeout);
			if (NCSCC_RC_SUCCESS != ret_err){
				ent->tmr.timer_id = 0;
				ent->tmr.tmr_type = PLMS_TMR_NONE;
				ent->tmr.context_info = NULL;
			}
		} else {
			/* The timer is already running.*/
		}
		
		ret_err = NCSCC_RC_SUCCESS;
	}	
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
	
}
/******************************************************************************
Name            : 
Description     : Call plmc_sa_plm_admin_lock to lock the EE.
		
Arguments       : 
Return Values   : 
Notes           : This routine takes care of marking management lost flag and 
		admin-operation-pendng flag if required.
******************************************************************************/
SaUint32T plms_ee_lock(PLMS_ENTITY *ent, SaUint32T is_adm_op, SaUint32T mngt_cbk)
{
	SaUint32T ret_err,cbk = 0;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	
	ret_err = plmc_sa_plm_admin_lock(ent->dn_name_str,plms_plmc_tcp_cbk);
	
	/* Lock operation failed.*/
	if(ret_err){
		if(is_adm_op){
			if (!plms_rdness_flag_is_set(ent,
			SA_PLM_RF_ADMIN_OPERATION_PENDING)){
				plms_readiness_flag_mark_unmark(ent,
					SA_PLM_RF_ADMIN_OPERATION_PENDING, 
					1/*mark*/,NULL,
					SA_NTF_MANAGEMENT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
				cbk = 1;	
			}
		}

		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			/* Set management lost flag.*/
			plms_readiness_flag_mark_unmark(ent,
				SA_PLM_RF_MANAGEMENT_LOST,1/*mark*/,
				NULL,SA_NTF_MANAGEMENT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

			cbk = 1;
		}
		/* Call managemnet lost callback.*/
		if (mngt_cbk && cbk){
			plms_mngt_lost_clear_cbk_call(ent,1/*mark*/);
		}
		ent->mngt_lost_tri = PLMS_MNGT_EE_LOCK;
		ret_err = NCSCC_RC_FAILURE;
	}else{
		/* Lock successful.*/
		ret_err = NCSCC_RC_SUCCESS;
	}	
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            : 
Description     :
		
Arguments       : 
Return Values   : 
Notes           : This routine takes care of marking management lost flag and 
		admin-operation-pendng flag if required.
******************************************************************************/
SaUint32T plms_ee_reboot(PLMS_ENTITY *ent, SaUint32T is_adm_op,SaUint32T mngt_cbk)
{
	SaUint32T ret_err,cbk = 0;
	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	if ( SA_PLM_EE_PRESENCE_UNINSTANTIATED == 
		ent->entity.ee_entity.saPlmEEPresenceState){
		LOG_ER("%s is in uninstantiated state, can not be \
		reboot.", ent->dn_name_str);
		return NCSCC_RC_FAILURE;
	}
	
	ret_err = plmc_sa_plm_admin_restart(ent->dn_name_str,plms_plmc_tcp_cbk);
	
	/* Restart operation failed.*/
	if(ret_err){
		if(is_adm_op){
			if (!plms_rdness_flag_is_set(ent,
			SA_PLM_RF_ADMIN_OPERATION_PENDING)){
				plms_readiness_flag_mark_unmark(ent,
					SA_PLM_RF_ADMIN_OPERATION_PENDING, 
					1/*mark*/,NULL,
					SA_NTF_MANAGEMENT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
				cbk = 1;	
			}
		}

		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			/* Set management lost flag.*/
			plms_readiness_flag_mark_unmark(ent,
				SA_PLM_RF_MANAGEMENT_LOST,1/*mark*/,
				NULL,SA_NTF_MANAGEMENT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

			cbk = 1;
		}
		/* Call managemnet lost callback.*/
		if (mngt_cbk && cbk){
			plms_mngt_lost_clear_cbk_call(ent,1/*mark*/);
		}
		
		ent->mngt_lost_tri = PLMS_MNGT_EE_RESTART;
		ret_err = NCSCC_RC_FAILURE;
	}else{
		/* Restart successful.*/
		ret_err = NCSCC_RC_SUCCESS;
	}	
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}

/******************************************************************************
Name            : 
Description     : Reset the HE on which the EE is running on.
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_ee_instantiate(PLMS_ENTITY *ent,SaUint32T is_adm_op,SaUint32T mngt_cbk)
{
	SaUint32T ret_err,cbk = 0;
	PLMS_ENTITY *parent_he;

	if (PLMS_EE_ENTITY != ent->entity_type)
		return NCSCC_RC_SUCCESS;
	
	
	TRACE_ENTER2("Entity: %s",ent->dn_name_str);

	/* As we are not supporting virtualization, the parent of the EE
	will be HE or domain.*/
	parent_he = ent->parent; 
	
	if (NULL == parent_he){
		/*TODO: The EE is direct child of domain. How to
		instantiate the EE??*/
		LOG_ER("Entity %s is direct child of domain. Instantiate\
				FAILED",ent->dn_name_str);
		TRACE_LEAVE2("Return Val: %d",NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}
	/* TODO: Virtualization is not taken into consideration.*/
	if (NULL == parent_he->entity.he_entity.saPlmHECurrEntityPath){
		/* TODO: HE is not present.*/
		LOG_ER("Parent HE of this EE %s is not present.",
						ent->dn_name_str);
		ret_err = NCSCC_RC_FAILURE; 
	}else{
		/* Reset the parent_he.*/
		TRACE("Reset the parent HE %s(ee_inst).",
						parent_he->dn_name_str);
		ret_err = plms_he_reset(parent_he,1/*adm_op*/,1/*mngt_cbk*/,
							SAHPI_COLD_RESET);
	}

	/* EE instantiation operation failed.*/
	if(NCSCC_RC_SUCCESS != ret_err){
		ent->mngt_lost_tri = PLMS_MNGT_EE_INST;
		if(is_adm_op){
			if (!plms_rdness_flag_is_set(ent,
			SA_PLM_RF_ADMIN_OPERATION_PENDING)){
				plms_readiness_flag_mark_unmark(ent,
					SA_PLM_RF_ADMIN_OPERATION_PENDING, 
					1/*mark*/,NULL,
					SA_NTF_MANAGEMENT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
				cbk = 1;	
			}
		}

		if (!plms_rdness_flag_is_set(ent,SA_PLM_RF_MANAGEMENT_LOST)){
			/* Set management lost flag.*/
			plms_readiness_flag_mark_unmark(ent,
				SA_PLM_RF_MANAGEMENT_LOST,1/*mark*/,
				NULL,SA_NTF_MANAGEMENT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

			cbk = 1;
		}
		/* Call managemnet lost callback.*/
		if (mngt_cbk && cbk){
			plms_mngt_lost_clear_cbk_call(ent,1/*mark*/);
		}
	}else{
		/* EE instantiation successful.*/
		/* Mark the presence state of the EE to terminating.*/
		plms_presence_state_set(ent,
					SA_PLM_EE_PRESENCE_INSTANTIATING,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
		/* Start the instantiation failed timer.*/
		if (!(ent->tmr.timer_id)){
			ent->tmr.tmr_type = PLMS_TMR_EE_INSTANTIATING;
			ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
			ent->entity.ee_entity.saPlmEEInstantiateTimeout);
			if (NCSCC_RC_SUCCESS != ret_err){
				ent->tmr.timer_id = 0;
				ent->tmr.tmr_type = PLMS_TMR_NONE;
				ent->tmr.context_info = NULL;
			}
		}else if (PLMS_TMR_EE_TERMINATING == ent->tmr.tmr_type){
			/* Stop the term timer and start the inst timer.*/
			plms_timer_stop(ent);
			ent->tmr.tmr_type = PLMS_TMR_EE_INSTANTIATING;
			ret_err = plms_timer_start(&ent->tmr.timer_id,ent,
			ent->entity.ee_entity.saPlmEEInstantiateTimeout);
			if (NCSCC_RC_SUCCESS != ret_err){
				ent->tmr.timer_id = 0;
				ent->tmr.tmr_type = PLMS_TMR_NONE;
				ent->tmr.context_info = NULL;
			}
		} else {
			/* The timer is already running.*/
		}
		
		ret_err = NCSCC_RC_SUCCESS;
	}
	
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : Logically the presence state of the ent should be
		inst-failed. But as this is a failure, we have to isolate the
		EE which means, uninstantiating the EE. Hence moving the EE to
		uninstantiated state.
		Isolated means, taking the EE to uninstantiated state. In this
		case, instantiation failed and hence I can mark the presence
		state to uninstantiated without trying to isolate the EE.
******************************************************************************/
SaUint32T plms_ee_inst_failed_tmr_exp(PLMS_ENTITY *ent)
{
	PLMS_GROUP_ENTITY *head;
	PLMS_CB *cb = plms_cb;
	SaUint32T ret_err;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	
	/* Clean up the timer context.*/
	ent->tmr.timer_id = 0;
	ent->tmr.tmr_type = PLMS_TMR_NONE;
	ent->tmr.context_info = NULL;

	/* TODO: Should I check if my parent is in insvc and
	min dep is ok. If not then return from here.*/

	/* Take the ent to instantiation-failed state.*/
	plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_INSTANTIATION_FAILED,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Mark the operational state to disabled.*/
	plms_op_state_set(ent,SA_PLM_OPERATIONAL_DISABLED,
			NULL,SA_NTF_OBJECT_OPERATION,
			SA_PLM_NTFID_STATE_CHANGE_ROOT);
	
	/* Take the ent to uninstantiated state (isolating the EE).*/
	plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_UNINSTANTIATED,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);
	
	/* Mark the isolation method.*/
	ent->iso_method = PLMS_ISO_EE_TERMINATED;

	if ((NULL != ent->trk_info) && (SA_PLM_CAUSE_EE_RESTART == 
	ent->trk_info->root_entity->adm_op_in_progress)){
		
		ret_err = saImmOiAdminOperationResult(cb->oi_hdl,
		ent->trk_info->inv_id,SA_AIS_ERR_FAILED_OPERATION);

		if (NCSCC_RC_SUCCESS != ret_err){
			LOG_ER("Adm operation result sending to IMM \
			failed for ent: %s,imm-ret-val: %d",
			ent->dn_name_str,ret_err);
		}
		
		/* Free the trk info and send failure to IMM.*/
		head = ent->trk_info->aff_ent_list;
		while(head){
			head->plm_entity->trk_info = NULL;
		}
		
		plms_trk_info_free(ent->trk_info);
	}
				
	TRACE_LEAVE2("eturn Val: %d",NCSCC_RC_SUCCESS);
	return NCSCC_RC_SUCCESS;	
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : Logically the presence state should be marked as termination-
		failed. But termination failed is considered as a failure. 
		This causes to isolate the EE i.e. make its parent HE to go to
		"assert reset state on the HE" or "inactive". In both the cases
		the presence state of the EE is to be set as uninstantiated. If
		the isolation failed, then mark management lost flag and isolate
		pending flag.
		
******************************************************************************/
SaUint32T plms_ee_term_failed_tmr_exp(PLMS_ENTITY *ent)
{
	SaUint32T can_isolate = 1,ret_err = NCSCC_RC_SUCCESS;
	PLMS_GROUP_ENTITY *chld_list = NULL,*head;

	TRACE_ENTER2("Entity: %s",ent->dn_name_str);
	
	/* Clean up the timer context.*/
	ent->tmr.timer_id = 0;
	ent->tmr.tmr_type = PLMS_TMR_NONE;
	ent->tmr.context_info = NULL;
	
	/* Take the ent to instantiation-failed state.*/
	plms_presence_state_set(ent,
				SA_PLM_EE_PRESENCE_TERMINATION_FAILED,
				NULL,SA_NTF_OBJECT_OPERATION,
				SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Mark the operational state to disabled.*/
	plms_op_state_set(ent, SA_PLM_OPERATIONAL_DISABLED,
			NULL,SA_NTF_OBJECT_OPERATION,
			SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Isolate the EE by "assert reset state on parent HE" or taking the
	"paretn HE to inactive".*/

	/* See any other children of the HE other than myself, is insvc.*/
	if (ent->parent){
		plms_chld_get(ent->parent->leftmost_child,&chld_list);
		head = chld_list;
		while(head){
			if ( 0 == strcmp(head->plm_entity->dn_name_str,ent->dn_name_str)){
				head = head->next;
				continue;
			}
			if (plms_is_rdness_state_set(head->plm_entity,
						SA_PLM_READINESS_IN_SERVICE)){
				can_isolate = 0;
				break;
			}
			head = head->next;
		}

		plms_ent_list_free(chld_list);
		chld_list = NULL;
	}else{
		/* EEs parent is domain*/
		LOG_ER("EE`s direct parent is Domain or parent not configured.\
		, hence the EE cannot be isolated.");
		
		plms_readiness_flag_mark_unmark(ent,SA_PLM_RF_MANAGEMENT_LOST,
		1,NULL, SA_NTF_OBJECT_OPERATION,SA_PLM_NTFID_STATE_CHANGE_ROOT);

		ent->mngt_lost_tri = PLMS_MNGT_EE_TERM;
		
		/* Call management lost callback for ent.*/
		plms_mngt_lost_clear_cbk_call(ent,1/*lost*/);
		
		return NCSCC_RC_FAILURE;
	}
	if (can_isolate){
		/* assert reset state on parent HE,
		(parent is HE as no virtualization). */
		ret_err = plms_he_reset(ent->parent,0/*adm_op*/,
		0/*mngt_cbk*/, SAHPI_RESET_ASSERT);
		if( NCSCC_RC_SUCCESS != ret_err){
			/* Deactivate the HE.*/
			ret_err = plms_he_deactivate(ent->parent,
			FALSE/*adm_op*/,TRUE/*mngt_cbk*/);
			if(NCSCC_RC_SUCCESS != ret_err){
				/* Isolation failed.*/
				can_isolate = 0;
				LOG_ER("EE %s Isolation FAILED.",
				ent->dn_name_str);
			}else{
				ent->iso_method = 
				PLMS_ISO_HE_DEACTIVATED;
				
				TRACE("Isolated the ent %s, Deactivate\
				parent HE successfull.",
				ent->dn_name_str);
						
				/* Clear admin pending for HE.*/	
				plms_readiness_flag_mark_unmark(
					ent->parent,
					SA_PLM_RF_ADMIN_OPERATION_PENDING,0,
					NULL, SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
					
				/* Clear management lost for HE.*/
				plms_readiness_flag_mark_unmark(
					ent->parent,
					SA_PLM_RF_MANAGEMENT_LOST,0,
					NULL, SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
			}
		}else{
			TRACE("Isolate the ent %s, Reset assert\
			parent HE successfull.",ent->dn_name_str);
			ent->iso_method = PLMS_ISO_HE_RESET_ASSERT;
		}
	}
	if(!can_isolate){
		/* Set isolate pending flag. Set management lost flag.*/
		plms_readiness_flag_mark_unmark(ent,
					SA_PLM_RF_ISOLATE_PENDING,1,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
		plms_readiness_flag_mark_unmark(ent,
					SA_PLM_RF_MANAGEMENT_LOST,1, 
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
		
		/* ent->mngt_lost_tri = PLMS_MNGT_EE_ISOLATE; */
		ent->mngt_lost_tri = PLMS_MNGT_EE_TERM;
		
		/* Call management lost callback for ent.*/
		plms_mngt_lost_clear_cbk_call(ent,1/*lost*/);
	}else{
		/* EE is isolated. Mark the presence state to uninstantiated.*/
		plms_presence_state_set(ent,
					SA_PLM_EE_PRESENCE_UNINSTANTIATED,
					NULL,SA_NTF_OBJECT_OPERATION,
					SA_PLM_NTFID_STATE_CHANGE_ROOT);
	}
	
	TRACE_LEAVE2("Return Val: %d",ret_err);
	return ret_err;
}

/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
SaUint32T plms_mbx_tmr_handler(PLMS_EVT *evt)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;
	
	switch(evt->req_evt.plm_tmr.tmr_type){
	case PLMS_TMR_EE_INSTANTIATING:
		ret_err = plms_ee_inst_failed_tmr_exp(
		(PLMS_ENTITY *)evt->req_evt.plm_tmr.context_info);
		break;
		
	case PLMS_TMR_EE_TERMINATING:
		ret_err = plms_ee_term_failed_tmr_exp(
		(PLMS_ENTITY *)evt->req_evt.plm_tmr.context_info);
		break;
	default:
		break;
	}

	return ret_err;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : Isolation pending flag takes precedence over admin operation
		pending flag.
******************************************************************************/
static SaUint32T plms_ee_terminated_mngt_flag_clear(PLMS_ENTITY *ent)
{
	TRACE("Clear mngtment lost flag/isolate pending flag/\
	admin pending flag. Reason: %d, Ent: %s",
	ent->mngt_lost_tri, ent->dn_name_str);

	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,
	NULL,SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Clear admin pending flag.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);
	
	/* Clear isolate pending flag.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ISOLATE_PENDING,0,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	/* ent->iso_method = PLMS_ISO_EE_TERMINATED; */
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : Isolation pending flag takes precedence over admin operation
		pending flag.
******************************************************************************/
static SaUint32T plms_tcp_discon_mngt_flag_clear(PLMS_ENTITY *ent)
{
	TRACE("Clear mngtment lost flag/isolate pending flag/\
	admin pending flag. Reason: %d, Ent: %s",ent->mngt_lost_tri,
	ent->dn_name_str);

	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,
	NULL,SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Clear admin pending flag.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);
	
	/* Clear isolate pending flag.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ISOLATE_PENDING,0,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	/* ent->iso_method = PLMS_ISO_EE_TERMINATED; */
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return NCSCC_RC_SUCCESS;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_tcp_connect_mngt_flag_clear(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	/* If isolate pending flag is set then isolate the entity and return
	from here.*/
	if(plms_rdness_flag_is_set(ent, SA_PLM_RF_ISOLATE_PENDING)) {
		
		if (NCSCC_RC_SUCCESS == plms_isolate_and_mngt_lost_clear(ent)) 
			return NCSCC_RC_FAILURE;
		else
			return NCSCC_RC_SUCCESS;
	}
	
	TRACE("Clear management lost flag. Reason:%d, ent: %s",
	ent->mngt_lost_tri,ent->dn_name_str);

	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Clear admin_pending flag if set.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0/*unmark*/,
	NULL, SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return ret_err;
}

/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_unlck_resp_mngt_flag_clear(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	/* If isolate pending flag is set then isolate the entity and return
	from here.*/
	if(plms_rdness_flag_is_set(ent, SA_PLM_RF_ISOLATE_PENDING)){
		
		if (NCSCC_RC_SUCCESS == plms_isolate_and_mngt_lost_clear(ent)) 
			return NCSCC_RC_FAILURE;
		else
			return NCSCC_RC_SUCCESS;
	}
	TRACE("Clear management lost flag. Reason:%d, ent: %s",
	ent->mngt_lost_tri,ent->dn_name_str);
	
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Clear admin_pending flag if set.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0/*unmark*/,
	NULL, SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return ret_err;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_lck_resp_mngt_flag_clear(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	/* If isolate pending flag is set then isolate the entity and return
	from here.*/
	if(plms_rdness_flag_is_set(ent, SA_PLM_RF_ISOLATE_PENDING)) {
		
		if (NCSCC_RC_SUCCESS == plms_isolate_and_mngt_lost_clear(ent)) 
			return NCSCC_RC_FAILURE;
		else
			return NCSCC_RC_SUCCESS;
	}
	
	TRACE("Clear management lost flag. Reason:%d, ent: %s",
	ent->mngt_lost_tri,ent->dn_name_str);

	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Clear admin_pending flag if set.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0/*unmark*/,
	NULL, SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return ret_err;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_lckinst_resp_mngt_flag_clear(PLMS_ENTITY *ent)
{

	TRACE("Clear mngtment lost flag/isolate pending flag/\
	admin pending flag. Reason: %d, Ent: %s",
	ent->mngt_lost_tri,ent->dn_name_str);
	
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0/*unmark*/,
	NULL, SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);
	
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ISOLATE_PENDING,0,
	NULL, SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return NCSCC_RC_SUCCESS; 
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_restart_resp_mngt_flag_clear(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	if(plms_rdness_flag_is_set(ent, SA_PLM_RF_ISOLATE_PENDING)) {
		if (NCSCC_RC_SUCCESS == plms_isolate_and_mngt_lost_clear(ent)) 
			return NCSCC_RC_FAILURE;
		else
			return NCSCC_RC_SUCCESS;
	}
	TRACE("Clear management lost flag. Reason:%d, ent: %s",
	ent->mngt_lost_tri,ent->dn_name_str);

	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Clear admin_pending flag if set.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0/*unmark*/,
	NULL, SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return ret_err;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : 
******************************************************************************/
static SaUint32T plms_os_info_resp_mngt_flag_clear(PLMS_ENTITY *ent)
{
	SaUint32T ret_err = NCSCC_RC_SUCCESS;

	if(plms_rdness_flag_is_set(ent, SA_PLM_RF_ISOLATE_PENDING)) {
		if (NCSCC_RC_SUCCESS == plms_isolate_and_mngt_lost_clear(ent)) 
			return NCSCC_RC_FAILURE;
		else
			return NCSCC_RC_SUCCESS;
	}
	TRACE("Clear management lost flag. Reason:%d, ent: %s",
	ent->mngt_lost_tri,ent->dn_name_str);

	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,NULL,
	SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	/* Clear admin_pending flag if set.*/
	plms_readiness_flag_mark_unmark(ent,
	SA_PLM_RF_ADMIN_OPERATION_PENDING,0/*unmark*/,
	NULL, SA_NTF_OBJECT_OPERATION,
	SA_PLM_NTFID_STATE_CHANGE_ROOT);

	ent->mngt_lost_tri = PLMS_MNGT_NONE;
	plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	return ret_err;
}
/******************************************************************************
Name            : 
Description     : 
		
Arguments       : 
Return Values   : 
Notes           : Isolation pending flag takes precedence over admin operation
		pending flag.
******************************************************************************/
SaUint32T plms_isolate_and_mngt_lost_clear(PLMS_ENTITY *ent)
{
	SaUint32T ret_err;
	SaUint32T is_iso = 0;

	/* If the operational state is disbled, then isolate the
	entity. Otherwise clear the management lost flag.*/
	if ( ((PLMS_HE_ENTITY == ent->entity_type) &&
	(SA_PLM_OPERATIONAL_DISABLED ==
	ent->entity.he_entity.saPlmHEOperationalState)) ||
	((PLMS_EE_ENTITY == ent->entity_type) &&
	(SA_PLM_OPERATIONAL_DISABLED ==
	ent->entity.ee_entity.saPlmEEOperationalState))) {
		
		ret_err = plms_ent_isolate(ent,FALSE,FALSE);
		if (NCSCC_RC_SUCCESS == ret_err)
			is_iso = 1;
	}else{
		is_iso = 1;
		ret_err = NCSCC_RC_FAILURE;
	}
	if (is_iso){
		TRACE("Clear management lost flag,set for ent isolation.\
		Ent: %s",ent->dn_name_str);
		
		plms_readiness_flag_mark_unmark(ent,
		SA_PLM_RF_MANAGEMENT_LOST,0/*unmark*/,NULL,
		SA_NTF_OBJECT_OPERATION,
		SA_PLM_NTFID_STATE_CHANGE_ROOT);

		/* Clear admin_pending as well as 
		isolate_pending flag if set.*/
		plms_readiness_flag_mark_unmark(ent,
		SA_PLM_RF_ISOLATE_PENDING,0/*unmark*/,
		NULL, SA_NTF_OBJECT_OPERATION,
		SA_PLM_NTFID_STATE_CHANGE_ROOT);
		
		plms_readiness_flag_mark_unmark(ent,
		SA_PLM_RF_ADMIN_OPERATION_PENDING,0/*unmark*/,
		NULL, SA_NTF_OBJECT_OPERATION,
		SA_PLM_NTFID_STATE_CHANGE_ROOT);

		ent->mngt_lost_tri = PLMS_MNGT_NONE;
		plms_mngt_lost_clear_cbk_call(ent,0/*unmark*/);
	}else{
		LOG_ER("Management-lost flag clear(ent_iso) FAILED. Ent: %s",
		ent->dn_name_str);
	}
	return ret_err;
}
