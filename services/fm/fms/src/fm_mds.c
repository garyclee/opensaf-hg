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

#include "fm.h"

const MDS_CLIENT_MSG_FORMAT_VER fm_fm_msg_fmt_map_table
            [FM_SUBPART_VER_MAX] = {FM_FM_MSG_FMT_VER_1};
            
const MDS_CLIENT_MSG_FORMAT_VER fm_fma_msg_fmt_map_table
            [FM_SUBPART_VER_MAX] = {FM_FMA_MSG_FMT_VER_1};

static uns32 fm_mds_callback (NCSMDS_CALLBACK_INFO *info);
static uns32 fm_mds_get_adest_hdls( FM_CB *cb);
static uns32 fm_mds_svc_evt(FM_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt);
static uns32 fm_mds_rcv_evt( FM_CB *cb,
                        MDS_CALLBACK_RECEIVE_INFO *rcv_info);

static uns32 fm_encode(MDS_CALLBACK_ENC_INFO *enc_info);
static uns32 fm_decode(MDS_CALLBACK_DEC_INFO *dec_info);
static uns32 fm_fm_mds_enc(MDS_CALLBACK_ENC_INFO *enc_info);
static uns32 fm_fm_mds_dec(MDS_CALLBACK_DEC_INFO *dec_info);

static uns32 fm_fill_mds_evt_post_fm_mbx(
                                FM_CB *cb, 
                                FM_EVT *fm_evt, 
                                uns8    slot,
                                uns8    sub_slot,   
                                FM_FSM_EVT_CODE evt_code);

uns32
fm_mds_sync_send(FM_CB *fm_cb, NCSCONTEXT msg,
                NCSMDS_SVC_ID svc_id,
                MDS_SEND_PRIORITY_TYPE priority,
                MDS_SENDTYPES  send_type,
                MDS_DEST   *i_to_dest,
                MDS_SYNC_SND_CTXT *mds_ctxt);
                
uns32 
fm_mds_async_send(FM_CB *fm_cb,NCSCONTEXT msg,
                         NCSMDS_SVC_ID svc_id,
                         MDS_SEND_PRIORITY_TYPE priority,
                         MDS_SENDTYPES  send_type,
                         MDS_DEST   i_to_dest,
                         NCSMDS_SCOPE_TYPE bcast_scope );

