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
FILE NAME: IFND_MDS.C
DESCRIPTION: IFND-MDS Routines
******************************************************************************/

#include "ifnd.h"
/* Static Function Declerations */
static uns32 ifnd_mds_enc(IFSV_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info);
static uns32 ifnd_mds_dec (IFSV_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info);
static uns32 ifnd_mds_rcv(IFSV_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info);
static void ifnd_mds_svc_evt(IFSV_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt);
static void ifnd_mds_cpy(IFSV_CB *cb, MDS_CALLBACK_COPY_INFO *svc_evt);
static uns32 ifnd_mds_callback (NCSMDS_CALLBACK_INFO *info);
static uns32 ifnd_mds_adest_get (IFSV_CB *cb);
extern uns32 ifnd_mds_ifd_up_event (IFSV_CB *ifsv_cb);

/* embedding subslot changes */
MDS_CLIENT_MSG_FORMAT_VER
      IFND_WRT_IFD_MSG_FMT_ARRAY[IFND_WRT_IFD_SUBPART_VER_RANGE]={
           1 /*msg format version for  subpart version 1*/, 2 /* embedding subslot changes */};
MDS_CLIENT_MSG_FORMAT_VER
      IFND_WRT_IFA_MSG_FMT_ARRAY[IFND_WRT_IFA_SUBPART_VER_RANGE]={
           1 /*msg format version for  subpart version 1*/, 2 /* embedding subslot changes */};
MDS_CLIENT_MSG_FORMAT_VER
      IFND_WRT_DRV_MSG_FMT_ARRAY[IFND_WRT_DRV_SUBPART_VER_RANGE]={
           1 /*msg format version for  subpart version 1*/, 2 /* embedding subslot changes */};

/****************************************************************************
 * Name          : ifnd_mds_callback
 *
 * Description   : Call back function provided to MDS. MDS will call this
 *                 function for enc/dec/cpy/rcv/svc_evt operations.
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_mds_callback (NCSMDS_CALLBACK_INFO *info)
{
   uns32    cb_hdl;
   IFSV_CB  *cb=NULL;
   uns32    rc = NCSCC_RC_SUCCESS;

   if(info == NULL)
      return rc;

   cb_hdl = (uns32) info->i_yr_svc_hdl;

   if((cb = (IFSV_CB*)ncshm_take_hdl(NCS_SERVICE_ID_IFND, cb_hdl)) == NULL)
   {
      /* RSR:TBD Log the event */
      return NCSCC_RC_SUCCESS;
   }

   switch(info->i_op)
   {
   case MDS_CALLBACK_ENC:
   case MDS_CALLBACK_ENC_FLAT:
      rc = ifnd_mds_enc(cb, &info->info.enc);
      break;   

   case MDS_CALLBACK_DEC:
   case MDS_CALLBACK_DEC_FLAT:
      rc = ifnd_mds_dec(cb, &info->info.dec);
      break;

   case MDS_CALLBACK_RECEIVE:
      rc = ifnd_mds_rcv(cb, &info->info.receive);
      break;

   case MDS_CALLBACK_COPY:
      ifnd_mds_cpy(cb, &info->info.cpy);
      break;

   case MDS_CALLBACK_SVC_EVENT:
      ifnd_mds_svc_evt(cb, &info->info.svc_evt);
      break;

   default:
      /* IFND will not get copy and callback event */
      rc = NCSCC_RC_FAILURE;
      break;
   }

   if(rc != NCSCC_RC_SUCCESS)
   {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "IFND::MDS CALL BACK FAILED ","FUNC::ifnd_mds_callback");
      /* RSR;TBD Log, */
   }

   ncshm_give_hdl((uns32)cb_hdl);

   return rc;

}

/****************************************************************************
 * Name          : ifnd_mds_adest_get
 *
 * Description   : This function Gets the Handles of local MDS destination
 *
 * Arguments     : cb   : IFND control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifnd_mds_adest_get (IFSV_CB *cb)
{
   NCSADA_INFO   arg;
   uns32         rc;

   memset(&arg,0,sizeof(NCSADA_INFO));

   arg.req                             = NCSADA_GET_HDLS;
   arg.info.adest_get_hdls.i_create_oac = TRUE;
   

   rc = ncsada_api(&arg);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR;TBD Log, */
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "IFND::ncsada_api FAILED ","FUNC::ifnd_mds_adest_get");
      return rc;
   }

   cb->my_mds_hdl       = arg.info.adest_get_hdls.o_mds_pwe1_hdl;
   cb->oac_hdl          = arg.info.adest_get_hdls.o_pwe1_oac_hdl;

   return rc;
}

