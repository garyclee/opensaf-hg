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
#include <unistd.h>

#include "logtest.h"

static SaInvocationT cb_invocation[8];
static SaAisErrorT cb_error[8];
static int cb_index;

static void logWriteLogCallbackT(SaInvocationT invocation, SaAisErrorT error)
{
    cb_invocation[cb_index] = invocation;
    cb_error[cb_index] = error;
    cb_index++;
}

void saLogWriteLogCallbackT_01(void)
{
    SaInvocationT invocation;
    struct pollfd fds[1];
    int ret;

    invocation = random();
    logCallbacks.saLogWriteLogCallback = logWriteLogCallbackT;
    safassert(saLogInitialize(&logHandle, &logCallbacks, &logVersion), SA_AIS_OK);
    safassert(saLogSelectionObjectGet(logHandle, &selectionObject), SA_AIS_OK);
    safassert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle), SA_AIS_OK);
    cb_index = 0;
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    safassert(saLogWriteLogAsync(logStreamHandle, invocation,
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord), SA_AIS_OK);

    fds[0].fd = (int) selectionObject;
    fds[0].events = POLLIN;
    ret = poll(fds, 1, 1000);
    assert(ret == 1);
    safassert(saLogDispatch(logHandle, SA_DISPATCH_ONE), SA_AIS_OK);
    safassert(saLogFinalize(logHandle), SA_AIS_OK);

    if (cb_invocation[0] == invocation)
        test_validate(cb_error[0], SA_AIS_OK);
    else
        test_validate(SA_AIS_ERR_LIBRARY, SA_AIS_OK);
}

void saLogWriteLogCallbackT_02(void)
{
    SaInvocationT invocation[3];
    struct pollfd fds[1];
    int ret, i;

    invocation[0] = random();
    invocation[1] = random();
    invocation[2] = random();
    logCallbacks.saLogWriteLogCallback = logWriteLogCallbackT;
    safassert(saLogInitialize(&logHandle, &logCallbacks, &logVersion), SA_AIS_OK);
    safassert(saLogSelectionObjectGet(logHandle, &selectionObject), SA_AIS_OK);
    safassert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle), SA_AIS_OK);
    cb_index = 0;
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    safassert(saLogWriteLogAsync(logStreamHandle, invocation[0],
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord), SA_AIS_OK);
    safassert(saLogWriteLogAsync(logStreamHandle, invocation[1],
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord), SA_AIS_OK);
    safassert(saLogWriteLogAsync(logStreamHandle, invocation[2],
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord), SA_AIS_OK);

    sleep(1); /* Let the writes make it to disk and back */
    fds[0].fd = (int) selectionObject;
    fds[0].events = POLLIN;
    ret = poll(fds, 1, 1000);
    assert(ret == 1);
    safassert(saLogDispatch(logHandle, SA_DISPATCH_ALL), SA_AIS_OK);
    safassert(saLogFinalize(logHandle), SA_AIS_OK);

    if (cb_index != 3)
    {
        printf("cb_index = %u\n", cb_index);
        test_validate(SA_AIS_ERR_LIBRARY, SA_AIS_OK);
        return;
    }

    for (i = 0; i < 3; i++)
    {
        if ((cb_invocation[i] != invocation[i]) || (cb_error[i] != SA_AIS_OK))
        {
            printf("%llu expected %llu, %u\n", cb_invocation[i], invocation[i], cb_error[i]);
            test_validate(cb_error[i], SA_AIS_OK);
            return;
        }
    }

    test_validate(SA_AIS_OK, SA_AIS_OK);
}

