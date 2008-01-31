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

  This module is the include file for Availability Directors checkpointing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVM_CKPT_EDU_H
#define AVM_CKPT_EDU_H


/* Function Definitions of avm_ckpt_edu.c */
extern uns32 avm_compile_ckpt_edp(AVM_CB_T *cb);

extern uns32
avm_edp_ckpt_validation_info(EDU_HDL       *hdl,
                               EDU_TKN       *edu_tkn,
                             NCSCONTEXT     ptr,
                             uns32         *ptr_data_len,
                             EDU_BUF_ENV   *buf_env,
                             EDP_OP_TYPE    op,
                             EDU_ERR       *o_err
                           );

extern uns32 avm_edp_ckpt_msg_ent(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);
extern uns32 avm_edp_ckpt_msg_evt(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);

extern uns32 avm_edp_ckpt_msg_evt_id(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);

extern uns32 avm_edp_ckpt_msg_dhcp_conf(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);

extern uns32 avm_edp_ckpt_msg_dhcp_state(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);

extern uns32 avm_edp_ckpt_msg_evt_list(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);

extern uns32 avm_edp_ckpt_msg_async_updt_cnt(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);

extern uns32
avm_edp_sensor_event(EDU_HDL       *hdl,
                     EDU_TKN       *edu_tkn,
                     NCSCONTEXT     ptr,
                     uns32         *ptr_data_len,
                     EDU_BUF_ENV   *buf_env,
                     EDP_OP_TYPE    op,
                     EDU_ERR       *o_err
                    );
extern uns32 
avm_edp_ckpt_msg_validation_info(
                            EDU_HDL       *hdl,
                              EDU_TKN       *edu_tkn,
                                 NCSCONTEXT     ptr,
                                 uns32         *ptr_data_len,
                                 EDU_BUF_ENV   *buf_env,
                                 EDP_OP_TYPE    op,
                                 EDU_ERR       *o_err
                               );

extern uns32 avm_edp_ckpt_msg_hlt_status(EDU_HDL *hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);

extern uns32 avm_edp_ckpt_msg_upgd_state(EDU_HDL *hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

#endif
