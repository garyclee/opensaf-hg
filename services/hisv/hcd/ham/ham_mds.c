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
*                                                                            *
*  MODULE NAME:  ham_mds.c                                                   *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This file contains routines used by HAM library for MDS interaction in    *
*  order to communicate with HCD. MDS callback routines and other routines   *
*  used to send or receive the message over MDS are defined in the file      *
*                                                                            *
*****************************************************************************/

#include "hcd.h"

/* MDS callback function declaration */
static uns32 ham_mds_callback(struct ncsmds_callback_info *info);
static uns32 ham_mds_cb_cpy      (struct ncsmds_callback_info *info);
static uns32 ham_mds_cb_enc      (struct ncsmds_callback_info *info);
static uns32 ham_mds_cb_dec      (struct ncsmds_callback_info *info);
static uns32 ham_mds_cb_enc_flat (struct ncsmds_callback_info *info);
static uns32 ham_mds_cb_dec_flat (struct ncsmds_callback_info *info);
static uns32 ham_mds_cb_rcv      (struct ncsmds_callback_info *info);
static uns32 ham_mds_svc_event   (struct ncsmds_callback_info *info);
static uns32 ham_mds_sys_event         (struct ncsmds_callback_info *info);
static uns32 ham_mds_quiesced_event    (struct ncsmds_callback_info *info);
static uns32 ham_mds_dir_rec_event     (struct ncsmds_callback_info *info);

/* routines create and destroy MDS VDEST */
static uns32 ham_mds_vdest_create(HAM_CB *ham_cb);
static uns32 ham_mds_vdest_destroy (HAM_CB *ham_cb);
/* function to put the received event to HAM mailbox */
static uns32 hpl_evt_mbx_put(HAM_CB *ham_cb, struct ncsmds_callback_info *mds_cb_info);

const MDS_CLIENT_MSG_FORMAT_VER hcd_hpl_msg_fmt_map_table[HCD_HPL_SUB_PART_VER_MAX] = {HISV_MSG_FMT_VER};

/****************************************************************************
 * Name          : ham_mds_callback
 *
 * Description   : Callback Dispatcher for various MDS operations.
 *
 * Arguments     : cb   : HAM-MDS control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 ham_mds_callback(struct ncsmds_callback_info *info)
{
   static NCSMDS_CALLBACK_API cb_set[MDS_CALLBACK_SVC_MAX] = {
         ham_mds_cb_cpy,          /* MDS_CALLBACK_COPY      */
         ham_mds_cb_enc,          /* MDS_CALLBACK_ENC       */
         ham_mds_cb_dec,          /* MDS_CALLBACK_DEC       */
         ham_mds_cb_enc_flat,     /* MDS_CALLBACK_ENC_FLAT  */
         ham_mds_cb_dec_flat,     /* MDS_CALLBACK_DEC_FLAT  */
         ham_mds_cb_rcv,          /* MDS_CALLBACK_RECEIVE   */
         ham_mds_svc_event,       /* MDS_CALLBACK_SVC_EVENT */
         ham_mds_sys_event,       /* MDS_CALLBACK_SYS_EVENT */
         ham_mds_quiesced_event,  /* MDS_CALLBACK_QUIESCED_ACK */
         ham_mds_dir_rec_event    /* MDS_CALLBACK_DIRECT_RECEIVE */
   };

   if (info->i_op <= MDS_CALLBACK_DIRECT_RECEIVE)
      return (*cb_set[info->i_op])(info);
   else
   {
      m_NCS_CONS_PRINTF("ham_mds_callback: info->i_op is beyond MDS_CALLBACK_SVC_MAX\n");
      return NCSCC_RC_FAILURE;
   }
}


