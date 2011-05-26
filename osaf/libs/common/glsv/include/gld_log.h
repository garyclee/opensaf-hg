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

#ifndef GLD_LOG_H
#define GLD_LOG_H

#define gld_log(severity, format, args...) _gld_log((severity), __FUNCTION__, (format), ##args)
void _gld_log(uint8_t severity, const char *function, const char *format, ...);

/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/
typedef enum gld_hdln_flex {
	GLD_PATRICIA_TREE_INIT_FAILED,
	GLD_PATRICIA_TREE_ADD_FAILED,
	GLD_PATRICIA_TREE_GET_FAILED,
	GLD_PATRICIA_TREE_DEL_FAILED,
	GLD_CREATE_HANDLE_FAILED,
	GLD_TAKE_HANDLE_FAILED,
	GLD_IPC_TASK_INIT,
	GLD_IPC_SEND_FAIL,
	GLD_UNKNOWN_EVT_RCVD,
	GLD_EVT_PROC_FAILED,
	GLD_A2S_EVT_PROC_FAILED,
	GLD_MBCSV_DISPATCH_FAILURE,
	GLD_UNKNOWN_GLND_EVT,
	GLD_HEALTH_KEY_DEFAULT_SET,
	GLD_MSG_FRMT_VER_INVALID,
	GLD_ND_RESTART_WAIT_TMR_EXP,
	GLD_ACTIVE_RMV_NODE
} GLD_HDLN_FLEX;

/******************************************************************************
 Logging offset indexes for AMF logging
 ******************************************************************************/
typedef enum gld_svc_prvdr_flex {
	GLD_AMF_READINESS_CB_OUT_OF_SERVICE,
	GLD_AMF_READINESS_CB_IN_SERVICE,
	GLD_AMF_READINESS_CB_STOPPING,
	GLD_AMF_CSI_SET_HA_STATE_ACTIVE,
	GLD_AMF_CSI_SET_HA_STATE_STANDBY,
	GLD_AMF_CSI_SET_HA_STATE_QUIESCED,
	GLD_AMF_RCVD_CONFIRM_CALL_BACK,
	GLD_AMF_RCVD_TERMINATE_CALLBACK,
	GLD_AMF_RCVD_HEALTHCHK,
	GLD_AMF_INIT_SUCCESS,
	GLD_AMF_INIT_ERROR,
	GLD_AMF_REG_ERROR,
	GLD_AMF_REG_SUCCESS,
	GLD_AMF_SEL_OBJ_GET_ERROR,
	GLD_AMF_DISPATCH_ERROR,
	GLD_AMF_HLTH_CHK_START_DONE,
	GLD_AMF_HLTH_CHK_START_FAIL,
	GLD_MDS_INSTALL_FAIL,
	GLD_MDS_INSTALL_SUCCESS,
	GLD_MDS_UNINSTALL_FAIL,
	GLD_MDS_SUBSCRIBE_FAIL,
	GLD_MDS_SEND_ERROR,
	GLD_MDS_CALL_BACK_ERROR,
	GLD_MDS_UNKNOWN_SVC_EVT,
	GLD_MDS_VDEST_DESTROY_FAIL
} GLD_SVC_PRVDR_FLEX;

/******************************************************************************
 Logging offset indexes for lock operations logging
 ******************************************************************************/
typedef enum gld_lck_oper_flex {
	GLD_OPER_RSC_OPER_ERROR,
	GLD_OPER_RSC_ID_ALLOC_FAIL,
	GLD_OPER_MASTER_RENAME_ERR,
	GLD_OPER_SET_ORPHAN_ERR,
	GLD_OPER_INCORRECT_STATE
} GLD_LCK_OPER_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging 
 ******************************************************************************/
typedef enum gld_memfail_flex {
	GLD_CB_ALLOC_FAILED,
	GLD_NODE_DETAILS_ALLOC_FAILED,
	GLD_EVT_ALLOC_FAILED,
	GLD_RSC_INFO_ALLOC_FAILED,
	GLD_A2S_EVT_ALLOC_FAILED,
	GLD_RES_MASTER_LIST_ALLOC_FAILED
} GLD_MEMFAIL_FLEX;

/******************************************************************************
 Logging offset indexes for API logging 
 ******************************************************************************/
