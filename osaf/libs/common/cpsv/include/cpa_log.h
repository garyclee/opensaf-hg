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

#ifndef CPA_LOG_H
#define CPA_LOG_H

/* Format types used in CPSv */
#ifndef NCSFL_TYPE_TICCL
#define NCSFL_TYPE_TICCL "TICCL"
#endif

#ifndef NCSFL_TYPE_TICCLL
#define NCSFL_TYPE_TICCLL "TICCLL"
#endif

#ifndef NCSFL_TYPE_TICCLLF
#define NCSFL_TYPE_TICCLLF "TICCLLF"
#endif

#ifndef NCSFL_TYPE_TICCLLFF
#define NCSFL_TYPE_TICCLLFF "TICCLLFF"
#endif

#ifndef NCSFL_TYPE_TICCLFFF
#define NCSFL_TYPE_TICCLFFF "TICCLFFF"
#endif

#ifndef NCSFL_TYPE_TICCLLFFF
#define NCSFL_TYPE_TICCLLFFF "TICCLLFFF"
#endif

/* Logging catagories used for CPSv */
#define NCSFL_LC_CKPT_MGMT 0x00008000	/* CPSv, Ckpt managenet, client management, inti, reg */
#define NCSFL_LC_CKPT_DATA 0x00004000	/* CPSv Read, write, section create, section delete etc */

/*******************************************************************************
 Logging offset indexes for CPA Headline logging
 ******************************************************************************/

typedef enum cpa_hdln_log_flex {
	CPA_SE_API_CREATE_SUCCESS,

	CPA_SE_API_CREATE_FAILED,
	CPA_SE_API_DESTROY_SUCCESS,
	CPA_SE_API_DESTROY_FAILED,
	CPA_SE_API_UNKNOWN_REQUEST,

	CPA_CREATE_HANDLE_FAILED,
	CPA_CB_RETRIEVAL_FAILED,
	CPA_CB_LOCK_INIT_FAILED,
	CPA_CB_LOCK_TAKE_FAILED,
	CPA_EDU_INIT_FAILED,
	CPA_MDS_REGISTER_FAILED,

	CPA_MDS_GET_HANDLE_FAILED,
	CPA_MDS_INSTALL_FAILED,
	CPA_MDS_CALLBK_FAILURE,
	CPA_MDS_CALLBK_UNKNOWN_OP,
	CPA_MDS_ENC_FLAT_FAILURE,
	CPA_VERSION_INCOMPATIBLE,

	CPA_CLIENT_NODE_ADD_FAILED,
	CPA_LOCAL_CKPT_NODE_ADD_FAILED,
	CPA_GLOBAL_CKPT_NODE_ADD_FAILED,
	CPA_CB_CREATE_FAILED,
	CPA_CB_DESTROY_FAILED,
	CPA_CPND_IS_DOWN,
	CPA_CLIENT_NODE_GET_FAILED,
	CPA_IOVECTOR_CHECK_FAILED,
} CPA_HDLN_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging 
 ******************************************************************************/
typedef enum cpa_memfail_log_flex {
	CPA_CB_ALLOC_FAILED,
	CPA_EVT_ALLOC_FAILED,
	CPA_CLIENT_NODE_ALLOC_FAILED,
	CPA_LOCAL_CKPT_NODE_ALLOC_FAILED,
	CPA_GLOBAL_CKPT_NODE_ALLOC_FAILED,
	CPA_SECT_ITER_NODE_ALLOC_FAILED,
	CPA_CALLBACK_INFO_ALLOC_FAILED,
	CPA_DATA_BUFF_ALLOC_FAILED,

} CPA_MEMFAIL_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for LOCK API logging
 ******************************************************************************/
