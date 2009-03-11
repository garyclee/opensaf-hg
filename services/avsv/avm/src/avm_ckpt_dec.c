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

  DESCRIPTION: This module contain all the decode routines require for decoding
  AvM data structures during checkpointing. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

avm_decode_ckpt_ent_chg
avm_decode_cold_sync_rsp_ent_state
avm_decode_ckpt_ent_state
avm_decode_ckpt_ent_dhcp_conf_chg
avm_decode_ckpt_ent_dhcp_state_chg
avm_decode_ckpt_ent_state_sensor
avm_decode_ckpt_evt_id
avm_decode_cold_sync_rsp 
avm_decode_ckpt_ent_upgd_state_chg
******************************************************************************
*/

#include "avm.h"

static uns32 
avm_decode_cold_sync_rsp_validation_info(AVM_CB_T             *cb,
                                         NCS_MBCSV_CB_DEC     *dec);
static uns32 
avm_decode_cold_sync_rsp_ent_cfg(AVM_CB_T             *cb,
                                 NCS_MBCSV_CB_DEC     *dec);
static uns32 
avm_decode_cold_sync_rsp_async_updt_cnt(AVM_CB_T           *cb,
                                        NCS_MBCSV_CB_DEC   *dec);

static uns32 
avm_decode_ckpt_ent_cfg(AVM_CB_T           *cb,
                        NCS_MBCSV_CB_DEC   *dec);

static uns32 
avm_decode_ckpt_ent_dhcp_conf_chg(AVM_CB_T           *cb,
                        NCS_MBCSV_CB_DEC   *dec);

static uns32 
avm_decode_ckpt_ent_dhcp_state_chg(AVM_CB_T           *cb,
                        NCS_MBCSV_CB_DEC   *dec);

static uns32 
avm_decode_ckpt_ent_state(AVM_CB_T           *cb,
                          NCS_MBCSV_CB_DEC   *dec);

static uns32
avm_decode_ckpt_ent_state_sensor(AVM_CB_T           *cb,
                                 NCS_MBCSV_CB_DEC   *dec);

static uns32 
avm_decode_ckpt_ent_add_dependent(AVM_CB_T           *cb,
                                  NCS_MBCSV_CB_DEC   *dec);
static uns32 
avm_decode_ckpt_ent_adm_op(AVM_CB_T           *cb,
                           NCS_MBCSV_CB_DEC   *dec);

static uns32 
avm_decode_ckpt_evt_id(AVM_CB_T           *cb,
                       NCS_MBCSV_CB_DEC   *dec);

static uns32 
avm_decode_ckpt_ent_upgd_state_chg(AVM_CB_T           *cb,
                        NCS_MBCSV_CB_DEC   *dec);

/*
 * Function list for decoding the async data.
 * We will jump into this function using the reo_type received
 * in the decode argument.
 */

const AVM_DECODE_CKPT_DATA_FUNC_PTR
         avm_dec_ckpt_data_func_list[AVM_CKPT_MSG_MAX] = 
{
   avm_decode_ckpt_ent_cfg,

   /* To update entity DHCP configuration */
   avm_decode_ckpt_ent_dhcp_conf_chg, 

   /* To update entity DHCP conf state */
   avm_decode_ckpt_ent_dhcp_state_chg, 

   /* Entity State update messages */
   avm_decode_ckpt_ent_state,
   avm_decode_ckpt_ent_state_sensor,
   avm_decode_ckpt_ent_add_dependent,
   avm_decode_ckpt_ent_adm_op,
   
   /* EDSv event id processed */
   avm_decode_ckpt_evt_id,

   /* To update entity upgd state */
   avm_decode_ckpt_ent_upgd_state_chg 
};

/*
 * Function list for decoding the cold sync response data
 * We will jump into this function using the reo_type received
 * in the cold sync repsonce argument.
 */

