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

#ifndef GLA_LOG_H
#define GLA_LOG_H

/******************************************************************************
 Logging offset indexes for GLA Headline logging
 ******************************************************************************/
typedef enum gla_hdln_log_flex {
	GLA_CB_CREATE_FAILED,
	GLA_CB_DESTROY_FAILED,
	GLA_CLIENT_TREE_INIT_FAILED,
	GLA_CLIENT_TREE_ADD_FAILED,
	GLA_CLIENT_TREE_DEL_FAILED,
	GLA_TAKE_HANDLE_FAILED,
	GLA_CREATE_HANDLE_FAILED,
	GLA_MDS_REGISTER_SUCCESS,
	GLA_MDS_REGISTER_FAILED,
	GLA_SE_API_CREATE_SUCCESS,
	GLA_SE_API_CREATE_FAILED,
	GLA_SE_API_DESTROY_SUCCESS,
	GLA_SE_API_DESTROY_FAILED,
	GLA_CB_RETRIEVAL_FAILED,
	GLA_VERSION_INCOMPATIBLE,
	GLA_MDS_GET_HANDLE_FAILED,
	GLA_MDS_CALLBK_FAILURE,
	GLA_GLND_SERVICE_UP,
	GLA_GLND_SERVICE_DOWN,
	GLA_MDS_ENC_FLAT_FAILURE,
	GLA_MDS_DEC_FLAT_FAILURE,
	GLA_AGENT_REGISTER_FAILURE,
	GLA_MSG_FRMT_VER_INVALID,
	GLA_MDS_DEC_FAILURE
} GLA_HDLN_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging 
 ******************************************************************************/
typedef enum gla_memfail_log_flex {
	GLA_CB_ALLOC_FAILED,
	GLA_CLIENT_ALLOC_FAILED,
	GLA_EVT_ALLOC_FAILED,
} GLA_MEMFAIL_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for LOCK API logging
 ******************************************************************************/
typedef enum gla_api_log_flex {
	GLA_API_LCK_INITIALIZE_FAIL,
	GLA_API_LCK_SELECTION_OBJECT_FAIL,
	GLA_API_LCK_OPTIONS_CHECK_FAIL,
	GLA_API_LCK_DISPATCH_FAIL,
	GLA_API_LCK_FINALIZE_FAIL,
	GLA_API_LCK_RESOURCE_OPEN_SYNC_FAIL,
	GLA_API_LCK_RESOURCE_OPEN_ASYNC_FAIL,
	GLA_API_LCK_RESOURCE_LOCK_SYNC_FAIL,
	GLA_API_LCK_RESOURCE_LOCK_ASYNC_FAIL,
	GLA_API_LCK_RESOURCE_UNLOCK_SYNC_FAIL,
	GLA_API_LCK_RESOURCE_UNLOCK_ASYNC_FAIL,
	GLA_API_LCK_RESOURCE_CLOSE_FAIL,
	GLA_API_LCK_RESOURCE_PURGE_FAIL,
	GLA_DISPATCH_ALL_CALLBK_FAIL,
	GLA_DISPATCH_ALL_RMV_IND,
	GLA_DISPATCH_BLOCK_CLIENT_DESTROYED,
} GLA_API_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for SE API logging 
 ******************************************************************************/
typedef enum gla_lock_log_flex {
	GLA_CB_LOCK_INIT_FAILED
} GLA_LOCK_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for Event logging 
 ******************************************************************************/
typedef enum gla_evt_log_flex {
	GLA_EVT_UNKNOWN
} GLA_EVT_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for system call fail logging 
 ******************************************************************************/
typedef enum gla_sys_call_log_flex {
	GLA_GET_SEL_OBJ_FAIL,
	GLA_SEL_OBJ_RMV_IND_FAIL,
} GLA_SYS_CALL_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for system call fail logging 
 ******************************************************************************/
typedef enum gla_data_send_log_flex {
	GLA_MDS_SEND_FAILURE
} GLA_DATA_SEND_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum gla_flex_sets {
	GLA_FC_HDLN,
	GLA_FC_MEMFAIL,
	GLA_FC_API,
	GLA_FC_NCS_LOCK,
	GLA_FC_EVT,
	GLA_FC_SYS_CALL,
	GLA_FC_DATA_SEND
} GLA_FLEX_SETS;

typedef enum gla_log_ids {
	GLA_LID_HDLN,
	GLA_LID_MEMFAIL,
	GLA_LID_API,
	GLA_LID_NCS_LOCK,
	GLA_LID_EVT,
	GLA_LID_SYS_CALL,
	GLA_LID_DATA_SEND
} GLA_LOG_IDS;

void gla_flx_log_reg(void);
void gla_flx_log_dereg(void);

EXTERN_C void gla_log_headline(uns8 hdln_id, uns8 sev);
EXTERN_C void gla_log_memfail(uns8 mf_id);
EXTERN_C void gla_log_api(uns8 api_id, uns8 sev);
EXTERN_C void gla_log_lockfail(uns8 lck_id, uns8 sev);
EXTERN_C void gla_log_evt(uns8 evt_id, uns32 node);
EXTERN_C void gla_log_sys_call(uns8 id, uns32 node);
EXTERN_C void gla_log_data_send(uns8 id, uns32 node, uns32 evt);

#define m_LOG_GLA_HEADLINE(id, sev)        gla_log_headline(id,sev)
#define m_LOG_GLA_MEMFAIL(id)              gla_log_memfail(id)
#define m_LOG_GLA_API(id,sev)              gla_log_api(id,sev)
#define m_LOG_GLA_LOCKFAIL(id,sev)         gla_log_lockfail(id,sev)
#define m_LOG_GLA_EVT(id,node)             gla_log_evt(evt_id,node)
#define m_LOG_GLA_SYS_CALL(id,node)        gla_log_sys_call(id,node)
#define m_LOG_GLA_DATA_SEND(id,node,evt)   gla_log_data_send(id,node,evt)

#endif
