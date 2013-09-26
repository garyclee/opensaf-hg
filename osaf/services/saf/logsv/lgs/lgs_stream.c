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

/*****************************************************************************
 * Important information
 * ---------------------
 * To prevent log service thread from "hanging" if communication with NFS is
 * not working or is very slow all functions using file I/O must run their file
 * handling in a separate thread.
 * For more information on how to do that see the following files:
 * lgs_file.h, lgs_file.c, lgs_filendl.c and lgs_filehdl.h
 * Examples can be found in this file, e.g. function fileopen(...) below
 */

#include "lgs.h"
#include "lgs_file.h"
#include "lgs_filehdl.h"

#define DEFAULT_NUM_APP_LOG_STREAMS 64
#define LGS_LOG_FILE_EXT ".log"
#define LGS_LOG_FILE_CONFIG_EXT ".cfg"

static NCS_PATRICIA_TREE stream_dn_tree;

static log_stream_t **stream_array;
/* We have at least the 3 well known streams. */
static unsigned int stream_array_size = 3;
/* Current number of streams */
static unsigned int numb_of_streams;

static int lgs_stream_array_insert(log_stream_t *stream, uint32_t id);
static int lgs_stream_array_insert_new(log_stream_t *stream, uint32_t *id);
static int lgs_stream_array_remove(int id);
static int get_number_of_log_files_h(log_stream_t *logStream, char *oldest_file);

/**
 * Open/create a file for append in non blocking mode.
 * @param filepath[in]
 * @param errno_save[out], errno if error
 * @return File descriptor or -1 if error
 */
static int fileopen_h(char *filepath, int *errno_save)
{
	lgsf_apipar_t apipar;
	lgsf_retcode_t api_rc;
	size_t filepath_len;
	int fd;
	
	TRACE_ENTER();
	
	osafassert(filepath != NULL);
	filepath_len = strlen(filepath)+1; /* Include terminating null character */
	
	if (filepath_len > PATH_MAX) {
		LOG_WA("Cannot open file, File path > PATH_MAX");
		fd = -1;
		goto done;
	}
	
	TRACE("%s - filepath \"%s\"",__FUNCTION__,filepath);
	
	/* Fill in API structure */
	apipar.req_code_in = LGSF_FILEOPEN;
	apipar.data_in_size = filepath_len;
	apipar.data_in = (void*) filepath;
	apipar.data_out_size = sizeof(int);
	apipar.data_out = (void *) errno_save;
	
	api_rc = log_file_api(&apipar);
	if (api_rc != LGSF_SUCESS) {
		TRACE("%s - API error %s",__FUNCTION__,lgsf_retcode_str(api_rc));
		fd = -1;
	} else {
		fd = apipar.hdl_ret_code_out;
	}
	
done:
	TRACE_LEAVE();
	return fd;
}

/**
 * Close with retry at EINTR
 * @param fd
 *
 * @return int
 */
static int fileclose_h(int fd)
{
	lgsf_apipar_t apipar;
	lgsf_retcode_t api_rc;
	int rc = 0;
	
	TRACE_ENTER();
	
	/* Fill in API structure */
	apipar.req_code_in = LGSF_FILECLOSE;
	apipar.data_in_size = sizeof(int);
	apipar.data_in = (void*) &fd;
	apipar.data_out_size = 0;
	apipar.data_out = NULL;
	
	api_rc = log_file_api(&apipar);
	if (api_rc != LGSF_SUCESS) {
		TRACE("%s - API error %s",__FUNCTION__,lgsf_retcode_str(api_rc));
		rc = -1;
	} else {
		rc = apipar.hdl_ret_code_out;
	}

	TRACE_LEAVE2("rc = %d",rc);
	return rc;
}

/**
 * Delete a file (unlink())
 * 
 * @param filepath A null terminated string containing the name of the file to
 *        be deleted
 * @return (-1) if error
 */
static int file_unlink_h(char *filepath)
{
	int rc = 0;
	lgsf_apipar_t apipar;
	lgsf_retcode_t api_rc;
	size_t filepath_len;
	
	TRACE_ENTER();
	filepath_len = strlen(filepath)+1; /* Include terminating null character */
	if (filepath_len > PATH_MAX) {
		LOG_WA("Cannot delete file, File path > PATH_MAX");
		rc = -1;
		goto done;
	}
	
	/* Fill in API structure */
	apipar.req_code_in = LGSF_DELETE_FILE;
	apipar.data_in_size = filepath_len;
	apipar.data_in = (void*) filepath;
	apipar.data_out_size = 0;
	apipar.data_out = NULL;
	
	api_rc = log_file_api(&apipar);
	if (api_rc != LGSF_SUCESS) {
		TRACE("%s - API error %s",__FUNCTION__,lgsf_retcode_str(api_rc));
		rc = -1;
	} else {
		rc = apipar.hdl_ret_code_out;
	}

done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
	
}

/**
 * Delete config file.
 * @param stream
 * 
 * @return -1 if error
 */