const AVM_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR
         avm_dec_cold_sync_rsp_data_func_list[] = 
{
   avm_decode_cold_sync_rsp_validation_info,
   avm_decode_cold_sync_rsp_ent_cfg,
   avm_decode_cold_sync_rsp_async_updt_cnt,
};
/****************************************************************************\
 * Function: avm_decode_ckpt_ent_cfg
 *
 * Purpose:  Decode entity state.
 *
 * Input: cb - CB pointer.
 *        enc - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_decode_ckpt_ent_cfg(AVM_CB_T           *cb,
                        NCS_MBCSV_CB_DEC   *dec)
{
   uns32                status   = NCSCC_RC_SUCCESS;
   EDU_ERR              ederror    = 0;
   AVM_ENT_INFO_T      *ent_info;
   AVM_ENT_INFO_T       dec_ent;
   AVM_ENT_INFO_T      *temp_ent_info;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_cfg");
   
   ent_info = &dec_ent;
   
   switch(dec->i_action)
   {   
      case NCS_MBCSV_ACT_ADD:
      { 
        status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, EDP_OP_TYPE_DEC, 
                                   (AVM_ENT_INFO_T**)&ent_info, &ederror, dec->i_peer_version);
      }
      break;
      
      case NCS_MBCSV_ACT_RMV:
      { 
         /* Send Key */
        status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, EDP_OP_TYPE_DEC, (AVM_ENT_INFO_T**)&ent_info, &ederror, 1, 2);
      }  
      break;
      
      default:
      {
         /* Log Error */
         m_AVM_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
         return NCSCC_RC_FAILURE;
      }  
   }
     
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avm_ckpt_update_ent_db(cb, ent_info, dec->i_action, &temp_ent_info);
   if(NCSCC_RC_SUCCESS == status)
   {
      cb->async_updt_cnt.ent_cfg_updt++;
   }
   
   return status;
}


/****************************************************************************\
 * Function: avm_decode_ckpt_ent_state
 *
 * Purpose:  Decode entire entity state data..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/

static uns32 
avm_decode_ckpt_ent_state(AVM_CB_T           *cb,
                          NCS_MBCSV_CB_DEC   *dec)
{
   uns32              status;
   EDU_ERR            ederror    = 0;
   AVM_ENT_INFO_T    *temp_ent_info;
   AVM_ENT_INFO_T     dec_ent;
   AVM_ENT_INFO_T    *ent_info;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_state");

   temp_ent_info = &dec_ent;

   /* decode eneity path and current state of the entity */
   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, EDP_OP_TYPE_DEC, &temp_ent_info, &ederror, 2, 2, 10);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &temp_ent_info->entity_path)))
   {
       m_AVM_LOG_INVALID_VAL_ERROR(dec->i_reo_type);
       return NCSCC_RC_FAILURE;   
   }
   
   ent_info->previous_state = temp_ent_info->previous_state;
   ent_info->current_state  = temp_ent_info->current_state;

   if((AVM_ENT_ACTIVE == ent_info->current_state) &&
      (AVM_ENT_INACTIVE == ent_info->previous_state))
   {
       avm_remove_dependents(ent_info);
   }

   cb->async_updt_cnt.ent_updt++;

   return status;
}

/****************************************************************************\
 * Function: avm_decode_ckpt_ent_state_sensor
 *
 * Purpose:  Decode entire entity state dnd sensor ata..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32
avm_decode_ckpt_ent_state_sensor(AVM_CB_T           *cb,
                                 NCS_MBCSV_CB_DEC   *dec)
{
   uns32               status;
   EDU_ERR             ederror    = 0;
   AVM_ENT_INFO_T     *ent_info;
   AVM_ENT_INFO_T      dec_ent;
   AVM_ENT_INFO_T     *temp_ent_info;
 
   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_state_sensor");

   temp_ent_info = &dec_ent;  
   
   /* decode entity path, current state an sesnors asserted */

   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, EDP_OP_TYPE_DEC, &temp_ent_info, &ederror, 5, 2, 12, 15, 16, 17);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &temp_ent_info->entity_path)))
   {
       m_AVM_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
       return NCSCC_RC_FAILURE;   
   }
   
   ent_info->previous_state = temp_ent_info->previous_state;
   ent_info->current_state  = temp_ent_info->current_state;

   avm_update_sensors(ent_info, temp_ent_info);

   cb->async_updt_cnt.ent_sensor_updt++;

   return status;
}


