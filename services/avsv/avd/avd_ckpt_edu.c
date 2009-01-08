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

  This file contains utility routines for managing encode/decode operations
  in AvSv checkpoint messages.
..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avd.h"
#include "avsv.h"

/*****************************************************************************

  PROCEDURE NAME:   avd_compile_ckpt_edp

  DESCRIPTION:      This function registers all the EDP's require 
                    for checkpointing.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avd_compile_ckpt_edp(AVD_CL_CB *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   EDU_ERR err = EDU_NORMAL;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_cb, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;
   
   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_avd_hlt, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_avnd_config, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_sg, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_su, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_si, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_si_dep, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_comp, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_csi, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_csi_param_list, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_su_si_rel, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_async_updt_cnt, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_sus_per_si_rank, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_comp_cs_type, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_ckpt_msg_cs_type_param, &err);
   if(rc != NCSCC_RC_SUCCESS)
      goto error;

   return rc;

error:
   m_AVD_LOG_INVALID_VAL_ERROR(err);
   /* EDU cleanup */
   m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
   return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_cb

  DESCRIPTION:      EDU program handler for "AVD_CL_CB" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_CL_CB" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_cb(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_CL_CB *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_cb_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_cb, 0, 0, 0, 
                    sizeof(AVD_CL_CB), 0, NULL},

        /* AVD Control block information */
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_CL_CB*)0)->init_state, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_CL_CB*)0)->cluster_init_time, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAUINT64T, 0, 0, 0, 
            (long)&((AVD_CL_CB*)0)->cluster_view_number, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_CL_CB*)0)->rcv_hb_intvl, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_CL_CB*)0)->snd_hb_intvl, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_CL_CB*)0)->amf_init_intvl, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_CL_CB*)0)->nodes_exit_cnt, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_CL_CB *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_CL_CB **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        /*m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_CL_CB));*/
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_cb_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}



