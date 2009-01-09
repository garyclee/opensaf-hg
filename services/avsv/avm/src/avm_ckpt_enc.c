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

  DESCRIPTION: This module contains all the decode routines required for encodin
  AvM data structures during checkpointing. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

avm_encode_ckpt_ent_cfg
avm_encode_cold_sync_rsp_ent_state
avm_encode_ckpt_ent_state
avm_encode_ckpt_ent_state_sensor
avm_encode_ckpt_ent_dhcp_conf_chg
avm_encode_ckpt_ent_dhcp_state_chg
avm_encode_ckpt_evt_id
avm_encode_cold_sync_rsp 
avm_encode_ckpt_ent_upgd_state_chg
******************************************************************************
*/
#include "avm.h"

static uns32 
avm_encode_cold_sync_rsp_validation_info(AVM_CB_T             *cb,
                                    NCS_MBCSV_CB_ENC     *enc);
static uns32
avm_encode_cold_sync_rsp_ent_state(AVM_CB_T             *cb,
                                   NCS_MBCSV_CB_ENC     *enc);
static uns32 
avm_encode_cold_sync_rsp_async_updt_cnt(AVM_CB_T           *cb,
                                        NCS_MBCSV_CB_ENC   *enc);


static uns32 
avm_encode_ckpt_ent_cfg(AVM_CB_T           *cb,
                        NCS_MBCSV_CB_ENC   *enc);

static uns32 avm_encode_ckpt_evt_id(AVM_CB_T           *cb,
                                    NCS_MBCSV_CB_ENC   *enc);

static uns32 
avm_encode_ckpt_ent_state(AVM_CB_T           *cb,
                          NCS_MBCSV_CB_ENC   *enc);

static uns32 
avm_encode_ckpt_ent_state_sensor(AVM_CB_T           *cb,
                                 NCS_MBCSV_CB_ENC   *enc);

static uns32 
avm_encode_ckpt_ent_add_dependent(AVM_CB_T           *cb,
                                  NCS_MBCSV_CB_ENC   *enc);

static uns32 
avm_encode_ckpt_evt_id(AVM_CB_T           *cb,
                       NCS_MBCSV_CB_ENC   *enc);

static uns32 
avm_encode_ckpt_ent_adm_op(AVM_CB_T           *cb,
                           NCS_MBCSV_CB_ENC   *enc);


static uns32
avm_complete_db(
                  AVM_CB_T          *cb,
                  NCS_MBCSV_CB_ENC  *enc,
                  NCS_BOOL           cold_sync
                );

/* to update entity DHCP configuration */
static uns32 
avm_encode_ckpt_ent_dhcp_conf_chg(AVM_CB_T  *cb,
                        NCS_MBCSV_CB_ENC    *enc);

/* to update entity DHCP conf state */
static uns32 
avm_encode_ckpt_ent_dhcp_state_chg(AVM_CB_T *cb,
                        NCS_MBCSV_CB_ENC    *enc);

/* to update entity upgrade state */
static uns32 
avm_encode_ckpt_ent_upgd_state_chg(AVM_CB_T *cb,
                        NCS_MBCSV_CB_ENC    *enc);

/*
 * Function list for encoding the async data.
 * We will jump into this function using the reo_type received
 * in the encode argument.
 */

const AVM_ENCODE_CKPT_DATA_FUNC_PTR
         avm_enc_ckpt_data_func_list[AVM_CKPT_MSG_MAX] = 
{
   /* To update entity configuration */
   avm_encode_ckpt_ent_cfg, 

   /* To update entity DHCP configuration */
   avm_encode_ckpt_ent_dhcp_conf_chg, 

   /* To update entity DHCP conf state */
   avm_encode_ckpt_ent_dhcp_state_chg, 

   /* To update entity current state */
   avm_encode_ckpt_ent_state,

   /* To update entity current state along with sensors asserted */
   avm_encode_ckpt_ent_state_sensor,
   
   avm_encode_ckpt_ent_add_dependent,
   
   /* To update adm state */
   avm_encode_ckpt_ent_adm_op,

   /* Event id processed at Active AvM  */
   avm_encode_ckpt_evt_id,

   /* To update entity upgrade state */
   avm_encode_ckpt_ent_upgd_state_chg
};

/*
 * Function list for encoding the cold sync response data
 * We will jump into this function using the reo_type received
 * in the cold sync repsonce argument.
 */
