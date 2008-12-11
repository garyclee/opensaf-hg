#include "tet_log.h"

void saLogFinalize_01(void)
{
    tet_printf("saLogFinalize() OK");
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogFinalize(logHandle);
    result(rc, SA_AIS_OK);
}

void saLogFinalize_02(void)
{
    tet_printf("saLogFinalize() with NULL log handle");
    rc = saLogFinalize(0);
    result(rc, SA_AIS_ERR_BAD_HANDLE);
}

