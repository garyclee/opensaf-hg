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

  This file includes the routines to handle the last step of terminating the 
  node.
 
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/
#include "avnd.h"
#include "nid_api.h"

/****************************************************************************
  Name          : avnd_evt_last_step_clean
 
  Description   : The last step term is done or failed. Now we use brute force
                  to clean.
  
                  This routine processes clean up of all SUs (NCS/APP if exist).
                  Even if the clean up of all components is not successful, 
                  We still go ahead and free the DB records coz this is 
                  last step anyway.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : All the errors are ignored and brute force is employed.

******************************************************************************/
static void avnd_last_step_clean(AVND_CB *cb)
{
   uns32         rc = NCSCC_RC_SUCCESS;
   AVND_SU       *su = 0;
   AVND_COMP     *comp=0;

   /* We have to loop to call cleanup script, However we cannot delete 
   ** record right here and have to loop again in call 
   ** avnd_compdb_destroy() coz comp_csi is not deleted.
   */
   comp = (AVND_COMP *) ncs_patricia_tree_getnext(&cb->compdb, (uns8 *)0);

   while(comp != 0)
   {
      avnd_comp_clc_cmd_execute(cb, comp, AVND_COMP_CLC_CMD_TYPE_CLEANUP);

      comp = (AVND_COMP *)
             ncs_patricia_tree_getnext(&cb->compdb, (uns8 *)&comp->name_net);
   }

   su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uns8 *)0);
   
   /* We have to loop to get rid of si_list, we cannot delete record right here
    * and have loop again in call avnd_sudb_destroy() coz compdb is not deleted.
    */
   while(su != 0)
   {
      avnd_su_si_del(cb, &su->name_net);

      su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uns8 *)&su->name_net);
   } /* end while SU */

   if(m_AVND_IS_AVD_HB_ON(cb))
   {
      /* stop the current hb-tmr */
      m_AVND_TMR_HB_STOP(cb);
   }

   /* Mark the destroy flag */
   cb->destroy = TRUE;
   
   /* NOTIFY THE NIS sript that we are done cleanup */
   nis_notify("DONE", &rc);

   return;
}

/****************************************************************************
  Name          : avnd_evt_last_step_term
 
  Description   : 1. Send a message to AVD about the signal.
                  2. Do the cleanup of the nodes
                  3. Call the NID API informing the completion of cleanup.
  
                  This routine processes clean up of all SUs (NCS/APP if exist).
                  Even if the clean up of all components is not successful, We still
                  go ahead and free the DB records coz this is last step 
                  anyway.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : All the errors are ignored and brute force deletes are
                  employed. There might be an issue with ASSERT in SU_REC_DEL
                  However, it doesnt matter coz this is during the last step
                  and NID script will timeout and kill anyway.
******************************************************************************/
uns32 avnd_evt_last_step_term(AVND_CB *cb, AVND_EVT *evt)
{
   uns32         rc = NCSCC_RC_SUCCESS;
   AVND_SU       *su = 0;
   NCS_BOOL      empty_sulist = TRUE;

   if(cb->term_state != AVND_TERM_STATE_SHUTTING_NCS_SI)
   {
      avnd_last_step_clean(cb);
   }
   else
   {
      /* send terminate for all NCS SUs and when done send nis_notify */

      cb->term_state = AVND_TERM_STATE_SHUTTING_NCS_SU;
      
      /* dont process unless AvD is up */
      if ( !m_AVND_CB_IS_AVD_UP(cb) ) 
         return rc;

      su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uns8 *)0);

      /* scan & drive the SU term by PRES_STATE FSM on each su */
      while(su != 0)
      {
         if( su->is_ncs == SA_FALSE )
         {
            su = (AVND_SU *)
                 ncs_patricia_tree_getnext(&cb->sudb, (uns8*)&su->name_net);
            continue;
         }

         /* to be on safer side lets remove the si's remaining if any */
         rc = avnd_su_si_remove(cb, su, 0);

         /* delete all the curr info on su & comp */
         rc = avnd_su_curr_info_del(cb, su);

         /* now terminate the su */
         if(( m_AVND_SU_IS_PREINSTANTIABLE(su) ) && 
            (su->pres != NCS_PRES_UNINSTANTIATED) &&
            (su->pres != NCS_PRES_INSTANTIATIONFAILED) &&
            (su->pres != NCS_PRES_TERMINATIONFAILED) )
         {
            empty_sulist = FALSE;

            /* trigger su termination for pi su */
            rc = avnd_su_pres_fsm_run(cb, su, 0, AVND_SU_PRES_FSM_EV_TERM);
         }
         
         su = (AVND_SU *)
              ncs_patricia_tree_getnext(&cb->sudb, (uns8*)&su->name_net);
      }

      if(empty_sulist == TRUE)
      {
         /* No SUs to be processed for termination.
         ** we are DONE. Inform NIS the same. 
         */

         if(m_AVND_IS_AVD_HB_ON(cb))
         {
            /* stop the current hb-tmr */
            m_AVND_TMR_HB_STOP(cb);
         }

         /* Mark the destroy flag */
         cb->destroy = TRUE;

         /* NOTIFY THE NIS sript that we are done cleanup */
         nis_notify("DONE", &rc);
    
      }

   }

   return rc;
}


