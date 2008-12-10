#include <sys/time.h>
#include <unistd.h>
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

void saLogWriteLogAsync_09(void)
{
    SaInvocationT invocation;
    tet_printf("saLogWriteAsyncLog() logSvcUsrName == NULL ");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    genLogRecord.logHeader.genericHdr.logSvcUsrName = NULL;
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogWriteLogAsync_10(void)
{
    SaInvocationT invocation;
    assert(setenv("SA_AMF_COMPONENT_NAME", "safComp=CompT_test_svc", 1) == 0);
    tet_printf("saLogWriteAsyncLog() logSvcUsrName == NULL and envset");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    genLogRecord.logHeader.genericHdr.logSvcUsrName = NULL;
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_OK);
}
void saLogWriteLogAsync_11(void)
{
    struct timeval currentTime;
    SaInvocationT invocation;
    tet_printf("saLogWriteAsyncLog() with logTimeStamp set");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    /* Fetch current system time for time stamp value */
    (void)gettimeofday(&currentTime, 0);
    genLogRecord.logTimeStamp = 
       ((unsigned)currentTime.tv_sec * 1000000000ULL) + \
       ((unsigned)currentTime.tv_usec * 1000ULL);

    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_OK);
}

void saLogWriteLogAsync_12(void)
{
    SaInvocationT invocation;

    tet_printf("saLogWriteAsyncLog() without logTimeStamp set");
    strcpy((char*)genLogRecord.logBuffer->logBuf, __FUNCTION__);
    genLogRecord.logBuffer->logBufSize = strlen(__FUNCTION__);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                             SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_OK);
}

void saLogWriteLogAsync_13(void)
{
    SaLogFileCreateAttributesT_2 appStream1LogFileCreateAttributes = {
        .logFilePathName = DEFAULT_APP_FILE_PATH_NAME,
        .logFileName = (SaStringT) "ticket203",
        .maxLogFileSize = DEFAULT_APP_LOG_FILE_SIZE,
        .maxLogRecordSize = 2048,
        .haProperty = SA_TRUE,
        .logFileFullAction = SA_LOG_FILE_FULL_ACTION_ROTATE,
        .maxFilesRotated = DEFAULT_MAX_FILE_ROTATED,
        .logFileFmt = DEFAULT_FORMAT_EXPRESSION
    };
    SaInvocationT invocation;
    SaNameT appStreamName = {
        .value = "safLgStr=ticket203",
        .length = sizeof(appStreamName.value)
    };

    tet_printf("saLogWriteAsyncLog() 1800 bytes logrecord (ticket #203)");
    memset(genLogRecord.logBuffer->logBuf, 'X', 1800);
    genLogRecord.logBuffer->logBufSize = 1800;
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &appStreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLogAsync(logStreamHandle, invocation, 0, &genLogRecord);
    sleep(1);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);

    result(rc, SA_AIS_OK);
}

