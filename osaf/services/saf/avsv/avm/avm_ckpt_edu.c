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
 
  DESCRIPTION:This module deals with posting of the received event to AvM FSM 
  with necessary Pre - processing done depending on the source of the event.
  ..............................................................................

  Function Included in this Module:

   avm_edp_ckpt_msg_async_updt_cnt
   avm_edp_inv_data_text_buff
   avm_edp_sensor_event
   avm_edp_ckpt_msg_evt_id
   avm_edp_ckpt_msg_dhcp_conf
   avm_edp_ckpt_msg_dhcp_state
   avm_edp_entity_path
   avm_edp_ckpt_msg_async_updt_cnt
   avm_edp_ckpt_msg_upgd_state
******************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "avm.h"

static uns32
avm_edp_inv_data_text_buff(EDU_HDL       *hdl,
                           EDU_TKN       *edu_tkn,
                           NCSCONTEXT     ptr,
                           uns32          *ptr_data_len,
                           EDU_BUF_ENV   *buf_env,
                           EDP_OP_TYPE    op,
                           EDU_ERR       *o_err
                          );

static uns32 
avm_edp_entity_path(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32          *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                   );

static uns32 
avm_edp_ent_desc_name(EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                    );
static uns32 
avm_edp_inv_data(EDU_HDL       *hdl,
                 EDU_TKN       *edu_tkn,
                 NCSCONTEXT     ptr,
                 uns32         *ptr_data_len,
                 EDU_BUF_ENV   *buf_env,
                 EDP_OP_TYPE    op,
                 EDU_ERR       *o_err
                );

static uns32 
avm_edp_ent_valid_location(EDU_HDL       *hdl,
                           EDU_TKN       *edu_tkn,
                           NCSCONTEXT     ptr,
                           uns32         *ptr_data_len,
                           EDU_BUF_ENV   *buf_env,
                           EDP_OP_TYPE    op,
                           EDU_ERR       *o_err
                          );
static uns32 
avm_edp_valid_location_range(EDU_HDL       *hdl,
                             EDU_TKN       *edu_tkn,
                             NCSCONTEXT     ptr,
                             uns32         *ptr_data_len,
                             EDU_BUF_ENV   *buf_env,
                             EDP_OP_TYPE    op,
                             EDU_ERR       *o_err
                            );

static uns32 
avm_edp_ent_info_list(EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                     );
static uns32
avm_edp_entity_path_str(
                         EDU_HDL     *hdl,
                         EDU_TKN     *edu_tkn,
                         NCSCONTEXT   ptr,
                         uns32       *ptr_data_len,
                         EDU_BUF_ENV *buf_env,
                         EDP_OP_TYPE  op,
                         EDU_ERR     *o_err
                       );
static uns32
avm_edp_entity_path(
                      EDU_HDL     *hdl,
                      EDU_TKN     *edu_tkn,
                      NCSCONTEXT   ptr,
                      uns32       *ptr_data_len,
                      EDU_BUF_ENV *buf_env,
                      EDP_OP_TYPE  op,
                      EDU_ERR     *o_err
                   );

static uns32 
avm_edp_ent_dhconf_name_type(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                   );

static uns32 
avm_edp_ent_per_label_conf(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                   );

static uns32 
avm_edp_ent_per_label_status(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                   );

