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
  
  This module deals with the creation, accessing and deletion of the SI-SI
  dependency database. It also deals with all the MIB operations like set, 
  get, getnext etc related to the SI-SI Dep instance table.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

/* static function prototypes */
static uns32 avd_check_si_dep_sponsors(AVD_CL_CB *cb, AVD_SI *si, NCS_BOOL take_action);
static void avd_update_si_dep_state_for_spons_unassign(AVD_CL_CB *cb, AVD_SI *dep_si, AVD_SI_SI_DEP *si_dep_rec);
static uns32 avd_si_dep_si_unassigned(AVD_CL_CB *cb, AVD_SI *si);
static uns32 avd_check_si_state_enabled(AVD_CL_CB *cb, AVD_SI *si);
static uns32 avd_sg_red_si_process_assignment(AVD_CL_CB *cb, AVD_SI *si);
static uns32 avd_si_dep_state_evt(AVD_CL_CB *cb, AVD_SI *si, AVD_SI_SI_DEP_INDX *si_dep_idx);
static void avd_si_dep_spons_state_modif(AVD_CL_CB *cb, AVD_SI *si, AVD_SI *si_dep, AVD_SI_DEP_SPONSOR_SI_STATE spons_state);
static void avd_si_si_dep_get_indx(NCSMIB_ARG *arg, AVD_SI_SI_DEP_INDX *indx);
static uns32 avd_si_si_dep_cyclic_dep_find(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx);
static void  avd_si_dep_start_unassign(AVD_CL_CB *cb, AVD_EVT *evt);


/*****************************************************************************
 * Function: avd_check_si_state_enabled 
 *
 * Purpose:  This function check the active and the quiescing HA states for
 *           SI of all service units.
 *
 * Input:   cb - Pointer to AVD ctrl-block
 *          si - Pointer to AVD_SI struct
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 **************************************************************************/
uns32 avd_check_si_state_enabled(AVD_CL_CB *cb, AVD_SI *si)
{
   AVD_SU_SI_REL  *susi = NULL;
   uns32          rc = NCSCC_RC_FAILURE;

   susi = si->list_of_sisu;
   while(susi != AVD_SU_SI_REL_NULL)
   {
      if ((susi->state == SA_AMF_HA_ACTIVE) || 
          (susi->state == SA_AMF_HA_QUIESCING))
      {
         rc = NCSCC_RC_SUCCESS;
         break;
      }
     
      susi = susi->si_next;
   }

   if (rc != NCSCC_RC_SUCCESS)
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_SPONSOR_UNASSIGNED);

   return rc;
}


/*****************************************************************************
 * Function: avd_si_dep_spons_list_del
 *
 * Purpose:  This function deletes the spons-SI node from the spons-list of 
 *           dependent-SI.
 *
 * Input:  cb - ptr to AVD control block
 *         si_dep_rec - ptr to AVD_SI_SI_DEP struct 
 *
 * Returns:
 *
 * NOTES:
 * 
 **************************************************************************/
void avd_si_dep_spons_list_del(AVD_CL_CB *cb, AVD_SI_SI_DEP *si_dep_rec)
{
   AVD_SI *dep_si = NULL;
   AVD_SPONS_SI_NODE *spons_si_node = NULL;
   AVD_SPONS_SI_NODE *del_spons_si_node = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_si_dep_spons_list_del");

   /* Dependent SI row Status should be active, if not return error*/
   if ((dep_si = avd_si_struc_find(cb, si_dep_rec->indx_mib.si_name_sec, FALSE))
       == AVD_SI_NULL)
   {
      /* LOG the message */
      return;
   }

   /* SI doesn't depend on any other SIs */
   if (dep_si->spons_si_list == NULL)
   {
      /* LOG the message */
      return;
   }

   /* Check if the sponsor is the first node of the list */
   if (m_CMP_NORDER_SANAMET(dep_si->spons_si_list->si->name_net, 
                            si_dep_rec->indx_mib.si_name_prim) == 0)
   {
      spons_si_node = dep_si->spons_si_list;
      dep_si->spons_si_list = spons_si_node->next;
      m_MMGR_FREE_AVD_SPONS_SI_LIST(spons_si_node); 
   }
   else
   {
      while (spons_si_node->next != NULL)
      {
         if(m_CMP_NORDER_SANAMET(spons_si_node->next->si->name_net, 
                                 si_dep_rec->indx_mib.si_name_prim) != 0)
         {
            spons_si_node = spons_si_node->next;
            continue;
         }

         /* Matched spons si node found, then delete */
         del_spons_si_node = spons_si_node->next;
         spons_si_node->next = spons_si_node->next->next;
         m_MMGR_FREE_AVD_SPONS_SI_LIST(del_spons_si_node); 
         break;
      }
   }
   
   return;
}


/*****************************************************************************
 * Function: avd_si_dep_spons_list_add
 *
 * Purpose:  This function adds the spons-SI node in the spons-list of 
 *           dependent-SI.
 *
 * Input:  cb - ptr to AVD control block
 *         dep_si - ptr to AVD_SI struct (dependent-SI node).
 *         spons_si - ptr to AVD_SI struct (sponsor-SI node).
 *
 * Returns:
 *
 * NOTES: 
 * 
 **************************************************************************/
uns32 avd_si_dep_spons_list_add(AVD_CL_CB *avd_cb, AVD_SI *dep_si, AVD_SI *spons_si) 
{
   AVD_SPONS_SI_NODE *spons_si_node;

   m_AVD_LOG_FUNC_ENTRY("avd_si_dep_spons_list_add");

   spons_si_node = m_MMGR_ALLOC_AVD_SPONS_SI_LIST;
   if(spons_si_node == NULL)
   {
     return NCSCC_RC_FAILURE;
   }

   m_NCS_MEMSET(spons_si_node, '\0', sizeof(AVD_SPONS_SI_NODE));

   spons_si_node->si = spons_si;

   spons_si_node->next = dep_si->spons_si_list;
   dep_si->spons_si_list = spons_si_node;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_si_dep_stop_tol_timer
 *
 * Purpose:  This function is been called when SI was unassigned, checks 
 *           whether tolerance timers are running for this SI (because
 *           of its sponsor-SI unassignment), if yes just stop them.  
 *
 * Input:  cb - ptr to AVD control block
 *         si - ptr to AVD_SI struct.
 *
 * Returns:
 *
 * NOTES: 
 * 
 **************************************************************************/
void avd_si_dep_stop_tol_timer(AVD_CL_CB *cb, AVD_SI *si)
{
   AVD_SI_SI_DEP_INDX indx;
   AVD_SI_SI_DEP *rec = NULL;
   AVD_SPONS_SI_NODE *spons_si_node = si->spons_si_list;

   m_AVD_LOG_FUNC_ENTRY("avd_si_dep_stop_tol_timer");

   m_NCS_MEMSET((char *)&indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));
   indx.si_name_sec.length = si->name_net.length;
   m_NCS_MEMCPY(indx.si_name_sec.value, si->name_net.value,
                m_NCS_OS_NTOHS(si->name_net.length));

   while (spons_si_node)
   {
      /* Need to stop the tolerance timer */
      indx.si_name_prim.length = spons_si_node->si->name_net.length;
      m_NCS_MEMCPY(indx.si_name_prim.value, spons_si_node->si->name_net.value,
                   m_NCS_OS_NTOHS(spons_si_node->si->name_net.length));

      if((rec = avd_si_si_dep_find(cb, &indx, TRUE)) != NULL)
      {
         if (rec->si_dep_timer.is_active == TRUE)
         {
            avd_stop_tmr(cb, &rec->si_dep_timer);

            if(si->tol_timer_count > 0)
            {
               si->tol_timer_count--;
            }
         }
      }

      spons_si_node = spons_si_node->next;
   }

   return;
}


