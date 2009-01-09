#include "tet_log.h"

void saLogSelectionObjectGet_01(void)
{
    tet_printf("saLogSelectionObjectGet() OK");
    assert(saLogInitialize(&logHandle, &logCallbacks, &logVersion) == SA_AIS_OK);
    rc = saLogSelectionObjectGet(logHandle, &selectionObject);
    result(rc, SA_AIS_OK);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
}

void saLogSelectionObjectGet_02(void)
{
    tet_printf("saLogSelectionObjectGet() with NULL log handle");
    rc = saLogSelectionObjectGet(0, &selectionObject);
    result(rc, SA_AIS_ERR_BAD_HANDLE);
}