/*************************************************************************
 * Function:  avm_load_ckpt_edp
 *
 * Purpose:   This function registers AvM EDU programs handlers with EDU.
 *
 * Input    :
 *             AVM_CB_T *cb pointer      
 * Output   :
 *
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32 
avm_load_ckpt_edp(AVM_CB_T *cb)
{
   uns32     rc  = NCSCC_RC_SUCCESS;
   EDU_ERR  err = EDU_NORMAL;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ckpt_msg_validation_info, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }


   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ent_dhconf_name_type, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ckpt_msg_dhcp_conf, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ckpt_msg_dhcp_state, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }
   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ckpt_msg_ent, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ckpt_msg_evt_id, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ckpt_msg_async_updt_cnt, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avm_edp_ckpt_msg_upgd_state, &err);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(err);
      m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
      return rc;
   }

   return rc;
}

/*************************************************************************
 * Function:  avm_edp_ckpt_msg_validation_info
 *
 * Purpose:   EDU program handler for AVM_ENT_INFO_T. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32 
avm_edp_ckpt_msg_validation_info(
                            EDU_HDL       *hdl,
                              EDU_TKN       *edu_tkn,
                                 NCSCONTEXT     ptr,
                                 uns32         *ptr_data_len,
                                 EDU_BUF_ENV   *buf_env,
                                 EDP_OP_TYPE    op,
                                 EDU_ERR       *o_err
                               )
{

   uns32             rc = NCSCC_RC_SUCCESS;
   AVM_VALID_INFO_T *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_msg_valid_info_rules[] = {

      {EDU_START, avm_edp_ckpt_msg_validation_info, 0, 0, 0,
           sizeof(AVM_VALID_INFO_T), 0, NULL},

      {EDU_EXEC, avm_edp_ent_desc_name, 0, 0, 0,
         (long)&((AVM_VALID_INFO_NULL)->desc_name), 0 , NULL }, 

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_VALID_INFO_NULL)->entity_type),      0, NULL  },

      {EDU_EXEC, avm_edp_inv_data, 0, 0, 0,
         (long)&((AVM_VALID_INFO_NULL)->inv_data), 0 , NULL }, 

      {EDU_EXEC, ncs_edp_int, 0, 0, 0,
         (long)&((AVM_VALID_INFO_NULL)->is_node),      0, NULL  },

      {EDU_EXEC, ncs_edp_int, 0, 0, 0,
         (long)&((AVM_VALID_INFO_NULL)->is_fru),      0, NULL  },

      {EDU_EXEC, avm_edp_ent_valid_location, EDQ_ARRAY, 0, 0,
         (long)&((AVM_VALID_INFO_NULL)->location),  MAX_POSSIBLE_PARENTS, NULL  },

      {EDU_EXEC, ncs_edp_uns32,   0, 0, 0,
         (long)&((AVM_VALID_INFO_NULL)->parent_cnt), 0, NULL },

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_VALID_INFO_T*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_VALID_INFO_T**)ptr;
         if(*d_ptr == AVM_VALID_INFO_NULL)
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         memset(*d_ptr, '\0', sizeof(AVM_VALID_INFO_T));
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_msg_valid_info_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}

/*************************************************************************
 * Function:  avm_edp_ckpt_msg_ent
 *
 * Purpose:   EDU program handler for AVM_ENT_INFO_T. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32 
avm_edp_ckpt_msg_ent(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT    ptr,
                      uns32        *ptr_data_len,
                      EDU_BUF_ENV  *buf_env,
                      EDP_OP_TYPE   op,
                      EDU_ERR      *o_err
                    )
{

   uns32           rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_T *struct_ptr = NULL, **d_ptr = NULL;
   uns16 base_ver_30a = 0;
   /* Base Version is the version of component which introduced */
   /* the new data struct.                                      */
   /* Hence data fields introduced in MBCSv interface version 2 */
   /* would use below value as base version                     */
   base_ver_30a = 2;


   EDU_INST_SET avm_ckpt_msg_ent_rules[] = {

      {EDU_START, avm_edp_ckpt_msg_ent, 0, 0, 0,
           sizeof(AVM_ENT_INFO_T), 0, NULL},
      
      {EDU_EXEC, ncs_edp_int, 0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->entity_type), 0 , NULL }, 

      {EDU_EXEC, avm_edp_entity_path, 0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->entity_path), 0, NULL  },

      {EDU_EXEC, avm_edp_entity_path_str, 0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->ep_str),  0, NULL},

      {EDU_EXEC, ncs_edp_int, 0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->is_node),     0, NULL  },

      {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->node_name),   0, NULL  },

      {EDU_EXEC, ncs_edp_uns32,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->act_policy),     0, NULL },

      {EDU_EXEC, avm_edp_ent_info_list, EDQ_POINTER, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->dependents),   0, NULL  },

      {EDU_EXEC, avm_edp_entity_path_str, 0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->dep_ep_str),  0, NULL},

      {EDU_EXEC, ncs_edp_int,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->depends_on_valid), 0, NULL },

      {EDU_EXEC, ncs_edp_int,  0,  0, 0,
         (long)&((AVM_ENT_INFO_NULL)->current_state), 0, NULL },

      {EDU_EXEC, ncs_edp_int,  0,  0, 0,
         (long)&((AVM_ENT_INFO_NULL)->previous_state), 0, NULL },

      {EDU_EXEC, ncs_edp_int,  0,  0, 0,
         (long)&((AVM_ENT_INFO_NULL)->previous_diff_state), 0, NULL }, 

      {EDU_EXEC, ncs_edp_uns32,  0,  0, 0,
         (long)&((AVM_ENT_INFO_NULL)->sensor_assert), 0, NULL },

      {EDU_EXEC, ncs_edp_uns32,  0,  0, 0,
         (long)&((AVM_ENT_INFO_NULL)->sensor_count),  0, NULL },

      {EDU_EXEC, avm_edp_sensor_event, EDQ_ARRAY, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->sensors), AVM_MAX_SENSOR_COUNT, NULL },

      {EDU_EXEC, avm_edp_ent_desc_name,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->desc_name),     0, NULL },

      {EDU_EXEC, avm_edp_ent_desc_name,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->parent_desc_name),     0, NULL },

      {EDU_EXEC, ncs_edp_uns32,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->adm_shutdown),     0, NULL },

      {EDU_EXEC, ncs_edp_uns32,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->adm_lock),     0, NULL },

      {EDU_EXEC, ncs_edp_uns32,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->adm_req),     0, NULL },

      {EDU_EXEC, ncs_edp_uns32,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->row_status),    0, NULL },

      {EDU_EXEC, avm_edp_ckpt_msg_dhcp_conf,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->dhcp_serv_conf),    0, NULL },

      {EDU_EXEC, avm_edp_ckpt_msg_dhcp_state,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->dhcp_serv_conf),    0, NULL },

      {EDU_EXEC, ncs_edp_int, 0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->is_fru),     0, NULL  },

      {EDU_VER_GE, NULL,   0, 0, 2, 0, 0, (EDU_EXEC_RTINE)((uns16 *)(&(base_ver_30a)))},

      {EDU_EXEC, avm_edp_ckpt_msg_upgd_state,   0, 0, 0,
         (long)&((AVM_ENT_INFO_NULL)->dhcp_serv_conf),    0, NULL },

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_INFO_T*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_ENT_INFO_T**)ptr;
         if(*d_ptr == AVM_ENT_INFO_NULL)
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         memset(*d_ptr, '\0', sizeof(AVM_ENT_INFO_T));
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_msg_ent_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}


