/*****************************************************************************
 *                                                                           *
 * NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
 * AGREEMENT FROM EMERSON, INC.                                              *
 *                                                                           *
 * THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
 * SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
 *                                                                           *
 * IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
 * PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
 *                                                                           *
 *****************************************************************************

 

..............................................................................

DESCRIPTION:

******************************************************************************
*/

#include "fma.h"


static uns32 fma_fm_msg_rcv(FMA_CB *cb,
                             MDS_DEST       fr_dest,
                             FMA_FM_MSG    *fm_fma);


static uns32 fma_mds_rcv(FMA_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info);
static uns32 fma_mds_register_adest(FMA_CB *cb);
static uns32 fma_mds_svc_evt(FMA_CB *fma_cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt);
static uns32 fma_mds_encode(MDS_CALLBACK_ENC_INFO *enc_info);
static uns32 fma_mds_decode(MDS_CALLBACK_DEC_INFO *dec_info);

const MDS_CLIENT_MSG_FORMAT_VER fma_fm_msg_fmt_map_table [FMA_SUBPART_VER_MAX] = {FMA_FM_MSG_FMT_VER_1};

/****************************************************************************
 * Name          : 
 *
 * Description   : 
 *
 * Arguments     : 
 *                 
 *
 * Return Values : 
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fma_fm_msg_rcv(FMA_CB *cb, MDS_DEST fr_dest, FMA_FM_MSG *fma_fm_msg)
{
    FMA_MBX_EVT_T  *fma_mbx_evt;
    uns32           rc = NCSCC_RC_SUCCESS;
    
    m_FMA_LOG_FUNC_ENTRY("fma_fm_msg_rcv");
    fma_mbx_evt = m_MMGR_ALLOC_FMA_MBX_EVT;
    if (fma_mbx_evt == FMA_MBX_EVT_NULL)
    {
        m_FMA_LOG_MEM(FMA_LOG_MBX_EVT_ALLOC, FMA_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
        m_MMGR_FREE_FMA_FM_MSG(fma_fm_msg);
        fma_fm_msg = NULL;
        return NCSCC_RC_FAILURE;
    }
    
    fma_mbx_evt->fr_dest = fr_dest;
    
    /* Formulate the mbx event for the received message */
    fma_mbx_evt->msg = fma_fm_msg;
 
    /* put the event in FMA mail box */
    if(m_NCS_IPC_SEND(&cb->mbx, fma_mbx_evt, NCS_IPC_PRIORITY_HIGH)
                        == NCSCC_RC_FAILURE)
    {
        m_FMA_LOG_MBX(FMA_LOG_MBX_SEND, FMA_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
        m_MMGR_FREE_FMA_FM_MSG(fma_fm_msg);
        fma_fm_msg = NULL;
        m_MMGR_FREE_FMA_MBX_EVT(fma_mbx_evt);
        fma_mbx_evt = NULL;
        rc = NCSCC_RC_FAILURE;
    }
    
    return rc;
}
/****************************************************************************
 * Name          : fma_mds_rcv
 *
 * Description   : MDS will call this function on receiving FM messages.
 *
 * Arguments     : cb - FMA Control Block
 *                 rcv_info - MDS Receive information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 fma_mds_rcv(FMA_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
   uns32            rc = NCSCC_RC_FAILURE;

   m_FMA_LOG_FUNC_ENTRY("fma_mds_rcv");
   /** Process if the message is from FM as we are expecting MDS message only from FM **/   
   if ((rcv_info->i_fr_svc_id == NCSMDS_SVC_ID_GFM) &&
             (rcv_info->i_to_svc_id == NCSMDS_SVC_ID_FMA))
   {
      rc = fma_fm_msg_rcv(cb, rcv_info->i_fr_dest, (FMA_FM_MSG *)rcv_info->i_msg);
      rcv_info->i_msg = (NCSCONTEXT)0x0;       
   }
   else
   {
      m_MMGR_FREE_FMA_FM_MSG(rcv_info->i_msg);
      rcv_info->i_msg = NULL;
   }

   return rc;
}
/****************************************************************************
 * Name          : fma_mds_callback
 *
 * Description   : 
 *
 * Arguments     : NCSMDS_CALLBACK_INFO *info: call back info.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 fma_mds_callback (NCSMDS_CALLBACK_INFO *info)
{
   uns32    cb_hdl;
   FMA_CB  *cb=NULL;
   uns32    rc = NCSCC_RC_SUCCESS;

   m_FMA_LOG_FUNC_ENTRY("fma_mds_callback"); 
   if(info == NULL)
      return rc;
 
   cb_hdl = (uns32) info->i_yr_svc_hdl;
   if((cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, cb_hdl)) == NULL)
   {
      m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_SUCCESS;
   }  
   switch(info->i_op)
   {
   case MDS_CALLBACK_ENC:
        if(info->info.enc.i_to_svc_id == NCSMDS_SVC_ID_GFM)
        {
            rc = fma_mds_encode(&info->info.enc);
            if (!rc)
               m_FMA_LOG_MDS(FMA_LOG_MDS_ENC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
        }
        else
        {
            m_FMA_LOG_MDS(FMA_LOG_MDS_ENC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
            rc =  NCSCC_RC_FAILURE;
        }
        break;
   case MDS_CALLBACK_ENC_FLAT:
        if(info->info.enc_flat.i_to_svc_id == NCSMDS_SVC_ID_GFM)
        {
            rc =  fma_mds_encode(&info->info.enc_flat);
            if(!rc)
              m_FMA_LOG_MDS(FMA_LOG_MDS_FLAT_ENC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
        }
        else
        {
             m_FMA_LOG_MDS(FMA_LOG_MDS_FLAT_ENC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
             rc = NCSCC_RC_FAILURE;
        }
        break;
   case MDS_CALLBACK_DEC:
        if(info->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_GFM)
        {
            rc = fma_mds_decode(&info->info.dec);
            if(!rc)
               m_FMA_LOG_MDS(FMA_LOG_MDS_DEC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
        }
        else
       {
          m_FMA_LOG_MDS(FMA_LOG_MDS_DEC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL); 
          rc =  NCSCC_RC_FAILURE;
       }
        break;
   case MDS_CALLBACK_DEC_FLAT:
      if(info->info.dec_flat.i_fr_svc_id == NCSMDS_SVC_ID_GFM)
      {
         rc =  fma_mds_decode(&info->info.dec_flat);
         if(!rc)
            m_FMA_LOG_MDS(FMA_LOG_MDS_FLAT_DEC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
      }
      else
      {
         m_FMA_LOG_MDS(FMA_LOG_MDS_FLAT_DEC_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
         rc = NCSCC_RC_FAILURE;
      }
 
      break;
 
   case MDS_CALLBACK_RECEIVE:
      rc = fma_mds_rcv(cb, &info->info.receive);
      if(!rc)
        m_FMA_LOG_MDS(FMA_LOG_MDS_RCV_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
      break;

   /** This case will not be hit for FM-FMA communication **/ 
   case MDS_CALLBACK_COPY:
        if(info->info.cpy.i_to_svc_id == NCSMDS_SVC_ID_GFM)
        {
           rc = fma_fm_mds_cpy(&info->info.cpy);
           if(!rc)
             m_FMA_LOG_MDS(FMA_LOG_MDS_CPY_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
        }
        
      break;

   case MDS_CALLBACK_SVC_EVENT:
       rc = fma_mds_svc_evt(cb, &(info->info.svc_evt));
       if(rc != NCSCC_RC_SUCCESS)
           m_FMA_LOG_MDS(FMA_LOG_MDS_SVEVT_CBK, FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL);
       break;
 
   default:
      rc = NCSCC_RC_FAILURE;
      break;
   }
 
   ncshm_give_hdl((uns32)cb_hdl);
 
   return rc;
}

