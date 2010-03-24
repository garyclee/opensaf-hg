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

  DESCRIPTION: This module does the initialisation of MBCSV and provides
  callback functions at Availability Directors. It contains both the
  callbacks to encode the structures at active and decode along with
  building the structures at standby availability directors. All this
  functionality happens in the context of the MBCSV.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#include <logtrace.h>
#include "avd.h"

static uns32 avsv_mbcsv_cb(NCS_MBCSV_CB_ARG *arg);
static uns32 avsv_mbcsv_process_enc_cb(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32 avsv_mbcsv_process_dec_cb(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32 avsv_mbcsv_process_peer_info_cb(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32 avsv_mbcsv_process_notify(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32 avsv_mbcsv_process_err_ind(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg);
static uns32 avsv_enqueue_async_update_msgs(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec);
static uns32 avsv_mbcsv_initialize(AVD_CL_CB *cb);
static uns32 avsv_mbcsv_open_ckpt(AVD_CL_CB *cb);
static uns32 avsv_get_mbcsv_sel_obj(AVD_CL_CB *cb);
static uns32 avsv_mbcsv_close_ckpt(AVD_CL_CB *cb);
static uns32 avsv_mbcsv_finalize(AVD_CL_CB *cb);
static uns32 avsv_validate_reo_type_in_csync(AVD_CL_CB *cb, uns32 reo_type);

extern const AVSV_ENCODE_CKPT_DATA_FUNC_PTR avsv_enc_ckpt_data_func_list[AVSV_CKPT_MSG_MAX];

extern const AVSV_DECODE_CKPT_DATA_FUNC_PTR avsv_dec_ckpt_data_func_list[AVSV_CKPT_MSG_MAX];

/****************************************************************************\
 * Function: avsv_mbcsv_register
 *
 * Purpose:  AVSV function to register AVD with MBCSv.
 *
 * Input: cb - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_mbcsv_register(AVD_CL_CB *cb)
{
	uns32 status = NCSCC_RC_SUCCESS;

	TRACE_ENTER();

	/*
	 * Send Async Update count to zero.
	 */
	memset(&cb->async_updt_cnt, 0, sizeof(AVSV_ASYNC_UPDT_CNT));

	/*
	 * Compile Ckpt EDP's
	 */
	if (NCSCC_RC_SUCCESS != (status = avd_compile_ckpt_edp(cb))) {
		/* Log error */ ;
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		goto done;
	}

	/*
	 * First initialize and then call open.
	 */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_initialize(cb))) {
		/* Log error */ ;
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		goto done;
	}

	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_open_ckpt(cb))) {
		/* Log error */ ;
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		goto done_final;
	}

	/*
	 * Get MBCSv selection object.
	 */
	if (NCSCC_RC_SUCCESS != (status = avsv_get_mbcsv_sel_obj(cb))) {
		/* Log error */ ;
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		goto done_close;
	}

	return status;

 done_close:
	avsv_mbcsv_close_ckpt(cb);
 done_final:
	avsv_mbcsv_finalize(cb);
 done:
	return status;
}

/****************************************************************************\
 * Function: avsv_mbcsv_deregister
 *
 * Purpose:  AVSV function to deregister AVD with MBCSv.
 *
 * Input: cb - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_mbcsv_deregister(AVD_CL_CB *cb)
{
	uns32 status = NCSCC_RC_SUCCESS;

	TRACE_ENTER();

	/*
	 * First close and then finalize.
	 */
	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_close_ckpt(cb))) {
		/* Log error */ ;
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		goto done;
	}

	if (NCSCC_RC_SUCCESS != (status = avsv_mbcsv_finalize(cb))) {
		/* Log error */ ;
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		goto done;
	}

 done:
	return status;
}

