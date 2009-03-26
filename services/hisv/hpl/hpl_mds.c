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
*  MODULE NAME:  hpl_mds.c                                                   *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This file contains routines used by HPL library for MDS interaction in    *
*  order to communicate with HCD. MDS callback functions and the functions   *
*  to send the message over MDS or process the received message/event are    *
*  available in this file.                                                   *
*                                                                            *
*****************************************************************************/

#include "hpl.h"

/* local functions declaration */
/* MDS callback functions */
static uns32 hpl_mds_callback(struct ncsmds_callback_info *info);
static uns32 hpl_mds_cb_cpy      (struct ncsmds_callback_info *info);
static uns32 hpl_mds_cb_enc      (struct ncsmds_callback_info *info);
static uns32 hpl_mds_cb_dec      (struct ncsmds_callback_info *info);
static uns32 hpl_mds_cb_enc_flat (struct ncsmds_callback_info *info);
static uns32 hpl_mds_cb_dec_flat (struct ncsmds_callback_info *info);
static uns32 hpl_mds_cb_rcv      (struct ncsmds_callback_info *info);
static uns32 hpl_mds_svc_event   (struct ncsmds_callback_info *info);
static uns32 hpl_mds_sys_event (struct ncsmds_callback_info *mds_cb_info);
static uns32 hpl_mds_quiesced_event (struct ncsmds_callback_info *mds_cb_info);
static uns32 hpl_mds_cb_rcv_direct (struct ncsmds_callback_info *mds_cb_info);

/* function to process the received message  */
static uns32 hpl_msg_proc        (HPL_CB *hpl_cb, HISV_MSG *hisv_msg,
                                  MDS_SEND_PRIORITY_TYPE prio);

const MDS_CLIENT_MSG_FORMAT_VER hpl_hcd_msg_fmt_map_table[HPL_HCD_SUB_PART_VER_MAX] = {HPL_MSG_FMT_VER};

/****************************************************************************
 * Name          : hpl_mds_callback
 *
 * Description   : Callback Dispatcher for various MDS operations.
 *
 * Arguments     : cb   : HPL-MDS control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_callback(struct ncsmds_callback_info *info)
{
   static NCSMDS_CALLBACK_API cb_set[MDS_CALLBACK_SVC_MAX] = {
         hpl_mds_cb_cpy,      /* MDS_CALLBACK_COPY      */
         hpl_mds_cb_enc,      /* MDS_CALLBACK_ENC       */
         hpl_mds_cb_dec,      /* MDS_CALLBACK_DEC       */
         hpl_mds_cb_enc_flat, /* MDS_CALLBACK_ENC_FLAT  */
         hpl_mds_cb_dec_flat, /* MDS_CALLBACK_DEC_FLAT  */
         hpl_mds_cb_rcv,      /* MDS_CALLBACK_RECEIVE   */
         hpl_mds_svc_event,    /* MDS_CALLBACK_SVC_EVENT */
         hpl_mds_sys_event, /* MDS_CALLBACK_SYS_EVENT */
         hpl_mds_quiesced_event, /* MDS_CALLBACK_QUIESCED_ACK */
         hpl_mds_cb_rcv_direct      /* MDS_CALLBACK_DIRECT_RECEIVE   */

   };

   if (info->i_op <= MDS_CALLBACK_DIRECT_RECEIVE)
      return (*cb_set[info->i_op])(info);
   else
   {
      printf("hpl_mds_callback: info->i_op is beyond MDS_CALLBACK_SVC_MAX\n");
      return NCSCC_RC_FAILURE;
   }
}


