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
  the SG state machine, for processing the events related to SG for N+M
  redundancy model.


..............................................................................

  FUNCTIONS INCLUDED in this file:
 
  avd_sg_npm_su_chose_asgn - Choose and assign SIs to SUs.
  avd_sg_npm_si_func - Function to process the new complete SI in the SG.
  avd_sg_npm_siswitch_func - function called when a operator does a SI switch.
  avd_sg_npm_su_fault_func - function is called when a SU failed and switchover
                            needs to be done.
  avd_sg_npm_su_insvc_func - function is called when a SU readiness state changes
                            to inservice from out of service
  avd_sg_npm_susi_sucss_func - processes successful SUSI assignment. 
  avd_sg_npm_susi_fail_func - processes failure of SUSI assignment.
  avd_sg_npm_realign_func - function called when SG operation is done or cluster
                           timer expires.
  avd_sg_npm_node_fail_func - function is called when the node has already failed and
                             the SIs have to be failed over.
  avd_sg_npm_su_admin_fail -  function is called when SU is LOCKED/SHUTDOWN.  
  avd_sg_npm_si_admin_down - function is called when SIs is LOCKED/SHUTDOWN.
  avd_sg_npm_sg_admin_down - function is called when SGs is LOCKED/SHUTDOWN.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/*****************************************************************************
 * Function: avd_sg_npm_su_next_asgn
 *
 * Purpose:  This function will identify the next SU that has active/standby 
 * assignments based on the state field. If the current SU is null the SG field 
 * will be used to identify the highest ranked active/standby SU.
 *
 * Input: cb - the AVD control block.
 *        sg - The pointer to the service group.
 *        su - The pointer to the current service unit.
 *        state - The HA state of the SU.
 *        
 *
 * Returns: pointer to the SU that is active/standby assigning/assigned.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

static AVD_SU *avd_sg_npm_su_next_asgn(AVD_CL_CB *cb,AVD_SG *sg,AVD_SU *su,SaAmfHAStateT state)
{
   AVD_SU *i_su;
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_su_next_asgn");
   
   if (su == AVD_SU_NULL)
   {
      i_su = sg->list_of_su;
   }else
   {
      i_su = su->sg_list_su_next;
   }
   
   while (i_su != AVD_SU_NULL)
   {
      if((i_su->list_of_susi != AVD_SU_SI_REL_NULL) &&
         (i_su->list_of_susi->state == state))
      {
         break;
      }
      
      i_su = i_su->sg_list_su_next;
   }
   
   return i_su;
}


/*****************************************************************************
 * Function: avd_sg_npm_su_othr
 *
 * Purpose:  This function will identify the other SU through an susi for a 
 *           given SU.
 *
 * Input: cb - the AVD control block.
 *        su - The pointer to the service unit.
 *        
 *
 * Returns: pointer to the SUSI pointing to the other SU .
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

static AVD_SU_SI_REL *avd_sg_npm_su_othr(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU_SI_REL *i_susi,*o_susi;
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_su_othr");
   
   i_susi = su->list_of_susi;
   o_susi = AVD_SU_SI_REL_NULL;
   
   while (i_susi != AVD_SU_SI_REL_NULL)
   {
      if (i_susi->si->list_of_sisu != i_susi)
      {
         o_susi = i_susi->si->list_of_sisu;
         if (o_susi->fsm != AVD_SU_SI_STATE_UNASGN)
            return o_susi;
      }else if (i_susi->si->list_of_sisu->si_next != AVD_SU_SI_REL_NULL)
      {
         o_susi = i_susi->si->list_of_sisu->si_next;
         if (o_susi->fsm != AVD_SU_SI_STATE_UNASGN)
            return o_susi;
      }
      
      i_susi = i_susi->su_next;
   }
   
   return o_susi;
}


/*****************************************************************************
 * Function: avd_sg_npm_su_chk_snd
 *
 * Purpose:  This function will verify that for a given standby SU, if there are 
 * any SUSI relationships whose SI relationship is not with the quiesced SU specified.
 * If yes a remove message will be sent to each of those SUSIs. It then sends a
 * active all message to the standby SU.
 *
 * Input: cb - the AVD control block.
 *        s_su - The pointer to the standby service unit.
 *        q_su - pointer to the quiesced SU.
 *        
 *
 * Returns: none.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

static void avd_sg_npm_su_chk_snd(AVD_CL_CB *cb,AVD_SU *s_su,AVD_SU *q_su)
{
   AVD_SU_SI_REL *i_susi;
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_su_chk_snd");
   
   for (i_susi = s_su->list_of_susi;i_susi != AVD_SU_SI_REL_NULL; i_susi = i_susi->su_next)
   {
      if ((i_susi->si->list_of_sisu != i_susi) && (i_susi->si->list_of_sisu->su == q_su))
         continue;
      
      if ((i_susi->si->list_of_sisu->si_next != AVD_SU_SI_REL_NULL) && 
         (i_susi->si->list_of_sisu->si_next->su == q_su))
         continue;
      
      i_susi->fsm = AVD_SU_SI_STATE_UNASGN;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);
      avd_snd_susi_msg(cb,s_su,i_susi,AVSV_SUSI_ACT_DEL);

   }/* for (i_susi = su->list_of_susi;i_susi != AVD_SU_SI_REL_NULL; i_susi = i_susi->su_next) */
   
   /* Now send active for all the remaining SUSIs. */
   avd_sg_su_si_mod_snd(cb,s_su,SA_AMF_HA_ACTIVE);
   
   return;
}

