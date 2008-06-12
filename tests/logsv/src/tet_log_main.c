#include <sys/time.h>
#include <unistd.h>
#include "tet_log.h"

SaNameT systemStreamName =
{
    .value = SA_LOG_STREAM_SYSTEM,
    .length = sizeof(SA_LOG_STREAM_SYSTEM)
};

SaNameT alarmStreamName =
{
    .value = SA_LOG_STREAM_ALARM,
    .length = sizeof(SA_LOG_STREAM_ALARM)
};

SaNameT notificationStreamName =
{
    .value = SA_LOG_STREAM_NOTIFICATION,
    .length = sizeof(SA_LOG_STREAM_NOTIFICATION)
};

SaNameT app1StreamName =
{
    .value = SA_LOG_STREAM_APPLICATION1,
    .length = sizeof(SA_LOG_STREAM_APPLICATION1)
};

SaNameT notifyingObject =
{
    .value = DEFAULT_NOTIFYING_OBJECT,
    .length = sizeof(DEFAULT_NOTIFYING_OBJECT)
};

SaNameT notificationObject =
{
    .value = DEFAULT_NOTIFICATION_OBJECT,
    .length = sizeof(DEFAULT_NOTIFICATION_OBJECT)
};

static char buf[256];

SaLogBufferT alarmStreamBuffer =
{
    .logBuf = (SaUint8T *) buf,
    .logBufSize = 0,
};

SaLogBufferT notificationStreamBuffer =
{
    .logBuf = (SaUint8T *) buf,
    .logBufSize = 0,
};

static SaLogBufferT genLogBuffer =
{
    .logBuf = (SaUint8T *) buf,
    .logBufSize = 0,
};

SaNameT logSvcUsrName = 
{
    .value = SA_LOG_STREAM_APPLICATION1,
    .length = sizeof(SA_LOG_STREAM_APPLICATION1)
};

SaLogRecordT genLogRecord =
{
    .logTimeStamp = SA_TIME_UNKNOWN,
    .logHdrType = SA_LOG_GENERIC_HEADER,
    .logHeader.genericHdr.notificationClassId = NULL,
    .logHeader.genericHdr.logSeverity = SA_LOG_SEV_FLAG_INFO,
    .logHeader.genericHdr.logSvcUsrName = &logSvcUsrName,
    .logBuffer = &genLogBuffer
};

SaVersionT logVersion = {'A', 0x02, 0x01}; 
SaAisErrorT rc;
SaLogHandleT logHandle;
SaLogStreamHandleT logStreamHandle;
SaLogCallbacksT logCallbacks = {NULL, NULL, NULL};
SaSelectionObjectT selectionObject;

struct tet_testlist
{
    void (*testfunc)();
    int icref;
    int tpnum;
};

static const char *saf_error[] =
{
    "SA_AIS_NOT_VALID",
    "SA_AIS_OK",
    "SA_AIS_ERR_LIBRARY",
    "SA_AIS_ERR_VERSION",
    "SA_AIS_ERR_INIT",
    "SA_AIS_ERR_TIMEOUT",
    "SA_AIS_ERR_TRY_AGAIN",
    "SA_AIS_ERR_INVALID_PARAM",
    "SA_AIS_ERR_NO_MEMORY",
    "SA_AIS_ERR_BAD_HANDLE",
    "SA_AIS_ERR_BUSY",
    "SA_AIS_ERR_ACCESS",
    "SA_AIS_ERR_NOT_EXIST",
    "SA_AIS_ERR_NAME_TOO_LONG",
    "SA_AIS_ERR_EXIST",
    "SA_AIS_ERR_NO_SPACE",
    "SA_AIS_ERR_INTERRUPT",
    "SA_AIS_ERR_NAME_NOT_FOUND",
    "SA_AIS_ERR_NO_RESOURCES",
    "SA_AIS_ERR_NOT_SUPPORTED",
    "SA_AIS_ERR_BAD_OPERATION",
    "SA_AIS_ERR_FAILED_OPERATION",
    "SA_AIS_ERR_MESSAGE_ERROR",
    "SA_AIS_ERR_QUEUE_FULL",
    "SA_AIS_ERR_QUEUE_NOT_AVAILABLE",
    "SA_AIS_ERR_BAD_FLAGS",
    "SA_AIS_ERR_TOO_BIG",
    "SA_AIS_ERR_NO_SECTIONS",
};

/* test statistics */
static int test_total;
static int test_passed;
static int test_failed;

void tet_result(int result)
{
    if (result == TET_PASS)
    {
        test_passed++;
        printf(" - PASSED\n");
    }
    else if (result == TET_FAIL)
    {
        test_failed++;
        printf(" - FAILED\n");
    }
    else
        printf(" - UNKNOWN\n");

    test_total++;
}

void result(SaAisErrorT rc, SaAisErrorT expected)
{
    if (rc == expected)
    {
        tet_result(TET_PASS);
    }
    else
    {
        tet_result(TET_FAIL);
        printf("\t%s expected %s\n", saf_error[rc], saf_error[expected]);
    }
}

SaTimeT getSaTimeT(void) 
{ 
   struct timeval tp; 
 
   gettimeofday(&tp, NULL);
 
   return (tp.tv_sec * SA_TIME_ONE_SECOND) +
       (tp.tv_usec * SA_TIME_ONE_MICROSECOND);
} 

