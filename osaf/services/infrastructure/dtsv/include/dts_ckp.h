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
#ifndef DTS_CKP_H
#define DTS_CKP_H

/* 
 * Async update count. It will be used for warm sync verification.
 */
/*typedef struct dtsv_async_updt_cnt
{
   uns32     dts_svc_reg_tbl_updt;
   uns32     dta_dest_list_updt;
   uns32     global_policy_updt;
}DTSV_ASYNC_UPDT_CNT;*/

/*
 * Bit map of data request.
 */
/*Smik - Not sure if we need this */
/*typedef struct avd_red_data_req_bit_map
{
   uns32     cb_updt:1;
   uns32     avnd_updt:1;
   uns32     sg_updt:1;
   uns32     su_updt:1;
   uns32     si_updt:1;
   uns32     sg_su_oprlist_updt:1;
   uns32     sg_admin_si_updt:1;
   uns32     su_si_rel_updt:1;
   uns32     comp_updt:1;
   uns32     csi_updt:1;
   uns32     hlt_updt:1;
}AVD_RED_DATA_REQ_BIT_MAP;*/

/*
 * Data response context.
 */
/*typedef struct dts_data_rsp_context
{

}DTS_DATA_RSP_CONTEXT;*/

/* 
 * Data structure for log file name checkpointing incase of Async updates 
 */
typedef struct dts_log_ckpt_data {
	char file_name[250];
	SVC_KEY key;
	uint8_t new_file;
} DTS_LOG_CKPT_DATA;
/*
 * Prototype for the DTSV checkpoint encode function pointer.
 */
typedef uns32 (*DTSV_ENCODE_CKPT_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_ENC *enc);

/*
 * Prototype for the DTSV checkpoint Decode function pointer.
 */
typedef uns32 (*DTSV_DECODE_CKPT_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_DEC *dec);

/*
 * Prototype for the DTSV checkpoint cold sync response encode function pointer.
 */
typedef uns32 (*DTSV_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);

/*
 * Prototype for the DTSV checkpoint cold sync response encode function pointer.
 */
typedef uns32 (*DTSV_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_DEC *enc, uns32 num_of_obj);

/* Function Definitions of dts_chkop.c */
uns32 dts_role_change(struct dts_cb *cb, SaAmfHAStateT haState);
uns32 dtsv_mbcsv_register(struct dts_cb *cb);
uns32 dtsv_mbcsv_deregister(struct dts_cb *cb);
uns32 dtsv_set_ckpt_role(struct dts_cb *cb, uns32 role);
uns32 dtsv_mbcsv_dispatch(struct dts_cb *cb, uns32 flag);
uns32 dtsv_send_ckpt_data(struct dts_cb *cb,
				   uns32 action, MBCSV_REO_HDL reo_hdl, uns32 reo_type, uns32 send_type);
uns32 dtsv_mbcsv_obj_set(struct dts_cb *cb, uns32 obj, uns32 val);
uns32 dtsv_send_data_req(struct dts_cb *cb);

/* Function Definitions of dts_ckpt_enc.c */
uns32 dtsv_encode_cold_sync_rsp(struct dts_cb *cb, NCS_MBCSV_CB_ENC *enc);
uns32 dtsv_encode_warm_sync_rsp(struct dts_cb *cb, NCS_MBCSV_CB_ENC *enc);
uns32 dtsv_encode_data_sync_rsp(struct dts_cb *cb, NCS_MBCSV_CB_ENC *enc);
uns32 dtsv_encode_all(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, NCS_BOOL csync);

/* Function Definitions of dts_ckpt_dec.c */
uns32 dtsv_decode_cold_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
uns32 dtsv_decode_warm_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
uns32 dtsv_decode_data_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
uns32 dtsv_decode_data_req(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);

#endif
