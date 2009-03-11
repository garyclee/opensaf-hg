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

  DESCRIPTION:This module does the encode/decode,copy,free,transmission
  and reception  of messages between the Availability Director and
  Availability Node Director.



..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_mds_enc - encodes AVD to AvND messages.
  avd_mds_enc_flat  - encodes flat AVD to AvND messages. Dummy not required.
  avd_mds_cpy - copies AVD to AvND messages.
  avd_mds_dec - decodes AvND to AVD messages.
  avd_mds_dec_flat - decodes flat AvND to AVD messages. Dummy not required.
  avd_d2n_msg_snd - transmits message to node director.
  avd_d2n_msg_bcast - broadcasts message to all node director.
  avd_n2d_msg_rcv - Procresses messages from AvND.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"



/****************************************************************************
  Name          : avd_mds_enc
 
  Description   : This is a callback routine that is invoked to encode
                  AvND-AvD messages.
 
  Arguments     : cb_hdl    - AvD control block Handle.
                  enc_info  - ptr to the MDS encode info.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : 
******************************************************************************/
uns32 avd_mds_enc(uns32 cb_hdl,MDS_CALLBACK_ENC_INFO *enc_info)
{
   AVD_CL_CB       *cb;
   EDU_ERR         ederror = 0;
   uns32           rc = NCSCC_RC_SUCCESS;

   m_AVD_LOG_FUNC_ENTRY("avd_mds_enc");
   
   /* get the CB from the handle manager */
   if ((cb = ncshm_take_hdl(NCS_SERVICE_ID_AVD,cb_hdl)) ==
     AVD_CL_CB_NULL)
   {
      /* log the problem */
      m_AVD_LOG_INVALID_VAL_FATAL(cb_hdl);
      return NCSCC_RC_FAILURE;
   }
   
   rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_dnd_msg, enc_info->io_uba, 
                          EDP_OP_TYPE_ENC, enc_info->i_msg, &ederror, enc_info->o_msg_fmt_ver);
   
   /* give back the handle */
   ncshm_give_hdl(cb_hdl);

   if( rc != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return rc;
   }

   m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_ENC_CBK);
   return rc;
}

/****************************************************************************
  Name          : avd_mds_enc_flat
 
  Description   : This is a callback routine that is invoked to encode
                  AvND-AvD messages in a flat buffer.
 
  Arguments     : cb_hdl    - AvD control block Handle.
                  enc_info  - ptr to the MDS encode info.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : 
******************************************************************************/
uns32 avd_mds_enc_flat(uns32 cb_hdl,MDS_CALLBACK_ENC_FLAT_INFO *enc_info)
{
   return avd_mds_enc(cb_hdl,(MDS_CALLBACK_ENC_INFO *)enc_info);
}


/****************************************************************************
  Name          : avd_mds_cpy
 
  Description   : This is a callback routine that is invoked to copy 
                  AvND-AvD messages. 
 
  Arguments     : cpy_info - ptr to the MDS copy info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info)
{
   AVD_DND_MSG *dst_msg;

   m_AVD_LOG_FUNC_ENTRY("avd_mds_cpy");
   
   if (cpy_info->i_msg == NULL)
   {
      /* log the problem */
      m_AVD_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;
   }

   dst_msg = m_MMGR_ALLOC_AVSV_DND_MSG;
   if (dst_msg == AVD_DND_MSG_NULL)
   {
      /* log error that the director is in degraded situation */
      m_AVD_LOG_MEM_FAIL_LOC(AVD_DND_MSG_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }
   
   m_AVD_LOG_RCVD_VAL(((long)dst_msg));
 
   if (NCSCC_RC_SUCCESS != 
      avsv_dnd_msg_copy(dst_msg, (AVSV_DND_MSG *)cpy_info->i_msg))
   {
      /* log error that the director is in degraded situation */
      m_AVD_LOG_MEM_FAIL_LOC(AVD_DND_MSG_ALLOC_FAILED);
      m_MMGR_FREE_AVSV_DND_MSG(dst_msg);
      return NCSCC_RC_FAILURE;
   }
   cpy_info->o_cpy = (NCSCONTEXT)dst_msg;
   m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_CPY_CBK);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : avd_mds_dec
 
  Description   : This is a callback routine that is invoked to decode
                  AvD-AvND messages.
 
  Arguments     : cb_hdl    - AvD control block Handle.
                  dec_info  - ptr to the MDS decode info.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_mds_dec (uns32 cb_hdl,MDS_CALLBACK_DEC_INFO *dec_info)
{
   AVD_CL_CB       *cb;
   EDU_ERR         ederror = 0;
   uns32           rc = NCSCC_RC_SUCCESS;

   m_AVD_LOG_FUNC_ENTRY("avd_mds_dec");
   
   /* get the CB from the handle manager */
   if ((cb = ncshm_take_hdl(NCS_SERVICE_ID_AVD,cb_hdl)) ==
     AVD_CL_CB_NULL)
   {
      /* log the problem */
      m_AVD_LOG_INVALID_VAL_FATAL(cb_hdl);
      return NCSCC_RC_FAILURE;
   }

   rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_dnd_msg, dec_info->io_uba,
                    EDP_OP_TYPE_DEC, (AVSV_DND_MSG**)&dec_info->o_msg, &ederror, dec_info->i_msg_fmt_ver);

   
   /* give back the handle */
   ncshm_give_hdl(cb_hdl);

   if( rc != NCSCC_RC_SUCCESS )
   {
      /* decode failed!!! */
      if(dec_info->o_msg != NULL)
      {
          avsv_dnd_msg_free(dec_info->o_msg);
          dec_info->o_msg = NULL;
      }

      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return rc;
   }

   m_AVD_LOG_RCVD_VAL(((long)dec_info->o_msg));
   m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_DEC_CBK);
   return rc;

}

