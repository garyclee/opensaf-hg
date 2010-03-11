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

#include <syslog.h>
#include <libgen.h>

#include <ncs_main_pvt.h>

#include <logtrace.h>
#include <avd.h>

static unsigned int trace_mask;
static const char *trace_file;

/**
 * USR2 signal handler to enable/disable trace (toggle)
 * @param sig
 */
static void sigusr2_handler(int sig)
{
	if (trace_mask == 0) {
		trace_mask = CATEGORY_ALL;
		syslog(LOG_NOTICE, "trace enabled to file %s, mask %x", trace_file, trace_mask);
	}
	else {
		LOG_NO("disabling trace...");
		trace_mask = 0;
	}

	(void) trace_category_set(trace_mask);
}

int main(int argc, char **argv)
{
	uns32 error;

	if ((trace_file = getenv("TRACEFILE")) != NULL) {
		if (logtrace_init(basename(argv[0]), trace_file) == 0) {
			char *p;

			if ((p = getenv("AMFD_TRACE_CATEGORIES")) != NULL) {
				trace_mask = strtoul(p, NULL, 0);
				trace_category_set(trace_mask);
				LOG_NO("trace enabled to file %s, mask %x", trace_file, trace_mask);
			}
		} else
			syslog(LOG_ERR, "logtrace_init FAILED for %s, tracing disabled", trace_file);
	}

	if (ncspvt_svcs_startup() != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "ncspvt_svcs_startup failed");
		goto done;
	}

	if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR) {
		syslog(LOG_ERR, "signal USR2 failed: %s", strerror(errno));
		goto done;
	}

	/* should never return */
	avd_main_proc();

	ncs_reboot("avd_main_proc exited");
	exit(1);

done:
	(void) nid_notify("AMFD", NCSCC_RC_FAILURE, &error);
	fprintf(stderr, "failed, exiting\n");
	exit(1);
}
