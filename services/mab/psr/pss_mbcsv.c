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
      
DESCRIPTION:
        
This include file contains subroutines for PSS checkpointing operations using
MBCSv.
          
******************************************************************************
*/

#include "psr.h"
#include "ncs_util.h"

#if (NCS_PSS_RED == 1)
#include "ncs_edu_pub.h"

static uns32 pss_red_err_ind_handle(NCS_MBCSV_CB_ARG *arg);

static char chk_pt_states[8][50] = {   
                                 "PSS_COLD_SYNC_IDLE",
                                 "PSS_COLD_SYNC_DATA_RESP_FAIL",
                                 "PSS_COLD_SYNC_COMPLETE",
                                 "PSS_COLD_SYNC_FAIL",
                                 "PSS_WARM_SYNC_COMPLETE",
                                 "PSS_WARM_SYNC_WAIT_FOR_DATA_RESP",
                                 "PSS_WARM_SYNC_DATA_RESP_FAIL",
                                 "PSS_WARM_SYNC_FAIL"
                                };

static char addrstr[255] = {0};

/****************************************************************************
 * Name          : psv_mbcsv_register
 *
 * Description   : This function initializes the mbcsv interface and
 *                 obtains a selection object from mbcsv.
 *                 
 * Arguments     : A pointer to the PSS control block (PSS_CB).
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_mbcsv_register(PSS_CB *pss_cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_MBCSV_ARG init_arg, sel_obj_arg;

   /* Initialize with MBCSv library */
   memset(&init_arg, '\0', sizeof(init_arg));
   /* init_arg.i_mbcsv_hdl = (NCS_MBCSV_HDL)NULL; */
   init_arg.i_op = NCS_MBCSV_OP_INITIALIZE;
   init_arg.info.initialize.i_mbcsv_cb = pss_mbcsv_callback;
   init_arg.info.initialize.i_version = PSS_MBCSV_VERSION;
   init_arg.info.initialize.i_service = NCS_SERVICE_ID_PSS;
 
   if(SA_AIS_OK != (rc = ncs_mbcsv_svc(&init_arg)))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_INITIALIZE_FAIL);
      return rc;
   }

   /* Store MBCSv handle in our global data */
   gl_pss_amf_attribs.handles.mbcsv_hdl = init_arg.info.initialize.o_mbcsv_hdl;

   /* Get MBCSv Selection Object */
   memset(&sel_obj_arg, '\0', sizeof(sel_obj_arg));
   sel_obj_arg.i_op = NCS_MBCSV_OP_SEL_OBJ_GET;
   sel_obj_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   sel_obj_arg.info.sel_obj_get.o_select_obj=0;

   if (SA_AIS_OK != (rc = ncs_mbcsv_svc(&sel_obj_arg)))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_GET_SELECT_OBJ_FAIL);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_GET_SELECT_OBJ_SUCCESS,
      sel_obj_arg.info.sel_obj_get.o_select_obj);

   /* Update the CB with mbcsv selectionobject */
   gl_pss_amf_attribs.handles.mbcsv_sel_obj = sel_obj_arg.info.sel_obj_get.o_select_obj;

   return rc;
}

/****************************************************************************
 * Name          : psv_mbcsv_deregister
 *
 * Description   : This function deregisters PSS with mbcsv interface.
 *                 
 * Arguments     : A pointer to the PSS control block (PSS_CB).
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_mbcsv_deregister(PSS_CB *pss_cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_FINALIZE;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_FINALIZE_FAIL);
      return NCSCC_RC_FAILURE;
   } 
   m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MBCSV_FINALIZE_SUCCESS);

   return rc;
}

/****************************************************************************
 * Name          : pss_mbcsv_obj_set
 *
 * Description   : Set MBCSv objects
 *                 
 * Arguments     : A pointer to the PSS PWE control block (PSS_PWE_CB).
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_mbcsv_obj_set(PSS_PWE_CB *pwe_cb,uns32 obj, uns32 val)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_OBJ_SET;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   mbcsv_arg.info.obj_set.i_ckpt_hdl = pwe_cb->ckpt_hdl;
   mbcsv_arg.info.obj_set.i_obj = obj;
   mbcsv_arg.info.obj_set.i_val = val;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_SET_OP_FAIL);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MBCSV_SET_OP_SUCCESS);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : pss_mbcsv_open_ckpt 
 *
 * Description   : This function opens a checkpoint with mbcsv. 
 *                 
 * Arguments     : A pointer to the PSS PWE control block (PSS_PWE_CB).
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_mbcsv_open_ckpt(PSS_PWE_CB *pwe_cb)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   /* Set the checkpoint open arguments */
   mbcsv_arg.i_op = NCS_MBCSV_OP_OPEN;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   mbcsv_arg.info.open.i_pwe_hdl = (uns32)pwe_cb->mds_pwe_handle;
   mbcsv_arg.info.open.i_client_hdl = pwe_cb->hm_hdl; /* pwe hm handle */

   if (SA_AIS_OK != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_OPEN_FAIL);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_OPEN_SUCCESS, pwe_cb->pwe_id);

   pwe_cb->ckpt_hdl = mbcsv_arg.info.open.o_ckpt_hdl;
   pwe_cb->ckpt_state = PSS_COLD_SYNC_IDLE;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_mbcsv_close_ckpt
 *
 * Description   : This function closes a checkpoint with mbcsv. 
 *                 
 * Arguments     : A pointer to the PSS PWE control block (PSS_PWE_CB).
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_mbcsv_close_ckpt(NCS_MBCSV_CKPT_HDL ckpt_hdl)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   /* Set the checkpoint open arguments */
   mbcsv_arg.i_op = NCS_MBCSV_OP_CLOSE;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   mbcsv_arg.info.close.i_ckpt_hdl = ckpt_hdl;

   if (SA_AIS_OK != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_CLOSE_SUCCESS);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MBCSV_CLOSE_SUCCESS);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_mbcsv_callback
 *
 * Description   : This callback is the single entry point for mbcsv to 
 *                 notify PSS of all checkpointing operations. 
 *
 * Arguments     : NCS_MBCSV_CB_ARG - Callback Info pertaining to the mbcsv
 *                 event from ACTIVE/STANDBY PSS peer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Based on the mbcsv message type, the corresponding mbcsv
 *                 message handler shall be invoked.
 *****************************************************************************/
