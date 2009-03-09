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

  MODULE NAME: rde_log.h


..............................................................................

  DESCRIPTION:



******************************************************************************
*/
#ifndef RDE_LOG_H
#define RDE_LOG_H

/* Forward declarations */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

Manifest Constants

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


#if (NCSRDE_LOG == 1)
#define m_RDE_ENTRY(funcName)                           \
         /* Use this for debugging to show Function Entry Exit */


#else

#define m_RDE_ENTRY(funcName)


#endif

/*****************************************************************************\
 *                                                                             *
 *   FLEXLOG NOTES                                                             *
 *                                                                             *
 *   FlexLog defines two attributes of a Log message:                          *
 *                                                                             *
 *       Severity                                                              *
 *       Category                                                              *
 *                                                                             *
 *   The Severity Level of a log message is independent of its Category        *
 *                                                                             *
 *                                                                             *
 *                                                                             *
\*****************************************************************************/



/*****************************************************************************\
 *                                                                             *
 *   FLEXLOG SEVERITY LEVELS                                                   *
 *                                                                             *
 *   FlexLog defines a spectrum of Log Severity Levels                         *
 *   as reproduced below for reference.                                        *
 *                                                                             *
 *   The RDE implementation uses the FlexLog Severity Level definitions        *
 *   directly.                                                                 *
 *                                                                             *
 *   #define NCSFL_SEV_EMERGENCY  0x01     System is unusable                  *
 *   #define NCSFL_SEV_ALERT      0x02     Action must be taken immediately    *
 *   #define NCSFL_SEV_CRITICAL   0x04     Critical condition                  *
 *   #define NCSFL_SEV_ERROR      0x08     Error conditions                    *
 *   #define NCSFL_SEV_WARNING    0x10     Warning condition                   *
 *   #define NCSFL_SEV_NOTICE     0x20     Normal, but significant condition   *
 *   #define NCSFL_SEV_INFO       0x40     Informational messages              *
 *   #define NCSFL_SEV_DEBUG      0x80     Debug-level messages                *
 *                                                                             *
 *                                                                             *
 *   syslog "levels" are defined similarly, as follows:                        *
 *                                                                             *
 *   #define LOG_EMERG           0        system is unusable                   *
 *   #define LOG_ALERT           1        action must be taken immediately     *
 *   #define LOG_CRIT            2        critical conditions                  *
 *   #define LOG_ERR             3        error conditions                     *
 *   #define LOG_WARNING         4        warning conditions                   *
 *   #define LOG_NOTICE          5        normal but significant condition     *
 *   #define LOG_INFO            6        informational                        *
 *   #define LOG_DEBUG           7        debug-level message                  *
 *                                                                             *
\*****************************************************************************/

#define RDE_SEV_EMERGENCY  NCSFL_SEV_EMERGENCY
#define RDE_SEV_ALERT      NCSFL_SEV_ALERT
#define RDE_SEV_CRITICAL   NCSFL_SEV_CRITICAL
#define RDE_SEV_ERROR      NCSFL_SEV_ERROR
#define RDE_SEV_WARNING    NCSFL_SEV_WARNING
#define RDE_SEV_NOTICE     NCSFL_SEV_NOTICE
#define RDE_SEV_INFO       NCSFL_SEV_INFO
#define RDE_SEV_DEBUG      NCSFL_SEV_DEBUG


#define RDE_LOG_EMERGENCY  0
#define RDE_LOG_ALERT      1
#define RDE_LOG_CRITICAL   2
#define RDE_LOG_ERROR      3
#define RDE_LOG_WARNING    4
#define RDE_LOG_NOTICE     5
#define RDE_LOG_INFO       6
#define RDE_LOG_DEBUG      7

