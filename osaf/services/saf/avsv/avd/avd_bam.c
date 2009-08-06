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

  This module deals with the AvD interaction with the BAM. The messages sent
  and received and synchronising with BAM module.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_bam_msg_rcv -- Function to process the MDS message from BAM.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"
#include "ncs_avsv_bam_msg.h"

void avsv_bam_msg_free(AVD_BAM_MSG *rcv_msg)
{
   /* Just free the memory */
   m_MMGR_FREE_AVD_BAM_MSG(rcv_msg);
}
/****************************************************************************
 *  Name          : avd_bam_msg_rcv
 * 
 *  Description   : This routine is invoked by AvD when a message arrives
 *  from BAM. It converts the message to the corresponding event and posts
 *  the message to the mailbox for processing by the main loop.
 * 
 *  Arguments     : cb_hdl     -  AvD cb Handle.
 *                  rcv_msg    -  ptr to the received message
 * 
 *  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * 
 *  Notes         : None.
 ***************************************************************************/

uns32 avd_bam_msg_rcv(uns32 cb_hdl, AVD_BAM_MSG *rcv_msg)
{
   AVD_EVT *evt=AVD_EVT_NULL;
   AVD_CL_CB   *cb=AVD_CL_CB_NULL;

   /* check that the message ptr is not NULL */
   if (rcv_msg == NULL)
   {
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return NCSCC_RC_FAILURE;
   }

   /* create the message event */
   evt = m_MMGR_ALLOC_AVD_EVT;
   if (evt == AVD_EVT_NULL)
   {
      /* log error */
      m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
      /* free the message and return */
      avsv_bam_msg_free(rcv_msg);
      return NCSCC_RC_FAILURE;
   }

   /* get the CB from the handle manager */
   if ((cb = (AVD_CL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVD,cb_hdl)) ==
        AVD_CL_CB_NULL)
   {
      /* log error */
      m_MMGR_FREE_AVD_EVT(evt);
      /* free the message and return */
      avsv_bam_msg_free(rcv_msg);
      return NCSCC_RC_FAILURE;
   }
   
   evt->cb_hdl = cb_hdl;
   evt->rcv_evt = (rcv_msg->msg_type + AVD_EVT_MDS_MAX );
   evt->info.bam_msg = rcv_msg;

   /*m_AVD_LOG_EVT_INFO(AVD_SND_BAM_MSG_EVENT,evt->rcv_evt);*/

   if (m_NCS_IPC_SEND(&cb->avd_mbx,evt,NCS_IPC_PRIORITY_HIGH) 
            != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
       /* return AvD CB handle*/
      ncshm_give_hdl(cb_hdl);
      /* log error */
      /* free the message */
      avsv_bam_msg_free(rcv_msg);
      evt->info.bam_msg = NULL;
      /* free the event and return */
      m_MMGR_FREE_AVD_EVT(evt);
      
      return NCSCC_RC_FAILURE;
   }

   m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);

   /* return AvD CB handle*/
   ncshm_give_hdl(cb_hdl);

   m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_RCV_CBK);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 *  Name          : avd_bam_cfg_done_func
 * 
 *  Description   : This routine is invoked by AvD when a message arrives
 *  from BAM. It converts the message to the corresponding event and posts
 *  the message to the mailbox for processing by the main loop.
 * 
 *  Arguments     : cb         -  AvD cb .
 *                  evt        -  ptr to the received event
 * 
 *  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * 
 *  Notes         : None.
 ***************************************************************************/

void avd_bam_cfg_done_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   AVD_BAM_MSG *bam_msg;

   if (evt->info.bam_msg == NULL)
   {
      /* log error that a message contents is missing */
      m_AVD_LOG_INVALID_VAL_ERROR(0);
      return;
   }

   bam_msg = evt->info.bam_msg;

   /* Do the sanity checks to see if the message is received in
   ** valid AVD init state
   */
   if( cb->init_state != AVD_CFG_READY)
   {
      /* log error that the node is in invalid state */
      m_AVD_LOG_INVALID_VAL_ERROR(cb->init_state);
      avsv_bam_msg_free(bam_msg);
      evt->info.bam_msg = NULL;
      return;
   }

   /* Perform the checksum validation */
   if( bam_msg->msg_info.msg.check_sum == cb->num_cfg_msgs )
   {
      /* stop the cfg_tmr */
      avd_stop_tmr(cb, &cb->init_phase_tmr.cfg_tmr);
      cb->init_state = AVD_CFG_DONE;
      avsv_bam_msg_free(bam_msg);
      evt->info.bam_msg = NULL;
   }
   else
   {
      m_AVD_LOG_INVALID_VAL_ERROR(bam_msg->msg_info.msg.check_sum);
      m_AVD_LOG_INVALID_VAL_ERROR(cb->num_cfg_msgs);

      /* stop the cfg_tmr */
      avd_stop_tmr(cb, &cb->init_phase_tmr.cfg_tmr);
      avsv_bam_msg_free(bam_msg);
      evt->info.bam_msg = NULL;      
      avd_post_restart_evt(cb);
   }
   return;
}

void avd_post_restart_evt(AVD_CL_CB *cb)
{
   AVD_EVT *new_evt = AVD_EVT_NULL;
   
   /* now post the restart event to the mail-box */
   new_evt = m_MMGR_ALLOC_AVD_EVT;
   if(new_evt == AVD_EVT_NULL)
   {
      /* we are in trouble no memory */
      m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
      return;
   }
   
   new_evt->cb_hdl = cb->cb_handle; 
   new_evt->rcv_evt = AVD_EVT_RESTART;

   /*m_AVD_LOG_EVT_INFO(AVD_EVT_RESTART, AVD_EVT_RESTART);*/

   if (m_NCS_IPC_SEND(&cb->avd_mbx,new_evt,NCS_IPC_PRIORITY_LOW) 
            != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
      m_MMGR_FREE_AVD_EVT(new_evt);
      return ;
   }

   m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);
   
   return;
}
void avd_tmr_cfg_exp_func(AVD_CL_CB *cb, AVD_EVT *evt)
{
   /* Do HPI restart */
   return;
}
