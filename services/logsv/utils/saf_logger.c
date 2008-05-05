
/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Ericsson AB
 *
 */

/*
 * This file contains a command line utility to write to the SAF LOG.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

#include <saAis.h>
#include <saLog.h>

#include "saf_error.h"

 enum
{
    LOG_STREAM_SYSTEM,
    LOG_STREAM_NOTIFICATION,
    LOG_STREAM_ALARM
};

/* Defines */
#define CALLBACK_USED 1
#define SA_LOG_STREAM_APPLICATION "safLgStr=saLogApplication"

#define DEFAULT_FLAG 0x0001
#define DEFAULT_NOTIFICATION_OBJECT "safSu=ntfo,safSg=ntfo,safApp=ntfo"
#define DEFAULT_NOTIFYING_OBJECT "safSu=ntfyo,safSg=ntfyo,safApp=ntfyo"
#define DEFAULT_APPLICATION_LOG_FILE_NAME "JavaApplication"
#define DEFAULT_APPLICATION_LOG_FILE_PATH "/tmp"
#define DEFAULT_ALM_LOG_BUFFER "Alarm from saflogger"
#define DEFAULT_APP_LOG_BUFFER "Application from saflogger"
#define DEFAULT_NOT_LOG_BUFFER "Notification from saflogger"
#define DEFAULT_SYSTEM_LOG_BUFFER "This is a system notification!!"
#define DEFAULT_FORMAT_EXPRESSION "@Cr @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sv @Sl \"@Cb\""
#define DEFAULT_ALM_LOG_REC_SIZE 256
#define DEFAULT_APP_LOG_REC_SIZE 128
#define DEFAULT_APP_LOG_FILE_SIZE 512
#define VENDOR_ID 193

/* Internal structs */
typedef struct
{
    SaVersionT              testVersion;
    unsigned int            testHandle;
    SaNtfEventTypeT         eventType;
    SaNameT                 notificationObject;
    SaNameT                 notifyingObject;
    SaNtfClassIdT           notificationClassId;
    SaTimeT                 eventTime;
    SaTimeT                 logTimeStamp;
    SaNtfIdentifierT        notificationId;
    SaLogRecordT            myNtfLogRecord;
    SaLogRecordT            myGenLogRecord;
    SaLogAckFlagsT          ackFlags;
    SaLogStreamOpenFlagsT   logStreamOpenFlags;
    SaNameT                 myApplicationLogStreamName;
    SaNtfClassIdT           genNotificationClassId;
    SaLogSeverityT          logSeverity;
    SaNameT                 logSvcUsrName;
    SaLogFileCreateAttributesT_2 appFileCreateAttributes;
    SaInt32T                timeout;
    unsigned int            hangTimeout;
    unsigned int            repeateSends;
    SaNameT                 applicationLogFileName;
    SaNameT                 applicationLogFilePath;
    SaNameT                 formatExpression;
    SaUint8T                *myLogBuf;
    SaInvocationT            invocation;
    SaDispatchFlagsT        dispatchFlag;
} saLogParamsT;

typedef SaUint16T       saLogFlagsT;

static SaLogCallbacksT logCallbacks = {0, 0, 0};

/* Release code, major version, minor version */
static SaVersionT version = {'A', 2, 1};

static char *progname="saflogger";

static void *_xmalloc(size_t size, const char *file, const char *function, unsigned int line)
{
    void *p = malloc(size);
    if (p == NULL)
    {
        fprintf(stderr, "malloc failed at %s:%s:%d\n", file, function, line);
        exit(EXIT_FAILURE);
    }

    return p;
}

#define xmalloc(sz) _xmalloc(sz, __FILE__, __FUNCTION__, __LINE__)

static SaTimeT get_current_SaTime(void)
{
    struct timeval tv;
    SaTimeT ntfTime;

    gettimeofday(&tv, 0);

    ntfTime = ((unsigned)tv.tv_sec * 1000000000ULL) + \
              ((unsigned)tv.tv_usec * 1000ULL);

    return ntfTime;  
}

/**
 * 
 */
static void usage(void)
{
    printf("\nNAME\n");
    printf("\t%s - write log record to log stream\n", progname);

    printf("\nSYNOPSIS\n");
    printf("\t%s [options] [message ...]\n", progname);

    printf("\nDESCRIPTION\n");
    printf("\t%s is a SAF LOG client used to write a log record into a specified log stream.\n", progname);

    printf("\nOPTIONS\n");

    printf("  -l or --alarm                  write to alarm stream\n");
    printf("  -n or --notification           write to notification stream\n");
    printf("  -y or --system                 write to system stream (default)\n");
    printf("  -a NAME or --application=NAME  write to application stream NAME\n");
    printf("  -s SEV or --severity=SEV       use severity SEV, default INFO\n");
    printf("  -p PER or --period=PER         log with period PER\n");
    printf("      valid severity names: emerg, alert, crit, err, warn , notice, info\n");
}