#define RDE_LOG_MSG_SIZE   256 
#define RDE_RDE_MSG_SIZE   16 
/*****************************************************************************\
 *                                                                             *
 *   FLEXLOG LOG CATEGORIES                                                    *
 *                                                                             *
 *   FlexLog defines a set of Log Categories                                   *
 *   as reproduced below for reference.                                        *
 *                                                                             *
 *   Subsystems can define their own categories, but are required to           *
 *   define them so that they map to the "right" FlexLog category.             *
 *                                                                             *
 *                                                                             *
 *                                                                             *
 *   #define NCSFL_LC_API         0x01       API or Callback invoked           *
 *   #define NCSFL_LC_HEADLINE    0x02       Basic noteworthy condition        *
 *   #define NCSFL_LC_STATE       0x03       State-transition announcement     *
 *   #define NCSFL_LC_TIMER       0x04       Timer stop/start/expire stuff     *
 *   #define NCSFL_LC_EVENT       0x10       Unusual, often async 'event'      *
 *   #define NCSFL_LC_DATA        0x20       data (ex. PDU, memory) info       *
 *   #define NCSFL_LC_FSM         0x40       Finite State Machine stuff        *
 *   #define NCSFL_LC_SVC_PRVDR   0x80       regarding my service providers    *
 *                                                                             *
 *   #define NCSFL_LC_MISC        0x00       The above categories don't fit    *
 *                                                                             *
 *                                                                             *
\*****************************************************************************/

#ifdef _WIN32
#define __func__ "****"
#endif


/**************************************************************************\
 *                                                                          *
 *                          Headlines                                       *
 *                                                                          *
\**************************************************************************/

typedef enum
{
  RDE_HDLN_RDE_BANNER,
  RDE_HDLN_RDE_BANNER_SPACE,
  RDE_HDLN_RDE_STARTING,
  RDE_HDLN_RDE_STOPPING,
  RDE_HDLN_TASK_STARTED_CLI,
  RDE_HDLN_TASK_STOPPED_CLI,
  RDE_HDLN_SIG_INT,
  RDE_HDLN_SIG_TERM,

  RDE_HDLN_MAX

} RDE_LOG_HEADLINES;



/**************************************************************************\
 *                                                                          *
 *                          Reportable Conditions                           *
 *                                                                          *
\**************************************************************************/

