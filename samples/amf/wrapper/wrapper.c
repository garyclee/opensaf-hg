
/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2011 The OpenSAF Foundation
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

  This file contains a generic AMF wrapper component. Its functionality is
  configured using environment variables.

  An AMF wrapper component "encapsulate the legacy software (hardware) into an
  SA-aware component." This implementation manages the life cycle and optionally
  performs monitoring if configured to.

******************************************************************************
*/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <syslog.h>
#include <libgen.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include <saAmf.h>

/* AMF Handle */
static SaAmfHandleT my_amf_hdl;

/* HealthCheck Key on which healthcheck is started */
static SaAmfHealthcheckKeyT my_healthcheck_key = {"Wrapper", 7};

/* HA state of the application */
static SaAmfHAStateT my_ha_state;

/* Distinguised Name of the AMF component */
static SaNameT my_comp_name;

static const char *start_script;
static const char *stop_script;
static const char *health_script;
static const char *pidfile;

/* Logical HA State names for nicer logging */
static const char *ha_state_name[] =
{
	"None",
	"Active",    /* SA_AMF_HA_ACTIVE       */
	"Standby",   /* SA_AMF_HA_STANDBY      */
	"Quiesced",  /* SA_AMF_HA_QUIESCED     */
	"Quiescing"  /* SA_AMF_HA_QUIESCING    */
};

static pid_t pid;

static int exec_command(const char *command)
{
	static unsigned long long cnt;
	int status;

	// cnt added to avoid the "repeated message reduction" syslog functionality
	// that can make it harder to debug a system.
	syslog(LOG_INFO, "(%llu) Executing '%s'", cnt++, command);

	status = system(command);
	if (status == -1) {
		syslog(LOG_ERR, "system command for '%s' FAILED", command);
		return status;
	}

	status = WEXITSTATUS(status);
	if (status != 0)
		syslog(LOG_NOTICE, "'%s' FAILED (%u)", command, status);

	return status;
}

static pid_t getpidfromfile(const char *pidfile, bool waitforfile)
{
        FILE *f;
        pid_t pid;

        assert(pidfile);

retry:
        f = fopen(pidfile, "r");
        if (f == NULL) {
                if ((errno == ENOENT) && waitforfile) {
                        sleep(1);
                        goto retry;
                }

                syslog(LOG_ERR, "could not open file %s - %s", pidfile, strerror(errno));
                exit(1);
        }

        if (fscanf(f, "%d", &pid) == 0) {
                syslog(LOG_ERR, "could not read PID from file %s", pidfile);
                exit(1);
        }

        if (fclose(f) != 0) {
                syslog(LOG_ERR, "could not close file");
                exit(1);
        }

        return pid;
}

static SaAisErrorT service_start(void)
{
	SaAisErrorT rc;
	int status;

	status = exec_command(start_script);

	if (status == 0) {
		rc = saAmfHealthcheckStart(my_amf_hdl, &my_comp_name, &my_healthcheck_key,
			SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_RESTART);
		if (rc != SA_AIS_OK) {
			syslog(LOG_ERR, "service_start: saAmfHealthcheckStart FAILED (%u)", rc);
			return SA_AIS_ERR_FAILED_OPERATION;
		}

		if (pidfile) {
			SaAmfRecommendedRecoveryT recrec = SA_AMF_NO_RECOMMENDATION;
			SaInt32T descendentsTreeDepth = 0;
			SaAmfPmErrorsT pmErr = SA_AMF_PM_ZERO_EXIT | SA_AMF_PM_NON_ZERO_EXIT;

			pid = getpidfromfile(pidfile, true);

			syslog(LOG_INFO, "Starting supervision of PID %u", pid);
			rc = saAmfPmStart(my_amf_hdl, &my_comp_name, pid, descendentsTreeDepth, pmErr, recrec);
			if (SA_AIS_OK != rc) {
				syslog(LOG_ERR, "service_start: saAmfPmStart FAILED (%u)", rc);
				return SA_AIS_ERR_FAILED_OPERATION;
			}
		}
	}

	if (status == 0)
		return SA_AIS_OK;
	else
		return SA_AIS_ERR_FAILED_OPERATION;
}