/****************************************************************************
* Name          : fm_mds_init
*
* Description   : Installs FMS with MDS and subscribes to FM events. 
*
* Arguments     : Pointer to FMS control block 
*
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
* Notes         : None.
*****************************************************************************/
uns32 fm_mds_init(FM_CB *cb)
{
   NCSMDS_INFO   arg;
   MDS_SVC_ID    svc_id[2] = { NCSMDS_SVC_ID_GFM, NCSMDS_SVC_ID_HCD };
    
   /* Get the MDS handles to be used. */
   if ( fm_mds_get_adest_hdls(cb) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }
    
   /* Install FM on ADEST. */
   memset(&arg,0,sizeof(NCSMDS_INFO));
   arg.i_mds_hdl        = cb->adest_pwe1_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_GFM;
   arg.i_op             = MDS_INSTALL;

   arg.info.svc_install.i_yr_svc_hdl      = (MDS_CLIENT_HDL)gl_fm_hdl;
   arg.info.svc_install.i_install_scope   = NCSMDS_SCOPE_NONE;
   arg.info.svc_install.i_svc_cb          = fm_mds_callback;
   arg.info.svc_install.i_mds_q_ownership = FALSE;
   arg.info.svc_install.i_mds_svc_pvt_ver = FM_MDS_SUB_PART_VERSION;
    
   if ( ncsmds_api(&arg) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(&arg,0,sizeof(NCSMDS_INFO));
   arg.i_mds_hdl        = cb->adest_pwe1_hdl;
   arg.i_svc_id         = NCSMDS_SVC_ID_GFM;
    
   /* Subcribe to AVM, AVND and FMSV MDS UP/DOWN events. */
   arg.i_op                            = MDS_SUBSCRIBE;
   arg.info.svc_subscribe.i_scope      = NCSMDS_SCOPE_NONE;
   arg.info.svc_subscribe.i_num_svcs   = 2;

   arg.info.svc_subscribe.i_svc_ids    = svc_id;

   if (ncsmds_api(&arg) != NCSCC_RC_SUCCESS)
   {
      /* Subcription failed amd hence uninstall.*/
      arg.i_op                         = MDS_UNINSTALL;
      ncsmds_api(&arg);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
* Name          : fm_mds_finalize
*
* Description   : Finalizes with MDS.
*
* Arguments     : pointer to FMS control block. 
*
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
*
* Notes         : None.
*****************************************************************************/
uns32 fm_mds_finalize(FM_CB *cb)
{
   NCSMDS_INFO  arg; 
   uns32        return_val;
   
   memset(&arg,0,sizeof(NCSMDS_INFO));
   arg.i_mds_hdl = (MDS_HDL)cb->adest_pwe1_hdl;
   arg.i_svc_id  = NCSMDS_SVC_ID_GFM;
   arg.i_op      = MDS_UNINSTALL;

   return_val = ncsmds_api(&arg); 

   return return_val;    
}


/****************************************************************************
* Name          : fm_mds_get_adest_hdls
*
* Description   : Gets a handle to the MDS ADEST 
*
* Arguments     : pointer to the FMS control block. 
*
* Return Values : 
*
* Notes         : None.
*****************************************************************************/
static uns32 fm_mds_get_adest_hdls( FM_CB *cb)
{
   NCSADA_INFO   arg;
     
   memset(&arg,0,sizeof(NCSADA_INFO));
     
   arg.req = NCSADA_GET_HDLS;
   arg.info.adest_get_hdls.i_create_oac = FALSE;

   if (ncsada_api(&arg) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   cb->adest = arg.info.adest_get_hdls.o_adest;
   cb->adest_hdl = arg.info.adest_get_hdls.o_mds_adest_hdl;
   cb->adest_pwe1_hdl = arg.info.adest_get_hdls.o_mds_pwe1_hdl;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
* Name          : fm_mds_callback
*
* Description   : Callback registered with MDS 
*
* Arguments     : pointer to the NCSMDS_CALLBACK_INFO structure
*
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
*
* Notes         : None.
*****************************************************************************/
static uns32 fm_mds_callback (NCSMDS_CALLBACK_INFO *info)
{
    uns32  cb_hdl;
    FM_CB  *cb;   
    uns32  return_val = NCSCC_RC_SUCCESS;

    if (info == NULL)
    {
       m_NCS_SYSLOG(NCS_LOG_INFO,"fm_mds_callback: Invalid param info\n");
       return NCSCC_RC_SUCCESS;
    }
    
    cb_hdl = (uns32) info->i_yr_svc_hdl;
    cb =(FM_CB *) ncshm_take_hdl(NCS_SERVICE_ID_GFM, cb_hdl);
    if (cb == NULL)
    {
       m_NCS_SYSLOG(NCS_LOG_INFO,"fm_mds_callback: CB retrieve failed\n");
       return NCSCC_RC_SUCCESS;
    }
    
   switch(info->i_op)
   {
   /* For intra-process MDS. Not required for us.*/
   case MDS_CALLBACK_COPY:
       break;

   case MDS_CALLBACK_ENC:
       return_val = fm_encode(&info->info.enc);            
       break;

   /* Calling enc for enc_flat too.*/    
   case MDS_CALLBACK_ENC_FLAT:
       return_val = fm_encode(&info->info.enc_flat);
       break;

   case MDS_CALLBACK_DEC:
       return_val = fm_decode(&info->info.dec);
       break;

   /* Calling dec for dec_flat too.*/    
   case MDS_CALLBACK_DEC_FLAT:
       return_val = fm_decode(&info->info.dec_flat);
       break;

   case MDS_CALLBACK_RECEIVE:
       return_val = fm_mds_rcv_evt(cb,&(info->info.receive));
       break;

   /* Received AVM/AVND/GFM UP/DOWN event.*/    
   case MDS_CALLBACK_SVC_EVENT:
       return_val = fm_mds_svc_evt(cb, &(info->info.svc_evt)); 
       break;

   default:
       return_val = NCSCC_RC_FAILURE;
       break;
   }
    
   ncshm_give_hdl(cb_hdl);

   return return_val;
}


/****************************************************************************
* Name          : fm_mds_svc_evt
*
* Description   : Function to process MDS service/control events.
*
* Arguments     : pointer to FM CB & MDS_CALLBACK_SVC_EVENT_INFO
*
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
*
* Notes         : None.
*****************************************************************************/
static uns32 fm_mds_svc_evt(FM_CB *cb, MDS_CALLBACK_SVC_EVENT_INFO *svc_evt)
{
   uns32  return_val = NCSCC_RC_SUCCESS;   
   uns8   peer_slot,peer_sub_slot,peer_chassis;
      
   if (NULL == svc_evt)
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,"fm_mds_svc_evt: svc_evt NULL.\n");
      return NCSCC_RC_FAILURE;
   }

   m_NCS_GET_PHYINFO_FROM_NODE_ID(svc_evt->i_node_id, &peer_chassis, &peer_slot, &peer_sub_slot);

   switch( svc_evt->i_change )
   { 
   case NCSMDS_DOWN:
       switch ( svc_evt->i_svc_id )
       {
       case NCSMDS_SVC_ID_GFM:
           /* Processing only for alternate node. */
           if ((peer_slot != cb->slot) && 
               (m_MDS_DEST_IS_AN_ADEST(svc_evt->i_dest) == TRUE ))
           {
              cb->peer_adest = 0;
           }
           break;

       default:
           m_NCS_SYSLOG(NCS_LOG_INFO,"Wrong MDS DOWN event type\n");
           break;
       }
       break;  
              
   case NCSMDS_UP:
       switch(svc_evt->i_svc_id)
       {
       case NCSMDS_SVC_ID_GFM:
           if ((peer_slot != cb->slot) && 
                (m_MDS_DEST_IS_AN_ADEST(svc_evt->i_dest) == TRUE ))
           {
              cb->peer_adest = svc_evt->i_dest;                        
           }
           break;

       case NCSMDS_SVC_ID_HCD:
           cb->is_platform = TRUE;
           break;
       default:
           break;
       }
       break;

   default:
       m_NCS_SYSLOG(NCS_LOG_INFO,"Wrong MDS event\n");
       break;
   }

   return return_val;
}


/***************************************************************************
* Name          : fm_mds_rcv_evt 
*
* Description   : Top level function that receives/processes MDS events.
*                                                                        
* Arguments     : Pointer to FM control block & MDS_CALLBACK_RECEIVE_INFO    
*                                                                           
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE          
*                                                                           
* Notes         : None.   
***************************************************************************/
static uns32 fm_mds_rcv_evt(FM_CB *cb, MDS_CALLBACK_RECEIVE_INFO *rcv_info)
{
   FM_EVT        *fm_evt;
   FMA_FM_MSG    *fma_rcv_msg;
   uns32         return_val = NCSCC_RC_SUCCESS;
   GFM_GFM_MSG   *gfm_rcv_msg=NULL;

   if (NULL == rcv_info)
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,"fm_mds_rcv_evt: rcv_info NULL.\n");
      return NCSCC_RC_FAILURE;
   }

   fm_evt = m_MMGR_ALLOC_FM_EVT;
   if( NULL == fm_evt )
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,"fm_mds_rcv_evt: fm_evt allocation FAILED.\n");
      return NCSCC_RC_FAILURE;
   }
         
   if ( NCSMDS_SVC_ID_FMA == rcv_info->i_fr_svc_id )
   {
      fma_rcv_msg = (FMA_FM_MSG *)rcv_info->i_msg;
      switch(fma_rcv_msg->msg_type)
      {
      case FMA_FM_EVT_HB_LOSS:
      case FMA_FM_EVT_HB_RESTORE:
      case FMA_FM_EVT_NODE_RESET_IND:
          return_val = fm_fill_mds_evt_post_fm_mbx(cb,fm_evt,
                          fma_rcv_msg->info.phy_addr.slot,fma_rcv_msg->info.phy_addr.site,
                          (fma_rcv_msg->msg_type + FM_EVT_FMA_START));
          break;
                                
      case FMA_FM_EVT_CAN_SMH_SW:
          fm_evt->evt_code = fma_rcv_msg->msg_type + FM_EVT_FMA_START ; 
          fm_evt->mds_ctxt = rcv_info->i_msg_ctxt; 
          fm_evt->fr_dest  = rcv_info->i_fr_dest;
          if (m_NCS_IPC_SEND(&cb->mbx,fm_evt,NCS_IPC_PRIORITY_HIGH) 
                            != NCSCC_RC_SUCCESS)
          {
             return_val = NCSCC_RC_FAILURE;
          }
          break;

      default:
          break;
      }

      /* Free fma_rcv_msg. Mem allocated in decode routine. */
      m_MMGR_FREE_FMA_FM_MSG(fma_rcv_msg);
   }
   else if (NCSMDS_SVC_ID_GFM == rcv_info->i_fr_svc_id)
   {
      gfm_rcv_msg = (GFM_GFM_MSG *)rcv_info->i_msg;
      switch(gfm_rcv_msg->msg_type)
      {               
      case GFM_GFM_EVT_NODE_RESET_IND:
          if (gfm_rcv_msg->info.node_reset_ind_info.slot == cb->slot &&
              gfm_rcv_msg->info.node_reset_ind_info.sub_slot == cb->sub_slot);
          {
             /* I have to reset myself as I got node reset indication from peer*/
             ncs_reboot("FMS: Reset by Peer.");
          }
          break;
                                
      default:
          m_NCS_SYSLOG(NCS_LOG_INFO,"Wrong MDS event from GFM.\n");
          return_val = NCSCC_RC_FAILURE;
          break;
      }
      /* Free gfm_rcv_msg here. Mem allocated in decode. */ 
      m_MMGR_FREE_FM_FM_MSG(gfm_rcv_msg);
   }
   if( NCSCC_RC_FAILURE == return_val )
   {
      m_MMGR_FREE_FM_EVT(fm_evt);
      fm_evt = NULL;
   }

   return return_val;
}


