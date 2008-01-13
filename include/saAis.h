
/*

  Header file of SA Forum AIS APIs (SAI-AIS-B.01.00.04)
  compiled on 14SEP2004 by sayandeb.saha@motorola.com.

*/

#ifndef _SA_AIS_H
#define _SA_AIS_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
    SA_FALSE = 0,
    SA_TRUE = 1
} SaBoolT;

typedef char                  SaInt8T;
typedef short                 SaInt16T;
typedef int                   SaInt32T;
typedef long long             SaInt64T;
typedef unsigned char         SaUint8T;
typedef unsigned short        SaUint16T;
typedef unsigned int          SaUint32T;
typedef unsigned long long    SaUint64T;

typedef SaInt64T              SaTimeT;
typedef SaUint64T             SaInvocationT;
typedef SaUint64T             SaSizeT;
typedef SaUint64T             SaOffsetT;
typedef SaUint64T             SaSelectionObjectT;
typedef SaUint64T             SaNtfIdentifierT;

#define SA_TIME_END              0x7FFFFFFFFFFFFFFFLL
#define SA_TIME_BEGIN            0x0
#define SA_TIME_UNKNOWN          0x8000000000000000LL

#define SA_TIME_ONE_MICROSECOND 1000
#define SA_TIME_ONE_MILLISECOND 1000000
#define SA_TIME_ONE_SECOND      1000000000
#define SA_TIME_ONE_MINUTE      60000000000
#define SA_TIME_ONE_HOUR        3600000000000
#define SA_TIME_ONE_DAY         86400000000000
#define SA_TIME_MAX             SA_TIME_END

#define SA_MAX_NAME_LENGTH 256
typedef struct {
    SaUint16T length;
    SaUint8T value[SA_MAX_NAME_LENGTH];
} SaNameT;

typedef struct {
    SaUint8T releaseCode;
    SaUint8T majorVersion;
    SaUint8T minorVersion;
} SaVersionT;

#define SA_TRACK_CURRENT 0x01
#define SA_TRACK_CHANGES 0x02
#define SA_TRACK_CHANGES_ONLY 0x04

typedef enum {
    SA_DISPATCH_ONE = 1,
    SA_DISPATCH_ALL = 2,
    SA_DISPATCH_BLOCKING = 3
} SaDispatchFlagsT;

typedef enum {
   SA_AIS_OK = 1,
   SA_AIS_ERR_LIBRARY = 2,
   SA_AIS_ERR_VERSION = 3,
   SA_AIS_ERR_INIT = 4,
   SA_AIS_ERR_TIMEOUT = 5,
   SA_AIS_ERR_TRY_AGAIN = 6,
   SA_AIS_ERR_INVALID_PARAM = 7,
   SA_AIS_ERR_NO_MEMORY = 8,
   SA_AIS_ERR_BAD_HANDLE = 9,
   SA_AIS_ERR_BUSY = 10,
   SA_AIS_ERR_ACCESS = 11,
   SA_AIS_ERR_NOT_EXIST = 12,
   SA_AIS_ERR_NAME_TOO_LONG = 13,
   SA_AIS_ERR_EXIST = 14,
   SA_AIS_ERR_NO_SPACE = 15,
   SA_AIS_ERR_INTERRUPT =16,
   SA_AIS_ERR_NAME_NOT_FOUND = 17,
   SA_AIS_ERR_NO_RESOURCES = 18,
   SA_AIS_ERR_NOT_SUPPORTED = 19,
   SA_AIS_ERR_BAD_OPERATION = 20,
   SA_AIS_ERR_FAILED_OPERATION = 21,
   SA_AIS_ERR_MESSAGE_ERROR = 22,
   SA_AIS_ERR_QUEUE_FULL = 23,
   SA_AIS_ERR_QUEUE_NOT_AVAILABLE = 24,
   SA_AIS_ERR_BAD_FLAGS = 25,
   SA_AIS_ERR_TOO_BIG = 26,
   SA_AIS_ERR_NO_SECTIONS = 27
} SaAisErrorT;

#ifdef  __cplusplus
}
#endif

#endif  /* _SA_AIS_H */


