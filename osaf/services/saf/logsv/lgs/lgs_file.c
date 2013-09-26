/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2013 The OpenSAF Foundation
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

#include "lgs_file.h"
#include "lgs_filehdl.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h> /* LLDTEST */

#include <logtrace.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <ncsgl_defs.h>

#include "osaf_utility.h"

#define MAX_WAITTIME_ms 500 /* ms */
#define GETTIME(x) osafassert(clock_gettime(CLOCK_REALTIME, &x) == 0);

static pthread_mutex_t lgs_ftcom_mutex;	/* For locking communication */
static pthread_cond_t request_cv;	/* File thread waiting for request */
static pthread_cond_t answer_cv;	/* API waiting for answer (timed) */

struct file_communicate {
	bool request_f;	/* True if pending request */
	bool answer_f;	/* True if pending answer */
	bool timeout_f; /* True if API has got a timeout. Thread shall not answer */
	lgsf_treq_t request_code;	/* Request code from API */
	int return_code;	/* Return code from handlers */
	size_t indata_size;
	void *indata;	/* In-parameters for handlers */
	size_t outdata_size;
	void *outdata;	/* Out data from handlers */
};

static struct file_communicate lgs_com_data = {
	.answer_f = false,
	.request_f = false,
	.request_code = LGSF_NOREQ,
	.return_code = LGSF_NORETC
};

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * Creates absolute time to use with pthread_cond_timedwait.
 * 
 * @param timeout_time[out]
 * @param timeout_ms[in] in ms
 */
static void get_timeout_time(struct timespec *timeout_time, long int timeout_ms)
{
	struct timespec start_time;
	uint64_t millisec1,millisec2;
	
	TRACE_ENTER2("LLDTEST");
	
	GETTIME(start_time);
	
	/* Convert to ms */
	millisec1 = (start_time.tv_sec * 1000) + (start_time.tv_nsec / 1000000);
	
	/* Add timeout time */
	millisec2 = millisec1+timeout_ms;
	
	/* Convert back to timespec */
	timeout_time->tv_sec = millisec2 / 1000;
	timeout_time->tv_nsec = (millisec2 % 1000) * 1000000;
	
	TRACE_LEAVE2("LLDTEST");
}

/*****************************************************************************
 * Thread handling
 *****************************************************************************/

/**
 * Thread:
 * Handle functions using file I/O
 * - Wait for request (cond_wait
 * - Handle request
 * - Return result
 * 
 * @param _init_params[in]
 * @return void
 */