/***************************************************************************
* Name          : fm_fill_mds_evt_post_fm_mbx 
*                                                                           
* Description   : Posts an event to mail box.     
*                                                                        
* Arguments     : Control Block, Pointer to event, slot, subslot and Event Code. 
*                                                                           
* Return Values :  NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                       
*                                                                           
* Notes         :   None.
***************************************************************************/
static uns32 fm_fill_mds_evt_post_fm_mbx(FM_CB *cb, FM_EVT *fm_evt, uns8 slot,
                                         uns8 sub_slot, FM_FSM_EVT_CODE evt_code)
{
   fm_evt->evt_code = evt_code;
   fm_evt->slot = slot;
   fm_evt->sub_slot = sub_slot;

   if (m_NCS_IPC_SEND(&cb->mbx,fm_evt,NCS_IPC_PRIORITY_HIGH) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************
* Name          : fm_mds_sync_send
*
* Description   : Sends a message to destination in SYNC.     
*                                                                        
* Arguments     :  Control Block, Pointer to message, Priority, Send Type, 
*                  Pointer to MDS DEST, Context of the message.   
*                                                                           
* Return Values :  NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                       
*                                                                           
* Notes         :  None.  
***************************************************************************/
uns32 fm_mds_sync_send(FM_CB *fm_cb, NCSCONTEXT msg, NCSMDS_SVC_ID svc_id,
                MDS_SEND_PRIORITY_TYPE priority, MDS_SENDTYPES  send_type,
                MDS_DEST   *i_to_dest, MDS_SYNC_SND_CTXT *mds_ctxt)
{
   NCSMDS_INFO  info;
   uns32        return_val;
    
   memset(&info, '\0', sizeof(NCSMDS_INFO));

   info.i_mds_hdl = (MDS_HDL)fm_cb->adest_pwe1_hdl;
   info.i_svc_id  = NCSMDS_SVC_ID_GFM;
   info.i_op      = MDS_SEND;

   info.info.svc_send.i_msg = msg;
   info.info.svc_send.i_priority = priority;
   info.info.svc_send.i_to_svc   = svc_id;

   if(mds_ctxt)
   {
      info.info.svc_send.i_sendtype = send_type;
      info.info.svc_send.info.rsp.i_sender_dest = *i_to_dest;
      info.info.svc_send.info.rsp.i_msg_ctxt = *mds_ctxt;
   }
   else
   {
      m_NCS_SYSLOG(NCS_LOG_INFO,"fm_mds_sync_send: mds_ctxt NULL\n");
      return NCSCC_RC_FAILURE;
   }
    
   return_val = ncsmds_api(&info);
    

   return return_val;
}


/***************************************************************************
* Name          : fm_mds_async_send 
*
*                                                                           
* Description   : Sends a message to destination in async. 
*                                                                        
* Arguments     :  Control Block, Pointer to message, Priority, Send Type, 
*                  Pointer to MDS DEST, Context of the message. 
*                                                                           
* Return Values :  NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                       
*                                                                           
* Notes         : None 
***************************************************************************/
uns32 fm_mds_async_send(FM_CB *fm_cb,NCSCONTEXT msg, NCSMDS_SVC_ID svc_id,
                         MDS_SEND_PRIORITY_TYPE priority, MDS_SENDTYPES  send_type,
                         MDS_DEST  i_to_dest, NCSMDS_SCOPE_TYPE bcast_scope)
{
   NCSMDS_INFO  info;
   uns32        return_val;
   
   if ((NCSMDS_SVC_ID_GFM == svc_id) || 
       (NCSMDS_SVC_ID_FMA == svc_id))
   {
      memset(&info, 0, sizeof(info));

      info.i_mds_hdl  = (MDS_HDL)fm_cb->adest_pwe1_hdl;
      info.i_op       = MDS_SEND;
      info.i_svc_id   = NCSMDS_SVC_ID_GFM;
       
      info.info.svc_send.i_msg = msg;
      info.info.svc_send.i_priority = priority;
      info.info.svc_send.i_sendtype = send_type;
      info.info.svc_send.i_to_svc   = svc_id;
       
      memset(&(info.info.svc_send.info.snd.i_to_dest), 0, 
                                                sizeof(MDS_DEST));
       
      if(bcast_scope)
      {
         info.info.svc_send.info.bcast.i_bcast_scope = bcast_scope;
      }
      else
      {
         info.info.svc_send.info.snd.i_to_dest = i_to_dest;
      }

      return_val = ncsmds_api(&info);
      if (NCSCC_RC_FAILURE == return_val)
      {
         m_NCS_SYSLOG(NCS_LOG_INFO,"fms async send failed \n");
      }
   }
   else
   {
      return_val = NCSCC_RC_FAILURE;
      m_NCS_SYSLOG(NCS_LOG_INFO,"fm_mds_async_send: MDS Send fail: alt gfm NOT UP/Invalid svc id.\n");
   }

   return return_val;
}


/*******************************************************************************
* Name          : fm_encode 
* 
* Description   : Encode callback registered with MDS.
*
* Arguments     : Pointer to the MDS callback info struct MDS_CALLBACK_ENC_INFO
*
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
* Notes         : None.  
********************************************************************************/
static uns32 fm_encode(MDS_CALLBACK_ENC_INFO *enc_info)
{
   if (NCSMDS_SVC_ID_GFM == enc_info->i_to_svc_id)
   {
      enc_info->o_msg_fmt_ver = m_MSG_FMT_VER_GET(enc_info->i_rem_svc_pvt_ver,
                                                  FM_SUBPART_VER_MIN,
                                                  FM_SUBPART_VER_MAX,
                                                  fm_fm_msg_fmt_map_table);

      if (enc_info->o_msg_fmt_ver < FM_FM_MSG_FMT_VER_1)
      {
         m_NCS_SYSLOG(NCS_LOG_INFO,"fm_encode: MSG FMT VER from GFM mis-match\n"); 
         return NCSCC_RC_FAILURE;
      }

      return fm_fm_mds_enc(enc_info);   
   }
   else if (NCSMDS_SVC_ID_FMA == enc_info->i_to_svc_id)
   {
      enc_info->o_msg_fmt_ver = m_MSG_FMT_VER_GET(enc_info->i_rem_svc_pvt_ver,
                                                  FM_SUBPART_VER_MIN,
                                                  FM_SUBPART_VER_MAX,
                                                  fm_fma_msg_fmt_map_table);

      if (enc_info->o_msg_fmt_ver < FM_FMA_MSG_FMT_VER_1)
      {
          m_NCS_SYSLOG(NCS_LOG_INFO,"fm_encode: MSG FMT VER from FMA mis-match\n"); 
          return NCSCC_RC_FAILURE;
      }
            
      return fma_fm_mds_enc(enc_info);
   }

   m_NCS_SYSLOG(NCS_LOG_INFO,"fm_encode: MDS MSG neither to FMA nor to GFM.\n");

   return NCSCC_RC_SUCCESS;
}


/********************************************************************************
* Name          : fm_decode
*
* Description   : Decode callback  
*
* Arguments     : Pointer to the MDS callback info struct MDS_CALLBACK_DEC_INFO
*
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                        
*                                                                           
* Notes         : None.
*********************************************************************************/
static uns32 fm_decode(MDS_CALLBACK_DEC_INFO *dec_info)
{
   if (NCSMDS_SVC_ID_GFM == dec_info->i_fr_svc_id)
   {
      if (!m_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                                 FM_SUBPART_VER_MIN,
                                 FM_SUBPART_VER_MAX,
                                 fm_fm_msg_fmt_map_table))
      {
         m_NCS_SYSLOG(NCS_LOG_INFO,"fm_decode: MSG FMT VER from GFM mis-match\n");
         return NCSCC_RC_FAILURE;
      }

      return fm_fm_mds_dec(dec_info);
   }
   else if (NCSMDS_SVC_ID_FMA == dec_info->i_fr_svc_id)
   {
      if (!m_MSG_FORMAT_IS_VALID(dec_info->i_msg_fmt_ver,
                        FM_SUBPART_VER_MIN,
                        FM_SUBPART_VER_MAX,
                        fm_fma_msg_fmt_map_table))
      {
         m_NCS_SYSLOG(NCS_LOG_INFO,"fm_decode: MSG FMT VER from FMA mis-match\n");
         return NCSCC_RC_FAILURE;
      }

      return fma_fm_mds_dec(dec_info);
        
   }
   m_NCS_SYSLOG(NCS_LOG_INFO,"fm_decode: MDS MSG neither from FMA nor from GFM.\n");

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************
* Name          : fm_fm_mds_enc
*
* Description   :  To encode GFM related messages
*                                                                        
* Arguments     :Pointer to the MDS callback info struct MDS_CALLBACK_DEC_INFO
*                                                                           
* Return Values :  NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                       
*                                                                           
* Notes         :    None.
***************************************************************************/
static uns32 fm_fm_mds_enc(MDS_CALLBACK_ENC_INFO *enc_info)
{
   GFM_GFM_MSG *msg;
   NCS_UBAID   *uba;
   uns8        *data;

   if (NULL == enc_info)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if((NULL == enc_info->i_msg) || (NULL == enc_info->io_uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    
   uba = enc_info->io_uba;
   msg = (GFM_GFM_MSG *)enc_info->i_msg;

   data = ncs_enc_reserve_space(uba, sizeof(uns8));
   if (NULL == data)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
        
   ncs_encode_8bit(&data, msg->msg_type);
   ncs_enc_claim_space(uba, sizeof(uns8));

   switch(msg->msg_type)
   {
   case GFM_GFM_EVT_NODE_RESET_IND:
       data = ncs_enc_reserve_space(uba,(3*sizeof(uns8)));
       if (NULL == data)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

       ncs_encode_8bit(&data, (NCS_BOOL)msg->info.node_reset_ind_info.shelf);
       ncs_encode_8bit(&data, msg->info.node_reset_ind_info.slot);
       ncs_encode_8bit(&data, msg->info.node_reset_ind_info.sub_slot);
       ncs_enc_claim_space(uba,3*sizeof(uns8));
       break;

   default:
       m_NCS_SYSLOG(NCS_LOG_INFO,"fm_fm_mds_enc: Invalid msg type for encode.\n");
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       break;
   }
    
   return NCSCC_RC_SUCCESS;
}


/***************************************************************************
* Name          : fm_fm_mds_dec
*                                                                           
* Description   : To decode GFM related messages                                                      
*                                                                        
* Arguments     : Ptr to the MDS callback info struct MDS_CALLBACK_DEC_INFO
*                                                                           
* Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        
*                                                                           
* Notes         : None.
***************************************************************************/
static uns32 fm_fm_mds_dec(MDS_CALLBACK_DEC_INFO *dec_info)
{
   GFM_GFM_MSG *msg;
   NCS_UBAID   *uba;
   uns8        *data;
   uns8        data_buff[256];

   if (NULL == dec_info->io_uba)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   msg = m_MMGR_ALLOC_FM_FM_MSG;
   if (NULL == msg)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   memset(msg, 0, sizeof(GFM_GFM_MSG));

   dec_info->o_msg = msg;
   uba = dec_info->io_uba;

   data = ncs_dec_flatten_space(uba,data_buff,sizeof(uns8));
   if (NULL == data)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   msg->msg_type = (GFM_GFM_MSG_TYPE)ncs_decode_8bit(&data);   
   ncs_dec_skip_space(uba,sizeof(uns8));

   switch(msg->msg_type)
   {
   case GFM_GFM_EVT_NODE_RESET_IND:
       data = ncs_dec_flatten_space(uba,data_buff, 3*sizeof(uns8));  
       if(NULL == data)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
             
       msg->info.node_reset_ind_info.shelf =(NCS_BOOL) ncs_decode_8bit(&data);
       msg->info.node_reset_ind_info.slot = (uns8)ncs_decode_8bit(&data); 
       msg->info.node_reset_ind_info.sub_slot = (uns8)ncs_decode_8bit(&data);
       ncs_dec_skip_space(uba,3*sizeof(uns8));
       break;

   default:
       m_NCS_SYSLOG(NCS_LOG_INFO,"fm_fm_mds_dec: Invalid msg for decoding.\n");
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       break;
   }

   return NCSCC_RC_SUCCESS;
}