/*************************************************************************
 * Function:  avm_edp_inv_data_text_buff
 *
 * Purpose:   EDU program handler for SaHpiTextBufferT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

static uns32 
avm_edp_inv_data_text_buff(EDU_HDL       *hdl,
                           EDU_TKN       *edu_tkn,
                           NCSCONTEXT     ptr,
                           uns32         *ptr_data_len,
                           EDU_BUF_ENV   *buf_env,
                           EDP_OP_TYPE    op,
                           EDU_ERR       *o_err
                          )    
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SaHpiTextBufferT *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET hpi_text_buff_rules[] = {
         {EDU_START, avm_edp_inv_data_text_buff, 0, 0, 0, sizeof(SaHpiTextBufferT), 0, NULL},
         {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((SaHpiTextBufferT*)0)->DataType, 0 , NULL},
         {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((SaHpiTextBufferT*)0)->Language, 0 , NULL},
         {EDU_EXEC, ncs_edp_uns8,  0, 0, 0,
            (long)&((SaHpiTextBufferT*)0)->DataLength, 0, NULL},
         {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
            (long)&((SaHpiTextBufferT*)0)->Data, SAHPI_MAX_TEXT_BUFFER_LENGTH, NULL},
         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (SaHpiTextBufferT*)ptr;
   }else if(op == EDP_OP_TYPE_DEC)
   {
      d_ptr = (SaHpiTextBufferT **)ptr;
      if(*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }
      memset(*d_ptr, '\0', sizeof(SaHpiTextBufferT));
      struct_ptr = *d_ptr;
   }else
   {
      struct_ptr = ptr;
   }      
      
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, hpi_text_buff_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;   
}


/*************************************************************************
 * Function:  avm_edp_sensor_event
 *
 * Purpose:   EDU program handler for AVM_SENSOR_EVENT_T. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on AVM_SENSOR_EVENT_T 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32
avm_edp_sensor_event(EDU_HDL       *hdl,
                     EDU_TKN       *edu_tkn,
                     NCSCONTEXT     ptr,
                     uns32         *ptr_data_len,
                     EDU_BUF_ENV   *buf_env,
                     EDP_OP_TYPE    op,
                     EDU_ERR       *o_err
                    )    
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_SENSOR_EVENT_T *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET ncs_edp_sensor_event_rules[] = {
         {EDU_START, avm_edp_sensor_event, 0, 0, 0, 
       sizeof(AVM_SENSOR_EVENT_T), 0, NULL},
         {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((AVM_SENSOR_EVENT_T*)0)->SensorNum, 0 , NULL},
         {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((AVM_SENSOR_EVENT_T*)0)->SensorType, 0 , NULL},
         {EDU_EXEC, ncs_edp_int,  0, 0, 0,
            (long)&((AVM_SENSOR_EVENT_T*)0)->EventCategory, 0, NULL},
         {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((AVM_SENSOR_EVENT_T*)0)->sensor_assert, 0, NULL},
         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_SENSOR_EVENT_T*)ptr;
   }else if(op == EDP_OP_TYPE_DEC)
   {
      d_ptr = (AVM_SENSOR_EVENT_T **)ptr;
      if(*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }
      memset(*d_ptr, '\0', sizeof(AVM_SENSOR_EVENT_T));
      struct_ptr = *d_ptr;
   }else
   {
      struct_ptr = ptr;
   }      
      
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, ncs_edp_sensor_event_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;   
}

/*************************************************************************
 * Function:  avm_edp_ckpt_msg_evt_id
 *
 * Purpose:   EDU program handler for event id. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on AVM_EVT_ID_T 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32 
avm_edp_ckpt_msg_evt_id(
                         EDU_HDL       *hdl,
                         EDU_TKN       *edu_tkn,
                         NCSCONTEXT     ptr,
                         uns32         *ptr_data_len,
                         EDU_BUF_ENV   *buf_env,
                         EDP_OP_TYPE    op,
                         EDU_ERR       *o_err
                       )
{

   uns32            rc = NCSCC_RC_SUCCESS;
   AVM_EVT_ID_T    *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_msg_evt_id_rules[] = {
      {EDU_START, avm_edp_ckpt_msg_evt_id, 0, 0, 0,
           sizeof(AVM_EVT_ID_T), 0, NULL},
      
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_EVT_ID_T*)0)->evt_id, 0 , NULL }, 
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_EVT_ID_T*)0)->src, 0, NULL  },

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_EVT_ID_T*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_EVT_ID_T**)ptr;
         if(*d_ptr == AVM_EVT_ID_NULL)
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         memset(*d_ptr, '\0', sizeof(AVM_EVT_ID_T));
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_msg_evt_id_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}


/*************************************************************************
 * Function:  avm_edp_entity
 *
 * Purpose:   EDU program handler for SaHpiEntityPathT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on SaHpiEntityPathT 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

static uns32 
avm_edp_entity(
                 EDU_HDL       *hdl,
                 EDU_TKN       *edu_tkn,
                 NCSCONTEXT     ptr,
                 uns32         *ptr_data_len,
                 EDU_BUF_ENV   *buf_env,
                 EDP_OP_TYPE    op,
                 EDU_ERR       *o_err
              )
{

   uns32          rc  = NCSCC_RC_SUCCESS;
   SaHpiEntityT    *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_entity_rules[] = {
      {EDU_START, avm_edp_entity, 0, 0, 0,
           sizeof(SaHpiEntityT), 0, NULL},
      
      {EDU_EXEC, ncs_edp_int, 0, 0, 0,
         (long)&((SaHpiEntityT*)0)->EntityType, 0 , NULL }, 
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
#ifdef HAVE_HPI_A01
         (long)&((SaHpiEntityT*)0)->EntityInstance, 0, NULL  },
#else
         (long)&((SaHpiEntityT*)0)->EntityLocation, 0, NULL  },
#endif

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (SaHpiEntityT*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (SaHpiEntityT**)ptr;
         if(*d_ptr == ((SaHpiEntityT*)0x0))
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         memset(*d_ptr, '\0', sizeof(SaHpiEntityT));
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_entity_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}

/*************************************************************************
 * Function:  avm_edp_ckpt_msg_async_updt_cnt
 *
 * Purpose:   EDU program handler fro AVM_ASYNC_UPDT_CNT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on AVM_ASYNC_UPDT
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 
extern uns32
avm_edp_ckpt_msg_async_updt_cnt(
                                 EDU_HDL       *hdl,
                                 EDU_TKN       *edu_tkn,
                                 NCSCONTEXT     ptr,
                                 uns32         *ptr_data_len,
                                 EDU_BUF_ENV   *buf_env,
                                 EDP_OP_TYPE    op,
                                 EDU_ERR       *o_err
                               )
{

   uns32               rc  = NCSCC_RC_SUCCESS;
   AVM_ASYNC_CNT_T    *struct_ptr = NULL, **d_ptr = NULL;
   uns16 base_ver_30a = 0;
   /* Base Version is the version of component which introduced */
   /* the new data struct.                                      */
   /* Hence data fields introduced in MBCSv interface version 2 */
   /* would use below value as base version                     */
   base_ver_30a = 2;

   EDU_INST_SET avm_ckpt_async_updt_cnt_rules[] = {
      {EDU_START, avm_edp_ckpt_msg_async_updt_cnt, 0, 0, 0,
           sizeof(AVM_ASYNC_CNT_T), 0, NULL},
      
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->ent_updt, 0 , NULL }, 

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->ent_sensor_updt, 0, NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
           (long)&((AVM_ASYNC_CNT_T*)0)->ent_dep_updt, 0, NULL},

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
           (long)&((AVM_ASYNC_CNT_T*)0)->ent_adm_op_updt, 0, NULL},

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->evt_id_updt, 0, NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->ent_cfg_updt, 0 , NULL }, 

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->ent_dhconf_updt, 0, NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->ent_dhstate_updt, 0, NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->dummy_updt, 0, NULL  },

      {EDU_VER_GE, NULL,   0, 0, 2, 0, 0, (EDU_EXEC_RTINE)((uns16 *)(&(base_ver_30a)))},

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ASYNC_CNT_T*)0)->ent_upgd_state_updt, 0, NULL  },

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ASYNC_CNT_T*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_ASYNC_CNT_T**)ptr;
         if(*d_ptr == ((AVM_ASYNC_CNT_T*)0x0))
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         memset(*d_ptr, '\0', sizeof(AVM_ASYNC_CNT_T));
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_async_updt_cnt_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;
}

