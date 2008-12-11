#include "tet_log.h"

void saLogStreamOpenCallbackT_01(void)
{
    tet_printf("saLogStreamOpenCallbackT() OK");
    result(SA_AIS_ERR_NOT_SUPPORTED, SA_AIS_OK);
}

