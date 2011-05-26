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

  This module is the include file for DTS checkpointing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef DTS_CKPT_EDU_H
#define DTS_CKPT_EDU_H

/* Function Definitions of dts_ckpt_edu.c */
uns32 dts_compile_ckpt_edp(DTS_CB *cb);
uns32 dtsv_edp_ckpt_msg_dts_log_ckpt_data_config(EDU_HDL *hdl,
							  EDU_TKN *edu_tkn, NCSCONTEXT ptr, uns32 *ptr_data_len,
							  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 dtsv_edp_ckpt_msg_dts_ll_file_config(EDU_HDL *hdl,
						    EDU_TKN *edu_tkn, NCSCONTEXT ptr, uns32 *ptr_data_len,
						    EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 dtsv_edp_ckpt_msg_dts_file_list_config(EDU_HDL *hdl,
						      EDU_TKN *edu_tkn, NCSCONTEXT ptr, uns32 *ptr_data_len,
						      EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 dtsv_edp_ckpt_msg_dts_svc_reg_tbl_config(EDU_HDL *hdl,
							EDU_TKN *edu_tkn, NCSCONTEXT ptr, uns32 *ptr_data_len,
							EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 dtsv_edp_ckpt_msg_dta_dest_list_config(EDU_HDL *hdl,
						      EDU_TKN *edu_tkn, NCSCONTEXT ptr, uns32 *ptr_data_len,
						      EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 dtsv_edp_ckpt_msg_global_policy_config(EDU_HDL *hdl,
						      EDU_TKN *edu_tkn, NCSCONTEXT ptr, uns32 *ptr_data_len,
						      EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 dtsv_edp_ckpt_msg_async_updt_cnt(EDU_HDL *hdl,
						EDU_TKN *edu_tkn, NCSCONTEXT ptr, uns32 *ptr_data_len,
						EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);

#endif