/****************************************************************************\
 * Function: avm_decode_ckpt_ent_dhcp_conf_chg
 *
 * Purpose:  Decode DHCP configuration change.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/

static uns32 
avm_decode_ckpt_ent_dhcp_conf_chg(AVM_CB_T           *cb,
                          NCS_MBCSV_CB_DEC   *dec)
{
   uns32              status;
   EDU_ERR            ederror    = 0;
   AVM_ENT_INFO_T    *temp_ent_info;
   AVM_ENT_INFO_T     dec_ent;
   AVM_ENT_INFO_T    *ent_info;
   AVM_ENT_DHCP_CONF  *dhcp_conf, *temp_conf;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_dhcp_conf_chg");

   temp_ent_info = &dec_ent;

   /* decode dhcp configuration */
   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, 
                          EDP_OP_TYPE_DEC, &temp_ent_info, &ederror, 2, 2, 22);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &temp_ent_info->entity_path)))
   {
       m_AVM_LOG_INVALID_VAL_ERROR(dec->i_reo_type);
       return NCSCC_RC_FAILURE;   
   }
   dhcp_conf = &ent_info->dhcp_serv_conf;
   temp_conf = &temp_ent_info->dhcp_serv_conf;
   avm_decode_ckpt_dhcp_conf_update(dhcp_conf, temp_conf);

   cb->async_updt_cnt.ent_dhconf_updt++;

   return status;
}


/****************************************************************************\
 * Function: avm_decode_ckpt_ent_dhcp_state_chg
 *
 * Purpose:  Decode DHCP state change.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/

static uns32 
avm_decode_ckpt_ent_dhcp_state_chg(AVM_CB_T           *cb,
                          NCS_MBCSV_CB_DEC   *dec)
{
   uns32              status;
   EDU_ERR            ederror    = 0;
   AVM_ENT_INFO_T    *temp_ent_info;
   AVM_ENT_INFO_T     dec_ent;
   AVM_ENT_INFO_T    *ent_info;
   AVM_ENT_DHCP_CONF  *dhcp_conf, *temp_conf;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_dhcp_state_chg");

   temp_ent_info = &dec_ent;

   /* decode dhcp state */
   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, 
                          EDP_OP_TYPE_DEC, &temp_ent_info, &ederror, 2, 2, 23);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &temp_ent_info->entity_path)))
   {
       m_AVM_LOG_INVALID_VAL_ERROR(dec->i_reo_type);
       return NCSCC_RC_FAILURE;   
   }
   dhcp_conf = &ent_info->dhcp_serv_conf;
   temp_conf = &temp_ent_info->dhcp_serv_conf;
   avm_decode_ckpt_dhcp_state_update(dhcp_conf, temp_conf);

   cb->async_updt_cnt.ent_dhstate_updt++;

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
avm_decode_ckpt_ent_add_dependent(AVM_CB_T             *cb,
                                    NCS_MBCSV_CB_DEC   *dec)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;
   
   AVM_ENT_INFO_T      dec_ent;
   AVM_ENT_INFO_T     *temp_ent_info;
   AVM_ENT_INFO_T     *ent_info;
   AVM_ENT_INFO_T     *dep_ent_info;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_add_dep");
   
   temp_ent_info = &dec_ent;

   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, EDP_OP_TYPE_DEC, &temp_ent_info, &ederror, 2, 2, 8);
    
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &temp_ent_info->entity_path)))
   {
      m_AVM_LOG_INVALID_VAL_ERROR(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }
   
   if(AVM_ENT_INFO_NULL == (dep_ent_info = avm_find_ent_str_info(cb, &temp_ent_info->dep_ep_str,TRUE)))
   {
      m_AVM_LOG_INVALID_VAL_ERROR(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }
   
   avm_add_dependent(ent_info, dep_ent_info);

   cb->async_updt_cnt.ent_dep_updt++;
   
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
**************************************************************************/
static uns32 
avm_decode_ckpt_ent_adm_op(AVM_CB_T           *cb,
                           NCS_MBCSV_CB_DEC   *dec)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;
   
   AVM_ENT_INFO_T      dec_ent;
   AVM_ENT_INFO_T     *temp_ent_info;
   AVM_ENT_INFO_T     *ent_info;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_adm_op");
   
   temp_ent_info = &dec_ent;

   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, EDP_OP_TYPE_DEC, &temp_ent_info, &ederror, 4, 2, 18, 19, 20);
    
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &temp_ent_info->entity_path)))
   {
      m_AVM_LOG_INVALID_VAL_ERROR(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }
   
   ent_info->adm_lock      = temp_ent_info->adm_lock;
   ent_info->adm_shutdown  = temp_ent_info->adm_shutdown;
   ent_info->adm_req       = temp_ent_info->adm_req;
   
   cb->async_updt_cnt.ent_adm_op_updt++;

   return status;
}