/*****************************************************************************
 * Function: avd_sg_npm_su_chose_asgn
 *
 * Purpose:  This function will identify SIs whose active assignments is not  
 * complete, search for in-service SUs that can take assignment for this SI and assign 
 * this unassigned SIs to them by Sending D2N-INFO_SU_SI_ASSIGN message for the 
 * SUs with role active for the SIs. It then adds the Assigning SUs to 
 * the SU operation list. If all the SIs are assigned active or no more SUs can
 * take active assignments. For each SI that has active assignment it verifies if
 * standby assignment exists, if no it identifies the standby SU corresponding to
 * this SU and assigns this SI to the SU in standby role. It then adds the Assigning SUs to 
 * the SU operation list. If no assignments happen, it returns NULL.
 *
 * Input: cb - the AVD control block
 *        sg - The pointer to the service group.
 *        
 *
 * Returns: pointer to the SU that is undergoing assignment. Null if
 *          no assignments need to happen.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

static AVD_SU *avd_sg_npm_su_chose_asgn(AVD_CL_CB *cb,AVD_SG *sg)
{
   AVD_SU *i_su = AVD_SU_NULL, *l_su = AVD_SU_NULL,*n_su;
   uns32 cnt = 0;
   AVD_SI *i_si;
   NCS_BOOL new_su = FALSE;
   NCS_BOOL su_found,load_su_found;
   NCS_BOOL actv_su_found = FALSE;
   AVD_SU_SI_REL *o_susi,*tmp_susi;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_su_chose_asgn");

   /* Identifying and assigning active assignments. */   
   i_si = sg->list_of_si;
   su_found = TRUE;
   actv_su_found = load_su_found = FALSE;
   
   while ((i_si != AVD_SI_NULL) && (su_found == TRUE))
   {
      /* Screen SI sponsors state and adjust the SI-SI dep state accordingly */
      avd_screen_sponsor_si_state(cb, i_si, FALSE);

      /* verify that the SI is ready and needs active assignments. */
      if ((i_si->admin_state != NCS_ADMIN_STATE_UNLOCK) ||
          (i_si->si_dep_state == AVD_SI_SPONSOR_UNASSIGNED) || 
          (i_si->si_dep_state == AVD_SI_UNASSIGNING_DUE_TO_DEP) ||
          (i_si->max_num_csi != i_si->num_csi))
      {
         i_si = i_si->sg_list_of_si_next;
         continue;
      }
      
      if (i_si->list_of_sisu != AVD_SU_SI_REL_NULL)
      {
         i_si = i_si->sg_list_of_si_next;
         actv_su_found = TRUE;
         continue;
      }
      
      su_found = FALSE;
      
      /* check if we already have a SU in hand that can take active assignments. */
      if (i_su != AVD_SU_NULL)
      {
         if ((i_su->si_max_active > i_su->si_curr_active) &&
            ((i_su->sg_of_su->su_max_active == 0) ||
            (i_su->sg_of_su->su_max_active > i_su->si_curr_active)))
         {
            /* found the SU assign the SI to the SU as active */
            su_found = TRUE;            
            
         }else/* if ((i_su->si_max_active > i_su->si_curr_active) &&
                    ((i_su->sg_of_su->su_max_active == 0) ||
                    (i_su->sg_of_su->su_max_active > i_su->si_curr_active))) */
         {

            if (i_su->si_max_active > i_su->si_curr_active)
            {
               load_su_found = TRUE;
            }               
            
            if (new_su == FALSE)
            {  
               /* we have last assigning SU identify the next SU that has
                * active assignments.
                */
               while ((i_su != AVD_SU_NULL) && (su_found == FALSE) &&
                  (cnt < sg->pref_num_active_su))
               {
                  i_su = avd_sg_npm_su_next_asgn(cb,sg,i_su,SA_AMF_HA_ACTIVE);
                  if (i_su != AVD_SU_NULL)
                  {
                     cnt ++;
                     if ((i_su->si_max_active > i_su->si_curr_active) &&
                         ((i_su->sg_of_su->su_max_active == 0) ||
                         (i_su->sg_of_su->su_max_active > i_su->si_curr_active)) )
                     {
                        /* Found the next assigning SU that can take SI. */
                        su_found = TRUE;
                     }else if (i_su->si_max_active > i_su->si_curr_active)
                     {
                        load_su_found = TRUE;
                     }
                        
                  }                                    
               }
               
               if ((i_su == AVD_SU_NULL) && 
                  (cnt < sg->pref_num_active_su))
               {
                  /* All the current active SUs are full. The SG can have
                   * more active assignments. identify the highest ranked
                   * in service unassigned SU in the SG.
                   */
                  new_su = TRUE;
                  i_su = sg->list_of_su;
                  while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
                  {
                     if ((i_su->readiness_state == NCS_IN_SERVICE) &&
                        (i_su->list_of_susi == AVD_SU_SI_REL_NULL))
                     {
                        su_found = TRUE;
                        cnt ++;
                        continue;
                     }                     
                     
                     /* choose the next SU */
                     i_su = i_su->sg_list_su_next;
            
                  } /* while (i_su != AVD_SU_NULL) */
                  
               } /* if ((i_su == AVD_SU_NULL) && 
                  (cnt < sg->pref_num_active_su)) */
               
            } /* if (new_su == FALSE) */
            else if (cnt < sg->pref_num_active_su)
            {
               /* we have last assigning new SU, New inservice unassigned SUs need 
                * to be used for the SI. So identify the next SU.
                */
               
               while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
               {
                  if ((i_su->readiness_state == NCS_IN_SERVICE) &&
                     (i_su->list_of_susi == AVD_SU_SI_REL_NULL))
                  {
                     su_found = TRUE;
                     cnt ++;
                     continue;
                  }                     
                  
                  /* choose the next SU */
                  i_su = i_su->sg_list_su_next;
            
               } /* while (i_su != AVD_SU_NULL) */
               
            }/* else if (cnt < sg->pref_num_active_su) */
            
         }/* else ((i_su->si_max_active > i_su->si_curr_active) &&
                  ((i_su->sg_of_su->su_max_active == 0) ||
                  (i_su->sg_of_su->su_max_active > i_su->si_curr_active))) */
            
      }else /* if (i_su != AVD_SU_NULL) */      
      {
         /* Identify the highest ranked active assigning SU.  */
         i_su = avd_sg_npm_su_next_asgn(cb,sg,AVD_SU_NULL,SA_AMF_HA_ACTIVE);
         while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
         {
            actv_su_found = TRUE;
            cnt ++;
            
            if ((i_su->si_max_active > i_su->si_curr_active) &&
                ((i_su->sg_of_su->su_max_active == 0) ||
                (i_su->sg_of_su->su_max_active > i_su->si_curr_active)))
            {
               /* Found the next assigning SU that can take SI. */
               su_found = TRUE;
            }else if (cnt < sg->pref_num_active_su)
            {
               if (i_su->si_max_active > i_su->si_curr_active)
                  load_su_found = TRUE;
               
               i_su = avd_sg_npm_su_next_asgn(cb,sg,i_su,SA_AMF_HA_ACTIVE);
            }else
            {
               if (i_su->si_max_active > i_su->si_curr_active)
                  load_su_found = TRUE;
               i_su = AVD_SU_NULL;
            }
         }
         
         if ((su_found == FALSE) && 
            (cnt < sg->pref_num_active_su))
         {
            /* All the current active SUs are full. The SG can have
             * more active assignments. identify the highest ranked
             * in service unassigned SU in the SG.
             */
            new_su = TRUE;
            i_su = sg->list_of_su;
            while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
            {
               if ((i_su->readiness_state == NCS_IN_SERVICE) &&
                  (i_su->list_of_susi == AVD_SU_SI_REL_NULL))
               {
                  su_found = TRUE;
                  actv_su_found = TRUE;
                  cnt ++;
                  continue;
               }                     
               
               /* choose the next SU */
               i_su = i_su->sg_list_su_next;
         
            } /* while (i_su != AVD_SU_NULL) */
            
         } /* if ((su_found == FALSE) && 
                  (cnt < sg->pref_num_active_su)) */
         
      } /* else (i_su != AVD_SU_NULL) */
      
      if (su_found == TRUE)
      {
         if(avd_new_assgn_susi(cb,i_su,i_si,SA_AMF_HA_ACTIVE, FALSE, &tmp_susi) == NCSCC_RC_SUCCESS)
         {
            /* Add the SU to the operation list */
            avd_sg_su_oper_list_add(cb,i_su,FALSE);
         }else
         {
            /* log a fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)i_si));
            m_AVD_LOG_INVALID_VAL_ERROR(((long)i_su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_si->name_net.value,i_si->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
         }
         
         /* choose the next SI */
         i_si = i_si->sg_list_of_si_next; 
      }
      
   } /* while ((i_si != AVD_SI_NULL) && (su_found == TRUE)) */

   /* The SUs were loaded only upto the preffered level. If still SIs
    * are left load them upto capability level.
    */
   if ((load_su_found == TRUE) && (i_si != AVD_SI_NULL) && (cnt == sg->pref_num_active_su))
   {
      /* Identify the highest ranked active assigning SU.  */
      i_su = avd_sg_npm_su_next_asgn(cb,sg,AVD_SU_NULL,SA_AMF_HA_ACTIVE);    
      while ((i_si != AVD_SI_NULL) && (i_su != AVD_SU_NULL))
      {
         /* Screen SI sponsors state and adjust the SI-SI dep state accordingly */
         avd_screen_sponsor_si_state(cb, i_si, FALSE);
         if ((i_si->admin_state != NCS_ADMIN_STATE_UNLOCK) ||
             (i_si->si_dep_state == AVD_SI_SPONSOR_UNASSIGNED) || 
             (i_si->si_dep_state == AVD_SI_UNASSIGNING_DUE_TO_DEP) ||
             (i_si->max_num_csi != i_si->num_csi) ||
             (i_si->list_of_sisu != AVD_SU_SI_REL_NULL))
         {
            i_si = i_si->sg_list_of_si_next;
            continue;
         }
         
         if (i_su->si_max_active <= i_su->si_curr_active)
         {
            i_su = avd_sg_npm_su_next_asgn(cb,sg,i_su,SA_AMF_HA_ACTIVE);
            continue;
         }
         
         if(avd_new_assgn_susi(cb,i_su,i_si,SA_AMF_HA_ACTIVE, FALSE, &tmp_susi) == NCSCC_RC_SUCCESS)
         {
            /* Add the SU to the operation list */
            avd_sg_su_oper_list_add(cb,i_su,FALSE);
         }else
         {
            /* log a fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)i_si));
            m_AVD_LOG_INVALID_VAL_ERROR(((long)i_su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_si->name_net.value,i_si->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
         }
                  
         /* choose the next SI */
         i_si = i_si->sg_list_of_si_next;
   
      } /* while ((i_si != AVD_SI_NULL) && (i_su != AVD_SU_NULL)) */

   }/* if ((load_su_found == TRUE) && (i_si != AVD_SI_NULL))*/
   
   if ((actv_su_found == FALSE) || (sg->su_oper_list.su != AVD_SU_NULL))
   {
      /* Either there are no active SUs or there are some SUs being assigned 
       * active SI assignment.
       */
      return sg->su_oper_list.su;
   }
   
   /* Identifying and assigning standby assignments. */   
   i_si = sg->list_of_si;
   new_su = FALSE;
   cnt = 0;
   l_su = AVD_SU_NULL;
   /* su with last inservice un assigned SUs.*/
   n_su = AVD_SU_NULL;
   while (i_si != AVD_SI_NULL)
   {      
      /* verify that the SI has active assignments and needs standby assignments. */ 
      if ((i_si->list_of_sisu == AVD_SU_SI_REL_NULL) || 
         (i_si->list_of_sisu->si_next != AVD_SU_SI_REL_NULL))
      {
         i_si = i_si->sg_list_of_si_next;
         continue;
      }
      
      su_found = FALSE;
      
      /* Identify the standby SU if it already exists */
      o_susi = avd_sg_npm_su_othr(cb,i_si->list_of_sisu->su);
      
      if (o_susi != AVD_SU_SI_REL_NULL)
      {
         i_su = o_susi->su;
         if ((i_su->si_max_standby > i_su->si_curr_standby) &&
            ((i_su->sg_of_su->su_max_standby == 0) ||
            (i_su->sg_of_su->su_max_standby > i_su->si_curr_standby)))
         {
            /* found the SU assign the SI to the SU as standby */
            su_found = TRUE;            
            
         }
      }else /* if (o_susi != AVD_SU_SI_REL_NULL) */      
      {
         if (l_su != AVD_SU_NULL)
         {
            if ((l_su->si_max_standby > l_su->si_curr_standby) &&
                ((i_su->sg_of_su->su_max_standby == 0) ||
                (l_su->sg_of_su->su_max_standby > l_su->si_curr_standby)))
            {
               /* found the SU assign the SI to the SU as standby */
               su_found = TRUE;
               i_su = l_su;               
               
            }else /* if ((l_su->si_max_standby > l_su->si_curr_standby) &&
                    ((i_su->sg_of_su->su_max_standby == 0) ||
                   (l_su->sg_of_su->su_max_standby > l_su->si_curr_standby))) */
            {
               if ((new_su == TRUE) && (cnt < sg->pre_num_standby_su))
               {
                  i_su = l_su;
                  while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
                  {
                     if ((i_su->readiness_state == NCS_IN_SERVICE) &&
                        (i_su->list_of_susi == AVD_SU_SI_REL_NULL))
                     {
                        su_found = TRUE;
                        cnt ++;
                        continue;
                     }                     
                     
                     /* choose the next SU */
                     i_su = i_su->sg_list_su_next;
               
                  } /* while (i_su != AVD_SU_NULL) */
                  
                  l_su = i_su;
                  
               }/* if ((new_su == TRUE) && (cnt < sg->pre_num_standby_su)) */
               else if (cnt < sg->pre_num_standby_su)
               {
                  i_su = avd_sg_npm_su_next_asgn(cb,sg,l_su,SA_AMF_HA_STANDBY);
                  while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
                  {
                     cnt ++;               
                     if ((i_su->si_max_standby > i_su->si_curr_standby) &&
                         ((i_su->sg_of_su->su_max_standby == 0) ||
                         (i_su->sg_of_su->su_max_standby > i_su->si_curr_standby)))
                        /* Found the next assigning SU that can take SI. */
                        su_found = TRUE;
                     else if (cnt < sg->pre_num_standby_su)
                        i_su = avd_sg_npm_su_next_asgn(cb,sg,i_su,SA_AMF_HA_STANDBY);
                     else
                        i_su = AVD_SU_NULL;
                  }
                  
                  if ((su_found == FALSE) && 
                     (cnt < sg->pre_num_standby_su))
                  {
                     /* All the current standby SUs are full. The SG can have
                      * more standby assignments. identify the highest ranked
                      * in service unassigned SU in the SG.
                      */
                     new_su = TRUE;
                     if (n_su == AVD_SU_NULL)
                        i_su = sg->list_of_su;
                     else
                        i_su = n_su;
                     
                     while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
                     {
                        if ((i_su->readiness_state == NCS_IN_SERVICE) &&
                           (i_su->list_of_susi == AVD_SU_SI_REL_NULL))
                        {
                           su_found = TRUE;
                           cnt ++;
                           continue;
                        }                     
                        
                        /* choose the next SU */
                        i_su = i_su->sg_list_su_next;
                  
                     } /* while (i_su != AVD_SU_NULL) */

                     n_su = i_su;
                     
                  } /* if ((su_found == FALSE) && 
                           (cnt < sg->pre_num_standby_su)) */
                  
                  l_su = i_su;
               } /* else if (cnt < sg->pre_num_standby_su) */
               
            } /* else ((l_su->si_max_standby > l_su->si_curr_standby) &&
                ((i_su->sg_of_su->su_max_standby == 0) ||
                (l_su->sg_of_su->su_max_standby > l_su->si_curr_standby))) */
            
         } /* if (l_su != AVD_SU_NULL) */
         else if (cnt == 0)
         {
            /* Since we havent yet tried assigning any new standby SUs.
             * Identify the highest ranked standby assigning SU.  
             */
            
            i_su = avd_sg_npm_su_next_asgn(cb,sg,AVD_SU_NULL,SA_AMF_HA_STANDBY);
            while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
            {
               cnt ++;               
               if ((i_su->si_max_standby > i_su->si_curr_standby) &&
                  ((i_su->sg_of_su->su_max_standby == 0) ||
                  (i_su->sg_of_su->su_max_standby > i_su->si_curr_standby)))
                  /* Found the next assigning SU that can take SI. */
                  su_found = TRUE;
               else if (cnt < sg->pre_num_standby_su)
                  i_su = avd_sg_npm_su_next_asgn(cb,sg,i_su,SA_AMF_HA_STANDBY);
               else
                  i_su = AVD_SU_NULL;
            }
            
            if ((su_found == FALSE) && 
               (cnt < sg->pref_num_active_su))
            {
               /* All the current standby SUs are full. The SG can have
                * more standby assignments. identify the highest ranked
                * in service unassigned SU in the SG.
                */
               new_su = TRUE;
               if (n_su == AVD_SU_NULL)
                  i_su = sg->list_of_su;
               else
                  i_su = n_su;
               while ((i_su != AVD_SU_NULL) && (su_found == FALSE))
               {
                  if ((i_su->readiness_state == NCS_IN_SERVICE) &&
                     (i_su->list_of_susi == AVD_SU_SI_REL_NULL))
                  {
                     su_found = TRUE;
                     cnt ++;
                     continue;
                  }                     
                  
                  /* choose the next SU */
                  i_su = i_su->sg_list_su_next;
            
               } /* while (i_su != AVD_SU_NULL) */
               
               n_su = i_su;
               
            } /* if ((su_found == FALSE) && 
                     (cnt < sg->pref_num_active_su)) */
            
            l_su = i_su;

         } /* else if (cnt == 0) */
         
         if (su_found == FALSE)
         {
            /* since no su has enough space to take this SI exit the loop here.*/
            break;
         }
         
      } /* else (o_susi != AVD_SU_SI_REL_NULL) */
      
      if (su_found == TRUE)
      {
         if(avd_new_assgn_susi(cb,i_su,i_si,SA_AMF_HA_STANDBY, FALSE, &tmp_susi) == NCSCC_RC_SUCCESS)
         {
            /* Add the SU to the operation list */
            avd_sg_su_oper_list_add(cb,i_su,FALSE);
         }else
         {
            /* log a fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)i_si));
            m_AVD_LOG_INVALID_VAL_ERROR(((long)i_su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_si->name_net.value,i_si->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
         }
         
      }

      /* choose the next SI */
      i_si = i_si->sg_list_of_si_next;      

   } /* while (i_si != AVD_SI_NULL) */
   

   return sg->su_oper_list.su;
}


/*****************************************************************************
 * Function: avd_sg_npm_si_func
 *
 * Purpose:  This function is called when a new SI is added to a SG. The SG is
 * of type N+M redundancy model. This function will perform the functionality
 * described in the SG FSM design. 
 *
 * Input: cb - the AVD control block
 *        si - The pointer to the service instance.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a N+M redundancy model specific function. If there are
 * any SIs being transitioned due to operator, this call will return
 * failure.
 *
 * 
 **************************************************************************/

uns32 avd_sg_npm_si_func(AVD_CL_CB *cb,AVD_SI *si)
{
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_si_func");
   m_AVD_LOG_RCVD_VAL(((long)si));
   
   m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
   
   /* If the SG FSM state is not stable just return success. */
   if (si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE)
   {
      return NCSCC_RC_SUCCESS;
   }
   
   if((cb->init_state != AVD_APP_STATE) &&
      (si->sg_of_si->sg_ncs_spec == SA_FALSE))
   {
      m_AVD_LOG_INVALID_VAL_ERROR(si->sg_of_si->sg_ncs_spec);
      return NCSCC_RC_SUCCESS;
   }
   
   if (avd_sg_npm_su_chose_asgn(cb,si->sg_of_si) == AVD_SU_NULL)
   {
      /* all the assignments have already been done in the SG. */
      return NCSCC_RC_SUCCESS;
   }
   
   /* change the FSM state */   
   m_AVD_SET_SG_FSM(cb,(si->sg_of_si),AVD_SG_FSM_SG_REALIGN);
   
   m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
 
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: avd_sg_npm_siswitch_func
 *
 * Purpose:  This function is called when a operator does a SI switch on
 * a SI that belongs to N+M redundancy model SG. 
 * This will trigger a role change action as described in the SG FSM design.
 *
 * Input: cb - the AVD control block
 *        si - The pointer to the SI that needs to be switched.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This is a N+M redundancy model specific function. The initial 
 * functionality of N+M SI switch is same as 2N switch so it just calls that
 * function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_npm_siswitch_func(AVD_CL_CB *cb,AVD_SI *si)
{  
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_siswitch_func");
   m_AVD_LOG_RCVD_VAL(((long)si));
   
   m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
   
   return avd_sg_2n_siswitch_func(cb,si);
}
 

 /*****************************************************************************
 * Function: avd_sg_npm_su_fault_su_oper
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_su_fault_func.
 * It is called if the SG is in AVD_SG_FSM_SU_OPER.
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

static uns32 avd_sg_npm_su_fault_su_oper(AVD_CL_CB *cb,AVD_SU *su)
{
   NCS_BOOL flag;
   AVD_AVND *su_node_ptr = NULL;
   
   if (su->sg_of_su->su_oper_list.su == su)
   {
      /* SU is in the SU oper list. If the admin state of the SU is 
       * shutdown change it to lock. Send a D2N-INFO_SU_SI_ASSIGN message
       * modify quiesced all message to the SU.
       */
      if (su->list_of_susi->state == SA_AMF_HA_QUIESCING)
      {
         if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
         {
            /* log a fatal error*/
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }

         m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

         if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
         {               
            m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
         }else if (su_node_ptr->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
         {
            m_AVD_IS_NODE_LOCK((su_node_ptr),flag);
            if (flag == TRUE)
            {
               m_AVD_SET_AVND_SU_ADMIN(cb,(su_node_ptr),NCS_ADMIN_STATE_LOCK);
            }
         }
      }
      
   }else /* if (su->sg_of_su->su_oper_list.su == su) */
   {
      /* SU not in the SU oper list. */
      
      if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
      {
         /* The SU has active SI assignments. send quiesced all message to 
          * the SU. Add the SU to the oper list. Change the state to SG realign.
          */
         if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
         {
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,su,FALSE);
         
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
         
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         
      }else /* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      {
         /* The SU has standby SI assignments. send D2N-INFO_SU_SI_ASSIGN
          * message remove all to the SU. Add this SU to the oper list.
          * Change the state to SG realign.
          */
         if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         avd_sg_su_oper_list_add(cb,su,FALSE);
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         
      } /* else (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      
   } /* else (su->sg_of_su->su_oper_list.su == su) */
 
   return NCSCC_RC_SUCCESS;
}

 /*****************************************************************************
 * Function: avd_sg_npm_su_fault_si_oper
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_su_fault_func.
 * It is called if the SG is in AVD_SG_FSM_SI_OPER or 
 * SG is in AVD_SG_FSM_SG_REALIGN state with a SI in the SI admin pointer.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the service unit.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This function is called in AVD_SG_FSM_SG_REALIGN also.
 *
 * 
 **************************************************************************/

static uns32 avd_sg_npm_su_fault_si_oper(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU_SI_REL  *l_susi;
   NCS_BOOL  l_flag;
   AVD_SU *o_su;
   
   if ((l_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
      == AVD_SU_SI_REL_NULL)
   {
      /* SI doesn't have any assignment with the SU */
      m_AVD_CHK_OPLIST(su,l_flag);
      if (l_flag == TRUE)
      {
         /* the SU is in the SU oper list. So no action return success. */               
         return NCSCC_RC_SUCCESS;
      }
      
      if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
      {
         /* The SU has active SI assignments. send D2N-INFO_SU_SI_ASSIGN 
          * message modify quiesced all message to the SU. Add the SU to 
          * the oper list.
          */
         if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
         {
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,su,FALSE);
         if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
         {
            /*It is in SI oper state change state to sg_realign */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }         
         
      }else /* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      {
         /* The SU has standby SI assignments. send D2N-INFO_SU_SI_ASSIGN
          * message remove all to the SU. Add this SU to the oper list.
          */
         if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,su,FALSE);
         if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
         {
            /*It is in SI oper state change state to sg_realign */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
         
      } /* else (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
   }/* if ((l_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
      == AVD_SU_SI_REL_NULL) */
   else if (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH)
   {
      /* the admin pointer SI has switch operation as TRUE */
      
      if (l_susi->state == SA_AMF_HA_QUIESCED)
      {
         if (l_susi->fsm == AVD_SU_SI_STATE_ASGND)
         {
            /* This SI relation with the SU is quiesced. 
             * Change the SI switch state to false. Remove the SI from 
             * the SI admin pointer. send a D2N -INFO_SU_SI_ASSIGN remove
             * all to the quiesced SU.  Add the SU to the SU 
             * operation list.  Identify the other SU assigned to this SI,
             * add it to the SU oper list.
             */
            m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
            if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
            {
               /* log fatal error */
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            avd_sg_su_oper_list_add(cb,su,FALSE);                  
            if (su->sg_of_su->admin_si->list_of_sisu == l_susi)
            {
               o_su = su->sg_of_su->admin_si->list_of_sisu->si_next->su;
            }else
            {
               o_su = su->sg_of_su->admin_si->list_of_sisu->su;
            }
            m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
            avd_sg_su_oper_list_add(cb,o_su,FALSE);
            
            if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
            {
               /*It is in SI oper state change state to sg_realign */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            }
            
         }/* if (l_susi->fsm == AVD_SU_SI_STATE_ASGND) */
         else if (l_susi->fsm == AVD_SU_SI_STATE_MODIFY)
         {
            /* This SI relation with the SU is quiesced assigning.
             * Change the SI switch state to false. Remove the SI from 
             * the SI admin pointer. Add the SU to the SU operation list.
             */
            m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
            m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
            avd_sg_su_oper_list_add(cb,su,FALSE);
            if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
            {
               /*It is in SI oper state change state to su_oper */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SU_OPER);            
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            }
         } /* else if (l_susi->fsm == AVD_SU_SI_STATE_MODIFY) */
      } /* if (l_susi->state == SA_AMF_HA_QUIESCED) */
      else if ((l_susi->state == SA_AMF_HA_STANDBY) && 
            (l_susi->fsm == AVD_SU_SI_STATE_ASGND))
      {
         /* This SI relation with the SU is standby assigned.
          * Send D2N-INFO_SU_SI_ASSIGN with remove all to this SU. 
          * Add this SU to operation list.
          */
         
         if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,su,FALSE);
         if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
         {
            /*It is in SI oper state change state to sg_realign */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }         
      }/*else if ((l_susi->state == SA_AMF_HA_STANDBY) && 
            (l_susi->fsm == AVD_SU_SI_STATE_ASGND)) */
      else if (l_susi->state == SA_AMF_HA_ACTIVE)
      {
         /* This SI relation with the SU is active assigning and the 
          * other SU is quiesced assigned. Send D2N-INFO_SU_SI_ASSIGN 
          * with modified quiesced all to this SU.
          */
         if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
         {
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
         {
            /*It is in SI oper state change state to sg_realign */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
         
      }/* else if (l_susi->state == SA_AMF_HA_ACTIVE) */
      
   } /* else if (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) */
   else if (su->sg_of_su->admin_si->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* The admin pointer SI admin operation is LOCK/shutdown */
      if ((l_susi->state == SA_AMF_HA_QUIESCED) || 
         (l_susi->state == SA_AMF_HA_QUIESCING))
      {
         /* This SI relation with the SU is quiesced/quiescing assigning.
          * Send D2N-INFO_SU_SI_ASSIGN with modified quiesced all to this
          * SU. Add the SU to the SU operation list. Mark the SI admin 
          * state UNLOCK. Remove the SI from the admin pointer.
          */
         if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
         {
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,su,FALSE);
         m_AVD_SET_SI_ADMIN(cb,(su->sg_of_su->admin_si),NCS_ADMIN_STATE_UNLOCK);
         m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
         
         if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
         {
            /*It is in SI oper state change state to su_oper */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SU_OPER);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
         
      }/* if ((l_susi->state == SA_AMF_HA_QUIESCED) || 
         (l_susi->state == SA_AMF_HA_QUIESCING)) */
      else if ((l_susi->state == SA_AMF_HA_STANDBY) && 
            (l_susi->fsm == AVD_SU_SI_STATE_ASGND))
      {
         /* The SI has standby HA state w.r.t the SU. 
          * Send D2N-INFO_SU_SI_ASSIGN with remove all to the SU. 
          * Add the SU to the SU oper list.
          */
         if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,su,FALSE);
         if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SI_OPER)
         {
            /*It is in SI oper state change state to sg_realign */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
      } /* if ((l_susi->state == SA_AMF_HA_STANDBY) && 
            (l_susi->fsm == AVD_SU_SI_STATE_ASGND)) */
      
   }/* else if (su->sg_of_su->admin_si->admin_state != NCS_ADMIN_STATE_UNLOCK) */

   
   return NCSCC_RC_SUCCESS;

}



 /*****************************************************************************
 * Function: avd_sg_npm_su_fault_sg_relgn
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_su_fault_func.
 * It is called if the SG is in AVD_SG_FSM_SG_REALIGN.
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

static uns32 avd_sg_npm_su_fault_sg_relgn(AVD_CL_CB *cb,AVD_SU *su)
{
   NCS_BOOL l_flag = FALSE,flag = FALSE;
   AVD_AVND *su_node_ptr = NULL;
   
   if (su->sg_of_su->admin_si != AVD_SI_NULL)
   {
      return avd_sg_npm_su_fault_si_oper(cb,su);
      
   } /* if ((su->sg_of_su->admin_si != AVD_SI_NULL) */
   else
   {
      /* SI is not undergoing any admin semantics. */
      m_AVD_CHK_OPLIST(su,l_flag);
      if (l_flag == TRUE)
      {
         /* SU is in the SU oper list. If the admin state of the SU is 
          * shutdown change it to lock. Send a D2N-INFO_SU_SI_ASSIGN message
          * modify quiesced all message to the SU. if the SU has standby 
          * SI assignments. send D2N -INFO_SU_SI_ASSIGN message remove all to
          * the SU.  if the SU has active SI assignments 
          * send D2N-INFO_SU_SI_ASSIGN message modify quiesced all message to
          * the SU.
          */
         if (su->list_of_susi->state == SA_AMF_HA_QUIESCING)
         {
            if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
            {
               /* log a fatal error*/
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            
            m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

            if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
            {               
               m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
            }else if (su_node_ptr->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
            {
               m_AVD_IS_NODE_LOCK((su_node_ptr),flag);
               if (flag == TRUE)
               {
                  m_AVD_SET_AVND_SU_ADMIN(cb,(su_node_ptr),NCS_ADMIN_STATE_LOCK);
               }
            }
         } /* if (su->list_of_susi->state == SA_AMF_HA_QUIESCING) */
         else if ((su->list_of_susi->state == SA_AMF_HA_ACTIVE) || 
            ((su->list_of_susi->su_next != AVD_SU_SI_REL_NULL) && 
            (su->list_of_susi->su_next->state == SA_AMF_HA_ACTIVE)))
         {
            if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
            {
               /* log a fatal error*/
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
         }/* else if ((su->list_of_susi->state == SA_AMF_HA_ACTIVE) || 
            ((su->list_of_susi->su_next != AVD_SU_SI_REL_NULL) && 
            (su->list_of_susi->su_next->state == SA_AMF_HA_ACTIVE))) */
         else if (((su->list_of_susi->state == SA_AMF_HA_STANDBY) && 
            (su->list_of_susi->fsm != AVD_SU_SI_STATE_UNASGN)) || 
            ((su->list_of_susi->su_next != AVD_SU_SI_REL_NULL) && 
            (su->list_of_susi->su_next->state == SA_AMF_HA_STANDBY)))
         {
            if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
            {
               /* log fatal error */
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
         }/* else if (((su->list_of_susi->state == SA_AMF_HA_STANDBY) && 
            (su->list_of_susi->fsm == AVD_SU_SI_STATE_ASGND)) || 
            ((su->list_of_susi->su_next != AVD_SU_SI_REL_NULL) && 
            (su->list_of_susi->su_next->state == SA_AMF_HA_STANDBY))) */
         
      }else /* if (l_flag == TRUE) */
      {
         /* SU not in the SU oper list. */
         
         if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
         {
            /* The SU has active SI assignments. send D2N-INFO_SU_SI_ASSIGN 
             * message modify quiesced all message to the SU. Add the SU to 
             * the oper list.
             */
            if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
            {
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            avd_sg_su_oper_list_add(cb,su,FALSE);
            
         }else /* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
         {
            /* The SU has standby SI assignments. send D2N-INFO_SU_SI_ASSIGN
             * message remove all to the SU. Add this SU to the oper list.
             */
            if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
            {
               /* log fatal error */
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            avd_sg_su_oper_list_add(cb,su,FALSE);
            
         } /* else (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
         
      } /* else (l_flag == TRUE) */
      
   } /* else ((su->sg_of_su->admin_si != AVD_SI_NULL) */

   return NCSCC_RC_SUCCESS;
}

 /*****************************************************************************
 * Function: avd_sg_npm_su_fault_func
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

uns32 avd_sg_npm_su_fault_func(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU_SI_REL *a_susi;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_su_fault_func");
   m_AVD_LOG_RCVD_VAL(((long)su));
   
   m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
   
   if(su->list_of_susi == AVD_SU_SI_REL_NULL)
      return NCSCC_RC_SUCCESS;
   
   /* Do the functionality based on the current state.*/
   switch(su->sg_of_su->sg_fsm_state)
   {
   case AVD_SG_FSM_STABLE:
      
      if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
      {
         /* this is a active SU. */
         /* change the state for all assignments to quiesced. */
         if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
         {
            /* log a fatal error*/
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         /* add the SU to the operation list and change the SG FSM to SU operation. */
         avd_sg_su_oper_list_add(cb,su,FALSE);   
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SU_OPER);
         
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         
      }else /* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      {
         /* means standby */
         /* change the state for all assignments to unassign and send delete mesage. */
         if(avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         /* add the SU to the operation list and change the SG FSM to SG realign. */
         avd_sg_su_oper_list_add(cb,su,FALSE);   
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
         
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         
      } /* else (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      break; /* case AVD_SG_FSM_STABLE: */
   case AVD_SG_FSM_SG_REALIGN:
     
      if (avd_sg_npm_su_fault_sg_relgn(cb,su) == NCSCC_RC_FAILURE)
         return NCSCC_RC_FAILURE;
      
      break; /* case AVD_SG_FSM_SG_REALIGN: */
   case AVD_SG_FSM_SU_OPER:
      
      if (avd_sg_npm_su_fault_su_oper(cb,su) == NCSCC_RC_FAILURE)
         return NCSCC_RC_FAILURE;
      
      break; /* case AVD_SG_FSM_SU_OPER: */
   case AVD_SG_FSM_SI_OPER:
   
      if (avd_sg_npm_su_fault_si_oper(cb,su) == NCSCC_RC_FAILURE)
         return NCSCC_RC_FAILURE;
      
      break; /* case AVD_SG_FSM_SI_OPER: */
   case AVD_SG_FSM_SG_ADMIN:      

      if (su->sg_of_su->admin_state == NCS_ADMIN_STATE_LOCK)
      {
         /* the SG is lock no action. */
         return NCSCC_RC_SUCCESS;
      }else if (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         /* the SI relationships to the SU is quiescing. Send   
          * D2N-INFO_SU_SI_ASSIGN modify quiesced all to the SU. Identify its
          * standby SU assignment send D2N-INFO_SU_SI_ASSIGN remove all for the
          * SU and add it to the SU operlist. 
          */
         if (su->list_of_susi->state == SA_AMF_HA_QUIESCING)
         {
            a_susi = avd_sg_npm_su_othr(cb,su);
            if (avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) == NCSCC_RC_FAILURE)
            {
               /* log fatal error */
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            
            if ((a_susi != AVD_SU_SI_REL_NULL) && 
               (a_susi->fsm == AVD_SU_SI_STATE_ASGND))
            {
               avd_sg_su_si_del_snd(cb,a_susi->su);
               avd_sg_su_oper_list_add(cb,a_susi->su,FALSE);
            }
         } /* if (su->list_of_susi->state == SA_AMF_HA_QUIESCING) */
         else if (su->list_of_susi->fsm == AVD_SU_SI_STATE_ASGND)
         {
             /* the SI relationships to the SU is standby Send 
              * D2N-INFO_SU_SI_ASSIGN remove all for the SU and 
              * add it to the SU operlist.
              */
            if (avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
            {
               /* log error */
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            avd_sg_su_oper_list_add(cb,su,FALSE);
            
         } /* else (su->list_of_susi->state == SA_AMF_HA_QUIESCING) */         
      }else
      {
         /* log fatal error */
         m_AVD_LOG_INVALID_VAL_ERROR(su->sg_of_su->admin_state);
         return NCSCC_RC_FAILURE;
      }
     
      break; /* case AVD_SG_FSM_SG_ADMIN: */   
   default:
      /* log fatal error about the invalid value */
      m_AVD_LOG_INVALID_VAL_FATAL(su->sg_of_su->sg_fsm_state);
      return NCSCC_RC_FAILURE;
      break;
   } /* switch(su->sg_of_su->sg_fsm_state) */

   return NCSCC_RC_SUCCESS;
}

 /*****************************************************************************
 * Function: avd_sg_npm_su_insvc_func
 *
 * Purpose:  This function is called when a SU readiness state changes
 * to inservice from out of service. The SG is of type N+M redundancy
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

uns32 avd_sg_npm_su_insvc_func(AVD_CL_CB *cb,AVD_SU *su)
{

   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_su_insvc_func");
   m_AVD_LOG_RCVD_VAL(((long)su));
   
   m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
   
   /* An SU will not become in service when the SG is being locked or shutdown.
    */
   if (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SG_ADMIN)
   {
      m_AVD_LOG_INVALID_VAL_FATAL(su->sg_of_su->sg_fsm_state);
      return NCSCC_RC_FAILURE;
   }
   
   /* If the SG FSM state is not stable just return success. */
   if (su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE)
   {
      return NCSCC_RC_SUCCESS;
   }   
   
   if((cb->init_state != AVD_APP_STATE) &&
      (su->sg_of_su->sg_ncs_spec == SA_FALSE))
   {
      return NCSCC_RC_SUCCESS;
   }
   
   if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
   {
      avd_sg_app_su_inst_func(cb,su->sg_of_su);

      /* all the assignments have already been done in the SG. */
      return NCSCC_RC_SUCCESS;
   }
   
   /* change the FSM state */   
   m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN); 
   m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
   return NCSCC_RC_SUCCESS;

}

 /*****************************************************************************
 * Function: avd_sg_npm_susi_sucss_sg_reln
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_susi_sucss_func.
 * It is called if the SG is in AVD_SG_FSM_SG_REALIGN.
 *
 * Input: cb - the AVD control block
 *        su - In case of entire SU related operation the SU for
 *               which the ack is received.
 *        susi - The pointer to the service unit service instance relationship.
 *        act  - The action received in the ack message.
 *        state - The HA state in the message.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static uns32 avd_sg_npm_susi_sucss_sg_reln(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state)
{
   AVD_SU_SI_REL *i_susi, *o_susi;
   NCS_BOOL flag;
   AVD_SU_SI_STATE old_fsm_state;
   AVD_AVND *su_node_ptr = NULL;
   
   
   if (susi == AVD_SU_SI_REL_NULL)
   {
      /* assign all */
      if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_QUIESCED))
      {
         /* quiesced all */
         
         if (su->sg_of_su->admin_si != AVD_SI_NULL)
         {
            /* SI in admin pointer */
            if ((su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH)
               && ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
               != AVD_SU_SI_REL_NULL))
            {
               /* SI swith state is true and the SI has relation to this SU. */
               if (i_susi->si->list_of_sisu != i_susi)
               {
                  o_susi = i_susi->si->list_of_sisu;
               } else
               {
                  o_susi = i_susi->si_next;
               }
               if(o_susi == AVD_SU_SI_REL_NULL)
               {
                  /* No standby SU, send D2N-INFO_SU_SI_ASSIGN message 
                   * modify active all assignment to this quiesced SU, remove 
                   * the SI from the SI admin pointer, change the switch value 
                   * to false. Add the SU to SU operlist. 
                   */
                  if (avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_ACTIVE) == NCSCC_RC_FAILURE)
                  {
                     /* log fatal error */
                     m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     return NCSCC_RC_FAILURE;
                  }
                  m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
                  avd_sg_su_oper_list_add(cb,su,FALSE);
                  m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
                  
               }/* if(o_susi == AVD_SU_SI_REL_NULL)*/
               else if ((o_susi->state == SA_AMF_HA_STANDBY) &&
                     (o_susi->su->readiness_state == NCS_IN_SERVICE))
               {
                  /* The standby SU is inservice has standby SI assignment from
                   * another SU, send remove message for each of those SI 
                   * assignment to the standby SU and send D2N-INFO_SU_SI_ASSIGN
                   * message modify active all assignments to that SU.
                   */
                  avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
               }else if ((o_susi->state == SA_AMF_HA_QUIESCED) &&
                     (o_susi->su->readiness_state == NCS_IN_SERVICE))
               {
                  /* This SU's SIs have quiesced assignment with another SU.
                   * which is inservice send D2N -INFO_SU_SI_ASSIGN message 
                   * modify active all assignments to that SU Add the SU to 
                   * oper list. Send D2N -INFO_SU_SI_ASSIGN message remove all 
                   * to this SU. Add this SU to the SU oper list. Remove the 
                   * SI from admin pointer. Set the SI switch state to false.
                   */
                  if (avd_sg_su_si_mod_snd(cb,o_susi->su,SA_AMF_HA_ACTIVE) == NCSCC_RC_FAILURE)
                  {
                     /* log fatal error */
                     m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     return NCSCC_RC_FAILURE;
                  }
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  avd_sg_su_si_del_snd(cb,su);
                  avd_sg_su_oper_list_add(cb,su,FALSE);
                  m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
                  m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
                  
               }/* if ((o_susi->state == SA_AMF_HA_QUIESCED) &&
                     (o_susi->su->readiness_state == NCS_IN_SERVICE)) */

            } else /* if ((su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH)
               && ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
               != AVD_SU_SI_REL_NULL)) */
            
            {
               m_AVD_CHK_OPLIST(su,flag);
   
               if (flag == FALSE)
               {
                  /* Log fatal error  */         
                  m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
                  return NCSCC_RC_FAILURE;
               }
               
               /* SU in the oper list */
               o_susi = avd_sg_npm_su_othr(cb,su);
               if ((o_susi != AVD_SU_SI_REL_NULL) &&
                  (o_susi->su->readiness_state == NCS_IN_SERVICE) &&
                   (o_susi->state == SA_AMF_HA_STANDBY) &&
                  (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
               {
                  /* The standby SU w.r.t this SU is inservice has standby 
                   * SI assignment from another SU, send remove message for 
                   * each of those SI assignment to the standby SU and 
                   * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                   * assignments to that SU. Add the SU to the oper list.                   
                   */
                  avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }
               
               /* Send D2N -INFO_SU_SI_ASSIGN message remove all to this SU. */
               avd_sg_su_si_del_snd(cb,su);
               
            } /* else  ((su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH)
               && ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
               != AVD_SU_SI_REL_NULL)) */
            
         }else /* if (su->sg_of_su->admin_si != AVD_SI_NULL) */
         {
            m_AVD_CHK_OPLIST(su,flag);
   
            if (flag == FALSE)
            {
               /* Log fatal error  */         
               m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            
            /* SU in the oper list */
            o_susi = avd_sg_npm_su_othr(cb,su);
            if ((o_susi != AVD_SU_SI_REL_NULL) &&
               (o_susi->su->readiness_state == NCS_IN_SERVICE) &&
                (o_susi->state == SA_AMF_HA_STANDBY) &&
               (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
            {
               /* The standby SU w.r.t this SU is inservice has standby 
                * SI assignment from another SU, send remove message for 
                * each of those SI assignment to the standby SU and 
                * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                * assignments to that SU. Add the SU to the oper list.                   
                */
               avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
            }
            
            /* Send D2N-INFO_SU_SI_ASSIGN message remove all to this SU. If
             * the SUs admin state is shutdown change it to LOCK.
             */
            m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

            if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
            {               
               m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
            }else if (su_node_ptr->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
            {
               m_AVD_IS_NODE_LOCK((su_node_ptr),flag);
               if (flag == TRUE)
               {
                  m_AVD_SET_AVND_SU_ADMIN(cb,(su_node_ptr),NCS_ADMIN_STATE_LOCK);
               }
            }
            avd_sg_su_si_del_snd(cb,su);
            
         } /* else (su->sg_of_su->admin_si != AVD_SI_NULL) */ 
         
      } /* if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_QUIESCED)) */
      else if (act == AVSV_SUSI_ACT_DEL)
      {
         /* remove all */
         if (su->list_of_susi->state == SA_AMF_HA_QUIESCED)
         {
            o_susi = avd_sg_npm_su_othr(cb,su);
            
            if ((o_susi != AVD_SU_SI_REL_NULL) &&
               (o_susi->su->readiness_state == NCS_IN_SERVICE) &&
                (o_susi->state == SA_AMF_HA_STANDBY) &&
                (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
            {
               /* The standby SU w.r.t this SU is inservice has standby 
                * SI assignment from another SU, send remove message for 
                * each of those SI assignment to the standby SU and 
                * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                * assignments to that SU. Add the SU to the oper list.                   
                */
               avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
            }
         }
         
         /* free all the SUSI relationships to this SU. remove this SU from the
          * SU oper list.SU in the operation list 
          */
         avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
         avd_sg_su_oper_list_del(cb,su,FALSE);
         
         if ((su->sg_of_su->su_oper_list.su == AVD_SU_NULL) &&
            (su->sg_of_su->admin_si == AVD_SI_NULL))
         {
            /* Both the SI admin pointer and SU oper list are empty. Do the
             * functionality as in stable state to verify if new assignments 
             * can be done. If yes stay in the same state. If no 
             * new assignments change state to stable.
             */
            if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
            {
               /* all the assignments have already been done in the SG. */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               avd_sg_app_su_inst_func(cb,su->sg_of_su);
            }
         }        
         
      } /* if (act == AVSV_SUSI_ACT_DEL) */
      else if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_ACTIVE))
      {
         for (i_susi = su->list_of_susi; 
            (i_susi != AVD_SU_SI_REL_NULL) && (i_susi->fsm == AVD_SU_SI_STATE_ASGND);
            i_susi = i_susi->su_next);
         
         if (i_susi == AVD_SU_SI_REL_NULL)
         {
            /* active all and the entire SU is assigned. */
            if ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
               (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
  ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
            != AVD_SU_SI_REL_NULL))
            {
               /* SI in admin pointer with switch state true and SI has 
                * relationship with this SU. Send D2N-INFO_SU_SI_ASSIGN with 
                * modify standby all to the Quiesced SU. Remove the SI from 
                * admin pointer and add the quiesced SU to the SU oper list.
                */
               if (su->sg_of_su->admin_si->list_of_sisu == i_susi)
               {
                  o_susi = i_susi->si_next;
               }else
               {
                  o_susi = su->sg_of_su->admin_si->list_of_sisu;
               }
               
               if (avd_sg_su_si_mod_snd(cb,o_susi->su,SA_AMF_HA_STANDBY) == NCSCC_RC_FAILURE)
               {
                  /* log fatal error */
                  m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                  return NCSCC_RC_FAILURE;
               }
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
               m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
               
            }/* if ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
               (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
  ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
            != AVD_SU_SI_REL_NULL)) */
            else
            {
               /* Remove the SU from the SU oper list. */
               avd_sg_su_oper_list_del(cb,su,FALSE);         
               if ((su->sg_of_su->su_oper_list.su == AVD_SU_NULL) &&
                  (su->sg_of_su->admin_si == AVD_SI_NULL))
               {
                  /* Both the SI admin pointer and SU oper list are empty. Do the
                   * functionality as in stable state to verify if new assignments 
                   * can be done. If yes stay in the same state. If no 
                   * new assignments change state to stable.
                   */
                  if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
                  {
                     /* all the assignments have already been done in the SG. */
                     m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                     m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                     avd_sg_app_su_inst_func(cb,su->sg_of_su);
                  }
               }                 
               
            } /* ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
               (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
  ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
            != AVD_SU_SI_REL_NULL)) */
         } /* if (i_susi == AVD_SU_SI_REL_NULL) */
         
      } /* if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_ACTIVE)) */
      else if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_STANDBY))
      {
         /* standby all for a SU. remove the SU from the SU oper list.*/
         avd_sg_su_oper_list_del(cb,su,FALSE);
         
         if ((su->sg_of_su->su_oper_list.su == AVD_SU_NULL) &&
            (su->sg_of_su->admin_si == AVD_SI_NULL))
         {
            /* Both the SI admin pointer and SU oper list are empty. Do the
             * functionality as in stable state to verify if new assignments 
             * can be done. If yes stay in the same state. If no 
             * new assignments change state to stable.
             */
            if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
            {
               /* all the assignments have already been done in the SG. */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               avd_sg_app_su_inst_func(cb,su->sg_of_su);
            }
         }   
         
      } /* else if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_STANDBY)) */

   } /* if (susi == AVD_SU_SI_REL_NULL) */
   else 
   {
      /* assign single SUSI */
      if (act == AVSV_SUSI_ACT_DEL)
      {
         /* remove for single SU SI. Free the SUSI relationship. If all the 
          * relationships are in the assigned state, remove the SU from the 
          * operation list and do the realignment of the SG.
          */
         /* free all the CSI assignments  */
         avd_compcsi_list_del(cb,susi,FALSE);
         /* Unassign the SUSI */
         m_AVD_SU_SI_TRG_DEL(cb,susi);
         
         for (i_susi = su->list_of_susi; 
            (i_susi != AVD_SU_SI_REL_NULL) && (i_susi->fsm == AVD_SU_SI_STATE_ASGND);
            i_susi = i_susi->su_next);
         
         if (i_susi == AVD_SU_SI_REL_NULL)
         {
            if ((su->list_of_susi != AVD_SU_SI_REL_NULL) && 
               (su->list_of_susi->state == SA_AMF_HA_ACTIVE))
            {
               /* active and the entire SU is assigned. */
               if ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
                  (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
     ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
               != AVD_SU_SI_REL_NULL))
               {
                  /* SI in admin pointer with switch state true and SI has 
                   * relationship with this SU. Send D2N-INFO_SU_SI_ASSIGN with 
                   * modify standby all to the Quiesced SU. Remove the SI from 
                   * admin pointer and add the quiesced SU to the SU oper list.
                   */
                  if (su->sg_of_su->admin_si->list_of_sisu == i_susi)
                  {
                     o_susi = i_susi->si_next;
                  }else
                  {
                     o_susi = su->sg_of_su->admin_si->list_of_sisu;
                  }
                  
                  if (avd_sg_su_si_mod_snd(cb,o_susi->su,SA_AMF_HA_STANDBY) == NCSCC_RC_FAILURE)
                  {
                     /* log fatal error */
                     m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     return NCSCC_RC_FAILURE;
                  }
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
                  m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
                  
               }/* if ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
                  (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
     ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
               != AVD_SU_SI_REL_NULL)) */
               else
               {
                  /* Remove the SU from the SU oper list. */
                  avd_sg_su_oper_list_del(cb,su,FALSE);         
                  if ((su->sg_of_su->su_oper_list.su == AVD_SU_NULL) &&
                     (su->sg_of_su->admin_si == AVD_SI_NULL))
                  {
                     /* Both the SI admin pointer and SU oper list are empty. Do the
                      * functionality as in stable state to verify if new assignments 
                      * can be done. If yes stay in the same state. If no 
                      * new assignments change state to stable.
                      */
                     if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
                     {
                        /* all the assignments have already been done in the SG. */
                        m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                        m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                        avd_sg_app_su_inst_func(cb,su->sg_of_su);
                     }
                  }                 
                  
               } /* ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
                  (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
     ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
               != AVD_SU_SI_REL_NULL)) */
               
            } /* if ((su->list_of_susi != AVD_SU_SI_REL_NULL) && 
               (su->list_of_susi->state == SA_AMF_HA_ACTIVE)) */
            else
            {
               /* Remove the SU from the SU oper list. */
               avd_sg_su_oper_list_del(cb,su,FALSE);         
               if ((su->sg_of_su->su_oper_list.su == AVD_SU_NULL) &&
                  (su->sg_of_su->admin_si == AVD_SI_NULL))
               {
                  /* Both the SI admin pointer and SU oper list are empty. Do the
                   * functionality as in stable state to verify if new assignments 
                   * can be done. If yes stay in the same state. If no 
                   * new assignments change state to stable.
                   */
                  if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
                  {
                     /* all the assignments have already been done in the SG. */
                     m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                     m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                     avd_sg_app_su_inst_func(cb,su->sg_of_su);
                  }
               }         
            } /* else ((su->list_of_susi != AVD_SU_SI_REL_NULL) && 
               (su->list_of_susi->state == SA_AMF_HA_ACTIVE)) */
               
         } /* if (i_susi == AVD_SU_SI_REL_NULL) */
         
      } /* if (act == AVSV_SUSI_ACT_DEL) */
      else if ((state == SA_AMF_HA_ACTIVE) || (state == SA_AMF_HA_STANDBY))
      {
         for (i_susi = su->list_of_susi; 
            (i_susi != AVD_SU_SI_REL_NULL) && (i_susi->fsm == AVD_SU_SI_STATE_ASGND);
            i_susi = i_susi->su_next);
         
         if (i_susi == AVD_SU_SI_REL_NULL)
         {
            /* active or standby for a SUSI and the entire SU is assigned. 
             * remove the SU from the SU oper list.
             */
            avd_sg_su_oper_list_del(cb,su,FALSE);
            
            if ((su->sg_of_su->su_oper_list.su == AVD_SU_NULL) &&
               (su->sg_of_su->admin_si == AVD_SI_NULL))
            {
               /* Both the SI admin pointer and SU oper list are empty. Do the
                * functionality as in stable state to verify if new assignments 
                * can be done. If yes stay in the same state. If no 
                * new assignments change state to stable.
                */
               if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
               {
                  /* all the assignments have already been done in the SG. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                  avd_sg_app_su_inst_func(cb,su->sg_of_su);
               }
            }   
         } /* if (i_susi == AVD_SU_SI_REL_NULL) */
         
      } /* if ((state == SA_AMF_HA_ACTIVE) || (state == SA_AMF_HA_STANDBY)) */
      else if ((state == SA_AMF_HA_QUIESCED) && 
               (susi->si == su->sg_of_su->admin_si) &&
               (NCS_ADMIN_STATE_UNLOCK != su->sg_of_su->admin_si->admin_state))
      {
         /* Message with modified quiesced for a SI in the admin pointer with.
          * admin values lock/shutdown. Send a D2N-INFO_SU_SI_ASSIGN with remove
          * for this SI to this SU, Add this SU to the SU oper list. 
          * Change admin state to lock for this SI. Identify its standby SU 
          * assignment, if the standby SU is not in the SU oper list. 
          * send D2N-INFO_SU_SI_ASSIGN remove for only this SI and Add the 
          * standby SU to the SU oper list. Remove the SI from the 
          * SI admin pointer.
          */
         old_fsm_state = susi->fsm;
         susi->fsm = AVD_SU_SI_STATE_UNASGN;  
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);       
         if (avd_snd_susi_msg(cb,susi->su,susi,AVSV_SUSI_ACT_DEL)
            == NCSCC_RC_FAILURE)
         {
            /* Log a fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->su->name_net.value,susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->si->name_net.value,susi->si->name_net.length);
            susi->fsm = old_fsm_state;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,susi->su,FALSE);
         m_AVD_SET_SI_ADMIN(cb,(su->sg_of_su->admin_si),NCS_ADMIN_STATE_LOCK);
         if (su->sg_of_su->admin_si->list_of_sisu == susi)
            i_susi = susi->si_next;
         else
            i_susi = su->sg_of_su->admin_si->list_of_sisu;
         
         if ((i_susi != AVD_SU_SI_REL_NULL) &&
            (i_susi->fsm != AVD_SU_SI_STATE_UNASGN))
         {
            i_susi->fsm = AVD_SU_SI_STATE_UNASGN;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);            
            avd_snd_susi_msg(cb,i_susi->su,i_susi,AVSV_SUSI_ACT_DEL);
            avd_sg_su_oper_list_add(cb,i_susi->su,FALSE);
         }
         
         m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));

      } /* if ((state == SA_AMF_HA_QUIESCED) && 
        (susi->si == su->sg_of_su->admin_si) &&
        (NCS_ADMIN_STATE_UNLOCK != su->sg_of_su->admin_si->admin_state)) */

   } /* else (susi == AVD_SU_SI_REL_NULL) */
   
   return NCSCC_RC_SUCCESS;
      
}

 /*****************************************************************************
 * Function: avd_sg_npm_susi_sucss_su_oper
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_susi_sucss_func.
 * It is called if the SG is in AVD_SG_FSM_SU_OPER.
 *
 * Input: cb - the AVD control block
 *        su - In case of entire SU related operation the SU for
 *               which the ack is received.
 *        susi - The pointer to the service unit service instance relationship.
 *        act  - The action received in the ack message.
 *        state - The HA state in the message.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static uns32 avd_sg_npm_susi_sucss_su_oper(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state)
{
   AVD_SU_SI_REL *i_susi;
   AVD_SU_SI_REL *o_susi;
   NCS_BOOL flag;
   AVD_AVND *su_node_ptr = NULL;
   
   if(su->list_of_susi == AVD_SU_SI_REL_NULL)
      return NCSCC_RC_SUCCESS;

  
   if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_QUIESCED) &&
      (su->sg_of_su->su_oper_list.su == su))
   {
      if (susi != AVD_SU_SI_REL_NULL)
      {
         /* Quiesced for a SUSI. change the SUSI state to quiesced assigned.
          * If all the SUSIs of the SU are assigned quiesced do the 
          * functionality of quiesced all. If not stay in the same state.
          */
         for (i_susi = su->list_of_susi; 
            (i_susi != AVD_SU_SI_REL_NULL) && (i_susi->fsm == AVD_SU_SI_STATE_ASGND);
            i_susi = i_susi->su_next);
         
         if (i_susi != AVD_SU_SI_REL_NULL)
            return NCSCC_RC_SUCCESS;
         
      }
      
      /* quiesced all and SU is in the operation list. If the admin state of
       * the SU is shutdown change it to lock.  
       */
     
      o_susi = avd_sg_npm_su_othr(cb,su);
      if ((o_susi != AVD_SU_SI_REL_NULL) &&
         (o_susi->su->readiness_state == NCS_IN_SERVICE) &&
          (o_susi->state == SA_AMF_HA_STANDBY) &&
           (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
      {
         /* The standby SU w.r.t this SU is inservice has standby 
          * SI assignment from another SU, send remove message for 
          * each of those SI assignment to the standby SU and 
          * send D2N -INFO_SU_SI_ASSIGN message modify active all 
          * assignments to that SU. Add the SU to the oper list.                   
          */
         avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
         avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
      }
      
      /* Send D2N-INFO_SU_SI_ASSIGN message remove all to this SU. If
       * the SUs admin state is shutdown change it to LOCK.
       */
      m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

      if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {               
         m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
      }else if (su_node_ptr->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         m_AVD_IS_NODE_LOCK((su_node_ptr),flag);
         if (flag == TRUE)
         {
            m_AVD_SET_AVND_SU_ADMIN(cb,(su_node_ptr),NCS_ADMIN_STATE_LOCK);
         }
      }
      
      avd_sg_su_si_del_snd(cb,su);
      m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
      m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
      
   } /* if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_QUIESCED) &&
      (su->sg_of_su->su_oper_list.su == su)) */
   else if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_DEL) && 
         (su->sg_of_su->su_oper_list.su != su))
   {
      /* delete all and SU is not in the operation list. Free the SUSI 
       * relationships for the SU. 
       */
      
      avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
      
   } /* if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_DEL) && 
         (su->sg_of_su->su_oper_list.su != su))*/
   else if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_DEL) && 
         (su->sg_of_su->su_oper_list.su == su))
   {
      /*remove all and SU is in the operation list */
      if (su->list_of_susi->state != SA_AMF_HA_QUIESCED)
      {
         /* log error */
         m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
         m_AVD_LOG_INVALID_VAL_FATAL(su->sg_of_su->sg_fsm_state); 
      }
     
      o_susi = avd_sg_npm_su_othr(cb,su);
         
      if ((o_susi != AVD_SU_SI_REL_NULL) &&
         (o_susi->su->readiness_state == NCS_IN_SERVICE) &&
          (o_susi->state == SA_AMF_HA_STANDBY) &&
          (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
      {
         /* The standby SU w.r.t this SU is inservice has standby 
          * SI assignment from another SU, send remove message for 
          * each of those SI assignment to the standby SU and 
          * send D2N -INFO_SU_SI_ASSIGN message modify active all 
          * assignments to that SU. Add the SU to the oper list.                   
          */
         avd_sg_npm_su_chk_snd(cb,o_susi->su,su); 
         avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);         
      }else if ((o_susi != AVD_SU_SI_REL_NULL) &&
         (o_susi->su->readiness_state != NCS_IN_SERVICE))
      {
         /* Send a D2N-INFO_SU_SI_ASSIGN with remove all to the standby SU 
          * add it to SU oper list.
          */
         avd_sg_su_si_del_snd(cb,o_susi->su);
         avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
      }
      
      /* free all the SUSI relationships to this SU. remove this SU from the
       * SU oper list.SU in the operation list 
       */
      avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
      avd_sg_su_oper_list_del(cb,su,FALSE);
  
      
      if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
      {
         /* SU oper list is empty. Do the
          * functionality as in stable state to verify if new assignments 
          * can be done. If yes stay in the same state. If no 
          * new assignments change state to stable.
          */
         if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
         {
            /* all the assignments have already been done in the SG. */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            avd_sg_app_su_inst_func(cb,su->sg_of_su);
         }else
         {
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
      }else
      {
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state); 
      }   
         
   } /* if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_DEL) && 
         (su->sg_of_su->su_oper_list.su == su))*/
     
   
   return NCSCC_RC_SUCCESS;
}



 /*****************************************************************************
 * Function: avd_sg_npm_susi_sucss_si_oper
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_susi_sucss_func.
 * It is called if the SG is in AVD_SG_FSM_SI_OPER.
 *
 * Input: cb - the AVD control block
 *        su - In case of entire SU related operation the SU for
 *               which the ack is received.
 *        susi - The pointer to the service unit service instance relationship.
 *        act  - The action received in the ack message.
 *        state - The HA state in the message.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static uns32 avd_sg_npm_susi_sucss_si_oper(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state)
{
   AVD_SU_SI_REL *i_susi, *o_susi;
   AVD_SU_SI_STATE old_fsm_state;
   
   if (susi != AVD_SU_SI_REL_NULL)
   {
      /* assign single SUSI */
      if (act == AVSV_SUSI_ACT_DEL)
      {
         /* remove for single SU SI. Free the SUSI relationship. If all the 
          * relationships are in the assigned state, remove the SU from the 
          * operation list and do the realignment of the SG.
          */
         /* free all the CSI assignments  */
         avd_compcsi_list_del(cb,susi,FALSE);
         /* Unassign the SUSI */
         m_AVD_SU_SI_TRG_DEL(cb,susi);
         
         if ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
           (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH))
         {
            /* SI in admin pointer with switch state true  */
            
            for (i_susi = su->list_of_susi; 
            (i_susi != AVD_SU_SI_REL_NULL) && (i_susi->fsm == AVD_SU_SI_STATE_ASGND);
            i_susi = i_susi->su_next);
            
            if (i_susi == AVD_SU_SI_REL_NULL)
            {
               /* entire SU is assigned. Send D2N-INFO_SU_SI_ASSIGN with 
                * modify standby all to the Quiesced SU. Remove the SI from 
                * admin pointer and add the quiesced SU to the SU oper list.
                */
               if (su->sg_of_su->admin_si->list_of_sisu == i_susi)
               {
                  o_susi = i_susi->si_next;
               }else
               {
                  o_susi = su->sg_of_su->admin_si->list_of_sisu;
               }
               
               if (avd_sg_su_si_mod_snd(cb,o_susi->su,SA_AMF_HA_STANDBY) == NCSCC_RC_FAILURE)
               {
                  /* log fatal error */
                  m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                  return NCSCC_RC_FAILURE;
               }
               
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
               m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);               
               
            } /* if (i_susi == AVD_SU_SI_REL_NULL) */
            
            /* Some more SIs need to be removed.  So dont do anything.*/
         } /* if ((su->sg_of_su->admin_si != AVD_SI_NULL) && 
           (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH)) */
         
      } /* if (act == AVSV_SUSI_ACT_DEL) */
      else if ((state == SA_AMF_HA_QUIESCED) && 
               (susi->si == su->sg_of_su->admin_si) &&
               (NCS_ADMIN_STATE_UNLOCK != su->sg_of_su->admin_si->admin_state))
      {
         /* Message with modified quiesced for a SI in the admin pointer with.
          * admin values lock/shutdown. Send a D2N-INFO_SU_SI_ASSIGN with remove
          * for this SI to this SU, Add this SU to the SU oper list. 
          * Change admin state to lock for this SI. Identify its standby SU assignment. 
          * send D2N-INFO_SU_SI_ASSIGN remove for only this SI and Add the 
          * standby SU to the SU oper list. Remove the SI from the 
          * SI admin pointer.
          */
         old_fsm_state = susi->fsm;
         susi->fsm = AVD_SU_SI_STATE_UNASGN;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);         
         if (avd_snd_susi_msg(cb,susi->su,susi,AVSV_SUSI_ACT_DEL)
            == NCSCC_RC_FAILURE)
         {
            /* Log a fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->su->name_net.value,susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->si->name_net.value,susi->si->name_net.length);
            susi->fsm = old_fsm_state;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
            return NCSCC_RC_FAILURE;
         }
         avd_sg_su_oper_list_add(cb,susi->su,FALSE);
         m_AVD_SET_SI_ADMIN(cb,(su->sg_of_su->admin_si),NCS_ADMIN_STATE_LOCK);
         if (su->sg_of_su->admin_si->list_of_sisu == susi)
            i_susi = susi->si_next;
         else
            i_susi = su->sg_of_su->admin_si->list_of_sisu;
         
         if (i_susi != AVD_SU_SI_REL_NULL)
         {
            i_susi->fsm = AVD_SU_SI_STATE_UNASGN;  
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);            
            avd_snd_susi_msg(cb,i_susi->su,i_susi,AVSV_SUSI_ACT_DEL);
            avd_sg_su_oper_list_add(cb,i_susi->su,FALSE);
         }         
         
         m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state); 

      } /* if ((state == SA_AMF_HA_QUIESCED) && 
        (susi->si == su->sg_of_su->admin_si) &&
        (NCS_ADMIN_STATE_UNLOCK != su->sg_of_su->admin_si->admin_state)) */

   }else /* if (susi != AVD_SU_SI_REL_NULL) */
   {
      if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_QUIESCED) &&
         (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
   ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
      != AVD_SU_SI_REL_NULL))
      {
         /* message with modified quiesced all and an SI is in the admin
          * pointer with switch value true
          */
         if (i_susi->si->list_of_sisu != i_susi)
         {
            o_susi = i_susi->si->list_of_sisu;
         } else
         {
            o_susi = i_susi->si_next;
         }
         if(o_susi == AVD_SU_SI_REL_NULL)
         {
            /* No standby SU, send D2N-INFO_SU_SI_ASSIGN message 
             * modify active all assignment to this quiesced SU, remove 
             * the SI from the SI admin pointer, change the switch value 
             * to false. Add the SU to SU operlist. 
             */
            if (avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_ACTIVE) == NCSCC_RC_FAILURE)
            {
               /* log fatal error */
               m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
               return NCSCC_RC_FAILURE;
            }
            m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
            avd_sg_su_oper_list_add(cb,su,FALSE);
            m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state); 
         }/* if(o_susi == AVD_SU_SI_REL_NULL)*/
         else if ((o_susi->state == SA_AMF_HA_STANDBY) &&
               (o_susi->su->readiness_state == NCS_IN_SERVICE))
         {
            /* The standby SU is inservice has standby SI assignment from
             * another SU, send remove message for each of those SI 
             * assignment to the standby SU and send D2N-INFO_SU_SI_ASSIGN
             * message modify active all assignments to that SU.
             */
            avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
         }
          
      } /* if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_QUIESCED) && 
         (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
   ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
      != AVD_SU_SI_REL_NULL))*/
      else if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_ACTIVE))
      {
         for (i_susi = su->list_of_susi; 
            (i_susi != AVD_SU_SI_REL_NULL) && (i_susi->fsm == AVD_SU_SI_STATE_ASGND);
            i_susi = i_susi->su_next);
         
         if (i_susi == AVD_SU_SI_REL_NULL)
         {
            /* active all and the entire SU is assigned. */
            if ((su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
  ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
            != AVD_SU_SI_REL_NULL))
            {
               /* SI in admin pointer with switch state true and SI has 
                * relationship with this SU. Send D2N-INFO_SU_SI_ASSIGN with 
                * modify standby all to the Quiesced SU. Remove the SI from 
                * admin pointer and add the quiesced SU to the SU oper list.
                */
               if (su->sg_of_su->admin_si->list_of_sisu == i_susi)
               {
                  o_susi = i_susi->si_next;
               }else
               {
                  o_susi = su->sg_of_su->admin_si->list_of_sisu;
               }
               
               if (avd_sg_su_si_mod_snd(cb,o_susi->su,SA_AMF_HA_STANDBY) == NCSCC_RC_FAILURE)
               {
                  /* log fatal error */
                  m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                  return NCSCC_RC_FAILURE;
               }
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
               m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state); 
            }/* if ((su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
  ((i_susi = avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE))
            != AVD_SU_SI_REL_NULL)) */
            
         } /* if (i_susi == AVD_SU_SI_REL_NULL) */
         
      } /* if ((act == AVSV_SUSI_ACT_MOD) && (state == SA_AMF_HA_ACTIVE)) */
      else
      {
         /* log error */
         m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
         m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
         m_AVD_LOG_INVALID_VAL_FATAL(su->sg_of_su->sg_fsm_state);
      }
   } /* else (susi != AVD_SU_SI_REL_NULL)*/
   

   return NCSCC_RC_SUCCESS;
}

 
/*****************************************************************************
 * Function: avd_sg_npm_susi_sucss_func
 *
 * Purpose:  This function is called when a SU SI ack function is
 * received from the AVND with success value. The SG FSM for N+M redundancy
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
 * NOTES: This is a N+M redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_npm_susi_sucss_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state)
{

   AVD_SU_SI_REL *o_susi;

   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_susi_sucss_func");
   m_AVD_LOG_RCVD_VAL(((long)su));
   m_AVD_LOG_RCVD_VAL(((long)susi));   
   m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
   
   switch(su->sg_of_su->sg_fsm_state)
   {
   case AVD_SG_FSM_STABLE:
      /* Do the action specified in the message if delete else no action. */
      if (act == AVSV_SUSI_ACT_DEL)
      {
         if (susi == AVD_SU_SI_REL_NULL)
         {
            avd_sg_su_asgn_del_util(cb, su, TRUE,FALSE);            
         }else
         {
            /* free all the CSI assignments  */
            avd_compcsi_list_del(cb,susi,FALSE);
            /* Unassign the SUSI */
            m_AVD_SU_SI_TRG_DEL(cb,susi);
         }
      }      
      /* log informational error. */
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      break; /* case AVD_SG_FSM_STABLE: */
   case AVD_SG_FSM_SG_REALIGN:      
   
      if (avd_sg_npm_susi_sucss_sg_reln(cb,su,susi,act,state) == NCSCC_RC_FAILURE)
         return NCSCC_RC_FAILURE;
      
      break; /* case AVD_SG_FSM_SG_REALIGN: */
   case AVD_SG_FSM_SU_OPER:
         
      if (avd_sg_npm_susi_sucss_su_oper(cb,su,susi,act,state) == NCSCC_RC_FAILURE)
         return NCSCC_RC_FAILURE;
      
      break; /* case AVD_SG_FSM_SU_OPER: */
   case AVD_SG_FSM_SI_OPER:

      if (avd_sg_npm_susi_sucss_si_oper(cb,su,susi,act,state) == NCSCC_RC_FAILURE)
         return NCSCC_RC_FAILURE;
      
      break; /* case AVD_SG_FSM_SI_OPER: */
   case AVD_SG_FSM_SG_ADMIN:
      if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         (state == SA_AMF_HA_QUIESCED))
      {
         /* quiesced all Send a D2N-INFO_SU_SI_ASSIGN with remove all for this SU. */
         if (avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         if (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
         {
            /* If the admin state is shutdown, identify the corresponding 
             * standby and send a D2N-INFO_SU_SI_ASSIGN with remove all for 
             * that SU and add it to the SU oper list.
             */
            o_susi = avd_sg_npm_su_othr(cb,su);
            if ((o_susi != AVD_SU_SI_REL_NULL) && 
               (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
            {
               avd_sg_su_si_del_snd(cb,o_susi->su);
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
            }

         } /* if (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN) */
         
      } /* if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         (state == SA_AMF_HA_QUIESCED)) */
      else if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_DEL))
      {
         /* remove all. Free all the SI assignments to this SU. If the 
          * SU oper list is empty,  change the SG admin state to LOCK, 
          * Change state to stable.
          */
         avd_sg_su_asgn_del_util(cb,su, TRUE,FALSE);
         avd_sg_su_oper_list_del(cb,su,FALSE);
         if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
         {
            m_AVD_SET_SG_ADMIN(cb,(su->sg_of_su),NCS_ADMIN_STATE_LOCK);
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);            
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state); 
         }
         
      } /* else if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_DEL))*/
      else
      {
         /* log a fatal error */           
         m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
         m_AVD_LOG_INVALID_VAL_FATAL(((uns32)act));
         m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
         return NCSCC_RC_FAILURE;
      }

      break; /* case AVD_SG_FSM_SG_ADMIN: */   
   default:
      /* log fatal error about the invalid value */
      m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
      m_AVD_LOG_INVALID_VAL_FATAL(((uns32)su->sg_of_su->sg_fsm_state));
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      return NCSCC_RC_FAILURE;
      break;
   } /* switch(su->sg_of_su->sg_fsm_state) */

   return NCSCC_RC_SUCCESS;
} 
 