uns32 pss_mbcsv_callback(NCS_MBCSV_CB_ARG *arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_BOOL is_in_sync = FALSE;
   
   m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_CB_INVOKED);
   if (arg == NULL)
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_CB_NULL_ARG_PTR);
      return NCSCC_RC_FAILURE;
   }

   switch(arg->i_op)
   {
   case NCS_MBCSV_CBOP_ENC:
      /* Encode Request from MBCSv */
      rc = pss_ckpt_encode_cbk_handler(arg);
      break;
   case NCS_MBCSV_CBOP_DEC:
      /* Decode Request from MBCSv */
      rc = pss_ckpt_decode_cbk_handler(arg, &is_in_sync);
      break;
   case NCS_MBCSV_CBOP_PEER:
      /* PSS Peer info from MBCSv */
      rc = pss_ckpt_peer_info_cbk_handler(arg);
      break;
   case NCS_MBCSV_CBOP_NOTIFY:
      /* NOTIFY info from PSS peer */
      rc = pss_ckpt_notify_cbk_handler(arg);
      break;
   case NCS_MBCSV_CBOP_ERR_IND:
      /* Peer error indication info */
      rc = pss_ckpt_err_ind_cbk_handler(arg);
      break;
   default:
      rc  = NCSCC_RC_FAILURE;
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_CB_INVLD_OP_PASSED);
      break;
   }/*End Switch(i_op)*/
 
   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_encode_cbk_handler
 *
 * Description   : This function invokes the corresponding encode routine
 *                 based on the MBCSv encode request type.
 *
 * Arguments     : arg - Pointer to NCS_MBCSV_CB_ARG with encode info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_encode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PSS_PWE_CB *pwe_cb = NULL;
   uns16      mbcsv_version = 0;

   m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ENC_CB_INVOKED);

   mbcsv_version = m_NCS_MBCSV_FMT_GET(
                                  cbk_arg->info.encode.i_peer_version,
                                  PSS_MBCSV_VERSION,
                                  PSS_MBCSV_VERSION_MIN);
   if( 0 == mbcsv_version )
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_MBCSV_PEER_VER, cbk_arg->info.encode.i_peer_version);
      return NCSCC_RC_FAILURE;
   }

   /* Get PSS_PWE_CB control block Handle. cbk_arg->arg->i_client_hdl! */
   if (NULL == (pwe_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_PSS,cbk_arg->i_client_hdl)))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_PWE_CB_TAKE_HANDLE_IN_MBCSVCB_FAILED);
      return NCSCC_RC_FAILURE;
   }

   switch(cbk_arg->info.encode.io_msg_type)
   {
   case NCS_MBCSV_MSG_ASYNC_UPDATE:
      /* Encode async update */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_ASYNC_UPDATE_ENC_STARTED, pwe_cb->pwe_id);
      rc = pss_ckpt_encode_async_update(cbk_arg);
      m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_ASYNC_UPDATE_ENC_STATUS, rc);
      break;
      
   case NCS_MBCSV_MSG_COLD_SYNC_REQ:
      /* Encode cold sync request */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_COLD_SYNC_REQ_ENC_DONE, pwe_cb->pwe_id);
      if(pwe_cb->is_cold_sync_done)
      {
         ncshm_give_hdl(cbk_arg->i_client_hdl);
         return NCSCC_RC_FAILURE;
      }
      break;
      
   case NCS_MBCSV_MSG_COLD_SYNC_RESP:
   case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
      if((pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) ||
         (pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_QUIESCING))
      {
         m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_COLD_SYNC_RESP_ENC_STARTED);
         rc = pss_ckpt_enc_coldsync_resp(pwe_cb, &cbk_arg->info.encode.io_uba, cbk_arg->info.encode.i_peer_version);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_COLD_SYNC_RESP_ENC_FAIL, pwe_cb->pwe_id, rc);
            ncshm_give_hdl(cbk_arg->i_client_hdl);
            return rc;
         }
         else
         {
            cbk_arg->info.encode.io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
            m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_COLD_SYNC_RESP_ENC_SUCCESS, pwe_cb->pwe_id);
         }
      } 
      else
      { 
         /* Do nothing */;;
         /* Log it. Ideally, this shouldn't occur.*/
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_COLD_SYNC_RESP_ENC_IN_NON_ACTIVE_ROLE, pwe_cb->pwe_id);
         ncshm_give_hdl(cbk_arg->i_client_hdl);
         return NCSCC_RC_FAILURE;
      } 
      break;
      
   case NCS_MBCSV_MSG_WARM_SYNC_REQ:
      /* Encode warm sync 'request' data */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_WARM_SYNC_REQ_ENC_DONE, pwe_cb->pwe_id);
      break;
      
   case NCS_MBCSV_MSG_WARM_SYNC_RESP:
   case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
      if((pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) ||
         (pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_QUIESCING))
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_WARM_SYNC_RESP_ENC_STARTED, pwe_cb->pwe_id);
         rc = pss_ckpt_enc_warmsync_resp(pwe_cb, &cbk_arg->info.encode.io_uba);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_WARM_SYNC_RESP_ENC_FAIL, pwe_cb->pwe_id, rc);
            ncshm_give_hdl(cbk_arg->i_client_hdl);
            return rc;
         }
         else
         {
            cbk_arg->info.encode.io_msg_type = NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE;
            m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_WARM_SYNC_RESP_ENC_SUCCESS, pwe_cb->pwe_id);
         }
      }
      else
      { 
         /* Do nothing */;;
         /* Log it. Ideally, this shouldn't occur.*/
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_WARM_SYNC_RESP_ENC_IN_NON_ACTIVE_ROLE, pwe_cb->pwe_id);
         ncshm_give_hdl(cbk_arg->i_client_hdl);
         return NCSCC_RC_FAILURE;
      } 
      break;

   case NCS_MBCSV_MSG_DATA_REQ:
      /* Encode Data 'request' data */
      if(pwe_cb->p_pss_cb->ha_state != SA_AMF_HA_STANDBY)
      {
         /* Do nothing */;;
         /* Log it. Ideally, this shouldn't occur.*/
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_DATA_REQ_ENC_IN_NON_STANDBY_ROLE, pwe_cb->pwe_id);
         ncshm_give_hdl(cbk_arg->i_client_hdl);
         return NCSCC_RC_FAILURE;
      } 
      break;

   case NCS_MBCSV_MSG_DATA_RESP:
      if((pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) ||
         (pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_QUIESCING))
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_DATA_RESP_ENC_STARTED, pwe_cb->pwe_id);
         rc = pss_ckpt_enc_data_resp(pwe_cb, &cbk_arg->info.encode.io_uba, cbk_arg->info.encode.i_peer_version);
         if (rc != NCSCC_RC_SUCCESS)
            m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_DATA_RESP_ENC_FAIL, pwe_cb->pwe_id, rc);
         else
         {
            cbk_arg->info.encode.io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE;
            m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_DATA_RESP_ENC_SUCCESS, pwe_cb->pwe_id);
         }
      }
      else
      { 
         /* Do nothing */;;
         /* Log it. Ideally, this shouldn't occur.*/
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_DATA_RESP_ENC_IN_NON_ACTIVE_ROLE, pwe_cb->pwe_id);
         ncshm_give_hdl(cbk_arg->i_client_hdl);
         return NCSCC_RC_FAILURE;
      } 
      break;
      
   default:
      /* Log Failure */
      m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_INVLD_ENC_MSG_TYPE,
         pwe_cb->pwe_id, cbk_arg->info.encode.io_msg_type);
      rc = NCSCC_RC_FAILURE;
      break;
   } /*End switch(io_msg_type)*/

   ncshm_give_hdl(cbk_arg->i_client_hdl);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_coldsync_resp
 *
 * Description   : This function encodes cold sync resp data., viz
 *                 checksum for PWE-CB.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 io_uba - Pointer to NCS_UBAID buffer.
 *                 peer_mbcsv_version - MBCSv version registered by Peer PSS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_coldsync_resp(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version)
{
   uns32      rc = NCSCC_RC_SUCCESS;
   NCS_BOOL   enc_lib_conf = FALSE;

   /* Currently, we shall send all the PWE-data in one send.
    * This shall avoid "delta data" problems that are associated during
    * multiple sends.
    */
   if(peer_mbcsv_version == 1)
      enc_lib_conf = TRUE;

   if(pss_enc_pwe_data_for_sync_with_standby(pwe_cb, io_uba, enc_lib_conf, peer_mbcsv_version) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_COLD_SYNC_RESP_PAYLOAD_FAIL, pwe_cb->pwe_id);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_warmsync_resp
 *
 * Description   : This function encodes warm sync resp data., viz
 *                 checksum for PWE-CB.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 io_uba - Pointer to NCS_UBAID buffer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_warmsync_resp(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns8 *data = NULL;

   if((data = ncs_enc_reserve_space(io_uba, sizeof(uns32))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_ckpt_enc_warmsync_resp()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   ncs_encode_32bit(&data, pwe_cb->async_cnt);
   ncs_enc_claim_space(io_uba, sizeof(uns32));

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_data_resp
 *
 * Description   : This function encodes data-request for PWE-data.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 io_uba - Pointer to NCS_UBAID buffer
 *                 peer_mbcsv_version - MBCSv version registered by Peer PSS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_data_resp(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_BOOL   enc_lib_conf = FALSE;
 
   /* Currently, we shall send all the PWE-data in one send.
    * This shall avoid "delta data" problems that are associated during
    * multiple sends.
    */   
   if(peer_mbcsv_version == 1)
      enc_lib_conf = TRUE;

   if(pss_enc_pwe_data_for_sync_with_standby(pwe_cb, io_uba, enc_lib_conf, peer_mbcsv_version) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_DATA_RESP_PAYLOAD_FAIL, pwe_cb->pwe_id);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_data_resp
 *
 * Description   : This function decodes data-response event., viz
 *                 checkpoint data for PWE-CB.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 cbk_arg - Pointer to NCS_MBCSV_CB_ARG with encode info.
 *                 peer_mbcsv_version - MBCSv version registered by Peer PSS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_data_resp(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_BOOL   dec_lib_conf = FALSE;
 
   /* Currently, we are sending all the PWE-data in one send.
    * This shall avoid "delta data" problems that are associated during
    * multiple sends.
    */
    if(peer_mbcsv_version == 1)
       dec_lib_conf = TRUE;

   if(pss_dec_pwe_data_for_sync_with_standby(pwe_cb, io_uba, dec_lib_conf, peer_mbcsv_version) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_DATA_RESP_PAYLOAD_FAIL, pwe_cb->pwe_id);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_enc_pwe_data_for_sync_with_standby
 *
 * Description   : This utility function encodes the pwe-data into buffer.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 io_uba - Pointer to NCS_UBAID buffer.
 *                 enc_lib_conf - Boolean to know whether pssv_lib_conf contents
 *                                need to be encoded.
 *                 peer_mbcsv_version - MBCSv version of the Peer PSS given
 *                                      by MBCSv in encode callback.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_enc_pwe_data_for_sync_with_standby(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba, NCS_BOOL enc_lib_conf, uns16 peer_mbcsv_version)
{
   uns32    rc = NCSCC_RC_SUCCESS, i = 0;
   uns8     *data = NULL, *p_pend_wbreq_cnt = NULL, *p_re_data_cnt = NULL, *p_tbl_entry_cnt = NULL;
   uns16    pend_wbreq_cnt = 0, re_data_cnt = 0, tbl_entry_cnt = 0;
   NCS_PATRICIA_NODE *pNode = NULL;
   PSS_CLIENT_KEY    client_key;
   PSS_CLIENT_ENTRY  *client_node = NULL;
   PSS_TBL_REC       *rec = NULL;
   PSS_SPCN_WBREQ_PEND_LIST *wbreq = NULL;
   EDU_ERR           ederror = EDU_NORMAL;

   /* Start populating PSS_PWE_CB data into the buffer. */
   /* Encode async_cnt */
   if((data = ncs_enc_reserve_space(io_uba, sizeof(uns32))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_enc_pwe_data_for_sync_with_standby()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   ncs_encode_32bit(&data, pwe_cb->async_cnt);
   ncs_enc_claim_space(io_uba, sizeof(uns32));

   if(pwe_cb->pwe_id == 1)
   {
      /* Encode pwe_cb->p_pss_cb->save_type in a 8-bit form. */
      if((data = ncs_enc_reserve_space(io_uba, sizeof(uns8))) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
            "pss_enc_pwe_data_for_sync_with_standby()");
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      ncs_encode_8bit(&data, (uns8)pwe_cb->p_pss_cb->save_type);
      ncs_enc_claim_space(io_uba, sizeof(uns8));

      rc = pss_ckpt_enc_reload_pssvspcnlist(io_uba, pwe_cb->p_pss_cb->spcn_list);
      if(rc != NCSCC_RC_SUCCESS)
      {
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      if(enc_lib_conf == TRUE)
      {
         rc = pss_ckpt_enc_reload_pssvlibconf(io_uba, (char*)&pwe_cb->p_pss_cb->lib_conf_file);
         if(rc != NCSCC_RC_SUCCESS)
         {
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
      }
   }

   /* Encode pwe_cb->spcn_wbreq_pend_list */
   if((p_pend_wbreq_cnt = ncs_enc_reserve_space(io_uba, sizeof(uns16))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_enc_pwe_data_for_sync_with_standby()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   ncs_enc_claim_space(io_uba, sizeof(uns16));
   if(pwe_cb->spcn_wbreq_pend_list != NULL)
   {
      for(wbreq = pwe_cb->spcn_wbreq_pend_list; wbreq != NULL; wbreq = wbreq->next)
      {
         if(pss_encode_pcn(io_uba, wbreq->pcn) != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, wbreq->pcn);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         pend_wbreq_cnt++;
      }
   }
   ncs_encode_16bit(&p_pend_wbreq_cnt, pend_wbreq_cnt);

   /* Encode both client_table and oaa_tree entries. Here, the entry :
    *    <count-of-pcns> [<PCN> <count-of-tables> [<MDS_DEST> <table-id>] ...] is sufficient. 
    */
   if((p_re_data_cnt = ncs_enc_reserve_space(io_uba, sizeof(uns16))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_enc_pwe_data_for_sync_with_standby()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   ncs_enc_claim_space(io_uba, sizeof(uns16));
   re_data_cnt = 0;

   memset(&client_key, '\0', sizeof(client_key));
   /* Start the while-loop on the oaa_tree and client_table */
   while( (pNode = ncs_patricia_tree_getnext(&pwe_cb->client_table,
                        (const uns8 *)&client_key)) != NULL )
   {
      client_node = (PSS_CLIENT_ENTRY *) pNode;
      client_key = client_node->key;
      ++re_data_cnt;

      /* PCN name */
      if(pss_encode_pcn(io_uba, (char*)&client_key.pcn) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, &client_key.pcn);
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }

      if((p_tbl_entry_cnt = ncs_enc_reserve_space(io_uba, sizeof(uns16))) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
            "pss_enc_pwe_data_for_sync_with_standby()");
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      ncs_enc_claim_space(io_uba, sizeof(uns16));
      tbl_entry_cnt = 0;

      for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
      {
         for(rec = client_node->hash[i], ederror = EDU_NORMAL; rec != NULL; rec = rec->next, ++tbl_entry_cnt)
         {
            rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, io_uba, 
               EDP_OP_TYPE_ENC, &rec->p_oaa_entry->key.mds_dest, &ederror, peer_mbcsv_version);
            if(rc != NCSCC_RC_SUCCESS)
            {
               m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_ENC_FAIL, pwe_cb->pwe_id);
               return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_ENC_SUCCESS, pwe_cb->pwe_id);

            if((data = ncs_enc_reserve_space(io_uba, sizeof(uns32))) == NULL)
            {
               m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
                  "pss_enc_pwe_data_for_sync_with_standby()");
               m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_TBL_ID_ENC_FAIL, rec->tbl_id);
               return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            ncs_encode_32bit(&data, (uns32)rec->tbl_id);
            ncs_enc_claim_space(io_uba, sizeof(uns32));
            m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_ENC_SUCCESS, pwe_cb->pwe_id, rec->tbl_id);
         }
      }
      ncs_encode_16bit(&p_tbl_entry_cnt, tbl_entry_cnt);
   }
   ncs_encode_16bit(&p_re_data_cnt, re_data_cnt);

   return rc;
}

/****************************************************************************
 * Name          : pss_dec_pwe_data_for_sync_with_standby
 *
 * Description   : This utility function decodes the pwe-data from buffer, and
 *                 updates it into the PSS_PWE_CB.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 io_uba - Pointer to NCS_UBAID buffer.
 *                 dec_lib_conf - Boolean to know whether pssv_lib_conf contents
 *                                need to be decoded.
 *                 peer_mbcsv_version - MBCSv version of the peer PSS given
 *                                      by MBCSv in decode callback.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_dec_pwe_data_for_sync_with_standby(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba, NCS_BOOL dec_lib_conf, uns16 peer_mbcsv_version)
{
   uns32                      rc = NCSCC_RC_SUCCESS, i = 0, j = 0, async_cnt = 0;
   uns8                       *data = NULL;
   uns16                      pend_wbreq_cnt = 0, re_data_cnt = 0, tbl_cnt = 0;
   PSS_SPCN_WBREQ_PEND_LIST   **pp_wbreq = NULL, *p_wbreq = NULL;
   PSS_OAA_ENTRY              *oaa_node = NULL;
   PSS_CLIENT_ENTRY           *client_node = NULL;
   MDS_DEST                   lcl_mdest;
   EDU_ERR                    ederror = EDU_NORMAL;
   NCS_BOOL                   flush_re_pat_trees = TRUE;
   uns32                      tbl_id = 0;

   /* Decode async_cnt */
   if((data = ncs_dec_flatten_space(io_uba, (uns8*)&async_cnt, sizeof(uns32))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_dec_pwe_data_for_sync_with_standby()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   async_cnt = ncs_decode_32bit(&data);
   ncs_dec_skip_space(io_uba, sizeof(uns32));

   if(pwe_cb->pwe_id == 1)
   {
      /* Decode pwe_cb->p_pss_cb->save_type in a 8-bit form. */
      if((data = ncs_dec_flatten_space(io_uba, (uns8*)&pwe_cb->p_pss_cb->save_type, sizeof(uns8))) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
            "pss_dec_pwe_data_for_sync_with_standby()");
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      pwe_cb->p_pss_cb->save_type = ncs_decode_8bit(&data);
      ncs_dec_skip_space(io_uba, sizeof(uns8));

      rc = pss_ckpt_dec_n_updt_reload_pssvspcnlist(pwe_cb, io_uba);
      if(rc != NCSCC_RC_SUCCESS)
      {
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      if(dec_lib_conf == TRUE)
      {
         rc = pss_ckpt_dec_n_updt_reload_pssvlibconf(pwe_cb, io_uba);
         if(rc != NCSCC_RC_SUCCESS)
         {
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
      }
   }

   /* Decode pwe_cb->spcn_wbreq_pend_list */
   if((data = ncs_dec_flatten_space(io_uba, (uns8*)&pend_wbreq_cnt, sizeof(uns16))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_dec_pwe_data_for_sync_with_standby()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   pend_wbreq_cnt = ncs_decode_16bit(&data);
   ncs_dec_skip_space(io_uba, sizeof(uns16));
   if(pwe_cb->spcn_wbreq_pend_list != NULL)
   {
      /* Clean the list. */
      pss_destroy_spcn_wbreq_pend_list(pwe_cb->spcn_wbreq_pend_list);
      pwe_cb->spcn_wbreq_pend_list = NULL;
   }
   pp_wbreq = &pwe_cb->spcn_wbreq_pend_list;
   for(i = 0; i < pend_wbreq_cnt; i++, pp_wbreq = &(*pp_wbreq)->next)
   {
      if((p_wbreq = m_MMGR_ALLOC_PSS_SPCN_WBREQ_PEND_LIST) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MMGR_SPCN_WBREQ_PEND_LIST_FAIL,
            "pss_dec_pwe_data_for_sync_with_standby()");
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      memset(p_wbreq, '\0', sizeof(PSS_SPCN_WBREQ_PEND_LIST));
      if(pss_decode_pcn(io_uba, &p_wbreq->pcn) != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_PSS_SPCN_WBREQ_PEND_LIST(p_wbreq);
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      *pp_wbreq = p_wbreq;
   }

   /* Decode both client_table and oaa_tree entries. Here, the entry looks like:
    *    <count-of-pcns> [<PCN> <count-of-tables> [<MDS_DEST> <table-id>] ...] 
    */
   if((data = ncs_dec_flatten_space(io_uba, (uns8*)&re_data_cnt, sizeof(uns16))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_dec_pwe_data_for_sync_with_standby()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   re_data_cnt = ncs_decode_16bit(&data);
   ncs_dec_skip_space(io_uba, sizeof(uns16));
   for(i = 0; i < re_data_cnt; i++)
   {
      char *p_pcn = NULL;
      PSS_RET_VAL prc = PSS_RET_NORMAL;

      if(pss_decode_pcn(io_uba, &p_pcn) != NCSCC_RC_SUCCESS)
      {
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }

      if((data = ncs_dec_flatten_space(io_uba, (uns8*)&tbl_cnt, sizeof(uns16))) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
            "pss_dec_pwe_data_for_sync_with_standby()");
         m_MMGR_FREE_MAB_PCN_STRING(p_pcn);
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      tbl_cnt = ncs_decode_16bit(&data);
      ncs_dec_skip_space(io_uba, sizeof(uns16));

      for(j = 0; j < tbl_cnt; j++)
      {
         MDS_DEST **pp_mdest = NULL, *p_mdest = NULL;

         ederror = EDU_NORMAL;
         memset(&lcl_mdest, '\0', sizeof(lcl_mdest));
         p_mdest = &lcl_mdest;
         pp_mdest = &p_mdest;
         rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, io_uba, 
            EDP_OP_TYPE_DEC, pp_mdest, &ederror, peer_mbcsv_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_DEC_FAIL, pwe_cb->pwe_id);
            m_MMGR_FREE_MAB_PCN_STRING(p_pcn);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_DEC_SUCCESS, pwe_cb->pwe_id);

         if((data = ncs_dec_flatten_space(io_uba, (uns8*)&tbl_id, sizeof(uns32))) == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
               "pss_dec_pwe_data_for_sync_with_standby()");
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_TBL_ID_DEC_FAIL, pwe_cb->pwe_id);
            m_MMGR_FREE_MAB_PCN_STRING(p_pcn);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         tbl_id = ncs_decode_32bit(&data);
         ncs_dec_skip_space(io_uba, sizeof(uns32));
         m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_DEC_SUCCESS, pwe_cb->pwe_id, tbl_id);

         /* Populate the oaa_tree and client_table now */
         if(flush_re_pat_trees)
         {
            /* Empty the tree before we starting filling again. This will be done only once
               for PWE. */
            pss_destroy_pwe_oaa_tree(&pwe_cb->oaa_tree, FALSE);
            pss_destroy_pwe_client_table(&pwe_cb->client_table, FALSE);
            flush_re_pat_trees = FALSE;
            m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_FLUSHED_BEFORE_POPULATING_RE_DATA, pwe_cb->pwe_id, tbl_id);
         }

         /* Add the client_table node now */
         client_node = pss_find_client_entry(pwe_cb, p_pcn, TRUE);
         if(client_node == NULL)
         {
            m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_RE_PCN_ADD_FAIL, p_pcn);
            m_MMGR_FREE_MAB_PCN_STRING(p_pcn);
            pss_destroy_pwe_oaa_tree(&pwe_cb->oaa_tree, FALSE);
            pss_destroy_pwe_client_table(&pwe_cb->client_table, FALSE);
            /* What to do? */
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         /* Add the OAA_tree node now. */
         oaa_node = pss_findadd_entry_in_oaa_tree(pwe_cb, &lcl_mdest, TRUE);
         if(oaa_node == NULL)
         {
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_RE_OAA_TREE_ADD_FAIL, pwe_cb->pwe_id);
            m_MMGR_FREE_MAB_PCN_STRING(p_pcn);
            pss_destroy_pwe_oaa_tree(&pwe_cb->oaa_tree, FALSE);
            pss_destroy_pwe_client_table(&pwe_cb->client_table, FALSE);
            /* What to do? */
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         if(pss_find_table_tree(pwe_cb, client_node, oaa_node, 
                        tbl_id, TRUE, &prc) == NULL)
         {
            if(prc != PSS_RET_MIB_DESC_NULL)
            {
               m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_RE_TBL_REC_ADD_FAIL, pwe_cb->pwe_id, tbl_id);
               m_MMGR_FREE_MAB_PCN_STRING(p_pcn);
               pss_destroy_pwe_oaa_tree(&pwe_cb->oaa_tree, FALSE);
               pss_destroy_pwe_client_table(&pwe_cb->client_table, FALSE);
               /* What to do? */
               return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            m_LOG_PSS_HDLN2_II(NCSFL_SEV_NOTICE, PSS_HDLN2_RE_TBL_REC_ADD_FAIL_MIB_DESC_NULL, pwe_cb->pwe_id, tbl_id);
         }
         else
         {
            m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_RE_TBL_REC_ADD_SUCCESS, pwe_cb->pwe_id, tbl_id);
         }
      }
      m_MMGR_FREE_MAB_PCN_STRING(p_pcn);
   }

   pwe_cb->async_cnt = async_cnt;

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_encode_async_update
 *
 * Description   : This function encodes data to be sent as an async update.
 *                 The caller of this function would set the address of the
 *                 record to be encoded in the reo_hdl field(while invoking
 *                 SEND_CKPT option of ncs_mbcsv_svc. 
 *
 * Arguments     : cbk_arg - Pointer to NCS MBCSV callback argument struct.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/
uns32 pss_ckpt_encode_async_update(NCS_MBCSV_CB_ARG *cbk_arg)
{
   PSS_CKPT_MSG *data=NULL;
   uns32        rc = NCSCC_RC_SUCCESS;
   PSS_PWE_CB   *pwe_cb = NULL;

   /* Set reo_hdl from callback arg to ckpt_rec */
   data=(PSS_CKPT_MSG *)((long)cbk_arg->info.encode.io_reo_hdl);
   if (data == NULL)
       return NCSCC_RC_FAILURE;
 
   if (NULL == (pwe_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_PSS,cbk_arg->i_client_hdl)))
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_PWE_CB_TAKE_HANDLE_IN_MBCSVCB_FAILED, pwe_cb->pwe_id);
      return NCSCC_RC_FAILURE;
   }

   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ASYNC_ENC_CALLBACK_EVT_START, pwe_cb->pwe_id, data->ckpt_data_type);
   switch(data->ckpt_data_type)
   {
   case PSS_CKPT_DATA_TBL_BIND:
      rc = pss_ckpt_enc_tbl_bind(pwe_cb, &cbk_arg->info.encode.io_uba,
         (PSS_CKPT_TBL_BIND_MSG*)&data->ckpt_data.tbl_bind, cbk_arg->info.encode.i_peer_version);
      break;

   case PSS_CKPT_DATA_TBL_UNBIND:
      rc = pss_ckpt_enc_tbl_unbind(pwe_cb, &cbk_arg->info.encode.io_uba,
                     (PSS_CKPT_TBL_UNBIND_MSG*)&data->ckpt_data.tbl_unbind, cbk_arg->info.encode.i_peer_version);
      break;

   case PSS_CKPT_DATA_OAA_DOWN:
      rc = pss_ckpt_enc_oaa_down(pwe_cb, &cbk_arg->info.encode.io_uba,
                     (PSS_CKPT_OAA_DOWN_MSG*)&data->ckpt_data.oaa_down, cbk_arg->info.encode.i_peer_version);
      break;

   case PSS_CKPT_DATA_PLBCK_SSN_INFO:
      rc = pss_ckpt_enc_plbck_ssn_info(pwe_cb, &cbk_arg->info.encode.io_uba,
                     (PSS_CKPT_PLBCK_SSN_INFO_MSG*)&data->ckpt_data.plbck_ssn_info, cbk_arg->info.encode.i_peer_version);
      break;

   case PSS_CKPT_DATA_PEND_WBREQ_TO_BAM:
      rc = pss_ckpt_enc_pend_wbreq_to_bam_info(&cbk_arg->info.encode.io_uba,
                     (PSS_CKPT_PEND_WBREQ_TO_BAM_MSG*)&data->ckpt_data.pend_wbreq_to_bam);
      break;

   case PSS_CKPT_DATA_BAM_CONF_DONE:
      rc = pss_ckpt_enc_bam_conf_done(&cbk_arg->info.encode.io_uba,
                     (PSS_CKPT_BAM_CONF_DONE_MSG*)&data->ckpt_data.bam_conf_done);
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVLIBCONF:
      rc = pss_ckpt_enc_reload_pssvlibconf(&cbk_arg->info.encode.io_uba,
                     (char*)data->ckpt_data.reload_pssvlibconf.lib_conf_file);
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVSPCNLIST:
      rc = pss_ckpt_enc_reload_pssvspcnlist(&cbk_arg->info.encode.io_uba,
                     (PSS_SPCN_LIST*)data->ckpt_data.reload_pssvspcnlist.spcn_list);
      break;

   default:
      rc = NCSCC_RC_FAILURE;
      m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ASYNC_ENC_CALLBACK_INVALID_EVT, pwe_cb->pwe_id, data->ckpt_data_type);
      break;
   }
   m_LOG_PSS_HDLN_III(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ASYNC_ENC_CALLBACK_EVT_STATUS, pwe_cb->pwe_id, data->ckpt_data_type, rc);

   ncshm_give_hdl(cbk_arg->i_client_hdl);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_decode_async_update
 *
 * Description   : This function decodes data to be sent as an async update.
 *                 The caller of this function would set the address of the
 *                 record to be encoded in the reo_hdl field(while invoking
 *                 SEND_CKPT option of ncs_mbcsv_svc. 
 *
 * Arguments     : cbk_arg - Pointer to NCS MBCSV callback argument struct.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/
uns32 pss_ckpt_decode_async_update(NCS_MBCSV_CB_ARG *cbk_arg)
{
   PSS_CKPT_DATA_TYPE   type;
   PSS_CKPT_MSG         data;
   PSS_PWE_CB           *pwe_cb = NULL;
   uns32                rc = NCSCC_RC_SUCCESS;

   type = (PSS_CKPT_DATA_TYPE)cbk_arg->info.decode.i_reo_type;

   memset(&data, '\0', sizeof(data));

   /* Get PSS_PWE_CB control block Handle. cbk_arg->arg->i_client_hdl! */
   if (NULL == (pwe_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_PSS,cbk_arg->i_client_hdl)))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_PWE_CB_TAKE_HANDLE_IN_MBCSVCB_FAILED);
      return NCSCC_RC_FAILURE;
   }

   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ASYNC_DEC_CALLBACK_EVT_START, pwe_cb->pwe_id, type);
   switch(type)
   {
   case PSS_CKPT_DATA_TBL_BIND:
      rc = pss_ckpt_dec_tbl_bind(pwe_cb, &cbk_arg->info.decode.i_uba,
                     (PSS_CKPT_TBL_BIND_MSG*)&data.ckpt_data.tbl_bind, cbk_arg->info.decode.i_peer_version);
      break;

   case PSS_CKPT_DATA_TBL_UNBIND:
      rc = pss_ckpt_dec_tbl_unbind(pwe_cb, &cbk_arg->info.decode.i_uba,
                     (PSS_CKPT_TBL_UNBIND_MSG*)&data.ckpt_data.tbl_unbind, cbk_arg->info.decode.i_peer_version);
      break;

   case PSS_CKPT_DATA_OAA_DOWN:
      rc = pss_ckpt_dec_oaa_down(pwe_cb, &cbk_arg->info.decode.i_uba,
                     (PSS_CKPT_OAA_DOWN_MSG*)&data.ckpt_data.oaa_down, cbk_arg->info.decode.i_peer_version);
      break;

   case PSS_CKPT_DATA_PLBCK_SSN_INFO:
      rc = pss_ckpt_dec_plbck_ssn_info(pwe_cb, &cbk_arg->info.decode.i_uba,
                     (PSS_CKPT_PLBCK_SSN_INFO_MSG*)&data.ckpt_data.plbck_ssn_info, cbk_arg->info.decode.i_peer_version);
      break;

   case PSS_CKPT_DATA_PEND_WBREQ_TO_BAM:
      rc = pss_ckpt_dec_pend_wbreq_to_bam_info(&cbk_arg->info.decode.i_uba,
                     (PSS_CKPT_PEND_WBREQ_TO_BAM_MSG*)&data.ckpt_data.pend_wbreq_to_bam);
      break;

   case PSS_CKPT_DATA_BAM_CONF_DONE:
      rc = pss_ckpt_dec_bam_conf_done(&cbk_arg->info.decode.i_uba,
                     (PSS_CKPT_BAM_CONF_DONE_MSG*)&data.ckpt_data.bam_conf_done);
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVLIBCONF:
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVSPCNLIST:
      break;

   default:
      rc = NCSCC_RC_FAILURE;
      m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ASYNC_DEC_CALLBACK_INVALID_EVT, pwe_cb->pwe_id, type);
      break;
   }
   m_LOG_PSS_HDLN_III(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ASYNC_DEC_CALLBACK_EVT_STATUS, pwe_cb->pwe_id, type, rc);

   if(rc != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(cbk_arg->i_client_hdl);
      return rc;
   }

   /*  Release the Handle, as we need to take this handle again
      in pss_do_evt() call */
   ncshm_give_hdl(cbk_arg->i_client_hdl);

   rc = pss_ckpt_convert_n_process_event(pwe_cb, &data, cbk_arg);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_decode_cbk_handler
 *
 * Description   : This function is the single entry point to all decode
 *                 requests from mbcsv. 
 *                 Invokes the corresponding decode routine based on the 
 *                 MBCSv decode request type.
 *
 * Arguments     : arg - Pointer to NCS_MBCSV_CB_ARG with decode info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_decode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg, NCS_BOOL *is_in_sync)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SaAisErrorT    saf_status = SA_AIS_OK; 
   PSS_PWE_CB *pwe_cb = NULL;
   uns16      mbcsv_version = 0;

   memset(addrstr, '\0', 255); 
   m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_CB_INVOKED);

   mbcsv_version =  m_NCS_MBCSV_FMT_GET(cbk_arg->info.decode.i_peer_version,
                                           PSS_MBCSV_VERSION,
                                           PSS_MBCSV_VERSION_MIN);
   if(0 == mbcsv_version)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_MBCSV_PEER_VER, cbk_arg->info.decode.i_peer_version);
      return NCSCC_RC_FAILURE;
   }

   /* Get PSS_PWE_CB control block Handle. cbk_arg->arg->i_client_hdl! */
   if (NULL == (pwe_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_PSS,cbk_arg->i_client_hdl)))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_PWE_CB_TAKE_HANDLE_IN_MBCSVCB_FAILED);
      return NCSCC_RC_FAILURE;
   }

   switch(cbk_arg->info.decode.i_msg_type)
   {
   case NCS_MBCSV_MSG_ASYNC_UPDATE:
      /* Decode async update */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_ASYNC_UPDATE_DEC_STARTED, pwe_cb->pwe_id);
      rc = pss_ckpt_decode_async_update(cbk_arg);
      if(rc != NCSCC_RC_SUCCESS)
      {
         /* This is fatal error. Report to AMF. */
         if(gl_pss_amf_attribs.amf_attribs.amfHandle != 0)
         {
            saf_status = saAmfComponentErrorReport(gl_pss_amf_attribs.amf_attribs.amfHandle,
               &gl_pss_amf_attribs.amf_attribs.compName,
               0, /* errorDetectionTime; AvNd will provide this value */
               SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
               /* log a notify headline, which says that an error report has been sent */ 
            m_LOG_PSS_HDLN2_III(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ERR_IND_AMF_ERR_REPORT, 
               pwe_cb->pwe_id, cbk_arg->info.error.i_code, saf_status);
         }
         sprintf(addrstr, "PWE-id:%d, STATUS: FAILURE", pwe_cb->pwe_id);
         ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,NCSFL_LC_HEADLINE,
                    NCSFL_SEV_ERROR, NCSFL_TYPE_TIC, PSS_HDLN_ASYNC_UPDATE_DEC_STATUS, addrstr);

      }

      break;
      
   case NCS_MBCSV_MSG_COLD_SYNC_REQ:
      /* Decode cold sync request */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_COLD_SYNC_REQ_DEC_DONE, pwe_cb->pwe_id);
      break;
      
   case NCS_MBCSV_MSG_COLD_SYNC_RESP:
   case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
      if(pwe_cb->ckpt_state != PSS_COLD_SYNC_COMPLETE)
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_COLD_SYNC_RESP_DEC_STARTED, pwe_cb->pwe_id);
         rc = pss_ckpt_dec_coldsync_resp(pwe_cb, &cbk_arg->info.decode.i_uba, cbk_arg->info.decode.i_peer_version);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_COLD_SYNC_RESP_DEC_FAIL, pwe_cb->pwe_id, rc);
            pwe_cb->ckpt_state = PSS_COLD_SYNC_FAIL;
            sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
            ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                       NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr); 
         }
         else
         {
            m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_COLD_SYNC_RESP_DEC_SUCCESS, pwe_cb->pwe_id);
            pwe_cb->ckpt_state = PSS_COLD_SYNC_COMPLETE;
            pwe_cb->is_cold_sync_done = TRUE;
            sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
            ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                       NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr);
            if(pwe_cb->is_amfresponse_for_active_pending)
            {
               /* If there is a pending AMF response to be sent, do it now */
               saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, 
                  pwe_cb->amf_invocation_id, SA_AIS_OK);
               pwe_cb->is_amfresponse_for_active_pending = FALSE;
               m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_INVKD_PENDING_AMF_RESP, pwe_cb->pwe_id);
            }
         }
      }
      else
      {
         /* Do nothing */;;
         /* Log it. Ideally, this shouldn't occur.*/
         m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_COLD_SYNC_RESP_DEC_IN_CKPT_STATE, pwe_cb->pwe_id, pwe_cb->ckpt_state);
      } 
      break;
      
   case NCS_MBCSV_MSG_WARM_SYNC_REQ:
      /* Decode warm sync 'request' data */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_WARM_SYNC_REQ_DEC_DONE, pwe_cb->pwe_id);
      break;
      
   case NCS_MBCSV_MSG_WARM_SYNC_RESP:
   case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
      if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_STANDBY)
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_WARM_SYNC_RESP_DEC_STARTED, pwe_cb->pwe_id);
         rc = pss_ckpt_dec_warmsync_resp(pwe_cb, &cbk_arg->info.decode.i_uba,  is_in_sync);

         if (rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_WARM_SYNC_RESP_DEC_FAIL, pwe_cb->pwe_id, rc);
            pwe_cb->ckpt_state = PSS_WARM_SYNC_FAIL;
            sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
            ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                       NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr); 
         }
         else
         {
            sprintf(addrstr, "PWE-id:%d, STATUS: SUCCESS", pwe_cb->pwe_id);
            ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG,
                       NCSFL_TYPE_TIC, PSS_HDLN_WARM_SYNC_RESP_DEC_SUCCESS, addrstr);
            memset(addrstr, '\0', 255);           
            if(*is_in_sync == TRUE)
            {
               pwe_cb->ckpt_state = PSS_WARM_SYNC_COMPLETE;
               sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                          NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                          NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr);
            }
            else
            {
               pwe_cb->ckpt_state = PSS_WARM_SYNC_WAIT_FOR_DATA_RESP;
               pwe_cb->warm_sync_in_progress = TRUE;
               sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                          NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                          NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr);

               if(pss_send_data_req(pwe_cb->ckpt_hdl)
                  != NCSCC_RC_SUCCESS)
               {
                  m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_SEND_DATA_REQ_FAIL, pwe_cb->pwe_id);
                  pwe_cb->ckpt_state = PSS_WARM_SYNC_FAIL;
                  memset(addrstr, '\0', 255); 
                  sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
                  ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                             NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                             NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr);

                  ncshm_give_hdl(cbk_arg->i_client_hdl);
                  return NCSCC_RC_FAILURE;
               }
            }
         }
      }
      else
      { 
         /* Log it. Ideally, this shouldn't occur. */
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_WARM_SYNC_RESP_DEC_IN_ACTIVE_ROLE, pwe_cb->pwe_id);
      } 
      break;

   case NCS_MBCSV_MSG_DATA_REQ:
      /* Decode Data 'request' data */
      if((pwe_cb->p_pss_cb->ha_state != SA_AMF_HA_ACTIVE) &&
         (pwe_cb->p_pss_cb->ha_state != SA_AMF_HA_QUIESCING))
      {
         /* Do nothing */;;
         /* Log it. Ideally, this shouldn't occur. */
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_DATA_REQ_DEC_IN_NON_ACTIVE_ROLE, pwe_cb->pwe_id);
         ncshm_give_hdl(cbk_arg->i_client_hdl);
         return NCSCC_RC_FAILURE;
      }
      m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_DATA_REQ_DEC_DONE, pwe_cb->pwe_id);
      break;

   case NCS_MBCSV_MSG_DATA_RESP:
   case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
      if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_STANDBY)
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_DATA_RESP_DEC_STARTED, pwe_cb->pwe_id);
         rc = pss_ckpt_dec_data_resp(pwe_cb, &cbk_arg->info.decode.i_uba, cbk_arg->info.decode.i_peer_version);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_DATA_RESP_DEC_FAIL, pwe_cb->pwe_id, rc);
            if(pwe_cb->ckpt_state == PSS_COLD_SYNC_COMPLETE)
            {
               /* Cold-sync was successful, but Warm-sync failed. */
               m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, 
                 PSS_HDLN_DATA_RESP_DEC_FAIL_WITH_COLD_SYNC_COMPLT_BUT_WARM_SYNC_FAIL, pwe_cb->pwe_id);
               pwe_cb->ckpt_state = PSS_WARM_SYNC_DATA_RESP_FAIL;
               sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                          NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr);
            }
            else
            {
               pwe_cb->ckpt_state = PSS_COLD_SYNC_FAIL;
               sprintf(addrstr, "PWE-id:%d, CHK PT STATE: %s ", pwe_cb->pwe_id, chk_pt_states[pwe_cb->ckpt_state]);
               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                          NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          NCSFL_TYPE_TIC, PSS_HDLN_CKPT_STATE, addrstr);
            }
            if(gl_pss_amf_attribs.amf_attribs.amfHandle != 0)
            {
               saf_status = saAmfComponentErrorReport(gl_pss_amf_attribs.amf_attribs.amfHandle,
                  &gl_pss_amf_attribs.amf_attribs.compName,
                  0, /* errorDetectionTime; AvNd will provide this value */
                  SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
               /* log a notify headline, which says that an error report has been sent */ 
               m_LOG_PSS_HDLN2_III(NCSFL_SEV_NOTICE, PSS_HDLN_MBCSV_ERR_IND_AMF_ERR_REPORT, 
                  pwe_cb->pwe_id, cbk_arg->info.error.i_code, saf_status);
            }
            ncshm_give_hdl(cbk_arg->i_client_hdl);
            return rc;
         }
         else
         {
            m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_DATA_RESP_DEC_SUCCESS, pwe_cb->pwe_id);
            pwe_cb->ckpt_state = PSS_WARM_SYNC_COMPLETE;
            pwe_cb->warm_sync_in_progress = FALSE;
         }
      }
      else
      { 
         /* Do nothing */;;
         /* Log it. Ideally, this shouldn't occur.*/
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_DATA_RESP_DEC_IN_NON_STANDBY_ROLE, pwe_cb->pwe_id);
      } 
      break;
      
      
   default:
      /* Log Failure */
      rc = NCSCC_RC_FAILURE;
      m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_INVLD_DEC_MSG_TYPE,
         pwe_cb->pwe_id, cbk_arg->info.decode.i_msg_type);
      break;
   } /*End switch(io_msg_type)*/

   ncshm_give_hdl(cbk_arg->i_client_hdl);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_coldsync_resp
 *
 * Description   : This function decodes cold sync resp data., viz
 *                 checksum for PWE-CB.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 cbk_arg - Pointer to NCS_MBCSV_CB_ARG with encode info.
 *                 peer_mbcsv_version - MBCSv version registered by Peer PSS.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_coldsync_resp(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba, uns16 peer_mbcsv_version)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_BOOL   dec_lib_conf = FALSE;

   /* Currently, we are sending all the PWE-data in one send.
    * This shall avoid "delta data" problems that are associated during
    * multiple sends.
    */
    if(peer_mbcsv_version == 1)
       dec_lib_conf = TRUE;

   if(pss_dec_pwe_data_for_sync_with_standby(pwe_cb, io_uba, dec_lib_conf, peer_mbcsv_version) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_COLD_SYNC_RESP_PAYLOAD_FAIL, pwe_cb->pwe_id);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_warmsync_resp
 *
 * Description   : This function decodes warm sync resp data., viz
 *                 checksum for PWE-CB.
 *
 * Arguments     : pwe_cb - pointer to the PSS PWE control block. 
 *                 cbk_arg - Pointer to NCS_MBCSV_CB_ARG with encode info.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_warmsync_resp(PSS_PWE_CB *pwe_cb, NCS_UBAID *io_uba, NCS_BOOL *is_in_sync)
{
   uns32 rc = NCSCC_RC_SUCCESS, async_cnt = 0;
   uns8 *data = NULL;

   if((data = ncs_dec_flatten_space(io_uba, (uns8*)&async_cnt, sizeof(uns32))) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_warmsync_resp()");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   async_cnt = ncs_decode_32bit(&data);
   ncs_dec_skip_space(io_uba, sizeof(uns32));

   (*is_in_sync) = TRUE;
   if(async_cnt != pwe_cb->async_cnt)
      (*is_in_sync) = FALSE;
   else if((pwe_cb->ckpt_state == PSS_COLD_SYNC_FAIL) ||
           (pwe_cb->ckpt_state == PSS_WARM_SYNC_FAIL) ||
           (pwe_cb->ckpt_state == PSS_WARM_SYNC_DATA_RESP_FAIL) ||
           (pwe_cb->ckpt_state == PSS_WARM_SYNC_WAIT_FOR_DATA_RESP))
      (*is_in_sync) = FALSE;

   return rc;
}

/****************************************************************************\
 * Function: pss_set_ckpt_role
 *
 * Purpose:  Set checkpoint role.
 *
 * Input: pwe_cb   - PSS PWE control block pointer.
 *        role     - Role to be set.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 pss_set_ckpt_role(PSS_PWE_CB *pwe_cb, SaAmfHAStateT role)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_CHG_ROLE;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   mbcsv_arg.info.chg_role.i_ckpt_hdl = pwe_cb->ckpt_hdl;
   mbcsv_arg.info.chg_role.i_ha_state = role;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_SET_CKPT_ROLE_FAIL, role);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_SET_CKPT_ROLE_SUCCESS, pwe_cb->pwe_id, role);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: pss_send_data_req
 *
 * Purpose:  Send data-req to Active.
 *
 * Input: pwe_cb        - PSS PWE control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 pss_send_data_req(NCS_MBCSV_CKPT_HDL ckpt_hdl)
{
   NCS_MBCSV_ARG     mbcsv_arg;
   NCS_UBAID         *uba = NULL;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_DATA_REQ;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   mbcsv_arg.info.send_data_req.i_ckpt_hdl = ckpt_hdl;
   uba = &mbcsv_arg.info.send_data_req.i_uba;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_SEND_DATA_REQ_FAIL);
      return NCSCC_RC_FAILURE;
   }

   m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_SEND_DATA_REQ_SUCCESS);
   return NCSCC_RC_SUCCESS;
}

/******************************************************************************
 * Name          : pss_ckpt_peer_info_cbk_handler 
 *
 * Description   : This callback is invoked by mbcsv when a peer info message
 *                 is received from PSS STANDBY. 
 *
 * Arguments     : NCS_MBCSV_ARG containing info pertaining to the STANDBY peer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_peer_info_cbk_handler(NCS_MBCSV_CB_ARG *arg)
{
   /* Currently nothing to be done */
   m_LOG_PSS_HEADLINE(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_PERR_INFO_CB_INVOKED);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_ckpt_notify_cbk_handler 
 *
 * Description   : This callback is invoked by mbcsv when a notify message
 *                 is received from PSS STANDBY. 
 *
 *
 * Arguments     : NCS_MBCSV_ARG - contains notification info from STANDBY.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_notify_cbk_handler (NCS_MBCSV_CB_ARG *arg)
{
   /* Currently nothing to be done */
   m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MBCSV_NOTIFY_CB_INVOKED);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : pss_ckpt_err_ind_cbk_handler
 *
 * Description   : This callback is invoked by mbcsv when a notify message
 *                 is received from PSS STANDBY. 
 *
 *
 * Arguments     : NCS_MBCSV_ARG - contains notification info from STANDBY.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_err_ind_cbk_handler (NCS_MBCSV_CB_ARG *arg)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Currently nothing to be done. */
   m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_MBCSV_ERR_IND_CB_INVOKED);

   rc = pss_red_err_ind_handle(arg);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_convert_n_process_event 
 *
 * Description   : Utility function to process async update ckpt events. 
 *
 *
 * Arguments     : pwe_cb  - Pointer to PSS_PWE_CB
 *                 data    - Pointer to the async-update msg PSS_CKPT_MSG
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_convert_n_process_event(PSS_PWE_CB *pwe_cb, PSS_CKPT_MSG *data, 
                                       NCS_MBCSV_CB_ARG *cbk_arg)
{
   uns32          rc = NCSCC_RC_SUCCESS;
   PSS_CKPT_DATA_TYPE type = 0;
   MAB_MSG        mab_msg;

   memset(&mab_msg, '\0', sizeof(mab_msg));

   /*  Now we will be passing PWE handle rather than its pointer */
   mab_msg.yr_hdl = NCS_INT64_TO_PTR_CAST(cbk_arg->i_client_hdl);

   type = (PSS_CKPT_DATA_TYPE)cbk_arg->info.decode.i_reo_type;
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ASYNC_EVT_HANDLING_START, pwe_cb->pwe_id, type);
   switch(type)
   {
   case PSS_CKPT_DATA_TBL_BIND:
      mab_msg.op = MAB_PSS_TBL_BIND;
      mab_msg.fr_card = data->ckpt_data.tbl_bind.fr_card;
      /* Memory ownership is transferred here. */
      mab_msg.data.data.oac_pss_tbl_bind.pcn_list = data->ckpt_data.tbl_bind.pcn_list;
      /* rc = pss_do_evt(&mab_msg, FALSE);*/ /* to ignore zero MDS_DEST error case */
      pss_do_evt(&mab_msg, FALSE);
      break;

   case PSS_CKPT_DATA_TBL_UNBIND:
      mab_msg.op = MAB_PSS_TBL_UNBIND;
      mab_msg.fr_card = data->ckpt_data.tbl_unbind.fr_card;
      mab_msg.data.data.oac_pss_tbl_unbind.tbl_id = data->ckpt_data.tbl_unbind.tbl_id;
      /*rc = pss_do_evt(&mab_msg, FALSE);*/ /*  to ignore zero MDS_DEST error case */
      pss_do_evt(&mab_msg, FALSE);
      break;

   case PSS_CKPT_DATA_OAA_DOWN:
      mab_msg.op = MAB_PSS_SVC_MDS_EVT;
      mab_msg.fr_svc = NCSMDS_SVC_ID_OAC;
      mab_msg.fr_card = data->ckpt_data.oaa_down.fr_card;
      mab_msg.data.data.pss_mds_svc_evt.change = NCSMDS_DOWN;
      /*rc = pss_do_evt(&mab_msg, FALSE);*/ /* to ignore zero MDS_DEST error case */
      pss_do_evt(&mab_msg, FALSE);
      break;

   case PSS_CKPT_DATA_PLBCK_SSN_INFO:
      pwe_cb->async_cnt ++; /* Update checksum for non-pss_do_evt() invoking code flows */
      rc = pss_process_re_plbck_ssn_info(pwe_cb, data);
      break;

   case PSS_CKPT_DATA_BAM_CONF_DONE:
      mab_msg.op = MAB_PSS_BAM_CONF_DONE;
      /* Memory ownership is transferred here. */
      mab_msg.data.data.bam_conf_done.pcn_list.pcn = data->ckpt_data.bam_conf_done.pcn_list.pcn;
      /*rc = pss_do_evt(&mab_msg, FALSE);*/ /*  to ignore zero MDS_DEST error case */
      pss_do_evt(&mab_msg, FALSE);
      break;

   case PSS_CKPT_DATA_PEND_WBREQ_TO_BAM:
      pwe_cb->async_cnt ++; /* Update checksum for non-pss_do_evt() invoking code flows */
      rc = pss_re_update_pend_wbreq_info(pwe_cb, &data->ckpt_data.pend_wbreq_to_bam,
              cbk_arg->info.decode.i_action);
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVLIBCONF:
      pwe_cb->async_cnt ++; /* Update checksum for non-pss_do_evt() invoking code flows */
      rc = pss_ckpt_dec_n_updt_reload_pssvlibconf(pwe_cb, &cbk_arg->info.decode.i_uba);
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVSPCNLIST:
      pwe_cb->async_cnt ++; /* Update checksum for non-pss_do_evt() invoking code flows */
      rc = pss_ckpt_dec_n_updt_reload_pssvspcnlist(pwe_cb, &cbk_arg->info.decode.i_uba);
      break;

   default:
      rc = NCSCC_RC_FAILURE;
      m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ASYNC_EVT_HANLDING_EVT_INVALID, pwe_cb->pwe_id, data->ckpt_data_type);
      break;
   }
   m_LOG_PSS_HDLN_III(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ASYNC_EVT_HANDLING_STATUS, pwe_cb->pwe_id, type, rc);

   return rc;
}

