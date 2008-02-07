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

  DESCRIPTION:  This file contains routines used by SRMA library for MDS 
                interaction.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#include "srma.h"

static MDS_CLIENT_MSG_FORMAT_VER
    gl_srma_wrt_srmnd_msg_fmt_array[SRMA_WRT_SRMND_SUBPART_VER_RANGE]={
           1 /*msg format version for srma subpart version 1*/};

/*****************************************************************************
                     Static Function Declarations 
*****************************************************************************/
static uns32 srma_mds_rcv(SRMA_CB *, MDS_CALLBACK_RECEIVE_INFO *);
static uns32 srma_mds_svc_evt(SRMA_CB *, MDS_CALLBACK_SVC_EVENT_INFO *);
static uns32 srma_mds_flat_enc(SRMA_CB *, MDS_CALLBACK_ENC_FLAT_INFO *);
static uns32 srma_mds_flat_dec(SRMA_CB *, MDS_CALLBACK_DEC_FLAT_INFO *);
static uns32 srma_mds_copy(SRMA_CB *, MDS_CALLBACK_COPY_INFO *);
static uns32 srma_mds_enc(SRMA_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info);
static uns32 srma_mds_dec(SRMA_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info);
static uns32 srma_mds_param_get(SRMA_CB *);
static void  srma_srmnd_mds_down(SRMA_CB *srma, MDS_DEST *srmnd_dest);


/****************************************************************************
  Name          :  srma_mds_reg
 
  Description   :  This routine registers the SRMA service with MDS. It does 
                   the following:
                   a) Gets the MDS handle & SRMA MDS address
                   b) installs SRMA service with MDS
                   c) Subscribes to MDS events
 
  Arguments     :  srma - ptr to the SRMA Control Block
 
  Return Values :  NCSCC_RC_SUCCESS (or)
                   NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_mds_reg (SRMA_CB *srma)
{
   NCSMDS_INFO  mds_info;
   MDS_SVC_ID   svc_id = NCSMDS_SVC_ID_SRMND;
   uns32        rc = NCSCC_RC_SUCCESS;

   /* get the MDS Handle & SRMA MDS Abs Address */
   rc = srma_mds_param_get(srma);
   if (rc != NCSCC_RC_SUCCESS) 
   {      
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_PRM_GET,
                     SRMSV_LOG_MDS_FAILURE,
                     NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   /* fill common fields */
   m_NCS_OS_MEMSET(&mds_info, 0, sizeof(NCSMDS_INFO));
   mds_info.i_mds_hdl = srma->mds_hdl;
   mds_info.i_svc_id  = NCSMDS_SVC_ID_SRMA;

   /*** install srma service with mds ***/
   mds_info.i_op = MDS_INSTALL;
   mds_info.info.svc_install.i_mds_q_ownership = FALSE;
   mds_info.info.svc_install.i_svc_cb = srma_mds_cbk;
   mds_info.info.svc_install.i_yr_svc_hdl = ((MDS_CLIENT_HDL)(long)srma->cb_hdl);
   mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
   mds_info.info.svc_install.i_mds_svc_pvt_ver = srma->srma_mds_ver;

   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS) 
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_REG,
                     SRMSV_LOG_MDS_FAILURE,
                     NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
   else
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_REG,
                     SRMSV_LOG_MDS_SUCCESS,
                     NCSFL_SEV_INFO);
   }
   
   /*** subscribe to srmnd mds event ***/
   mds_info.i_op = MDS_SUBSCRIBE;
   mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
   mds_info.info.svc_subscribe.i_svc_ids = &svc_id;
   mds_info.info.svc_subscribe.i_num_svcs = 1;

   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS) 
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_SUBSCRIBE,
                     SRMSV_LOG_MDS_FAILURE,
                     NCSFL_SEV_ERROR);
      /* Unregister SRMA service from MDS */
      rc = srma_mds_unreg(srma);
   }
   else
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_SUBSCRIBE,
                     SRMSV_LOG_MDS_SUCCESS,
                     NCSFL_SEV_INFO);
   }
   
   return rc;
}


