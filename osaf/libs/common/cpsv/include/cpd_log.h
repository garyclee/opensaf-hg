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

  MODULE : CPD_LOG.H

..............................................................................

  DESCRIPTION:

  This module contains logging/tracing defines for CPD.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef CPD_LOG_H
#define CPD_LOG_H

#define cpd_log(severity, format, args...) _cpd_log((severity), __FUNCTION__, (format), ##args)

/******************************************************************************\
            Logging offset indexes for Headline logging
\******************************************************************************/
typedef enum cpd_hdln_flex {
	CPD_CREATE_SUCCESS,
	CPD_CREATE_FAILED,
	CPD_INIT_SUCCESS,
	CPD_INIT_FAILED,
	CPD_CB_HDL_CREATE_FAILED,
	CPD_CREATE_HDL_FAILED,
	CPD_CB_HDL_TAKE_FAILED,
	CPD_AMF_INIT_SUCCESS,
	CPD_AMF_INIT_FAILED,
	CPD_LM_INIT_SUCCESS,
	CPD_LM_INIT_FAILED,
	CPD_MDS_INIT_SUCCESS,
	CPD_MDS_INIT_FAILED,
	CPD_REG_COMP_SUCCESS,
	CPD_REG_COMP_FAILED,
	CPD_BIND_SUCCESS,
	CPD_EDU_BIND_SUCCESS,
	CPD_EDU_UNBIND_SUCCESS,
	CPD_UNBIND_SUCCESS,
	CPD_DEREG_COMP_SUCCESS,
	CPD_MDS_SHUT_SUCCESS,
	CPD_MDS_SHUT_FAILED,
	CPD_LM_SHUT_SUCCESS,
	CPD_AMF_SHUT_SUCCESS,
	CPD_AMF_DESTROY_FAILED,
	CPD_DESTROY_SUCCESS,
	CPD_COMP_NOT_INSERVICE,
	CPD_DONOT_EXIST,
	CPD_MEMORY_ALLOC_FAIL,
	CPD_CKPT_INFO_NODE_ADD_FAILED,
	CPD_CKPT_INFO_NODE_GET_FAILED,
	CPD_CKPT_MAP_INFO_ADD_FAILED,
	CPD_CPND_INFO_NODE_FAILED,
	CPD_CPND_INFO_NODE_GET_FAILED,
	CPD_IPC_CREATE_FAIL,
	CPD_IPC_ATTACH_FAIL,
	CPD_TASK_CREATE_FAIL,
	CPD_TASK_START_FAIL,
	CPD_HEALTHCHECK_START_FAIL,
	CPD_MDS_REGISTER_FAILED,
	CPD_AMF_REGISTER_FAILED,
	CPD_INIT_FAIL,
	CPD_DESTROY_FAIL,
	CPD_CB_RETRIEVAL_FAILED,
	CPD_AMF_GET_SEL_OBJ_FAILURE,
	CPD_AMF_DISPATCH_FAILURE,
	CPD_MBCSV_DISPATCH_FAILURE,
	CPD_MDS_GET_HDL_FAILED,
	CPD_MDS_INSTALL_FAILED,
	CPD_MDS_UNREG_FAILED,
	CPD_MDS_SEND_FAIL,
	CPD_PROCESS_WRONG_EVENT,
	CPD_A2S_CKPT_CREATE_FAILED,
	CPD_CLM_REGISTER_FAIL,
	CPD_MAP_INFO_GET_FAILED,
	CPD_CPND_NODE_DOES_NOT_EXIST,
	CPD_MDS_DEC_FAILED,
	CPD_MDS_ENC_FLAT_FAILED,
	CPD_MDS_DEC_FLAT_FAILED,
	CPD_IPC_SEND_FAILED,
	CPD_MDS_VDEST_CREATE_FAILED,
	CPD_MAP_NODE_DELETE_FAILED,
	CPD_CPND_INFO_NODE_DELETE_FAILED,
	CPD_CKPT_TREE_INIT_FAILED,
	CPD_MAP_TREE_INIT_FAILED,
	CPD_CPND_INFO_TREE_INIT_FAILED,
	CPD_CKPT_REPLOC_TREE_INIT_FAILED,
	CPD_SCALARTBL_REG_FAILED,
	CPD_CKPTTBL_REG_FAILED,
	CPD_REPLOCTBL_REG_FAILED,
	CPD_CKPT_CREATE_SUCCESS,
	CPD_CKPT_CREATE_FAILURE,
	CPD_NON_COLOC_CKPT_CREATE_SUCCESS,
	CPD_NON_COLOC_CKPT_CREATE_FAILURE,
	CPD_NON_COLOC_CKPT_DESTROY_SUCCESS,
	CPD_EVT_UNLINK_SUCCESS,
	CPD_PROC_UNLINK_SET_FAILED,
	CPD_CKPT_RDSET_SUCCESS,
	CPD_CKPT_ACTIVE_SET_SUCCESS,
	CPD_CKPT_ACTIVE_CHANGE_SUCCESS,
	CPD_CLM_GET_SEL_OBJ_FAILURE,
	CPD_CLM_DISPATCH_FAILURE,
	CPD_CLM_CLUSTER_TRACK_FAIL,
} CPD_HDLN_FLEX;

