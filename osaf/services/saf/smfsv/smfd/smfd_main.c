/*      OpenSAF
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

#include <configmake.h>

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <poll.h>

#include <ncs_main_pvt.h>

#include "smfd.h"
#include "smfsv_defs.h"
#include "smfd_evt.h"

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   TYPE DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */

static smfd_cb_t _smfd_cb;
smfd_cb_t *smfd_cb = &_smfd_cb;

static int category_mask;

/* ========================================================================
 *   FUNCTION PROTOTYPES
 * ========================================================================
 */

/**
 * USR1 signal handler to activate AMF
 * @param sig
 */
static void usr1_sig_handler(int sig)
{
	/*signal(SIGUSR1, SIG_IGN); */
	if (smfd_cb->amf_hdl == 0)
		ncs_sel_obj_ind(smfd_cb->usr1_sel_obj);
}

/**
 * USR2 signal handler to enable/disable trace (toggle)
 * @param sig
 */
static void usr2_sig_handler(int sig)
{
	if (category_mask == 0) {
		category_mask = CATEGORY_ALL;
		printf("Enabling traces");
	} else {
		category_mask = 0;
		printf("Disabling traces");
	}

	if (trace_category_set(category_mask) == -1)
		printf("trace_category_set failed");
}

/****************************************************************************
 * Name          : smfd_cb_init
 *
 * Description   : This function initializes the SMFD_CB. 
 *                 
 *
 * Arguments     : smfd_cb * - Pointer to the SMFD_CB.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 smfd_cb_init(smfd_cb_t * smfd_cb)
{
	TRACE_ENTER();

	/* Assign Initial HA state */
	smfd_cb->ha_state = SMFD_HA_INIT_STATE;

	/* TODO Assign Version. Currently, hardcoded, This will change later */
	smfd_cb->smf_version.releaseCode = SMF_RELEASE_CODE;
	smfd_cb->smf_version.majorVersion = SMF_MAJOR_VERSION;
	smfd_cb->smf_version.minorVersion = SMF_MINOR_VERSION;

	smfd_cb->amf_hdl = 0;

	smfd_cb->backupCreateCmd = NULL;
	smfd_cb->bundleCheckCmd = NULL;
	smfd_cb->nodeCheckCmd = NULL;
	smfd_cb->repositoryCheckCmd = NULL;
	smfd_cb->clusterRebootCmd = NULL;

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/**
 * Initialize smfd
 * 
 * @return uns32
 */
