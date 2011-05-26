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

#ifndef EDS_LOG_H
#define EDS_LOG_H

/******************************************************************************
 Logging offset indexes for EDS Headline logging
 ******************************************************************************/
typedef enum eds_hdln_log_flex {
	EDS_API_MSG_DISPATCH_FAILED,
	EDS_CB_CREATE_FAILED,
	EDS_CB_CREATE_HANDLE_FAILED,
	EDS_CB_DESTROY_FAILED,
	EDS_CB_INIT_FAILED,
	EDS_MBCSV_INIT_FAILED,
	EDS_INSTALL_SIGHDLR_FAILED,
	EDS_TABLE_REGISTER_FAILED,
	EDS_CB_TAKE_HANDLE_FAILED,
	EDS_CB_TAKE_RETD_EVT_HDL_FAILED,
	EDS_RET_EVT_HANDLE_CREATE_FAILED,
	EDS_EVENT_PROCESSING_FAILED,
	EDS_TASK_CREATE_FAILED,
	EDS_TASK_START_FAILED,
	EDS_REG_LIST_ADD_FAILED,
	EDS_REG_LIST_DEL_FAILED,
	EDS_REG_LIST_GET_FAILED,
	EDS_VERSION_INCOMPATIBLE,
	EDS_AMF_DISPATCH_FAILURE,
	EDS_AMF_INIT_FAILED,
	EDS_AMF_REG_FAILED,
	EDS_AMF_DESTROY_FAILED,
	EDS_AMF_RESPONSE_FAILED,
	EDS_IPC_CREATE_FAILED,
	EDS_IPC_ATTACH_FAILED,
	EDS_MDS_CALLBACK_PROCESS_FAILED,
	EDS_MDS_GET_HANDLE_FAILED,
	EDS_MDS_INIT_FAILED,
	EDS_MDS_UNINSTALL_FAILED,
	EDS_MDS_VDEST_DESTROY_FAILED,

	/* Memory allocation Failure */
	EDS_MEM_ALLOC_FAILED,

	EDS_AMF_REMOVE_CALLBACK_CALLED,
	EDS_AMF_TERMINATE_CALLBACK_CALLED,
	EDS_CB_INIT_SUCCESS,
	EDS_MAIL_BOX_CREATE_ATTACH_SUCCESS,
	EDS_MDS_VDEST_CREATE_SUCCESS,
	EDS_MDS_VDEST_CREATE_FAILED,
	EDS_MDS_INSTALL_SUCCESS,
	EDS_MDS_INSTALL_FAILED,
	EDS_MDS_SUBSCRIBE_SUCCESS,
	EDS_MDS_SUBSCRIBE_FAILED,
	EDS_MDS_INIT_ROLE_CHANGE_SUCCESS,
	EDS_MDS_INIT_ROLE_CHANGE_FAILED,
	EDS_MDS_INIT_SUCCESS,
	EDS_MBCSV_INIT_SUCCESS,
	EDS_INSTALL_SIGHDLR_SUCCESS,
	EDS_MAIN_PROCESS_START_SUCCESS,
	EDS_TBL_REGISTER_SUCCESS,
	EDS_GOT_SIGUSR1_SIGNAL,
	EDS_AMF_READINESS_CB_OUT_OF_SERVICE,
	EDS_AMF_READINESS_CB_IN_SERVICE,
	EDS_AMF_READINESS_CB_STOPPING,
	EDS_AMF_RCVD_CSI_SET_CLBK,
	EDS_AMF_CSI_SET_HA_STATE_INVALID,
	EDS_MDS_CSI_ROLE_CHANGE_SUCCESS,
	EDS_MDS_CSI_ROLE_CHANGE_FAILED,
	EDS_AMF_RCVD_CONFIRM_CALL_BACK,
	EDS_AMF_RCVD_HEALTHCHK,
	EDS_AMF_INIT_SUCCESS,
	EDS_AMF_INIT_ERROR,
	EDS_AMF_GET_SEL_OBJ_SUCCESS,
	EDS_AMF_GET_SEL_OBJ_FAILURE,
	EDS_AMF_ENV_NAME_SET_FAIL,
	EDS_AMF_COMP_NAME_GET_SUCCESS,
	EDS_AMF_COMP_NAME_GET_FAIL,
	EDS_AMF_INIT_OK,
	EDS_AMF_COMP_REGISTER_SUCCESS,
	EDS_AMF_COMP_REGISTER_FAILED,
	EDS_AMF_REG_SUCCESS,
	EDS_AMF_DISPATCH_ERROR,
	EDS_AMF_HLTH_CHK_START_DONE,
	EDS_AMF_HLTH_CHK_START_FAIL,

	EDS_MBCSV_SUCCESS,
	EDS_MBCSV_FAILURE,

	EDS_MDS_SUCCESS,
	EDS_MDS_FAILURE,
	EDS_TIMER_START_FAIL,
	EDS_TIMER_STOP_FAIL,
	EDS_LL_PROCESING_FAILURE,
	EDS_LL_PROCESING_SUCCESS,
	EDS_INIT_FAILURE,
	EDS_INIT_SUCCESS,
	EDS_FINALIZE_FAILURE,
	EDS_FINALIZE_SUCCESS,
	EDS_CHN_OPEN_SYNC_FAILURE,
	EDS_CHN_OPEN_SYNC_SUCCESS,
	EDS_CHN_OPEN_ASYNC_FAILURE,
	EDS_CHN_OPEN_ASYNC_SUCCESS,
	EDS_CHN_CLOSE_FAILURE,
	EDS_CHN_CLOSE_SUCCESS,
	EDS_CHN_UNLINK_FAILURE,
	EDS_CHN_UNLINK_SUCCESS,
	EDS_PUBLISH_FAILURE,
	EDS_PUBLISH_LOST_EVENT,
	EDS_SUBSCRIBE_FAILURE,
	EDS_SUBSCRIBE_LOST_EVENT,
	EDS_UNSUBSCRIBE_FAILURE,
	EDS_UNSUBSCRIBE_SUCCESS,
	EDS_RETENTION_TMR_CLR_FAILURE,
	EDS_RETENTION_TMR_CLR_SUCCESS,
	EDS_RETENTION_TMR_EXP_FAILURE,
	EDS_EVT_UNKNOWN,
	EDS_REMOVE_CALLBACK_CALLED,
	EDS_CLM_INIT_FAILED,
	EDS_CLM_REGISTRATION_FAILED,
	EDS_CLM_REGISTRATION_SUCCESS,
	EDS_CLM_SEL_OBJ_GET_FAILED,
	EDS_CLM_NODE_GET_FAILED,
	EDS_CLM_CLUSTER_TRACK_FAILED,
	EDS_CLM_CLUSTER_TRACK_CBK_SUCCESS,
	EDS_CLM_CLUSTER_TRACK_CBK_FAILED,
	EDS_CLUSTER_CHANGE_NOTIFY_SEND_FAILED,

} EDS_HDLN_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for Event logging
 ******************************************************************************/
