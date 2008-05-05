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

#include "lgs.h"
#include "lgs_util.h"
#include "lgs_fmt.h"

#define ALARM_STREAM_ENV_PREFIX "ALARM"
#define NOTIFICATION_STREAM_ENV_PREFIX "NOTIFICATION"
#define SYSTEM_STREAM_ENV_PREFIX "SYSTEM"

#define START_YEAR 1900
#define LOG_VER_EXP "LOG_SVC_VERSION:"
#define FMAT_EXP "FORMAT:"
#define CFG_EXP_MAX_FILE_SIZE "MAX_FILE_SIZE:"
#define CFG_EXP_FIXED_LOG_REC_SIZE "FIXED_LOG_REC_SIZE:"
#define CFG_EXP_LOG_FULL_ACTION "LOG_FULL_ACTION:"

void lgs_fillConfigDataValues(configurationFileDataT
                              *configFileData,
                              log_stream_t* stream)
{
    m_GET_MY_VERSION(configFileData->version);
    configFileData->action = DEFAULT_ALM_ACTION;
    configFileData->maxLogFileSize = stream->maxLogFileSize;
    configFileData->fixedLogRecordSize = stream->fixedLogRecordSize;
    configFileData->numberOfRotations = stream->maxFilesRotated;
    configFileData->formatExpression = stream->logFileFormat;
}

void lgs_writeLogFileConfigFile(FILE *filePtr,
    configurationFileDataT *configurationFileData)
{
    /* version */
    fprintf(filePtr,
            "%s %c.%d.%d\n",
            LOG_VER_EXP,
            configurationFileData->version.releaseCode,
            configurationFileData->version.majorVersion,
            configurationFileData->version.minorVersion);

    /* Format expression */
    fprintf(filePtr,
            "%s%s\n",
            FMAT_EXP,
            configurationFileData->formatExpression);

    /* Max logfile size */
    fprintf(filePtr,
            "%s %d\n",
            CFG_EXP_MAX_FILE_SIZE,
            (int)configurationFileData->maxLogFileSize);

    /* Fixed log record size */
    fprintf(filePtr,
            "%s %d\n",
            CFG_EXP_FIXED_LOG_REC_SIZE,
            configurationFileData->fixedLogRecordSize);

    /* Log file full action */
    fprintf(filePtr,
            "%s %s %d\n",
            CFG_EXP_LOG_FULL_ACTION,
            configurationFileData->action,
            configurationFileData->numberOfRotations);
}

/**
 * Get date and time string as mandated by SAF LOG file naming
 * 
 * @return char*
 */
char *lgs_get_time(void)
{
    struct tm *timeStampData;
    static char timeStampString[LGS_CREATE_CLOSE_TIME_LEN];
    char srcString[5];
    uns32 stringSize;
    uns32 characters = 0;
    time_t         testTime;

    time(&testTime);
    timeStampData = localtime(&testTime);

    stringSize = 5 * sizeof (char);
    characters = snprintf(srcString, 
                          (size_t)stringSize, 
                          "%d", 
                          (timeStampData->tm_year + START_YEAR));

    strncpy(timeStampString, 
            srcString, 
            stringSize);

    stringSize = 3 * sizeof (char);
    characters = snprintf(srcString, 
                          (size_t)stringSize, 
                          "%02d", 
                          (timeStampData->tm_mon + 1));

    strncat(timeStampString, 
            srcString, 
            stringSize);

    characters = snprintf(srcString, 
                          (size_t)stringSize, 
                          "%02d", 
                          (timeStampData->tm_mday));

    strncat(timeStampString, 
            srcString, 
            stringSize);

    strncat(timeStampString, "_", (2 * sizeof (char)));

    characters = snprintf(srcString, 
                          (size_t)stringSize, 
                          "%02d", 
                          (timeStampData->tm_hour));
    strncat(timeStampString, 
            srcString, 
            stringSize);

    characters = snprintf(srcString, 
                          (size_t)stringSize, 
                          "%02d", 
                          (timeStampData->tm_min));
    strncat(timeStampString, 
            srcString, 
            stringSize);

    characters = snprintf(srcString, 
                          (size_t)stringSize, 
                          "%02d", 
                          (timeStampData->tm_sec));
    strncat(timeStampString, 
            srcString, 
            stringSize);

    timeStampString[15] = '\0';
    return timeStampString;
}

SaTimeT lgs_get_SaTime(void)
{
    return time(NULL) * SA_TIME_ONE_SECOND;
}

uns32 lgs_setFmtString(log_stream_t *stream, char* fmatStr)
{
    int sz = strlen(fmatStr)+1;

    if (stream->logFileFormat != NULL)
        free(stream->logFileFormat);

    stream->logFileFormat = malloc(sz);
    if (NULL == stream->logFileFormat)
    {
        TRACE("could not alloc memory\n");
        return NCSCC_RC_OUT_OF_MEM;
    }
    strncpy(stream->logFileFormat, fmatStr, sz);
    return NCSCC_RC_SUCCESS;
}

int lgs_dir_exist(char *baseDir)
{
    int rv=-1;
    struct stat statbuf;

    rv=stat(baseDir, &statbuf);
    if (rv ==-1)
    {
        return 0;
    }
    else
    {
        if (S_ISDIR(statbuf.st_mode))
        {
            return 1;
        }
        else
        {
            TRACE(" File exists but is not a directory");
            return -1;
        }
    }

}

/**
 * Rename a file to include a timestamp in the name
 * @param path
 * @param old_name
 * @param time_stamp
 * @param suffix
 * 
 * @return int
 */