static uns32 initialize_smfd(const char *progname)
{
	uns32 rc;
	FILE *fp = NULL;
	char path[NAME_MAX + 32];

	TRACE_ENTER();

	/* Initialize smfd control block */
	if (smfd_cb_init(smfd_cb) != NCSCC_RC_SUCCESS) {
		TRACE("smfd_cb_init FAILED");
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* Create pidfile */
	sprintf(path, PKGPIDDIR "%s.pid", basename(progname));
	if ((fp = fopen(path, "w")) == NULL) {
		LOG_ER("Could not open %s", path);
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* Write PID to it */
	if (fprintf(fp, "%d", getpid()) < 1) {
		fclose(fp);
		LOG_ER("Could not write to: %s", path);
		rc = NCSCC_RC_FAILURE;
		goto done;
	}
	fclose(fp);

	/* Create the mailbox used for communication with SMFND */
	if ((rc = m_NCS_IPC_CREATE(&smfd_cb->mbx)) != NCSCC_RC_SUCCESS) {
		TRACE("m_NCS_IPC_CREATE FAILED %d", rc);
		goto done;
	}

	/* Attach mailbox to this thread */
	if ((rc = m_NCS_IPC_ATTACH(&smfd_cb->mbx) != NCSCC_RC_SUCCESS)) {
		TRACE("m_NCS_IPC_ATTACH FAILED %d", rc);
		goto done;
	}

	/* Create a selection object for USR1 signal handling */
	if ((rc =
	     ncs_sel_obj_create(&smfd_cb->usr1_sel_obj)) != NCSCC_RC_SUCCESS) {
		LOG_ER("ncs_sel_obj_create failed");
		goto done;
	}

	/* Init mds communication */
	if ((rc = smfd_mds_init(smfd_cb)) != NCSCC_RC_SUCCESS) {
		TRACE("smfd_mds_init FAILED %d", rc);
		return rc;
	}

	/* Init campaign OI */
	if ((rc = campaign_oi_init(smfd_cb)) != NCSCC_RC_SUCCESS) {
		TRACE("campaign_oi_init FAILED %d", rc);
		return rc;
	}

	/* Check if AMF started */
	if (smfd_cb->nid_started == 0) {
		/* Started by AMF, so let's init AMF */
		if ((rc = smfd_amf_init(smfd_cb)) != NCSCC_RC_SUCCESS) {
			LOG_ER("init amf failed");
			goto done;
		}
	}

 done:
	TRACE_LEAVE();
	return (rc);
}

typedef enum {
	SMFD_AMF_FD,
	SMFD_MBX_FD,
	SMFD_COI_FD,
	SMFD_MAX_FD
} smfd_pollfd_t;

struct pollfd fds[SMFD_MAX_FD];

/**
 * Restore USR1 signal handling.
 */
uns32 smfd_amf_disconnected(smfd_cb_t * cb)
{
	fds[SMFD_AMF_FD].fd = cb->usr1_sel_obj.rmv_obj;
	cb->amf_hdl = 0;
	return NCSCC_RC_SUCCESS;
}

/**
 * Forever wait on events and process them.
 */
static void main_process(void)
{
	NCS_SEL_OBJ mbx_fd;
	SaAisErrorT error = SA_AIS_OK;

	TRACE_ENTER();

	mbx_fd = ncs_ipc_get_sel_obj(&smfd_cb->mbx);

	/* Set up all file descriptors to listen to */
	if (smfd_cb->nid_started)
		fds[SMFD_AMF_FD].fd = smfd_cb->usr1_sel_obj.rmv_obj;
	else
		fds[SMFD_AMF_FD].fd = smfd_cb->amfSelectionObject;

	fds[SMFD_AMF_FD].events = POLLIN;
	fds[SMFD_MBX_FD].fd = mbx_fd.rmv_obj;
	fds[SMFD_MBX_FD].events = POLLIN;
	fds[SMFD_COI_FD].fd = smfd_cb->campaignSelectionObject;
	fds[SMFD_COI_FD].events = POLLIN;

	while (1) {
		int ret = poll(fds, SMFD_MAX_FD, -1);

		if (ret == -1) {
			if (errno == EINTR)
				continue;

			LOG_ER("poll failed - %s", strerror(errno));
			break;
		}

		/* Process all the AMF messages */
		if (fds[SMFD_AMF_FD].revents & POLLIN) {
			if (smfd_cb->amf_hdl != 0) {
				/* dispatch all the AMF pending function */
				if ((error =
				     saAmfDispatch(smfd_cb->amf_hdl,
						   SA_DISPATCH_ALL)) !=
				    SA_AIS_OK) {
					LOG_ER("saAmfDispatch failed: %u",
					       error);
					break;
				}
			} else {
				TRACE("SIGUSR1 event rec");

				if (smfd_amf_init(smfd_cb) != NCSCC_RC_SUCCESS) {
					LOG_ER("init amf failed");
					break;
				}

				TRACE("AMF Initialization SUCCESS......");
				fds[SMFD_AMF_FD].fd =
				    smfd_cb->amfSelectionObject;
			}
		}

		/* Process all the Mail box events */
		if (fds[SMFD_MBX_FD].revents & POLLIN) {
			/* dispatch all the MBX events */
			smfd_process_mbx(&smfd_cb->mbx);
		}

		/* Process all the Imm callback events */
		if (fds[SMFD_COI_FD].revents & POLLIN) {
			if ((error =
			     saImmOiDispatch(smfd_cb->campaignOiHandle,
					     SA_DISPATCH_ALL)) != SA_AIS_OK) {
				LOG_ER("saImmOiDispatch FAILED: %u", error);
				break;
			}
		}
	}
}

/**
 * The main routine for the smfd daemon.
 * @param argc
 * @param argv
 * 
 * @return int
 */
int main(int argc, char *argv[])
{
	char *value;

	/* Determine how this process was started, by NID or AMF */
	if (getenv("SA_AMF_COMPONENT_NAME") != NULL) {
		/* AMF start */
		smfd_cb->nid_started = 0;
	} else {
		/* NID start */
		smfd_cb->nid_started = 1;
	}

	/* Initialize trace system first of all so we can see what is going on. */
	if ((value = getenv("SMFD_TRACE_PATHNAME")) != NULL) {
		if (logtrace_init("opensaf-smfd", value) != 0) {
			syslog(LOG_ERR, "logtrace_init FAILED, exiting...");
			goto done;
		}

		if ((value = getenv("SMFD_TRACE_CATEGORIES")) != NULL) {
			/* Do not care about categories now, get all */
			trace_category_set(CATEGORY_ALL);
		}
	}

	if (ncspvt_svcs_startup(argc, argv) != NCSCC_RC_SUCCESS) {
		LOG_ER("ncspvt_svcs_startup failed");
		goto done;
	}

	if (initialize_smfd(argv[0]) != NCSCC_RC_SUCCESS) {
		LOG_ER("initialize_smfd failed");
		goto done;
	}

	if (signal(SIGUSR1, usr1_sig_handler) == SIG_ERR) {
		LOG_ER("signal SIGUSR1 failed");
		goto done;
	}

	if (signal(SIGUSR2, usr2_sig_handler) == SIG_ERR) {
		LOG_ER("signal SIGUSR2 failed");
		goto done;
	}

	if (smfd_cb->nid_started == 1) {
		if (nid_notify("SMFD", NCSCC_RC_SUCCESS, NULL) !=
		    NCSCC_RC_SUCCESS)
			LOG_ER("nid_notify failed");
	}

	main_process();

	exit(EXIT_SUCCESS);
 done:
	if (smfd_cb->nid_started == 1) {
		if (nid_notify("SMFD", NCSCC_RC_FAILURE, NULL) !=
		    NCSCC_RC_SUCCESS)
			LOG_ER("nid_notify failed");
	}

	LOG_ER("SMFD failed, exiting...");
	exit(EXIT_FAILURE);
}