/****************************************************************************
  Name          :  srma_mds_unreg
 
  Description   :  This routine unregisters the SRMA service from MDS.
 
  Arguments     :  srma - ptr to the SRMA Control Block
 
  Return Values :  NCSCC_RC_SUCCESS (or)
                   NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32  srma_mds_unreg(SRMA_CB *srma)
{
   NCSMDS_INFO  mds_info;
   uns32        rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&mds_info, 0, sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl = srma->mds_hdl;
   mds_info.i_svc_id  = NCSMDS_SVC_ID_SRMA;
   mds_info.i_op = MDS_UNINSTALL;

   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_UNREG,
                     SRMSV_LOG_MDS_FAILURE,
                     NCSFL_SEV_ERROR);
   }
   else
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_UNREG,
                     SRMSV_LOG_MDS_SUCCESS,
                     NCSFL_SEV_INFO);
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_mds_enc
 
  Description   :  This routine is invoked to encode SRMA messages.
 
  Arguments     :  cb       - ptr to the SRMA control block
                   enc_info - ptr to the MDS encode info
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  This routine also frees the message after encoding it in 
                   the userbuf.
******************************************************************************/
uns32 srma_mds_enc(SRMA_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{
   SRMA_MSG *msg;
   EDU_ERR  ederror = 0;   
   uns32    rc = NCSCC_RC_SUCCESS;   
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;

   msg_fmt_version = m_NCS_SRMA_ENC_MSG_FMT_GET(
                               enc_info->i_rem_svc_pvt_ver,
                               SRMA_WRT_SRMND_SUBPART_VER_MIN,
                               SRMA_WRT_SRMND_SUBPART_VER_MAX,
                               gl_srma_wrt_srmnd_msg_fmt_array); 
   if (0 == msg_fmt_version)
   {
      m_SRMA_LOG_MDS_VER("MDS Enc Callback","Failure Msg to ",enc_info->i_to_svc_id,\
                        "Remote Service Pvt Version",enc_info->i_rem_svc_pvt_ver);

      return NCSCC_RC_FAILURE; 

   }
   enc_info->o_msg_fmt_ver = msg_fmt_version;

   msg = (SRMA_MSG *)enc_info->i_msg;
   switch (msg->msg_type)
   {
   case SRMA_UNREGISTER_MSG:
   case SRMA_START_MON_MSG:
   case SRMA_STOP_MON_MSG:
   case SRMA_CREATE_RSRC_MON_MSG:
   case SRMA_DEL_RSRC_MON_MSG:
   case SRMA_MODIFY_RSRC_MON_MSG:
   case SRMA_GET_WATERMARK_MSG:
       rc = m_NCS_EDU_EXEC(&cb->edu_hdl,
                           srmsv_edp_agent_msg,
                           enc_info->io_uba, 
                           EDP_OP_TYPE_ENC,
                           msg,
                           &ederror);
       break;

   case SRMA_BULK_CREATE_MON_MSG:
       rc = m_NCS_EDU_EXEC(&cb->edu_hdl,
                           srmsv_edp_agent_enc_bulk_create,
                           enc_info->io_uba, 
                           EDP_OP_TYPE_ENC,
                           msg,
                           &ederror);
       break;

   default:
      m_SRMSV_ASSERT(0);
      break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_mds_dec
 
  Description   :  This routine is invoked to decode SRMND message.
 
  Arguments     :  cb       - ptr to the SRMA control block
                   dec_info - ptr to the MDS decode info
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_mds_dec(SRMA_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
   EDU_ERR ederror = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   if( 0 == m_NCS_SRMA_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                               SRMA_WRT_SRMND_SUBPART_VER_MIN,
                               SRMA_WRT_SRMND_SUBPART_VER_MAX,
                               gl_srma_wrt_srmnd_msg_fmt_array)) 
    {
        m_SRMA_LOG_MDS_VER("Decode Callback","Failure : Msg From",dec_info->i_fr_svc_id,\
                        "Message Format Version",dec_info->i_msg_fmt_ver); 
       return NCSCC_RC_FAILURE;
    } 

   switch (dec_info->i_fr_svc_id)
   {
   case NCSMDS_SVC_ID_SRMND:
       rc = m_NCS_EDU_EXEC(&cb->edu_hdl,
                           srmsv_edp_nd_msg,
                           dec_info->io_uba,
                           EDP_OP_TYPE_DEC,
                           (SRMND_MSG**)&dec_info->o_msg,
                           &ederror);
       if (rc != NCSCC_RC_SUCCESS)
       {
          if (dec_info->o_msg != NULL)
          {
             srmsv_agent_nd_msg_free((SRMND_MSG *)dec_info->o_msg);
             dec_info->o_msg = NULL;
          }
          return rc;
       }
       break;

   default:
      m_SRMSV_ASSERT(0);
      break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srma_mds_copy
 
  Description   :  This routine is invoked to copy SRMA message and send it to
                   the respective SRMND. (will be called only when the SRMSv
                   Agent and ND binds in the same process space.
 
  Arguments     :  cb      - ptr to the SRMA control block
                   cp_info - ptr to the MDS COPY info
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_mds_copy(SRMA_CB *srma, MDS_CALLBACK_COPY_INFO *cp_info)
{
   SRMA_MSG *msg;
   USE (cp_info->i_last);
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version; 
   
   msg = (SRMA_MSG *)cp_info->i_msg;

   msg_fmt_version = m_NCS_SRMA_ENC_MSG_FMT_GET(
                               cp_info->i_rem_svc_pvt_ver,
                               SRMA_WRT_SRMND_SUBPART_VER_MIN,
                               SRMA_WRT_SRMND_SUBPART_VER_MAX,
                               gl_srma_wrt_srmnd_msg_fmt_array); 
   if (0 == msg_fmt_version)
   {
         m_SRMA_LOG_MDS_VER("MDS Copy Callback","Failure Msg to ",cp_info->i_to_svc_id,\
                        "Remote Service Pvt Version",cp_info->i_rem_svc_pvt_ver);
         return NCSCC_RC_FAILURE;
   }

   cp_info->o_msg_fmt_ver = msg_fmt_version;  

   if (msg->msg_type == SRMA_BULK_CREATE_MON_MSG)
   { 
      SRMA_CREATE_RSRC_MON_NODE *bulk_rsrc; 
      SRMA_SRMND_USR_NODE *usr_srmnd = msg->info.bulk_create_mon.srmnd_usr_rsrc;      
      SRMA_RSRC_MON       *rsrc = SRMA_RSRC_MON_NULL;

      /* Get the count of number of resources */
      rsrc = usr_srmnd->start_rsrc_mon;
      while (rsrc) 
      {
         bulk_rsrc = m_MMGR_ALLOC_SRMA_CREATE_RSRC;
         if (!bulk_rsrc)
         {
            /* TBD to clean the cp_info */
            return NCSCC_RC_FAILURE;
         }
         
         /* Update bulk-rsrc data and add it to the respective list */
         bulk_rsrc->rsrc.rsrc_hdl = rsrc->rsrc_mon_hdl;
         bulk_rsrc->rsrc.mon_info.rsrc_info = rsrc->rsrc_info;
         bulk_rsrc->rsrc.mon_info.monitor_data = rsrc->monitor_data;
         bulk_rsrc->next_rsrc_mon = msg->info.bulk_create_mon.bulk_rsrc;
         msg->info.bulk_create_mon.bulk_rsrc = bulk_rsrc;

         rsrc = rsrc->next_srmnd_rsrc;
      }                  
   }

   /* Now create the msg and copy the cp_info */
   msg = m_MMGR_ALLOC_SRMA_MSG;
   if (!msg)
   {  
      /* TBD to clean the cp_info */
      return NCSCC_RC_FAILURE;
   }

   m_NCS_MEMSET(msg, 0, sizeof(SRMA_MSG));

   m_NCS_MEMCPY(msg, cp_info->i_msg, sizeof(SRMA_MSG));
   cp_info->o_cpy = (uns8*)msg;

   return NCSCC_RC_SUCCESS;   
}


/****************************************************************************
  Name          :  srma_mds_cbk
 
  Description   :  This routine is a callback routine that is provided to MDS.
                   MDS calls this routine for encode, decode, copy, receive &
                   service event notification operations.
 
  Arguments     :  info - ptr to the MDS callback info
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_mds_cbk(NCSMDS_CALLBACK_INFO *info)
{
   SRMA_CB *srma = SRMA_CB_NULL;
   uns32   rc = NCSCC_RC_SUCCESS;

   if (!info)
      return rc;

   /* retrieve srma cb */
   if ((srma = (SRMA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMA, 
                               (uns32)info->i_yr_svc_hdl)) == NULL)
   {
      m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                     SRMSV_LOG_HDL_FAILURE,
                     NCSFL_SEV_ERROR);
      return rc;
   }

   m_SRMA_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);

   switch(info->i_op)
   {
   case MDS_CALLBACK_RECEIVE:
      {
         /* acquire cb write lock */
         m_NCS_LOCK(&srma->lock, NCS_LOCK_WRITE);
         
         /* process the received msg */
         rc = srma_mds_rcv(srma, &info->info.receive);
         
         /* release cb write lock */
         m_NCS_UNLOCK(&srma->lock, NCS_LOCK_WRITE);
         
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_RCV_CBK,
                           SRMSV_LOG_MDS_FAILURE,
                           NCSFL_SEV_ERROR);
         }
         else
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_RCV_CBK,
                           SRMSV_LOG_MDS_SUCCESS,
                           NCSFL_SEV_INFO);
         } 
      }
      break;

   case MDS_CALLBACK_COPY:
       srma_mds_copy(srma, &info->info.cpy);