/****************************************************************************
 * Name          : ham_mds_cb_cpy
 *
 * Description   : This function copies an events received.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 ham_mds_cb_cpy(struct ncsmds_callback_info *info)
{
   HISV_MSG *msg, *hisv_msg;
   info->info.cpy.o_msg_fmt_ver = HISV_MSG_FMT_VER;
   /* get the message */
   msg = (HISV_MSG*)info->info.cpy.i_msg;

   if (NULL == (hisv_msg = m_MMGR_ALLOC_HISV_MSG))
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_cpy: m_MMGR_ALLOC_HISV_MSG failed\n");
      return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMCPY(hisv_msg, msg, sizeof(HISV_MSG));
   info->info.cpy.o_cpy = (uns8*)hisv_msg;
   return NCSCC_RC_SUCCESS;
}



/****************************************************************************
 * Name          : ham_mds_cb_enc
 *
 * Description   : This is a callback routine that is invoked to encode the
 *                 messages being sent to HPL ADEST. HAM sends its chassis-id
 *                 to HPL ADEST and the message is encoded here for sending
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 ham_mds_cb_enc (struct ncsmds_callback_info *info)
{
   HISV_MSG        *msg;
   NCS_UBAID       *uba;
   uns8            *p8;
   int32           total_bytes = 0;
   uns32           data_len = sizeof(MDS_DEST);

  if(NCSMDS_SVC_ID_HPL == info->info.enc.i_to_svc_id )
  {
    info->info.enc.o_msg_fmt_ver =  m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
                                   HCD_HPL_SUB_PART_VER_MIN,
                                   HCD_HPL_SUB_PART_VER_MAX,
                                   hcd_hpl_msg_fmt_map_table);


    if(info->info.enc.o_msg_fmt_ver  < HISV_MSG_FMT_VER)
    {
      m_LOG_HISV_DTS_CONS("Mds Encode fails :Message being sent to incompatiable version \n");
      return NCSCC_RC_FAILURE;
    }
  } 
 
   /* get the message */
   msg = (HISV_MSG*)info->info.enc.i_msg;
   uba = info->info.enc.io_uba;

   if ((NULL == uba) || (NULL == msg))
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_enc: error uba\n");
      return NCSCC_RC_FAILURE;
   }
   /* encode the return type located in message */
   p8 = ncs_enc_reserve_space(uba, 4);
   ncs_encode_32bit(&p8, msg->info.cbk_info.ret_type);
   ncs_enc_claim_space(uba, 4);
   total_bytes += 4;

   if (msg->info.cbk_info.ret_type == HPL_HAM_VDEST_RET)
   {
      /* encode the chassis-id located in message */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_vdest.chassis_id);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;

      /* encode the HAM MDS VDEST value */
      ncs_encode_n_octets_in_uba(uba, (uns8 *)&msg->info.cbk_info.hpl_ret.h_vdest.ham_dest, (int32)data_len);
      total_bytes += data_len;

      /* encode HAM status value */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_vdest.ham_status);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;
   }
   else
   {
      /* encode the return value located in message */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_gen.ret_val);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;

      /* encode the data_len located in message */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_gen.data_len);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;

      data_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;
      /* encode the HAM MDS VDEST value */
      ncs_encode_n_octets_in_uba(uba, (uns8 *)msg->info.cbk_info.hpl_ret.h_gen.data, (int32)data_len);
      total_bytes += data_len;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ham_mds_cb_dec
 *
 * Description   : This is a callback routine that is invoked to decode the
 *                 received messages. HAM gets the request messages from HPL
 *                 and it decodes them here before further processing later.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_cb_dec (struct ncsmds_callback_info *info)
{
   uns8      *p8;
   HISV_EVT  *evt;
   NCS_UBAID *uba = info->info.dec.io_uba;
   uns8       local_data[20];
   uns32      data_len, total_bytes = 0;
 
  if(NCSMDS_SVC_ID_HPL ==info->info.dec.i_fr_svc_id)
  {
     if(!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
                                   HCD_HPL_SUB_PART_VER_MIN,
                                   HCD_HPL_SUB_PART_VER_MAX,
                                   hcd_hpl_msg_fmt_map_table))
    {
      m_LOG_HISV_DTS_CONS("Mds Decode fails: Message received from incompatiable version \n");
      return NCSCC_RC_FAILURE;
    }
  }                            
 
   /** allocate an HISV_EVENT now **/
   if (NULL == (evt = m_MMGR_ALLOC_HISV_EVT))
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_dec: m_MMGR_ALLOC_HISV_EVT failed\n");
      return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(evt, '\0', sizeof(HISV_EVT));

   /** Allocate a new msg in both sync/async cases
    **/
   info->info.dec.o_msg = (uns8*)evt;

   /* decode the HPI command received */
   p8  = ncs_dec_flatten_space(uba, local_data, 4);
   evt->msg.info.api_info.cmd = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   total_bytes += 4;

   /* decode the argument for HPI command */
   p8  = ncs_dec_flatten_space(uba, local_data, 4);
   evt->msg.info.api_info.arg = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   total_bytes += 4;

   /* decode the length of data present */
   p8  = ncs_dec_flatten_space(uba, local_data, 4);
   evt->msg.info.api_info.data_len = ncs_decode_32bit(&p8);
   data_len = evt->msg.info.api_info.data_len;
   ncs_dec_skip_space(uba, 4);
   total_bytes += 4;

   if (data_len > 0)
   {
      /* if data is present, it contains entity path string. decode it */
      evt->msg.info.api_info.data = m_MMGR_ALLOC_HISV_DATA(data_len);
      if (NULL == evt->msg.info.api_info.data)
      {
         m_LOG_HISV_DTS_CONS("ham_mds_cb_dec: m_MMGR_ALLOC_HISV_DATA failed\n");
         m_MMGR_FREE_HISV_EVT(evt);
         return NCSCC_RC_FAILURE;
      }
      ncs_decode_n_octets_from_uba(uba, evt->msg.info.api_info.data, data_len);
      total_bytes += data_len;
   }
   else
   {
      /* command does not need data */
      evt->msg.info.api_info.data_len = 0;
      evt->msg.info.api_info.data = NULL;
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ham_mds_cb_enc_flat
 *
 * Description   : MDS encode and flatten
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_cb_enc_flat(struct ncsmds_callback_info *info)
{
   HISV_MSG        *msg;
   NCS_UBAID       *uba;
   uns8            *p8;
   int32           total_bytes = 0;
   uns32           data_len = sizeof(MDS_DEST);


  if(NCSMDS_SVC_ID_HPL == info->info.enc.i_to_svc_id )
  {
   info->info.enc.o_msg_fmt_ver =  m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
                                   HCD_HPL_SUB_PART_VER_MIN,
                                   HCD_HPL_SUB_PART_VER_MAX,
                                   hcd_hpl_msg_fmt_map_table);


    if(info->info.enc.o_msg_fmt_ver  < HISV_MSG_FMT_VER)
    {
      m_LOG_HISV_DTS_CONS("Mds Encode fails :Message being sent to incompatiable version \n");
      return NCSCC_RC_FAILURE;

    }
  }
 
   /* get the message */
   msg = (HISV_MSG*)info->info.enc.i_msg;
   uba = info->info.enc.io_uba;

   if ((NULL == uba) || (NULL == msg))
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_enc_flat: uba error\n");
      return NCSCC_RC_FAILURE;
   }
   /* encode the return type located in message */
   p8 = ncs_enc_reserve_space(uba, 4);
   ncs_encode_32bit(&p8, msg->info.cbk_info.ret_type);
   ncs_enc_claim_space(uba, 4);
   total_bytes += 4;

   if (msg->info.cbk_info.ret_type == HPL_HAM_VDEST_RET)
   {
      /* encode the chassis-id located in message */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_vdest.chassis_id);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;

      /* encode the HAM MDS VDEST value */
      ncs_encode_n_octets_in_uba(uba, (uns8 *)&msg->info.cbk_info.hpl_ret.h_vdest.ham_dest, (int32)data_len);
      total_bytes += data_len;

      /* encode HAM status value */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_vdest.ham_status);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;
   }
   else
   {
      /* encode the return value located in message */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_gen.ret_val);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;

      /* encode the data_len located in message */
      p8 = ncs_enc_reserve_space(uba, 4);
      ncs_encode_32bit(&p8, msg->info.cbk_info.hpl_ret.h_gen.data_len);
      ncs_enc_claim_space(uba, 4);
      total_bytes += 4;

      data_len = msg->info.cbk_info.hpl_ret.h_gen.data_len;
      /* encode the HAM MDS VDEST value */
      ncs_encode_n_octets_in_uba(uba, (uns8 *)msg->info.cbk_info.hpl_ret.h_gen.data, (int32)data_len);
      total_bytes += data_len;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ham_mds_cb_dec_flat
 *
 * Description   : MDS decode and flatten
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_cb_dec_flat(struct ncsmds_callback_info *info)
{
   HISV_EVT  *evt;
   HISV_MSG  *msg;
   NCS_UBAID *uba = info->info.dec_flat.io_uba;
   uns32      data_len;

   if (NCSMDS_SVC_ID_HPL != info->info.dec_flat.i_fr_svc_id)
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_dec_flat: invalid service-id in ham_mds_cb_dec_flat\n");
      return NCSCC_RC_FAILURE;
   }

   if(!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec_flat.i_msg_fmt_ver,
                                   HCD_HPL_SUB_PART_VER_MIN,
                                   HCD_HPL_SUB_PART_VER_MAX,
                                   hcd_hpl_msg_fmt_map_table))
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_dec_flat fails :Message received from incompatiable version \n");
      return NCSCC_RC_FAILURE;
      /* Drop the msg if the msg fmt version does not match */
   }                            

   /** allocate an HISV_EVENT now **/
   if (NULL == (evt = m_MMGR_ALLOC_HISV_EVT))
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_dec_flat: m_MMGR_ALLOC_HISV_EVT failed\n");
      return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(evt, '\0', sizeof(HISV_EVT));

   /** Allocate a new msg in both sync/async cases
    **/
   info->info.dec_flat.o_msg = (uns8*)evt;
   msg = &evt->msg;
   ncs_decode_n_octets(uba->ub,(uns8*)msg,sizeof(HISV_MSG));
   data_len = msg->info.api_info.data_len;

   if (data_len > 0)
   {
      /* if data is present, it contains entity path string. decode it */
      msg->info.api_info.data = m_MMGR_ALLOC_HISV_DATA(data_len);

      if (NULL == msg->info.api_info.data)
      {
         m_LOG_HISV_DTS_CONS("ham_mds_cb_dec_flat: m_MMGR_ALLOC_HISV_DATA failed\n");
         m_MMGR_FREE_HISV_EVT(evt);
         return NCSCC_RC_FAILURE;
      }
      ncs_decode_n_octets(uba->ub, msg->info.api_info.data, data_len);
   }
   else
   {
      /* command does not need data */
      msg->info.api_info.data_len = 0;
      msg->info.api_info.data = NULL;
   }
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : ham_mds_cb_rcv
 *
 * Description   : This is a callback routine that is invoked when message
 *                 is received. HAM receives the request message from HPL and
 *                 it is processed here after decoding. It sends the received
 *                 message as an event to HAM mail-box, for further processing
 *                 to be done by HAM thread.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 ham_mds_cb_rcv (struct ncsmds_callback_info *mds_cb_info)
{
   HISV_EVT *hisv_evt = (HISV_EVT *)mds_cb_info->info.receive.i_msg;
   HAM_CB   *ham_cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* retrieve HAM CB */
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD,
                (uns32) mds_cb_info->i_yr_svc_hdl)))
   {
      hisv_evt_destroy(hisv_evt);
      mds_cb_info->info.receive.i_msg = NULL;
      rc = NCSCC_RC_FAILURE;
      m_LOG_HISV_DTS_CONS("ham_mds_cb_rcv: error\n");
      return rc;
   }
   /* formulate the event for the received message */
   hisv_evt->cb_hdl        = (uns32)mds_cb_info->i_yr_svc_hdl;
   hisv_evt->fr_node_id    = mds_cb_info->info.receive.i_node_id;
   hisv_evt->fr_dest       = mds_cb_info->info.receive.i_fr_dest;
   hisv_evt->rcvd_prio     = mds_cb_info->info.receive.i_priority;
   hisv_evt->mds_ctxt      = mds_cb_info->info.receive.i_msg_ctxt;
   hisv_evt->to_svc        = mds_cb_info->info.receive.i_fr_svc_id;

   /** The HISV_MSG has already been decoded into the event earlier
    ** put the event in HAM mail box.
    **/
   if(m_NCS_IPC_SEND(&ham_cb->mbx, hisv_evt, mds_cb_info->info.receive.i_priority)
                     == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("ham_mds_cb_rcv: failed to deliver msg on mail-box");
      hisv_evt_destroy(hisv_evt);
      rc = NCSCC_RC_FAILURE;
   }
   mds_cb_info->info.receive.i_msg = NULL;

   /* return HAM CB */
   ncshm_give_hdl((uns32)mds_cb_info->i_yr_svc_hdl);

   return rc;
}