/*****************************************************************
 * Name          : fma_mds_svc_evt
 *
 * Description   :  
 *
 * Arguments     : 
 *
 * Return Values : NCSCC_RC_SUCCESS
 *
 * Notes         : 
 ******************************************************************/
static uns32 fma_mds_svc_evt(FMA_CB *fma_cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    uns32 node_id;

    m_FMA_LOG_FUNC_ENTRY("fma_mds_svc_evt");

    node_id = svc_evt->i_node_id;
    switch (svc_evt->i_change)
    {
        case NCSMDS_DOWN:
            switch(svc_evt->i_svc_id)
            {
                case NCSMDS_SVC_ID_GFM:
                    /* reset the fm_dest */
                    if(m_MDS_DEST_IS_AN_ADEST(svc_evt->i_dest) == TRUE && fma_cb->my_node_id == node_id)
                    {
                        fma_cb->fm_up = FALSE;
                        fma_cb->fm_dest = 0;
                    }
                    break;
                default:
                    break;
            }
            break;
        case NCSMDS_UP:
            switch(svc_evt->i_svc_id)
            {
                case NCSMDS_SVC_ID_GFM:
                    /* set the fm_dest */
                    if(m_MDS_DEST_IS_AN_ADEST(svc_evt->i_dest) == TRUE && fma_cb->my_node_id == node_id)
                    {
                        fma_cb->fm_up = TRUE;
                        fma_cb->fm_dest = svc_evt->i_dest;
                    }
                    break;
                default:
                    break;
            }
        default:
            break;
     }
     
     return rc;
}

