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
 
  DESCRIPTION:This module deals wih registering AvM with MDS and provides the
  callbacks for MDS .
  ..............................................................................

  Function Included in this Module:

  avm_mds_initialize - 
  avm_mds_finalize   -
  avm_vdest_create   -
  avm_vdest_destroy  -
  avm_mds_msg_send   -
  avm_mds_msg_rcv    -
 
******************************************************************************
*/


#include "avm.h"
#include "saClm.h"
#include "avm_avd.h"
#include "avm_fsm.h"
#include "avm_db.h"
#include "avm_cb.h"
#include "avm_evt.h"
#include "avm_mem.h"
#include "lfm_avm_intf.h"

/* MDS callback function declaration */
static uns32 avm_mds_callback (struct ncsmds_callback_info *info);
static uns32 avm_mds_cpy      (struct ncsmds_callback_info *info);
static uns32 avm_mds_enc      (struct ncsmds_callback_info *info);
static uns32 avm_mds_dec      (struct ncsmds_callback_info *info);
static uns32 avm_mds_enc_flat (struct ncsmds_callback_info *info);
static uns32 avm_mds_dec_flat (struct ncsmds_callback_info *info);
static uns32 avm_mds_rcv      (struct ncsmds_callback_info *info);
static uns32 avm_mds_svc_event(struct ncsmds_callback_info *info);
static uns32 avm_mds_sys_evt  (struct ncsmds_callback_info *info);
static uns32 avm_avd_msg_rcv(MDS_CLIENT_HDL cb_hdl, AVD_AVM_MSG_T *avd_avm);
static uns32 avm_bam_msg_rcv(MDS_CLIENT_HDL cb_hdl, BAM_AVM_MSG_T *bam_avm);

#if (MOT_ATCA == 1)
static uns32 avm_lfm_msg_rcv(MDS_CLIENT_HDL cb_hdl,MDS_DEST fr_dest, LFM_AVM_MSG_T *lfm_avm, 
                             MDS_SYNC_SND_CTXT *mds_ctxt);
#endif

static uns32 avm_mds_register_adest(AVM_CB_T  *cb);
/* routines to create and destroy MDS VDEST */
static uns32 avm_mds_vdest_create(AVM_CB_T  *cb);
static uns32 avm_mds_vdest_destroy(AVM_CB_T *cb);

static uns32
avm_mds_quiesced_ack(struct ncsmds_callback_info *mds_cb_info);

static uns32
avm_mds_avd_up(struct ncsmds_callback_info *mds_cb_info);

const MDS_CLIENT_MSG_FORMAT_VER avm_avd_msg_fmt_map_table[AVM_AVD_SUBPART_VER_MAX] = {AVSV_AVD_AVM_MSG_FMT_VER_1};
const MDS_CLIENT_MSG_FORMAT_VER avm_lfm_msg_fmt_map_table[AVM_LFM_SUBPART_VER_MAX] = {NCS_AVM_LFM_MSG_FMT_VER_1};
const MDS_CLIENT_MSG_FORMAT_VER avm_bam_msg_fmt_map_table[AVM_BAM_SUBPART_VER_MAX] = {AVSV_AVM_BAM_MSG_FMT_VER_1};

/***********************************************************************
 * Name          : avm_mds_callback
 *
 * Description   : Callback Dispatcher for various MDS operations.
 *
 * Arguments     : cb   : AVM-MDS control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 **********************************************************************/

static uns32 avm_mds_callback(struct ncsmds_callback_info *info)
{
   static NCSMDS_CALLBACK_API cb_set[MDS_CALLBACK_SVC_MAX] = {
   avm_mds_cpy,      /* MDS_CALLBACK_COPY      */
   avm_mds_enc,      /* MDS_CALLBACK_ENC       */
   avm_mds_dec,      /* MDS_CALLBACK_DEC       */
   avm_mds_enc_flat, /* MDS_CALLBACK_ENC_FLAT  */
   avm_mds_dec_flat, /* MDS_CALLBACK_DEC_FLAT  */
   avm_mds_rcv,      /* MDS_CALLBACK_RECEIVE   */
   avm_mds_svc_event,    /* MDS_CALLBACK_SVC_EVENT */
   avm_mds_sys_evt,
   avm_mds_quiesced_ack
   };
   
   if((MDS_CALLBACK_COPY <= info->i_op) && 
      (MDS_CALLBACK_QUIESCED_ACK >= info->i_op))
   {
      return (*cb_set[info->i_op])(info);
   }else
   {
      m_AVM_LOG_INVALID_VAL_FATAL(info->i_op);
      return NCSCC_RC_FAILURE;
   }
}

