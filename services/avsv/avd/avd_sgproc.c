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

  DESCRIPTION: This file contains the SG processing routines for the AVD.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_new_assgn_susi - creates and assigns the role to
                       the SUSI relationship and sends the message.
  avd_su_oper_state_func - processes SU operational state change message.
  avd_su_si_assign_func - Processes SUSI message response.
  avd_sg_app_node_su_inst_func - processes the request to instantiate all the
                                 application SUs on a node.
  avd_sg_app_su_inst_func - processes the request to evaluate the SG for
                           Instantiations and terminations of SUs in the SG.
  avd_sg_app_node_admin_func - processes the request to do UNLOCK or LOCK or shutdown
                               of the AMF application SUs on the node.
  avd_sg_app_sg_admin_func -  processes the request to do UNLOCK or LOCK or shutdown
                              of the AMF application SUs on the SG.
  avd_node_susi_fail_func -  un assign all the SUSIs on the faulted node.
  avd_sg_su_oper_list_add - Util Add SU to the list of SUs undergoing operation.
  avd_sg_su_oper_list_del - Util Del SU to the list of SUs undergoing operation.
  avd_sg_su_asgn_del_util -  utility routine that changes the assigning or
                             modifing FSM to assigned for all the SUSIs for
                             the SU. If delete it removes all the SUSIs 
                             assigned to the SU.  
  avd_sg_su_si_mod_snd - utility function that assigns the state specified
                            to all the SUSIs of the SU.
  avd_sg_su_si_del_snd - utility functions that deletes all the SUSIs to a SU.  
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/*****************************************************************************
 * Function: avd_new_assgn_susi
 *
 * Purpose:  This function creates and assigns the given role to the SUSI
 * relationship and sends the message to the AVND having the
 * SU accordingly. 
 *
 * Input: cb - the AVD control block
 *        su - The pointer to SU.
 *        si - The pointer to SI.
 *      role - The HA role that needs to be assigned.
 *      ckpt - Flag indicating if called on standby.
 *      ret_ptr - Pointer to the pointer of the SUSI structure created.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_new_assgn_susi(AVD_CL_CB *cb,AVD_SU *su, AVD_SI *si,
                         SaAmfHAStateT role, NCS_BOOL ckpt, 
                         AVD_SU_SI_REL **ret_ptr)
{
   AVD_SU_SI_REL *susi;
   AVD_COMP_CSI_REL *compcsi;
   NCS_BOOL l_flag;
   AVD_COMP *l_comp;
   AVD_CSI *l_csi;

   m_AVD_LOG_FUNC_ENTRY("avd_new_assgn_susi");
   
   m_AVD_LOG_RCVD_VAL(((long)su));
   m_AVD_LOG_RCVD_VAL(((long)si));
   
   if ((susi = avd_susi_struc_crt(cb,si,su))
      == AVD_SU_SI_REL_NULL)
   {
      /* log error that the AVD is in degraded situation */      
      m_AVD_LOG_INVALID_VAL_FATAL(((uns32)role));
      m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
      m_AVD_LOG_INVALID_VAL_FATAL(((long)si));
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(si->name_net.value,si->name_net.length);
      return NCSCC_RC_FAILURE;
   }

   m_AVD_LOG_RCVD_VAL(((long)susi));
   
   susi->fsm = AVD_SU_SI_STATE_ASGN;
   susi->state = role;

   l_flag = TRUE;

   l_csi = si->list_of_csi;

   /* reset the assign flag */
   l_comp = su->list_of_comp;
   while (l_comp != AVD_COMP_NULL)
   {
      l_comp->assign_flag = FALSE;
      l_comp = l_comp->su_comp_next;
   }
   
   while((l_csi != AVD_CSI_NULL) && (l_flag == TRUE))
   {
      /* find the component that has to be assigned this CSI */

      l_comp = su->list_of_comp;
      while (l_comp != AVD_COMP_NULL)
      {
         if ((l_comp->row_status == NCS_ROW_ACTIVE) && (l_comp->assign_flag == FALSE) &&
            (avd_comp_cs_type_find_match(cb,l_csi,l_comp) == NCSCC_RC_SUCCESS))
            break;

         l_comp = l_comp->su_comp_next;
      }

      if (l_comp == AVD_COMP_NULL)
      {
         
         l_flag = FALSE;
         continue;
      }

      if ((compcsi = avd_compcsi_struc_crt(cb,susi))
         == AVD_COMP_CSI_REL_NULL)
      {
         /* free all the CSI assignments and end this loop */
         m_AVD_LOG_INVALID_VAL_FATAL(((long)l_comp));
         m_AVD_LOG_INVALID_VAL_FATAL(((long)l_csi));
         m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(l_comp->comp_info.name_net.value,l_comp->comp_info.name_net.length);
         m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(l_csi->name_net.value,l_csi->name_net.length);
         avd_compcsi_list_del(cb,susi,TRUE);
         l_flag = FALSE;
         continue;
      }

      m_AVD_LOG_RCVD_VAL(((long)compcsi));
     
      l_comp->assign_flag = TRUE; 
      compcsi->comp = l_comp;
      compcsi->csi = l_csi;
      compcsi->susi = susi;
      /* Add it to the CSI list */
      if (l_csi->list_compcsi == AVD_COMP_CSI_REL_NULL)
      {
         l_csi->list_compcsi = compcsi;
      }else
      {
         compcsi->csi_csicomp_next = l_csi->list_compcsi;
         l_csi->list_compcsi = compcsi;
      }
      l_csi->compcsi_cnt++;
      
      l_csi = l_csi->si_list_of_csi_next;
   } /* while((l_csi != AVD_CSI_NULL) && (l_flag == TRUE)) */

   if (l_flag == FALSE)
   {
      /* log an error that a component type for which the CSI has to be
       * assigned is missing.
       */
      m_AVD_LOG_INVALID_VAL_FATAL(((uns32)role));
      m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
      m_AVD_LOG_INVALID_VAL_FATAL(((long)l_csi));
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(l_csi->name_net.value,l_csi->name_net.length);
      /* free all the CSI assignments and end this loop */
      avd_compcsi_list_del(cb,susi,TRUE);
      /* Unassign the SUSI */
      avd_susi_struc_del(cb,susi,TRUE);
      return NCSCC_RC_FAILURE;
   }

   /* Now send the message about the SU SI assignment to
    * the AvND. Send message only if this function is not 
    * called while doing checkpoint update.
    */
   
   if (FALSE == ckpt) 
   {
      if (role == SA_AMF_HA_ACTIVE)
      {
         ++ su->si_curr_active;
      }
      else
      {
         ++ su->si_curr_standby;
      }

      if (avd_snd_susi_msg(cb,su,susi,AVSV_SUSI_ACT_ASGN)
         != NCSCC_RC_SUCCESS)
      {
         m_AVD_LOG_INVALID_VAL_ERROR(((uns32)role));
         m_AVD_LOG_INVALID_VAL_ERROR(((long)susi));
         m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
         m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->si->name_net.value,susi->si->name_net.length);      
         /* free all the CSI assignments and end this loop */
         avd_compcsi_list_del(cb,susi,TRUE);
         /* Unassign the SUSI */
         avd_susi_struc_del(cb,susi,TRUE);
         if (role == SA_AMF_HA_ACTIVE)
         {
            -- su->si_curr_active;
         }else
         {
            -- su->si_curr_standby;
         }
         
         return NCSCC_RC_FAILURE;
      }
      
      m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_AVD_SU_CONFIG);
      if (susi->si->list_of_sisu->si_next == AVD_SU_SI_REL_NULL)
      {
         avd_gen_si_oper_state_chg_trap(cb,susi->si);
      }

      avd_gen_su_ha_state_changed_trap(cb,susi);
   }

   *ret_ptr = susi;
   return NCSCC_RC_SUCCESS;

}



