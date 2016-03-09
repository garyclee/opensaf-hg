/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
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
 * Author(s): Ericsson AB
 *
 */

#include "lga_state.h"
#include <stdlib.h>
#include <syslog.h>
#include <pthread.h>
#include <osaf_poll.h>
#include <ncs_osprm.h>
#include <osaf_time.h>
#include <saf_error.h>

/***
 * Common data
 */

/* Selection object for terminating recovery thread for state 2 (after timeout)
 * NOTE: Only for recovery2_thread
 */
static NCS_SEL_OBJ state2_terminate_sel_obj;

static pthread_t recovery2_thread_id = 0;
static pthread_mutex_t lga_recov_mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************
 * Functions used with server down recovery handling
 ******************************************************************************/

static int start_recovery2_thread(void);

/******************************************************************************
 * Functions for sending messages to the server used in the recovery functions
 * Handles TRY AGAIN
 ******************************************************************************/

/**
 * Send an initialize message to the server. A number of retries is done if
 * return code is TRY AGAIN
 * The server returns a client id
 *
 * @param client_id[out]
 * @return -1 on error (client id not valid)
 */
static int send_initialize_msg(uint32_t *client_id)
{
	SaVersionT version = {
		.releaseCode = LOG_RELEASE_CODE,
		.majorVersion = LOG_MAJOR_VERSION,
		.minorVersion = LOG_MINOR_VERSION
	};

	SaAisErrorT ais_rc = SA_AIS_ERR_TRY_AGAIN;
	uint32_t ncs_rc = NCSCC_RC_SUCCESS;
	int rc = 0;
	lgsv_msg_t i_msg, *o_msg = NULL;
	uint32_t try_again_cnt = 10;
	const uint32_t sleep_delay_ms = 100;

	TRACE_ENTER();

	/* Populate the message to be sent to the LGS */
	memset(&i_msg, 0, sizeof(lgsv_msg_t));
	i_msg.type = LGSV_LGA_API_MSG;
	i_msg.info.api_info.type = LGSV_INITIALIZE_REQ;
	i_msg.info.api_info.param.init.version = version;

	/* Send a message to LGS to obtain a client_id
	 */
	while(try_again_cnt > 0) {
		ncs_rc = lga_mds_msg_sync_send(&lga_cb, &i_msg, &o_msg,
			LGS_WAIT_TIME,MDS_SEND_PRIORITY_HIGH);
		if (ncs_rc != NCSCC_RC_SUCCESS) {
			LOG_NO("%s lga_mds_msg_sync_send() Fail %d",
				__FUNCTION__, ncs_rc);
			rc = -1;
			goto done;
		}

		ais_rc = o_msg->info.api_resp_info.rc;
		if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
			usleep(sleep_delay_ms * 1000);
		} else {
			break;
		}
		try_again_cnt--;
	}
	if (SA_AIS_OK != ais_rc) {
		TRACE("%s LGS error response %s", __FUNCTION__, saf_error(ais_rc));
		rc = -1;
		goto free_done;
	}

	/* Initialize succeeded */
	*client_id = o_msg->info.api_resp_info.param.init_rsp.client_id;

free_done:
	/* Free up the response message */
	lga_msg_destroy(o_msg);

done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
}

/**
 * Send an open message to the server. A number of retries is done if
 * return code is TRY AGAIN
 * The server returns client id and a stream id
 *
 * @param lstream_id[out] Log stream id received from server
 * @param p_stream[in]    Pointer to a stream record
 * @return -1 on error (stream id not valid)
 */
