#include "tet_log.h"

void saLogDispatch_01(void)
{
    tet_printf("saLogDispatch() OK");
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogDispatch(logHandle, SA_DISPATCH_ALL);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