const AVM_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR
         avm_enc_cold_sync_rsp_data_func_list[AVM_CKPT_COLD_SYNC_MSG_MAX] = 
{
   avm_encode_cold_sync_rsp_validation_info,   
   avm_encode_cold_sync_rsp_ent_state,
   avm_encode_cold_sync_rsp_async_updt_cnt,
};


/****************************************************************************\
 * Function: avm_encode_ckpt_ent_cfg
 *
 * Purpose:  Encode entity state.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_ent_cfg(AVM_CB_T           *cb,
                        NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent");

   switch(enc->io_action)
   {   
      case NCS_MBCSV_ACT_ADD:
      case NCS_MBCSV_ACT_UPDATE:   
      { 
 
        status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, EDP_OP_TYPE_ENC,
                                   (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version);
      }
      break;
      
      case NCS_MBCSV_ACT_RMV:
      { 
         /* Send Key */
/*         cb->edu_hdl.to_version = enc->i_peer_version; */
         status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, EDP_OP_TYPE_ENC, 
                                        (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version, 1, 2);
      }  
      break;
      
      default:
      {
          /* Log Error */
          m_AVM_LOG_INVALID_VAL_FATAL(enc->io_reo_type);
         return NCSCC_RC_FAILURE;
      }  
   }
     
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }
   
   return status;
}

/****************************************************************************\
 * Function: avm_encode_ckpt_ent_state
 *
 * Purpose:  Encode entity state.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_ent_state(AVM_CB_T           *cb,
                    NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent_state");

    status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, EDP_OP_TYPE_ENC,
                                   (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version, 2, 2, 10);
    
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }
   
   return status;
}


/****************************************************************************\
 * Function: avm_encode_ckpt_ent_state_sensor
 *
 * Purpose:  Encode 
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_ent_state_sensor(AVM_CB_T           *cb,
                                 NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;

   EDU_ERR             ederror  = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent_state_sensor");

   status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, EDP_OP_TYPE_ENC,
                                  (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version, 5, 2, 9, 12, 13, 14);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   return status;
}

/**************************************************************************
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_ent_adm_op(AVM_CB_T           *cb,
                           NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent_state");

   status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, EDP_OP_TYPE_ENC,
                                  (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version, 4, 2, 18, 19, 20);
    
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }
   
   return status;
}

/**************************************************************************
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_ent_add_dependent(AVM_CB_T           *cb,
                                  NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent_add_dep");

   status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, EDP_OP_TYPE_ENC,
                                  (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version, 2, 2, 8);
    
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }
   
   return status;
}

/****************************************************************************\
 * Function: avm_encode_ckpt_evt_id
 *
 * Purpose:  Encode event id of Event processed at Active
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_evt_id(AVM_CB_T           *cb,
                       NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_evt_id");

   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_evt_id, &enc->io_uba, EDP_OP_TYPE_ENC, 
                               (AVM_EVT_ID_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }
   return status;
}

/****************************************************************************\
 * Function: avm_encode_ckpt_ent_dhcp_conf_chg
 *
 * Purpose:  Encode to update entity DHCP configuration
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_ent_dhcp_conf_chg(AVM_CB_T           *cb,
                                 NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;

   EDU_ERR             ederror  = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent_dhcp_conf_chg");

   status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba,
                          EDP_OP_TYPE_ENC, (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version, 2, 2, 22);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   return status;
}

/****************************************************************************\
 * Function: avm_encode_ckpt_ent_dhcp_state_chg
 *
 * Purpose:  Encode to update entity DHCP conf state
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_ckpt_ent_dhcp_state_chg(AVM_CB_T           *cb,
                                 NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;

   EDU_ERR             ederror  = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent_dhcp_state_chg");

   status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, 
                          EDP_OP_TYPE_ENC, (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, enc->i_peer_version, 2, 2, 23);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   return status;
}

/****************************************************************************\
 * Function: avm_encode_ckpt_ent_upgd_state_chg 
 *
 * Purpose:  Encode to update entity upgrade state
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32
avm_encode_ckpt_ent_upgd_state_chg(AVM_CB_T           *cb,
                                 NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;

   EDU_ERR             ederror  = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_ckpt_ent_upgd_state_chg");

   /***************************************************************
      If the peer version is less than 2, it can't understand 
      these fields. So, just return Success   
   ***************************************************************/  
   if(enc->i_peer_version < 2)
   {
      /* vivsp4 */
      /* Should return failure otherwise mbcsv send empty message to peer */
#if 0
      return NCSCC_RC_SUCCESS;
#endif
      return NCSCC_RC_FAILURE;
   }

   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, 
                          EDP_OP_TYPE_ENC, (AVM_ENT_INFO_T*)(NCS_INT64_TO_PTR_CAST(enc->io_reo_hdl)), &ederror, 2, 2, 26);