/*****************************************************************************
 * Function: avd_si_dep_si_unassigned
 *
 * Purpose:  This function removes the active and the quiescing HA states for
 *           SI from all service units and moves SI dependency state to
 *           SPONSOR_UNASSIGNED state.  
 *
 * Input: cb - The AVD control block
 *        si - Pointer to AVD_SI struct
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 **************************************************************************/
uns32 avd_si_dep_si_unassigned(AVD_CL_CB *cb, AVD_SI *si)
{
   AVD_SU_SI_REL  *susi = NULL;
   uns32          rc = NCSCC_RC_FAILURE;
   AVD_SU_SI_STATE old_fsm_state;


   if (si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE)
   {
      return rc;
   }

   susi = si->list_of_sisu;
   while(susi != AVD_SU_SI_REL_NULL)
   {
      if ((susi->state == SA_AMF_HA_ACTIVE) ||
          (susi->state == SA_AMF_HA_QUIESCING))
      {
         old_fsm_state = susi->fsm;
         susi->fsm = AVD_SU_SI_STATE_UNASGN;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);

         rc = avd_snd_susi_msg(cb, susi->su, susi, AVSV_SUSI_ACT_DEL);
         if (NCSCC_RC_SUCCESS != rc)
         {
            /* LOG the erro */
            susi->fsm = old_fsm_state;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
            return rc;
         }

         /* add the su to su-oper list */
         avd_sg_su_oper_list_add(cb, susi->su, FALSE);
      }

      susi = susi->si_next;
   }

   m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_SPONSOR_UNASSIGNED);

   /* transition to sg-realign state */
   m_AVD_SET_SG_FSM(cb, si->sg_of_si, AVD_SG_FSM_SG_REALIGN);

   /* Check if this SI is a sponsor SI for some other SI, the take appropriate action */
   avd_si_dep_spons_state_modif(cb, si, NULL, AVD_SI_DEP_SPONSOR_UNASSIGNED);

   return rc;
}


/*****************************************************************************
 * Function: avd_sg_red_si_process_assignment 
 *
 * Purpose:  This function process SI assignment process on a redundancy model
 *           that was associated with the SI.
 *
 * Input: cb - The AVD control block
 *        si - Pointer to AVD_SI struct
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_INV_VAL
 *
 * NOTES:
 *
 **************************************************************************/
uns32 avd_sg_red_si_process_assignment(AVD_CL_CB *cb, AVD_SI *si)
{
   if (si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE)
   {
      return NCSCC_RC_FAILURE;
   }

   if((si->max_num_csi == si->num_csi) &&
      (si->admin_state == NCS_ADMIN_STATE_UNLOCK) &&
      (cb->init_state  == AVD_APP_STATE))
   {
      switch(si->sg_of_si->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
          if(avd_sg_2n_si_func(cb, si) != NCSCC_RC_SUCCESS)
          {
             return NCSCC_RC_FAILURE;
          }
          break;

      case AVSV_SG_RED_MODL_NWAYACTV:
          if(avd_sg_nacvred_si_func(cb, si) != NCSCC_RC_SUCCESS)
          {
             return NCSCC_RC_FAILURE;
          }
          break;

      case AVSV_SG_RED_MODL_NWAY:
          if(avd_sg_nway_si_func(cb, si) != NCSCC_RC_SUCCESS)
          {
             return NCSCC_RC_FAILURE;
          }
          break;

      case AVSV_SG_RED_MODL_NPM:
          if(avd_sg_npm_si_func(cb, si) != NCSCC_RC_SUCCESS)
          {
             return NCSCC_RC_FAILURE;
          }
          break;

      case AVSV_SG_RED_MODL_NORED:
      default:
          if(avd_sg_nored_si_func(cb, si) != NCSCC_RC_SUCCESS)
          {
             return NCSCC_RC_FAILURE;
          }
          break;
      }
   
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_ASSIGNED);
      return NCSCC_RC_SUCCESS;
   }

   return NCSCC_RC_FAILURE;
}


/*****************************************************************************
 * Function: avd_si_dep_delete
 *
 * Purpose:  This function deletes the SI-SI dependency records associated
 *           to a particular SI. 
 *
 * Input:  cb - ptr to AVD control block
 *         si - ptr to AVD_SI struct.
 *
 * Returns: 
 *
 * NOTES:
 * 
 **************************************************************************/
void avd_si_dep_delete(AVD_CL_CB *cb, AVD_SI *si)
{
   AVD_SI_SI_DEP *rec = NULL;
   AVD_SI_SI_DEP_INDX spons_indx;
   AVD_SI_SI_DEP_INDX dep_indx;

   m_AVD_LOG_FUNC_ENTRY("avd_si_dep_delete");

   /* Get the sponsor index */
   m_NCS_MEMSET((char *)&spons_indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));
   spons_indx.si_name_prim.length = si->name_net.length;
   m_NCS_MEMCPY(spons_indx.si_name_prim.value, si->name_net.value,
                m_NCS_OS_NTOHS(si->name_net.length));

   m_NCS_MEMSET((char *)&dep_indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));
   dep_indx.si_name_sec.length = si->name_net.length;
   m_NCS_MEMCPY(dep_indx.si_name_sec.value, si->name_net.value,
                m_NCS_OS_NTOHS(si->name_net.length));

   rec = avd_si_si_dep_find_next(cb, &spons_indx, TRUE);
   while (rec != NULL)
   {
      if (m_CMP_NORDER_SANAMET(rec->indx_mib.si_name_prim, 
                               spons_indx.si_name_prim) != 0)
      {
         break;
      }
      
      dep_indx.si_name_prim.length = spons_indx.si_name_sec.length;
      m_NCS_MEMCPY(dep_indx.si_name_prim.value, spons_indx.si_name_sec.value,
                   m_NCS_OS_NTOHS(spons_indx.si_name_sec.length));  
  
      if (ncs_patricia_tree_del(&cb->si_dep.spons_anchor, &rec->tree_node_mib)
         != NCSCC_RC_SUCCESS)
      { 
         /* LOG the message */
      }

      if((rec = avd_si_si_dep_find(cb, &dep_indx, FALSE)) != NULL)
      {
         if(ncs_patricia_tree_del(&cb->si_dep.dep_anchor, &rec->tree_node)
                      != NCSCC_RC_SUCCESS)
         {
            /* log error */
            return;
         }
      }
  
      /* Since the SI Dep. is been removed, adjust the dependent SI dep state accordingly */
      si = avd_si_struc_find(cb, rec->indx.si_name_prim, FALSE);

      if (rec->si_dep_timer.is_active == TRUE)
      {
         avd_stop_tmr(cb, &rec->si_dep_timer);

         if (si->tol_timer_count > 0)
            si->tol_timer_count--;
      }

      avd_screen_sponsor_si_state(cb, si, TRUE); 

      if(rec)
        m_MMGR_FREE_AVD_SI_SI_DEP(rec);

      rec = avd_si_si_dep_find_next(cb, &spons_indx, TRUE);
   }

   return;
}


