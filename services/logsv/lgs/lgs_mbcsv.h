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

#ifndef LGS_CKPT_H
#define LGS_CKPT_H

#include "lgs.h"

#define LGS_MBCSV_VERSION 1
#define LGS_MBCSV_VERSION_MIN 1 

/* Checkpoint message types(Used as 'reotype' w.r.t mbcsv)  */

/* Checkpoint update messages are processed similair to lgsv internal
 * messages handling,  so the types are defined such that the values 
 * can be mapped straight with lgsv message types
 */

typedef enum lgsv_ckpt_rec_type
{
  LGS_CKPT_MSG_BASE, 
  LGS_CKPT_INITIALIZE_REC=LGS_CKPT_MSG_BASE,
  LGS_CKPT_FINALIZE_REC,
  LGS_CKPT_AGENT_DOWN,
  LGS_CKPT_LOG_WRITE,
  LGS_CKPT_OPEN_STREAM,
  LGS_CKPT_CLOSE_STREAM,
  LGS_CKPT_MSG_MAX
} LGS_CKPT_DATA_TYPE;

/* Structures for Checkpoint data(to be replicated at the standby) */

/* Registrationlist checkpoint record, used in cold/async checkpoint updates */
typedef struct lgsv_ckpt_reg_msg
{
  uns32               reg_id; /* Registration Id at Active */
  MDS_DEST            lga_client_dest; /* Handy when an LGA instance goes away */
  lgs_stream_list_t   *stream_list;
} LGS_CKPT_REG_MSG;

/* finalize checkpoint record, used in cold/async checkpoint updates */
typedef struct lgsv_ckpt_finalize_msg
{
  uns32       reg_id; /* Registration Id at Active */
} LGS_CKPT_FINALIZE_MSG;

typedef struct lgs_ckpt_write_log
{
  uns32 streamId;
  uns32 recordId;
  uns32 curFileSize;
  char* logFileCurrent;
} LGS_CKPT_WRITE_LOG;

typedef struct lgs_ckpt_stream_open
{
    uns32 streamId;
    uns32 regId;
    /* correspond to SaLogFileCreateAttributes */
    char* logFile;         /* logfile name */
    char* logPath;         /* logfile path */
    uns64 maxFileSize;     /* max file size configurable */
    int32 maxLogRecordSize; /* constant size of the records */
    int32 logFileFullAction;
    int32 maxFilesRotated;        /* max number of rotation files configurable*/     
    char *fileFmt;
    /* end correspond to SaLogFileCreateAttributes */
    char *logStreamName;
    uns64 creationTimeStamp;
    uns32 numOpeners;
    char* logFileCurrent;
    logStreamTypeT streamType;
} LGS_CKPT_STREAM_OPEN;

typedef struct lgs_ckpt_stream_close
{
    uns32 streamId;
    uns32 regId;
} LGS_CKPT_STREAM_CLOSE;

/* Checkpoint message containing lgs data of a particular type.
 * Used during cold and async updates.
 */
typedef struct lgsv_ckpt_header
{
/* SaVersionT          version;  Active peer's Version */
   LGS_CKPT_DATA_TYPE  ckpt_rec_type; /* Type of lgs data carried in this checkpoint */
   uns32               num_ckpt_records; /* =1 for async updates,>=1 for cold sync */
   uns32               data_len; /* Total length of encoded checkpoint data of this type */
} LGS_CKPT_HEADER;

typedef struct lgsv_ckpt_msg
{
   LGS_CKPT_HEADER header;
   union
   {
         LGS_CKPT_REG_MSG                   reg_rec;
         LGS_CKPT_FINALIZE_MSG              finalize_rec;
         LGS_CKPT_WRITE_LOG                 writeLog;  
         MDS_DEST                           agent_dest;
         LGS_CKPT_STREAM_OPEN               stream_open;
         LGS_CKPT_STREAM_CLOSE              stream_close;
   } ckpt_rec;
/*   LGS_CKPT_REC        *next;  This may not be needed */
} LGS_CKPT_DATA;

/* NOTE: All Statistical data should be checkpointed as well. 
 * To maintain count for published messages, an empty CKPT_MSG shall be sent
 * for all LGS_CKPT_PUBLISH_REC types 
 */
typedef struct lgs_ckpt_message
{
   uns32 tot_data_len; /* Combined length of all record types */
   LGS_CKPT_DATA *ckpt_data; /* Multiple instances of ckpt_data for different lgs records */
} LGS_CKPT_MSG;

/* Error codes sent by STANDBY during checkpoint updates */
typedef enum lgs_ckpt_enc_err
{
  LGS_REG_ENC_ERROR=0,
  LGS_OPEN_ENC_ERROR,
  LGS_WRITE_ENC_ERROR,
  LGS_FINALIZE_ENC_ERROR
}LGS_CKPT_ENC_ERROR;

typedef enum lgs_ckpt_dec_err
{
  LGS_REG_DEC_ERROR=0,
  LGS_OPEN_DEC_ERROR,
  LGS_WRITE_DEC_ERROR,
  LGS_FINALIZE_DEC_ERROR,
}LGS_CKPT_DEC_ERROR;

#define m_LGS_FILL_VERSION(rec) { \
 rec.version.releaseCode='A'; \
 rec.version.majorVersion=0x01; \
 rec.versionminorVersion=0x01; \
}

