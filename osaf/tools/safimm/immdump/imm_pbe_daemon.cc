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

	/*
	if ((stream = log_stream_get_by_name((char *)objectName->value)) == NULL) {
		LOG_ER("Stream %s not found", objectName->value);
		(void)immutil_saImmOiAdminOperationResult(pbeOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
		goto done;
	}
	*/

	if (opId == 4711) {
		/*
		if (severityFilter == stream->severityFilter) {
			(void)immutil_saImmOiAdminOperationResult(pbeOiHandle, invocation, SA_AIS_ERR_NO_OP);
			goto done;
		}

		TRACE("Changing severity for stream %s to %u", stream->name, severityFilter);
		stream->severityFilter = severityFilter;

		(void)immutil_update_one_rattr(pbeOiHandle, (char *)objectName->value,
					       "saLogStreamSeverityFilter", SA_IMM_ATTR_SAUINT32T,
					       &stream->severityFilter);

		(void)immutil_saImmOiAdminOperationResult(pbeOiHandle, invocation, SA_AIS_OK);
		*/
	} else {
		LOG_ER("Invalid operation ID, should be %d ", 4711);
		(void)immutil_saImmOiAdminOperationResult(pbeOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
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

	TRACE_ENTER();

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
	const SaImmAttrModificationT_2 *attrMod;

	TRACE_ENTER();

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		LOG_ER("Failed to find CCB object for %llu", ccbId);
		rc = SA_AIS_ERR_BAD_OPERATION;
		goto done;
	}

	/*
	 ** "check that the sequence of change requests contained in the CCB is 
	 ** valid and that no errors will be generated when these changes
	 ** are applied."
	 */
	ccbUtilOperationData = ccbUtilCcbData->operationListHead;
	while (ccbUtilOperationData != NULL) {
		int i = 0;
		TRACE("Change stream %s", ccbUtilOperationData->param.modify.objectName->value);

		/*
		if ((stream = log_stream_get_by_name((char *)ccbUtilOperationData->param.modify.objectName->value)) ==
		    NULL) {
			LOG_ER("Stream %s not found", ccbUtilOperationData->param.modify.objectName->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
		*/

		attrMod = ccbUtilOperationData->param.modify.attrMods[i++];
		while (attrMod != NULL) {
			void *value;
			const SaImmAttrValuesT_2 *attribute = &attrMod->modAttr;

			TRACE("attribute %s", attribute->attrName);

			if (attribute->attrValuesNumber == 0) {
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}

			value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saLogStreamFileName")) {
				/*
				struct stat pathstat;
				char *fileName = *((char **)value);
				if (stat(fileName, &pathstat) == 0) {
					LOG_ER("File %s already exist", fileName);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("fileName: %s", fileName);
				*/
			} else if (!strcmp(attribute->attrName, "saLogStreamMaxLogFileSize")) {
				SaUint64T maxLogFileSize = *((SaUint64T *)value);
				TRACE("maxLogFileSize: %llu", maxLogFileSize);
			} else if (!strcmp(attribute->attrName, "saLogStreamFixedLogRecordSize")) {
				SaUint32T fixedLogRecordSize = *((SaUint32T *)value);
				if (fixedLogRecordSize > 0/*stream->maxLogFileSize*/) {
					LOG_ER("fixedLogRecordSize out of range");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("fixedLogRecordSize: %u", fixedLogRecordSize);
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFullAction")) {
				/*
				SaLogFileFullActionT logFullAction = *((SaUint32T *)value);
				if (logFullAction > SA_LOG_FILE_FULL_ACTION_ROTATE) {
					LOG_ER("logFullAction out of range");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("logFullAction: %u", logFullAction);
				*/
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFullHaltThreshold")) {
				SaUint32T logFullHaltThreshold = *((SaUint32T *)value);
				if (logFullHaltThreshold >= 100) {
					LOG_ER("logFullHaltThreshold out of range");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("logFullHaltThreshold: %u", logFullHaltThreshold);
			} else if (!strcmp(attribute->attrName, "saLogStreamMaxFilesRotated")) {
				SaUint32T maxFilesRotated = *((SaUint32T *)value);
				if (maxFilesRotated > 128) {
					LOG_ER("Unreasonable maxFilesRotated: %x", maxFilesRotated);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("maxFilesRotated: %u", maxFilesRotated);
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFileFormat")) {
				char *logFileFormat = *((char **)value);
				TRACE("logFileFormat: %s", logFileFormat);
				/*
				if (!lgs_is_valid_format_expression(logFileFormat, stream->streamType, &dummy)) {
					LOG_ER("Invalid logFileFormat: %s", logFileFormat);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				*/
			} else if (!strcmp(attribute->attrName, "saLogStreamSeverityFilter")) {
				SaUint32T severityFilter = *((SaUint32T *)value);
				if (severityFilter > 0x7f) {
					LOG_ER("Invalid severity: %x", severityFilter);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("severityFilter: %u", severityFilter);
			} else {
				LOG_ER("invalid attribute");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}

			attrMod = ccbUtilOperationData->param.modify.attrMods[i++];
		}
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
	const SaImmAttrModificationT_2 *attrMod;

	char *fileName = NULL;
	SaUint64T maxLogFileSize = 0;
	SaUint32T fixedLogRecordSize = 0;
	SaUint32T logFullHaltThreshold = 0;
	SaUint32T maxFilesRotated = 0;
	char *logFileFormat = NULL;
	SaUint32T severityFilter = 0;

	TRACE_ENTER();

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		LOG_ER("Failed to find CCB object for %llu", ccbId);
		goto done;
	}

	ccbUtilOperationData = ccbUtilCcbData->operationListHead;
	while (ccbUtilOperationData != NULL) {
		int i = 0;
		TRACE("Apply changes to %s", ccbUtilOperationData->param.modify.objectName->value);

		attrMod = ccbUtilOperationData->param.modify.attrMods[i++];
		while (attrMod != NULL) {
			void *value;
			const SaImmAttrValuesT_2 *attribute = &attrMod->modAttr;

			TRACE("attribute %s", attribute->attrName);
			/* check for nrofValues > 0 */
			value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saLogStreamFileName")) {
				fileName = *((char **)value);
			} else if (!strcmp(attribute->attrName, "saLogStreamMaxLogFileSize")) {
				maxLogFileSize = *((SaUint64T *)value);
			} else if (!strcmp(attribute->attrName, "saLogStreamFixedLogRecordSize")) {
				fixedLogRecordSize = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFullAction")) {

			} else if (!strcmp(attribute->attrName, "saLogStreamLogFullHaltThreshold")) {
				logFullHaltThreshold = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saLogStreamMaxFilesRotated")) {
				maxFilesRotated = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFileFormat")) {
				logFileFormat = *((char **)value);
			} else if (!strcmp(attribute->attrName, "saLogStreamSeverityFilter")) {
				severityFilter = *((SaUint32T *)value);
			} else {
				LOG_ER("invalid attribute");
				goto done;
			}

			attrMod = ccbUtilOperationData->param.modify.attrMods[i++];
		}

		ccbUtilOperationData = ccbUtilOperationData->next;
	}

	TRACE("dummy %s %llu %u %u %u %s %u", fileName, maxLogFileSize, fixedLogRecordSize,
		logFullHaltThreshold, maxFilesRotated, logFileFormat, severityFilter);

 done:
	ccbutil_deleteCcbData(ccbUtilCcbData);
	TRACE_LEAVE();
}

static void saImmOiCcbAbortCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;

	TRACE_ENTER();

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
	TRACE_ENTER();

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
	SaImmAttrNameT attributeName;
	int i = 0;
	int numOpeners = 0;
	char *stream = (char *)objectName->value;

	TRACE_ENTER();

	if (stream == NULL) {
		LOG_ER("saImmOiRtAttrUpdateCallback, stream %s not found", objectName->value);
		rc = SA_AIS_ERR_FAILED_OPERATION;	/* not really covered in spec */
		goto done;
	}

	while ((attributeName = attributeNames[i++]) != NULL) {
		TRACE("Attribute %s", attributeName);
		if (!strcmp(attributeName, "saLogStreamNumOpeners")) {
			(void)immutil_update_one_rattr(immOiHandle, (char *)objectName->value,
						       attributeName, SA_IMM_ATTR_SAUINT32T, &numOpeners);
		} else {
			LOG_ER("saImmOiRtAttrUpdateCallback, unknown attribute %s", attributeName);
			rc = SA_AIS_ERR_FAILED_OPERATION;	/* not really covered in spec */
			goto done;
		}
	}

 done:
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
	TRACE("Control block information");
	TRACE("  comp_name:      %s", lgs_cb->comp_name.value);
	TRACE("  log_version:    %c.%02d.%02d", lgs_cb->log_version.releaseCode,
	      lgs_cb->log_version.majorVersion, lgs_cb->log_version.minorVersion);
	TRACE("  mds_role:       %u", lgs_cb->mds_role);
	TRACE("  last_client_id: %u", lgs_cb->last_client_id);
	TRACE("  ha_state:       %u", lgs_cb->ha_state);
	TRACE("  ckpt_state:     %u", lgs_cb->ckpt_state);
	TRACE("  root_dir:       %s", lgs_cb->logsv_root_dir);

	TRACE("Client information");
	client = (log_client_t *)ncs_patricia_tree_getnext(&lgs_cb->client_tree, NULL);
	while (client != NULL) {
		lgs_stream_list_t *s = client->stream_list_root;
		TRACE("  client_id: %u", client->client_id);
		TRACE("    lga_client_dest: %llx", client->mds_dest);

		while (s != NULL) {
			TRACE("    stream id: %u", s->stream_id);
			s = s->next;
		}
		client = (log_client_t *)ncs_patricia_tree_getnext(&lgs_cb->client_tree,
								   (uns8 *)&client->client_id_net);
	}

	TRACE("Streams information");
	stream = (log_stream_t *)log_stream_getnext_by_name(NULL);
	while (stream != NULL) {
		log_stream_print(stream);
		stream = (log_stream_t *)log_stream_getnext_by_name(stream->name);
	}
	log_stream_id_print();
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

#ifdef IMM_PBE

#else

#endif


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

	rc = saImmOiImplementerSet(pbeOiHandle, OPENSAF_IMM_PBE_IMPL_NAME);
	while ((rc == SA_AIS_ERR_TRY_AGAIN || rc == SA_AIS_ERR_EXIST) && 
		(msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOiImplementerSet(pbeOiHandle, OPENSAF_IMM_PBE_IMPL_NAME);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiImplementerSet for %s failed %u", OPENSAF_IMM_PBE_IMPL_NAME, rc);
		return rc;
	}
	/* Second implementer: OPENSAF_IMM_SERVICE_IMPL_NAME */

	rc = saImmOiClassImplementerSet(pbeOiHandle, OPENSAF_IMM_CLASS_NAME);
	while ((rc == SA_AIS_ERR_TRY_AGAIN) && (msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;	
		rc = saImmOiClassImplementerSet(pbeOiHandle, OPENSAF_IMM_CLASS_NAME);
	}

	TRACE_LEAVE();

	return rc;
}


void pbeDaemon(SaImmHandleT immHandle, void* dbHandle, ClassMap* classIdMap)
{

	SaAisErrorT error = SA_AIS_OK;
	//uns32 rc;

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