typedef enum gld_api_flex {
	GLD_SE_API_CREATE_SUCCESS,
	GLD_SE_API_CREATE_FAILED,
	GLD_SE_API_DESTROY_SUCCESS,
	GLD_SE_API_DESTROY_FAILED,
	GLD_SE_API_UNKNOWN
} GLD_API_FLEX;

/******************************************************************************
 Logging offset indexes for Event logging 
 ******************************************************************************/
typedef enum gld_evt_flex {
	GLD_EVT_RSC_OPEN,
	GLD_EVT_RSC_CLOSE,
	GLD_EVT_SET_ORPHAN,
	GLD_EVT_MDS_GLND_UP,
	GLD_EVT_MDS_GLND_DOWN,
	GLD_A2S_EVT_RSC_OPEN_SUCCESS,
	GLD_A2S_EVT_RSC_OPEN_FAILED,
	GLD_A2S_EVT_RSC_CLOSE_SUCCESS,
	GLD_A2S_EVT_RSC_CLOSE_FAILED,
	GLD_A2S_EVT_SET_ORPHAN_FAILED,
	GLD_A2S_EVT_SET_ORPHAN_SUCCESS,
	GLD_A2S_EVT_MDS_GLND_UP,
	GLD_A2S_EVT_MDS_GLND_DOWN,
	GLD_A2S_EVT_ADD_NODE_FAILED,
	GLD_A2S_EVT_ADD_RSC_FAILED
} GLD_EVT_FLEX;

typedef enum gld_mbcsv_flex {
	GLD_NCS_MBCSV_SVC_FAILED,
	GLD_MBCSV_INIT_SUCCESS,
	GLD_MBCSV_INIT_FAILED,
	GLD_MBCSV_OPEN_SUCCESS,
	GLD_MBCSV_OPEN_FAILED,
	GLD_MBCSV_CLOSE_FAILED,
	GLD_MBCSV_GET_SEL_OBJ_SUCCESS,
	GLD_MBCSV_GET_SEL_OBJ_FAILURE,
	GLD_MBCSV_CHGROLE_FAILED,
	GLD_MBCSV_FINALIZE_FAILED,
	GLD_MBCSV_CALLBACK_SUCCESS,
	GLD_MBCSV_CALLBACK_FAILED,
	GLD_STANDBY_MSG_PROCESSING,
	GLD_PROCESS_SB_MSG_FAILED,
	GLD_STANDBY_CREATE_EVT,
	GLD_A2S_RSC_OPEN_ASYNC_SUCCESS,
	GLD_A2S_RSC_OPEN_ASYNC_FAILED,
	GLD_A2S_RSC_CLOSE_ASYNC_SUCCESS,
	GLD_A2S_RSC_CLOSE_ASYNC_FAILED,
	GLD_A2S_RSC_SET_ORPHAN_ASYNC_SUCCESS,
	GLD_A2S_RSC_SET_ORPHAN_ASYNC_FAILED,
	GLD_A2S_RSC_NODE_DOWN_ASYNC_SUCCESS,
	GLD_A2S_RSC_NODE_DOWN_ASYNC_FAILED,
	GLD_A2S_RSC_NODE_UP_ASYNC_SUCCESS,
	GLD_A2S_RSC_NODE_UP_ASYNC_FAILED,
	GLD_A2S_RSC_NODE_OPERATIONAL_ASYNC_SUCCESS,
	GLD_A2S_RSC_NODE_OPERATIONAL_ASYNC_FAILED,
	GLD_STANDBY_RSC_OPEN_EVT_FAILED,
	GLD_STANDBY_RSC_CLOSE_EVT_FAILED,
	GLD_STANDBY_RSC_SET_ORPHAN_EVT_FAILED,
	GLD_STANDBY_GLND_DOWN_EVT_FAILED,
	GLD_STANDBY_DESTADD_EVT_FAILED,
	GLD_STANDBY_DESTDEL_EVT_FAILED,
	GLD_ENC_RESERVE_SPACE_FAILED,
	EDU_EXEC_ASYNC_RSC_OPEN_EVT_FAILED,
	EDU_EXEC_ASYNC_RSC_CLOSE_EVT_FAILED,
	EDU_EXEC_ASYNC_SET_ORPHAN_EVT_FAILED,
	EDU_EXEC_ASYNC_GLND_DOWN_EVT_FAILED,
	EDU_EXEC_COLDSYNC_EVT_FAILED
} GLD_MBCSV_FLEX;

