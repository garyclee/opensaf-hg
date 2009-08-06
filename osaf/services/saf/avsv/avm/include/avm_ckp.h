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
#ifndef AVM_CKP_H
#define AVM_CKP_H

#define AVM_MBCSV_SUB_PART_VERSION      2
#define AVM_MBCSV_SUB_PART_VERSION_MIN  1

typedef struct avm_async_updt_cnt {
	uns32 ent_updt;
	uns32 ent_sensor_updt;
} AVM_ASYNC_UPDT_CNT;

typedef uns32 (*AVM_ENCODE_CKPT_DATA_FUNC_PTR) (AVM_CB_T *cb, NCS_MBCSV_CB_ENC *enc);

typedef uns32 (*AVM_DECODE_CKPT_DATA_FUNC_PTR) (AVM_CB_T *cb, NCS_MBCSV_CB_DEC *dec);

typedef uns32 (*AVM_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (AVM_CB_T *cb, NCS_MBCSV_CB_ENC *enc);

typedef uns32 (*AVM_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (AVM_CB_T *cb, NCS_MBCSV_CB_DEC *enc);

extern uns32 avm_load_ckpt_edp(AVM_CB_T *cb);
extern void avm_role_change(AVM_CB_T *cb, AVM_EVT_T *evt);
extern uns32 avm_mbc_register(AVM_CB_T *cb);
extern uns32 avm_mbc_finalize(AVM_CB_T *cb);
extern uns32 avm_set_ckpt_role(AVM_CB_T *cb, uns32 role);
extern uns32 avm_mbc_dispatch(AVM_CB_T *cb, uns32 flag);
extern uns32 avm_mbc_warm_sync_off(AVM_CB_T *cb);
extern uns32 avm_send_ckpt_data(AVM_CB_T *cb, uns32 action, MBCSV_REO_HDL reo_hdl, uns32 reo_type, uns32 send_type);

extern uns32 avm_send_data_req(AVM_CB_T *cb);

extern uns32 avm_encode_cold_sync_rsp(AVM_CB_T *cb, NCS_MBCSV_CB_ENC *enc);
extern uns32 avm_encode_warm_sync_rsp(AVM_CB_T *cb, NCS_MBCSV_CB_ENC *enc);
extern uns32 avm_encode_data_sync_rsp(AVM_CB_T *cb, NCS_MBCSV_CB_ENC *enc);

extern uns32 avm_decode_cold_sync_rsp(AVM_CB_T *cb, NCS_MBCSV_CB_DEC *dec);
extern uns32 avm_decode_warm_sync_rsp(AVM_CB_T *cb, NCS_MBCSV_CB_DEC *dec);
extern uns32 avm_decode_data_sync_rsp(AVM_CB_T *cb, NCS_MBCSV_CB_DEC *dec);
extern uns32 avm_decode_data_req(AVM_CB_T *cb, NCS_MBCSV_CB_DEC *dec);

#endif
