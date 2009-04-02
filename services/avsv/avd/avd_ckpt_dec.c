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
  AVD data structures during checkpointing. 

..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

/* Declaration of async update functions */
static uns32  avsv_decode_ckpt_avd_cb_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_avnd_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_sg_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_su_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_si_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_sg_admin_si(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_su_si_rel(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_comp_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_csi_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_hlt_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_oper_su(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_cb_cl_view_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avnd_node_up_info(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avnd_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avnd_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avnd_node_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avnd_rcv_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avnd_snd_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avnd_avm_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_sg_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_sg_adjust_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_sg_su_assigned_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_sg_su_spare_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_sg_su_uninst_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_sg_fsm_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_si_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_si_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_term_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_rediness_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_act_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_su_preinstan(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_si_su_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_si_su_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_si_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_si_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_comp_proxy_comp_name_net(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_comp_curr_num_csi_actv(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_comp_curr_num_csi_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_comp_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_comp_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_comp_restart_count(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_oper_su(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_sus_per_si_rank_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_comp_cs_type_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_ckpt_avd_cs_type_param_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);

/* Declaration of static cold sync decode functions */
static uns32  avsv_decode_cold_sync_rsp_avd_cb_config(AVD_CL_CB *cb, 
                                                      NCS_MBCSV_CB_DEC *dec,
                                                      uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_avnd_config(AVD_CL_CB *cb,
                                                        NCS_MBCSV_CB_DEC *dec,
                                                        uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_sg_config(AVD_CL_CB *cb,
                                                      NCS_MBCSV_CB_DEC *dec,
                                                      uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_su_config(AVD_CL_CB *cb,
                                                      NCS_MBCSV_CB_DEC *dec,
                                                      uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_si_config(AVD_CL_CB *cb,
                                                      NCS_MBCSV_CB_DEC *dec,
                                                      uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_sg_su_oper_list(AVD_CL_CB *cb,
                                                            NCS_MBCSV_CB_DEC *dec, 
                                                            uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_sg_admin_si(AVD_CL_CB *cb,
                                                        NCS_MBCSV_CB_DEC *dec,
                                                        uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_comp_config(AVD_CL_CB *cb,
                                                        NCS_MBCSV_CB_DEC *dec,
                                                        uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_csi_config(AVD_CL_CB *cb,
                                                       NCS_MBCSV_CB_DEC *dec, 
                                                       uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_su_si_rel(AVD_CL_CB *cb,
                                                      NCS_MBCSV_CB_DEC *dec,
                                                      uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_hlt_config(AVD_CL_CB *cb,
                                                       NCS_MBCSV_CB_DEC *dec,
                                                       uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_async_updt_cnt(AVD_CL_CB *cb,
                                                       NCS_MBCSV_CB_DEC *dec,
                                                       uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_sus_per_si_rank_config(AVD_CL_CB *cb,
                                                       NCS_MBCSV_CB_DEC *dec, 
                                                       uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_comp_cs_type_config(AVD_CL_CB *cb,
                                                       NCS_MBCSV_CB_DEC *dec, 
                                                       uns32 num_of_obj);
static uns32  avsv_decode_cold_sync_rsp_avd_cs_type_param_config(AVD_CL_CB *cb,
                                                       NCS_MBCSV_CB_DEC *dec, 
                                                       uns32 num_of_obj);

static uns32  avsv_decode_ckpt_avd_si_dep_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32  avsv_decode_cold_sync_rsp_avd_si_dep_config(AVD_CL_CB *cb,
                                                      NCS_MBCSV_CB_DEC *dec,
                                                      uns32 num_of_obj);

/*
 * Function list for decoding the async data.
 * We will jump into this function using the reo_type received 
 * in the decode argument.
 */

const AVSV_DECODE_CKPT_DATA_FUNC_PTR 
          avsv_dec_ckpt_data_func_list[AVSV_CKPT_MSG_MAX] = 
{
   avsv_decode_ckpt_avd_cb_config,
   avsv_decode_ckpt_avd_avnd_config,
   avsv_decode_ckpt_avd_sg_config,
   avsv_decode_ckpt_avd_su_config,
   avsv_decode_ckpt_avd_si_config,
   avsv_decode_ckpt_avd_oper_su,
   avsv_decode_ckpt_avd_sg_admin_si,
   avsv_decode_ckpt_avd_comp_config,
   avsv_decode_ckpt_avd_cs_type_param_config,
   avsv_decode_ckpt_avd_csi_config,
   avsv_decode_ckpt_avd_comp_cs_type_config,
   avsv_decode_ckpt_avd_su_si_rel,
   avsv_decode_ckpt_avd_hlt_config,
   avsv_decode_ckpt_avd_sus_per_si_rank_config,
   avsv_decode_ckpt_avd_si_dep_config,

   /* 
    * Messages to update independent fields.
    */

   /* CB Async Update messages*/
   avsv_decode_ckpt_cb_cl_view_num,

   /* AVND Async Update messages */
   avsv_decode_ckpt_avnd_node_up_info,
   avsv_decode_ckpt_avnd_su_admin_state,
   avsv_decode_ckpt_avnd_oper_state,
   avsv_decode_ckpt_avnd_node_state,
   avsv_decode_ckpt_avnd_rcv_msg_id,
   avsv_decode_ckpt_avnd_snd_msg_id,
   avsv_decode_ckpt_avnd_avm_oper_state,

   /* SG Async Update messages */
   avsv_decode_ckpt_sg_admin_state,
   avsv_decode_ckpt_sg_adjust_state,
   avsv_decode_ckpt_sg_su_assigned_num,
   avsv_decode_ckpt_sg_su_spare_num,
   avsv_decode_ckpt_sg_su_uninst_num,
   avsv_decode_ckpt_sg_fsm_state,

   /* SU Async Update messages */
   avsv_decode_ckpt_su_si_curr_active,
   avsv_decode_ckpt_su_si_curr_stby,
   avsv_decode_ckpt_su_admin_state,
   avsv_decode_ckpt_su_term_state,
   avsv_decode_ckpt_su_switch,
   avsv_decode_ckpt_su_oper_state,
   avsv_decode_ckpt_su_pres_state,
   avsv_decode_ckpt_su_rediness_state,
   avsv_decode_ckpt_su_act_state,
   avsv_decode_ckpt_su_preinstan,

   /* SI Async Update messages */
   avsv_decode_ckpt_si_su_curr_active,
   avsv_decode_ckpt_si_su_curr_stby,
   avsv_decode_ckpt_si_switch,
   avsv_decode_ckpt_si_admin_state,

   /* COMP Async Update messages */
   avsv_decode_ckpt_comp_proxy_comp_name_net,
   avsv_decode_ckpt_comp_curr_num_csi_actv,
   avsv_decode_ckpt_comp_curr_num_csi_stby,
   avsv_decode_ckpt_comp_oper_state,
   avsv_decode_ckpt_comp_pres_state,
   avsv_decode_ckpt_comp_restart_count
};

/*
 * Function list for decoding the cold sync response data
 * We will jump into this function using the reo_type received 
 * in the cold sync repsonce argument.
 */
const AVSV_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR 
          avsv_dec_cold_sync_rsp_data_func_list[] = 
{
   avsv_decode_cold_sync_rsp_avd_cb_config,
   avsv_decode_cold_sync_rsp_avd_avnd_config,
   avsv_decode_cold_sync_rsp_avd_sg_config,
   avsv_decode_cold_sync_rsp_avd_su_config,
   avsv_decode_cold_sync_rsp_avd_si_config,
   avsv_decode_cold_sync_rsp_avd_sg_su_oper_list,
   avsv_decode_cold_sync_rsp_avd_sg_admin_si,
   avsv_decode_cold_sync_rsp_avd_comp_config,
   avsv_decode_cold_sync_rsp_avd_cs_type_param_config,
   avsv_decode_cold_sync_rsp_avd_csi_config,
   avsv_decode_cold_sync_rsp_avd_comp_cs_type_config,
   avsv_decode_cold_sync_rsp_avd_su_si_rel,
   avsv_decode_cold_sync_rsp_avd_hlt_config,
   avsv_decode_cold_sync_rsp_avd_sus_per_si_rank_config,
   avsv_decode_cold_sync_rsp_avd_si_dep_config,
   avsv_decode_cold_sync_rsp_avd_async_updt_cnt
};


/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_cb_config
 *
 * Purpose:  Decode entire CB data..
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
static uns32  avsv_decode_ckpt_avd_cb_config(AVD_CL_CB *cb_ptr, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_CL_CB       *cb;

   cb = cb_ptr;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_cb_config");
   /* 
    * For updating CB, action is always to do update. We don't have add and remove
    * action on CB. So call EDU to decode CB data.
    */
   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cb, &dec->i_uba, 
                          EDP_OP_TYPE_DEC, (AVD_CL_CB**)&cb, &ederror, dec->i_peer_version);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   /* Since update is successful, update async update count */
   cb->async_updt_cnt.cb_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_avnd_config
 *
 * Purpose:  Decode entire AVD_AVND data..
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
static uns32  avsv_decode_ckpt_avd_avnd_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND  *avnd_ptr;
   AVD_AVND dec_avnd;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_avnd_config");

   avnd_ptr = &dec_avnd;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, &ederror, 1, 1);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_avnd(cb, avnd_ptr, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.avnd_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_sg_config
 *
 * Purpose:  Decode entire AVD_SG data..
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
static uns32  avsv_decode_ckpt_avd_sg_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG *sg_ptr;
   AVD_SG dec_sg;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_sg_config");

   sg_ptr = &dec_sg;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, &ederror, 1, 1);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_sg_data(cb, sg_ptr, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.sg_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_su_config
 *
 * Purpose:  Decode entire AVD_SU data..
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
static uns32  avsv_decode_ckpt_avd_su_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_su_config");

   su_ptr = &dec_su;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, &ederror, 1, 1);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_su_data(cb, su_ptr, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_si_dep_config
 *
 * Purpose:  Decode entire AVD_SI_SI_DEP data..
 *
 * Input: cb - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
static uns32 avsv_decode_ckpt_avd_si_dep_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI_SI_DEP *si_ptr_dec;
   AVD_SI_SI_DEP dec_si_dep;
   EDU_ERR ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_si_dep_config");

   si_ptr_dec = &dec_si_dep;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si_dep,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI_SI_DEP**)&si_ptr_dec, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_si_dep,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI_SI_DEP**)&si_ptr_dec, &ederror, 1, 1);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_si_dep_data(cb, si_ptr_dec, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.si_dep_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_si_config
 *
 * Purpose:  Decode entire AVD_SI data..
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
static uns32  avsv_decode_ckpt_avd_si_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI  *si_ptr_dec;
   AVD_SI  dec_si;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_si_config");

   si_ptr_dec = &dec_si;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI**)&si_ptr_dec, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_si,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI**)&si_ptr_dec, &ederror, 1, 1);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_si_data(cb, si_ptr_dec, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.si_updt++;

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_sg_admin_si
 *
 * Purpose:  Decode entire AVD_SG_ADMIN_SI data..
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
static uns32  avsv_decode_ckpt_avd_sg_admin_si(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_sg_admin_si");

   status = avsv_update_sg_admin_si(cb, &dec->i_uba, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.sg_admin_si_updt++;

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_su_si_rel
 *
 * Purpose:  Decode entire AVD_SU_SI_REL data..
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
static uns32  avsv_decode_ckpt_avd_su_si_rel(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32                    status = NCSCC_RC_SUCCESS;
   AVSV_SU_SI_REL_CKPT_MSG  *su_si_ckpt;
   AVSV_SU_SI_REL_CKPT_MSG  dec_su_si_ckpt;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_su_si_rel");

   su_si_ckpt = &dec_su_si_ckpt;
   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su_si_rel, 
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVSV_SU_SI_REL_CKPT_MSG**)&su_si_ckpt, 
         &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su_si_rel, &dec->i_uba,
         EDP_OP_TYPE_DEC, (AVSV_SU_SI_REL_CKPT_MSG**)&su_si_ckpt, &ederror, 2, 1, 2);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
   }

   avsv_updt_su_si_rel(cb, su_si_ckpt, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.su_si_rel_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_comp_config
 *
 * Purpose:  Decode entire AVD_COMP data..
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
static uns32  avsv_decode_ckpt_avd_comp_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_comp_config");
   comp_ptr = &dec_comp;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, &ederror, 1, 1);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_comp_data(cb, comp_ptr, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.comp_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_csi_config
 *
 * Purpose:  Decode entire AVD_CSI data..
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
static uns32  avsv_decode_ckpt_avd_csi_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_CSI *csi_ptr_dec;
   AVD_CSI dec_csi;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_csi_config");

   csi_ptr_dec = &dec_csi;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_UPDATE:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_csi,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_CSI**)&csi_ptr_dec, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_csi,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_CSI**)&csi_ptr_dec, &ederror, 1, 1);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_csi_data(cb, csi_ptr_dec, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.csi_updt++;

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_hlt_config
 *
 * Purpose:  Decode entire AVD_HLT data..
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
static uns32  avsv_decode_ckpt_avd_hlt_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_HLT *hlt_ptr;
   AVD_HLT dec_hlt;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_hlt_config");

   hlt_ptr = &dec_hlt;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_UPDATE:
   case NCS_MBCSV_ACT_ADD:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avd_hlt,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_HLT**)&hlt_ptr, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avd_hlt,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_HLT**)&hlt_ptr, &ederror, 3, 1, 2, 3);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_hlt_data(cb, hlt_ptr, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.hlt_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_oper_su
 *
 * Purpose:  Decode Operation SU name.
 *
 * Input: cb - CB pointer.
 *        dec - Encode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32  avsv_decode_ckpt_avd_oper_su(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_oper_su");

   su_ptr = &dec_su;

   /* 
    * In case of both Add and remove request send the operation SU name. 
    * We don't have update for this reo_type.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_ADD:
   case NCS_MBCSV_ACT_RMV:
      /* Send entire data */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &dec->i_uba, 
                          EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, &ederror, 1, 1);
      break;

   case NCS_MBCSV_ACT_UPDATE:
   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_decode_add_rmv_su_oper_list(cb, su_ptr, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.sg_su_oprlist_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_cb_cl_view_num
 *
 * Purpose:  Decode cluster view number.
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
static uns32  avsv_decode_ckpt_cb_cl_view_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_cb_cl_view_num");

   /* 
    * Action in this case is just to update. If action passed is add/rmv then log
    * error. Call EDU decode to decode this field.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_cb, &dec->i_uba, 
                          EDP_OP_TYPE_DEC, (AVD_CL_CB**)&cb, &ederror, 1, 3);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   /* If update is successful, update async update count */
   cb->async_updt_cnt.cb_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avnd_node_up_info
 *
 * Purpose:  Decode avnd node up info.
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
static uns32  avsv_decode_ckpt_avnd_node_up_info(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32           status = NCSCC_RC_SUCCESS;
   AVD_AVND        *avnd_ptr;
   AVD_AVND        dec_avnd;
   EDU_ERR         ederror = 0;
   AVD_AVND        *avnd_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avnd_node_up_info");

   avnd_ptr = &dec_avnd;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, 
         &ederror, 6,1,2,4,5,6,7);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (avnd_struct = 
      avd_avnd_struc_find_nodeid(cb, avnd_ptr->node_info.nodeId)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   avnd_struct->node_info.nodeAddress = avnd_ptr->node_info.nodeAddress;
   avnd_struct->node_info.member = avnd_ptr->node_info.member;
   avnd_struct->node_info.bootTimestamp = avnd_ptr->node_info.bootTimestamp;
   avnd_struct->node_info.initialViewNumber = avnd_ptr->node_info.initialViewNumber;
   avnd_struct->adest = avnd_ptr->adest;

   cb->async_updt_cnt.avnd_updt++;
   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avnd_su_admin_state
 *
 * Purpose:  Decode avnd su admin state info.
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
static uns32  avsv_decode_ckpt_avnd_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND *avnd_ptr;
   AVD_AVND dec_avnd;
   EDU_ERR         ederror = 0;
   AVD_AVND        *avnd_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avnd_su_admin_state");

   avnd_ptr = &dec_avnd;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, 
         &ederror, 2, 1, 8);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (avnd_struct = 
      avd_avnd_struc_find_nodeid(cb, avnd_ptr->node_info.nodeId)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   avnd_struct->su_admin_state = avnd_ptr->su_admin_state;

   cb->async_updt_cnt.avnd_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avnd_oper_state
 *
 * Purpose:  Decode avnd oper state info.
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
static uns32  avsv_decode_ckpt_avnd_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND *avnd_ptr;
   AVD_AVND dec_avnd;
   EDU_ERR         ederror = 0;
   AVD_AVND        *avnd_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avnd_oper_state");

   avnd_ptr = &dec_avnd;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, 
         &ederror, 2, 1, 9);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (avnd_struct = 
      avd_avnd_struc_find_nodeid(cb, avnd_ptr->node_info.nodeId)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   avnd_struct->oper_state = avnd_ptr->oper_state;

   cb->async_updt_cnt.avnd_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avnd_node_state
 *
 * Purpose:  Decode avnd node state info.
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
static uns32  avsv_decode_ckpt_avnd_node_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND *avnd_ptr;
   AVD_AVND dec_avnd;
   EDU_ERR         ederror = 0;
   AVD_AVND        *avnd_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avnd_node_state");

   avnd_ptr = &dec_avnd;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, 
         &ederror, 2, 1, 11);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (avnd_struct = 
      avd_avnd_struc_find_nodeid(cb, avnd_ptr->node_info.nodeId)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   avnd_struct->node_state = avnd_ptr->node_state;

   cb->async_updt_cnt.avnd_updt++;
   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avnd_rcv_msg_id
 *
 * Purpose:  Decode avnd receive message ID.
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
static uns32  avsv_decode_ckpt_avnd_rcv_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND *avnd_ptr;
   AVD_AVND dec_avnd;
   EDU_ERR         ederror = 0;
   AVD_AVND        *avnd_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avnd_rcv_msg_id");

   avnd_ptr = &dec_avnd;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, 
         &ederror, 2, 1, 15);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (avnd_struct = 
      avd_avnd_struc_find_nodeid(cb, avnd_ptr->node_info.nodeId)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   avnd_struct->rcv_msg_id = avnd_ptr->rcv_msg_id;

   cb->async_updt_cnt.avnd_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avnd_snd_msg_id
 *
 * Purpose:  Decode avnd Send message ID.
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
static uns32  avsv_decode_ckpt_avnd_snd_msg_id(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND *avnd_ptr;
   AVD_AVND dec_avnd;
   EDU_ERR         ederror = 0;
   AVD_AVND        *avnd_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avnd_snd_msg_id");

   avnd_ptr = &dec_avnd;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, 
         &ederror, 2, 1, 16);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (avnd_struct = 
      avd_avnd_struc_find_nodeid(cb, avnd_ptr->node_info.nodeId)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   avnd_struct->snd_msg_id = avnd_ptr->snd_msg_id;

   cb->async_updt_cnt.avnd_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avnd_avm_oper_state
 *
 * Purpose:  Decode avnd avm oper state info.
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
static uns32  avsv_decode_ckpt_avnd_avm_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_AVND *avnd_ptr;
   AVD_AVND dec_avnd;
   EDU_ERR         ederror = 0;
   AVD_AVND        *avnd_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avnd_avm_oper_state");

   avnd_ptr = &dec_avnd;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, 
         &ederror, 2, 1, 17);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (avnd_struct = 
      avd_avnd_struc_find_nodeid(cb, avnd_ptr->node_info.nodeId)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   avnd_struct->avm_oper_state = avnd_ptr->avm_oper_state;

   cb->async_updt_cnt.avnd_updt++;

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_ckpt_sg_admin_state
 *
 * Purpose:  Decode SG admin state.
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
static uns32  avsv_decode_ckpt_sg_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG *sg_ptr;
   AVD_SG dec_sg;
   EDU_ERR         ederror = 0;
   AVD_SG *sg_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_sg_admin_state");

   sg_ptr = &dec_sg;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, 
         &ederror, 2, 1, 10);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (sg_struct = 
      avd_sg_struc_find(cb, sg_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   sg_struct->admin_state = sg_ptr->admin_state;

   cb->async_updt_cnt.sg_updt++;
   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_sg_adjust_state
 *
 * Purpose:  Decode SG adjust state.
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
static uns32  avsv_decode_ckpt_sg_adjust_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG *sg_ptr;
   AVD_SG dec_sg;
   EDU_ERR         ederror = 0;
   AVD_SG *sg_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_sg_adjust_state");

   sg_ptr = &dec_sg;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, 
         &ederror, 2, 1, 11);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (sg_struct = 
      avd_sg_struc_find(cb, sg_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   sg_struct->adjust_state = sg_ptr->adjust_state;

   cb->async_updt_cnt.sg_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_sg_su_assigned_num
 *
 * Purpose:  Decode SG su assign number.
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
static uns32  avsv_decode_ckpt_sg_su_assigned_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG *sg_ptr;
   AVD_SG dec_sg;
   EDU_ERR         ederror = 0;
   AVD_SG *sg_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_sg_su_assigned_num");

   sg_ptr = &dec_sg;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, 
         &ederror, 2, 1, 18);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (sg_struct = 
      avd_sg_struc_find(cb, sg_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   sg_struct->su_assigned_num = sg_ptr->su_assigned_num;

   cb->async_updt_cnt.sg_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_sg_su_spare_num
 *
 * Purpose:  Decode SG su spare number.
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
static uns32  avsv_decode_ckpt_sg_su_spare_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG *sg_ptr;
   AVD_SG dec_sg;
   EDU_ERR         ederror = 0;
   AVD_SG *sg_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_sg_su_spare_num");

   sg_ptr = &dec_sg;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, 
         &ederror, 2, 1, 19);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (sg_struct = 
      avd_sg_struc_find(cb, sg_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   sg_struct->su_spare_num = sg_ptr->su_spare_num;

   cb->async_updt_cnt.sg_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_sg_su_uninst_num
 *
 * Purpose:  Decode SG su uninst number.
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
static uns32  avsv_decode_ckpt_sg_su_uninst_num(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG *sg_ptr;
   AVD_SG dec_sg;
   EDU_ERR         ederror = 0;
   AVD_SG *sg_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_sg_su_uninst_num");

   sg_ptr = &dec_sg;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, 
         &ederror, 2, 1, 20);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (sg_struct = 
      avd_sg_struc_find(cb, sg_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   sg_struct->su_uninst_num = sg_ptr->su_uninst_num;

   cb->async_updt_cnt.sg_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_sg_fsm_state
 *
 * Purpose:  Decode SG FSM state.
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
static uns32  avsv_decode_ckpt_sg_fsm_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SG *sg_ptr;
   AVD_SG dec_sg;
   EDU_ERR         ederror = 0;
   AVD_SG *sg_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_sg_fsm_state");

   sg_ptr = &dec_sg;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, 
         &ederror, 2, 1, 21);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (sg_struct = 
      avd_sg_struc_find(cb, sg_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   sg_struct->sg_fsm_state = sg_ptr->sg_fsm_state;

   cb->async_updt_cnt.sg_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_si_curr_active
 *
 * Purpose:  Decode SU Current number of Active SI.
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
static uns32  avsv_decode_ckpt_su_si_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_si_curr_active");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 6);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->si_curr_active = su_ptr->si_curr_active;

   cb->async_updt_cnt.su_updt++;
   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_si_curr_stby
 *
 * Purpose:  Decode SU Current number of Standby SI.
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
static uns32  avsv_decode_ckpt_su_si_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_si_curr_stby");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 7);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->si_curr_standby = su_ptr->si_curr_standby;

   cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_admin_state
 *
 * Purpose:  Decode SU Admin state.
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
static uns32  avsv_decode_ckpt_su_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_admin_state");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 9);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->admin_state = su_ptr->admin_state;

   cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_term_state
 *
 * Purpose:  Decode SU Admin state to terminate service.
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
static uns32  avsv_decode_ckpt_su_term_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_term_state");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 10);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->term_state = su_ptr->term_state;

   cb->async_updt_cnt.su_updt++;
   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_switch
 *
 * Purpose:  Decode SU toggle SI.
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
static uns32  avsv_decode_ckpt_su_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_switch");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 11);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->su_switch = su_ptr->su_switch;

   cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_oper_state
 *
 * Purpose:  Decode SU Operation state.
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
static uns32  avsv_decode_ckpt_su_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_oper_state");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 12);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->oper_state = su_ptr->oper_state;

   cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_pres_state
 *
 * Purpose:  Decode SU Presdece state.
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
static uns32  avsv_decode_ckpt_su_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_pres_state");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 13);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->pres_state = su_ptr->pres_state;

   cb->async_updt_cnt.su_updt++;

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_ckpt_su_rediness_state
 *
 * Purpose:  Decode SU Rediness state.
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
static uns32  avsv_decode_ckpt_su_rediness_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_rediness_state");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 15);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->readiness_state = su_ptr->readiness_state;

   cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_act_state
 *
 * Purpose:  Decode SU action state.
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
static uns32  avsv_decode_ckpt_su_act_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_act_state");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 17);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->su_act_state = su_ptr->su_act_state;

   cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_su_preinstan
 *
 * Purpose:  Decode SU preinstatible object.
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
static uns32  avsv_decode_ckpt_su_preinstan(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;
   AVD_SU *su_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_su_preinstan");

   su_ptr = &dec_su;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, 
         &ederror, 2, 1, 18);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (su_struct = 
      avd_su_struc_find(cb, su_ptr->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   su_struct->su_preinstan = su_ptr->su_preinstan;

   cb->async_updt_cnt.su_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_si_su_curr_active
 *
 * Purpose:  Decode number of active SU assignment for this SI.
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
static uns32  avsv_decode_ckpt_si_su_curr_active(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI *si_ptr_dec;
   AVD_SI dec_si;
   EDU_ERR         ederror = 0;
   AVD_SI *si_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_si_su_curr_active");

   si_ptr_dec = &dec_si;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_si,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI**)&si_ptr_dec, 
         &ederror, 2, 1, 4);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (si_struct = 
      avd_si_struc_find(cb, si_ptr_dec->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   si_struct->su_curr_active = si_ptr_dec->su_curr_active;

   cb->async_updt_cnt.si_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_si_su_curr_stby
 *
 * Purpose:  Decode number of standby SU assignment for this SI.
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
static uns32  avsv_decode_ckpt_si_su_curr_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI *si_ptr_dec;
   AVD_SI dec_si;
   EDU_ERR         ederror = 0;
   AVD_SI *si_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_si_su_curr_stby");

   si_ptr_dec = &dec_si;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_si,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI**)&si_ptr_dec, 
         &ederror, 2, 1, 5);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (si_struct = 
      avd_si_struc_find(cb, si_ptr_dec->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   si_struct->su_curr_standby = si_ptr_dec->su_curr_standby;

   cb->async_updt_cnt.si_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_si_switch
 *
 * Purpose:  Decode SI switch.
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
static uns32  avsv_decode_ckpt_si_switch(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI *si_ptr_dec;
   AVD_SI dec_si;
   EDU_ERR         ederror = 0;
   AVD_SI *si_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_si_switch");

   si_ptr_dec = &dec_si;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_si,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI**)&si_ptr_dec, 
         &ederror, 2, 1, 8);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (si_struct = 
      avd_si_struc_find(cb, si_ptr_dec->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   si_struct->si_switch = si_ptr_dec->si_switch;

   cb->async_updt_cnt.si_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_si_admin_state
 *
 * Purpose:  Decode SI admin state.
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
static uns32  avsv_decode_ckpt_si_admin_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_SI *si_ptr_dec;
   AVD_SI dec_si;
   EDU_ERR         ederror = 0;
   AVD_SI *si_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_si_admin_state");

   si_ptr_dec = &dec_si;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_si,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI**)&si_ptr_dec, 
         &ederror, 2, 1, 9);

   if(status != NCSCC_RC_SUCCESS)
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (si_struct = 
      avd_si_struc_find(cb, si_ptr_dec->name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   si_struct->admin_state = si_ptr_dec->admin_state;

   cb->async_updt_cnt.si_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_comp_proxy_comp_name_net
 *
 * Purpose:  Decode COMP proxy compnent name.
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
static uns32  avsv_decode_ckpt_comp_proxy_comp_name_net(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;
   AVD_COMP *comp_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_comp_proxy_comp_name_net");

   comp_ptr = &dec_comp;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, 
         &ederror, 2, 1, 23);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (comp_struct = 
      avd_comp_struc_find(cb, comp_ptr->comp_info.name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   comp_struct->proxy_comp_name_net = comp_ptr->proxy_comp_name_net;

   cb->async_updt_cnt.comp_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_comp_curr_num_csi_actv
 *
 * Purpose:  Decode COMP Current number of CSI active.
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
static uns32  avsv_decode_ckpt_comp_curr_num_csi_actv(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;
   AVD_COMP *comp_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_comp_curr_num_csi_actv");

   comp_ptr = &dec_comp;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, 
         &ederror, 2, 1, 32);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (comp_struct = 
      avd_comp_struc_find(cb, comp_ptr->comp_info.name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   comp_struct->curr_num_csi_actv = 
      comp_ptr->curr_num_csi_actv;

   cb->async_updt_cnt.comp_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_comp_curr_num_csi_stby
 *
 * Purpose:  Decode COMP Current number of CSI standby.
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
static uns32  avsv_decode_ckpt_comp_curr_num_csi_stby(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;
   AVD_COMP *comp_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_comp_curr_num_csi_stby");

   comp_ptr = &dec_comp;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, 
         &ederror, 2, 1, 33);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (comp_struct = 
      avd_comp_struc_find(cb, comp_ptr->comp_info.name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   comp_struct->curr_num_csi_stdby = 
      comp_ptr->curr_num_csi_stdby;

   cb->async_updt_cnt.comp_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_comp_oper_state
 *
 * Purpose:  Decode COMP Operation State.
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
static uns32  avsv_decode_ckpt_comp_oper_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;
   AVD_COMP *comp_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_comp_oper_state");

   comp_ptr = &dec_comp;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, 
         &ederror, 2, 1, 34);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (comp_struct = 
      avd_comp_struc_find(cb, comp_ptr->comp_info.name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   comp_struct->oper_state = comp_ptr->oper_state;

   cb->async_updt_cnt.comp_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_comp_pres_state
 *
 * Purpose:  Decode COMP Presdece State.
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
static uns32  avsv_decode_ckpt_comp_pres_state(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;
   AVD_COMP *comp_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_comp_pres_state");

   comp_ptr = &dec_comp;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, 
         &ederror, 2, 1, 35);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (comp_struct = 
      avd_comp_struc_find(cb, comp_ptr->comp_info.name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   comp_struct->pres_state = comp_ptr->pres_state;

   cb->async_updt_cnt.comp_updt++;

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_ckpt_comp_restart_count
 *
 * Purpose:  Decode COMP Restart count.
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
static uns32  avsv_decode_ckpt_comp_restart_count(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;
   AVD_COMP *comp_struct;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_comp_restart_count");

   comp_ptr = &dec_comp;

   /* 
    * Action in this case is just to update.
    */
   status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, 
         &ederror, 2, 1, 36);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   if (NULL == (comp_struct = 
      avd_comp_struc_find(cb, comp_ptr->comp_info.name_net, FALSE)))
   {
      /* Log Error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   /* Update the fields received in this checkpoint message */
   comp_struct->restart_cnt = comp_ptr->restart_cnt;

   cb->async_updt_cnt.comp_updt++;

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp
 *
 * Purpose:  Decode cold sync response message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_decode_cold_sync_rsp(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32  status = NCSCC_RC_SUCCESS;
   uns32  num_of_obj;
   uns8 *ptr;
   uns8   logbuff[SA_MAX_NAME_LENGTH];

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp");

   /* 
    * Since at decode we need to find out how many objects of particular data
    * type are sent, decode that information at the begining of the message.
    */
   ptr = ncs_dec_flatten_space(&dec->i_uba,
      (uns8*)&num_of_obj, sizeof(uns32));
   num_of_obj = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

   /* 
    * Decode data received in the message.
    */
   dec->i_action = NCS_MBCSV_ACT_ADD;
   memset(logbuff,'\0',SA_MAX_NAME_LENGTH);
   sprintf(logbuff, "\n\nReceived reotype = %d num obj = %d --------------------\n", dec->i_reo_type,num_of_obj);
   m_AVD_LOG_FUNC_ENTRY(logbuff);
   status = 
      avsv_dec_cold_sync_rsp_data_func_list[dec->i_reo_type](cb, dec, num_of_obj);

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_cb_config
 *
 * Purpose:  Decode entire CB data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_cb_config(AVD_CL_CB *cb, 
                     NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_CL_CB       *cb_ptr;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_cb_config");

   cb_ptr = cb;
   /* 
    * Send the CB data.
    */
   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cb, &dec->i_uba, 
                          EDP_OP_TYPE_DEC, (AVD_CL_CB**)&cb_ptr, &ederror, dec->i_peer_version);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_avnd_config
 *
 * Purpose:  Decode entire AVD_AVND data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_avnd_config(AVD_CL_CB *cb,
                 NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32        status = NCSCC_RC_SUCCESS;
   uns32        count = 0;
   EDU_ERR      ederror = 0;
   AVD_AVND     *avnd_ptr;
   AVD_AVND     dec_avnd;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_avnd_config");

   avnd_ptr = &dec_avnd;

   /* 
    * Walk through the entire list and decode the entire AVND data received.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_AVND**)&avnd_ptr, &ederror, dec->i_peer_version);

      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avsv_ckpt_add_rmv_updt_avnd(cb, avnd_ptr, NCS_MBCSV_ACT_ADD);

      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_sg_config
 *
 * Purpose:  Decode entire AVD_SG data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_sg_config(AVD_CL_CB *cb, 
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32        count = 0;
   EDU_ERR         ederror = 0;
   AVD_SG       dec_sg;
   AVD_SG       *sg_ptr;

   sg_ptr = &dec_sg;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_sg_config");

   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sg,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SG**)&sg_ptr, &ederror, dec->i_peer_version);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }
      
      status = avsv_ckpt_add_rmv_updt_sg_data(cb, sg_ptr, dec->i_action);

      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_su_config
 *
 * Purpose:  Decode entire AVD_SU data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_su_config(AVD_CL_CB *cb,
                NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32        count = 0;
   EDU_ERR         ederror = 0;
   AVD_SU       dec_su;
   AVD_SU       *su_ptr;

   su_ptr = &dec_su;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_su_config");

   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, &ederror, dec->i_peer_version);

      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avsv_ckpt_add_rmv_updt_su_data(cb, su_ptr, 
         dec->i_action);

      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_si_dep_config
 *
 * Purpose:  Decode entire AVD_SI data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_si_dep_config(AVD_CL_CB *cb,
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32   count = 0;
   AVD_SI_SI_DEP *si_ptr_dec;
   AVD_SI_SI_DEP dec_si;
   EDU_ERR ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_si_dep_config");

   si_ptr_dec = &dec_si;

   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si_dep,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI_SI_DEP**)&si_ptr_dec, &ederror, dec->i_peer_version);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }
      
      status = avsv_ckpt_add_rmv_updt_si_dep_data(cb, si_ptr_dec, dec->i_action);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_si_config
 *
 * Purpose:  Decode entire AVD_SI data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_si_config(AVD_CL_CB *cb,
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32        count = 0;
   AVD_SI       *si_ptr_dec;
   AVD_SI       dec_si;
   EDU_ERR      ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_si_config");

   si_ptr_dec = &dec_si;

   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_si,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SI**)&si_ptr_dec, &ederror, dec->i_peer_version);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }
      
      status = avsv_ckpt_add_rmv_updt_si_data(cb, si_ptr_dec, dec->i_action);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_sg_su_oper_list
 *
 * Purpose:  Decode entire AVD_SG_SU_OPER_LIST data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_sg_su_oper_list(AVD_CL_CB *cb, 
                          NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32        cs_count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_sg_su_oper_list");

   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (cs_count = 0; cs_count < num_of_obj; cs_count++)
   {
      if (NCSCC_RC_SUCCESS != avsv_decode_oper_su(cb, dec))
         break;
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_sg_admin_si
 *
 * Purpose:  Decode entire AVD_SG_ADMIN_SI data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_sg_admin_si(AVD_CL_CB *cb,
                NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_sg_admin_si");

   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = avsv_update_sg_admin_si(cb, &dec->i_uba, NCS_MBCSV_ACT_ADD);

      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }

   }

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_su_si_rel
 *
 * Purpose:  Decode entire AVD_SU_SI_REL data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_su_si_rel(AVD_CL_CB *cb,
                 NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVSV_SU_SI_REL_CKPT_MSG  *su_si_ckpt;
   AVSV_SU_SI_REL_CKPT_MSG  dec_su_si_ckpt;
   EDU_ERR         ederror = 0;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_su_si_rel");

   su_si_ckpt = &dec_su_si_ckpt;

   /* 
    * Walk through the entire list and send the entire list data.
    * We will walk the SU tree and all the SU_SI relationship for that SU
    * are sent.We will send the corresponding COMP_CSI relationship for that SU_SI
    * in the same update.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_su_si_rel, 
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVSV_SU_SI_REL_CKPT_MSG**)&su_si_ckpt, 
         &ederror, dec->i_peer_version);
      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      }

      status = avsv_updt_su_si_rel(cb, su_si_ckpt, NCS_MBCSV_ACT_ADD);

      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_comp_config
 *
 * Purpose:  Decode entire AVD_COMP data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_comp_config(AVD_CL_CB *cb,
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVD_COMP *comp_ptr;
   AVD_COMP dec_comp;
   EDU_ERR         ederror = 0;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_comp_config");

   comp_ptr = &dec_comp;
   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP**)&comp_ptr, &ederror, dec->i_peer_version);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }
      
      status = avsv_ckpt_add_rmv_updt_comp_data(cb, comp_ptr, 
         NCS_MBCSV_ACT_ADD);

      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }

   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_csi_config
 *
 * Purpose:  Decode entire AVD_CSI data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_csi_config(AVD_CL_CB *cb,
                NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32           status = NCSCC_RC_SUCCESS;
   AVD_CSI *csi_ptr_dec;
   AVD_CSI dec_csi;
   EDU_ERR         ederror = 0;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_csi_config");

   csi_ptr_dec = &dec_csi;
   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_csi,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_CSI**)&csi_ptr_dec, &ederror, dec->i_peer_version);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avsv_ckpt_add_rmv_updt_csi_data(cb, csi_ptr_dec,
         NCS_MBCSV_ACT_ADD);

      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_hlt_config
 *
 * Purpose:  Decode entire AVD_HLT data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_hlt_config(AVD_CL_CB *cb,
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_HLT *hlt_ptr;
   AVD_HLT dec_hlt;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_hlt_config");

   hlt_ptr = &dec_hlt;

   /* 
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_avd_hlt,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_HLT**)&hlt_ptr, &ederror, dec->i_peer_version);
      
      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }
      
      status = avsv_ckpt_add_rmv_updt_hlt_data(cb, hlt_ptr, 
         dec->i_action);
      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }
      
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_async_updt_cnt
 *
 * Purpose:  Send the latest async update count. This count will be used 
 *           during warm sync for verifying the data at stnadby. 
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
static uns32  avsv_decode_cold_sync_rsp_avd_async_updt_cnt(AVD_CL_CB *cb,
                   NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVSV_ASYNC_UPDT_CNT  *updt_cnt;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_async_updt_cnt");

   updt_cnt = &cb->async_updt_cnt;
   /* 
    * Decode and send async update counts for all the data structures.
    */
   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_async_updt_cnt,
      &dec->i_uba, EDP_OP_TYPE_DEC, &updt_cnt, &ederror, dec->i_peer_version);
   
   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_warm_sync_rsp
 *
 * Purpose:  Decode Warm sync response message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_decode_warm_sync_rsp(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVSV_ASYNC_UPDT_CNT  *updt_cnt;
   AVSV_ASYNC_UPDT_CNT   dec_updt_cnt;
   EDU_ERR         ederror = 0;
   uns8   logbuff[SA_MAX_NAME_LENGTH];

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_warm_sync_rsp");

   updt_cnt = &dec_updt_cnt;

   /* 
    * Decode latest async update counts. (In the same manner we received
    * in the last message of the cold sync response.
    */
   status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_async_updt_cnt,
      &dec->i_uba, EDP_OP_TYPE_DEC, &updt_cnt, &ederror, dec->i_peer_version);

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
   }

   memset(logbuff,'\0',SA_MAX_NAME_LENGTH);
   sprintf(logbuff,"\nUPDATE CNTS RCVD(Active -> Standby): cb=%d, avnd=%d, sg=%d, su=%d, si=%d, ol=%d, \n as=%d, rel=%d, co=%d, cs=%d, pl=%d, hlt=%d, sus=%d, ccstype=%d, cspar=%d \n",
           updt_cnt->cb_updt, updt_cnt->avnd_updt, updt_cnt->sg_updt, updt_cnt->su_updt, updt_cnt->si_updt, updt_cnt->sg_su_oprlist_updt,
          updt_cnt->sg_admin_si_updt, updt_cnt->su_si_rel_updt, updt_cnt->comp_updt, updt_cnt->csi_updt, updt_cnt->csi_parm_list_updt,
          updt_cnt->hlt_updt, updt_cnt->sus_per_si_rank_updt, updt_cnt->comp_cs_type_sup_updt, updt_cnt->cs_type_param_updt);
   m_AVD_LOG_FUNC_ENTRY(logbuff);


   memset(logbuff,'\0',SA_MAX_NAME_LENGTH);
   sprintf(logbuff,"\nUPDATE CNTS AT STANDBY: cb=%d, avnd=%d, sg=%d, su=%d, si=%d, ol=%d, \n as=%d, rel=%d, co=%d, cs=%d, pl=%d, hlt=%d sus=%d, ccstype=%d, cspar=%d \n",
           cb->async_updt_cnt.cb_updt, cb->async_updt_cnt.avnd_updt, cb->async_updt_cnt.sg_updt, cb->async_updt_cnt.su_updt,
           cb->async_updt_cnt.si_updt, cb->async_updt_cnt.sg_su_oprlist_updt, cb->async_updt_cnt.sg_admin_si_updt,
           cb->async_updt_cnt.su_si_rel_updt, cb->async_updt_cnt.comp_updt, cb->async_updt_cnt.csi_updt, cb->async_updt_cnt.csi_parm_list_updt,
           cb->async_updt_cnt.hlt_updt, cb->async_updt_cnt.sus_per_si_rank_updt, cb->async_updt_cnt.comp_cs_type_sup_updt,
           cb->async_updt_cnt.cs_type_param_updt);
   m_AVD_LOG_FUNC_ENTRY(logbuff);

   /*
    * Compare the update counts of the Standby with Active.
    * if they matches return success. If it fails then 
    * send a data request.
    */
   if (0 != memcmp(updt_cnt, &cb->async_updt_cnt,
      sizeof(AVSV_ASYNC_UPDT_CNT)))
   {
      /* Log this as a Notify kind of message....this means we 
       * have mismatch in warm sync and we need to resync. 
       * Stanby AVD is unavailable for failure */

      cb->stby_sync_state = AVD_STBY_OUT_OF_SYNC;

      /*
       * Remove All data structures from the standby. We will get them again
       * during our data responce sync-up.
       */
      avd_data_clean_up(cb);

      /*
       * Now send data request, which will sync Standby with Active.
       */
      if (NCSCC_RC_SUCCESS != avsv_send_data_req(cb))
      {
         /* Log error */
         m_AVD_LOG_INVALID_VAL_FATAL(dec->i_msg_type);
      }

      status = NCSCC_RC_FAILURE;
   }

   return status;
}


/****************************************************************************\
 * Function: avsv_decode_data_sync_rsp
 *
 * Purpose:  Decode Data sync response message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_decode_data_sync_rsp(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32  status = NCSCC_RC_SUCCESS;
   uns32  num_of_obj;
   uns8 *ptr;
   uns8   logbuff[SA_MAX_NAME_LENGTH];

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_data_sync_rsp");

   /* 
    * Since at decode we need to find out how many objects of particular data
    * type are sent, decode that information at the begining of the message.
    */
   ptr = ncs_dec_flatten_space(&dec->i_uba,
      (uns8*)&num_of_obj, sizeof(uns32));
   num_of_obj = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

   memset(logbuff,'\0',SA_MAX_NAME_LENGTH);
   sprintf(logbuff,"\n\nReceived reotype  data sync = %d num obj = %d --------------------\n", dec->i_reo_type,num_of_obj);
   m_AVD_LOG_FUNC_ENTRY(logbuff);
   /* 
    * Decode data received in the message.
    */
   dec->i_action = NCS_MBCSV_ACT_ADD;
   status = 
      avsv_dec_cold_sync_rsp_data_func_list[dec->i_reo_type](cb, dec, num_of_obj);

   return status;

}


/****************************************************************************\
 * Function: avsv_decode_data_req
 *
 * Purpose:  Decode Data request message.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32  avsv_decode_data_req(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   m_AVD_LOG_FUNC_ENTRY("avsv_decode_data_req");

   /*
    * Don't decode anything...just return success.
    */
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_decode_oper_su
 *
 * Purpose:  Decode Operation SU's received.
 *
 * Input: cb  - CB pointer.
 *        dec - Decode arguments passed by MBCSV.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32  avsv_decode_oper_su(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32  num_of_oper_su, count;
   uns8 *ptr;
   AVD_SU *su_ptr;
   AVD_SU dec_su;
   EDU_ERR         ederror = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_oper_su");

   ptr = ncs_dec_flatten_space(&dec->i_uba,
      (uns8*)&num_of_oper_su, sizeof(uns32));
   num_of_oper_su = ncs_decode_32bit(&ptr);
   ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

   su_ptr = &dec_su;

   /* 
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then 
    * remove entry from the list.
    */
   for (count = 0; count < num_of_oper_su; count++)
   {
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &dec->i_uba, 
         EDP_OP_TYPE_DEC, (AVD_SU**)&su_ptr, &ederror, 1, 1);

      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avsv_decode_add_rmv_su_oper_list(cb, su_ptr, dec->i_action);
      if( status != NCSCC_RC_SUCCESS )
      {
         m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
         return status;
      }
   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_sus_per_si_rank_config
 *
 * Purpose:  Decode entire AVD_SUS_PER_SI_RANK data..
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
static uns32  avsv_decode_ckpt_avd_sus_per_si_rank_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_SUS_PER_SI_RANK *su_si_rank;
   AVD_SUS_PER_SI_RANK su_si;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_sus_per_si_rank_config");

   su_si_rank = &su_si;

   /*
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_UPDATE:
   case NCS_MBCSV_ACT_ADD:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sus_per_si_rank,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SUS_PER_SI_RANK**)&su_si_rank, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_sus_per_si_rank,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SUS_PER_SI_RANK**)&su_si_rank, &ederror, 2, 1, 2);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_sus_per_si_rank_data(cb, su_si_rank, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.sus_per_si_rank_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_comp_cs_type_config
 *
 * Purpose:  Decode entire AVD_COMP_CS_TYPE data..
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
static uns32  avsv_decode_ckpt_avd_comp_cs_type_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_COMP_CS_TYPE * comp_cs;
   AVD_COMP_CS_TYPE comp_cs_type;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_comp_cs_type_config");

   comp_cs = &comp_cs_type;

   /*
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_UPDATE:
   case NCS_MBCSV_ACT_ADD:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp_cs_type,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP_CS_TYPE**)&comp_cs, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_comp_cs_type,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP_CS_TYPE**)&comp_cs, &ederror, 2, 1, 2);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_comp_cs_type_data(cb, comp_cs, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.comp_cs_type_sup_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_ckpt_avd_cs_type_param_config
 *
 * Purpose:  Decode entire AVD_CS_TYPE_PARAM data..
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
static uns32  avsv_decode_ckpt_avd_cs_type_param_config(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_CS_TYPE_PARAM * cs_param;
   AVD_CS_TYPE_PARAM cs_type_param;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_ckpt_avd_cs_type_param_config");

   cs_param = &cs_type_param;

   /*
    * Check for the action type (whether it is add, rmv or update) and act
    * accordingly. If it is add then create new element, if it is update
    * request then just update data structure, and if it is remove then
    * remove entry from the list.
    */
   switch(dec->i_action)
   {
   case NCS_MBCSV_ACT_UPDATE:
   case NCS_MBCSV_ACT_ADD:
      /* Send entire data */
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cs_type_param,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_CS_TYPE_PARAM**)&cs_param, &ederror, dec->i_peer_version);
      break;

   case NCS_MBCSV_ACT_RMV:
      /* Send only key information */
      status = ncs_edu_exec(&cb->edu_hdl, avsv_edp_ckpt_msg_cs_type_param,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_CS_TYPE_PARAM**)&cs_param, &ederror, 2, 1, 2);
      break;

   default:
      /* Log error */
      m_AVD_LOG_INVALID_VAL_FATAL(dec->i_reo_type);
      return NCSCC_RC_FAILURE;
   }

   if( status != NCSCC_RC_SUCCESS )
   {
      /* Encode failed!!! */
      m_AVD_LOG_INVALID_VAL_FATAL(ederror);
      return status;
   }

   status = avsv_ckpt_add_rmv_updt_cs_type_param_data(cb, cs_param, dec->i_action);

   /* If update is successful, update async update count */
   if (NCSCC_RC_SUCCESS == status)
      cb->async_updt_cnt.cs_type_param_updt++;

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_sus_per_si_rank_config
 *
 * Purpose:  Decode entire AVD_SUS_PER_SI_RANK data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_sus_per_si_rank_config(AVD_CL_CB *cb,
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_SUS_PER_SI_RANK *su_si_ptr;
   AVD_SUS_PER_SI_RANK dec_su_si;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_sus_per_si_rank_config");

   su_si_ptr = &dec_su_si;

   /*
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_sus_per_si_rank,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_SUS_PER_SI_RANK**)&su_si_ptr, &ederror, dec->i_peer_version);

      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avsv_ckpt_add_rmv_updt_sus_per_si_rank_data(cb, su_si_ptr,
         dec->i_action);
      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }

   }

   return status;

}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_comp_cs_type_config
 *
 * Purpose:  Decode entire AVD_COMP_CS_TYPE data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_comp_cs_type_config(AVD_CL_CB *cb,
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_COMP_CS_TYPE *comp_cs_ptr;
   AVD_COMP_CS_TYPE dec_comp_cs;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_comp_cs_type_config");

   comp_cs_ptr = &dec_comp_cs;

   /*
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_comp_cs_type,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_COMP_CS_TYPE**)&comp_cs_ptr, &ederror, dec->i_peer_version);

      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avsv_ckpt_add_rmv_updt_comp_cs_type_data(cb, comp_cs_ptr,
         dec->i_action);
      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }

   }

   return status;
}

/****************************************************************************\
 * Function: avsv_decode_cold_sync_rsp_avd_cs_type_param_config
 *
 * Purpose:  Decode entire AVD_CS_TYPE_PARAM data..
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
static uns32  avsv_decode_cold_sync_rsp_avd_cs_type_param_config(AVD_CL_CB *cb,
                  NCS_MBCSV_CB_DEC *dec, uns32 num_of_obj)
{
   uns32 status = NCSCC_RC_SUCCESS;
   EDU_ERR         ederror = 0;
   AVD_CS_TYPE_PARAM *cs_param_ptr;
   AVD_CS_TYPE_PARAM dec_cs_param;
   uns32        count = 0;

   m_AVD_LOG_FUNC_ENTRY("avsv_decode_cold_sync_rsp_avd_cs_type_param_config");

   cs_param_ptr = &dec_cs_param;

   /*
    * Walk through the entire list and send the entire list data.
    */
   for (count = 0; count < num_of_obj; count++)
   {
      status = m_NCS_EDU_VER_EXEC(&cb->edu_hdl, avsv_edp_ckpt_msg_cs_type_param,
         &dec->i_uba, EDP_OP_TYPE_DEC, (AVD_CS_TYPE_PARAM**)&cs_param_ptr, &ederror, dec->i_peer_version);

      if( status != NCSCC_RC_SUCCESS )
      {
         /* Encode failed!!! */
         m_AVD_LOG_INVALID_VAL_FATAL(ederror);
         return status;
      }

      status = avsv_ckpt_add_rmv_updt_cs_type_param_data(cb, cs_param_ptr,
         dec->i_action);
      if( status != NCSCC_RC_SUCCESS )
      {
         return NCSCC_RC_FAILURE;
      }

   }

   return status;
}

