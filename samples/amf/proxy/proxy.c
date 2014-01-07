
/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2013 The OpenSAF Foundation
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

  This file contains a sample AMF proxy component. It behaves nicely and
  responds OK to every AMF request.

  It can be used as a template for making a service SA-Aware proxy component.

  Can currently only handle one proxied component

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
#include <stdbool.h>
#include <assert.h>
#include <saAmf.h>

/* AMF Handle for the proxy */
static SaAmfHandleT proxy_amf_hdl;

/* Proxy healthcheck key */
static SaAmfHealthcheckKeyT proxy_healthcheck_key = {"default", 7};

/* Current HA state of the proxy */
static SaAmfHAStateT proxy_ha_state;

/* DN of the proxy component */
static SaNameT proxy_comp_name;

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
 * Returns true if comp_name is the same as the proxy's name
 * @param comp_name
 * @return
 */
static bool is_proxy_comp_name(const SaNameT *comp_name)
{
	if (memcmp(comp_name->value, proxy_comp_name.value, SA_MAX_NAME_LENGTH) == 0)
		return true;
	else
		return false;
}

/**
 * Assigns proxy or instantiates proxied component
 * @param invocation
 * @param comp_name
 * @param ha_state
 * @param csi_desc
 */
static void csiSetCallback(SaInvocationT invocation,
						   const SaNameT *comp_name,
						   SaAmfHAStateT ha_state,
						   SaAmfCSIDescriptorT csi_desc)
{
	SaAisErrorT rc, error;
	SaAmfCSIAttributeT *attr;
	int i, status = 0;

	if (csi_desc.csiFlags == SA_AMF_CSI_ADD_ONE) {
		syslog(LOG_INFO, "csiSetCallback: '%s' ADD '%s' HAState %s",
			comp_name->value, csi_desc.csiName.value, ha_state_name[ha_state]);

		/* For debug log the CSI attributes, they could
		** define the workload characteristics */
		for (i = 0; i < csi_desc.csiAttr.number; i++) {
			attr = &csi_desc.csiAttr.attr[i];
			syslog(LOG_DEBUG, "\tname: %s, value: %s",
				attr->attrName, attr->attrValue);
		}

	} else if (csi_desc.csiFlags == SA_AMF_CSI_TARGET_ALL) {
		syslog(LOG_INFO, "csiSetCallback: '%s' CHANGE HAState to %s for all assigned CSIs",
			comp_name->value, ha_state_name[ha_state]);
	} else {
		syslog(LOG_INFO, "csiSetCallback: '%s' CHANGE HAState to %s for '%s'",
			comp_name->value, ha_state_name[ha_state], csi_desc.csiName.value);
	}

	switch (ha_state) {
	case SA_AMF_HA_ACTIVE:
		if (is_proxy_comp_name(comp_name) == false) {
			// instantiate a proxied component
			syslog(LOG_INFO, "Instantiating '%s'", comp_name->value);

			// start health checks for proxied component
			SaAmfHealthcheckKeyT key1 = {"shallow", 7};
			rc = saAmfHealthcheckStart(proxy_amf_hdl, comp_name, &key1,
				SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_RESTART);
			if (rc != SA_AIS_OK) {
				syslog(LOG_ERR, "saAmfHealthcheckStart proxied FAILED - %u", rc);
			}

			SaAmfHealthcheckKeyT key2 = {"deep", 4};
			rc = saAmfHealthcheckStart(proxy_amf_hdl, comp_name, &key2,
				SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_RESTART);
			if (rc != SA_AIS_OK) {
				syslog(LOG_ERR, "saAmfHealthcheckStart proxied FAILED - %u", rc);
			}
		}
		break;
	case SA_AMF_HA_STANDBY:
		break;
	case SA_AMF_HA_QUIESCED:
		// LOCK of a proxied SU ends up in CSI remove
		break;
	case SA_AMF_HA_QUIESCING:
		// SHUTDOWN of a proxied SU ends up in CSI remove
		break;
	default:
		syslog(LOG_ERR, "unhandled HA state %u", ha_state);
		status = -1;
		break;
	}

	if (status == 0)
		error = SA_AIS_OK;
	else
		error = SA_AIS_ERR_FAILED_OPERATION;

	rc = saAmfResponse(proxy_amf_hdl, invocation, error);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}

	if (ha_state == SA_AMF_HA_QUIESCING) {
		/* "gracefully quiescing CSI work assignment" */
		sleep(1);
		rc = saAmfCSIQuiescingComplete(proxy_amf_hdl, invocation, SA_AIS_OK);
		if (rc != SA_AIS_OK) {
			syslog(LOG_ERR, "saAmfCSIQuiescingComplete FAILED - %u", rc);
			exit(1);
		}
	}
}

/**
 * Removes assignment from proxy or terminates proxied component
 *
 * @param invocation
 * @param comp_name
 * @param csi_name
 * @param csi_flags
 */
static void csiRemoveCallback(SaInvocationT invocation,
							  const SaNameT *comp_name,
							  const SaNameT *csi_name,
							  SaAmfCSIFlagsT csi_flags)
{
	syslog(LOG_INFO, "csiRemoveCallback: '%s'", comp_name->value);

	if (is_proxy_comp_name(comp_name) == true) {
		/* Reset the HA state */
		proxy_ha_state = 0;
	} else {
		// terminate proxied component here before calling saAmfResponse
	}

	SaAisErrorT rc = saAmfResponse(proxy_amf_hdl, invocation, SA_AIS_OK);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}
}