/*****************************************************************************
 * Function: avd_sg_npm_susi_fail_func
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
 * NOTES: This is a N+M redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_npm_susi_fail_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state)
{
   AVD_SU_SI_REL *o_susi;
   AVD_SU_SI_STATE old_fsm_state;
   NCS_BOOL flag;
   AVD_AVND *su_node_ptr = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_susi_fail_func");
   m_AVD_LOG_RCVD_VAL(((long)su));
   m_AVD_LOG_RCVD_VAL(((long)susi));   
   m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
   
   switch(su->sg_of_su->sg_fsm_state)
   {
   case AVD_SG_FSM_STABLE:
      /* Do the action specified in the message if delete else no action. */
      if (act == AVSV_SUSI_ACT_DEL)
      {
         if (susi == AVD_SU_SI_REL_NULL)
         {
            avd_sg_su_asgn_del_util(cb, su, TRUE,FALSE);
         }else
         {
            /* free all the CSI assignments  */
            avd_compcsi_list_del(cb,susi,FALSE);
            /* Unassign the SUSI */
            m_AVD_SU_SI_TRG_DEL(cb,susi);
         }
      }

      /* log fatal error. */      
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      break; /* case AVD_SG_FSM_STABLE: */
   case AVD_SG_FSM_SG_REALIGN:
   case AVD_SG_FSM_SU_OPER:
      
      if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING)) 
         && (su->sg_of_su->su_oper_list.su == su))
      {
         /* quiesced all and SU is OOS. Send a D2N-INFO_SU_SI_ASSIGN remove
          * all to the SU.
          */
         if (avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* Log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         } 

         m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

         if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
         {               
            m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
         }else if (su_node_ptr->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
         {
            m_AVD_IS_NODE_LOCK((su_node_ptr),flag);
            if (flag == TRUE)
            {
               m_AVD_SET_AVND_SU_ADMIN(cb,(su_node_ptr),NCS_ADMIN_STATE_LOCK);
            }
         }
         
      }else
      {
         /* No action as other call back failure will cause operation disable 
          * event to be sent by AvND.
          */
         m_AVD_LOG_RCVD_VAL(((long)su));
      }
      
      break; /* case AVD_SG_FSM_SG_REALIGN: 
              * case AVD_SG_FSM_SU_OPER:
              */

   case AVD_SG_FSM_SI_OPER:
      
      if ((susi != AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING)) 
         && (susi->si == su->sg_of_su->admin_si))
      {
         /* message with modified quiesced for a SI in the admin pointer with 
          * admin values lock/shutdown. Send a D2N-INFO_SU_SI_ASSIGN with remove for 
          * this SI to this SU. Remove the SI from the admin pointer. Add the 
          * SU to operation list also add the other SU to which this SI has 
          * assignment to the operation list. Change state to SG_realign.
          */         
         
         old_fsm_state = susi->fsm;
         susi->fsm = AVD_SU_SI_STATE_UNASGN;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
         if (avd_snd_susi_msg(cb,susi->su,susi,AVSV_SUSI_ACT_DEL)
            == NCSCC_RC_FAILURE)
         {
            /* Log a fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->su->name_net.value,susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->si->name_net.value,susi->si->name_net.length);
            susi->fsm = old_fsm_state;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
            return NCSCC_RC_FAILURE;
         }
         
         m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
         
         avd_sg_su_oper_list_add(cb,susi->su,FALSE);
         
         if (susi->si->list_of_sisu != susi)
         {
            o_susi =  susi->si->list_of_sisu;
         }else
         {
            o_susi = susi->si_next;
         }
         
         if ((o_susi != AVD_SU_SI_REL_NULL) && (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
         {
            avd_snd_susi_msg(cb,o_susi->su,o_susi,AVSV_SUSI_ACT_DEL);
            avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
         }
         
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
      } /* if ((susi != AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING)) 
         && (susi->si == su->sg_of_su->admin_si)) */
      else if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         (state == SA_AMF_HA_QUIESCED) && (su->sg_of_su->admin_si != AVD_SI_NULL)
         && (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
(avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE)
      != AVD_SU_SI_REL_NULL) && (su->readiness_state == NCS_IN_SERVICE))
      {
         /* Message with modified quiesced all and an SI is in the admin 
          * pointer with switch value true.  Send D2N-INFO_SU_SI_ASSIGN with 
          * modified active all to the SU. Change the switch value of the SI .
          * to false. Remove the SI from the admin pointer. Add the SU to 
          * operation list. Change state to SG_realign.
          */
         
         
         if (avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_ACTIVE) 
            == NCSCC_RC_FAILURE)
         {
            /* log fatal error */            
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         avd_sg_su_oper_list_add(cb,su,FALSE);
         
         m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
         
         m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
         
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);            
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
          
      } /* if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         (state == SA_AMF_HA_QUIESCED) && (su->sg_of_su->admin_si != AVD_SI_NULL)
         && (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) &&
(avd_su_susi_struc_find(cb,su,su->sg_of_su->admin_si->name_net,FALSE)
      != AVD_SU_SI_REL_NULL) && (su->readiness_state == NCS_IN_SERVICE)) */
      else 
      {
         /* message with remove for a SU SI relationship having standby value.
          * message with modified active for a SI in the admin pointer with
          * switch value true; message with remove for a SU SI relationship
          * having quiesced value. Ignore this message. It will trigger the
          * SU to go disabled and an operation state message will be received
          * for the SU.
          */
         m_AVD_LOG_RCVD_VAL(((long)su));
      } 

      break; /* case AVD_SG_FSM_SI_OPER: */
   case AVD_SG_FSM_SG_ADMIN:
      
      if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         (state == SA_AMF_HA_QUIESCED) && 
         (su->sg_of_su->admin_state == NCS_ADMIN_STATE_LOCK))
      {
         /* SG is admin lock. Message is quiesced all. 
          * Send a D2N-INFO_SU_SI_ASSIGN with remove all for this SU.
          */
         if (avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* Log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
      } /* if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         (state == SA_AMF_HA_QUIESCED) && 
         (su->sg_of_su->admin_state == NCS_ADMIN_STATE_LOCK))*/
      else if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING))&& 
         (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN))
      {
         /* SG is admin shutdown. The message is quiescing all or quiesced all.
          * Change admin state to lock and identify its standby SU assignment 
          * send D2N-INFO_SU_SI_ASSIGN remove all for the SU. 
          * Send a D2N-INFO_SU_SI_ASSIGN with remove all for this SU.
          */
         if (avd_sg_su_si_del_snd(cb,su) == NCSCC_RC_FAILURE)
         {
            /* Log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         o_susi = avd_sg_npm_su_othr(cb,su);
         if ((o_susi != AVD_SU_SI_REL_NULL) && 
            (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
         {
            avd_sg_su_si_del_snd(cb,o_susi->su);
            avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
         }
         
      } /* else if ((susi == AVD_SU_SI_REL_NULL) && (act == AVSV_SUSI_ACT_MOD) &&
         ((state == SA_AMF_HA_QUIESCED) || (state == SA_AMF_HA_QUIESCING))&& 
         (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN))*/
      else
      {
         /* Other cases log a informational message. AvND would treat that as
          * a operation disable for the SU and cause the SU to go OOS, which
          * will trigger the FSM.
          */
         m_AVD_LOG_RCVD_VAL(((long)su));
      }

      break; /* case AVD_SG_FSM_SG_ADMIN: */   
   default:
      /* log fatal error about the invalid value */      
      m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
      m_AVD_LOG_INVALID_VAL_FATAL(((uns32)su->sg_of_su->sg_fsm_state));
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      return NCSCC_RC_FAILURE;
      break;
   } /* switch(su->sg_of_su->sg_fsm_state) */
   
   return NCSCC_RC_SUCCESS;

} 



 /*****************************************************************************
 * Function: avd_sg_npm_realign_func
 *
 * Purpose:  This function will call the chose assign function to check and
 * assign SIs. If any assigning is being done it adds the SU to the operation
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

uns32 avd_sg_npm_realign_func(AVD_CL_CB *cb,AVD_SG *sg)
{
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_realign_func");
   m_AVD_LOG_RCVD_VAL(((long)sg));
   
   m_AVD_LOG_RCVD_VAL(sg->sg_fsm_state);
   /* If the SG FSM state is not stable just return success. */   
   
   if((cb->init_state != AVD_APP_STATE) &&
      (sg->sg_ncs_spec == SA_FALSE))
   {
      return NCSCC_RC_SUCCESS;
   }
   
   if (sg->sg_fsm_state != AVD_SG_FSM_STABLE)
   {
      m_AVD_SET_SG_ADJUST(cb,sg,AVSV_SG_STABLE);
      avd_sg_app_su_inst_func(cb, sg);
      return NCSCC_RC_SUCCESS;
   }
   
   if (avd_sg_npm_su_chose_asgn(cb,sg) == AVD_SU_NULL)
   {
      /* all the assignments have already been done in the SG. */
      m_AVD_SET_SG_ADJUST(cb,sg,AVSV_SG_STABLE);
      avd_sg_app_su_inst_func(cb, sg);
      return NCSCC_RC_SUCCESS;
   }
   
   /*  change the FSM state */
   
   m_AVD_SET_SG_ADJUST(cb,sg,AVSV_SG_STABLE);
   
   m_AVD_SET_SG_FSM(cb,(sg),AVD_SG_FSM_SG_REALIGN);
   m_AVD_LOG_RCVD_VAL(sg->sg_fsm_state);
   
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sg_npm_node_fail_sg_relgn
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_node_fail_func.
 * It is called if the SG is in AVD_SG_FSM_SG_REALIGN.
 *
 * Input: cb - the AVD control block
 *        su - The SU that has faulted because of the node failure.
 *        
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static void avd_sg_npm_node_fail_sg_relgn(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU_SI_REL *l_susi, *o_susi, *ot_susi;
   NCS_BOOL l_flag=FALSE;
   NCS_BOOL flag;
   AVD_AVND *su_node_ptr = NULL;
   
   if (su->sg_of_su->admin_si != AVD_SI_NULL)
   {
      l_susi=o_susi=AVD_SU_SI_REL_NULL;
      if (su->sg_of_su->admin_si->list_of_sisu->su == su)
      {
         l_susi = su->sg_of_su->admin_si->list_of_sisu;
         o_susi = l_susi->si_next;
      }else if ((su->sg_of_su->admin_si->list_of_sisu->si_next != AVD_SU_SI_REL_NULL) &&
               (su->sg_of_su->admin_si->list_of_sisu->si_next->su == su))
      {
         o_susi = su->sg_of_su->admin_si->list_of_sisu;
         l_susi = o_susi->si_next;
      }
      if (l_susi == AVD_SU_SI_REL_NULL)
      {
         /* SI doesn't have any assignment with the SU. */
         m_AVD_CHK_OPLIST(su,l_flag);
         if (l_flag == TRUE)
         {
            /* SU is in the SU oper list. */
            if ((su->list_of_susi->state == SA_AMF_HA_QUIESCED) && 
               (su->list_of_susi->fsm == AVD_SU_SI_STATE_MODIFY))
            {
               /* the SI relationships to the SU state is quiesced assigning. */
               ot_susi = avd_sg_npm_su_othr(cb,su);
         
               if ((ot_susi != AVD_SU_SI_REL_NULL) &&
                   (ot_susi->su->readiness_state == NCS_IN_SERVICE) &&
                   (ot_susi->state == SA_AMF_HA_STANDBY) &&
                   (ot_susi->fsm != AVD_SU_SI_STATE_UNASGN))
               {
                  /* The standby SU w.r.t this SU is inservice has standby 
                   * SI assignment from another SU, send remove message for 
                   * each of those SI assignment to the standby SU and 
                   * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                   * assignments to that SU. Add the SU to the oper list.                   
                   */
                  avd_sg_npm_su_chk_snd(cb,ot_susi->su,su);
                  avd_sg_su_oper_list_add(cb,ot_susi->su,FALSE);
               }
               
            }/* if ((su->list_of_susi->state == SA_AMF_HA_QUIESCED) && 
               (su->list_of_susi->fsm = AVD_SU_SI_STATE_MODIFY)) */
            
            /* remove the SU from the operation list. */
            avd_sg_su_oper_list_del(cb,su,FALSE);

         }else /*if (l_flag == TRUE) */
         {
            /* SU not in the SU oper list */
            if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
            {
               /* The SU is active SU, identify the inservice standby SU for this SU.
                */
               
               ot_susi = avd_sg_npm_su_othr(cb,su);
               
               if ((ot_susi != AVD_SU_SI_REL_NULL) &&
                  (ot_susi->su->readiness_state == NCS_IN_SERVICE) &&
                   (ot_susi->state == SA_AMF_HA_STANDBY)&&
                   (ot_susi->fsm != AVD_SU_SI_STATE_UNASGN))
               {
                  /* The standby SU w.r.t this SU is inservice has standby 
                   * SI assignment from another SU, send remove message for 
                   * each of those SI assignment to the standby SU and 
                   * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                   * assignments to that SU. Add the SU to the oper list.                   
                   */
                  avd_sg_npm_su_chk_snd(cb,ot_susi->su,su);
                  avd_sg_su_oper_list_add(cb,ot_susi->su,FALSE);
               }
               
            }/* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
            
         } /* else (l_flag == TRUE) */
      
         /* Free all the SI assignments to this SU.*/
         avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
         
      }else /* if (l_susi == AVD_SU_SI_REL_NULL) */      
      {
         /* SI has assignment with the SU. */
         if (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH)
         {
            /* the admin pointer SI has switch operation as TRUE. */
            if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
               (l_susi->fsm == AVD_SU_SI_STATE_MODIFY))
            {
               /* this SI relation with the SU is quiesced assigning. */
               if (o_susi != AVD_SU_SI_REL_NULL)
               {
                  if ((o_susi->fsm == AVD_SU_SI_STATE_ASGND) &&
                     (o_susi->su->readiness_state == NCS_IN_SERVICE))
                  {
                     /* The SUs corresponding standby/quiesced SU is in-service.
                      * If the standby SU has standby SI assignment from 
                      * another SU, send remove message for each of those SI 
                      * assignment to the standby SU and send D2N-INFO_SU_SI_ASSIGN
                      * message modify active all assignments to that SU.
                      * Add the standby SU to operation list.
                      */
                      avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
                      avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  }else if (o_susi->su->readiness_state != NCS_IN_SERVICE)
                  {
                     /* Add the standby SU to operation list. */
                     avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  }
               }
               
               /* remove the SU from the operation list. */
               avd_sg_su_oper_list_del(cb,su,FALSE);
               
               /* Free all the SI assignments to this SU.*/
               avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
               
               /* Change the SI switch state to false. Remove the SI from the 
                * SI admin pointer.
                */
               m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
               m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
               
             
               if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
               {
                  /* both the SI admin pointer and SU oper list are empty.
                   * Do the functionality as in stable state to verify if 
                   * new assignments can be done. If yes stay in the same state.
                   * If no new assignments change state to stable.
                   */
                  if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
                  {
                     /* all the assignments have already been done in the SG. */
                     m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                     m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                  }
               }
               
            }/* if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
               (l_susi->fsm == AVD_SU_SI_STATE_MODIFY)) */
            else if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
               (l_susi->fsm == AVD_SU_SI_STATE_ASGND))
            {
               /* this SI relation with the SU is quiesced assigned. */
               if (o_susi != AVD_SU_SI_REL_NULL)
               {
                  /* Add the other SU to operation list. */
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }
               /* Change the SI switch state to false. Remove the SI from the 
                * SI admin pointer.
                */
               m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
               m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
               
               /* Free all the SI assignments to this SU.*/
               avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
               
               if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
               {
                  /* both the SI admin pointer and SU oper list are empty.
                   * Do the functionality as in stable state to verify if 
                   * new assignments can be done. If yes stay in the same state.
                   * If no new assignments change state to stable.
                   */
                  if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
                  {
                     /* all the assignments have already been done in the SG. */
                     m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                     m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                  }
               }
            }/* else if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
               (l_susi->fsm == AVD_SU_SI_STATE_ASGND))*/
            else if (l_susi->state == SA_AMF_HA_STANDBY)
            {
               /* this SI relation with the SU is standby. */
               if (l_susi->fsm != AVD_SU_SI_STATE_ASGND)
                  /* remove the SU from the operation list. */
                  avd_sg_su_oper_list_del(cb,su,FALSE);
               
               /* Free all the SI assignments to this SU.*/
               avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            }/* else if (l_susi->state == SA_AMF_HA_STANDBY) */
            else if (l_susi->state == SA_AMF_HA_ACTIVE)
            {
               /* this SI relation with the SU is standby. */
               if (o_susi != AVD_SU_SI_REL_NULL)
               {
                  if ((o_susi->fsm == AVD_SU_SI_STATE_ASGND) &&
                     (o_susi->su->readiness_state == NCS_IN_SERVICE))
                  {
                     /* The SUs corresponding quiesced SU is in-service.
                      * send D2N-INFO_SU_SI_ASSIGN
                      * message modify active all assignments to that SU.
                      * Add the quiesced SU to operation list.
                      */
                      avd_sg_su_si_mod_snd(cb,o_susi->su,SA_AMF_HA_ACTIVE);
                     /* Add the quiesced SU to operation list. */
                     avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  }else if (o_susi->su->readiness_state != NCS_IN_SERVICE)
                  {
                     /* Add the quiesced SU to operation list. */
                     avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  }
               }
               
               /* Change the SI switch state to false. Remove the SI from the 
                * SI admin pointer.
                */
               m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
               m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
               
               /* Free all the SI assignments to this SU.*/
               avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
               
               if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
               {
                  /* both the SI admin pointer and SU oper list are empty.
                   * Do the functionality as in stable state to verify if 
                   * new assignments can be done. If yes stay in the same state.
                   * If no new assignments change state to stable.
                   */
                  if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
                  {
                     /* all the assignments have already been done in the SG. */
                     m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                     m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                  }
               }               
            } /* else if (l_susi->state == SA_AMF_HA_ACTIVE) */
               
         }else /* if (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) */
         {
            /* admin pointer SI admin operation is LOCK/shutdown. */
            if ((l_susi->state == SA_AMF_HA_QUIESCING) ||
               (l_susi->state == SA_AMF_HA_QUIESCED))
            {
               /* Change the admin operation to UNLOCK. */
               m_AVD_SET_SI_ADMIN(cb,(su->sg_of_su->admin_si),NCS_ADMIN_STATE_UNLOCK);
               
               if (o_susi != AVD_SU_SI_REL_NULL)
               {
                  if ((o_susi->fsm == AVD_SU_SI_STATE_ASGND) &&
                     (o_susi->su->readiness_state == NCS_IN_SERVICE))
                  {
                     /* The SUs corresponding standby/quiesced SU is in-service.
                      * If the standby SU has standby SI assignment from 
                      * another SU, send remove message for each of those SI 
                      * assignment to the standby SU and send D2N-INFO_SU_SI_ASSIGN
                      * message modify active all assignments to that SU.
                      * Add the standby SU to operation list.
                      */
                     avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
                     avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  }else if (o_susi->su->readiness_state != NCS_IN_SERVICE)
                  {
                     /* Add the standby SU to operation list. */
                     avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
                  }
               }
               
               
               /* Free all the SI assignments to this SU.*/
               avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
               
               /*  Remove the SI from the SI admin pointer.  */
               m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));               
             
               if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
               {
                  /* both the SI admin pointer and SU oper list are empty.
                   * Do the functionality as in stable state to verify if 
                   * new assignments can be done. If yes stay in the same state.
                   * If no new assignments change state to stable.
                   */
                  if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
                  {
                     /* all the assignments have already been done in the SG. */
                     m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                     m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
                  }
               }
               
            }/* if ((l_susi->state == SA_AMF_HA_QUIESCING) ||
               (l_susi->state == SA_AMF_HA_QUIESCED)) */
            else if (l_susi->state == SA_AMF_HA_STANDBY)
            {
               /* this SI relation with the SU is standby. */
               if (l_susi->fsm != AVD_SU_SI_STATE_ASGND)
                  /* remove the SU from the operation list. */
                  avd_sg_su_oper_list_del(cb,su,FALSE);
               
               /* Free all the SI assignments to this SU.*/
               avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
               
            }/* else if (l_susi->state == SA_AMF_HA_STANDBY) */
         }/* else (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) */
         

      } /* else (l_susi == AVD_SU_SI_REL_NULL) */
      
   }else /* if (su->sg_of_su->admin_si != AVD_SI_NULL) */
   {
      m_AVD_CHK_OPLIST(su,l_flag);
      if (l_flag == TRUE)
      {
         /* SU is in the SU oper list. */
         if ((su->list_of_susi->state == SA_AMF_HA_QUIESCED) || 
            (su->list_of_susi->state == SA_AMF_HA_QUIESCING) ||
            (su->list_of_susi->state == SA_AMF_HA_ACTIVE))
         {
            /* the SI relationships to the SU state is active/quiesced/quiescing assigning. */
            ot_susi = avd_sg_npm_su_othr(cb,su);
      
            if (ot_susi != AVD_SU_SI_REL_NULL)
            {
               if ((ot_susi->su->readiness_state == NCS_IN_SERVICE) &&
                   (ot_susi->state == SA_AMF_HA_STANDBY) &&
                   (ot_susi->fsm != AVD_SU_SI_STATE_UNASGN))
               {
                  /* The standby SU w.r.t this SU is inservice has standby 
                   * SI assignment from another SU, send remove message for 
                   * each of those SI assignment to the standby SU and 
                   * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                   * assignments to that SU. Add the SU to the oper list.                   
                   */
                  avd_sg_npm_su_chk_snd(cb,ot_susi->su,su);
                  avd_sg_su_oper_list_add(cb,ot_susi->su,FALSE);
               }else if (ot_susi->su->readiness_state != NCS_IN_SERVICE)
               {
                  avd_sg_su_oper_list_add(cb,ot_susi->su,FALSE);
               }
            }
            
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            
            /* remove the SU from the operation list. */
            avd_sg_su_oper_list_del(cb,su,FALSE);
            
            m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

            if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
            {               
               m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
            }else if (su_node_ptr->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
            {
               m_AVD_IS_NODE_LOCK((su_node_ptr),flag);
               if (flag == TRUE)
               {
                  m_AVD_SET_AVND_SU_ADMIN(cb,(su_node_ptr),NCS_ADMIN_STATE_LOCK);
               }
            }
            
         }else /* if (su->list_of_susi->state == SA_AMF_HA_QUIESCED) || 
            (su->list_of_susi->state == SA_AMF_HA_QUIESCING) ||
            (su->list_of_susi->state == SA_AMF_HA_ACTIVE)) */
         
         {
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            
            /* remove the SU from the operation list. */
            avd_sg_su_oper_list_del(cb,su,FALSE);
         }/* else (su->list_of_susi->state == SA_AMF_HA_QUIESCED) || 
            (su->list_of_susi->state == SA_AMF_HA_QUIESCING) ||
            (su->list_of_susi->state == SA_AMF_HA_ACTIVE)) */
         
         if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
         {
            /* both the SI admin pointer and SU oper list are empty.
             * Do the functionality as in stable state to verify if 
             * new assignments can be done. If yes stay in the same state.
             * If no new assignments change state to stable.
             */
            if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
            {
               /* all the assignments have already been done in the SG. */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            }
         }

      }else /*if (l_flag == TRUE) */
      {
         /* SU not in the SU oper list */
         if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
         {
            /* The SU is active SU, identify the inservice standby SU for this SU.
             */
            
            o_susi = avd_sg_npm_su_othr(cb,su);
            
            if ((o_susi != AVD_SU_SI_REL_NULL) &&
               (o_susi->su->readiness_state == NCS_IN_SERVICE) &&
                (o_susi->state == SA_AMF_HA_STANDBY)&&
                (o_susi->fsm != AVD_SU_SI_STATE_UNASGN))
            {
               /* The standby SU w.r.t this SU is inservice has standby 
                * SI assignment from another SU, send remove message for 
                * each of those SI assignment to the standby SU and 
                * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                * assignments to that SU. Add the SU to the oper list.                   
                */
               avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
            }
            
         }/* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
         
         /* Free all the SI assignments to this SU.*/
         avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
         
      } /* else (l_flag == TRUE) */      
      
   }/* else (su->sg_of_su->admin_si != AVD_SI_NULL) */
}