/***********************************************************************
 * Name          : avm_mds_cpy
 *
 * Description   : This function copies an events received.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 **********************************************************************/

static uns32 avm_mds_cpy(struct ncsmds_callback_info *info)
{
   AVM_AVD_MSG_T  *dst_msg;
   
#if (MOT_ATCA == 1) 
      
   if(info->info.cpy.i_to_svc_id == NCSMDS_SVC_ID_LFM)
   {
      info->info.cpy.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.cpy.i_rem_svc_pvt_ver,
                                            AVM_LFM_SUBPART_VER_MIN,
                                            AVM_LFM_SUBPART_VER_MAX,
                                            avm_lfm_msg_fmt_map_table);

      if(info->info.cpy.o_msg_fmt_ver < NCS_AVM_LFM_MSG_FMT_VER_1)
      {
         m_AVM_LOG_INVALID_VAL_CRITICAL(info->info.cpy.o_msg_fmt_ver);   
         return NCSCC_RC_FAILURE;
      }

      return lfm_avm_mds_cpy(&info->info.cpy);
   }
#endif
   
   info->info.cpy.o_msg_fmt_ver = avm_avd_msg_fmt_map_table[info->info.cpy.i_rem_svc_pvt_ver];

   dst_msg              = (AVM_AVD_MSG_T*)info->info.cpy.i_msg;
   info->info.cpy.o_cpy = (uns8*)dst_msg;
   m_AVM_LOG_MDS(AVM_LOG_MDS_CPY_CBK, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);   
   return NCSCC_RC_SUCCESS;
 }

static uns32 avm_mds_enc(struct ncsmds_callback_info *info)
{
 
#if (MOT_ATCA == 1)
   if(info->info.enc.i_to_svc_id == NCSMDS_SVC_ID_LFM)
   {
      info->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
                                            AVM_LFM_SUBPART_VER_MIN,
                                            AVM_LFM_SUBPART_VER_MAX,
                                            avm_lfm_msg_fmt_map_table);

      if(info->info.enc.o_msg_fmt_ver < NCS_AVM_LFM_MSG_FMT_VER_1)
      {
         m_AVM_LOG_INVALID_VAL_CRITICAL(info->info.enc.o_msg_fmt_ver);
         return NCSCC_RC_FAILURE;
      }
      
      return lfm_avm_mds_enc(&info->info.enc);
   }
   else
