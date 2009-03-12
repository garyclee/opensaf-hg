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
FILE NAME: IFD_MDS.C
DESCRIPTION: IFSV-MDS Routines
******************************************************************************/

#include "ifd.h"

static void ifd_mds_cpy(IFSV_CB *cb, MDS_CALLBACK_COPY_INFO *svc_evt);
static uns32 ifd_mds_enc(IFSV_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info);
static uns32 ifd_mds_dec (IFSV_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info);
static uns32 ifd_mds_rcv(IFSV_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info);
static void ifd_mds_svc_evt(IFSV_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt);
static uns32 ifd_mds_callback (NCSMDS_CALLBACK_INFO *info);
static uns32 ifd_mds_vdest_create (IFSV_CB *cb);
static uns32 ifd_mds_vdest_destroy (IFSV_CB *cb);
static uns32 ifd_mds_ifnd_down_evt(MDS_DEST *mds_dest, IFSV_CB *cb);
uns32 ifd_mds_quiesced_process(IFSV_CB* cb);

/* embedding subslot changes for backward compatibility */
MDS_CLIENT_MSG_FORMAT_VER
      IFD_WRT_IFND_MSG_FMT_ARRAY[IFD_WRT_IFND_SUBPART_VER_RANGE]={
           1 /*msg format version for  subpart version 1*/, 2 /* embedding subslot changes */};


/****************************************************************************
 * Name          : ifd_mds_callback
 *
 * Description   : Call back function provided to MDS. MDS will call this
 *                 function for enc/dec/cpy/rcv/svc_evt operations.
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifd_mds_callback (NCSMDS_CALLBACK_INFO *info)
{
   uns32    cb_hdl;
   IFSV_CB  *cb=NULL;
   uns32    rc = NCSCC_RC_SUCCESS;

   if(info == NULL)
      return rc;

   cb_hdl = (uns32) info->i_yr_svc_hdl;

   if((cb = (IFSV_CB*)ncshm_take_hdl(NCS_SERVICE_ID_IFD, cb_hdl)) == NULL)
   {
      /* RSR:TBD Log the event */
      return NCSCC_RC_SUCCESS;
   }

   switch(info->i_op)
   {
   case MDS_CALLBACK_ENC:
   case MDS_CALLBACK_ENC_FLAT:
      rc = ifd_mds_enc(cb, &info->info.enc);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mds_enc() returned failure and the ret value is :",rc);
      }
      break;   

   case MDS_CALLBACK_DEC:
   case MDS_CALLBACK_DEC_FLAT:
      rc = ifd_mds_dec(cb, &info->info.dec);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mds_dec() returned failure and the ret value is :",rc);
      }
      break;

   case MDS_CALLBACK_RECEIVE:
      rc = ifd_mds_rcv(cb, &info->info.receive);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mds_rcv() returned failure and the ret value is :",rc);
      }
      break;

   case MDS_CALLBACK_COPY:
      ifd_mds_cpy(cb, &info->info.cpy);
      break;
      
   case MDS_CALLBACK_SVC_EVENT:
      ifd_mds_svc_evt(cb, &info->info.svc_evt);
      break;

   case MDS_CALLBACK_QUIESCED_ACK:
      rc = ifd_mds_quiesced_process(cb);
      if(rc != NCSCC_RC_SUCCESS)
      {
       m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mds_quiesced_process() returned failure and the ret value is :",rc);
      }
      break;

   default:
      /* IFD will not get copy and callback event */
      rc = NCSCC_RC_FAILURE;
      break;
   }

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR;TBD Log, */
   }

   ncshm_give_hdl((uns32)cb_hdl);

   return rc;

}

/****************************************************************************
 * Name          : ifd_mds_vdest_create
 *
 * Description   : This function Creates the Virtual destination for IFD
 *
 * Arguments     : cb   : IFD control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifd_mds_vdest_create (IFSV_CB *cb)
{
   NCSVDA_INFO   arg;
   uns32         rc=NCSCC_RC_SUCCESS;
/*   SaNameT       name = {4, "IFD"}; */

   memset(&arg,0,sizeof(NCSVDA_INFO));
   cb->my_dest = IFD_VDEST_ID;

   arg.req                             = NCSVDA_VDEST_CREATE;
   