/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_avd_hlt

  DESCRIPTION:      EDU program handler for "AVD_HLT" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_HLT" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_avd_hlt(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_HLT *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_hlt_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_avd_hlt, 0, 0, 0, 
                    sizeof(AVD_HLT), 0, NULL},

         /* Fill here AVD HLT data structure encoding rules */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0,
            (long)&((AVD_HLT*)0)->key_name.comp_name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0,
            (long)&((AVD_HLT *)0)->key_name.key_len_net, 4, NULL},
        {EDU_EXEC, ncs_edp_saamfhealthcheckkeyt, 0, 0, 0,
            (long)&((AVD_HLT*)0)->key_name.name, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_HLT*)0)->period, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_HLT*)0)->max_duration, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_HLT*)0)->row_status, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_HLT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_HLT **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_HLT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_hlt_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_avnd_config

  DESCRIPTION:      EDU program handler for "AVD_AVND" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_AVND" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_avnd_config(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_AVND *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_avnd_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_avnd_config, 0, 0, 0, 
                    sizeof(AVD_AVND), 0, NULL},

         /* Fill here AVD AVND config data structure encoding rules */
        {EDU_EXEC, m_NCS_EDP_SACLMNODEIDT, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->node_info.nodeId, 0, NULL},
        {EDU_EXEC, ncs_edp_saclmnodeaddresst, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->node_info.nodeAddress, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->node_info.nodeName, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->node_info.member, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->node_info.bootTimestamp, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAUINT64T, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->node_info.initialViewNumber, 0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->adest, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->su_admin_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->oper_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->row_status, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->node_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->type, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->su_failover_prob, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->su_failover_max, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->rcv_msg_id, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->snd_msg_id, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_AVND*)0)->avm_oper_state, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_AVND *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_AVND **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_AVND));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_avnd_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_sg

  DESCRIPTION:      EDU program handler for "AVD_SG" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_SG" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_sg(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_SG *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_sg_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_sg, 0, 0, 0, 
                    sizeof(AVD_SG), 0, NULL},

         /* Fill here AVD SG data structure encoding rules */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_SG*)0)->name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_redundancy_model, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_failback, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->pref_num_active_su, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->pre_num_standby_su, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->pref_num_insvc_su, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->pref_num_assigned_su, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_max_active, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_max_standby, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SG*)0)->admin_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SG*)0)->adjust_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->sg_ncs_spec, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SG*)0)->row_status, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_SG*)0)->comp_restart_prob, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->comp_restart_max, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_restart_prob, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_restart_max, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_assigned_num, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_spare_num, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SG*)0)->su_uninst_num, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SG*)0)->sg_fsm_state, 0, NULL},

         /*
          * Rules for Admin SI and SU operation list to be added
          * here.
          */
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_SG *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_SG **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_SG));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_sg_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_su

  DESCRIPTION:      EDU program handler for "AVD_SU" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_SU" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_su(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_SU *struct_ptr = NULL, **d_ptr = NULL;
    uns16 ver_compare = 0;

    ver_compare = AVD_MBCSV_SUB_PART_VERSION;

    EDU_INST_SET avsv_ckpt_msg_su_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_su, 0, 0, 0, 
                    sizeof(AVD_SU), 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_SU*)0)->name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->rank, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->num_of_comp, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->si_max_active, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->si_max_standby, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->si_curr_active, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->si_curr_standby, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->is_su_failover, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SU*)0)->admin_state, 0, NULL},
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, 
            (long)&((AVD_SU*)0)->term_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->su_switch, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SU*)0)->oper_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SU*)0)->pres_state, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0, 
            (long)&((AVD_SU*)0)->sg_name, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SU*)0)->readiness_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SU*)0)->row_status, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SU*)0)->su_act_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SU*)0)->su_preinstan, 0, NULL},

        {EDU_VER_GE, NULL, 0, 0, 2, 0, 0, (EDU_EXEC_RTINE)((uns16 *)(&(ver_compare)))},

        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, 
            (long)&((AVD_SU*)0)->su_is_external, 0, NULL},

         /* Fill here AVD SU data structure encoding rules */
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_SU *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_SU **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_SU));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_su_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_si_dep

  DESCRIPTION:      EDU program handler for "AVD_SI_SI_DEP" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_SI_SI_DEP" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_si_dep(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                               NCSCONTEXT ptr, uns32 *ptr_data_len, 
                               EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                               EDU_ERR *o_err)
{
    uns32         rc = NCSCC_RC_SUCCESS;
    AVD_SI_SI_DEP *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_si_dep_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_si_dep, 0, 0, 0, 
                    sizeof(AVD_SI_SI_DEP), 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_SI_SI_DEP*)0)->indx_mib.si_name_prim, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_SI_SI_DEP*)0)->indx_mib.si_name_sec, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_SI_SI_DEP*)0)->tolerance_time, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SI_SI_DEP*)0)->row_status, 0, NULL},
         /* Fill here AVD SI data structure encoding rules */
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
       struct_ptr = (AVD_SI_SI_DEP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
       d_ptr = (AVD_SI_SI_DEP **)ptr;
       if(*d_ptr == NULL)
       {
          *o_err = EDU_ERR_MEM_FAIL;
          return NCSCC_RC_FAILURE;
       }
       m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_SI_SI_DEP));
       struct_ptr = *d_ptr;
    }
    else
    {
       struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_si_dep_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_si

  DESCRIPTION:      EDU program handler for "AVD_SI" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_SI" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_si(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_SI *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_si_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_si, 0, 0, 0, 
                    sizeof(AVD_SI), 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_SI*)0)->name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SI*)0)->rank, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SI*)0)->su_config_per_si, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SI*)0)->su_curr_active, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SI*)0)->su_curr_standby, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SI*)0)->max_num_csi, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SI*)0)->num_csi, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_SI*)0)->si_switch, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SI*)0)->admin_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_SI*)0)->row_status, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0, 
            (long)&((AVD_SI*)0)->sg_name, 0, NULL},
         /* Fill here AVD SI data structure encoding rules */
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_SI *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_SI **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_SI));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_si_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}

