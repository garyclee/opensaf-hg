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

  MODULE NAME: SRMND_MDS.C
 
..............................................................................

  DESCRIPTION: This file contains SRMND specific MDS routines.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/
#include "srmnd.h"

static MDS_CLIENT_MSG_FORMAT_VER
    gl_srmnd_wrt_srma_msg_fmt_array[SRMND_WRT_SRMA_SUBPART_VER_RANGE]={
           1 /*msg format version for srmnd subpart version 1*/};

/*****************************************************************************
                     Static Function Declarations 
*****************************************************************************/
static uns32 srmnd_mds_rcv(SRMND_CB *, MDS_CALLBACK_RECEIVE_INFO *);
static uns32 srmnd_mds_svc_evt(SRMND_CB *, MDS_CALLBACK_SVC_EVENT_INFO *);
static uns32 srmnd_mds_flat_enc(SRMND_CB *, MDS_CALLBACK_ENC_FLAT_INFO *);
static uns32 srmnd_mds_flat_dec(SRMND_CB *, MDS_CALLBACK_DEC_FLAT_INFO *);
static uns32 srmnd_mds_enc(SRMND_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info);
static uns32 srmnd_mds_dec(SRMND_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info);
static uns32 srmnd_mds_param_get(SRMND_CB *);
static uns32 srmnd_mds_copy(SRMND_CB *srmnd, MDS_CALLBACK_COPY_INFO *cp_info);