/*   arg.info.vdest_create.info.named.i_name  = name;
   arg.info.vdest_create.i_create_type   = NCSVDA_VDEST_CREATE_NAMED;*/
   arg.info.vdest_create.i_persistent  = FALSE;
   arg.info.vdest_create.i_policy      = NCS_VDEST_TYPE_DEFAULT;
   arg.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
   arg.info.vdest_create.i_create_oac   = TRUE;
   arg.info.vdest_create.info.specified.i_vdest = cb->my_dest;

   rc = ncsvda_api(&arg);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR;TBD Log, */
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsvda_api() returned failure and the ret value is :",rc);
      return rc;
   }

/*   cb->my_dest          = arg.info.vdest_create.info.named.o_vdest; */
   cb->my_mds_hdl       = arg.info.vdest_create.o_mds_pwe1_hdl;
   cb->oac_hdl          = arg.info.vdest_create.o_pwe1_oac_hdl;

   return rc;
}

/****************************************************************************
 * Name          : ifd_mds_vdest_destroy
 *
 * Description   : This function Destroys the Virtual destination of IFD
 *
 * Arguments     : cb   : IFD control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ifd_mds_vdest_destroy (IFSV_CB *cb)
{
   NCSVDA_INFO   arg;   
   uns32          rc;   

   memset(&arg,0,sizeof(NCSVDA_INFO));

   arg.req                             = NCSVDA_VDEST_DESTROY;   
   arg.info.vdest_destroy.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;

   arg.info.vdest_destroy.i_vdest      = cb->my_dest;

   rc = ncsvda_api(&arg);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR;TBD Log, */
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsvda_api() returned failure and the ret value is :",rc);
      return rc;
   }      
   return rc;
}

/****************************************************************************
 * Name          : ifd_mds_init
 *
 * Description   : This function intializes the MDS procedures for IFD 
 *
 * Arguments     : cb   : IFD control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifd_mds_init (IFSV_CB *cb)
{
   NCSMDS_INFO   arg;
   MDS_SVC_ID    svc_id;
   
   uns32                rc;

   /* Create the vertual Destination for IFD */
   rc = ifd_mds_vdest_create(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifd_mds_vdest_create() returned failure and the ret value is :",rc);
      m_IFD_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,1);
      return rc;
   }

   /* Install your service into MDS */
   memset(&arg,0,sizeof(NCSMDS_INFO));

   arg.i_mds_hdl        = cb->my_mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFD;
   arg.i_op             = MDS_INSTALL;

   arg.info.svc_install.i_yr_svc_hdl      = cb->cb_hdl;
   arg.info.svc_install.i_install_scope   = NCSMDS_SCOPE_NONE;
   arg.info.svc_install.i_svc_cb          = ifd_mds_callback;
   arg.info.svc_install.i_mds_q_ownership = FALSE;
   arg.info.svc_install.i_mds_svc_pvt_ver = IFD_SVC_PVT_SUBPART_VERSION;
   
   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsmds_api() returned failure and the ret value is :",rc);
      m_IFD_LOG_API_L(IFSV_LOG_MDS_INIT_FAILURE,2);
      return rc;
   }

   /* IFD is subscribing for IFND MDS service */
   memset(&arg,0,sizeof(NCSMDS_INFO));
   arg.i_mds_hdl                       = cb->my_mds_hdl;
   arg.i_svc_id                        = NCSMDS_SVC_ID_IFD;
   arg.i_op                            = MDS_SUBSCRIBE;
   arg.info.svc_subscribe.i_scope      = NCSMDS_SCOPE_NONE;
   arg.info.svc_subscribe.i_num_svcs   = 1;
   svc_id                              = NCSMDS_SVC_ID_IFND;
   arg.info.svc_subscribe.i_svc_ids    = &svc_id;

   rc = ncsmds_api(&arg);

   /* IFD is subscribing for IFD MDS service */
   memset(&arg,0,sizeof(NCSMDS_INFO));
   arg.i_mds_hdl                       = cb->my_mds_hdl;
   arg.i_svc_id                        = NCSMDS_SVC_ID_IFD;
   arg.i_op                            = MDS_RED_SUBSCRIBE;
   arg.info.svc_subscribe.i_scope      = NCSMDS_SCOPE_NONE;
   arg.info.svc_subscribe.i_num_svcs   = 1;
   svc_id                              = NCSMDS_SVC_ID_IFD;
   arg.info.svc_subscribe.i_svc_ids    = &svc_id;

   rc = ncsmds_api(&arg);
   return rc;
}

