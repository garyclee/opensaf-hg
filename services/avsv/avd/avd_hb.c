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

  This module deals with sending and processing the heart beating messages
  between the AvDs. It also contains functionality to process the up and
  down events of the other AvD.

..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

/*****************************************************************************
 * Function: avd_init_heartbeat
 *
 * Purpose:  This function initiates the heart beating between two AVD's. We 
 *           are using MBCSV's notify messages for heart beating. This function
 *           is called on receiving peer_info call back, which is the indication
 *           of peer AVD coming up. OR Do we call it when standby sync-up with 
 *           Active??
 *
 * Input:    cb - AVD control block.
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_init_heartbeat(AVD_CL_CB *cb)
{
   m_AVD_LOG_FUNC_ENTRY("avd_init_heartbeat");

   /* 
    * Send the heartbeat message to the other AvD, start the send heartbeat timer and  
    * start the receive timer for receiving heart beat messages from the other AvD.
    */
   if (NCSCC_RC_SUCCESS != avd_snd_hb_msg(cb))
   {
      /* Log error */
      m_AVD_LOG_CKPT_EVT(AVD_HB_MSG_SND_FAILURE, 
            NCSFL_SEV_EMERGENCY, NCSCC_RC_FAILURE);
   }

   m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

   avd_stop_tmr(cb, &cb->heartbeat_send_avd);
   m_AVD_SND_HB_TMR_START(cb);

   avd_stop_tmr(cb, &cb->heartbeat_rcv_avd);
   m_AVD_RCV_HB_TMR_START(cb);

   m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_d2d_heartbeat_msg_func
 *
 * Purpose:  This function is the handler for other director heartbeat event
 * indicating the arrival of the heartbeat message from the other director. 
 * This function will restart the heartbeat timer and verify that the information
 * received is unchanged from the past.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_d2d_heartbeat_msg_func(AVD_CL_CB *cb)
{
   m_AVD_LOG_FUNC_ENTRY("avd_d2d_heartbeat_msg_func");

   m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);
   avd_stop_tmr(cb, &cb->heartbeat_rcv_avd);

   /* Restart the receive heart beat timer for receiving
    * next heart beat message. */

   m_AVD_RCV_HB_TMR_START(cb);
   
   /* Inform AVM About the first heart beat recevied 
      from peer AVD */

   if(FALSE == cb->avd_hrt_beat_rcvd)
   {
     avd_avm_d_hb_restore_msg(cb, cb->node_id_avd_other);
     /* message sent to AVM */
     cb->avd_hrt_beat_rcvd = TRUE;
   }
 
   m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

   return;
}


/*****************************************************************************
 * Function: avd_tmr_snd_hb_func
 *
 * Purpose:  This function is the handler for send heartbeat timer event
 * indicating the expiry of the send heartbeat timer on the active AVD. This 
 * function will send the heartbeat message to the other director and  
 * restart the send heartbeat timer.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_tmr_snd_hb_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   m_AVD_LOG_FUNC_ENTRY("avd_tmr_snd_hb_func");

   if ((cb->node_id_avd_other == 0) ||
         (cb->other_avd_adest == 0) )
      return;

   if (evt->info.tmr.type != AVD_TMR_SND_HB)
   {
      /* log error that a wrong timer type value */
      m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.type);
      return;
   }

   /* Send Heart beat notify message to peer */
   if (NCSCC_RC_SUCCESS != avd_snd_hb_msg(cb))
   {
      /* Log error */
      m_AVD_LOG_CKPT_EVT(AVD_HB_MSG_SND_FAILURE, 
            NCSFL_SEV_EMERGENCY, NCSCC_RC_FAILURE);
   }

   m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);
   avd_stop_tmr(cb, &cb->heartbeat_send_avd);

   m_AVD_SND_HB_TMR_START(cb);
   /*avd_start_tmr(cb,&(cb->heartbeat_send_avd),cb->snd_hb_intvl); */
   m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

   return;
}


/*****************************************************************************
 * Function: avd_tmr_rcv_hb_d_func
 *
 * Purpose:  This function is the handler for receive directors heartbeat 
 * timer event on active director indicating the expiry of the receive 
 * heartbeat timer related to the standby director. This function will 
 * issue restart request to HPI to restart the system controller card on
 * which the standby is running. It then marks the node director on that
 * card as down and migrates all the Service instances to other inservice SUs.
 * this is similar to avd_mds_avd_down_func.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_tmr_rcv_hb_d_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   AVD_AVND *avnd = AVD_AVND_NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_tmr_rcv_hb_d_func");
   m_AVD_LOG_CKPT_EVT(AVD_HB_MISS_WITH_PEER, 
         NCSFL_SEV_NOTICE, cb->node_id_avd_other);
   syslog(LOG_WARNING, "AVD: Heart Beat missed with standby director on %x",
          cb->node_id_avd_other);

   /* we need not do a sync send to stanby */
   cb->sync_required = FALSE;

   if (evt->info.tmr.type != AVD_TMR_RCV_HB_D)
   {
      /* log error that a wrong timer type value */
      m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.type);
      return;
   }

   /* Inform AVM About this */
   avd_avm_d_hb_lost_msg(cb, cb->node_id_avd_other);
  
   /*Set the first heat beat variable to False */
    cb->avd_hrt_beat_rcvd = FALSE;

   /* get avnd ptr to call avd_avm_mark_nd_absent */
   if( (avnd = avd_avnd_struc_find_nodeid(cb, cb->node_id_avd_other)) == AVD_AVND_NULL)
   {
      /* we can't do anything without getting avnd ptr. just return */
      m_AVD_LOG_INVALID_VAL_FATAL(cb->node_id_avd_other);
      return;
   }

   /* check if the node was undergoing shutdown, if so send shutdown response */
   if(avnd->node_state == AVD_AVND_STATE_SHUTTING_DOWN)
      avd_avm_send_reset_req(cb, &avnd->node_info.nodeName); 


   return;
}