/*************************************************************************
 * Function:  avm_edp_inv_data
 *
 * Purpose:   EDU program handler for SaHpiTextBufferT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

static uns32 
avm_edp_inv_data(EDU_HDL       *hdl,
                 EDU_TKN       *edu_tkn,
                 NCSCONTEXT     ptr,
                 uns32         *ptr_data_len,
                 EDU_BUF_ENV   *buf_env,
                 EDP_OP_TYPE    op,
                 EDU_ERR       *o_err
                )    
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_INV_DATA_T *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_edp_inv_data_rules[] = {
         {EDU_START, avm_edp_inv_data, 0, 0, 0, sizeof(AVM_INV_DATA_T), 0, NULL},

         {EDU_EXEC, avm_edp_inv_data_text_buff, 0, 0, 0,
            (long)&((AVM_INV_DATA_T*)0)->product_name, 0 , NULL},

         {EDU_EXEC, avm_edp_inv_data_text_buff, 0, 0, 0,
            (long)&((AVM_INV_DATA_T*)0)->product_version, 0 , NULL},

         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_INV_DATA_T*)ptr;
   }else if(op == EDP_OP_TYPE_DEC)
   {
      d_ptr = (AVM_INV_DATA_T **)ptr;
      if(*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }
      memset(*d_ptr, '\0', sizeof(AVM_INV_DATA_T));
      struct_ptr = *d_ptr;
   }else
   {
      struct_ptr = ptr;
   }      
      
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_edp_inv_data_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;   
}

 /*************************************************************************
 * Function:  avm_edp_ent_desc_name
 *
 * Purpose:   EDU program handler for SaHpiTextBufferT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

static uns32 
avm_edp_ent_desc_name(EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                    )    
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_DESC_NAME_T *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_edp_ent_desc_name_rules[] = {
         {EDU_START, avm_edp_ent_desc_name, 0, 0, 0, 
              sizeof(AVM_ENT_DESC_NAME_T), 0, NULL},

         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
            (long)&((AVM_ENT_DESC_NAME_T*)0)->length, 0 , NULL},

         {EDU_EXEC,ncs_edp_uns8, EDQ_ARRAY, 0, 0,
            (long)&((AVM_ENT_DESC_NAME_T*)0)->name, NCS_MAX_INDEX_LEN, NULL},

         {EDU_END, 0, 0, 0, 0, 0, 0, NULL}
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_DESC_NAME_T*)ptr;
   }else if(op == EDP_OP_TYPE_DEC)
   {
      d_ptr = (AVM_ENT_DESC_NAME_T **)ptr;
      if(*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }
      memset(*d_ptr, '\0', sizeof(AVM_ENT_DESC_NAME_T));
      struct_ptr = *d_ptr;
   }else
   {
      struct_ptr = ptr;
   }      
      
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_edp_ent_desc_name_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;   
}

/*************************************************************************
 * Function:  avm_edp_ent_valid_location
 *
 * Purpose:   EDU program handler for SaHpiTextBufferT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 
static uns32 
avm_edp_ent_valid_location(EDU_HDL       *hdl,
                           EDU_TKN       *edu_tkn,
                           NCSCONTEXT     ptr,
                           uns32         *ptr_data_len,
                           EDU_BUF_ENV   *buf_env,
                           EDP_OP_TYPE    op,
                           EDU_ERR       *o_err
                          )    
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_DESC_NAME_T *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_edp_ent_valid_location_rules[] = {
         {EDU_START, avm_edp_ent_valid_location, 0, 0, 0, sizeof(AVM_ENT_VALID_LOCATION_T), 0, NULL},

         {EDU_EXEC, avm_edp_ent_desc_name, 0, 0, 0,
            (long)&((AVM_ENT_VALID_LOCATION_T*)0)->parent, 0 , NULL},

         {EDU_EXEC, avm_edp_valid_location_range, EDQ_ARRAY, 0, 0,
            (long)&((AVM_ENT_VALID_LOCATION_T*)0)->range, AVM_MAX_LOCATION_RANGE, NULL},

         {EDU_EXEC,ncs_edp_uns32, 0, 0, 0,
            (long)&((AVM_ENT_VALID_LOCATION_T*)0)->count, 0 , NULL},

         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_DESC_NAME_T*)ptr;
   }else if(op == EDP_OP_TYPE_DEC)
   {
      d_ptr = (AVM_ENT_DESC_NAME_T **)ptr;
      if(*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }
      memset(*d_ptr, '\0', sizeof(AVM_ENT_VALID_LOCATION_T));
      struct_ptr = *d_ptr;
   }else
   {
      struct_ptr = ptr;
   }      
      
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_edp_ent_valid_location_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;   
}

 /*************************************************************************
 * Function:  avm_edp_valid_location_range
 *
 * Purpose:   EDU program handler for SaHpiTextBufferT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 
static uns32 
avm_edp_valid_location_range(EDU_HDL       *hdl,
                             EDU_TKN       *edu_tkn,
                             NCSCONTEXT     ptr,
                             uns32         *ptr_data_len,
                             EDU_BUF_ENV   *buf_env,
                             EDP_OP_TYPE    op,
                             EDU_ERR       *o_err
                            )    
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_VALID_LOC_RANGE_T *struct_ptr = NULL;
   AVM_VALID_LOC_RANGE_T **d_ptr = NULL;

   EDU_INST_SET avm_edp_valid_location_range_rules[] = {
         {EDU_START, avm_edp_valid_location_range, 0, 0, 0, sizeof(AVM_VALID_LOC_RANGE_T), 0, NULL},

         {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
            (long)&((AVM_VALID_LOC_RANGE_T*)0)->min, 0 , NULL},

         {EDU_EXEC,ncs_edp_uns32, 0, 0, 0,
            (long)&((AVM_VALID_LOC_RANGE_T*)0)->max, 0, NULL},

         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_VALID_LOC_RANGE_T*)ptr;
   }else if(op == EDP_OP_TYPE_DEC)
   {
      d_ptr = (AVM_VALID_LOC_RANGE_T **)ptr;
      if(*d_ptr == NULL)
      {
         *o_err = EDU_ERR_MEM_FAIL;
         return NCSCC_RC_FAILURE;
      }
      memset(*d_ptr, '\0', sizeof(AVM_VALID_LOC_RANGE_T));
      struct_ptr = *d_ptr;
   }else
   {
      struct_ptr = ptr;
   }      
      
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_edp_valid_location_range_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;   
}

/***********************************************************************
 * Function:  avm_edp_ent_info_list
 *
 * Purpose:   EDU program handler for SaHpiTextBufferT. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
***********************************************************************/ 
static uns32 
avm_edp_ent_info_list(EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                     )    
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T *struct_ptr = NULL;
   AVM_ENT_INFO_LIST_T **d_ptr = NULL;
   AVM_ENT_INFO_T      *temp_ent_info;   

   EDU_INST_SET avm_edp_ent_info_list_rules[] = {
         {EDU_START, avm_edp_ent_info_list, EDQ_LNKLIST, 0, 0, sizeof(AVM_ENT_INFO_LIST_T), 0, NULL},

         {EDU_EXEC, avm_edp_entity_path_str, EDQ_POINTER, 0, 0,
            (long)&((AVM_ENT_INFO_LIST_T*)0)->ep_str, 0 , NULL},

         {EDU_TEST_LL_PTR, avm_edp_ent_info_list, 0, 0, 0,
            (long)&((AVM_ENT_INFO_LIST_T*)0)->next, 0, NULL},

         {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_INFO_LIST_T*)ptr;
   }else if(op == EDP_OP_TYPE_DEC)
   {
      temp_ent_info = m_MMGR_ALLOC_AVM_ENT_INFO;   
      d_ptr = (AVM_ENT_INFO_LIST_T **)ptr;
      if(*d_ptr == NULL)
      {
         *d_ptr = m_MMGR_ALLOC_AVM_ENT_INFO_LIST;
         if(*d_ptr == NULL)
          {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
          }
      }   
      memset(*d_ptr, '\0', sizeof(AVM_ENT_INFO_LIST_T));
      memset(temp_ent_info, '\0', sizeof(AVM_ENT_INFO_T));
      (*d_ptr)->ent_info    = temp_ent_info;
      (*d_ptr)->ep_str = &temp_ent_info->ep_str;
      struct_ptr = *d_ptr;
   }else
   {
      struct_ptr = ptr;
   }      
      
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_edp_ent_info_list_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;   
}