/****************************************************************************
 * Name          : ham_mds_svc_event
 *
 * Description   : This is a callback routine that is invoked to inform HAM
 *                 of MDS events. HAM had subscribed to these events through
 *                 MDS subscription.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 ham_mds_svc_event (struct ncsmds_callback_info *mds_cb_info)
{
   uns32   ham_mds_hdl;
   uns32 rc = NCSCC_RC_SUCCESS;
   HAM_CB *ham_cb;

   /* Retrieve the HAM_CB hdl */
   ham_mds_hdl = (uns32) mds_cb_info->i_yr_svc_hdl;

   m_LOG_HISV_DTS_CONS("ham_mds_svc_event: HAM Rcvd MDS subscribe evt from HPL\n");
   /* mds_cb_info->info.svc_evt.i_svc_id; */

   /* First make sure that this event is indeed for us*/
   if (mds_cb_info->info.svc_evt.i_your_id != NCSMDS_SVC_ID_HCD)
   {
      m_LOG_HISV_DTS_CONS("ham_mds_svc_event: HAM received unknown MDS event\n");
      /* mds_cb_info->info.svc_evt.i_your_id; */
      return NCSCC_RC_FAILURE;
   }

   /* Take the HAM hdl */
   if((ham_cb = (HAM_CB*)ncshm_take_hdl(NCS_SERVICE_ID_HCD, ham_mds_hdl)) == NULL)
   {
      /* HISV_ADD_LOG_HERE- MDS Callback error */
      m_LOG_HISV_DTS_CONS("ham_mds_svc_event: call back error\n");
      return NCSCC_RC_SUCCESS;
   }

   switch (mds_cb_info->info.svc_evt.i_change)
   {
      case NCSMDS_DOWN:
      if(mds_cb_info->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_HPL)
      {
         /** If HPL registered MDS goes down?
          **/
      }
      break;
      case NCSMDS_UP:
         switch(mds_cb_info->info.svc_evt.i_svc_id)
         {
            case NCSMDS_SVC_ID_HPL:
               /** HPL registered MDS is UP, send our chassis_id to it.
                **/
               ham_cb->hpl_dest = mds_cb_info->info.svc_evt.i_dest;
               /** when HPL is registered to MDS, HAM sends its chassis-id
                ** to its ADEST
                **/
               /* m_LOG_HISV_DTS_CONS("ham_mds_svc_event: Putting in event mbx\n"); */
               hpl_evt_mbx_put(ham_cb, mds_cb_info);
               break;
            default:
               break;
         }
         break;
      default:
         break;
   }
   /* Give the hdl back */
   ncshm_give_hdl((uns32)ham_mds_hdl);
   return rc;
}