/****************************************************************************
 * Name          : ifnd_mds_init
 *
 * Description   : This function intializes the MDS procedures for IFND 
 *
 * Arguments     : cb   : IFND control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifnd_mds_init (IFSV_CB *cb)
{
   NCSMDS_INFO          arg;
   uns32                rc = NCSCC_RC_SUCCESS;
   MDS_SVC_ID           svc_id[2] = {0, 0};

   /* Create the vertual Destination for IFD */
   rc = ifnd_mds_adest_get(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,1);
      return rc;
   }

   /* Install your service into MDS */
   memset(&arg,0,sizeof(NCSMDS_INFO));

   arg.i_mds_hdl        = cb->my_mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFND;
   arg.i_op             = MDS_INSTALL;

   /* just check below statement - TBD:RSR*/
   arg.info.svc_install.i_yr_svc_hdl      = cb->cb_hdl;
   arg.info.svc_install.i_install_scope     = NCSMDS_SCOPE_NONE;
   arg.info.svc_install.i_svc_cb          = ifnd_mds_callback;
   arg.info.svc_install.i_mds_q_ownership = FALSE;
   arg.info.svc_install.i_mds_svc_pvt_ver = IFND_SVC_PVT_SUBPART_VERSION;
   
   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,rc);
      goto install_err;
   }

   /* Store the self destination */
   cb->my_dest = arg.info.svc_install.o_dest;
   cb->my_node_id = m_NCS_NODE_ID_FROM_MDS_DEST(arg.info.svc_install.o_dest);
   cb->my_anc  = arg.info.svc_install.o_anc;

   /* IFND is subscribing for IFD MDS service */
   arg.i_op                            = MDS_SUBSCRIBE;
   arg.info.svc_subscribe.i_scope        = NCSMDS_SCOPE_NONE;
   arg.info.svc_subscribe.i_num_svcs   = 1;
   svc_id[0] = NCSMDS_SVC_ID_IFD;
   arg.info.svc_subscribe.i_svc_ids    = svc_id;

   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,3);
      goto subr_err;
   }

   /* Start the NCS_IFSV_IFND_MDS_DEST_GET_TMR  */
   cb->tmr.tmr_type = NCS_IFSV_IFND_MDS_DEST_GET_TMR;
   cb->tmr.svc_id = cb->my_svc_id;
   cb->tmr.info.reserved = 0;

   ifsv_tmr_start(&cb->tmr, cb);
   /* Start the state from NCS_IFSV_IFND_MDS_DEST_GET_STATE */
   cb->ifnd_state = NCS_IFSV_IFND_MDS_DEST_GET_STATE;

   /* IFND is subscribing for IFDRV MDS service */
   arg.info.svc_subscribe.i_scope = NCSMDS_SCOPE_INTRANODE;
   arg.info.svc_subscribe.i_num_svcs   = 2;
   svc_id[0] = NCSMDS_SVC_ID_IFDRV;
   svc_id[1] = NCSMDS_SVC_ID_IFA;

   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,4);
      goto subr_err;
   }

   m_IFND_LOG_API_L(IFSV_LOG_MDS_INIT_DONE,0);
   return rc;

subr_err:
   /* Uninstall the service */
   arg.i_op                         = MDS_UNINSTALL;
   rc = ncsmds_api(&arg);   /* Don't care the rc */

install_err:
   return rc;
}

/****************************************************************************
 * Name          : ifnd_mds_shut
 *
 * Description   : This function un-registers the IFD Service with MDS.
 *
 * Arguments     : cb   : IFD control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifnd_mds_shut (IFSV_CB *cb)
{
   NCSMDS_INFO          arg;
   uns32                rc;

   /* Un-install your service into MDS. 
   No need to cancel the services that are subscribed*/
   memset(&arg,0,sizeof(NCSMDS_INFO));

   arg.i_mds_hdl        = cb->my_mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFND;
   arg.i_op             = MDS_UNINSTALL;

   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_API_L(IFSV_LOG_MDS_DESTROY_FAILURE,NCSMDS_SVC_ID_IFND);
      return rc;
   }
   m_IFND_LOG_API_L(IFSV_LOG_MDS_DESTROY_DONE,NCSMDS_SVC_ID_IFND);

   return rc;
}