/*****************************************************************************
 * Function: avd_su_oper_state_func
 *
 * Purpose:  This function is the handler for the operational state change event
 * indicating the arrival of the operational state change message from 
 * node director. It will process the message and call the redundancy model
 * specific event processing routine.
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

void avd_su_oper_state_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   AVD_DND_MSG *n2d_msg;
   AVD_AVND *avnd;
   AVD_SU *su,*i_su;
   NCS_READINESS_STATE old_state;
   AVD_AVND *su_node_ptr = NULL;
   
   m_AVD_LOG_FUNC_ENTRY("avd_su_oper_state_func");

   if (evt->info.avnd_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   n2d_msg = evt->info.avnd_msg;
   
   m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);

   if ((avnd = 
   avd_msg_sanity_chk(cb,evt,n2d_msg->msg_info.n2d_opr_state.node_id,AVSV_N2D_OPERATION_STATE_MSG))
   == AVD_AVND_NULL)
   {
      /* sanity failed return */
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   }


   if ((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
      (avnd->node_state == AVD_AVND_STATE_GO_DOWN) ||
      ((avnd->rcv_msg_id + 1) != n2d_msg->msg_info.n2d_opr_state.msg_id))
   {
      /* log information error that the node is in invalid state */
      m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
      m_AVD_LOG_INVALID_VAL_ERROR(avnd->rcv_msg_id);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   }
          
   /* 
    * Send the Ack message to the node, indicationg that the message with this
    * message ID is received successfully.
    */
   m_AVD_SET_AVND_RCV_ID(cb,avnd,(n2d_msg->msg_info.n2d_opr_state.msg_id));

   if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS)
   {
      /* log error that the director is not able to send the message */
      m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
   }
  
   /* Find and validate the SU. */
   
   /* get the SU from the tree */

   if((su = avd_su_struc_find(cb,n2d_msg->msg_info.n2d_opr_state.su_name_net,FALSE))
      == AVD_SU_NULL)
   {
      /* log error that the SU mentioned in the message is not present */      
      m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_CRITICAL,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   }

   if (su->row_status != NCS_ROW_ACTIVE)
   {
      /* log error that the SU is in invalid state */      
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   }

   m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

   if (su_node_ptr != avnd)
   {
      /* log fatal error that the SU is in invalid state */      
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   } 

   /* Verify that the SU and node oper state is diabled and rcvr is failfast */
   if((n2d_msg->msg_info.n2d_opr_state.su_oper_state == NCS_OPER_STATE_DISABLE) &&
         (n2d_msg->msg_info.n2d_opr_state.node_oper_state == NCS_OPER_STATE_DISABLE) &&
         (n2d_msg->msg_info.n2d_opr_state.rec_rcvr == AVSV_AMF_NODE_FAILFAST))
   {
      /* as of now do the same opearation as ncs su failure */
      m_AVD_SET_SU_OPER(cb,su,NCS_OPER_STATE_DISABLE);
  
     if((avnd->type == AVSV_AVND_CARD_SYS_CON) && (avnd->node_info.nodeId == cb->node_id_avd))
     { 
         m_NCS_DBG_PRINTF("\n---------------------------------------------------------------\n"); 
         m_NCS_DBG_PRINTF("\n Component in %s requested FAILFAST\n",su->name_net.value); 
         m_NCS_DBG_PRINTF("\n---------------------------------------------------------------\n"); 
     }

      avd_nd_ncs_su_failed(cb,avnd);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   }  
   
   /* Verify that the SU operation state is disable and do the processing.*/
   if(n2d_msg->msg_info.n2d_opr_state.su_oper_state == NCS_OPER_STATE_DISABLE)
   {
      /* if the SU is NCS SU, call the node FSM routine to handle the failure.
       */
      if(su->sg_of_su->sg_ncs_spec == SA_TRUE)
      {
         m_AVD_SET_SU_OPER(cb,su,NCS_OPER_STATE_DISABLE);
       
         avd_nd_ncs_su_failed(cb,avnd);
         avsv_dnd_msg_free(n2d_msg);
         evt->info.avnd_msg = NULL;
         return;
      }
      
      /* If the cluster timer hasnt expired, mark the SU operation state
       * disabled.      
       */
      
      if(cb->init_state == AVD_INIT_DONE)
      {
         m_AVD_SET_SU_OPER(cb,su,NCS_OPER_STATE_DISABLE);
         m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
         if(n2d_msg->msg_info.n2d_opr_state.node_oper_state == NCS_OPER_STATE_DISABLE)
         {
            /* Mark the node operational state as disable and make all the
             * application SUs in the node as O.O.S.
             */
            m_AVD_SET_AVND_OPER(cb,avnd,NCS_OPER_STATE_DISABLE);
            i_su = avnd->list_of_su;
            while(i_su != AVD_SU_NULL)
            {
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);
               i_su = i_su->avnd_list_su_next;
            }
         } /* if(n2d_msg->msg_info.n2d_opr_state.node_oper_state == NCS_OPER_STATE_DISABLE)*/
      }/* if(cb->init_state == AVD_INIT_DONE) */
      else if(cb->init_state == AVD_APP_STATE)
      {
         m_AVD_SET_SU_OPER(cb,su,NCS_OPER_STATE_DISABLE);
         m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
         if(n2d_msg->msg_info.n2d_opr_state.node_oper_state == NCS_OPER_STATE_DISABLE)
         {
            /* Mark the node operational state as disable and make all the
             * application SUs in the node as O.O.S. Also call the SG FSM
             * to do the reallignment of SIs for assigned SUs.
             */
            m_AVD_SET_AVND_OPER(cb,avnd,NCS_OPER_STATE_DISABLE);
            i_su = avnd->list_of_su;
            while(i_su != AVD_SU_NULL)
            {

               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);
               if (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
               {
                  /* Since assignments exists call the SG FSM.
                   */
                  switch(i_su->sg_of_su->su_redundancy_model)
                  {
                  case AVSV_SG_RED_MODL_2N:
                     if (avd_sg_2n_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
                     {
                        /* Bad situation. Free the message and return since
                         * receive id was not processed the event will again
                         * comeback which we can then process.
                         */
                        
                        /* log error about the failure */                    
                        m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
                        avsv_dnd_msg_free(n2d_msg);
                        evt->info.avnd_msg = NULL;
                        return;
                     }                  
                     break;

                  case AVSV_SG_RED_MODL_NWAYACTV:
                     if (avd_sg_nacvred_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
                     {
                        /* Bad situation. Free the message and return since
                         * receive id was not processed the event will again
                         * comeback which we can then process.
                         */
                        
                        /* log error about the failure */                    
                        m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
                        avsv_dnd_msg_free(n2d_msg);
                        evt->info.avnd_msg = NULL;
                        return;
                     }                  
                     break;

                  case AVSV_SG_RED_MODL_NWAY:
                     if (avd_sg_nway_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
                     {
                        /* Bad situation. Free the message and return since
                         * receive id was not processed the event will again
                         * comeback which we can then process.
                         */
                        
                        /* log error about the failure */                    
                        m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
                        avsv_dnd_msg_free(n2d_msg);
                        evt->info.avnd_msg = NULL;
                        return;
                     }                  
                     break;
                     
                  case AVSV_SG_RED_MODL_NPM:
                     if (avd_sg_npm_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
                     {
                        /* Bad situation. Free the message and return since
                         * receive id was not processed the event will again
                         * comeback which we can then process.
                         */
                        
                        /* log error about the failure */                    
                        m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
                        avsv_dnd_msg_free(n2d_msg);
                        evt->info.avnd_msg = NULL;
                        return;
                     }                  
                     break;


                  case AVSV_SG_RED_MODL_NORED:
                  default:
                     if (avd_sg_nored_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
                     {
                        /* Bad situation. Free the message and return since
                         * receive id was not processed the event will again
                         * comeback which we can then process.
                         */
                        
                        /* log error about the failure */                    
                        m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,i_su->name_net.length);
                        avsv_dnd_msg_free(n2d_msg);
                        evt->info.avnd_msg = NULL;
                        return;
                     }       
                     break;
                  } 

               }
               
               /* Verify the SG to check if any instantiations need
                * to be done for the SG on which this SU exists.
                */
               if(avd_sg_app_su_inst_func(cb, i_su->sg_of_su) == NCSCC_RC_FAILURE)
               {
                  /* Bad situation. Free the message and return since
                   * receive id was not processed the event will again
                   * comeback which we can then process.
                   */
                  
                  /* log error about the failure */                                     
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->sg_of_su->name_net.value,i_su->sg_of_su->name_net.length);
                  avsv_dnd_msg_free(n2d_msg);
                  evt->info.avnd_msg = NULL;
                  return;
               }
               
               i_su = i_su->avnd_list_su_next;
            }/* while(i_su != AVD_SU_NULL)*/
            
         } else  /* if(n2d_msg->msg_info.n2d_opr_state.node_oper_state == NCS_OPER_STATE_DISABLE)*/
         {
            if (su->list_of_susi != AVD_SU_SI_REL_NULL)
            {
               /* Since assignments exists call the SG FSM.
                */
               switch(su->sg_of_su->su_redundancy_model)
               {
               case AVSV_SG_RED_MODL_2N:
                  if (avd_sg_2n_su_fault_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                     
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }            
                  break;

               case AVSV_SG_RED_MODL_NWAY:
                  if (avd_sg_nway_su_fault_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                     
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }            
                  break;

               case AVSV_SG_RED_MODL_NPM:
                  if (avd_sg_npm_su_fault_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                     
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }            
                  break;

               case AVSV_SG_RED_MODL_NWAYACTV:
                  if (avd_sg_nacvred_su_fault_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                     
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }            
                  break;
               case AVSV_SG_RED_MODL_NORED:
               default:
                  if (avd_sg_nored_su_fault_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                     
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }       
                  break;
               }  
               
            }
            
            /* Verify the SG to check if any instantiations need
             * to be done for the SG on which this SU exists.
             */
            if(avd_sg_app_su_inst_func(cb, su->sg_of_su) == NCSCC_RC_FAILURE)
            {
               /* Bad situation. Free the message and return since
                * receive id was not processed the event will again
                * comeback which we can then process.
                */
               
               /* log error about the failure */                                     
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->sg_of_su->name_net.value,su->sg_of_su->name_net.length);
               avsv_dnd_msg_free(n2d_msg);
               evt->info.avnd_msg = NULL;
               return;
            }
            
         } /*else (n2d_msg->msg_info.n2d_opr_state.node_oper_state == NCS_OPER_STATE_DISABLE) */
         
      } /* else if(cb->init_state == AVD_APP_STATE)*/
      
   } /* if(n2d_msg->msg_info.n2d_opr_state.su_oper_state == NCS_OPER_STATE_DISABLE) */
   else if(n2d_msg->msg_info.n2d_opr_state.su_oper_state == NCS_OPER_STATE_ENABLE)
   {
      m_AVD_SET_SU_OPER(cb,su,NCS_OPER_STATE_ENABLE);
      /* if the SU is NCS SU, mark the SU readiness state as in service and call
       * the SG FSM.
       */      
      if(su->sg_of_su->sg_ncs_spec == SA_TRUE)
      {         
         if(su->readiness_state == NCS_OUT_OF_SERVICE)
         {            
            m_AVD_SET_SU_REDINESS(cb,su,NCS_IN_SERVICE);
            /* Run the SG FSM */
            switch(su->sg_of_su->su_redundancy_model)
            {
            case AVSV_SG_RED_MODL_2N:
               if(avd_sg_2n_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
               {
                  /* Bad situation. Free the message and return since
                   * receive id was not processed the event will again
                   * comeback which we can then process.
                   */
                  
                  /* log error about the failure */                                    
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                  m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                  avsv_dnd_msg_free(n2d_msg);
                  evt->info.avnd_msg = NULL;
                  return;
               }       
               break;

            case AVSV_SG_RED_MODL_NWAY:
               if(avd_sg_nway_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
               {
                  /* Bad situation. Free the message and return since
                   * receive id was not processed the event will again
                   * comeback which we can then process.
                   */
                  
                  /* log error about the failure */                                    
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                  m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                  avsv_dnd_msg_free(n2d_msg);
                  evt->info.avnd_msg = NULL;
                  return;
               }       
               break;

            case AVSV_SG_RED_MODL_NPM:
               if(avd_sg_npm_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
               {
                  /* Bad situation. Free the message and return since
                   * receive id was not processed the event will again
                   * comeback which we can then process.
                   */
                  
                  /* log error about the failure */                                    
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                  m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                  avsv_dnd_msg_free(n2d_msg);
                  evt->info.avnd_msg = NULL;
                  return;
               }       
               break;
               
            case AVSV_SG_RED_MODL_NORED:
            default:
               if(avd_sg_nored_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
               {
                  /* Bad situation. Free the message and return since
                   * receive id was not processed the event will again
                   * comeback which we can then process.
                   */
                  
                  /* log error about the failure */                                    
                  m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                  m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                  avsv_dnd_msg_free(n2d_msg);
                  evt->info.avnd_msg = NULL;
                  return;
               }     
               break;
            }  
            
         }
      }else /* if(su->sg_of_su->sg_ncs_spec == SA_TRUE) */
      {
         old_state = su->readiness_state;      
         m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);
         
         if(m_AVD_APP_SU_IS_INSVC(su,su_node_ptr))
         {
            m_AVD_SET_SU_REDINESS(cb,su,NCS_IN_SERVICE);
            if((cb->init_state == AVD_APP_STATE) &&
               (old_state == NCS_OUT_OF_SERVICE))
            {
               /* An application SU has become in service call SG FSM */
               switch(su->sg_of_su->su_redundancy_model)
               {
               case AVSV_SG_RED_MODL_2N:
                  if(avd_sg_2n_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                    
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }
                  break;

               case AVSV_SG_RED_MODL_NWAY:
                  if(avd_sg_nway_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                    
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }
                  break;
                  
               case AVSV_SG_RED_MODL_NPM:
                  if(avd_sg_npm_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                    
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }
                  break;

               case AVSV_SG_RED_MODL_NWAYACTV:
                  if(avd_sg_nacvred_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                    
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }
                  break;
               case AVSV_SG_RED_MODL_NORED:
               default:
                  if(avd_sg_nored_su_insvc_func(cb,su) == NCSCC_RC_FAILURE)
                  {
                     /* Bad situation. Free the message and return since
                      * receive id was not processed the event will again
                      * comeback which we can then process.
                      */
                     
                     /* log error about the failure */                                    
                     m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
                     avsv_dnd_msg_free(n2d_msg);
                     evt->info.avnd_msg = NULL;
                     return;
                  }
                  break;
               }  
               
            }
         } /* if(m_AVD_APP_SU_IS_INSVC(su)) */
         
      } /* else (su->sg_of_su->sg_ncs_spec == SA_TRUE) */


   } /* else if(n2d_msg->msg_info.n2d_opr_state.su_oper_state == NCS_OPER_STATE_ENABLE) */

   avsv_dnd_msg_free(n2d_msg);
   evt->info.avnd_msg = NULL;
   
   return;
   
}

/*****************************************************************************
 * Function: avd_ncs_su_mod_rsp
 *
 * Purpose:  This function is the handler for su si assign response 
 * from the node director for the NCS SU modify  su si assign message.    
 * it verify if all the other NCS SU have also got assigned. If assigned
 * it does the role specific functionality.
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

void avd_ncs_su_mod_rsp(AVD_CL_CB *cb,AVD_AVND *avnd, AVSV_N2D_INFO_SU_SI_ASSIGN_MSG_INFO *assign)
{
   AVD_SU *i_su = AVD_SU_NULL;
   AVD_AVND *avnd_other = AVD_AVND_NULL;
   SaBoolT ncs_done = SA_TRUE;
   uns32  rc = NCSCC_RC_SUCCESS;
  
    m_AVD_LOG_FUNC_ENTRY("avd_ncs_su_mod_rsp");

  /* Active -> Quiesced && resp = Success */ 
   if((cb->avail_state_avd == SA_AMF_HA_QUIESCED) && 
      (assign->ha_state == SA_AMF_HA_QUIESCED) &&
      (assign->error == NCSCC_RC_SUCCESS))
   {
      i_su = avnd->list_of_ncs_su;
      while(i_su != AVD_SU_NULL) 
      {
         if((i_su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
               (i_su->list_of_susi->fsm != AVD_SU_SI_STATE_ASGND))
         {
              ncs_done = SA_FALSE;
              break;
         }

         i_su = i_su->avnd_list_su_next;
      }

      if(ncs_done == SA_TRUE)
      {
         /* If other AvD is present and we are able to set mds role */
         if((cb->node_id_avd_other != 0) && 
            (NCSCC_RC_SUCCESS == avd_mds_set_vdest_role(cb, SA_AMF_HA_QUIESCED)))
           {
              /* We need to send the role to AvND. */
              rc = avd_avnd_send_role_change(cb, cb->node_id_avd,SA_AMF_HA_QUIESCED);
              if(NCSCC_RC_SUCCESS != rc)
              {
                m_AVD_PXY_PXD_ERR_LOG(
                "avd_role_switch_actv_qsd: role sent failed. Node Id and role are",
                NULL,cb->node_id_avd, cb->avail_state_avd,0,0);
              }
              else
              {
                 /* we should send the above data verify msg right now */
                 avd_d2n_msg_dequeue(cb);
              }

               return;
           }

         /*  We failed to switch, Send Active to all NCS Su's having 2N redun model &
             present in this node */
         avd_avm_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_QUIESCED);
         cb->avail_state_avd = SA_AMF_HA_ACTIVE;
         cb->role_switch = SA_FALSE;
         
         for(i_su = avnd->list_of_ncs_su; i_su != AVD_SU_NULL; i_su = i_su->avnd_list_su_next)
         {
            if((i_su->list_of_susi != 0) && 
               (i_su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
               (i_su->list_of_susi->state == SA_AMF_HA_QUIESCED))
            {
               m_AVD_SET_SG_FSM(cb,(i_su->sg_of_su),AVD_SG_FSM_SG_REALIGN); 
               avd_sg_su_si_mod_snd(cb, i_su, SA_AMF_HA_ACTIVE);
            }
         }
      } /* ncs_done == SA_TRUE */

      return;
   }


   /* Active -> Quiesed && resp = Failure */
   if((cb->avail_state_avd == SA_AMF_HA_QUIESCED) && 
      (assign->ha_state == SA_AMF_HA_QUIESCED) &&
      (assign->error == NCSCC_RC_FAILURE))
   {
      avd_avm_role_rsp(cb, NCSCC_RC_FAILURE, SA_AMF_HA_QUIESCED);
      cb->avail_state_avd = SA_AMF_HA_ACTIVE;
      cb->role_switch = SA_FALSE;

      for(i_su = avnd->list_of_ncs_su; i_su != AVD_SU_NULL; i_su = i_su->avnd_list_su_next)
      {
         if((i_su->list_of_susi != 0) && 
            (i_su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
            (i_su->list_of_susi->state == SA_AMF_HA_QUIESCED))
         {
            m_AVD_SET_SG_FSM(cb,(i_su->sg_of_su),AVD_SG_FSM_SG_REALIGN); 
            avd_sg_su_si_mod_snd(cb, i_su, SA_AMF_HA_ACTIVE);
         }
      } /* for */
      
      return;
   }

   /* Standby -> Active && Success */
   if((cb->avail_state_avd == SA_AMF_HA_ACTIVE) && 
      (assign->ha_state == SA_AMF_HA_ACTIVE) &&
      (assign->error == NCSCC_RC_SUCCESS))
   {
      i_su = avnd->list_of_ncs_su;
      while(i_su != AVD_SU_NULL) 
      {
         if((i_su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
               (i_su->list_of_susi->fsm != AVD_SU_SI_STATE_ASGND))
         {
            ncs_done = SA_FALSE;
            break;
         }

         i_su = i_su->avnd_list_su_next;
      }

      if(ncs_done == SA_TRUE)
      {
         avd_avm_role_rsp(cb, NCSCC_RC_SUCCESS, SA_AMF_HA_ACTIVE);
         cb->role_switch = SA_FALSE; /* almost done with switch */

         /* get the avnd on other SCXB from node_id of other AvD*/
         if(AVD_AVND_NULL == (avnd_other = avd_avnd_struc_find_nodeid(cb,
                                                    cb->node_id_avd_other)))
         {
            m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
            return ;
         }
         
         /* Now change All the NCS SU's of the other SCXB which should be
            in Quiesed state to Standby */
         
         for(i_su = avnd_other->list_of_ncs_su; i_su != AVD_SU_NULL; i_su = i_su->avnd_list_su_next)
         {
            if((i_su->list_of_susi != 0) &&
               (i_su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
               (i_su->list_of_susi->state == SA_AMF_HA_QUIESCED))
            {
               avd_sg_su_si_mod_snd(cb, i_su, SA_AMF_HA_STANDBY);
               avd_sg_su_oper_list_add(cb,i_su, FALSE);
               m_AVD_SET_SG_FSM(cb,(i_su->sg_of_su),AVD_SG_FSM_SG_REALIGN); 
            }
         }

               
         /* Switch the APP SU's who have are Active on other node and has 2N redund*/
         i_su = avnd_other->list_of_su;
         while(i_su != AVD_SU_NULL) 
         {
            if((i_su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
               (i_su->sg_of_su->sg_fsm_state == AVD_SG_FSM_STABLE) &&
               (i_su->list_of_susi != 0) &&
               (i_su->list_of_susi->state == SA_AMF_HA_ACTIVE)) 
            {
               m_AVD_SET_SU_SWITCH(cb,i_su,AVSV_SI_TOGGLE_SWITCH);

               if(avd_sg_2n_suswitch_func(cb, i_su) != NCSCC_RC_SUCCESS)
               {
                  m_AVD_SET_SU_SWITCH(cb,i_su,AVSV_SI_TOGGLE_STABLE);
               }
            }

            i_su = i_su->avnd_list_su_next;
         }
      }

      return ;
   }

  
   /* Standby -> Active && resp = Failure */
   /* We are expecting a su faiolver for this ncs su */ 

return;
}
/*****************************************************************************
 * Function: avd_su_si_assign_func
 *
 * Purpose:  This function is the handler for su si assign event
 * indicating the arrival of the response to the su si assign message from
 * node director. It then call the redundancy model specific routine to process
 * this event. 
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

void avd_su_si_assign_func(AVD_CL_CB *cb,AVD_EVT *evt)
{

   AVD_DND_MSG *n2d_msg;
   AVD_AVND *avnd, *su_node_ptr = NULL;
   AVD_SU *su;
   AVD_SU_SI_REL *susi;
   NCS_BOOL q_flag = FALSE,qsc_flag = FALSE;

   m_AVD_LOG_FUNC_ENTRY("avd_su_si_assign_func");
   
   if (evt->info.avnd_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   n2d_msg = evt->info.avnd_msg;
   
   m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);

   if ((avnd = 
   avd_msg_sanity_chk(cb,evt,n2d_msg->msg_info.n2d_su_si_assign.node_id,AVSV_N2D_INFO_SU_SI_ASSIGN_MSG))
   == AVD_AVND_NULL)
   {
      /* sanity failed return */
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   }


   if ((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
      (avnd->node_state == AVD_AVND_STATE_GO_DOWN) ||
      ((avnd->rcv_msg_id + 1) != n2d_msg->msg_info.n2d_su_si_assign.msg_id))
   {
      /* log information error that the node is in invalid state */
      m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_state);
      m_AVD_LOG_INVALID_VAL_ERROR(avnd->rcv_msg_id);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return;
   }

   /* update the receive id count */
   m_AVD_SET_AVND_RCV_ID(cb,avnd,(n2d_msg->msg_info.n2d_su_si_assign.msg_id));

   /* 
    * Send the Ack message to the node, indicationg that the message with this
    * message ID is received successfully.
    */
   if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS)
   {
      /* log error that the director is not able to send the message */
      m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);

   }


   if (n2d_msg->msg_info.n2d_su_si_assign.si_name_net.length == 0)
   {

      /* get the SU from the tree since this is across the
       * SU operation. 
       */

      if((su = avd_su_struc_find(cb,n2d_msg->msg_info.n2d_su_si_assign.su_name_net,FALSE))
         == AVD_SU_NULL)
      {
         /* log error that the SU mentioned in the message is not present */         
         m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(n2d_msg->msg_info.n2d_su_si_assign.su_name_net.value,n2d_msg->msg_info.n2d_su_si_assign.su_name_net.length);
         avsv_dnd_msg_free(n2d_msg);
         evt->info.avnd_msg = NULL;
         return;
      }

      if(su->list_of_susi == AVD_SU_SI_REL_NULL)
      {
         /* log Info error that the SU mentioned is not in proper state. */         
         m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
         m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);
         avsv_dnd_msg_free(n2d_msg);
         evt->info.avnd_msg = NULL;
         return;
      }
      
      switch (n2d_msg->msg_info.n2d_su_si_assign.msg_act)
      {
      case AVSV_SUSI_ACT_DEL:
         /* AvND can force a abrupt removal of assignments */
         su->si_curr_standby = 0;
         su->si_curr_active = 0;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_AVD_SU_CONFIG);
         break;

      case AVSV_SUSI_ACT_MOD:
         /* Verify that the SUSI is in the modify state for the same HA state. */
         susi = su->list_of_susi;
         while (susi != AVD_SU_SI_REL_NULL)
         {            
            if((susi->state != n2d_msg->msg_info.n2d_su_si_assign.ha_state)
               && (susi->state != SA_AMF_HA_QUIESCING)
               && (n2d_msg->msg_info.n2d_su_si_assign.ha_state != SA_AMF_HA_QUIESCED)
               && (susi->fsm != AVD_SU_SI_STATE_UNASGN))
            {
               /* some other event has caused further state change ignore
                * this message, by accepting the receive id and  droping the message.
                * message id has already been accepted above.
                */
               
               avsv_dnd_msg_free(n2d_msg);
               evt->info.avnd_msg = NULL;
               return;            
            }else if ((susi->state == SA_AMF_HA_QUIESCING)
               && (susi->fsm != AVD_SU_SI_STATE_UNASGN))
            {
               qsc_flag = TRUE;
            }
            
            susi = susi->su_next;

         } /* while (susi != AVD_SU_SI_REL_NULL) */
         if(n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            if (n2d_msg->msg_info.n2d_su_si_assign.ha_state == SA_AMF_HA_QUIESCING)
            {
               q_flag = TRUE;               
               avd_sg_su_asgn_del_util(cb, su, FALSE,FALSE);
            }else 
            {
               if (n2d_msg->msg_info.n2d_su_si_assign.ha_state == SA_AMF_HA_ACTIVE)
               {
                  if (su->si_curr_standby != 0)                     
                     su->si_curr_active = su->si_curr_standby;
                  su->si_curr_standby = 0;
               }else if (n2d_msg->msg_info.n2d_su_si_assign.ha_state == SA_AMF_HA_STANDBY)
               {
                  su->si_curr_standby = su->si_curr_active;
                  su->si_curr_active = 0;
               }

               m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_AVD_SU_CONFIG);

               /* set the  assigned or quiesced state in the SUSIs. */
               avd_sg_su_asgn_del_util(cb,su,FALSE,qsc_flag);
            }
         }
         break;
      default:
         /* log fatal error that the message is not proper. */
         m_AVD_LOG_INVALID_VAL_FATAL(n2d_msg->msg_info.n2d_su_si_assign.msg_act);
         avsv_dnd_msg_free(n2d_msg);
         evt->info.avnd_msg = NULL;
         return;
         break;
      } /* switch (n2d_msg->msg_info.n2d_su_si_assign.msg_act) */

      m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

      /* Are we in the middle of role switch */
      if((cb->role_switch == SA_TRUE) &&
         (su->sg_of_su->sg_ncs_spec == SA_TRUE) && 
         (n2d_msg->msg_info.n2d_su_si_assign.msg_act == AVSV_SUSI_ACT_MOD) &&
         (su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
         ((su_node_ptr->type == AVSV_AVND_CARD_SYS_CON) ||
          (cb->node_id_avd == su_node_ptr->node_info.nodeId)))
      {
         if(AVSV_AVND_CARD_SYS_CON  != su_node_ptr->type)
            m_AVD_LOG_INVALID_VAL_ERROR(((uns32)su_node_ptr->type));
       
         avd_ncs_su_mod_rsp(cb,avnd,&n2d_msg->msg_info.n2d_su_si_assign);

      }else if ((cb->avail_state_avd == SA_AMF_HA_QUIESCED) &&
         (su->sg_of_su->sg_ncs_spec == SA_TRUE) && 
         (su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_2N) &&
         (n2d_msg->msg_info.n2d_su_si_assign.msg_act == AVSV_SUSI_ACT_MOD) &&
         (n2d_msg->msg_info.n2d_su_si_assign.ha_state == SA_AMF_HA_QUIESCED) &&
         (su_node_ptr->type == AVSV_AVND_CARD_SYS_CON))
      {
         /*ignore this case expecting the other guy to shoot us down. */
      }else
      {
        
         /* Call the redundancy model specific procesing function. Dont call
          * in case of acknowledgment for quiescing.
          */
         switch(su->sg_of_su->su_redundancy_model)
         {
         case AVSV_SG_RED_MODL_2N:
            /* Now process the acknowledge message based on
             * Success or failure.
             */
            if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
            {
               if (q_flag == FALSE)
               {
                  avd_sg_2n_susi_sucss_func(cb,su,AVD_SU_SI_REL_NULL,
                  n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);      
               }
            }else
            {
               m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_ERROR,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
               avd_sg_2n_susi_fail_func(cb,su,AVD_SU_SI_REL_NULL,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
            }
            break;

         case AVSV_SG_RED_MODL_NWAY:
            /* Now process the acknowledge message based on
             * Success or failure.
             */
            if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
            {
               if (q_flag == FALSE)
               {
                  avd_sg_nway_susi_sucss_func(cb,su,AVD_SU_SI_REL_NULL,
                  n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);      
               }
            }else
            {
               m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_ERROR,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
               avd_sg_nway_susi_fail_func(cb,su,AVD_SU_SI_REL_NULL,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
            }
            break;

         case AVSV_SG_RED_MODL_NWAYACTV:
            /* Now process the acknowledge message based on
             * Success or failure.
             */
            if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
            {
               if (q_flag == FALSE)
               {
                  avd_sg_nacvred_susi_sucss_func(cb,su,AVD_SU_SI_REL_NULL,
                  n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);      
               }
            }else
            {
               m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_ERROR,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
               avd_sg_nacvred_susi_fail_func(cb,su,AVD_SU_SI_REL_NULL,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
            }
            break;
            
         case AVSV_SG_RED_MODL_NPM:
            /* Now process the acknowledge message based on
             * Success or failure.
             */
            if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
            {
               if (q_flag == FALSE)
               {
                  avd_sg_npm_susi_sucss_func(cb,su,AVD_SU_SI_REL_NULL,
                  n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);      
               }
            }else
            {
               m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_ERROR,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
               avd_sg_npm_susi_fail_func(cb,su,AVD_SU_SI_REL_NULL,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
            }
            break;

         case AVSV_SG_RED_MODL_NORED:
         default:
            /* Now process the acknowledge message based on
             * Success or failure.
             */
            if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
            {
               if (q_flag == FALSE)
               {
                  avd_sg_nored_susi_sucss_func(cb,su,AVD_SU_SI_REL_NULL,
                  n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);      
               }
            }else
            {
               m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_ERROR,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
               avd_sg_nored_susi_fail_func(cb,su,AVD_SU_SI_REL_NULL,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
            }
            break;
         }
      }

   }else /* if (n2d_msg->msg_info.n2d_su_si_assign.si_name.length == 0) */
   {
      /* Single SU SI assignment find the SU SI structure */

      if ((susi = 
         avd_susi_struc_find(cb,n2d_msg->msg_info.n2d_su_si_assign.su_name_net,
         n2d_msg->msg_info.n2d_su_si_assign.si_name_net,FALSE)) == AVD_SU_SI_REL_NULL)
      {
         /* Acknowledgement for a deleted SU SI ignore the message */
   
         /* log information error */
         m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(n2d_msg->msg_info.n2d_su_si_assign.su_name_net.value,n2d_msg->msg_info.n2d_su_si_assign.su_name_net.length);
         m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(n2d_msg->msg_info.n2d_su_si_assign.si_name_net.value,n2d_msg->msg_info.n2d_su_si_assign.si_name_net.length);
         avsv_dnd_msg_free(n2d_msg);
         evt->info.avnd_msg = NULL;
         return;
      }
      
      switch (n2d_msg->msg_info.n2d_su_si_assign.msg_act)
      {
      case AVSV_SUSI_ACT_DEL:
         /* AvND can force a abrupt removal of assignments */
         if (susi->state == SA_AMF_HA_STANDBY)
         {
            susi->su->si_curr_standby--;
            m_AVD_SI_DEC_STDBY_CURR_SU(susi->si);
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb,susi->su,AVSV_CKPT_SU_SI_CURR_STBY);
         }
         else
         {
            susi->su->si_curr_active--;
            m_AVD_SI_DEC_ACTV_CURR_SU(susi->si);
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb,susi->su,AVSV_CKPT_SU_SI_CURR_ACTIVE);
         }
         break;

      case AVSV_SUSI_ACT_ASGN:
         /* Verify that the SUSI is in the assign state for the same HA state. */
         if((susi->fsm != AVD_SU_SI_STATE_ASGN) ||
            (susi->state != n2d_msg->msg_info.n2d_su_si_assign.ha_state))
         {
            /* some other event has caused further state change ignore
             * this message, by accepting the receive id and
             * droping the message.
             */

            /* log Info error that the susi mentioned is not in proper state. */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->su->name_net.value,susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->si->name_net.value,susi->si->name_net.length);
            avsv_dnd_msg_free(n2d_msg);
            evt->info.avnd_msg = NULL;
            return;            
         }
         if(n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            susi->fsm = AVD_SU_SI_STATE_ASGND;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);

            /* generate the susi ha state change trap */
            avd_gen_su_si_assigned_trap(cb, susi);

            /* trigger pg upd */
            avd_pg_susi_chg_prc(cb, susi);
         }
         break;

      case AVSV_SUSI_ACT_MOD:
         /* Verify that the SUSI is in the modify state for the same HA state. */
         if((susi->fsm != AVD_SU_SI_STATE_MODIFY) ||
            ((susi->state != n2d_msg->msg_info.n2d_su_si_assign.ha_state)
            && (susi->state != SA_AMF_HA_QUIESCING)
         && (n2d_msg->msg_info.n2d_su_si_assign.ha_state != SA_AMF_HA_QUIESCED)))
         {
            /* some other event has caused further state change ignore
             * this message, by accepting the receive id and
             * droping the message.
             */

            /* log Info error that the susi mentioned is not in proper state. */
            m_AVD_LOG_INVALID_VAL_ERROR(((long)susi));
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->su->name_net.value,susi->su->name_net.length);
            m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(susi->si->name_net.value,susi->si->name_net.length);
            avsv_dnd_msg_free(n2d_msg);
            evt->info.avnd_msg = NULL;
            return;            
         }
         
         if(n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            if (n2d_msg->msg_info.n2d_su_si_assign.ha_state == SA_AMF_HA_QUIESCING)
            {
               q_flag = TRUE;
               susi->fsm =  AVD_SU_SI_STATE_ASGND;
               m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
            }else 
            {
               if (susi->state == SA_AMF_HA_QUIESCING)
               {
                  susi->state = SA_AMF_HA_QUIESCED;
                  m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
                  avd_gen_su_ha_state_changed_trap(cb, susi);
               }else
               {
                  if (n2d_msg->msg_info.n2d_su_si_assign.ha_state == SA_AMF_HA_ACTIVE)
                  {
                     if (susi->su->si_curr_standby != 0)
                     {
                        susi->su->si_curr_active ++;
                        susi->su->si_curr_standby --;
                        m_AVD_SI_INC_ACTV_CURR_SU(susi->si);
                        m_AVD_SI_DEC_STDBY_CURR_SU(susi->si);
                        m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi->su, AVSV_CKPT_AVD_SU_CONFIG);
                     }                        
                        
                  }else if (n2d_msg->msg_info.n2d_su_si_assign.ha_state == SA_AMF_HA_STANDBY)
                  {
                     susi->su->si_curr_standby ++;
                     susi->su->si_curr_active --;
                     m_AVD_SI_INC_STDBY_CURR_SU(susi->si);
                     m_AVD_SI_DEC_ACTV_CURR_SU(susi->si);
                     m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi->su, AVSV_CKPT_AVD_SU_CONFIG);
                  }
               }
               
               /* set the assigned in the SUSIs. */
               susi->fsm = AVD_SU_SI_STATE_ASGND;
               m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);
            }

            /* generate the susi ha state change trap */
            avd_gen_su_si_assigned_trap(cb, susi);

            /* trigger pg upd */
            avd_pg_susi_chg_prc(cb, susi);
         }         
         break;

      default:
         /* log fatal error that the message is not proper. */
         m_AVD_LOG_INVALID_VAL_FATAL(n2d_msg->msg_info.n2d_su_si_assign.msg_act);
         m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_ERROR,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);
         avsv_dnd_msg_free(n2d_msg);
         evt->info.avnd_msg = NULL;
         return;
         break;
      } /* switch (n2d_msg->msg_info.n2d_su_si_assign.msg_act) */

      /* Call the redundancy model specific procesing function Dont call
       * in case of acknowledgment for quiescing.
       */

      switch(susi->si->sg_of_si->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
         /* Now process the acknowledge message based on
          * Success or failure.
          */
         if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            if (q_flag == FALSE)
            {
               avd_sg_2n_susi_sucss_func(cb,susi->su,susi,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
               if ((n2d_msg->msg_info.n2d_su_si_assign.msg_act == AVSV_SUSI_ACT_ASGN) &&
                  (susi->su->sg_of_su->sg_ncs_spec == SA_TRUE))
               {
                  /* Since a NCS SU has been assigned trigger the node FSM.*/
                  /* For (ncs_spec == SA_TRUE), su will not be external, so su
                     will have node attached.*/
                  avd_nd_ncs_su_assigned(cb,susi->su->su_on_node);
               }           
      
            }
         }else
         {
            avd_sg_2n_susi_fail_func(cb,susi->su,susi,
            n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
         }
         break;

      case AVSV_SG_RED_MODL_NWAY:
         /* Now process the acknowledge message based on
          * Success or failure.
          */
         if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            if (q_flag == FALSE)
            {
               avd_sg_nway_susi_sucss_func(cb,susi->su,susi,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
               if ((n2d_msg->msg_info.n2d_su_si_assign.msg_act == AVSV_SUSI_ACT_ASGN) &&
                  (susi->su->sg_of_su->sg_ncs_spec == SA_TRUE))
               {
                  /* Since a NCS SU has been assigned trigger the node FSM.*/
                  /* For (ncs_spec == SA_TRUE), su will not be external, so su
                     will have node attached.*/
                  avd_nd_ncs_su_assigned(cb,susi->su->su_on_node);
               }           
            }
         }else
         {
            avd_sg_nway_susi_fail_func(cb,susi->su,susi,
            n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
         }
         break;

      case AVSV_SG_RED_MODL_NWAYACTV:
         /* Now process the acknowledge message based on
          * Success or failure.
          */
         if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            if (q_flag == FALSE)
            {
               avd_sg_nacvred_susi_sucss_func(cb,susi->su,susi,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);      
            }
         }else
         {
            avd_sg_nacvred_susi_fail_func(cb,susi->su,susi,
            n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
         }
         break;
         
      case AVSV_SG_RED_MODL_NPM:
         /* Now process the acknowledge message based on
          * Success or failure.
          */
         if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            if (q_flag == FALSE)
            {
               avd_sg_npm_susi_sucss_func(cb,susi->su,susi,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);      
            }
         }else
         {
            avd_sg_npm_susi_fail_func(cb,susi->su,susi,
            n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
         }
         break;
         
      case AVSV_SG_RED_MODL_NORED:
      default:
         /* Now process the acknowledge message based on
          * Success or failure.
          */
         if (n2d_msg->msg_info.n2d_su_si_assign.error == NCSCC_RC_SUCCESS)
         {
            if (q_flag == FALSE)
            {
               avd_sg_nored_susi_sucss_func(cb,susi->su,susi,
               n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
               if ((n2d_msg->msg_info.n2d_su_si_assign.msg_act == AVSV_SUSI_ACT_ASGN) &&
                  (susi->su->sg_of_su->sg_ncs_spec == SA_TRUE))
               {
                  /* Since a NCS SU has been assigned trigger the node FSM.*/
                  /* For (ncs_spec == SA_TRUE), su will not be external, so su
                     will have node attached.*/
                  avd_nd_ncs_su_assigned(cb,susi->su->su_on_node);
               }           
      
            }
         }else
         {
            avd_sg_nored_susi_fail_func(cb,susi->su,susi,
            n2d_msg->msg_info.n2d_su_si_assign.msg_act,n2d_msg->msg_info.n2d_su_si_assign.ha_state);
         }
         
         break;
         
      } /* switch(susi->si->sg_of_si->su_redundancy_model) */
      
   } /* else (n2d_msg->msg_info.n2d_su_si_assign.si_name.length == 0) */

   /* Free the messages */
   avsv_dnd_msg_free(n2d_msg);
   evt->info.avnd_msg = NULL;
   
   return;

}


/*****************************************************************************
 * Function: avd_sg_app_node_su_inst_func
 *
 * Purpose:  This function processes the request to instantiate all the
 *           application SUs on a node. If the
 *           AvD is in AVD_INIT_DONE state i.e the AMF timer hasnt expired
 *           all the pre-instantiable SUs will be instantiated by sending presence
 *           message. The non pre-instantiable SUs operation state will be made
 *           enable. If the state is AVD_APP_STATE, then for each SU the
 *           corresponding SG will be evaluated for instantiations.
 *
 * Input: cb - the AVD control block
 *        avnd - The pointer to the node whose application SUs need to
 *               be instantiated.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_app_node_su_inst_func(AVD_CL_CB *cb, AVD_AVND *avnd)
{
   AVD_SU *i_su;
   AVD_AVND *su_node_ptr = NULL;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_app_node_su_inst_func");
   
   if (cb->init_state == AVD_INIT_DONE)
   {
      i_su = avnd->list_of_su;
      while(i_su != AVD_SU_NULL)
      {

         if ((i_su->num_of_comp == i_su->curr_num_comp) &&
            (i_su->term_state == FALSE) &&
            (i_su->pres_state == NCS_PRES_UNINSTANTIATED))
         { 
            if(i_su->su_preinstan == TRUE)
            {
               /* instantiate all the pre-instatiable SUs */
               avd_snd_presence_msg(cb,i_su,FALSE);
            }else
            {
               /* mark the non preinstatiable as enable. */
               m_AVD_SET_SU_OPER(cb,i_su,NCS_OPER_STATE_ENABLE);

               m_AVD_GET_SU_NODE_PTR(cb,i_su,su_node_ptr);
         
               if(m_AVD_APP_SU_IS_INSVC(i_su,su_node_ptr))
               {
                  m_AVD_SET_SU_REDINESS(cb,i_su,NCS_IN_SERVICE);
               }
            }
         } 
         
         i_su = i_su->avnd_list_su_next;
         
      } /* while(i_su != AVD_SU_NULL) */

   }/* if (cb->init_state == AVD_INIT_DONE) */
   else if (cb->init_state == AVD_APP_STATE)
   {
      i_su = avnd->list_of_su;
      while(i_su != AVD_SU_NULL)
      {

         if ((i_su->num_of_comp == i_su->curr_num_comp) &&
            (i_su->term_state == FALSE) &&
            (i_su->pres_state == NCS_PRES_UNINSTANTIATED))
         { 
            /* Look at the SG and do the instantiations. */
            avd_sg_app_su_inst_func(cb,i_su->sg_of_su);
         } 
         
         i_su = i_su->avnd_list_su_next;
         
      } /* while(i_su != AVD_SU_NULL) */
      
   } /* if (cb->init_state == AVD_APP_STATE) */
   
   return NCSCC_RC_SUCCESS;
    
}