extern void saLogInitialize_01(void);
extern void saLogInitialize_02(void);
extern void saLogInitialize_03(void);
extern void saLogInitialize_04(void);
extern void saLogInitialize_05(void);
extern void saLogInitialize_06(void);
extern void saLogInitialize_07(void);
extern void saLogInitialize_08(void);
extern void saLogInitialize_09(void);
extern void saLogSelectionObjectGet_01(void);
extern void saLogFinalize_01(void);
extern void saLogFinalize_02(void);
extern void saLogDispatch_01(void);
extern void saLogLimitGet_01(void);
extern void saLogStreamOpen_2_01(void);
extern void saLogStreamOpen_2_02(void);
extern void saLogStreamOpen_2_03(void);
extern void saLogStreamOpen_2_04(void);
extern void saLogStreamOpen_2_05(void);
extern void saLogStreamOpen_2_06(void);
extern void saLogStreamOpen_2_08(void);
extern void saLogStreamOpen_2_09(void);
extern void saLogStreamOpen_2_10(void);
extern void saLogStreamOpen_2_11(void);
extern void saLogStreamOpen_2_12(void);
extern void saLogStreamOpen_2_13(void);
extern void saLogStreamOpen_2_14(void);
extern void saLogStreamOpen_2_15(void);
extern void saLogStreamOpen_2_16(void);
extern void saLogStreamOpen_2_17(void);
extern void saLogStreamOpen_2_18(void);
extern void saLogStreamOpen_2_19(void);

extern void saLogStreamOpenAsync_2_01(void);
extern void saLogStreamOpenCallbackT_01(void);
extern void saLogWriteLog_01(void);
extern void saLogWriteLogAsync_01(void);
extern void saLogWriteLogAsync_02(void);
extern void saLogWriteLogAsync_03(void);
extern void saLogWriteLogAsync_04(void);
extern void saLogWriteLogAsync_05(void);
extern void saLogWriteLogAsync_06(void);
extern void saLogWriteLogAsync_07(void);
extern void saLogWriteLogAsync_09(void);
extern void saLogWriteLogAsync_10(void);
extern void saLogWriteLogAsync_11(void);
extern void saLogWriteLogAsync_12(void);
extern void saLogWriteLogCallbackT_01(void);
extern void saLogWriteLogCallbackT_02(void);
extern void saLogStreamClose_01(void);
extern void saLogFilterSetCallbackT_01(void);

static struct tet_testlist api_testlist[] =
{
    /* Library Life cycle */
    {saLogInitialize_01, 1},
    {saLogInitialize_02, 1},
    {saLogInitialize_03, 1},
    {saLogInitialize_04, 1},
    {saLogInitialize_05, 1},
    {saLogInitialize_06, 1},
    {saLogInitialize_07, 1},
    {saLogInitialize_08, 1},
    {saLogInitialize_09, 1},
    {saLogSelectionObjectGet_01, 2},
    {saLogDispatch_01, 3},
    {saLogFinalize_01, 4},
    {saLogFinalize_02, 4},

    /* Log Service Operations */
    {saLogStreamOpen_2_01, 5},
    {saLogStreamOpen_2_02, 5},
    {saLogStreamOpen_2_03, 5},
    {saLogStreamOpen_2_04, 5},
    {saLogStreamOpen_2_05, 5},
    {saLogStreamOpen_2_06, 5},
    {saLogStreamOpen_2_08, 5},
    {saLogStreamOpen_2_09, 5},
    {saLogStreamOpen_2_10, 5},
    {saLogStreamOpen_2_11, 5},
    {saLogStreamOpen_2_12, 5},
    {saLogStreamOpen_2_13, 5},
    {saLogStreamOpen_2_14, 5},
    {saLogStreamOpen_2_15, 5},
    {saLogStreamOpen_2_16, 5},
    {saLogStreamOpen_2_17, 5},
    {saLogStreamOpen_2_18, 5},
    {saLogStreamOpen_2_19, 5},
    {saLogStreamOpenAsync_2_01, 6},
    {saLogStreamOpenCallbackT_01, 7},
    {saLogWriteLog_01, 8},
    {saLogWriteLogAsync_01, 9},
    {saLogWriteLogAsync_02, 9},
    {saLogWriteLogAsync_03, 9},
    {saLogWriteLogAsync_04, 9},
    {saLogWriteLogAsync_05, 9},
    {saLogWriteLogAsync_06, 9},
    {saLogWriteLogAsync_07, 9},
    {saLogWriteLogAsync_09, 9},
    {saLogWriteLogAsync_10, 9},
    {saLogWriteLogAsync_11, 9},
    {saLogWriteLogAsync_12, 9},
    {saLogWriteLogCallbackT_01, 10},
    {saLogWriteLogCallbackT_02, 10},
    {saLogFilterSetCallbackT_01, 11},
    {saLogStreamClose_01, 12},

    /* Limit Fetch API */
    {saLogLimitGet_01, 13},
    {NULL, 0}
};

int main(int argc, char **argv) 
{
    int i = 0;
    int curr_comp_number;
    struct tet_testlist *test;

    srandom(getpid());

    /* Run the tests */
    test = &api_testlist[i];
    curr_comp_number = test->icref;
    while (test->testfunc != NULL)
    {
        test->testfunc();
        i++;
        test = &api_testlist[i];
#if 0
        if (curr_comp_number != test->icref)
            printf("\n");
#endif
        curr_comp_number = test->icref;
        //usleep(100000);
    }

    /* Print result */
    printf("\n\n=====================================\n\n");
    printf("   Test Result:\n");
    printf("      Passed: %u\n", test_passed);
    printf("      Failed: %u\n", test_failed);
    printf("      Total:  %u\n", test_total);

    return 0; 
}  