typedef enum cpd_memfail_flex {
	CPD_CB_ALLOC_FAILED,
	CPD_EVT_ALLOC_FAILED,
	CPD_CREF_INFO_ALLOC_FAIL,
	CPD_CPND_DEST_INFO_ALLOC_FAILED,
	NCS_ENC_RESERVE_SPACE_FAILED,
	CPD_MBCSV_MSG_ALLOC_FAILED,
	CPD_CPND_INFO_ALLOC_FAILED,
} CPD_MEMFAIL_LOG_FLEX;

/******************************************************************************\
            Logging offset indexes for Database logging
\******************************************************************************/
typedef enum cpd_db_flex {
	CPD_DB_ADD_SUCCESS,
	CPD_DB_ADD_FAILED,
	CPD_REP_DEL_SUCCESS,
	CPD_REP_DEL_FAILED,
	CPD_REP_ADD_SUCCESS,
	CPD_CKPT_DEL_SUCCESS,
	CPD_DB_DEL_FAILED,
	CPD_DB_UPD_SUCCESS,
	CPD_DB_UPD_FAILED,
} CPD_DB_FLEX;

typedef enum cpd_mbcsv_flex {
	CPD_NCS_MBCSV_SVC_FAILED,
	CPD_MBCSV_INIT_FAILED,
	CPD_MBCSV_OPEN_FAILED,
	CPD_MBCSV_GET_SEL_OBJ_FAILURE,
	CPD_MBCSV_CHGROLE_FAILED,
	CPD_MBCSV_FINALIZE_FAILED,
	CPD_SB_PROC_RETENTION_SET_FAILED,
	CPD_SB_PROC_ACTIVE_SET_FAILED,
	CPD_STANDBY_MSG_PROCESSING,
	CPD_PROCESS_SB_MSG_FAILED,
	CPD_STANDBY_CREATE_EVT,
	CPD_A2S_CKPT_CREATE_ASYNC_SUCCESS,
	CPD_A2S_CKPT_CREATE_ASYNC_FAILED,
	CPD_A2S_CKPT_UNLINK_ASYNC_SUCCESS,
	CPD_A2S_CKPT_UNLINK_ASYNC_FAILED,
	CPD_A2S_CKPT_RDSET_ASYNC_SUCCESS,
	CPD_A2S_CKPT_RDSET_ASYNC_FAILED,
	CPD_A2S_CKPT_AREP_SET_ASYNC_SUCCESS,
	CPD_A2S_CKPT_AREP_SET_ASYNC_FAILED,
	CPD_A2S_CKPT_DESTADD_ASYNC_SUCCESS,
	CPD_A2S_CKPT_DESTADD_ASYNC_FAILED,
	CPD_A2S_CKPT_DESTDEL_ASYNC_SUCCESS,
	CPD_A2S_CKPT_DESTDEL_ASYNC_FAILED,
	EDU_EXEC_ASYNC_CREATE_FAILED,
	EDU_EXEC_ASYNC_UNLINK_FAILED,
	EDU_EXEC_ASYNC_RDSET_FAILED,
	EDU_EXEC_ASYNC_AREP_FAILED,
	EDU_EXEC_ASYNC_DESTADD_FAILED,
	EDU_EXEC_ASYNC_DESTDEL_FAILED,
	EDU_EXEC_ASYNC_USRINFO_FAILED,
	CPD_STANDBY_CREATE_EVT_FAILED,
	CPD_STANDBY_UNLINK_EVT_FAILED,
	CPD_STANDBY_RDSET_EVT_FAILED,
	CPD_STANDBY_AREP_EVT_FAILED,
	CPD_STANDBY_DESTADD_EVT_FAILED,
	CPD_STANDBY_DESTDEL_EVT_FAILED,
	CPD_MBCSV_CLOSE_FAILED,
	CPD_MBCSV_CALLBACK_FAILED,
	CPD_MBCSV_CALLBACK_SUCCESS,
	CPD_A2S_CKPT_USR_INFO_FAILED,
	CPD_A2S_CKPT_USR_INFO_SUCCESS,
	CPD_MBCSV_ASYNC_UPDATE_SEND_FAILED,
	CPD_MBCSV_WARM_SYNC_COUNT_MISMATCH,
	CPD_STANDBY_DESTADD_EVT_SUCCESS,
	CPD_STANDBY_DESTDEL_EVT_INVOKED,
} CPD_MBCSV_FLEX;