/*****************************************************************************
 * Function: avd_si_dep_state_evt
 *
 * Purpose:  This function prepares the event to send AVD_EVT_SI_DEP_STATE
 *           event. This event is sent on expiry of tolerance timer or the 
 *           sponsor SI was unassigned and tolerance timer is "0".
 *
 * Input:  cb - ptr to AVD control block
 *         si - ptr to AVD_SI struct.
 *
 * Returns: 
 *
 * NOTES:
 * 
 **************************************************************************/
uns32  avd_si_dep_state_evt(AVD_CL_CB *cb, AVD_SI *si, AVD_SI_SI_DEP_INDX *si_dep_idx)
{
   AVD_EVT *evt = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_si_dep_state_evt");

   evt = m_MMGR_ALLOC_AVD_EVT;
   if (evt == AVD_EVT_NULL)
   {         
      m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_MEMSET(evt,'\0',sizeof(AVD_EVT));

   /* Update evt struct, using tmr field even though this field is not
    * relevant for this event, but it accommodates the required data.
    */
   evt->cb_hdl = cb->cb_handle;
   evt->rcv_evt = AVD_EVT_SI_DEP_STATE;
  
   /* si_dep_idx is NULL for ASSIGN event non-NULL for UNASSIGN event */ 
   if (si_dep_idx != NULL)
   {
      evt->info.tmr.spons_si_name = si_dep_idx->si_name_prim;
      evt->info.tmr.dep_si_name = si_dep_idx->si_name_sec;
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_READY_TO_UNASSIGN);
   }
   else  /* For ASSIGN evt, just enough to feed SI name */
   {
      evt->info.tmr.spons_si_name.length = si->name_net.length;
      m_NCS_MEMCPY(evt->info.tmr.spons_si_name.value,
                   si->name_net.value,
                   m_NCS_OS_NTOHS(si->name_net.length));
   }

   m_AVD_LOG_RCVD_VAL(((long)evt));
   m_AVD_LOG_EVT_INFO(AVD_RCVD_EVENT, evt->rcv_evt);
   
   if (m_NCS_IPC_SEND(&cb->avd_mbx, evt, NCS_IPC_PRIORITY_HIGH) 
            != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
      m_MMGR_FREE_AVD_EVT(evt);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_tmr_si_dep_tol_func
 *
 * Purpose:  On expiry of tolerance timer in SI-SI dependency context, this
 *           function initiates the process of SI unassignment due to its 
 *           sponsor moved to unassigned state.
 *
 * Input:  cb - ptr to AVD control block
 *         evt - ptr to AVD_EVT struct.
 *
 * Returns: 
 *
 * NOTES:
 * 
 **************************************************************************/
void avd_tmr_si_dep_tol_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVD_SI *si = NULL;
   AVD_SI *spons_si = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_tmr_si_dep_tol_func");

   if (evt->info.tmr.type != AVD_TMR_SI_DEP_TOL)
   {
      /* log error that a wrong timer type value */
      m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.type);
      return;
   }

   si = avd_si_struc_find(cb, evt->info.tmr.dep_si_name, FALSE);
   spons_si = avd_si_struc_find(cb, evt->info.tmr.spons_si_name, FALSE);

   if ((si == NULL) || (spons_si == NULL))
   {
      /* Nothing to do here as SI/spons-SI itself lost their existence */
      return;
   }

   /* Since the tol-timer is been expired, can decrement tol_timer_count of 
    * the SI. 
    */
   if (si->tol_timer_count > 0)
      si->tol_timer_count--;

   switch(si->si_dep_state)
   {
   case AVD_SI_NO_DEPENDENCY:
   case AVD_SI_SPONSOR_UNASSIGNED:
   case AVD_SI_ASSIGNED:
   case AVD_SI_UNASSIGNING_DUE_TO_DEP:
       /* LOG the ERROR message. Before moving to this state, need to ensure 
        * all the tolerance timers of this SI are in stopped state.
        */       
       return;

   default:
       break;
   }

   /* Before starting unassignment process of SI, check once again whether 
    * sponsor SIs are assigned back,if so move the SI state to ASSIGNED state 
    */
   avd_screen_sponsor_si_state(cb, si, FALSE);
   if (si->si_dep_state == AVD_SI_ASSIGNED)
   {
      /* Nothing to do further */
      return;
   }

   /* If the SI is already been unassigned, nothing to proceed for 
    * unassignment 
    */ 
   if (si->list_of_sisu == AVD_SU_SI_REL_NULL)
   {
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_SPONSOR_UNASSIGNED);
      return;
   } 
      
   /* Check if spons_si SI-Dep state is not in ASSIGNED state, then 
    * only initiate the unassignment process. Because the SI can be 
    * in this state due to some other spons-SI.
    */  
   if (avd_check_si_state_enabled(cb, spons_si) != NCSCC_RC_SUCCESS)
   {
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_UNASSIGNING_DUE_TO_DEP);

      if (avd_si_dep_si_unassigned(cb, si) != NCSCC_RC_SUCCESS)
      {
         /* Log the error */
      }
   }

   return;
}


/*****************************************************************************
 * Function: avd_check_si_dep_sponsors
 *
 * Purpose:  This function checks whether sponsor SIs of an SI are in enabled /
 *           disabled state.  
 *
 * Input:    cb - Pointer to AVD control block
 *           si - Pointer to AVD_SI struct 
 *           take_action - If TRUE, process the impacts (SI-SI dependencies)
 *                      on dependent-SIs (of sponsor-SI being enabled/disabled)
 *
 * Returns: NCSCC_RC_SUCCESS - if sponsor-SI is in enabled state 
 *          NCSCC_RC_FAILURE - if sponsor-SI is in disabled state 
 *
 * NOTES:  
 * 
 **************************************************************************/
uns32 avd_check_si_dep_sponsors(AVD_CL_CB *cb, AVD_SI *si, NCS_BOOL take_action)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVD_SPONS_SI_NODE *spons_si_node = si->spons_si_list;

   while (spons_si_node)
   {
      if (avd_check_si_state_enabled(cb, spons_si_node->si) != NCSCC_RC_SUCCESS)
      {
         rc = NCSCC_RC_FAILURE;
         if (take_action)
         {
            avd_si_dep_spons_state_modif(cb, spons_si_node->si, si, 
                                        AVD_SI_DEP_SPONSOR_UNASSIGNED);
         }
         else
         {
            /* Sponsors are not in enabled state */
            break;
         }
      }

      spons_si_node = spons_si_node->next;
   }
  
   /* All of the sponsors are in enabled state */ 
   if ((rc == NCSCC_RC_SUCCESS) && (take_action))
   {
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_ASSIGNED);
   
      avd_si_dep_spons_state_modif(cb, si, NULL, AVD_SI_DEP_SPONSOR_ASSIGNED);
   }

   return rc;
}


/*****************************************************************************
 * Function: avd_sg_screen_si_si_dependencies
 *
 * Purpose:  This function screens SI-SI dependencies of the SG SIs. 
 *
 * Input: cb - The AVD control block
 *        sg - Pointer to AVD_SG struct 
 *
 * Returns: 
 *
 * NOTES:  
 * 
 **************************************************************************/