#endif
   {
      m_AVM_LOG_DEBUG("MDS not supposed to Invoke enc func", NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
}      

static uns32 avm_mds_dec(struct ncsmds_callback_info *info)
{      
#if (MOT_ATCA == 1)
   if(info->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_LFM)
   {
      if(!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
                                AVM_LFM_SUBPART_VER_MIN,
                                AVM_LFM_SUBPART_VER_MAX,
                                avm_lfm_msg_fmt_map_table))
      {
         /* log the problem */
         m_AVM_LOG_INVALID_VAL_CRITICAL(info->info.dec.i_msg_fmt_ver);
         return NCSCC_RC_FAILURE;
      }

      return lfm_avm_mds_dec(&info->info.dec);
   }
   else
#endif
   {
      m_AVM_LOG_DEBUG("MDS not suppoded to invoke dec func", NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
}

static uns32 avm_mds_enc_flat(struct ncsmds_callback_info *info)
{     
#if (MOT_ATCA == 1) 
   if(info->info.enc_flat.i_to_svc_id == NCSMDS_SVC_ID_LFM)
   {
      info->info.enc_flat.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(info->info.enc_flat.i_rem_svc_pvt_ver,
                                            AVM_LFM_SUBPART_VER_MIN,
                                            AVM_LFM_SUBPART_VER_MAX,
                                            avm_lfm_msg_fmt_map_table);

      if(info->info.enc_flat.o_msg_fmt_ver < NCS_AVM_LFM_MSG_FMT_VER_1)
      {
         m_AVM_LOG_INVALID_VAL_CRITICAL(info->info.enc_flat.o_msg_fmt_ver);
         return NCSCC_RC_FAILURE;
      }

      return lfm_avm_mds_enc(&info->info.enc_flat);
   }
   else
#endif
   {
      m_AVM_LOG_DEBUG("MDS not supposed to invoke enc flat func", NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
}

static uns32 avm_mds_dec_flat(struct ncsmds_callback_info *info)
{     
#if (MOT_ATCA == 1) 
   if(info->info.dec_flat.i_fr_svc_id == NCSMDS_SVC_ID_LFM)
   {
      if(!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec_flat.i_msg_fmt_ver,
                                AVM_LFM_SUBPART_VER_MIN,
                                AVM_LFM_SUBPART_VER_MAX,
                                avm_lfm_msg_fmt_map_table))
      {
         /* log the problem */
         m_AVM_LOG_INVALID_VAL_CRITICAL(info->info.dec_flat.i_msg_fmt_ver);
         return NCSCC_RC_FAILURE;
      }
      return lfm_avm_mds_dec(&info->info.dec_flat);
   }
   else
#endif
   {
      m_AVM_LOG_DEBUG("MDS not supposed to invoke dec flat func", NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
}

static uns32 avm_mds_sys_evt(struct ncsmds_callback_info *info)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_AVM_LOG_DEBUG("MDS not supposed to invoke sys evt func", NCSFL_SEV_CRITICAL);
  return rc;

}
/***********************************************************************
 * Name          : avm_avd_msg_rcv 
 *
 * Description   : MDS Callback. 
 *
 * Arguments     : cb   : AVM control Block pointer.
 *                 avd_avm  : Msg received from AvD
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 **********************************************************************/
static uns32 avm_avd_msg_rcv(
                             MDS_CLIENT_HDL cb_hdl,
                             AVD_AVM_MSG_T    *avd_avm
                            )
{
   AVM_EVT_T     *avm_evt;
   AVM_CB_T      *avm_cb = NULL;
   uns32          rc = NCSCC_RC_SUCCESS;

   /* retrieve AVM CB */
   if (NULL == (avm_cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM,
                (uns32)cb_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL); 
      m_MMGR_FREE_AVD_AVM_MSG(avd_avm);
      return NCSCC_RC_FAILURE;
   }

   avm_evt = m_MMGR_ALLOC_AVM_EVT;
   if (AVM_EVT_NULL == avm_evt)
   {
      m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      m_MMGR_FREE_AVD_AVM_MSG(avd_avm);
      ncshm_give_hdl((uns32)cb_hdl);
      return NCSCC_RC_FAILURE;
   }

   avm_evt->src = AVM_EVT_AVD;   

   /* formulate the event for the received message */
   avm_evt->evt.avd_evt      = avd_avm; 
   
   /* put the event in AVM mail box */
   if(m_NCS_IPC_SEND(&avm_cb->mailbox, avm_evt, NCS_IPC_PRIORITY_HIGH)
                    == NCSCC_RC_FAILURE)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVD_AVM_MSG(avd_avm);
      m_MMGR_FREE_AVM_EVT(avm_evt); 
      ncshm_give_hdl((uns32)cb_hdl);
      rc = NCSCC_RC_FAILURE;
   }

   /* return AVM CB */
   ncshm_give_hdl((uns32)cb_hdl);
   
   return rc;
}

#if (MOT_ATCA == 1)
static uns32 avm_lfm_msg_rcv(MDS_CLIENT_HDL    cb_hdl,
                             MDS_DEST          fr_dest,
                             LFM_AVM_MSG_T    *lfm_avm,
                             MDS_SYNC_SND_CTXT *mds_ctxt)
{
   AVM_EVT_T     *avm_evt;
   AVM_CB_T      *avm_cb = NULL;
   uns32          rc = NCSCC_RC_SUCCESS;


   /* retrieve AVM CB */
   if (NULL == (avm_cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM,
                (uns32)cb_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL); 
      lfm_avm_msg_free(lfm_avm);
      return NCSCC_RC_FAILURE;
   }

   avm_evt = m_MMGR_ALLOC_AVM_EVT;
   if (AVM_EVT_NULL == avm_evt)
   {
      m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      lfm_avm_msg_free(lfm_avm);
      ncshm_give_hdl((uns32)cb_hdl);
      return NCSCC_RC_FAILURE;
   }

   avm_evt->src = AVM_EVT_LFM;   

   /* formulate the event for the received message */
   avm_evt->evt.lfm_evt.msg      = lfm_avm; 
   avm_evt->evt.lfm_evt.fr_dest  = fr_dest; 
   avm_evt->evt.lfm_evt.mds_ctxt = *mds_ctxt;
   
   /* put the event in AVM mail box */
   if(m_NCS_IPC_SEND(&avm_cb->mailbox, avm_evt, NCS_IPC_PRIORITY_HIGH)
                    == NCSCC_RC_FAILURE)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVM_EVT(avm_evt); 
      ncshm_give_hdl((uns32)cb_hdl);
      rc = NCSCC_RC_FAILURE;
   }

   /* return AVM CB */
   ncshm_give_hdl((uns32)cb_hdl);
   
   return rc;
}

#endif
static uns32 avm_bam_msg_rcv(MDS_CLIENT_HDL    cb_hdl,
                             BAM_AVM_MSG_T    *bam_avm)
{
   AVM_EVT_T     *avm_evt;
   AVM_CB_T      *avm_cb = NULL;
   uns32          rc = NCSCC_RC_SUCCESS;


   /* retrieve AVM CB */
   if (NULL == (avm_cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM,
                (uns32)cb_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL); 
      m_MMGR_FREE_BAM_AVM_MSG(bam_avm);
      return NCSCC_RC_FAILURE;
   }

   avm_evt = m_MMGR_ALLOC_AVM_EVT;
   if (AVM_EVT_NULL == avm_evt)
   {
      m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      m_MMGR_FREE_AVD_AVM_MSG(bam_avm);
      ncshm_give_hdl((uns32)cb_hdl);
      return NCSCC_RC_FAILURE;
   }

   avm_evt->src = AVM_EVT_BAM;   

   /* formulate the event for the received message */
   avm_evt->evt.bam_evt      = bam_avm; 
   
   /* put the event in AVM mail box */
   if(m_NCS_IPC_SEND(&avm_cb->mailbox, avm_evt, NCS_IPC_PRIORITY_HIGH)
                    == NCSCC_RC_FAILURE)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVM_EVT(avm_evt); 
      ncshm_give_hdl((uns32)cb_hdl);
      rc = NCSCC_RC_FAILURE;
   }

   /* return AVM CB */
   ncshm_give_hdl((uns32)cb_hdl);
   
   return rc;
}

/**********************************************************************
 * Name          : avm_mds_cb_rcv
 *
 * Description   : This is a callback routine that is invoked when msg
   is received at AVM. This function simply posts the ms
 *                 to AvM mailbox .
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 avm_mds_rcv (struct ncsmds_callback_info *mds_cb_info)
{
   uns32          rc = NCSCC_RC_SUCCESS;

   if(NULL == mds_cb_info->info.receive.i_msg)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;
   }

   if ((mds_cb_info->info.receive.i_fr_svc_id == NCSMDS_SVC_ID_AVD) &&
       (mds_cb_info->info.receive.i_to_svc_id == NCSMDS_SVC_ID_AVM))
   {
     rc = avm_avd_msg_rcv(mds_cb_info->i_yr_svc_hdl, (AVD_AVM_MSG_T *)mds_cb_info->info.receive.i_msg);
     mds_cb_info->info.receive.i_msg = (NCSCONTEXT)0x0;
     if(NCSCC_RC_SUCCESS != rc)
     {
        return rc;
     }
  }else if ((mds_cb_info->info.receive.i_fr_svc_id == NCSMDS_SVC_ID_BAM) &&
             (mds_cb_info->info.receive.i_to_svc_id == NCSMDS_SVC_ID_AVM))
   {
       rc = avm_bam_msg_rcv(mds_cb_info->i_yr_svc_hdl, (BAM_AVM_MSG_T *)mds_cb_info->info.receive.i_msg);
      mds_cb_info->info.receive.i_msg = (NCSCONTEXT)0x0;
      if(NCSCC_RC_SUCCESS != rc)
      {
         return rc;
      }
   }else
#if (MOT_ATCA == 1)
       if ((mds_cb_info->info.receive.i_fr_svc_id == NCSMDS_SVC_ID_LFM) &&
           (mds_cb_info->info.receive.i_to_svc_id == NCSMDS_SVC_ID_AVM))
   {

      rc = avm_lfm_msg_rcv(mds_cb_info->i_yr_svc_hdl, 
                        mds_cb_info->info.receive.i_fr_dest,
                        (LFM_AVM_MSG *)mds_cb_info->info.receive.i_msg,
                        (MDS_SYNC_SND_CTXT *) &mds_cb_info->info.receive.i_msg_ctxt);

      mds_cb_info->info.receive.i_msg = (NCSCONTEXT)0x0;
      if(NCSCC_RC_SUCCESS != rc)
      {
         return rc;
      }
   }else
#endif
   {
      /* Loge Here */
         mds_cb_info->info.receive.i_msg = NULL;
         return NCSCC_RC_FAILURE;
   }   

   mds_cb_info->info.receive.i_msg = NULL;
   return rc;
}