/*****************************************************************************
 * Function: avd_sg_app_su_inst_func
 *
 * Purpose:  This function processes the request to evaluate the SG for
 *           Instantiations and terminations of SUs in the SG. This routine
 *           is used only when AvD is in AVD_APP_STATE state i.e the AMF 
 *           timer has expired. It Instantiates all the high ranked 
 *           pre-instantiable SUs and for all non pre-instanble SUs if
 *           they are in uninstantiated state, the operation state will
 *           be made active. Once the preffered inservice SU count is
 *           meet all the instatiated,unassigned pre-instantiated SUs will
 *           be terminated. 
 *
 * Input: cb - the AVD control block
 *        sg - The pointer to SG whose SUs need to be instantiated.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: Only if the AvD is in AVD_APP_STATE this routine should be used.
 *
 * 
 **************************************************************************/

uns32 avd_sg_app_su_inst_func(AVD_CL_CB *cb, AVD_SG *sg)
{
   uns32 num_insvc_su=0;
   uns32 num_asgd_su=0;
   uns32 num_su=0;
   uns32 num_try_insvc_su=0;
   AVD_SU *i_su;
   AVD_AVND *su_node_ptr = NULL;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_app_su_inst_func");
   
   i_su = sg->list_of_su;
   while (i_su != AVD_SU_NULL)
   {
      m_AVD_GET_SU_NODE_PTR(cb,i_su,su_node_ptr);
      num_su ++;
      /* Check if the SU is inservice */
      if(i_su->readiness_state == NCS_IN_SERVICE)
      {
         num_insvc_su ++;
         if ((i_su->list_of_susi == AVD_SU_SI_REL_NULL) &&
            (i_su->su_preinstan == TRUE) &&
            (sg->pref_num_insvc_su < (num_insvc_su + num_try_insvc_su)))
         {
            /* enough inservice SUs are already there terminate this
             * SU.
             */
            if(avd_snd_presence_msg(cb,i_su,TRUE) == NCSCC_RC_SUCCESS)
            {
               /* mark the SU operation state as disable and readiness state
                * as out of service.
                */
               m_AVD_SET_SU_OPER(cb,i_su,NCS_OPER_STATE_DISABLE);
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);
               num_insvc_su --;
            }
         }else if (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
         {
            num_asgd_su ++;
         }
      } /* if(i_su->readiness_state == NCS_IN_SERVICE) */ 
      else if (i_su->num_of_comp == i_su->curr_num_comp)
      {
         /* if the SU is non preinstantiable and disable and if the node
          * operational state is enable make the operation state enable.
          */
         if((i_su->su_preinstan == FALSE) && 
            (i_su->oper_state == NCS_OPER_STATE_DISABLE) &&
            (i_su->pres_state == NCS_PRES_UNINSTANTIATED) &&
            (su_node_ptr->oper_state == NCS_OPER_STATE_ENABLE) &&
            (i_su->term_state == FALSE))
         {
            m_AVD_SET_SU_OPER(cb,i_su,NCS_OPER_STATE_ENABLE);
            m_AVD_GET_SU_NODE_PTR(cb,i_su,su_node_ptr);
         
            if(m_AVD_APP_SU_IS_INSVC(i_su,su_node_ptr))
            {
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_IN_SERVICE);
               switch(i_su->sg_of_su->su_redundancy_model)
               {
               case AVSV_SG_RED_MODL_2N:                   
                  avd_sg_2n_su_insvc_func(cb,i_su);          
                  break;

               case AVSV_SG_RED_MODL_NWAY:                   
                  avd_sg_nway_su_insvc_func(cb,i_su);          
                  break;

               case AVSV_SG_RED_MODL_NWAYACTV:                   
                  avd_sg_nacvred_su_insvc_func(cb,i_su);          
                  break;
               
               case AVSV_SG_RED_MODL_NPM:                   
                  avd_sg_npm_su_insvc_func(cb,i_su);          
                  break;

               case AVSV_SG_RED_MODL_NORED:
               default:                   
                  avd_sg_nored_su_insvc_func(cb,i_su);  
                  break;
               } 
               if (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
               {
                  num_asgd_su ++;
               }
               num_insvc_su ++;
            }
            
         }else if((i_su->su_preinstan == TRUE) &&
                  (sg->pref_num_insvc_su > (num_insvc_su + num_try_insvc_su)) &&
                  (i_su->pres_state == NCS_PRES_UNINSTANTIATED) &&
                  (su_node_ptr->oper_state == NCS_OPER_STATE_ENABLE) &&
                  (i_su->term_state == FALSE))
         {
            /* Try to Instantiate this SU*/
            if(avd_snd_presence_msg(cb,i_su,FALSE) == NCSCC_RC_SUCCESS)
            {
               num_try_insvc_su ++;
            }
         }
      }   /* else if (i_su->num_of_comp == i_su->curr_num_comp) */

      i_su = i_su->sg_list_su_next;
      
   } /* while (i_su != AVD_SU_NULL) */
   
   /* The entire SG has been scanned for reinstatiations and terminations.
    * Fill the numbers gathered into the SG.
    */
   sg->su_assigned_num = num_asgd_su;
   m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_SU_ASSIGNED_NUM);

   sg->su_spare_num = num_insvc_su - num_asgd_su;
   m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_SU_SPARE_NUM);
   
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_sg_app_node_admin_func
 *
 * Purpose:  This function processes the request to do UNLOCK or LOCK or shutdown
 * of the AMF application SUs on the node. It first verifies that the
 * SGs belonging to the node are all stable and then it sets the readiness 
 * state of each of the SU on the node and calls the SG FSM for each of the
 * SUs.
 *
 * Input: cb - the AVD control block
 *        avnd - The pointer to the node whose application SUs need to
 *               be administratively modified.
 *        new_admin_state - The adminstate to which the node should change.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_app_node_admin_func(AVD_CL_CB *cb, AVD_AVND *avnd, 
                                 NCS_ADMIN_STATE new_admin_state)
{
   AVD_SU *i_su, *i_su_sg;
   NCS_BOOL su_admin=FALSE;
   AVD_SU_SI_REL *curr_susi;
   AVD_AVND *i_su_node_ptr = NULL;
   AVD_AVND *i_su_sg_node_ptr = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_sg_app_node_admin_func");
   
   /* If the node is not yet operational just modify the admin state field
    * incase of shutdown to lock and return success as this will only cause
    * state filed change and no semantics need to be followed.
    */
   if(avnd->oper_state == NCS_OPER_STATE_DISABLE)
   {
      if (new_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         m_AVD_SET_AVND_SU_ADMIN(cb,avnd,NCS_ADMIN_STATE_LOCK);
      }
      else
      {
         m_AVD_SET_AVND_SU_ADMIN(cb,avnd,new_admin_state);
      }
      
      return NCSCC_RC_SUCCESS;
   }
   
   /* Based on the admin operation that is been done call the corresponding.
    * Redundancy model specific functionality for each of the SUs on 
    * the node.
    */

   switch(new_admin_state)
   {
   case NCS_ADMIN_STATE_UNLOCK:
      
      i_su = avnd->list_of_su;
      while(i_su != AVD_SU_NULL)
      {
         /* if SG to which this SU belongs and has SI assignments is undergoing 
          * su semantics return error.
          */
         if((i_su->list_of_susi != AVD_SU_SI_REL_NULL) &&
            (i_su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE))
         {
            
            /* Dont go ahead as a SG that is undergoing transition is
             * there related to this node.
             */
            avd_log(NCSFL_SEV_ERROR, "invalid sg state %u for unlock",
                i_su->sg_of_su->sg_fsm_state);
            return NCSCC_RC_FAILURE;
         }
         
         /* get the next SU on the node */
         i_su = i_su->avnd_list_su_next;
      } /* while(i_su != AVD_SU_NULL) */
      
      /* For each of the SUs calculate the readiness state. This routine is called
       * only when AvD is in AVD_APP_STATE. call the SG FSM with the new readiness
       * state.
       */
      
      m_AVD_SET_AVND_SU_ADMIN(cb,avnd,new_admin_state);
      
      i_su = avnd->list_of_su;
      while(i_su != AVD_SU_NULL)
      {
         m_AVD_GET_SU_NODE_PTR(cb,i_su,i_su_node_ptr);
         
         if(m_AVD_APP_SU_IS_INSVC(i_su,i_su_node_ptr))
         {
            m_AVD_SET_SU_REDINESS(cb,i_su,NCS_IN_SERVICE);
            switch(i_su->sg_of_su->su_redundancy_model)
            {
            case AVSV_SG_RED_MODL_2N:                
               avd_sg_2n_su_insvc_func(cb,i_su);
               break;
               
            case AVSV_SG_RED_MODL_NWAY:                
               avd_sg_nway_su_insvc_func(cb,i_su);
               break;
            
            case AVSV_SG_RED_MODL_NPM:                
               avd_sg_npm_su_insvc_func(cb,i_su);
               break;
               
            case AVSV_SG_RED_MODL_NWAYACTV:                
               avd_sg_nacvred_su_insvc_func(cb,i_su);
               break;

            case AVSV_SG_RED_MODL_NORED:
            default:
               avd_sg_nored_su_insvc_func(cb,i_su);     
               break;
            } 
            
            /* Since an SU has come in-service re look at the SG to see if other
             * instantiations or terminations need to be done.
             */
            avd_sg_app_su_inst_func(cb, i_su->sg_of_su);            
         }
         /* get the next SU on the node */
         i_su = i_su->avnd_list_su_next;
      }
      break; /* case NCS_ADMIN_STATE_UNLOCK: */
   case NCS_ADMIN_STATE_LOCK:
   case NCS_ADMIN_STATE_SHUTDOWN:
      
      i_su = avnd->list_of_su;
      while(i_su != AVD_SU_NULL)
      { 
         if(i_su->list_of_susi != AVD_SU_SI_REL_NULL)
         {
            /* verify that two assigned SUs belonging to the same SG are not
             * on this node 
             */
            i_su_sg = i_su->sg_of_su->list_of_su;
            while (i_su_sg != AVD_SU_NULL)
            {
              m_AVD_GET_SU_NODE_PTR(cb,i_su,i_su_node_ptr);
              m_AVD_GET_SU_NODE_PTR(cb,i_su_sg,i_su_sg_node_ptr);

              if ((i_su != i_su_sg) &&
                 (i_su_node_ptr == i_su_sg_node_ptr) &&
                 (i_su_sg->list_of_susi != AVD_SU_SI_REL_NULL))
              {
                 avd_log(NCSFL_SEV_ERROR, "two SUs on same node");
                 return NCSCC_RC_FAILURE;
              }
                  
              i_su_sg = i_su_sg->sg_list_su_next;
            
            }/* while (i_su_sg != AVD_SU_NULL) */

            /* if SG to which this SU belongs and has SI assignments is undergoing 
             * any semantics other than for this SU return error.
             */
            if (i_su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE)
            {
               if((i_su->sg_of_su->sg_fsm_state != AVD_SG_FSM_SU_OPER) ||
                  (avnd->su_admin_state != NCS_ADMIN_STATE_SHUTDOWN) || 
                  (new_admin_state != NCS_ADMIN_STATE_LOCK))
               {
                  avd_log(NCSFL_SEV_ERROR, "invalid sg state %u for lock/shutdown",
                      i_su->sg_of_su->sg_fsm_state);
                  return NCSCC_RC_FAILURE;
               }
            }/*if (i_su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE)*/
            
            if (i_su->list_of_susi->state == SA_AMF_HA_ACTIVE)
            {   
               su_admin = TRUE;
            }else if ((avnd->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN) &&
                   (su_admin == FALSE) && 
                   (i_su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_NWAY))
            {
               for (curr_susi = i_su->list_of_susi;
                 (curr_susi) && ((SA_AMF_HA_ACTIVE != curr_susi->state) ||
                 ((AVD_SU_SI_STATE_UNASGN == curr_susi->fsm)));
                 curr_susi = curr_susi->su_next);
               if (curr_susi) su_admin = TRUE;
            }               
            
         } /* if(i_su->list_of_susi != AVD_SU_SI_REL_NULL) */
         
         /* get the next SU on the node */
         i_su = i_su->avnd_list_su_next;
      } /* while(i_su != AVD_SU_NULL) */
      
      m_AVD_SET_AVND_SU_ADMIN(cb,avnd,new_admin_state);
      
      /* Now call the SG FSM for each of the SUs that have SI assignment. */
      i_su = avnd->list_of_su;
      while(i_su != AVD_SU_NULL)
      {
         m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);
         if(i_su->list_of_susi != AVD_SU_SI_REL_NULL)
         {
            switch(i_su->sg_of_su->su_redundancy_model)
            {
            case AVSV_SG_RED_MODL_2N:
               avd_sg_2n_su_admin_fail(cb,i_su,avnd);
               break;

            case AVSV_SG_RED_MODL_NWAY:
               avd_sg_nway_su_admin_fail(cb,i_su,avnd);
               break;
            
            case AVSV_SG_RED_MODL_NPM:
               avd_sg_npm_su_admin_fail(cb,i_su,avnd);
               break;

            case AVSV_SG_RED_MODL_NWAYACTV:
               avd_sg_nacvred_su_admin_fail(cb,i_su,avnd);
               break;

            case AVSV_SG_RED_MODL_NORED:
            default:
               avd_sg_nored_su_admin_fail(cb,i_su,avnd);
               break;
            }
         }
                     
         /* since an SU is now OOS we need to take a relook at the SG. */
         avd_sg_app_su_inst_func(cb, i_su->sg_of_su);
         
         /* get the next SU on the node */
         i_su = i_su->avnd_list_su_next;
      }
      
      if ((avnd->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN) && (su_admin == FALSE))
      {
         m_AVD_SET_AVND_SU_ADMIN(cb,avnd,NCS_ADMIN_STATE_LOCK);
      }
      
      break; /* case NCS_ADMIN_STATE_LOCK: case NCS_ADMIN_STATE_SHUTDOWN: */
   default:
      avd_log(NCSFL_SEV_ERROR, "fatal error");
      return NCSCC_RC_FAILURE;
      break;
   }    

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_sg_app_sg_admin_func
 *
 * Purpose:  This function processes the request to do UNLOCK or LOCK or shutdown
 * of the AMF application SUs on the SG. It first verifies that the
 * SGs is stable and then it sets the readiness 
 * state of each of the SU on the node and calls the SG FSM for the
 * SG.
 *
 * Input: cb - the AVD control block
 *        sg - The pointer to the sg which needs to be administratively modified.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_app_sg_admin_func(AVD_CL_CB *cb, AVD_SG *sg)
{
   AVD_SU *i_su;
   AVD_AVND *i_su_node_ptr = NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_sg_app_sg_admin_func");
   
   /* Based on the admin operation that is been done call the corresponding.
    * Redundancy model specific functionality for the SG.
    */

   switch(sg->admin_state)
   {
   case NCS_ADMIN_STATE_UNLOCK:
      /* Dont allow UNLOCK if the SG FSM is not stable. */
      if (sg->sg_fsm_state != AVD_SG_FSM_STABLE)
         return NCSCC_RC_FAILURE;
      /* For each of the SUs calculate the readiness state. This routine is called
       * only when AvD is in AVD_APP_STATE. call the SG FSM with the new readiness
       * state.
       */
   
      i_su = sg->list_of_su;
      while(i_su != AVD_SU_NULL)
      {
         m_AVD_GET_SU_NODE_PTR(cb,i_su,i_su_node_ptr);
         
         if(m_AVD_APP_SU_IS_INSVC(i_su,i_su_node_ptr))
         {
            m_AVD_SET_SU_REDINESS(cb,i_su,NCS_IN_SERVICE);           
         }
         /* get the next SU on the node */
         i_su = i_su->sg_list_su_next;
      }
      
      switch(sg->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
         if (avd_sg_2n_realign_func(cb,sg) == NCSCC_RC_FAILURE)
         {
            /* set all the SUs to OOS return failure */
            i_su = sg->list_of_su;
            while(i_su != AVD_SU_NULL)
            {
               
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);  
               /* get the next SU of the SG */
               i_su = i_su->sg_list_su_next;
            }
            
            return NCSCC_RC_FAILURE;
         } /* if (avd_sg_2n_realign_func(cb,sg) == NCSCC_RC_FAILURE) */
         break;

      case AVSV_SG_RED_MODL_NWAY:
         if (avd_sg_nway_realign_func(cb,sg) == NCSCC_RC_FAILURE)
         {
            /* set all the SUs to OOS return failure */
            i_su = sg->list_of_su;
            while(i_su != AVD_SU_NULL)
            {
               
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);  
               /* get the next SU of the SG */
               i_su = i_su->sg_list_su_next;
            }
            
            return NCSCC_RC_FAILURE;
         } /* if (avd_sg_nway_realign_func(cb,sg) == NCSCC_RC_FAILURE) */
         break;
         
      case AVSV_SG_RED_MODL_NPM:
         if (avd_sg_npm_realign_func(cb,sg) == NCSCC_RC_FAILURE)
         {
            /* set all the SUs to OOS return failure */
            i_su = sg->list_of_su;
            while(i_su != AVD_SU_NULL)
            {
               
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);  
               /* get the next SU of the SG */
               i_su = i_su->sg_list_su_next;
            }
            
            return NCSCC_RC_FAILURE;
         } /* if (avd_sg_nway_realign_func(cb,sg) == NCSCC_RC_FAILURE) */
         break;
         
      case AVSV_SG_RED_MODL_NWAYACTV:
         if (avd_sg_nacvred_realign_func(cb,sg) == NCSCC_RC_FAILURE)
         {
            /* set all the SUs to OOS return failure */
            i_su = sg->list_of_su;
            while(i_su != AVD_SU_NULL)
            {
               
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);  
               /* get the next SU of the SG */
               i_su = i_su->sg_list_su_next;
            }
            
            return NCSCC_RC_FAILURE;
         } /* if (avd_sg_nacvred_realign_func(cb,sg) == NCSCC_RC_FAILURE) */
         break;
      case AVSV_SG_RED_MODL_NORED:
      default:
         if (avd_sg_nored_realign_func(cb,sg) == NCSCC_RC_FAILURE)
         {
            /* set all the SUs to OOS return failure */
            i_su = sg->list_of_su;
            while(i_su != AVD_SU_NULL)
            {
               
               m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);  
               /* get the next SU of the SG */
               i_su = i_su->sg_list_su_next;
            }
            
            return NCSCC_RC_FAILURE;
         } /* if (avd_sg_nored_realign_func(cb,sg) == NCSCC_RC_FAILURE) */
         break;
      }      
      
      break; /* case NCS_ADMIN_STATE_UNLOCK: */
   case NCS_ADMIN_STATE_LOCK:
   case NCS_ADMIN_STATE_SHUTDOWN:
      
      if ((sg->sg_fsm_state != AVD_SG_FSM_STABLE) &&
         (sg->sg_fsm_state != AVD_SG_FSM_SG_ADMIN))
         return NCSCC_RC_FAILURE;
  
      switch(sg->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
         if (avd_sg_2n_sg_admin_down(cb,sg) == NCSCC_RC_FAILURE)
         {         
            return NCSCC_RC_FAILURE;
         }
         break;

      case AVSV_SG_RED_MODL_NWAY:
         if (avd_sg_nway_sg_admin_down(cb,sg) == NCSCC_RC_FAILURE)
         {         
            return NCSCC_RC_FAILURE;
         }
         break;
         
      case AVSV_SG_RED_MODL_NPM:
         if (avd_sg_npm_sg_admin_down(cb,sg) == NCSCC_RC_FAILURE)
         {         
            return NCSCC_RC_FAILURE;
         }
         break;

      case AVSV_SG_RED_MODL_NWAYACTV:
         if (avd_sg_nacvred_sg_admin_down(cb,sg) == NCSCC_RC_FAILURE)
         {         
            return NCSCC_RC_FAILURE;
         }
         break;

      case AVSV_SG_RED_MODL_NORED:
      default:
         if (avd_sg_nored_sg_admin_down(cb,sg) == NCSCC_RC_FAILURE)
         {         
            return NCSCC_RC_FAILURE;
         }
         break;
      }            
      
      i_su = sg->list_of_su;
 
      while(i_su != AVD_SU_NULL)
      {
         m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);
         /* get the next SU of the SG */
         i_su = i_su->sg_list_su_next;
      }
      break; /* case NCS_ADMIN_STATE_LOCK: case NCS_ADMIN_STATE_SHUTDOWN: */
   default:
      /* log fatal error */
      return NCSCC_RC_FAILURE;
      break;
   }    

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_node_susi_fail_func
 *
 * Purpose:  This function is called to un assign all the SUSIs on
 * the node after the node is found to be down. This function Makes all 
 * the SUs on the node as O.O.S and failover all the SUSI assignments based 
 * on their service groups. It will then delete all the SUSI assignments 
 * corresponding to the SUs on this node if any left.
 *
 * Input: cb - the AVD control block
 *        avnd - The AVND pointer of the node whose SU SI assignments need
 *                to be deleted.
 *
 * Returns: None.
 *
 * NOTES: none.
 * 
 **************************************************************************/