/*****************************************************************
 * Name          : fma_mds_register_adest
 *
 * Description   :  
 *
 * Arguments     : 
 *
 * Return Values : 
 *
 * Notes         : 
 ******************************************************************/
static uns32
fma_mds_register_adest(FMA_CB *cb)
{
    NCSADA_INFO ada_info;
    NCSMDS_INFO mds_info;
 
    m_FMA_LOG_FUNC_ENTRY("fma_mds_register_adest");
    m_NCS_MEMSET(&ada_info, '\0', sizeof(NCSADA_INFO));
 
    ada_info.req = NCSADA_GET_HDLS;
    ada_info.info.adest_get_hdls.i_create_oac = FALSE;
    if ((ncsada_api(&ada_info)) != NCSCC_RC_SUCCESS)
    {
        m_FMA_LOG_MDS(FMA_LOG_MDS_HDLS_GET, FMA_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
        return NCSCC_RC_FAILURE;
    }
 
    /* Store the info returned by MDS */
    cb->mds_hdl = (NCSCONTEXT) ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
    cb->my_dest = ada_info.info.adest_get_hdls.o_adest;
 
    m_NCS_OS_MEMSET(&mds_info, 0, sizeof(mds_info));
 
    mds_info.i_mds_hdl = (MDS_HDL) cb->mds_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_FMA;
    mds_info.i_op = MDS_INSTALL;
    mds_info.info.svc_install.i_mds_q_ownership = FALSE;
    mds_info.info.svc_install.i_svc_cb = fma_mds_callback;
    mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL) cb->cb_hdl;

    mds_info.info.svc_install.i_mds_svc_pvt_ver      = FMA_MDS_SUB_PART_VERSION;
 
    mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
    if(ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
    {
       m_FMA_LOG_MDS(FMA_LOG_MDS_INSTALL, FMA_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
       return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;
   
}

/****************************************************************************
  Name          : fma_mds_reg
 
  Description   :  
 
  Arguments     : 
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :
******************************************************************************/
uns32 fma_mds_reg (FMA_CB *cb)
{
    NCSMDS_INFO  mds_info;
    MDS_SVC_ID   svc[1] = {0};
        
    m_FMA_LOG_FUNC_ENTRY("fma_mds_reg");
    if ( fma_mds_register_adest(cb) != NCSCC_RC_SUCCESS )
    {
        m_FMA_LOG_MDS(FMA_LOG_MDS_REG, FMA_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
        return NCSCC_RC_FAILURE;
    }
    
    m_NCS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));
    
    mds_info.i_mds_hdl        = (MDS_HDL) cb->mds_hdl;
    mds_info.i_svc_id         = NCSMDS_SVC_ID_FMA;
    mds_info.i_op             = MDS_SUBSCRIBE;

    mds_info.info.svc_subscribe.i_scope         = NCSMDS_SCOPE_NONE;
    mds_info.info.svc_subscribe.i_num_svcs      = 1;
    mds_info.info.svc_subscribe.i_svc_ids       = svc;

    svc[0] = NCSMDS_SVC_ID_GFM;

    /* register to MDS */
    if ( ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
    {
    return NCSCC_RC_FAILURE;
    }
    
    return NCSCC_RC_SUCCESS;   
   
}

/***********************************************************************
 * Name          : fma_mds_dereg
 *
 * Description   : 
 *
 * Arguments     : 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 **********************************************************************/
      
uns32 fma_mds_dereg (FMA_CB *cb)
{     
   NCSMDS_INFO          mds_info;
   uns32                rc;
      
   m_FMA_LOG_FUNC_ENTRY("fma_mds_dereg");      
   /* Un-install FMA service from MDS */
   m_NCS_OS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));
      
   mds_info.i_mds_hdl        = (MDS_HDL) cb->mds_hdl;
   mds_info.i_svc_id         = NCSMDS_SVC_ID_FMA;
   mds_info.i_op             = MDS_UNINSTALL;
      
   /* request to un-install from the MDS service */
   rc = ncsmds_api(&mds_info);
      
   if (rc != NCSCC_RC_SUCCESS)
   {  
      m_FMA_LOG_MDS(FMA_LOG_MDS_UNREG, FMA_LOG_MDS_FAILURE, NCSFL_SEV_INFO);
   }
 
   return rc;
}

/*****************************************************************************
 * Function:    fma_fm_mds_msg_sync_send
 *
 * Purpose:     
 *
 * Input:       
 *         
 *
 * Returns:     
 *
 * NOTES:
 *
 * 
 **************************************************************************/
uns32 fma_fm_mds_msg_sync_send(FMA_CB *fma_cb, FMA_FM_MSG *i_msg, FMA_FM_MSG **o_msg, uns32 timeout)
{
    NCSMDS_INFO mds_info;
    uns32       rc = NCSCC_RC_SUCCESS;
    
    m_FMA_LOG_FUNC_ENTRY("fma_fm_mds_msg_sync_send");
    
    if (!i_msg)
        return NCSCC_RC_FAILURE; /* nothing to send */
    if(!o_msg)
        return NCSCC_RC_FAILURE;
    
    if(fma_cb->fm_up == FALSE)
        return NCSCC_RC_NO_TO_SVC;
    
    m_NCS_OS_MEMSET(&mds_info, 0, sizeof(NCSMDS_INFO));
    
    mds_info.i_mds_hdl = (MDS_HDL)fma_cb->mds_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_FMA;
    mds_info.i_op = MDS_SEND;
    
    
    MDS_SEND_INFO            *send_info = &mds_info.info.svc_send;
    MDS_SENDTYPE_SNDRSP_INFO *send = &send_info->info.sndrsp;
    
    /* populate the send info */
    mds_info.info.svc_send.i_msg = (NCSCONTEXT)i_msg;
    mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_GFM;
    mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM; /* Discuss the priority assignments TBD */
    mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SNDRSP;
    mds_info.info.svc_send.info.sndrsp.i_to_dest = fma_cb->fm_dest;
    
    if(timeout)
        mds_info.info.svc_send.info.sndrsp.i_time_to_wait = timeout;
    else
        send->i_time_to_wait = FMA_FM_SYNC_MSG_DEF_TIMEOUT;
    
    /* send the message & block until FM responds or operation times out */
    rc = ncsmds_api(&mds_info);
    if (rc != NCSCC_RC_SUCCESS)
    {
        m_FMA_LOG_MDS(FMA_LOG_MDS_SENDRSP,FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL );
        return rc;
    }
    
    *o_msg = (FMA_FM_MSG *)mds_info.info.svc_send.info.sndrsp.o_rsp;
    mds_info.info.svc_send.info.sndrsp.o_rsp = 0;
    
    return rc;
    
}

/*****************************************************************************
 * Function:    fma_fm_mds_msg_async_send
 *
 * Purpose:     This routine sends a asynchronous MDS message to FM.
 *
 * Input:       
 *              
 *
 * Returns:     NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
 **************************************************************************/
 
uns32 fma_fm_mds_msg_async_send(FMA_CB *cb, FMA_FM_MSG *msg)
{
    NCSMDS_INFO   mds_info;
    MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
    uns32         rc = NCSCC_RC_SUCCESS;
    MDS_SENDTYPE_SND_INFO *snd;

    m_FMA_LOG_FUNC_ENTRY("fma_fm_mds_msg_async_send");
    
    if(cb->fm_up == FALSE)
        return NCSCC_RC_NO_TO_SVC;
    
    /* populate the mds params */
    m_NCS_MEMSET(&mds_info, '\0', sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl = (MDS_HDL) cb->mds_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_FMA;
    mds_info.i_op = MDS_SEND;

    send_info->i_msg    = msg;
    send_info->i_to_svc = NCSMDS_SVC_ID_GFM;
    send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;
    send_info->i_sendtype = MDS_SENDTYPE_SND;

    snd = &send_info->info.snd;
    /* TODO: scope of bcast TBD */
    snd->i_to_dest = cb->fm_dest;

    /* send the message */
    rc = ncsmds_api(&mds_info);
    if (rc != NCSCC_RC_SUCCESS)
       m_FMA_LOG_MDS( FMA_LOG_MDS_SEND,FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL );
    return rc;
}

/*****************************************************************************
 * Function:    fma_fm_mds_msg_bcast
 *
 * Purpose:     This routine sends a MDS message bcast to FM.
 *
 * Input:       
 *              
 *
 * Returns:     NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 *
 **************************************************************************/
 
uns32 fma_fm_mds_msg_bcast(FMA_CB *cb, FMA_FM_MSG *msg)
{
    NCSMDS_INFO   mds_info;
    MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
    uns32         rc = NCSCC_RC_SUCCESS;
    MDS_SENDTYPE_BCAST_INFO *bcast;

    m_FMA_LOG_FUNC_ENTRY("fma_fm_mds_msg_bcast");
    
    /* populate the mds params */
    m_NCS_MEMSET(&mds_info, '\0', sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl = (MDS_HDL) cb->mds_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_FMA;
    mds_info.i_op = MDS_SEND;

    send_info->i_msg    = msg;
    send_info->i_to_svc = NCSMDS_SVC_ID_GFM;
    send_info->i_priority = MDS_SEND_PRIORITY_MEDIUM;
    send_info->i_sendtype = MDS_SENDTYPE_BCAST;

    bcast = &send_info->info.bcast;
    bcast->i_bcast_scope = NCSMDS_SCOPE_NONE;

    /* send the message */
    rc = ncsmds_api(&mds_info);
    if (rc != NCSCC_RC_SUCCESS)
       m_FMA_LOG_MDS( FMA_LOG_MDS_SEND,FMA_LOG_MDS_FAILURE,NCSFL_SEV_CRITICAL );
    return rc;
}

/**************************************************************************
 * Name        :  fma_mds_encode
 *
 * Description :  Encode the FMA_FM_MSG structure
 *
 * Arguments   :
 *
 * Return Value:
 *
 * NOTES:
 *
 **************************************************************************/
static uns32 fma_mds_encode(MDS_CALLBACK_ENC_INFO *enc_info)
{
    uns32 rc = NCSCC_RC_SUCCESS; 
    
    if(enc_info == NULL)
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

    enc_info->o_msg_fmt_ver = m_MSG_FMT_VER_GET(
                                    enc_info->i_rem_svc_pvt_ver,
                                    FMA_SUBPART_VER_MIN,
                                    FMA_SUBPART_VER_MAX,
                                    fma_fm_msg_fmt_map_table);
    if ( enc_info->o_msg_fmt_ver < FMA_FM_MSG_FMT_VER_1 )
    {
        return NCSCC_RC_FAILURE;
    }

    rc = fma_fm_mds_enc(enc_info);

    return rc;
}

/**************************************************************************
 * Name        :  fma_mds_decode
 *
 * Description :  Decode the FMA_FM_MSG structure
 *
 * Arguments   :
 *
 * Return Value:
 *
 * NOTES:
 *
 **************************************************************************/
static uns32 fma_mds_decode(MDS_CALLBACK_DEC_INFO *dec_info)
{
    uns32 rc = NCSCC_RC_SUCCESS;

    if(!m_MSG_FORMAT_IS_VALID(
                        dec_info->i_msg_fmt_ver,
                        FMA_SUBPART_VER_MIN,
                        FMA_SUBPART_VER_MAX,
                        fma_fm_msg_fmt_map_table))
    {
        return NCSCC_RC_FAILURE;
    }

    rc = fma_fm_mds_dec(dec_info);

    return rc;
}

