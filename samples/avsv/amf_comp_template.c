
/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2009 The OpenSAF Foundation
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
 * Author(s): Ericsson
 */

/*****************************************************************************

  DESCRIPTION:

  This file contains the AMF interface routines...

******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>
#include <syslog.h>
#include <libgen.h>
#include <signal.h>

#include <saAmf.h>


/* AMF Handle */
static SaAmfHandleT gl_amf_hdl = 0;

/* Component Name (provided by AMF) */
static SaNameT gl_comp_name;

/* HealthCheck Key on which healthcheck is started */
static SaAmfHealthcheckKeyT gl_healthcheck_key = {"AmfDemo", 7};

/* HA state of the application */
static SaAmfHAStateT gl_ha_state = 0;

/* Canned strings for HA State */
static const char *ha_state_str[] =
{
	"None",
	"Active",    /* SA_AMF_HA_ACTIVE       */
	"Standby",   /* SA_AMF_HA_STANDBY      */
	"Quiesced",  /* SA_AMF_HA_QUIESCED     */
	"Quiescing"  /* SA_AMF_HA_QUIESCING    */
};

/* Canned strings for CSI Flags */
static const char *csi_flag_str[] =
{
	"None",
	"Add One",    /* SA_AMF_CSI_ADD_ONE    */
	"Target One", /* SA_AMF_CSI_TARGET_ONE */
	"None",
	"Target All", /* SA_AMF_CSI_TARGET_ALL */
};

/**
 * 
 * @param inv
 * @param comp_name
 * @param ha_state
 * @param csi_desc
 */
static void amf_csi_set_callback(SaInvocationT inv, const SaNameT *comp_name,
	SaAmfHAStateT ha_state, SaAmfCSIDescriptorT  csi_desc)
{
	SaAisErrorT rc;
	SaAmfHAStateT my_ha_state;

	syslog(LOG_INFO, " Dispatched 'CSI Set' in '%s' CSIName: '%s' HAState: %s CSIFlags: %s", 
		comp_name->value, csi_desc.csiName.value, ha_state_str[ha_state],
		csi_flag_str[csi_desc.csiFlags]);

	gl_ha_state = ha_state;

	rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
	if ( SA_AIS_OK != rc ) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}

	if (csi_desc.csiFlags & SA_AMF_CSI_ADD_ONE) {
		SaAmfCSIAttributeT *attr;
		int i;
		for (i = 0; i < csi_desc.csiAttr.number; i++) {
			attr = &csi_desc.csiAttr.attr[i];
			syslog(LOG_DEBUG, "\tname: %s, value: %s", attr->attrName, attr->attrValue);
		}
	}

	if (ha_state == SA_AMF_HA_QUIESCING) {
		/* "gracefully quiescing CSI work assignment" */
		sleep(1);
		rc = saAmfCSIQuiescingComplete(gl_amf_hdl, inv, SA_AIS_OK);
		if ( SA_AIS_OK != rc ) {
			syslog(LOG_ERR, "saAmfCSIQuiescingComplete FAILED - %u", rc);
			exit(1);
		}

		rc = saAmfHAStateGet(gl_amf_hdl, &gl_comp_name, &csi_desc.csiName, &my_ha_state);
		if ( SA_AIS_OK != rc ) {
			syslog(LOG_ERR, "saAmfHAStateGet FAILED - %u", rc);
			exit(1);
		}

		syslog(LOG_INFO, "My HA state is %s", ha_state_str[my_ha_state]);
	}
}

/**
 * 
 * @param inv
 * @param comp_name
 * @param csi_name
 * @param csi_flags
 */
static void amf_csi_remove_callback(SaInvocationT inv, const SaNameT *comp_name,
	const SaNameT  *csi_name, SaAmfCSIFlagsT csi_flags)
{
	SaAisErrorT rc;

	syslog(LOG_INFO, "Dispatched 'CSI Remove' in '%s' CSI: '%s' CSIFlags: %s",
	       comp_name->value, csi_name->value, csi_flag_str[csi_flags]);

	/* Reset the ha state */
	gl_ha_state = 0;

	rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
	if ( SA_AIS_OK != rc ) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}
}

/**
 * 
 * @param inv
 * @param comp_name
 * @param health_check_key
 */
static void amf_healthcheck_callback(SaInvocationT        inv, 
				   const SaNameT        *comp_name,
				   SaAmfHealthcheckKeyT *health_check_key)
{
	SaAisErrorT rc;
	static int healthcheck_count = 0;

	healthcheck_count++;

	syslog(LOG_DEBUG, "Dispatched healthCheck %u in '%s'", 
		healthcheck_count, comp_name->value);

	rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
	if ( SA_AIS_OK != rc ) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}
}