void avd_node_susi_fail_func(AVD_CL_CB *cb,AVD_AVND *avnd)
{
   AVD_SU *i_su;
   AVD_COMP *i_comp;


   m_AVD_LOG_FUNC_ENTRY("avd_node_susi_fail_func");
   
   /* run through all the NCS SUs, make all of them O.O.S. Set
    * assignments for the NCS SGs of which the SUs are members. Also
    * Set the operation state and presence state for the SUs and components to
    * disable and uninstantiated.  All the functionality for NCS SUs is done in
    * one loop as more than one NCS SU per SG in one node is not supported.
    */

   i_su = avnd->list_of_ncs_su;
   while (i_su != AVD_SU_NULL)
   {
      m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);
      /* Set the operation and presence state for the SU and all its
       * components.
       */
      m_AVD_SET_SU_OPER(cb,i_su,NCS_OPER_STATE_DISABLE);
      i_su->pres_state = NCS_PRES_UNINSTANTIATED;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_su, AVSV_CKPT_SU_PRES_STATE);
      i_comp = i_su->list_of_comp;
      while(i_comp != AVD_COMP_NULL)
      {
         i_comp->curr_num_csi_actv = 0;
         i_comp->curr_num_csi_stdby = 0;
         i_comp->oper_state = NCS_OPER_STATE_DISABLE;
         i_comp->pres_state = NCS_PRES_UNINSTANTIATED;
         i_comp->restart_cnt = 0;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_comp, AVSV_CKPT_AVD_COMP_CONFIG);
         i_comp = i_comp->su_comp_next;
      }
      
      switch(i_su->sg_of_su->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
         /* Now analyze the service group for the new HA state
          * assignments and send the SU SI assign messages
          * accordingly.
          */
         avd_sg_2n_node_fail_func(cb,i_su); 
         break;

      case AVSV_SG_RED_MODL_NWAY:
         /* Now analyze the service group for the new HA state
          * assignments and send the SU SI assign messages
          * accordingly.
          */
         avd_sg_nway_node_fail_func(cb,i_su); 
         break;
      
      case AVSV_SG_RED_MODL_NPM:
         /* Now analyze the service group for the new HA state
          * assignments and send the SU SI assign messages
          * accordingly.
          */
         avd_sg_npm_node_fail_func(cb,i_su); 
         break;

      case AVSV_SG_RED_MODL_NWAYACTV:
         /* Now analyze the service group for the new HA state
          * assignments and send the SU SI assign messages
          * accordingly.
          */
         avd_sg_nacvred_node_fail_func(cb,i_su); 
         break;
      case AVSV_SG_RED_MODL_NORED:
      default:
         /* Now analyze the service group for the new HA state
          * assignments and send the SU SI assign messages
          * accordingly.
          */
         avd_sg_nored_node_fail_func(cb,i_su);        
         break;
      }
      
      /* Free all the SU SI assignments for all the SIs on the
       * the SU if there are any.
       */

      while (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
      {

         /* free all the CSI assignments  */
          avd_compcsi_list_del(cb,i_su->list_of_susi,FALSE);
          /* Unassign the SUSI */
          m_AVD_SU_SI_TRG_DEL(cb,i_su->list_of_susi);             
      }

      i_su->si_curr_active = 0;
      i_su->si_curr_standby = 0;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_su, AVSV_CKPT_AVD_SU_CONFIG);
      
      i_su = i_su->avnd_list_su_next;

   } /* while (i_su != AVD_SU_NULL) */
   
   
   /* Run through the list of application SUs make all of them O.O.S. 
    */
   i_su = avnd->list_of_su;
   while (i_su != AVD_SU_NULL)
   {
      m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);
      /* Set the operation and presence state for the SU and all its
       * components.
       */
      m_AVD_SET_SU_OPER(cb,i_su,NCS_OPER_STATE_DISABLE);
      i_su->pres_state = NCS_PRES_UNINSTANTIATED;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_su, AVSV_CKPT_SU_PRES_STATE);
      i_comp = i_su->list_of_comp;
      while(i_comp != AVD_COMP_NULL)
      {
         i_comp->curr_num_csi_actv = 0;
         i_comp->curr_num_csi_stdby = 0;
         i_comp->oper_state = NCS_OPER_STATE_DISABLE;
         i_comp->pres_state = NCS_PRES_UNINSTANTIATED;
         i_comp->restart_cnt = 0;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_comp, AVSV_CKPT_AVD_COMP_CONFIG);
         i_comp = i_comp->su_comp_next;
      }

      i_su = i_su->avnd_list_su_next;
   } /* while (i_su != AVD_SU_NULL) */
   
   /* If the AvD is in AVD_APP_STATE run through all the application SUs and 
    * reassign all the SUSI assignments for the SG of which the SU is a member
    */

   if(cb->init_state == AVD_APP_STATE)
   {
      i_su = avnd->list_of_su;
      while (i_su != AVD_SU_NULL)
      {
         switch(i_su->sg_of_su->su_redundancy_model)
         {
         case AVSV_SG_RED_MODL_2N:
            /* Now analyze the service group for the new HA state
             * assignments and send the SU SI assign messages
             * accordingly.
             */
            avd_sg_2n_node_fail_func(cb,i_su);
            break;

         case AVSV_SG_RED_MODL_NWAY:
            /* Now analyze the service group for the new HA state
             * assignments and send the SU SI assign messages
             * accordingly.
             */
            avd_sg_nway_node_fail_func(cb,i_su);
            break;
         
         case AVSV_SG_RED_MODL_NPM:
            /* Now analyze the service group for the new HA state
             * assignments and send the SU SI assign messages
             * accordingly.
             */
            avd_sg_npm_node_fail_func(cb,i_su);
            break;

         case AVSV_SG_RED_MODL_NWAYACTV:
            /* Now analyze the service group for the new HA state
             * assignments and send the SU SI assign messages
             * accordingly.
             */
            avd_sg_nacvred_node_fail_func(cb,i_su);
            break;

         case AVSV_SG_RED_MODL_NORED:
         default:
            /* Now analyze the service group for the new HA state
             * assignments and send the SU SI assign messages
             * accordingly.
             */
            avd_sg_nored_node_fail_func(cb,i_su); 
            break;
         }
         /* Free all the SU SI assignments for all the SIs on the
          * the SU if there are any.
          */
   
         while (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
         {
   
            /* free all the CSI assignments  */
             avd_compcsi_list_del(cb,i_su->list_of_susi,FALSE);
             /* Unassign the SUSI */
             m_AVD_SU_SI_TRG_DEL(cb,i_su->list_of_susi);             
         }
   
         i_su->si_curr_active = 0;
         i_su->si_curr_standby = 0;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_su, AVSV_CKPT_AVD_SU_CONFIG);
         
         /* Since a SU has gone out of service relook at the SG to
          * re instatiate and terminate SUs if needed.
          */
         avd_sg_app_su_inst_func(cb, i_su->sg_of_su);
   
         i_su = i_su->avnd_list_su_next;
   
      } /* while (i_su != AVD_SU_NULL) */
      
   } /* if(cb->init_state == AVD_APP_STATE) */


   return;
}


