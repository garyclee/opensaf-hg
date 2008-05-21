#include "tet_log.h"

void saLogInitialize_01(void)
{
    tet_printf("saLogInitialize() OK");
    rc = saLogInitialize(&logHandle, &logCallbacks, &logVersion);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

void saLogInitialize_02(void)
{
    tet_printf("saLogInitialize() with NULL pointer to handle");
    rc = saLogInitialize(NULL, &logCallbacks, &logVersion);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogInitialize_03(void)
{
    tet_printf("saLogInitialize() with NULL pointer to callbacks");
    rc = saLogInitialize(&logHandle, NULL, &logVersion);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
    result(rc, SA_AIS_OK);
}

void saLogInitialize_04(void)
{
    tet_printf("saLogInitialize() with NULL callbacks AND version");
    rc = saLogInitialize(&logHandle, NULL, NULL);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogInitialize_05(void)
{
    tet_printf("saLogInitialize() with uninitialized handle");
    rc = saLogInitialize(0, &logCallbacks, &logVersion);
    result(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saLogInitialize_06(void)
{
    SaVersionT version = {0, 0, 0};

    tet_printf("saLogInitialize() with uninitialized version");
    rc = saLogInitialize(&logHandle, &logCallbacks, &version);
    result(rc, SA_AIS_ERR_VERSION);
}

void saLogInitialize_07(void)
{
    SaVersionT version = {'B', 1, 1};

    tet_printf("saLogInitialize() with too high release level");
    rc = saLogInitialize(&logHandle, &logCallbacks, &version);
    result(rc, SA_AIS_ERR_VERSION);
}

void saLogInitialize_08(void)
{
    SaVersionT version = {'A', 2, 1};

    tet_printf("saLogInitialize() with minor version set to 1");
    rc = saLogInitialize(&logHandle, &logCallbacks, &version);
    result(rc, SA_AIS_OK);
    assert(saLogFinalize(logHandle) == SA_AIS_OK);
}

void saLogInitialize_09(void)
{
    SaVersionT version = {'A', 3, 0};

    tet_printf("saLogInitialize() with major version set to 3");
    rc = saLogInitialize(&logHandle, &logCallbacks, &version);
    result(rc, SA_AIS_ERR_VERSION);
}