typedef enum
{

  RDE_COND_UNSUPPORTED_RDA_CMD,
  RDE_COND_INVALID_RDA_CMD,
  RDE_LOG_COND_NO_PID_FILE,
  RDE_LOG_COND_NO_SHELF_NUMBER,
  RDE_LOG_COND_NO_SLOT_NUMBER,
  RDE_LOG_COND_NO_SITE_NUMBER,
  RDE_LOG_COND_PID_FILE_OPEN,
  RDE_COND_RDE_VERSION,
  RDE_COND_RDE_BUILD_DATE,
  RDE_COND_TASK_CREATE_FAILED,
  RDE_COND_SEM_CREATE_FAILED,
  RDE_COND_SEM_TAKE_FAILED,
  RDE_COND_SEM_GIVE_FAILED,
  RDE_COND_LOCK_CREATE_FAILED,
  RDE_COND_LOCK_CREATE_SUCCESS,
  RDE_COND_LOCK_TAKE_FAILED,
  RDE_COND_LOCK_TAKE_SUCCESS,
  RDE_COND_LOCK_GIVE_FAILED,
  RDE_COND_LOCK_GIVE_SUCCESS,
  RDE_COND_BAD_COMMAND,
  RDE_COND_RDA_CMD,
  RDE_COND_RDA_RESPONSE,
  RDE_COND_SELECT_ERROR,
  RDE_COND_SOCK_CREATE_FAIL,
  RDE_COND_SOCK_REMOVE_FAIL,
  RDE_COND_SOCK_BIND_FAIL,
  RDE_COND_SOCK_ACCEPT_FAIL,
  RDE_COND_SOCK_CLOSE_FAIL,
  RDE_COND_SOCK_SEND_FAIL,
  RDE_COND_SOCK_RECV_FAIL,
  RDE_COND_SOCK_NULL_NAME,
  RDE_COND_SOCK_NULL_ADDR,
  RDE_COND_RDA_SOCK_CREATED,
  RDE_COND_MEM_DUMP,
  RDE_COND_NO_MEM_DEBUG,
  RDE_COND_MEM_IGNORE,
  RDE_COND_MEM_REMEMBER,
  RDE_COND_HA_ROLE,
  RDE_COND_ACTIVE_PLANE,
  /* HA for RDE */
  RDE_COND_SETENV_COMP_NAME_FAIL,
  RDE_COND_SETENV_HC_KEYE_FAIL,
  RDE_COND_AMF_DISPATCH_FAIL,
  RDE_COND_CSI_SET_INVOKED,
  RDE_COND_HEALTH_CHK_INVOKED,
  RDE_COND_CSI_REMOVE_INVOKED,
  RDE_COND_COMP_TERMINATE_INVOKED,
  RDE_COND_AMF_INIT_FAILED,
  RDE_COND_AMF_INIT_DONE,
  RDE_COND_AMF_HEALTH_CHK_START_FAIL,
  RDE_COND_AMF_HEALTH_CHK_START_DONE,
  RDE_COND_AMF_GET_OBJ_FAILED,
  RDE_COND_AMF_GET_NAME_FAILED,
  RDE_COND_AMF_REG_FAILED,
  RDE_COND_AMF_REG_DONE,
  RDE_COND_AMF_UNREG_FAILED,
  RDE_COND_AMF_UNREG_DONE,
  RDE_COND_AMF_DESTROY_DONE,
  RDE_COND_PIPE_OPEN_FAILED,
  RDE_COND_PIPE_OPEN_DONE,
  RDE_COND_PIPE_READ_FAILED,
  RDE_COND_PIPE_CREATE_FAILED,
  RDE_COND_NCS_LIB_AVA_CREATE_FAILED,
  RDE_COND_NCS_LIB_INIT_DONE,
  RDE_COND_CLUSRTER_ID_INFO,
  RDE_COND_NODE_ID_INFO,
  RDE_COND_PCON_ID_INFO,
  RDE_COND_HEALTH_CHK_RSP_SENT,
  RDE_COND_CSI_SET_RSP_SENT,
  RDE_COND_GET_NODE_ID_FAILED,
  RDE_COND_CORE_AGENTS_START_FAILED,
  RDE_COND_PIPE_COMP_NAME,
  RDE_COND_PIPE_HC_KEY,

  /* RDE TO RDE */
  RDE_RDE_SOCK_CREATE_FAIL,
  RDE_RDE_SOCK_BIND_FAIL,
  RDE_RDE_SOCK_ACCEPT_FAIL,
  RDE_RDE_SOCK_CLOSE_FAIL,
  RDE_RDE_SOCK_WRITE_FAIL,
  RDE_RDE_SOCK_LISTEN_FAIL,
  RDE_RDE_SOCK_READ_FAIL,
  RDE_RDE_SOCK_CONN_FAIL,
  RDE_RDE_SOCK_REMOVE_FAIL,
  RDE_RDE_CLIENT_SOCK_CREATE_FAIL,
  RDE_RDE_SELECT_ERROR,
  RDE_RDE_SELECT_TIMED_OUT,
  RDE_RDE_RECV_INVALID_MESSAGE,
  RDE_RDE_UNABLE_TO_OPEN_CONFIG_FILE,
  RDE_RDE_FAIL_TO_CLOSE_CONFIG_FILE,
  RDE_RDE_ROLE_IS_NOT_VALID,
  RDE_RDE_SET_SOCKOPT_ERROR,
  RDE_RDE_INFO,
  RDE_RDE_ERROR,

  RDE_LOG_COND_MAX

} RDE_LOG_CONDITIONS;




/**************************************************************************\
 *                                                                          *
 *                          Memory Allocation Failures                      *
 *                                                                          *
\**************************************************************************/

typedef enum
{
  RDE_MEM_FAIL_IPMC_SERIAL,
  RDE_MEM_FAIL_IPMC_CMD,
  RDE_MEM_FAIL_CLI_SOCK,
  RDE_MEM_FAIL_CLI_CMD,
  RDE_MEM_FAIL_CLI_ADDR

} RDE_LOG_MEM_FAIL;



/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
******************************************************************************/