/*****************************************************************************
 * Function: avd_sg_su_oper_list_add
 *
 * Purpose:  This function adds the SU to the list of SUs undergoing
 * operation. It allocates the holding structure.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the SU.
 *        ckpt - TRUE - add is called from ckpt update.
 *               FALSE - add is called from fsm.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_su_oper_list_add(AVD_CL_CB *cb, AVD_SU *su, NCS_BOOL ckpt)
{
   AVD_SG_OPER **i_su_opr;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_su_oper_list_add");
   
   /* Check that the current pointer in the SG is empty and not same as
    * the SU to be added.
    */
   if(su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
   {
      su->sg_of_su->su_oper_list.su = su;

      if (!ckpt)
         m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, su, AVSV_CKPT_AVD_SG_OPER_SU);

      return NCSCC_RC_SUCCESS;
   }
   
   if(su->sg_of_su->su_oper_list.su == su)
   {
      /* Log that it is already added return success. */
      m_AVD_LOG_RCVD_VAL(((long)su));
      return NCSCC_RC_SUCCESS;
   }
   
   i_su_opr = &su->sg_of_su->su_oper_list.next;
   while(*i_su_opr != AVD_SG_OPER_NULL)
   {
      if((*i_su_opr)->su == su)
      {
         /* Log that it is already added return success. */
         m_AVD_LOG_RCVD_VAL(((long)su));
         return NCSCC_RC_SUCCESS;
      }
      i_su_opr = &((*i_su_opr)->next);
   }
   
   /* Allocate the holder structure for having the pointer to the SU */
   *i_su_opr = m_MMGR_ALLOC_AVD_SG_OPER;
   
   if(*i_su_opr == AVD_SG_OPER_NULL)
   {
      /* log error about failure */
      m_AVD_LOG_MEM_FAIL_LOC(AVD_SG_OPER_ALLOC_FAILED);      
      m_AVD_LOG_INVALID_VAL_FATAL(((long)su));
      m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(su->name_net.value,su->name_net.length);      
      return NCSCC_RC_FAILURE;
   }
   
   m_AVD_LOG_RCVD_VAL(((long)(*i_su_opr)));
   m_AVD_LOG_RCVD_NAME_NET_VAL(su->name_net.value,su->name_net.length);
   /* Fill the content */
   (*i_su_opr)->su = su;
   (*i_su_opr)->next = AVD_SG_OPER_NULL;

   if (!ckpt)
      m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, su, AVSV_CKPT_AVD_SG_OPER_SU);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_sg_su_oper_list_del
 *
 * Purpose:  This function deletes the SU from the list of SUs undergoing
 * operation. It frees the holding structure.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the SU.
 *        ckpt - TRUE - add is called from ckpt update.
 *               FALSE - add is called from fsm.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_su_oper_list_del(AVD_CL_CB *cb, AVD_SU *su, NCS_BOOL ckpt)
{
   AVD_SG_OPER **i_su_opr, *temp_su_opr;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_su_oper_list_del");
   
   if(su->sg_of_su->su_oper_list.su == AVD_SU_NULL)
   {
      /* Log an error message that this shouldnt happen.*/
      m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
      return NCSCC_RC_SUCCESS;
   }
   
   if(su->sg_of_su->su_oper_list.su == su)
   {
      if(su->sg_of_su->su_oper_list.next != AVD_SG_OPER_NULL)
      {
         temp_su_opr = su->sg_of_su->su_oper_list.next;
         su->sg_of_su->su_oper_list.su = temp_su_opr->su;
         su->sg_of_su->su_oper_list.next = temp_su_opr->next;
         temp_su_opr->next = AVD_SG_OPER_NULL;
         temp_su_opr->su = AVD_SU_NULL;
         m_MMGR_FREE_AVD_SG_OPER(temp_su_opr);
      }else
      {
         su->sg_of_su->su_oper_list.su = AVD_SU_NULL;
      }   
         
      if(!ckpt)
         m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, su, AVSV_CKPT_AVD_SG_OPER_SU);      
      return NCSCC_RC_SUCCESS;
   }
   

   i_su_opr = &su->sg_of_su->su_oper_list.next;
   while(*i_su_opr != AVD_SG_OPER_NULL)
   {
      if((*i_su_opr)->su == su)
      {
         temp_su_opr = *i_su_opr;
         *i_su_opr = temp_su_opr->next;
         temp_su_opr->next = AVD_SG_OPER_NULL;
         temp_su_opr->su = AVD_SU_NULL;
         m_MMGR_FREE_AVD_SG_OPER(temp_su_opr);

         if(!ckpt)
            m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, su, AVSV_CKPT_AVD_SG_OPER_SU);

         return NCSCC_RC_SUCCESS;
      }
      
      i_su_opr = &((*i_su_opr)->next);
   }

   return NCSCC_RC_FAILURE;
}

