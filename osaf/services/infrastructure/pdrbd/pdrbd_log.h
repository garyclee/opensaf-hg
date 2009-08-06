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
 */

/******************************************************************************
  DESCRIPTION:

  This module contains logging/tracing functions.
*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef PDRBD_LOG_H
#define PDRBD_LOG_H



#include "pdrbd.h"

/*******************************************************************************************/

/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/

typedef enum pdrbd_hdln_logs
  {
    PDRBD_HDLN_PDRBD_CREATE_FAILURE,
    PDRBD_HDLN_PDRBD_CREATE_SUCCESS,
    PDRBD_HDLN_PDRBD_FLS_REGN_SUCCESS,
    PDRBD_HDLN_PDRBD_PIPE_CREATE_FAILURE,
    PDRBD_HDLN_PDRBD_PIPE_CREATE_SUCCESS,
    PDRBD_HDLN_PDRBD_PIPE_OPEN_FAILURE,
    PDRBD_HDLN_PDRBD_PIPE_OPEN_SUCCESS,
    PDRBD_HDLN_PDRBD_MBOX_CREATE_FAILURE,
    PDRBD_HDLN_PDRBD_MBOX_CREATE_SUCCESS,
    PDRBD_HDLN_PDRBD_MBOX_ATTACH_FAILURE,
    PDRBD_HDLN_PDRBD_MBOX_ATTACH_SUCCESS,
    PDRBD_HDLN_PDRBD_THREAD_CREATE_FAILURE,
    PDRBD_HDLN_PDRBD_THREAD_CREATE_SUCCESS,
    PDRBD_HDLN_PDRBD_THREAD_START_FAILURE,
    PDRBD_HDLN_PDRBD_THREAD_START_SUCCESS,
    PDRBD_HDLN_NULL_INST

  } PDRBD_HDLN_LOGS;

/******************************************************************************
 Logging offset indexes for Service Provider (MDS) logging
 ******************************************************************************/

typedef enum pdrbd_amf_logs
  {
    PDRBD_SP_AMF_INSTALL_FAILURE,
    PDRBD_SP_AMF_INSTALL_SUCCESS,
    PDRBD_SP_AMF_COMP_NAME_GET_FAILURE,
    PDRBD_SP_AMF_COMP_NAME_GET_SUCCESS,
    PDRBD_SP_AMF_COMP_REGN_FAILURE,
    PDRBD_SP_AMF_COMP_REGN_SUCCESS,
    PDRBD_SP_AMF_HCHK_START_FAILURE,
    PDRBD_SP_AMF_HCHK_START_SUCCESS,
    PDRBD_SP_AMF_GET_SEL_OBJ_FAILURE,
    PDRBD_SP_AMF_GET_SEL_OBJ_SUCCESS,
    PDRBD_SP_AMF_DESPATCH_FAILURE,
    PDRBD_SP_AMF_DESPATCH_SUCCESS,
    PDRBD_SP_AMF_RESPONSE_FAILURE,
    PDRBD_SP_AMF_RESPONSE_SUCCESS,
    PDRBD_SP_AMF_UNINSTALL_FAILURE,
    PDRBD_SP_AMF_UNINSTALL_SUCCESS,
    PDRBD_SP_AMF_COMP_UNREGN_FAILURE,
    PDRBD_SP_AMF_COMP_UNREGN_SUCCESS,
    PDRBD_SP_AMF_ACTIVE_ROLE_ASSIGN,
    PDRBD_SP_AMF_STANDBY_ROLE_ASSIGN,
    PDRBD_SP_AMF_QUIESCED_ROLE_ASSIGN,
    PDRBD_SP_AMF_INVALID_ROLE_ASSIGN,
    PDRBD_SP_AMF_REMOVE_CB_INVOKE,
    PDRBD_SP_AMF_HLTH_CHK_CB_INVOKE,
    PDRBD_SP_AMF_TERMINATE_CB_INVOKE

  } PDRBD_AMF_LOGS;