/****************************************************************************
 * Name          : ifnd_mds_cpy
 *
 * Description   : IFND is used to copy the buffer when copy when the IfA/IfD
 *                 lies on the same process.
 *
 * Arguments     : 
 *   cb          : IFND control Block.
 *   enc_info    : Svc evt info.
 *
 * Return Values : None
 *
 * Notes         : we assume that IfA/IfD should not lie on the same 
 *                 process, so hitting this function is not possible, any way
 *                 for the compatibility we have written this callback 
 *                 function.
 *****************************************************************************/

static void ifnd_mds_cpy(IFSV_CB *cb, MDS_CALLBACK_COPY_INFO *cpy_info)
{  
   uns8*  stream;
   IFSV_EVT *evt;
   USE (cpy_info->i_last);

   /* cpy_info->o_msg_fmt_ver is not filled as we assume that Ifa/IfD will not lie on the same process and this function will not be hit*/
   evt = m_MMGR_ALLOC_IFSV_EVT;

   if(!evt)
      return;
   memset(evt, 0, sizeof(IFSV_EVT));

   stream = (uns8*)evt;

   strcpy((char*)stream,(char*)cpy_info->i_msg);
   cpy_info->o_cpy = (void*)stream;
   return;
}

/****************************************************************************
 * Name          : ifnd_mds_svc_evt
 *
 * Description   : IFND is informed when MDS events occurr that he has 
 *                 subscribed to
 *
 * Arguments     : 
 *   cb          : IFND control Block.
 *   enc_info    : Svc evt info.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/

static void ifnd_mds_svc_evt(IFSV_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt)
{

   switch (svc_evt->i_change) 
   {
   case NCSMDS_DOWN:
   {
      m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_SVC_DOWN,svc_evt->i_svc_id,\
         svc_evt->i_node_id);
      switch(svc_evt->i_svc_id)
      {
      case NCSMDS_SVC_ID_IFD:         
         /* When the IfD goes down just get the IfD UP flag to down, so that 
          * once if IfND finds the IfD is comming up then it would send all
          * its owned data to IfD and send an record sync info to IfD, so 
          * that it would be in sync with the IfD 
          */
         cb->ifd_card_up = FALSE;
         if ((cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE) ||
             (cb->ifnd_state == NCS_IFSV_IFND_OPERATIONAL_STATE))       
         {
           ifnd_mds_evt(&svc_evt->i_dest,cb,IFND_EVT_IFD_DOWN);
         }
         break;

      case NCSMDS_SVC_ID_IFA:
         if (cb->ifnd_state == NCS_IFSV_IFND_OPERATIONAL_STATE)
         {
           ifnd_mds_evt(&svc_evt->i_dest,cb,IFND_EVT_IFA_DOWN_IN_OPER_STATE);
         }
         else if(cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE)
         {
           ifnd_mds_evt(&svc_evt->i_dest,cb,
                        IFND_EVT_IFA_DOWN_IN_DATA_RET_STATE);
         }
         else if(cb->ifnd_state == NCS_IFSV_IFND_MDS_DEST_GET_STATE)
           ifnd_mds_evt(&svc_evt->i_dest,cb,
                        IFND_EVT_IFA_DOWN_IN_MDS_DEST_STATE);
         break;

      case NCSMDS_SVC_ID_IFDRV:
         if (cb->ifnd_state == NCS_IFSV_IFND_OPERATIONAL_STATE)
         {
           ifnd_mds_evt(&svc_evt->i_dest,cb,
                        IFND_EVT_IFDRV_DOWN_IN_OPER_STATE);
         }
         else if(cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE)
         {
           ifnd_mds_evt(&svc_evt->i_dest,cb,
                        IFND_EVT_IFDRV_DOWN_IN_DATA_RET_STATE);
         }
         else if(cb->ifnd_state == NCS_IFSV_IFND_MDS_DEST_GET_STATE)
           ifnd_mds_evt(&svc_evt->i_dest,cb,
                        IFND_EVT_IFDRV_DOWN_IN_MDS_DEST_STATE);
         break;

      default:
         break;
      } /* switch(svc_evt->i_svc_id) */
      break;
   }/* case NCSMDS_GONE, case NCSMDS_DOWN */
   case NCSMDS_UP:
   {
      m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_SVC_UP,svc_evt->i_svc_id,\
         svc_evt->i_node_id);
      switch(svc_evt->i_svc_id)
      {
      case NCSMDS_SVC_ID_IFD:
         ifnd_rcv_ifd_up_evt(&svc_evt->i_dest, cb);
         break;
      case NCSMDS_SVC_ID_IFA:
      {
       if (cb->ifnd_state == NCS_IFSV_IFND_MDS_DEST_GET_STATE)       
       {
         ifnd_mds_svc_evt_ifa(&svc_evt->i_dest, cb);
       }
         break;
      } /* case NCSMDS_SVC_ID_IFA  */
      case NCSMDS_SVC_ID_IFDRV:  
      {
       if (cb->ifnd_state == NCS_IFSV_IFND_OPERATIONAL_STATE)
       {
         idim_send_ifnd_up_evt(&svc_evt->i_dest, cb);
       }
       else if (cb->ifnd_state == NCS_IFSV_IFND_MDS_DEST_GET_STATE)       
       {
         ifnd_mds_svc_evt_ifdrv(&svc_evt->i_dest, cb); 
       }

       break;
      } /* case NCSMDS_SVC_ID_IFDRV */

      default:
         break;
      } /* switch(svc_evt->i_svc_id) */
      break;
   } /* case NCSMDS_UP. */
   case NCSMDS_NO_ACTIVE:   /* The only active has disappeared. Awaiting a new one */
   {
      switch(svc_evt->i_svc_id)
      {
      case NCSMDS_SVC_ID_IFD:
         /* When the IfD goes down or goes into quiesced state
          * just get the IfD UP flag to down, so that
          * once if IfND finds the IfD is comming up then it would send all
          * its owned data to IfD and send an record sync info to IfD, so
          * that it would be in sync with the IfD
          */
         printf("received NO_ACTIVE for IFD \n"); 
         cb->ifd_card_up = FALSE;
         break;
      }
      break;
   }
   case NCSMDS_NEW_ACTIVE:  /* A new ACTIVE has appeared */
   {
      switch(svc_evt->i_svc_id)
      {
      case NCSMDS_SVC_ID_IFD:
         /* When the IfD comes UP from down or quiesced state
          * just get the IfD UP flag to UP, so that
          * ifnd can start processing the requests which it has rejected 
          * because of IFD down 
          */
         printf("received NEW Active event for IFD \n");
         cb->ifd_card_up = TRUE;
         /* Send an event in the mail box.*/
         ifnd_mds_ifd_up_event(cb);
         break;
      }
      break;
   }
   default:
      break;
   }/* switch (svc_evt->i_change)  */  

  return;
}