/****************************************************************************
 * Name          : ham_mds_sys_event
 *
 * Description   : MDS sys event
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_sys_event(struct ncsmds_callback_info *info)
{
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : ham_mds_quiesced_event
 *
 * Description   : MDS quiesced event
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_quiesced_event(struct ncsmds_callback_info *info)
{
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : ham_mds_dir_rec_event
 *
 * Description   : MDS direct receive event
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_dir_rec_event(struct ncsmds_callback_info *info)
{
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_evt_mbx_put
 *
 * Description   : This function puts the request event on HAM mail-box, to
 *                 request the HAM send its chassis-id to HPL ADEST. This
 *                 function creates the request and sends it to HAM mailbox.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
hpl_evt_mbx_put(HAM_CB *ham_cb, struct ncsmds_callback_info *mds_cb_info)
{
   HISV_EVT * hisv_evt;
   HISV_MSG * msg;
   uns32 data_len = sizeof(MDS_DEST);

   /* allocate the event */
   if (NULL == (hisv_evt = m_MMGR_ALLOC_HISV_EVT))
   {
      m_LOG_HISV_DTS_CONS("hpl_evt_mbx_put: memory error\n");
      return NCSCC_RC_FAILURE;
   }
   msg = &hisv_evt->msg;

   /* allocate for data space to hold HAM VDEST in message */
   if (NULL == (msg->info.api_info.data = m_MMGR_ALLOC_HISV_DATA(data_len)))
   {
      m_LOG_HISV_DTS_CONS("hpl_evt_mbx_put: memory errors\n");
      m_MMGR_FREE_HISV_EVT(hisv_evt);
      return NCSCC_RC_FAILURE;
   }

   /* initialize MDS event attributes */
   hisv_evt->cb_hdl        = (uns32)mds_cb_info->i_yr_svc_hdl;
   hisv_evt->fr_node_id    = mds_cb_info->info.receive.i_node_id;
   hisv_evt->fr_dest       = mds_cb_info->info.receive.i_fr_dest;
   hisv_evt->rcvd_prio     = mds_cb_info->info.receive.i_priority;
   hisv_evt->mds_ctxt      = mds_cb_info->info.receive.i_msg_ctxt;
   hisv_evt->to_svc        = mds_cb_info->info.receive.i_fr_svc_id;

   /** populate the mds message with command to get the chassis-id
    ** of chassis managed by HAM.
    **/
   m_HAM_HISV_ENTITY_MSG_FILL(hisv_evt->msg, HISV_CHASSIS_ID_GET, 0,
                              data_len, &ham_cb->hpl_dest);

   /* send the request to HAM mailbox */
   if(m_NCS_IPC_SEND(&ham_cb->mbx, hisv_evt, NCS_IPC_PRIORITY_NORMAL) == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("hpl_evt_mbx_put: failed to deliver msg on mail-box\n");
      hisv_evt_destroy(hisv_evt);
      return (NCSCC_RC_FAILURE);
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hisv_evt_destroy
 *
 * Description   : Free the block of memory allocated for storing the
 *                 received event on MDS. Allocation generally done at
 *                 decode callback is freed here after the message event
 *                 processing is done.
 *
 * Arguments     : hisv_evt - pointer to HISV event and associate message.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32
hisv_evt_destroy(HISV_EVT *hisv_evt)
{
   HISV_MSG *msg;

   if (NULL == hisv_evt)
      return NCSCC_RC_FAILURE;
   msg = &hisv_evt->msg;

   /* free the data (if any) in the message */
   if (msg->info.api_info.data != NULL)
      m_MMGR_FREE_HISV_DATA(msg->info.api_info.data);

   /* free the event block allocated */
   m_MMGR_FREE_HISV_EVT(hisv_evt);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ham_mds_vdest_create
 *
 * Description   : This function creats the MDS VDEST for HAM
 *
 * Arguments     : ham_cb - pointer HAM control block structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_vdest_create(HAM_CB *ham_cb)
{
   NCSVDA_INFO vda_info;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&vda_info, '\0', sizeof(NCSVDA_INFO));

   ham_cb->ham_vdest = HISV_VDEST_ID;
   ham_cb->my_anc = V_DEST_QA_1;

   vda_info.req = NCSVDA_VDEST_CREATE;
   vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
   vda_info.info.vdest_create.i_create_oac = FALSE;/* To simplify */
   vda_info.info.vdest_create.i_persistent = TRUE; /* Up-to-the application */
   vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
   /* We are using fixed values here for now */
   vda_info.info.vdest_create.info.specified.i_vdest = ham_cb->ham_vdest;

   /* Create the VDEST address */
   if (NCSCC_RC_SUCCESS != (rc =ncsvda_api(&vda_info)))
   {
       /* LOG */
       m_LOG_HISV_DTS_CONS("ham_mds_vdest_create: error creating vdest\n");
       return rc;
   }
   /* Store the info returned by MDS */
   ham_cb->mds_hdl          = vda_info.info.vdest_create.o_mds_pwe1_hdl;
   ham_cb->mds_vdest_hdl    = vda_info.info.vdest_create.o_mds_vdest_hdl;

   return NCSCC_RC_SUCCESS;

   return rc;
}

/****************************************************************************
 * Name          : ham_mds_change_role
 *
 * Description   : This function informs mds of our vdest role change
 *
 * Arguments     : cb   : HAM control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 ham_mds_change_role(HAM_CB *cb)
{
   NCSVDA_INFO arg;

   m_NCS_OS_MEMSET(&arg, 0, sizeof(NCSVDA_INFO));

   arg.req = NCSVDA_VDEST_CHG_ROLE;
   arg.info.vdest_chg_role.i_vdest = cb->ham_vdest;
   arg.info.vdest_chg_role.i_new_role = cb->mds_role;

   if (ncsvda_api(&arg) != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DTS_CONS("ham_mds_change_role: error changing role\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : ham_mds_initialize
 *
 * Description   : This routine registers the HISv-HAM to MDS and assigns
 *                 the MDS callback functions. HAM registers to MDS on a
 *                 virtual address and subscribes to receive MDS events
 *                 from MDS registration of HPL.
 *
 * Arguments     : cb - ptr to the HAM control block structure
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ham_mds_initialize(HAM_CB *ham_cb)
{
   NCSMDS_INFO  mds_info;
   uns32 rc =   NCSCC_RC_SUCCESS;
   MDS_SVC_ID   svc = NCSMDS_SVC_ID_HPL;

   /* Create the VDEST for HAM */
   if (NCSCC_RC_SUCCESS != (rc = ham_mds_vdest_create(ham_cb)))
      return rc;

   m_NCS_OS_MEMSET(&mds_info, 0, sizeof(mds_info));

   /* Install your service into MDS */
   m_NCS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl        = ham_cb->mds_hdl;
   mds_info.i_svc_id         = NCSMDS_SVC_ID_HCD;
   mds_info.i_op             = MDS_INSTALL;

   mds_info.info.svc_install.i_yr_svc_hdl      = (MDS_CLIENT_HDL)ham_cb->cb_hdl;
   mds_info.info.svc_install.i_install_scope   = NCSMDS_SCOPE_NONE;
   mds_info.info.svc_install.i_svc_cb          = ham_mds_callback;
   mds_info.info.svc_install.i_mds_q_ownership = FALSE;
   mds_info.info.svc_install.i_mds_svc_pvt_ver = HCD_MDS_SUB_PART_VERSION;
   if (NCSCC_RC_SUCCESS != (rc = ncsmds_api(&mds_info)))
   {
      /* Log */
      m_LOG_HISV_DTS_CONS("ham_mds_initialize: error\n");
      return rc;
   }

   /* Now subscribe for HAM events in MDS */
   m_NCS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl        = ham_cb->mds_hdl;
   mds_info.i_svc_id         = NCSMDS_SVC_ID_HCD;
   mds_info.i_op             = MDS_SUBSCRIBE;

   mds_info.info.svc_subscribe.i_scope         = NCSMDS_SCOPE_NONE;
   mds_info.info.svc_subscribe.i_num_svcs      = 1;
   mds_info.info.svc_subscribe.i_svc_ids       = &svc;

   /* register to MDS */
   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DTS_CONS("ham_mds_initialize: error\n");
      return rc;
   }
   return rc;
}