static int send_stream_open_msg(uint32_t *lstream_id,
	lga_log_stream_hdl_rec_t *p_stream)
{
	SaAisErrorT ais_rc = SA_AIS_OK;
	uint32_t ncs_rc = NCSCC_RC_SUCCESS;
	int rc = 0;
	lgsv_msg_t i_msg, *o_msg;
	lgsv_stream_open_req_t *open_param;
	lga_client_hdl_rec_t *p_client = p_stream->parent_hdl;
	uint32_t try_again_cnt = 10;
	const uint32_t sleep_delay_ms = 100;

	TRACE_ENTER();
	TRACE("\t log_stream_name \"%s\", lgs_client_id=%d",
		p_stream->log_stream_name.value, p_client->lgs_client_id);

	/* Populate a stream open message to the LGS
	 */
	memset(&i_msg, 0, sizeof(lgsv_msg_t));
	open_param = &i_msg.info.api_info.param.lstr_open_sync;

	/* Set the open parameters to open a stream for recovery */
	open_param->client_id = p_client->lgs_client_id;
	open_param->lstr_name = p_stream->log_stream_name;
	open_param->logFileFmt = NULL;
	open_param->logFileFmtLength = 0;
	open_param->maxLogFileSize = 0;
	open_param->maxLogRecordSize = 0;
	open_param->haProperty = SA_FALSE;
	open_param->logFileFullAction = 0;
	open_param->maxFilesRotated = 0;
	open_param->lstr_open_flags = 0;

	i_msg.type = LGSV_LGA_API_MSG;
	i_msg.info.api_info.type = LGSV_STREAM_OPEN_REQ;

	/* Send a message to LGS to obtain a stream_id
	 */
	while(try_again_cnt > 0) {
		ncs_rc = lga_mds_msg_sync_send(&lga_cb, &i_msg, &o_msg,
			LGS_WAIT_TIME, MDS_SEND_PRIORITY_HIGH);
		if (ncs_rc != NCSCC_RC_SUCCESS) {
			rc = -1;
			TRACE("%s lga_mds_msg_sync_send() Fail %d", __FUNCTION__, ncs_rc);
			goto done;
		}

		ais_rc = o_msg->info.api_resp_info.rc;
		if (ais_rc == SA_AIS_ERR_TRY_AGAIN) {
			usleep(sleep_delay_ms * 1000);
		} else {
			break;
		}
		try_again_cnt--;
	}
	if (SA_AIS_OK != ais_rc) {
		TRACE("%s LGS error response %s", __FUNCTION__, saf_error(ais_rc));
		rc = -1;
		goto free_done;
	}

	/* Open succeeded */
	*lstream_id = o_msg->info.api_resp_info.param.lstr_open_rsp.lstr_id;

free_done:
	/* Free up the response message */
	lga_msg_destroy(o_msg);

done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
}

/******************************************************************************
 * Recovery functions
 ******************************************************************************/

/**
 * Register an existing client with the server
 *  - Send an initialize request to the server
 *  - A new client id is received replacing the old no longer valid one
 *
 * This function shall only be used in recovery. It is assumed that
 * lga_serv_downstate_set() has been called
 *
 * @param p_client[in] Pointer to a client record
 * @return -1 on error
 *          0 client is successfully initialized
 *          1 client was already initialized
 */
static int initialize_one_client(lga_client_hdl_rec_t *p_client)
{
	int rc = 0;
	uint32_t client_id;

	TRACE_ENTER();

	if (p_client->initialized_flag == true) {
		/* The client is already initialized */
		rc = 1;
		goto done;
	}

	rc = send_initialize_msg(&client_id);
	if (rc == -1) {
		TRACE("%s initialize_msg_send Fail", __FUNCTION__);
	}

	/* Restore the client Id with the one returned by the LGS and
	 * set the initialized flag
	 */
	osaf_mutex_lock_ordie(&lga_cb.cb_lock);
	p_client->lgs_client_id = client_id;
	p_client->initialized_flag = true;
	osaf_mutex_unlock_ordie(&lga_cb.cb_lock);

done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
}

/**
 * Register a stream that was opened by the client with the server
 *  - Send a stream open request to the server (no parameter list)
 *  - A new stream id is received replacing the old no longer valid one
 *
 * @param p_stream[in] Pointer to a stream record
 * @return -1 on error
 *          0 The stream was successfully recovered
 *          1 The stream was already recovered
 */
static int recover_one_stream(lga_log_stream_hdl_rec_t *p_stream)
{
	int rc = 0;
	uint32_t stream_id;

	TRACE_ENTER();

	if (p_stream->recovered_flag == true) {
		/* The stream is already recovered */
		rc = 1;
		goto done;
	}

	rc = send_stream_open_msg(&stream_id, p_stream);
	if (rc == -1) {
		TRACE("%s open_stream_msg_send Fail", __FUNCTION__);
		goto done;
	}

	/* Restore the stream Id with the Id returned by the LGS and
	 * set the recovered flag
	 */
	osaf_mutex_lock_ordie(&lga_cb.cb_lock);
	p_stream->lgs_log_stream_id = stream_id;
	p_stream->recovered_flag = true;
	osaf_mutex_unlock_ordie(&lga_cb.cb_lock);

done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
}