static int delete_config_file(log_stream_t *stream)
{
	int rc = 0;
	int n;
	char pathname[PATH_MAX];

	TRACE_ENTER();

	/* create absolute path for config file */
	n = snprintf(pathname, PATH_MAX, "%s/%s/%s.cfg", lgs_cb->logsv_root_dir, stream->pathName, stream->fileName);
	if (n >= PATH_MAX) {
		LOG_WA("Config file could not be deleted, path > PATH_MAX");
		rc = -1;
		goto done;
	}

	rc = file_unlink_h(pathname);
	
done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
}

static int rotate_if_needed(log_stream_t *stream)
{
	char oldest_file[PATH_MAX];
	int rc = 0;
	int file_cnt;

	TRACE_ENTER();

	/* Rotate out files from previous lifes */
	if ((file_cnt = get_number_of_log_files_h(stream, oldest_file)) == -1) {
		rc = -1;
		goto done;
	}

	/*
	 ** Remove until we have one less than allowed, we are just about to
	 ** create a new one again.
	 */
	while (file_cnt >= stream->maxFilesRotated) {
		TRACE_1("remove oldest file: %s", oldest_file);
		if ((rc = file_unlink_h(oldest_file)) == -1) {
			LOG_NO("could not delete: %s - %s", oldest_file, strerror(errno));
			goto done;
		}

		if ((file_cnt = get_number_of_log_files_h(stream, oldest_file)) == -1) {
			rc = -1;
			goto done;
		}
	}

 done:
	TRACE_LEAVE2("rc = %d", rc);
	return rc;
}

static uint32_t log_stream_add(NCS_PATRICIA_NODE *node, const char *key)
{
	uint32_t rc = NCSCC_RC_SUCCESS;

	node->key_info = (uint8_t *)key;

	if ( NULL == ncs_patricia_tree_get(&stream_dn_tree,node->key_info)){
		rc = ncs_patricia_tree_add(&stream_dn_tree, node);
		if (rc != NCSCC_RC_SUCCESS) {
			LOG_WA("ncs_patricia_tree_add FAILED for '%s' %u", key, rc);
			node->key_info = NULL;
			goto done;
		}
	}

 done:
	return rc;
}

static uint32_t log_stream_remove(const char *key)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	log_stream_t *stream;

	stream = (log_stream_t *)ncs_patricia_tree_get(&stream_dn_tree, (uint8_t *)key);
	if (stream == NULL) {
		TRACE_2("ncs_patricia_tree_get FAILED");
		rc = NCSCC_RC_FAILURE;
		goto done;
	}
	
	if (ncs_patricia_tree_get(&stream_dn_tree,stream->pat_node.key_info)){
		if ((rc = ncs_patricia_tree_del(&stream_dn_tree, &stream->pat_node)) != NCSCC_RC_SUCCESS) {
			LOG_WA("ncs_patricia_tree_del FAILED for  '%s' %u",key,rc);
			goto done;
		}
	}

 done:
	return rc;
}

/**
 * Initiate the files belonging to a stream
 * 
 * @param stream
 */
static void log_initiate_stream_files(log_stream_t *stream)
{
	int errno_save;
	
	TRACE_ENTER();
	
	/* Initiate stream file descriptor to flag that no valid descriptor exist */
	stream->fd = -1;
	
	/* Delete to get counting right. It might not exist. */
	(void)delete_config_file(stream);

	/* Remove files from a previous life if needed */
	if (rotate_if_needed(stream) == -1) {
		TRACE("%s - rotate_if_needed() FAIL",__FUNCTION__);
		goto done;
	}

	if (lgs_make_reldir_h(stream->pathName) != 0){
		TRACE("%s - lgs_make_dir_h() FAIL",__FUNCTION__);
		goto done;
	}

	if (lgs_create_config_file_h(stream) != 0) {
		TRACE("%s - lgs_create_config_file_h() FAIL",__FUNCTION__);
		goto done;
	}

	if ((stream->fd = log_file_open(stream, &errno_save)) == -1) {
		TRACE("%s - Could not open '%s' - %s",__FUNCTION__,
				stream->logFileCurrent, strerror(errno_save));
		goto done;
	}

done:
	TRACE_LEAVE();
}

log_stream_t *log_stream_get_by_name(const char *name)
{
	char nname[SA_MAX_NAME_LENGTH + 1];

	/* Create null-terminated stream name */
	strcpy(nname, name);
	memset(&nname[strlen(name)], 0, SA_MAX_NAME_LENGTH + 1 - strlen(name));

	return (log_stream_t *)ncs_patricia_tree_get(&stream_dn_tree, (uint8_t *)nname);
}