#if 0      
       /* SRMA never resides with SRMND */
       m_SRMSV_ASSERT(0); 
#endif       
       break;

   case MDS_CALLBACK_SVC_EVENT:
      {
         rc = srma_mds_svc_evt(srma, &info->info.svc_evt);         
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_SVEVT_CBK,
                           SRMSV_LOG_MDS_FAILURE,
                           NCSFL_SEV_ERROR);
         }
         else
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_SVEVT_CBK,
                           SRMSV_LOG_MDS_SUCCESS,
                           NCSFL_SEV_INFO);
         }
      }
      break;

   case MDS_CALLBACK_ENC_FLAT:
      {
         rc = srma_mds_flat_enc(srma, &info->info.enc_flat);         
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_FLAT_ENC_CBK,
                           SRMSV_LOG_MDS_FAILURE,
                           NCSFL_SEV_ERROR);
         }
         else
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_FLAT_ENC_CBK,
                           SRMSV_LOG_MDS_SUCCESS,
                           NCSFL_SEV_INFO);
         }
      }
      break;   

   case MDS_CALLBACK_DEC_FLAT:
      {
         rc = srma_mds_flat_dec(srma, &info->info.dec_flat);         
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_FLAT_DEC_CBK,
                           SRMSV_LOG_MDS_FAILURE,
                           NCSFL_SEV_ERROR);
         }
         else
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_FLAT_DEC_CBK,
                           SRMSV_LOG_MDS_SUCCESS,
                           NCSFL_SEV_INFO);
         }
      }
      break;

   case MDS_CALLBACK_DEC:
       rc = srma_mds_dec(srma, &info->info.dec);
       if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_DEC_CBK,
                           SRMSV_LOG_MDS_FAILURE,
                           NCSFL_SEV_ERROR);
         }
         else
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_DEC_CBK,
                           SRMSV_LOG_MDS_SUCCESS,
                           NCSFL_SEV_INFO);
         }
       break;

   case MDS_CALLBACK_ENC:
       rc = srma_mds_enc(srma, &info->info.enc);
       if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_ENC_CBK,
                           SRMSV_LOG_MDS_FAILURE,
                           NCSFL_SEV_ERROR);
         }
         else
         {
            m_SRMA_LOG_MDS(SRMSV_LOG_MDS_ENC_CBK,
                           SRMSV_LOG_MDS_SUCCESS,
                           NCSFL_SEV_INFO);
         }
       break;

   default:
       m_SRMSV_ASSERT(0);
       break;
   }

   /* return srma cb */
   ncshm_give_hdl((uns32)info->i_yr_svc_hdl);

   return rc;
}