/*****************************************************************************
 * Function: avd_sg_npm_node_fail_su_oper
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_node_fail_func.
 * It is called if the SG is in AVD_SG_FSM_SU_OPER.
 *
 * Input: cb - the AVD control block
 *        su - The SU that has faulted because of the node failure.
 *        
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static void avd_sg_npm_node_fail_su_oper(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU_SI_REL *o_susi;
   NCS_BOOL flag;
   AVD_AVND *su_node_ptr = NULL;
   
   if(su->sg_of_su->su_oper_list.su == su)
   {
      /* SU is in the SU oper list. */
      
      /* the SI relationships to the SU state is quiesced/quiescing assigning. */
      o_susi = avd_sg_npm_su_othr(cb,su);
      
      if (o_susi != AVD_SU_SI_REL_NULL)
      {
         if (o_susi->su->readiness_state == NCS_IN_SERVICE)
         {
            /* The standby SU w.r.t this SU is inservice has standby 
             * SI assignment from another SU, send remove message for 
             * each of those SI assignment to the standby SU and 
             * send D2N -INFO_SU_SI_ASSIGN message modify active all 
             * assignments to that SU. Add the SU to the oper list.                   
             */
            avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
            avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
         }else
         {
            avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
         }
      }
      
      /* Free all the SI assignments to this SU.*/
      avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
      
      /* remove the SU from the operation list. */
      avd_sg_su_oper_list_del(cb,su,FALSE);
      
      m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);
      if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {               
         m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
      }else if (su_node_ptr->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         m_AVD_IS_NODE_LOCK((su_node_ptr),flag);
         if (flag == TRUE)
         {
            m_AVD_SET_AVND_SU_ADMIN(cb,(su_node_ptr),NCS_ADMIN_STATE_LOCK);
         }
      }
      
      if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
      {
         /* SU oper list are empty.
          * Do the functionality as in stable state to verify if 
          * new assignments can be done. If yes change state to SG realign.
          * If no new assignments change state to stable.
          */
         if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
         {
            /* all the assignments have already been done in the SG. */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }else
         {
            /* new assignments need to be done in the SG. */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
      }else
      {
         /* Change state to SG realign. */
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
      }
   }else /* if(su->sg_of_su->su_oper_list.su == su) */
   {         
      /* the SU is not the same as the SU in the list */         
      if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
      {
         /* The SU is active SU, identify the inservice standby SU for this SU.
          */
         o_susi = avd_sg_npm_su_othr(cb,su);
         
         if ((o_susi != AVD_SU_SI_REL_NULL) &&
            (o_susi->su->readiness_state == NCS_IN_SERVICE))
         {
            /* The standby SU w.r.t this SU is inservice has standby 
             * SI assignment from another SU, send remove message for 
             * each of those SI assignment to the standby SU and 
             * send D2N -INFO_SU_SI_ASSIGN message modify active all 
             * assignments to that SU. Add the SU to the oper list.                   
             */
            avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
            avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
            /* Change state to SG realign. */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
         
      }/* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      
      /* Free all the SI assignments to this SU.*/
      avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
      
   } /* else (su->sg_of_su->su_oper_list.su == su) */
   
   return;

}