typedef enum eds_event_log_flex {
	EDS_EVENT_HDR_LOG
} EDS_EVENT_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum eds_flex_sets {
	EDS_FC_HDLN,
	EDS_FC_HDLNF,
	EDS_FC_EVENT,
} EDS_FLEX_SETS;

typedef enum eds_log_ids {
	EDS_LID_HDLN,
	EDS_LID_HDLNF,
	EDS_LID_EVENT,
} EDS_LOG_IDS;

/*****************************************************************************
                          Macros for Logging
*****************************************************************************/

#define m_LOG_EDS_EVENT(id,pubname,evt_id,pubtime,pri,rettime) eds_log_event(id,pubname,evt_id,pubtime,pri,rettime)
#define m_LOG_EDSV_S(id,category,sev,rc,fname,fno,data) eds_log(id,category,sev,rc,fname,fno,data)
#define m_LOG_EDSV_SF(id,category,sev,rc,fname,fno,data,dest) eds_log_f(id,category,sev,rc,fname,fno,data,dest)

/*****************************************************************************
                       Extern Function Declarations
*****************************************************************************/

#if (NCS_EDSV_LOG == 1)

void eds_flx_log_reg(void);
void eds_flx_log_dereg(void);
void eds_log(uint8_t id, uint32_t category, uint8_t sev, long rc, char *fname, uint32_t fno, uint32_t data);
void eds_log_f(uint8_t id, uint32_t category, uint8_t sev, uint32_t rc, char *fname, uint32_t fno, uint32_t data, uns64 dest);
void eds_log_event(uint8_t id, int8_t *pub_name, uint32_t evt_id, uint32_t pubtime, uint32_t pri, uint32_t rettime);
void eds_log_lost_event(uint8_t id, int8_t *pub_name, uint32_t evt_id, uint32_t pubtime, uint32_t pri);
#endif   /* ((NCS_DTA == 1) && (NCS_EDSV_LOG == 1)) */

#endif