static void *file_hndl_thread(void *noparam)
{
	int rc;
	int hndl_rc;
	void *inbuf;
	void *outbuf;
	uint32_t max_outsize;
	
//#define LLD_DELAY_TST
#ifdef LLD_DELAY_TST /* LLDTEST Let "file system" hang for n sec after start */
	static bool lld_start_f = true;
	const unsigned int lld_sleep_sec = 10;
#endif
	
	TRACE("LLDTEST: %s - is started",__FUNCTION__);
	
	osaf_mutex_lock_ordie(&lgs_ftcom_mutex); /* LOCK */
	while(1) {
		/* Wait for request */
		//TRACE("LLDTEST: %s - Wait for request",__FUNCTION__);
		if (lgs_com_data.request_f == false) {
			rc = pthread_cond_wait(&request_cv, &lgs_ftcom_mutex); /* -> UNLOCK -> LOCK */
			if (rc != 0) osaf_abort(rc);
		} else {
			//TRACE("LLDTEST: %s - Request recived",__FUNCTION__);

			/* Handle communication buffer */
			if (lgs_com_data.indata_size != 0) {
				inbuf = malloc(lgs_com_data.indata_size);
				memcpy(inbuf, lgs_com_data.indata, lgs_com_data.indata_size);
			} else {
				inbuf = NULL;
			}
			
			if (lgs_com_data.outdata_size != 0) {
				outbuf = malloc(lgs_com_data.outdata_size);
			} else {
				outbuf = NULL;
			}
			max_outsize = lgs_com_data.outdata_size;

			/* Handle the request.
			 * A handler is handling file operations that may 'hang'. Therefore
			 * the mutex cannot be locked since that may cause the main thread
			 * to hang.
			 * The handler is supposed to know the format of it's own in and
			 * out data. The data is given to the corresponding API that also is
			 * assumed to know the data format.
			 */
			osaf_mutex_unlock_ordie(&lgs_ftcom_mutex); /* UNLOCK */
		
#ifdef LLD_DELAY_TST /* LLDTEST Wait first time thread is used */
			if (lld_start_f == true) {
				lld_start_f = false;
				TRACE("LLDTEST: file_hndl_thread sleeping");
				sleep(lld_sleep_sec);
				goto lld_wait_end;
			}
#endif
			
			switch (lgs_com_data.request_code)	{
			case LGSF_FILEOPEN:
				TRACE("LLDTEST: %s - LGSF_FILEOPEN received",__FUNCTION__);
				hndl_rc = fileopen_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_FILECLOSE:
				TRACE("LLDTEST: %s - LGSF_FILECLOSE received",__FUNCTION__);
				hndl_rc = fileclose_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_DELETE_FILE:
				TRACE("LLDTEST: %s - LGSF_DELETE_FILE received",__FUNCTION__);
				hndl_rc = delete_file_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_GET_NUM_LOGFILES:
				TRACE("LLDTEST: %s - LGSF_GET_NUM_LOGFILES received",__FUNCTION__);
				hndl_rc = get_number_of_log_files_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_MAKELOGDIR:
				TRACE("LLDTEST: %s - LGSF_MAKELOGDIR received",__FUNCTION__);
				hndl_rc = make_log_dir_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_WRITELOGREC:
				TRACE("LLDTEST: %s - LGSF_WRITELOGREC received",__FUNCTION__);
				hndl_rc = write_log_record_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_CREATECFGFILE:
				TRACE("LLDTEST: %s - LGSF_CREATECFGFILE received",__FUNCTION__);
				hndl_rc = create_config_file_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_RENAME_FILE:
				TRACE("LLDTEST: %s - LGSF_RENAME_FILE received",__FUNCTION__);
				hndl_rc = rename_file_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_CHECKPATH:
				TRACE("LLDTEST: %s - LGSF_CHECKPATH received",__FUNCTION__);
				hndl_rc = check_path_exists_hdl(inbuf, outbuf, max_outsize);
				break;
			case LGSF_CHECKDIR:
				TRACE("LLDTEST: %s - LGSF_CHECKDIR received",__FUNCTION__);
				hndl_rc = path_is_writeable_dir_hdl(inbuf, outbuf, max_outsize);
			default:
				break;
			}
			
#ifdef LLD_DELAY_TST
			lld_wait_end:
				TRACE("LLDTEST: file_hndl_thread is awake!");
#endif
			
			osaf_mutex_lock_ordie(&lgs_ftcom_mutex); /* LOCK */

			/* Handle answer flag and return data
			 * Note: This must be done after handler is done (handler may hang)
			 */
			//TRACE("LLDTEST: %s - Handle return data",__FUNCTION__);
			lgs_com_data.request_f = false; /* Prepare to take a new request */
			lgs_com_data.request_code = LGSF_NOREQ;
			free(inbuf);
			
			/* The following cannot be done if the API has timed out */
			if (lgs_com_data.timeout_f == false) {
				lgs_com_data.answer_f = true;
				lgs_com_data.return_code = hndl_rc;
				if (lgs_com_data.outdata_size != 0) {
					memcpy(lgs_com_data.outdata, outbuf, lgs_com_data.outdata_size);
					free(outbuf);
				} else {
					lgs_com_data.outdata = NULL;
				}

				/* Signal the API function that we are done */
				//TRACE("LLDTEST: %s - Signal answer",__FUNCTION__);
				rc = pthread_cond_signal(&answer_cv); 
				if (rc != 0) osaf_abort(rc);
				//TRACE("LLDTEST: %s - Answer signaled",__FUNCTION__);
			} else {
				free(outbuf);
			}
		}	
	} /* End while(1) */
}

/**
 * Start the file handling thread.
 * 
 * @param 
 */
static int start_file_thread(void)
{
	int rc = 0;
	int tbd_inpar=1;
	pthread_t thread;

	TRACE_ENTER2("LLDTEST");

	/* Init thread handling */
	rc = pthread_mutex_init(&lgs_ftcom_mutex, NULL);
	if (rc != 0) {
		LOG_ER("pthread_mutex_init fail %s",strerror(errno));
		goto done;
	}
	pthread_cond_init (&request_cv,NULL);
	if (rc != 0) {
		LOG_ER("pthread_cond_init fail %s",strerror(errno));
		goto done;
	}
	pthread_cond_init (&answer_cv,NULL);
	if (rc != 0) {
		LOG_ER("pthread_cond_init fail %s",strerror(errno));
		goto done;
	}

	/* Create thread. 
	 */
	rc = pthread_create(&thread, NULL, file_hndl_thread, (void *) &tbd_inpar);
	if (rc != 0) {
		LOG_ER("pthread_create fail %s",strerror(errno));
		goto done;
	}

done:
	TRACE_LEAVE2("LLDTEST rc=%d",rc);
	return rc;
}

/**
 * Initialize threaded file handling
 */
uint32_t lgs_file_init(void)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	
	TRACE_ENTER2("LLDTEST");
	
	if (start_file_thread() != 0) {
		rc = NCSCC_RC_FAILURE;
	}
	
	TRACE_LEAVE2("LLDTEST rc=%d",rc);
	return rc;
}

