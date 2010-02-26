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
 * Author(s): Ericsson AB
 *
 */

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <configmake.h>

#include <ncs_main_pvt.h>
#include <rda_papi.h>

#include "immd.h"

static IMMD_CB _immd_cb;
IMMD_CB *immd_cb = &_immd_cb;

static int category_mask;

#define FD_USR1 0
#define FD_AMF 0
#define FD_MBCSV 1
#define FD_MBX 2

/* ========================================================================
 *   FUNCTION PROTOTYPES
 * ========================================================================
 */

/**
 * Callback from RDA. Post a message/event to the IMMD mailbox.
 * @param cb_hdl
 * @param cb_info
 * @param error_code
 */
static void rda_cb(uns32 cb_hdl, PCS_RDA_CB_INFO *cb_info, PCSRDA_RETURN_CODE error_code)
{
	uns32 rc;
	IMMSV_EVT *evt;

	TRACE_ENTER();

	evt = calloc(1, sizeof(IMMSV_EVT));
	if (NULL == evt) {
		LOG_ER("calloc failed");
		goto done;
	}

	evt->type = IMMSV_EVT_TYPE_IMMD;
	evt->info.immd.type = IMMD_EVT_LGA_CB;
	evt->info.immd.info.rda_info.io_role = cb_info->info.io_role;

	rc = ncs_ipc_send(&immd_cb->mbx, (NCS_IPC_MSG *)evt, MDS_SEND_PRIORITY_HIGH);
	if (rc != NCSCC_RC_SUCCESS)
		LOG_ER("IPC send failed %d", rc);

done:
	TRACE_LEAVE();
}

/**
 * USR2 signal handler to dump information from all data structures
 * @param sig
 */
static void usr2_sig_handler(int sig)
{
	int old_category_mask = category_mask;

	if (trace_category_set(CATEGORY_ALL) == -1)
		printf("IMMD: trace_category_set failed");

	TRACE("IMMD Control block information");

	if (trace_category_set(old_category_mask) == -1)
		printf("trace_category_set failed");
}

/**
 * USR1 signal is used when AMF wants instantiate us as a
 * component. Wake up the main thread so it can register with
 * AMF.
 * 
 * @param i_sig_num
 */
static void sigusr1_handler(int sig)
{
	(void)sig;
	signal(SIGUSR1, SIG_IGN);
	ncs_sel_obj_ind(immd_cb->usr1_sel_obj);
	TRACE("Got USR1 signal");
}

/**
 * Initialize immd
 * 
 * @return uns32
 */