void avd_sg_screen_si_si_dependencies(AVD_CL_CB *cb, AVD_SG *sg)
{
   AVD_SI *si = NULL;

   si = sg->list_of_si;
   while (si != AVD_SI_NULL)
   {
      if (avd_check_si_state_enabled(cb, si) == NCSCC_RC_SUCCESS)
      {
         /* SI was in enabled state, so check for the SI-SI dependencies
          * conditions and update the SI accordingly.
          */
         avd_check_si_dep_sponsors(cb, si, TRUE);
      }
      else
      {
         avd_screen_sponsor_si_state(cb, si, TRUE); 
         if ((si->si_dep_state == AVD_SI_SPONSOR_UNASSIGNED) ||
             (si->si_dep_state == AVD_SI_NO_DEPENDENCY))
         {
            /* Check if this SI is a sponsor SI for some other SI, the take appropriate action */
            avd_si_dep_spons_state_modif(cb, si, NULL, AVD_SI_DEP_SPONSOR_UNASSIGNED);
         }
      }

      si = si->sg_list_of_si_next;
   }

   return;
}


/*****************************************************************************
 * Function: avd_screen_sponsor_si_state 
 *
 * Purpose:  This function checks whether the sponsor SIs are in ASSIGNED 
 *           state. If they are in assigned state, dependent SI changes its
 *           si_dep state accordingly.
 *
 * Input: cb - The AVD control block
 *        si - Pointer to AVD_SI struct 
 *
 * Returns: 
 *
 * NOTES:  
 * 
 **************************************************************************/
void avd_screen_sponsor_si_state(AVD_CL_CB *cb, AVD_SI *si, NCS_BOOL start_assignment)
{
   m_AVD_LOG_FUNC_ENTRY("avd_screen_sponsor_si_state");

   /* Change the SI dependency state only when all of its sponsor SIs are 
    * in assigned state.
    */
   if (avd_check_si_dep_sponsors(cb, si, FALSE) != NCSCC_RC_SUCCESS)
   {
      /* Nothing to do here, just return */
      return;
   }

   switch(si->si_dep_state)
   {
   case AVD_SI_TOL_TIMER_RUNNING:
   case AVD_SI_READY_TO_UNASSIGN: 
       m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_ASSIGNED);
       break;

   case AVD_SI_SPONSOR_UNASSIGNED:
       /* If SI-SI dependency cfg's are removed for this SI the update the 
        * SI dep state with AVD_SI_NO_DEPENDENCY 
        */  
       m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_NO_DEPENDENCY);

   case AVD_SI_NO_DEPENDENCY:
       /* Initiate the process of ASSIGNMENT state, as all of its sponsor SIs
        * are in ASSIGNED state, should be done only when SI is been unassigned
        * due to SI-SI dependency (sponsor unassigned).
        */
       if (start_assignment)
       {
          if (avd_sg_red_si_process_assignment(cb, si) == NCSCC_RC_SUCCESS)
          {
             /* Check if this SI is a sponsor SI for some other SI, the take appropriate action */
             avd_si_dep_spons_state_modif(cb, si, NULL, AVD_SI_DEP_SPONSOR_ASSIGNED);
          }
          else
          {
             /* LOG the error message */
          }
       }
       break;
  
   default:
      return;
   }

   return;
}


/*****************************************************************************
 * Function: avd_process_si_dep_state_evt
 *
 * Purpose:  This function starts SI unassignment process due to sponsor was  
 *           unassigned (SI-SI dependency logics).
 *
 * Input: cb - The AVD control block
 *        evt - Pointer to AVD_EVT struct 
 *
 * Returns: 
 *
 * NOTES:
 * 
 **************************************************************************/
void  avd_process_si_dep_state_evt(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVD_SI *si = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_process_si_dep_state_evt");

   if (evt == NULL) 
   {
      /* Log the message */
      return;
   }

   /* Check whether rcv_evt is AVD_EVT_SI_DEP_STATE event, if not LOG the 
    * error message.
    */
   if (evt->rcv_evt != AVD_EVT_SI_DEP_STATE) 
   {
      /* Log the message */
      return;
   }

   if (evt->info.tmr.dep_si_name.length == 0)
   {
      /* Its an ASSIGN evt */
      si = avd_si_struc_find(cb, evt->info.tmr.spons_si_name, FALSE);
      if (si != NULL)
      {
         avd_screen_sponsor_si_state(cb, si, TRUE); 
      }
   }
   else /* Process UNASSIGN evt */
   {
      avd_si_dep_start_unassign(cb, evt);
   }
  
   return;
}


/*****************************************************************************
 * Function: avd_si_dep_start_unassign
 *
 * Purpose:  This function starts SI unassignment process due to sponsor was  
 *           unassigned (SI-SI dependency logics).
 *
 * Input: cb - The AVD control block
 *        evt - Pointer to AVD_EVT struct 
 *
 * Returns: 
 *
 * NOTES:
 * 
 **************************************************************************/
void  avd_si_dep_start_unassign(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVD_SI *si = NULL;
   AVD_SI *spons_si = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_si_dep_start_unassign");

   si = avd_si_struc_find(cb, evt->info.tmr.dep_si_name, FALSE);
   spons_si = avd_si_struc_find(cb, evt->info.tmr.spons_si_name, FALSE);

   if ((!si) || (!spons_si))
   {
      /* Log the ERROR message */
      return;
   }

   if (si->si_dep_state != AVD_SI_READY_TO_UNASSIGN)
   {
      /* Log the message */
      return;
   }

   /* Before starting unassignment process of SI, check again whether all its
    * sponsors are moved back to assigned state, if so it is not required to
    * initiate the unassignment process for the (dependent) SI.
    */
   avd_screen_sponsor_si_state(cb, si, FALSE); 
   if (si->si_dep_state == AVD_SI_ASSIGNED)
      return;

   /* If the SI is already been unassigned, nothing to proceed for 
    * unassignment 
    */ 
   if (si->list_of_sisu == AVD_SU_SI_REL_NULL)
   {
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_SPONSOR_UNASSIGNED);
      return;
   } 
      
   /* Check if spons_si SI-Dep state is not in ASSIGNED state, then 
    * only initiate the unassignment process. Because the SI can be 
    * in this state due to some other spons-SI.
    */  
   if (avd_check_si_state_enabled(cb, spons_si) != NCSCC_RC_SUCCESS)
   {
      m_AVD_SET_SI_DEP_STATE(cb, si, AVD_SI_UNASSIGNING_DUE_TO_DEP);

      if (avd_si_dep_si_unassigned(cb, si) != NCSCC_RC_SUCCESS)
      {
         /* Log the error */
      }
   }

   return;
}


/*****************************************************************************
 * Function: avd_update_si_dep_state_for_spons_unassign 
 *
 * Purpose:  Upon sponsor SI is unassigned, this function updates the SI  
 *           dependent states either to AVD_SI_READY_TO_UNASSIGN / 
 *           AVD_SI_TOL_TIMER_RUNNING states.
 *
 * Input:  cb - ptr to AVD control block
 *         dep_si - ptr to AVD_SI struct (dependent SI).
 *         si_dep_rec - ptr to AVD_SI_SI_DEP struct 
 *
 * Returns: 
 *
 * NOTES:
 * 
 **************************************************************************/