/****************************************************************************
  Name          :  srmnd_mds_reg
 
  Description   :  This routine registers the SRMND service with MDS. It does 
                   the following:
                   a) Gets the MDS handle & SRMND MDS address
                   b) installs SRMND service with MDS
                   c) Subscribes to MDS events
 
  Arguments     :  srmnd - ptr to the SRMND Control Block
 
  Return Values :  NCSCC_RC_SUCCESS (or)
                   NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_mds_reg (SRMND_CB *srmnd)
{
   NCSMDS_INFO  mds_info;
   MDS_SVC_ID   svc_id = NCSMDS_SVC_ID_SRMA;
   uns32        rc = NCSCC_RC_SUCCESS;

   /* get the MDS Handle & SRMND MDS Abs Address */
   rc = srmnd_mds_param_get(srmnd);
   if (rc != NCSCC_RC_SUCCESS) 
   {
      m_SRMND_LOG_MDS(SRMSV_LOG_MDS_PRM_GET,
                      SRMSV_LOG_MDS_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return rc;
   }

   /* fill common fields */
   memset(&mds_info, 0, sizeof(NCSMDS_INFO));
   mds_info.i_mds_hdl = srmnd->mds_hdl;
   mds_info.i_svc_id  = NCSMDS_SVC_ID_SRMND;

   /*** install srmnd service with mds ***/
   mds_info.i_op = MDS_INSTALL;
   mds_info.info.svc_install.i_mds_q_ownership = FALSE;
   mds_info.info.svc_install.i_svc_cb = srmnd_mds_cbk;
   mds_info.info.svc_install.i_yr_svc_hdl = ((MDS_CLIENT_HDL)(long)srmnd->cb_hdl);
   mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
   mds_info.info.svc_install.i_mds_svc_pvt_ver = srmnd->srmnd_mds_ver;

   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS) 
   {
      m_SRMND_LOG_MDS(SRMSV_LOG_MDS_INSTALL,
                      SRMSV_LOG_MDS_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return rc;
   }
   
   /*** subscribe to srmnd mds event ***/
   mds_info.i_op = MDS_SUBSCRIBE;
   mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
   mds_info.info.svc_subscribe.i_svc_ids = &svc_id;
   mds_info.info.svc_subscribe.i_num_svcs = 1;

   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS) 
   {
      m_SRMND_LOG_MDS(SRMSV_LOG_MDS_SUBSCRIBE,
                      SRMSV_LOG_MDS_FAILURE,
                      NCSFL_SEV_CRITICAL);
      /* Unregister SRMND service from MDS */
      srmnd_mds_unreg(srmnd);
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_unreg
 
  Description   :  This routine unregisters the SRMND service from MDS.
 
  Arguments     :  srmnd - ptr to the SRMND Control Block
 
  Return Values :  NCSCC_RC_SUCCESS (or)
                   NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32  srmnd_mds_unreg(SRMND_CB *srmnd)
{
   NCSMDS_INFO  mds_info;
   uns32        rc = NCSCC_RC_SUCCESS;

   memset(&mds_info, 0, sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl = srmnd->mds_hdl;
   mds_info.i_svc_id  = NCSMDS_SVC_ID_SRMND;
   mds_info.i_op = MDS_UNINSTALL;

   rc = ncsmds_api(&mds_info);

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_enc
 
  Description   :  This routine is invoked to encode SRMND messages.
 
  Arguments     :  cb       - ptr to the SRMND control block
                   enc_info - ptr to the MDS encode info
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  This routine also frees the message after encoding it in 
                   the userbuf.
******************************************************************************/
uns32 srmnd_mds_enc(SRMND_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{
   SRMND_MSG *msg;
   EDU_ERR   ederror = 0;   
   uns32     rc = NCSCC_RC_SUCCESS;
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;


   msg_fmt_version = m_NCS_SRMND_ENC_MSG_FMT_GET(
                               enc_info->i_rem_svc_pvt_ver,
                               SRMND_WRT_SRMA_SUBPART_VER_MIN,
                               SRMND_WRT_SRMA_SUBPART_VER_MAX,
                               gl_srmnd_wrt_srma_msg_fmt_array);
   if (0 == msg_fmt_version)
   {
        m_SRMND_LOG_MDS_VER("MDS Enc Callback","Failure Msg to ",enc_info->i_to_svc_id,\
                        "Remote Service Pvt Version",enc_info->i_rem_svc_pvt_ver);

         return NCSCC_RC_FAILURE;

   }
   enc_info->o_msg_fmt_ver = msg_fmt_version;

   msg = (SRMND_MSG *)enc_info->i_msg;
   switch (msg->msg_type)
   {
   case SRMND_CREATED_MON_MSG:
   case SRMND_APPL_NOTIF_MSG:
   case SRMND_SYNCHRONIZED_MSG:
   case SRMND_WATERMARK_VAL_MSG:
   case SRMND_WATERMARK_EXIST_MSG:
   case SRMND_PROC_EXIT_MSG:
       rc = m_NCS_EDU_EXEC(&cb->edu_hdl,
                           srmsv_edp_nd_msg,
                           enc_info->io_uba, 
                           EDP_OP_TYPE_ENC,
                           msg,
                           &ederror);
       break;


   /* SRMND acknowledges SRMND_BULK_CREATED_MON_MSG event
     for the received SRMA_BULK_CREATE_MON_MSG from SRMA. 

      SRMA_BULK_CREATE_MON_MSG msg  was no longer exists
      in SRMA so that 
      this flow is not supposed to execute in SRMND */   
   default:
      assert(0);
      break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_dec
 
  Description   :  This routine is invoked to decode SRMA message.
 
  Arguments     :  cb       - ptr to the SRMND control block
                   dec_info - ptr to the MDS decode info
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_mds_dec(SRMND_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
   EDU_ERR ederror = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   if( 0 == m_NCS_SRMND_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                               SRMND_WRT_SRMA_SUBPART_VER_MIN,
                               SRMND_WRT_SRMA_SUBPART_VER_MAX,
                               gl_srmnd_wrt_srma_msg_fmt_array))
    {
         m_SRMND_LOG_MDS_VER("Decode Callback","Failure : Msg From",dec_info->i_fr_svc_id,\
                        "Message Format Version",dec_info->i_msg_fmt_ver);

         return NCSCC_RC_FAILURE;

    }

   /* start decoding the message that has come from SRMSv agent */
   switch (dec_info->i_fr_svc_id)
   {
   case NCSMDS_SVC_ID_SRMA:
       rc = m_NCS_EDU_EXEC(&cb->edu_hdl,
                           srmsv_edp_agent_msg,
                           dec_info->io_uba,
                           EDP_OP_TYPE_DEC,
                           (SRMA_MSG**)&dec_info->o_msg,
                           &ederror);
       if (rc != NCSCC_RC_SUCCESS)
       {
          if (dec_info->o_msg != NULL)
          {
             srmnd_del_srma_msg(dec_info->o_msg);
             dec_info->o_msg = NULL;
          }
          return rc;
       }
       break;

   default:
      assert(0);
      break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_copy
 
  Description   :  This routine is invoked to copy SRMND message and send it to
                   the respective SRMA. (will be called only when the SRMSv
                   Agent and ND binds in the same process space.
 
  Arguments     :  cb      - ptr to the SRMA control block
                   cp_info - ptr to the MDS COPY info
 
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_mds_copy(SRMND_CB *srmnd, MDS_CALLBACK_COPY_INFO *cp_info)
{
   SRMND_MSG *msg;   
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;
   USE (cp_info->i_last);
   
   msg = (SRMND_MSG *)cp_info->i_msg;
   msg_fmt_version = m_NCS_SRMND_ENC_MSG_FMT_GET(
                               cp_info->i_rem_svc_pvt_ver,
                               SRMND_WRT_SRMA_SUBPART_VER_MIN,
                               SRMND_WRT_SRMA_SUBPART_VER_MAX,
                               gl_srmnd_wrt_srma_msg_fmt_array);
   if (0 == msg_fmt_version)
   {
         m_SRMND_LOG_MDS_VER("MDS Copy Callback","Failure Msg to ",cp_info->i_to_svc_id,\
                        "Remote Service Pvt Version",cp_info->i_rem_svc_pvt_ver);

         return NCSCC_RC_FAILURE;
   }

   cp_info->o_msg_fmt_ver = msg_fmt_version;

   if (msg->msg_type == SRMND_BULK_CREATED_MON_MSG)
   {
      SRMND_CREATED_RSRC_MON *bulk_rsrc; 
      SRMND_RSRC_MON_NODE *rsrc =
          msg->info.bulk_rsrc_mon.srma_usr_node->start_rsrc_mon_node;      
      
      while (rsrc) 
      {
         bulk_rsrc = m_MMGR_ALLOC_SRMND_CREATED_RSRC;
         if (!bulk_rsrc)
         {
            /* TBD to clean the cp_info */
            return NCSCC_RC_FAILURE;
         } 
        
         /* Update bulk-rsrc data and add it to the respective list */
         bulk_rsrc->srma_rsrc_hdl = rsrc->srma_rsrc_hdl;
         bulk_rsrc->srmnd_rsrc_hdl = rsrc->rsrc_mon_hdl;
         bulk_rsrc->next_srmnd_rsrc_mon = msg->info.bulk_rsrc_mon.bulk_rsrc_mon;
         msg->info.bulk_rsrc_mon.bulk_rsrc_mon = bulk_rsrc;

         rsrc = rsrc->next_srma_usr_rsrc;
      }  
   }

   /* Now create the msg and copy the cp_info */
   msg = m_MMGR_ALLOC_SRMND_MSG;
   if (!msg)
   {  
      /* TBD to clean the cp_info */
      return NCSCC_RC_FAILURE;
   }

   memset(msg, 0, sizeof(SRMND_MSG));

   memcpy(msg, cp_info->i_msg, sizeof(SRMND_MSG));
   cp_info->o_cpy = (uns8*)msg;

   return NCSCC_RC_SUCCESS;   
}


/****************************************************************************
  Name          : srmnd_mds_cbk
 
  Description   : This routine is a callback routine that is provided to MDS.
                  MDS calls this routine for encode, decode, copy, receive &
                  service event notification operations.
 
  Arguments     : info - ptr to the MDS callback info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 srmnd_mds_cbk (NCSMDS_CALLBACK_INFO *info)
{
   SRMND_CB *srmnd = SRMND_CB_NULL;
   uns32    rc = NCSCC_RC_SUCCESS;

   if (!info)
      return rc;

   /* retrieve srmnd cb */
   if((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND, 
                            (uns32)info->i_yr_svc_hdl)) == SRMND_CB_NULL)
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_ERROR);
      return rc;
   }

   switch(info->i_op)
   {
   case MDS_CALLBACK_RECEIVE:
      {
         /* process the received msg */
         rc = srmnd_mds_rcv(srmnd, &info->info.receive);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_RCV_CBK,
                            SRMSV_LOG_MDS_FAILURE,
                            NCSFL_SEV_CRITICAL);
         }
         else
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_RCV_CBK,
                            SRMSV_LOG_MDS_SUCCESS,
                            NCSFL_SEV_INFO);
         }
      }
      break;

   case MDS_CALLBACK_COPY:
       srmnd_mds_copy(srmnd, &info->info.cpy);
       break;

   case MDS_CALLBACK_SVC_EVENT:
      {
         rc = srmnd_mds_svc_evt(srmnd, &info->info.svc_evt);         
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_SVEVT_CBK,
                            SRMSV_LOG_MDS_FAILURE,
                            NCSFL_SEV_CRITICAL);
         }
         else
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_SVEVT_CBK,
                            SRMSV_LOG_MDS_SUCCESS,
                            NCSFL_SEV_INFO);
         }
      }
      break;

   case MDS_CALLBACK_ENC_FLAT:
      {
         rc = srmnd_mds_flat_enc(srmnd, &info->info.enc_flat);         
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_FLAT_ENC_CBK,
                            SRMSV_LOG_MDS_FAILURE,
                            NCSFL_SEV_CRITICAL);
         }
         else
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_FLAT_ENC_CBK,
                            SRMSV_LOG_MDS_SUCCESS,
                            NCSFL_SEV_INFO);
         }
      }
      break;   

   case MDS_CALLBACK_DEC_FLAT:
      {
         rc = srmnd_mds_flat_dec(srmnd, &info->info.dec_flat);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_FLAT_DEC_CBK,
                            SRMSV_LOG_MDS_FAILURE,
                            NCSFL_SEV_CRITICAL);
         }
         else
         {
            m_SRMND_LOG_MDS(SRMSV_LOG_MDS_FLAT_DEC_CBK,
                            SRMSV_LOG_MDS_SUCCESS,
                            NCSFL_SEV_INFO);
         }
      }
      break;

   case MDS_CALLBACK_DEC:
       rc = srmnd_mds_dec(srmnd, &info->info.dec);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_MDS(SRMSV_LOG_MDS_DEC_CBK,
                          SRMSV_LOG_MDS_FAILURE,
                          NCSFL_SEV_CRITICAL);
       }
       else
       {
          m_SRMND_LOG_MDS(SRMSV_LOG_MDS_DEC_CBK,
                          SRMSV_LOG_MDS_SUCCESS,
                          NCSFL_SEV_INFO);
       }
       break;

   case MDS_CALLBACK_ENC:
       rc = srmnd_mds_enc(srmnd, &info->info.enc);
       if (rc != NCSCC_RC_SUCCESS)
       {
          m_SRMND_LOG_MDS(SRMSV_LOG_MDS_ENC_CBK,
                          SRMSV_LOG_MDS_FAILURE,
                          NCSFL_SEV_CRITICAL);
       }
       else
       {
          m_SRMND_LOG_MDS(SRMSV_LOG_MDS_ENC_CBK,
                          SRMSV_LOG_MDS_SUCCESS,
                          NCSFL_SEV_INFO);
       }
       break;

   default:
       assert(0);
       break;
   }

   /* return srmnd cb */
   ncshm_give_hdl((uns32)info->i_yr_svc_hdl);

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_send
 
  Description   :  This routine sends the SRMND message to SRMND. The send 
                   operation may be a 'normal' send or a synchronous call that 
                   blocks until the response is received from SRMND.
 
  Arguments     :  srmnd  - ptr to the SRMND CB
                   i_msg - ptr to the SRMND message
                   o_msg - double ptr to SRMND message response
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/timeout
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_mds_send (SRMND_CB  *srmnd,
                      SRMND_MSG *srmnd_msg,
                      MDS_DEST  *srma_dest)
{
   NCSMDS_INFO mds_info;
   uns32       rc = NCSCC_RC_SUCCESS;
   MDS_SEND_INFO *send_info = NULL; 

   memset(&mds_info, 0, sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl = srmnd->mds_hdl;
   mds_info.i_svc_id = NCSMDS_SVC_ID_SRMND;
   mds_info.i_op = MDS_SEND;

   send_info = &mds_info.info.svc_send;

   /* populate the send info */
   send_info->i_msg = (NCSCONTEXT)srmnd_msg;
   send_info->i_to_svc = NCSMDS_SVC_ID_SRMA;
   send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;
   send_info->i_sendtype = MDS_SENDTYPE_SND;
   send_info->info.snd.i_to_dest = *srma_dest;
 
   /* send the message */
   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_MDS(SRMSV_LOG_MDS_SEND,
                      SRMSV_LOG_MDS_FAILURE,
                      NCSFL_SEV_CRITICAL);
   }
   else
   {
      m_SRMND_LOG_MDS(SRMSV_LOG_MDS_SEND,
                      SRMSV_LOG_MDS_SUCCESS,
                      NCSFL_SEV_INFO);
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_rcv
 
  Description   :  This routine is invoked when SRMND message is received 
                   from SRMND.
 
  Arguments     :  srmnd - ptr to the SRMND control block
                   rcv_info - ptr to the MDS receive info
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_mds_rcv(SRMND_CB *srmnd, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
   uns32     rc = NCSCC_RC_SUCCESS;
   SRMND_EVT *srmnd_evt = NULL;

   if( 0 == m_NCS_SRMND_MSG_FORMAT_IS_VALID(rcv_info->i_msg_fmt_ver,
                               SRMND_WRT_SRMA_SUBPART_VER_MIN,
                               SRMND_WRT_SRMA_SUBPART_VER_MAX,
                               gl_srmnd_wrt_srma_msg_fmt_array))
   {
       m_SRMND_LOG_MDS_VER("Receive Callback","Failure : Msg From",rcv_info->i_fr_svc_id,\
                        "Message Format Version",rcv_info->i_msg_fmt_ver);
       return NCSCC_RC_FAILURE;
   }


   /* If SRMND oper status is not UP then don't process any message */
   if (srmnd->oper_status != SRMSV_ND_OPER_STATUS_UP)
      return NCSCC_RC_FAILURE;   
     
   /* Create an event for SRMND */
   srmnd_evt = srmnd_evt_create(srmnd->cb_hdl,
                                &rcv_info->i_fr_dest,
                                rcv_info->i_msg,
                                SRMND_SRMA_EVT_TYPE);
   /* send the event */
   if (srmnd_evt)
      rc = m_NCS_IPC_SEND(&srmnd->mbx, srmnd_evt, NCS_IPC_PRIORITY_NORMAL);

   /* if failure, free the event */
   if ((rc != NCSCC_RC_SUCCESS) && (srmnd_evt))
      srmnd_evt_destroy(srmnd_evt);

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_svc_evt
 
  Description   :  This routine is invoked to inform SRMND of MDS events. SRMND 
                   had subscribed to these events during MDS registration.
 
  Arguments     :  srmnd - ptr to the SRMND control block
                   evt_info - ptr to the MDS event info
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_mds_svc_evt(SRMND_CB *srmnd, MDS_CALLBACK_SVC_EVENT_INFO *evt_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* assign mds-dest values for SRMND & SRMND as per the MDS event */
   switch (evt_info->i_change)
   {
   case NCSMDS_UP:
      switch(evt_info->i_svc_id)
      {
      case NCSMDS_SVC_ID_SRMA:
          /* Just ask SRMA to synchronize the database */
          srmnd_send_msg(srmnd,
                         (NCSCONTEXT)&evt_info->i_dest,
                         SRMND_SYNCHRONIZED_MSG);
          break;

      default:
          assert(0);
      }
      break;

   case NCSMDS_DOWN:
      switch(evt_info->i_svc_id)
      {
      case NCSMDS_SVC_ID_SRMA:
          {
             SRMND_EVT *srmnd_evt = NULL;

             /* Create an event for SRMND */
             srmnd_evt = srmnd_evt_create(srmnd->cb_hdl,
                                          &evt_info->i_dest,
                                          NULL,
                                          SRMND_SRMA_DN_EVT_TYPE);
             /* send the event */
             if (srmnd_evt)
                rc = m_NCS_IPC_SEND(&srmnd->mbx, srmnd_evt, NCS_IPC_PRIORITY_HIGH);

             /* if failure, free the event */
             if ((rc != NCSCC_RC_SUCCESS) && (srmnd_evt))
                srmnd_evt_destroy(srmnd_evt);
          }
          break;
         
      default:
          assert(0);
          break;
      }
      break;

   default:
      break;
   }
   
   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_flat_enc
 
  Description   :  This routine is invoked to encode SRMND messages.
 
  Arguments     :  srmnd - ptr to the SRMND control block
                   enc_info - ptr to the MDS encode info
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  This routine also frees the message after encoding it in 
                   the userbuf.
******************************************************************************/
uns32 srmnd_mds_flat_enc (SRMND_CB *srmnd, MDS_CALLBACK_ENC_FLAT_INFO *enc_info)
{
   SRMND_MSG *msg = (SRMND_MSG*)enc_info->i_msg;
   uns32     rc = NCSCC_RC_SUCCESS;
   
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;


   msg_fmt_version = m_NCS_SRMND_ENC_MSG_FMT_GET(
                               enc_info->i_rem_svc_pvt_ver,
                               SRMND_WRT_SRMA_SUBPART_VER_MIN,
                               SRMND_WRT_SRMA_SUBPART_VER_MAX,
                               gl_srmnd_wrt_srma_msg_fmt_array);
   if (0 == msg_fmt_version)
   {  
       m_SRMND_LOG_MDS_VER("MDS Enc Flat Callback","Failure Msg to ",enc_info->i_to_svc_id,\
                        "Remote Service Pvt Version",enc_info->i_rem_svc_pvt_ver); 
       return NCSCC_RC_FAILURE;

   }

   enc_info->o_msg_fmt_ver = msg_fmt_version;

   /* encode into userbuf */
   if ((rc = ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                        (uns8 *)msg,
                                        sizeof(SRMND_MSG))) != NCSCC_RC_SUCCESS)
      return rc;
   
   if (msg->msg_type == SRMND_BULK_CREATED_MON_MSG)
   {
      SRMND_RSRC_MON_NODE *rsrc = msg->info.bulk_rsrc_mon.srma_usr_node->start_rsrc_mon_node;
      uns32               rsrc_num = 0;

      /* Get the rsrc monitor cfg number */
      while (rsrc)
      {
          rsrc_num++;
          rsrc = rsrc->next_srma_usr_rsrc;
      } /* End of WHILE loop */      

      ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                     (uns8 *)&rsrc_num,
                                     sizeof(uns32));

      /* Point to the first rsrc of the list */
      rsrc = msg->info.bulk_rsrc_mon.srma_usr_node->start_rsrc_mon_node;

      while (rsrc)
      {
          ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                     (uns8 *)&rsrc->srma_rsrc_hdl,
                                     sizeof(uns32));

          ncs_encode_n_octets_in_uba(enc_info->io_uba,
                                     (uns8 *)&rsrc->rsrc_mon_hdl,
                                     sizeof(uns32));

          rsrc = rsrc->next_srma_usr_rsrc;
      } /* End of WHILE loop */      
   }
   
   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_flat_dec
 
  Description   :  This routine is invoked to decode SRMND messages.
 
  Arguments     :  srmnd - ptr to the SRMND control block
                   rcv_info - ptr to the MDS decode info
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_mds_flat_dec(SRMND_CB *srmnd, MDS_CALLBACK_DEC_FLAT_INFO *dec_info)
{
   uns32    rc = NCSCC_RC_SUCCESS;
   SRMA_MSG *msg;   

   if( 0 == m_NCS_SRMND_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                               SRMND_WRT_SRMA_SUBPART_VER_MIN,
                               SRMND_WRT_SRMA_SUBPART_VER_MAX,
                               gl_srmnd_wrt_srma_msg_fmt_array))
     {
         m_SRMND_LOG_MDS_VER("Decode Flat Callback","Failure : Msg From",dec_info->i_fr_svc_id,\
                        "Message Format Version",dec_info->i_msg_fmt_ver);

         return NCSCC_RC_FAILURE;

     }

   if ((msg = m_MMGR_ALLOC_SRMA_MSG) == NULL)
   {
      m_SRMND_LOG_SRMSV_MEM(SRMSV_MEM_SRMA_MSG,
                            SRMSV_MEM_ALLOC_FAILED,
                            NCSFL_SEV_CRITICAL);

      return NCSCC_RC_FAILURE;
   }

   memset(msg, 0, sizeof(SRMA_MSG));
   
   /* decode the top level SRMND msg contents */
   if ((rc = ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                     (uns8 *)msg,
                                     sizeof(SRMA_MSG))) != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_SRMA_MSG(msg);
      return rc;
   }

   if (msg->msg_type == SRMA_BULK_CREATE_MON_MSG)
   {
      SRMA_CREATE_RSRC_MON_NODE *bulk_rsrc;
      uns32                     rsrc_num = 0;

      /* Get the number of resource monitor data from the userbuf */
      ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                   (uns8 *)&rsrc_num,
                                    sizeof(uns32));
      while (rsrc_num)
      {
          if ((bulk_rsrc = m_MMGR_ALLOC_SRMA_CREATE_RSRC) == NULL)
          {
             m_SRMND_LOG_SRMSV_MEM(SRMSV_MEM_SRMA_CRT_RSRC,
                                   SRMSV_MEM_ALLOC_FAILED,
                                   NCSFL_SEV_CRITICAL);

             srmnd_del_srma_msg(msg);

             return NCSCC_RC_FAILURE;
          }

          memset(bulk_rsrc, 0, sizeof(SRMA_CREATE_RSRC_MON_NODE));

          ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                   (uns8 *)&bulk_rsrc->rsrc.rsrc_hdl,
                                    sizeof(uns32));

          ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                   (uns8 *)&bulk_rsrc->rsrc.mon_info.rsrc_info,
                                    sizeof(NCS_SRMSV_RSRC_INFO));

          ncs_decode_n_octets_from_uba(dec_info->io_uba,
                                   (uns8 *)&bulk_rsrc->rsrc.mon_info.monitor_data,
                                    sizeof(NCS_SRMSV_MON_DATA));

          /* Adding the node?? so check whether bulk rsrs mon list is empty ?? */
          if (msg->info.bulk_create_mon.bulk_rsrc == NULL)
          {
             /* Oh! yes it is empty.. so let me add as first node of the list */
             msg->info.bulk_create_mon.bulk_rsrc = bulk_rsrc;
          }
          else
          {
             /* Oh! no it is not empty.. then also let me add as a first node
                of the list */
             bulk_rsrc->next_rsrc_mon = msg->info.bulk_create_mon.bulk_rsrc;
             msg->info.bulk_create_mon.bulk_rsrc = bulk_rsrc;
          }
          
          rsrc_num--;
      }      
   }

   /* decode successful */
   dec_info->o_msg = (NCSCONTEXT)msg;

   return rc;
}


/****************************************************************************
  Name          :  srmnd_mds_param_get
 
  Description   :  This routine gets the MDS handle & SRMND MDS address.
 
  Arguments     :  srmnd - ptr to the SRMND Control Block
 
  Return Values :  NCSCC_RC_SUCCESS (or)
                   NCSCC_RC_FAILURE
 
  Notes         :  None.
*****************************************************************************/
static uns32  srmnd_mds_param_get(SRMND_CB *srmnd)
{
   NCSADA_INFO ada_info;
   uns32       rc = NCSCC_RC_SUCCESS;

   memset(&ada_info, 0, sizeof(ada_info));

   ada_info.req = NCSADA_GET_HDLS;
   ada_info.info.adest_get_hdls.i_create_oac = FALSE;

   /* Invoke Absolute Destination Address (ADA) request */
   rc = ncsada_api(&ada_info);
   if (NCSCC_RC_SUCCESS != rc)
      return rc;
   
   /* Store values returned by ADA */
   srmnd->mds_hdl = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
   srmnd->srmnd_dest = ada_info.info.adest_get_hdls.o_adest;

   return rc;
}