/*****************************************************************************
 * Function: avd_sg_npm_node_fail_si_oper
 *
 * Purpose:  This function is a subfunctionality of avd_sg_npm_node_fail_func.
 * It is called if the SG is in AVD_SG_FSM_SI_OPER.
 *
 * Input: cb - the AVD control block
 *        su - The SU that has faulted because of the node failure.
 *        
 *
 * Returns: None.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static void avd_sg_npm_node_fail_si_oper(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU_SI_REL *l_susi, *o_susi, *ot_susi;

   l_susi = o_susi =  AVD_SU_SI_REL_NULL;
   
   if (su->sg_of_su->admin_si->list_of_sisu->su == su)
   {
      l_susi = su->sg_of_su->admin_si->list_of_sisu;
      o_susi = l_susi->si_next;
   }else if ((su->sg_of_su->admin_si->list_of_sisu->si_next != AVD_SU_SI_REL_NULL) &&
            (su->sg_of_su->admin_si->list_of_sisu->si_next->su == su))
   {
      o_susi = su->sg_of_su->admin_si->list_of_sisu;
      l_susi = o_susi->si_next;
   }
   
   if (l_susi == AVD_SU_SI_REL_NULL)
   {
      /* SI doesn't have any assignment with the SU. */
      
      if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
      {
         /* The SU is active SU, identify the inservice standby SU for this SU.
          */
         
         ot_susi = avd_sg_npm_su_othr(cb,su);
         
         if ((ot_susi != AVD_SU_SI_REL_NULL) &&
            (ot_susi->su->readiness_state == NCS_IN_SERVICE) &&
             (ot_susi->state == SA_AMF_HA_STANDBY)&&
             (ot_susi->fsm == AVD_SU_SI_STATE_ASGND))
         {
            /* The standby SU w.r.t this SU is inservice has standby 
             * SI assignment from another SU, send remove message for 
             * each of those SI assignment to the standby SU and 
             * send D2N -INFO_SU_SI_ASSIGN message modify active all 
             * assignments to that SU. Add the SU to the oper list.                   
             */
            avd_sg_npm_su_chk_snd(cb,ot_susi->su,su);
            avd_sg_su_oper_list_add(cb,ot_susi->su,FALSE);
            /* Change state to SG realign. */
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
         
      }/* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
   
      /* Free all the SI assignments to this SU.*/
      avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
      
   }else/* if (l_susi == AVD_SU_SI_REL_NULL) */
   {
      /* SI has assignment with the SU. */
      if (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH)
      {
         /* the admin pointer SI has switch operation as TRUE. */
         if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
            (l_susi->fsm == AVD_SU_SI_STATE_MODIFY))
         {
            /* this SI relation with the SU is quiesced assigning. */
            if (o_susi != AVD_SU_SI_REL_NULL)
            {
               if (o_susi->su->readiness_state == NCS_IN_SERVICE)
               {
                  /* The SUs corresponding standby/quiesced SU is in-service.
                   * If the standby SU has standby SI assignment from 
                   * another SU, send remove message for each of those SI 
                   * assignment to the standby SU and send D2N-INFO_SU_SI_ASSIGN
                   * message modify active all assignments to that SU.
                   * Add the standby SU to operation list.
                   */
                  avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }else
               {
                  /* Add the standby SU to operation list. */
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }
            }            
            
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            
            /* Change the SI switch state to false. Remove the SI from the 
             * SI admin pointer.
             */
            m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
            m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
            
          
            if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
            {
               /* both the SI admin pointer and SU oper list are empty.
                * Do the functionality as in stable state to verify if 
                * new assignments can be done. If yes stay in the same state.
                * If no new assignments change state to stable.
                */
               if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
               {
                  /* all the assignments have already been done in the SG. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }else
               {
                  /* Change state to SG realign. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }
            }else
            {
               /* Change state to SG realign. */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            }
            
         }/* if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
            (l_susi->fsm == AVD_SU_SI_STATE_MODIFY)) */
         else if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
            (l_susi->fsm == AVD_SU_SI_STATE_ASGND))
         {
            /* this SI relation with the SU is quiesced assigned. */
            if (o_susi != AVD_SU_SI_REL_NULL)
            {
               /* Add the other SU to operation list. */
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
            }
            /* Change the SI switch state to false. Remove the SI from the 
             * SI admin pointer.
             */
            m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
            m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
            
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            
            if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
            {
               /* both the SI admin pointer and SU oper list are empty.
                * Do the functionality as in stable state to verify if 
                * new assignments can be done. If yes stay in the same state.
                * If no new assignments change state to stable.
                */
               if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
               {
                  /* all the assignments have already been done in the SG. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }else
               {
                  /* Change state to SG realign. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }
            }else
            {
               /* Change state to SG realign. */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            }
         }/* else if ((l_susi->state == SA_AMF_HA_QUIESCED) &&
            (l_susi->fsm == AVD_SU_SI_STATE_ASGND))*/
         else if (l_susi->state == SA_AMF_HA_STANDBY)
         {  
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
         }/* else if (l_susi->state == SA_AMF_HA_STANDBY) */
         else if (l_susi->state == SA_AMF_HA_ACTIVE)
         {
            /* this SI relation with the SU is standby. */
            if (o_susi != AVD_SU_SI_REL_NULL)
            {
               if (o_susi->su->readiness_state == NCS_IN_SERVICE)
               {
                  /* The SUs corresponding quiesced SU is in-service.
                   * send D2N-INFO_SU_SI_ASSIGN
                   * message modify active all assignments to that SU.
                   * Add the quiesced SU to operation list.
                   */
                   avd_sg_su_si_mod_snd(cb,o_susi->su,SA_AMF_HA_ACTIVE);
                  /* Add the quiesced SU to operation list. */
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }else
               {
                  /* Add the quiesced SU to operation list. */
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }
            }
            
            /* Change the SI switch state to false. Remove the SI from the 
             * SI admin pointer.
             */
            m_AVD_SET_SI_SWITCH(cb,(su->sg_of_su->admin_si),AVSV_SI_TOGGLE_STABLE);
            m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));
            
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            
            if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
            {
               /* both the SI admin pointer and SU oper list are empty.
                * Do the functionality as in stable state to verify if 
                * new assignments can be done. If yes stay in the same state.
                * If no new assignments change state to stable.
                */
               if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
               {
                  /* all the assignments have already been done in the SG. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }else
               {
                  /* Change state to SG realign. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }
            }else
            {
               /* Change state to SG realign. */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            }            
         } /* else if (l_susi->state == SA_AMF_HA_ACTIVE) */
            
      }else /* if (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) */
      {
         /* admin pointer SI admin operation is LOCK/shutdown. */
         if ((l_susi->state == SA_AMF_HA_QUIESCING) ||
            (l_susi->state == SA_AMF_HA_QUIESCED))
         {
            /* Change the admin operation to UNLOCK. */
            m_AVD_SET_SI_ADMIN(cb,(su->sg_of_su->admin_si),NCS_ADMIN_STATE_UNLOCK);
            
            if (o_susi != AVD_SU_SI_REL_NULL)
            {
               if (o_susi->su->readiness_state == NCS_IN_SERVICE)
               {
                  /* The SUs corresponding standby/quiesced SU is in-service.
                   * If the standby SU has standby SI assignment from 
                   * another SU, send remove message for each of those SI 
                   * assignment to the standby SU and send D2N-INFO_SU_SI_ASSIGN
                   * message modify active all assignments to that SU.
                   * Add the standby SU to operation list.
                   */
                  avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }else
               {
                  /* Add the standby SU to operation list. */
                  avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               }
            }
            
            
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            
            /*  Remove the SI from the SI admin pointer.  */
            m_AVD_CLEAR_SG_ADMIN_SI(cb,(su->sg_of_su));               
          
            if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
            {
               /* both the SI admin pointer and SU oper list are empty.
                * Do the functionality as in stable state to verify if 
                * new assignments can be done. If yes stay in the same state.
                * If no new assignments change state to stable.
                */
               if (avd_sg_npm_su_chose_asgn(cb,su->sg_of_su) == AVD_SU_NULL)
               {
                  /* all the assignments have already been done in the SG. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }else
               {
                  /* Change state to SG realign. */
                  m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
                  m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
               }
            }else
            {
               /* Change state to SG realign. */
               m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
               m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
            }      
            
         }/* if ((l_susi->state == SA_AMF_HA_QUIESCING) ||
            (l_susi->state == SA_AMF_HA_QUIESCED)) */
         else if (l_susi->state == SA_AMF_HA_STANDBY)
         {  
            /* Free all the SI assignments to this SU.*/
            avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
            
         }/* else if (l_susi->state == SA_AMF_HA_STANDBY) */
      }/* else (su->sg_of_su->admin_si->si_switch == AVSV_SI_TOGGLE_SWITCH) */
         
      
   }/* else (l_susi == AVD_SU_SI_REL_NULL) */
   

   return;

}