/****************************************************************************
 * Name          : pss_process_re_plbck_ssn_info 
 *
 * Description   : Utility function to process Plbck-ssn-info async-message
 *                 from Active.
 *
 * Arguments     : pwe_cb  - Pointer to PSS_PWE_CB
 *                 data    - Pointer to the async-update msg PSS_CKPT_MSG
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_process_re_plbck_ssn_info(PSS_PWE_CB *pwe_cb, PSS_CKPT_MSG *data)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   switch(data->ckpt_data.plbck_ssn_info.plbck_ssn_in_progress)
   {
   case TRUE:
      switch(pwe_cb->curr_plbck_ssn_info.plbck_ssn_in_progress)
      {
      case TRUE:
         /* Modifications to the ssn_info is happening now. */
         rc = pss_mdfy_plbck_info(pwe_cb, data);
         m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_RE_NEW_PLBCK_SSN_DETAILS_MODIFIED, pwe_cb->pwe_id);
         break;
         
      default:    /* FALSE */
         /* New playback session is starting. Copy the info in pwe_cb. */
         pwe_cb->curr_plbck_ssn_info = data->ckpt_data.plbck_ssn_info;
         memset(&data->ckpt_data.plbck_ssn_info, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));
         m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_RE_NEW_PLBCK_SSN_DETAILS_COPIED, pwe_cb->pwe_id);
         break;
      }
      break;

   default:    /* FALSE */
      switch(pwe_cb->curr_plbck_ssn_info.plbck_ssn_in_progress)
      {
      case TRUE:
         /* Playback session is now complete. Free the info from pwe_cb. */
         pss_flush_plbck_ssn_info(&pwe_cb->curr_plbck_ssn_info);
         m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_RE_PLBCK_SSN_DETAILS_FLUSHED, pwe_cb->pwe_id);
         break;
         
      default:    /* FALSE */
         /* Shouldn't arrive here at all!!! Log error. */
         pss_flush_plbck_ssn_info(&data->ckpt_data.plbck_ssn_info);
         m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_RE_PLBCK_SSN_DETAILS_FLUSHED_EVEN_WHEN_NO_PLBCK_ACTIVE, pwe_cb->pwe_id);
         rc = NCSCC_RC_FAILURE;
         break;
      }
      break;
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_mdfy_plbck_info 
 *
 * Description   : Utility function to modify Plbck-ssn-info in PSS_PWE_CB
 *
 * Arguments     : pwe_cb  - Pointer to PSS_PWE_CB
 *                 data    - Pointer to the async-update msg PSS_CKPT_MSG
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_mdfy_plbck_info(PSS_PWE_CB *pwe_cb, PSS_CKPT_MSG *data)
{
   pwe_cb->curr_plbck_ssn_info.plbck_ssn_in_progress = data->ckpt_data.plbck_ssn_info.plbck_ssn_in_progress;
   pwe_cb->curr_plbck_ssn_info.is_warmboot_ssn = data->ckpt_data.plbck_ssn_info.is_warmboot_ssn;
   pwe_cb->curr_plbck_ssn_info.info.info2.partial_delete_done = data->ckpt_data.plbck_ssn_info.info.info2.partial_delete_done;
   pwe_cb->curr_plbck_ssn_info.tbl_id = data->ckpt_data.plbck_ssn_info.tbl_id;
   pwe_cb->curr_plbck_ssn_info.mib_obj_id = data->ckpt_data.plbck_ssn_info.mib_obj_id;

   if((pwe_cb->curr_plbck_ssn_info.is_warmboot_ssn) && 
      (pwe_cb->curr_plbck_ssn_info.info.info2.partial_delete_done))
   {
      /* Free the first request from pwe_cb->curr_plbck_ssn_info.info.info2.wb_req */
      pss_partial_flush_warmboot_plbck_ssn_info(&pwe_cb->curr_plbck_ssn_info);
      pwe_cb->curr_plbck_ssn_info.info.info2.partial_delete_done = FALSE;
      pwe_cb->curr_plbck_ssn_info.info.info2.wbreq_cnt = data->ckpt_data.plbck_ssn_info.info.info2.wbreq_cnt;
   }
   else
   {
      if(pwe_cb->curr_plbck_ssn_info.pcn != NULL)
         m_MMGR_FREE_MAB_PCN_STRING(pwe_cb->curr_plbck_ssn_info.pcn);
      pwe_cb->curr_plbck_ssn_info.pcn = data->ckpt_data.plbck_ssn_info.pcn;

      pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = data->ckpt_data.plbck_ssn_info.mib_idx.i_inst_len;
      if(pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids != NULL)
      {
         m_MMGR_FREE_MIB_INST_IDS(pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids);
      }
      pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = data->ckpt_data.plbck_ssn_info.mib_idx.i_inst_ids;

      /* Free ownership of memory from data->ckpt_data.plbck_ssn_info */
      memset(&data->ckpt_data.plbck_ssn_info, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_flush_plbck_ssn_info 
 *
 * Description   : Utility function to flush PSS_CURR_PLBCK_SSN_INFO data
 *                 structure
 *
 * Arguments     : ssn_info - Pointer to PSS_CURR_PLBCK_SSN_INFO structure
 *
 * Return Values : void
 *
 * Notes         : None.
 *****************************************************************************/
void pss_flush_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *ssn_info)
{
   MAB_PSS_WARMBOOT_REQ *req = NULL, *nxt_req = NULL;

   if(ssn_info->pcn != NULL)
      m_MMGR_FREE_MAB_PCN_STRING(ssn_info->pcn);
   ssn_info->pcn = NULL;

   if(ssn_info->mib_idx.i_inst_ids != NULL)
   {
      m_MMGR_FREE_MIB_INST_IDS(ssn_info->mib_idx.i_inst_ids);
   }
   ssn_info->mib_idx.i_inst_ids = NULL;
   ssn_info->mib_idx.i_inst_len = 0;

   /* Free the Warmboot ssn info */
   if(ssn_info->is_warmboot_ssn)
   {
      req = &ssn_info->info.info2.wb_req;

      if(req->pcn_list.pcn != NULL)
      {
         m_MMGR_FREE_MAB_PCN_STRING(req->pcn_list.pcn);
         req->pcn_list.pcn = NULL;
      }
      if(req->pcn_list.tbl_list != NULL)
      {
         pss_free_tbl_list(req->pcn_list.tbl_list);
         req->pcn_list.tbl_list = NULL;
      }

      req = req->next; /* First element is not an allocated memory element. */
      while(req != NULL)
      {
         nxt_req = req->next;

         if(req->pcn_list.pcn != NULL)
         {
            m_MMGR_FREE_MAB_PCN_STRING(req->pcn_list.pcn);
            req->pcn_list.pcn = NULL;
         }
         if(req->pcn_list.tbl_list != NULL)
         {
            pss_free_tbl_list(req->pcn_list.tbl_list);
            req->pcn_list.tbl_list = NULL;
         }

         m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(req);
         req = nxt_req;
      }
   }

   memset(ssn_info, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));

   return;
}

/****************************************************************************
 * Name          : pss_partial_flush_warmboot_plbck_ssn_info
 *
 * Description   : Utility function to flush PSS_CURR_PLBCK_SSN_INFO data
 *                 structure partially, for the first warmboot-request only.
 *
 * Arguments     : ssn_info - Pointer to PSS_CURR_PLBCK_SSN_INFO structure
 *
 * Return Values : void
 *
 * Notes         : None.
 *****************************************************************************/
void pss_partial_flush_warmboot_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *ssn_info)
{
   MAB_PSS_WARMBOOT_REQ *req = NULL, *nxt_req = NULL;

   if(ssn_info->pcn != NULL)
      m_MMGR_FREE_MAB_PCN_STRING(ssn_info->pcn);
   ssn_info->pcn = NULL;

   if(ssn_info->mib_idx.i_inst_ids != NULL)
   {
      m_MMGR_FREE_MIB_INST_IDS(ssn_info->mib_idx.i_inst_ids);
   }
   ssn_info->mib_idx.i_inst_ids = NULL;
   ssn_info->mib_idx.i_inst_len = 0;

   /* Free the Warmboot ssn info */
   if(ssn_info->is_warmboot_ssn)
   {
      req = &ssn_info->info.info2.wb_req;

      if(req->pcn_list.pcn != NULL)
      {
         m_MMGR_FREE_MAB_PCN_STRING(req->pcn_list.pcn);
         req->pcn_list.pcn = NULL;
      }
      if(req->pcn_list.tbl_list != NULL)
      {
         pss_free_tbl_list(req->pcn_list.tbl_list);
         req->pcn_list.tbl_list = NULL;
      }

      /* Now, cascade the list forward */
      nxt_req = req->next;
      if(nxt_req != NULL)
      {
         *req = *nxt_req; /* cascading done. */
         /* Now, free only the nxt_req pointer, not the entire list. */
         m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(nxt_req);
      }
      else
      {
         memset(ssn_info, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));
      }
   }

   return;
}