/****************************************************************************\
 * Function: avm_decode_ckpt_evt_id
 *
 * Purpose:  Decode event id
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_decode_ckpt_evt_id(AVM_CB_T           *cb,
                       NCS_MBCSV_CB_DEC   *dec)
{
   uns32               status;
   EDU_ERR             ederror    = 0;
   AVM_EVT_ID_T       *evt_id;
   AVM_EVT_ID_T        dec_evt_id;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_evt_id");

   evt_id = &dec_evt_id; 

   /* decode event id processed ate active AvM */
   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_evt_id, &dec->i_uba, EDP_OP_TYPE_DEC, 
                               &evt_id, &ederror, dec->i_peer_version);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   /* Update the event id processed at Active in Stanbdy , If the event is already queued, dequeue it or else add the event is in the queue */
   status = avm_updt_evt_q(cb, AVM_EVT_NULL, evt_id->evt_id, TRUE);

   if(NCSCC_RC_SUCCESS != status)
   {
      return NCSCC_RC_FAILURE;
   }
   cb->async_updt_cnt.evt_id_updt++;

   return status;
}

/****************************************************************************\
 * Function: avm_decode_cold_sync_rsp_validation_info
 *
 * Purpose:  Decode entity.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_decode_cold_sync_rsp_validation_info(AVM_CB_T             *cb,
                                         NCS_MBCSV_CB_DEC     *dec)
{
   uns32               status     = NCSCC_RC_SUCCESS;
   uns32               count;
   EDU_ERR             ederror    = 0;
   AVM_VALID_INFO_T   *valid_info;
   AVM_VALID_INFO_T    dec_valid;
   uns32               num_of_inst;

   uns8 *buff;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_cold_sync_rsp_ckpt_valid_info");

   buff = ncs_dec_flatten_space(&dec->i_uba, (uns8*)&num_of_inst, sizeof(uns32));
   num_of_inst = ncs_decode_32bit(&buff);
   ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

   valid_info = &dec_valid; 

   /* Decode all the entities and add the into DB of stanbdy */
   for(count  = 0; count < num_of_inst; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_validation_info, &dec->i_uba, EDP_OP_TYPE_DEC,
                                  &valid_info, &ederror, dec->i_peer_version);

      if(NCSCC_RC_SUCCESS != status)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avm_ckpt_update_valid_info_db(cb, valid_info, dec->i_action);
   
      if(NCSCC_RC_SUCCESS != status)
      {
         return NCSCC_RC_FAILURE;
      }
   }

   status = avm_bld_valid_info_parent_child_relation(cb);
   return status;
}