typedef enum
{
   RDE_FC_HEADLINE,
   RDE_FC_CONDITION,
   RDE_FC_MEM_FAIL

} RDE_LOG_TYPES;


typedef enum
{
   RDE_FMT_HEADLINE,
   RDE_FMT_HEADLINE_NUM,
   RDE_FMT_HEADLINE_NUM_BOX,
   RDE_FMT_HEADLINE_TIME,
   RDE_FMT_HEADLINE_TIME_BOX,
   RDE_FMT_HEADLINE_STR,
   RDE_FMT_MEM_FAIL

} RDE_LOG_FORMATS;

/*****************************************************************************\
 *                                                                             *
 *                             Function Prototypes                             *
 *                                                                             *
 *                                                                             *
\*****************************************************************************/

EXTERN_C const char *rde_get_log_levelstr  (uns32 log_level) ;

EXTERN_C void  rde_log_headline         (uns32 line, const char * file, const char * func, uns8 id);
EXTERN_C void  rde_log_condition        (uns32 line, const char * file, const char * func, uns8 sev, uns8 id);
EXTERN_C void  rde_log_condition_int    (uns32 line, const char * file, const char * func, uns8 sev, uns8 id, uns32  val);
EXTERN_C void  rde_log_condition_float  (uns32 line, const char * file, const char * func, uns8 sev, uns8 id, DOUBLE val);
EXTERN_C void  rde_log_condition_string (uns32 line, const char * file, const char * func, uns8 sev, uns8 id, const char * val);
EXTERN_C void  rde_log_mem_fail         (uns32 line, const char * file, const char * func, uns8 id);
EXTERN_C void  rde_log_event            (uns32 line, const char * file, const char * func, uns8 sev, uns8 id);
EXTERN_C const char * rde_get_log_level_str (uns32 log_level);
EXTERN_C void rdeSysLog (uns32 severity, char * logMessage);



/*****************************************************************************\
 *                                                                             *
 *                             Macro Mappings                                  *
 *                                                                             *
 *                                                                             *
\*****************************************************************************/



#define m_RDE_LOG_HDLN_BOX(id) \
                            rde_log_headline(RDE_HDLN_BANNER);         \
                            rde_log_headline(RDE_HDLN_BANNER_SPACE);   \
                            rde_log_headline(id);                    \
                            rde_log_headline(RDE_HDLN_BANNER_SPACE);   \
                            rde_log_headline(RDE_HDLN_BANNER)

#define m_RDE_LOG_HDLN_BOX_HDR(sev, id) \
                            rde_log_headline(sev, RDE_HDLN_BANNER);         \
                            rde_log_headline(sev, RDE_HDLN_BANNER_SPACE);   \
                            rde_log_headline(sev, id);                    \
                            rde_log_headline(sev, RDE_HDLN_BANNER_SPACE);

#define m_RDE_LOG_HDLN(id)                rde_log_headline         (__LINE__, __FILE__,  __func__, id)
#define m_RDE_LOG_COND(sev, id)           rde_log_condition        (__LINE__, __FILE__,  __func__, sev, id)
#define m_RDE_LOG_COND_L(sev, id, val)    rde_log_condition_int    (__LINE__, __FILE__,  __func__, sev, id, val)
#define m_RDE_LOG_COND_F(sev, id, val)    rde_log_condition_float  (__LINE__, __FILE__,  __func__, sev, id, val)
#define m_RDE_LOG_COND_C(sev, id, val)    rde_log_condition_string (__LINE__, __FILE__,  __func__, sev, id, val)
#define m_RDE_LOG_MEM_FAIL(id)            rde_log_mem_fail         (__LINE__, __FILE__,  __func__, id)
#define m_RDE_LOG_EVENT(sev, id)          rde_log_event            (__LINE__, __FILE__,  __func__, sev, id)

#define m_RDE_DBG_FILE_LINE        "%s:%d: "
#define m_RDE_DBG_FILE_LINE_ARG    __FILE__,__LINE__

/* DTSv versioning changes */
#define NCS_RDE_VERSION 2

#endif /* RDE_LOG_H */