#if 0 /* remove this code later */
   status = m_NCS_EDU_SEL_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, 
                    EDP_OP_TYPE_ENC, (AVM_ENT_INFO_T*)enc->io_reo_hdl, &ederror,enc->i_peer_version, 2, 2, 25);
#endif

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   return status;
  
}


/****************************************************************************\
 * Function: avm_encode_cold_sync_rsp
 *
 * Purpose:  Encode cold sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
extern uns32 
avm_encode_cold_sync_rsp(AVM_CB_T            *cb,
                         NCS_MBCSV_CB_ENC    *enc)
{
   uns32 rc;   
   m_AVM_LOG_FUNC_ENTRY("avm_encode_data_sync_rsp");
   
   rc = avm_complete_db(cb, enc, TRUE);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(rc);
   }   
   cb->cold_sync = TRUE;
   return rc;
}

/*************************************************************************\
* Function: avm_encode_cold_sync_rsp_validation_info
*
* Purpose:  Encode entire DB data..
*
* Input: cb - CB pointer.
*        enc - Encode arguments passed by MBCSV.
*
* Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
* NOTES:
*
*
\************************************************************************/

static uns32 
avm_encode_cold_sync_rsp_validation_info(AVM_CB_T             *cb,
                                    NCS_MBCSV_CB_ENC     *enc)
{
   uns32                   status     = NCSCC_RC_SUCCESS;

   EDU_ERR                 ederror    = 0;
   AVM_VALID_INFO_T       *valid_info;
   AVM_ENT_DESC_NAME_T     desc_name;

   uns32 num_of_inst = 0;
   uns8  *enc_cnt_loc; 


   m_AVM_LOG_FUNC_ENTRY("avm_encode_cold_sync_rsp_ckpt_ent_state");

   enc_cnt_loc = ncs_enc_reserve_space(&enc->io_uba, sizeof(uns32));
   
   ncs_enc_claim_space(&enc->io_uba, sizeof(uns32));

   m_NCS_MEMSET(desc_name.name, '\0', NCS_MAX_INDEX_LEN);

   for(valid_info = (AVM_VALID_INFO_T*) ncs_patricia_tree_getnext(&cb->db.valid_info_anchor, (uns8*)desc_name.name); 
       valid_info != AVM_VALID_INFO_NULL; 
       valid_info = (AVM_VALID_INFO_T*) ncs_patricia_tree_getnext(&cb->db.valid_info_anchor, (uns8*)desc_name.name))
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_validation_info, &enc->io_uba, EDP_OP_TYPE_ENC,
                                  valid_info, &ederror, enc->i_peer_version);

      if(NCSCC_RC_SUCCESS != status)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }
      
      m_NCS_MEMCPY(desc_name.name, valid_info->desc_name.name, NCS_MAX_INDEX_LEN);

      (num_of_inst)++;
   }

   if(enc_cnt_loc != NULL)
   {   
      ncs_encode_32bit(&enc_cnt_loc, num_of_inst);
   }

   return status;
}

/*************************************************************************\
* Function: avm_encode_cold_sync_rsp_ent_state
*
* Purpose:  Encode entire DB data..
*
* Input: cb - CB pointer.
*        enc - Encode arguments passed by MBCSV.
*
* Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
* NOTES:
*
*
\************************************************************************/

static uns32 
avm_encode_cold_sync_rsp_ent_state(AVM_CB_T             *cb,
                                   NCS_MBCSV_CB_ENC     *enc)
{
   uns32              status     = NCSCC_RC_SUCCESS;

   EDU_ERR            ederror    = 0;
   AVM_ENT_INFO_T    *ent_info;
   AVM_ENT_PATH_STR_T ep;

   uns32 num_of_inst = 0;
   uns8  *enc_cnt_loc; 


   m_AVM_LOG_FUNC_ENTRY("avm_encode_cold_sync_rsp_ckpt_ent_state");

   enc_cnt_loc = ncs_enc_reserve_space(&enc->io_uba, sizeof(uns32));
   
   ncs_enc_claim_space(&enc->io_uba, sizeof(uns32));

   m_NCS_MEMSET(ep.name, '\0', AVM_MAX_INDEX_LEN); 
   ep.length = 0;

   for(ent_info = avm_find_next_ent_str_info(cb, &ep); 
       ent_info != AVM_ENT_INFO_NULL; 
       ent_info = (AVM_ENT_INFO_T*) avm_find_next_ent_str_info(cb, &ep))
   {
      if(NCS_ROW_ACTIVE != ent_info->row_status)
      {
         m_NCS_MEMCPY(ep.name, ent_info->ep_str.name, AVM_MAX_INDEX_LEN);
         ep.length = m_NCS_OS_NTOHS(ent_info->ep_str.length);
         continue;
      }
       
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &enc->io_uba, EDP_OP_TYPE_ENC, 
                                  ent_info, &ederror, enc->i_peer_version);

      m_NCS_DBG_PRINTF("\n Label name = %s",ent_info->dhcp_serv_conf.label1.name.name);

      if(NCSCC_RC_SUCCESS != status)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      m_NCS_MEMCPY(ep.name, ent_info->ep_str.name, AVM_MAX_INDEX_LEN);
      ep.length = m_NCS_OS_NTOHS(ent_info->ep_str.length);
      
      (num_of_inst)++;
   }

   if(enc_cnt_loc != NULL)
   {   
      ncs_encode_32bit(&enc_cnt_loc, num_of_inst);
   }

   return status;
}