static uns32
avm_edp_entity_path_str(
                         EDU_HDL     *hdl,
                         EDU_TKN     *edu_tkn,
                         NCSCONTEXT   ptr,
                         uns32       *ptr_data_len,
                         EDU_BUF_ENV *buf_env,
                         EDP_OP_TYPE  op,
                         EDU_ERR     *o_err
                       )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_PATH_STR_T *struct_ptr = NULL, **d_ptr = NULL;
   
   EDU_INST_SET avm_ckpt_entity_path_str_rules[] = {

       {EDU_START, avm_edp_entity_path_str, 0, 0, 0,
            sizeof(AVM_ENT_PATH_STR_T), 0, NULL},

       {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
            (long)&((AVM_ENT_PATH_STR_T*)0)->length, 0, NULL},

       {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
            (long)&((AVM_ENT_PATH_STR_T*)0)->name, AVM_MAX_INDEX_LEN, NULL },

       {EDU_END, 0, 0, 0, 0, 0, 0, NULL }
   };
   
   if (op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_PATH_STR_T*)ptr;
   }else
     if (op == EDP_OP_TYPE_DEC)
     {
        d_ptr = (AVM_ENT_PATH_STR_T**)ptr;
        if(*d_ptr == (AVM_ENT_PATH_STR_T*)0x0)
        {
           *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*d_ptr, '\0', sizeof(AVM_ENT_PATH_STR_T));
        struct_ptr = *d_ptr;
     }else
     {
        struct_ptr = ptr;
     }    
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_entity_path_str_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;
}         

