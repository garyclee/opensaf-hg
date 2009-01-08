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
 * Test program for the log record formatter
 *
 * Compile: $ gcc -Wall -o formatter -I ../../../include/ -I ../inc lgs_fmt_main.c lgs_fmt.c
 * Run:     $ ./formatter
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <saLog.h>

#include "lgs_fmt.h"

#define DEFAULT_ALM_NOT_FORMAT_EXP "@Cr @Ct @Nt @Ne6 @No30 @Ng30 \"@Cb\""
#define DEFAULT_APP_SYS_FORMAT_EXP "@Cr @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sv @Sl \"@Cb\""

static SaTimeT getTime(void)
{
    struct timeval currentTime;
    SaTimeT logTime;

    /* Fetch current system time for time stamp value */
    (void)gettimeofday(&currentTime, 0);

    logTime = ((unsigned)currentTime.tv_sec * 1000000000ULL) + \
              ((unsigned)currentTime.tv_usec * 1000ULL);

    return logTime;  
}

static void format_log_record(SaLogRecordT *logRecord, unsigned int logBufSize,
    char *format, unsigned int buf_size, SaAisErrorT expected)
{
    static int logRecordIdCounter = 0;
    char buf[4096];
    int i;
    SaAisErrorT err;

    printf("%u, %u:", logBufSize, buf_size);
    memset(buf, 0, sizeof(buf));
    logRecord->logBuffer->logBufSize = logBufSize;
    err = lgs_format_log_record(logRecord, format, buf_size, buf, logRecordIdCounter++);
    if (err != expected)
    {
        printf(" ERROR: expected %u got %u\n", expected, err);
        exit(1);
    }

    i = printf("%s", buf);
    printf("\tlen %d\n", i);
}

int main(int argc, char **argv)
{
    SaLogRecordT gen_logRecord, alm_logRecord;
    SaNameT logSvcUsrName = {.value = "qwerty", .length = 7};
    SaLogBufferT logBuffer;
    SaUint8T logBuf[2048];
    SaNtfClassIdT notificationClassId = {1, 2, 3};
    SaBoolT twelveHourModeFlag = SA_FALSE;

    memset(logBuf, 'x', sizeof(logBuf));
    logBuffer.logBuf = logBuf;

    gen_logRecord.logTimeStamp = getTime();
    gen_logRecord.logHdrType = SA_LOG_GENERIC_HEADER;
    gen_logRecord.logHeader.genericHdr.notificationClassId = NULL;
    gen_logRecord.logHeader.genericHdr.logSvcUsrName = &logSvcUsrName;
    gen_logRecord.logHeader.genericHdr.logSeverity = SA_LOG_SEV_WARNING;
    gen_logRecord.logBuffer = &logBuffer;

    alm_logRecord.logTimeStamp = getTime();
    alm_logRecord.logHdrType = SA_LOG_NTF_HEADER;
    alm_logRecord.logHeader.ntfHdr.notificationId = SA_NTF_IDENTIFIER_UNUSED;
    alm_logRecord.logHeader.ntfHdr.eventType = SA_NTF_ALARM_PROCESSING;
    alm_logRecord.logHeader.ntfHdr.notificationObject = &logSvcUsrName;
    alm_logRecord.logHeader.ntfHdr.notifyingObject = &logSvcUsrName;
    alm_logRecord.logHeader.ntfHdr.notificationClassId = &notificationClassId;
    alm_logRecord.logHeader.ntfHdr.eventTime = getTime();
    alm_logRecord.logBuffer = &logBuffer;

    /*************** Test Application/System formatting *****************   */
    format_log_record(&gen_logRecord, 12, DEFAULT_APP_SYS_FORMAT_EXP, 64, SA_AIS_OK);
    format_log_record(&gen_logRecord, 64, DEFAULT_APP_SYS_FORMAT_EXP, 64, SA_AIS_OK);
    /* Test truncation indication (@Cx) */
    format_log_record(&gen_logRecord, 12, "@Cr @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sv @Sl @Cx \"@Cb\"", 64, SA_AIS_OK);
    format_log_record(&gen_logRecord, 64, "@Cr @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sv @Sl @Cx \"@Cb\"", 64, SA_AIS_OK);
    format_log_record(&gen_logRecord, 300, DEFAULT_APP_SYS_FORMAT_EXP, 256, SA_AIS_OK);
    format_log_record(&gen_logRecord, 2048, DEFAULT_APP_SYS_FORMAT_EXP, 256, SA_AIS_OK);
    format_log_record(&gen_logRecord, 2048, DEFAULT_APP_SYS_FORMAT_EXP, 1800, SA_AIS_OK);
    format_log_record(&gen_logRecord, 1000, DEFAULT_APP_SYS_FORMAT_EXP, 512, SA_AIS_OK);
    /* test invalid format char */
    format_log_record(&gen_logRecord, 12, "@xxx @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sv @Sl @Cx \"@Cb\"", 64, SA_AIS_ERR_INVALID_PARAM);


    /*************** Test Alarm/Notification formatting *********************/
    format_log_record(&alm_logRecord, 8, DEFAULT_ALM_NOT_FORMAT_EXP, 140, SA_AIS_OK);
    format_log_record(&alm_logRecord, 128, DEFAULT_ALM_NOT_FORMAT_EXP, 128, SA_AIS_OK);
    /* Test truncation indication (@Cx) */
    format_log_record(&alm_logRecord, 8, "@Cr @Ct @Nt @Ne6 @No30 @Ng30 @Cx \"@Cb\"", 140, SA_AIS_OK);
    format_log_record(&alm_logRecord, 128, "@Cr @Ct @Nt @Ne6 @No30 @Ng30 @Cx \"@Cb\"", 128, SA_AIS_OK);


    /*************** Test validation of format strings *********************/
    assert(lgs_is_valid_format_expression("@Cr @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sv @Sl @Cx \"@Cb\"",
                                          SA_LOG_GENERIC_HEADER,
                                          &twelveHourModeFlag) == SA_TRUE);
    assert(lgs_is_valid_format_expression("@Cr @Ch:@Cn:@Cs @x @Cm/@Cd/@CY @Sv @Sl @Cx \"@Cb\"",
                                          SA_LOG_GENERIC_HEADER,
                                          &twelveHourModeFlag) == SA_FALSE);

    return 0;
}