/**
 * Checks health of proxied component
 * 
 * @param inv
 * @param comp_name
 * @param health_check_key
 */
static void healthcheckCallback(SaInvocationT inv,
								const SaNameT *comp_name,
								SaAmfHealthcheckKeyT *key)
{
	SaAisErrorT rc;

	syslog(LOG_DEBUG, "healthcheckCallback '%s', key '%s'", comp_name->value,
		key->key);

	if (is_proxy_comp_name(comp_name) == true) {
		rc = saAmfResponse(proxy_amf_hdl, inv, SA_AIS_OK);
	} else {
		/*
		 * check health of proxied component and report found errors using
		 * saAmfComponentErrorReport_4()
		 *
		 * SA_AIS_OK - The healthcheck completed successfully.
		 * SA_AIS_ERR_FAILED_OPERATION - The component failed to successfully
		 * execute the given healthcheck and has reported an error on the faulty
		 * component by invoking saAmfComponentErrorReport_4().
		 */
		rc = saAmfResponse(proxy_amf_hdl, inv, SA_AIS_OK);
	}

	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}
}

static void proxiedComponentInstantiateCallback(
		SaInvocationT invocation,
		const SaNameT *proxiedCompName)
{
	// proxied comp is non pre instantiable, should not get here
	syslog(LOG_ERR, "proxiedComponentInstantiateCallback - npi comp");
}

static void proxiedComponentCleanupCallback(
    SaInvocationT invocation,
    const SaNameT *proxiedCompName)
{
	// proxied comp is non pre instantiable, should not get here
	syslog(LOG_ERR, "proxiedComponentCleanupCallback - npi comp");
}

/**
 * Terminates proxy component
 * 
 * @param inv
 * @param comp_name
 */
static void componentTerminateCallback(SaInvocationT inv,
					  				   const SaNameT *comp_name)
{
	syslog(LOG_INFO, "componentTerminateCallback: '%s'", comp_name->value);

	// proxied are non-pre-instantiable, should only get here for the proxy
	assert(is_proxy_comp_name(comp_name) == true);

	SaAisErrorT rc = saAmfResponse(proxy_amf_hdl, inv, SA_AIS_OK);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfResponse FAILED - %u", rc);
		exit(1);
	}

	syslog(LOG_NOTICE, "exiting");
	exit(0);
}

/**
 * Creates PID file
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
 * Initializes with AMF
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
	amf_callbacks.saAmfCSISetCallback = csiSetCallback;
	amf_callbacks.saAmfCSIRemoveCallback = csiRemoveCallback;
	amf_callbacks.saAmfHealthcheckCallback = healthcheckCallback;
	amf_callbacks.saAmfComponentTerminateCallback = componentTerminateCallback;

	// must provide these even with non-pre-instantiable proxied components
	// otherwise register for proxied fails
	amf_callbacks.saAmfProxiedComponentInstantiateCallback =
			proxiedComponentInstantiateCallback;
	amf_callbacks.saAmfProxiedComponentCleanupCallback =
			proxiedComponentCleanupCallback;

	rc = saAmfInitialize(&proxy_amf_hdl, &amf_callbacks, &api_ver);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, " saAmfInitialize FAILED %u", rc);
		goto done;
	}

	rc = saAmfSelectionObjectGet(proxy_amf_hdl, amf_sel_obj);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfSelectionObjectGet FAILED %u", rc);
		goto done;
	}

	rc = saAmfComponentNameGet(proxy_amf_hdl, &proxy_comp_name);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentNameGet FAILED %u", rc);
		goto done;
	}

	rc = saAmfComponentRegister(proxy_amf_hdl, &proxy_comp_name, 0);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentRegister FAILED %u", rc);
		goto done;
	}
	
	// register proxied components
	const char *name = getenv("PROXIED_X_DN");
	SaNameT comp_name;
	comp_name.length = sprintf((char*)comp_name.value, "%s", name);
	syslog(LOG_INFO, "registering: 'X' with DN '%s'", comp_name.value);
	rc = saAmfComponentRegister(proxy_amf_hdl, &comp_name, &proxy_comp_name);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentRegister proxied FAILED %u", rc);
	}

	name = getenv("PROXIED_Y_DN");
	comp_name.length = sprintf((char*)comp_name.value, "%s", name);
	syslog(LOG_INFO, "registering: 'Y' with DN '%s'", comp_name.value);
	rc = saAmfComponentRegister(proxy_amf_hdl, &comp_name, &proxy_comp_name);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentRegister proxied FAILED %u", rc);
	}

	rc = saAmfHealthcheckStart(proxy_amf_hdl, &proxy_comp_name, &proxy_healthcheck_key,
		SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_RESTART);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfHealthcheckStart proxy FAILED - %u", rc);
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
			rc = saAmfDispatch(proxy_amf_hdl, SA_DISPATCH_ONE);
			if (rc != SA_AIS_OK) {
				syslog(LOG_ERR, "saAmfDispatch FAILED %u", rc);
				goto done;
			}
		}
	}

done:
	return EXIT_FAILURE;
}