/**
 * Write log record to stream
 * @param logStreamName
 * @param logFileCreateAttributes
 * @param logStreamOpenFlags
 * @param logRecord
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT write_log_record(
    const SaNameT *logStreamName,
    const SaLogFileCreateAttributesT_2 *logFileCreateAttributes,
    SaLogStreamOpenFlagsT logStreamOpenFlags,
    const SaLogRecordT *logRecord,
    int period)
{
    SaLogHandleT logHandle;
    SaLogStreamHandleT logStreamHandle;
    SaAisErrorT errorCode;
    SaSelectionObjectT selObj;
    unsigned int retries = 5; /* retry for 5 * 100 ms = 5s */
    SaInvocationT invocation;
    int i = 0;
    int write_index = 0;

    errorCode = saLogInitialize(&logHandle, &logCallbacks, &version);
    if (errorCode != SA_AIS_OK)
    {
        fprintf(stderr, "saLogInitialize FAILED: %s\n", saf_error(errorCode));
        return errorCode;
    }

    errorCode = saLogSelectionObjectGet(logHandle, &selObj);
    if (errorCode != SA_AIS_OK)
    {
        printf ("saLogSelectionObjectGet FAILED: %s\n", saf_error(errorCode));
        return errorCode;
    }

    /* Open the log stream */
    errorCode = saLogStreamOpen_2(logHandle,
                                  logStreamName,
                                  logFileCreateAttributes,    
                                  logStreamOpenFlags, 
                                  SA_TIME_ONE_SECOND,
                                  &logStreamHandle);
    if (SA_AIS_OK != errorCode)
    {
        fprintf(stderr, "saLogStreamOpen_2 FAILED: %s\n", saf_error(errorCode));
        return errorCode;
    }

    if (logRecord->logBuffer != NULL)
        write_index = strlen(logRecord->logBuffer->logBuf);

    do
    {
        if (logRecord->logBuffer != NULL && period > 0)
        {
            /* add unique ID to each log */
            sprintf(&logRecord->logBuffer->logBuf[write_index], " - %u", i++);
            logRecord->logBuffer->logBufSize = strlen(logRecord->logBuffer->logBuf);
        }
        retries = 5;
        errorCode = saLogWriteLogAsync(logStreamHandle,
                                       invocation,
                                       0, 
                                       logRecord);

        if ((errorCode != SA_AIS_OK) && (errorCode != SA_AIS_ERR_TRY_AGAIN))
        {
            fprintf(stderr, "saLogWriteLogAsync FAILED: %s\n", saf_error(errorCode));
            return errorCode;
        }

        if (errorCode == SA_AIS_ERR_TRY_AGAIN)
            usleep(100000); /* 100 ms */

        if (period > 0)
            sleep(period);
    } while ((SA_AIS_ERR_TRY_AGAIN == errorCode && --retries > 0) || period > 0);

//    waitForCallback(logHandle, (int)selObj, logParams);

    /* Close log stream */
    errorCode = saLogStreamClose(logStreamHandle);
    if (SA_AIS_OK != errorCode)
    {
        fprintf(stderr, "saLogStreamClose FAILED: %s\n", saf_error(errorCode));
        return errorCode;
    }

    errorCode = saLogFinalize(logHandle);
    if (SA_AIS_OK != errorCode)
    {
        fprintf(stderr, "saLogFinalize FAILED: %s\n",  saf_error(errorCode));
        return errorCode;
    }

    return errorCode;
}