/******************************************************************************
   Logging offset indices for pipe message process logging
*******************************************************************************/
typedef enum pdrbdPipeLogs
{
   PDRBD_PIPE_MSG_INCOMPLETE,
   PDRBD_PIPE_SET_PRI_FAILURE,
   PDRBD_PIPE_SET_PRI_SUCCESS,
   PDRBD_PIPE_SET_SEC_FAILURE,
   PDRBD_PIPE_SET_SEC_SUCCESS,
   PDRBD_PIPE_SET_QUI_FAILURE,
   PDRBD_PIPE_SET_QUI_SUCCESS,
   PDRBD_PIPE_DRBDADM_NOT_FND,
   PDRBD_PIPE_DRBD_MOD_LOD_FAILURE,
   PDRBD_PIPE_PRTN_MNT_FAILURE,
   PDRBD_PIPE_PRTN_UMNT_FAILURE,
   PDRBD_PIPE_DRBDADM_ERR,
   PDRBD_PIPE_REM_FAILURE,
   PDRBD_PIPE_REM_SUCCESS,
   PDRBD_PIPE_TERM_FAILURE,
   PDRBD_PIPE_TERM_SUCCESS,
   PDRBD_PIPE_CLMT_FAILURE,
   PDRBD_PIPE_CLMT_SUCCESS,
   PDRBD_PIPE_CS_UNCONFIGURED,
   PDRBD_PIPE_CS_STANDALONE,
   PDRBD_PIPE_CS_UNCONNECTED,
   PDRBD_PIPE_CS_WFCONNECTION,
   PDRBD_PIPE_CS_WFREPORTPARAMS,
   PDRBD_PIPE_CS_CONNECTED,
   PDRBD_PIPE_CS_TIMEOUT,
   PDRBD_PIPE_CS_BROKENPIPE,
   PDRBD_PIPE_CS_NETWORKFAILURE,
   PDRBD_PIPE_CS_WFBITMAPS,
   PDRBD_PIPE_CS_WFBITMAPT,
   PDRBD_PIPE_CS_SYNCSOURCE,
   PDRBD_PIPE_CS_SYNCTARGET,
   PDRBD_PIPE_CS_PAUSEDSYNCS,
   PDRBD_PIPE_CS_PAUSEDSYNCT,
   PDRBD_PIPE_CS_INVALID

} PDRBD_PIPE_LOGS;


/******************************************************************************
 Logging offset indexes for Memory Fail logging
 ******************************************************************************/
typedef enum pdrbd_mem_logs
  {
    PDRBD_MF_ALLOC_EVT_SUCCESS,
    PDRBD_MF_ALLOC_EVT_FAIL

  } PDRBD_MEM_LOGS;


/******************************************************************************
 Logging offset indexes for timer events
 ******************************************************************************/

typedef enum pdrbd_script_logs
  {
    /* Add all script return info here */
    PDRBD_SCRIPT_CB_GET_FAILURE,
    PDRBD_SCRIPT_EVT_CREATE_FAILURE,
    PDRBD_SCRIPT_EXEC_FAILURE,
    PDRBD_SCRIPT_EXEC_SUCCESS,
    PDRBD_SCRIPT_EXIT_NORMAL,
    PDRBD_SCRIPT_EXIT_ERROR,
    PDRBD_SCRIPT_CTX_NOT_FOUND

  } PDRBD_SCRIPT_LOGS;


/******************************************************************************
 Logging offset indexes for Miscellaneous Errors like MDS and Others
 ******************************************************************************/

