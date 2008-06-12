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

  This module deals with the AvD interaction with the AvM. 


..............................................................................

  FUNCTIONS INCLUDED in this file:

  avd_avm_nd_shutdown_func
  avd_handle_nd_failover_shutdown
  avd_chk_failover_shutdown_cxt

  avd_chk_nd_shutdown_valid
  avd_avm_mark_nd_absent

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#include "avd.h"

/****************************************************************************
 *  Name          : avd_avm_mark_nd_absent
 * 
 *  Description   : removing SUSIs avd_node_susi_fail_func() is also 
 *                  integral part of this func.
 *              
 *                  This function is called in the ctx of node extraction
 *                  or in response from AvM for the HPI reset issued by AVD.
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  avnd       - AVD_AVND node struc
 * 
 *  Return Values : TRUE/FALSE
 * 
 *  Notes         : The integrity of avnd need not be checked coz the state
 *                  and all other check have been done at the invoking 
 *                  function. 
 *
 ***************************************************************************/
void avd_avm_mark_nd_absent(AVD_CL_CB *cb, AVD_AVND *avnd)
{
   /* check whether we are in the context of shutdown, if yes
      we have to respond back to avm for the shutdown request
   */
   if(avnd->node_state == AVD_AVND_STATE_SHUTTING_DOWN)
   {
      avd_avm_send_shutdown_resp(cb, &avnd->node_info.nodeName, NCSCC_RC_SUCCESS);
   }

   /* first set the oper_state to disable */
   avnd->oper_state = NCS_OPER_STATE_DISABLE;

   avnd->node_state = AVD_AVND_STATE_ABSENT;
   avnd->avm_oper_state = NCS_OPER_STATE_ENABLE;

   avnd->node_info.bootTimestamp = 0;
   m_NCS_MEMSET(&(avnd->node_info.nodeAddress),'\0',sizeof(SaClmNodeAddressT));

   m_NCS_MEMSET(&(avnd->adest),'\0',sizeof(MDS_DEST));
   avnd->rcv_msg_id = 0;
   avnd->snd_msg_id = 0;

   /* Increment the view number as a node has left the
    * cluster.
    */   

   cb->cluster_num_nodes --;
   avnd->node_info.initialViewNumber = 0;
   ++(cb->cluster_view_number);
   avnd->node_info.member = SA_FALSE;

   /* Increment node failfast counter */
   cb->nodes_exit_cnt ++;

   m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVD_AVND_CONFIG);
   m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, cb, AVSV_CKPT_AVD_CB_CONFIG);


   /* Broadcast the node update to all the node directors only
    * those that are up will use the update.
    */

   avd_snd_node_update_msg(cb,avnd);
   
   /* failover all the SuSIs */
   avd_node_susi_fail_func(cb,avnd);
   return;
}

/****************************************************************************
 *  Name          : avd_chk_nd_shutdown_valid
 * 
 *  Description   : 
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  avnd       - AVD_AVND node struc
 * 
 *  Return Values : TRUE/FALSE
 * 
 *  Notes         : 
 *
 ***************************************************************************/

static SaBoolT avd_chk_nd_shutdown_valid(AVD_CL_CB *cb, AVD_AVND *avnd)
{
   AVD_SU      *i_su, *i_su_sg;

   if(avnd->type == AVSV_AVND_CARD_SYS_CON )
   {
      if( (cb->node_id_avd == avnd->node_info.nodeId) &&
          (cb->avail_state_avd == SA_AMF_HA_ACTIVE) )
      return SA_FALSE;
   }

   i_su = avnd->list_of_su;
   while(i_su != AVD_SU_NULL)
   {
      
      /* verify that two assigned SUs belonging to the same SG are not
       * on this node
       */
      if(i_su->list_of_susi != AVD_SU_SI_REL_NULL)
      {
         i_su_sg = i_su->sg_of_su->list_of_su;
         while (i_su_sg != AVD_SU_NULL)
         {
            if ((i_su != i_su_sg) &&
               (i_su->su_on_node == i_su_sg->su_on_node) &&
               (i_su_sg->list_of_susi != AVD_SU_SI_REL_NULL))
            {
               return SA_FALSE;
            }
            
            i_su_sg = i_su_sg->sg_list_su_next;
         }
         
         /* if SG to which this SU belongs is undergoing 
          * any semantics return false.
          */
         
         if (i_su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE)
         {
            
            /* Dont go ahead as a SG that is undergoing transition is
             * there related to this node.
             */
            return SA_FALSE;
         }
         
      } /* if(i_su->list_of_susi != AVD_SU_SI_REL_NULL) */
      
      /* get the next SU on the node */
      i_su = i_su->avnd_list_su_next;
   }
   return SA_TRUE;
}

