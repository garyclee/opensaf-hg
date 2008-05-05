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
* Module Inclusion Control...
*/
#ifndef LGSV_MSG_H
#define LGSV_MSG_H

#include "saLog.h"

/* to be simple in the begining we use fixed size*/
#define LGS_MAX_STRING_LEN 64
#define LGS_CREATE_CLOSE_TIME_LEN 16
#define LGS_LOG_FILE_EXT_LEN 5
#define LGS_LOG_FILE_EXT ".log"
#define LGS_LOG_FILE_CONFIG_EXT ".cfg"
#define UNDERSCORE "_"
#define UNDERSCORE_LEN 2

/* AMF API enums */
typedef enum lgsv_msg_type
{
   LGSV_BASE_MSG,
   LGSV_LGA_API_MSG = LGSV_BASE_MSG,
   LGSV_LGA_MISC_MSG,
   LGSV_LGS_MISC_MSG,
   LGSV_LGS_CBK_MSG,
   LGSV_LGA_API_RESP_MSG,
   LGSV_MSG_MAX
} LGSV_MSG_TYPE;

/* LGSV API enums */
typedef enum lgsv_api_msg_type
{
   LGSV_API_BASE_MSG,
   LGSV_LGA_INITIALIZE = LGSV_API_BASE_MSG,
   LGSV_LGA_FINALIZE,
   LGSV_LGA_LSTR_OPEN_SYNC,
   LGSV_LGA_LSTR_CLOSE,
   LGSV_LGA_WRITE_LOG_ASYNC,
   LGSV_API_MAX
} LGSV_API_TYPE;

/* LGSV Callback enums */

typedef enum lgsv_cbk_msg_type
{
   LGSV_CBK_BASE_MSG,
   LGSV_LGS_LSTR_OPEN = LGSV_CBK_BASE_MSG,
   LGSV_LGS_WRITE_LOG_CBK, 
   LGSV_LGS_CBK_MAX
} LGSV_CBK_TYPE;

typedef enum lgsv_api_resp_msg_type
{
   LGSV_LGA_INITIALIZE_RSP_MSG = 1,
   LGSV_LGA_LSTR_OPEN_SYNC_RSP_MSG,
   LGSV_LGA_API_RSP_MAX
} LGSV_API_RSP_TYPE;

/* 
* API & callback API parameter definitions.
* These definitions are used for 
* 1) encoding & decoding messages between LGA & LGS.
* 2) storing the callback parameters in the pending callback 
*    list.
*/

/*** API Parameter definitions ***/
typedef struct lgsv_lga_init_param_tag
{
    SaVersionT                version;
} LGSV_LGA_INIT_PARAM;

typedef struct lgsv_lga_finalize_param_tag
{
   uns32                     reg_id;
} LGSV_LGA_FINALIZE_PARAM;

typedef struct lgsv_lga_lstr_open_sync_param_tag
{
   uns32     reg_id;
   SaNameT   lstr_name;
   SaNameT   logFileName;
   SaNameT   logFilePathName;
   SaUint64T maxLogFileSize;
   SaUint32T maxLogRecordSize;
   SaBoolT haProperty;
   SaLogFileFullActionT logFileFullAction;
   SaUint16T maxFilesRotated;
   SaUint16T logFileFmtLength;
   SaUint8T  logFileFmt[LGS_MAX_STRING_LEN];
   SaUint8T  lstr_open_flags;
} LGSV_LGA_LSTR_OPEN_SYNC_PARAM;

typedef struct lgsv_lga_chan_open_async_param_tag
{
   uns32                     reg_id;
   uns8                      chan_open_flags;
   SaNameT                   chan_name;
} LGSV_LGA_CHAN_OPEN_ASYNC_PARAM;

typedef struct lgsv_lga_lstr_close_param_tag
{
   uns32                     reg_id;
   uns32                     lstr_id;
   uns32                     lstr_open_id;
} LGSV_LGA_LSTR_CLOSE_PARAM;