log_stream_t *log_stream_getnext_by_name(const char *name)
{
	char nname[SA_MAX_NAME_LENGTH + 1];

	if (name != NULL) {
		/* Create SA_MAX_NAME_LENGTH stream name */
		strcpy(nname, name);
		memset(&nname[strlen(name)], 0, SA_MAX_NAME_LENGTH + 1 - strlen(name));
		return (log_stream_t *)ncs_patricia_tree_getnext(&stream_dn_tree, (uint8_t *)nname);
	} else
		return (log_stream_t *)ncs_patricia_tree_getnext(&stream_dn_tree, NULL);
}

void log_stream_print(log_stream_t *stream)
{
	osafassert(stream != NULL);

	TRACE_2("******** Stream %s ********", stream->name);
	TRACE_2("  fileName:             %s", stream->fileName);
	TRACE_2("  pathName:             %s", stream->pathName);
	TRACE_2("  maxLogFileSize:       %llu", stream->maxLogFileSize);
	TRACE_2("  fixedLogRecordSize:   %u", stream->fixedLogRecordSize);
	if (stream->streamType == STREAM_TYPE_APPLICATION)
		TRACE_2("  haProperty:           %u", stream->haProperty);
	TRACE_2("  logFullAction:        %u", stream->logFullAction);
	TRACE_2("  logFullHaltThreshold: %u", stream->logFullHaltThreshold);
	TRACE_2("  maxFilesRotated:      %u", stream->maxFilesRotated);
	TRACE_2("  logFileFormat:        %s", stream->logFileFormat);
	TRACE_2("  severityFilter:       %x", stream->severityFilter);
	TRACE_2("  creationTimeStamp:    %llu", stream->creationTimeStamp);
	TRACE_2("  numOpeners:           %u", stream->numOpeners);
	TRACE_2("  streamId:             %u", stream->streamId);
	TRACE_2("  fd:                   %d", stream->fd);
	TRACE_2("  logFileCurrent:       %s", stream->logFileCurrent);
	TRACE_2("  curFileSize:          %u", stream->curFileSize);
	TRACE_2("  logRecordId:          %u", stream->logRecordId);
	TRACE_2("  streamType:           %u", stream->streamType);
	TRACE_2("  filtered:             %llu", stream->filtered);
}

void log_stream_delete(log_stream_t **s)
{
	log_stream_t *stream;

	osafassert(s != NULL && *s != NULL);
	stream = *s;

	TRACE_ENTER2("%s", stream->name);

	if (lgs_cb->ha_state == SA_AMF_HA_ACTIVE) {
		if ((stream->streamType == STREAM_TYPE_APPLICATION) &&
				(!strncmp(stream->name, "safLgStr=", 9))) {
			SaAisErrorT rv;
			TRACE("Stream is closed, I am HA active so remove IMM object");
			SaNameT objectName;
			strcpy((char *)objectName.value, stream->name);
			objectName.length = strlen((char *)objectName.value);
			rv = saImmOiRtObjectDelete(lgs_cb->immOiHandle, &objectName);
			if (rv != SA_AIS_OK) {
				/* no retry; avoid blocking LOG service #1886 */
				LOG_WA("saImmOiRtObjectDelete returned %u for %s",
						rv, stream->name);
			}
		}
	}

	if (stream->streamId != 0)
		lgs_stream_array_remove(stream->streamId);

	if (stream->pat_node.key_info != NULL)
		log_stream_remove(stream->name);

	if (stream->logFileFormat != NULL)
		free(stream->logFileFormat);

	free(stream);
	*s = NULL;
	TRACE_LEAVE();
}

/**
 * Create a new stream object. If HA state active, create the
 * correspronding IMM runtime object.
 * 
 * @param name
 * @param filename
 * @param pathname
 * @param maxLogFileSize
 * @param fixedLogRecordSize
 * @param logFullAction
 * @param maxFilesRotated
 * @param logFileFormat
 * @param streamType
 * @param stream_id
 * @param twelveHourModeFlag
 * @param logRecordId
 * 
 * @return log_stream_t*
 */