/****************************************************************************\
 * Function: avm_decode_cold_sync_rsp
 *
 * Purpose:  Decode cold sync response message 
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uns32 avm_decode_cold_sync_rsp(AVM_CB_T             *cb,
                               NCS_MBCSV_CB_DEC    *dec)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 status = NCSCC_RC_SUCCESS;
   uns32 i;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_cold_sync_rsp");

   dec->i_action = NCS_MBCSV_ACT_ADD;

   for(i = dec->i_reo_type; i < AVM_CKPT_COLD_SYNC_MSG_MAX; i++)
   {
      status = avm_dec_cold_sync_rsp_data_func_list[i](cb, dec);
      if(NCSCC_RC_SUCCESS != status)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(i);
         rc = NCSCC_RC_FAILURE;
      }
   }
   
   if(NCSCC_RC_SUCCESS == status)
   {
     cb->config_state = AVM_CONFIG_DONE;
   }
   return status;

}

/****************************************************************************\
 * Function: avm_decode_cold_sync_rsp_ent_cfg
 *
 * Purpose:  Decode entity.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
static uns32 
avm_decode_cold_sync_rsp_ent_cfg(AVM_CB_T             *cb,
                                 NCS_MBCSV_CB_DEC     *dec)
{
   uns32                status     = NCSCC_RC_SUCCESS;
   uns32                count;
   EDU_ERR              ederror    = 0;
   AVM_ENT_INFO_T      *ent_info;
   AVM_ENT_INFO_T       dec_ent;
   AVM_ENT_INFO_T      *temp_ent[AVM_MAX_ENTITIES];
   AVM_ENT_INFO_T      *temp_ent_info;
   AVM_ENT_INFO_T      *t_ent_info;
   AVM_ENT_INFO_LIST_T *temp_ent_info_list;
   AVM_ENT_INFO_LIST_T *t_ent_info_list;
   AVM_ENT_INFO_LIST_T *temp;

   uns32                num_of_inst;
   uns32                temp_ent_cnt = 0;
   uns32                i = 0; 
   uns32                rc = 0;

   uns8 *buff;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_cold_sync_rsp_ckpt_ent_cfg");


   buff = ncs_dec_flatten_space(&dec->i_uba, (uns8*)&num_of_inst, sizeof(uns32));
   num_of_inst = ncs_decode_32bit(&buff);
   ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

   ent_info = &dec_ent; 

   /* Decode all the entities and add the into DB of stanbdy */
   for(count  = 0; count < num_of_inst; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, EDP_OP_TYPE_DEC,
                                  &ent_info, &ederror, dec->i_peer_version);

      if(NCSCC_RC_SUCCESS != status)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avm_ckpt_update_ent_db(cb, ent_info, dec->i_action, &(temp_ent_info));

      if(NCSCC_RC_SUCCESS != status)
      {
         return NCSCC_RC_FAILURE;
      }

      for(temp_ent_info_list  = temp_ent_info->dependents; 
          temp_ent_info_list != AVM_ENT_INFO_LIST_NULL; 
          temp_ent_info_list  = temp_ent_info_list->next)
      {
          m_AVM_ENT_IS_VALID(temp_ent_info_list->ent_info, rc);
          if(!rc)
          {
             temp_ent[temp_ent_cnt++] = temp_ent_info;
             break;
          }
      }
   }
   
   for(i = 0; i < temp_ent_cnt; i++)
   {
      ent_info = temp_ent[i];

      for(temp_ent_info_list  = ent_info->dependents; 
          temp_ent_info_list != AVM_ENT_INFO_LIST_NULL; 
          temp_ent_info_list  = temp_ent_info_list->next)
      {
          m_AVM_ENT_IS_VALID(temp_ent_info_list->ent_info, rc)
          if(!rc)
          {
             t_ent_info = avm_find_ent_str_info(cb, temp_ent_info_list->ep_str ,FALSE);
             if(AVM_ENT_INFO_NULL == t_ent_info)
             {
               for(t_ent_info_list = ent_info->dependents; t_ent_info_list != AVM_ENT_INFO_LIST_NULL;)
               {
                  m_AVM_ENT_IS_VALID(t_ent_info_list->ent_info, rc);
                  if(!rc)
                  {
                     m_MMGR_FREE_AVM_ENT_INFO(t_ent_info_list->ent_info);
                  }  
                  temp =  t_ent_info_list;
                  t_ent_info_list = t_ent_info_list->next;
                  m_MMGR_FREE_AVM_ENT_INFO_LIST(temp);
               }
               ent_info->dependents = AVM_ENT_INFO_LIST_NULL;
               m_AVM_LOG_INVALID_VAL_FATAL(0);
               status = NCSCC_RC_FAILURE;
               break;
             }else
             {
                m_MMGR_FREE_AVM_ENT_INFO(temp_ent_info_list->ent_info);
                temp_ent_info_list->ent_info    = ent_info;
                temp_ent_info_list->ep_str      = &ent_info->ep_str;
             }
          }
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avm_decode_cold_sync_rsp_async_updt_cnt
 *
 * Purpose: Decode Async Update Count 
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
avm_decode_cold_sync_rsp_async_updt_cnt(AVM_CB_T           *cb,
                                        NCS_MBCSV_CB_DEC   *dec)
{
   uns32               status   = NCSCC_RC_SUCCESS;
   EDU_ERR             ederror    = 0;
   AVM_ASYNC_CNT_T    *updt_cnt;

   updt_cnt = &cb->async_updt_cnt;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_cold_sync_rsp_async_updt_cnt");

   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_async_updt_cnt, &dec->i_uba, EDP_OP_TYPE_DEC,
                               &updt_cnt, &ederror, dec->i_peer_version);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
   }

   return status;
}


