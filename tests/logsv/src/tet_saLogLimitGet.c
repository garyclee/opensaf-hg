#include "tet_log.h"

void saLogLimitGet_01(void)
{
    SaLimitValueT limitValue;

    tet_printf("saLogLimitGet() OK");
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogLimitGet(logHandle, SA_LOG_MAX_NUM_CLUSTER_APP_LOG_STREAMS_ID, &limitValue);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