/****************************************************************************
 *  Name          : avd_avm_nd_shutdown_func
 * 
 *  Description   : This routine is invoked by AvD when a message arrives
 *  from AVM. It converts the message to the corresponding event and posts
 *  the message to the mailbox for processing by the main loop.
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  evt        -  ptr to the received event
 * 
 *  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * 
 *  Notes         : Check for already existing failover context and if yes,
 *                  set the AVD state appropriately so that when the 
 *                  failover handling is complete the additional action of
 *                  terminating the components is also handled before sending
 *                  response to AvM.
 ***************************************************************************/

void avd_avm_nd_shutdown_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVM_AVD_MSG_T    *msg;
   AVM_LIST_NODE_T  *tmpNode = AVM_LIST_NODE_NULL;
   AVD_AVND         *avnd = AVD_AVND_NULL;
   NCS_BOOL         failover_pending = FALSE;
   AVD_SU           *i_su = AVD_SU_NULL;

   if (evt->info.avm_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   msg = evt->info.avm_msg;

   /* Do the sanity checks to see if the message is received in
   ** valid AVD state
   */

   if (cb->init_state < AVD_CFG_DONE)
   {
      /* This is a error situation. Without completing
       * initialization the AVD will not respond to
       * node.
       */

      /* log error */
      avm_avd_free_msg(&msg);
      m_AVD_LOG_INVALID_VAL_FATAL(cb->init_state);
      return;
   }

   if(msg->avm_avd_msg.shutdown_req.head.count == 0)
   {
      /* Nothing to be done return */
      avm_avd_free_msg(&msg);
      return;
   }

   /*for all the nodes in the list sent by AvM */
   tmpNode = msg->avm_avd_msg.shutdown_req.head.node;

   while(tmpNode)
   {
      /* first get the avnd structure from the name in message */
      avnd = avd_avnd_struc_find(cb, tmpNode->node_name);

      if(avnd == AVD_AVND_NULL)
      {
         /* LOG an error ? */
         m_AVD_LOG_INVALID_NAME_VAL_ERROR(tmpNode->node_name.value, tmpNode->node_name.length);
         /* Ignore sending failure of shutdown_response coz 
            AVM will drop it anyway */
         tmpNode = tmpNode->next;
         continue;
      }

      if((avnd->node_state == AVD_AVND_STATE_ABSENT) ||
         (avnd->node_state == AVD_AVND_STATE_NO_CONFIG) ||
         (avnd->node_state == AVD_AVND_STATE_NCS_INIT))
      {
        tmpNode = tmpNode->next;

        if (avnd->type != AVSV_AVND_CARD_SYS_CON)
        {
           m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);
          /* clean up the heartbeat timer for this node. */
           avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));
           m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);
         }
        /* Mark the Node as Shutting Down */ 
        avnd->node_state = AVD_AVND_STATE_SHUTTING_DOWN; 
        /*Mark the Node as Absent */
         avd_avm_mark_nd_absent(cb, avnd);

         continue;
      }

       /* Check if this node is in semantics of shutdown */
      if(avnd->node_state == AVD_AVND_STATE_SHUTTING_DOWN) 
      {
         tmpNode = tmpNode->next;
         continue;
      }

     /* Check if this node is in semantics of node going down */
      if(avnd->node_state == AVD_AVND_STATE_GO_DOWN) 
      {
         avnd->node_state = AVD_AVND_STATE_SHUTTING_DOWN;
         avnd->avm_oper_state = NCS_OPER_STATE_DISABLE;
         tmpNode = tmpNode->next;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_AVM_OPER_STATE);
         continue;
      }


      if(avd_chk_nd_shutdown_valid(cb, avnd) == SA_FALSE)
      {
         tmpNode = tmpNode->next;
         avd_avm_send_shutdown_resp(cb, &avnd->node_info.nodeName, NCSCC_RC_FAILURE);
         continue;
      }

      /* Check if this node is already in the context of node failover */
      if((avnd->avm_oper_state == NCS_OPER_STATE_DISABLE) &&
         (avnd->node_state == AVD_AVND_STATE_PRESENT) )
      {
         /* check for the susi list and if not empty just change the node_state
         ** and continue. coz the failover semantics are still on.
         **
         ** if empty terminate the app_sus and then go to NCS_SUs.
         */

         failover_pending = FALSE;
         i_su = avnd->list_of_su;

         while(i_su != AVD_SU_NULL)
         {
            if(i_su->list_of_susi != AVD_SU_SI_REL_NULL)
            {
               failover_pending = TRUE;
               break;
            }
            i_su = i_su->avnd_list_su_next;
         }

         if(failover_pending == TRUE)
         {
            avnd->node_state = AVD_AVND_STATE_SHUTTING_DOWN;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
         }
         else
         {
            avnd->node_state = AVD_AVND_STATE_SHUTTING_DOWN;
            avnd->avm_oper_state = NCS_OPER_STATE_DISABLE;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_AVM_OPER_STATE);
            avd_snd_shutdown_app_su_msg(cb, avnd);
            avd_handle_nd_failover_shutdown(cb, avnd, SA_TRUE);
         }
         tmpNode = tmpNode->next;
         continue;
      }

      avnd->node_state = AVD_AVND_STATE_SHUTTING_DOWN;
      avnd->avm_oper_state = NCS_OPER_STATE_DISABLE;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE);
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_AVM_OPER_STATE);
      avd_handle_nd_failover_shutdown(cb, avnd, SA_FALSE);

      tmpNode = tmpNode->next;
   }

   avm_avd_free_msg(&msg);
   return;
}


