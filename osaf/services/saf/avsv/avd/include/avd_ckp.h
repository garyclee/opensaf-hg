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
#ifndef AVD_CKP_H
#define AVD_CKP_H

#define AVD_MBCSV_SUB_PART_VERSION_3    3
#define AVD_MBCSV_SUB_PART_VERSION      2
#define AVD_MBCSV_SUB_PART_VERSION_MIN  1

struct avd_evt_tag;
struct cl_cb_tag;

/* 
 * SU SI Relationship checkpoint encode/decode message structure..
 */
typedef struct avsv_su_si_rel_ckpt_msg {
	SaNameT su_name;
	SaNameT si_name;
	SaAmfHAStateT state;
	uns32 fsm;		/* The SU SI FSM state */
        SaBoolT csi_add_rem;
        SaNameT comp_name;
        SaNameT csi_name;
} AVSV_SU_SI_REL_CKPT_MSG;

/*
 * SI transfer fields checkpoint encode/decode message structure
 */
typedef struct avsv_si_trans_ckpt_msg {
	SaNameT sg_name;
	SaNameT si_name;
	SaNameT min_su_name;
	SaNameT max_su_name;
} AVSV_SI_TRANS_CKPT_MSG;

/* 
 * Async Update message queue.
 */
typedef struct avsv_async_updt_msg_queue {
	struct avsv_async_updt_msg_queue *next;

	NCS_MBCSV_CB_DEC dec;
} AVSV_ASYNC_UPDT_MSG_QUEUE;

typedef struct avsv_async_updt_msg_queue_list {
	AVSV_ASYNC_UPDT_MSG_QUEUE *async_updt_queue;
	AVSV_ASYNC_UPDT_MSG_QUEUE *tail;	/* Tail of the queue */
} AVSV_ASYNC_UPDT_MSG_QUEUE_LIST;

/* 
 * Async update count. It will be used for warm sync verification.
 */
typedef struct avsv_async_updt_cnt {
	uns32 cb_updt;
	uns32 node_updt;
	uns32 app_updt;
	uns32 sg_updt;
	uns32 su_updt;
	uns32 si_updt;
	uns32 sg_su_oprlist_updt;
	uns32 sg_admin_si_updt;
	uns32 siass_updt;
	uns32 comp_updt;
	uns32 csi_updt;
	uns32 compcstype_updt;
	uns32 si_trans_updt;
} AVSV_ASYNC_UPDT_CNT;

/*
 * Prototype for the AVSV checkpoint encode function pointer.
 */
typedef uns32 (*AVSV_ENCODE_CKPT_DATA_FUNC_PTR) (struct cl_cb_tag * cb, NCS_MBCSV_CB_ENC *enc);

/*
 * Prototype for the AVSV checkpoint Decode function pointer.
 */
typedef uns32 (*AVSV_DECODE_CKPT_DATA_FUNC_PTR) (struct cl_cb_tag * cb, NCS_MBCSV_CB_DEC *dec);

/*
 * Prototype for the AVSV checkpoint cold sync response encode function pointer.
 */
typedef uns32 (*AVSV_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct cl_cb_tag * cb,
							  NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);

/*
 * Prototype for the AVSV checkpoint cold sync response encode function pointer.
 */
typedef uns32 (*AVSV_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct cl_cb_tag * cb,
							  NCS_MBCSV_CB_DEC *enc, uns32 num_of_obj);

/* Function Definations of avd_chkop.c */
extern uns32 avd_active_role_initialization(struct cl_cb_tag *cb, SaAmfHAStateT role);
EXTERN_C void avd_role_change_evh(struct cl_cb_tag *cb, struct avd_evt_tag *evt);
EXTERN_C uns32 avsv_mbcsv_register(struct cl_cb_tag *cb);
EXTERN_C uns32 avsv_mbcsv_deregister(struct cl_cb_tag *cb);
EXTERN_C uns32 avsv_set_ckpt_role(struct cl_cb_tag *cb, uns32 role);
EXTERN_C uns32 avsv_mbcsv_dispatch(struct cl_cb_tag *cb, uns32 flag);
EXTERN_C uns32 avsv_send_ckpt_data(struct cl_cb_tag *cb,
				   uns32 action, MBCSV_REO_HDL reo_hdl, uns32 reo_type, uns32 send_type);
EXTERN_C uns32 avsv_send_hb_ntfy_msg(struct cl_cb_tag *cb);
EXTERN_C uns32 avsv_mbcsv_obj_set(struct cl_cb_tag *cb, uns32 obj, uns32 val);
EXTERN_C uns32 avsv_send_data_req(struct cl_cb_tag *cb);
EXTERN_C uns32 avsv_dequeue_async_update_msgs(struct cl_cb_tag *cb, NCS_BOOL pr_or_fr);

/* Function Definations of avd_ckpt_enc.c */
EXTERN_C uns32 avsv_encode_cold_sync_rsp(struct cl_cb_tag *cb, NCS_MBCSV_CB_ENC *enc);
EXTERN_C uns32 avsv_encode_warm_sync_rsp(struct cl_cb_tag *cb, NCS_MBCSV_CB_ENC *enc);
EXTERN_C uns32 avsv_encode_data_sync_rsp(struct cl_cb_tag *cb, NCS_MBCSV_CB_ENC *enc);

/* Function Definations of avd_ckpt_dec.c */
EXTERN_C uns32 avsv_decode_cold_sync_rsp(struct cl_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);
EXTERN_C uns32 avsv_decode_warm_sync_rsp(struct cl_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);
EXTERN_C uns32 avsv_decode_data_sync_rsp(struct cl_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);
EXTERN_C uns32 avsv_decode_data_req(struct cl_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);
EXTERN_C uns32 avd_avnd_send_role_change(struct cl_cb_tag *cb, NODE_ID, uns32 role);

#endif