/***********************************************************************
 * Name          : avm_mds_svc_event
 *
 * Description   : This is a callback routine that is invoked to inform
 *                 AVM that AVD hs come up i.e MDS connection has been
 *   established with AVD.
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 avm_mds_svc_event (struct ncsmds_callback_info *mds_cb_info)
{
   MDS_CLIENT_HDL  cb_hdl;
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_CB_T *cb;

   /* Retrieve the HAM_CB hdl */
   cb_hdl = mds_cb_info->i_yr_svc_hdl;

   /* First make sure that this event is indeed for us*/
   if (mds_cb_info->info.svc_evt.i_your_id != NCSMDS_SVC_ID_AVM)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(mds_cb_info->info.svc_evt.i_your_id);
      return NCSCC_RC_FAILURE;
   }

   /* Take the AVM Control Block */
   if((cb = (AVM_CB_T*)ncshm_take_hdl(NCS_SERVICE_ID_AVM, (uns32)cb_hdl)) == NULL)
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   switch (mds_cb_info->info.svc_evt.i_change)
   {
       case NCSMDS_DOWN:
       {
         m_AVM_LOG_DEBUG("MDS down", NCSFL_SEV_INFO);
       }
       break;

       case NCSMDS_UP:
       switch(mds_cb_info->info.svc_evt.i_svc_id)
       {
           case NCSMDS_SVC_ID_AVD:
           {
              m_AVM_LOG_DEBUG("MDS up and Rcvd AVD ack", NCSFL_SEV_INFO);
              
              if(m_NCS_MDS_DEST_EQUAL(&cb->adest, &mds_cb_info->info.svc_evt.i_dest))
              {
                 avm_mds_avd_up(mds_cb_info);
              }   

              /* cb->avd_dest = mds_cb_info->info.svc_evt.i_dest; */
           }
           break;

#if (MOT_ATCA == 1)
           case NCSMDS_SVC_ID_LFM:
           {
              cb->is_platform = TRUE;
           }
           break;
#endif
           default:
           break;
       }
       break;

       default:
       break;
   } 

   m_AVM_LOG_MDS(AVM_LOG_MDS_SVEVT_CBK, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

   /* Give the hdl back */
   ncshm_give_hdl((uns32)cb_hdl);
   return rc;
}