log_stream_t *log_stream_new(SaNameT *dn,
			     const char *filename,
			     const char *pathname,
			     SaUint64T maxLogFileSize,
			     SaUint32T fixedLogRecordSize,
			     SaLogFileFullActionT logFullAction,
			     SaUint32T maxFilesRotated,
			     const char *logFileFormat,
			     logStreamTypeT streamType, int stream_id, SaBoolT twelveHourModeFlag, uint32_t logRecordId)
{
	int rc;
	log_stream_t *stream = NULL;

	osafassert(dn != NULL);
	TRACE_ENTER2("%s, l: %u", dn->value, dn->length);

	if (lgs_relative_path_check_ts(pathname)) {
		goto done;
	}
	stream = calloc(1, sizeof(log_stream_t));
	if (stream == NULL) {
		LOG_WA("log_stream_new calloc FAILED");
		goto done;
	}
	memcpy(stream->name, dn->value, dn->length);
	stream->name[SA_MAX_NAME_LENGTH] = '\0';
	strncpy(stream->fileName, filename, sizeof(stream->fileName));
	strncpy(stream->pathName, pathname, sizeof(stream->pathName));
	stream->maxLogFileSize = maxLogFileSize;
	stream->fixedLogRecordSize = fixedLogRecordSize;
	stream->haProperty = SA_TRUE;
	stream->logFullAction = logFullAction;
	stream->streamId = stream_id;
	stream->logFileFormat = strdup(logFileFormat);
	if (stream->logFileFormat == NULL) {
		log_stream_delete(&stream);
		goto done;
	}
	stream->maxFilesRotated = maxFilesRotated;
	stream->creationTimeStamp = lgs_get_SaTime();
	stream->fd = -1;
	stream->severityFilter = 0x7f;	/* by default all levels are allowed */
	stream->streamType = streamType;
	stream->twelveHourModeFlag = twelveHourModeFlag;
	stream->logRecordId = logRecordId;

	/* Add stream to tree */
	if (log_stream_add(&stream->pat_node, stream->name) != NCSCC_RC_SUCCESS) {
		log_stream_delete(&stream);
		goto done;
	}

	/* Add stream to array */
	if (stream->streamId == -1)
		rc = lgs_stream_array_insert_new(stream, &stream->streamId);
	else
		rc = lgs_stream_array_insert(stream, stream->streamId);

	if (rc < 0) {
		LOG_WA("Add stream to array FAILED");
		log_stream_delete(&stream);
		goto done;
	}

	if (lgs_cb->ha_state == SA_AMF_HA_ACTIVE) {
		char *dndup = strdup(stream->name);
		char *parent_name = strchr(stream->name, ',');
		char *rdnstr;
		SaNameT parent, *parentName = NULL;

		if (parent_name != NULL) {
			rdnstr = strtok(dndup, ",");
			parent_name++;	/* FIX, vulnerable for malformed DNs */
			parentName = &parent;
			strcpy((char *)parent.value, parent_name);
			parent.length = strlen((char *)parent.value);
		} else
			rdnstr = stream->name;

		void *arr1[] = { &rdnstr };
		const SaImmAttrValuesT_2 attr_safLgStr = {
			.attrName = "safLgStr",
			.attrValueType = SA_IMM_ATTR_SASTRINGT,
			.attrValuesNumber = 1,
			.attrValues = arr1
		};
		char *str2 = stream->fileName;
		void *arr2[] = { &str2 };
		const SaImmAttrValuesT_2 attr_safLogStreamFileName = {
			.attrName = "saLogStreamFileName",
			.attrValueType = SA_IMM_ATTR_SASTRINGT,
			.attrValuesNumber = 1,
			.attrValues = arr2
		};
		char *str3 = stream->pathName;
		void *arr3[] = { &str3 };
		const SaImmAttrValuesT_2 attr_safLogStreamPathName = {
			.attrName = "saLogStreamPathName",
			.attrValueType = SA_IMM_ATTR_SASTRINGT,
			.attrValuesNumber = 1,
			.attrValues = arr3
		};
		void *arr4[] = { &stream->maxLogFileSize };
		const SaImmAttrValuesT_2 attr_saLogStreamMaxLogFileSize = {
			.attrName = "saLogStreamMaxLogFileSize",
			.attrValueType = SA_IMM_ATTR_SAUINT64T,
			.attrValuesNumber = 1,
			.attrValues = arr4
		};
		void *arr5[] = { &stream->fixedLogRecordSize };
		const SaImmAttrValuesT_2 attr_saLogStreamFixedLogRecordSize = {
			.attrName = "saLogStreamFixedLogRecordSize",
			.attrValueType = SA_IMM_ATTR_SAUINT32T,
			.attrValuesNumber = 1,
			.attrValues = arr5
		};
		void *arr6[] = { &stream->haProperty };
		const SaImmAttrValuesT_2 attr_saLogStreamHaProperty = {
			.attrName = "saLogStreamHaProperty",
			.attrValueType = SA_IMM_ATTR_SAUINT32T,
			.attrValuesNumber = 1,
			.attrValues = arr6
		};
		void *arr7[] = { &stream->logFullAction };
		const SaImmAttrValuesT_2 attr_saLogStreamLogFullAction = {
			.attrName = "saLogStreamLogFullAction",
			.attrValueType = SA_IMM_ATTR_SAUINT32T,
			.attrValuesNumber = 1,
			.attrValues = arr7
		};
		void *arr8[] = { &stream->maxFilesRotated };
		const SaImmAttrValuesT_2 attr_saLogStreamMaxFilesRotated = {
			.attrName = "saLogStreamMaxFilesRotated",
			.attrValueType = SA_IMM_ATTR_SAUINT32T,
			.attrValuesNumber = 1,
			.attrValues = arr8
		};
		char *str9 = stream->logFileFormat;
		void *arr9[] = { &str9 };
		const SaImmAttrValuesT_2 attr_saLogStreamLogFileFormat = {
			.attrName = "saLogStreamLogFileFormat",
			.attrValueType = SA_IMM_ATTR_SASTRINGT,
			.attrValuesNumber = 1,
			.attrValues = arr9
		};
		void *arr10[] = { &stream->severityFilter };
		const SaImmAttrValuesT_2 attr_saLogStreamSeverityFilter = {
			.attrName = "saLogStreamSeverityFilter",
			.attrValueType = SA_IMM_ATTR_SAUINT32T,
			.attrValuesNumber = 1,
			.attrValues = arr10
		};
		void *arr11[] = { &stream->creationTimeStamp };
		const SaImmAttrValuesT_2 attr_saLogStreamCreationTimestamp = {
			.attrName = "saLogStreamCreationTimestamp",
			.attrValueType = SA_IMM_ATTR_SATIMET,
			.attrValuesNumber = 1,
			.attrValues = arr11
		};
		const SaImmAttrValuesT_2 *attrValues[] = {
			&attr_safLgStr,
			&attr_safLogStreamFileName,
			&attr_safLogStreamPathName,
			&attr_saLogStreamMaxLogFileSize,
			&attr_saLogStreamFixedLogRecordSize,
			&attr_saLogStreamHaProperty,
			&attr_saLogStreamLogFullAction,
			&attr_saLogStreamMaxFilesRotated,
			&attr_saLogStreamLogFileFormat,
			&attr_saLogStreamSeverityFilter,
			&attr_saLogStreamCreationTimestamp,
			NULL
		};

		/* parentName needs to be configurable? */
		{
			SaAisErrorT rv;

			rv = saImmOiRtObjectCreate_2(lgs_cb->immOiHandle,
							     "SaLogStream", parentName, attrValues);
			free(dndup);

			if (rv != SA_AIS_OK) {
				/* no retry; avoid blocking LOG service #1886 */
				LOG_WA("saImmOiRtObjectCreate_2 returned %u for %s, parent %s",
				      rv, stream->name, parent_name);
			}
		}
	}

 done:
	TRACE_LEAVE();
	return stream;
}