/****************************************************************************
  Name          : avnd_check_su_shutdown_done
 
  Description   : Call the NID API informing the completion of init.
  
 
  Arguments     : cb  - ptr to the AvND control block
                  is_ncs - boolean to indicate if the SU terminated is
                           an NCS SU.
 
  Return Values : None.
 
  Notes         : 
******************************************************************************/
void avnd_check_su_shutdown_done(AVND_CB *cb, NCS_BOOL is_ncs)
{
   uns32         rc = NCSCC_RC_SUCCESS;
   AVND_SU       *su = 0;

   su = (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uns8 *)0);

   /* scan SU term by PRES_STATE FSM on each su */
   while(su != 0)
   {
      if( su->is_ncs != is_ncs )
      {
         su = (AVND_SU *)
              ncs_patricia_tree_getnext(&cb->sudb, (uns8*)&su->name_net);
         continue;
      }
      
      /* Check the state of the SU if they are in final state */
      if((su->pres != NCS_PRES_UNINSTANTIATED) &&
         (su->pres != NCS_PRES_INSTANTIATIONFAILED) &&
         (su->pres != NCS_PRES_TERMINATIONFAILED) )
      {
         /* There are still some SUs to be terminated. We are not done 
         ** so just return */
         return;
      }
      su = (AVND_SU *)
              ncs_patricia_tree_getnext(&cb->sudb, (uns8*)&su->name_net);
   }

   if(is_ncs == TRUE)
   {
      /* All NCS SUs have finished their termination. Now call the 
      ** cleanup of CB
      */
 
      /* Mark the destroy flag */
      cb->destroy = TRUE;

      /* NOTIFY THE NIS sript that we are done cleanup */
      nis_notify("DONE", &rc);
   }
   else
   {
      /* No SUs to be processed for termination.
      ** send the response message to AVD informing DONE. 
      */
      cb->term_state = AVND_TERM_STATE_SHUTTING_APP_DONE;
      avnd_snd_shutdown_app_su_msg(cb);
   }

   return ;
}

/****************************************************************************
  Name          : avnd_evt_avd_set_leds_msg
 
  Description   : Call the NID API informing the completion of init.
  
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : 
******************************************************************************/
uns32 avnd_evt_avd_set_leds_msg(AVND_CB *cb, AVND_EVT *evt)
{
   AVSV_D2N_SET_LEDS_MSG_INFO *info = &evt->info.avd->msg_info.d2n_set_leds;
   uns32                      rc = NCSCC_RC_SUCCESS;

   if (info->msg_id != (cb->rcv_msg_id+1))
   {
      /* Log Error */
      rc = NCSCC_RC_FAILURE;
      m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_EMERGENCY, 
                AVND_LOG_MSG_ID_MISMATCH, info->msg_id);

      goto done;
   }

   cb->rcv_msg_id = info->msg_id;

   if(cb->led_state == AVND_LED_STATE_GREEN)
   {
      /* Nothing to be done we have already got this msg */
      goto done;
   }

   cb->led_state = AVND_LED_STATE_GREEN;

   /* Notify the NIS script/deamon that we have fully come up */
   m_NCS_NID_NOTIFY(NCSCC_RC_SUCCESS);

done:
   return rc;
}
