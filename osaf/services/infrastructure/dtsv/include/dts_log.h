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

  DESCRIPTION:

  This module contains logging/tracing functions.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef DTS_LOG_H
#define DTS_LOG_H

#include <ncsgl_defs.h>

/*******************************************************************************************/

/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/

typedef enum dts_hdln_flex {
	DTS_HDLN_DTS_CREATE_FAILED,
	DTS_HDLN_DTS_CREATE_SUCCESS,
	DTS_HDLN_DTS_DESTROY_FAILED,
	DTS_HDLN_DTS_DESTROY_SUCCESS,
	DTS_HDLN_NULL_INST,
	DTS_HDLN_RECEIVED_CLI_REQ,
	DTS_HDLN_CLI_INV_TBL
} DTS_HDLN_FLEX;

/******************************************************************************
 Logging offset indexes for Service Provider (MDS) logging
 ******************************************************************************/

typedef enum dts_svc_prvdr_flex {
	DTS_SP_MDS_INSTALL_FAILED,
	DTS_SP_MDS_INSTALL_SUCCESS,
	DTS_SP_MDS_UNINSTALL_FAILED,
	DTS_SP_MDS_UNINSTALL_SUCCESS,
	DTS_SP_MDS_SUBSCR_FAILED,
	DTS_SP_MDS_SUBSCR_SUCCESS,
	DTS_SP_MDS_SND_MSG_SUCCESS,
	DTS_SP_MDS_SND_MSG_FAILED,
	DTS_SP_MDS_RCV_MSG,
	DTS_SP_MDS_RCV_EVT,

} DTS_SVC_PRVDR_FLEX;

/******************************************************************************
 Logging offset indexes for Lock logging 
 ******************************************************************************/

typedef enum dts_locks_flex {
	DTS_LK_LOCKED,
	DTS_LK_UNLOCKED,
	DTS_LK_CREATED,
	DTS_LK_DELETED,

} DTS_LOCKS_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging 
 ******************************************************************************/

typedef enum dts_memfail_flex {
	DTS_MF_DTSMSG_CREATE,
	DTS_MF_DTSTBL_CREATE,
	DTS_MF_DTS_REGTBL_CREATE,
	DTS_MF_DTS_VCD_TBL_CREATE
} DTS_MEMFAIL_FLEX;

/******************************************************************************
 Logging offset indexes for API logging 
 ******************************************************************************/

typedef enum dts_api_flex {
	DTS_API_SVC_CREATE,
	DTS_API_SVC_DESTROY,
	DTS_AMF_INIT_SUCCESS,
	DTS_FAIL_OVER_SUCCESS,
	DTS_FAIL_OVER_FAILED,
	DTS_AMF_ROLE_CHG,
	DTS_AMF_ROLE_FAIL,
	DTS_REG_TBL_CLEAR,
	DTS_AMF_FINALIZE,
	DTS_CSI_SET_CB_RESP,
	DTS_AMF_HEALTH_CHECK,
	DTS_CSI_RMV_CB_RESP,
	DTS_CSI_TERM_CB,
	DTS_AMF_UP_SIG,
	DTS_MBCSV_REG,
	DTS_OPEN_CKPT,
	DTS_MBCSV_DEREG,
	DTS_CLOSE_CKPT,
	DTS_MBCSV_FIN,
	DTS_MDS_ROLE_CHG_FAIL,
	DTS_CKPT_CHG_FAIL,
	DTS_INIT_ROLE_ACTIVE,
	DTS_INIT_ROLE_STANDBY,
	DTS_SPEC_RELOAD_CMD,
	DTS_LOG_DEL_FAIL
} DTS_API_FLEX;

/******************************************************************************
 Logging offset indexes for Event logging 
 ******************************************************************************/