static uns32
avm_mds_quiesced_ack(struct ncsmds_callback_info *mds_cb_info)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    MDS_CLIENT_HDL cb_hdl;

    AVM_CB_T  *avm_cb =  NULL;
    AVM_EVT_T *avm_evt;

   /* Retrieve the AvM_CB hdl */
   cb_hdl = mds_cb_info->i_yr_svc_hdl;

   /* retrieve AVM CB */
   if (NULL == (avm_cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM,
                (uns32)cb_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL); 
      return NCSCC_RC_FAILURE;
   }

   avm_evt = m_MMGR_ALLOC_AVM_EVT;
   if (AVM_EVT_NULL == avm_evt)
   {
      m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      ncshm_give_hdl((uns32)cb_hdl);
      return NCSCC_RC_FAILURE;
   }

   avm_evt->src          = AVM_EVT_MDS;
   avm_evt->fsm_evt_type = AVM_ROLE_EVT_MDS_QUIESCED_ACK; 

   /* put the event in AVM mail box */
   if(m_NCS_IPC_SEND(&avm_cb->mailbox, avm_evt, NCS_IPC_PRIORITY_HIGH)
                    == NCSCC_RC_FAILURE)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVM_EVT(avm_evt); 
      ncshm_give_hdl((uns32)cb_hdl);
      rc = NCSCC_RC_FAILURE;
   }

   /* Give the hdl back */
   ncshm_give_hdl((uns32)cb_hdl);

   return rc;
}

static uns32
avm_mds_avd_up(struct ncsmds_callback_info *mds_cb_info)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    MDS_CLIENT_HDL cb_hdl;

    AVM_CB_T  *avm_cb =  NULL;
    AVM_EVT_T *avm_evt;

   /* Retrieve the AvM_CB hdl */
   cb_hdl = mds_cb_info->i_yr_svc_hdl;

   /* retrieve AVM CB */
   if (NULL == (avm_cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM,
                (uns32)cb_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL); 
      return NCSCC_RC_FAILURE;
   }

   avm_evt = m_MMGR_ALLOC_AVM_EVT;
   if (AVM_EVT_NULL == avm_evt)
   {
      m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      ncshm_give_hdl((uns32)cb_hdl);
      return NCSCC_RC_FAILURE;
   }

   avm_evt->src          = AVM_EVT_MDS;
   avm_evt->fsm_evt_type = AVM_ROLE_EVT_AVD_UP; 

   /* put the event in AVM mail box */
   if(m_NCS_IPC_SEND(&avm_cb->mailbox, avm_evt, NCS_IPC_PRIORITY_HIGH)
                    == NCSCC_RC_FAILURE)
   {
      m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVM_EVT(avm_evt); 
      ncshm_give_hdl((uns32)cb_hdl);
      rc = NCSCC_RC_FAILURE;
   }

   /* Give the hdl back */
   ncshm_give_hdl((uns32)cb_hdl);

   return rc;
}