/*****************************************************************************
 * Function: avd_sg_npm_node_fail_func
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
 * NOTES: This is a N+M redundancy model specific function.
 *
 * 
 **************************************************************************/

void avd_sg_npm_node_fail_func(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU *o_su;
   AVD_SU_SI_REL *o_susi;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_node_fail_func");
   m_AVD_LOG_RCVD_VAL(((long)su));
   
   m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
   
   if(su->list_of_susi == AVD_SU_SI_REL_NULL)
      return;
   
   switch(su->sg_of_su->sg_fsm_state)
   {
   case AVD_SG_FSM_STABLE:
      
      if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
      {
         /* The SU is active SU, identify the inservice standby SU for this SU.
          * If standby SU doesnt exists stay in the stable state.
          */
         
         o_susi = avd_sg_npm_su_othr(cb,su);
         
         if (o_susi != AVD_SU_SI_REL_NULL)
         {
            if ((o_susi->su->readiness_state == NCS_IN_SERVICE) &&
                (o_susi->state == SA_AMF_HA_STANDBY))
            {
               /* The standby SU w.r.t this SU is inservice has standby 
                * SI assignment from another SU, send remove message for 
                * each of those SI assignment to the standby SU and 
                * send D2N -INFO_SU_SI_ASSIGN message modify active all 
                * assignments to that SU. Add the SU to the oper list.                   
                */
               avd_sg_npm_su_chk_snd(cb,o_susi->su,su);
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
            }else
            {
               /* the standby is OOS. Add the standby SU to operation list.*/
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               
            }
            /*Change state to SG_realign.*/
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         } /* if (o_su != AVD_SU_NULL) */
         
         /* Free all the SI assignments to this SU.*/
         avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
         
      }else /* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      {
         /* means standby */
         /* identify an SU in the list that is in-service and 
          * send D2N-INFO_SU_SI_ASSIGN message new standby for all the SIs on
          * this SU to that are in-service SU. Remove and free all the SI 
          * assignments to this SU. Add the newly assigned SU to operation list
          * and Change state to SG_realign. If no other in-service SU exists, 
          * stay in the stable state.
          */
         
         avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
         if ((o_su = avd_sg_npm_su_chose_asgn(cb,su->sg_of_su)) != AVD_SU_NULL)
         {
            /* add the SU to the operation list and change the SG FSM to SG realign. */
            avd_sg_su_oper_list_add(cb,o_su,FALSE);   
            m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
            m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
         }
         
      } /* else (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */

      break; /* case AVD_SG_FSM_STABLE: */
   case AVD_SG_FSM_SG_REALIGN:

      avd_sg_npm_node_fail_sg_relgn(cb,su);

      break; /* case AVD_SG_FSM_SG_REALIGN: */
   case AVD_SG_FSM_SU_OPER:
      
      avd_sg_npm_node_fail_su_oper(cb,su);
   
      break; /* case AVD_SG_FSM_SU_OPER: */
   case AVD_SG_FSM_SI_OPER:
      
      avd_sg_npm_node_fail_si_oper(cb,su);
      
      break; /* case AVD_SG_FSM_SI_OPER: */
   case AVD_SG_FSM_SG_ADMIN:
      if (su->list_of_susi->state == SA_AMF_HA_QUIESCING)
      {
         /* The SU is quiescing.  */
         o_susi = avd_sg_npm_su_othr(cb,su);
         if (o_susi != AVD_SU_SI_REL_NULL)
         {
            /* if the other SU w.r.t to SIs of this SU is not unassigning, 
             * send a D2N-INFO_SU_SI_ASSIGN with a remove all for that SU if it
             * is in-service. Add that SU to the SU oper list.
             */
            if (o_susi->fsm != AVD_SU_SI_STATE_UNASGN)
            {
               avd_sg_su_oper_list_add(cb,o_susi->su,FALSE);
               if (o_susi->su->readiness_state == NCS_IN_SERVICE)
               {
                  avd_sg_su_si_del_snd(cb,o_susi->su);
               }
            }
         }         

      } /* if (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN) */
      
      /* Free all the SI assignments to this SU.*/
      avd_sg_su_asgn_del_util(cb,su,TRUE,FALSE);
      
      /* remove the SU from the operation list. */
      avd_sg_su_oper_list_del(cb,su,FALSE);
      
      if (su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
      {
         if (su->sg_of_su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
         {
            m_AVD_SET_SG_ADMIN(cb,(su->sg_of_su),NCS_ADMIN_STATE_LOCK);
         }
         
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_STABLE);
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
      }

      break; /* case AVD_SG_FSM_SG_ADMIN: */   
   default:
      /* log fatal error about the invalid value */     
      m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
      m_AVD_LOG_INVALID_VAL_FATAL(((uns32)su->sg_of_su->sg_fsm_state));
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      return;
      break;
   } /* switch(su->sg_of_su->sg_fsm_state) */
   
   return;
}

 
/*****************************************************************************
 * Function: avd_sg_npm_su_admin_fail
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
 * NOTES: This is a N+M redundancy model specific function. The avnd pointer
 * value is valid only if this is a SU operation being done because of the node
 * admin change.
 *
 * 
 **************************************************************************/