#define m_LGSV_FILL_ASYNC_UPDATE_REG(ckpt,regid,dest) {\
  ckpt.header.ckpt_rec_type=LGS_CKPT_INITIALIZE_REC; \
  ckpt.header.num_ckpt_records=1; \
  ckpt.header.data_len=1; \
  ckpt.ckpt_rec.reg_rec.reg_id = regid;\
  ckpt.ckpt_rec.reg_rec.lga_client_dest = dest;\
}


#define m_LGSV_FILL_ASYNC_UPDATE_FINALIZE(ckpt,regid){ \
  ckpt.header.ckpt_rec_type=LGS_CKPT_FINALIZE_REC; \
  ckpt.header.num_ckpt_records=1; \
  ckpt.header.data_len=1; \
  ckpt.ckpt_rec.finalize_rec.reg_id= regid;\
}

#define m_LGSV_FILL_ASYNC_UPDATE_AGENT_DOWN(ckpt,dest) {\
  ckpt.header.ckpt_rec_type=LGS_CKPT_AGENT_DOWN; \
  ckpt.header.num_ckpt_records=1;\
  ckpt.header.data_len=1;\
  ckpt.ckpt_rec.agent_dest=dest;\
}

typedef uns32 (*LGS_CKPT_HDLR)(lgs_cb_t *cb, LGS_CKPT_DATA *data);
uns32 lgs_mbcsv_init(lgs_cb_t *lgs_cb);
uns32 lgs_mbcsv_change_HA_state(lgs_cb_t *cb);
uns32 lgs_mbcsv_dispatch(NCS_MBCSV_HDL mbcsv_hdl);

uns32 lgs_mbcsv_callback(NCS_MBCSV_CB_ARG *arg); /* Common Callback interface to mbcsv */
uns32 lgs_ckpt_encode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg);
uns32 lgs_ckpt_enc_cold_sync_data(lgs_cb_t *lgs_cb, NCS_MBCSV_CB_ARG *cbk_arg,NCS_BOOL data_req);
uns32 lgs_edu_enc_reg_list(lgs_cb_t *cb, NCS_UBAID *uba);
uns32 lgs_edu_enc_streams(lgs_cb_t *cb, NCS_UBAID *uba);

uns32 lgs_ckpt_encode_async_update(lgs_cb_t *lgs_cb,EDU_HDL edu_hdl,NCS_MBCSV_CB_ARG *cbk_arg);
uns32 lgs_ckpt_decode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg);
uns32 lgs_ckpt_decode_async_update(lgs_cb_t *cb,NCS_MBCSV_CB_ARG *cbk_arg);
uns32 lgs_ckpt_decode_cold_sync(lgs_cb_t *cb,NCS_MBCSV_CB_ARG *cbk_arg);
uns32 lgs_process_ckpt_data(lgs_cb_t *cb, LGS_CKPT_DATA *data);
uns32 lgs_ckpt_proc_reg_rec(lgs_cb_t *cb, LGS_CKPT_DATA *data);
uns32 lgs_ckpt_proc_finalize_rec(lgs_cb_t* cb, LGS_CKPT_DATA *data);
uns32 lgs_ckpt_proc_agent_down_rec(lgs_cb_t* cb, LGS_CKPT_DATA *data);
uns32 lgs_ckpt_proc_log_write(lgs_cb_t* cb, LGS_CKPT_DATA *data);
uns32 lgs_ckpt_proc_open_stream(lgs_cb_t* cb, LGS_CKPT_DATA *data);
uns32 lgs_send_async_update(lgs_cb_t *cb,LGS_CKPT_DATA *ckpt_rec,uns32 action);
uns32 lgsv_ckpt_send_cold_sync(lgs_cb_t *lgs_cb);
uns32 lgs_ckpt_send_async_update(lgs_cb_t *lgs_cb);
uns32 lgs_ckpt_peer_info_cbk_handler(NCS_MBCSV_CB_ARG *arg);
uns32 lgs_ckpt_notify_cbk_handler (NCS_MBCSV_CB_ARG *arg);
uns32 lgs_ckpt_err_ind_cbk_handler (NCS_MBCSV_CB_ARG *arg);
uns32 lgs_edp_ed_ckpt_hdr(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                  EDU_ERR *o_err);
uns32 lgs_edp_ed_stream_list(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,    
                                 NCSCONTEXT ptr, uns32 *ptr_data_len,
                                 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                 EDU_ERR *o_err);
uns32 lgs_edp_ed_reg_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                  EDU_ERR *o_err);
uns32 lgs_edp_ed_write_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                  EDU_ERR *o_err);
uns32 lgs_edp_ed_open_stream_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                  EDU_ERR *o_err);

uns32 lgs_edp_ed_finalize_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                  EDU_ERR *o_err);
uns32 lgs_edp_ed_header_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                  EDU_ERR *o_err);
int32 lgs_ckpt_msg_test_type(NCSCONTEXT arg);
uns32 lgs_edp_ed_ckpt_msg(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uns32 *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                  EDU_ERR *o_err);
void lgs_enc_ckpt_header(uns8 *pdata,LGS_CKPT_HEADER header);

uns32 lgs_dec_ckpt_header(NCS_UBAID *uba, LGS_CKPT_HEADER *header);

#endif /* !LGSV_CKPT_H*/