/****************************************************************************
 * Name          : hpl_mds_cb_cpy
 *
 * Description   : This function copies an events received.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_cb_cpy(struct ncsmds_callback_info *info)
{
   HISV_EVT  *evt;
   HISV_MSG  *hisv_msg = info->info.cpy.i_msg;
   uns32      data_len;

   /** allocate an HISV_EVENT now **/
   if (NULL == (evt = m_MMGR_ALLOC_HISV_EVT))
   {
      m_LOG_HISV_DEBUG("m_MMGR_ALLOC_HISV_EVT failed\n");
      return NCSCC_RC_FAILURE;
   }
   info->info.cpy.o_msg_fmt_ver = HPL_MSG_FMT_VER;
   info->info.cpy.o_cpy = (uns8*)evt;
   memset(evt, '\0', sizeof(HISV_EVT));
   memcpy(&evt->msg, hisv_msg, sizeof(HISV_MSG));

   data_len = hisv_msg->info.api_info.data_len;

   if (data_len > 0)
   {
      /* decode the entity path string */
      evt->msg.info.api_info.data = m_MMGR_ALLOC_HISV_DATA(data_len);
      if (NULL == evt->msg.info.api_info.data)
      {
         m_LOG_HISV_DEBUG("m_MMGR_ALLOC_HISV_EVT failed\n");
         m_MMGR_FREE_HISV_EVT(evt);
         return NCSCC_RC_FAILURE;
      }
      memcpy(evt->msg.info.api_info.data, hisv_msg->info.api_info.data, (int32)data_len);
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
 * Name          : hpl_mds_cb_enc
 *
 * Description   : This is a callback routine that is invoked to encode the
 *                 HISV message being sent to HAM VDEST. HPI commands and
 *                 data are encoded here before sending to HAM.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_cb_enc (struct ncsmds_callback_info *info)
{
   HISV_MSG        *msg;
   NCS_UBAID       *uba;
   uns8            *p8;
   uns32           data_len;
   int32           total_bytes = 0;


  if( NCSMDS_SVC_ID_HCD == info->info.enc.i_to_svc_id )
  {
     info->info.enc.o_msg_fmt_ver =  m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
                                     HPL_HCD_SUB_PART_VER_MIN,
                                     HPL_HCD_SUB_PART_VER_MAX,
                                     hpl_hcd_msg_fmt_map_table);
   
   
    if(info->info.enc.o_msg_fmt_ver  < HPL_MSG_FMT_VER)
    {
       m_LOG_HISV_DEBUG("Mds Encode fails :Message being sent to incompatiable version \n");
       return NCSCC_RC_FAILURE;
    } 
  }

   msg = (HISV_MSG*)info->info.enc.i_msg;
   uba = info->info.enc.io_uba;

   if ((NULL == uba) || (NULL == msg))
      return NCSCC_RC_FAILURE;

   /* encode the HPI command */
   p8 = ncs_enc_reserve_space(uba, 4);
   ncs_encode_32bit(&p8, msg->info.api_info.cmd);
   ncs_enc_claim_space(uba, 4);
   total_bytes += 4;

   /* encode the argument data to HPI command */
   p8 = ncs_enc_reserve_space(uba, 4);
   ncs_encode_32bit(&p8, msg->info.api_info.arg);
   ncs_enc_claim_space(uba, 4);
   total_bytes += 4;

   /* length of associated data sent with this command */
   data_len = msg->info.api_info.data_len;
   p8 = ncs_enc_reserve_space(uba, 4);
   ncs_encode_32bit(&p8, data_len);
   ncs_enc_claim_space(uba, 4);
   total_bytes += 4;

   /** actual data sent with the command. (contains entity-path
    ** of the resource
    **/
   ncs_encode_n_octets_in_uba(uba, msg->info.api_info.data, (int32)data_len);
   total_bytes += data_len;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_cb_dec
 *
 * Description   : This is a callback routine that is invoked to decode the
 *                 received messages. HAM sends its VDEST and chassis-id
 *                 and the message received here would be decoded for further
 *                 processing in receive callback routine.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
hpl_mds_cb_dec (struct ncsmds_callback_info *info)
{
   uns8      *p8;
   HISV_MSG  *msg;
   NCS_UBAID *uba = info->info.dec.io_uba;
   uns32     mds_dest_size = sizeof(MDS_DEST);
   uns8      local_data[sizeof(MDS_DEST) + 1];
   uns32     total_bytes = 0, data_len = 0;
  

  if ((info->i_yr_svc_id == NCSMDS_SVC_ID_HPL) 
     && (info->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_HCD))
  {
    if(!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
                                   HPL_HCD_SUB_PART_VER_MIN,
                                   HPL_HCD_SUB_PART_VER_MAX,
                                   hpl_hcd_msg_fmt_map_table))
    {
      m_LOG_HISV_DEBUG("Mds Decode fails: Message received from incompatiable version \n");
      return NCSCC_RC_FAILURE;
    }
  }
 
   /** Allocate a new msg in both sync/async cases
    **/
   msg = m_MMGR_ALLOC_HISV_MSG;
   memset(msg, '\0', sizeof(HISV_MSG));

   /* get the message */
   info->info.dec.o_msg = (uns8*)msg;

   /* flatten the spaces to read chassis_id */
   p8  = ncs_dec_flatten_space(uba, local_data, 4);
   msg->info.cbk_info.ret_type = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   total_bytes += 4;

   switch (msg->info.cbk_info.ret_type)
   {
   case   HPL_HAM_VDEST_RET:
      /* flatten the spaces to read chassis_id */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      msg->info.cbk_info.hpl_ret.h_vdest.chassis_id = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;

      /* read the MDS VDEST from the message */
      ncs_decode_n_octets_from_uba(uba, (uns8 *)&msg->info.cbk_info.hpl_ret.h_vdest.ham_dest, mds_dest_size);
      total_bytes += mds_dest_size;

      /* read the status value of HAM */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      msg->info.cbk_info.hpl_ret.h_vdest.ham_status = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;
      break;

   case HPL_GENERIC_DATA:
      /* flatten the spaces to read ret_val */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      data_len = msg->info.cbk_info.hpl_ret.h_gen.ret_val = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;

      /* flatten the spaces to read data_len */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      data_len = msg->info.cbk_info.hpl_ret.h_gen.data_len = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;

      if (data_len > 0)
      {
         /* if data is present, it contains return value from HPI. decode it */
         msg->info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(data_len);
         if (NULL == msg->info.cbk_info.hpl_ret.h_gen.data)
         {
            m_LOG_HISV_DEBUG("m_MMGR_ALLOC_HISV_DATA failed\n");
            m_MMGR_FREE_HISV_MSG(msg);
            return NCSCC_RC_FAILURE;
         }
         ncs_decode_n_octets_from_uba(uba, msg->info.cbk_info.hpl_ret.h_gen.data, data_len);
         total_bytes += data_len;
      }
      else
      {
         /* command does not need data */
         msg->info.cbk_info.hpl_ret.h_gen.data_len = 0;
         msg->info.cbk_info.hpl_ret.h_gen.data = NULL;
      }
      break;

   default:
      m_MMGR_FREE_HISV_MSG(msg);
      m_LOG_HISV_DEBUG("hpl_mds_cb_dec, invalid return type\n");
                                       /* msg->info.cbk_info.ret_type; */
      return NCSCC_RC_FAILURE;
      break;
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_cb_enc_flat
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
hpl_mds_cb_enc_flat(struct ncsmds_callback_info *info)
{
   HISV_MSG        *msg;
   NCS_UBAID       *uba;
   uns32           data_len;


  if( NCSMDS_SVC_ID_HCD == info->info.enc_flat.i_to_svc_id )
  {
    info->info.enc.o_msg_fmt_ver =  m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
                                   HPL_HCD_SUB_PART_VER_MIN,
                                   HPL_HCD_SUB_PART_VER_MAX,
                                   hpl_hcd_msg_fmt_map_table);
   
   
    if(info->info.enc.o_msg_fmt_ver  < HPL_MSG_FMT_VER)
    {
      m_LOG_HISV_DEBUG("Mds Encode fails :Message being sent to incompatiable version \n");
      return NCSCC_RC_FAILURE;
    }
  }
 
   /* get the message */
   msg = (HISV_MSG*)info->info.enc_flat.i_msg;
   uba = info->info.enc_flat.io_uba;

   if ((NULL == uba) || (NULL == msg))
      return NCSCC_RC_FAILURE;
   switch(info->info.enc_flat.i_to_svc_id)
   {
   case NCSMDS_SVC_ID_HCD:
      {    
         msg = (HISV_MSG*)info->info.enc_flat.i_msg;
         ncs_encode_n_octets_in_uba(uba, (uns8*)msg, sizeof(HISV_MSG));
         data_len = msg->info.api_info.data_len;
         if (data_len > 0)
            ncs_encode_n_octets(uba->ub, (uns8*)msg->info.api_info.data, data_len);

      }
      break;
   default:
      m_LOG_HISV_DEBUG("hpl_mds_cb_enc_flat, invalid service id \n");
                                        /* info->info.enc_flat.i_to_svc_id; */
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_cb_dec_flat
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
hpl_mds_cb_dec_flat(struct ncsmds_callback_info *info)
{
   uns8      *p8;
   HISV_MSG  *msg;
   NCS_UBAID *uba = info->info.dec_flat.io_uba;
   uns32     mds_dest_size = sizeof(MDS_DEST);
   uns8      local_data[sizeof(MDS_DEST) + 1];
   uns32     total_bytes = 0, data_len = 0;
  

  if ((info->i_yr_svc_id == NCSMDS_SVC_ID_HPL) 
     && (info->info.dec_flat.i_fr_svc_id == NCSMDS_SVC_ID_HCD))
  {
    if(!m_NCS_MSG_FORMAT_IS_VALID(info->info.dec_flat.i_msg_fmt_ver,
                                   HPL_HCD_SUB_PART_VER_MIN,
                                   HPL_HCD_SUB_PART_VER_MAX,
                                   hpl_hcd_msg_fmt_map_table))
    {
      m_LOG_HISV_DEBUG("Mds Decode fails: Message received from incompatiable version \n");
      return NCSCC_RC_FAILURE;
    }
  }

   /** Allocate a new msg in both sync/async cases
    **/
   msg = m_MMGR_ALLOC_HISV_MSG;
   memset(msg, '\0', sizeof(HISV_MSG));
    
   /* get the message */
   info->info.dec_flat.o_msg = (uns8*)msg;
    
   /* flatten the spaces to read chassis_id */
   p8  = ncs_dec_flatten_space(uba, local_data, 4);
   msg->info.cbk_info.ret_type = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   total_bytes += 4;
    
   switch (msg->info.cbk_info.ret_type)
   {
   case   HPL_HAM_VDEST_RET:
      /* flatten the spaces to read chassis_id */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      msg->info.cbk_info.hpl_ret.h_vdest.chassis_id = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;
    
      /* read the MDS VDEST from the message */
      ncs_decode_n_octets_from_uba(uba, (uns8 *)&msg->info.cbk_info.hpl_ret.h_vdest.ham_dest, mds_dest_size);
      total_bytes += mds_dest_size;
    
      /* read the status value of HAM */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      msg->info.cbk_info.hpl_ret.h_vdest.ham_status = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;
      break;

   case HPL_GENERIC_DATA:
      /* flatten the spaces to read ret_val */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      data_len = msg->info.cbk_info.hpl_ret.h_gen.ret_val = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;
    
      /* flatten the spaces to read data_len */
      p8  = ncs_dec_flatten_space(uba, local_data, 4);
      data_len = msg->info.cbk_info.hpl_ret.h_gen.data_len = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      total_bytes += 4;
    
      if (data_len > 0)
      {
         /* if data is present, it contains return value from HPI. decode it */
         msg->info.cbk_info.hpl_ret.h_gen.data = m_MMGR_ALLOC_HISV_DATA(data_len);
         if (NULL == msg->info.cbk_info.hpl_ret.h_gen.data)
         {
            m_LOG_HISV_DEBUG("m_MMGR_ALLOC_HISV_DATA failed\n");
            m_MMGR_FREE_HISV_MSG(msg);
            return NCSCC_RC_FAILURE;
         }
         ncs_decode_n_octets_from_uba(uba, msg->info.cbk_info.hpl_ret.h_gen.data, data_len);
         total_bytes += data_len;
      }
      else
      {
         /* command does not need data */
         msg->info.cbk_info.hpl_ret.h_gen.data_len = 0;
         msg->info.cbk_info.hpl_ret.h_gen.data = NULL;
      }
      break;
     
   default:
      m_MMGR_FREE_HISV_MSG(msg);
      m_LOG_HISV_DEBUG("hpl_mds_cb_dec_flat, invalid return type\n");
                                       /* msg->info.cbk_info.ret_type; */
      return NCSCC_RC_FAILURE;
      break;
   }
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : hpl_mds_cb_rcv
 *
 * Description   : This is a callback routine that is invoked when message
 *                 is received. HAM sends its VDEST and chassis-id which is
 *                 received by HPL here.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_cb_rcv (struct ncsmds_callback_info *mds_cb_info)
{
   HISV_MSG *hisv_msg = (HISV_MSG *)mds_cb_info->info.receive.i_msg;
   HPL_CB   *hpl_cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* retrieve HPL CB */
   if (NULL == (hpl_cb = (HPL_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HPL,
                (uns32) mds_cb_info->i_yr_svc_hdl)))
   {
      m_MMGR_FREE_HISV_MSG(hisv_msg);
      mds_cb_info->info.receive.i_msg = NULL;
      rc = NCSCC_RC_FAILURE;
      return rc;
   }

   /** Lock HPL_CB
    **/
   m_NCS_LOCK(&hpl_cb->cb_lock, NCS_LOCK_WRITE);

   /** process the message to make a mapping table to map
    ** chassis-id to HAM MDS VDEST.
    **/
   rc = hpl_msg_proc(hpl_cb,
      hisv_msg, mds_cb_info->info.receive.i_priority);

   /** Unlock the HPL_CB
    **/
   m_NCS_UNLOCK(&hpl_cb->cb_lock, NCS_LOCK_WRITE);

   /* free the decoded HISV_MSG received */
   m_MMGR_FREE_HISV_MSG(hisv_msg);
   mds_cb_info->info.receive.i_msg = NULL;

   /* return HPL CB */
   ncshm_give_hdl((uns32)mds_cb_info->i_yr_svc_hdl);

   return rc;
}


/****************************************************************************
 * Name          : hpl_mds_svc_event
 *
 * Description   : This is a callback routine that is invoked to inform HPL
 *                 of MDS events. HPL had subscribed to these events through
 *                 MDS subscription.
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_svc_event (struct ncsmds_callback_info *mds_cb_info)
{
   uns32 hpl_mds_hdl;
   HPL_CB *hpl_cb;

   /* Retrieve the HPL_CB hdl */
   hpl_mds_hdl = (uns32) mds_cb_info->i_yr_svc_hdl;
   printf("HPL Rcvd MDS subscribe evt from HAM\n");
   m_LOG_HISV_DEBUG("HPL Rcvd MDS subscribe evt from HAM\n");
                     /* mds_cb_info->info.svc_evt.i_svc_id; */

   /* Take the HPL hdl */
   if((hpl_cb = (HPL_CB*)ncshm_take_hdl(NCS_SERVICE_ID_HPL, hpl_mds_hdl)) == NULL)
   {
      /* HISV_ADD_LOG_HERE- MDS Callback error */
      return NCSCC_RC_SUCCESS;
   }

   switch (mds_cb_info->info.svc_evt.i_change)
   {
      case NCSMDS_DOWN:
      if(mds_cb_info->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_HCD)
      {
         /** TBD what to do if HCD goes down
          ** Hold on to the subscription if possible
          ** to send them out if HCD comes back up
          ** TODO?
          **/
      }
      break;
      case NCSMDS_UP:
         switch(mds_cb_info->info.svc_evt.i_svc_id)
         {
            case NCSMDS_SVC_ID_HCD:
               /** got the VDEST of HCD instance, request for chass_id
                ** Now the VDEST of HCD is received in separate message.
                ** nothing done here...
                **/
               break;
            default:
               break;
         }
         break;
      default:
         break;
   }

   /* Give the hdl back */
   ncshm_give_hdl((uns32)hpl_mds_hdl);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_sys_event, 
 *
 * Description   : This is a dummy callback routine
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_sys_event (struct ncsmds_callback_info *mds_cb_info)
{
   printf("hpl_mds_sys_event\n");
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_quiesced_event, 
 *
 * Description   : This is a dummy callback routine
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_quiesced_event (struct ncsmds_callback_info *mds_cb_info)
{
   printf("hpl_mds_quiesced_event\n");
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_cb_rcv_direct, 
 *
 * Description   : This is a dummy callback routine
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_mds_cb_rcv_direct (struct ncsmds_callback_info *mds_cb_info)
{
   printf("hpl_mds_cb_rcv_direct\n");
   return NCSCC_RC_SUCCESS;
}



/****************************************************************************
 * Name          : hpl_msg_proc
 *
 * Description   : This routine is used to process the ASYNC incoming
 *                 HPL messages from HAM. It updates the HAM info in HPL_CB.
 *                 HAM sends its VDEST and chassis-id which is used by
 *                 HPL to update its "chassis-id to HAM VDEST" mapping table
 *
 * Arguments     : hpl_cb - pointer to HPL control block structure.
 *                 hisv_msg - HISV message received from HAM.
 *                 prio - priority of this message.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
hpl_msg_proc(HPL_CB *hpl_cb, HISV_MSG *hisv_msg, MDS_SEND_PRIORITY_TYPE prio)
{
   HAM_INFO   *ham_info = NULL, *ham_inst, *ham_prev = NULL;

   if (hisv_msg->info.cbk_info.ret_type != HPL_HAM_VDEST_RET)
   {
      m_LOG_HISV_DEBUG("UnKnown return type from HAM instance\n");
      return NCSCC_RC_FAILURE;
   }

   switch (hisv_msg->info.cbk_info.hpl_ret.h_vdest.ham_status)
   {
      /* HAM is UP with MDS registration. */
      case HAM_MDS_UP:
         ham_inst = hpl_cb->ham_inst;
         /** check if the table already contain the chassis-id information
          ** for this HAM.
          **/
         while ((ham_inst != NULL) &&
                (ham_inst->chassis_id != hisv_msg->info.cbk_info.hpl_ret.h_vdest.chassis_id))
            ham_inst = ham_inst->ham_next;

         if (ham_inst != NULL)   /* entry already exists, just update it */
         {
            memcpy(&ham_inst->ham_dest, &hisv_msg->info.cbk_info.hpl_ret.h_vdest.ham_dest,
                         (int32)sizeof(MDS_DEST) );
            ham_inst->chassis_id = hisv_msg->info.cbk_info.hpl_ret.h_vdest.chassis_id;
            break;
         }
         /* update the table with the mapping of chassis-id and this HAM VDEST */
         /* allocate space in mapping table */
         ham_info = m_MMGR_ALLOC_HAM_INFO;
         ham_info->ham_next = NULL;

         /* put the new entry in the table */
         if (hpl_cb->ham_inst == NULL)
            hpl_cb->ham_inst = ham_info;
         else
         {
            ham_inst = hpl_cb->ham_inst;
            while (ham_inst->ham_next != NULL)
               ham_inst = ham_inst->ham_next;
            ham_inst->ham_next = ham_info;
         }
         /* fill the information in new entry */
         memcpy(&ham_info->ham_dest, &hisv_msg->info.cbk_info.hpl_ret.h_vdest.ham_dest,
                      (int32)sizeof(MDS_DEST) );
         ham_info->chassis_id = hisv_msg->info.cbk_info.hpl_ret.h_vdest.chassis_id;
         break;

      /* HAM goes DOWN with MDS registration. */
      case HAM_MDS_DOWN:
         ham_inst = hpl_cb->ham_inst;
         /* if the table is empty, break */
         if (ham_inst == NULL) break;

         /* check if the mapping entry is first in the table */
         if (ham_inst->chassis_id == hisv_msg->info.cbk_info.hpl_ret.h_vdest.chassis_id)
         {
            /* update the head node of table and delete entry */
            hpl_cb->ham_inst = ham_inst->ham_next;
            m_MMGR_FREE_HAM_INFO(ham_inst);
            break;
         }

         ham_prev = ham_inst;
         /* look for the entry in the table */
         while ((ham_inst != NULL) &&
                (ham_inst->chassis_id != hisv_msg->info.cbk_info.hpl_ret.h_vdest.chassis_id))
         {
            ham_prev = ham_inst;
            ham_inst = ham_inst->ham_next;
         }
         /* ham_inst (if not NULL) contains the matching entry */
         if ((ham_inst != NULL) && (ham_prev != NULL))
         {
            /* update table and delete the entry */
            ham_prev->ham_next = ham_inst->ham_next;
            m_MMGR_FREE_HAM_INFO(ham_inst);
         }
         break;

      default:
         m_LOG_HISV_DEBUG("UnKnown MDS status of HAM instance\n");
         break;
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_msg_async_send
 *
 * Description   : This routine sends the HPL message to HAM VDEST. It is
 *                 invoked from HPL APIs.
 *
 * Arguments     : cb  - ptr to the HPL CB
 *                 i_msg - ptr to the HISv request message .
 *                 ham_dest - MDS VDEST of HAM instance.
 *                 prio - priority of this message.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_mds_msg_async_send (HPL_CB *cb, HISV_MSG *i_msg,
                              MDS_DEST *ham_dest, uns32 prio)
{
   NCSMDS_INFO     mds_info;

   if(NULL == i_msg)
      return NCSCC_RC_FAILURE;

   memset(&mds_info, '\0', sizeof(NCSMDS_INFO));
   mds_info.i_mds_hdl = cb->mds_hdl;
   mds_info.i_svc_id = NCSMDS_SVC_ID_HPL;
   mds_info.i_op = MDS_SEND;

   /* fill the main send structure */
   mds_info.info.svc_send.i_msg = (NCSCONTEXT)i_msg;
   mds_info.info.svc_send.i_priority = prio;
   mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_HCD;
   mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;

   /* fill the sub send strcuture */
   memcpy(&mds_info.info.svc_send.info.snd.i_to_dest, ham_dest,
                (int32)sizeof(MDS_DEST));

   /* send the message */
   return ncsmds_api(&mds_info);
}


/****************************************************************************
 * Name          : hpl_mds_msg_sync_send
 *
 * Description   : This routine sends the HPL messages to HAM VDEST. It is
 *                 invoked from HPL APIs. Sync waits for response from HAM.
 *
 * Arguments     : cb  - ptr to the HPL CB
 *                 i_msg - ptr to the HISv request message .
 *                 ham_dest - MDS VDEST of HAM instance.
 *                 prio - priority of this message.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

HISV_MSG* hpl_mds_msg_sync_send (HPL_CB *cb, HISV_MSG *i_msg,
                              MDS_DEST *ham_dest, uns32 prio, uns32 timeout)
{
   NCSMDS_INFO     mds_info;
   int32 rc;

   if(NULL == i_msg)
      return NULL;

   memset(&mds_info, '\0', sizeof(NCSMDS_INFO));
   mds_info.i_mds_hdl = cb->mds_hdl;
   mds_info.i_svc_id = NCSMDS_SVC_ID_HPL;
   mds_info.i_op = MDS_SEND;

   /* fill the main send structure */
   mds_info.info.svc_send.i_msg = (NCSCONTEXT)i_msg;
   mds_info.info.svc_send.i_priority = prio;
   mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_HCD;
   mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SNDRSP;

   /* fill the send rsp strcuture */
   mds_info.info.svc_send.info.sndrsp.i_time_to_wait = timeout;

   /* fill the sub send strcuture */
   memcpy(&mds_info.info.svc_send.info.sndrsp.i_to_dest, ham_dest,
                (int32)sizeof(MDS_DEST));

   /* send the message */
   rc = ncsmds_api(&mds_info);

   if (rc == NCSCC_RC_FAILURE)
      return NULL;

   return (mds_info.info.svc_send.info.sndrsp.o_rsp);
}


/****************************************************************************
 * Name          : get_ham_dest
 *
 * Description   : Gets the MDS VDEST of HAM managing chassis identified by
 *                 the chassis_id. Makes a look-up in mapping table.
 *
 * Arguments     : hpl_cb - pointer to HPL control block structure.
 *                 ham_dest - pointer to retrieve the HAM MDS VDEST.
 *                 chassis_id - chassis identifier.
 *
 * Return Values : MDS VDEST of HAM which is managing chassis identified by
 *                 the chassis_id.
 *                 NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 get_ham_dest (HPL_CB *hpl_cb, MDS_DEST *ham_dest, uns32 chassis_id)
{
   HAM_INFO *ham_inst;
   uns32 rc;

   /** Lock HPL_CB
    **/
   m_NCS_LOCK(&hpl_cb->cb_lock, NCS_LOCK_WRITE);

   /* look-up the table for matching chassis_id entry */
   ham_inst = hpl_cb->ham_inst;
   while ((ham_inst != NULL) && (ham_inst->chassis_id != chassis_id))
      ham_inst = ham_inst->ham_next;

   /* matching entry not found */
   if (ham_inst == NULL)
   {
      /** UnLock HPL_CB
       **/
      m_NCS_UNLOCK(&hpl_cb->cb_lock, NCS_LOCK_WRITE);
      rc = NCSCC_RC_FAILURE;
      m_LOG_HISV_DEBUG("No HAM Managing the given chassis\n");
      return rc;
   }
   /* store the matching HAM MDS VDEST */
   *ham_dest = ham_inst->ham_dest;

   /** UnLock HPL_CB
    **/
   m_NCS_UNLOCK(&hpl_cb->cb_lock, NCS_LOCK_WRITE);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_initialize
 *
 * Description   : This routine registers the HISv-HPL from MDS.
 *
 * Arguments     : cb - ptr to the HPL data block
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hpl_mds_initialize(HPL_CB *hpl_cb)
{
   NCSADA_INFO  ada_info;
   NCSMDS_INFO  mds_info;
   uns32 rc =   NCSCC_RC_SUCCESS;
   MDS_SVC_ID   svc = NCSMDS_SVC_ID_HCD;

   /* Create the ADEST for HPL and get the pwe hdl */
   memset(&ada_info, 0, sizeof(ada_info));

   ada_info.req = NCSADA_GET_HDLS;
   ada_info.info.adest_get_hdls.i_create_oac = FALSE;

   if ( NCSCC_RC_SUCCESS != (rc = ncsada_api(&ada_info)))
   {
      /* to log an event and return */
      return NCSCC_RC_FAILURE;
   }

   /** Store the info obtained from MDS ADEST creation
    **/
   hpl_cb->mds_hdl   = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;
   hpl_cb->hpl_dest  = ada_info.info.adest_get_hdls.o_adest;

   hpl_cb->ham_inst = NULL;

   memset(&mds_info, 0, sizeof(mds_info));

   /* Do common stuff */
   mds_info.i_mds_hdl = hpl_cb->mds_hdl;
   mds_info.i_svc_id = NCSMDS_SVC_ID_HPL; /* see ncs_mdsev.h */

   /* Install with MDS */
   mds_info.i_op = MDS_INSTALL;
   mds_info.info.svc_install.i_mds_q_ownership = FALSE;
   mds_info.info.svc_install.i_svc_cb = hpl_mds_callback;
   /* TODO */
   mds_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)hpl_cb->cb_hdl;
   mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE; /* PWE scope */
   mds_info.info.svc_install.i_mds_svc_pvt_ver = HPL_MDS_SUB_PART_VERSION;
   if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DEBUG("ERROR:MDS installation failed\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   /* Now subscribe for events that will be generated by MDS */
   memset(&mds_info,'\0',sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl = hpl_cb->mds_hdl;
   mds_info.i_svc_id = NCSMDS_SVC_ID_HPL; /* see ncs_mdsev.h */
   mds_info.i_op = MDS_SUBSCRIBE;

   mds_info.info.svc_subscribe.i_num_svcs = 1;
   mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
   mds_info.info.svc_subscribe.i_svc_ids = &svc;

   /* register to MDS */
   if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DEBUG("ERROR:MDS subscription failed\n");
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_mds_finalize
 *
 * Description   : This routine unregisters the HPL from MDS.
 *
 * Arguments     : cb - ptr to the HPL data block
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ******************************************************************************/
uns32 hpl_mds_finalize (HPL_CB *hpl_cb)
{
   NCSMDS_INFO          mds_info;

   /* Un-install your service into MDS.
   No need to cancel the services that are subscribed*/
   memset(&mds_info,'\0',sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl     = hpl_cb->mds_hdl;
   mds_info.i_svc_id      = NCSMDS_SVC_ID_HPL;
   mds_info.i_op          = MDS_UNINSTALL;

   /* un-install from MDS */
   if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
   {
      /* Log Error */
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}