/************************************************************** 
 * Function: avm_decode_data_sync_rsp
 *
 * Purpose:  Decode Data sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
 *************************************************************/
extern uns32
avm_decode_data_sync_rsp(
                           AVM_CB_T          *cb,
                           NCS_MBCSV_CB_DEC  *dec
                        )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 status = NCSCC_RC_SUCCESS;
   uns32 i;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_data_sync_rsp");

   dec->i_action = NCS_MBCSV_ACT_ADD;

   for(i = dec->i_reo_type; i < AVM_CKPT_COLD_SYNC_MSG_MAX; i++)
   {
      status = avm_dec_cold_sync_rsp_data_func_list[i](cb, dec);
      
      if(NCSCC_RC_SUCCESS != status)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(i);
         rc = NCSCC_RC_FAILURE;
      }
   }

   return rc;
}

/************************************************************** 
 * Function: avm_decode_warm_sync_rsp
 *
 * Purpose:  Decode Warm Sync Data sync response message.
 *
 * Input: cb  - CB pointer.
 *        enc - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
 *************************************************************/
extern uns32
avm_decode_warm_sync_rsp(
                           AVM_CB_T          *cb,
                           NCS_MBCSV_CB_DEC  *dec
                        )
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVM_ASYNC_CNT_T *updt_cnt;
   AVM_ASYNC_CNT_T  dec_updt_cnt;
   EDU_ERR ederror = 0;  
   uns8  sprbuf[500]; /* Using hard code value since it doesnot add any value if  defined as a macro  */ 

   m_AVM_LOG_FUNC_ENTRY("avm_decode_warm_sync_rsp");
   
   updt_cnt = &dec_updt_cnt;
   
   /* Decode latest asyn update count */
   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avm_edp_ckpt_msg_async_updt_cnt, &dec->i_uba,
                              EDP_OP_TYPE_DEC, &updt_cnt, &ederror, dec->i_peer_version);
   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
   }
  
   memset(sprbuf, '\0', sizeof(sprbuf));   
   sprintf(sprbuf, " Standby ent_updt = %d ent_cfg_updt = %d adm_op_updt = %d evt_id_updt = %d dhconf_updt = %d dhstate_updt = %d \n Active  ent_updt = %d ent_cfg_updt = %d adm_op_updt = %d evt_id_updt = %d dhconf_updt = %d dhstate_updt = %d upgd_state_updt = %d\n", cb->async_updt_cnt.ent_updt, cb->async_updt_cnt.ent_cfg_updt, cb->async_updt_cnt.ent_adm_op_updt, cb->async_updt_cnt.evt_id_updt, cb->async_updt_cnt.ent_dhconf_updt, cb->async_updt_cnt.ent_dhstate_updt, updt_cnt->ent_updt, updt_cnt->ent_cfg_updt, updt_cnt->ent_adm_op_updt, updt_cnt->evt_id_updt, updt_cnt->ent_dhconf_updt, updt_cnt->ent_dhstate_updt, updt_cnt->ent_upgd_state_updt);
   
   m_AVM_LOG_GEN_EP_STR("Async Update Counts", sprbuf, NCSFL_SEV_INFO);
   
   if(m_NCS_MEMCMP(updt_cnt, &cb->async_updt_cnt, sizeof(AVM_ASYNC_CNT_T)))
   {
      m_AVM_LOG_GEN_EP_STR("Async Update Counts", sprbuf, NCSFL_SEV_ERROR);

      /* Standby not in sync with active. Star syncing up with Active */
      cb->stby_in_sync = FALSE;
      
      /* Cleanup the data at Standby */
      avm_cleanup_db(cb);
      
      /* Send data request , which will sync standby with Active */
      
      if(NCSCC_RC_SUCCESS != avm_send_data_req(cb))
      {
         m_AVM_LOG_INVALID_VAL_FATAL(dec->i_msg_type);
      }
         
      status = NCSCC_RC_FAILURE;
   }

   return status;
}