/**
 * Create a new stream object. Do not create an IMM object.
 * @param name
 * @param stream_id
 * 
 * @return log_stream_t*
 */
log_stream_t *log_stream_new_2(SaNameT *name, int stream_id)
{
	int rc;
	log_stream_t *stream = NULL;

	osafassert(name != NULL);
	TRACE_ENTER2("%s, l: %u", name->value, (unsigned int)name->length);

	stream = calloc(1, sizeof(log_stream_t));
	if (stream == NULL) {
		LOG_WA("calloc FAILED");
		goto done;
	}
	memcpy(stream->name, name->value, name->length);
	stream->name[SA_MAX_NAME_LENGTH] = '\0';
	stream->streamId = stream_id;
	stream->creationTimeStamp = lgs_get_SaTime();
	stream->fd = 0;
	stream->severityFilter = 0x7f;	/* by default all levels are allowed */

	/* Add stream to tree */
	if (log_stream_add(&stream->pat_node, stream->name) != NCSCC_RC_SUCCESS) {
		log_stream_delete(&stream);
		goto done;
	}

	/* Add stream to array */
	if (stream->streamId == -1)
		rc = lgs_stream_array_insert_new(stream, &stream->streamId);
	else
		rc = lgs_stream_array_insert(stream, stream->streamId);

	if (rc < 0) {
		log_stream_delete(&stream);
		goto done;
	}

 done:
	TRACE_LEAVE();
	return stream;
}

/**
 * Open log file associated with stream
 * The file is opened in non blocking mode.
 * @param stream
 * @param errno_save - errno from open() returned here if supplied
 *
 * @return int - the file descriptor or -1 on errors
 */
int log_file_open(log_stream_t *stream, int *errno_save)
{
	int fd;
	char pathname[PATH_MAX];
	int errno_ret;
	int n;

	TRACE_ENTER();

	n = snprintf(pathname, PATH_MAX, "%s/%s/%s.log",
			lgs_cb->logsv_root_dir, stream->pathName, stream->logFileCurrent);
	if (n >= PATH_MAX) {
		LOG_WA("Cannot open log file, path > PATH_MAX");
		fd = -1;
		goto done;
	}
	
	TRACE("%s - Opening file \"%s\"",__FUNCTION__,pathname);
	fd = fileopen_h(pathname, &errno_ret);
	if (errno_save != 0) {
		*errno_save = errno_ret;
	}

done:
	TRACE_LEAVE();
	return fd;
}

SaAisErrorT log_stream_open(log_stream_t *stream)
{
	SaAisErrorT rc = SA_AIS_OK;
	
	osafassert(stream != NULL);
	TRACE_ENTER2("%s, numOpeners=%u", stream->name, stream->numOpeners);

	/* first time open? */
	if (stream->numOpeners == 0) {
		/* Create and save current log file name */
		snprintf(stream->logFileCurrent, NAME_MAX, "%s_%s", stream->fileName, lgs_get_time());
		log_initiate_stream_files(stream);
	}
	
	/* Opening a stream will always succeed. If file system problem a new
	 * attempt to open the files is done when trying to write to the stream
	 */
	stream->numOpeners++;

	TRACE_LEAVE2("rc=%u, numOpeners=%u", rc, stream->numOpeners);
	return rc;
}