uns32 avd_sg_npm_su_admin_fail(AVD_CL_CB *cb,AVD_SU *su, AVD_AVND *avnd)
{
   
   uns32 rc;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_su_admin_fail");
   m_AVD_LOG_RCVD_VAL(((long)su));
   m_AVD_LOG_RCVD_VAL(((long)avnd));
   
   m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
   if((cb->init_state != AVD_APP_STATE) &&
      (su->sg_of_su->sg_ncs_spec == SA_FALSE))
   {
      return NCSCC_RC_FAILURE;
   }
   
   switch (su->sg_of_su->sg_fsm_state)
   {
   case AVD_SG_FSM_STABLE:
      if (su->list_of_susi->state == SA_AMF_HA_ACTIVE)
      {
         rc = NCSCC_RC_FAILURE;

         /* this is a active SU. */
         if ((su->admin_state == NCS_ADMIN_STATE_LOCK) ||
         ((avnd != AVD_AVND_NULL) && (avnd->su_admin_state == NCS_ADMIN_STATE_LOCK)))
         {
            /* change the state for all assignments to quiesced. */
            rc = avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED);
         }else if ((su->admin_state == NCS_ADMIN_STATE_SHUTDOWN) ||
         ((avnd != AVD_AVND_NULL) && (avnd->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)))
         {
            /* change the state for all assignments to quiesceing. */
            rc = avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCING);            
         }
         
         if(rc != NCSCC_RC_SUCCESS)
         {
            /* loga fatal error*/
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         /* add the SU to the operation list and change the SG FSM to SU operation. */
         avd_sg_su_oper_list_add(cb,su,FALSE);   
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SU_OPER);
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
      }else /* if (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */
      {
         /* means standby */
         /* change the state for all assignments to unassign and send delete mesage. */
         if(avd_sg_su_si_del_snd(cb,su) != NCSCC_RC_SUCCESS)
         {
            /* log fatal error */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
         
         /* add the SU to the operation list and change the SG FSM to SG realign. */
         avd_sg_su_oper_list_add(cb,su,FALSE);
         if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
            m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
         m_AVD_SET_SG_FSM(cb,(su->sg_of_su),AVD_SG_FSM_SG_REALIGN);
         m_AVD_LOG_RCVD_VAL(su->sg_of_su->sg_fsm_state);
      } /* else (su->list_of_susi->state == SA_AMF_HA_ACTIVE) */      
      break; /* case AVD_SG_FSM_STABLE: */
   case AVD_SG_FSM_SU_OPER:
      if ((su->sg_of_su->su_oper_list.su == su) &&
         (su->list_of_susi->state == SA_AMF_HA_QUIESCING) &&
         ((su->admin_state == NCS_ADMIN_STATE_LOCK) ||
         ((avnd != AVD_AVND_NULL) && (avnd->su_admin_state == NCS_ADMIN_STATE_LOCK))))
      {
         /* If the SU is in the operation list and the SU admin state is lock.
          * send D2N-INFO_SU_SI_ASSIGN modify quiesced message to the SU. 
          */
         if(avd_sg_su_si_mod_snd(cb,su,SA_AMF_HA_QUIESCED) != NCSCC_RC_SUCCESS)
         {
            /* loga fatal error*/
            m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
            return NCSCC_RC_FAILURE;
         }
      }
      break; /* case AVD_SG_FSM_SU_OPER: */
   default:
      /* Log a error */     
      m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
      m_AVD_LOG_INVALID_VAL_ERROR(((uns32)su->sg_of_su->sg_fsm_state));
      m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
      return NCSCC_RC_FAILURE;
      break;
   } /* switch (su->sg_of_su->sg_fsm_state) */
   
   return NCSCC_RC_SUCCESS;
}

 
/*****************************************************************************
 * Function: avd_sg_npm_si_admin_down
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
 * NOTES: This is a N+M redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_npm_si_admin_down(AVD_CL_CB *cb,AVD_SI *si)
{
   AVD_SU_SI_REL *a_susi;
   AVD_SU_SI_STATE old_fsm_state;
   SaAmfHAStateT old_state;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_si_admin_down");
   m_AVD_LOG_RCVD_VAL(((long)si));
   
   m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
   
   if((cb->init_state != AVD_APP_STATE) &&
      (si->sg_of_si->sg_ncs_spec == SA_FALSE))
   {
      return NCSCC_RC_FAILURE;
   }
   if (si->list_of_sisu == AVD_SU_SI_REL_NULL)
   {
      return NCSCC_RC_SUCCESS;
   }
   switch (si->sg_of_si->sg_fsm_state)
   {
   case AVD_SG_FSM_STABLE:
      
      if (si->list_of_sisu->state == SA_AMF_HA_ACTIVE)
      {
         a_susi = si->list_of_sisu;
      }else
      {
         a_susi = si->list_of_sisu->si_next;
      }
      if (si->admin_state == NCS_ADMIN_STATE_LOCK)
      {
         /* SI lock. Identify the SU that is active send D2N-INFO_SU_SI_ASSIGN
          * modify quiesced for only this SI. Change state to
          * SI_operation state. Add it to admin SI pointer.
          */

         /* change the state of the SI to quiesced */
         
         old_state = a_susi->state;
         old_fsm_state = a_susi->fsm;
         a_susi->state = SA_AMF_HA_QUIESCED;
         a_susi->fsm = AVD_SU_SI_STATE_MODIFY;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, a_susi, AVSV_CKPT_AVD_SU_SI_REL);
         avd_gen_su_ha_state_changed_trap(cb, a_susi);
         if(avd_snd_susi_msg(cb,a_susi->su,a_susi,AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE)
         {
            /* loga fatal error*/
            m_AVD_LOG_INVALID_VAL_ERROR(((long)a_susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(a_susi->su->name_net.value,a_susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(a_susi->si->name_net.value,a_susi->si->name_net.length);
            a_susi->state = old_state;
            a_susi->fsm = old_fsm_state;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, a_susi, AVSV_CKPT_AVD_SU_SI_REL);
            avd_gen_su_ha_state_changed_trap(cb, a_susi);
            return NCSCC_RC_FAILURE;
         }
         
         
         /* add the SI to the admin list and change the SG FSM to SI operation. */
         m_AVD_SET_SG_ADMIN_SI(cb,si);
         m_AVD_SET_SG_FSM(cb,(si->sg_of_si),AVD_SG_FSM_SI_OPER);
         m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
      } /* if (si->admin_state == NCS_ADMIN_STATE_LOCK) */
      else if (si->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         /* SI shutdown. Identify the SU that is active 
          * send D2N-INFO_SU_SI_ASSIGN modify quiescing for only this SI. 
          * Change state to SI_operation state. Add it to admin SI pointer.
          */
         

         /* change the state of the SI to quiescing */
         
         old_state = a_susi->state;
         old_fsm_state = a_susi->fsm;
         a_susi->state = SA_AMF_HA_QUIESCING;
         a_susi->fsm = AVD_SU_SI_STATE_MODIFY;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, a_susi, AVSV_CKPT_AVD_SU_SI_REL);
         avd_gen_su_ha_state_changed_trap(cb, a_susi);
         if(avd_snd_susi_msg(cb,a_susi->su,a_susi,AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE)
         {
            /* loga fatal error*/
            m_AVD_LOG_INVALID_VAL_ERROR(((long)a_susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(a_susi->su->name_net.value,a_susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(a_susi->si->name_net.value,a_susi->si->name_net.length);
            a_susi->state = old_state;
            a_susi->fsm = old_fsm_state;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, a_susi, AVSV_CKPT_AVD_SU_SI_REL);
            avd_gen_su_ha_state_changed_trap(cb, a_susi);
            return NCSCC_RC_FAILURE;
         }
         
         /* add the SI to the admin list and change the SG FSM to SI operation. */
         m_AVD_SET_SG_ADMIN_SI(cb,si);
         m_AVD_SET_SG_FSM(cb,(si->sg_of_si),AVD_SG_FSM_SI_OPER);
         m_AVD_LOG_RCVD_VAL(si->sg_of_si->sg_fsm_state);
      } /* if (si->admin_state == NCS_ADMIN_STATE_SHUTDOWN) */
      break; /* case AVD_SG_FSM_STABLE: */
   case AVD_SG_FSM_SI_OPER:
      if ((si->sg_of_si->admin_si == si) &&
         (si->admin_state == NCS_ADMIN_STATE_LOCK))
      {
         /* If the SI is in the admin pointer and the SI admin state is shutdown, 
          * change the admin state of the SI to lock and 
          * send D2N-INFO_SU_SI_ASSIGN modify quiesced message to the SU that
          * is being assigned quiescing state. Send a D2N-INFO_SU_SI_ASSIGN 
          * remove message for this SI to the other SU.
          */
         if (si->list_of_sisu->state == SA_AMF_HA_QUIESCING)
         {
            a_susi = si->list_of_sisu;
         }else
         {
            a_susi = si->list_of_sisu->si_next;
         }
         

         /* change the state of the SI to quiesced */
         
         old_state = a_susi->state;
         old_fsm_state = a_susi->fsm;
         a_susi->state = SA_AMF_HA_QUIESCED;
         a_susi->fsm = AVD_SU_SI_STATE_MODIFY;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, a_susi, AVSV_CKPT_AVD_SU_SI_REL);
         avd_gen_su_ha_state_changed_trap(cb, a_susi);
         if(avd_snd_susi_msg(cb,a_susi->su,a_susi,AVSV_SUSI_ACT_MOD) == NCSCC_RC_FAILURE)
         {
            /* loga fatal error*/
            m_AVD_LOG_INVALID_VAL_ERROR(((long)a_susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(a_susi->su->name_net.value,a_susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(a_susi->si->name_net.value,a_susi->si->name_net.length);
            a_susi->state = old_state;
            a_susi->fsm = old_fsm_state;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, a_susi, AVSV_CKPT_AVD_SU_SI_REL);
            avd_gen_su_ha_state_changed_trap(cb, a_susi);
            return NCSCC_RC_FAILURE;
         }         
      }
      break; /* case AVD_SG_FSM_SI_OPER: */
   default:
      /* Log a error */    
      m_AVD_LOG_INVALID_VAL_ERROR(((long)si));
      m_AVD_LOG_INVALID_VAL_ERROR(((uns32)si->sg_of_si->sg_fsm_state));
      m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(si->name_net.value,si->name_net.length);
      return NCSCC_RC_FAILURE;
      break;
   } /* switch (si->sg_of_si->sg_fsm_state) */
   
   return NCSCC_RC_SUCCESS;
}

 
/*****************************************************************************
 * Function: avd_sg_npm_sg_admin_down
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
 * NOTES: This is a N+M redundancy model specific function.
 *
 * 
 **************************************************************************/

uns32 avd_sg_npm_sg_admin_down(AVD_CL_CB *cb,AVD_SG *sg)
{
   AVD_SU *i_su;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_npm_sg_admin_down");
   m_AVD_LOG_RCVD_VAL(((long)sg));
   
   m_AVD_LOG_RCVD_VAL(sg->sg_fsm_state);
   
   if((cb->init_state != AVD_APP_STATE) &&
      (sg->sg_ncs_spec == SA_FALSE))
   {
      return NCSCC_RC_FAILURE;
   }
   
   switch (sg->sg_fsm_state)
   {
   case AVD_SG_FSM_STABLE:
      if (sg->admin_state == NCS_ADMIN_STATE_LOCK)
      {
         /* SG lock. Identify the SUs that are active send D2N-INFO_SU_SI_ASSIGN
          * modify quiesced all for the SUs. identify standby SUs send 
          * D2N-INFO_SU_SI_ASSIGN remove all for standby SU, add each of these 
          * SUs to the SU oper list. Change state to SG_admin. If no active SU 
          * exist, no action, stay in stable state.
          */
         
         i_su = sg->list_of_su;
         while (i_su != AVD_SU_NULL)
         {
            if (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
            {
               if (i_su->list_of_susi->state == SA_AMF_HA_ACTIVE)
                  avd_sg_su_si_mod_snd(cb,i_su,SA_AMF_HA_QUIESCED);
               else
                  avd_sg_su_si_del_snd(cb,i_su);
               
               /* add the SU to the operation list */
               avd_sg_su_oper_list_add(cb,i_su,FALSE);
            }
            
            i_su = i_su->sg_list_su_next;
         }         

      } /* if (sg->admin_state == NCS_ADMIN_STATE_LOCK) */
      else if (sg->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         /* SG shutdown. Identify the SU that are  active send 
          * D2N-INFO_SU_SI_ASSIGN modify quiescing all for this SUs, add 
          * each of these SUs to the SU oper list.Change state to SG_admin. 
          * If no active SU exist, change the SG admin state to LOCK, stay 
          * in stable state.
          */
         i_su = sg->list_of_su;
         while (i_su != AVD_SU_NULL)
         {
            if (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
            {
               if (i_su->list_of_susi->state == SA_AMF_HA_ACTIVE)
               {
                  avd_sg_su_si_mod_snd(cb,i_su,SA_AMF_HA_QUIESCING);
         
                  /* add the SU to the operation list */
                  avd_sg_su_oper_list_add(cb,i_su,FALSE);
               }               
            }            
            i_su = i_su->sg_list_su_next;
         }
         
      } /* if (sg->admin_state == NCS_ADMIN_STATE_SHUTDOWN) */
      else
      {
         return NCSCC_RC_FAILURE; 
      }
      
      if (sg->su_oper_list.su != AVD_SU_NULL)
      {      
         m_AVD_SET_SG_FSM(cb,(sg),AVD_SG_FSM_SG_ADMIN);
         m_AVD_LOG_RCVD_VAL(sg->sg_fsm_state);
      }

      break; /* case AVD_SG_FSM_STABLE: */
   case AVD_SG_FSM_SG_ADMIN:
      if (sg->admin_state == NCS_ADMIN_STATE_LOCK)
      {
         /* If the SG admin state is shutdown, change the admin state of the SG
          * to lock and send D2N-INFO_SU_SI_ASSIGN modify quiesced messages to
          * the SUs with quiescing assignment. Also send D2N-INFO_SU_SI_ASSIGNs
          * with remove all to the SUs with standby assignment and add the 
          * standby SUs to the SU oper list.
          */
         i_su = sg->list_of_su;
         while (i_su != AVD_SU_NULL)
         {
            if (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
            {
               if (i_su->list_of_susi->state == SA_AMF_HA_QUIESCING)
               {
                  avd_sg_su_si_mod_snd(cb,i_su,SA_AMF_HA_QUIESCED);
               }
               else if ((i_su->list_of_susi->state == SA_AMF_HA_STANDBY) &&
                     (i_su->list_of_susi->fsm == AVD_SU_SI_STATE_ASGND))
               {
                  avd_sg_su_si_del_snd(cb,i_su);
                  /* add the SU to the operation list */
                  avd_sg_su_oper_list_add(cb,i_su,FALSE);
               }
               
            }
            
            i_su = i_su->sg_list_su_next;
         }         
         
      } /* if (sg->admin_state == NCS_ADMIN_STATE_LOCK) */
      break; /* case AVD_SG_FSM_SG_ADMIN: */
   default:
      /* Log a error */    
      m_AVD_LOG_INVALID_VAL_FATAL(((long)sg));
      m_AVD_LOG_INVALID_VAL_FATAL(((uns32)sg->sg_fsm_state));
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(sg->name_net.value,sg->name_net.length);
      return NCSCC_RC_FAILURE;
      break;
   } /* switch (sg->sg_fsm_state) */
   
   return NCSCC_RC_SUCCESS;
}
