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

#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <saImmOm.h>
#include <saImmOi.h>

#include "immutil.h"
#include "lgs.h"
#include "lgs_util.h"

/* TYPE DEFINITIONS
 * ----------------
 */
typedef struct {
	/* --- Corresponds to IMM Class SaLogConfig --- */
	char logRootDirectory[PATH_MAX];
	SaUint32T logMaxLogrecsize;
	SaUint32T logStreamSystemHighLimit;
	SaUint32T logStreamSystemLowLimit;
	SaUint32T logStreamAppHighLimit;
	SaUint32T logStreamAppLowLimit;
	SaUint32T logMaxApplicationStreams;
	/* --- end correspond to IMM Class --- */

	bool logInitiated;
	bool OpenSafLogConfig_class_exist;

	bool logRootDirectory_noteflag;
	bool logMaxLogrecsize_noteflag;
	bool logStreamSystemHighLimit_noteflag;
	bool logStreamSystemLowLimit_noteflag;
	bool logStreamAppHighLimit_noteflag;
	bool logStreamAppLowLimit_noteflag;
	bool logMaxApplicationStreams_noteflag;
} lgs_conf_t;

/* DATA DECLARATIONS
 * -----------------
 */

/* LOG configuration */
/* Default values are used if no configuration object can be found in IMM */
/* See function lgs_imm_logconf_get */
static lgs_conf_t _lgs_conf = {
	.logRootDirectory = "",
	.logMaxLogrecsize = 1024,
	.logStreamSystemHighLimit = 0,
	.logStreamSystemLowLimit = 0,
	.logStreamAppHighLimit = 0,
	.logStreamAppLowLimit = 0,
	.logMaxApplicationStreams = 64,

	.logInitiated = false,
	.OpenSafLogConfig_class_exist = false,
			
	.logRootDirectory_noteflag = false,
	.logMaxLogrecsize_noteflag = false,
	.logStreamSystemHighLimit_noteflag = false,
	.logStreamSystemLowLimit_noteflag = false,
	.logStreamAppHighLimit_noteflag = false,
	.logStreamAppLowLimit_noteflag = false,
	.logMaxApplicationStreams_noteflag = false
};
static const lgs_conf_t *lgs_conf = &_lgs_conf;

static bool we_are_applier_flag = false;

const unsigned int sleep_delay_ms = 500;
const unsigned int max_waiting_time_ms = 60 * 1000;	/* 60 secs */

/* Must be able to index this array using streamType */
static char *log_file_format[] = {
	DEFAULT_ALM_NOT_FORMAT_EXP,
	DEFAULT_ALM_NOT_FORMAT_EXP,
	DEFAULT_APP_SYS_FORMAT_EXP
};

static SaVersionT immVersion = { 'A', 2, 11 };

static const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT)"safLogService";
static const SaImmOiImplementerNameT applierName = (SaImmOiImplementerNameT)"@safLogService";

extern struct ImmutilWrapperProfile immutilWrapperProfile;

/* FUNCTIONS
 * ---------
 */

/**
 * Pack and send a stream checkpoint using mbcsv
 * @param cb
 * @param logStream
 * 
 * @return uns32
 */
static uint32_t lgs_ckpt_stream(log_stream_t *stream)
{
	lgsv_ckpt_msg_t ckpt;
	uint32_t rc;

	TRACE_ENTER();

	memset(&ckpt, 0, sizeof(ckpt));
	ckpt.header.ckpt_rec_type = LGS_CKPT_CFG_STREAM;
	ckpt.header.num_ckpt_records = 1;
	ckpt.header.data_len = 1;

	ckpt.ckpt_rec.stream_cfg.name = (char *)stream->name;
	ckpt.ckpt_rec.stream_cfg.fileName = stream->fileName;
	ckpt.ckpt_rec.stream_cfg.pathName = stream->pathName;
	ckpt.ckpt_rec.stream_cfg.maxLogFileSize = stream->maxLogFileSize;
	ckpt.ckpt_rec.stream_cfg.fixedLogRecordSize = stream->fixedLogRecordSize;
	ckpt.ckpt_rec.stream_cfg.logFullAction = stream->logFullAction;
	ckpt.ckpt_rec.stream_cfg.logFullHaltThreshold = stream->logFullHaltThreshold;
	ckpt.ckpt_rec.stream_cfg.maxFilesRotated = stream->maxFilesRotated;
	ckpt.ckpt_rec.stream_cfg.logFileFormat = stream->logFileFormat;
	ckpt.ckpt_rec.stream_cfg.severityFilter = stream->severityFilter;
	ckpt.ckpt_rec.stream_cfg.logFileCurrent = stream->logFileCurrent;

	rc = lgs_ckpt_send_async(lgs_cb, &ckpt, NCS_MBCSV_ACT_ADD);

	TRACE_LEAVE();
	return rc;
}

/**
 * Check if the given path is valid.
 * We must have rwx access.
 * @param pathname
 * return: true  = Path is valid
 *         false = Path is invalid
 */
static bool lgs_check_path_permissions(char* pathname)
{
	bool rc=true;
	struct stat pathstat;

	TRACE_ENTER();

	/* Check if the path points to a directory */
	if (stat(pathname, &pathstat) != 0) {
		TRACE("lgs_check_path_permissions() Directory %s does not exist",pathname);
		rc = false;
		goto done;
	}
	if (!S_ISDIR(pathstat.st_mode)) {
		TRACE("lgs_check_path_permissions() %s is not a directory",pathname);
		rc = false;
		goto done;
	}

	/* Check is we have correct permissions. Note that we check permissions for
	 * real UID
	 */
	if (access(pathname,(R_OK | W_OK | X_OK)) != 0) {
		TRACE("lgs_check_path_permissions() permission denied for %s, error %s",
				pathname,strerror(errno));
		rc = false;
		goto done;
	}

done:
	TRACE_LEAVE();
	return rc;
}

/**
 * LOG Admin operation handling. This function is executed as an
 * IMM callback. In LOG A.02.01 only SA_LOG_ADMIN_CHANGE_FILTER
 * is supported.
 * 
 * @param immOiHandle
 * @param invocation
 * @param objectName
 * @param opId
 * @param params
 */
