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
 * Author(s): Emerson Network Power
 *
 */

/*****************************************************************************
..............................................................................

  MODULE NAME:  OS_DEFS.H

  DESCRIPTION:

  This module contains declarations for the OS-PRIMS implementation for the
  Template.

******************************************************************************
*/
#ifndef OS_DEFS_H
#define OS_DEFS_H

/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                     OPERATING SYSTEM INCLUDE FILES                      **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>
#include <grp.h> 
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>    /* Added for message-queues : PM : 28/10/03 */
#include <sys/msg.h>    /* Added for message-queues : PM : 28/10/03 */
#include <syslog.h>
#ifndef __USE_GNU 
struct msgbuf
{
   long int mtype;
   char mtext[1];
};
#endif /* else defined in <sys/msg.h> */
#include <sys/time.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/resource.h>

#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include <errno.h>
/*#include <asm/ioctls.h>*/
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <ctype.h>
#include <termios.h>

#include <math.h>

/* includes for DLL */
#include <dlfcn.h>

#if (NCSSYSM_STKTRACE == 1)
#include <execinfo.h>
#endif

#include <assert.h>
#include <sys/wait.h>

/** Meaningless expression to remove "unreferenced variable" compiler
 ** warnings.  Depending on the compiler and alternate definition may be:
 **/
#define USE(x)                  (x=x)
#define CONST_USE(x) {void * dummy = &x;}

/*****************************************************************************

  INCLUDE trg_defs.h IF THE END USER DEFINES "USE_TARGET_SYSTEM_TYPEDEFS" non zero

  The end user is responsible to supply contents of trg_defs.h file.

 ****************************************************************************/

# if(USE_TARGET_SYSTEM_TYPEDEFS != 0)
#include "trg_defs.h"
#endif

#if 0
#include "ncsgl_defs.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif


/*****************************************************************************

                     DECLARATIONS USED BY H&J BASE CODE

 ****************************************************************************/

#define LEAPDLL_API
#define MABOAC_API
#define MABCOM_AP
#define MABMAC_API
#define MABMAS_API
#define MABPSR_API
#define RMSDLL_API
#define CLIDLL_API
#define IFDDLL_API
#define IFNDDLL_API
#define IFADLL_API
#define GLDDLL_API
#define GLNDDLL_API
#define GLADLL_API
#define SPADLL_API
#define DTSDLL_API
#define MBCSVDLL_API
#define DTADLL_API
#define MQSVDLL_API
#define MQADLL_API
#define MQDDLL_API
#define MQNDDLL_API
#define CPADLL_API
#define CPDDLL_API
#define CPNDDLL_API
#define APS_DLL_API

/* Linux supports "long long" which is a 64-bit data type */
#define NCS_64BIT_DATA_TYPE_SUPPORT 1
#define NCS_UNS64_DEFINED 1

/************************************************************
*                 syslog priorities mapping                 *
*                                                           * 
*************************************************************/
#define  NCS_OS_LOG_EMERG    LOG_EMERG   /* system is unusable */
#define  NCS_OS_LOG_ALERT    LOG_ALERT  /* action must be taken immediately */
#define  NCS_OS_LOG_CRIT     LOG_CRIT   /* critical conditions */
#define  NCS_OS_LOG_ERR      LOG_ERR      /* error conditions */
#define  NCS_OS_LOG_WARNING  LOG_WARNING   /* warning conditions */
#define  NCS_OS_LOG_NOTICE   LOG_NOTICE  /* normal but significant condition */
#define  NCS_OS_LOG_INFO     LOG_INFO   /* informational */
#define  NCS_OS_LOG_DEBUG    LOG_DEBUG  /* debug-level messages */

extern void ncs_os_atomic_init(void);
extern void ncs_os_atomic_destroy(void);

#ifndef m_NCS_OS_DBG_PRINTF
EXTERN_C LEAPDLL_API int ncs_dbg_logscreen(const char *str , ...); 
#define m_NCS_OS_DBG_PRINTF         ncs_dbg_logscreen
#endif

#ifndef m_NCS_OS_SYSLOG
EXTERN_C LEAPDLL_API void ncs_syslog(int priority, const char *str , ...); 
#define m_NCS_OS_SYSLOG         ncs_syslog
#endif

#ifndef m_NCS_OS_ATOMIC_INC
extern void ncs_os_atomic_inc(void *puns32);
#define m_NCS_OS_ATOMIC_INC(p_uns32) ncs_os_atomic_inc(p_uns32);
#endif


#ifndef m_NCS_OS_ATOMIC_DEC
extern void ncs_os_atomic_dec(void *p_uns32);
#define m_NCS_OS_ATOMIC_DEC(p_uns32) ncs_os_atomic_dec(p_uns32);
#endif

extern void get_msec_time(uns32 * seconds, uns32 * millisec);

#ifndef m_NCS_OS_GET_UPTIME 
extern uns32 ncs_get_uptime(uns64 *o_uptime);
#define m_NCS_OS_GET_UPTIME(p_uptime)  ncs_get_uptime(p_uptime)
#endif