/****************************************************************************
 *  Name          : avd_handle_nd_failover_shutdown
 * 
 *  Description   : This routine is invoked by AvD to handle the shutdown
 *                  request. 
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  avnd       -  ptr to AVD_AVND structure
 * 
 *  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * 
 *  Notes         : 
 ***************************************************************************/

void avd_handle_nd_failover_shutdown(AVD_CL_CB *cb, AVD_AVND *avnd, SaBoolT for_ncs)
{
   AVD_SU     *i_su = AVD_SU_NULL;
   NCS_BOOL   assign_list_empty = TRUE;

   if(avnd == AVD_AVND_NULL)
   {
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }


   /* Mark the node operational state as disable and make all the
    * application SUs in the node as O.O.S. Also call the SG FSM
    * to do the reallignment of SIs for assigned SUs.
    */
   if(for_ncs == SA_TRUE)
      i_su = avnd->list_of_ncs_su;
   else
      i_su = avnd->list_of_su;
      
   while(i_su != AVD_SU_NULL)
   {

      m_AVD_SET_SU_REDINESS(cb,i_su,NCS_OUT_OF_SERVICE);  
      if (i_su->list_of_susi != AVD_SU_SI_REL_NULL)
      {
         /* Since assignments exists call the SG FSM.
          */
         assign_list_empty = FALSE;

         switch(i_su->sg_of_su->su_redundancy_model)
         {
         case AVSV_SG_RED_MODL_2N:
            if (avd_sg_2n_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
            {
               /* log error about the failure */                    
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,
                                                    i_su->name_net.length);
               return;
            }                  
            break;

         case AVSV_SG_RED_MODL_NWAY:
            if (avd_sg_nway_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
            {
               /* log error about the failure */                    
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,
                                                    i_su->name_net.length);
               return;
            }                  
            break;
            
         case AVSV_SG_RED_MODL_NWAYACTV:
            if (avd_sg_nacvred_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
            {
               /* log error about the failure */                    
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,
                                                    i_su->name_net.length);
               return;
            }                  
            break;
            
         case AVSV_SG_RED_MODL_NPM:
            if (avd_sg_npm_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
            {
               /* log error about the failure */                    
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,
                                                    i_su->name_net.length);
               return;
            }                  
            break;

         case AVSV_SG_RED_MODL_NORED:
         default:
            if (avd_sg_nored_su_fault_func(cb,i_su) == NCSCC_RC_FAILURE)
            {
               /* log error about the failure */                    
               m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(i_su->name_net.value,
                                                    i_su->name_net.length);
               return;
            }       
            break;
         } 

      }
      
      i_su = i_su->avnd_list_su_next;
   }/* while(i_su != AVD_SU_NULL)*/

   if(assign_list_empty == TRUE)
   {
      /* move to next state */
      avd_chk_failover_shutdown_cxt(cb, avnd, for_ncs);
   }
}