/******************************************************************************
 * Recovery 2 thread handling functions
 ******************************************************************************/

/**
 * Thread for handling recovery step 2
 * Wait for timeout or stop request
 * After timeout, recover all not already recovered clients
 *
 * @param int timeout_time_in Timeout time in s
 * @return NULL
 */
static void *recovery2_thread(void *dummy)
{
	int rc = 0;
	struct timespec seed_ts;
	int timeout_ms;

	TRACE_ENTER();

	/* Create a random timeout time in msec within an interval
	 */
	/* Set seed. Use nanoseconds from clock */
	osaf_clock_gettime(CLOCK_MONOTONIC, &seed_ts);
	srandom((unsigned int) seed_ts.tv_nsec);
	/* Interval 400 - 500 sec */
	timeout_ms = (int) (random() % 100 + 400) * 1000;

	/* Wait for timeout or a signal to terminate
	 */
	rc = osaf_poll_one_fd(state2_terminate_sel_obj.rmv_obj, timeout_ms);
	if (rc == -1) {
		TRACE("%s osaf_poll_one_fd Fail %s", __FUNCTION__, strerror(errno));
		goto done;
	}

	if (rc == 0) {
		/* Timeout; Set recovery state 2 */
		osaf_mutex_lock_ordie(&lga_cb.cb_lock);
		lga_cb.lga_state = LGA_RECOVERY2;
		osaf_mutex_unlock_ordie(&lga_cb.cb_lock);
		TRACE("%s Poll timeout. Enter LGA_RECOVERY2 state", __FUNCTION__);
	} else {
		/* Stop signal received */
		TRACE("%s Stop signal received", __FUNCTION__);
		goto done;
	}

	/* Execute recovery 2
	 * Recover one client at the time
	 * If fail to recover remove the client. This means that the client
	 * handle is invalidated
	 * When all clients are recovered or if exit is requested exit the
	 * thread
	 */
	TRACE("%s Execute recovery 2", __FUNCTION__);
	/* First client */
	lga_client_hdl_rec_t *p_client;
	p_client = lga_cb.client_list;

	while (p_client != NULL) {
		/* Exit if requested to */
		rc = osaf_poll_one_fd(state2_terminate_sel_obj.rmv_obj, 0);
		if (rc > 0) {
			/* We have been signaled to exit */
			goto done;
		}
		/* Recover clients one at a time */
		rc = recover_one_client(p_client);
		TRACE("\t Client %d is recovered", p_client->lgs_client_id);
		if (rc == -1) {
			TRACE("%s recover_one_client Fail Deleting cllient (id %d)",
				__FUNCTION__, p_client->lgs_client_id);
			/* Fail to recover this client
			 * Remove (handle invalidated)
			 */
			osaf_mutex_lock_ordie(&lga_cb.cb_lock);
			(void) delete_one_client(&lga_cb.client_list, p_client);
			osaf_mutex_unlock_ordie(&lga_cb.cb_lock);
		}

		/* Next client */
		p_client = p_client->next;
	}

	/* All clients are recovered (or removed).
	 * Or recovery is aborted
	 * Change to not recovering state
	 * LGA_NORMAL
	 */
	osaf_mutex_lock_ordie(&lga_cb.cb_lock);
	lga_cb.lga_state = LGA_NORMAL;
	TRACE("\t Setting lga_state = LGA_NORMAL");
	osaf_mutex_unlock_ordie(&lga_cb.cb_lock);

done:
	/* Cleanup and Exit thread */
	(void) ncs_sel_obj_destroy(&state2_terminate_sel_obj);

	TRACE_LEAVE();
	return NULL;
}

/**
 * Setup and start the thread for recovery state 2
 *
 * @return -1 on error
 */
