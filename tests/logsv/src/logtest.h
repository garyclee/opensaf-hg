
#ifndef tet_log_h
#define tet_log_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <saLog.h>
#include <assert.h>
#include <utest.h>
#include <util.h>

#define SA_LOG_STREAM_APPLICATION1 "safLgStr=saLogApplication1"
#define SA_LOG_STREAM_APPLICATION2 "safLgStr=saLogApplication2"
#define SA_LOG_STREAM_APPLICATION3 "safLgStr=saLogApplication3"
#define DEFAULT_APP_LOG_REC_SIZE 128
#define DEFAULT_APP_LOG_FILE_SIZE 512
#define DEFAULT_FORMAT_EXPRESSION "@Cr @Ch:@Cn:@Cs @Cm/@Cd/@CY @Sv @Sl \"@Cb\""
#define SA_LOG_STREAM_APPLICATION1 "safLgStr=saLogApplication1"
#define DEFAULT_ALM_LOG_REC_SIZE 1024  
#define DEFAULT_ALM_LOG_BUFFER "Alarm stream test" 
#define DEFAULT_NOT_LOG_REC_SIZE 1024  
#define DEFAULT_NOT_LOG_BUFFER "Notification stream test" 
#define DEFAULT_NOTIFYING_OBJECT "safSu=ntfyo,safSg=ntfyo,safApp=ntfyo" 
#define DEFAULT_NOTIFICATION_OBJECT "safSu=ntfo,safSg=ntfo,safApp=ntfo"
#define DEFAULT_APP_FILE_PATH_NAME "logtest"
#define DEFAULT_APP_FILE_NAME "app1"
#define DEFAULT_MAX_FILE_ROTATED 4

extern SaNameT systemStreamName;
extern SaNameT alarmStreamName;
extern SaNameT notificationStreamName;
extern SaNameT app1StreamName;
extern SaNameT notifyingObject;
extern SaNameT notificationObject;
extern SaVersionT logVersion;
extern SaAisErrorT rc;
extern SaLogHandleT logHandle;
extern SaLogStreamHandleT logStreamHandle;
extern SaLogCallbacksT logCallbacks;
extern SaLogBufferT alarmStreamBuffer;
extern SaLogBufferT notificationStreamBuffer;
extern SaSelectionObjectT selectionObject;
extern SaNameT logSvcUsrName;
extern SaLogRecordT genLogRecord;

#endif