/****************************************************************************
 * Name          : ifd_mds_shut
 *
 * Description   : This function un-registers the IFD Service with MDS.
 *
 * Arguments     : cb   : IFD control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ifd_mds_shut (IFSV_CB *cb)
{
   NCSMDS_INFO          arg;
   uns32                rc;

   /* Un-install your service into MDS */
   memset(&arg,0,sizeof(NCSMDS_INFO));

   arg.i_mds_hdl        = cb->my_mds_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_IFD;
   arg.i_op             = MDS_UNINSTALL;

   rc = ncsmds_api(&arg);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsmds_api() returned failure and the ret value is :",rc);
      m_IFD_LOG_API_L(IFSV_LOG_MDS_DESTROY_FAILURE,NCSMDS_SVC_ID_IFD);
      return rc;
   }

   m_IFD_LOG_API_L(IFSV_LOG_MDS_DESTROY_DONE,NCSMDS_SVC_ID_IFD);
   /* Destroy the vertual Destination of IFD */
   rc = ifd_mds_vdest_destroy(cb);
   return rc;
}

/****************************************************************************
 * Name          : ifd_mds_rcv
 *
 * Description   : MDS will call this function on receiving IFD messages.
 *
 * ARGUMENTS:
 *   cb          : IFD control Block.
 *   enc_info    : Received information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ifd_mds_rcv(IFSV_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
  
   IFSV_EVT   *evt = (IFSV_EVT *) rcv_info->i_msg;

  /* Fill IFD CB handle in the received message */
  evt->cb_hdl  = cb->cb_hdl;

  /* Update the sender info, usefull in sync requests */
   evt->sinfo.ctxt = rcv_info->i_msg_ctxt;
   evt->sinfo.dest = rcv_info->i_fr_dest;
   evt->sinfo.to_svc = rcv_info->i_fr_svc_id;
   if(rcv_info->i_rsp_reqd) 
   {
      evt->sinfo.stype = MDS_SENDTYPE_RSP;
   }

  /* Put it in IFD's Event Queue */
  if(m_IFD_EVT_SEND(&cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL) == NCSCC_RC_FAILURE)
  {
    /* Log the error */
    return (NCSCC_RC_FAILURE);
  }

  return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : ifd_mds_svc_evt
 *
 * Description   : IFD is informed when MDS events occurr that he has 
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

static void ifd_mds_svc_evt(IFSV_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt)
{

  MDS_DEST mds_dest;
  IFSV_TMR     *tmr=IFSV_NULL;
  uns32        ii;

  memcpy(&mds_dest,&svc_evt->i_dest, sizeof(MDS_DEST));

   switch (svc_evt->i_change) 
   {
   
   case NCSMDS_DOWN:
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_MDS_SVC_DOWN,svc_evt->i_svc_id,\
            svc_evt->i_node_id);
      switch(svc_evt->i_svc_id)
      {
      case NCSMDS_SVC_ID_IFND:                  
       m_NCS_CONS_PRINTF("received IfND Down Event \n");
       if(cb->ha_state == SA_AMF_HA_ACTIVE)
       {
         /* Send it in the mail box */
         ifd_mds_ifnd_down_evt(&svc_evt->i_dest, cb);
         break;
       }
/* This piece of code is added to remove hack. If any issues remove this code */
       else  /* if state is standby */ 
       {
          m_NCS_CONS_PRINTF("State is Standby : FROM mds call back func\n ");
          if (cb->ifnd_rec_flush_tmr == IFSV_NULL )  
          {
             tmr = m_MMGR_ALLOC_IFSV_TMR;
             cb->ifnd_rec_flush_tmr = tmr;
             
             if (tmr == IFSV_NULL)
             {
                break;
             }

             memset( tmr,0,sizeof(IFSV_TMR));
            
             m_NCS_CONS_PRINTF("Forming Timer Event in MDS Cbk \n");

             /* Start a timer for 1 min to wait for IfD to get active */
             tmr->tmr_type = NCS_IFSV_IFD_IFND_REC_FLUSH_TMR;
             tmr->svc_id = cb->my_svc_id;
             ifsv_tmr_start(tmr,cb);
          }
          /* push the ifnd addr into storage */
          for (ii = 0; ii < MAX_IFND_NODES; ii++)
          {
             if (cb->ifnd_mds_addr[ii].valid != TRUE)
             {
                m_NCS_CONS_PRINTF("Marking %dth Entry to be deleted \n",ii);
                cb->ifnd_mds_addr[ii].valid = TRUE;
                memcpy(&cb->ifnd_mds_addr[ii].ifndAddr,&svc_evt->i_dest, sizeof(MDS_DEST));
                break;
             }
          }  
          break;
       }
/* Till this point the code is modified */          
      default:
         break;
      }
      break;
   case NCSMDS_UP:      
      {
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_MDS_SVC_UP,svc_evt->i_svc_id,\
            svc_evt->i_node_id);
         break;
      }
   default:
      break;
   }

  return;
}