extern NCS_BOOL ncs_is_root(void);
#define m_GET_MSEC_TIME_STAMP(seconds, millisec) get_msec_time(seconds, millisec)

#define NCS_MKDIR_DEFINED

/*****************************************************************************
 **                                                                         **
 **             OS Prims prototypes                                         **
 **                                                                         **
 ****************************************************************************/

int getversion(void);

int ncs_console_status(void);
int ncs_unbuf_getch(void);
void ncs_stty_reset(void);   /* developed as a part of fxing the bug 58609 */

/*****************************************************************************
 **                                                                         **
 **             Message queue primitive defintions                          **
 **                                                                         **
 ****************************************************************************/

#define NCS_OS_MQ_KEY         key_t
#define NCS_OS_MQ_HDL         int  
#define NCS_OS_MQ_MSG_LL_HDR  long /* The "header" part of <struct msgbuf> */
#define NCS_OS_MQ_MAX_PAYLOAD 4500 /* This is actually dependant on 
                                      configuration. Usual defaults in LINUX
                                      are about 8192. : Phani:28/10/03 */

/*****************************************************************************
 **                                                                         **
 **             POSIX Message queue primitive defintions                          **
 **                                                                         **
 ****************************************************************************/

#define NCS_OS_POSIX_MQD      uns32
#define NCS_OS_POSIX_TIMESPEC struct timespec 

/*****************************************************************************
 **                                                                         **
 **             Console macro definitions                                   **
 **                                                                         **
 ****************************************************************************/

#define m_NCS_OS_UNBUF_GETCHAR           ncs_unbuf_getch

unsigned int linux_char_normalizer(void);
#define m_NCS_OS_NORMALIZE_CHR           linux_char_normalizer

#define __NCSINC_LINUX__

#define m_NCS_OS_REBOOT system("reboot &")


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                   System Timestamp Interface Macros                     **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/




int64 ncs_os_time_ms(void);  
#define m_NCS_OS_GET_TIME_MS   ncs_os_time_ms()

uns64 ncs_os_time_ns(void); 
#define m_NCS_OS_GET_TIME_NS   ncs_os_time_ns()

/* Over riding macros for linux */

/* gid is of gid_t*/
#define m_NCS_OS_GETGRGID(gid) getgrgid(gid)

/* num_of_groups is of int ,list is of gid_t [] */
#define m_NCS_OS_GETGROUPS(num_of_groups,list) getgroups(num_of_groups,list)

#define m_NCS_OS_TIME_TO_STR(timestamp, asc_timestamp)  \
{ \
    struct tm IR58027_temp_tm/* special auto var-name to avoid conflict with macro arguments */; \
    strftime((char *)(asc_timestamp), 32, "%X", localtime_r(&timestamp, &IR58027_temp_tm)); \
}

#define m_NCS_OS_DATE_TIME_TO_STR(timestamp, asc_timestamp)  \
{ \
    struct tm IR58027_temp_tm/* special auto var-name to avoid conflict with macro arguments */; \
    strftime((char *)(asc_timestamp), 40, "%d%b%Y_%H.%M.%S", localtime_r(&timestamp, &IR58027_temp_tm)); \
}

#define m_NCS_OS_GET_ASCII_TIME_STAMP(timestamp, asc_timestamp)  \
{ \
    struct tm IR58027_temp_tm; \
    (timestamp) = (time_t) (time((time_t *) 0)); \
    strftime((char *)(asc_timestamp), 32, "%X", localtime_r(&timestamp, &IR58027_temp_tm)); \
}

#define m_NCS_OS_GET_ASCII_DATE_TIME_STAMP(timestamp, asc_timestamp)  \
{ \
    struct tm IR58027_temp_tm; \
    timestamp=(time_t) (time((time_t *) 0)); \
    strftime((char *)(asc_timestamp), 40, "%d%b%Y_%H.%M.%S", localtime_r(&timestamp, &IR58027_temp_tm)); \
}

#define m_NCS_OS_GET_ASCII_HDR_DATE_TIME_STAMP(timestamp, asc_timestamp)  \
{ \
    struct tm IR58027_temp_tm; \
    timestamp=(time_t) (time((time_t *) 0)); \
    strftime((char *)(asc_timestamp), 40, "%d %B %Y %H:%M:%S", localtime_r(&timestamp, &IR58027_temp_tm)); \
}

/******* Macros for DLL's ********/
#ifndef NCS_USE_DLIB 
#define NCS_USE_DLIB (1)
#endif


#if (NCS_USE_DLIB == 1)
#define NCS_OS_DLIB_HDL                        void *
#define m_NCS_OS_DLIB_ATTR                     (RTLD_LAZY|RTLD_GLOBAL)
#define m_NCS_OS_DLIB_LOAD(file,attr)          dlopen(file,attr)
#define m_NCS_OS_DLIB_SYMBOL(lib_hdl,symbol)   dlsym(lib_hdl,symbol)
#define m_NCS_OS_DLIB_ERROR()                  dlerror()
#define m_NCS_OS_DLIB_CLOSE(lib_hdl)           dlclose(lib_hdl)
#endif