/****************************************************************************\
 * Function: avm_encode_cold_sync_rsp_async_updt_cnt
 *
 * Purpose:  Encode entity state.
 *
 * Input: cb - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_encode_cold_sync_rsp_async_updt_cnt(AVM_CB_T           *cb,
                                        NCS_MBCSV_CB_ENC   *enc)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_encode_cold_sync_rsp_async_updt_cnt");

   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_async_updt_cnt, &enc->io_uba, EDP_OP_TYPE_ENC, 
                               &cb->async_updt_cnt, &ederror, enc->i_peer_version);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }
   
   return status;
}

/****************************************************************************\
 * Function: avm_encode_warm_sync_rsp
 *
 * Purpose:  Encode Warm sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avm_encode_warm_sync_rsp(AVM_CB_T *cb, NCS_MBCSV_CB_ENC *enc)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   uns8  sprbuf[200]; /* Using hard code value since it doesnot add any value if  defined as a macro  */ 

   m_AVM_LOG_FUNC_ENTRY("avm_encode_warm_sync_rsp");

   m_NCS_MEMSET(sprbuf, '\0', sizeof(sprbuf));   

   sprintf(sprbuf, " ent_updt = %d ent_cfg_updt = %d adm_op_updt = %d evt_id_updt = %d \n", cb->async_updt_cnt.ent_updt, cb->async_updt_cnt.ent_cfg_updt, cb->async_updt_cnt.ent_adm_op_updt, cb->async_updt_cnt.evt_id_updt);

   m_AVM_LOG_GEN_EP_STR("Async Update Counts", sprbuf, NCSFL_SEV_INFO);

   /* 
    * Encode and send latest async update counts. (In the same manner we sent
    * in the last message of the cold sync response.
    */

   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_async_updt_cnt,
      &enc->io_uba, EDP_OP_TYPE_ENC, &cb->async_updt_cnt, &ederror, enc->i_peer_version);
   
   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
   }

   return status;
}

/************************************************************** 
 * Function: avm_encode_data_sync_rsp
 *
 * Purpose:  Encode Warm sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
 *************************************************************/
extern uns32
avm_encode_data_sync_rsp(
                           AVM_CB_T          *cb,
                           NCS_MBCSV_CB_ENC  *enc
                        )
{
   uns32 rc;   
   m_AVM_LOG_FUNC_ENTRY("avm_encode_data_sync_rsp");
   
   rc = avm_complete_db(cb, enc, FALSE);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(rc);
   }   
   return rc;
}

/************************************************************** 
 * Function: avm_complete_db
 *
 * Purpose:  Encode Warm sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *************************************************************/
static uns32
avm_complete_db(
                  AVM_CB_T          *cb,
                  NCS_MBCSV_CB_ENC  *enc,
                  NCS_BOOL           cold_sync
                )
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 i;

   m_AVM_LOG_FUNC_ENTRY("avm_complete_db");
   
   for(i = enc->io_reo_type; i < AVM_CKPT_COLD_SYNC_MSG_MAX; i++)
   {
      status = avm_enc_cold_sync_rsp_data_func_list[i](cb, enc);
      if(NCSCC_RC_SUCCESS != status)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(i);
         rc = NCSCC_RC_FAILURE;
      }
   }

   if(NCSCC_RC_SUCCESS == status)
   {
      if(cold_sync)
      {
         enc->io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
      }else
      {
         enc->io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE;
      }
   }
   
   return rc;
}