/****************************************************************************
 * Name          : ifd_mds_cpy
 *
 * Description   : IFD is used to copy the buffer when copy when the IfND
 *                 lies on the same process.
 *
 * Arguments     : 
 *   cb          : IFND control Block.
 *   enc_info    : Svc evt info.
 *
 * Return Values : None
 *
 * Notes         : we assume that IfND should not lie on the same 
 *                 process, so hitting this function is not possible, any way
 *                 for the compatibility we have written this callback 
 *                 function.
 *****************************************************************************/

static void ifd_mds_cpy(IFSV_CB *cb, MDS_CALLBACK_COPY_INFO *svc_evt)
{  
   uns8*  stream;
   /*svc_evt->o_msg_fmt_ver is not filled as ifnd will not be in same process
     as ifd*/
   stream = (uns8*)m_MMGR_ALLOC_IFSV_EVT;
   USE (svc_evt->i_last);
   
   strcpy((char*)stream,(char*)svc_evt->i_msg);
   svc_evt->o_cpy = (void*)stream;
   return;
}

/*****************************************************************************
  PROCEDURE NAME: ifd_mds_enc
  DESCRIPTION   : 
  This function encodes an events sent from IFD.
  ARGUMENTS:
      cb        : IFD control Block.
      enc_info  : Info for encoding

  RETURNS       : NCSCC_RC_SUCCESS/Error Code.
      
  NOTES:
*****************************************************************************/
static uns32 ifd_mds_enc(IFSV_CB *cb, MDS_CALLBACK_ENC_INFO *enc_info)
{
   EDU_ERR   ederror = 0;   
   uns32     rc = NCSCC_RC_SUCCESS;

   enc_info->o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(enc_info->i_rem_svc_pvt_ver,
                               IFD_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT,
                               IFD_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT,
                               IFD_WRT_IFND_MSG_FMT_ARRAY);
   if(0 == enc_info->o_msg_fmt_ver)
   {
     m_IFD_LOG_HEAD_LINE(IFSV_LOG_MDS_ENC_FAILURE,enc_info->i_to_svc_id,\
                          enc_info->i_rem_svc_pvt_ver);
     return NCSCC_RC_FAILURE; 
   }

   /* embedding subslot changes for backward compatibility */
   rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifsv_evt, enc_info->io_uba, 
                       EDP_OP_TYPE_ENC, (IFSV_EVT*)enc_info->i_msg, &ederror, enc_info->o_msg_fmt_ver);
   if(rc != NCSCC_RC_SUCCESS)
   {
      /* Free calls to be added here. */
      m_NCS_EDU_PRINT_ERROR_STRING(ederror);
      m_IFD_LOG_HEAD_LINE(IFSV_LOG_MDS_ENC_FAILURE,enc_info->i_to_svc_id,\
         ((IFSV_EVT*)(enc_info->i_msg))->type);
      return rc;
   }
   return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   ifd_mds_dec

  DESCRIPTION   :
  This function decodes a buffer containing an IFD events that was received 
  from off card.

  ARGUMENTS:        
      cb        : IFD control Block.
      dec_info  : Info for decoding

  RETURNS       : NCSCC_RC_SUCCESS/Error Code.

  NOTES:

*****************************************************************************/
static uns32 ifd_mds_dec (IFSV_CB *cb, MDS_CALLBACK_DEC_INFO *dec_info)
{
   EDU_ERR   ederror = 0;
   IFSV_EVT  *ifsv_evt;
   uns32     rc = NCSCC_RC_SUCCESS;

   if( 0 == m_NCS_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                                      IFD_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT,
                                      IFD_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT,
                                      IFD_WRT_IFND_MSG_FMT_ARRAY))
   {
     m_IFD_LOG_HEAD_LINE(IFSV_LOG_MDS_DEC_FAILURE,dec_info->i_fr_svc_id,\
                             dec_info->i_msg_fmt_ver);
     return NCSCC_RC_FAILURE;
   }
   ifsv_evt        = m_MMGR_ALLOC_IFSV_EVT;
   if (ifsv_evt == NULL)
   {
      return NCSCC_RC_FAILURE;
   }
   memset(ifsv_evt,0,sizeof(IFSV_EVT));
   dec_info->o_msg = (NCSCONTEXT)ifsv_evt;

   /* embedding subslot changes for backward compatibility */
   rc = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, ifsv_edp_ifsv_evt, dec_info->io_uba, 
                       EDP_OP_TYPE_DEC, (IFSV_EVT**)&dec_info->o_msg, &ederror, dec_info->i_msg_fmt_ver);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_IFSV_EVT(ifsv_evt);
      m_NCS_EDU_PRINT_ERROR_STRING(ederror);
      m_IFD_LOG_HEAD_LINE(IFSV_LOG_MDS_DEC_FAILURE,dec_info->i_fr_svc_id,\
         0);
      return rc;
   }
   ifsv_evt->priority = NCS_IPC_PRIORITY_NORMAL;
   return rc;
}

/****************************************************************************\
 PROCEDURE NAME : ifd_mds_quiesced_process

 DESCRIPTION    : IFD sending the event to it's mail box

 ARGUMENTS      : ifsv_cb : IFD control Block

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
uns32 ifd_mds_quiesced_process(IFSV_CB *ifsv_cb)
{
   IFSV_EVT     *pEvt=0;
   uns32        rc= NCSCC_RC_SUCCESS;

   if(ifsv_cb->is_quisced_set)
   {
      pEvt  =  m_MMGR_ALLOC_IFSV_EVT ;
      if(!pEvt)
      {
        return NCSCC_RC_FAILURE;
      }
      memset(pEvt, 0, sizeof(IFSV_EVT));
      pEvt->type = IFD_EVT_QUISCED_STATE_INFO;
      /* Fill IFD CB handle in the received message */
      pEvt->cb_hdl  = ifsv_cb->cb_hdl;
   
      /* Put it in MQD's Event Queue */
      rc =m_IFD_EVT_SEND(&ifsv_cb->mbx, pEvt, NCS_IPC_PRIORITY_NORMAL);
      if(rc != NCSCC_RC_SUCCESS)
      {
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"m_IFD_EVT_SEND() returned failure and the ret value is :",rc);
         m_MMGR_FREE_IFSV_EVT(pEvt);
      }
   }
   return rc;
}

/****************************************************************************\
 PROCEDURE NAME : ifd_mds_ifnd_down_evt

 DESCRIPTION    : IFD sending the event to it's mail box

 ARGUMENTS      : mds_dest : Pointer to MDS Destination of IFND going down
                  ifsv_cb : IFD control Block

 RETURNS        : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
\*****************************************************************************/
static uns32 ifd_mds_ifnd_down_evt(MDS_DEST *mds_dest, IFSV_CB *ifsv_cb)
{
   IFSV_EVT     *pEvt=0;
   uns32        rc= NCSCC_RC_SUCCESS;

   pEvt  =  m_MMGR_ALLOC_IFSV_EVT ;
   if(!pEvt)
   {
       return NCSCC_RC_FAILURE;
   }
   memset(pEvt, 0, sizeof(IFSV_EVT));
   pEvt->type = IFD_EVT_IFND_DOWN;
   /* Fill IFD CB handle in the received message */
   pEvt->cb_hdl  = ifsv_cb->cb_hdl;
   memcpy(&pEvt->info.ifd_evt.info.mds_dest, mds_dest, sizeof(MDS_DEST));

   /* Put it in MQD's Event Queue */
   rc =m_IFD_EVT_SEND(&ifsv_cb->mbx, pEvt, NCS_IPC_PRIORITY_NORMAL);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"m_IFD_EVT_SEND() returned failure for IFD_EVT_IFND_DOWN and the ret value is :",rc);
      m_MMGR_FREE_IFSV_EVT(pEvt);
   }
return rc;
}