/****************************************************************************
  Name          :  srma_mds_send
 
  Description   :  This routine sends the SRMA message to SRMND. The send 
                   operation may be a 'normal' send or a synchronous call that 
                   blocks until the response is received from SRMND.
 
  Arguments     :  srma  - ptr to the SRMA CB
                   i_msg - ptr to the SRMA message
                   o_msg - double ptr to SRMA message response
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/timeout
 
  Notes         :  None.
******************************************************************************/
uns32 srma_mds_send (SRMA_CB *srma,
                     SRMA_MSG *srma_msg,
                     MDS_DEST *srmnd_dest)
{
   NCSMDS_INFO mds_info;
   uns32       rc = NCSCC_RC_SUCCESS;
   MDS_SEND_INFO *send_info = NULL;
   
   m_NCS_OS_MEMSET(&mds_info, 0, sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl = srma->mds_hdl;
   mds_info.i_svc_id = NCSMDS_SVC_ID_SRMA;
   mds_info.i_op = MDS_SEND;
  
   send_info = &mds_info.info.svc_send;

   /* populate the send info */
   send_info->i_msg = (NCSCONTEXT)srma_msg;
   send_info->i_to_svc = NCSMDS_SVC_ID_SRMND;
   send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;

   /* If srmnd_info is NULL, then it is a broadcast send else it is a 
      unicast send */
   if (srmnd_dest)
   {
      send_info->i_sendtype = MDS_SENDTYPE_SND;
      send_info->info.snd.i_to_dest = *srmnd_dest;
   }
   else
   {
      send_info->i_sendtype = MDS_SENDTYPE_BCAST;
      send_info->info.bcast.i_bcast_scope = NCSMDS_SCOPE_NONE;
   }

   /* send the message */
   rc = ncsmds_api(&mds_info);
   if (rc == NCSCC_RC_SUCCESS)
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_SEND,
                     SRMSV_LOG_MDS_SUCCESS,
                     NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMA_LOG_MDS(SRMSV_LOG_MDS_SEND,
                     SRMSV_LOG_MDS_FAILURE,
                     NCSFL_SEV_ERROR);
   }
   return rc;
}