/****************************************************************************
  Name          : avd_mds_dec_flat
 
  Description   : This is a callback routine that is invoked to decode
                  AvD-AvND messages from flat buffers.
 
  Arguments     : cb_hdl    - AvD control block Handle.
                  dec_info  - ptr to the MDS decode info.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_mds_dec_flat(uns32 cb_hdl,MDS_CALLBACK_DEC_FLAT_INFO *dec_info)
{
   return avd_mds_dec (cb_hdl,(MDS_CALLBACK_DEC_INFO *)dec_info);
}

/****************************************************************************
  Name          : avd_d2n_msg_enqueue

  Description   : This is a routine used for enqueuing messages which are being
                  sent to the node director.
 
  Arguments     : cb     :  The control block of AvD
                  snd_msg: the send message that needs to be sent.
                  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_d2n_msg_enqueue(AVD_CL_CB *cb, NCSMDS_INFO *snd_mds)
{
   AVSV_ND_MSG_QUEUE   *nd_msg;
   /*
    * Allocate the message and put it in the queue.
    * We will have to send it at later point of time.
    */
   if (NULL == (nd_msg = m_MMGR_ALLOC_D2N_MSG))
   {
      /* Log error */
      return NCSCC_RC_FAILURE;
   }

   memset(nd_msg, '\0', sizeof(AVSV_ND_MSG_QUEUE));

   memcpy(&nd_msg->snd_msg, snd_mds, sizeof(NCSMDS_INFO));

   m_AVD_DTOND_MSG_PUSH(cb, nd_msg);

   return NCSCC_RC_SUCCESS;

}


/****************************************************************************
  Name          : avd_d2n_msg_dequeue

  Description   : This is a routine used for dequeuing messages which are to be
                  sent to the node director.
 
  Arguments     : cb     :  The control block of AvD

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_d2n_msg_dequeue(AVD_CL_CB *cb)
{
   AVSV_ND_MSG_QUEUE   *queue_elem;
   uns32   rc = NCSCC_RC_SUCCESS;
   /*
    * De-queue messages from the Queue and then do the MDS send.
    */

   m_AVD_DTOND_MSG_POP(cb, queue_elem); 

   while (queue_elem != NULL) 
   {
     /*
      * Now do MDS send.
      */
      if((rc = ncsmds_api(&queue_elem->snd_msg)) != NCSCC_RC_SUCCESS)
      {
         m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SEND);
      }
      else
      {
         m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_SEND);
      }

      m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,
         queue_elem->snd_msg.info.svc_send.i_msg,
         sizeof(AVD_DND_MSG),queue_elem->snd_msg.info.svc_send.i_msg);

      avsv_dnd_msg_free((AVD_DND_MSG*)queue_elem->snd_msg.info.svc_send.i_msg);

      m_MMGR_FREE_D2N_MSG(queue_elem);
      
      m_AVD_DTOND_MSG_POP(cb, queue_elem);
   }

   return rc;
}


