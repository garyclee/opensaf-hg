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

  This module is the include file for Availability Node Directors checkpointing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVND_MBCSV_H
#define AVND_MBCSV_H

#define AVND_MBCSV_SUB_PART_VERSION      1
#define AVND_MBCSV_SUB_PART_VERSION_MIN  1
struct avnd_evt_tag;
struct avnd_cb_tag;

/* This is the case when switch over has happened and timer runing for
   external components on ACT has expired. We need to ignore it.*/
#define m_AVND_CHECK_FOR_STDBY_FOR_EXT_COMP(cb,ext_comp)     \
   (((SA_AMF_HA_STANDBY == (cb)->avail_state_avnd) && \
      (TRUE == (ext_comp))) ? NCSCC_RC_SUCCESS : NCSCC_RC_FAILURE)
/*
 * Async Update message queue.
 */
typedef struct avnd_async_updt_msg_queue {
	struct avnd_async_updt_msg_queue *next;

	NCS_MBCSV_CB_DEC dec;
} AVND_ASYNC_UPDT_MSG_QUEUE;

typedef struct avnd_async_updt_msg_queue_list {
	AVND_ASYNC_UPDT_MSG_QUEUE *async_updt_queue;
	AVND_ASYNC_UPDT_MSG_QUEUE *tail;	/* Tail of the queue */
} AVND_ASYNC_UPDT_MSG_QUEUE_LIST;

/*
 * Async update count. It will be used for warm sync verification.
 */
typedef struct avnd_async_updt_cnt {
	uns32 hlth_config_updt;
	uns32 su_updt;
	uns32 comp_updt;
	uns32 su_si_updt;
	uns32 siq_updt;
	uns32 csi_updt;
	uns32 comp_hlth_rec_updt;
	uns32 comp_cbk_rec_updt;
} AVND_ASYNC_UPDT_CNT;

/*
 * Prototype for the AVSV checkpoint encode function pointer.
 */
typedef uns32 (*AVND_ENCODE_CKPT_DATA_FUNC_PTR) (struct avnd_cb_tag * cb, NCS_MBCSV_CB_ENC *enc);

/*
 * Prototype for the AVSV checkpoint Decode function pointer.
 */
typedef uns32 (*AVND_DECODE_CKPT_DATA_FUNC_PTR) (struct avnd_cb_tag * cb, NCS_MBCSV_CB_DEC *dec);

/*
 * Prototype for the AVSV checkpoint cold sync response encode function pointer.
 */
typedef uns32 (*AVND_ENCODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct avnd_cb_tag * cb,
							  NCS_MBCSV_CB_ENC *enc, uns32 *num_of_obj);

/*
 * Prototype for the AVSV checkpoint cold sync response encode function pointer.
 */
typedef uns32 (*AVND_DECODE_COLD_SYNC_RSP_DATA_FUNC_PTR) (struct avnd_cb_tag * cb,
							  NCS_MBCSV_CB_DEC *enc, uns32 num_of_obj);

uns32 avnd_mbcsv_register(struct avnd_cb_tag *cb);
uns32 avnd_mbcsv_deregister(struct avnd_cb_tag *cb);
uns32 avnd_set_mbcsv_ckpt_role(struct avnd_cb_tag *cb, uns32 role);
uns32 avnd_mbcsv_dispatch(struct avnd_cb_tag *cb, uns32 flag);
uns32 avnd_send_ckpt_data(struct avnd_cb_tag *cb,
				   uns32 action, MBCSV_REO_HDL reo_hdl, uns32 reo_type, uns32 send_type);
uns32 avnd_send_hb_ntfy_msg(struct avnd_cb_tag *cb);
uns32 avnd_mbcsv_obj_set(struct avnd_cb_tag *cb, uns32 obj, uns32 val);
uns32 avnd_send_data_req(struct avnd_cb_tag *cb);
uns32 avnd_dequeue_async_update_msgs(struct avnd_cb_tag *cb, NCS_BOOL pr_or_fr);

uns32 avnd_encode_cold_sync_rsp(struct avnd_cb_tag *cb, NCS_MBCSV_CB_ENC *enc);
uns32 avnd_encode_warm_sync_rsp(struct avnd_cb_tag *cb, NCS_MBCSV_CB_ENC *enc);
uns32 avnd_encode_data_sync_rsp(struct avnd_cb_tag *cb, NCS_MBCSV_CB_ENC *enc);

uns32 avnd_decode_cold_sync_rsp(struct avnd_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);
uns32 avnd_decode_warm_sync_rsp(struct avnd_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);
uns32 avnd_decode_data_sync_rsp(struct avnd_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);
uns32 avnd_decode_data_req(struct avnd_cb_tag *cb, NCS_MBCSV_CB_DEC *dec);

uns32 avnd_mds_mbcsv_reg(struct avnd_cb_tag *cb);
uns32 avnd_ckpt_for_ext(struct avnd_cb_tag *cb, MBCSV_REO_HDL reo_hdl, uns32 reo_type);

#endif