/****************************************************************************
 * Name          : pss_re_sync 
 *
 * Description   : Utility function to send async-updates to Standby.
 *
 * Arguments     : pwe_cb     - Pointer to PSS_PWE_CB
 *                 mab_msg    - Pointer to the MAB_MSG
 *                 ckpt_type  - Async-update event type
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_re_sync(PSS_PWE_CB *pwe_cb, MAB_MSG *mab_msg, PSS_CKPT_DATA_TYPE ckpt_type)
{
   PSS_CKPT_MSG      msg;
   NCS_MBCSV_ARG     mbcsv_arg;
   uns32             rc = NCSCC_RC_SUCCESS, len = 0;

   if((pwe_cb == NULL) || (mab_msg == NULL))
      return NCSCC_RC_FAILURE;

   memset(&msg, '\0', sizeof(msg));
   msg.ckpt_data_type = ckpt_type;

   m_LOG_PSS_HDLN2_II(NCSFL_SEV_DEBUG, PSS_HDLN_RE_SYNC_CKPT_EVT_TYPE, pwe_cb->pwe_id, ckpt_type);
   switch(ckpt_type)
   {
   case PSS_CKPT_DATA_TBL_BIND:
      msg.ckpt_data.tbl_bind.fr_card = mab_msg->fr_card;
      rc = pss_duplicate_pcn_list(&mab_msg->data.data.oac_pss_tbl_bind.pcn_list, 
               &msg.ckpt_data.tbl_bind.pcn_list);
      if(rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN2_II(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_CKPT_EVT_POPULATE_FAIL, pwe_cb->pwe_id, ckpt_type);
         return rc;
      }
      break;

   case PSS_CKPT_DATA_TBL_UNBIND:
      msg.ckpt_data.tbl_unbind.fr_card = mab_msg->fr_card;
      msg.ckpt_data.tbl_unbind.tbl_id = mab_msg->data.data.oac_pss_tbl_unbind.tbl_id;
      break;

   case PSS_CKPT_DATA_OAA_DOWN:
      msg.ckpt_data.oaa_down.fr_card = mab_msg->fr_card;
      break;

   case PSS_CKPT_DATA_BAM_CONF_DONE:
      len = strlen(mab_msg->data.data.bam_conf_done.pcn_list.pcn);
      if((msg.ckpt_data.bam_conf_done.pcn_list.pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len+1)) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_PCN_STRING_ALLOC_FAIL,
            "pss_re_sync()");
         m_LOG_PSS_HDLN2_II(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_CKPT_EVT_POPULATE_FAIL, pwe_cb->pwe_id, ckpt_type);
         return NCSCC_RC_FAILURE;
      }
      memset(msg.ckpt_data.bam_conf_done.pcn_list.pcn, '\0', len + 1);
      strcpy(msg.ckpt_data.bam_conf_done.pcn_list.pcn, mab_msg->data.data.bam_conf_done.pcn_list.pcn);
      break;

   default:
      m_LOG_PSS_HDLN2_II(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_INVALID_CKPT_EVT_TYPE, pwe_cb->pwe_id, ckpt_type);
      return NCSCC_RC_FAILURE;
   }

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_CKPT;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   mbcsv_arg.info.send_ckpt.i_action = NCS_MBCSV_ACT_ADD;
   mbcsv_arg.info.send_ckpt.i_ckpt_hdl = pwe_cb->ckpt_hdl;
   mbcsv_arg.info.send_ckpt.i_reo_hdl  = (MBCSV_REO_HDL)(long)(&msg);
   /* set the type of data */
   mbcsv_arg.info.send_ckpt.i_reo_type = (uns32)ckpt_type;
   mbcsv_arg.info.send_ckpt.i_send_type = NCS_MBCSV_SND_USR_ASYNC;
   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HDLN2_III(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_CKPT_EVT_SND_FAIL, pwe_cb->pwe_id, ckpt_type, rc);
      pss_free_re_msg(&msg);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HDLN2_III(NCSFL_SEV_DEBUG, PSS_HDLN_RE_SYNC_CKPT_EVT_SND_SUCCESS, pwe_cb->pwe_id, ckpt_type, rc);
   pss_free_re_msg(&msg);

   return rc;
}