/****************************************************************************
 *  Name          : avd_chk_failover_shutdown_cxt
 * 
 *  Description   : This is called in the context when the SUSI is 
 *                  deleted to check the context if node failover or
 *                  node shutdown is in progress if yes, we check for
 *                  the emptiness and then do the required processing.
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  avnd       -  ptr to AVD_AVND structure
 *                  is_ncs     -  flag to indicate if in cxt of NCS SUs
 * 
 *  Return Values : None
 * 
 *  Notes         : 
 ***************************************************************************/

void avd_chk_failover_shutdown_cxt(AVD_CL_CB *cb, AVD_AVND *avnd, 
                                   SaBoolT is_ncs)
{
   AVD_SU     *i_su = AVD_SU_NULL;
   /* check for avm_oper_state if its disable we are in context of 
   ** failover or shutdown 
   */
   if(avnd->avm_oper_state == NCS_OPER_STATE_ENABLE)
      return;

   if(is_ncs == SA_FALSE)
   {

      i_su = avnd->list_of_su;

      while(i_su != AVD_SU_NULL)
      {
         if(i_su->list_of_susi != AVD_SU_SI_REL_NULL)
         {
            /* More susi removal pending on this SU nothing to be
            ** done now.
            */
            return;
         }
         i_su = i_su->avnd_list_su_next;
      }
      /* If we are here we walked thru the entire list and found it empty 
      ** All the APP_sus are now unassigned. 
      ** 
      ** Check for the context failover/shutdown
      ** if failover send a  response to AvM and consider done with FAILOVER
      ** else call a function to send terminate message to AvND and
      ** process NCS_SUs
      */
      if(avnd->node_state != AVD_AVND_STATE_SHUTTING_DOWN) 
      {
         avd_avm_send_failover_resp(cb, &avnd->node_info.nodeName, NCSCC_RC_SUCCESS);
         return;
      }
      else
      {
         avd_snd_shutdown_app_su_msg(cb, avnd);
         return;
      }

      
   }
   else /*if(is_ncs == SA_TRUE ) */
   {
      /* make sure this happens only in the shutdown context */
      if(avnd->node_state != AVD_AVND_STATE_SHUTTING_DOWN) 
         return; /* Should log an error ? */

      /* Now check if all the susi's of NCS SUs on the entire node 
      ** are deleted
      */
      i_su = avnd->list_of_ncs_su;

      while(i_su != AVD_SU_NULL)
      {
         if(i_su->list_of_susi != AVD_SU_SI_REL_NULL)
         {
            /* More susi removal pending on this SU nothing to  be
            ** done now.
            */
            return;
         }
         i_su = i_su->avnd_list_su_next;
      }
      /* If we are here we walked thru the entire list and found it empty 
      ** All the ncs_sus are now unassigned. now stop heartbeating with AvND
      ** If the AvND is SYSCON stop AvD-AvD HeartBeat and send
      ** response to AvM
      */

      m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

      /* stop the timer if it exists */
      avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));

      m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

      avd_avm_mark_nd_absent(cb, avnd);
   } /* End is_ncs == TRUE */

}



