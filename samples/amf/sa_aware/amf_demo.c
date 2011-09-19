
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

  This file contains a sample AMF component. It behaves nicely and responds OK
  to every AMF request.
  It can be used as a template for making a service SA-Aware.

  References are made to sequence diagrams in the AMF specification B.04

******************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <syslog.h>
#include <libgen.h>
#include <signal.h>
#include <saAmf.h>

/* Some dummies in place of real service logic */
int foo_activate(void)
{
	return 0;
}

int foo_deactivate(void)
{
	return 0;
}

int foo_healthcheck(void)
{
	return 0;
}

/* AMF Handle */
static SaAmfHandleT my_amf_hdl;

/* HealthCheck Key on which ddhealthcheck is started */
static SaAmfHealthcheckKeyT my_healthcheck_key = {"AmfDemo", 7};

/* HA state of the application */
static SaAmfHAStateT my_ha_state;

/* Distinguised Name of the AMF component */
static SaNameT my_comp_name;

/* Logical HA State names for nicer logging */
static const char *ha_state_name[] =
{
	"None",
	"Active",    /* SA_AMF_HA_ACTIVE       */
	"Standby",   /* SA_AMF_HA_STANDBY      */
	"Quiesced",  /* SA_AMF_HA_QUIESCED     */
	"Quiescing"  /* SA_AMF_HA_QUIESCING    */
};

/**
 * AMF invokes this callback to assign a new workload (ADD_ONE) or
 * to change state of an already assigned workload (TARGET_ALL).
 * The callback is used for the initial assignment, as a consequence
 * of admin operations and fail/switch-over
 * 
 * See example sequence diagrams in chapter 10.
 * 
 * @param invocation
 * @param comp_name
 * @param ha_state
 * @param csi_desc
 */
static void amf_csi_set_callback(SaInvocationT invocation,
								 const SaNameT *comp_name,
								 SaAmfHAStateT ha_state,
								 SaAmfCSIDescriptorT csi_desc)
{
	SaAisErrorT rc, error;
	SaAmfCSIAttributeT *attr;
	int i, status;

	if (csi_desc.csiFlags == SA_AMF_CSI_ADD_ONE) {

		syslog(LOG_INFO, "CSI Set - add '%s' HAState %s", 
			csi_desc.csiName.value, ha_state_name[ha_state]);

		/* For debug log the CSI attributes, they could
		** define the workload characteristics */
		for (i = 0; i < csi_desc.csiAttr.number; i++) {
			attr = &csi_desc.csiAttr.attr[i];
			syslog(LOG_DEBUG, "\tname: %s, value: %s",
				attr->attrName, attr->attrValue);
		}

	} else if (csi_desc.csiFlags == SA_AMF_CSI_TARGET_ALL) {
		syslog(LOG_INFO, "CSI Set - HAState %s for all assigned CSIs", 
			ha_state_name[ha_state]);
	} else {
		syslog(LOG_INFO, "CSI Set - HAState %s for '%s'", 
			ha_state_name[ha_state], csi_desc.csiName.value);
	}

	switch (ha_state) {
	case SA_AMF_HA_ACTIVE:
		status = foo_activate();
		break;
	case SA_AMF_HA_STANDBY:
		/* 
		 * Not much to do in this simple example code
		 * For real one could open a checkpoint for reads
		 * Open a communication channel for listening
		 * etc.
		 */
		status = 0;
		break;
	case SA_AMF_HA_QUIESCED:
		/* the effect of admin op lock on SU or node or ... */
		status = foo_deactivate();
		break;
	case SA_AMF_HA_QUIESCING:
		/* the effect of admin op lock on SU or node or ... */
		status = 0;
		break;
	default:
		syslog(LOG_ERR, "unmanaged HA state %u", ha_state);
		status = -1;
		break;
	}

	if (status == 0)
		error = SA_AIS_OK;
	else
		error = SA_AIS_ERR_FAILED_OPERATION;

	rc = saAmfResponse(my_amf_hdl, invocation, error);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}

	if (ha_state == SA_AMF_HA_QUIESCING) {
		/* "gracefully quiescing CSI work assignment" */
		sleep(1);
		rc = saAmfCSIQuiescingComplete(my_amf_hdl, invocation, SA_AIS_OK);
		if (rc != SA_AIS_OK) {
			syslog(LOG_ERR, "saAmfCSIQuiescingComplete FAILED - %u", rc);
			exit(1);
		}

		rc = saAmfHAStateGet(my_amf_hdl, comp_name, &csi_desc.csiName, &my_ha_state);
		if (rc != SA_AIS_OK) {
			syslog(LOG_ERR, "saAmfHAStateGet FAILED - %u", rc);
			exit(1);
		}

		syslog(LOG_INFO, "My HA state is %s", ha_state_name[my_ha_state]);
	}
}