typedef enum dts_event_flex {
	DTS_EV_SVC_REG_REQ_RCV,
	DTS_EV_SVC_DE_REG_REQ_RCV,
	DTS_EV_SVC_REG_SUCCESSFUL,
	DTS_EV_SVC_REG_FAILED,
	DTS_EV_SVC_DEREG_SUCCESSFUL,
	DTS_EV_SVC_DEREG_FAILED,
	DTS_EV_SVC_ALREADY_REG,
	DTS_EV_SVC_REG_ENT_ADD,
	DTS_EV_SVC_REG_ENT_ADD_FAIL,
	DTS_EV_SVC_REG_ENT_RMVD,
	DTS_EV_SVC_REG_ENT_UPDT,
	DTS_EV_LOG_SVC_KEY_WRONG,
	DTS_EV_DTA_DOWN,
	DTS_EV_DTA_UP,
	DTS_EV_DTA_DEST_ADD_SUCC,
	DTS_EV_DTA_DEST_RMV_SUCC,
	DTS_EV_DTA_DEST_ADD_FAIL,
	DTS_EV_DTA_DEST_RMV_FAIL,
	DTS_EV_DTA_DEST_PRESENT,
	DTS_EV_SVC_DTA_ADD,
	DTS_EV_SVC_DTA_RMV,
	DTS_EV_DTA_SVC_ADD,
	DTS_EV_DTA_SVC_RMV,
	DTS_EV_DTA_SVC_RMV_FAIL,
	DTS_EV_SVC_DTA_RMV_FAIL,
	DTS_EV_SVC_REG_NOTFOUND,
	DTS_MDS_VDEST_CHG,
	DTS_SET_FAIL,
	DTS_LOG_DEV_NONE,
	DTS_SPEC_SVC_NAME_NULL,
	DTS_SPEC_SVC_NAME_ERR,
	DTS_SPEC_SET_ERR,
	DTS_SPEC_STR_ERR,
	DTS_SPEC_FMAT_ERR,
	DTS_SPEC_REG_SUCC,
	DTS_SPEC_REG_FAIL,
	DTS_LOG_SETIDX_ERR,
	DTS_LOG_STRIDX_ERR,
	DTS_SPEC_NOT_REG,
	DTS_INVALID_FMAT,
	DTS_FMAT_ID_MISMATCH,
	DTS_FMAT_TYPE_MISMATCH,
	DTS_LOG_CONV_FAIL
} DTS_EVENT_FLEX;

/******************************************************************************
 Logging offset indexes for checkpointing operations 
 ******************************************************************************/

typedef enum dts_chkop_flex {
	DTS_CSYNC_ENC_START,
	DTS_CSYNC_ENC_FAILED,
	DTS_CSYNC_ENC_COMPLETE,
	DTS_CSYNC_ENC_SVC_REG,
	DTS_CSYNC_ENC_DTA_DEST,
	DTS_CSYNC_ENC_GLOBAL_POLICY,
	DTS_CSYNC_ENC_UPDT_COUNT,
	DTS_CSYNC_DEC_START,
	DTS_CSYNC_DEC_FAILED,
	DTS_CSYNC_DEC_COMPLETE,
	DTS_CSYNC_DEC_SVC_REG,
	DTS_CSYNC_DEC_DTA_DEST,
	DTS_CSYNC_DEC_GLOBAL_POLICY,
	DTS_CSYNC_DEC_UPDT_COUNT,
	DTS_WSYNC_ENC_START,
	DTS_WSYNC_ENC_FAILED,
	DTS_WSYNC_ENC_COMPLETE,
	DTS_WSYNC_DEC_START,
	DTS_WSYNC_DEC_FAILED,
	DTS_WSYNC_DEC_COMPLETE,
	DTS_WSYNC_FAILED,
	DTS_WSYNC_DATA_MISMATCH,
	DTS_ASYNC_SVC_REG,
	DTS_ASYNC_DTA_DEST,
	DTS_ASYNC_GLOBAL_POLICY,
	DTS_ASYNC_LOG_FILE,
	DTS_ASYNC_FAILED,
	DTS_COLD_SYNC_TIMER_EXP,
	DTS_COLD_SYNC_CMPLT_EXP,
	DTS_WARM_SYNC_TIMER_EXP,
	DTS_WARM_SYNC_CMPLT_EXP,
	DTS_DEC_DATA_RESP,
	DTS_DEC_DATA_REQ,
	DTS_ENC_DATA_RESP,
	DTS_DATA_RESP_CMPLT_EXP,
	DTS_DATA_RESP_TERM,
	DTS_ASYNC_CNT_MISMATCH,
	DTS_ACT_ASSIGN_NOT_INSYNC,
	DTS_CSYNC_IN_CSYNC,
	DTS_CSYNC_REQ_ENC,
	DTS_DATA_SYNC_CMPLT
} DTS_CHKOP_FLEX;