/****************************************************************************
  Name          :  srma_mds_rcv
 
  Description   :  This routine is invoked when SRMA message is received from 
                   SRMND.
 
  Arguments     :  srma - ptr to the SRMA control block
                   rcv_info - ptr to the MDS receive info
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srma_mds_rcv(SRMA_CB *srma, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
   SRMND_MSG *msg = (SRMND_MSG *)rcv_info->i_msg;
   uns32     rc = NCSCC_RC_FAILURE;

   if( 0 == m_NCS_SRMA_MSG_FORMAT_IS_VALID(rcv_info->i_msg_fmt_ver,
                               SRMA_WRT_SRMND_SUBPART_VER_MIN,
                               SRMA_WRT_SRMND_SUBPART_VER_MAX,
                               gl_srma_wrt_srmnd_msg_fmt_array))
   {
       m_SRMA_LOG_MDS_VER("Receive Callback","Failure : Msg From",rcv_info->i_fr_svc_id,\
                        "Message Format Version",rcv_info->i_msg_fmt_ver);  
       return NCSCC_RC_FAILURE;

   }
 
   /* process the message */
   if (!(msg))
      return rc;

   rc = srma_process_srmnd_msg(srma, msg, &rcv_info->i_fr_dest);

   /* free the received message now */
   srma_del_srmnd_msg(msg);

   return rc;
}


/****************************************************************************
  Name          :  srma_srmnd_mds_down
 
  Description   :  This routine updates SRMND specific node with MDS DOWN 
                   state.
 
  Arguments     :  srma - ptr to the SRMA control block
                   evt_info - ptr to the MDS event info
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
void srma_srmnd_mds_down(SRMA_CB *srma, MDS_DEST *srmnd_dest)
{
   SRMA_SRMND_INFO *srmnd;

   if ((srmnd = srma_check_srmnd_exists(srma, m_NCS_NODE_ID_FROM_MDS_DEST(*srmnd_dest))) != NULL)
   {
      m_NCS_OS_MEMSET(&srmnd->srmnd_dest, 0, sizeof(MDS_DEST));
      srmnd->is_srmnd_up = FALSE;

      /* If no rsrc's are configured.. so delete the respective 
         SRMND node from the SRMA database */
      if (srmnd->start_usr_node == NULL)
         srma_del_srmnd_node(srma, srmnd);
   }
   
   return;
}