/**
 * AMF invokes this callback to remove a previously assigned workload.
 * As a consequence of admin lock of the SU, a CSI would first get QUIESCED
 * and then removed.
 * See Figure 44, page 405
 * 
 * @param invocation
 * @param comp_name
 * @param csi_name
 * @param csi_flags
 */
static void amf_csi_remove_callback(SaInvocationT invocation,
									const SaNameT *comp_name,
									const SaNameT *csi_name,
									SaAmfCSIFlagsT csi_flags)
{
	SaAisErrorT rc;

	if (csi_flags == SA_AMF_CSI_TARGET_ALL)
		syslog(LOG_INFO, "CSI Remove for all CSIs");
	else if (csi_flags == SA_AMF_CSI_TARGET_ONE)
		syslog(LOG_INFO, "CSI Remove for '%s'", csi_name->value);
	else
		// A non valid case, see 7.9.3
		abort();

	/* Reset the HA state */
	my_ha_state = 0;

	rc = saAmfResponse(my_amf_hdl, invocation, SA_AIS_OK);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}
}

/**
 * AMF invoked this callback periodically to assess our health.
 * 
 * @param inv
 * @param comp_name
 * @param health_check_key
 */
static void amf_healthcheck_callback(SaInvocationT inv,
									 const SaNameT *comp_name,
									 SaAmfHealthcheckKeyT *health_check_key)
{
	SaAisErrorT rc, status = SA_AIS_OK;
	static int healthcheck_count = 0;

	healthcheck_count++;

	syslog(LOG_DEBUG, "Health check %u", healthcheck_count);

	/* Check the status of our service but only if active */
	if ((my_ha_state == SA_AMF_HA_ACTIVE) && (foo_healthcheck() != 0)) {
		/* 7.8.2 - an error report should be done before returning failed op */
		rc = saAmfComponentErrorReport(my_amf_hdl, &my_comp_name, 0,
			SA_AMF_COMPONENT_RESTART, SA_NTF_IDENTIFIER_UNUSED);
		if (rc != SA_AIS_OK) {
			syslog(LOG_ERR, "saAmfComponentErrorReport FAILED - %u", rc);
			exit(1);
		}
		status = SA_AIS_ERR_FAILED_OPERATION;
	}

	rc = saAmfResponse(my_amf_hdl, inv, status);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}
}

/**
 * AMF invokes this callback as a consequence of admin operations
 * such as SU or node lock-instantiation.
 * 
 * @param inv
 * @param comp_name
 */
static void amf_comp_terminate_callback(SaInvocationT inv,
										const SaNameT *comp_name)
{
	SaAisErrorT rc;

	syslog(LOG_NOTICE, "Terminating");

	rc = saAmfResponse(my_amf_hdl, inv, SA_AIS_OK);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}

	exit(0);
}

/**
 * Create a PID file in directory
 * 
 * @param directory
 * @param filename_prefix
 */
