/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 * Copyright Ericsson AB 2008, 2017 - All Rights Reserved.
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
#include "logtest.h"

#define MAX_DATA	256
#define MAX_CLIENTS	2
static SaLogSeverityFlagsT log_severity[8];
static SaLogStreamHandleT log_streamHandle[8];
static int cb_index;

static void logFilterSetCallbackT(SaLogStreamHandleT logStreamHandle, SaLogSeverityFlagsT logSeverity)
{
	log_streamHandle[cb_index] = logStreamHandle;
	log_severity[cb_index] = logSeverity;
	cb_index++;
}

static SaLogFileCreateAttributesT_2 appStreamLogFileCreateAttributes =
{
	.logFilePathName = DEFAULT_APP_FILE_PATH_NAME,
	.logFileName = DEFAULT_APP_FILE_NAME,
	.maxLogFileSize = DEFAULT_APP_LOG_FILE_SIZE,
	.maxLogRecordSize = DEFAULT_APP_LOG_REC_SIZE,
	.haProperty = SA_TRUE,
	.logFileFullAction = SA_LOG_FILE_FULL_ACTION_ROTATE,
	.maxFilesRotated = DEFAULT_MAX_FILE_ROTATED,
	.logFileFmt = DEFAULT_FORMAT_EXPRESSION
};

void saLogFilterSetCallbackT_01(void)
{
	int ret;
	SaAisErrorT rc;
	struct pollfd fds[1];
	char command[MAX_DATA];
	const unsigned int serverity_filter = 7;
	SaUint32T v_saLogStreamSeverityFilter = 127;

	logCallbacks.saLogFilterSetCallback = logFilterSetCallbackT;
	rc = logInitialize();
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		return;
	}
	rc = saLogSelectionObjectGet(logHandle, &selectionObject);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		goto done;
	}
	rc = logStreamOpen(&systemStreamName);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	cb_index = 0;
	/* Backup saLogStreamSeverityFilter value */
	get_attr_value(&systemStreamName, "saLogStreamSeverityFilter",
		       &v_saLogStreamSeverityFilter);

	sprintf(command, "immcfg %s -a saLogStreamSeverityFilter=%u 2> /dev/null",
	        SA_LOG_STREAM_SYSTEM, serverity_filter);
	ret = systemCall(command);
	if (ret != 0) {
		test_validate(ret, 0);
		goto done;
	}

	fds[0].fd = (int) selectionObject;
	fds[0].events = POLLIN;
	ret = poll(fds, 1, 1000);
	if (ret != 1) {
		fprintf(stderr, " poll log callback failed: %d \n", ret);
		test_validate(ret, 1);
		goto done;
	}

	rc = saLogDispatch(logHandle, SA_DISPATCH_ONE);
	if (rc != SA_AIS_OK) {
		fprintf(stderr, " saLogDispatch failed: %d \n", (int)rc);
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	if (log_streamHandle[0] == logStreamHandle && log_severity[0] == serverity_filter) {
		test_validate(SA_AIS_OK, SA_AIS_OK);
	} else {
		test_validate(0, SA_AIS_OK);
	}

done:
	logCallbacks.saLogFilterSetCallback = NULL;
	logFinalize();
	/* Restore saLogStreamSeverityFilter attribute */
	sprintf(command, "immcfg %s -a saLogStreamSeverityFilter=%d 2> /dev/null",
		SA_LOG_STREAM_SYSTEM, v_saLogStreamSeverityFilter);
	systemCall(command);
}

void saLogFilterSetCallbackT_02(void)
{
	int ret;
	SaAisErrorT rc;
	struct pollfd fds[1];
	char command[MAX_DATA];
	const unsigned int serverity_filter = 7;

	logCallbacks.saLogFilterSetCallback = logFilterSetCallbackT;
	rc = logInitialize();
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		return;
	}
	rc = saLogSelectionObjectGet(logHandle, &selectionObject);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	rc = logAppStreamOpen(&app1StreamName, &appStreamLogFileCreateAttributes);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	cb_index = 0;
	sprintf(command, "immadm -o 1 -p saLogStreamSeverityFilter:SA_UINT32_T:%u %s 2> /dev/null",
	        serverity_filter, SA_LOG_STREAM_APPLICATION1);
	ret = systemCall(command);
	if (ret != 0) {
		test_validate(ret, 0);
		goto done;
	}

	fds[0].fd = (int) selectionObject;
	fds[0].events = POLLIN;
	ret = poll(fds, 1, 1000);
	if (ret != 1) {
		fprintf(stderr, " poll log callback failed: %d \n", ret);
		test_validate(ret, 1);
		goto done;
	}

	rc = saLogDispatch(logHandle, SA_DISPATCH_ONE);
	if (rc != SA_AIS_OK) {
		fprintf(stderr, " saLogDispatch failed: %d \n", (int)rc);
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	if (log_streamHandle[0] == logStreamHandle && log_severity[0] == serverity_filter) {
		test_validate(SA_AIS_OK, SA_AIS_OK);
	} else {
		test_validate(0, SA_AIS_OK);
	}

done:
	logCallbacks.saLogFilterSetCallback = NULL;
	logFinalize();
}