/******************************************************************************
 Logging offset indexes for Circular buffer operation logging 
 ******************************************************************************/

typedef enum dts_cbop_flex {
	DTS_CBOP_ALLOCATED,
	DTS_CBOP_FREED,
	DTS_CBOP_CLEARED,
	DTS_CBOP_DUMPED
} DTS_CBOP_FLEX;

/******************************************************************************
 Logging offset indexes for Debug String logging 
 ******************************************************************************/

typedef enum dts_dbg_flex {
	DTS_GLOBAL,
	DTS_NODE,
	DTS_SERVICE,
	DTS_IGNORE
} DTS_DBG_FLEX;
/******************************************************************************/

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum dts_flex_sets {
	DTS_FC_HDLN,
	DTS_FC_SVC_PRVDR_FLEX,
	DTS_FC_LOCKS,
	DTS_FC_MEMFAIL,
	DTS_FC_API,
	DTS_FC_EVT,
	DTS_FC_CIRBUFF,
	DTS_FC_STR,
	DTS_FC_UPDT,
	DTS_FC_GENLOG
} DTS_FLEX_SETS;

typedef enum dts_log_ids {
	DTS_LID_HDLN,
	DTS_LID_SVC_PRVDR_FLEX,
	DTS_LID_LOCKS,
	DTS_LID_MEMFAIL,
	DTS_LID_API,
	DTS_LID_EVT,
	DTS_LID_CB_LOG,
	DTS_LID_STR,
	DTS_LID_STRL,
	DTS_LID_STRL_SVC,
	DTS_LID_STRL_SVC_NAME,
	DTS_LID_STRLL,
	DTS_LID_LFILE,
	DTS_LID_LOGDEL,
	DTS_LID_CHKP,
	DTS_LID_MDS_EVT,
	DTS_LID_AMF_EVT,
	DTS_LID_SPEC_ERR_EVT,
	DTS_LID_SPEC_REG,
	DTS_LID_LOG_ERR,
	DTS_LID_LOG_ERR1,
	DTS_LID_LOG_ERR2,
	DTS_LID_WSYNC_ERR,
	DTS_LID_ASYNC_UPDT,
	DTS_LID_FLOW_UP,
	DTS_LID_FLOW_DOWN,
	DTS_LID_GENLOG
} DTS_LOG_IDS;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                          DTS Logging Control

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#if (DTS_LOG == 1)

uns32 dts_reg_strings(void);
uns32 dts_dereg_strings(void);
uns32 dts_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
uns32 dts_log_bind(void);
uns32 dts_log_unbind(void);

void log_dts_headline(uint8_t hdln_id);
void log_dts_svc_prvdr(uint8_t sp_id);
void log_dts_lock(uint8_t lck_id, void *lck);
void log_dts_memfail(uint8_t mf_id);
void log_dts_api(uint8_t api_id);
void log_dts_evt(uint8_t evt_id, SS_SVC_ID svc_id, uns32 node, uns32 adest);
void log_dts_cbop(uint8_t op_id, SS_SVC_ID svc_id, uns32 node);
void log_dts_dbg(uint8_t id, char *str, NODE_ID node, SS_SVC_ID svc);
void log_dts_dbg_name(uint8_t id, char *str, uns32 svc_id, char *svc);
void log_dts_chkp_evt(uint8_t id);