static uns32 immd_initialize(const char *progname)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	char *immd_argv[] = { "", "MDS_SUBSCRIPTION_TMR_VAL=1" };
	const char *trace_file;

	/* Create PID file */
	{
		char path[256];
		FILE *fp;

		snprintf(path, sizeof(path), PKGPIDDIR "%s.pid", basename(progname));
		fp = fopen(path, "w");
		if (fp == NULL) {
			syslog(LOG_ERR, "fopen '%s' failed: %s", path, strerror(errno));
			exit(EXIT_FAILURE);
		}
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}

	/* Initialize trace system first of all so we can see what is going on. */
	if ((trace_file = getenv("IMMD_TRACE_PATHNAME")) != NULL) {
		if (logtrace_init(basename(progname), trace_file) != 0) {
			syslog(LOG_ERR, "logtrace_init FAILED, exiting...");
			exit(EXIT_FAILURE);
		}

		if (getenv("IMMD_TRACE_CATEGORIES") != NULL) {
			/* Do not care about categories now, get all */
			trace_category_set(CATEGORY_ALL);
		}
	}

	TRACE_ENTER();

	if (ncspvt_svcs_startup(2, immd_argv) != NCSCC_RC_SUCCESS) {
		LOG_ER("ncspvt_svcs_startup failed");
		goto done;
	}

	if ((rc = rda_get_role(&immd_cb->ha_state)) != NCSCC_RC_SUCCESS) {
		LOG_ER("rda_get_role FAILED");
		goto done;
	}

	if ((rc = rda_register_callback(0, rda_cb)) != NCSCC_RC_SUCCESS) {
		LOG_ER("rda_register_callback FAILED %u", rc);
		goto done;
	}

	/* Init the EDU Handle */
	m_NCS_EDU_HDL_INIT(&immd_cb->edu_hdl);

	if ((rc = immd_cb_db_init(immd_cb)) != NCSCC_RC_SUCCESS) {
		LOG_ER("immd_cb_db_init FAILED");
		goto done;
	}

	/* Create a mail box */
	if ((rc = m_NCS_IPC_CREATE(&immd_cb->mbx)) != NCSCC_RC_SUCCESS) {
		LOG_ER("m_NCS_IPC_CREATE FAILED");
		goto done;
	}

	/* Attach the IPC to mail box */
	if ((rc = m_NCS_IPC_ATTACH(&immd_cb->mbx)) != NCSCC_RC_SUCCESS) {
		LOG_ER("m_NCS_IPC_ATTACH FAILED");
		goto done;
	}

	if ((rc = immd_mds_register(immd_cb)) != NCSCC_RC_SUCCESS) {
		LOG_ER("immd_mds_register FAILED %d", rc);
		goto done;
	}

	/* Initialise with the MBCSV service  */
	if ((rc = immd_mbcsv_register(immd_cb)) != NCSCC_RC_SUCCESS) {
		LOG_ER("immd_mbcsv_register FAILED %d", rc);
		goto done;
	}

	if ((rc = immd_mbcsv_chgrole(immd_cb)) != NCSCC_RC_SUCCESS) {
		LOG_ER("immd_mbcsv_chgrole FAILED %d", rc);
		goto done;
	}

	/* Create a selection object */
	if ((rc = ncs_sel_obj_create(&immd_cb->usr1_sel_obj)) != NCSCC_RC_SUCCESS) {
		LOG_ER("ncs_sel_obj_create failed");
		goto done;
	}

	/*
	 ** Initialize a signal handler that will use the selection object.
	 ** The signal is sent from our script when AMF does instantiate.
	 */
	if ((signal(SIGUSR1, sigusr1_handler)) == SIG_ERR) {
		LOG_ER("signal USR1 failed: %s", strerror(errno));
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	if (signal(SIGUSR2, usr2_sig_handler) == SIG_ERR) {
		LOG_ER("signal USR2 failed: %s", strerror(errno));
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	syslog(LOG_INFO, "Initialization Success, role %s",
	       (immd_cb->ha_state == SA_AMF_HA_ACTIVE) ? "ACTIVE" : "STANDBY");

 done:
	if (nid_notify("IMMD", rc, NULL) != NCSCC_RC_SUCCESS) {
		LOG_ER("nid_notify failed");
		rc = NCSCC_RC_FAILURE;
	}

	TRACE_LEAVE();
	return rc;
}

/**
 * The main routine for the IMM director daemon.
 * @param argc
 * @param argv
 * 
 * @return int
 */
int main(int argc, char *argv[])
{
	SaAisErrorT error;
	NCS_SEL_OBJ mbx_fd;
	struct pollfd fds[3];

	if (immd_initialize(argv[0]) != NCSCC_RC_SUCCESS) {
		TRACE("initialize_immd failed");
		goto done;
	}

	/* Get file descriptor for mailbox */
	mbx_fd = ncs_ipc_get_sel_obj(&immd_cb->mbx);

	/* Set up all file descriptors to listen to */
	fds[FD_USR1].fd = immd_cb->usr1_sel_obj.rmv_obj;
	fds[FD_USR1].events = POLLIN;
	fds[FD_MBCSV].fd = immd_cb->mbcsv_sel_obj;
	fds[FD_MBCSV].events = POLLIN;
	fds[FD_MBX].fd = mbx_fd.rmv_obj;
	fds[FD_MBX].events = POLLIN;

	while (1) {
		int ret = poll(fds, 3, -1);

		if (ret == -1) {
			if (errno == EINTR)
				continue;

			LOG_ER("poll failed - %s", strerror(errno));
			break;
		}

		if (fds[FD_MBCSV].revents & POLLIN) {
			if (immd_mbcsv_dispatch(immd_cb) != NCSCC_RC_SUCCESS) {
				LOG_ER("MBCSv Dispatch Failed");
				break;
			}
		}

		if (fds[FD_MBX].revents & POLLIN)
			immd_process_evt();

		if (fds[FD_AMF].revents & POLLIN) {
			if (immd_cb->amf_hdl != 0) {
				error = saAmfDispatch(immd_cb->amf_hdl, SA_DISPATCH_ALL);
				if (error != SA_AIS_OK) {
					LOG_ER("saAmfDispatch failed: %u", error);
					break;
				}
			} else {
				TRACE("SIGUSR1 event rec");
				ncs_sel_obj_rmv_ind(immd_cb->usr1_sel_obj, TRUE, TRUE);
				ncs_sel_obj_destroy(immd_cb->usr1_sel_obj);

				if (immd_amf_init(immd_cb) != NCSCC_RC_SUCCESS)
					break;

				TRACE("AMF Initialization SUCCESS......");
				fds[FD_AMF].fd = immd_cb->amf_sel_obj;
			}
		}
	}

 done:
	LOG_ER("Failed, exiting...");
	TRACE_LEAVE();
	exit(1);
}