/*****************************************************************
 * Name          : avm_mds_set_vdest_role
 *
 * Description   : AvM function to set Vdest role 
 *
 * Arguments     : cb    : AvM control Block pointer.
 *                 role  : role given to Vdest 
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 ******************************************************************/
extern uns32
avm_mds_set_vdest_role(AVM_CB_T       *cb,
                       SaAmfHAStateT   role  
                      )
{
    NCSVDA_INFO vda_info;

    /* Assign Role Now */
    m_NCS_MEMSET(&vda_info, '\0', sizeof(vda_info));

    vda_info.req = NCSVDA_VDEST_CHG_ROLE;
    vda_info.info.vdest_chg_role.i_vdest = cb->vaddr;
    vda_info.info.vdest_chg_role.i_new_role = role; /* for now */

    if(ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
    {
       m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_ROL, AVM_LOG_MDS_FAILURE, NCSFL_SEV_EMERGENCY);
       return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;  
}

/*****************************************************************
 * Name          : avm_mds_register_adest
 *
 * Description   : AvM function to regisetr on adest 
 *
 * Arguments     : cb    : AvM control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 ******************************************************************/
static uns32
avm_mds_register_adest(AVM_CB_T *avm_cb)
{
    NCSADA_INFO ada_info;
    NCSMDS_INFO mds_info;  
    uns32 rc = NCSCC_RC_SUCCESS;

    m_NCS_MEMSET(&ada_info, '\0', sizeof(NCSADA_INFO));

    ada_info.req = NCSADA_GET_HDLS;
    ada_info.info.adest_get_hdls.i_create_oac = FALSE;

    if (NCSCC_RC_SUCCESS != (rc =ncsada_api(&ada_info)))
    {
        m_AVM_LOG_MDS(AVM_LOG_MDS_ADEST_HDL, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL); 
        return rc;
    }

    /* Store the info returned by MDS */
    avm_cb->adest_pwe_hdl =  ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
    avm_cb->adest         =  ada_info.info.adest_get_hdls.o_adest;

    m_NCS_OS_MEMSET(&mds_info, 0, sizeof(mds_info));
   
    mds_info.i_mds_hdl = avm_cb->adest_pwe_hdl;
    mds_info.i_svc_id   = NCSMDS_SVC_ID_AVM;  
    mds_info.i_op = MDS_INSTALL;
    mds_info.info.svc_install.i_mds_q_ownership = FALSE;
    mds_info.info.svc_install.i_svc_cb = avm_mds_callback;
    mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)avm_cb->cb_hdl;
    mds_info.info.svc_install.i_mds_svc_pvt_ver = AVM_MDS_SUB_PART_VERSION;
   
    mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
    if(ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
    {
      return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : avm_mds_vdest_create
 *
 * Description   : This function creats the MDS VDEST for AVM
 *
 * Arguments     : avm_cb - pointer AVM control block structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
avm_mds_vdest_create(AVM_CB_T *avm_cb)
{
    NCSVDA_INFO vda_info;
    uns32 rc = NCSCC_RC_SUCCESS;

    m_NCS_MEMSET(&vda_info, '\0', sizeof(NCSVDA_INFO));

    /* prepare the cb with the vaddress */
    m_NCS_SET_VDEST_ID_IN_MDS_DEST(avm_cb->vaddr, AVM_VDEST_ID);

    vda_info.req = NCSVDA_VDEST_CREATE;
    vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
    vda_info.info.vdest_create.i_create_oac = TRUE;
    vda_info.info.vdest_create.i_persistent = FALSE;
    vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;

    vda_info.info.vdest_create.info.specified.i_vdest = avm_cb->vaddr;

    /* Create the VDEST address */
    if (NCSCC_RC_SUCCESS != (rc =ncsvda_api(&vda_info)))
    {
        m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_CRT, AVM_LOG_MDS_FAILURE, NCSFL_SEV_EMERGENCY); 
        return rc;
    }

    m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_CRT, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO); 
    /* Store the info returned by MDS */
  
    avm_cb->mab_hdl       =  vda_info.info.vdest_create.o_pwe1_oac_hdl; 
    avm_cb->vaddr_pwe_hdl =  vda_info.info.vdest_create.o_mds_pwe1_hdl;
    avm_cb->vaddr_hdl     =  vda_info.info.vdest_create.o_mds_vdest_hdl;

    if(NCSCC_RC_SUCCESS != avm_mds_set_vdest_role(avm_cb, avm_cb->ha_state))
    {
       m_NCS_MEMSET(&vda_info,'\0',sizeof(NCSVDA_INFO));
       vda_info.req = NCSVDA_VDEST_DESTROY;
       vda_info.info.vdest_destroy.i_vdest = avm_cb->vaddr;

       ncsvda_api(&vda_info);
       m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_ROL, AVM_LOG_MDS_FAILURE, NCSFL_SEV_EMERGENCY);
       return NCSCC_RC_FAILURE;
    }   

    m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_ROL, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************
 * Name          : avm_mds_register_vdest
 *
 * Description   : AvM function to register on vdest 
 *
 * Arguments     : cb    : AvM control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 ******************************************************************/
static uns32
avm_mds_register_vdest(AVM_CB_T *avm_cb)
{
   
    uns32 rc = NCSCC_RC_SUCCESS;
    NCSMDS_INFO  mds_info;
   

    /* Create the VDEST for HAM */
    if (NCSCC_RC_SUCCESS != (rc = avm_mds_vdest_create(avm_cb)))
    {
       m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_CRT, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
       return rc;
    }  
    m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_CRT, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

    m_NCS_OS_MEMSET(&mds_info, 0, sizeof(mds_info));

    /* Install your service into MDS */
    m_NCS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));
    mds_info.i_mds_hdl        = avm_cb->vaddr_pwe_hdl;
    mds_info.i_svc_id         = NCSMDS_SVC_ID_AVM;
    mds_info.i_op             = MDS_INSTALL;

    mds_info.info.svc_install.i_yr_svc_hdl      = (MDS_CLIENT_HDL)avm_cb->cb_hdl;
    mds_info.info.svc_install.i_install_scope   = NCSMDS_SCOPE_NONE;
    mds_info.info.svc_install.i_svc_cb          = avm_mds_callback;
    mds_info.info.svc_install.i_mds_q_ownership = FALSE;
    mds_info.info.svc_install.i_mds_svc_pvt_ver = AVM_MDS_SUB_PART_VERSION;

    if (NCSCC_RC_SUCCESS != (rc = ncsmds_api(&mds_info)))
    {
        m_AVM_LOG_MDS(AVM_LOG_MDS_INSTALL, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL); 
        return rc;
    }
    m_AVM_LOG_MDS(AVM_LOG_MDS_INSTALL, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO); 
    return rc;
}