/****************************************************************************\
 * Function: avsv_mbcsv_callback
 *
 * Purpose:  AVSV MBCSV call back function.
 *
 * Input: arg - MBCSV callback argument pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_cb(NCS_MBCSV_CB_ARG *arg)
{
	uns32 status;

	switch (arg->i_op) {
	case NCS_MBCSV_CBOP_ENC:
		status = avsv_mbcsv_process_enc_cb(avd_cb, arg);
		break;

	case NCS_MBCSV_CBOP_DEC:
		status = avsv_mbcsv_process_dec_cb(avd_cb, arg);
		break;

	case NCS_MBCSV_CBOP_PEER:
		status = avsv_mbcsv_process_peer_info_cb(avd_cb, arg);
		break;

	case NCS_MBCSV_CBOP_NOTIFY:
		status = avsv_mbcsv_process_notify(avd_cb, arg);
		break;

	case NCS_MBCSV_CBOP_ERR_IND:
		status = avsv_mbcsv_process_err_ind(avd_cb, arg);
		break;

	default:
		LOG_ER("avsv_mbcsv_cb: invalid op %u", arg->i_op);
		status = NCSCC_RC_FAILURE;
		break;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_mbcsv_process_enc_cb
 *
 * Purpose:  AVSV MBCSV encode call back function.
 *
 * Input: arg - MBCSV callback argument pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_process_enc_cb(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
	uns32 status = NCSCC_RC_SUCCESS;

	TRACE_ENTER();

	switch (arg->info.encode.io_msg_type) {
	case NCS_MBCSV_MSG_ASYNC_UPDATE:
		{
			/*
			 * If we are sending sync commit message then there is nothing to 
			 * encode wo return.
			 */
			if (AVSV_SYNC_COMMIT == arg->info.encode.io_reo_type)
				return status;

			/* Encode Async update message */
			if (arg->info.encode.io_reo_type >= AVSV_CKPT_MSG_MAX) {
				m_AVD_LOG_INVALID_VAL_FATAL(arg->info.encode.io_reo_type);
				return NCSCC_RC_FAILURE;
			}

			status = avsv_enc_ckpt_data_func_list[arg->info.encode.io_reo_type] (cb, &arg->info.encode);

			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_ASYNC_UPDATE, NCSFL_SEV_INFO, arg->info.encode.io_reo_type);
		}
		break;

	case NCS_MBCSV_MSG_COLD_SYNC_REQ:
		{
			/* Encode Cold Sync Request message 
			 * Nothing is there to send at this point of time.
			 * But we can set cold_sync_in_progress which indicates that 
			 * cold sync in on.
			 */
			cb->stby_sync_state = AVD_STBY_OUT_OF_SYNC;
			m_AVD_LOG_CKPT_EVT(AVD_COLD_SYNC_REQ_RCVD, NCSFL_SEV_INFO, 1);
		}
		break;

	case NCS_MBCSV_MSG_COLD_SYNC_RESP:
		{
			/* Encode Cold Sync Response message */
			status = avsv_encode_cold_sync_rsp(cb, &arg->info.encode);
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_COLD_SYNC_RESP, NCSFL_SEV_INFO, arg->info.encode.io_reo_type);
		}
		break;

	case NCS_MBCSV_MSG_WARM_SYNC_REQ:
		{
			/* Encode Warm Sync Request message 
			 * Nothing is there to send at this point of time.
			 */
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_WARM_SYNC_REQ, NCSFL_SEV_INFO, 1);
		}
		break;

	case NCS_MBCSV_MSG_WARM_SYNC_RESP:
		{
			/* Encode Warm Sync Response message */
			status = avsv_encode_warm_sync_rsp(cb, &arg->info.encode);
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_WARM_SYNC_RESP, NCSFL_SEV_INFO, arg->info.encode.io_reo_type);
			arg->info.encode.io_msg_type = NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE;
		}
		break;

	case NCS_MBCSV_MSG_DATA_RESP:
		{
			/* Encode Data Response message */
			status = avsv_encode_data_sync_rsp(cb, &arg->info.encode);
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DATA_RESP, NCSFL_SEV_INFO, arg->info.encode.io_reo_type);
		}
		break;

	case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
	case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
	case NCS_MBCSV_MSG_DATA_REQ:
	case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
	default:
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(arg->info.encode.io_msg_type);
		status = NCSCC_RC_FAILURE;
	}

	return status;
}