typedef enum pdrbd_misc_logs
  {
    /* Add all script return info here */
    PDRBD_MISC_MDS_GET_HDL_FAIL,
    PDRBD_MISC_MDS_REG_FAIL,
    PDRBD_MISC_PROXIED_COMP_FILE_ABSENT,
    PDRBD_MISC_MAX_PROXIED_COMP_EXCEED,
    PDRBD_MISC_NO_VALID_ENTRIES,
    PDRBD_MISC_MSNG_COMP_ID_IN_CONF_FILE,
    PDRBD_MISC_MSNG_SU_ID_IN_CONF_FILE,
    PDRBD_MISC_MSNG_RSRC_NAME_IN_CONF_FILE,
    PDRBD_MISC_MSNG_DEVICE_NAME_IN_CONF_FILE,
    PDRBD_MISC_MSNG_MNT_PNT_IN_CONF_FILE,
    PDRBD_MISC_MSNG_DATA_DSC_NAME_IN_CONF_FILE,
    PDRBD_MISC_MSNG_METADATA_DSC_NAME_IN_CONF_FILE,
    PDRBD_MISC_AMF_INITIALIZE_FAILURE,
    PDRBD_MISC_AMF_INITIALIZE_SUCCESS,
    PDRBD_MISC_AMF_REG_FAILED,
    PDRBD_MISC_AMF_REG_SUCCESS,
    PDRBD_MISC_AMF_ERROR_RPT_FAILED,
    PDRBD_MISC_AMF_ERROR_RTP_SUCCESS,
    PDRBD_MISC_AMF_CBK_INIT_FAILED,
    PDRBD_MISC_AMF_CBK_INIT_SUCCESS,
    PDRBD_MISC_AMF_CBK_UNINIT_FAILED,
    PDRBD_MISC_AMF_CBK_UNINIT_SUCCESS,
    PDRBD_MISC_AMF_COMP_NAME_REG_FAILED,
    PDRBD_MISC_AMF_COMP_NAME_REG_SUCCESS,
    PDRBD_MISC_AMF_COMP_NAME_UNREG_FAILED,
    PDRBD_MISC_AMF_COMP_NAME_UNREG_SUCCESS,
    PDRBD_MISC_AMF_COMP_RCVD_ACT_ROLE,
    PDRBD_MISC_AMF_ACT_CSI_SCRIPT_EXEC_FAILED,
    PDRBD_MISC_AMF_ACT_CSI_SCRIPT_EXEC_SUCCESS,
    PDRBD_MISC_AMF_COMP_RCVD_STDBY_ROLE,
    PDRBD_MISC_AMF_STDBY_CSI_SCRIPT_EXEC_FAILED,
    PDRBD_MISC_AMF_STDBY_CSI_SCRIPT_EXEC_SUCCESS,
    PDRBD_MISC_AMF_COMP_RCVD_QUISCED_ROLE,
    PDRBD_MISC_AMF_QSED_CSI_SCRIPT_EXEC_FAILED,
    PDRBD_MISC_AMF_QSED_CSI_SCRIPT_EXEC_SUCCESS,
    PDRBD_MISC_AMF_COMP_RCVD_INVALID_ROLE,
    PDRBD_MISC_AMF_COMP_RCVD_RMV_CBK,
    PDRBD_MISC_AMF_COMP_RCVD_TERM_CBK,
    PDRBD_MISC_AMF_COMP_TERM_SCRIPT_EXEC_FAILED,
    PDRBD_MISC_AMF_HLT_CHK_SCRIPT_TMD_OUT,
    PDRBD_MISC_INVOKING_SCRIPT_TO_CLEAN_RSRC_METADATA,
    PDRBD_MISC_CLEAN_METADATA_SCRIPT_EXEC_FAILED,
    PDRBD_MISC_RECONNECTING_RESOURCE,
    PDRBD_MISC_PROXIED_CONF_FILE_PARSE_FAILED,
    PDRBD_MISC_UNABLE_TO_PARSE_PROXIED_INFO,
    PDRBD_MISC_PROXIED_INFO_FROM_CONF_FILE,
    PDRBD_MISC_UNABLE_TO_RETRIEVE_NODE_ID,  
    PDRBD_MISC_PSEUDO_DRBD_HCK_SRC_EXEC_FAILED,
    PDRBD_MISC_AMF_COMP_RMV_SCRIPT_EXEC_FAILED,
    PDRBD_MISC_GET_ADEST_HDL_FAILED,          
    PDRBD_MISC_MDS_HDL_IS_NULL,              
    PDRBD_MISC_MDS_INSTALL_SUBSCRIBE_FAILED,
    PDRBD_MISC_MDS_SUBSCRIPTION_FAILED,    
    PDRBD_MISC_MDS_CBK_WITH_WRONG_OPERATION, 
    PDRBD_MISC_MDS_EVT_RECD,                
    PDRBD_MISC_MDS_DOWN_EVT_RCVD,          
    PDRBD_MISC_MDS_UP_EVT_RCVD,           
    PDRBD_MISC_RCVD_NULL_USR_BUFF,       
    PDRBD_MISC_RCVD_NULL_MSG_DATA,      
    PDRBD_MISC_NCS_ENC_RESERVE_SPC_NULL,        
    PDRBD_MISC_WRONG_MSG_TYPE_RCVD,            
    PDRBD_MISC_RCVD_NULL_USR_BUF_FOR_DEC,     
    PDRBD_MISC_RCVD_NULL_MSG_BUF_FOR_DEC,    
    PDRBD_MISC_MEM_ALLOC_FAILED_FOR_DEC,    
    PDRBD_MISC_DEC_FLAT_SPACE_RET_NULL,    
    PDRBD_MISC_WRNG_MSG_TYPE_RCVD_DEC,    
    PDRBD_MISC_MDS_ASYNC_SEND_FAILED,
    PDRBD_MISC_PROXIED_COMP_LEN_EXCEED_MAX_LEN,
    PDRBD_MISC_PROXIED_SUID_LEN_EXCEED_MAX_LEN,
    PDRBD_MISC_PROXIED_RSC_LEN_EXCEED_MAX_LEN,
    PDRBD_MISC_PROXIED_DEV_LEN_EXCEED_MAX_LEN,
    PDRBD_MISC_PROXIED_MNT_PNT_LEN_EXCEED_MAX_LEN,
    PDRBD_MISC_PROXIED_DATA_DSC_LEN_EXCEED_MAX_LEN,
    PDRBD_MISC_PROXIED_META_DATA_DSC_LEN_EXCEED_MAX_LEN

  } PDRBD_MISC_LOGS;