/***********************************************************************
 * Name          : avm_mds_initialize
 *
 * Description   : This routine registers the AVM to MDS and assigns
 *                 the MDS callback functions.
 *
 * Arguments     : cb - ptr to the AVM control block structure
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 **********************************************************************/

uns32 avm_mds_initialize(AVM_CB_T *cb)
{
    NCSMDS_INFO  mds_info;
    uns32 rc =   NCSCC_RC_SUCCESS;
    MDS_SVC_ID   svc[1] = {0}; 

   if(NCSCC_RC_SUCCESS != (rc = avm_mds_register_adest(cb)))
   {
      m_AVM_LOG_MDS(AVM_LOG_MDS_ADEST_REG, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
      return rc;
   }

   if(NCSCC_RC_SUCCESS != (rc = avm_mds_register_vdest(cb)))
   {
      m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_REG, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
      return rc;
   }

    /* Now subscribe for AVM events in MDS */
    m_NCS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl        = cb->adest_pwe_hdl;
    mds_info.i_svc_id         = NCSMDS_SVC_ID_AVM;
    mds_info.i_op             = MDS_SUBSCRIBE;

    mds_info.info.svc_subscribe.i_scope         = NCSMDS_SCOPE_INTRANODE;
    mds_info.info.svc_subscribe.i_num_svcs      = 1;
    mds_info.info.svc_subscribe.i_svc_ids       = svc;

    svc[0] = NCSMDS_SVC_ID_AVD;

    /* register to MDS */
    rc = ncsmds_api(&mds_info);
    if (rc != NCSCC_RC_SUCCESS)
    {
        m_AVM_LOG_MDS(AVM_LOG_MDS_SUBSCRIBE, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL); 
        return rc;
    }

#if (MOT_ATCA == 1)
    /* Now subscribe for AVM events in MDS */
    m_NCS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl        = cb->vaddr_pwe_hdl;
    mds_info.i_svc_id         = NCSMDS_SVC_ID_AVM;
    mds_info.i_op             = MDS_SUBSCRIBE;

    mds_info.info.svc_subscribe.i_scope         = NCSMDS_SCOPE_NONE;
    mds_info.info.svc_subscribe.i_num_svcs      = 1;
    mds_info.info.svc_subscribe.i_svc_ids       = svc;

    svc[0] = NCSMDS_SVC_ID_LFM;

    /* register to MDS */
    rc = ncsmds_api(&mds_info);
    if (rc != NCSCC_RC_SUCCESS)
    {
        m_AVM_LOG_MDS(AVM_LOG_MDS_SUBSCRIBE, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL); 
        return rc;
    }
#endif

    m_AVM_LOG_MDS(AVM_LOG_MDS_SUBSCRIBE, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_INFO); 
    return rc;
}

/***********************************************************************
 * Name          : avm_mds_vdest_destroy
 *
 * Description   : This function Destroys the Virtual destination of AV 
 *
 * Arguments     : cb   : pointer to AVM control block structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 **********************************************************************/
static uns32
avm_mds_vdest_destroy (AVM_CB_T *cb)
{
   NCSVDA_INFO    vda_info;
   uns32          rc;

   m_NCS_MEMSET(&vda_info,'\0',sizeof(NCSVDA_INFO));

   vda_info.req                             = NCSVDA_VDEST_DESTROY;
   vda_info.info.vdest_destroy.i_vdest      = cb->vaddr;

   /* destroy the AVM VDEST */
   if(NCSCC_RC_SUCCESS != ( rc = ncsvda_api(&vda_info)))
   {
      m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_DESTROY, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
      return rc;
   }
   m_AVM_LOG_MDS(AVM_LOG_MDS_VDEST_DESTROY, AVM_LOG_MDS_SUCCESS, NCSFL_SEV_CRITICAL);
   return rc;
}


/***********************************************************************
 * Name          : avm_mds_finalize
 *
 * Description   : This function un-registers the AVM Service with MDS.
 *
 * Arguments     : cb - pointer to AVM control block structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 **********************************************************************/

uns32 avm_mds_finalize (AVM_CB_T *cb)
{
   NCSMDS_INFO          mds_info;
   uns32                rc;

   m_AVM_LOG_FUNC_ENTRY("avm_mds_finalize");

   /* Un-install AVM service from MDS */
   m_NCS_OS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl        = cb->vaddr_pwe_hdl;
   mds_info.i_svc_id         = NCSMDS_SVC_ID_AVM;
   mds_info.i_op             = MDS_UNINSTALL;

   /* request to un-install from the MDS service */
   rc = ncsmds_api(&mds_info);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_MDS(AVM_LOG_MDS_UNINSTALL, AVM_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
      return rc;
   }

   /* Destroy the virtual Destination of AVM */
   rc = avm_mds_vdest_destroy(cb);
   return rc;
 }


/***********************************************************************
 * Name          : avm_mds_msg_send
 *
 * Description   : 
 *
 * Arguments     : cb  - ptr to the AVM CB
 *                 i_msg - ptr to the AvM message
 *                 dest  - MDS destination to send to (AVD).
 *                 prio - MDS priority of this message
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/timeout
 *
 * Notes         : None.
 **********************************************************************/

extern uns32
avm_mds_msg_send(
                  AVM_CB_T                *cb, 
                  AVM_AVD_MSG_T           *msg, 
                  MDS_DEST                *dest,
                  MDS_SEND_PRIORITY_TYPE  prio
                )
{
    NCSMDS_INFO   mds_info;
    MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
    uns32         rc = NCSCC_RC_SUCCESS;
    MDS_SENDTYPE_SND_INFO *send;

    /* populate the mds params */
    m_NCS_MEMSET(&mds_info, '\0', sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl = cb->adest_pwe_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_AVM;
    mds_info.i_op = MDS_SEND;

    send_info->i_msg    = msg;
    send_info->i_to_svc = NCSMDS_SVC_ID_AVD;
    send_info->i_priority = prio;

    send = &send_info->info.snd;
    send_info->i_sendtype = MDS_SENDTYPE_SND;
    send->i_to_dest = *dest;

    /* send the message */
    rc = ncsmds_api(&mds_info);

    return rc;
 }