static SaAisErrorT service_stop(void)
{
	SaAisErrorT rc;
	int status;

	if (pidfile) {
		SaAmfPmErrorsT pmErr = SA_AMF_PM_ZERO_EXIT | SA_AMF_PM_NON_ZERO_EXIT;

		rc = saAmfPmStop(my_amf_hdl, &my_comp_name, SA_AMF_PM_PROC, pid, pmErr);
		if ((SA_AIS_OK != rc) && (SA_AIS_ERR_NOT_EXIST != rc)) {
			syslog(LOG_ERR, "saAmfPmStop FAILED (%u)", rc);
		}
	}

	rc = saAmfHealthcheckStop(my_amf_hdl, &my_comp_name, &my_healthcheck_key);
	if ((rc != SA_AIS_OK) && (rc != SA_AIS_ERR_NOT_EXIST)) {
		syslog(LOG_ERR, "saAmfHealthcheckStop FAILED (%u)", rc);
	}

	status = exec_command(stop_script);
	if (status == 0)
		return SA_AIS_OK;
	else
		return SA_AIS_ERR_FAILED_OPERATION;
}

static void csi_set_callback(SaInvocationT invocation,
							 const SaNameT *comp_name,
							 SaAmfHAStateT ha_state,
							 SaAmfCSIDescriptorT csi_desc)
{
	SaAisErrorT rc, status = SA_AIS_OK;
	SaAmfCSIAttributeT *attr;
	int i;

	if (csi_desc.csiFlags == SA_AMF_CSI_ADD_ONE) {

		syslog(LOG_DEBUG, "CSI Set - add '%s' HAState %s", 
			csi_desc.csiName.value, ha_state_name[ha_state]);

		for (i = 0; i < csi_desc.csiAttr.number; i++) {
			attr = &csi_desc.csiAttr.attr[i];
			syslog(LOG_DEBUG, "   name: %s, value: %s", attr->attrName, attr->attrValue);
			setenv((char*)attr->attrName, (char*)attr->attrValue, 1);
		}

	} else {
		assert(csi_desc.csiFlags == SA_AMF_CSI_TARGET_ALL);
		syslog(LOG_DEBUG, "CSI Set - HAState %s", ha_state_name[ha_state]);
	}

	switch (ha_state) {
	case SA_AMF_HA_ACTIVE:
		status = service_start();
		break;
	case SA_AMF_HA_STANDBY:
		break;
	case SA_AMF_HA_QUIESCED:
		status = service_stop();
		break;
	case SA_AMF_HA_QUIESCING:
		break;
	default:
		syslog(LOG_ERR, "CSI Set: unknown HA state (%u)", ha_state);
		exit(1);
		break;
	}

	my_ha_state = ha_state;

	rc = saAmfResponse(my_amf_hdl, invocation, status);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "CSI Set: saAmfResponse FAILED (%u)", rc);
		exit(1);
	}

	if (ha_state == SA_AMF_HA_QUIESCING) {
		status = service_stop();
		rc = saAmfCSIQuiescingComplete(my_amf_hdl, invocation, status);
		if (rc != SA_AIS_OK) {
			syslog(LOG_ERR, "CSI Set: saAmfCSIQuiescingComplete FAILED (%u)", rc);
			exit(1);
		}
	}
}

static void csi_remove_callback(SaInvocationT invocation,
								const SaNameT *comp_name,
								const SaNameT *csi_name,
								SaAmfCSIFlagsT csi_flags)
{
	SaAisErrorT rc, status = SA_AIS_OK;

	syslog(LOG_DEBUG, "CSI Remove callback");

	assert(csi_flags == SA_AMF_CSI_TARGET_ALL);

	if (my_ha_state == SA_AMF_HA_ACTIVE)
		status = service_stop();

	/* Reset HA state */
	my_ha_state = 0;

	rc = saAmfResponse(my_amf_hdl, invocation, status);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "CSI remove callback: saAmfResponse FAILED (%u)", rc);
		exit(1);
	}
}

