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

#include <logtrace.h>
#include <daemon.h>

#include "glnd.h"

static int __init_glnd(void)
{
	NCS_LIB_REQ_INFO lib_create;
	char *trace_mask_env;
	unsigned int trace_mask;

	if ((trace_mask_env = getenv("LCKND_TRACE_CATEGORIES")) != NULL) {
		trace_mask = strtoul(trace_mask_env, NULL, 0);
		trace_category_set(trace_mask);
	}

	/* Init LIB_CREATE request for Server */
	memset(&lib_create, 0, sizeof(lib_create));
	lib_create.i_op = NCS_LIB_REQ_CREATE;
	lib_create.info.create.argc = 0;
	lib_create.info.create.argv = NULL;

	if (ncs_agents_startup() != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	/* Init GLND */
	m_NCS_DBG_PRINTF("\nGLSV:GLND:ON");
	if (glnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "glnd_lib_req FAILED\n");
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}

	return (NCSCC_RC_SUCCESS);
}

int main(int argc, char *argv[])
{
	daemonize(argc, argv);

	if (__init_glnd() != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "__init_glnd() failed");
		exit(EXIT_FAILURE);
	}

	while (1) {
		m_NCS_TASK_SLEEP(0xfffffff0);
	}

	exit(1);
}