static SaLogSeverityT get_severity(char *severity)
{
    if (strcmp(severity, "emerg") == 0)
        return SA_LOG_SEV_FLAG_EMERGENCY;

    if (strcmp(severity, "alert") == 0)
        return SA_LOG_SEV_FLAG_ALERT;

    if (strcmp(severity, "crit") == 0)
        return SA_LOG_SEV_FLAG_CRITICAL;

    if (strcmp(severity, "error") == 0)
        return SA_LOG_SEV_FLAG_ERROR;

    if (strcmp(severity, "warn") == 0)
        return SA_LOG_SEV_FLAG_WARNING;

    if (strcmp(severity, "notice") == 0)
        return SA_LOG_SEV_FLAG_NOTICE;

    if (strcmp(severity, "info") == 0)
        return SA_LOG_SEV_FLAG_INFO;

    usage();
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int c;
    SaNameT logStreamName = {.length = 0};
    SaLogFileCreateAttributesT_2 *logFileCreateAttributes = NULL;
    SaLogFileCreateAttributesT_2 appLogFileCreateAttributes;
    SaLogStreamOpenFlagsT logStreamOpenFlags = 0;
    SaLogRecordT logRecord;
    SaNameT logSvcUsrName;
    SaLogBufferT logBuffer;
    SaNtfClassIdT notificationClassId = {1, 2, 3};
    struct option long_options[] =
    {
        {"application", no_argument, 0, 'a'},
        {"alarm", no_argument, 0, 'l'},
        {"notification", no_argument, 0, 'n'},
        {"system", no_argument, 0, 'y'},
        {"severity", required_argument, 0, 's'},
        {"period", required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };
    int period = 0;

    strcpy(logSvcUsrName.value, "saflogger");
    logSvcUsrName.length = strlen(logSvcUsrName.value);

    /* Setup default values */
    strcpy(logStreamName.value, SA_LOG_STREAM_SYSTEM); /* system stream is default */
    logRecord.logTimeStamp = SA_TIME_UNKNOWN; /* LOG service should supply timestamp */
    logRecord.logHdrType = SA_LOG_GENERIC_HEADER;
    logRecord.logHeader.genericHdr.notificationClassId = NULL;
    logRecord.logHeader.genericHdr.logSeverity = SA_LOG_SEV_FLAG_INFO;
    logRecord.logHeader.genericHdr.logSvcUsrName = &logSvcUsrName;
    logRecord.logBuffer = NULL;
    appLogFileCreateAttributes.logFilePathName = "/repl_opensaf/saflog/saflogger";
    appLogFileCreateAttributes.maxLogFileSize = DEFAULT_APP_LOG_FILE_SIZE;
    appLogFileCreateAttributes.maxLogRecordSize = DEFAULT_APP_LOG_REC_SIZE;
    appLogFileCreateAttributes.haProperty = SA_TRUE;
    appLogFileCreateAttributes.logFileFullAction = SA_LOG_FILE_FULL_ACTION_ROTATE;
    appLogFileCreateAttributes.maxFilesRotated = 4;
    appLogFileCreateAttributes.logFileFmt = DEFAULT_FORMAT_EXPRESSION;

    while (1)
    {
        c = getopt_long(argc, argv, "lnya:s:p:", long_options, NULL);
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'l':
                strcpy(logStreamName.value, SA_LOG_STREAM_ALARM);
                logRecord.logHdrType = SA_LOG_NTF_HEADER;
                break;
            case 'n':
                strcpy(logStreamName.value, SA_LOG_STREAM_NOTIFICATION);
                logRecord.logHdrType = SA_LOG_NTF_HEADER;
                break;
            case 'y':
                strcpy(logStreamName.value, SA_LOG_STREAM_SYSTEM);
                break;
            case 'a':
                sprintf(logStreamName.value, "safLgStr=%s", optarg);
                logFileCreateAttributes = &appLogFileCreateAttributes;
                appLogFileCreateAttributes.logFileName = strdup(optarg);
                logStreamOpenFlags = SA_LOG_STREAM_CREATE;
                break;
            case 's':
                logRecord.logHeader.genericHdr.logSeverity = get_severity(optarg);
                break;
            case 'p':
                period = atoi(optarg);
                break;
            case '?':
            default:
                usage();
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (logRecord.logHdrType == SA_LOG_NTF_HEADER)
    {
        /* Setup some valid values */
        logRecord.logHeader.ntfHdr.notificationId = SA_NTF_IDENTIFIER_UNUSED;
        logRecord.logHeader.ntfHdr.eventType = SA_NTF_ALARM_PROCESSING;
        logRecord.logHeader.ntfHdr.notificationObject = &logSvcUsrName;
        logRecord.logHeader.ntfHdr.notifyingObject = &logSvcUsrName;
        logRecord.logHeader.ntfHdr.notificationClassId = &notificationClassId;
        logRecord.logHeader.ntfHdr.eventTime = get_current_SaTime();
    }

    logStreamName.length = strlen(logStreamName.value);

    /* Create body of log record (if any) */
    if (optind < argc)
    {
        int sz;
        char *logBuf = NULL;
        sz = strlen(argv[optind]);
        logBuf = xmalloc(sz + 64); /* add space for index/id in periodic printouts */
        strcpy(logBuf, argv[optind]);
        logBuffer.logBufSize = sz;
        logBuffer.logBuf = logBuf;
        logRecord.logBuffer = &logBuffer;
    }

    if (write_log_record(&logStreamName, logFileCreateAttributes,
                         logStreamOpenFlags, &logRecord, period) != SA_AIS_OK)
        return EXIT_FAILURE;
    else
        return EXIT_SUCCESS;
}

