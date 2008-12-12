#include "tet_log.h"

void saLogStreamOpenAsync_2_01(void)
{
    SaInvocationT invocation;

    tet_printf("saLogStreamOpenAsync_2() '%s' OK", SA_LOG_STREAM_SYSTEM);
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogStreamOpenAsync_2(logHandle, &systemStreamName, NULL, 0, invocation);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

