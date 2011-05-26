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
#ifndef DTS_CKPT_UPDT_H
#define DTS_CKPT_UPDT_H

/* Function Definations of avd_ckpt_updt.c */
uns32 dtsv_ckpt_add_rmv_updt_dta_dest(DTS_CB *cb,
					       DTA_DEST_LIST *dtadest, NCS_MBCSV_ACT_TYPE action, SVC_KEY key);
uns32 dtsv_ckpt_add_rmv_updt_svc_reg(DTS_CB *cb,
					      DTS_SVC_REG_TBL *svcreg,
					      NCS_MBCSV_ACT_TYPE action);
uns32 dtsv_ckpt_add_rmv_updt_global_policy(DTS_CB *cb,
						    GLOBAL_POLICY *gp, DTS_FILE_LIST *file_list,
						    NCS_MBCSV_ACT_TYPE action);
uns32 dtsv_ckpt_add_rmv_updt_dts_log(DTS_CB *cb, DTS_LOG_CKPT_DATA *data, NCS_MBCSV_ACT_TYPE action);
uns32 dts_data_clean_up(DTS_CB *cb);

/* Macro to set dts_svc_reg_tbl attributes */
#define m_DTS_SET_SVC_REG_TBL(svc, param) \
{  \
   svc->per_node_logging = param->per_node_logging; \
   svc->svc_policy.enable = param->svc_policy.enable; \
   svc->svc_policy.category_bit_map = param->svc_policy.category_bit_map; \
   svc->svc_policy.severity_bit_map = param->svc_policy.severity_bit_map; \
   svc->svc_policy.log_dev = param->svc_policy.log_dev; \
   svc->svc_policy.log_file_size = param->svc_policy.log_file_size; \
   svc->svc_policy.file_log_fmt = param->svc_policy.file_log_fmt; \
   svc->svc_policy.cir_buff_size = param->svc_policy.cir_buff_size; \
   svc->svc_policy.buff_log_fmt = param->svc_policy.buff_log_fmt; \
   svc->device.last_rec_id = param->device.last_rec_id; \
   svc->device.file_log_fmt_change = param->device.file_log_fmt_change; \
   svc->device.buff_log_fmt_change = param->device.buff_log_fmt_change; \
}

#endif