/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_comp

  DESCRIPTION:      EDU program handler for "AVD_COMP" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_COMP" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_comp(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_COMP    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_comp_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_comp, 0, 0, 0, 
                    sizeof(AVD_COMP), 0, NULL},

         /* Fill here AVD COMP data structure encoding rules */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.name_net, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAAMFCOMPONENTCAPABILITYMODELT, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.cap, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.category, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.init_len, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.init_info, AVSV_MISC_STR_MAX_SIZE, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.init_time, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.term_len, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.term_info, AVSV_MISC_STR_MAX_SIZE, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.term_time, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.clean_len, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.clean_info, AVSV_MISC_STR_MAX_SIZE, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.clean_time, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.amstart_len, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.amstart_info, AVSV_MISC_STR_MAX_SIZE, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.amstart_time, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.amstop_len, 0, NULL},
        {EDU_EXEC, ncs_edp_uns8, EDQ_ARRAY, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.amstop_info, AVSV_MISC_STR_MAX_SIZE, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.amstop_time, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.inst_level, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAAMFRECOMMENDEDRECOVERYT, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.def_recvr, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.max_num_inst, 0, NULL},
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.comp_restart, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->proxy_comp_name_net, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->inst_retry_delay, 0, NULL},
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->nodefail_cleanfail, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->max_num_inst_delay, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.max_num_amstart, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.max_num_amstop, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->row_status, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->max_num_csi_actv, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->max_num_csi_stdby, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->curr_num_csi_actv, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->curr_num_csi_stdby, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->oper_state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->pres_state, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->restart_cnt, 0, NULL},
        {EDU_EXEC, ncs_edp_ncs_bool, 0, 0, 0, 
            (long)&((AVD_COMP*)0)->comp_info.am_enable, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
            (long)&((AVD_COMP*)0)->comp_info.terminate_callback_timeout, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
            (long)&((AVD_COMP*)0)->comp_info.csi_set_callback_timeout, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
            (long)&((AVD_COMP*)0)->comp_info.quiescing_complete_timeout, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
            (long)&((AVD_COMP*)0)->comp_info.csi_rmv_callback_timeout, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
            (long)&((AVD_COMP*)0)->comp_info.proxied_inst_callback_timeout, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SATIMET, 0, 0, 0,
            (long)&((AVD_COMP*)0)->comp_info.proxied_clean_callback_timeout, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_COMP *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_COMP **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_COMP));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_comp_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_csi

  DESCRIPTION:      EDU program handler for "AVD_CSI" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_CSI" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_csi(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_CSI    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_csi_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_csi, 0, 0, 0, 
                    sizeof(AVD_CSI), 0, NULL},

         /* Fill here AVD CSI data structure encoding rules */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVD_CSI*)0)->name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0, 
            (long)&((AVD_CSI*)0)->csi_type, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_CSI*)0)->rank, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVD_CSI*)0)->row_status, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVD_CSI*)0)->num_active_params, 0, NULL},
        {EDU_EXEC, avsv_edp_ckpt_msg_csi_param_list, EDQ_POINTER, 0, 0, 
            (long)&((AVD_CSI*)0)->list_param, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_CSI *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_CSI **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_CSI));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_csi_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_csi_param_list

  DESCRIPTION:      EDU program handler for "AVD_CSI_PARAM" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_CSI_PARAM" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_csi_param_list(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_CSI_PARAM    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_csi_param_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_csi_param_list,EDQ_LNKLIST, 0, 0, 
                    sizeof(AVD_CSI_PARAM), 0, NULL},

        /*
         * Fill here AVD CSI PARAM list data structure encoding rules 
         * ACTIVE AVD will send entire list.
         */
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
            (long)&((AVD_CSI_PARAM*)0)->param.name, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
            (long)&((AVD_CSI_PARAM*)0)->param.value, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((AVD_CSI_PARAM*)0)->row_status, 0, NULL},

        {EDU_TEST_LL_PTR, avsv_edp_ckpt_msg_csi_param_list, 0, 0, 0, 
            (long)&((AVD_CSI_PARAM*)0)->param_next, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_CSI_PARAM *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_CSI_PARAM **)ptr;
        if(*d_ptr == NULL)
        {
           *d_ptr = m_MMGR_ALLOC_AVD_CSI_PARAM;

           if (*d_ptr == NULL)
           {
              *o_err = EDU_ERR_MEM_FAIL;
              return NCSCC_RC_FAILURE;
           }
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_CSI_PARAM));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_csi_param_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_su_si_rel

  DESCRIPTION:      EDU program handler for "AVD_SU_SI_REL" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVD_SU_SI_REL" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_su_si_rel(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVSV_SU_SI_REL_CKPT_MSG    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_su_si_rel_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_su_si_rel, 0, 0, 0, 
                    sizeof(AVSV_SU_SI_REL_CKPT_MSG), 0, NULL},

        /*
         * Fill here SU_SI_REL data structure encoding rules 
         */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVSV_SU_SI_REL_CKPT_MSG*)0)->su_name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0, 
            (long)&((AVSV_SU_SI_REL_CKPT_MSG*)0)->si_name_net, 0, NULL},
        {EDU_EXEC, m_NCS_EDP_SAAMFHASTATET, 0, 0, 0, 
            (long)&((AVSV_SU_SI_REL_CKPT_MSG*)0)->state, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0, 
            (long)&((AVSV_SU_SI_REL_CKPT_MSG*)0)->fsm, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVSV_SU_SI_REL_CKPT_MSG *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVSV_SU_SI_REL_CKPT_MSG **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVSV_SU_SI_REL_CKPT_MSG));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_su_si_rel_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_async_updt_cnt

  DESCRIPTION:      EDU program handler for "AVSV_ASYNC_UPDT_CNT" data. This 
                    function is invoked by EDU for performing encode/decode 
                    operation on "AVSV_ASYNC_UPDT_CNT" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 avsv_edp_ckpt_msg_async_updt_cnt(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVSV_ASYNC_UPDT_CNT    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_async_updt_cnt_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_async_updt_cnt, 0, 0, 0, 
                    sizeof(AVSV_ASYNC_UPDT_CNT), 0, NULL},

        /*
         * Fill here Async Update data structure encoding rules 
         */
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->cb_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->avnd_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->sg_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->su_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->si_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->sg_su_oprlist_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->sg_admin_si_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->su_si_rel_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->comp_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->csi_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->csi_parm_list_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, 
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->hlt_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->sus_per_si_rank_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->comp_cs_type_sup_updt, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
            (long)&((AVSV_ASYNC_UPDT_CNT*)0)->cs_type_param_updt, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVSV_ASYNC_UPDT_CNT *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVSV_ASYNC_UPDT_CNT **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVSV_ASYNC_UPDT_CNT));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_async_updt_cnt_rules, struct_ptr, 
            ptr_data_len, buf_env, op, o_err);

    return rc;
}