/*****************************************************************************
 * Function: avd_tmr_rcv_hb_init_func
 *
 * Purpose:  This function is the handler for receive directors heartbeat 
 * init timer event indicating the expiry of the receive heartbeat timer timer 
 * related to the other director. This function will assume that no other system
 * controller is up so it will assume its HA state to be active and change its
 * init state to up.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_tmr_rcv_hb_init_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   return;
}


/*****************************************************************************
 * Function: avd_mds_avd_up_func
 *
 * Purpose:  This function is the handler for the other AvD up event from
 * mds. The function right now is just a place holder.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_mds_avd_up_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   return;
}


/*****************************************************************************
 * Function: avd_mds_avd_down_func
 *
 * Purpose:  This function is the handler for the standby AvD down event from
 * mds. This function will issue restart request to HPI to restart the 
 * system controller card on which the standby is running. It then marks the 
 * node director on that card as down and migrates all the Service instances 
 * to other inservice SUs. It works similar to avd_tmr_rcv_hb_d_func.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_mds_avd_down_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   return;
}


/*****************************************************************************
 * Function: avd_standby_tmr_rcv_hb_d_func
 *
 * Purpose:  This function is the handler for receive directors heartbeat 
 * timer event on standby director indicating the expiry of the receive 
 * heartbeat timer related to the active director. This function will 
 * issue restart request to HPI to restart the system controller card on
 * which the active is running. It then changes assumes its HA state as
 * active and informs the same to CPSV. It does all the functionality
 * to become active from standby. It then marks the node director on that
 * card as down and migrates all the Service instances to other inservice SUs.
 * this is similar to avd_standby_avd_down_func.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_standby_tmr_rcv_hb_d_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   m_AVD_LOG_FUNC_ENTRY("avd_standby_tmr_rcv_hb_d_func");
   syslog(LOG_WARNING, "AVD: Heart Beat missed with active director on %x",
          cb->node_id_avd_other);

   m_AVD_LOG_CKPT_EVT(AVD_HB_MISS_WITH_PEER, 
         NCSFL_SEV_NOTICE, cb->node_id_avd_other);

   if (evt->info.tmr.type != AVD_TMR_RCV_HB_D)
   {
      /* log error that a wrong timer type value */
      m_AVD_LOG_INVALID_VAL_FATAL(evt->info.tmr.type);
      return;
   }

  /*Set the first heat beat variable to False */
   cb->avd_hrt_beat_rcvd = FALSE;

   /* Inform AVM About this */
   avd_avm_d_hb_lost_msg(cb, cb->node_id_avd_other);

   return;
}


/*****************************************************************************
 * Function: avd_standby_avd_down_func
 *
 * Purpose:  This function is the handler for the active AvD down event from
 * mds. This function will issue restart request to HPI to restart the 
 * system controller card on which the active is running. It then marks the 
 * node director on that card as down and migrates all the Service instances 
 * to other inservice SUs. It works similar to avd_standby_tmr_rcv_hb_d_func.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_standby_avd_down_func(AVD_CL_CB *cb,AVD_EVT *evt)
{
   return;
}


/*****************************************************************************
 * Function: avd_rcv_hb_msg
 *
 * Purpose:  This function receives D2D HB messages 
 *
 * Input: cb - Pointer to the AVD control block
 *        avnd - Pointer to the AVND structure of the node.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

void avd_rcv_hb_d_msg(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVD_D2D_MSG *d2d_msg = evt->info.avd_msg;
 
   m_AVD_LOG_FUNC_ENTRY("avd_rcv_hb_msg");
   

   /* update our data */
   cb->node_id_avd_other = d2d_msg->msg_info.d2d_hrt_bt.node_id;
   cb->avail_state_avd_other = d2d_msg->msg_info.d2d_hrt_bt.avail_state;
   

   /* So we have received HB message. Time to process it */
   avd_d2d_heartbeat_msg_func(cb);

   /* free this msg */
   avsv_d2d_msg_free(d2d_msg);

   return ;
}