/****************************************************************************
  Name          : srma_mds_svc_evt
 
  Description   : This routine is invoked to inform SRMA of MDS events. SRMA 
                  had subscribed to these events during MDS registration.
 
  Arguments     : srma       - ptr to the SRMA control block
                  evt_info - ptr to the MDS event info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 srma_mds_svc_evt(SRMA_CB *srma, MDS_CALLBACK_SVC_EVENT_INFO *evt_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* assign mds-dest values for SRMND & SRMA as per the MDS event */
   switch (evt_info->i_change)
   {
   case NCSMDS_UP:
      switch(evt_info->i_svc_id)
      {
      case NCSMDS_SVC_ID_SRMND:
          /* synchronize SRMND specific SRMA resource monitor data with SRMND */
          rc = srma_update_sync_cbk_info(srma, &evt_info->i_dest);   
          break;

      default:
          m_SRMSV_ASSERT(0);
          break;
      }
      break;

   case NCSMDS_DOWN:
      switch(evt_info->i_svc_id)
      {
      case NCSMDS_SVC_ID_SRMND:
          srma_srmnd_mds_down(srma, &evt_info->i_dest);
          break;
         
      default:
          m_SRMSV_ASSERT(0);
          break;
      }
      break;

   default:
      break;
   }
   
   return rc;
}


/****************************************************************************
  Name          :  srma_mds_flat_enc
 
  Description   :  This routine is invoked to encode SRMND messages.
 
  Arguments     :  srma       - ptr to the SRMA control block
                   enc_info - ptr to the MDS encode info
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  This routine also frees the message after encoding it in 
                   the userbuf.
******************************************************************************/
uns32 srma_mds_flat_enc (SRMA_CB *srma, MDS_CALLBACK_ENC_FLAT_INFO *enc_info)
{
   SRMA_MSG *msg = (SRMA_MSG*)enc_info->i_msg;
   uns32     rc = NCSCC_RC_SUCCESS;   
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;


   msg_fmt_version = m_NCS_SRMA_ENC_MSG_FMT_GET(
                               enc_info->i_rem_svc_pvt_ver,
                               SRMA_WRT_SRMND_SUBPART_VER_MIN,
                               SRMA_WRT_SRMND_SUBPART_VER_MAX,
                               gl_srma_wrt_srmnd_msg_fmt_array);
   if (0 == msg_fmt_version)
   {
      m_SRMA_LOG_MDS_VER("MDS Enc Flat Call Back","Failure Msg to ",enc_info->i_to_svc_id,\
                        "Remote Service Pvt Version",enc_info->i_rem_svc_pvt_ver); 

      return NCSCC_RC_FAILURE;

   }

   enc_info->o_msg_fmt_ver = msg_fmt_version;

   /* encode into userbuf */
   if ((rc = ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                       (uns8 *)msg,
                                       sizeof(SRMA_MSG))) != NCSCC_RC_SUCCESS)
      return rc;
 
   if (msg->msg_type == SRMA_BULK_CREATE_MON_MSG)
   { 
      SRMA_SRMND_USR_NODE *usr_srmnd = msg->info.bulk_create_mon.srmnd_usr_rsrc;      
      SRMA_RSRC_MON       *rsrc = SRMA_RSRC_MON_NULL;
      uns32               rsrc_num = 0;
      
      /* Get the rsrc's number that to be updated in UBUF */
      rsrc = usr_srmnd->start_rsrc_mon;
      while (rsrc) 
      {
         rsrc_num++;
         rsrc = rsrc->next_srmnd_rsrc;
      }                

      /* Update the number of resources */
      ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                 (uns8 *)&rsrc_num,
                                 sizeof(uns32));
     
      /* Update all the resource mon data to userbuf */
      rsrc = usr_srmnd->start_rsrc_mon;
      while (rsrc) 
      {
          /* Update the rsrc hdl information */
          ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                    (uns8 *)&rsrc->rsrc_mon_hdl,
                                    sizeof(uns32));

          /* Update the rsrc info data */
          ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                     (uns8 *)&rsrc->rsrc_info,
                                     sizeof(NCS_SRMSV_RSRC_INFO));

          /* Update the monitor data */
          ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                     (uns8 *)&rsrc->monitor_data,
                                     sizeof(NCS_SRMSV_MON_DATA));

          rsrc = rsrc->next_srmnd_rsrc;
      }
   }
   
   return rc;
}