static int start_recovery2_thread(void)
{
	int rc = 0;
	uint32_t ncs_rc = NCSCC_RC_SUCCESS;
	pthread_attr_t attr;

	TRACE_ENTER();

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	/* Create a selection object for signaling the recovery2 thread */
	ncs_rc = ncs_sel_obj_create(&state2_terminate_sel_obj);
	if (ncs_rc != NCSCC_RC_SUCCESS) {
		TRACE("%s ncs_sel_obj_create Fail", __FUNCTION__);
		rc = -1;
		goto done;
	}

	/* Create the thread
	 */
	if (pthread_create(&recovery2_thread_id, &attr, recovery2_thread, NULL)	!= 0) {
		/* pthread_create() error handling */
		TRACE("\t pthread_create FAILED: %s", strerror(errno));
		rc = -1;
		(void) ncs_sel_obj_destroy(&state2_terminate_sel_obj);
		goto done;
	}
	pthread_attr_destroy(&attr);

done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
}

/**
 * Stops the recovery 2 thread
 * It is safe to call this function also if the thread is not running
 */
static void stop_recovery2_thread(void)
{
	uint32_t ncs_rc = 0;

	TRACE_ENTER();

	/* Check if the thread is running */
	osaf_mutex_lock_ordie(&lga_cb.cb_lock);
	lga_state_t lga_state = lga_cb.lga_state;
	osaf_mutex_unlock_ordie(&lga_cb.cb_lock);

	if (lga_state == LGA_NORMAL) {
		/* No thread to stop */
		TRACE("%s LGA_NORMAL no thread to stop", __FUNCTION__);
		goto done;
	}

	/* Signal the thread to stop */
	ncs_rc = ncs_sel_obj_ind(&state2_terminate_sel_obj);
	if (ncs_rc != NCSCC_RC_SUCCESS) {
		TRACE("%s ncs_sel_obj_ind Fail", __FUNCTION__);
	}

	done:
	TRACE_LEAVE();
	return;
}

/******************************************************************************
 * Recovery state handling functions
 ******************************************************************************/

/******************************************************************************
 * Server Down
 *
 * Initiate recovery handling and set LGA_NO_SERVER state
 * LGA_NO_SERVER: State set in MDS event handler when server down event
 *                lga_no_server_state_set()
 ******************************************************************************/

/**
 * Recovery state LGA_NO_SERVER
 *
 * Setup server down state
 * - Mark all clients and their streams as not recovered
 * - Remove recovery thread if exist
 * - Set LGA_NO_SERVER state
 *
 */
void lga_no_server_state_set(void)
{
	lga_client_hdl_rec_t *p_client = lga_cb.client_list;
	lga_log_stream_hdl_rec_t *p_stream;

	TRACE_ENTER();

	/* Stop recovery thread for state 2 if exist */
	stop_recovery2_thread();

	/* Set LGA_NO_SERVER state */
	osaf_mutex_lock_ordie(&lga_cb.cb_lock);
	lga_cb.lga_state = LGA_NO_SERVER;
	TRACE("\t lga_state = LGA_NO_SERVER");
	osaf_mutex_unlock_ordie(&lga_cb.cb_lock);

	while (p_client != NULL) {
		/* Set Client flags for all clients */
		p_client->initialized_flag = false;
		p_client->recovered_flag = false;

		/* Set stream flags for all streams in every client */
		p_stream = p_client->stream_list;
		while (p_stream != NULL) {
			p_stream->recovered_flag = false;

			p_stream = p_stream->next;
		}

		p_client = p_client->next;
	}

	TRACE_LEAVE();
}

/******************************************************************************
 * Recovery state 1
 *
 * Recover when needed. This means that when a client requests a service
 * the client and all its open streams is recovered in server.
 * Handler functions for this can be found in this section.
 * LGA_RECOVERY1: State set in MDS event handler when server up event
 *                lga_serv_recov1state_set(). Starting recovery thread
 *
 * LGA_RECOVERY2: State set in recovery thread after timeout
 *                lga_serv_recov2state_set()
 *
 * LGA_NORMAL:    State set when all clients are recovered. This is done in
 *                recovery thread when done. After setting this state the
 *                thread will exit
 ******************************************************************************/

/**
 * If previous state was LGA_NO_SERVER (headless)
 * Start the recovery2_thread. This will start timeout timer for recovery 2
 * state.
 * Set state to LGA_RECOVERY1
 */