void avd_update_si_dep_state_for_spons_unassign(AVD_CL_CB *cb, AVD_SI *dep_si,
                                                AVD_SI_SI_DEP *si_dep_rec)
{
   m_AVD_LOG_FUNC_ENTRY("avd_update_si_dep_state_for_spons_unassign");


   switch(dep_si->si_dep_state) 
   { 
   case AVD_SI_ASSIGNED:
       if (si_dep_rec->tolerance_time > 0) 
       {
          /* Start the tolerance timer */
          m_AVD_SET_SI_DEP_STATE(cb, dep_si, AVD_SI_TOL_TIMER_RUNNING);

          /* Start the tolerance timer */
          m_AVD_SI_DEP_TOL_TMR_START(cb, si_dep_rec);

          if(dep_si->tol_timer_count)
          {
            /* It suppose to be "0", LOG the err msg and continue. */
          }

          dep_si->tol_timer_count = 1;
       }
       else
       {
          /* Send an event to start SI unassignment process */
          avd_si_dep_state_evt(cb, dep_si, &si_dep_rec->indx_mib);
       }
       break;

   case AVD_SI_TOL_TIMER_RUNNING:
   case AVD_SI_READY_TO_UNASSIGN:
       if (si_dep_rec->tolerance_time > 0)
       { 
          if (si_dep_rec->si_dep_timer.is_active != TRUE)
          {
             /* Start the tolerance timer */
             m_AVD_SI_DEP_TOL_TMR_START(cb, si_dep_rec);

             /* SI is already in AVD_SI_TOL_TIMER_RUNNING state, so just 
              * increment tol_timer_count indicates that >1 sponsors are 
              * in unassigned state for this SI.
              */
             dep_si->tol_timer_count++;
          }
          else
          {
             /* Should not happen, LOG the error message & go ahead */
          }
       }
       else
       {
          if (!si_dep_rec->unassign_event)
          {
             si_dep_rec->unassign_event = TRUE;

             /* Send an event to start SI unassignment process */
             avd_si_dep_state_evt(cb, dep_si, &si_dep_rec->indx_mib);
          }
       }
       break;
 
   case AVD_SI_NO_DEPENDENCY:
       m_AVD_SET_SI_DEP_STATE(cb, dep_si, AVD_SI_SPONSOR_UNASSIGNED);
       break;
       
   default:
       break;
   }

   return;
}


/*****************************************************************************
 * Function: avd_si_dep_spons_state_modif 
 *
 * Purpose:  Upon SI is assigned/unassigned and if this SI turns out to be 
 *           sponsor SI for some of the SIs (dependents),then update the states
 *           of dependent SIs accordingly (either to AVD_SI_READY_TO_UNASSIGN / 
 *           AVD_SI_TOL_TIMER_RUNNING states).
 *
 * Input:  cb - ptr to AVD control block
 *         si - ptr to AVD_SI struct (sponsor SI).
 *         si_dep - ptr to AVD_SI struct (dependent SI), NULL for all
 *                  dependent SIs of the above sponsor SI.
 *
 * Returns: 
 *
 * NOTES:
 * 
 **************************************************************************/
void avd_si_dep_spons_state_modif(AVD_CL_CB *cb, AVD_SI *si, AVD_SI *si_dep,
                                 AVD_SI_DEP_SPONSOR_SI_STATE spons_state)
{
   AVD_SI *dep_si = NULL;
   AVD_SI_SI_DEP_INDX  si_indx;
   AVD_SI_SI_DEP *si_dep_rec = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_si_dep_spons_state_modif");

   if (si_dep != NULL)
   {
      /* if spons-SI & dep-SI belongs to the same SG, then not required to 
       * state change for the dep-SI as it can be taken care as part of SG
       * screening.
       */
      if (m_CMP_NORDER_SANAMET(si->sg_of_si->name_net, 
                               si_dep->sg_of_si->name_net) == 0)
      {
         return;
      }
   }  

   m_NCS_MEMSET((char *)&si_indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));
   si_indx.si_name_prim.length  = si->name_net.length;
   m_NCS_MEMCPY(si_indx.si_name_prim.value,
                si->name_net.value,
                m_NCS_OS_NTOHS(si_indx.si_name_prim.length));

   /* If si_dep is NULL, means adjust the SI dep states for all depended 
    * SIs of the sponsor SI.
    */
   if (si_dep == NULL)
   {
      si_dep_rec = avd_si_si_dep_find_next(cb, &si_indx, TRUE);
      while (si_dep_rec != NULL)
      {
         if (m_CMP_NORDER_SANAMET(si_dep_rec->indx_mib.si_name_prim, 
                                  si_indx.si_name_prim) != 0)
         {
            /* Seems no more node exists in spons_anchor tree with 
             * "si_indx.si_name_prim" as primary key 
             */
            break;
         }

         dep_si = (AVD_SI *)ncs_patricia_tree_get(&cb->si_anchor, 
                               (uns8*)&si_dep_rec->indx_mib.si_name_sec);
         if (dep_si == NULL)
         {
            /* No corresponding SI node?? some thing wrong */
            si_dep_rec = avd_si_si_dep_find_next(cb, &si_dep_rec->indx_mib, TRUE);

            /* LOG the failure */
            continue;
         }

         /* if spons-SI & dep-SI belongs to the same SG, then not required to 
          * state change for the dep-SI as it can be taken care as part of SG
          * screening.
          */
         if (m_CMP_NORDER_SANAMET(si->sg_of_si->name_net, 
                                  dep_si->sg_of_si->name_net) == 0)
         {
            continue;
         }

         if (spons_state == AVD_SI_DEP_SPONSOR_UNASSIGNED) 
         {
            avd_update_si_dep_state_for_spons_unassign(cb, dep_si, si_dep_rec);
         }
         else
         {
            avd_si_dep_state_evt(cb, dep_si, NULL);
         }

         si_dep_rec = avd_si_si_dep_find_next(cb, &si_dep_rec->indx_mib, TRUE);
      }
   }
   else
   {
      /* Just ignore and return if spons_state is AVD_SI_DEP_SPONSOR_ASSIGNED */
      if (spons_state == AVD_SI_DEP_SPONSOR_ASSIGNED) 
         return;

      /* Frame the index completely to the associated si_dep_rec */
      si_indx.si_name_sec.length  = si_dep->name_net.length;
      m_NCS_MEMCPY(si_indx.si_name_sec.value,
                   si_dep->name_net.value,
                   m_NCS_OS_NTOHS(si_dep->name_net.length));
      
      si_dep_rec = avd_si_si_dep_find(cb, &si_indx, TRUE);

      if (si_dep_rec != NULL)
      {
         /* se_dep_rec primary key should match with sponsor SI name */
         if (m_CMP_NORDER_SANAMET(si_dep_rec->indx_mib.si_name_prim, 
                                  si_indx.si_name_prim) != 0)
            return;

         avd_update_si_dep_state_for_spons_unassign(cb, si_dep, si_dep_rec);
      }
   }

   return;
}