/****************************************************************************
 * Name          : ham_mds_vdest_destroy
 *
 * Description   : This function Destroys the Virtual destination of HAM
 *
 * Arguments     : cb   : pointer to HAM control block structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
ham_mds_vdest_destroy (HAM_CB *ham_cb)
{
   NCSVDA_INFO    vda_info;
   uns32          rc;

   m_NCS_MEMSET(&vda_info,'\0',sizeof(NCSVDA_INFO));

   vda_info.req                             = NCSVDA_VDEST_DESTROY;
   vda_info.info.vdest_destroy.i_vdest      = ham_cb->ham_vdest;

   /* destroy the HAM VDEST */
   if(NCSCC_RC_SUCCESS != ( rc = ncsvda_api(&vda_info)))
   {
      m_LOG_HISV_DTS_CONS("ham_mds_vdest_destroy: error\n");
      return rc;
   }
   return rc;
}


/****************************************************************************
 * Name          : ham_mds_finalize
 *
 * Description   : This function un-registers the HAM Service with MDS.
 *
 * Arguments     : cb - pointer to HAM control block structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ham_mds_finalize (HAM_CB *cb)
{
   NCSMDS_INFO          mds_info;
   uns32                rc;

   /* Un-install HAM service from MDS */
   m_NCS_OS_MEMSET(&mds_info,'\0',sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl        = cb->mds_hdl;
   mds_info.i_svc_id         = NCSMDS_SVC_ID_HCD;
   mds_info.i_op             = MDS_UNINSTALL;

   /* request to un-install from the MDS service */
   rc = ncsmds_api(&mds_info);

   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DTS_CONS("ham_mds_finalize: error\n");
      return rc;
   }

   /* Destroy the virtual Destination of HAM */
   rc = ham_mds_vdest_destroy(cb);
   return rc;
}