typedef enum cpd_generic_flex {
	CPD_HEALTH_CHK_CB_SUCCESS,
	CPD_VDEST_CHG_ROLE_FAILED,
	CPD_CSI_SET_CB_SUCCESS,
	CPD_AMF_COMP_NAME_GET_FAILED,
	CPD_AMF_COMP_REG_SUCCESS,
	CPD_AMF_COMP_REG_FAILED,
	CPD_AMF_COMP_TERM_CB_INVOKED,
	CPD_AMF_CSI_RMV_CB_INVOKED,
	CPD_MDS_REG_SUCCESS,
	CPD_MDS_SUBSCRIBE_FAILED,
} CPD_GENERIC_FLEX;

/******************************************************************************\
      Logging offset indexes for canned constant strings for the ASCII SPEC
\******************************************************************************/
typedef enum cpd_flex_sets {
	CPD_FC_HDLN,
	CPD_FC_DB,
	CPD_FC_MEMFAIL,
	CPD_FC_MBCSV,
	CPD_FC_GENERIC,
	CPD_FC_GENLOG
} CPD_FLEX_SETS;

typedef enum cpd_log_ids {
	CPD_LID_HEADLINE,
	CPD_LID_DB,
	CPD_LID_MEMFAIL,
	CPD_LID_MBCSV,
	CPD_LID_TIFCL,
	CPD_LID_TICL,
	CPD_LID_TILCL,
	CPD_LID_TICFCL,
	CPD_LID_TILLCL,
	CPD_LID_TICCL,
	CPD_LID_TIFFCL,
	CPD_LID_TICFFCL,
	CPD_LID_GENLOG
} CPD_LOG_IDS;

#define NCSFL_TYPE_TIFCL "TIFCL"
#define NCSFL_TYPE_TILCL "TILCL"
#define NCSFL_TYPE_TICFCL "TICFCL"
#define NCSFL_TYPE_TILLCL "TILLCL"
#define NCSFL_TYPE_TICCL "TICCL"
#define NCSFL_TYPE_TIFFCL "TIFFCL"
#define NCSFL_TYPE_TICFFCL "TICFFCL"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                          CPD Logging Control
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void cpd_flx_log_reg(void);
void cpd_flx_log_dereg(void);