/******************************************************************************
 Logging offset indexes for TIMER Failure logging
 ******************************************************************************/
typedef enum gld_timer_log_flex {
	GLD_TIMER_START_FAIL,
	GLD_TIMER_STOP_FAIL
} GLD_TIMER_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum gld_flex_sets {
	GLD_FC_HDLN,
	GLD_FC_MEMFAIL,
	GLD_FC_API,
	GLD_FC_EVT,
	GLD_FC_SVC_PRVDR,
	GLD_FC_LCK_OPER,
	GLD_FC_MBCSV,
	GLD_FC_TIMER,
	GLD_FC_GENLOG
} GLD_FLEX_SETS;

typedef enum gld_log_ids {
	GLD_LID_HDLN,
	GLD_LID_MEMFAIL,
	GLD_LID_API,
	GLD_LID_EVT,
	GLD_LID_SVC_PRVDR,
	GLD_LID_LCK_OPER,
	GLD_LID_MBCSV,
	GLD_LID_TIMER,
	GLD_LID_GENLOG
} GLD_LOG_IDS;

#if (NCS_GLSV_LOG)
void gld_flx_log_reg();
void gld_flx_log_dereg();

void gld_log_headline(uint8_t hdln_id, uint8_t sev, char *file_name, uns32 line_no, SaUint32T node_id);
void gld_log_memfail(uint8_t mf_id, char *file_name, uns32 line_no);
void gld_log_api(uint8_t api_id, uint8_t sev, char *file_name, uns32 line_no);
void gld_log_evt(uint8_t evt_id, uint8_t sev, char *file_name, uns32 line_no, uns32 rsc_id, uns32 node_id);
void gld_log_svc_prvdr(uint8_t sp_id, uint8_t sev, char *file_name, uns32 line_no);
void gld_log_lck_oper(uint8_t lck_id, uint8_t sev, char *file_name, uns32 line_no,
			       char *rsc_name, uns32 rsc_id, uns32 node_id);
void gld_mbcsv_log(uint8_t id, uint8_t sev, char *file_name, uns32 line_no);
void gld_log_timer(uint8_t id, uns32 type, char *file_name, uns32 line_no);

void gld_flx_log_reg(void);

void gld_flx_log_dereg(void);

#define m_LOG_GLD_MBCSV(id,sev,file_name,line_no)             gld_mbcsv_log(id,sev,file_name,line_no)
#define m_LOG_GLD_HEADLINE(id,sev,file_name,line_no,node_id)        gld_log_headline(id,sev,file_name,line_no,node_id)
#define m_LOG_GLD_MEMFAIL(id,file_name,line_no)              gld_log_memfail(id,file_name,line_no)
#define m_LOG_GLD_API(id,sev,file_name,line_no)              gld_log_api(id,sev,file_name,line_no)
#define m_LOG_GLD_EVT(id,sev,file_name,line_no,rsc_id,node_id)      gld_log_evt(id,sev,file_name,line_no,rsc_id,node_id)
#define m_LOG_GLD_SVC_PRVDR(id,sev,file_name,line_no)       gld_log_svc_prvdr(id,sev,file_name,line_no)
#define m_LOG_GLD_LCK_OPER(id,sev,file_name,line_no,name,rsc_id,node_id) gld_log_lck_oper(id,sev,file_name,line_no, \
                                                        name,rsc_id,node_id)
#define m_LOG_GLD_TIMER(id,type,file_name,line_no)           gld_log_timer(id,type,file_name,line_no)
#else
#define m_LOG_GLD_HEADLINE(id,sev,file_name,line_no,node_id)
#define m_LOG_GLD_MEMFAIL(id,file_name,line_no)
#define m_LOG_GLD_API(id,sev,file_name,line_no)
#define m_LOG_GLD_EVT(id,sev,file_name,line_no,rsc_id,node_id)
#define m_LOG_GLD_SVC_PRVDR(id,sev,file_name,line_no)
#define m_LOG_GLD_LCK_OPER(id,sev,file_name,line_no,name,rsc_id,node_id)
#define m_LOG_GLD_MBCSV(id,sev,file_name,line_no)
#define m_LOG_GLD_TIMER(id,type,file_name,line_no)
#endif   /* NCS_GLSV_LOG == 1 */

#endif