/****************************************************************************
 * Name          : ham_mds_msg_send
 *
 * Description   : This routine sends the information to HPL. The send
 *                 operation is a 'normal' MDS send. HAM needs to send its
 *                 chassis-id and VDEST to HPL and it uses this function
 *                 for sending the MDS message to HPL.
 *
 * Arguments     : cb  - ptr to the HAM CB
 *                 i_msg - ptr to the HISv message
 *                 dest  - MDS destination to send to (HPL).
 *                 prio - MDS priority of this message
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/timeout
 *
 * Notes         : None.
 *****************************************************************************/

uns32 ham_mds_msg_send (HAM_CB *cb, HISV_MSG *msg, MDS_DEST *dest,
                        MDS_SEND_PRIORITY_TYPE prio, uns32 send_type, HISV_EVT *rcv)
{
   NCSMDS_INFO   mds_info;
   MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
   uns32         rc = NCSCC_RC_SUCCESS;
   MDS_SENDTYPE_SND_INFO *send;

   /* populate the mds params */
   m_NCS_MEMSET(&mds_info, '\0', sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl = cb->mds_hdl;
   mds_info.i_svc_id = NCSMDS_SVC_ID_HCD;
   mds_info.i_op = MDS_SEND;

   send_info->i_msg    = msg;
   send_info->i_to_svc = NCSMDS_SVC_ID_HPL;
   send_info->i_priority = prio;

   if (send_type == MDS_SENDTYPE_RSP)
   {
      send_info->info.rsp.i_msg_ctxt = rcv->mds_ctxt;
      send_info->i_sendtype = send_type;
      send_info->info.rsp.i_sender_dest = *dest;
      send_info->i_to_svc = rcv->to_svc;
   }
   else
   {
      send = &send_info->info.snd;
      send_info->i_sendtype = send_type;
      send->i_to_dest = *dest;
   }
   /* send the message */
   rc = ncsmds_api(&mds_info);
   return rc;
}
