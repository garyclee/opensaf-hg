/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2010 The OpenSAF Foundation
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
 * Author(s): Wind River Systems
 *
 */

#include <ncssysf_tsk.h>
#include <nid_api.h>

#include <logtrace.h>
#include <daemon.h>

#include "dts.h"

static int __init_dts(void)
{
	NCS_LIB_REQ_INFO lib_create;
	char *env;

	/* Init LIB_CREATE request for Server */
	memset(&lib_create, 0, sizeof(lib_create));
	lib_create.i_op = NCS_LIB_REQ_CREATE;
	lib_create.info.create.argc = 0;
	lib_create.info.create.argv = NULL;

	if (ncs_agents_startup() != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	/* DTSV default service severity level */
	env = getenv("DTS_LOG_ALL");
	if (env != NULL) {
		if (atoi(env)) {
			TRACE("\nINFO:DTS default service severity = ALL\n");
			gl_severity_filter = GLOBAL_SEVERITY_FILTER_ALL;
		}
	}

	TRACE("\nDTSV:DTS:ON");
	if (dts_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "dts_lib_req FAILED\n");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return (NCSCC_RC_SUCCESS);
}


int main(int argc, char *argv[])
{
	daemonize(argc, argv);

	if (__init_dts() != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "__init_dts() failed");
		exit(EXIT_FAILURE);
	}

	if (nid_notify("DTSV", NCSCC_RC_SUCCESS, NULL) != NCSCC_RC_SUCCESS) {
		LOG_ER("nid_notify failed");
		exit(EXIT_FAILURE);
	}

	dts_do_evts(&gl_dts_mbx);

	LOG_ER("DTS is exiting");

	exit(1);
}