/****************************************************************************\
 * Function: avm_decode_ckpt_ent_upgd_state_chg
 *
 * Purpose:  Decode upgd state change.
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/

static uns32 
avm_decode_ckpt_ent_upgd_state_chg(AVM_CB_T           *cb,
                          NCS_MBCSV_CB_DEC   *dec)
{
   uns32              status;
   EDU_ERR            ederror    = 0;
   AVM_ENT_INFO_T    *temp_ent_info;
   AVM_ENT_INFO_T     dec_ent;
   AVM_ENT_INFO_T    *ent_info;
   AVM_ENT_DHCP_CONF  *dhcp_conf, *temp_conf;
   uns32              my_ver = AVM_MBCSV_SUB_PART_VERSION;

   m_AVM_LOG_FUNC_ENTRY("avm_decode_ckpt_ent_upgd_state_chg");

   /**************************************************************
      If my version is < 2, I can't understand these fields :) 
      It shouldn't fall into this case 
   **************************************************************/

   if (my_ver < 2)
      return NCSCC_RC_SUCCESS;    

   temp_ent_info = &dec_ent;

   /* decode dhcp state */
   status = ncs_edu_exec(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &dec->i_uba, 
                          EDP_OP_TYPE_DEC, &temp_ent_info, &ederror, 2, 2, 26);

   if(NCSCC_RC_SUCCESS != status)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(cb, &temp_ent_info->entity_path)))
   {
       m_AVM_LOG_INVALID_VAL_ERROR(dec->i_reo_type);
       return NCSCC_RC_FAILURE;   
   }
   dhcp_conf = &ent_info->dhcp_serv_conf;
   temp_conf = &temp_ent_info->dhcp_serv_conf;
   avm_decode_ckpt_upgd_state_update(dhcp_conf, temp_conf);

   cb->async_updt_cnt.ent_upgd_state_updt++;

   return status;
}