typedef enum cpa_api_log_flex {
	CPA_API_CKPT_INIT_SUCCESS,
	CPA_API_CKPT_INIT_FAIL,
	CPA_API_CKPT_SEL_OBJ_GET_SUCCESS,
	CPA_API_CKPT_SEL_OBJ_GET_FAIL,
	CPA_API_CKPT_DISPATCH_SUCCESS,
	CPA_API_CKPT_DISPATCH_FAIL,
	CPA_API_CKPT_FINALIZE_SUCCESS,
	CPA_API_CKPT_FINALIZE_FAIL,
	CPA_API_CKPT_OPEN_SUCCESS,
	CPA_API_CKPT_OPEN_FAIL,
	CPA_API_CKPT_ASYNC_OPEN_SUCCESS,
	CPA_API_CKPT_ASYNC_OPEN_FAIL,
	CPA_API_CKPT_CLOSE_SUCCESS,
	CPA_API_CKPT_CLOSE_FAIL,
	CPA_API_CKPT_UNLINK_SUCCESS,
	CPA_API_CKPT_UNLINK_FAIL,
	CPA_API_CKPT_RDSET_SUCCESS,
	CPA_API_CKPT_RDSET_FAILED,
	CPA_API_CKPT_AREP_SET_SUCCESS,
	CPA_API_CKPT_AREP_SET_FAILED,
	CPA_API_CKPT_STATUS_GET_SUCCESS,
	CPA_API_CKPT_STATUS_GET_FAILED,
	CPA_API_CKPT_SECT_CREAT_SUCCESS,
	CPA_API_CKPT_SECT_CREAT_FAILED,
	CPA_API_CKPT_SECT_DEL_SUCCESS,
	CPA_API_CKPT_SECT_DEL_FAILED,
	CPA_API_CKPT_SECT_EXP_SET_SUCCESS,
	CPA_API_CKPT_SECT_EXP_SET_FAILED,
	CPA_API_CKPT_SECT_ITER_INIT_SUCCESS,
	CPA_API_CKPT_SECT_ITER_INIT_FAILED,
	CPA_API_CKPT_SECT_ITER_NEXT_SUCCESS,
	CPA_API_CKPT_SECT_ITER_NEXT_FAILED,
	CPA_API_CKPT_SECT_ITER_FIN_SUCCESS,
	CPA_API_CKPT_SECT_ITER_FIN_FAILED,
	CPA_API_CKPT_WRITE_SUCCESS,
	CPA_API_CKPT_WRITE_FAILED,
	CPA_API_CKPT_OVERWRITE_SUCCESS,
	CPA_API_CKPT_OVERWRITE_FAILED,
	CPA_API_CKPT_READ_SUCCESS,
	CPA_API_CKPT_READ_FAILED,
	CPA_API_CKPT_SYNC_SUCCESS,
	CPA_API_CKPT_SYNC_FAILED,
	CPA_API_CKPT_SYNC_ASYNC_SUCCESS,
	CPA_API_CKPT_SYNC_ASYNC_FAILED,
	CPA_API_CKPT_REG_ARR_CBK_SUCCESS,
	CPA_API_CKPT_REG_ARR_CBK_FAILED,
	CPA_API_CKPT_SECT_ID_FREE_SUCCESS,
	CPA_API_CKPT_SECT_ID_FREE_FAILED
} CPA_API_LOG_FLEX;

typedef enum cpa_db_log_flex {
	CPA_LCL_CKPT_NODE_GET_FAILED,
	CPA_GBL_CKPT_FIND_ADD_FAILED,
	CPA_SECT_ITER_NODE_ADD_FAILED,
	CPA_SECT_ITER_NODE_GET_FAILED,
	CPA_SECT_ITER_NODE_DELETE_FAILED,
	CPA_BUILD_DATA_ACCESS_FAILED,
	CPA_PROC_REPLICA_READ_FAILED,
	CPA_PROC_RMT_REPLICA_READ_FAILED
} CPA_DB_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for MDS send  API logging
 ******************************************************************************/
typedef enum cpa_data_send_log_flex {
	CPA_MDS_SEND_FAILURE,
	CPA_MDS_SEND_TIMEOUT
} CPA_DATA_SEND_LOG_FLEX;