/*****************************************************************************
 * Function: avd_si_si_dep_get_indx
 *
 * Purpose:  This function gets the index data from the instanceID of the MIB
 *           arg structure
 *
 * Input: arg - ptr to the MIB arg structure
 *        indx - Index to retrieve. 
 *
 * Returns: nothing 
 *
 * NOTES:
 *
 **************************************************************************/
void avd_si_si_dep_get_indx(NCSMIB_ARG *arg, AVD_SI_SI_DEP_INDX *indx)
{
   uns32 p_idx_len = 0;
   uns32 s_idx_len = 0;
   uns32 i = 0;

   /* Prepare the si-si dependency database key from the instant ID */
   p_idx_len = (SaUint16T)arg->i_idx.i_inst_ids[0];
   indx->si_name_sec.length  = m_NCS_OS_HTONS(p_idx_len);

   for(i = 0; i < p_idx_len; i++)
   {
     indx->si_name_sec.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   s_idx_len = (SaUint16T)arg->i_idx.i_inst_ids[p_idx_len + 1];
   indx->si_name_prim.length = m_NCS_OS_HTONS(s_idx_len);

   for(i = 0; i < s_idx_len; i++)
   {
     indx->si_name_prim.value[i] = (uns8)(arg->i_idx.i_inst_ids[p_idx_len + 1 + 1 + i]);
   }

   return;
}


/*****************************************************************************
 * Function: avd_si_si_dep_struc_crt
 *
 * Purpose: This function will create and add a AVD_SI_SI_DEP structure to the
 *          trees if an element with the same key value doesn't exist in the
 *          tree.
 *
 * Input: cb - the AVD control block
 *        indx - Index for the row to be added. 
 *
 * Returns: The pointer to AVD_SI_SI_DEP structure allocated and added.
 *
 * NOTES:
 *
 **************************************************************************/
AVD_SI_SI_DEP *avd_si_si_dep_struc_crt(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx)
{
   AVD_SI_SI_DEP *rec = NULL;
   uns32   si_prim_len = m_NCS_OS_NTOHS(indx->si_name_prim.length);
   uns32   si_sec_len = m_NCS_OS_NTOHS(indx->si_name_sec.length);


   m_AVD_LOG_FUNC_ENTRY("avd_si_si_dep_struc_crt");

   /* Allocate a new block structure for mib rec now */
   if ((rec = m_MMGR_ALLOC_AVD_SI_SI_DEP) == NULL)
   {
      /* log an error 
        m_AVD_LOG_MEM_FAIL(AVD_SI_SI_DEP_ALLOC_FAILED);
      */
      return NULL;
   }

   m_NCS_MEMSET(rec, '\0', sizeof(AVD_SI_SI_DEP));

   rec->indx_mib.si_name_prim.length = indx->si_name_prim.length;
   m_NCS_MEMCPY(rec->indx_mib.si_name_prim.value,indx->si_name_prim.value,
                si_prim_len);

   rec->indx_mib.si_name_sec.length = indx->si_name_sec.length;
   m_NCS_MEMCPY(rec->indx_mib.si_name_sec.value,indx->si_name_sec.value,
                si_sec_len);

   rec->indx.si_name_prim.length = indx->si_name_sec.length;
   m_NCS_MEMCPY(rec->indx.si_name_prim.value,indx->si_name_sec.value,
                si_sec_len);

   rec->indx.si_name_sec.length = indx->si_name_prim.length;
   m_NCS_MEMCPY(rec->indx.si_name_sec.value,indx->si_name_prim.value,
                si_prim_len);

   rec->row_status = NCS_ROW_NOT_READY;

   rec->tree_node_mib.key_info = (uns8*)&(rec->indx_mib);
   rec->tree_node_mib.bit   = 0;
   rec->tree_node_mib.left  = NCS_PATRICIA_NODE_NULL;
   rec->tree_node_mib.right = NCS_PATRICIA_NODE_NULL;

   rec->tree_node.key_info = (uns8*)&(rec->indx);
   rec->tree_node.bit   = 0;
   rec->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   rec->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if(ncs_patricia_tree_add(&cb->si_dep.spons_anchor, &rec->tree_node_mib)
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_SI_SI_DEP(rec);
      return NULL;
   }
   
   if(ncs_patricia_tree_add(&cb->si_dep.dep_anchor, &rec->tree_node)
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      ncs_patricia_tree_del(&cb->si_dep.spons_anchor,&rec->tree_node_mib);
      m_MMGR_FREE_AVD_SI_SI_DEP(rec);
      return NULL;
   }

   return rec;
}


/*****************************************************************************
 * Function: avd_si_si_dep_find 
 *
 * Purpose:  This function will find a AVD_SI_SI_DEP structure in the tree 
 *           with indx value as key. Indices can be provided as per the order
 *           mention in the mib or in the reverse of that. 
 *
 * Input: cb - The AVD control block
 *        indx - The key.
 *
 * Returns: The pointer to AVD_SG_SI_RANK structure found in the tree.
 *
 * NOTES: Set the isMibIdx flag value to 1 if indices are as defined by mib and 0
 *        if it is in reverse order
 * 
 **************************************************************************/
AVD_SI_SI_DEP *avd_si_si_dep_find(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx, 
                                  NCS_BOOL isMibIdx)
{
   AVD_SI_SI_DEP *rec = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_si_si_dep_find");

   if(isMibIdx)
   {
      rec = (AVD_SI_SI_DEP *)ncs_patricia_tree_get(&cb->si_dep.spons_anchor, (uns8*)indx);
   }
   else
   {
      rec = (AVD_SI_SI_DEP *)ncs_patricia_tree_get(&cb->si_dep.dep_anchor, (uns8*)indx);
      if (rec != NULL)
      {
         /* Adjust the pointer */
         rec = (AVD_SI_SI_DEP *)(((char *)rec)
                    - (((char *)&(AVD_SI_SI_DEP_NULL->tree_node))
                    - ((char *)AVD_SI_SI_DEP_NULL)));
      }
   }

   return rec;
}


/*****************************************************************************
 * Function: avd_si_si_dep_find_next 
 *
 * Purpose:  This function will find next AVD_SI_SI_DEP structure in the tree
 *           with indx value as key. Indices can be provided as per the order
 *           mention in the mib or in the reverse of that. 
 *
 * Input: cb - the AVD control block
 *        indx - The key.
 *
 * Returns: The next pointer to AVD_SG_SI_RANK structure found in the tree.
 *
 * NOTES: Set the isMibIdx flag value to 1 if indices are as defined by mib and 0
 *        if it is in reverse order
 * 
 **************************************************************************/
AVD_SI_SI_DEP *avd_si_si_dep_find_next(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx,
                                       NCS_BOOL isMibIdx)
{
   AVD_SI_SI_DEP *rec = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_si_si_dep_find_next");

   if (isMibIdx)
   {
      rec = (AVD_SI_SI_DEP *)ncs_patricia_tree_getnext(&cb->si_dep.spons_anchor, (uns8*)indx);
   }
   else
   {
      rec = (AVD_SI_SI_DEP *)ncs_patricia_tree_getnext(&cb->si_dep.dep_anchor, (uns8*)indx);
      if (rec != NULL)
      {
          /* Adjust the pointer */
          rec = (AVD_SI_SI_DEP *)(((char *)rec)
                      - (((char *)&(AVD_SI_SI_DEP_NULL->tree_node))
                       - ((char *)AVD_SI_SI_DEP_NULL)));
      }
   }

   return rec;
}


/*****************************************************************************
 * Function: avd_si_si_dep_del_row 
 *
 * Purpose:  This function will delete and free AVD_SI_SI_DEP structure from 
 *           the tree. It will delete the record from both patricia trees
 *
 * Input: cb - The AVD control block
 *        si - Pointer to service instance row 
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 * 
 **************************************************************************/
uns32 avd_si_si_dep_del_row(AVD_CL_CB *cb, AVD_SI_SI_DEP *rec)
{
   AVD_SI_SI_DEP *si_dep_rec = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_si_si_dep_del_row");

   if (rec == NULL)
      return NCSCC_RC_FAILURE;

   if((si_dep_rec = avd_si_si_dep_find(cb, &rec->indx, FALSE)) != NULL)
   {
      if(ncs_patricia_tree_del(&cb->si_dep.dep_anchor, &si_dep_rec->tree_node)
                      != NCSCC_RC_SUCCESS)
      {
         /* log error */
         return NCSCC_RC_FAILURE;
      }
   }

   si_dep_rec = NULL;

   if((si_dep_rec = avd_si_si_dep_find(cb, &rec->indx_mib, TRUE)) != NULL)
   {
      if(ncs_patricia_tree_del(&cb->si_dep.spons_anchor, &si_dep_rec->tree_node_mib)
                      != NCSCC_RC_SUCCESS)
      {
         /* log error */
         return NCSCC_RC_FAILURE;
      }
   }

   if(si_dep_rec)
     m_MMGR_FREE_AVD_SI_SI_DEP(si_dep_rec);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_si_si_dep_cyclic_dep_find 
 *
 * Purpose:  This function will use to evalute that is new record will because
 *           of cyclic dependency exist in SIs or not.
 *
 * Input: cb   - The AVD control block
 *        indx - Index of the new rec needs to be evaluated.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: Return value NCSCC_RC_SUCCESS means it found a cyclic dependency or
 *        buffer is not sufficient to process this request.
 *
 **************************************************************************/
uns32 avd_si_si_dep_cyclic_dep_find(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx)
{
   AVD_SI_SI_DEP *rec = NULL;
   AVD_SI_DEP_NAME_LIST *start = NULL;
   AVD_SI_DEP_NAME_LIST *temp  = NULL;
   AVD_SI_DEP_NAME_LIST *last  = NULL;
   uns32 rc = NCSCC_RC_FAILURE;
   AVD_SI_SI_DEP_INDX  idx;

   m_AVD_LOG_FUNC_ENTRY("avd_si_si_dep_cyclic_dep_find");

   if(m_CMP_NORDER_SANAMET(indx->si_name_prim, indx->si_name_sec) == 0)
   {
      /* dependent SI and Sponsor SI can not be same 
         Cyclic dependency found return sucess
       */
         return NCSCC_RC_SUCCESS;
   }

   if( (start = m_MMGR_ALLOC_AVD_SI_NAME_LIST) == NULL)
   {
      /*Insufficient memory, record can not be added */
      return NCSCC_RC_SUCCESS;
   }
   else
   {
      start->si_name = indx->si_name_prim;
      start->next = NULL;
      last = start;
   }

   while(last)
   {
      m_NCS_MEMSET((char *)&idx, '\0', sizeof(AVD_SI_SI_DEP_INDX));

      idx.si_name_prim.length = last->si_name.length;
      m_NCS_MEMCPY(idx.si_name_prim.value, last->si_name.value,
                    m_NCS_OS_NTOHS(last->si_name.length));

      rec = avd_si_si_dep_find_next(cb, &idx, FALSE);

      while ((rec != NULL) && (m_CMP_NORDER_SANAMET(rec->indx.si_name_prim, idx.si_name_prim) == 0))
      {
         if(m_CMP_NORDER_SANAMET(indx->si_name_sec, rec->indx.si_name_sec) == 0)
         {
            /* Cyclic dependency found */
            rc = NCSCC_RC_SUCCESS;
            break;
         }
         
         /* Search if this SI name already exist in the list */
         temp = start;
         if(m_CMP_NORDER_SANAMET(temp->si_name, rec->indx.si_name_sec) != 0)
         {
            while((temp->next != NULL) && 
                    (m_CMP_NORDER_SANAMET(temp->next->si_name, rec->indx.si_name_sec) != 0))
            {
               temp = temp->next;
            }
       
            /* SI Name not found in the list, add it*/
            if(temp->next == NULL)
            {
               if( (temp->next = m_MMGR_ALLOC_AVD_SI_NAME_LIST) == NULL )
               {
                  /* Insufficient memory space */
                  rc = NCSCC_RC_SUCCESS;
                  break;
               }

               temp->next->si_name = rec->indx.si_name_sec;
               temp->next->next = NULL;
            }
         }

         rec = avd_si_si_dep_find_next(cb, &rec->indx, FALSE);
      }

      if(rc == NCSCC_RC_SUCCESS)
      {
         break;
      }
      else
      {
         last = last->next;
      }
   }

   /* Free the allocated SI name list */
   while(start)
   {
      temp = start->next;
      m_MMGR_FREE_AVD_SI_NAME_LIST(start);
      start = temp;
   }

   return rc;
}


/*****************************************************************************
 * Function: saamfsisideptableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVD control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/
uns32 saamfsisideptableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                 NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI_SI_DEP *rec = NULL;
   AVD_SI_SI_DEP_INDX indx;

   m_AVD_LOG_FUNC_ENTRY("saamfsisideptableentry_get");

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_MEMSET((char*)&indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));

   /* Get the indx data from the MIB arg */
   avd_si_si_dep_get_indx(arg, &indx);

   rec = avd_si_si_dep_find(avd_cb, &indx, TRUE);

   if (rec == NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)rec;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsisideptableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/
uns32 saamfsisideptableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_SI_SI_DEP  *rec = (AVD_SI_SI_DEP *)data;
   
   m_AVD_LOG_FUNC_ENTRY("saamfsisideptableentry_extract");

   if (rec == NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case saAmfSISIDepToltime_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer, rec->tolerance_time);
      break;

   default:
      /* call the MIBLIB utility routine for standard object types */
      if ((var_info != NULL) && (var_info->offset != 0))
         return ncsmiblib_get_obj_val(param, var_info, data, buffer);
      else
         return NCSCC_RC_NO_OBJECT;
      break;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsisideptableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. 
 * the name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 * 
 **************************************************************************/
uns32 saamfsisideptableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                             NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI        *spons_si = AVD_SI_NULL;
   AVD_SI        *dep_si = AVD_SI_NULL;
   AVD_SI_SI_DEP *rec = NULL;
   AVD_SI_SI_DEP_INDX indx;

   m_AVD_LOG_FUNC_ENTRY("saamfsisideptableentry_set");

   /* Only ACTIVE AvD should process this function */
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_MEMSET((char*)&indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));

   /* Get the indx data from the MIB arg */
   avd_si_si_dep_get_indx(arg, &indx);

   /* Sponsor SI row Status should be ACTIVE, if not return error*/
   if((spons_si = avd_si_struc_find(avd_cb, indx.si_name_prim, FALSE)) == AVD_SI_NULL) 
   {
      return NCSCC_RC_INV_VAL;
   }

   if(spons_si->row_status != NCS_ROW_ACTIVE)
   {
      return NCSCC_RC_INV_VAL;
   }

   /* Dependent SI row Status should be ACTIVE, if not return error*/
   if((dep_si = avd_si_struc_find(avd_cb, indx.si_name_sec, FALSE)) == AVD_SI_NULL)
   {
      return NCSCC_RC_INV_VAL;
   }

   if(dep_si->row_status != NCS_ROW_ACTIVE)
   {
      return NCSCC_RC_INV_VAL;
   }

   /* Search if this record already exist */
   rec = avd_si_si_dep_find(avd_cb, &indx, TRUE);

   /* Record not found then need to be created */
   if(rec == NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfSISIDepRowStatus_ID)
         && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT))
      {
         /* Invalid row status object */
         return NCSCC_RC_INV_VAL;
      }

      /* Check addition of this record should not create the cyclic 
       * dependencies among SIs 
       */
      if(avd_si_si_dep_cyclic_dep_find(avd_cb, &indx) == NCSCC_RC_SUCCESS)
      {
         /* Return value that record cannot be added due to cyclic dependency
          * or insufficient buffer 
          */
         return NCSCC_RC_INV_VAL;
      }

      if(test_flag == TRUE)
        return NCSCC_RC_SUCCESS;

      m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

      /* Add the record */
      rec = avd_si_si_dep_struc_crt(avd_cb, &indx);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if (rec == NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE;
      }

      /* Add the spons si to the spons_si_list of dependent SI */
      if (avd_si_dep_spons_list_add(avd_cb, dep_si, spons_si) == NCSCC_RC_FAILURE)
      {
         /* Delete the created SI dep records */
         avd_si_si_dep_del_row(avd_cb, rec);

         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE;
      }
   }
   else
   {
      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfSISIDepRowStatus_ID)
      {
         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:

            if (test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }
            
            /* set the value, checkpoint the entire record. */
            rec->row_status = NCS_ROW_ACTIVE;
        
            /* Checkpoint the data to standby */
            m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(avd_cb, rec, AVSV_CKPT_AVD_SI_DEP_CONFIG);

            /* Move the dependent SI to appropriate state, if the configured 
             * sponsor is not in assigned state. Not required to check with
             * the remaining states of SI Dep states, as they are kind of 
             * intermittent states towards SPONSOR_UNASSIGNED/ASSIGNED states. 
             */
            if ((avd_cb->init_state  == AVD_APP_STATE) &&
                ((spons_si->si_dep_state == AVD_SI_NO_DEPENDENCY) ||
                (spons_si->si_dep_state == AVD_SI_SPONSOR_UNASSIGNED)))
            {
               avd_si_dep_spons_state_modif(avd_cb, spons_si, dep_si, 
                                        AVD_SI_DEP_SPONSOR_UNASSIGNED);
            }

            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_NOT_IN_SERVICE:
         case NCS_ROW_DESTROY:

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }

            if(arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_DESTROY)
            {
               /* check if it is active currently */
               if(rec->row_status == NCS_ROW_ACTIVE)
               {
                  /* checkpoint to the standby as this row will be deleted */
                  m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, rec, AVSV_CKPT_AVD_SI_DEP_CONFIG);
               }

               /* If SI is in tolerance timer running state because of this
                * SI-SI dep cfg, then stop the tolerance timer.
                */
               if (rec->si_dep_timer.is_active == TRUE)
               {
                  avd_stop_tmr(cb, &rec->si_dep_timer);

                  if(dep_si->tol_timer_count > 0)
                    dep_si->tol_timer_count--;
               }

               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

               avd_si_dep_spons_list_del(avd_cb, rec);

               avd_si_si_dep_del_row(avd_cb, rec);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
     
               /* Update the SI according to its existing sponsors state */ 
               avd_screen_sponsor_si_state(cb, dep_si, TRUE);

               return NCSCC_RC_SUCCESS;
            }

            rec->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;

            break;

         default:
            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);

            /* Invalid row status value */
            return NCSCC_RC_INV_VAL;
            break;
         }
      }
   }

   /* We now have the si-si dep block */
   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   if(arg->req.info.set_req.i_param_val.i_param_id == saAmfSISIDepToltime_ID)
   {
     rec->tolerance_time = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
     if(rec->row_status == NCS_ROW_ACTIVE)
     {
        /* Checkpoint the data to standby */
        m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, rec, AVSV_CKPT_AVD_SI_DEP_CONFIG);
     }
   }
   else if(arg->req.info.set_req.i_param_val.i_param_id == saAmfSISIDepRowStatus_ID)
   {
     /* Nothing to do here, This condition is only to avoid the use of miblib 
      * utility routine
      */
   }
   else
   {
      return NCSCC_RC_NO_OBJECT;
   }
   
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsisideptableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. 
 * the name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVD control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide 
 * the getnext functionality.
 * 
 **************************************************************************/
