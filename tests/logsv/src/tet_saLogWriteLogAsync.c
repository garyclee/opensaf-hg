#include "tet_log.h"

void saLogWriteLogAsync_01(void)
{
    SaInvocationT invocation;

    tet_printf("saLogWriteAsyncLog() '%s' OK", SA_LOG_STREAM_SYSTEM);
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_OK);
}

void saLogWriteLogAsync_02(void)
{
    SaInvocationT invocation;
    SaLogRecordT ntfLogRecord;
    SaNtfIdentifierT notificationId = random();
    SaNtfClassIdT notificationClassId;

    tet_printf("saLogWriteAsyncLog() '%s' OK", SA_LOG_STREAM_ALARM);
    ntfLogRecord.logBuffer = &alarmStreamBuffer;
    strcpy((char*)ntfLogRecord.logBuffer->logBuf, __FUNCTION__);
    ntfLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);

    notificationClassId.vendorId = 193;
    notificationClassId.majorId  = 1;
    notificationClassId.minorId  = 2;
    ntfLogRecord.logTimeStamp = getSaTimeT();
    ntfLogRecord.logHdrType = SA_LOG_NTF_HEADER;
    ntfLogRecord.logHeader.ntfHdr.notificationId = notificationId;
    ntfLogRecord.logHeader.ntfHdr.eventType = SA_NTF_ALARM_QOS;
    ntfLogRecord.logHeader.ntfHdr.notificationObject = &notificationObject;
    ntfLogRecord.logHeader.ntfHdr.notifyingObject = &notifyingObject;
    ntfLogRecord.logHeader.ntfHdr.notificationClassId = &notificationClassId;
    ntfLogRecord.logHeader.ntfHdr.eventTime = getSaTimeT();

    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &alarmStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &ntfLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_OK);
}

void saLogWriteLogAsync_03(void)
{
    SaInvocationT invocation;
    SaLogRecordT ntfLogRecord;
    SaNtfIdentifierT notificationId = random();
    SaNtfClassIdT notificationClassId;

    tet_printf("saLogWriteAsyncLog() '%s' OK", SA_LOG_STREAM_NOTIFICATION);
    ntfLogRecord.logBuffer = &notificationStreamBuffer;
    strcpy((char*)ntfLogRecord.logBuffer->logBuf, __FUNCTION__);
    ntfLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);

    notificationClassId.vendorId = 193;
    notificationClassId.majorId  = 1;
    notificationClassId.minorId  = 2;
    ntfLogRecord.logTimeStamp = getSaTimeT();
    ntfLogRecord.logHdrType = SA_LOG_NTF_HEADER;
    ntfLogRecord.logHeader.ntfHdr.notificationId = notificationId;
    ntfLogRecord.logHeader.ntfHdr.eventType = SA_NTF_ALARM_QOS;
    ntfLogRecord.logHeader.ntfHdr.notificationObject = &notificationObject;
    ntfLogRecord.logHeader.ntfHdr.notifyingObject = &notifyingObject;
    ntfLogRecord.logHeader.ntfHdr.notificationClassId = &notificationClassId;
    ntfLogRecord.logHeader.ntfHdr.eventTime = getSaTimeT();

    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &notificationStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &ntfLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_OK);
}

void saLogWriteLogAsync_04(void)
{
    SaInvocationT invocation;

    tet_printf("saLogWriteAsyncLog() with NULL logStreamHandle");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(0, invocation, 0, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_ERR_BAD_HANDLE);
}

void saLogWriteLogAsync_05(void)
{
    SaInvocationT invocation;

    tet_printf("saLogWriteAsyncLog() with invalid logStreamHandle");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(-1, invocation, 0, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_ERR_BAD_HANDLE);
}

void saLogWriteLogAsync_06(void)
{
    SaInvocationT invocation;

    tet_printf("saLogWriteAsyncLog() with invalid ackFlags");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, -1, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogWriteLogAsync_07(void)
{
    SaInvocationT invocation;

    tet_printf("saLogWriteAsyncLog() with NULL logRecord ptr");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, NULL);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogWriteLogAsync_08(void)
{
    tet_printf("saLogWriteAsyncLog() with invalid logRecord ptr");
#if 0
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, 1, 0, (const SaLogRecordT *)-1);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
#endif
    tet_result(TET_FAIL);
}