static uns32
avm_edp_entity_path(
                      EDU_HDL     *hdl,
                      EDU_TKN     *edu_tkn,
                      NCSCONTEXT   ptr,
                      uns32       *ptr_data_len,
                      EDU_BUF_ENV *buf_env,
                      EDP_OP_TYPE  op,
                      EDU_ERR     *o_err
                   )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SaHpiEntityPathT *struct_ptr = NULL, **d_ptr = NULL;
   
   EDU_INST_SET avm_ckpt_entity_path_rules[] = {

       {EDU_START, avm_edp_entity_path, 0, 0, 0,
            sizeof(AVM_ENT_PATH_STR_T), 0, NULL},

       {EDU_EXEC, avm_edp_entity, EDQ_ARRAY, 0, 0,
            (long)&((SaHpiEntityPathT*)0)->Entry, SAHPI_MAX_ENTITY_PATH, NULL },

       {EDU_END, 0, 0, 0, 0, 0, 0, NULL }
   };
   
   if (op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (SaHpiEntityPathT*)ptr;
   }else
     if (op == EDP_OP_TYPE_DEC)
     {
        d_ptr = (SaHpiEntityPathT**)ptr;
        if(*d_ptr == (SaHpiEntityPathT*)0x0)
        {
           *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*d_ptr, '\0', sizeof(SaHpiEntityPathT));
        struct_ptr = *d_ptr;
     }else
     {
        struct_ptr = ptr;
     }    
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_entity_path_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);
   return rc;
}    