/****************************************************************************\
 * Function: avsv_mbcsv_process_dec_cb
 *
 * Purpose:  AVSV MBCSV decode call back function.
 *
 * Input: arg - MBCSV callback argument pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_process_dec_cb(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
	uns32 status = NCSCC_RC_SUCCESS;

	TRACE_ENTER();

	switch (arg->info.decode.i_msg_type) {
	case NCS_MBCSV_MSG_ASYNC_UPDATE:
		{
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_ASYNC_UPDATE, NCSFL_SEV_INFO, arg->info.decode.i_reo_type);
			/* Decode Async update message */
			if (AVD_STBY_IN_SYNC == cb->stby_sync_state) {
				if (AVSV_SYNC_COMMIT != arg->info.decode.i_reo_type) {
					/* 
					 * Enqueue async update message till sync commit message is 
					 * received from the Active.
					 */
					avsv_enqueue_async_update_msgs(cb, &arg->info.decode);
				} else {
					/*
					 * we have received sync commit message. So process all async
					 * ckpt updates received till now.
					 */
					avsv_dequeue_async_update_msgs(cb, TRUE);
				}
			} else {
				/* Nothing is there to decode in this case */
				if (AVSV_SYNC_COMMIT == arg->info.decode.i_reo_type)
					return status;

				/* 
				 * Cold sync is in progress, then drop the async update for which
				 * cold sync response are yet to come.
				 */
				if (NCSCC_RC_SUCCESS != avsv_validate_reo_type_in_csync(cb,
											arg->info.decode.i_reo_type)) {
					/* Free userbuff and return without decoding */
					ncs_reset_uba(&arg->info.decode.i_uba);
					break;
				}

				/*
				 * During cold sync operation or in case of warm sync failure
				 * and standby require to be sync-up, process the async updates as and
				 * when they received. Dont enqueue them.
				 */
				if (arg->info.decode.i_reo_type < AVSV_CKPT_MSG_MAX) {
					status =
					    avsv_dec_ckpt_data_func_list[arg->info.decode.i_reo_type] (cb,
												       &arg->info.
												       decode);
				} else {
					m_AVD_LOG_INVALID_VAL_FATAL(arg->info.decode.i_reo_type);
					status = NCSCC_RC_FAILURE;
				}
			}
		}
		break;

	case NCS_MBCSV_MSG_COLD_SYNC_REQ:
		{
			/* 
			 * Decode Cold Sync request message 
			 * Nothing is there to decode.
			 */
			TRACE("COLD_SYNC_REQ");

			if (cb->init_state < AVD_INIT_DONE) {
				TRACE("invalid init state (%u) for cold sync req", cb->init_state);
				status = NCSCC_RC_FAILURE;
			}
		}
		break;

	case NCS_MBCSV_MSG_COLD_SYNC_RESP:
	case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
		{
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_COLD_SYNC_RESP, NCSFL_SEV_INFO, arg->info.decode.i_reo_type);

			/* Decode Cold Sync Response message */
			status = avsv_decode_cold_sync_rsp(cb, &arg->info.decode);

			if (NCSCC_RC_SUCCESS != status) {
				if (NCSCC_RC_SUCCESS != avd_data_clean_up(cb)) {
					/* Log Error ; FATAL; It should never happen unless there is a bug. */
					m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
				}

				m_AVD_LOG_INVALID_VAL_FATAL(arg->info.decode.i_reo_type);
				status = NCSCC_RC_FAILURE;
				break;
			}

			/* 
			 * If we have received cold sync complete message then mark standby
			 * as in sync. 
			 */
			if ((NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE == arg->info.decode.i_msg_type) &&
			    (NCSCC_RC_SUCCESS == status)) {
				m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE,
						   NCSFL_SEV_NOTICE, arg->info.decode.i_reo_type);

				cb->stby_sync_state = AVD_STBY_IN_SYNC;

				/* Start Heart Beating with the peer */
				avd_init_heartbeat(cb);
			}

			cb->synced_reo_type = arg->info.decode.i_reo_type;

			/* start sending heartbeat as soon as we get cb information. 
			 * Note:- we are not expecting any heartbeat to reach us and
			 * we are not starting rcv timer
			 */
			if (arg->info.decode.i_reo_type == 0) {
				/* Start Heart Beating with the peer */
				AVD_EVT evt;
				memset(&evt, 0, sizeof(evt));
				evt.info.tmr.type = AVD_TMR_SND_HB;
				avd_tmr_snd_hb_evh(cb, &evt);
			}

		}
		break;

	case NCS_MBCSV_MSG_WARM_SYNC_REQ:
		{
			/* 
			 * Decode Warm Sync Request message 
			 * Nothing is there to decode.
			 */
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_WARM_SYNC_REQ, NCSFL_SEV_INFO, 1);
		}
		break;

	case NCS_MBCSV_MSG_WARM_SYNC_RESP:
	case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
		{
			/* Decode Warm Sync Response message */
			status = avsv_decode_warm_sync_rsp(cb, &arg->info.decode);

			/* If we find mismatch in data or warm sync fails set in_sync to FALSE */
			if (NCSCC_RC_FAILURE == status) {
				m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_WARM_SYNC_RESP_FAILURE,
						   NCSFL_SEV_ALERT, arg->info.decode.i_reo_type);
				cb->stby_sync_state = AVD_STBY_OUT_OF_SYNC;
			}

			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_WARM_SYNC_RESP, NCSFL_SEV_INFO, arg->info.decode.i_reo_type);
		}
		break;

	case NCS_MBCSV_MSG_DATA_REQ:
		{
			/* Decode Data request message */
			status = avsv_decode_data_req(cb, &arg->info.decode);
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DATA_REQ, NCSFL_SEV_INFO, 1);
		}
		break;

	case NCS_MBCSV_MSG_DATA_RESP:
	case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
		{
			m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DATA_RESP, NCSFL_SEV_INFO, arg->info.decode.i_reo_type);

			/* Decode Data response and data response complete message */
			status = avsv_decode_data_sync_rsp(cb, &arg->info.decode);

			if (NCSCC_RC_SUCCESS != status) {
				m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DATA_RSP_DECODE_FAILURE,
						   NCSFL_SEV_EMERGENCY, arg->info.decode.i_reo_type);

				if (NCSCC_RC_SUCCESS != avd_data_clean_up(cb)) {
					/* Log Error ; FATAL; It should never happen unless there is a bug. */
					m_AVD_LOG_INVALID_VAL_FATAL(arg->info.decode.i_reo_type);
					break;
				}

				/*
				 * Now send data request, which will sync Standby with Active.
				 */
				if (NCSCC_RC_SUCCESS != avsv_send_data_req(cb)) {
					/* Log error */
					m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
					break;
				}

				break;
			}

			if (NCS_MBCSV_MSG_DATA_RESP_COMPLETE == arg->info.decode.i_msg_type) {
				cb->stby_sync_state = AVD_STBY_IN_SYNC;
				m_AVD_LOG_CKPT_EVT(AVD_MBCSV_MSG_DATA_RESP_COMPLETE,
						   NCSFL_SEV_INFO, arg->info.decode.i_reo_type);
			}
		}
		break;

	default:
		m_AVD_LOG_INVALID_VAL_FATAL(arg->info.decode.i_msg_type);
		status = NCSCC_RC_FAILURE;
		break;

	}
	return status;

}