void cpd_log_ascii_dereg(void);
uns32 cpd_log_ascii_reg(void);
uns32 cpsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);

void cpd_headline_log(uint8_t, uint8_t);
void cpd_db_status_log(uint8_t, char *);
void cpd_memfail_log(uint8_t);
void cpd_mbcsv_log(uint8_t, uint8_t);
void _cpd_log(uint8_t severity, const char *function, const char *format, ...);
#define m_LOG_CPD_HEADLINE(id, sev) cpd_headline_log(id, sev)
#define m_LOG_CPD_DB_STATUS(id, str) cpd_db_status_log(id, str)
#define m_LOG_CPD_MEMFAIL(id) cpd_memfail_log(id)
#define m_LOG_CPD_MBCSV(id,sev)  cpd_mbcsv_log(id,sev)

#define m_LOG_CPD_FCL(id,strid,sev,ckptid,filename,lineno) ncs_logmsg(NCS_SERVICE_ID_CPD, \
                                                             CPD_LID_TIFCL,strid,NCSFL_LC_HEADLINE,sev,NCSFL_TYPE_TIFCL,\
                                                             id,ckptid,filename,lineno)

#define m_LOG_CPD_CL(id,strid,sev,filename,lineno) ncs_logmsg(NCS_SERVICE_ID_CPD,CPD_LID_TICL,strid, \
                                                                 NCSFL_LC_HEADLINE,sev,NCSFL_TYPE_TICL,id,filename,lineno)

#define m_LOG_CPD_LCL(id,strid,sev,nodeid,filename,lineno) ncs_logmsg(NCS_SERVICE_ID_CPD,CPD_LID_TILCL, \
                                                                               strid,NCSFL_LC_HEADLINE,sev,NCSFL_TYPE_TILCL,id, \
                                                                               nodeid,filename,lineno)

#define m_LOG_CPD_CFCL(id,strid,sev,ckptname,ckptid,filename,lineno) ncs_logmsg(NCS_SERVICE_ID_CPD, \
                                                                        CPD_LID_TICFCL,strid,NCSFL_LC_HEADLINE,sev, \
                                                                        NCSFL_TYPE_TICFCL,id,ckptname,ckptid,\
                                                                        filename,lineno)

#define m_LOG_CPD_LLCL(id,strid,sev,cnt1,cnt2,filename,lineno) ncs_logmsg(NCS_SERVICE_ID_CPD, \
                                                                CPD_LID_TILLCL,strid,NCSFL_LC_HEADLINE,sev,\
                                                                NCSFL_TYPE_TILLCL,id,cnt1,cnt2,filename,lineno)

#define m_LOG_CPD_CCL(id,strid,sev,ckptname,filename,lineno)  ncs_logmsg(NCS_SERVICE_ID_CPD,CPD_LID_TICCL,strid, \
                                                                  NCSFL_LC_HEADLINE,sev,NCSFL_TYPE_TICCL,id,ckptname,\
                                                                  filename,lineno)

#define m_LOG_CPD_FFCL(id,strid,sev,ckptid,mdest,filename,lineno) ncs_logmsg(NCS_SERVICE_ID_CPD,CPD_LID_TIFFCL,\
                                                                  strid,NCSFL_LC_HEADLINE,sev,NCSFL_TYPE_TIFFCL,id, \
                                                                  ckptid,mdest,filename,lineno)

#define m_LOG_CPD_CFFCL(id,strid,sev,ckptname,ckptid,mdest,filename,lineno) ncs_logmsg(NCS_SERVICE_ID_CPD,CPD_LID_TICFFCL,\
                                                                  strid,NCSFL_LC_HEADLINE,sev,NCSFL_TYPE_TICFFCL,id, ckptname,\
                                                                  ckptid,mdest,filename,lineno)

#endif   /* CPD_LOG_H */