/****************************************************************************
 *  Name          : avd_avm_nd_failover_func
 * 
 *  Description   : 
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  evt        -  ptr to the received event
 * 
 *  Return Values : None
 * 
 *  Notes         : 
 *
 ***************************************************************************/

void avd_avm_nd_failover_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVM_AVD_MSG_T    *msg;
   AVM_LIST_NODE_T  *tmpNode = AVM_LIST_NODE_NULL;
   AVD_AVND         *avnd = AVD_AVND_NULL;
   NCS_BOOL         failover_pending = FALSE;
   AVD_SU           *i_su = AVD_SU_NULL;

   if (evt->info.avm_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   msg = evt->info.avm_msg;

   /* Do the sanity checks to see if the message is received in
   ** valid AVD state
   */

   if (cb->init_state < AVD_CFG_DONE)
   {
      /* This is a error situation. Without completing
       * initialization the AVD will not respond to
       * node.
       */

      /* log error */
      avm_avd_free_msg(&msg);
      m_AVD_LOG_INVALID_VAL_FATAL(cb->init_state);
      return;
   }

   if(msg->avm_avd_msg.failover_req.head.count == 0)
   {
      /* Nothing to be done return */
      avm_avd_free_msg(&msg);
      return;
   }

   /*for all the nodes in the list sent by AvM */
   tmpNode = msg->avm_avd_msg.failover_req.head.node;

   while(tmpNode)
   {
      /* first get the avnd structure from the name in message */
      avnd = avd_avnd_struc_find(cb, tmpNode->node_name);

      if(avnd == AVD_AVND_NULL)
      {
         /* LOG an error ? */
         m_AVD_LOG_INVALID_NAME_VAL_ERROR(tmpNode->node_name.value, tmpNode->node_name.length);
         tmpNode = tmpNode->next;
         continue;
      }

      /* Check if this node is already in the context of node failover */
      if((avnd->avm_oper_state == NCS_OPER_STATE_DISABLE) &&
         (avnd->node_state == AVD_AVND_STATE_PRESENT) )
      {
         /* check for the susi list and if not empty just change the node_state
         ** and continue. coz the failover semantics are still on.
         **
         ** if empty terminate the app_sus and then go to NCS_SUs.
         */

         failover_pending = FALSE;
         i_su = avnd->list_of_su;

         while(i_su != AVD_SU_NULL)
         {
            if(i_su->list_of_susi != AVD_SU_SI_REL_NULL)
            {
               failover_pending = TRUE;
               break;
            }
            i_su = i_su->avnd_list_su_next;
         }

         if(failover_pending == FALSE)
         {
            avnd->avm_oper_state = NCS_OPER_STATE_DISABLE;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_AVM_OPER_STATE);
            /* call the func handle_nd_shutdown the context will
            ** take care of failover semantics 
            */
            avd_handle_nd_failover_shutdown(cb, avnd, SA_FALSE);
         }
         tmpNode = tmpNode->next;
         continue;

      }

      avnd->avm_oper_state = NCS_OPER_STATE_DISABLE;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_AVM_OPER_STATE);

      avd_handle_nd_failover_shutdown(cb, avnd, SA_FALSE);

      tmpNode = tmpNode->next;
   }

   avm_avd_free_msg(&msg);
   return;
}

/****************************************************************************
 *  Name          : avd_avm_fault_domain_rsp
 * 
 *  Description   : 
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  evt        -  ptr to the received event
 * 
 *  Return Values : None
 * 
 *  Notes         : 
 *
 ***************************************************************************/

void avd_avm_fault_domain_rsp(AVD_CL_CB *cb, AVD_EVT *evt)
{
   return;
}


