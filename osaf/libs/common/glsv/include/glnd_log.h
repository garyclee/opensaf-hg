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

#ifndef GLND_LOG_H
#define GLND_LOG_H

/******************************************************************************
 Logging offset indexes for GLND Headline logging
 ******************************************************************************/
typedef enum glnd_hdln_log_flex {
	GLND_CB_CREATE_FAILED,
	GLND_CB_CREATE_SUCCESS,
	GLND_TASK_CREATE_FAILED,
	GLND_TASK_CREATE_SUCCESS,
	GLND_TASK_START_FAILED,
	GLND_TASK_START_SUCCESS,
	GLND_CB_DESTROY_FAILED,
	GLND_CB_TAKE_HANDLE_FAILED,
	GLND_AMF_GET_SEL_OBJ_FAILURE,
	GLND_AMF_DISPATCH_FAILURE,
	GLND_CLIENT_TREE_INIT_FAILED,
	GLND_AGENT_TREE_INIT_FAILED,
	GLND_RSC_TREE_INIT_FAILED,
	GLND_IPC_CREATE_FAILED,
	GLND_IPC_ATTACH_FAILED,
	GLND_MDS_REGISTER_FAILED,
	GLND_MDS_REGISTER_SUCCESS,
	GLND_AMF_INIT_FAILED,
	GLND_AMF_INIT_SUCCESS,
	GLND_AMF_DESTROY_FAILED,
	GLND_AMF_REGISTER_FAILED,
	GLND_AMF_REGISTER_SUCCESS,
	GLND_AMF_RESPONSE_FAILED,
	GLND_MDS_GET_HANDLE_FAILED,
	GLND_MDS_UNREGISTER_FAILED,
	GLND_MDS_CALLBACK_PROCESS_FAILED,
	GLND_AMF_HEALTHCHECK_START_FAILED,
	GLND_AMF_HEALTHCHECK_START_SUCCESS,
	GLND_RSC_REQ_CREATE_HANDLE_FAILED,
	GLND_RSC_NODE_ADD_SUCCESS,
	GLND_RSC_NODE_DESTROY_SUCCESS,
	GLND_RSC_LOCK_REQ_DESTROY,
	GLND_RSC_LOCK_GRANTED,
	GLND_RSC_LOCK_QUEUED,
	GLND_RSC_UNLOCK_SUCCESS,
	GLND_NEW_MASTER_RSC,
	GLND_MASTER_ELECTION_RSC,
	GLND_MSG_FRMT_VER_INVALID,
	GLND_DEC_FAIL,
	GLND_ENC_FAIL
} GLND_HDLN_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging 
 ******************************************************************************/
typedef enum glnd_memfail_log_flex {
	GLND_CB_ALLOC_FAILED,
	GLND_CLIENT_ALLOC_FAILED,
	GLND_RSC_REQ_LIST_ALLOC_FAILED,
	GLND_AGENT_ALLOC_FAILED,
	GLND_CLIENT_RSC_LIST_ALLOC_FAILED,
	GLND_CLIENT_RSC_LOCK_LIST_ALLOC_FAILED,
	GLND_EVT_ALLOC_FAILED,
	GLND_RSC_NODE_ALLOC_FAILED,
	GLND_RESTART_RSC_NODE_ALLOC_FAILED,
	GLND_RESTART_RSC_INFO_ALLOC_FAILED,
	GLND_RESTART_BACKUP_EVENT_ALLOC_FAILED,
	GLND_RSC_LOCK_LIST_ALLOC_FAILED,
	GLND_RESTART_RSC_LOCK_LIST_ALLOC_FAILED,
	GLND_LOCK_LIST_ALLOC_FAILED,
	GLND_IO_VECTOR_ALLOC_FAILED
} GLND_MEMFAIL_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for LOCK API logging
 ******************************************************************************/
typedef enum glnd_api_log_flex {
	GLND_AGENT_TREE_ADD_FAILED,
	GLND_AGENT_TREE_DEL_FAILED,
	GLND_AGENT_NODE_FIND_FAILED,
	GLND_CLIENT_TREE_ADD_FAILED,
	GLND_CLIENT_TREE_DEL_FAILED,
	GLND_CLIENT_NODE_FIND_FAILED,
	GLND_RSC_REQ_NODE_ADD_FAILED,
	GLND_RSC_NODE_FIND_FAILED,
	GLND_RSC_NODE_ADD_FAILED,
	GLND_RSC_NODE_DESTROY_FAILED,
	GLND_RSC_LOCAL_LOCK_REQ_FIND_FAILED,
	GLND_RSC_GRANT_LOCK_REQ_FIND_FAILED,
	GLND_AGENT_NODE_NOT_FOUND,
} GLND_API_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for Event logging 
 ******************************************************************************/
typedef enum glnd_evt_log_flex {
	GLND_EVT_UNKNOWN,
	GLND_EVT_PROCESS_FAILURE,
	GLND_EVT_RECIEVED,
} GLND_EVT_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for system call fail logging 
 ******************************************************************************/
typedef enum glnd_data_send_log_flex {
	GLND_MDS_SEND_FAILURE,
	GLND_MDS_GLD_DOWN,
} GLND_DATA_SEND_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for TIMER Failure logging
 ******************************************************************************/
typedef enum glnd_timer_log_flex {
	GLND_TIMER_START_FAIL,
	GLND_TIMER_STOP_FAIL
} GLND_TIMER_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for GLND logging
 ******************************************************************************/