typedef enum cpa_generic_log_flex {
	CPA_API_SUCCESS,
	CPA_API_FAILED,
	CPA_PROC_SUCCESS,
	CPA_PROC_FAILED,
	CPA_MEM_ALLOC_FAILED
} CPA_GENERIC_LOG_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum cpa_flex_sets {
	CPA_FC_GEN,		/* Generic string set */
	CPA_FC_HDLN,
	CPA_FC_MEMFAIL,
	CPA_FC_API,
	CPA_FC_DATA_SEND,
	CPA_FC_DB
} CPA_FLEX_SETS;

typedef enum cpa_log_ids {
	CPA_LID_HDLN,
	CPA_LID_MEMFAIL,
	CPA_LID_API,
	CPA_LID_DATA_SEND,
	CPA_LID_DB,
	CPA_LID_TICCL,
	CPA_LID_TICCLL,
	CPA_LID_TICCLLF,
	CPA_LID_TICCLLFF,
	CPA_LID_TICCLFFF,
	CPA_LID_TICCLLFFF
} CPA_LOG_IDS;

void cpa_flx_log_reg(void);
void cpa_flx_log_dereg(void);
uint32_t cpa_reg_strings();

uint32_t cpa_log_ascii_reg(void);
void cpa_log_ascii_dereg(void);

void cpa_log_headline(uint8_t hdln_id, uint8_t sev);
void cpa_log_memfail(uint8_t mf_id);
void cpa_log_api(uint8_t api_id, uint8_t sev);
void cpa_log_data_send(uint8_t id, uint32_t node, uint32_t evt_id);
void cpa_log_db(uint8_t id, uint8_t sev);

#define m_LOG_CPA_HEADLINE(id, sev)       cpa_log_headline(id,sev)
#define m_LOG_CPA_MEMFAIL(id)             cpa_log_memfail(id)
#define m_LOG_CPA_API(id,sev)             cpa_log_api(id,sev)
#define m_LOG_CPA_DATA_SEND(id,node, evt) cpa_log_data_send(id,node, evt)
#define m_LOG_CPA_DB(id,sev)              cpa_log_db(id,sev)

#define m_LOG_CPA_CCL(id, category, severity, str1, fname, fno) ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_TICCL,   \
                                                               CPA_FC_GEN, category, severity, NCSFL_TYPE_TICCL, \
                                                               id, str1, fname, fno)

#define m_LOG_CPA_CCLL(id, category, severity, str1, fname, fno, rc) ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_TICCLL,   \
                                                               CPA_FC_GEN, category, severity, NCSFL_TYPE_TICCLL, \
                                                               id, str1, fname, fno, rc)

#define m_LOG_CPA_CCLLF(id, category, severity, str1, fname, fno, rc, fl1) ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_TICCLLF,   \
                                                               CPA_FC_GEN, category, severity, NCSFL_TYPE_TICCLLF, \
                                                               id, str1, fname, fno, rc, fl1)

#define m_LOG_CPA_CCLLFF(id, category, severity, str1, fname, fno, rc, fl1, fl2)            \
                                                           ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_TICCLLFF,   \
                                                           CPA_FC_GEN, category, severity, NCSFL_TYPE_TICCLLFF, \
                                                           id, str1, fname, fno, rc, fl1, fl2)

#define m_LOG_CPA_CCLFFF(id, category, severity, str1, fname, fno, fl1, fl2, fl3)            \
                                                           ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_TICCLFFF,   \
                                                           CPA_FC_GEN, category, severity, NCSFL_TYPE_TICCLFFF, \
                                                           id, str1, fname, fno, fl1, fl2, fl3)

#define m_LOG_CPA_CCLLFFF(id, category, severity, str1, fname, fno, rc, fl1, fl2, fl3)            \
                                                           ncs_logmsg(NCS_SERVICE_ID_CPA, CPA_LID_TICCLLFFF,   \
                                                           CPA_FC_GEN, category, severity, NCSFL_TYPE_TICCLLFFF, \
                                                           id, str1, fname, fno, rc, fl1, fl2, fl3)

#endif