/****************************************************************************
 *
 * POSIX Locks - override OS_PRIMS default
 *
 ***************************************************************************/

typedef struct posixlock {
  pthread_mutex_t  lock;
  unsigned long    count;
  pthread_t        tid;
} POSIXLOCK;

#define NCS_OS_LOCK POSIXLOCK

extern NCS_OS_LOCK * get_cloexec_lock(void);

/* Extern defenition for sysf_fopen */
extern FILE * ncs_os_fopen(const char* fpath, const char* fmode);

#ifndef m_NCS_OS_LOG_FOPEN
#define m_NCS_OS_LOG_FOPEN(fname,fmode) ncs_os_fopen(fname,fmode)
#endif

#ifndef m_NCS_OS_FOPEN_R
#define m_NCS_OS_FOPEN_R(filename)   ncs_os_fopen(filename,"r")
#endif


#define m_NCS_OS_NTOHL_P(p8) (uns32)((*(uns8*)p8<<24)|(*(uns8*)(p8+1)<<16)| \
    (*(uns8*)(p8+2)<<8)|(*(uns8*)(p8+3)))
#define m_NCS_OS_NTOHS_P(p8) (uns16)((*(uns8*)p8<<8)|*((uns8*)(p8+1)))


#define m_NCS_OS_HTONL_P(p8,v32) { \
   *p8     = (uns8)(v32>>24); \
   *(p8+1) = (uns8)(v32>>16); \
   *(p8+2) = (uns8)(v32>>8);  \
   *(p8+3) = (uns8)v32; }

#define m_NCS_OS_HTONS_P(p8,v16) { \
   *p8     = (uns8)(v16>>8); \
   *(p8+1) = (uns8)v16; }


/*****************************************************************************
 **                                                                         **
 **                              Stack Trace                                **
 **                                                                         **
 ****************************************************************************/
#if (NCSSYSM_STKTRACE == 1)
#define NCSSYS_STKTRACE_FRAMES_MAX 128
typedef struct ncs_os_stktrace_info
{
    uns32   flags;            /* fetched=1, */
    struct  ncs_os_stktrace_info *next;
    uns32   ticks;
    int     addr_count;       /* number of addresses in stack trace (dwarray) */
    void   *addr_array[128];  /* array of function addresses */
} NCS_OS_STKTRACE_ENTRY;

void ncs_os_stacktrace_get(NCS_OS_STKTRACE_ENTRY * st);
void ncs_os_stacktrace_expand(NCS_OS_STKTRACE_ENTRY * st, uns8 *outstr, uns32 * outlen);

#define m_NCS_OS_STACKTRACE_GET     ncs_os_stacktrace_get
#define m_NCS_OS_STACKTRACE_EXPAND  ncs_os_stacktrace_expand

#endif

/****************************************************************************
 * m_NCS_OS_TARGET_INIT
 *
 * Macro arguments
 *  none
 *
 * Macro return codes
 *  void
 *
 ***************************************************************************/
#define m_NCS_OS_TARGET_INIT ncs_os_target_init()
extern unsigned int ncs_os_target_init(void);



/*****************************************************************************
 **                                                                         **
 **                   Task Priorities                                       **
 **                                                                         **
 ****************************************************************************/
#define NCS_OS_TASK_PRIORITY_0          1
#define NCS_OS_TASK_PRIORITY_2          2
#define NCS_OS_TASK_PRIORITY_3          3
#define NCS_OS_TASK_PRIORITY_4          4
#define NCS_OS_TASK_PRIORITY_5          5
#define NCS_OS_TASK_PRIORITY_6          6
#define NCS_OS_TASK_PRIORITY_7          7
#define NCS_OS_TASK_PRIORITY_8          8
#define NCS_OS_TASK_PRIORITY_9          9
#define NCS_OS_TASK_PRIORITY_10         10
#define NCS_OS_TASK_PRIORITY_11         11
#define NCS_OS_TASK_PRIORITY_12         12
#define NCS_OS_TASK_PRIORITY_13         13
#define NCS_OS_TASK_PRIORITY_14         14
#define NCS_OS_TASK_PRIORITY_15         15
#define NCS_OS_TASK_PRIORITY_16         16




/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **             ncs_sel_obj_*  primitive definitions                        **
 **                                                                         **
 **           Override default definition in ncs_osprm.h                    **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/
#ifndef NCS_SEL_OBJ_DEFINED
#define NCS_SEL_OBJ_DEFINED
#endif
typedef struct ncs_sel_obj{ 
   int raise_obj; 
   int rmv_obj; 
}NCS_SEL_OBJ;
typedef fd_set NCS_SEL_OBJ_SET;

#ifdef  __cplusplus
}
#endif

#endif   /* OS_DEFS_H */