uns32 saamfsisideptableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI_SI_DEP * rec = NULL;
   AVD_SI_SI_DEP_INDX indx;
   uns32 p_idx_len = 0;
   uns32 s_idx_len = 0;
   uns32 i = 0;

   m_AVD_LOG_FUNC_ENTRY("saamfsisideptableentry_next");

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_MEMSET((char*)&indx, '\0', sizeof(AVD_SI_SI_DEP_INDX));

   if (arg->i_idx.i_inst_len != 0)
   {
      /* Prepare the si-si dependency database key from the instant ID */
      p_idx_len = (SaUint16T)arg->i_idx.i_inst_ids[0];
      indx.si_name_prim.length  = m_NCS_OS_HTONS(p_idx_len);

      for(i = 0; i < p_idx_len; i++)
      {
         indx.si_name_prim.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }

      if((SaUint16T)arg->i_idx.i_inst_len > p_idx_len + 1)
      {
         s_idx_len = (SaUint16T)arg->i_idx.i_inst_ids[p_idx_len + 1];
         indx.si_name_sec.length = m_NCS_OS_HTONS(s_idx_len);

         for(i = 0; i < s_idx_len; i++)
         {
            indx.si_name_sec.value[i] = (uns8)(arg->i_idx.i_inst_ids[p_idx_len + 1 + 1 + i]);
         }
      }
   }

   rec = avd_si_si_dep_find_next(avd_cb, &indx, TRUE);

   if (rec == NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the sponser si name and dependent si name */

   p_idx_len = m_NCS_OS_NTOHS(rec->indx_mib.si_name_prim.length);
   s_idx_len = m_NCS_OS_NTOHS(rec->indx_mib.si_name_sec.length);

   *next_inst_id_len = p_idx_len + 1 + s_idx_len + 1;

   next_inst_id[0] = p_idx_len;
   for(i = 0; i < p_idx_len; i++)
   {
      next_inst_id[i + 1] = (uns32)(rec->indx_mib.si_name_prim.value[i]);
   }

   next_inst_id[p_idx_len + 1] = s_idx_len;
   for(i = 0; i < s_idx_len; i++)
   {
      next_inst_id[p_idx_len + 1 + i + 1] = (uns32)(rec->indx_mib.si_name_sec.value[i]);
   }

   *data = (NCSCONTEXT)rec;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsisideptableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/
uns32 saamfsisideptableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}