void lga_serv_recov1state_set(void)
{
	TRACE_ENTER();

	osaf_mutex_lock_ordie(&lga_cb.cb_lock);
	if (lga_cb.lga_state != LGA_NO_SERVER) {
		/* We have not been headless. No recovery shall be done */
		TRACE("%s Previous state was not LGA_NO_SERVER lga_stat = %d",
			__FUNCTION__, lga_cb.lga_state);
		osaf_mutex_unlock_ordie(&lga_cb.cb_lock);
		goto done;
	} else {
		lga_cb.lga_state = LGA_RECOVERY1;
		TRACE("lga_state = %d (2->RECOVERY1)",
			lga_cb.lga_state);
		osaf_mutex_unlock_ordie(&lga_cb.cb_lock);
	}

	start_recovery2_thread();

done:
	TRACE_LEAVE();
	return;
}

/**
 * Recover the client and all streams in its stream list
 *  - Send an initialize request to the server
 *  - Send a stream open request with no parameters to the server
 *    for each stream.
 *  - If success mark the stream and client as recovered
 *  - If fail to initiate the client or recover one of its streams the
 *    client shall be finalized. A finalize request is sent to the server.
 *    If error return code from server it is ignored. This may happen e.g. if
 *    the client was never initialized.
 *
 * The reason for finalizing the whole client if only one stream cannot be
 * recovered is that the client will get BAD_HANDLE when requesting a service
 * for that stream. This may cause the client to initialize a new client and
 * the old client will remain as a resource leek.
 *
 * @param p_client[in] Pointer to a client record
 * @return -1 on error
 */
int recover_one_client(lga_client_hdl_rec_t *p_client)
{
	int rc = 0;
	lga_log_stream_hdl_rec_t *p_stream;

	TRACE_ENTER();

	/* This function may be called both in the recovery thread and in the
	 * client thread. A possible scenario is that the recovery 2 thread
	 * times out and calls this function in recovery mode 2 while there
	 * a client recovery is still ongoing started in mode 1. The recovery 2
	 * thread must wait until ongoing recovery is done
	 */
	osaf_mutex_lock_ordie(&lga_recov_mutex);
	/* We may have been waiting at mutex while the client was recovered
	 * so it may already been recovered.
	 */
	if (p_client->recovered_flag == true) {
		/* Client is already recovered */
		TRACE("\t Already recovered");
		goto done;
	}

	rc = initialize_one_client(p_client);
	if (rc == -1) {
		TRACE("%s initialize_one_client() Fail client Id %d",
			__FUNCTION__, p_client->lgs_client_id);
		goto done;
	}

	/* Recover all streams registered with this client */
	p_stream = p_client->stream_list;
	while (p_stream != NULL) {
		TRACE("\t Recover client=%d, stream=%d",
			p_client->lgs_client_id, p_stream->lgs_log_stream_id);
		rc = recover_one_stream(p_stream);
		if (rc == -1) {
			TRACE("%s recover_one_stream() Fail "
			"client Id %d stream Id %d", __FUNCTION__,
				p_client->lgs_client_id,
				p_stream->lgs_log_stream_id);
			goto done;
		}

		p_stream = p_stream->next;
	}

	osaf_mutex_lock_ordie(&lga_cb.cb_lock);
	p_client->recovered_flag = true;
	osaf_mutex_unlock_ordie(&lga_cb.cb_lock);

done:
	osaf_mutex_unlock_ordie(&lga_recov_mutex);

	TRACE_LEAVE();
	return rc;
}

/**
 * Delete one client
 * Wrapper for function lga_hdl_rec_del() is used (lga_utol.c)
 * This wrapper adds a control to make sure that the function cannot
 * be used by the recovery 2 thread and the client thread at the same time
 *
 * @param list_head
 * @param rm_node
 */
uint32_t delete_one_client(
	lga_client_hdl_rec_t **list_head,
	lga_client_hdl_rec_t *rm_node
	)
{
	TRACE_ENTER2();
	uint32_t ncs_rc;

	osaf_mutex_lock_ordie(&lga_recov_mutex);
	ncs_rc = lga_hdl_rec_del(list_head, rm_node);
	osaf_mutex_unlock_ordie(&lga_recov_mutex);

	TRACE_LEAVE();
	return ncs_rc;
}