/****************************************************************************
  Name          :  srma_mds_flat_dec
 
  Description   :  This routine is invoked to decode SRMND messages.
 
  Arguments     :  srma     - ptr to the SRMA control block
                   rcv_info - ptr to the MDS decode info
  
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32  srma_mds_flat_dec(SRMA_CB *srma, MDS_CALLBACK_DEC_FLAT_INFO *dec_info)
{
   uns32     rc = NCSCC_RC_SUCCESS;
   SRMND_MSG *msg;

   if( 0 == m_NCS_SRMA_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                               SRMA_WRT_SRMND_SUBPART_VER_MIN,
                               SRMA_WRT_SRMND_SUBPART_VER_MAX,
                               gl_srma_wrt_srmnd_msg_fmt_array)) 
     { 
        m_SRMA_LOG_MDS_VER("Decode Flat Callback","Failure : Msg From",dec_info->i_fr_svc_id,\
                        "Message Format Version",dec_info->i_msg_fmt_ver);

       return NCSCC_RC_FAILURE;

     }
  
   if ((msg = m_MMGR_ALLOC_SRMND_MSG) == NULL)
   {
      m_SRMA_LOG_SRMSV_MEM(SRMSV_MEM_SRMND_MSG,
                           SRMSV_MEM_ALLOC_FAILED,
                           NCSFL_SEV_CRITICAL);

      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(msg, 0, sizeof(SRMND_MSG));
   
   /* decode the top level SRMND msg contents */
   if ((rc = ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                          (uns8 *)msg,
                                          sizeof(SRMND_MSG))) != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_SRMND_MSG(msg);
      return rc;
   }

   if (msg->msg_type == SRMND_BULK_CREATED_MON_MSG)
   {
      SRMND_CREATED_RSRC_MON *bulk_rsrc;
      uns32                  rsrc_num = 0;

      /* Get the number of resource monitor data from the userbuf */
      ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                   (uns8 *)&rsrc_num,
                                    sizeof(uns32));

       while (rsrc_num)
       {
          if ((bulk_rsrc = m_MMGR_ALLOC_SRMND_CREATED_RSRC) == NULL)
          {
             m_SRMA_LOG_SRMSV_MEM(SRMSV_MEM_SRMND_CRTD_RSRC,
                                  SRMSV_MEM_ALLOC_FAILED,
                                  NCSFL_SEV_CRITICAL);

             /* Free off the allocated buffer */ 
             srma_del_srmnd_msg(msg);

             return NCSCC_RC_FAILURE;
          }

          m_NCS_OS_MEMSET(bulk_rsrc, 0, sizeof(SRMND_CREATED_RSRC_MON));

          ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                   (uns8 *)&bulk_rsrc->srma_rsrc_hdl,
                                    sizeof(uns32));

          ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                   (uns8 *)&bulk_rsrc->srmnd_rsrc_hdl,
                                    sizeof(uns32));

          /* Adding the node?? so check whether bulk rsrs mon list is empty ?? */
          if (msg->info.bulk_rsrc_mon.bulk_rsrc_mon == NULL)
          {
             /* Oh! yes it is empty.. so let me add as first node of the list */
             msg->info.bulk_rsrc_mon.bulk_rsrc_mon = bulk_rsrc;
          }
          else
          {
             /* Oh! no it is not empty.. then also let me add as a first node
                of the list */
             bulk_rsrc->next_srmnd_rsrc_mon = msg->info.bulk_rsrc_mon.bulk_rsrc_mon;
             msg->info.bulk_rsrc_mon.bulk_rsrc_mon = bulk_rsrc;
          }

          rsrc_num--;
      }      
   }

   /* decode successful */
   dec_info->o_msg = (NCSCONTEXT)msg;

   return rc;
}


/****************************************************************************
  Name          :  srma_mds_param_get
 
  Description   :  This routine gets the MDS handle & SRMA MDS address.
 
  Arguments     :  srma - ptr to the SRMA Control Block
 
  Return Values :  NCSCC_RC_SUCCESS (or)
                   NCSCC_RC_FAILURE
 
  Notes         :  None.
*****************************************************************************/
static uns32  srma_mds_param_get(SRMA_CB *srma)
{
   NCSADA_INFO ada_info;
   uns32       rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&ada_info, 0, sizeof(ada_info));

   ada_info.req = NCSADA_GET_HDLS;
   ada_info.info.adest_get_hdls.i_create_oac = FALSE;

   /* Invoke Absolute Destination Address (ADA) request */
   rc = ncsada_api(&ada_info);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;
   
   /* Store values returned by ADA */
   srma->mds_hdl   = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
   srma->srma_dest = ada_info.info.adest_get_hdls.o_adest;

   return rc;
}