/**
 * 
 * @param inv
 * @param comp_name
 */
static void amf_comp_terminate_callback(SaInvocationT inv, const SaNameT *comp_name)
{
	SaAisErrorT rc;

	syslog(LOG_NOTICE, "Dispatched 'Component Terminate' in '%s'", 
	       comp_name->value);

	rc = saAmfResponse(gl_amf_hdl, inv, SA_AIS_OK);
	if ( SA_AIS_OK != rc ) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}

	exit(0);
}

static void create_pid_file(const char *filename_prefix)
{
	char path[256];
	FILE *fp;
	
	snprintf(path, sizeof(path), "/var/run/%s.pid", filename_prefix);
	fp = fopen(path, "w");
	if (fp == NULL)	{
		syslog(LOG_ERR, "fopen failed: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(fp, "%d\n", getpid());
	fclose(fp);
}

static void sigterm_handler(int sig)
{
	syslog(LOG_NOTICE, "exiting");
	exit(EXIT_SUCCESS);
}

/**
 * 
 * @param argc
 * @param argv
 * 
 * @return int
 */
int main(int argc, char **argv)
{
	SaAmfCallbacksT    amf_callbacks;
	SaVersionT         ver = {.releaseCode = 'B', ver.majorVersion = 0x01, ver.minorVersion = 0x01};
	SaAisErrorT        rc;
	SaSelectionObjectT amf_sel_obj;
	struct pollfd fds[1];
	char *comp_name = getenv("SA_AMF_COMPONENT_NAME");

	if ((signal(SIGTERM, sigterm_handler)) == SIG_ERR) {
		syslog(LOG_ERR, "signal TERM failed: %s", strerror(errno));
		goto done;
	}

	create_pid_file(comp_name);

	/* Use syslog for logging. */
	openlog(basename(argv[0]), LOG_PID, LOG_USER);

	syslog(LOG_INFO, "'%s' started", comp_name);

	memset(&amf_callbacks, 0, sizeof(SaAmfCallbacksT));
	amf_callbacks.saAmfCSISetCallback = amf_csi_set_callback;
	amf_callbacks.saAmfCSIRemoveCallback = amf_csi_remove_callback;
	amf_callbacks.saAmfHealthcheckCallback = amf_healthcheck_callback;
	amf_callbacks.saAmfComponentTerminateCallback = amf_comp_terminate_callback;

	rc = saAmfInitialize(&gl_amf_hdl, &amf_callbacks, &ver);
	if (SA_AIS_OK != rc) {
		syslog(LOG_ERR, " saAmfInitialize FAILED %u", rc);
		goto done;
	}

	rc = saAmfSelectionObjectGet(gl_amf_hdl, &amf_sel_obj);
	if (SA_AIS_OK != rc) {
		syslog(LOG_ERR, "saAmfSelectionObjectGet FAILED %u", rc);
		goto done;
	}

	rc = saAmfComponentNameGet(gl_amf_hdl, &gl_comp_name);
	if (SA_AIS_OK != rc) {
		syslog(LOG_ERR, "saAmfComponentNameGet FAILED %u", rc);
		goto done;
	}

	rc = saAmfComponentRegister(gl_amf_hdl, &gl_comp_name, 0);
	if (SA_AIS_OK != rc) {
		syslog(LOG_ERR, "saAmfComponentRegister FAILED %u", rc);
		goto done;
	}
	
	rc = saAmfHealthcheckStart(gl_amf_hdl, &gl_comp_name, &gl_healthcheck_key,
		SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_RESTART);
	if ( SA_AIS_OK != rc ) {
		syslog(LOG_ERR, "saAmfHealthcheckStart FAILED - %u", rc);
		goto done;
	}

	syslog(LOG_INFO, "'%s' registered with AMF", gl_comp_name.value);

	fds[0].fd = amf_sel_obj;
	fds[0].events = POLLIN;

	while (1) {
		int res = poll(fds, 1, -1);

		if (res == -1) {
			if (errno == EINTR)
				continue;
			else {
				syslog(LOG_ERR, "poll FAILED - %s", strerror(errno));
				goto done;
			}
		}

		if (fds[0].revents & POLLIN) {
			rc = saAmfDispatch(gl_amf_hdl, SA_DISPATCH_ONE);
			if (SA_AIS_OK != rc) {
				syslog(LOG_ERR, "saAmfDispatch FAILED %u", rc);
				goto done;
			}
		}
	}

done:
	return -1;
}

