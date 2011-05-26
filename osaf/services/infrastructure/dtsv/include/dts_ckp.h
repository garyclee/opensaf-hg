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
   uint32_t     dts_svc_reg_tbl_updt;
   uint32_t     dta_dest_list_updt;
   uint32_t     global_policy_updt;
}DTSV_ASYNC_UPDT_CNT;*/

/*
 * Bit map of data request.
 */
/*Smik - Not sure if we need this */
/*typedef struct avd_red_data_req_bit_map
{
   uint32_t     cb_updt:1;
   uint32_t     avnd_updt:1;
   uint32_t     sg_updt:1;
   uint32_t     su_updt:1;
   uint32_t     si_updt:1;
   uint32_t     sg_su_oprlist_updt:1;
   uint32_t     sg_admin_si_updt:1;
   uint32_t     su_si_rel_updt:1;
   uint32_t     comp_updt:1;
   uint32_t     csi_updt:1;
   uint32_t     hlt_updt:1;
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
typedef uint32_t (*DTSV_ENCODE_CKPT_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_ENC *enc);

/*
 * Prototype for the DTSV checkpoint Decode function pointer.
 */
typedef uint32_t (*DTSV_DECODE_CKPT_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_DEC *dec);

/*
 * Prototype for the DTSV checkpoint cold sync response encode function pointer.
 */
typedef uint32_t (*DTSV_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_ENC *enc, uint32_t *num_of_obj);

/*
 * Prototype for the DTSV checkpoint cold sync response encode function pointer.
 */
typedef uint32_t (*DTSV_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct dts_cb * cb, NCS_MBCSV_CB_DEC *enc, uint32_t num_of_obj);

/* Function Definitions of dts_chkop.c */
uint32_t dts_role_change(struct dts_cb *cb, SaAmfHAStateT haState);
uint32_t dtsv_mbcsv_register(struct dts_cb *cb);
uint32_t dtsv_mbcsv_deregister(struct dts_cb *cb);
uint32_t dtsv_set_ckpt_role(struct dts_cb *cb, uint32_t role);
uint32_t dtsv_mbcsv_dispatch(struct dts_cb *cb, uint32_t flag);
uint32_t dtsv_send_ckpt_data(struct dts_cb *cb,
				   uint32_t action, MBCSV_REO_HDL reo_hdl, uint32_t reo_type, uint32_t send_type);
uint32_t dtsv_mbcsv_obj_set(struct dts_cb *cb, uint32_t obj, uint32_t val);
uint32_t dtsv_send_data_req(struct dts_cb *cb);

/* Function Definitions of dts_ckpt_enc.c */
uint32_t dtsv_encode_cold_sync_rsp(struct dts_cb *cb, NCS_MBCSV_CB_ENC *enc);
uint32_t dtsv_encode_warm_sync_rsp(struct dts_cb *cb, NCS_MBCSV_CB_ENC *enc);
uint32_t dtsv_encode_data_sync_rsp(struct dts_cb *cb, NCS_MBCSV_CB_ENC *enc);
uint32_t dtsv_encode_all(DTS_CB *cb, NCS_MBCSV_CB_ENC *enc, NCS_BOOL csync);

/* Function Definitions of dts_ckpt_dec.c */
uint32_t dtsv_decode_cold_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
uint32_t dtsv_decode_warm_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
uint32_t dtsv_decode_data_sync_rsp(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);
uint32_t dtsv_decode_data_req(DTS_CB *cb, NCS_MBCSV_CB_DEC *dec);

#endif