/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum mbcsv_flex_sets
  {
    PDRBD_FC_HDLN,
    PDRBD_FC_AMF,
    PDRBD_FC_PIPE,
    PDRBD_FC_MEM,
    PDRBD_FC_SCRIPT,
    PDRBD_FC_MISC

  } PDRBD_FLEX_SETS;


typedef enum mbcsv_log_ids
  {
    PDRBD_LID_HDLN,
    PDRBD_LID_AMF,
    PDRBD_LID_PIPE,
    PDRBD_LID_MEM,
    PDRBD_LID_SCRIPT,
    PDRBD_LID_MISC,
    PDRBD_LID_MISC_INDEX,
    PDRBD_LID_OTHER

  } PDRBD_LOG_IDS;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                          PDRBD Logging Control

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#if (PDRBD_LOG == 1)

EXTERN_C  uns32 mbcsv_reg_strings ();
EXTERN_C  uns32 mbcsv_dereg_strings();
EXTERN_C  uns32 mbcsv_log_bind(void);
EXTERN_C  uns32 mbcsv_log_unbind(void);

EXTERN_C  uns32 pdrbd_log_bind(void);
EXTERN_C  uns32 pdrbd_log_unbind(void);

#define m_LOG_PDRBD_HEADLINE(hdln_id, sev, val) \
ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_HDLN, PDRBD_FC_HDLN, \
           NCSFL_LC_HEADLINE, sev, "TIL", hdln_id, val)

#define m_LOG_PDRBD_AMF(amf_id, sev, val)  \
   ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_AMF, PDRBD_FC_AMF, \
           NCSFL_LC_SVC_PRVDR, sev, "TIL", amf_id, val)

#define m_LOG_PDRBD_PIPE(pipe_id, sev, val)  \
   ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_PIPE, PDRBD_FC_PIPE, \
           NCSFL_LC_SVC_PRVDR, sev, "TIL", pipe_id, val)

#define m_LOG_PDRBD_MEM(mem_id, sev, val)  \
ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_MEM, PDRBD_FC_MEM, NCSFL_LC_MEMORY, \
         sev, "TIL", mem_id, val)

#define m_LOG_PDRBD_SCRIPT(scr_id, sev, val)  \
ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_SCRIPT, PDRBD_FC_SCRIPT, NCSFL_LC_API, \
         sev, "TIL", scr_id, val)

#define m_LOG_PDRBD_DBG_OTHER(str, ser, val)   \
ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_OTHER, PDRBD_FC_MISC, NCSFL_LC_MISC, \
           NCSFL_SEV_WARNING, "TCL", str, val);

#define m_LOG_PDRBD_MISC(index, sev, str)   \
{ \
if(str != NULL) \
   ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_MISC, PDRBD_FC_MISC, NCSFL_LC_MISC, \
           sev, "TIC", index, str);\
else \
   ncs_logmsg(NCS_SERVICE_ID_PDRBD, PDRBD_LID_MISC_INDEX, PDRBD_FC_MISC, NCSFL_LC_MISC, \
           sev, "TI", index);\
}

#else

#define m_LOG_PDRBD_HEADLINE(hdln_id, sev, val)
#define m_LOG_PDRBD_AMF(amf_id, sev, val)
#define m_LOG_PDRBD_PIPE(pipe_id, sev, val)
#define m_LOG_PDRBD_MEM(mem_id, sev, val)
#define m_LOG_PDRBD_SCRIPT(scr_id, sev, val)
#define m_LOG_PDRBD_DBG_OTHER(str, ser, val)
#define m_LOG_PDRBD_MISC(str, ser, val)

#endif /* PDRBD_LOG == 1 */

/* DTSv versioning support */
#define PDRBD_LOG_VERSION 2

#endif /* PDRBD_LOG_H */
