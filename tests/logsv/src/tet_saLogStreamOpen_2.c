#include "tet_log.h"

static SaLogFileCreateAttributesT_2 appStream1LogFileCreateAttributes =
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

static void init_file_create_attributes(void)
{
    appStream1LogFileCreateAttributes.logFilePathName = DEFAULT_APP_FILE_PATH_NAME;
    appStream1LogFileCreateAttributes.logFileName = DEFAULT_APP_FILE_NAME;
    appStream1LogFileCreateAttributes.maxLogFileSize = DEFAULT_APP_LOG_FILE_SIZE;
    appStream1LogFileCreateAttributes.maxLogRecordSize = DEFAULT_APP_LOG_REC_SIZE;
    appStream1LogFileCreateAttributes.haProperty = SA_TRUE;
    appStream1LogFileCreateAttributes.logFileFullAction = SA_LOG_FILE_FULL_ACTION_ROTATE;
    appStream1LogFileCreateAttributes.maxFilesRotated = DEFAULT_MAX_FILE_ROTATED;
    appStream1LogFileCreateAttributes.logFileFmt = DEFAULT_FORMAT_EXPRESSION;
}

void saLogStreamOpen_2_01(void)
{
    tet_printf("saLogStreamOpen_2() '%s' OK", SA_LOG_STREAM_SYSTEM);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

void saLogStreamOpen_2_02(void)
{
    tet_printf("saLogStreamOpen_2() '%s' OK", SA_LOG_STREAM_NOTIFICATION);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpen_2(logHandle, &notificationStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

void saLogStreamOpen_2_03(void)
{
    tet_printf("saLogStreamOpen_2() '%s' OK", SA_LOG_STREAM_ALARM);

    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpen_2(logHandle, &alarmStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

void saLogStreamOpen_2_04(void)
{
    tet_printf("saLogStreamOpen_2() '%s' OK", SA_LOG_STREAM_APPLICATION1);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                           SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

void saLogStreamOpen_2_05(void)
{
    SaLogStreamHandleT logStreamHandle1, logStreamHandle2;

    tet_printf("saLogStreamOpen_2() '%s' twice - OK", SA_LOG_STREAM_APPLICATION1);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);

    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                           SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle1);

    if (rc != SA_AIS_OK)
    {
        tet_result(TET_FAIL);
        return;
    }
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                           SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle2);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

void saLogStreamOpen_2_06(void)
{
    tet_printf("saLogStreamOpen_2() with NULL ptr to handle");
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, NULL);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_07(void)
{
    tet_printf("saLogStreamOpen_2() with invalid ptr to handle");
#if 0
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, (SaLogStreamHandleT*) -1);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_BAD_HANDLE);
#endif
    tet_result(TET_FAIL);
}

void saLogStreamOpen_2_08(void)
{
    tet_printf("saLogStreamOpen_2() with NULL name");
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpen_2(logHandle, NULL, NULL, 0,
                           SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_09(void)
{
    tet_printf("saLogStreamOpen_2() open app stream second time with altered logFileName");
    init_file_create_attributes();
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    appStream1LogFileCreateAttributes.logFileName = "changed_filename";
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_10(void)
{
    tet_printf("saLogStreamOpen_2() open app stream second time with altered logFilePathName");
    init_file_create_attributes();
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    appStream1LogFileCreateAttributes.logFilePathName = "/new_file/path";
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_11(void)
{
    tet_printf("saLogStreamOpen_2() open app stream second time with altered logFileFmt");
    init_file_create_attributes();
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    appStream1LogFileCreateAttributes.logFileFmt = "@Cr @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sl @Sv\"@Cb\"";
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_12(void)
{
    tet_printf("saLogStreamOpen_2() open app stream second time with altered maxLogFileSize");
    init_file_create_attributes();
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    appStream1LogFileCreateAttributes.maxLogFileSize++;
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_13(void)
{
    tet_printf("saLogStreamOpen_2() open app stream second time with altered maxLogRecordSize");
    init_file_create_attributes();
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    appStream1LogFileCreateAttributes.maxLogRecordSize++;
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_14(void)
{
    tet_printf("saLogStreamOpen_2() open app stream second time with altered maxFilesRotated");
    init_file_create_attributes();
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    appStream1LogFileCreateAttributes.maxFilesRotated++;
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);

    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogStreamOpen_2_15(void)
{
    tet_printf("saLogStreamOpen_2() open app stream second time with altered haProperty");
    init_file_create_attributes();
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    appStream1LogFileCreateAttributes.haProperty = SA_FALSE;
    rc = saLogStreamOpen_2(logHandle, &app1StreamName, &appStream1LogFileCreateAttributes,
                             SA_LOG_STREAM_CREATE, SA_TIME_ONE_SECOND, &logStreamHandle);

    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