/*************************************************************************
 * Function:  avm_edp_ckpt_msg_dhcp_conf
 *
 * Purpose:   EDU program handler for AVM_ENT_DHCP_CONF. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32 
avm_edp_ckpt_msg_dhcp_conf(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT    ptr,
                      uns32        *ptr_data_len,
                      EDU_BUF_ENV  *buf_env,
                      EDP_OP_TYPE   op,
                      EDU_ERR      *o_err
                    )
{
   uns32           rc = NCSCC_RC_SUCCESS;
   AVM_ENT_DHCP_CONF *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_msg_dhcp_conf_rules[] = {

      {EDU_START, avm_edp_ckpt_msg_dhcp_conf, 0, 0, 0,
           sizeof(AVM_ENT_DHCP_CONF), 0, NULL},
      
      {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->mac_address), (AVM_MAX_MAC_ENTRIES * AVM_MAC_DATA_LEN), NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->net_boot),     0, NULL  },

      {EDU_EXEC, ncs_edp_uns8,   EDQ_ARRAY, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->tftp_serve_ip),     4, NULL },

      {EDU_EXEC, avm_edp_ent_dhconf_name_type, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->pref_label),   0, NULL  },

      {EDU_EXEC, avm_edp_ent_per_label_conf, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->label1),   0, NULL  },

      {EDU_EXEC, avm_edp_ent_per_label_conf, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->label2),   0, NULL  },

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_DHCP_CONF*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_ENT_DHCP_CONF**)ptr;
         if(*d_ptr == AVM_ENT_DHCP_CONF_NULL)
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_msg_dhcp_conf_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}

/*************************************************************************
 * Function:  avm_edp_ckpt_msg_dhcp_state
 *
 * Purpose:   EDU program handler for AVM_ENT_DHCP_STATE. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32 
avm_edp_ckpt_msg_dhcp_state(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT    ptr,
                      uns32        *ptr_data_len,
                      EDU_BUF_ENV  *buf_env,
                      EDP_OP_TYPE   op,
                      EDU_ERR      *o_err
                    )
{
   uns32           rc = NCSCC_RC_SUCCESS;
   AVM_ENT_DHCP_CONF *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_msg_dhcp_state_rules[] = {

      {EDU_START, avm_edp_ckpt_msg_dhcp_state, 0, 0, 0,
           sizeof(AVM_ENT_DHCP_CONF), 0, NULL},
      
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->def_label_num),     0, NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->default_chg),   0, NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->upgd_prgs),   0, NULL  },

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->cur_act_label_num),     0, NULL  },

      {EDU_EXEC, avm_edp_ent_per_label_status, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->label1),   0, NULL  },

      {EDU_EXEC, avm_edp_ent_per_label_status, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->label2),   0, NULL  },

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_DHCP_CONF*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_ENT_DHCP_CONF**)ptr;
         if(*d_ptr == AVM_ENT_DHCP_CONF_NULL)
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_msg_dhcp_state_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}


/*************************************************************************
 * Function:  avm_edp_ent_dhconf_name_type
 *
 * Purpose:   EDU program handler for AVM_DHCP_CONF_NAME_TYPE. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on SaHpiEntityPathT 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

static uns32 
avm_edp_ent_dhconf_name_type(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                   )
{

   uns32          rc  = NCSCC_RC_SUCCESS;
   AVM_DHCP_CONF_NAME_TYPE    *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_entity_rules[] = {
      {EDU_START, avm_edp_ent_dhconf_name_type, 0, 0, 0,
           sizeof(AVM_DHCP_CONF_NAME_TYPE), 0, NULL},
      
      {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
         (long)&((AVM_DHCP_CONF_NAME_TYPE*)0)->name, AVM_NAME_STR_LENGTH , NULL }, 

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_DHCP_CONF_NAME_TYPE*)0)->length, 0, NULL  },

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_DHCP_CONF_NAME_TYPE*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_DHCP_CONF_NAME_TYPE**)ptr;
         if(*d_ptr == ((AVM_DHCP_CONF_NAME_TYPE*)0x0))
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         memset(*d_ptr, '\0', sizeof(AVM_DHCP_CONF_NAME_TYPE));
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_entity_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}

/*************************************************************************
 * Function:  avm_edp_ent_per_label_conf
 *
 * Purpose:   EDU program handler for AVM_PER_LABEL_CONF. This is invoked 
 *            when EDU has to perfomr ENCODE and DECODE on SaHpiEntityPathT 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

static uns32 
avm_edp_ent_per_label_conf(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                   )
{

   uns32          rc  = NCSCC_RC_SUCCESS;
   AVM_PER_LABEL_CONF    *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_entity_rules[] = {
      {EDU_START, avm_edp_ent_per_label_conf, 0, 0, 0,
           sizeof(AVM_PER_LABEL_CONF), 0, NULL},
      
      {EDU_EXEC, avm_edp_ent_dhconf_name_type, 0, 0, 0,
         (long)&((AVM_PER_LABEL_CONF*)0)->name, 0 , NULL }, 

      {EDU_EXEC, avm_edp_ent_dhconf_name_type, 0, 0, 0,
         (long)&((AVM_PER_LABEL_CONF*)0)->file_name, 0 , NULL }, 

      {EDU_EXEC, avm_edp_ent_dhconf_name_type, 0, 0, 0,
         (long)&((AVM_PER_LABEL_CONF*)0)->sw_version, 0 , NULL }, 

      {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
         (long)&((AVM_PER_LABEL_CONF*)0)->install_time, AVM_TIME_STR_LEN , NULL }, 

      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_PER_LABEL_CONF*)0)->conf_chg, 0 , NULL }, 

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_PER_LABEL_CONF*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_PER_LABEL_CONF**)ptr;
         if(*d_ptr == ((AVM_PER_LABEL_CONF*)0x0))
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         memset(*d_ptr, '\0', sizeof(AVM_PER_LABEL_CONF));
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_entity_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}

/*************************************************************************
 * Function:  avm_edp_ent_per_label_status
 *
 * Purpose:   EDU program handler for AVM_PER_LABEL_CONF status. This is 
 *            invoked when EDU has to perfomr ENCODE and DECODE on 
 *            SaHpiEntityPathT 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

static uns32 
avm_edp_ent_per_label_status(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT     ptr,
                      uns32         *ptr_data_len,
                      EDU_BUF_ENV   *buf_env,
                      EDP_OP_TYPE    op,
                      EDU_ERR       *o_err
                   )
{

   uns32          rc  = NCSCC_RC_SUCCESS;
   AVM_PER_LABEL_CONF    *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_entity_rules[] = {
      {EDU_START, avm_edp_ent_per_label_status, 0, 0, 0,
           sizeof(AVM_PER_LABEL_CONF), 0, NULL},
      
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_PER_LABEL_CONF*)0)->status, 0 , NULL }, 

      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_PER_LABEL_CONF*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_PER_LABEL_CONF**)ptr;
         if(*d_ptr == ((AVM_PER_LABEL_CONF*)0x0))
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_entity_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}

/*************************************************************************
 * Function:  avm_edp_ckpt_msg_dummy
 *
 * Purpose:   EDU program handler for HEALTH_STATUS. This is invoked 
 *            when EDU has to perform ENCODE and DECODE on HEALTH_STATUS.
 * 
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/
extern uns32
avm_edp_ckpt_msg_dummy(
                                 EDU_HDL       *hdl,
                                 EDU_TKN       *edu_tkn,
                                 NCSCONTEXT     ptr,
                                 uns32         *ptr_data_len,
                                 EDU_BUF_ENV   *buf_env,
                                 EDP_OP_TYPE    op,
                                 EDU_ERR       *o_err
                               )
{

   uns32               rc  = NCSCC_RC_SUCCESS;
   AVM_CB_T    *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_hlt_status_rules[] = {
      {EDU_START, avm_edp_ckpt_msg_dummy, 0, 0, 0,
           sizeof(AVM_CB_T), 0, NULL},
      
      {EDU_EXEC, ncs_edp_uns32,   EDQ_ARRAY, 0, 0,
         (long)((AVM_CB_T *)0)->dummy_status,     4, NULL },
      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };

   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_CB_T*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_CB_T**)ptr;
         if(*d_ptr == (AVM_CB_T*)0x0)
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_hlt_status_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;
}

/*************************************************************************
 * Function:  avm_edp_ckpt_msg_upgd_state 
 *
 * Purpose:   EDU program handler for AVM_ENT_DHCP_STATE. This is invoked 
 *            when EDU has to perform ENCODE and DECODE on it 
 *
 * Input    :
 *           EDU_HDL    
 *           EDU_TKN   
 *           NCSCONTEXT 
 *           uns32     
 *           EDU_BUF_ENV  
 *           EDP_OP_TYPE 
 *           EDU_ERR    
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 * NOTES    :
 *
 ************************************************************************/ 