int lgs_file_rename(const char *path, const char *old_name,
                const char *time_stamp, const char *suffix)
{
    int ret;
    char oldpath[PATH_MAX + NAME_MAX];
    char newpath[PATH_MAX + NAME_MAX];

    sprintf(oldpath, "%s/%s%s", path, old_name, suffix);
    sprintf(newpath, "%s/%s_%s%s", path, old_name, time_stamp, suffix);
    TRACE_4("Rename file from %s", oldpath);
    TRACE_4("              to %s", newpath);
    if ((ret = rename(oldpath, newpath)) == -1)
        LOG_ER("rename: FAILED - %s",  strerror(errno));

    return ret;
}

int lgs_file_exist(const char *i_filename)
{
  int rv;
  struct stat statbuf;

  rv=stat(i_filename, &statbuf);
  if (rv ==-1)
  {
    return 0;
  }
  else
  {
    if (S_ISREG(statbuf.st_mode))
    {
      return 1;
    }
    else
    {
      TRACE("File %s is not a regular file\n", i_filename);
      return 0;
    }
  }
}

/**
 * Initialize one stream. Get configuration for the stream. Currently from
 * environment variables, later from IMM.
 * @param stream
 * @param stream_id
 * @param streamName
 * @param fmtExpr
 * @param env_prefix
 * 
 * @return uns32
 */
static uns32 init_stream(log_stream_t **stream, SaUint32T stream_id,
                         char* streamName, char* fmtExpr,
                         const char *env_prefix,
                         logStreamTypeT streamType)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    char *filename, *path, *format, *value;
    char env[100];
    SaNameT name;
    SaUint64T maxLogFileSize;
    SaUint32T fixedLogRecordSize;
    SaUint32T maxFilesRotated;
    SaBoolT twelveHourModeFlag;

    TRACE_ENTER();
    TRACE("%s", streamName);

    memset(&name, 0, sizeof(name));
    strcpy(name.value, streamName);
    name.length = strlen(name.value);

    /* FileName _must_ be configured according to spec. */
    sprintf(env, "LOG_%s_FILE_NAME", env_prefix);
    if ((filename = getenv(env)) == NULL)
    {
        LOG_ER("%s missing in environment", env);
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    /* PathName _must_ be configured according to spec. */
    sprintf(env, "LOG_%s_PATH_NAME", env_prefix);
    if ((path = getenv(env)) == NULL)
    {
        LOG_ER("%s missing in environment", env);
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    sprintf(env, "LOG_%s_LOG_FILE_FORMAT", env_prefix);
    if ((format = getenv(env)) != NULL)
    {
        if (lgs_validateFormatExpression((SaStringT) format, streamType,
                                     &twelveHourModeFlag) == SA_FALSE)
        {
            LOG_WA("validateFormatExpression for '%s' FAILED, using default", format);
            format = fmtExpr;
        }
    }
    else
        format = fmtExpr;

    maxLogFileSize = DEFAULT_MAX_LOG_FILE_SIZE;
    sprintf(env, "LOG_%s_MAX_LOG_FILE_SIZE", env_prefix);
    if ((value = getenv(env)) != NULL)
        maxLogFileSize = atoi(value);

    fixedLogRecordSize = DEFAULT_FIXED_LOG_RECORD_SIZE;
    sprintf(env, "LOG_%s_FIXED_LOG_RECORD_SIZE", env_prefix);
    if ((value = getenv(env)) != NULL)
        fixedLogRecordSize = atoi(value);

    maxFilesRotated = DEFAULT_MAX_FILES_ROTATED;
    sprintf(env, "LOG_%s_MAX_FILES_ROTATED", env_prefix);
    if ((value = getenv(env)) != NULL)
        maxFilesRotated = atoi(value);

    *stream = log_stream_new(&name, filename, path, maxLogFileSize,
                             fixedLogRecordSize, SA_TRUE, maxFilesRotated,
                             SA_LOG_FILE_FULL_ACTION_ROTATE, format,
                             streamType, STREAM_NEW);
    (*stream)->twelveHourModeFlag = twelveHourModeFlag; // FIX??

done:
    TRACE_LEAVE();
    return rc;
}

/**
 * Create all well known streams.
 * @param lgs_cb
 * 
 * @return uns32
 */
uns32 lgs_create_known_streams(lgs_cb_t *lgs_cb)
{
    int rc = 0;

    TRACE_ENTER();

    rc = init_stream(&lgs_cb->alarmStream, 0, SA_LOG_STREAM_ALARM,
                     DEFAULT_ALM_NOT_FORMAT_EXP, ALARM_STREAM_ENV_PREFIX,
                     STREAM_TYPE_ALARM);
    if (rc != NCSCC_RC_SUCCESS)
        goto done;

    rc = init_stream(&lgs_cb->notificationStream, 1, SA_LOG_STREAM_NOTIFICATION,
                     DEFAULT_ALM_NOT_FORMAT_EXP, NOTIFICATION_STREAM_ENV_PREFIX,
                     STREAM_TYPE_NOTIFICATION);
    if (rc != NCSCC_RC_SUCCESS)
        goto done;

    rc = init_stream(&lgs_cb->systemStream, 2, SA_LOG_STREAM_SYSTEM,
                     DEFAULT_APP_SYS_FORMAT_EXP, SYSTEM_STREAM_ENV_PREFIX,
                     STREAM_TYPE_SYSTEM);
    if (rc != NCSCC_RC_SUCCESS)
        goto done;

done:
    TRACE_LEAVE();
    return rc;
}

