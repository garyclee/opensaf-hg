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

#include <ncsgl_defs.h>
#include <ncs_main_pvt.h>
#include <ncs_lib.h> /* NCS_LIB_REQ_INFO */
#include <ncssysf_sem.h>

#include <avnd_dl_api.h>

static NCSCONTEXT avnd_sem;
extern char gl_nid_svc_name[];

static void main_avnd_usr1_signal_hdlr(int sig)
{
	if (sig == SIGUSR1)
		m_NCS_SEM_GIVE(avnd_sem);
}

int main(int argc, char **argv)
{
	uns32 error;
	NCS_LIB_REQ_INFO lib_create;

	strcpy(gl_nid_svc_name, "PCAP");

	if (ncspvt_svcs_startup(argc, argv, NULL) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "ncspvt_svcs_startup failed\n");
		goto done;
	}

	lib_create.i_op = NCS_LIB_REQ_CREATE;
	lib_create.info.create.argc = argc;
	lib_create.info.create.argv = argv;

	if (avnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		fprintf(stderr, "AVND lib request failed\n");
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
	(void) nid_notify("PCAP", NCSCC_RC_FAILURE, &error);
	fprintf(stderr, "failed, exiting\n");
	exit(1);
}