/**
 * if ref. count allows, close the associated file, rename it and delete the
 * stream object.
 * @param stream
 * 
 * @return int
 */
int log_stream_close(log_stream_t **s)
{
	int rc = 0;
	log_stream_t *stream = *s;

	osafassert(stream != NULL);
	TRACE_ENTER2("%s, numOpeners=%u", stream->name, stream->numOpeners);
	
	osafassert(stream->numOpeners > 0);
	stream->numOpeners--;

	if (stream->numOpeners == 0) {
		/* standard streams can never be deleted */
		osafassert(stream->streamType == STREAM_TYPE_APPLICATION);
		if (stream->fd != -1) {
			char *timeString = lgs_get_time();
			if ((rc = fileclose_h(stream->fd)) == -1) {
				LOG_ER("log_stream_close FAILED: %s", strerror(errno));
				goto done;
			}

			rc = lgs_file_rename_h(stream->pathName, stream->logFileCurrent, timeString, LGS_LOG_FILE_EXT);
			if (rc == -1)
				goto done;

			rc = lgs_file_rename_h(stream->pathName, stream->fileName, timeString, LGS_LOG_FILE_CONFIG_EXT);
			if (rc == -1)
				goto done;
		}


		log_stream_delete(s);
		stream = NULL;
	}

 done:
	TRACE_LEAVE2("rc=%d, numOpeners=%u", rc, stream ? stream->numOpeners : 0);
	return rc;
}

/**
 * Close the stream associated file.
 * @param stream
 * 
 * @return int
 */
int log_stream_file_close(log_stream_t *stream)
{
	int rc = 0;

	osafassert(stream != NULL);
	TRACE_ENTER2("%s", stream->name);

	osafassert(stream->numOpeners > 0);

	if (stream->fd != -1) {
		if ((rc = fileclose_h(stream->fd)) == -1) {
			LOG_ER("log_stream_file_close FAILED: %s", strerror(errno));
			stream->fd = -1; /* Force reset the fd, otherwise fd will be stale. */
		} else
			stream->fd = -1;
	}

	TRACE_LEAVE2("%d", rc);
	return rc;
}

/**
 * Return number of log files in a dir and the name of the oldest file.
 * @param logStream[in]
 * @param oldest_file[out]
 * 
 * @return -1 if error else number of log files
 */
static int get_number_of_log_files_h(log_stream_t *logStream, char *oldest_file)
{
	lgsf_apipar_t apipar;
	lgsf_retcode_t api_rc;
	gnolfh_in_t parameters_in;
	size_t path_len;
	int rc, n;
	
	TRACE_ENTER();
	
	n = snprintf(parameters_in.file_name, SA_MAX_NAME_LENGTH, "%s", logStream->fileName);
	if (n >= SA_MAX_NAME_LENGTH) {
		rc = -1;
		LOG_WA("file_name > SA_MAX_NAME_LENGTH");
		goto done;
	}
	n = snprintf(parameters_in.logsv_root_dir, PATH_MAX, "%s", lgs_cb->logsv_root_dir);
	if (n >= PATH_MAX) {
		rc = -1;
		LOG_WA("logsv_root_dir > PATH_MAX");
		goto done;
	}
	n = snprintf(parameters_in.pathName, PATH_MAX, "%s", logStream->pathName);
	if (n >= PATH_MAX) {
		rc = -1;
		LOG_WA("pathName > PATH_MAX");
		goto done;
	}
	
	path_len =	strlen(parameters_in.file_name) + 
				strlen(parameters_in.logsv_root_dir) +
				strlen(parameters_in.pathName);
	if (path_len > PATH_MAX) {
		LOG_WA("Path to log files > PATH_MAX");
		rc = -1;
		goto done;
	}
	
	/* Fill in API structure */
	apipar.req_code_in = LGSF_GET_NUM_LOGFILES;
	apipar.data_in_size = sizeof(gnolfh_in_t);
	apipar.data_in = (void*) &parameters_in;
	apipar.data_out_size = PATH_MAX;
	apipar.data_out = (void *) oldest_file;
	
	api_rc = log_file_api(&apipar);
	if (api_rc != LGSF_SUCESS) {
		TRACE("%s - API error %s",__FUNCTION__,lgsf_retcode_str(api_rc));
		rc = -1;
	} else {
		rc = apipar.hdl_ret_code_out;
	}

done:
	TRACE_LEAVE();
	return rc;
}

/**
 * log_stream_write will write a number of bytes to the associated file. If
 * the file size gets too big, the file is closed, renamed and a new file is
 * opened. If there are too many files, the oldest file will be deleted.
 * 
 * @param stream
 * @param buf
 * @param count
 * 
 * @return int 0 No error
 *            -1 on error
 *            -2 Write failed because of write timeout or EWOULDBLOCK/EAGAIN
 */
