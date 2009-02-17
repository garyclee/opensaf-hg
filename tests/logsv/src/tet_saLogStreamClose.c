#include "tet_log.h"

void saLogStreamClose_01(void)
{
    tet_printf("saLogStreamClose OK");
    assert(saLogInitialize(&logHandle, NULL, &logVersion) == SA_AIS_OK);
    assert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, &logStreamHandle) == SA_AIS_OK);
    rc = saLogStreamClose(logStreamHandle);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