#define dts_log(severity, format, args...) _dts_log((severity), __FUNCTION__, (format), ##args)
void _dts_log(uint8_t severity, const char *function, const char *format, ...);

#define m_LOG_DTS_HEADLINE(id)             log_dts_headline (id    )
#define m_LOG_DTS_SVC_PRVDR(id)            log_dts_svc_prvdr(id    )
#define m_LOG_DTS_LOCK(id, lck)            log_dts_lock     (id,lck)
#define m_LOG_DTS_MEMFAIL(id)              log_dts_memfail  (id    )
#define m_LOG_DTS_API(id)                  log_dts_api      (id    )
#define m_LOG_DTS_EVT(id, sid, node, ad)   log_dts_evt      (id, sid, node, ad)
#define m_LOG_DTS_CBOP(id, sid, node)      log_dts_cbop     (id, sid, node)
#define m_LOG_DTS_CHKOP(id)                log_dts_chkp_evt (id)
#define m_LOG_DTS_DBGSTR(id, s, nd, sv)    log_dts_dbg      (id, s, nd, sv)
/* Added new macro for new function to print service name */
#define m_LOG_DTS_DBGSTR_NAME(id, s, nd, sv)  log_dts_dbg_name  (id, s, nd, sv)
#define m_LOG_DTS_DBGSTRL(id, s, f, l)     ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STRL, DTS_FC_STR, NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TICCL", id, s, f, l);
#define m_LOG_DTS_DBGSTRL_SVC(id, s, f, l, svc)  ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STRL_SVC, DTS_FC_STR, NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TICCLL", id, s, f, l, svc);
/* Added a new macro */
#define m_LOG_DTS_DBGSTRL_SVC_NAME(id, s, f, l, svc)  ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STRL_SVC_NAME, DTS_FC_STR, NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TICCLC", id, s, f, l, svc);
#define m_LOG_DTS_DBGSTRLL(id, s, l1, l2)  ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STRLL, DTS_FC_STR, NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TICLL", id, s, l1, l2);
#define m_LOG_DTS_LFILE(s, node, svc)             ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LFILE, DTS_FC_STR, NCSFL_LC_MISC, NCSFL_SEV_NOTICE, "TCLL", s, node, svc);
#define m_LOG_DTS_LOGDEL(node, svc, count)               ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOGDEL, DTS_FC_STR, NCSFL_LC_MISC, NCSFL_SEV_NOTICE, "TLLL", node, svc, count);
#define m_LOG_DTS_SVCREG_ADD_FAIL(id, sid, node, ad) ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_ERROR, "TILLL", id, sid, node, ad);
#else

uns32 dts_reg_strings(char *fname);

#define m_LOG_DTS_HEADLINE(id)
#define m_LOG_DTS_SVC_PRVDR(id)
#define m_LOG_DTS_LOCK(id, lck)
#define m_LOG_DTS_MEMFAIL(id)
#define m_LOG_DTS_API(id)
#define m_LOG_DTS_EVT(id, sid, node, ad)
#define m_LOG_DTS_CBOP(id, sid, node)
#define m_LOG_DTS_CHKOP(id)
#define m_LOG_DTS_DBGSTR(id, s, nd, sv)
#define m_LOG_DTS_DBGSTRL(id, s, f, l)
#define m_LOG_DTS_DBGSTRL_SVC(id, s, f, l, svc)
#define m_LOG_DTS_DBGSTRLL(id, s, l1, l2)
#define m_LOG_DTS_LFILE(id, s)
#endif   /* DTS_LOG == 1 */

#endif   /* DTS_LOG_H */
