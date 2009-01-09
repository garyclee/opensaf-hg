#include "tet_log.h"

void saLogWriteLog_01(void)
{
    tet_printf("saLogWriteLog() '%s' OK", SA_LOG_STREAM_SYSTEM);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogWriteLog(logStreamHandle, SA_TIME_ONE_SECOND, &genLogRecord);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

