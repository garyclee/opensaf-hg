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

#include "imm_dumper.hh"
#include <errno.h>
#include <signal.h>
//#include <fcntl.h>
//#include <assert.h>
#include <saImmOi.h>
#include "immutil.h"
#include <poll.h>
#include <stdio.h>
#include <cstdlib>


#define FD_IMM 0

const unsigned int sleep_delay_ms = 500;
const unsigned int max_waiting_time_ms = 5 * 1000;	/* 5 secs */
/*static const SaImmClassNameT immServiceClassName = "SaImmMngt";*/

static SaImmOiHandleT pbeOiHandle;
static SaSelectionObjectT immOiSelectionObject;
static int category_mask;

static struct pollfd fds[1];
static nfds_t nfds = 1;

static void saImmOiAdminOperationCallback(SaImmOiHandleT immOiHandle,
					  SaInvocationT invocation,
					  const SaNameT *objectName,
					  SaImmAdminOperationIdT opId, const SaImmAdminOperationParamsT_2 **params)
{
	const SaImmAdminOperationParamsT_2 *param = params[0];
	if(param) {TRACE("bogus trace"); goto done;}

	TRACE_ENTER();

	if (opId == 4711) {
		/*
		(void)immutil_update_one_rattr(immOiHandle, (char *)objectName->value,
					       "saLogStreamSeverityFilter", SA_IMM_ATTR_SAUINT32T,
					       &stream->severityFilter);
		*/

		(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_OK);
	} else {
		LOG_ER("Invalid operation ID, should be %d ", 4711);
		(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
	}
 done:
	TRACE_LEAVE();
}

static SaAisErrorT saImmOiCcbObjectModifyCallback(SaImmOiHandleT immOiHandle,
						  SaImmOiCcbIdT ccbId,
						  const SaNameT *objectName, const SaImmAttrModificationT_2 **attrMods)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;

	TRACE_ENTER2("Modify callback for CCB:%llu object:%s", ccbId, objectName->value);
	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			LOG_ER("Failed to get CCB objectfor %llu", ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

	/* "memorize the modification request" */
	ccbutil_ccbAddModifyOperation(ccbUtilCcbData, objectName, attrMods);

 done:
	TRACE_LEAVE();
	return rc;
}

static SaAisErrorT saImmOiCcbCompletedCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	struct CcbUtilOperationData *ccbUtilOperationData;
	//const SaImmAttrModificationT_2 *attrMod;

	TRACE_ENTER2("Completed callback for CCB:%llu", ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		LOG_ER("Failed to find CCB object for %llu", ccbId);
		rc = SA_AIS_ERR_BAD_OPERATION;
		goto done;
	}

	goto done;

	ccbUtilOperationData = ccbUtilCcbData->operationListHead;
	while (ccbUtilOperationData != NULL) {

		ccbUtilOperationData = ccbUtilOperationData->next;
	}

 done:
	TRACE_LEAVE();
	return rc;
}

static void saImmOiCcbApplyCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;
	struct CcbUtilOperationData *ccbUtilOperationData;
	//const SaImmAttrModificationT_2 *attrMod;

	TRACE_ENTER2("PBE APPLY CALLBACK cleanup CCB:%llu", ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		LOG_ER("Failed to find CCB object for %llu", ccbId);
		goto done;
	}

	ccbUtilOperationData = ccbUtilCcbData->operationListHead;

	ccbutil_deleteCcbData(ccbUtilCcbData);
 done:
	TRACE_LEAVE();
}

static void saImmOiCcbAbortCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;

	TRACE_ENTER2("ABORT callback. Cleanup CCB %llu", ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) != NULL)
		ccbutil_deleteCcbData(ccbUtilCcbData);
	else
		LOG_ER("Failed to find CCB object for %llu", ccbId);

	TRACE_LEAVE();
}

static SaAisErrorT saImmOiCcbObjectCreateCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId,
	const SaImmClassNameT className, const SaNameT *parentName, const SaImmAttrValuesT_2 **attr)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	TRACE_ENTER2("CREATE CALLBACK CCB:%llu class:%s parent:%s", ccbId, className, parentName->value);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			LOG_ER("Failed to get CCB objectfor %llu", ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

	/* "memorize the creation request" */
	ccbutil_ccbAddCreateOperation(ccbUtilCcbData, className, parentName, attr);

 done:
	TRACE_LEAVE();
	return SA_AIS_OK;
}