/****************************************************************************
 * Name          : pss_re_direct_sync 
 *
 * Description   : Another Utility function to send async-updates to Standby.
 *
 * Arguments     : pwe_cb     - Pointer to PSS_PWE_CB
 *                 mab_msg    - Pointer to the MAB_MSG
 *                 ckpt_type  - Async-update event type
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_re_direct_sync(PSS_PWE_CB *pwe_cb, NCSCONTEXT i_msg, PSS_CKPT_DATA_TYPE ckpt_type, NCS_MBCSV_ACT_TYPE act_type)
{
   PSS_CKPT_MSG      msg;
   NCS_MBCSV_ARG     mbcsv_arg;
   uns32             rc = NCSCC_RC_SUCCESS;

   if((pwe_cb == NULL) || (i_msg == NULL))
      return NCSCC_RC_FAILURE;

   memset(&msg, '\0', sizeof(msg));
   msg.ckpt_data_type = ckpt_type;

   m_LOG_PSS_HDLN2_II(NCSFL_SEV_DEBUG, PSS_HDLN_RE_SYNC_CKPT_EVT_TYPE, pwe_cb->pwe_id, ckpt_type);
   switch(ckpt_type)
   {
   case PSS_CKPT_DATA_PLBCK_SSN_INFO:
      {
         /* Input is the PSS_CURR_PLBCK_SSN_INFO itself. */
         PSS_CURR_PLBCK_SSN_INFO *ssn_info = (PSS_CURR_PLBCK_SSN_INFO *)i_msg;
         rc = pss_duplicate_plbck_ssn_info(ssn_info, &msg.ckpt_data.plbck_ssn_info);
         if(rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN2_II(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_CKPT_EVT_POPULATE_FAIL, pwe_cb->pwe_id, ckpt_type);
            return rc;
         }
      }
      break;

   case PSS_CKPT_DATA_PEND_WBREQ_TO_BAM:
      {
         uns16 len = 0;
         char *src_pcn = (char *)i_msg; /* Input is the PCN string itself */

         len = strlen(src_pcn);
         if((msg.ckpt_data.pend_wbreq_to_bam.pcn_list.pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len+1))
            == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_PCN_STRING_ALLOC_FAIL,
               "pss_re_direct_sync()");
            m_LOG_PSS_HDLN2_II(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_CKPT_EVT_POPULATE_FAIL, pwe_cb->pwe_id, ckpt_type);
            return NCSCC_RC_FAILURE;
         }
         memset(msg.ckpt_data.pend_wbreq_to_bam.pcn_list.pcn, '\0', len + 1);
         strcpy(msg.ckpt_data.pend_wbreq_to_bam.pcn_list.pcn, src_pcn);
      }
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVLIBCONF:
      msg.ckpt_data.reload_pssvlibconf.lib_conf_file = ((PSS_CKPT_MSG*)i_msg)->ckpt_data.reload_pssvlibconf.lib_conf_file; /* This is the file name */
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVSPCNLIST:
      msg.ckpt_data.reload_pssvspcnlist.spcn_list = ((PSS_CKPT_MSG*)i_msg)->ckpt_data.reload_pssvspcnlist.spcn_list; /* This is the list pointer */
      break;

   default:
      m_LOG_PSS_HDLN2_II(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_INVALID_CKPT_EVT_TYPE, pwe_cb->pwe_id, ckpt_type);
      return NCSCC_RC_FAILURE;
   }

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_CKPT;
   mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
   mbcsv_arg.info.send_ckpt.i_action = act_type;
   mbcsv_arg.info.send_ckpt.i_ckpt_hdl = pwe_cb->ckpt_hdl;
   mbcsv_arg.info.send_ckpt.i_reo_hdl  = (MBCSV_REO_HDL)(long)(&msg);
   /* set the type of data */
   mbcsv_arg.info.send_ckpt.i_reo_type = (uns32)ckpt_type;
   mbcsv_arg.info.send_ckpt.i_send_type = NCS_MBCSV_SND_USR_ASYNC;
   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      m_LOG_PSS_HDLN2_III(NCSFL_SEV_ERROR, PSS_HDLN_RE_SYNC_CKPT_EVT_SND_FAIL, pwe_cb->pwe_id, ckpt_type, rc);
      pss_free_re_msg(&msg);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HDLN2_III(NCSFL_SEV_DEBUG, PSS_HDLN_RE_SYNC_CKPT_EVT_SND_SUCCESS, pwe_cb->pwe_id, ckpt_type, rc);
   pss_free_re_msg(&msg);

   return rc;
}

/****************************************************************************
 * Name          : pss_duplicate_pcn_list 
 *
 * Description   : Utility function to duplicate MAB_PSS_CLIENT_LIST data 
 *                 structure.
 *
 * Arguments     : src  - Pointer to source MAB_PSS_CLIENT_LIST structure
 *                 dest - Pointer to destination MAB_PSS_CLIENT_LIST structure
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_duplicate_pcn_list(MAB_PSS_CLIENT_LIST *src, MAB_PSS_CLIENT_LIST *dest)
{
   uns16 len = 0;
   MAB_PSS_TBL_LIST *p_src_tbl = NULL, *p_dst_tbl = NULL, **pp_dst_tbl = NULL;

   if((src == NULL) || (dest == NULL))
      return NCSCC_RC_FAILURE;

   if(src->pcn == NULL)
   {
      /* Nothing to be duplicated. Return normally. */
      return NCSCC_RC_SUCCESS;
   }

   len = strlen(src->pcn);
   if((dest->pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len+1)) == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_PCN_STRING_ALLOC_FAIL,
         "pss_duplicate_pcn_list()");
      return NCSCC_RC_FAILURE;
   }
   memset(dest->pcn, '\0', len + 1);
   strcpy(dest->pcn, src->pcn);

   for(p_src_tbl = src->tbl_list, pp_dst_tbl = &dest->tbl_list;
       p_src_tbl != NULL;
       p_src_tbl = p_src_tbl->next, pp_dst_tbl = &(*pp_dst_tbl)->next)
   {
      if((p_dst_tbl = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
            "pss_duplicate_pcn_list()");
         pss_free_client_list(dest);
         return NCSCC_RC_FAILURE;
      }
      memset(p_dst_tbl, '\0', sizeof(MAB_PSS_TBL_LIST));
      p_dst_tbl->tbl_id = p_src_tbl->tbl_id;
      *pp_dst_tbl = p_dst_tbl;
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_free_client_list 
 *
 * Description   : Utility function to free MAB_PSS_CLIENT_LIST data 
 *                 structure.
 *
 * Arguments     : list  - Pointer to MAB_PSS_CLIENT_LIST
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
void pss_free_client_list(MAB_PSS_CLIENT_LIST *list)
{
   MAB_PSS_TBL_LIST *tmp = NULL, *nxt_tmp = NULL;

   if(!list)
      return;

   if(list->pcn)
      m_MMGR_FREE_MAB_PCN_STRING(list->pcn);

   tmp = list->tbl_list;
   while(tmp != NULL)
   {
      nxt_tmp = tmp->next;
      m_MMGR_FREE_MAB_PSS_TBL_LIST(tmp);
      tmp = nxt_tmp;
   }
   memset(list, '\0', sizeof(MAB_PSS_CLIENT_LIST));

   return;
}

/****************************************************************************
 * Name          : pss_duplicate_plbck_ssn_info 
 *
 * Description   : Utility function to duplicate PSS_CURR_PLBCK_SSN_INFO data 
 *                 structure.
 *
 * Arguments     : src  - Pointer to source PSS_CURR_PLBCK_SSN_INFO structure
 *                 dest - Pointer to destination PSS_CURR_PLBCK_SSN_INFO structure
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_duplicate_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *src, PSS_CURR_PLBCK_SSN_INFO *dst)
{
   uns32 rc = NCSCC_RC_SUCCESS, len = 0;

   if(!src || !dst)
      return NCSCC_RC_FAILURE;

   memset(dst, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));
   if(memcmp(dst, src, sizeof(PSS_CURR_PLBCK_SSN_INFO)) == 0)
      return NCSCC_RC_SUCCESS;

   /**dst = *src;*/
   dst->plbck_ssn_in_progress = src->plbck_ssn_in_progress;
   dst->is_warmboot_ssn = src->is_warmboot_ssn;
   dst->tbl_id = src->tbl_id;
   
   if(src->pcn != NULL)
   {
      len = strlen(src->pcn);
      if((dst->pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len + 1)) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_PCN_STRING_ALLOC_FAIL,
            "pss_duplicate_plbck_ssn_info()");
         memset(dst, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));
         return NCSCC_RC_FAILURE;
      }
      memset(dst->pcn, '\0', len + 1);
      strcpy(dst->pcn, src->pcn);
   }
   else
   {
      /* This is the first async update during the playback session. */
      if(src->is_warmboot_ssn)
      {
         if(pss_dup_re_wbreq_info(&src->info.info2.wb_req, &dst->info.info2.wb_req) != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MAB_PSS_WARMBOOT_REQ_ALLOC_FAIL,
               "pss_duplicate_plbck_ssn_info()");
            return NCSCC_RC_FAILURE;
         }
         dst->info.info2.mds_dest = src->info.info2.mds_dest;
         dst->info.info2.seq_num = src->info.info2.seq_num;
      }
      else
      {
         strcpy((char*)&dst->info.alt_profile, 
            (char*)&src->info.alt_profile);
      }
   }

   if(src->mib_idx.i_inst_len != 0)
   {
      dst->mib_idx.i_inst_len = src->mib_idx.i_inst_len;
      dst->mib_idx.i_inst_ids = ncsmib_inst_memcopy(src->mib_idx.i_inst_len, src->mib_idx.i_inst_ids);
      if(dst->mib_idx.i_inst_ids == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MIB_INST_IDS_ALLOC_FAIL,
            "pss_duplicate_plbck_ssn_info()");
         if(dst->pcn != NULL)
            m_MMGR_FREE_MAB_PCN_STRING(dst->pcn);
         else
         {
            pss_free_re_wbreq_info(&dst->info.info2.wb_req);
         }
         memset(dst, '\0', sizeof(PSS_CURR_PLBCK_SSN_INFO));
         return NCSCC_RC_FAILURE;
      }
      dst->mib_idx.i_inst_len = src->mib_idx.i_inst_len;
   }
   dst->mib_obj_id = src->mib_obj_id;

   return rc;
}

/****************************************************************************
 * Name          : pss_gen_wbreq_from_cb
 *
 * Description   : Utility function to duplicate PSS_CURR_PLBCK_SSN_INFO data 
 *                 structure.
 *
 * Arguments     : src  - Pointer to source PSS_CURR_PLBCK_SSN_INFO structure
 *                 dest - Pointer to destination PSS_CURR_PLBCK_SSN_INFO structure
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_gen_wbreq_from_cb(PSS_CURR_PLBCK_SSN_INFO *src, MAB_PSS_WARMBOOT_REQ *wbreq)
{
   uns32 rc = NCSCC_RC_SUCCESS, len = 0;
   MAB_PSS_WARMBOOT_REQ *o_req = NULL, *in_req = NULL, **o_pp_req = NULL;
   MAB_PSS_TBL_LIST *p_tbl = NULL, **pp_tbl = NULL;

   if(!src || !wbreq)
      return NCSCC_RC_FAILURE;

   memset(wbreq, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
   if(memcmp(wbreq, &src->info.info2.wb_req, sizeof(MAB_PSS_WARMBOOT_REQ)) == 0)
      return NCSCC_RC_SUCCESS;

   for(o_pp_req = &wbreq, in_req = &src->info.info2.wb_req; in_req != NULL; 
       in_req = in_req->next, o_pp_req = &(*o_pp_req)->next)
   {
      if(*o_pp_req == NULL)
      {
          if((*o_pp_req = m_MMGR_ALLOC_MAB_PSS_WARMBOOT_REQ) == NULL)
          {
             m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MAB_PSS_WARMBOOT_REQ_ALLOC_FAIL,
                "pss_gen_wbreq_from_cb()");
              return NCSCC_RC_FAILURE;
          }
          memset(*o_pp_req, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
      }
      o_req = (*o_pp_req);
      *o_req = *in_req;
      o_req->next = NULL;   /* safety measure */
      if(in_req->pcn_list.pcn != NULL)
      {
         len = strlen(in_req->pcn_list.pcn);
         o_req->pcn_list.pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len + 1);
         if(o_req->pcn_list.pcn == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_PCN_STRING_ALLOC_FAIL,
                "pss_gen_wbreq_from_cb()");
            return NCSCC_RC_FAILURE;
         }

         memset(o_req->pcn_list.pcn, '\0', len + 1);
         strcpy(o_req->pcn_list.pcn, in_req->pcn_list.pcn);

         o_req->pcn_list.tbl_list = NULL;
         pp_tbl = &o_req->pcn_list.tbl_list;
         for(p_tbl = in_req->pcn_list.tbl_list; p_tbl != NULL;
             p_tbl = p_tbl->next, pp_tbl = &(*pp_tbl)->next)
         {
            if((*pp_tbl = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL)
            {
               m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
                  "pss_gen_wbreq_from_cb()");
               m_MMGR_FREE_MAB_PCN_STRING(o_req->pcn_list.pcn);
               pss_free_tbl_list(o_req->pcn_list.tbl_list);
               memset(o_req, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
               /* Free all the earlier ones. TBD */
               return NCSCC_RC_FAILURE;
            }
            memset((*pp_tbl), '\0', sizeof(MAB_PSS_TBL_LIST));
            (*pp_tbl)->tbl_id = p_tbl->tbl_id;
         }
      }
   }

   return rc;
}