int log_stream_write_h(log_stream_t *stream, const char *buf, size_t count)
{
	int rc = 0;
	lgsf_apipar_t apipar;
	void *params_in;
	wlrh_t *header_in_p;
	char *logrec_p;
	size_t params_in_size;
	lgsf_retcode_t api_rc;
	int write_errno=0;

	osafassert(stream != NULL && buf != NULL);
	TRACE_ENTER2("%s", stream->name);

	/* Open files on demand e.g. on new active after fail/switch-over. This
	 * enables LOG to cope with temporary file system problems. */
	if (stream->fd == -1) {
		/* Create directory and log files if they were not created at
		 * stream open or reopen files if bad file descriptor.
		 */
		log_initiate_stream_files(stream);
		
		if (stream->fd == -1) {
			TRACE("%s - Initiating stream files \"%s\" Failed", __FUNCTION__,
					stream->name);
		} else {
			TRACE("%s - stream files initiated", __FUNCTION__);
		}
	}
	
	TRACE("%s - stream->fd = %d",__FUNCTION__,stream->fd);
	/* Write the log record
	 */
	/* allocate memory for header + log record */
	params_in_size = sizeof(wlrh_t) + count;
	params_in = malloc(params_in_size);
	
	header_in_p = (wlrh_t *) params_in;
	logrec_p = (char *) (params_in + sizeof(wlrh_t));
	
	header_in_p->fd = stream->fd;
	header_in_p->fixedLogRecordSize = stream->fixedLogRecordSize;
	header_in_p->record_size = count;
	memcpy(logrec_p, buf, count);
	
	/* Fill in API structure */
	apipar.req_code_in = LGSF_WRITELOGREC;
	apipar.data_in_size = params_in_size;
	apipar.data_in = params_in;
	apipar.data_out_size = sizeof(int);
	apipar.data_out = (void *) &write_errno;
	
	api_rc = log_file_api(&apipar);
	if (api_rc == LGSF_TIMEOUT) {
		TRACE("%s - API error %s",__FUNCTION__,lgsf_retcode_str(api_rc));
		rc = -2;
	} else if (api_rc != LGSF_SUCESS) {
		TRACE("%s - API error %s",__FUNCTION__,lgsf_retcode_str(api_rc));
		rc = -1;
	} else {
		rc = apipar.hdl_ret_code_out;
	}

	free(params_in);
	/* End write the log record */	
	
	if ((rc == -1) || (rc == -2)) {
		/* If writing failed always invalidate the stream file descriptor.
		 */
		/* Careful with log level here to avoid syslog flooding */
		LOG_IN("write '%s' failed \"%s\"", stream->logFileCurrent,
				strerror(write_errno));
		
		if (stream->fd != -1) {
			/* Try to close the file and invalidate the stream fd */
			fileclose_h(stream->fd);
			stream->fd = -1;
		}
		
		if ((write_errno == EAGAIN) || (write_errno == EWOULDBLOCK)) {
			/* Change return code to timeout if EAGAIN (would block) */
			TRACE("Write would block");
			rc = -2;
		}
		goto done;
	}
 
	/* Handle file size and rotate if needed
	 */
	rc = 0;
	stream->curFileSize += count;

	if ((stream->curFileSize + count) > stream->maxLogFileSize) {
		int errno_save;
		char *current_time = lgs_get_time();

		if ((rc = fileclose_h(stream->fd)) == -1) {
			LOG_IN("close FAILED: %s", strerror(errno));
			goto done;
		}
		stream->fd = -1;

		rc = lgs_file_rename_h(stream->pathName, stream->logFileCurrent, current_time, LGS_LOG_FILE_EXT);
		if (rc == -1)
			goto done;

		/* Invalidate logFileCurrent for this stream */
		stream->logFileCurrent[0] = 0;

		/* Remove oldest file if needed */
		if ((rc = rotate_if_needed(stream)) == -1)
			goto done;

		stream->creationTimeStamp = lgs_get_SaTime();
		snprintf(stream->logFileCurrent, NAME_MAX, "%s_%s", stream->fileName, current_time);
		if ((stream->fd = log_file_open(stream, &errno_save)) == -1) {
			LOG_IN("Could not open '%s' - %s", stream->logFileCurrent, strerror(errno_save));
			rc = -1;
			goto done;
		}

		stream->curFileSize = 0;
	}

 done:
	TRACE_LEAVE2("rc=%d", rc);
	return rc;
}

/**
 * Get stream from array
 * @param lgs_cb
 * @param id
 * 
 * @return log_stream_t*
 */
log_stream_t *log_stream_get_by_id(uint32_t id)
{
	log_stream_t *stream = NULL;

	if (0 <= id && id < stream_array_size)
		stream = stream_array[id];

	return stream;
}

/**
 * Insert stream into array at specified position.
 * @param stream
 * @param id
 * 
 * @return int 0 if OK, <0 if failed
 */