/****************************************************************************\
 * Function: avsv_mbcsv_process_peer_info_cb
 *
 * Purpose:  AVSV MBCSV peer Info call back function.
 *
 * Input: arg - MBCSV callback argument pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_process_peer_info_cb(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
	TRACE_ENTER();

	/* Used to refer while sending checkpoint msg to its peer AVD */
	cb->avd_peer_ver = arg->info.peer.i_peer_version;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_process_notify_msg
 *
 * Purpose:  AVSV MBCSV process Notify message.
 *
 * Input: arg - MBCSV callback argument pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_process_notify(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
	m_AVD_LOG_INVALID_VAL_FATAL(0);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_process_err_ind
 *
 * Purpose:  AVSV MBCSV process error indication.
 *
 * Input: arg - MBCSV callback argument pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_process_err_ind(AVD_CL_CB *cb, NCS_MBCSV_CB_ARG *arg)
{
	TRACE_ENTER();

	switch (arg->info.error.i_code) {
	case NCS_MBCSV_COLD_SYNC_TMR_EXP:
		avd_log(NCSFL_SEV_WARNING, "cold sync tmr exp");
		break;

	case NCS_MBCSV_WARM_SYNC_TMR_EXP:
		avd_log(NCSFL_SEV_WARNING, "warm sync tmr exp");
		break;

	case NCS_MBCSV_DATA_RSP_CMPLT_TMR_EXP:
		avd_log(NCSFL_SEV_WARNING, "data rsp cmplt tmr exp");
		break;

	case NCS_MBCSV_COLD_SYNC_CMPL_TMR_EXP:
		avd_log(NCSFL_SEV_WARNING, "cold sync cmpl tmr exp");
		break;

	case NCS_MBCSV_WARM_SYNC_CMPL_TMR_EXP:
		avd_log(NCSFL_SEV_WARNING, "warm sync cmpl tmr exp");
		break;

	case NCS_MBCSV_DATA_RESP_TERMINATED:
		avd_log(NCSFL_SEV_WARNING, "data rsp term");
		break;

	case NCS_MBCSV_COLD_SYNC_RESP_TERMINATED:
		avd_log(NCSFL_SEV_WARNING, "cold sync rsp term");
		break;

	case NCS_MBCSV_WARM_SYNC_RESP_TERMINATED:
		avd_log(NCSFL_SEV_WARNING, "warm sync rsp term");
		break;

	default:
		m_AVD_LOG_INVALID_VAL_FATAL(arg->info.error.i_code);
		break;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_initialize
 *
 * Purpose:  Initialize AVSV with MBCSV
 *
 * Input: cb       - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_initialize(AVD_CL_CB *cb)
{
	NCS_MBCSV_ARG mbcsv_arg;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_INITIALIZE;
	mbcsv_arg.info.initialize.i_service = NCS_SERVICE_ID_AVD;
	mbcsv_arg.info.initialize.i_mbcsv_cb = avsv_mbcsv_cb;
	mbcsv_arg.info.initialize.i_version = AVD_MBCSV_SUB_PART_VERSION;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	cb->mbcsv_hdl = mbcsv_arg.info.initialize.o_mbcsv_hdl;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_open_ckpt
 *
 * Purpose:  Open checkpoint.
 *
 * Input: cb   - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_open_ckpt(AVD_CL_CB *cb)
{
	NCS_MBCSV_ARG mbcsv_arg;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_OPEN;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;
	mbcsv_arg.info.open.i_pwe_hdl = cb->vaddr_pwe_hdl;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	cb->ckpt_hdl = mbcsv_arg.info.open.o_ckpt_hdl;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_set_ckpt_role
 *
 * Purpose:  Set checkpoint role.
 *
 * Input: cb       - AVD control block pointer.
 *        role     - Role to be set.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_set_ckpt_role(AVD_CL_CB *cb, uns32 role)
{
	NCS_MBCSV_ARG mbcsv_arg;
	uns32 rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_CHG_ROLE;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;
	mbcsv_arg.info.chg_role.i_ckpt_hdl = cb->ckpt_hdl;
	mbcsv_arg.info.chg_role.i_ha_state = role;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		LOG_ER("ncs_mbcsv_svc NCS_MBCSV_OP_CHG_ROLE %u failed", role);
		return NCSCC_RC_FAILURE;
	}

	return rc;
}

/****************************************************************************\
 * Function: avd_avnd_send_role_change
 *
 * Purpose:  Set checkpoint role.
 *
 * Input: cb       - AVD control block pointer.
 *        node_id  - Node id of the AvND.
 *        role     - Role to be set.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uns32 avd_avnd_send_role_change(AVD_CL_CB *cb, NODE_ID node_id, uns32 role)
{
	AVD_DND_MSG *d2n_msg = NULL;
	AVD_AVND *avnd = NULL;

	m_AVD_PXY_PXD_ENTRY_LOG("avd_avnd_send_role_change: Comp, node_id and role are", NULL, node_id, role, 0, 0);

	/* Check wether we are sending to peer controller AvND, which is of older
	   version. Don't send role change to older version Controler AvND. */
	if ((node_id == cb->node_id_avd_other) && (cb->peer_msg_fmt_ver < AVSV_AVD_AVND_MSG_FMT_VER_2)) {
		goto done;
	}

	/* It may happen that this function has been called before AvND has come
	   up, so just return SUCCESS. Send the role change when AvND comes up. */
	if ((avnd = avd_node_find_nodeid(node_id)
	    ) == NULL) {
		m_AVD_PXY_PXD_ERR_LOG("avd_avnd_send_role_change: avnd is NULL. Comp,node_id and role are",
				      NULL, node_id, role, 0, 0);
		goto done;
	}

	d2n_msg = calloc(1, sizeof(AVSV_DND_MSG));
	if (d2n_msg == AVD_DND_MSG_NULL) {
		/* log error that the director is in degraded situation */
		m_AVD_LOG_MEM_FAIL_LOC(AVD_DND_MSG_ALLOC_FAILED);
		m_AVD_LOG_INVALID_VAL_FATAL(avnd->node_info.nodeId);
		return NCSCC_RC_FAILURE;
	}

	d2n_msg->msg_type = AVSV_D2N_ROLE_CHANGE_MSG;
	d2n_msg->msg_info.d2n_role_change_info.msg_id = ++(avnd->snd_msg_id);
	d2n_msg->msg_info.d2n_role_change_info.node_id = avnd->node_info.nodeId;
	d2n_msg->msg_info.d2n_role_change_info.role = role;

	/* send the message */
	if (avd_d2n_msg_snd(cb, avnd, d2n_msg) != NCSCC_RC_SUCCESS) {
		/* log error that the director is not able to send the message */
		--(avnd->snd_msg_id);
		m_AVD_LOG_INVALID_VAL_ERROR(avnd->node_info.nodeId);
		m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_ERROR, d2n_msg, sizeof(AVD_DND_MSG), d2n_msg);
		/* free the message */

		avsv_dnd_msg_free(d2n_msg);
		return NCSCC_RC_FAILURE;
	}

	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_SND_MSG_ID);

	avd_log(NCSFL_SEV_NOTICE, "Role sent SUCC. node_id=%x, role=%u", node_id, role);

	/*avsv_dnd_msg_free(d2n_msg); */

 done:
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_get_mbcsv_sel_obj
 *
 * Purpose:  Get MBCSv Selection object.
 *
 * Input: cb        - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_get_mbcsv_sel_obj(AVD_CL_CB *cb)
{
	NCS_MBCSV_ARG mbcsv_arg;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_SEL_OBJ_GET;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	cb->mbcsv_sel_obj = mbcsv_arg.info.sel_obj_get.o_select_obj;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_dispatch
 *
 * Purpose:  Perform dispatch operation on MBCSV selection object.
 *
 * Input: cb        - AVD control block pointer.
 *        sel_obj   - Selection object returned.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_mbcsv_dispatch(AVD_CL_CB *cb, uns32 flag)
{
	NCS_MBCSV_ARG mbcsv_arg;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_DISPATCH;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;
	mbcsv_arg.info.dispatch.i_disp_flags = flag;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		LOG_ER("ncs_mbcsv_svc NCS_MBCSV_OP_DISPATCH %u failed", flag);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_send_ckpt_data
 *
 * Purpose:  Send checkpoint data to standby using the supplied send type.
 *
 * Input: cb        - AVD control block pointer.
 *        action    - Action to be perform (add, remove or update)
 *        reo_hdl   - Redudant object handle.
 *        reo_type  - Redudant object type.
 *        send_type - Send type to be used.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_send_ckpt_data(AVD_CL_CB *cb, uns32 action, MBCSV_REO_HDL reo_hdl, uns32 reo_type, uns32 send_type)
{
	NCS_MBCSV_ARG mbcsv_arg;

	/* 
	 * Validate HA state. If my HA state is Standby then don't send 
	 * async updates. In all other states, MBCSv will take care of sending
	 * async updates.
	 */
	if (SA_AMF_HA_STANDBY == cb->avail_state_avd)
		return NCSCC_RC_SUCCESS;

	TRACE_ENTER();
	/*
	 * Get mbcsv_handle and checkpoint handle from CB.
	 */
	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_CKPT;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;
	mbcsv_arg.info.send_ckpt.i_action = action;
	mbcsv_arg.info.send_ckpt.i_ckpt_hdl = cb->ckpt_hdl;
	mbcsv_arg.info.send_ckpt.i_reo_hdl = reo_hdl;
	mbcsv_arg.info.send_ckpt.i_reo_type = reo_type;
	mbcsv_arg.info.send_ckpt.i_send_type = send_type;

	/*
	 * Before sendig this message, update async update count.
	 */
	switch (reo_type) {
	case AVSV_CKPT_AVD_CB_CONFIG:
	case AVSV_CKPT_CB_CL_VIEW_NUM:
		cb->async_updt_cnt.cb_updt++;
		break;

	case AVSV_CKPT_AVD_NODE_CONFIG:
	case AVSV_CKPT_AVND_NODE_UP_INFO:
	case AVSV_CKPT_AVND_ADMIN_STATE:
	case AVSV_CKPT_AVND_OPER_STATE:
	case AVSV_CKPT_AVND_NODE_STATE:
	case AVSV_CKPT_AVND_RCV_MSG_ID:
	case AVSV_CKPT_AVND_SND_MSG_ID:
	case AVSV_CKPT_AVND_AVM_OPER_STATE:
		cb->async_updt_cnt.node_updt++;
		break;

	case AVSV_CKPT_AVD_APP_CONFIG:
		cb->async_updt_cnt.app_updt++;
		break;

	case AVSV_CKPT_AVD_SG_CONFIG:
	case AVSV_CKPT_SG_ADMIN_STATE:
	case AVSV_CKPT_SG_ADJUST_STATE:
	case AVSV_CKPT_SG_SU_ASSIGNED_NUM:
	case AVSV_CKPT_SG_SU_SPARE_NUM:
	case AVSV_CKPT_SG_SU_UNINST_NUM:
	case AVSV_CKPT_SG_FSM_STATE:
		cb->async_updt_cnt.sg_updt++;
		break;

	case AVSV_CKPT_AVD_SU_CONFIG:
	case AVSV_CKPT_SU_SI_CURR_ACTIVE:
	case AVSV_CKPT_SU_SI_CURR_STBY:
	case AVSV_CKPT_SU_ADMIN_STATE:
	case AVSV_CKPT_SU_TERM_STATE:
	case AVSV_CKPT_SU_SWITCH:
	case AVSV_CKPT_SU_OPER_STATE:
	case AVSV_CKPT_SU_PRES_STATE:
	case AVSV_CKPT_SU_READINESS_STATE:
	case AVSV_CKPT_SU_ACT_STATE:
	case AVSV_CKPT_SU_PREINSTAN:
		cb->async_updt_cnt.su_updt++;
		break;

	case AVSV_CKPT_AVD_SI_CONFIG:
	case AVSV_CKPT_SI_SU_CURR_ACTIVE:
	case AVSV_CKPT_SI_SU_CURR_STBY:
	case AVSV_CKPT_SI_SWITCH:
	case AVSV_CKPT_SI_ADMIN_STATE:
		cb->async_updt_cnt.si_updt++;
		break;

	case AVSV_CKPT_AVD_SG_OPER_SU:
		cb->async_updt_cnt.sg_su_oprlist_updt++;
		break;

	case AVSV_CKPT_AVD_SG_ADMIN_SI:
		cb->async_updt_cnt.sg_admin_si_updt++;
		break;

	case AVSV_CKPT_AVD_COMP_CONFIG:
	case AVSV_CKPT_COMP_CURR_PROXY_NAME:
	case AVSV_CKPT_COMP_CURR_NUM_CSI_ACTV:
	case AVSV_CKPT_COMP_CURR_NUM_CSI_STBY:
	case AVSV_CKPT_COMP_OPER_STATE:
	case AVSV_CKPT_COMP_READINESS_STATE:
	case AVSV_CKPT_COMP_PRES_STATE:
	case AVSV_CKPT_COMP_RESTART_COUNT:
		cb->async_updt_cnt.comp_updt++;
		break;
	case AVSV_CKPT_AVD_SI_ASS:
		cb->async_updt_cnt.siass_updt++;
		break;

	case AVSV_SYNC_COMMIT:
		break;

	case AVSV_CKPT_AVD_COMP_CS_TYPE_CONFIG:
		cb->async_updt_cnt.compcstype_updt++;
		break;

	default:
		return NCSCC_RC_SUCCESS;
	}

	/*
	 * Now send this update.
	 */
	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_close_ckpt
 *
 * Purpose:  Close checkpoint.
 *
 * Input: cb        - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_close_ckpt(AVD_CL_CB *cb)
{
	NCS_MBCSV_ARG mbcsv_arg;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_CLOSE;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;
	mbcsv_arg.info.close.i_ckpt_hdl = cb->ckpt_hdl;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_finalize
 *
 * Purpose:  Finalize AVSV with MBCSV
 *
 * Input: cb        - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_mbcsv_finalize(AVD_CL_CB *cb)
{
	NCS_MBCSV_ARG mbcsv_arg;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_FINALIZE;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_mbcsv_obj_set
 *
 * Purpose:  Set MBCSv objects
 *
 * Input: cb        - AVD control block pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_mbcsv_obj_set(AVD_CL_CB *cb, uns32 obj, uns32 val)
{
	NCS_MBCSV_ARG mbcsv_arg;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_OBJ_SET;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;
	mbcsv_arg.info.obj_set.i_ckpt_hdl = cb->ckpt_hdl;
	mbcsv_arg.info.obj_set.i_obj = obj;
	mbcsv_arg.info.obj_set.i_val = val;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_enqueue_async_update_msgs
 *
 * Purpose:  Enqueue async update messages.
 *
 * Input: cb  - AVD CB pointer.
 *        dec - Decode message content pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_enqueue_async_update_msgs(AVD_CL_CB *cb, NCS_MBCSV_CB_DEC *dec)
{
	AVSV_ASYNC_UPDT_MSG_QUEUE *updt_msg;

	TRACE_ENTER();
	/*
	 * This is a FIFO queue. Add message at the tail of the queue.
	 */
	if (NULL == (updt_msg = calloc(1, sizeof(AVSV_ASYNC_UPDT_MSG_QUEUE)))) {
		/* Log error */
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	updt_msg->dec = *dec;

	/* Add this message in our FIFO queue */
	if (!(cb->async_updt_msgs.async_updt_queue))
		cb->async_updt_msgs.async_updt_queue = (updt_msg);
	else
		cb->async_updt_msgs.tail->next = (updt_msg);

	cb->async_updt_msgs.tail = (updt_msg);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_dequeue_async_update_msgs
 *
 * Purpose:  De-queue async update messages.
 *
 * Input: cb - AVD CB pointer.
 *        pr_or_fr - TRUE - If we have to process the message.
 *                   FALSE - If we have to FREE the message.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_dequeue_async_update_msgs(AVD_CL_CB *cb, NCS_BOOL pr_or_fr)
{
	uns32 status = NCSCC_RC_SUCCESS;
	AVSV_ASYNC_UPDT_MSG_QUEUE *updt_msg;

	TRACE_ENTER();

	/*
	 * This is a FIFO queue. Remove first message first entered in the 
	 * queue and then process it.
	 */
	while (NULL != (updt_msg = cb->async_updt_msgs.async_updt_queue)) {
		cb->async_updt_msgs.async_updt_queue = updt_msg->next;

		/*
		 * Process de-queued message.
		 */
		if (pr_or_fr)
			status = avsv_dec_ckpt_data_func_list[updt_msg->dec.i_reo_type] (cb, &updt_msg->dec);

		free(updt_msg);
	}

	/* All messages are dequeued. Set tail to NULL */
	cb->async_updt_msgs.tail = NULL;

	return status;
}

/****************************************************************************\
 * Function: avsv_send_data_req
 *
 * Purpose:  Encode data request to be sent.
 *
 * Input: cb - AVD CB pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_send_data_req(AVD_CL_CB *cb)
{
	NCS_MBCSV_ARG mbcsv_arg;
	NCS_UBAID *uba = NULL;

	TRACE_ENTER();

	memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

	mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_DATA_REQ;
	mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;

	uba = &mbcsv_arg.info.send_data_req.i_uba;

	memset(uba, '\0', sizeof(NCS_UBAID));

	mbcsv_arg.info.send_data_req.i_ckpt_hdl = cb->ckpt_hdl;

	if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg)) {
		m_AVD_LOG_INVALID_VAL_FATAL(NCSCC_RC_FAILURE);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: avsv_validate_reo_type_in_csync
 *
 * Purpose:  Vaidate reo_type received during cold sync updates. Return success,
 *           if cold sync is over for this reo_type. Return failure if standby
 *           is still to get cold sync for this reo_type.
 *
 * Input: cb - AVD CB pointer.
 *        reo_type - reo type need to be validated during cold sync.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
static uns32 avsv_validate_reo_type_in_csync(AVD_CL_CB *cb, uns32 reo_type)
{
	uns32 status = NCSCC_RC_FAILURE;

	TRACE_ENTER();

	switch (reo_type) {
	case AVSV_CKPT_AVD_CB_CONFIG:
	case AVSV_CKPT_CB_CL_VIEW_NUM:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_CB_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;

		/* AVND Async Update messages */
	case AVSV_CKPT_AVD_NODE_CONFIG:
	case AVSV_CKPT_AVND_NODE_UP_INFO:
	case AVSV_CKPT_AVND_ADMIN_STATE:
	case AVSV_CKPT_AVND_OPER_STATE:
	case AVSV_CKPT_AVND_NODE_STATE:
	case AVSV_CKPT_AVND_RCV_MSG_ID:
	case AVSV_CKPT_AVND_SND_MSG_ID:
	case AVSV_CKPT_AVND_AVM_OPER_STATE:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_NODE_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;

	case AVSV_CKPT_AVD_APP_CONFIG:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_APP_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;

		/* SG Async Update messages */
	case AVSV_CKPT_AVD_SG_CONFIG:
	case AVSV_CKPT_SG_ADMIN_STATE:
	case AVSV_CKPT_SG_ADJUST_STATE:
	case AVSV_CKPT_SG_SU_ASSIGNED_NUM:
	case AVSV_CKPT_SG_SU_SPARE_NUM:
	case AVSV_CKPT_SG_SU_UNINST_NUM:
	case AVSV_CKPT_SG_FSM_STATE:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_SG_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;

		/* SU Async Update messages */
	case AVSV_CKPT_AVD_SU_CONFIG:
	case AVSV_CKPT_SU_SI_CURR_ACTIVE:
	case AVSV_CKPT_SU_SI_CURR_STBY:
	case AVSV_CKPT_SU_ADMIN_STATE:
	case AVSV_CKPT_SU_TERM_STATE:
	case AVSV_CKPT_SU_SWITCH:
	case AVSV_CKPT_SU_OPER_STATE:
	case AVSV_CKPT_SU_PRES_STATE:
	case AVSV_CKPT_SU_READINESS_STATE:
	case AVSV_CKPT_SU_ACT_STATE:
	case AVSV_CKPT_SU_PREINSTAN:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_SU_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;

		/* SI Async Update messages */
	case AVSV_CKPT_AVD_SI_CONFIG:
	case AVSV_CKPT_SI_SU_CURR_ACTIVE:
	case AVSV_CKPT_SI_SU_CURR_STBY:
	case AVSV_CKPT_SI_SWITCH:
	case AVSV_CKPT_SI_ADMIN_STATE:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_SI_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;

	case AVSV_CKPT_AVD_SG_OPER_SU:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_SG_OPER_SU)
			status = NCSCC_RC_SUCCESS;
		break;

	case AVSV_CKPT_AVD_SG_ADMIN_SI:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_SG_ADMIN_SI)
			status = NCSCC_RC_SUCCESS;
		break;

		/* COMP Async Update messages */
	case AVSV_CKPT_AVD_COMP_CONFIG:
	case AVSV_CKPT_COMP_CURR_PROXY_NAME:
	case AVSV_CKPT_COMP_CURR_NUM_CSI_ACTV:
	case AVSV_CKPT_COMP_CURR_NUM_CSI_STBY:
	case AVSV_CKPT_COMP_OPER_STATE:
	case AVSV_CKPT_COMP_PRES_STATE:
	case AVSV_CKPT_COMP_RESTART_COUNT:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_COMP_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;
	case AVSV_CKPT_AVD_SI_ASS:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_SI_ASS)
			status = NCSCC_RC_SUCCESS;
		break;

	case AVSV_CKPT_AVD_COMP_CS_TYPE_CONFIG:
		if (cb->synced_reo_type >= AVSV_CKPT_AVD_COMP_CS_TYPE_CONFIG)
			status = NCSCC_RC_SUCCESS;
		break;

	}
	return status;
}

/***************************************************************************\
 *
 * Function   :avsv_send_hb_ntfy_msg
 *
 * Purpose:  Function for sending the heart-beat notify message to the 
 *           peer. We assume that AVD has one peer only. So when we send
 *           heart beat message then it will get sent to only peer.
 *           We are sending following info with this message:
 *           1) Node ID 2) Rcv HB timer 3) SND HB timer 4) HA state
 *
 * Input: cb - AVD CB pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uns32 avsv_send_hb_ntfy_msg(AVD_CL_CB *cb)
{
	return NCSCC_RC_SUCCESS;
}