static void healthcheck_callback(SaInvocationT inv,
								 const SaNameT *comp_name,
								 SaAmfHealthcheckKeyT *health_check_key)
{
	int status;
	SaAisErrorT rc = SA_AIS_OK;

	status = exec_command(health_script);
	if (status != 0) {
		rc = saAmfComponentErrorReport(my_amf_hdl, &my_comp_name, 0,
			SA_AMF_NO_RECOMMENDATION, SA_NTF_IDENTIFIER_UNUSED);

		if (rc != SA_AIS_OK) {
			syslog(LOG_ERR, "HC callback: saAmfComponentErrorReport FAILED (%u)", rc);
			exit(1);
		}

		rc = SA_AIS_ERR_FAILED_OPERATION;
	}

	rc = saAmfResponse(my_amf_hdl, inv, rc);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "HC callback: saAmfResponse FAILED (%u)", rc);
		exit(1);
	}
}

static void terminate_callback(SaInvocationT inv, const SaNameT *comp_name)
{
	syslog(LOG_NOTICE, "Terminating");
	exit(0);
}

static void create_pid_file(void)
{
	FILE *fp;
	char *path;

	path = getenv("WRAPPERPIDFILE");
	if (path == NULL) {
		syslog(LOG_ERR, "Variable WRAPPERPIDFILE missing");
		exit(EXIT_FAILURE);
	}

	fp = fopen(path, "w");
	if (fp == NULL)	{
		syslog(LOG_ERR, "fopen '%s' failed: %s", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(fp, "%d\n", getpid());
	fclose(fp);
}

static SaAisErrorT amf_initialize(SaSelectionObjectT *amf_sel_obj)
{
	SaAisErrorT rc;
	SaAmfCallbacksT amf_callbacks = {0};
	SaVersionT api_ver =
		{.releaseCode = 'B', api_ver.majorVersion = 0x01, api_ver.minorVersion = 0x01};

	amf_callbacks.saAmfCSISetCallback = csi_set_callback;
	amf_callbacks.saAmfCSIRemoveCallback = csi_remove_callback;
	amf_callbacks.saAmfHealthcheckCallback = healthcheck_callback;
	amf_callbacks.saAmfComponentTerminateCallback = terminate_callback;

	rc = saAmfInitialize(&my_amf_hdl, &amf_callbacks, &api_ver);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, " saAmfInitialize FAILED (%u)", rc);
		goto done;
	}

	rc = saAmfSelectionObjectGet(my_amf_hdl, amf_sel_obj);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfSelectionObjectGet FAILED (%u)", rc);
		goto done;
	}

	rc = saAmfComponentNameGet(my_amf_hdl, &my_comp_name);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentNameGet FAILED (%u)", rc);
		goto done;
	}

	rc = saAmfComponentRegister(my_amf_hdl, &my_comp_name, 0);
	if (rc != SA_AIS_OK) {
		syslog(LOG_ERR, "saAmfComponentRegister FAILED (%u)", rc);
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
	int logmask;

	if (daemon(0, 0) == -1) {
		syslog(LOG_ERR, "daemon failed: %s", strerror(errno));
		goto done;
	}

	create_pid_file();

	/* Cancel certain signals that would kill us */
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	openlog(basename(argv[0]), LOG_PID, LOG_USER);

	// TBD: Configure logmask from args/vars
	logmask = LOG_UPTO(LOG_INFO);
//	setlogmask(logmask);

	start_script = getenv("STARTSCRIPT");
	if (start_script == NULL) {
		syslog(LOG_ERR, "Variable STARTCRIPT missing");
		goto done;
	}

	stop_script = getenv("STOPSCRIPT");
	if (stop_script == NULL) {
		syslog(LOG_ERR, "Variable STOPSCRIPT missing");
		goto done;
	}

	health_script = getenv("HEALTHCHECKSCRIPT");
	if (health_script == NULL) {
		syslog(LOG_ERR, "Variable HEALTHCHECKSCRIPT missing");
		goto done;
	}

	pidfile = getenv("PIDFILE");

	if (amf_initialize(&amf_sel_obj) != SA_AIS_OK)
		goto done;


	fds[0].fd = amf_sel_obj;
	fds[0].events = POLLIN;

	syslog(LOG_INFO, "'%s' started", getenv("SA_AMF_COMPONENT_NAME"));

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