static int lgs_stream_array_insert(log_stream_t *stream, uint32_t id)
{
	int rc = 0;

	if (numb_of_streams >= stream_array_size) {
		rc = -1;
		goto exit;
	}

	if (id >= stream_array_size) {
		rc = -2;
		goto exit;
	}

	if (stream_array[stream->streamId] != NULL) {
		rc = -3;
		goto exit;
	}

	stream_array[id] = stream;
	numb_of_streams++;

 exit:
	return rc;
}

/**
 * Add new stream to array. Update the stream ID.
 * @param lgs_cb
 * @param stream
 * 
 * @return int
 */
static int lgs_stream_array_insert_new(log_stream_t *stream, uint32_t *id)
{
	int rc = -1;
	int i;

	osafassert(id != NULL);

	if (numb_of_streams >= stream_array_size)
		goto exit;

	for (i = 0; i < stream_array_size; i++) {
		if (stream_array[i] == NULL) {
			stream_array[i] = stream;
			*id = i;
			numb_of_streams++;
			rc = 0;
			break;
		}
	}

 exit:
	return rc;
}

/**
 * Remove stream from array
 * @param lgs_cb
 * @param n
 * 
 * @return int
 */
static int lgs_stream_array_remove(int id)
{
	int rc = -1;

	if (0 <= id && id < stream_array_size) {
		osafassert(stream_array[id] != NULL);
		stream_array[id] = NULL;
		rc = 0;
		osafassert(numb_of_streams > 0);
		numb_of_streams--;
	}

	return rc;
}

/**
 * Dump the stream array
 */
void log_stream_id_print(void)
{
	int i;

	TRACE("  Current number of streams: %u", numb_of_streams);
	for (i = 0; i < stream_array_size; i++) {
		if (stream_array[i] != NULL)
			TRACE("    Id %u - %s", i, stream_array[i]->name);
	}
}

uint32_t log_stream_init(void)
{
	NCS_PATRICIA_PARAMS param;
	SaUint32T value;
	bool wa_flag = false;

	/* Get configuration of how many application streams we should allow. */
	value = *(SaUint32T *) lgs_imm_logconf_get(LGS_IMM_LOG_MAX_APPLICATION_STREAMS, &wa_flag);
	/* Check for correct conversion and a reasonable amount */
	if ( wa_flag == true) {
		LOG_WA("Env var LOG_MAX_APPLICATION_STREAMS has wrong value, using default");
		stream_array_size += DEFAULT_NUM_APP_LOG_STREAMS;
	} else {
		stream_array_size += value;
	}

	TRACE("Max %u application log streams", stream_array_size - 3);
	stream_array = calloc(1, sizeof(log_stream_t *) * stream_array_size);
	if (stream_array == NULL) {
		LOG_WA("calloc FAILED");
		return NCSCC_RC_FAILURE;
	}

	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));

	param.key_size = SA_MAX_NAME_LENGTH;

	return ncs_patricia_tree_init(&stream_dn_tree, &param);
}

/**
 * Close log file, change name of log file, optionally create new log and
 * config file. Basically the same logic as described in 3.1.6.4
 * in A.02.01.
 * create_files_f = true; New files are created
 * create_files_f = false; New files are not created
 * @param conf_mode
 * @param stream
 * @param current_file_name
 * 
 * @return int
 */
int log_stream_config_change(bool create_files_f, log_stream_t *stream, const char *current_file_name)
{
	int rc;
	char *current_time;

	TRACE_ENTER2("%s", stream->name);

	/* Peer sync needed due to change in logFileCurrent */
	current_time = lgs_get_time();
	
	if (stream->fd == -1) {
		/* lgs has not yet recieved any stream operation request after this swtchover/failover.
		 *  stream shall be opened on-request after a switchover, failover
		 */	
		TRACE("log file of the stream: %s does not exist",stream->name);
	} else {
		/* close the existing log file, and only when there is a valid fd */

		if ((rc = fileclose_h(stream->fd)) == -1) {
			LOG_ER("log_stream_config_change file close  FAILED: %s", strerror(errno));
			goto done;
		}

		if ((rc = lgs_file_rename_h(stream->pathName, stream->logFileCurrent, current_time, LGS_LOG_FILE_EXT)) == -1) {
			goto done;
		}

		if ((rc = lgs_file_rename_h(stream->pathName, current_file_name, current_time, LGS_LOG_FILE_CONFIG_EXT)) == -1) {
			goto done;
		}
	}

	/* Creating the new config file */
	if (create_files_f == LGS_STREAM_CREATE_FILES) {
		if ((rc = lgs_create_config_file_h(stream)) != 0)
			goto done;

		sprintf(stream->logFileCurrent, "%s_%s", stream->fileName, current_time);

		/* Create the new log file based on updated configuration */
		stream->fd = log_file_open(stream, NULL);
	}

	if (stream->fd == -1)
		rc = -1;
	else
		rc = 0;

 done:
	TRACE_LEAVE();
	return rc;
}