extern uns32 
avm_edp_ckpt_msg_upgd_state(
                      EDU_HDL       *hdl,
                      EDU_TKN       *edu_tkn,
                      NCSCONTEXT    ptr,
                      uns32        *ptr_data_len,
                      EDU_BUF_ENV  *buf_env,
                      EDP_OP_TYPE   op,
                      EDU_ERR      *o_err
                    )
{
   uns32           rc = NCSCC_RC_SUCCESS;
   AVM_ENT_DHCP_CONF *struct_ptr = NULL, **d_ptr = NULL;

   EDU_INST_SET avm_ckpt_msg_upgd_state_rules[] = {

      {EDU_START, avm_edp_ckpt_msg_upgd_state, 0, 0, 0,
           sizeof(AVM_ENT_DHCP_CONF), 0, NULL},
      {EDU_EXEC, avm_edp_ent_dhconf_name_type, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->ipmc_helper_node), 0, NULL},
      {EDU_EXEC, avm_edp_entity_path_str, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->ipmc_helper_ent_path), 0, NULL},
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->upgrade_type), 0, NULL},
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->ipmb_addr), 0, NULL},
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->ipmc_upgd_state), 0, NULL},
      {EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->pld_bld_ipmc_status), 0, NULL},
      {EDU_EXEC, ncs_edp_uns8, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->pld_rtm_ipmc_status), 0, NULL},
      {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
         (long)&((AVM_ENT_DHCP_CONF_NULL)->bios_upgd_state), 0, NULL},
      {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
   };


   if(op == EDP_OP_TYPE_ENC)
   {
      struct_ptr = (AVM_ENT_DHCP_CONF*)ptr;
   }else
      if(op == EDP_OP_TYPE_DEC)
      {
         d_ptr = (AVM_ENT_DHCP_CONF**)ptr;
         if(*d_ptr == AVM_ENT_DHCP_CONF_NULL)
         {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
         }
         struct_ptr = *d_ptr;
      }else
      {
         struct_ptr = ptr;
      }
   rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avm_ckpt_msg_upgd_state_rules, struct_ptr, ptr_data_len, buf_env, op, o_err);

   return rc;

}