/****************************************************************************
 *  Name          : avd_avm_nd_reset_rsp_func
 * 
 *  Description   : 
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  evt        -  ptr to the received event
 * 
 *  Return Values : None
 * 
 *  Notes         :  
 *
 ***************************************************************************/

void avd_avm_nd_reset_rsp_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVM_AVD_MSG_T    *msg;
   AVD_AVND         *avnd = AVD_AVND_NULL;

   if (evt->info.avm_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   msg = evt->info.avm_msg;

   /* Do the sanity checks to see if the message is received in
   ** valid AVD state
   */

   if (cb->init_state < AVD_CFG_DONE)
   {
      /* This is a error situation. Without completing
       * initialization the AVD will not respond to
       * node.
       */

      /* log error */
      avm_avd_free_msg(&msg);
      m_AVD_LOG_INVALID_VAL_FATAL(cb->init_state);
      return;
   }

   /* first get the avnd structure from the name in message */
   avnd = avd_avnd_struc_find(cb, msg->avm_avd_msg.reset_resp.node_name);

   if(avnd == AVD_AVND_NULL)
   {
      /* LOG an error ? */
      /*m_AVD_LOG_INVALID_NAME_VAL_ERROR(tmpNode->node_name, tmpNode->node_name.length);*/
      avm_avd_free_msg(&msg);
      return;
   }

   /* common for both the success/failure response */
   if(avnd->node_info.nodeId == cb->node_id_avd)
   {
      /* Call leap macro to shutdown this node */
      m_NCS_DBG_PRINTF("\nAvSv: Card going for reboot - Some one has reset this card\n");
      m_NCS_SYSLOG(NCS_LOG_ERR,"NCS_AvSv: Card going for reboot - Some one has reset this card");
      m_NCS_REBOOT;
   }
   else if(avnd->node_state != AVD_AVND_STATE_ABSENT) 
   {

      m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

      /* clean up the heartbeat timer for this node. */
      avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));
      
     m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);


      avd_avm_mark_nd_absent(cb, avnd);
   }

   avm_avd_free_msg(&msg);
   return;
}


/****************************************************************************
 *  Name          : avd_avm_nd_oper_st_func
 * 
 *  Description   : 
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  evt        -  ptr to the received event
 * 
 *  Return Values : None
 * 
 *  Notes         :  
 *
 ***************************************************************************/

