#include "tet_log.h"
#include <poll.h>
#include <unistd.h>

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

    tet_printf("saLogWriteLogCallbackT() SA_DISPATCH_ONE");

    invocation = random();
    logCallbacks.saLogWriteLogCallback = logWriteLogCallbackT;
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogSelectionObjectGet(logHandle, &selectionObject) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    cb_index = 0;
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogWriteLogAsync(logStreamHandle, invocation,
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord) == SA_AIS_OK);

    fds[0].fd = (int) selectionObject;
    fds[0].events = POLLIN;
    ret = poll(fds, 1, 1000);
    assert(ret == 1);
    assert(saLogDispatch(logHandle, SA_DISPATCH_ONE) == SA_AIS_OK);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    if ((cb_invocation[0] == invocation) && (cb_error[0] == SA_AIS_OK))
        tet_result(TET_PASS);
    else
        tet_result(TET_FAIL);
}

void saLogWriteLogCallbackT_02(void)
{
    SaInvocationT invocation[3];
    struct pollfd fds[1];
    int ret, i;

    tet_printf("saLogWriteLogCallbackT() SA_DISPATCH_ALL");

    invocation[0] = random();
    invocation[1] = random();
    invocation[2] = random();
    logCallbacks.saLogWriteLogCallback = logWriteLogCallbackT;
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogSelectionObjectGet(logHandle, &selectionObject) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    cb_index = 0;
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogWriteLogAsync(logStreamHandle, invocation[0],
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord) == SA_AIS_OK);
    assert(saLogWriteLogAsync(logStreamHandle, invocation[1],
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord) == SA_AIS_OK);
    assert(saLogWriteLogAsync(logStreamHandle, invocation[2],
                              SA_LOG_RECORD_WRITE_ACK, &genLogRecord) == SA_AIS_OK);

    sleep(1); /* Let the writes make it to disk and back */
    fds[0].fd = (int) selectionObject;
    fds[0].events = POLLIN;
    ret = poll(fds, 1, 1000);
    assert(ret == 1);
    assert(saLogDispatch(logHandle, SA_DISPATCH_ALL) == SA_AIS_OK);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    if (cb_index != 3)
    {
        printf("cb_index = %u\n", cb_index);
        tet_result(TET_FAIL);
        return;
    }

    for (i = 0; i < 3; i++)
    {
        if ((cb_invocation[i] != invocation[i]) || (cb_error[i] != SA_AIS_OK))
        {
            printf("%llu expected %llu, %u\n", cb_invocation[i], invocation[i], cb_error[i]);
            result(cb_error[i], SA_AIS_OK);
            return;
        }
    }

    tet_result(TET_PASS);
}