void saLogFilterSetCallbackT_03(void)
{
	int ret;
	SaAisErrorT rc;
	struct pollfd fds[1];
	char command[MAX_DATA];
	const unsigned int serverity_filter[2] = {7, 15};
	SaUint32T v_saLogStreamSeverityFilter = 127;
	SaLogStreamHandleT logStreamHandle[2];

	logCallbacks.saLogFilterSetCallback = logFilterSetCallbackT;
	rc = saLogInitialize(&logHandle, &logCallbacks, &logVersion);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		return;
	}
	rc = saLogSelectionObjectGet(logHandle, &selectionObject);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	rc = saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
			       SA_TIME_ONE_SECOND, &logStreamHandle[0]);
	if (rc != SA_AIS_OK) {
		fprintf(stderr, " saLogStreamOpen_2 for system stream failed: %d \n", (int)rc);
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStreamLogFileCreateAttributes,
			       SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle[1]);
	if (rc != SA_AIS_OK) {
		fprintf(stderr, " saLogStreamOpen_2 app stream failed: %d \n", (int)rc);
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	cb_index = 0;
	/* Backup saLogStreamSeverityFilter value */
	get_attr_value(&systemStreamName, "saLogStreamSeverityFilter",
		       &v_saLogStreamSeverityFilter);
	/* Changing severity filter for system and app1 stream */
	sprintf(command, "immcfg %s -a saLogStreamSeverityFilter=%d 2> /dev/null",
	        SA_LOG_STREAM_SYSTEM, serverity_filter[0]);
	ret = systemCall(command);
	if (ret != 0) {
		test_validate(ret, 0);
		goto done;
	}
	sleep(1);
	sprintf(command, "immadm -o 1 -p saLogStreamSeverityFilter:SA_UINT32_T:%d %s 2> /dev/null",
	        serverity_filter[1], SA_LOG_STREAM_APPLICATION1);
	ret = systemCall(command);
	if (ret != 0) {
		test_validate(ret, 0);
		goto done;
	}

	fds[0].fd = (int) selectionObject;
	fds[0].events = POLLIN;
	ret = poll(fds, 1, 1000);
	if (ret != 1) {
		fprintf(stderr, " poll log callback failed: %d \n", ret);
		test_validate(ret, 1);
		goto done;
	}

	rc = saLogDispatch(logHandle, SA_DISPATCH_ALL);
	if (rc != SA_AIS_OK) {
		fprintf(stderr, " saLogDispatch failed: %d \n", (int)rc);
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	if (cb_index != 2) {
		printf("cb_index = %d\n", cb_index);
		test_validate(SA_AIS_ERR_LIBRARY, SA_AIS_OK);
		goto done;
	}

	for (int i = 0; i < 2; i++) {
		if ((log_streamHandle[i] != logStreamHandle[i]) || (log_severity[i] != serverity_filter[i])) {
	            printf("log streamHandle: %llu,  expected %llu \n", log_streamHandle[i], logStreamHandle[i]);
	            printf("log severity filter: %d,  expected %d \n", log_severity[i], serverity_filter[i]);
	            test_validate(0, SA_AIS_OK);
	            goto done;
	        }
	}

	test_validate(SA_AIS_OK, SA_AIS_OK);

done:
	logCallbacks.saLogFilterSetCallback = NULL;
	logFinalize();
	/* Restore saLogStreamSeverityFilter attribute */
	sprintf(command, "immcfg %s -a saLogStreamSeverityFilter=%d 2> /dev/null",
		SA_LOG_STREAM_SYSTEM, v_saLogStreamSeverityFilter);
	systemCall(command);
}


void saLogFilterSetCallbackT_04(void)
{
	int ret;
	SaAisErrorT rc;
	struct pollfd fds[MAX_CLIENTS];
	char command[MAX_DATA];
	const unsigned int serverity_filter[MAX_CLIENTS] = {7, 15};
	SaUint32T v_saLogStreamSeverityFilter = 127;
	SaLogStreamHandleT logStreamHandle[MAX_CLIENTS];
	SaLogHandleT logHandle[MAX_CLIENTS];
	SaSelectionObjectT selectionObject[MAX_CLIENTS];

	logCallbacks.saLogFilterSetCallback = logFilterSetCallbackT;
	for (int i = 0; i < MAX_CLIENTS; i++) {
		rc = saLogInitialize(&logHandle[i], &logCallbacks, &logVersion);
		if (rc != SA_AIS_OK) {
			test_validate(rc, SA_AIS_OK);
			return;
		}

		rc = saLogSelectionObjectGet(logHandle[i], &selectionObject[i]);
		if (rc != SA_AIS_OK) {
			test_validate(rc, SA_AIS_OK);
			goto done;
		}

		fds[i].fd = (int) selectionObject[i];
		fds[i].events = POLLIN;

		rc = saLogStreamOpen_2(logHandle[i], &systemStreamName, NULL, 0,
				       SA_TIME_ONE_SECOND, &logStreamHandle[i]);
		if (rc != SA_AIS_OK) {
			test_validate(rc, SA_AIS_OK);
			goto done;
		}
	}

	/* Close stream handle 1 */
	saLogStreamClose(logStreamHandle[1]);

	cb_index = 0;
	/* Backup saLogStreamSeverityFilter value */
	get_attr_value(&systemStreamName, "saLogStreamSeverityFilter",
		       &v_saLogStreamSeverityFilter);
	/* Changing severity filter for system and app1 stream */
	sprintf(command, "immcfg %s -a saLogStreamSeverityFilter=%d 2> /dev/null",
	        SA_LOG_STREAM_SYSTEM, serverity_filter[0]);
	ret = systemCall(command);
	if (ret != 0) {
		test_validate(ret, 0);
		goto done;
	}

	ret = poll(fds, 2, 1000);
	if (ret <= 0) {
		fprintf(stderr, " poll log callback failed: %d \n", ret);
		test_validate(ret, 1);
		goto done;
	}

	if (fds[1].revents & POLLIN) {
		fprintf(stderr, " ERROR, get callback while handle closed stream\n");
		test_validate(0, SA_AIS_OK);
	}

	if (fds[0].revents & POLLIN) {
		rc = saLogDispatch(logHandle[0], SA_DISPATCH_ALL);
		if (rc != SA_AIS_OK) {
			fprintf(stderr, " saLogDispatch failed: %d \n", (int)rc);
			test_validate(rc, SA_AIS_OK);
			goto done;
		}

		if (cb_index != 1) {
			fprintf(stderr, "cb_index = %d\n", cb_index);
			test_validate(SA_AIS_ERR_LIBRARY, SA_AIS_OK);
			goto done;
		}

		if (log_streamHandle[0] == logStreamHandle[0] && log_severity[0] == serverity_filter[0]) {
			test_validate(SA_AIS_OK, SA_AIS_OK);
		} else {
			test_validate(0, SA_AIS_OK);
		}
	} else {
		fprintf(stderr, " ERROR, Can not receive any callback\n");
		test_validate(0, SA_AIS_OK);
	}

done:
	logCallbacks.saLogFilterSetCallback = NULL;
	logFinalize();
	/* Restore saLogStreamSeverityFilter attribute */
	sprintf(command, "immcfg %s -a saLogStreamSeverityFilter=%d 2> /dev/null",
		SA_LOG_STREAM_SYSTEM, v_saLogStreamSeverityFilter);
	systemCall(command);
}

void saLogFilterSetCallbackT_05(void)
{
	int ret;
	SaAisErrorT rc;
	struct pollfd fds[1];
	char command[MAX_DATA];
	const unsigned int serverity_filter = 7;

	SaVersionT old_version = { 'A', 0x02, 0x02 };
	logCallbacks.saLogFilterSetCallback = logFilterSetCallbackT;
	rc = saLogInitialize(&logHandle, &logCallbacks, &old_version);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		return;
	}

	rc = saLogSelectionObjectGet(logHandle, &selectionObject);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	rc = logAppStreamOpen(&app1StreamName, &appStreamLogFileCreateAttributes);
	if (rc != SA_AIS_OK) {
		test_validate(rc, SA_AIS_OK);
		goto done;
	}

	cb_index = 0;
	sprintf(command, "immadm -o 1 -p saLogStreamSeverityFilter:SA_UINT32_T:%u %s 2> /dev/null",
	        serverity_filter, SA_LOG_STREAM_APPLICATION1);
	ret = systemCall(command);
	if (ret != 0) {
		rc_validate(ret, 0);
		goto done;
	}

	fds[0].fd = (int) selectionObject;
	fds[0].events = POLLIN;
	ret = poll(fds, 1, 1000);
	if (ret == 1) {
		fprintf(stderr, " poll log callback failed: %d \n", ret);
		rc_validate(ret, 0);
		goto done;
	} else {
		rc_validate(0, 0);;
	}

done:
	logCallbacks.saLogFilterSetCallback = NULL;
	logFinalize();
}

__attribute__ ((constructor)) static void saLibraryLifeCycle_constructor(void)
{
    test_suite_add(17, "Log Severity filter Callback");
    test_case_add(17, saLogFilterSetCallbackT_01, "saLogFilterSetCallbackT, severity filter is changed for cfg stream");
    test_case_add(17, saLogFilterSetCallbackT_02, "saLogFilterSetCallbackT, severity filter is changed for runtime stream");
    test_case_add(17, saLogFilterSetCallbackT_03, "saLogFilterSetCallbackT, severity filter is changed for runtime & cfg streams");
    test_case_add(17, saLogFilterSetCallbackT_04, "saLogFilterSetCallbackT, after closing stream");
    test_case_add(17, saLogFilterSetCallbackT_05, "saLogFilterSetCallbackT, if client initialize with version < A.02.03, ER");
}
