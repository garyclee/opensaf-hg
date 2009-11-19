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

#include <stdio.h>
#include <syslog.h>
#include <libgen.h>

#include <ncs_main_pvt.h>
#include <ncs_lib.h>
#include <ncssysf_sem.h>

#include <logtrace.h>

#include <avd_dl_api.h>
#include <avnd_dl_api.h>
#include <avm_dl_api.h>

static NCSCONTEXT avnd_sem;
extern char gl_nid_svc_name[];

static void main_avnd_usr1_signal_hdlr(int sig)
{
	if (sig == SIGUSR1)
		m_NCS_SEM_GIVE(avnd_sem);
}

/**
 * USR2 signal handler to enable/disable trace (toggle)
 * @param sig
 */
static void sigusr2_handler(int sig)
{
	static int category_mask;

	if (category_mask == 0)
		category_mask = CATEGORY_ALL;
	else
		category_mask = 0;

	(void) trace_category_set(category_mask);
}

int main(int argc, char **argv)
{
	uns32 error;
	NCS_LIB_REQ_INFO lib_create;
	char *scap_argv[] = { "", "MDS_SUBSCRIPTION_TMR_VAL=1" };
	const char *trace_file;

	if ((trace_file = getenv("SCAP_TRACE_PATHNAME")) != NULL) {
		char *p;

		if (logtrace_init(basename(argv[0]), trace_file) == 0) {
			if ((p = getenv("SCAP_TRACE_CATEGORIES")) != NULL) {
				unsigned int mask = atoi(p);
				trace_category_set(mask);
			}

			syslog(LOG_NOTICE, "trace enabled to file %s", trace_file);
		} else
			syslog(LOG_ERR, "logtrace_init FAILED for %s, tracing disabled", trace_file);
	}

	if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR) {
		syslog(LOG_ERR, "signal USR2 failed: %s", strerror(errno));
		goto done;
	}

	strcpy(gl_nid_svc_name, "SCAP");

	if (ncspvt_svcs_startup(2, scap_argv, NULL) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "ncspvt_svcs_startup failed\n");
		goto done;
	}

	lib_create.i_op = NCS_LIB_REQ_CREATE;
	lib_create.info.create.argc = argc;
	lib_create.info.create.argv = argv;

	setenv("AVD", "ON", 1);
	if (avd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "AVD lib request failed\n");
		goto done;
	}

	if (avnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "AVND lib request failed\n");
		goto done;
	}

	if (avm_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "AVM lib request failed\n");
		goto done;
	}

	signal(SIGUSR1, main_avnd_usr1_signal_hdlr);

	if ( m_NCS_SEM_CREATE(&avnd_sem) == NCSCC_RC_FAILURE) {
		fprintf(stderr, "sem create failed\n");
		goto done;
	}

	printf("%s successfully started\n", argv[0]);

	while (1) {
		NCS_LIB_REQ_INFO lib_destroy;
		m_NCS_SEM_TAKE(avnd_sem);
		memset(&lib_destroy, 0, sizeof(lib_destroy));
		lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
		avnd_lib_req(&lib_destroy);
	}

	exit(0);

done:
	(void) nid_notify("SCAP", NCSCC_RC_FAILURE, &error);
	fprintf(stderr, "failed, exiting\n");
	exit(1);
}