/****************************************************************************
  Name          : avd_d2n_msg_snd

  Description   : This is a routine invoked by AVD for transmitting
  messages to node director.
 
  Arguments     : cb     :  The control block of AvD
                  nd_node:  Node director node to which this message
                            will be sent.

                  snd_msg: the send message that needs to be sent.
                  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uns32 avd_d2n_msg_snd(AVD_CL_CB *cb, AVD_AVND *nd_node,AVD_DND_MSG *snd_msg)
{
   NCSMDS_INFO snd_mds;
   uns32 rc;

   m_AVD_LOG_FUNC_ENTRY("avd_d2n_msg_snd");
   m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,snd_msg,sizeof(AVD_DND_MSG),snd_msg);
   
   memset(&snd_mds,'\0',sizeof(NCSMDS_INFO));

   snd_mds.i_mds_hdl = cb->adest_hdl;
   snd_mds.i_svc_id = NCSMDS_SVC_ID_AVD;
   snd_mds.i_op = MDS_SEND;
   snd_mds.info.svc_send.i_msg = (NCSCONTEXT)snd_msg;
   snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_AVND;
   snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
   snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
   snd_mds.info.svc_send.info.snd.i_to_dest = nd_node->adest;

   rc = avd_d2n_msg_enqueue(cb, &snd_mds);

   return rc;
}



/****************************************************************************
  Name          : avd_d2n_msg_bcast

  Description   : This is a routine invoked by AVD for broadcasting
  messages to node directors.
 
  Arguments     : cb     :  The control block of AvD
                  bcast_msg: the broadcast message that needs to be sent.
                  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uns32 avd_d2n_msg_bcast(AVD_CL_CB *cb, AVD_DND_MSG *bcast_msg)
{
   NCSMDS_INFO snd_mds;
   uns32 rc;

   m_AVD_LOG_FUNC_ENTRY("avd_d2n_msg_bcast");
   m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG,bcast_msg,sizeof(AVD_DND_MSG),bcast_msg);
   
   memset(&snd_mds,'\0',sizeof(NCSMDS_INFO));

   snd_mds.i_mds_hdl = cb->adest_hdl;
   snd_mds.i_svc_id = NCSMDS_SVC_ID_AVD;
   snd_mds.i_op = MDS_SEND;
   snd_mds.info.svc_send.i_msg = (NCSCONTEXT)bcast_msg;
   snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_AVND;
   snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
   snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_BCAST;
   snd_mds.info.svc_send.info.bcast.i_bcast_scope = NCSMDS_SCOPE_NONE;

   if((rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SEND);
      return rc;
   }

   m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_SEND);
   return NCSCC_RC_SUCCESS;
}




/****************************************************************************
  Name          : avd_n2d_msg_rcv
 
  Description   : This routine is invoked by AvD when a message arrives
  from AvND. It converts the message to the corresponding event and posts
  the message to the mailbox for processing by the main loop.
 
  Arguments     : cb_hdl     -  AvD cb Handle.
                  rcv_msg    -  ptr to the received message
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uns32 avd_n2d_msg_rcv(uns32 cb_hdl, AVD_DND_MSG *rcv_msg, NODE_ID node_id, uns16 msg_fmt_ver)
{
   AVD_EVT *evt=AVD_EVT_NULL;
   AVD_CL_CB   *cb=AVD_CL_CB_NULL;

   m_AVD_LOG_FUNC_ENTRY("avd_n2d_msg_rcv");
   
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
      avsv_dnd_msg_free(rcv_msg);
      return NCSCC_RC_FAILURE;
   }
   
   m_AVD_LOG_RCVD_VAL(((long)evt));

   /* get the CB from the handle manager */
   if ((cb = (AVD_CL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVD,cb_hdl)) ==
        AVD_CL_CB_NULL)
   {
      /* log error */
      m_AVD_LOG_INVALID_VAL_FATAL(cb_hdl);
      m_MMGR_FREE_AVD_EVT(evt);
      /* free the message and return */
      avsv_dnd_msg_free(rcv_msg);
      return NCSCC_RC_FAILURE;
   }

   if(node_id == cb->node_id_avd_other)   
   {
     /* We need to maintain version information of peer AvND. We shouldn't
        send role change message to older version Controller AvND.*/
     cb->peer_msg_fmt_ver = msg_fmt_ver;
   }
   evt->cb_hdl = cb_hdl;
   evt->rcv_evt = (rcv_msg->msg_type - AVSV_N2D_CLM_NODE_UP_MSG) 
                  + AVD_EVT_NODE_UP_MSG;
   evt->info.avnd_msg = rcv_msg;

   m_AVD_LOG_EVT_INFO(AVD_SND_AVND_MSG_EVENT,evt->rcv_evt);

   if(evt->rcv_evt == AVD_EVT_HEARTBEAT_MSG)
   {
      if (m_NCS_IPC_SEND(&cb->avd_hb_mbx,evt,NCS_IPC_PRIORITY_VERY_HIGH) 
            != NCSCC_RC_SUCCESS)
      {
         m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
          /* return AvD CB handle*/
         ncshm_give_hdl(cb_hdl);
         /* log error */
         /* free the message */
         avsv_dnd_msg_free(rcv_msg);
         evt->info.avnd_msg = NULL;
         /* free the event and return */
         m_MMGR_FREE_AVD_EVT(evt);
         
         return NCSCC_RC_FAILURE;
      }

   }/* evt->rcv_evt == AVD_EVT_HEARTBEAT_MS*/
   else if (m_NCS_IPC_SEND(&cb->avd_mbx,evt,NCS_IPC_PRIORITY_HIGH) 
            != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
       /* return AvD CB handle*/
      ncshm_give_hdl(cb_hdl);
      /* log error */
      /* free the message */
      avsv_dnd_msg_free(rcv_msg);
      evt->info.avnd_msg = NULL;
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
