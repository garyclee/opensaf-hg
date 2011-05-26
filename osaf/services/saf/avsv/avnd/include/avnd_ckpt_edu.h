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
#ifndef AVND_CKPT_EDU_H
#define AVND_CKPT_EDU_H

/* Function Definations of avnd_ckpt_edu.c */
uns32 avnd_edp_ckpt_msg_hlt_config(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					    NCSCONTEXT ptr, uns32 *ptr_data_len,
					    EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_su(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				    NCSCONTEXT ptr, uns32 *ptr_data_len,
				    EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_su_si(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				       NCSCONTEXT ptr, uns32 *ptr_data_len,
				       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_comp(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				      NCSCONTEXT ptr, uns32 *ptr_data_len,
				      EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_csi_rec(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					 NCSCONTEXT ptr, uns32 *ptr_data_len,
					 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_siq(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				     NCSCONTEXT ptr, uns32 *ptr_data_len,
				     EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_comp_hc(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					 NCSCONTEXT ptr, uns32 *ptr_data_len,
					 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_comp_cbk_rec(EDU_HDL *hdl, EDU_TKN *edu_tkn,
					      NCSCONTEXT ptr, uns32 *ptr_data_len,
					      EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uns32 avnd_edp_ckpt_msg_async_updt_cnt(EDU_HDL *hdl, EDU_TKN *edu_tkn,
				       NCSCONTEXT ptr, uns32 *ptr_data_len,
				       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);

#endif
