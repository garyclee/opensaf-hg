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

#ifndef MQND_MIB_H
#define MQND_MIB_H
uns32 samsgqueueentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);

uns32 samsgqueueentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
			     NCSMIB_SETROW_PARAM_VAL *params, struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

uns32 samsgqueueentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);

uns32 samsgqueueentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

uns32 samsgqueueentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
			   NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);

/* samsgqueuepriorityentry_tbl    */

uns32 samsgqueuepriorityentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);

uns32 samsgqueuepriorityentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				     NCSMIB_SETROW_PARAM_VAL *params,
				     struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

uns32 samsgqueuepriorityentry_extract(NCSMIB_PARAM_VAL *param,
				      NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);

uns32 samsgqueuepriorityentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

uns32 samsgqueuepriorityentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg,
				   NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);

#define m_MQSV_SNMPTM_UNS64_TO_PARAM(param,buffer,val64) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = 8; \
   param->info.i_oct = (uns8 *)buffer; \
   m_NCS_OS_HTONLL_P(param->info.i_oct,val64); \
}

uns32 mqsv_reg_mqndmib_queue_priority_tbl_row(MQND_CB *cb, SaNameT mqindex_name, uns32 priority_index, uns32 *row_hdl);

uns32 mqnd_unreg_mib_row(MQND_CB *cb, uns32 tbl_id, uns32 row_hdl);

uns32 mqsv_reg_mqndmib_queue_tbl_row(MQND_CB *cb, SaNameT mqindex_name, uns32 *row_hdl);

uns32 mqnd_reg_with_mab(MQND_CB *cb);
uns32 mqnd_reg_with_miblib(void);
/* To register the 'samsgqueueentry' table information with MibLib */
EXTERN_C uns32 samsgqueueentry_tbl_reg(void);

/* To register the 'samsgqueuepriorityentry' table information with MibLib */
EXTERN_C uns32 samsgqueuepriorityentry_tbl_reg(void);

#endif
