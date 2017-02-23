/*      -*- OpenSAF  -*-
 *
 * Copyright Ericsson AB 2017 - All Rights Reserved.
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

#ifndef LOG_LOGD_LGS_MBCSV_V6_H_
#define LOG_LOGD_LGS_MBCSV_V6_H_

#include "log/logd/lgs.h"
#include "lgs_config.h"
#include "lgs_mbcsv_v2.h"
#include "lgs_mbcsv_v5.h"

typedef struct {
  char *name;
  char *fileName;
  char *pathName;
  SaUint64T maxLogFileSize;
  SaUint32T fixedLogRecordSize;
  SaBoolT haProperty;     /* app log stream only */
  SaLogFileFullActionT logFullAction;
  SaUint32T logFullHaltThreshold; /* !app log stream */
  SaUint32T maxFilesRotated;
  char *logFileFormat;
  SaUint32T severityFilter;
  char *logFileCurrent;
  char *dest_names;
  uint64_t c_file_close_time_stamp; /* Time in sec for file rename on Active */
} lgs_ckpt_stream_cfg_v3_t;

/* Initialize checkpoint record, used in cold/async checkpoint updates */
typedef struct {
  uint32_t client_id;     /* Client Id at Active */
  MDS_DEST mds_dest;      /* Handy when an LGA instance goes away */
  lgs_stream_list_t *stream_list;
  SaVersionT client_ver;  /* Version of client that was initialized */
} lgs_ckpt_initialize_msg_v6_t;

typedef struct {
  lgsv_ckpt_header_t header;
  union {
    lgs_ckpt_initialize_msg_v6_t initialize_client;
    lgs_ckpt_finalize_msg_v2_t finalize_client;
    lgs_ckpt_write_log_v2_t write_log;
    lgs_ckpt_agent_down_v2_t agent_down;
    lgs_ckpt_stream_open_t stream_open;
    lgs_ckpt_stream_close_v2_t stream_close;
    lgs_ckpt_stream_cfg_v3_t stream_cfg;
    lgs_ckpt_lgs_cfg_v5_t lgs_cfg;
  } ckpt_rec;
} lgsv_ckpt_msg_v6_t;

uint32_t edp_ed_cfg_stream_rec_v6(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                  NCSCONTEXT ptr, uint32_t *ptr_data_len,
                                  EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
uint32_t edp_ed_ckpt_msg_v6(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                            NCSCONTEXT ptr, uint32_t *ptr_data_len,
                            EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);

uint32_t edp_ed_reg_rec_v6(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uint32_t *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);

#endif  // LOG_LOGD_LGS_MBCSV_V6_H_