/*****************************************************************************
 * Function: avd_sg_su_asgn_del_util
 *
 * Purpose:  This function is a utility routine that changes the assigning or
 * modifing FSM to assigned for all the SUSIs for the SU. If delete it removes
 * all the SUSIs assigned to the SU.    
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the SU.
 *        del_flag - The delete flag indicating if this is a delete.
 *        q_flag - The flag indicating if the HA state needs to be changed to
 *                 quiesced.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_sg_su_asgn_del_util(AVD_CL_CB *cb, AVD_SU *su, NCS_BOOL del_flag,
                              NCS_BOOL q_flag)
{
   AVD_SU_SI_REL *i_susi;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_su_asgn_del_util");
   
   i_susi = su->list_of_susi;
   if (del_flag == TRUE)
   {
      while (su->list_of_susi != AVD_SU_SI_REL_NULL)
      {
         /* free all the CSI assignments  */
         avd_compcsi_list_del(cb,su->list_of_susi,FALSE);
         /* Unassign the SUSI */
         m_AVD_SU_SI_TRG_DEL(cb,su->list_of_susi);
      }
      
      su->si_curr_standby = 0;
      su->si_curr_active = 0;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_AVD_SU_CONFIG);
   }else
   {
      if (q_flag == TRUE)
      {
         while (i_susi != AVD_SU_SI_REL_NULL)
         {
            if (i_susi->fsm != AVD_SU_SI_STATE_UNASGN)
            {
               i_susi->state = SA_AMF_HA_QUIESCED;
               m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);
               avd_gen_su_ha_state_changed_trap(cb, i_susi);

               /* generate the susi ha state change trap */
               avd_gen_su_si_assigned_trap(cb, i_susi);

               /* trigger pg upd */
               avd_pg_susi_chg_prc(cb, i_susi);

            }
            
            i_susi = i_susi->su_next;
         }

      }else
      {
         while (i_susi != AVD_SU_SI_REL_NULL)
         {
            if (i_susi->fsm != AVD_SU_SI_STATE_UNASGN)
            {
               i_susi->fsm = AVD_SU_SI_STATE_ASGND;
               m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);

               /* generate the susi ha state change trap */
               avd_gen_su_si_assigned_trap(cb, i_susi);

               /* trigger pg upd */
               avd_pg_susi_chg_prc(cb, i_susi);
            }            

            /* update the si counters */
            if (SA_AMF_HA_ACTIVE == i_susi->state)
            {
               m_AVD_SI_INC_ACTV_CURR_SU(i_susi->si);
               m_AVD_SI_DEC_STDBY_CURR_SU(i_susi->si);
            }

            /* update the si counters */
            if (SA_AMF_HA_ACTIVE == i_susi->state)
            {
               m_AVD_SI_INC_ACTV_CURR_SU(i_susi->si);
               m_AVD_SI_DEC_STDBY_CURR_SU(i_susi->si);
            }

            i_susi = i_susi->su_next;
         }
      }
   }
   
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: avd_sg_su_si_mod_snd
 *
 * Purpose:  This function is a utility function that assigns the state specified
 * to all the SUSIs that are assigned to the SU. If a failure happens it will
 * revert back to the orginal state.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the SU whose SUSIs needs to be modified.
 *        state - The HA state to which the state need to be modified. 
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This utility is used by 2N and N-way actice redundancy models.
 *
 * 
 **************************************************************************/