/****************************************************************************
 * Name          : pss_free_plbck_ssn_info 
 *
 * Description   : Utility function to free PSS_CKPT_MSG contents.
 *
 * Arguments     : re_msg  - Pointer to PSS_CKPT_MSG structure
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
void pss_free_plbck_ssn_info(PSS_CURR_PLBCK_SSN_INFO *ssn_info)
{
   if(!ssn_info)
      return;

   if(ssn_info->is_warmboot_ssn == TRUE)
   {
      pss_free_re_wbreq_info(&ssn_info->info.info2.wb_req);
   }
   if(ssn_info->pcn != NULL)
   {
      m_MMGR_FREE_MAB_PCN_STRING(ssn_info->pcn);
      ssn_info->pcn = NULL;
   }

   if(ssn_info->mib_idx.i_inst_len != 0)
   {
      m_MMGR_FREE_MIB_INST_IDS(ssn_info->mib_idx.i_inst_ids);
      ssn_info->mib_idx.i_inst_ids = NULL;
      ssn_info->mib_idx.i_inst_len = 0;
   }

   return;
}

/****************************************************************************
 * Name          : pss_free_re_msg 
 *
 * Description   : Utility function to free PSS_CKPT_MSG contents.
 *
 * Arguments     : re_msg  - Pointer to PSS_CKPT_MSG structure
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
void pss_free_re_msg(PSS_CKPT_MSG *re_msg)
{
   switch(re_msg->ckpt_data_type)
   {
   case PSS_CKPT_DATA_TBL_BIND:
      pss_free_client_list(&re_msg->ckpt_data.tbl_bind.pcn_list);
      re_msg->ckpt_data.tbl_bind.pcn_list.pcn = NULL;
      re_msg->ckpt_data.tbl_bind.pcn_list.tbl_list = NULL;
      break;

   case PSS_CKPT_DATA_TBL_UNBIND:
      break;

   case PSS_CKPT_DATA_OAA_DOWN:
      break;

   case PSS_CKPT_DATA_PLBCK_SSN_INFO:
      pss_free_plbck_ssn_info(&re_msg->ckpt_data.plbck_ssn_info);
      break;

   case PSS_CKPT_DATA_PEND_WBREQ_TO_BAM:
      m_MMGR_FREE_MAB_PCN_STRING(re_msg->ckpt_data.pend_wbreq_to_bam.pcn_list.pcn);
      re_msg->ckpt_data.pend_wbreq_to_bam.pcn_list.pcn = NULL;
      break;

   case PSS_CKPT_DATA_BAM_CONF_DONE:
      if(re_msg->ckpt_data.bam_conf_done.pcn_list.pcn != NULL)
      {
         m_MMGR_FREE_MAB_PCN_STRING(re_msg->ckpt_data.bam_conf_done.pcn_list.pcn);
         re_msg->ckpt_data.bam_conf_done.pcn_list.pcn = NULL;
      }
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVLIBCONF:
      break;

   case PSS_CKPT_DATA_RELOAD_PSSVSPCNLIST:
      break;

   default:
      break;
   }

   return;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_tbl_bind
 *
 * Description   : Utility function to encode table-bind event to Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_tbl_bind(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_BIND_MSG *bind, uns16 peer_mbcsv_version)
{
   EDU_ERR     ederror = EDU_NORMAL;
   uns32       rc = NCSCC_RC_SUCCESS;

   rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
            EDP_OP_TYPE_ENC, &bind->fr_card, &ederror, peer_mbcsv_version);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_ENC_FAIL, pwe_cb->pwe_id);
      return rc;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_ENC_SUCCESS, pwe_cb->pwe_id);

   rc = pss_encode_pcn_list(uba, &bind->pcn_list);
   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_tbl_bind
 *
 * Description   : Utility function to decode table-bind event at Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_tbl_bind(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_BIND_MSG *bind, uns16 peer_mbcsv_version)
{
   EDU_ERR     ederror = EDU_NORMAL;
   MDS_DEST    **pp_mdest = NULL, *p_mdest = NULL;
   uns32       rc = NCSCC_RC_SUCCESS;

   p_mdest = &bind->fr_card;
   pp_mdest = &p_mdest;
   rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
            EDP_OP_TYPE_DEC, pp_mdest, &ederror, peer_mbcsv_version);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_DEC_FAIL, pwe_cb->pwe_id);
      return rc;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_DEC_SUCCESS, pwe_cb->pwe_id);

   rc = pss_decode_pcn_list(uba, &bind->pcn_list);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_tbl_unbind
 *
 * Description   : Utility function to encode table-unbind event to Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_tbl_unbind(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_UNBIND_MSG *unbind, uns16 peer_mbcsv_version)
{
   EDU_ERR     ederror = EDU_NORMAL;
   uns32       rc = NCSCC_RC_SUCCESS;
   uns8        *p8 = NULL;

   rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
            EDP_OP_TYPE_ENC, &unbind->fr_card, &ederror, peer_mbcsv_version);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_ENC_FAIL, pwe_cb->pwe_id);
      return rc;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_ENC_SUCCESS, pwe_cb->pwe_id);

   p8 = ncs_enc_reserve_space(uba, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_ckpt_enc_tbl_unbind()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_TBL_ID_ENC_FAIL, pwe_cb->pwe_id);
      return NCSCC_RC_FAILURE;
   }
   ncs_encode_32bit(&p8, unbind->tbl_id);
   ncs_enc_claim_space(uba, 4);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_ENC_SUCCESS, pwe_cb->pwe_id, unbind->tbl_id);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_tbl_unbind
 *
 * Description   : Utility function to decode table-unbind event at Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_tbl_unbind(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_TBL_UNBIND_MSG *unbind, uns16 peer_mbcsv_version)
{
   EDU_ERR     ederror = EDU_NORMAL;
   MDS_DEST    **pp_mdest = NULL, *p_mdest = NULL;
   uns32       rc = NCSCC_RC_SUCCESS;
   uns8        *p8 = NULL;

   p_mdest = &unbind->fr_card;
   pp_mdest = &p_mdest;
   rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
            EDP_OP_TYPE_DEC, pp_mdest, &ederror, peer_mbcsv_version);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_DEC_FAIL, pwe_cb->pwe_id);
      return rc;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_DEC_SUCCESS, pwe_cb->pwe_id);

   p8 = ncs_dec_flatten_space(uba, (uns8*)&unbind->tbl_id, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_tbl_unbind()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_TBL_ID_DEC_FAIL, pwe_cb->pwe_id);
      return NCSCC_RC_FAILURE;
   }
   unbind->tbl_id = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_DEC_SUCCESS, pwe_cb->pwe_id, unbind->tbl_id);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_oaa_down
 *
 * Description   : Utility function to encode OAA-DOWN event to Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_oaa_down(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_OAA_DOWN_MSG *oaa_down, uns16 peer_mbcsv_version)
{
   EDU_ERR     ederror = EDU_NORMAL;
   uns32       rc = NCSCC_RC_SUCCESS;

   rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
            EDP_OP_TYPE_ENC, &oaa_down->fr_card, &ederror, peer_mbcsv_version);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_ENC_FAIL, pwe_cb->pwe_id);
      return rc;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_ENC_SUCCESS, pwe_cb->pwe_id);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_oaa_down
 *
 * Description   : Utility function to decode OAA-DOWN event at Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_oaa_down(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_OAA_DOWN_MSG *oaa_down, uns16 peer_mbcsv_version)
{
   EDU_ERR     ederror = EDU_NORMAL;
   MDS_DEST    **pp_mdest = NULL, *p_mdest = NULL;
   uns32       rc = NCSCC_RC_SUCCESS;

   p_mdest = &oaa_down->fr_card;
   pp_mdest = &p_mdest;
   rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
            EDP_OP_TYPE_DEC, pp_mdest, &ederror, peer_mbcsv_version);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_DEC_FAIL, pwe_cb->pwe_id);
      return rc;
   }
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_DEC_SUCCESS, pwe_cb->pwe_id);

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_plbck_ssn_info
 *
 * Description   : Utility function to encode Playback-session info to Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_plbck_ssn_info(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_PLBCK_SSN_INFO_MSG *plbck_ssn, uns16 peer_mbcsv_version)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns8  bool_val = 0x00, *p8 = NULL, *p_wbreq_cnt = NULL;
   uns16 wb_req_cnt = 0;
   MAB_PSS_WARMBOOT_REQ *wbreq = NULL;
   PSS_RE_PLBCK_BITMASK  bmask = 0x00;
   EDU_ERR           ederror = EDU_NORMAL;
   PSS_CKPT_PLBCK_SSN_INFO_MSG lcl_ssn;

   memset(&lcl_ssn, '\0', sizeof(PSS_CKPT_PLBCK_SSN_INFO_MSG));
   if(memcmp(plbck_ssn, &lcl_ssn, sizeof(PSS_CKPT_PLBCK_SSN_INFO_MSG)) == 0)
   {
      m_PSS_RE_PLBCK_SET_SSN_IS_DONE(bmask);
   }
   else
   {
      if(plbck_ssn->plbck_ssn_in_progress)
         m_PSS_RE_PLBCK_SET_SSN_IN_PROGRESS(bmask);

      if(plbck_ssn->is_warmboot_ssn)
         m_PSS_RE_PLBCK_SET_SSN_WARMBOOT(bmask);

      if(plbck_ssn->pcn)
         m_PSS_RE_PLBCK_SET_SSN_PCN_VALID(bmask);
   }

   p8 = ncs_enc_reserve_space(uba, 1);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_ckpt_enc_plbck_ssn_info()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_BMASK_FAIL, pwe_cb->pwe_id);
      return NCSCC_RC_FAILURE;
   }
   ncs_encode_8bit(&p8, bmask);
   ncs_enc_claim_space(uba, 1);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ENC_BMASK_DONE, pwe_cb->pwe_id, (int)bmask);
   if(m_PSS_RE_PLBCK_IS_SSN_DONE(bmask))
   {
      /* Buffer is done. */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_PLBCK_SSNINFO_SESSION_DONE, pwe_cb->pwe_id);
      return rc;
   }

   p8 = ncs_enc_reserve_space(uba, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_ckpt_enc_plbck_ssn_info()");
      return NCSCC_RC_FAILURE;
   }
   ncs_encode_32bit(&p8, plbck_ssn->tbl_id);
   ncs_enc_claim_space(uba, 4);

   p8 = ncs_enc_reserve_space(uba, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_ckpt_enc_plbck_ssn_info()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MIB_IDX_LEN_ENC_FAIL, pwe_cb->pwe_id);
      return NCSCC_RC_FAILURE;
   }
   ncs_encode_32bit(&p8, plbck_ssn->mib_idx.i_inst_len);
   ncs_enc_claim_space(uba, 4);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MIB_IDX_LEN_ENC_SUCCESS, pwe_cb->pwe_id, plbck_ssn->mib_idx.i_inst_len);

   if(plbck_ssn->mib_idx.i_inst_len != 0)
   {
      if(ncsmib_inst_encode(plbck_ssn->mib_idx.i_inst_ids, 
            plbck_ssn->mib_idx.i_inst_len, uba) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MIB_IDX_ENC_FAIL, pwe_cb->pwe_id);
         return NCSCC_RC_FAILURE;
      }
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MIB_IDX_ENC_SUCCESS, pwe_cb->pwe_id);
   }

   p8 = ncs_enc_reserve_space(uba, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
         "pss_ckpt_enc_plbck_ssn_info()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_MIB_OBJ_ID_FAIL, pwe_cb->pwe_id);
      return NCSCC_RC_FAILURE;
   }
   ncs_encode_32bit(&p8, plbck_ssn->mib_obj_id);
   ncs_enc_claim_space(uba, 4);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ENC_MIB_OBJ_ID_DONE, pwe_cb->pwe_id, plbck_ssn->mib_obj_id);

   if(plbck_ssn->pcn != NULL)
   {
      if(pss_encode_pcn(uba, plbck_ssn->pcn) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, plbck_ssn->pcn);
         return NCSCC_RC_FAILURE;
      }
   }

   if(plbck_ssn->is_warmboot_ssn)
   {
      p8 = ncs_enc_reserve_space(uba, 4);
      if(p8 == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
            "pss_ckpt_enc_plbck_ssn_info()");
         m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PARTIAL_DELETE_DONE_FAIL, pwe_cb->pwe_id);
         return NCSCC_RC_FAILURE;
      }
      ncs_encode_32bit(&p8, plbck_ssn->info.info2.partial_delete_done);
      ncs_enc_claim_space(uba, 4);
      m_LOG_PSS_HDLN2_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ENC_PARTIAL_DELETE_DONE_SUCCESS, pwe_cb->pwe_id,
              plbck_ssn->info.info2.partial_delete_done);

      /* This should be sent only once. */
      if(plbck_ssn->pcn == NULL)
      {
         /* This indicates session is starting now only. So, send the plbck_ssn->info.info2.wb_req
            structure now. */
         /* Fix a count for each MAB_PSS_WARMBOOT_REQ entry, because there can be a list of
            wb-requests here. */
         p_wbreq_cnt = ncs_enc_reserve_space(uba, 2);
         if(p_wbreq_cnt == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
               "pss_ckpt_enc_plbck_ssn_info()");
            return NCSCC_RC_FAILURE;
         }
         ncs_enc_claim_space(uba, 2);

         /* Encode seq_num */
         p8 = ncs_enc_reserve_space(uba, 4);
         if(p8 == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
               "pss_ckpt_enc_plbck_ssn_info()");
            m_LOG_PSS_HDLN_II(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_MIB_OBJ_ID_FAIL, 
               pwe_cb->pwe_id, plbck_ssn->info.info2.seq_num);
            return NCSCC_RC_FAILURE;
         }
         ncs_encode_32bit(&p8, plbck_ssn->info.info2.seq_num);
         ncs_enc_claim_space(uba, 4);
         m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ENC_SEQ_NUM_DONE, 
             pwe_cb->pwe_id, plbck_ssn->info.info2.seq_num);

         /* Encode MDS_DEST */
         rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
                  EDP_OP_TYPE_ENC, &plbck_ssn->info.info2.mds_dest, &ederror, peer_mbcsv_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_ENC_FAIL, pwe_cb->pwe_id);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_ENC_SUCCESS, pwe_cb->pwe_id);

         for(wbreq = &plbck_ssn->info.info2.wb_req; wbreq != NULL; 
             wbreq = wbreq->next, ++ wb_req_cnt)
         {
            p8 = ncs_enc_reserve_space(uba, 1);
            if(p8 == NULL)
            {
               m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
                  "pss_ckpt_enc_plbck_ssn_info()");
               return NCSCC_RC_FAILURE;
            }
            bool_val = ((wbreq->is_system_client) ? 0x01 : 0x00);
            ncs_encode_8bit(&p8, bool_val);
            ncs_enc_claim_space(uba, 1);
            m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ENC_IS_SYS_CLIENT_BOOL_DONE, 
               pwe_cb->pwe_id, wbreq->is_system_client);

            rc = pss_encode_pcn_list(uba, &wbreq->pcn_list);
         }
         ncs_encode_16bit(&p_wbreq_cnt, wb_req_cnt);
      }
   }  /* if(plbck_ssn->is_warmboot_ssn) */
   else
   {
      /* This should be sent only once. */
      if(plbck_ssn->pcn == NULL)
      {
         uns16 str_len = strlen((char*)plbck_ssn->info.alt_profile);

         p8 = ncs_enc_reserve_space(uba, 2);
         if(p8 == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
               "pss_ckpt_enc_plbck_ssn_info()");
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         ncs_encode_16bit(&p8, str_len);
         ncs_enc_claim_space(uba, 2);

         if(ncs_encode_n_octets_in_uba(uba, (uns8*)&plbck_ssn->info.alt_profile, 
               str_len) != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_ALT_PROFILE_FAIL, 
               (char*)&plbck_ssn->info.alt_profile);
            return NCSCC_RC_FAILURE;
         }
         m_LOG_PSS_HDLN_STR(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_ENC_ALT_PROFILE_DONE, 
            (char*)&plbck_ssn->info.alt_profile);
      }
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_plbck_ssn_info
 *
 * Description   : Utility function to decode Playback-session info at Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_plbck_ssn_info(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba, PSS_CKPT_PLBCK_SSN_INFO_MSG *plbck_ssn, uns16 peer_mbcsv_version)
{
   uns32 rc = NCSCC_RC_SUCCESS, i =0;
   uns8  bool_val = 0x00, *p8 = NULL;
   uns16 wb_req_cnt = 0;
   PSS_RE_PLBCK_BITMASK  bmask = 0x00;
   MAB_PSS_WARMBOOT_REQ *wbreq = NULL, **pp_wbreq = NULL, *tmp = NULL;
   EDU_ERR           ederror = EDU_NORMAL;

   p8 = ncs_dec_flatten_space(uba, (uns8*)&bmask, 1);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_plbck_ssn_info()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_BMASK_FAIL, pwe_cb->pwe_id);
      pss_flush_plbck_ssn_info(plbck_ssn);
      return NCSCC_RC_FAILURE;
   }
   bmask = ncs_decode_8bit(&p8);
   ncs_dec_skip_space(uba, 1);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_BMASK_DONE, pwe_cb->pwe_id, (int)bmask);

   if(m_PSS_RE_PLBCK_IS_SSN_DONE(bmask))
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_PLBCK_SSNINFO_SESSION_DONE, pwe_cb->pwe_id);

      /* Clear the info */
      pss_flush_plbck_ssn_info(plbck_ssn);

      /* Buffer is done. */
      return rc;
   }

   plbck_ssn->plbck_ssn_in_progress = m_PSS_RE_PLBCK_IS_SSN_IN_PROGRESS(bmask);
   plbck_ssn->is_warmboot_ssn = m_PSS_RE_PLBCK_IS_WARMBOOT_SSN(bmask);

   p8 = ncs_dec_flatten_space(uba, (uns8*)&plbck_ssn->tbl_id, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_plbck_ssn_info()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_TBL_ID_DEC_FAIL, pwe_cb->pwe_id);
      pss_flush_plbck_ssn_info(plbck_ssn);
      return NCSCC_RC_FAILURE;
   }
   plbck_ssn->tbl_id = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_DEC_SUCCESS, pwe_cb->pwe_id, plbck_ssn->tbl_id);

   p8 = ncs_dec_flatten_space(uba, (uns8*)&plbck_ssn->mib_idx.i_inst_len, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_plbck_ssn_info()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MIB_IDX_LEN_DEC_FAIL, pwe_cb->pwe_id);
      pss_flush_plbck_ssn_info(plbck_ssn);
      return NCSCC_RC_FAILURE;
   }
   plbck_ssn->mib_idx.i_inst_len = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MIB_IDX_LEN_DEC_SUCCESS, pwe_cb->pwe_id, plbck_ssn->mib_idx.i_inst_len);

   if(plbck_ssn->mib_idx.i_inst_len != 0)
   {
      if(ncsmib_inst_decode((uns32**)&plbck_ssn->mib_idx.i_inst_ids,
            plbck_ssn->mib_idx.i_inst_len, uba) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MIB_IDX_DEC_FAIL, pwe_cb->pwe_id);
         pss_flush_plbck_ssn_info(plbck_ssn);
         return NCSCC_RC_FAILURE;
      }
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MIB_IDX_DEC_SUCCESS, pwe_cb->pwe_id);
   }

   p8 = ncs_dec_flatten_space(uba, (uns8*)&plbck_ssn->mib_obj_id, 4);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_plbck_ssn_info()");
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_MIB_OBJ_ID_FAIL, pwe_cb->pwe_id);
      pss_flush_plbck_ssn_info(plbck_ssn);
      return NCSCC_RC_FAILURE;
   }
   plbck_ssn->mib_obj_id = ncs_decode_32bit(&p8);
   ncs_dec_skip_space(uba, 4);
   m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_MIB_OBJ_ID_DONE, pwe_cb->pwe_id, plbck_ssn->mib_obj_id);

   if(m_PSS_RE_PLBCK_IS_PCN_VALID(bmask))
   {
      if(pss_decode_pcn(uba, &plbck_ssn->pcn) != NCSCC_RC_SUCCESS)
      {
         pss_flush_plbck_ssn_info(plbck_ssn);
         return NCSCC_RC_FAILURE;
      }
   }

   if(plbck_ssn->is_warmboot_ssn)
   {
      p8 = ncs_dec_flatten_space(uba, (uns8*)&plbck_ssn->info.info2.partial_delete_done, 4);
      if(p8 == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
            "pss_ckpt_dec_plbck_ssn_info()");
         m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_PARTIAL_DELETE_DONE_FAIL, pwe_cb->pwe_id);
         pss_flush_plbck_ssn_info(plbck_ssn);
         return NCSCC_RC_FAILURE;
      }
      plbck_ssn->info.info2.partial_delete_done = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);
      m_LOG_PSS_HDLN2_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_PARTIAL_DELETE_DONE_SUCCESS, pwe_cb->pwe_id, 
           plbck_ssn->info.info2.partial_delete_done);

      /* This will be received only once. */
      if(plbck_ssn->pcn == NULL)
      {
         MDS_DEST **pp_mdest = NULL, *p_mdest = NULL;
         /* This indicates session is starting now only. So, send the plbck_ssn->info.info2.wb_req
            structure now. */
         /* Fix a count for each MAB_PSS_WARMBOOT_REQ entry, because there can be a list of
            wb-requests here. */
         p8 = ncs_dec_flatten_space(uba, (uns8*)&wb_req_cnt, 2);
         if(p8 == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
               "pss_ckpt_dec_plbck_ssn_info()");
            pss_flush_plbck_ssn_info(plbck_ssn);
            return NCSCC_RC_FAILURE;
         }
         wb_req_cnt = ncs_decode_16bit(&p8);
         ncs_dec_skip_space(uba, 2);

         /* Decode seq_num */
         p8 = ncs_dec_flatten_space(uba, (uns8*)&plbck_ssn->info.info2.seq_num, 4);
         if(p8 == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
               "pss_ckpt_dec_plbck_ssn_info()");
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_SEQ_NUM_FAIL, pwe_cb->pwe_id);
            pss_flush_plbck_ssn_info(plbck_ssn);
            return NCSCC_RC_FAILURE;
         }
         plbck_ssn->info.info2.seq_num = ncs_decode_32bit(&p8);
         ncs_dec_skip_space(uba, 4);
         m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_SEQ_NUM_DONE, pwe_cb->pwe_id, plbck_ssn->info.info2.seq_num);

         /* Decode MDS_DEST */
         p_mdest = &plbck_ssn->info.info2.mds_dest;
         pp_mdest = &p_mdest;
         rc = m_NCS_EDU_VER_EXEC(&pwe_cb->p_pss_cb->edu_hdl, ncs_edp_mds_dest, uba, 
                    EDP_OP_TYPE_DEC, pp_mdest, &ederror, peer_mbcsv_version);
         if(rc != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_MDEST_DEC_FAIL, pwe_cb->pwe_id);
            pss_flush_plbck_ssn_info(plbck_ssn);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_MDEST_DEC_SUCCESS, pwe_cb->pwe_id);

         wbreq = &plbck_ssn->info.info2.wb_req;
         pp_wbreq = &wbreq;
         for(i = 0; i < wb_req_cnt; i++, pp_wbreq = &(*pp_wbreq)->next)
         {
            if(i != 0)
            {
               if((tmp = m_MMGR_ALLOC_MAB_PSS_WARMBOOT_REQ) == NULL)
               {
                  m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MAB_PSS_WARMBOOT_REQ_ALLOC_FAIL,
                     "pss_ckpt_dec_plbck_ssn_info()");
                  return NCSCC_RC_FAILURE;
               }
               memset(tmp, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
               *pp_wbreq = tmp;
            }
            else
            {
               tmp = wbreq;   /* First iteration */
            }

            p8 = ncs_dec_flatten_space(uba, (uns8*)&bool_val, 1);
            if(p8 == NULL)
            {
               m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
                  "pss_ckpt_dec_plbck_ssn_info()");
               pss_flush_plbck_ssn_info(plbck_ssn);
               return NCSCC_RC_FAILURE;
            }
            bool_val = ncs_decode_8bit(&p8);
            ncs_dec_skip_space(uba, 1);
            tmp->is_system_client = (bool_val ? 0x01 : 0x00);
            m_LOG_PSS_HDLN_II(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_IS_SYS_CLIENT_BOOL_DONE, 
               pwe_cb->pwe_id, tmp->is_system_client);

            rc = pss_decode_pcn_list(uba, &tmp->pcn_list);
         }
      }  /* if(plbck_ssn->pcn == NULL) */
   }  /* if(plbck_ssn->is_warmboot_ssn) */
   else
   {
      /* This will be received only once. */
      if(plbck_ssn->pcn == NULL)
      {
         uns16 str_len = 0;

         p8 = ncs_dec_flatten_space(uba, (uns8*)&str_len, 2);
         if(p8 == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
               "pss_ckpt_dec_plbck_ssn_info()");
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         }
         str_len = ncs_decode_16bit(&p8);
         ncs_dec_skip_space(uba, 2);

         memset(&plbck_ssn->info.alt_profile, '\0', NCS_PSS_MAX_PROFILE_NAME);
         if(ncs_decode_n_octets_from_uba(uba, (uns8*)&plbck_ssn->info.alt_profile,
               str_len) != NCSCC_RC_SUCCESS)
         {
            m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_OCTETS_FAIL);
            m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_ALT_PROFILE_FAIL);
            pss_flush_plbck_ssn_info(plbck_ssn);
            return NCSCC_RC_FAILURE;
         }
         m_LOG_PSS_HDLN_STR(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_ALT_PROFILE_DONE, 
            (char*)&plbck_ssn->info.alt_profile);
      }
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_bam_conf_done
 *
 * Description   : Utility function to encode BAM's CONF_DONE event to Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_bam_conf_done(NCS_UBAID *uba, PSS_CKPT_BAM_CONF_DONE_MSG *bam_conf_done)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if(pss_encode_pcn(uba, bam_conf_done->pcn_list.pcn) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, bam_conf_done->pcn_list.pcn);
      return NCSCC_RC_FAILURE;
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_bam_conf_done
 *
 * Description   : Utility function to decode BAM's CONF_DONE event at Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_bam_conf_done(NCS_UBAID *uba, PSS_CKPT_BAM_CONF_DONE_MSG *bam_conf_done)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if(pss_decode_pcn(uba, &bam_conf_done->pcn_list.pcn) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_reload_pssvlibconf
 *
 * Description   : Utility function to encode PSS's RELOAD_PSSVLIBCONF event to Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_reload_pssvlibconf(NCS_UBAID *uba, char *file_name)
{
   uns32 rc = NCSCC_RC_SUCCESS, cnt = 0;
   FILE *fh = NULL;
   char libname[256], appname[64];
   uns32 read = 0;
   uns8 *p_line_cnt = NULL;
    
   /* The "pssv_lib_conf" file is mandatory for PSS to start up */
   fh = sysf_fopen(file_name, "r");
   if(fh == NULL)
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_LIB_CONF_FILE_OPEN_FAIL);
      return NCSCC_RC_FAILURE;
   }
    
   memset(&libname, '\0', sizeof(libname));
   memset(&appname, '\0', sizeof(appname));
   while(((read = fscanf(fh, "%s %s", (char*)&libname, (char*)&appname)) == 2) &&
         (read != EOF))
   {
      if(p_line_cnt == NULL)
      {
         p_line_cnt = ncs_enc_reserve_space(uba, 2);
         if(p_line_cnt == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
              "pss_ckpt_enc_reload_pssvlibconf()");
            sysf_fclose(fh);
            return NCSCC_RC_FAILURE;
         }
         ncs_enc_claim_space(uba, 2);
      }
      ++cnt;
      if(pss_encode_str(uba, (char*)&libname) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_LIBNAME_FAIL, (char*)&libname);
         sysf_fclose(fh);
         return NCSCC_RC_FAILURE;
      }
      if(pss_encode_str(uba, (char*)&appname) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_APPNAME_FAIL, (char*)&appname);
         sysf_fclose(fh);
         return NCSCC_RC_FAILURE;
      }

      m_LOG_PSS_LIB_INFO(NCSFL_SEV_INFO, PSS_LIB_INFO_CONF_FILE_ENTRY_MBCSV_ENC_DONE, &libname, &appname);
      memset(&libname, '\0', sizeof(libname));
      memset(&appname, '\0', sizeof(appname));
   }   /* while loop */
   sysf_fclose(fh);

   if(p_line_cnt != NULL)
   {
      ncs_encode_16bit(&p_line_cnt, cnt);
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_n_updt_reload_pssvlibconf
 *
 * Description   : Utility function to decode PSS's RELOAD_PSSVLIBCONF event at Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_n_updt_reload_pssvlibconf(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba) 
{
   uns32 status = NCSCC_RC_SUCCESS, i = 0, cnt = 0;
   char libname[256], fullname[256], appname[64], funcname[256];
   char file_libname[16][256]={"\0"}, file_appname[16][64]={"\0"};
   int  loop=0, read = 0;
   NCS_BOOL entry_exists = FALSE;
   uns8 *p8 = NULL;
   uns32 (*app_routine)(NCSCONTEXT) = NULL; 
   NCS_OS_DLIB_HDL     *lib_hdl = NULL;
   int8  *dl_error = NULL;
   FILE  *fh = NULL;

   p8 = ncs_dec_flatten_space(uba, (uns8*)&cnt, 2);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_n_updt_reload_pssvlibconf()");
      return NCSCC_RC_FAILURE;
   }
   cnt = ncs_decode_16bit(&p8);
   ncs_dec_skip_space(uba, 2);

   pss_flush_all_mib_tbl_info(pwe_cb->p_pss_cb);  /* Removes all earlier load MIB description info */
   fh = sysf_fopen((char*)&pwe_cb->p_pss_cb->lib_conf_file, "r+"); /* Create new file again */
   if(fh == NULL)
   {
      m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_LIB_CONF_FILE_OPEN_IN_APPEND_MODE_FAIL);
      return NCSCC_RC_FAILURE;
   }

   while(((read = fscanf(fh, "%s %s", (char*)file_libname[loop], (char*)file_appname[loop])) == 2) &&
         (read != EOF) && loop++ < 16);

   if(read != EOF)
   {
      /* Log error occured while reading pssv_lib_conf, also log loop value */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_READ_LIB_CONF_FAIL, loop);
      sysf_fclose(fh);
      return NCSCC_RC_FAILURE;
   }

   /* Merging the contents of the resident pssv_lib_conf and received lib_conf information */
   for(i = 0; i < cnt; i++)
   {
      memset(&libname, '\0', sizeof(libname));
      memset(&appname, '\0', sizeof(appname));
      if(pss_decode_str(uba, (char*)&libname) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_LIBNAME_FAIL);
         sysf_fclose(fh);
         return NCSCC_RC_FAILURE;
      }
      if(pss_decode_str(uba, (char*)&appname) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_LIBNAME_FAIL);
         sysf_fclose(fh);
         return NCSCC_RC_FAILURE;
      }

      for(loop = 0, entry_exists = FALSE; loop < 16; loop++)
      {
         if((strcmp(libname, file_libname[loop]) == 0) && (strcmp(appname, file_appname[loop]) == 0))
            entry_exists = TRUE;
      }
      if(entry_exists == FALSE)
         sysf_fprintf(fh, "%s %s\n", libname, appname);
   }

   fseek(fh, 0L, SEEK_SET);

   memset(&libname, '\0', sizeof(libname));
   memset(&appname, '\0', sizeof(appname));
   while(((read = fscanf(fh, "%s %s", (char*)&libname, (char*)&appname)) == 2) &&
         (read != EOF))
   {
      /* Load the library, and invoke the lib-register-function */
      memset(&fullname, '\0', sizeof(fullname));
      sprintf((char*)&fullname, "%s", (char*)&libname);
      lib_hdl = m_NCS_OS_DLIB_LOAD(fullname, m_NCS_OS_DLIB_ATTR);
      if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL)
      {
         /* log the error returned from dlopen() */
         m_LOG_PSS_LIB_OP(NCSFL_SEV_ERROR, PSS_LIB_OP_CONF_FILE_OPEN_FAIL, dl_error);
         continue;
      }

      /* Get the registration symbol "__<appname>_pssv_reg" from the appname */
      memset(&funcname, '\0', sizeof(funcname));
      sprintf((char*)&funcname, "__%s_pssv_reg", (char*)&appname);
      app_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, funcname);
      if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL)
      {
         /* log the error returned from dlopen() */
         m_LOG_PSS_LIB_OP(NCSFL_SEV_ERROR, PSS_LIB_OP_SYM_LKUP_ERROR, dl_error);
         m_NCS_OS_DLIB_CLOSE(lib_hdl);
         continue;
      }
 
      /* Invoke the registration symbol */
      status = (*app_routine)((NCSCONTEXT)pwe_cb->p_pss_cb);  
      if (status != NCSCC_RC_SUCCESS)
      { 
         m_LOG_PSS_LIB_OP(NCSFL_SEV_ERROR, PSS_LIB_OP_SYM_RET_ERROR, dl_error);
         m_NCS_OS_DLIB_CLOSE(lib_hdl);
         continue;
      }
      m_LOG_PSS_LIB_INFO(NCSFL_SEV_INFO, PSS_LIB_INFO_CONF_FILE_ENTRY_LOADED, &libname, &funcname);
      m_NCS_OS_DLIB_CLOSE(lib_hdl);
      memset(&libname, '\0', sizeof(libname));
      memset(&appname, '\0', sizeof(appname));
   }
   sysf_fclose(fh);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_ckpt_enc_reload_pssvspcnlist
 *
 * Description   : Utility function to encode PSS's RELOAD_PSSVSPCNLIST event to Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_enc_reload_pssvspcnlist(NCS_UBAID *uba, PSS_SPCN_LIST *spcn_list)
{
   uns32 rc = NCSCC_RC_SUCCESS, cnt = 0;
   uns8 *p_line_cnt = NULL, *p8 = NULL;
   PSS_SPCN_LIST *tmp = spcn_list;

   for(tmp = spcn_list; tmp != NULL; tmp = tmp->next, cnt++)
   {
      if(p_line_cnt == NULL)
      {
         p_line_cnt = ncs_enc_reserve_space(uba, 2);
         if(p_line_cnt == NULL)
         {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
              "pss_ckpt_enc_reload_pssvspcnlist()");
            return NCSCC_RC_FAILURE;
         }
         ncs_enc_claim_space(uba, 2);
      }
      if(pss_encode_pcn(uba, tmp->pcn) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, tmp->pcn);
         return NCSCC_RC_FAILURE;
      }
      p8 = ncs_enc_reserve_space(uba, 4);
      if(p8 == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
              "pss_ckpt_enc_reload_pssvspcnlist()");
         return NCSCC_RC_FAILURE;
      }
      ncs_encode_32bit(&p8, tmp->plbck_frm_bam);
      ncs_enc_claim_space(uba, 4);

      m_LOG_PSS_INFO(NCSFL_SEV_INFO, PSS_INFO_SPCN_ENC_DONE, tmp->pcn, tmp->plbck_frm_bam);
   }   /* while loop */

   if(p_line_cnt != NULL)
   {
      ncs_encode_16bit(&p_line_cnt, cnt);
   }

   return rc;
}