typedef struct lgsv_lga_write_log_async_param_tag
{
   SaInvocationT             invocation;
   uns32                     ack_flags;
   uns32                     reg_id;
   uns32                     lstr_id;
   uns32                     lstr_open_id;
   SaLogRecordT              *logRecord;
} LGSV_LGA_WRITE_LOG_ASYNC_PARAM;

/* API param definition */
typedef struct lgsv_api_info_tag
{
   LGSV_API_TYPE    type;   /* api type */
   union 
   {
      LGSV_LGA_INIT_PARAM                 init;
      LGSV_LGA_FINALIZE_PARAM             finalize;
      LGSV_LGA_LSTR_OPEN_SYNC_PARAM       lstr_open_sync;
      LGSV_LGA_LSTR_CLOSE_PARAM           lstr_close;
      LGSV_LGA_WRITE_LOG_ASYNC_PARAM      write_log_async;
   } param;
} LGSV_API_INFO;


/*** Callback Parameter definitions ***/

typedef struct lgsv_lga_chan_open_cb_param_tag
{
   SaNameT                   chan_name;
   uns32                     chan_id;
   uns32                     chan_open_id;
   uns8                      chan_open_flags;
   uns32                     lga_chan_hdl; /* filled in at the LGA with channelHandle, use 0 at LGS */
   SaAisErrorT                  error;
} LGSV_LGA_CHAN_OPEN_CBK_PARAM;


typedef struct lgsv_lga_write_tag
{
    SaAisErrorT              error;
}LGSV_LGA_WRITE_CKB;

/* wrapper structure for all the callbacks */
typedef struct lgsv_cbk_info
{
   LGSV_CBK_TYPE    type;        /* callback type */
   uns32            lgs_reg_id;  /* lgs reg_id */
   SaInvocationT    inv;         /* invocation value */   
   LGSV_LGA_WRITE_CKB  write_cbk;
} LGSV_CBK_INFO;

/* API Response parameter definitions */
typedef struct lgsv_lga_initialize_rsp_tag
{
   uns32                     reg_id;
} LGSV_LGA_INITIALIZE_RSP;

typedef struct lgsv_lga_lstr_open_sync_rsp_tag
{
   uns32                     lstr_id;
} LGSV_LGA_LSTR_OPEN_SYNC_RSP;

typedef struct lgsv_lga_publish_rsp_tag
{
   void *dummy;
} LGSV_LGA_PUBLISH_RSP;

/* wrapper structure for all API responses 
 */
typedef struct lgsv_api_rsp_info_tag
{
   LGSV_API_RSP_TYPE    type; /* callback type */
   SaAisErrorT             rc;   /* return code */
   union 
   {
      LGSV_LGA_INITIALIZE_RSP         init_rsp;
      LGSV_LGA_LSTR_OPEN_SYNC_RSP     lstr_open_rsp;
   } param;
} LGSV_API_RSP_INFO;

/* wrapper structure for LGA-LGS misc messages 
 */
typedef struct lgsv_lga_misc_info_tag
{
   void *dummy;
} LGSV_LGA_MISC_MSG_INFO;

/* wrapper structure for LGA-LGS misc messages 
 */
typedef struct lgsv_lgs_misc_info_tag
{
   void *dummy;
} LGSV_LGS_MISC_MSG_INFO;

/* message used for LGA-LGS interaction */
typedef struct lgsv_msg
{
   struct lgsv_msg *next;   /* for mailbox processing */
   LGSV_MSG_TYPE    type;   /* message type */
   union 
   {
      /* elements encoded by LGA (& decoded by LGS) */
      LGSV_API_INFO              api_info;       /* api info */
      LGSV_LGA_MISC_MSG_INFO     lgsv_lga_misc_info; 
      
      /* elements encoded by LGS (& decoded by LGA) */
      LGSV_CBK_INFO          cbk_info;       /* callbk info */
      LGSV_API_RSP_INFO      api_resp_info;  /* api response info */
      LGSV_LGS_MISC_MSG_INFO lgsv_lgs_misc_info;
   } info;
} LGSV_MSG;

#define LGSV_WAIT_TIME 500  

#endif /* !LGSV_MSG_H*/