typedef enum glnd_log_flex {
	GLND_COMING_UP_FIRST_TIME,
	GLND_RESTARTED,
	GLND_SHM_CREATE_FAILURE,
	GLND_SHM_CREATE_SUCCESS,
	GLND_SHM_OPEN_FAILURE,
	GLND_SHM_OPEN_SUCCESS,
	GLND_RESOURCE_SHM_WRITE_FAILURE,
	GLND_RESOURCE_SHM_READ_FAILURE,
	GLND_LCK_LIST_SHM_WRITE_FAILURE,
	GLND_LCK_LIST_SHM_READ_FAILURE,
	GLND_EVT_LIST_SHM_WRITE_FAILURE,
	GLND_EVT_LIST_SHM_READ_FAILURE,
	GLND_RESTART_BUILD_DATABASE_SUCCESS,
	GLND_RESTART_BUILD_DATABASE_FAILURE,
	GLND_RESTART_RESOURCE_TREE_BUILD_SUCCESS,
	GLND_RESTART_RESOURCE_TREE_BUILD_FAILURE,
	GLND_RESTART_LCK_LIST_BUILD_SUCCESS,
	GLND_RESTART_LCK_LIST_BUILD_FAILURE,
	GLND_RESTART_EVT_LIST_BUILD_SUCCESS,
	GLND_RESTART_EVT_LIST_BUILD_FAILURE
} GLND_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum glnd_flex_sets {
	GLND_FC_HDLN,
	GLND_FC_HDLN_TIL,
	GLND_FC_HDLN_TILL,
	GLND_FC_HDLN_TILLL,
	GLND_FC_MEMFAIL,
	GLND_FC_API,
	GLND_FC_EVT,
	GLND_FC_DATA_SEND,
	GLND_FC_TIMER,
	GLND_FC_LOG
} GLND_FLEX_SETS;

typedef enum glnd_log_ids {
	GLND_LID_HDLN,
	GLND_LID_HDLN_TIL,
	GLND_LID_HDLN_TILL,
	GLND_LID_HDLN_TILLL,
	GLND_LID_MEMFAIL,
	GLND_LID_API,
	GLND_LID_EVT,
	GLND_LID_DATA_SEND,
	GLND_LID_TIMER,
	GLND_LID_LOG
} GLND_LOG_IDS;

void glnd_flx_log_reg(void);
void glnd_flx_log_dereg(void);

void glnd_log_headline(uint8_t hdln_id, uint8_t sev, char *file_name, uint32_t line_no);
void glnd_log_headline_TIL(uint8_t hdln_id, char *file_name, uint32_t line_no, uint32_t p1);
void glnd_log_headline_TILL(uint8_t hdln_id, char *file_name, uint32_t line_no, uint32_t p1, uint32_t p2);
void glnd_log_headline_TILLL(uint8_t hdln_id, char *file_name, uint32_t line_no, uint32_t p1, uint32_t p2, uint32_t p3);
void glnd_log_memfail(uint8_t mf_id, char *file_name, uint32_t line_no);
void glnd_log_api(uint8_t api_id, uint8_t sev, char *file_name, uint32_t line_no);
void glnd_log_evt(uint8_t evt_id, char *file_name, uint32_t line_no, uint32_t type, uint32_t node, uint32_t hdl, uint32_t rsc,
			   uint32_t lck);
void glnd_log_data_send(uint8_t id, char *file_name, uint32_t line_no, uint32_t node, uint32_t evt);
void glnd_log_timer(uint8_t id, uint32_t type, char *file_name, uint32_t line_no);
void glnd_log(uint8_t id, uint32_t category, uint8_t sev, uint32_t rc, char *file_name, uint32_t line_no, SaUint64T handle_id,
		       SaUint32T res_id, SaUint64T lock_id);

#define m_LOG_GLND_HEADLINE(id,sev,file_name,line_no)            glnd_log_headline(id,sev,file_name,line_no)
#define m_LOG_GLND_HEADLINE_TI(id,file_name,line_no,p1)          glnd_log_headline_TIL(id,file_name,line_no,p1)
#define m_LOG_GLND_HEADLINE_TIL(id,file_name,line_no,p1,p2)       glnd_log_headline_TILL(id,file_name,line_no,p1,p2)
#define m_LOG_GLND_HEADLINE_TILL(id,file_name,line_no,p1,p2,p3)   glnd_log_headline_TILLL(id,file_name,line_no,p1,p2,p3)
#define m_LOG_GLND_MEMFAIL(id,file_name,line_no)                  glnd_log_memfail(id,file_name,line_no)
#define m_LOG_GLND_API(id,sev,file_name,line_no)                  glnd_log_api(id,sev,file_name,line_no)
#define m_LOG_GLND_EVT(id,file_name,line_no,type,node,hdl,rsc,lck) glnd_log_evt(id,file_name,line_no,type,node,hdl,rsc,lck)
#define m_LOG_GLND_DATA_SEND(id,file_name,line_no,node,evt)       glnd_log_data_send(id,file_name,line_no,node,evt)
#define m_LOG_GLND_TIMER(id,type,file_name,line_no)               glnd_log_timer(id,type,file_name,line_no)
#define m_LOG_GLND(id,category,sev,rc,file_name,line_no,handle_id, res_id, lock_id)  glnd_log(id,category,sev,rc,file_name,line_no,handle_id, res_id, lock_id)

#endif