void avd_avm_nd_oper_st_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVM_AVD_MSG_T    *msg;
   AVM_LIST_NODE_T  *tmpNode = AVM_LIST_NODE_NULL;
   AVD_AVND         *avnd = AVD_AVND_NULL;
   AVD_SU           *i_su = AVD_SU_NULL;

   if (evt->info.avm_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   msg = evt->info.avm_msg;

   /* Do the sanity checks to see if the message is received in
   ** valid AVD state
   */

   if (cb->init_state < AVD_CFG_DONE)
   {
      /* This is a error situation. Without completing
       * initialization the AVD will not respond to
       * node.
       */

      /* log error */
      avm_avd_free_msg(&msg);
      m_AVD_LOG_INVALID_VAL_FATAL(cb->init_state);
      return;
   }

   if(msg->avm_avd_msg.operstate.head.count == 0)
   {
      /* Nothing to be done return */
      avm_avd_free_msg(&msg);
      return;
   }

   /*for all the nodes in the list sent by AvM */
   tmpNode = msg->avm_avd_msg.operstate.head.node;

   while(tmpNode)
   {
      /* first get the avnd structure from the name in message */
      avnd = avd_avnd_struc_find(cb, tmpNode->node_name);

      if(avnd == AVD_AVND_NULL)
      {
         /* LOG an error ? */
         m_AVD_LOG_INVALID_NAME_VAL_ERROR(tmpNode->node_name.value, tmpNode->node_name.length);
         tmpNode = tmpNode->next;
         continue;
      }

      switch(msg->avm_avd_msg.operstate.oper_state)
      {
      case AVM_NODES_ABSENT:
   
         /* common for both the success/failure response */
         if(avnd->node_info.nodeId == cb->node_id_avd)
         {
            /* Call leap macro to shutdown this node */
            m_NCS_DBG_PRINTF("\nAvSv: Card going for reboot - Got absent for this node\n");
            m_NCS_SYSLOG(NCS_LOG_ERR,"NCS_AvSv: Card going for reboot - Got absent for this node");
            m_NCS_REBOOT;
         }else if(avnd->node_state != AVD_AVND_STATE_ABSENT)
         {
            /* Extraction was done so mark the AvND to ABSENT */
            /* If this was not when the node_state is shutting down
            ** do necessary clean-up
            */
            m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

            /* clean up the heartbeat timer for this node. */
            avd_stop_tmr(cb, &(avnd->heartbeat_rcv_avnd));
      
            m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

            avd_avm_mark_nd_absent(cb, avnd);
         }

         break;
      case AVM_NODES_ENABLED:
         /* This means the node has been plugged in. Change the 
         ** AvM oper state and set the readiness state of the SUs 
         ** and take necessary IN_SVC actions
         */
         
         if(avnd->node_state != AVD_AVND_STATE_SHUTTING_DOWN) 
         {

            avnd->avm_oper_state = NCS_OPER_STATE_ENABLE;

            /* For each of the SUs calculate the readiness state. 
            ** call the SG FSM with the new readiness state.
             */
         
            i_su = avnd->list_of_su;
            while(i_su != AVD_SU_NULL)
            {
               if(m_AVD_APP_SU_IS_INSVC(i_su))
               {
                  i_su->readiness_state = NCS_IN_SERVICE;
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
                  
                  /* Since an SU has come in-service re look at the SG to see if other
                   * instantiations or terminations need to be done.
                   */
                  avd_sg_app_su_inst_func(cb, i_su->sg_of_su);            
               }
               /* get the next SU on the node */
               i_su = i_su->avnd_list_su_next;
            }
         }
         break; /* case AVM_NODES_ENABLED: */

      case AVM_NODES_DISABLED:
      default:
         break;
      } /* END SWITCH */
         
      tmpNode = tmpNode->next;
   }
  
   avm_avd_free_msg(&msg); 
   return;
}


/*****************************************************************************
 * Function: avd_shutdown_app_su_resp_func
 *
 * Purpose:  This function is the handler for response  
 * event indicating that all the Application SUs have been terminated.
 * Now the AVD will continue processing the NCS SUs.
 *
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

void avd_shutdown_app_su_resp_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   AVD_DND_MSG *n2d_msg;
   AVD_AVND *avnd;

   m_AVD_LOG_FUNC_ENTRY("avd_shutdown_app_su_resp_func");

   if (evt->info.avnd_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   n2d_msg = evt->info.avnd_msg;
   
   m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,n2d_msg,sizeof(AVD_DND_MSG),n2d_msg);

   if ((avnd = avd_avnd_struc_find_nodeid(cb,
             n2d_msg->msg_info.n2d_shutdown_app_su.node_id)) == AVD_AVND_NULL)
   {
      /* log error that the node id is invalid */
      m_AVD_LOG_INVALID_VAL_ERROR(n2d_msg->msg_info.n2d_shutdown_app_su.node_id);
      avsv_dnd_msg_free(n2d_msg);
      evt->info.avnd_msg = NULL;
      return ;
   }
   
   /* update the receive id count */
   /* checkpoint the AVND structure : receive id */
   m_AVD_SET_AVND_RCV_ID(cb,avnd,(n2d_msg->msg_info.n2d_shutdown_app_su.msg_id));

   /* 
    * Send the Ack message to the node, indicationg that the message with this
    * message ID is received successfully.
    */
   if (avd_snd_node_ack_msg(cb, avnd, avnd->rcv_msg_id) != NCSCC_RC_SUCCESS)
   {
      /* log error that the director is not able to send the message */
      m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
   }

   avd_handle_nd_failover_shutdown(cb, avnd, SA_TRUE);

   avsv_dnd_msg_free(n2d_msg);
   evt->info.avnd_msg = NULL;
   return;
}



