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
 * Author(s): Ericsson AB
 *            Wind River Systems
 *
 */

#include <logtrace.h>
#include <daemon.h>

#include <nid_api.h>
#include "avd.h"

static int __init_avd(void)
{
	if (ncs_agents_startup() != NCSCC_RC_SUCCESS)
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

	return (NCSCC_RC_SUCCESS);
}

int main(int argc, char *argv[])
{
	uint32_t error;
	char * ee_id = NULL;
        uint32_t node_id;

	daemonize(argc, argv);

	if (__init_avd() != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "__init_avd() failed");
		goto done;
	}

	/* should never return */
	avd_main_proc();

        node_id = m_NCS_GET_NODE_ID;
	opensaf_reboot(node_id, ee_id, "avd_main_proc exited");
	exit(1);

done:
	(void) nid_notify("AMFD", NCSCC_RC_FAILURE, &error);
	fprintf(stderr, "failed, exiting\n");
	exit(1);
}