/****************************************************************************
 * Name          : ifnd_mds_rcv
 *
 * Description   : MDS will call this function on receiving IFND messages.
 *
 * Arguments     : cb - IFND Control Block
 *                 rcv_info - MDS Receive information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 ifnd_mds_rcv(IFSV_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{   
   IFSV_EVT         *ifsv_evt = (IFSV_EVT *) rcv_info->i_msg;   
   SYSF_MBX         *mbx;        


  /* Update the sender info, usefull in sync requests */
   ifsv_evt->sinfo.ctxt = rcv_info->i_msg_ctxt;
   ifsv_evt->sinfo.dest = rcv_info->i_fr_dest;
   ifsv_evt->sinfo.to_svc = rcv_info->i_fr_svc_id;
   if(rcv_info->i_rsp_reqd) 
   {
      ifsv_evt->sinfo.stype = MDS_SENDTYPE_RSP;
   }

  /* Put it in IFND's Event Queue */
  if (rcv_info->i_fr_svc_id == NCSMDS_SVC_ID_IFDRV)
  {
     mbx = &cb->idim_mbx;               
  } else
  {
     mbx = &cb->mbx;
     ifsv_evt->cb_hdl  = cb->cb_hdl;
  }
  if(m_IFND_EVT_SEND(mbx, rcv_info->i_msg, NCS_IPC_PRIORITY_NORMAL)
     == NCSCC_RC_FAILURE)
  {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "IFND::m_IFND_EVT_SEND  FAILED ","FUNC::ifnd_mds_rcv");
     m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MSG_QUE_SEND_FAIL,(long)mbx);
     return (NCSCC_RC_FAILURE);
  }

  return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: ifnd_mds_enc
  DESCRIPTION   : 
  This function encodes an events sent from IFND.
  ARGUMENTS:
      cb        : IFND control Block.
      enc_info  : Info for encoding

  RETURNS       : NCSCC_RC_SUCCESS/Error Code.
      
  NOTES:
*****************************************************************************/
static uns32 ifnd_mds_enc(IFSV_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{
   EDU_ERR   ederror = 0;
   uns32     rc = NCSCC_RC_FAILURE;

   if ((enc_info->i_to_svc_id == NCSMDS_SVC_ID_IFD) || 
      (enc_info->i_to_svc_id == NCSMDS_SVC_ID_IFA))
   {
      if(enc_info->i_to_svc_id == NCSMDS_SVC_ID_IFD)
       {
         enc_info->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(
                                       enc_info->i_rem_svc_pvt_ver,
                                       IFND_WRT_IFD_SUBPART_VER_AT_MIN_MSG_FMT,
                                       IFND_WRT_IFD_SUBPART_VER_AT_MAX_MSG_FMT,
                                       IFND_WRT_IFD_MSG_FMT_ARRAY);
       }
      else
       {
        enc_info->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(
                                       enc_info->i_rem_svc_pvt_ver,
                                       IFND_WRT_IFA_SUBPART_VER_AT_MIN_MSG_FMT,
                                       IFND_WRT_IFA_SUBPART_VER_AT_MAX_MSG_FMT,
                                       IFND_WRT_IFA_MSG_FMT_ARRAY);
       }
      if(0 == enc_info->o_msg_fmt_ver)
       {
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_ENC_FAILURE,enc_info->i_to_svc_id,\
                        enc_info->i_rem_svc_pvt_ver);
         return NCSCC_RC_FAILURE; 
       }

      /* embedding subslot changes */
      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifsv_evt, enc_info->io_uba, 
               EDP_OP_TYPE_ENC, (IFSV_EVT*)enc_info->i_msg, &ederror, enc_info->o_msg_fmt_ver);
      if(rc != NCSCC_RC_SUCCESS)
      {
         /* Free calls to be added here. */
         m_NCS_EDU_PRINT_ERROR_STRING(ederror);
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_ENC_FAILURE,enc_info->i_to_svc_id,\
            ((IFSV_EVT*)(enc_info->i_msg))->type);
         return rc;
      }
   } else if (enc_info->i_to_svc_id == NCSMDS_SVC_ID_IFDRV)
   {
      enc_info->o_msg_fmt_ver=m_NCS_ENC_MSG_FMT_GET(enc_info->i_rem_svc_pvt_ver,
                               IFND_WRT_DRV_SUBPART_VER_AT_MIN_MSG_FMT,
                               IFND_WRT_DRV_SUBPART_VER_AT_MAX_MSG_FMT,
                               IFND_WRT_DRV_MSG_FMT_ARRAY);
      if(0 == enc_info->o_msg_fmt_ver)
      {
       m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_ENC_FAILURE,enc_info->i_to_svc_id,\
                        enc_info->i_rem_svc_pvt_ver);
       return NCSCC_RC_FAILURE;
      }

      /* embedding subslot changes for backward compatibility*/
      /* encode the message sended to the if driver */
      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_idim_hw_req_info, 
               enc_info->io_uba, EDP_OP_TYPE_ENC, 
               (NCS_IFSV_HW_DRV_REQ*)enc_info->i_msg, &ederror, enc_info->o_msg_fmt_ver);
      if(rc != NCSCC_RC_SUCCESS)
      {
         /* Free calls to be added here. */
         m_NCS_EDU_PRINT_ERROR_STRING(ederror);
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_ENC_FAILURE,enc_info->i_to_svc_id,\
            ((NCS_IFSV_HW_DRV_REQ*)(enc_info->i_msg))->req_type);
         return rc;
      }
   }   
   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   ifnd_mds_dec

  DESCRIPTION   :
  This function decodes a buffer containing an IFND events that was received 
  from off card.

  ARGUMENTS:        
      cb        : IFND control Block.
      dec_info  : Info for decoding

  RETURNS       : NCSCC_RC_SUCCESS/Error Code.

  NOTES:

*****************************************************************************/
static uns32 ifnd_mds_dec (IFSV_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
   EDU_ERR   ederror = 0;   
   IFSV_EVT  *ifsv_evt;
   IFSV_IDIM_EVT *idim_evt;
   uns32     rc = NCSCC_RC_FAILURE;
   
   if ((dec_info->i_fr_svc_id == NCSMDS_SVC_ID_IFD) ||
      (dec_info->i_fr_svc_id == NCSMDS_SVC_ID_IFA))
   {
      
      if(dec_info->i_fr_svc_id == NCSMDS_SVC_ID_IFD)
       {
         if( 0 == m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                                IFND_WRT_IFD_SUBPART_VER_AT_MIN_MSG_FMT,
                                IFND_WRT_IFD_SUBPART_VER_AT_MAX_MSG_FMT,
                                IFND_WRT_IFD_MSG_FMT_ARRAY))
         {
          m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_DEC_FAILURE,dec_info->i_fr_svc_id,\
                                        dec_info->i_msg_fmt_ver);
          return NCSCC_RC_FAILURE;
         }
       }
      else
       {
         if( 0 == m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                                IFND_WRT_IFA_SUBPART_VER_AT_MIN_MSG_FMT,
                                IFND_WRT_IFA_SUBPART_VER_AT_MAX_MSG_FMT,
                                IFND_WRT_IFA_MSG_FMT_ARRAY))
         {
          m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_DEC_FAILURE,dec_info->i_fr_svc_id,\
                                        dec_info->i_msg_fmt_ver);
          return NCSCC_RC_FAILURE;
         }
       }

      /* Allocate the IfSv event */      
      ifsv_evt         = m_MMGR_ALLOC_IFSV_EVT;
      if (ifsv_evt == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      memset(ifsv_evt,0,sizeof(IFSV_EVT));
      dec_info->o_msg  = (NCSCONTEXT)ifsv_evt;

      /* embedding subslot changes for backward compatibility */
      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifsv_evt, dec_info->io_uba, 
               EDP_OP_TYPE_DEC, (IFSV_EVT**)&dec_info->o_msg, &ederror, dec_info->i_msg_fmt_ver);      
      if(rc != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_IFSV_EVT(dec_info->o_msg);
         m_NCS_EDU_PRINT_ERROR_STRING(ederror);
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_DEC_FAILURE,dec_info->i_fr_svc_id,0);
         return rc;
      }
      ifsv_evt->cb_hdl   = cb->cb_hdl;
      ifsv_evt->priority = NCS_IPC_PRIORITY_NORMAL;
   } else if (dec_info->i_fr_svc_id == NCSMDS_SVC_ID_IFDRV)
   {
      if( 0 == m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                                      IFND_WRT_DRV_SUBPART_VER_AT_MIN_MSG_FMT,
                                      IFND_WRT_DRV_SUBPART_VER_AT_MAX_MSG_FMT,
                                      IFND_WRT_DRV_MSG_FMT_ARRAY))
      {
        m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_DEC_FAILURE,dec_info->i_fr_svc_id,\
                             dec_info->i_msg_fmt_ver);
        return NCSCC_RC_FAILURE;
      }

      /* Allocate the IDIM events */
      idim_evt = m_MMGR_ALLOC_IFSV_IDIM_EVT;
      if (idim_evt == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      memset(idim_evt,0,sizeof(IFSV_IDIM_EVT));
      dec_info->o_msg = (NCSCONTEXT)&idim_evt->info.hw_info;

      /* embedding subslot changes for backward compatibility */
      /* decode the message received from the driver */
      rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_idim_hw_rcv_info, 
               dec_info->io_uba, EDP_OP_TYPE_DEC, 
               (NCS_IFSV_HW_INFO**)&dec_info->o_msg, &ederror, dec_info->i_msg_fmt_ver);
      
      if(rc != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_IFSV_IDIM_EVT(idim_evt);
         m_NCS_EDU_PRINT_ERROR_STRING(ederror);
         m_IFND_LOG_HEAD_LINE(IFSV_LOG_MDS_DEC_FAILURE,dec_info->i_fr_svc_id,0);
         return rc;
      }
      
      idim_evt->evt_type = IFSV_IDIM_EVT_RECV_HW_DRV_MSG;
      idim_evt->priority = NCS_IPC_PRIORITY_HIGH;
      dec_info->o_msg    = (NCSCONTEXT)idim_evt;
   }
   return rc;
}