static void create_pid_file(const char *directory, const char *filename_prefix)
{
	char path[256];
	FILE *fp;

	snprintf(path, sizeof(path), "%s/%s.pid", directory, filename_prefix);

	fp = fopen(path, "w");
	if (fp == NULL)	{
		syslog(LOG_ERR, "fopen '%s' failed: %s", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(fp, "%d\n", getpid());
	fclose(fp);
}

/**
 * Our TERM signal handler
 * @param sig
 */
static void sigterm_handler(int sig)
{
	/* Don't log in a signal handler! But we're exiting anyway... */
	syslog(LOG_NOTICE, "exiting (caught term signal)");
	exit(EXIT_SUCCESS);
}

/**
 * Initialize with AMF
 * @param amf_sel_obj [out]
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT amf_initialize(SaSelectionObjectT *amf_sel_obj)
{
	SaAisErrorT rc;
	SaAmfCallbacksT amf_callbacks = {0};
	SaVersionT api_ver =
		{.releaseCode = 'B', api_ver.majorVersion = 0x01, api_ver.minorVersion = 0x01};

	/* Initialize our callbacks */
	amf_callbacks.saAmfCSISetCallback = amf_csi_set_callback;
	amf_callbacks.saAmfCSIRemoveCallback = amf_csi_remove_callback;
	amf_callbacks.saAmfHealthcheckCallback = amf_healthcheck_callback;
	amf_callbacks.saAmfComponentTerminateCallback = amf_comp_terminate_callback;

	rc = saAmfInitialize(&my_amf_hdl, &amf_callbacks, &api_ver);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, " saAmfInitialize FAILED %u", rc);
		goto done;
	}

	rc = saAmfSelectionObjectGet(my_amf_hdl, amf_sel_obj);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfSelectionObjectGet FAILED %u", rc);
		goto done;
	}

	rc = saAmfComponentNameGet(my_amf_hdl, &my_comp_name);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentNameGet FAILED %u", rc);
		goto done;
	}

	rc = saAmfComponentRegister(my_amf_hdl, &my_comp_name, 0);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentRegister FAILED %u", rc);
		goto done;
	}
	
	rc = saAmfHealthcheckStart(my_amf_hdl, &my_comp_name, &my_healthcheck_key,
		SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_RESTART);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfHealthcheckStart FAILED - %u", rc);
		goto done;
	}
done:
	return rc;
}

int main(int argc, char **argv)
{
	SaAisErrorT rc;
	SaSelectionObjectT amf_sel_obj;
	struct pollfd fds[1];
	char *env_comp_name;

	/* Environment variable "SA_AMF_COMPONENT_NAME" exist when started by AMF */
	if ((env_comp_name = getenv("SA_AMF_COMPONENT_NAME")) == NULL) {
		fprintf(stderr, "not started by AMF exiting...\n");
		exit(EXIT_FAILURE);
	}

	/* Daemonize ourselves and detach from terminal.
	** This important since our start script will hang forever otherwise.
	** Note daemon() is not LSB but impl by libc so fairly portable...
	*/
	if (daemon(0, 0) == -1) {
		syslog(LOG_ERR, "daemon failed: %s", strerror(errno));
		goto done;
	}

	/* Install a TERM handler just to log and visualize when cleanup is called */
	if ((signal(SIGTERM, sigterm_handler)) == SIG_ERR) {
		syslog(LOG_ERR, "signal TERM failed: %s", strerror(errno));
		goto done;
	}

	/* Create a PID file which is needed by our CLC-CLI script.
	** Use AMF component name as file name so multiple instances of this
	** component can be managed by the same script.
	*/
	create_pid_file("/tmp", env_comp_name);

	/* Use syslog for logging */
	openlog(basename(argv[0]), LOG_PID, LOG_USER);

	/* Make a log to associate component name with PID */
	syslog(LOG_INFO, "'%s' started", env_comp_name);

	if (amf_initialize(&amf_sel_obj) != SA_AIS_OK)
		goto done;

	syslog(LOG_INFO, "Registered with AMF and HC started");

	fds[0].fd = amf_sel_obj;
	fds[0].events = POLLIN;

	/* Loop forever waiting for events on watched file descriptors */
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
			/* An AMF event is received, call AMF dispatch which in turn will
			 * call our installed callbacks. In context of this main thread.
			 */
			rc = saAmfDispatch(my_amf_hdl, SA_DISPATCH_ONE);
			if (rc != SA_AIS_OK) {
				syslog(LOG_ERR, "saAmfDispatch FAILED %u", rc);
				goto done;
			}
		}
	}

done:
	return EXIT_FAILURE;
}