static void saImmOiAdminOperationCallback(SaImmOiHandleT immOiHandle,
					  SaInvocationT invocation,
					  const SaNameT *objectName,
					  SaImmAdminOperationIdT opId, const SaImmAdminOperationParamsT_2 **params)
{
	SaUint32T severityFilter;
	const SaImmAdminOperationParamsT_2 *param = params[0];
	log_stream_t *stream;

	TRACE_ENTER();

	if (lgs_cb->ha_state != SA_AMF_HA_ACTIVE) {
		TRACE("saImmOiAdminOperationCallback() Should never ger here");
		goto done;
	}

	if ((stream = log_stream_get_by_name((char *)objectName->value)) == NULL) {
		LOG_ER("Stream %s not found", objectName->value);
		(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
		goto done;
	}

	if (opId == SA_LOG_ADMIN_CHANGE_FILTER) {
		/* Only allowed to update runtime objects (application streams) */
		if (stream->streamType != STREAM_TYPE_APPLICATION) {
			LOG_ER("Admin op change filter for non app stream");
			(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
			goto done;
		}

		if (strcmp(param->paramName, "saLogStreamSeverityFilter") != 0) {
			LOG_ER("Admin op change filter, invalid param name");
			(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
			goto done;
		}

		if (param->paramType != SA_IMM_ATTR_SAUINT32T) {
			LOG_ER("Admin op change filter: invalid parameter type");
			(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
			goto done;
		}

		severityFilter = *((SaUint32T *)param->paramBuffer);

		if (severityFilter > 0x7f) {	/* not a level, a bitmap */
			LOG_ER("Admin op change filter: invalid severity");
			(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
			goto done;
		}

		if (severityFilter == stream->severityFilter) {
			(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_NO_OP);
			goto done;
		}

		TRACE("Changing severity for stream %s to %u", stream->name, severityFilter);
		stream->severityFilter = severityFilter;

		(void)immutil_update_one_rattr(immOiHandle, (char *)objectName->value,
					       "saLogStreamSeverityFilter", SA_IMM_ATTR_SAUINT32T,
					       &stream->severityFilter);

		(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_OK);

		/* Checkpoint to standby LOG server */
		lgs_ckpt_stream(stream);
	} else {
		LOG_ER("Invalid operation ID, should be %d (one) for change filter", SA_LOG_ADMIN_CHANGE_FILTER);
		(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_INVALID_PARAM);
	}
 done:
	TRACE_LEAVE();
}

static SaAisErrorT saImmOiCcbObjectDeleteCallback(SaImmOiHandleT immOiHandle,
					     SaImmOiCcbIdT ccbId, const SaNameT *objectName)
{
	TRACE_ENTER();
	TRACE("saImmOiCcbObjectDeleteCallback() not allowed");
	TRACE_LEAVE();
	return SA_AIS_ERR_FAILED_OPERATION;
}

static SaAisErrorT saImmOiCcbObjectCreateCallback(SaImmOiHandleT immOiHandle,
					       SaImmOiCcbIdT ccbId,
					       const SaImmClassNameT className,
					       const SaNameT *parentName, const SaImmAttrValuesT_2 **attr)
{
	TRACE_ENTER();
	TRACE("saImmOiCcbObjectCreateCallback() not allowed");
	TRACE_LEAVE();
	return SA_AIS_ERR_FAILED_OPERATION;
}

/**
 * Validate and "memorize" a change request
 * 
 * @param immOiHandle
 * @param ccbId
 * @param objectName
 * @param attrMods
 * @return 
 */
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

/**
 * The CCB is now complete. Verify that the changes can be applied
 * 
 * @param immOiHandle
 * @param ccbId
 * @return 
 */
static SaAisErrorT saImmOiCcbCompletedCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	struct CcbUtilOperationData *ccbUtilOperationData;
	log_stream_t *stream=NULL;
	const SaImmAttrModificationT_2 *attrMod;

	TRACE_ENTER();


	/* Handling of OpenSafLogConfig.
	 *	   If the updated attribute is logRootDirectory
	 *	   If Not active.
	 *		- Do nothing.
	 *     If active.
	 *		If updated attribute is logRootDirectory:
	 *		- Check IF contained path exists and is r+w
	 *		ELSE return SA_AIS_ERR_FAILED_OPERATION
	 */

	if (lgs_cb->ha_state != SA_AMF_HA_ACTIVE) {
		TRACE("State Not Active. Nothing to do, we are an applier");
		goto done;
	}

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

		/* Is there a valid object
		 */
		if (strcmp( (char *) ccbUtilOperationData->param.modify.objectName->value, LGS_IMM_LOG_CONFIGURATION) == 0) {
			TRACE("Handle an OpenSafLogConfig object");
		} else if ((stream = log_stream_get_by_name((char *)ccbUtilOperationData->param.modify.objectName->value)) !=
		    NULL) {
			TRACE("Handle a SaLogStreamConfig object");
		} else {
			LOG_ER("Object %s not found", ccbUtilOperationData->param.modify.objectName->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

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

			/* Handle OpenSafLogConfig class parameters
			   ----------------------------------------
			 */
			if (!strcmp(attribute->attrName, "logRootDirectory")) {
				char *pathName = *((char **)value);
				if (!lgs_check_path_permissions(pathName)) {
					TRACE("pathName: %s is NOT accepted", pathName);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("pathName: %s is accepted", pathName);
			} else if (!strcmp(attribute->attrName, "logMaxLogrecsize")) {
				TRACE("%s cannot be changed",attribute->attrName);
				rc = SA_AIS_ERR_FAILED_OPERATION;
				goto done;
			} else if (!strcmp(attribute->attrName, "logStreamSystemHighLimit")) {
				TRACE("%s cannot be changed",attribute->attrName);
				rc = SA_AIS_ERR_FAILED_OPERATION;
				goto done;
			} else if (!strcmp(attribute->attrName, "logStreamSystemLowLimit")) {
				TRACE("%s cannot be changed",attribute->attrName);
				rc = SA_AIS_ERR_FAILED_OPERATION;
				goto done;
			} else if (!strcmp(attribute->attrName, "logStreamAppHighLimit")) {
				TRACE("%s cannot be changed",attribute->attrName);
				rc = SA_AIS_ERR_FAILED_OPERATION;
				goto done;
			} else if (!strcmp(attribute->attrName, "logStreamAppLowLimit")) {
				TRACE("%s cannot be changed",attribute->attrName);
				rc = SA_AIS_ERR_FAILED_OPERATION;
				goto done;
			} else if (!strcmp(attribute->attrName, "logMaxApplicationStreams")) {
				TRACE("%s cannot be changed",attribute->attrName);
				rc = SA_AIS_ERR_FAILED_OPERATION;
				goto done;
			}


			/* Handle SaLogStreamConfig class parameters
			   -----------------------------------------
			 */
			else if (!strcmp(attribute->attrName, "saLogStreamFileName")) {
				struct stat pathstat;
				char *fileName = *((char **)value);
				if (stat(fileName, &pathstat) == 0) {
					LOG_ER("File %s already exist", fileName);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("fileName: %s", fileName);
			} else if (!strcmp(attribute->attrName, "saLogStreamMaxLogFileSize")) {
				SaUint64T maxLogFileSize = *((SaUint64T *)value);
				TRACE("maxLogFileSize: %llu", maxLogFileSize);
			} else if (!strcmp(attribute->attrName, "saLogStreamFixedLogRecordSize")) {
				SaUint32T fixedLogRecordSize = *((SaUint32T *)value);
				if (fixedLogRecordSize > stream->maxLogFileSize) {
					LOG_ER("fixedLogRecordSize out of range");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				TRACE("fixedLogRecordSize: %u", stream->fixedLogRecordSize);
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFullAction")) {
				SaLogFileFullActionT logFullAction = *((SaUint32T *)value);
				if ((logFullAction < SA_LOG_FILE_FULL_ACTION_WRAP) || 
						(logFullAction > SA_LOG_FILE_FULL_ACTION_ROTATE)){
					LOG_ER("logFullAction out of range");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				if ((logFullAction == SA_LOG_FILE_FULL_ACTION_WRAP) ||(logFullAction == SA_LOG_FILE_FULL_ACTION_HALT)){
					LOG_ER("logFullAction:Current Implementation doesn't support  Wrap and halt");
					rc = SA_AIS_ERR_NOT_SUPPORTED;
					goto done;
				}
				TRACE("logFullAction: %u", logFullAction);
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
				SaBoolT dummy;
				char *logFileFormat = *((char **)value);
				TRACE("logFileFormat: %s", logFileFormat);

				if (!lgs_is_valid_format_expression(logFileFormat, stream->streamType, &dummy)) {
					LOG_ER("Invalid logFileFormat: %s", logFileFormat);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
			} else if (!strcmp(attribute->attrName, "saLogStreamSeverityFilter")) {

				if ((stream->streamType != STREAM_TYPE_ALARM)
                                    && (stream->streamType != STREAM_TYPE_NOTIFICATION)) {

					SaUint32T severityFilter = *((SaUint32T *)value);
					if (severityFilter > 0x7f) {
						LOG_ER("Invalid severity: %x", severityFilter);
						rc = SA_AIS_ERR_BAD_OPERATION;
						goto done;
					}
					TRACE("severityFilter: %u", severityFilter);
				} else {
					LOG_ER("severityFilter cannot be changed for Alarm amd Notification Stream");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
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
	TRACE_LEAVE2("rc=%u",rc);
	return rc;
}

/**
 * Configuration changes are done and now it's time to act on the changes
 * 
 * @param immOiHandle
 * @param ccbId
 */
static void saImmOiCcbApplyCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;
	struct CcbUtilOperationData *ccbUtilOperationData;
	log_stream_t *stream = NULL;
	const SaImmAttrModificationT_2 *attrMod;
	int new_cfg_file_needed=0;
	char current_file_name[NAME_MAX];

	TRACE_ENTER();

	/* Handling of OpenSafLogConfig.
	 *		   IF Not active.
	 *			- Update lgs_conf only with new path (logRootDirectory).
	           IF active.
				- Update lgs_conf only with new path (logRootDirectory).
				- Close all open logfiles and .cfg files.
	 *			- Open all logfiles and .cfg files in new directory.
	 */

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		LOG_ER("Failed to find CCB object for %llu", ccbId);
		goto done;
	}

	ccbUtilOperationData = ccbUtilCcbData->operationListHead;
	while (ccbUtilOperationData != NULL) {
		int i = 0;
		TRACE("Apply changes to object %s", ccbUtilOperationData->param.modify.objectName->value);

		/* Is there a valid object */
		if (strcmp( (char *) ccbUtilOperationData->param.modify.objectName->value, LGS_IMM_LOG_CONFIGURATION) == 0) {
			TRACE("Handle an OpenSafLogConfig object");
		} else if ((stream = log_stream_get_by_name((char *)ccbUtilOperationData->param.modify.objectName->value)) !=
		    NULL) {
			TRACE("Handle a SaLogStream");
			strncpy(current_file_name, stream->fileName, sizeof(current_file_name));
			new_cfg_file_needed = 0;
		} else {
			LOG_ER("Object %s not found", ccbUtilOperationData->param.modify.objectName->value);
			goto done;
		}

		attrMod = ccbUtilOperationData->param.modify.attrMods[i++];
		while (attrMod != NULL) {
			void *value;
			const SaImmAttrValuesT_2 *attribute = &attrMod->modAttr;

			TRACE("attribute %s", attribute->attrName);

			value = attribute->attrValues[0];

			/* Handle OpenSafLogConfig class parameters
			   ----------------------------------------
			 */
			if (!strcmp(attribute->attrName, "logRootDirectory")) {
				TRACE("Apply changes to %s",attribute->attrName);

				/* Update saved configuration on both active and stanby */
				char *new_pathName = *((char **)value);
				
				if (lgs_cb->ha_state == SA_AMF_HA_STANDBY) {
					/* We are standby. Update log root path */
					TRACE("Apply, HA State is Standby. Update root path");
					strcpy((char *) lgs_conf->logRootDirectory, new_pathName);
					TRACE("New path is: %s",lgs_conf->logRootDirectory);
				}
				else if (lgs_cb->ha_state == SA_AMF_HA_ACTIVE) {
					/* We are active. Handle files */
					TRACE("Apply, HA State is Active. Handle files");

					char *current_time = lgs_get_time();

					stream = log_stream_getnext_by_name(NULL);
					while (stream != NULL) {
						TRACE("Handling file %s",stream->logFileCurrent);
						/* Close and rename old files */
						strncpy(current_file_name, stream->fileName, sizeof(current_file_name));
						if (log_stream_config_change(!LGS_STREAM_CREATE_FILES,stream, current_file_name) != 0) {
							LOG_ER("Old log files could not be renamed and closed, root-dir: %s",
									lgs_cb->logsv_root_dir);
							goto done;
						}
						stream = log_stream_getnext_by_name(stream->name);
					}

					/* Initialize the new path */
					strcpy((char *) lgs_conf->logRootDirectory, new_pathName);

					stream = log_stream_getnext_by_name(NULL);
					while (stream != NULL) {
						/* Create new files */
						/* Creating the new config file */
						if (lgs_create_config_file(stream) != 0) {
							LOG_ER("New config file could not be created for stream: %s",
									stream->name);
							goto done;
						}

						/* Create the new log file based on updated configuration */
						sprintf(stream->logFileCurrent, "%s_%s", stream->fileName, current_time);
						if ((stream->fd = log_file_open(stream, NULL)) == -1) {
							LOG_ER("New log file could not be created for stream: %s",
									stream->name);
							goto done;
						}
						stream = log_stream_getnext_by_name(stream->name);
					}
				}
			}

			/* Handle SaLogStreamConfig class parameters
			   -----------------------------------------
			 */
			else if (!strcmp(attribute->attrName, "saLogStreamFileName")) {
				char *fileName = *((char **)value);
				strcpy(stream->fileName, fileName);
				new_cfg_file_needed = 1;
			} else if (!strcmp(attribute->attrName, "saLogStreamMaxLogFileSize")) {
				SaUint64T maxLogFileSize = *((SaUint64T *)value);
				stream->maxLogFileSize = maxLogFileSize;
				new_cfg_file_needed = 1;
			} else if (!strcmp(attribute->attrName, "saLogStreamFixedLogRecordSize")) {
				SaUint32T fixedLogRecordSize = *((SaUint32T *)value);
				stream->fixedLogRecordSize = fixedLogRecordSize;
				new_cfg_file_needed = 1;
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFullAction")) {
				SaLogFileFullActionT logFullAction = *((SaUint32T *)value);
				stream->logFullAction = logFullAction;
				new_cfg_file_needed = 1;
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFullHaltThreshold")) {
				SaUint32T logFullHaltThreshold = *((SaUint32T *)value);
				stream->logFullHaltThreshold = logFullHaltThreshold;
			} else if (!strcmp(attribute->attrName, "saLogStreamMaxFilesRotated")) {
				SaUint32T maxFilesRotated = *((SaUint32T *)value);
				stream->maxFilesRotated = maxFilesRotated;
				new_cfg_file_needed = 1;
			} else if (!strcmp(attribute->attrName, "saLogStreamLogFileFormat")) {
				char *logFileFormat = *((char **)value);
				if (stream->logFileFormat != NULL)
					free(stream->logFileFormat);
				stream->logFileFormat = strdup(logFileFormat);
				new_cfg_file_needed = 1;
			} else if (!strcmp(attribute->attrName, "saLogStreamSeverityFilter")) {
				SaUint32T severityFilter = *((SaUint32T *)value);
				stream->severityFilter = severityFilter;
			} else {
				LOG_ER("invalid attribute");
				goto done;
			}

			attrMod = ccbUtilOperationData->param.modify.attrMods[i++];
		}

		if (stream != NULL) {
			/* Do only if this is a stream */
			if (new_cfg_file_needed) {
				if (log_stream_config_change(LGS_STREAM_CREATE_FILES,stream, current_file_name) != 0) {
					LOG_ER("log_stream_config_change failed");
					exit(1);
				}
			}

			/* Checkpoint to standby LOG server */
			lgs_ckpt_stream(stream);
		}

		ccbUtilOperationData = ccbUtilOperationData->next;
	}

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

/**
 * IMM requests us to update a non cached runtime attribute. The
 * only available attribute is saLogStreamNumOpeners.
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
	log_stream_t *stream = log_stream_get_by_name((char *)objectName->value);

	TRACE_ENTER();

	if (lgs_cb->ha_state != SA_AMF_HA_ACTIVE) {
		TRACE("State is STANDBY, do nothing");
		goto done;
	}

	if (stream == NULL) {
		LOG_ER("saImmOiRtAttrUpdateCallback, stream %s not found", objectName->value);
		rc = SA_AIS_ERR_FAILED_OPERATION;	/* not really covered in spec */
		goto done;
	}

	while ((attributeName = attributeNames[i++]) != NULL) {
		TRACE("Attribute %s", attributeName);
		if (!strcmp(attributeName, "saLogStreamNumOpeners")) {
			(void)immutil_update_one_rattr(immOiHandle, (char *)objectName->value,
						       attributeName, SA_IMM_ATTR_SAUINT32T, &stream->numOpeners);
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

/**
 * Allocate new stream object. Get configuration from IMM and
 * initialize the stream object.
 * @param dn
 * @param in_stream
 * @param stream_id
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT stream_create_and_configure(const char *dn, log_stream_t **in_stream, int stream_id)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaImmHandleT omHandle;
	SaNameT objectName;
	SaVersionT immVersion = { 'A', 2, 1 };
	SaImmAccessorHandleT accessorHandle;
	SaImmAttrValuesT_2 *attribute;
	SaImmAttrValuesT_2 **attributes;
	int i = 0;
	log_stream_t *stream;

	TRACE_ENTER2("(%s)", dn);

	strncpy((char *)objectName.value, dn, SA_MAX_NAME_LENGTH);
	objectName.length = strlen((char *)objectName.value);

	*in_stream = stream = log_stream_new_2(&objectName, stream_id);

	if (stream == NULL) {
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	/* Happens to be the same, ugly! FIX */
	stream->streamType = stream_id;

	(void)immutil_saImmOmInitialize(&omHandle, NULL, &immVersion);
	(void)immutil_saImmOmAccessorInitialize(omHandle, &accessorHandle);

	/* Get all attributes of the object */
        if (immutil_saImmOmAccessorGet_2(accessorHandle, &objectName, NULL, &attributes) != SA_AIS_OK) {
                LOG_ER("Configuration for %s not found", objectName.value);
		rc = SA_AIS_ERR_NOT_EXIST;
		goto done;
        }

	while ((attribute = attributes[i++]) != NULL) {
		void *value;

		if (attribute->attrValuesNumber == 0)
			continue;

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saLogStreamFileName")) {
			strcpy(stream->fileName, *((char **)value));
			TRACE("fileName: %s", stream->fileName);
		} else if (!strcmp(attribute->attrName, "saLogStreamPathName")) {
			strcpy(stream->pathName, *((char **)value));
			TRACE("pathName: %s", stream->pathName);
		} else if (!strcmp(attribute->attrName, "saLogStreamMaxLogFileSize")) {
			stream->maxLogFileSize = *((SaUint64T *)value);
			TRACE("maxLogFileSize: %llu", stream->maxLogFileSize);
		} else if (!strcmp(attribute->attrName, "saLogStreamFixedLogRecordSize")) {
			stream->fixedLogRecordSize = *((SaUint32T *)value);
			TRACE("fixedLogRecordSize: %u", stream->fixedLogRecordSize);
		} else if (!strcmp(attribute->attrName, "saLogStreamLogFullAction")) {
			stream->logFullAction = *((SaUint32T *)value);
			TRACE("logFullAction: %u", stream->logFullAction);
		} else if (!strcmp(attribute->attrName, "saLogStreamLogFullHaltThreshold")) {
			stream->logFullHaltThreshold = *((SaUint32T *)value);
			TRACE("logFullHaltThreshold: %u", stream->logFullHaltThreshold);
		} else if (!strcmp(attribute->attrName, "saLogStreamMaxFilesRotated")) {
			stream->maxFilesRotated = *((SaUint32T *)value);
			TRACE("maxFilesRotated: %u", stream->maxFilesRotated);
		} else if (!strcmp(attribute->attrName, "saLogStreamLogFileFormat")) {
			SaBoolT dummy;
			char *logFileFormat = *((char **)value);
			if (!lgs_is_valid_format_expression(logFileFormat, stream->streamType, &dummy)) {
				LOG_WA("Invalid logFileFormat for stream %s, using default", stream->name);

				if ((stream->streamType == STREAM_TYPE_SYSTEM) ||
				    (stream->streamType == STREAM_TYPE_APPLICATION))
					logFileFormat = DEFAULT_APP_SYS_FORMAT_EXP;

				if ((stream->streamType == STREAM_TYPE_ALARM) ||
				    (stream->streamType == STREAM_TYPE_NOTIFICATION))
					logFileFormat = DEFAULT_ALM_NOT_FORMAT_EXP;
			}

			stream->logFileFormat = strdup(logFileFormat);
			TRACE("logFileFormat: %s", stream->logFileFormat);
		} else if (!strcmp(attribute->attrName, "saLogStreamSeverityFilter")) {
			stream->severityFilter = *((SaUint32T *)value);
			TRACE("severityFilter: %u", stream->severityFilter);
		}
	}

	if (stream->logFileFormat == NULL)
		stream->logFileFormat = strdup(log_file_format[stream->streamType]);

 done:
	(void)immutil_saImmOmAccessorFinalize(accessorHandle);
	(void)immutil_saImmOmFinalize(omHandle);

	TRACE_LEAVE();
	return rc;
}

/**
 * Get Log configuration from IMM. See SaLogConfig class.
 * The configuration will be read from object 'dn' and
 * written to struct lgsConf.
 * Returns SA_AIS_ERR_NOT_EXIST if no object 'dn' exist.
 * 
 * @param dn
 * @param lgsConf
 * 
 * @return SaAisErrorT
 * SA_AIS_OK, SA_AIS_ERR_NOT_EXIST
 */
static SaAisErrorT read_logsv_config_obj(const char *dn, lgs_conf_t *lgsConf) {
	SaAisErrorT rc = SA_AIS_OK;
	SaImmHandleT omHandle;
	SaNameT objectName;
	SaImmAccessorHandleT accessorHandle;
	SaImmAttrValuesT_2 *attribute;
	SaImmAttrValuesT_2 **attributes;
	int i = 0;
	int param_cnt = 0;

	TRACE_ENTER2("(%s)", dn);

	strncpy((char *) objectName.value, dn, SA_MAX_NAME_LENGTH);
	objectName.length = strlen((char *) objectName.value);

	/* NOTE: immutil init osaf_assert if error */
	(void) immutil_saImmOmInitialize(&omHandle, NULL, &immVersion);
	(void) immutil_saImmOmAccessorInitialize(omHandle, &accessorHandle);

	/* Get all attributes of the object */
	if (immutil_saImmOmAccessorGet_2(accessorHandle, &objectName, NULL, &attributes) != SA_AIS_OK) {
		lgsConf->OpenSafLogConfig_class_exist = false;
		rc = SA_AIS_ERR_NOT_EXIST;
		goto done;
	}
	else {
		lgsConf->OpenSafLogConfig_class_exist = true;
	}

	while ((attribute = attributes[i++]) != NULL) {
		void *value;

		if (attribute->attrValuesNumber == 0)
			continue;

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "logRootDirectory")) {
			strncpy((char *) lgsConf->logRootDirectory, *((char **) value), PATH_MAX);
			param_cnt++;
			TRACE("logRootDirectory: %s", lgsConf->logRootDirectory);
		} else if (!strcmp(attribute->attrName, "logMaxLogrecsize")) {
			lgsConf->logMaxLogrecsize = *((SaUint32T *) value);
			param_cnt++;
			TRACE("logMaxLogrecsize: %u", lgsConf->logMaxLogrecsize);
		} else if (!strcmp(attribute->attrName, "logStreamSystemHighLimit")) {
			lgsConf->logStreamSystemHighLimit = *((SaUint32T *) value);
			param_cnt++;
			TRACE("logStreamSystemHighLimit: %u", lgsConf->logStreamSystemHighLimit);
		} else if (!strcmp(attribute->attrName, "logStreamSystemLowLimit")) {
			lgsConf->logStreamSystemLowLimit = *((SaUint32T *) value);
			param_cnt++;
			TRACE("logStreamSystemLowLimit: %u", lgsConf->logStreamSystemLowLimit);
		} else if (!strcmp(attribute->attrName, "logStreamAppHighLimit")) {
			lgsConf->logStreamAppHighLimit = *((SaUint32T *) value);
			param_cnt++;
			TRACE("logStreamAppHighLimit: %u", lgsConf->logStreamAppHighLimit);
		} else if (!strcmp(attribute->attrName, "logStreamAppLowLimit")) {
			lgsConf->logStreamAppLowLimit = *((SaUint32T *) value);
			param_cnt++;
			TRACE("logStreamAppLowLimit: %u", lgsConf->logStreamAppLowLimit);
		} else if (!strcmp(attribute->attrName, "logMaxApplicationStreams")) {
			lgsConf->logMaxApplicationStreams = *((SaUint32T *) value);
			param_cnt++;
			TRACE("logMaxApplicationStreams: %u", lgsConf->logMaxApplicationStreams);
		}
	}

	/* Check if missing parameters */
	if (param_cnt != LGS_IMM_LOG_NUMBER_OF_PARAMS) {
		lgsConf->OpenSafLogConfig_class_exist = false;
		rc = SA_AIS_ERR_NOT_EXIST;
		LOG_ER("read_logsv_configuration(), Parameter error. All parameters could not be read");
	}

done:
	(void) immutil_saImmOmAccessorFinalize(accessorHandle);
	(void) immutil_saImmOmFinalize(omHandle);

	TRACE_LEAVE();
	return rc;
}

/**
 * Handle logsv configuration environment variables.
 * This function shall be called only if no configuration object is found in IMM.
 * The lgsConf struct contains default values but shall be updated
 * according to environment variables if there are any. See file logd.conf
 * If an environment variable is faulty the corresponding value will be set to
 * it's default value and a corresponding error flag will be set.
 * 
 * @param *lgsConf
 * 
 */
static void read_logsv_config_environ_var(lgs_conf_t *lgsConf) {
	char *val_str;
	unsigned long int val_uint;

	TRACE_ENTER2("Configured using default values and environment variables");

	/* logRootDirectory */
	if ((val_str = getenv("LOGSV_ROOT_DIRECTORY")) != NULL) {
		strncpy((char *) lgsConf->logRootDirectory, val_str, PATH_MAX);
		lgsConf->logRootDirectory_noteflag = false;
	} else {
		TRACE("LOGSV_ROOT_DIRECTORY not found");
		lgsConf->logRootDirectory_noteflag = true;
	}
	TRACE("logRootDirectory=%s, logRootDirectory_noteflag=%u",
			lgsConf->logRootDirectory, lgsConf->logRootDirectory_noteflag);

	/* logMaxLogrecsize */
	if ((val_str = getenv("LOGSV_MAX_LOGRECSIZE")) != NULL) {
		val_uint = strtoul(val_str, NULL, 0);
		if ((errno != 0) || (val_uint > UINT_MAX)) {
			LOG_ER("Illegal value for LOGSV_MAX_LOGRECSIZE - %s, default %u",
					strerror(errno), lgsConf->logMaxLogrecsize);
			lgsConf->logMaxLogrecsize_noteflag = true;
		} else {
			lgsConf->logMaxLogrecsize = (SaUint32T) val_uint;
			lgsConf->logMaxLogrecsize_noteflag = false;
		}
	} else { /* No environment variable use default value */
		lgsConf->logMaxLogrecsize_noteflag = false;
	}
	TRACE("logMaxLogrecsize=%u, logMaxLogrecsize_noteflag=%u",
			lgsConf->logMaxLogrecsize, lgsConf->logMaxLogrecsize_noteflag);

	/* logStreamSystemHighLimit */
	if ((val_str = getenv("LOG_STREAM_SYSTEM_HIGH_LIMIT")) != NULL) {
		val_uint = strtoul(val_str, NULL, 0);
		if ((errno != 0) || (val_uint > UINT_MAX)) {
			LOG_ER("Illegal value for LOG_STREAM_SYSTEM_HIGH_LIMIT - %s, default %u",
					strerror(errno), lgsConf->logStreamSystemHighLimit);
			lgsConf->logStreamSystemHighLimit_noteflag = true;
		} else {
			lgsConf->logStreamSystemHighLimit = (SaUint32T) val_uint;
			lgsConf->logStreamSystemHighLimit_noteflag = false;
		}
	} else { /* No environment variable use default value */
		lgsConf->logStreamSystemHighLimit_noteflag = false;
	}
	TRACE("logStreamSystemHighLimit=%u, logStreamSystemHighLimit_noteflag=%u",
			lgsConf->logStreamSystemHighLimit, lgsConf->logStreamSystemHighLimit_noteflag);

	/* logStreamSystemLowLimit */
	if ((val_str = getenv("LOG_STREAM_SYSTEM_LOW_LIMIT")) != NULL) {
		val_uint = strtoul(val_str, NULL, 0);
		if ((errno != 0) || (val_uint > UINT_MAX)) {
			LOG_ER("Illegal value for LOG_STREAM_SYSTEM_LOW_LIMIT - %s, default %u",
					strerror(errno), lgsConf->logStreamSystemLowLimit);
			lgsConf->logStreamSystemLowLimit_noteflag = true;
		} else {
			lgsConf->logStreamSystemLowLimit = (SaUint32T) val_uint;
			lgsConf->logStreamSystemLowLimit_noteflag = false;
		}
	} else { /* No environment variable use default value */
		lgsConf->logStreamSystemLowLimit_noteflag = false;
	}
	TRACE("logStreamSystemLowLimit=%u, logStreamSystemLowLimit_noteflag=%u",
			lgsConf->logStreamSystemLowLimit, lgsConf->logStreamSystemLowLimit_noteflag);

	/* logStreamAppHighLimit */
	if ((val_str = getenv("LOG_STREAM_APP_HIGH_LIMIT")) != NULL) {
		val_uint = strtoul(val_str, NULL, 0);
		if ((errno != 0) || (val_uint > UINT_MAX)) {
			LOG_ER("Illegal value for LOG_STREAM_APP_HIGH_LIMIT - %s, default %u",
					strerror(errno), lgsConf->logStreamAppHighLimit);
			lgsConf->logStreamAppHighLimit_noteflag = true;
		} else {
			lgsConf->logStreamAppHighLimit = (SaUint32T) val_uint;
			lgsConf->logStreamAppHighLimit_noteflag = false;
		}
	} else { /* No environment variable use default value */
		lgsConf->logStreamAppHighLimit_noteflag = false;
	}
	TRACE("logStreamAppHighLimit=%u, logStreamAppHighLimit_noteflag=%u",
			lgsConf->logStreamAppHighLimit, lgsConf->logStreamAppHighLimit_noteflag);

	/* logStreamAppLowLimit */
	if ((val_str = getenv("LOG_STREAM_APP_LOW_LIMIT")) != NULL) {
		val_uint = strtoul(val_str, NULL, 0);
		if ((errno != 0) || (val_uint > UINT_MAX)) {
			LOG_ER("Illegal value for LOG_STREAM_APP_LOW_LIMIT - %s, default %u",
					strerror(errno), lgsConf->logStreamAppLowLimit);
			lgsConf->logStreamAppLowLimit_noteflag = true;
		} else {
			lgsConf->logStreamAppLowLimit = (SaUint32T) val_uint;
			lgsConf->logStreamAppLowLimit_noteflag = false;
		}
	} else { /* No environment variable use default value */
		lgsConf->logStreamAppLowLimit_noteflag = false;
	}
	TRACE("logStreamAppLowLimit=%u, logStreamAppLowLimit_noteflag=%u",
			lgsConf->logStreamAppLowLimit, lgsConf->logStreamAppLowLimit_noteflag);

	/* logMaxApplicationStreams */
	if ((val_str = getenv("LOG_MAX_APPLICATION_STREAMS")) != NULL) {
		val_uint = strtoul(val_str, NULL, 0);
		if ((errno != 0) || (val_uint > UINT_MAX)) {
			LOG_ER("Illegal value for LOG_MAX_APPLICATION_STREAMS - %s, default %u",
					strerror(errno), lgsConf->logMaxApplicationStreams);
			lgsConf->logMaxApplicationStreams_noteflag = true;
		} else {
			lgsConf->logMaxApplicationStreams = (SaUint32T) val_uint;
			lgsConf->logMaxApplicationStreams_noteflag = false;
		}
	} else { /* No environment variable use default value */
		lgsConf->logMaxApplicationStreams_noteflag = false;
	}
	TRACE("logMaxApplicationStreams=%u, logMaxApplicationStreams_noteflag=%u",
			lgsConf->logMaxApplicationStreams, lgsConf->logMaxApplicationStreams_noteflag);

	TRACE_LEAVE();
}

/**
 * Get log service configuration parameter. See SaLogConfig class.
 * The configuration will be read from IMM. If no config object exits
 * a configuration from environment variables and default will be used.
 * 
 * @param lgs_logconfGet_t param
 * Defines what configuration parameter to return
 * 
 * @param bool* noteflag
 * Is set to true if no valid configuration object exists and an invalid
 * environment variable is defined. In this case the parameter value returned
 * is the default value. 
 * It is valid to set this parameter to NULL if no error information is needed.
 * NOTE: A parameter is considered to be invalid if the corresponding environment
 *       variable cannot be converted to a value or if a mandatory environment
 *       variable is missing. This function does not check if the value is within
 *       allowed limits.
 * 
 * @return void *
 * Returns a pointer to the parameter. See struct lgs_conf
 *  
 */
const void *lgs_imm_logconf_get(lgs_logconfGet_t param, bool *noteflag)
{
	lgs_conf_t *lgs_conf_p;

	lgs_conf_p = (lgs_conf_t *) lgs_conf;
	/* Check if parameters has to be fetched from IMM */
	if (lgs_conf->logInitiated != true) {
		if (read_logsv_config_obj(LGS_IMM_LOG_CONFIGURATION, lgs_conf_p) != SA_AIS_OK) {
			LOG_NO("No or invalid log service configuration object");
			read_logsv_config_environ_var( lgs_conf_p);
		}
		lgs_conf_p->logInitiated = true;
	}

	switch (param) {
	case LGS_IMM_LOG_ROOT_DIRECTORY:
		if (noteflag != NULL) {
			*noteflag = lgs_conf->logRootDirectory_noteflag;
		}
		return (char *) lgs_conf->logRootDirectory;
	case LGS_IMM_LOG_MAX_LOGRECSIZE:
		if (noteflag != NULL) {
			*noteflag = lgs_conf->logMaxLogrecsize_noteflag;
		}
		return (SaUint32T *) &lgs_conf->logMaxLogrecsize;
	case LGS_IMM_LOG_STREAM_SYSTEM_HIGH_LIMIT:
		if (noteflag != NULL) {
			*noteflag = lgs_conf->logStreamSystemHighLimit_noteflag;
		}
		return (SaUint32T *) &lgs_conf->logStreamSystemHighLimit;
	case LGS_IMM_LOG_STREAM_SYSTEM_LOW_LIMIT:
		if (noteflag != NULL) {
			*noteflag = lgs_conf->logStreamSystemLowLimit_noteflag;
		}
		return (SaUint32T *) &lgs_conf->logStreamSystemLowLimit;
	case LGS_IMM_LOG_STREAM_APP_HIGH_LIMIT:
		if (noteflag != NULL) {
			*noteflag = lgs_conf->logStreamAppHighLimit_noteflag;
		}
		return (SaUint32T *) &lgs_conf->logStreamAppHighLimit;
	case LGS_IMM_LOG_STREAM_APP_LOW_LIMIT:
		if (noteflag != NULL) {
			*noteflag = lgs_conf->logStreamAppLowLimit_noteflag;
		}
		return (SaUint32T *) &lgs_conf->logStreamAppLowLimit;
	case LGS_IMM_LOG_MAX_APPLICATION_STREAMS:
		if (noteflag != NULL) {
			*noteflag = lgs_conf->logMaxApplicationStreams_noteflag;
		}
		return (SaUint32T *) &lgs_conf->logMaxApplicationStreams;
	case LGS_IMM_LOG_OPENSAFLOGCONFIG_CLASS_EXIST:
		if (noteflag != NULL) {
			*noteflag = false;
		}
		return (bool *) &lgs_conf->OpenSafLogConfig_class_exist;

	case LGS_IMM_LOG_NUMBER_OF_PARAMS:
	case LGS_IMM_LOG_NUMEND:
	default:
		LOG_ER("Invalid parameter %u",param);
		osafassert(0); /* Should never happen */
		break;
	}
	return NULL; /* Dummy */
}

static const SaImmOiCallbacksT_2 callbacks = {
	.saImmOiAdminOperationCallback = saImmOiAdminOperationCallback,
	.saImmOiCcbAbortCallback = saImmOiCcbAbortCallback,
	.saImmOiCcbApplyCallback = saImmOiCcbApplyCallback,
	.saImmOiCcbCompletedCallback = saImmOiCcbCompletedCallback,
	.saImmOiCcbObjectCreateCallback = saImmOiCcbObjectCreateCallback,
	.saImmOiCcbObjectDeleteCallback = saImmOiCcbObjectDeleteCallback,
	.saImmOiCcbObjectModifyCallback = saImmOiCcbObjectModifyCallback,
	.saImmOiRtAttrUpdateCallback = saImmOiRtAttrUpdateCallback
};

/**
 * Give up applier role.
 */
void lgs_giveup_imm_applier(lgs_cb_t *cb)
{
	TRACE_ENTER();

	if (we_are_applier_flag == true){
		immutilWrapperProfile.nTries = 250;
		(void)immutil_saImmOiImplementerClear(cb->immOiHandle);
		immutilWrapperProfile.nTries = 20;
		
		we_are_applier_flag = false;
		TRACE("Applier cleared");
	} else {
		TRACE("We are not an applier");
	}
	
	TRACE_LEAVE();
}

/**
 * Become object and class Applier
 */
void lgs_become_imm_applier(lgs_cb_t *cb)
{
	TRACE_ENTER();

	/* Become an applier only if an OpenSafLogConfig object exists */
	if ( false == *(bool*) lgs_imm_logconf_get(LGS_IMM_LOG_OPENSAFLOGCONFIG_CLASS_EXIST, NULL)) {
		TRACE_LEAVE2("Cannot be applier. OpenSafLogConfig object does not exist");
		return;
	}

	immutilWrapperProfile.nTries = 250; /* LOG will be blocked until IMM responds */
	(void)immutil_saImmOiImplementerSet(cb->immOiHandle, applierName);
	(void)immutil_saImmOiClassImplementerSet(cb->immOiHandle, "OpenSafLogConfig");
	immutilWrapperProfile.nTries = 20; /* Reset retry time to more normal value. */

	we_are_applier_flag = true;

	TRACE_LEAVE();
}


/**
 * Retrieve the LOG stream configuration from IMM using the
 * IMM-OM interface and initialize the corresponding information
 * in the LOG control block. Initialize the LOG IMM-OI
 * interface. Become class implementer.
 */
SaAisErrorT lgs_imm_activate(lgs_cb_t *cb)
{
	SaAisErrorT rc = SA_AIS_OK;
	log_stream_t *stream;

	TRACE_ENTER();
    
	if ((rc = stream_create_and_configure(SA_LOG_STREAM_ALARM, &cb->alarmStream, 0)) != SA_AIS_OK)
		goto done;

	if ((rc = stream_create_and_configure(SA_LOG_STREAM_NOTIFICATION, &cb->notificationStream, 1)) != SA_AIS_OK)
		goto done;

	if ((rc = stream_create_and_configure(SA_LOG_STREAM_SYSTEM, &cb->systemStream, 2)) != SA_AIS_OK)
		goto done;

	immutilWrapperProfile.nTries = 250; /* After loading,allow missed sync of large data to complete */

	(void)immutil_saImmOiImplementerSet(cb->immOiHandle, implementerName);
	(void)immutil_saImmOiClassImplementerSet(cb->immOiHandle, "SaLogStreamConfig");

	/* Do this only if the class exists */
	if ( true == *(bool*) lgs_imm_logconf_get(LGS_IMM_LOG_OPENSAFLOGCONFIG_CLASS_EXIST, NULL)) {
		(void)immutil_saImmOiClassImplementerSet(cb->immOiHandle, "OpenSafLogConfig");
	}

	immutilWrapperProfile.nTries = 20; /* Reset retry time to more normal value. */


	/* Update creation timestamp, must be object implementer first */
	(void)immutil_update_one_rattr(cb->immOiHandle, SA_LOG_STREAM_ALARM,
				       "saLogStreamCreationTimestamp", SA_IMM_ATTR_SATIMET,
				       &cb->alarmStream->creationTimeStamp);
	(void)immutil_update_one_rattr(cb->immOiHandle, SA_LOG_STREAM_NOTIFICATION,
				       "saLogStreamCreationTimestamp", SA_IMM_ATTR_SATIMET,
				       &cb->notificationStream->creationTimeStamp);
	(void)immutil_update_one_rattr(cb->immOiHandle, SA_LOG_STREAM_SYSTEM,
				       "saLogStreamCreationTimestamp", SA_IMM_ATTR_SATIMET,
				       &cb->systemStream->creationTimeStamp);

	/* Open all streams */
	stream = log_stream_getnext_by_name(NULL);
	while (stream != NULL) {
		if (log_stream_open(stream) != SA_AIS_OK)
			goto done;

		stream = log_stream_getnext_by_name(stream->name);
	}

 done:
	TRACE_LEAVE();
	return rc;
}

/**
 * Become object and class implementer/applier. Wait max
 * 'max_waiting_time_ms'.
 * @param _cb
 * 
 * @return void*
 */
static void *imm_impl_set(void *_cb)
{
	SaAisErrorT rc;
	int msecs_waited;
	lgs_cb_t *cb = (lgs_cb_t *)_cb;
	SaImmOiImplementerNameT iname;

	TRACE_ENTER();

	/* Become object implementer or applier dependent on if standby or active
	 * state
	 */
	if (cb->ha_state == SA_AMF_HA_ACTIVE) {
		iname = implementerName;
	} else {
		iname = applierName;
	}

	msecs_waited = 0;
	rc = saImmOiImplementerSet(cb->immOiHandle, iname);
	while ((rc == SA_AIS_ERR_TRY_AGAIN) && (msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOiImplementerSet(cb->immOiHandle, iname);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiImplementerSet failed %u", rc);
		exit(EXIT_FAILURE);
	}

	/* Become class implementer/applier for the OpenSafLogConfig class if it
	 * exists
	 */
	if ( true == *(bool*) lgs_imm_logconf_get(LGS_IMM_LOG_OPENSAFLOGCONFIG_CLASS_EXIST, NULL)) {
		(void)immutil_saImmOiClassImplementerSet(cb->immOiHandle, "OpenSafLogConfig");
	}

	/* Become class implementer for the SaLogStreamConfig class only if we are
	 * active.
	 */
	if (cb->ha_state == SA_AMF_HA_ACTIVE) {
		msecs_waited = 0;
		rc = saImmOiClassImplementerSet(cb->immOiHandle, "SaLogStreamConfig");
		while ((rc == SA_AIS_ERR_TRY_AGAIN) && (msecs_waited < max_waiting_time_ms)) {
			usleep(sleep_delay_ms * 1000);
			msecs_waited += sleep_delay_ms;
			rc = saImmOiClassImplementerSet(cb->immOiHandle, "SaLogStreamConfig");
		}
	}
	
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiClassImplementerSet failed %u", rc);
		exit(EXIT_FAILURE);
	}

	TRACE_LEAVE();
	return NULL;
}

/**
 * Become object and class implementer or applier, non-blocking.
 * @param cb
 */
void lgs_imm_impl_set(lgs_cb_t *cb)
{
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	/* In standby state: Become applier for configuration object if it exists.
	 * In active state: Become object implementer.
	 */
	if (cb->ha_state == SA_AMF_HA_STANDBY) {
		if (!*(bool*) lgs_imm_logconf_get(LGS_IMM_LOG_OPENSAFLOGCONFIG_CLASS_EXIST, NULL)) {
			return;
		}
	}

	TRACE_ENTER();
	if (pthread_create(&thread, &attr, imm_impl_set, cb) != 0) {
		LOG_ER("pthread_create FAILED: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	pthread_attr_destroy(&attr);
	
	TRACE_LEAVE();
}

/**
 * Initialize the OI interface and get a selection object. Wait
 * max 'max_waiting_time_ms'.
 * @param cb
 * 
 * @return SaAisErrorT
 */
SaAisErrorT lgs_imm_init(lgs_cb_t *cb)
{
	SaAisErrorT rc;
	int msecs_waited;

	TRACE_ENTER();

	msecs_waited = 0;
	rc = saImmOiInitialize_2(&cb->immOiHandle, &callbacks, &immVersion);
	while ((rc == SA_AIS_ERR_TRY_AGAIN) && (msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOiInitialize_2(&cb->immOiHandle, &callbacks, &immVersion);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOiInitialize_2 failed %u", rc);
		return rc;
	}

	msecs_waited = 0;
	rc = saImmOiSelectionObjectGet(cb->immOiHandle, &cb->immSelectionObject);
	while ((rc == SA_AIS_ERR_TRY_AGAIN) && (msecs_waited < max_waiting_time_ms)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOiSelectionObjectGet(cb->immOiHandle, &cb->immSelectionObject);
	}

	if (rc != SA_AIS_OK)
		LOG_ER("saImmOiSelectionObjectGet failed %u", rc);

	TRACE_LEAVE();

	return rc;
}