uns32 avd_sg_su_si_mod_snd(AVD_CL_CB *cb,AVD_SU *su,SaAmfHAStateT state)
{
   AVD_SU_SI_REL *i_susi;
   SaAmfHAStateT old_ha_state = SA_AMF_HA_ACTIVE;
   AVD_SU_SI_STATE old_state = AVD_SU_SI_STATE_ASGN;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_su_si_mod_snd");
   
   /* change the state for all assignments to the specified state. */
   i_susi = su->list_of_susi;
   while (i_susi != AVD_SU_SI_REL_NULL)
   {
   
      if (i_susi->fsm == AVD_SU_SI_STATE_UNASGN)
      {
         /* Ignore the SU SI that are getting deleted. */
         i_susi = i_susi->su_next;
         continue;
      }

      /* All The SU SIs will be in the same state */
      
      old_ha_state =  i_susi->state;
      old_state = i_susi->fsm;
      
      i_susi->state = state;
      i_susi->fsm = AVD_SU_SI_STATE_MODIFY;       
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);
      avd_gen_su_ha_state_changed_trap(cb, i_susi);
      i_susi = i_susi->su_next;
   }

   /* Now send a single message about the SU SI assignment to
    * the AvND for all the SIs assigned to the SU.
    */
   if (avd_snd_susi_msg(cb,su,AVD_SU_SI_REL_NULL,AVSV_SUSI_ACT_MOD)
         != NCSCC_RC_SUCCESS)
   {      
      /* log a fatal error that a message couldn't be sent */
      m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
      m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
      i_susi = su->list_of_susi;
      while (i_susi != AVD_SU_SI_REL_NULL)
      {
   
         if (i_susi->fsm == AVD_SU_SI_STATE_UNASGN)
         {
            /* Ignore the SU SI that are getting deleted. */
            i_susi = i_susi->su_next;
            continue;
         }

         i_susi->state = old_ha_state;
         i_susi->fsm = old_state;       
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);
         avd_gen_su_ha_state_changed_trap(cb, i_susi);
         i_susi = i_susi->su_next;
      }

      return NCSCC_RC_FAILURE;
   } /* if (avd_snd_susi_msg(cb,su,AVD_SU_SI_REL_NULL,AVSV_SUSI_ACT_MOD)
         != NCSCC_RC_SUCCESS)  */

   return NCSCC_RC_SUCCESS;   
}