/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_sus_per_si_rank

  DESCRIPTION:      EDU program handler for "AVD_SUS_PER_SI_RANK" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVD_SUS_PER_SI_RANK" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/

uns32 avsv_edp_ckpt_msg_sus_per_si_rank(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_SUS_PER_SI_RANK    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_sus_per_si_rank_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_sus_per_si_rank, 0, 0, 0,
                    sizeof(AVD_SUS_PER_SI_RANK), 0, NULL},

         /* Fill here AVD SUS PER SI RANK data structure encoding rules */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0,
            (long)&((AVD_SUS_PER_SI_RANK*)0)->indx.si_name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0,
            (long)&((AVD_SUS_PER_SI_RANK*)0)->indx.su_rank_net, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet, 0, 0, 0,
            (long)&((AVD_SUS_PER_SI_RANK*)0)->su_name, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((AVD_SUS_PER_SI_RANK*)0)->row_status, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_SUS_PER_SI_RANK *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_SUS_PER_SI_RANK **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_SUS_PER_SI_RANK));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_sus_per_si_rank_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);

    return rc;

}

/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_comp_cs_type

  DESCRIPTION:      EDU program handler for "AVD_COMP_CS_TYPE" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVD_COMP_CS_TYPE" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/

uns32 avsv_edp_ckpt_msg_comp_cs_type(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_COMP_CS_TYPE    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_comp_cs_type_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_comp_cs_type, 0, 0, 0,
                    sizeof(AVD_COMP_CS_TYPE), 0, NULL},

         /* Fill here AVD COMP CS TYPE data structure encoding rules */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0,
            (long)&((AVD_COMP_CS_TYPE*)0)->indx.comp_name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0,
            (long)&((AVD_COMP_CS_TYPE *)0)->indx.csi_type_name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((AVD_COMP_CS_TYPE*)0)->row_status, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_COMP_CS_TYPE *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_COMP_CS_TYPE **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_COMP_CS_TYPE));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_comp_cs_type_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);

    return rc;

}

/*****************************************************************************

  PROCEDURE NAME:   avsv_edp_ckpt_msg_cs_type_param

  DESCRIPTION:      EDU program handler for "AVD_CS_TYPE_PARAM" data. This
                    function is invoked by EDU for performing encode/decode
                    operation on "AVD_CS_TYPE_PARAM" data.

  RETURNS:          NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/

uns32 avsv_edp_ckpt_msg_cs_type_param(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32               rc = NCSCC_RC_SUCCESS;
    AVD_CS_TYPE_PARAM    *struct_ptr = NULL, **d_ptr = NULL;

    EDU_INST_SET avsv_ckpt_msg_cs_type_param_rules[ ] = {
        {EDU_START, avsv_edp_ckpt_msg_cs_type_param, 0, 0, 0,
                    sizeof(AVD_CS_TYPE_PARAM), 0, NULL},

         /* Fill here AVD CS TYPE PARAM data structure encoding rules */
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0,
            (long)&((AVD_CS_TYPE_PARAM*)0)->indx.type_name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_sanamet_net, 0, 0, 0,
            (long)&((AVD_CS_TYPE_PARAM *)0)->indx.param_name_net, 0, NULL},
        {EDU_EXEC, ncs_edp_int, 0, 0, 0,
            (long)&((AVD_CS_TYPE_PARAM*)0)->row_status, 0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if(op == EDP_OP_TYPE_ENC)
    {
        struct_ptr = (AVD_CS_TYPE_PARAM *)ptr;
    }
    else if(op == EDP_OP_TYPE_DEC)
    {
        d_ptr = (AVD_CS_TYPE_PARAM **)ptr;
        if(*d_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
             return NCSCC_RC_FAILURE;
        }
        m_NCS_MEMSET(*d_ptr, '\0', sizeof(AVD_CS_TYPE_PARAM));
        struct_ptr = *d_ptr;
    }
    else
    {
        struct_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(hdl, edu_tkn, avsv_ckpt_msg_cs_type_param_rules, struct_ptr,
            ptr_data_len, buf_env, op, o_err);

    return rc;

}
