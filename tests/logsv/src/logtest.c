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

#include <sys/time.h>
#include <unistd.h>

#include "logtest.h"

SaNameT systemStreamName =
{
    .value = SA_LOG_STREAM_SYSTEM,
    .length = sizeof(SA_LOG_STREAM_SYSTEM)
};

SaNameT alarmStreamName =
{
    .value = SA_LOG_STREAM_ALARM,
    .length = sizeof(SA_LOG_STREAM_ALARM)
};

SaNameT notificationStreamName =
{
    .value = SA_LOG_STREAM_NOTIFICATION,
    .length = sizeof(SA_LOG_STREAM_NOTIFICATION)
};

SaNameT app1StreamName =
{
    .value = SA_LOG_STREAM_APPLICATION1,
    .length = sizeof(SA_LOG_STREAM_APPLICATION1)
};

SaNameT notifyingObject =
{
    .value = DEFAULT_NOTIFYING_OBJECT,
    .length = sizeof(DEFAULT_NOTIFYING_OBJECT)
};

SaNameT notificationObject =
{
    .value = DEFAULT_NOTIFICATION_OBJECT,
    .length = sizeof(DEFAULT_NOTIFICATION_OBJECT)
};

static char buf[2048];

SaLogBufferT alarmStreamBuffer =
{
    .logBuf = (SaUint8T *) buf,
    .logBufSize = 0,
};

SaLogBufferT notificationStreamBuffer =
{
    .logBuf = (SaUint8T *) buf,
    .logBufSize = 0,
};

static SaLogBufferT genLogBuffer =
{
    .logBuf = (SaUint8T *) buf,
    .logBufSize = 0,
};

SaNameT logSvcUsrName = 
{
    .value = SA_LOG_STREAM_APPLICATION1,
    .length = sizeof(SA_LOG_STREAM_APPLICATION1)
};

SaLogRecordT genLogRecord =
{
    .logTimeStamp = SA_TIME_UNKNOWN,
    .logHdrType = SA_LOG_GENERIC_HEADER,
    .logHeader.genericHdr.notificationClassId = NULL,
    .logHeader.genericHdr.logSeverity = SA_LOG_SEV_FLAG_INFO,
    .logHeader.genericHdr.logSvcUsrName = &logSvcUsrName,
    .logBuffer = &genLogBuffer
};

SaVersionT logVersion = {'A', 0x02, 0x01}; 
SaAisErrorT rc;
SaLogHandleT logHandle;
SaLogStreamHandleT logStreamHandle;
SaLogCallbacksT logCallbacks = {NULL, NULL, NULL};
SaSelectionObjectT selectionObject;

#if 0
static struct tet_testlist api_testlist[] =
{
    /* Log Service Operations */
    {saLogStreamOpen_2_01, 5},
    {saLogStreamOpen_2_02, 5},
    {saLogStreamOpen_2_03, 5},
    {saLogStreamOpen_2_04, 5},
    {saLogStreamOpen_2_05, 5},
    {saLogStreamOpen_2_06, 5},
    {saLogStreamOpen_2_08, 5},
    {saLogStreamOpen_2_09, 5},
    {saLogStreamOpen_2_10, 5},
    {saLogStreamOpen_2_11, 5},
    {saLogStreamOpen_2_12, 5},
    {saLogStreamOpen_2_13, 5},
    {saLogStreamOpen_2_14, 5},
    {saLogStreamOpen_2_15, 5},
    {saLogStreamOpen_2_16, 5},
    {saLogStreamOpen_2_17, 5},
    {saLogStreamOpen_2_18, 5},
    {saLogStreamOpen_2_19, 5},
    {saLogStreamOpenAsync_2_01, 6},
    {saLogStreamOpenCallbackT_01, 7},
    {saLogWriteLog_01, 8},
    {saLogWriteLogAsync_01, 9},
    {saLogWriteLogAsync_02, 9},
    {saLogWriteLogAsync_03, 9},
    {saLogWriteLogAsync_04, 9},
    {saLogWriteLogAsync_05, 9},
    {saLogWriteLogAsync_06, 9},
    {saLogWriteLogAsync_07, 9},
    {saLogWriteLogAsync_09, 9},
    {saLogWriteLogAsync_10, 9},
    {saLogWriteLogAsync_11, 9},
    {saLogWriteLogAsync_12, 9},
    {saLogWriteLogAsync_13, 9},
    {saLogWriteLogCallbackT_01, 10},
    {saLogWriteLogCallbackT_02, 10},
    {saLogFilterSetCallbackT_01, 11},
    {saLogStreamClose_01, 12},

    /* Limit Fetch API */
    {saLogLimitGet_01, 13},
    {NULL, 0}
};
#endif
int main(int argc, char **argv) 
{
    int suite = ALL_SUITES, tcase = ALL_TESTS;

    srandom(getpid());

    if (argc > 1)
    {
        suite = atoi(argv[1]);
    }

    if (argc > 2)
    {
        tcase = atoi(argv[2]);
    }

    if (suite == 0)
    {
        test_list();
        return 0;
    }

    return test_run(suite, tcase);
}  