static SaAisErrorT saImmOiCcbObjectDeleteCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId,
	const SaNameT *objectName)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	TRACE_ENTER2("DELETE CALLBACK CCB:%llu object:%s", ccbId, objectName->value);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			LOG_ER("Failed to get CCB objectfor %llu", ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

	/* "memorize the deletion request" */
	ccbutil_ccbAddDeleteOperation(ccbUtilCcbData, objectName);

 done:
	TRACE_LEAVE();
	return SA_AIS_OK;
}
/**
 * IMM requests us to update a non cached runtime attribute.
 * @param immOiHandle
 * @param objectName
 * @param attributeNames
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT saImmOiRtAttrUpdateCallback(SaImmOiHandleT immOiHandle,
					       const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	SaAisErrorT rc = SA_AIS_OK;
	//SaImmAttrNameT attributeName;

	TRACE_ENTER2("RT ATTR UPDATE CALLBACK");

	TRACE_LEAVE();
	return rc;
}

static const SaImmOiCallbacksT_2 callbacks = {
	saImmOiAdminOperationCallback,
	saImmOiCcbAbortCallback,
	saImmOiCcbApplyCallback,
	saImmOiCcbCompletedCallback,
	saImmOiCcbObjectCreateCallback,
	saImmOiCcbObjectDeleteCallback,
	saImmOiCcbObjectModifyCallback,
	saImmOiRtAttrUpdateCallback
};

/**
 * Ssignal handler to dump information from all data structures
 * @param sig
 */
static void dump_sig_handler(int sig)
{
	/*
	int old_category_mask = category_mask;

	if (trace_category_set(CATEGORY_ALL) == -1)
		printf("trace_category_set failed");

	if (trace_category_set(old_category_mask) == -1)
		printf("trace_category_set failed");
	*/
}

/**
 * USR2 signal handler to enable/disable trace (toggle)
 * @param sig
 */
static void sigusr2_handler(int sig)
{
	if (category_mask == 0) {
		category_mask = CATEGORY_ALL;
		printf("Enabling traces");
		dump_sig_handler(sig); /* Do a dump each time we toggle on.*/
	} else {
		category_mask = 0;
		printf("Disabling traces");
	}

	if (trace_category_set(category_mask) == -1)
		printf("trace_category_set failed");
}

SaAisErrorT pbe_daemon_imm_oi_init()
{
	SaAisErrorT rc;
	unsigned int msecs_waited = 0;
	SaVersionT  immVersion;
	TRACE_ENTER();

	immVersion.releaseCode = RELEASE_CODE;
	immVersion.majorVersion = MAJOR_VERSION;
	immVersion.minorVersion = MINOR_VERSION;

	rc = saImmOiInitialize_2(&pbeOiHandle, &callbacks, &immVersion);
	while ((rc == SA_AIS_ERR_TRY_AGAIN) && (msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOiInitialize_2(&pbeOiHandle, &callbacks, &immVersion);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiInitialize_2 failed %u", rc);
		return rc;
	}

	rc = saImmOiSelectionObjectGet(pbeOiHandle, &immOiSelectionObject);
	/* SelectionObjectGet is library local, no need for retry loop */
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiSelectionObjectGet failed %u", rc);
		return rc;
	}

	rc = saImmOiImplementerSet(pbeOiHandle, (char *) OPENSAF_IMM_PBE_IMPL_NAME);
	while ((rc == SA_AIS_ERR_TRY_AGAIN || rc == SA_AIS_ERR_EXIST) && 
		(msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOiImplementerSet(pbeOiHandle, (char *) OPENSAF_IMM_PBE_IMPL_NAME);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiImplementerSet for %s failed %u", OPENSAF_IMM_PBE_IMPL_NAME, rc);
		return rc;
	}
	/* Second implementer: OPENSAF_IMM_SERVICE_IMPL_NAME */

	rc = saImmOiClassImplementerSet(pbeOiHandle, (char *) OPENSAF_IMM_CLASS_NAME);
	while ((rc == SA_AIS_ERR_TRY_AGAIN) && (msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;	
		rc = saImmOiClassImplementerSet(pbeOiHandle, (char *) OPENSAF_IMM_CLASS_NAME);
	}

	TRACE_LEAVE();

	return rc;
}


void pbeDaemon(SaImmHandleT immHandle, void* dbHandle, ClassMap* classIdMap)
{

	SaAisErrorT error = SA_AIS_OK;
	//uns32 rc;
	//Assign dbHandle to static var.

	if (pbe_daemon_imm_oi_init() != SA_AIS_OK) {
		exit(1);
	}

	if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR) {
		LOG_ER("signal USR2 failed: %s", strerror(errno));
		exit(1);
	}
	/* Set up all file descriptors to listen to */
	fds[FD_IMM].fd = immOiSelectionObject;
	fds[FD_IMM].events = POLLIN;

        while(1) {
		TRACE("PBE Daemon entering poll");
		int ret = poll(fds, nfds, -1);
		TRACE("PBE Daemon returned from poll ret: %d", ret);
		if (ret == -1) {
			if (errno == EINTR)
				continue;

			LOG_ER("poll failed - %s", strerror(errno));
			break;
		}

		if (pbeOiHandle && fds[FD_IMM].revents & POLLIN) {
			error = saImmOiDispatch(pbeOiHandle, SA_DISPATCH_ALL);

			if (error == SA_AIS_ERR_BAD_HANDLE) {
				TRACE("saImmOiDispatch returned BAD_HANDLE");
				pbeOiHandle = 0;
				break;
			}
		}
		/* Attch as OI for 
		   SaImmMngt: safRdn=immManagement,safApp=safImmService
		   ?? OpensafImm: opensafImm=opensafImm,safApp=safImmService ??
		*/
	}

	LOG_NO("FAILED EXITING...");
	TRACE_LEAVE();
	exit(1);
}