/**
 * Generic file API handler
 * Handles everything that is generic thread handling for the APIs
 * 
 * @param apipar_in
 * @return A lgsf return code
 */
lgsf_retcode_t log_file_api(lgsf_apipar_t *apipar_in)
{
	lgsf_retcode_t api_rc = LGSF_SUCESS;
	int rc = 0;
	struct timespec timeout_time, start_time, end_time;
	uint64_t stime_ms, etime_ms, dtime_ms;
	
	TRACE_ENTER2("LLDTEST");
	
	osaf_mutex_lock_ordie(&lgs_ftcom_mutex); /* LOCK */
	
	/* If busy_f is true the file thread is hanging. In this case don't send
	 * a request. Make several attempts?
	 */
	if (lgs_com_data.request_f == true) {
		api_rc = LGSF_BUSY;
		TRACE("LLDTEST: %s - LGSF_BUSY",__FUNCTION__);
		goto api_exit;
	}
	
	/* Enter data for a request */
	lgs_com_data.request_code = apipar_in->req_code_in;
	if (apipar_in->data_in_size != 0) {
		lgs_com_data.indata = malloc(apipar_in->data_in_size);
		memcpy(lgs_com_data.indata, apipar_in->data_in, apipar_in->data_in_size);
	} else {
		lgs_com_data.indata = NULL;
	}
	lgs_com_data.indata_size = apipar_in->data_in_size;
	
	if (apipar_in->data_out_size != 0) {
		lgs_com_data.outdata = malloc(apipar_in->data_out_size);
	} else {
		lgs_com_data.outdata = NULL;
	}
	lgs_com_data.outdata_size = apipar_in->data_out_size;
	
	lgs_com_data.request_f = true;	/* We have a pending request */
	lgs_com_data.timeout_f = false;
	
	/* Wake up the thread */
	//TRACE("LLDTEST: Wake up thread");
	rc = pthread_cond_signal(&request_cv);
	if (rc != 0) osaf_abort(rc);
	
	/* Wait for an answer */
	//TRACE("LLDTEST: Waiting for answer");
	GETTIME(start_time);
	
	get_timeout_time(&timeout_time, MAX_WAITTIME_ms);
	
	while (lgs_com_data.answer_f == false) {
		rc = pthread_cond_timedwait(
				&answer_cv, &lgs_ftcom_mutex, &timeout_time); /* -> UNLOCK -> LOCK */
		if (rc == ETIMEDOUT) {
			TRACE("Timed out before answer");
			api_rc = LGSF_TIMEOUT;
			lgs_com_data.timeout_f = true; /* Inform thread about timeout */
			goto done;
		} else if (rc != 0) {
			TRACE("pthread wait Failed - %s",strerror(rc));
			osaf_abort(rc);
		}
	}
	
	/* We have an answer
	 * NOTE!
	 * The out-data buffer 'par_out' must be big enough to handle the
	 * 'outdata'. It is assumed that the calling function knows the format of
	 * the returned data.
	 */
	//TRACE("LLDTEST: An answer is received");
	apipar_in->hdl_ret_code_out = lgs_com_data.return_code;
	memcpy(apipar_in->data_out, lgs_com_data.outdata, lgs_com_data.outdata_size);
	//TRACE("LLDTEST: Return code for request - %d",lgs_com_data.return_code);

	GETTIME(end_time);
	stime_ms = (start_time.tv_sec * 1000) + (start_time.tv_nsec / 1000000);
	etime_ms = (end_time.tv_sec * 1000) + (end_time.tv_nsec / 1000000);
	dtime_ms = etime_ms - stime_ms;
	TRACE("LLDTEST: Time waited for answer %ld ms",dtime_ms);
	
	/* Prepare to take a new answer */
	lgs_com_data.answer_f = false;
	lgs_com_data.return_code = LGSF_NORETC;
	
done:
	/* Prepare for new request/answer cycle */
	if (lgs_com_data.indata != NULL) free(lgs_com_data.indata);
	if (lgs_com_data.outdata != NULL) free(lgs_com_data.outdata);

api_exit:
	osaf_mutex_unlock_ordie(&lgs_ftcom_mutex); /* UNLOCK */
	TRACE_LEAVE2("LLDTEST");
	return api_rc;	
}

/**
 * Return lgsf return code as a string
 * @param rc
 * @return 
 */
char *lgsf_retcode_str(lgsf_retcode_t rc)
{
	static char errstr[256];
	
	switch (rc) {
	case LGSF_SUCESS:
		return "LGSF_SUCESS";
	case LGSF_BUSY:
		return "LGSF_BUSY";
	case LGSF_TIMEOUT:
		return "LGSF_TIMEOUT";
	case LGSF_FAIL:
		return "LGSF_FAIL";
	default:
		sprintf(errstr,"Unknown lgsf_retcode %d",rc);
		return errstr;
	}
	return "dummy"; 
}