/*****************************************************************************
 * Function: avd_sg_su_si_del_snd
 *
 * Purpose:  This function is a utility function that makes all the SUSIs that
 * are assigned to the SU as unassign. If a failure happens it will
 * revert back to the orginal state.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the SU whose SUSIs needs to be modified.
 *        
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: This utility is used by 2N and N-way actice redundancy models.
 *
 * 
 **************************************************************************/

uns32 avd_sg_su_si_del_snd(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU_SI_REL *i_susi;
   AVD_SU_SI_STATE old_state = AVD_SU_SI_STATE_ASGN;
   
   m_AVD_LOG_FUNC_ENTRY("avd_sg_su_si_del_snd");
   
   /* change the state for all assignments to the specified state. */
   i_susi = su->list_of_susi;
   while (i_susi != AVD_SU_SI_REL_NULL)
   {
      old_state = i_susi->fsm;
      i_susi->fsm = AVD_SU_SI_STATE_UNASGN;       
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);
      i_susi = i_susi->su_next;
   }

   /* Now send a single delete message about the SU SI assignment to
    * the AvND for all the SIs assigned to the SU.
    */
   if (avd_snd_susi_msg(cb,su,AVD_SU_SI_REL_NULL,AVSV_SUSI_ACT_DEL)
         != NCSCC_RC_SUCCESS)
   {      
      /* log a fatal error that a message couldn't be sent */
      m_AVD_LOG_INVALID_VAL_ERROR(((long)su));
      m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(su->name_net.value,su->name_net.length);
      i_susi = su->list_of_susi;
      while (i_susi != AVD_SU_SI_REL_NULL)
      {
         i_susi->fsm = old_state;   
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, i_susi, AVSV_CKPT_AVD_SU_SI_REL);
         i_susi = i_susi->su_next;
      }

      return NCSCC_RC_FAILURE;
   } /* if (avd_snd_susi_msg(cb,su,AVD_SU_SI_REL_NULL,AVSV_SUSI_ACT_DEL)
         != NCSCC_RC_SUCCESS)  */

   return NCSCC_RC_SUCCESS;   
}