/****************************************************************************
 * Name          : pss_ckpt_dec_n_updt_reload_pssvspcnlist
 *
 * Description   : Utility function to decode PSS's RELOAD_PSSVSPCNLIST event at Standby
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_ckpt_dec_n_updt_reload_pssvspcnlist(PSS_PWE_CB *pwe_cb, NCS_UBAID *uba)
{
   uns32 i = 0, cnt = 0;
   char *pcn = NULL;
   PSS_SPCN_LIST *list = NULL, *tmp = NULL;
   uns8 *p8 = NULL;
   NCS_BOOL boolean_val = FALSE;
   FILE  *fh;

   /* Do not unlink, the file could be a symbolic link. Truncate file instead. */
   fh = sysf_fopen((char*)&pwe_cb->p_pss_cb->spcn_list_file, "w");
   if(fh == NULL)
   {
      m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_LIB_CONF_FILE_OPEN_IN_APPEND_MODE_FAIL);
      return NCSCC_RC_FAILURE;
   }

   p8 = ncs_dec_flatten_space(uba, (uns8*)&cnt, 2);
   if(p8 == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_ckpt_dec_n_updt_reload_pssvspcnlist()");
      sysf_fclose(fh);
      return NCSCC_RC_FAILURE;
   }
   cnt = ncs_decode_16bit(&p8);
   ncs_dec_skip_space(uba, 2);

   pss_destroy_spcn_list(pwe_cb->p_pss_cb->spcn_list);
   pwe_cb->p_pss_cb->spcn_list = NULL;

   for(i = 0; i < cnt; i++)
   {
      pcn = NULL;
      if(pss_decode_pcn(uba, &pcn) != NCSCC_RC_SUCCESS)
      {
         m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_PCN_FAIL);
         sysf_fclose(fh);
         return NCSCC_RC_FAILURE;
      }
      p8 = ncs_dec_flatten_space(uba, (uns8*)&boolean_val, 4);
      if(p8 == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
            "pss_ckpt_dec_n_updt_reload_pssvspcnlist()");
         sysf_fclose(fh);
         return NCSCC_RC_FAILURE;
      }
      boolean_val = ncs_decode_32bit(&p8);
      ncs_dec_skip_space(uba, 4);

      if((tmp = m_MMGR_ALLOC_MAB_PSS_SPCN_LIST) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_SPCN_LIST,
             "pss_ckpt_dec_n_updt_reload_pssvspcnlist()");
         sysf_fclose(fh);
         return NCSCC_RC_FAILURE;
      }
      memset(tmp, '\0', sizeof(PSS_SPCN_LIST));
      tmp->pcn = pcn;
      tmp->plbck_frm_bam = boolean_val;

      if(tmp->plbck_frm_bam)
         sysf_fprintf(fh, "%s %s\n", tmp->pcn, m_PSS_SPCN_SOURCE_BAM);
      else
         sysf_fprintf(fh, "%s %s\n", tmp->pcn, m_PSS_SPCN_SOURCE_PSSV);

      tmp->next = list;
      list = tmp;

      m_LOG_PSS_INFO(NCSFL_SEV_INFO, PSS_INFO_SPCN_DEC_DONE, pcn, boolean_val);
   }
   pwe_cb->p_pss_cb->spcn_list = list;

   sysf_fclose(fh);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_encode_pcn_list
 *
 * Description   : Utility function to encode MAB_PSS_CLIENT_LIST 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_encode_pcn_list(NCS_UBAID *uba, MAB_PSS_CLIENT_LIST *pcn_list)
{
   MAB_PSS_TBL_LIST    *p_tbl = NULL;
   uns8     *data, *p_tbl_cnt;
   uns16    tbl_cnt = 0;

   if(pcn_list->pcn == NULL)
   {
      m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, pcn_list->pcn);
      return NCSCC_RC_FAILURE;
   }

   if(pss_encode_pcn(uba, pcn_list->pcn) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, pcn_list->pcn);
      return NCSCC_RC_FAILURE;
   }

   p_tbl_cnt = ncs_enc_reserve_space(uba, 2);
   if(p_tbl_cnt == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
            "pss_encode_pcn_list()");
      return NCSCC_RC_FAILURE;
   }
   ncs_enc_claim_space(uba, 2);
   
   p_tbl = pcn_list->tbl_list;
   while(p_tbl != NULL)
   {
      data = ncs_enc_reserve_space(uba, 4);
      if(data == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
            "pss_encode_pcn_list()");
         return NCSCC_RC_FAILURE;
      }
      ncs_encode_32bit(&data, p_tbl->tbl_id);
      ncs_enc_claim_space(uba, 4);
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_ENC_SUCCESS, p_tbl->tbl_id);
      
      p_tbl = p_tbl->next;
      tbl_cnt++;
   };
   ncs_encode_16bit(&p_tbl_cnt, tbl_cnt);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : pss_decode_pcn_list
 *
 * Description   : Utility function to decode MAB_PSS_CLIENT_LIST
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 pss_decode_pcn_list(NCS_UBAID *uba, MAB_PSS_CLIENT_LIST *pcn_list)
{
   MAB_PSS_TBL_LIST    *p_tbl = NULL, **pp_tbl = NULL;
   uns8     *data;
   uns16    tbl_cnt = 0, i;

   if(pss_decode_pcn(uba, &pcn_list->pcn) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;

   data = ncs_dec_flatten_space(uba, (uns8*)&tbl_cnt, 2);
   if(data == NULL)
   {
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
         "pss_decode_pcn_list()");
      return NCSCC_RC_FAILURE;
   }
   tbl_cnt = ncs_decode_16bit(&data);
   ncs_dec_skip_space(uba, 2);

   pp_tbl = &pcn_list->tbl_list;
   for(i = 0; i < tbl_cnt; i++, pp_tbl = &(*pp_tbl)->next)
   {
      if((p_tbl = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
            "pss_decode_pcn_list()");
         pss_free_client_list(pcn_list);
         return NCSCC_RC_FAILURE;
      }
      memset(p_tbl, '\0', sizeof(MAB_PSS_TBL_LIST));

      data = ncs_dec_flatten_space(uba, (uns8*)&p_tbl->tbl_id, 2);
      if(data == NULL)
      {
         m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
            "pss_decode_pcn_list()");
         pss_free_client_list(pcn_list);
         return NCSCC_RC_FAILURE;
      }
      p_tbl->tbl_id = ncs_decode_32bit(&data);
      ncs_dec_skip_space(uba, 4);
      m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_TBL_ID_DEC_SUCCESS, p_tbl->tbl_id);

      *pp_tbl = p_tbl;
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_vdest_role_quiesced

  DESCRIPTION:       This function changes the role of the PSS to Quiesced.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_process_vdest_role_quiesced(MAB_MSG *msg)
{
   PSS_PWE_CB         *pwe_cb = NULL;
   SaAisErrorT        saf_status = SA_AIS_OK; 
   uns32              rc = NCSCC_RC_SUCCESS;

   pwe_cb = (PSS_PWE_CB *) msg->yr_hdl;
   if(pwe_cb == NULL)
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_VDEST_ROLE_QUIESCED_INVLD_PWE);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

   rc = pss_set_ckpt_role(pwe_cb, SA_AMF_HA_QUIESCED);
   if(rc != NCSCC_RC_SUCCESS)
   {
      saf_status = SA_AIS_ERR_FAILED_OPERATION; 
   }

   /* send the response to AMF */     
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_QUIESCING)
   {
      saAmfCSIQuiescingComplete(gl_pss_amf_attribs.amf_attribs.amfHandle, 
         pwe_cb->amf_invocation_id, saf_status);
   }
   else
   {
      saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, pwe_cb->amf_invocation_id, saf_status);
   }
   pwe_cb->amf_invocation_id = 0;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_resume_active_role_activity

  DESCRIPTION:       This function performs immediate activities of the Active,
                     upon transitioning to the Active role(from Standy, or
                     Quiescing or Quiesced roles).
                     The activities are :
                        - Resume the Warmboot-session which was being 
                          played-back by the old-Active

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_resume_active_role_activity(MAB_MSG *msg)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   PSS_PWE_CB *pwe_cb = NULL;

   if((pwe_cb = msg->yr_hdl) == NULL)
   {
      return NCSCC_RC_FAILURE;
   }
   m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN2_RE_RESUME_ACTIVE_FUNCTIONALITY_INVOKED, pwe_cb->pwe_id);

   /* Continue the broken playback sessions here. */
   pwe_cb->processing_pending_active_events = TRUE;

   if(pwe_cb->curr_plbck_ssn_info.plbck_ssn_in_progress)
   {
      MAB_MSG lcl_msg;

      memset(&lcl_msg, '\0', sizeof(lcl_msg));
      lcl_msg.yr_hdl = (NCSCONTEXT)pwe_cb;

      if(pwe_cb->curr_plbck_ssn_info.is_warmboot_ssn)
      { 
         /* Invoke warmboot-playback function handler */
         (void)pss_process_oac_warmboot(&lcl_msg);
      } 
      else
      { 
         /* Invoke on-demand playback function handler */
         memset((char*)&pwe_cb->p_pss_cb->existing_profile,
            '\0', NCS_PSS_MAX_PROFILE_NAME);
         strcpy((char*)&pwe_cb->p_pss_cb->existing_profile,
            (char*)&pwe_cb->curr_plbck_ssn_info.info.alt_profile);

         (void)pss_on_demand_playback(pwe_cb->p_pss_cb, 
            gl_pss_amf_attribs.csi_list, 
            (char*)&pwe_cb->curr_plbck_ssn_info.info.alt_profile);
      } 
   }

   pwe_cb->processing_pending_active_events = FALSE;
   m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN2_RE_RESUME_ACTIVE_FUNCTIONALITY_COMPLETED, pwe_cb->pwe_id);

   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_re_update_pend_wbreq_info

  DESCRIPTION:       This function updates the pending-wbreq-list at Standby.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_re_update_pend_wbreq_info(PSS_PWE_CB *pwe_cb, 
            PSS_CKPT_PEND_WBREQ_TO_BAM_MSG *pend_wbreq_to_bam,
            NCS_MBCSV_ACT_TYPE action)
{
   if(action == NCS_MBCSV_ACT_RMV)
   {
      pss_del_entry_frm_spcn_wbreq_pend_list(pwe_cb, pend_wbreq_to_bam->pcn_list.pcn);
   }
   else
   {
      pss_add_entry_to_spcn_wbreq_pend_list(pwe_cb, pend_wbreq_to_bam->pcn_list
.pcn);
   }
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_encode_str
 * Purpose:       Encodes the string into the ubaid.
 ****************************************************************************/
uns32 pss_encode_str(NCS_UBAID *uba, char *str_ptr)
{
    uns8*    data;
    uns16    len = strlen(str_ptr);

    if(str_ptr == NULL)
    {
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    data = ncs_enc_reserve_space(uba, 2);
    if(data == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
           "pss_encode_str()");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    ncs_encode_16bit(&data,len);
    ncs_enc_claim_space(uba, 2);

    if(len != 0)
    {
        if(ncs_encode_n_octets_in_uba(uba, str_ptr, len) != NCSCC_RC_SUCCESS)
        {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_OCTETS_FAIL,
               "pss_encode_str()");
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_decode_str
 * Purpose:       Decodes the string 
 ****************************************************************************/
uns32 pss_decode_str(NCS_UBAID *uba, char *p_str)
{
    uns8*    data;
    uns16    len = 0;

    if(p_str == NULL)
    {
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    data = ncs_dec_flatten_space(uba, (uns8*)&len, 2);
    if(data == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
           "pss_decode_str()");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    len = ncs_decode_16bit(&data);
    ncs_dec_skip_space(uba, 2);

    if(len != 0)
    {
       memset(p_str, '\0', len+1);
       if(ncs_decode_n_octets_from_uba(uba, p_str, len) != NCSCC_RC_SUCCESS)
       {
           return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       }
    }
    else
    {
       if(p_str != NULL)
       {
          p_str[0] = '\0'; /* Nullify the string */
       }
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_encode_pcn
 * Purpose:       Encodes the <PCN> string into the ubaid.
 ****************************************************************************/
uns32 pss_encode_pcn(NCS_UBAID *uba, char *pcn)
{
    uns8*    data;
    uns16    len = strlen(pcn);

    if(pcn == NULL)
    {
       m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL);
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    data = ncs_enc_reserve_space(uba, 2);
    if(data == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_RESERVE_SPACE_FAIL,
           "pss_encode_pcn()");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    ncs_encode_16bit(&data,len);
    ncs_enc_claim_space(uba, 2);

    if(len != 0)
    {
        if(ncs_encode_n_octets_in_uba(uba, pcn, len) != NCSCC_RC_SUCCESS)
        {
            m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_ENC_OCTETS_FAIL,
               "pss_encode_pcn()");
            m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_ENC_PCN_FAIL, pcn);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_decode_pcn
 * Purpose:       Decodes the <PCN> string 
 ****************************************************************************/
uns32 pss_decode_pcn(NCS_UBAID *uba, char **pcn)
{
    uns8*    data;
    uns16    len = 0;

    if(pcn == NULL)
    {
       m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_PP_PCN_NULL);
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    data = ncs_dec_flatten_space(uba, (uns8*)&len, 2);
    if(data == NULL)
    {
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
           "pss_decode_pcn()");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    len = ncs_decode_16bit(&data);
    ncs_dec_skip_space(uba, 2);

    if(len != 0)
    {
       if(*pcn == NULL)
       {
          if((*pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len+1)) == NULL)
          {
             m_LOG_PSS_MEMFAIL(NCSFL_SEV_ERROR, PSS_MF_PCN_STRING_ALLOC_FAIL,
               "pss_decode_pcn()");
             return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          }
       }
       memset(*pcn, '\0', len+1);
       if(ncs_decode_n_octets_from_uba(uba, *pcn, len) != NCSCC_RC_SUCCESS)
       {
           m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_MBCSV_DEC_PCN_FAIL,
             "pss_decode_pcn()");
           m_MMGR_FREE_MAB_PCN_STRING(*pcn);
           return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       }
       m_LOG_PSS_HDLN_STR(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DEC_PCN_SUCCESS,
          (*pcn));
    }
    else
    {
       if(*pcn != NULL)
       {
          *pcn[0] = '\0'; /* Nullify the string */
       }
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_ckpt_enc_pend_wbreq_to_bam_info
 * Purpose:       Encodes the pending Warmboot-request-to-BAM info
 ****************************************************************************/
uns32 pss_ckpt_enc_pend_wbreq_to_bam_info(NCS_UBAID *uba, PSS_CKPT_PEND_WBREQ_TO_BAM_MSG *msg)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   rc = pss_encode_pcn_list(uba, &msg->pcn_list);

   return rc;
}

/****************************************************************************
 * Function Name: pss_ckpt_dec_pend_wbreq_to_bam_info
 * Purpose:       Decodes the pending Warmboot-request-to-BAM info
 ****************************************************************************/
uns32 pss_ckpt_dec_pend_wbreq_to_bam_info(NCS_UBAID *uba, PSS_CKPT_PEND_WBREQ_TO_BAM_MSG *msg)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   rc = pss_decode_pcn_list(uba, &msg->pcn_list);

   return rc;
}

/****************************************************************************\
*  Name:          pss_red_err_ind_handle                                     * 
*                                                                            *
*  Description:   Handles the error indications given by MBCSv Interface     * 
*                                                                            *
*  Arguments:     PSS_PWE_CB *pwe_cb - PSS control block for this PWE        *
*                 NCS_MBCSV_CB_ARG *arg - Error information                  * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Handled the error properly              *  
\****************************************************************************/
static uns32 pss_red_err_ind_handle(NCS_MBCSV_CB_ARG *cbk_arg) 
{
   SaAisErrorT    saf_status = SA_AIS_OK; 
   PSS_PWE_CB     *pwe_cb = NULL;

   /* Get PSS_PWE_CB control block Handle. cbk_arg->arg->i_client_hdl! */
   if (NULL == (pwe_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_PSS,cbk_arg->i_client_hdl)))
   {
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_PWE_CB_TAKE_HANDLE_IN_MBCSVCB_FAILED);
      return NCSCC_RC_FAILURE;
   }
   
   /* if this error indication happens in the Active PSS, do nothing */
   if((pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) ||
      (pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_QUIESCING))
   {
      /* ignore the errors in ACTIVE State */ 
      ncshm_give_hdl(cbk_arg->i_client_hdl);
      return NCSCC_RC_SUCCESS; 
   }
   /* process the error code in the standby MAS */ 
   if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_STANDBY)
   {
      /* based on the error indication, take action */ 
      switch (cbk_arg->info.error.i_code)
      {
         /* Cold Sync timer expired without getting response */
      case NCS_MBCSV_COLD_SYNC_TMR_EXP: 
         /* expected fall through, since the same error handling to be done */ 

         /* Cold Sync complete timer expired */
      case NCS_MBCSV_COLD_SYNC_CMPL_TMR_EXP: 
         /* expected fall through, since the same error handling to be done */ 

         /* Data Response complete timer expired */
      case NCS_MBCSV_DATA_RSP_CMPLT_TMR_EXP: 
         /* if the big boss is available, escalate the problem */ 
         /* Need re-evaluate the requirement to send ErrorReport to AMF. */
         if(gl_pss_amf_attribs.amf_attribs.amfHandle != 0)
         {
            saf_status = saAmfComponentErrorReport(gl_pss_amf_attribs.amf_attribs.amfHandle,
               &gl_pss_amf_attribs.amf_attribs.compName,
               0, /* errorDetectionTime; AvNd will provide this value */
               SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
            /* log a notify headline, which says that an error report has been sent */ 
            m_LOG_PSS_HDLN2_III(NCSFL_SEV_NOTICE, PSS_HDLN_MBCSV_ERR_IND_AMF_ERR_REPORT, 
               pwe_cb->pwe_id, cbk_arg->info.error.i_code, saf_status);
         }
         if (saf_status != SA_AIS_OK)
         {
            ncshm_give_hdl(cbk_arg->i_client_hdl);
            return NCSCC_RC_FAILURE; 
         }
         break; 
         
         /* 
          * anyway,  cold sync encode will be called by MBCSv after this error condition,
          * at that moment, all the required data in the CB is reset.  
          * So, no point in doing the same here. 
          * expected fall through, since the same error handling to be done
          */
         
         /* Warm Sync timer expired without getting response */
      case NCS_MBCSV_WARM_SYNC_TMR_EXP: 
         
         /* expected fall through, since the same error handling to be done */ 
         
         /* Warm Sync complete timer expired */
      case NCS_MBCSV_WARM_SYNC_CMPL_TMR_EXP:
         
         /* expected fall through, since the same error handling to be done */ 
         break; 
         
      default: 
         break; 
      } /* end of switch */ 
   } /* only if standby */ 
   
   ncshm_give_hdl(cbk_arg->i_client_hdl);
   return NCSCC_RC_SUCCESS; 
}

#endif /* #if (NCS_PSS_RED == 1) */
