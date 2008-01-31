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



..............................................................................

  DESCRIPTION:

  This module is the include file for BOM Access Manager logging.
  
******************************************************************************
*/

#ifndef BAM_LOG_H
#define BAM_LOG_H

#include "bam.h"
#include "ncs_log.h"
/* DTS header file */
#include "dta_papi.h"
#include "dts_papi.h"

/* DTSv version support */
#define BAM_LOG_VERSION 1

/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/
typedef enum bam_hdln_flex
{
   BAM_CREATE_HANDLE_FAILED,
   BAM_TAKE_HANDLE_FAILED,
   BAM_TAKE_LOCK_FAILED,
   BAM_IPC_TASK_INIT,
   BAM_UNKNOWN_EVT_RCVD,
   BAM_EVT_PROC_FAILED,
   BAM_FILE_ARG_IGNORED,
   BAM_UNKNOWN_MSG
} BAM_HDLN_FLEX;

/******************************************************************************
 Logging offset indexes for AMF logging
 ******************************************************************************/
typedef enum bam_svc_prvdr_flex
{
   BAM_MDS_INSTALL_FAIL,
   BAM_MDS_INSTALL_SUCCESS,
   BAM_MDS_UNINSTALL_FAIL,
   BAM_MDS_SUBSCRIBE_FAIL,
   BAM_MDS_SUBSCRIBE_SUCCESS,
   BAM_MDS_SEND_ERROR,
   BAM_MDS_CALL_BACK_ERROR,
   BAM_MDS_UNKNOWN_SVC_EVT,
   BAM_MDS_VDEST_CREATE_FAIL,
   BAM_MDS_VDEST_DESTROY_FAIL,
   BAM_MDS_SEND_SUCCESS
} BAM_SVC_PRVDR_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging 
 ******************************************************************************/

typedef enum bam_memfail_flex
{     
   BAM_CB_ALLOC_FAILED,
   BAM_MBX_ALLOC_FAILED,
   BAM_ALLOC_FAILED,
   BAM_MESSAGE_ALLOC_FAILED,
   BAM_EVT_ALLOC_FAILED
} BAM_MEMFAIL_FLEX;

/******************************************************************************
 Logging offset indexes for API logging 
 ******************************************************************************/
typedef enum bam_api_flex
{
   BAM_SE_API_CREATE_SUCCESS,
   BAM_SE_API_CREATE_FAILED,
   BAM_SE_API_DESTROY_SUCCESS,
   BAM_SE_API_DESTROY_FAILED,
   BAM_SE_API_UNKNOWN
} BAM_API_FLEX;

/******************************************************************************
 Logging offset indexes for info logging 
 ******************************************************************************/
typedef enum bam_info_flex
{
   BAM_PARSE_SUCCESS,
   BAM_PARSE_ERROR,
   BAM_NO_OID_MATCH,
   BAM_NO_PROTO_MATCH,
   BAM_INVALID_DOMNODE,
   BAM_BOMFILE_ERROR,
   BAM_ENT_PAT_ADD_FAIL,
   BAM_ENT_PAT_INIT_FAIL,
   BAM_NCS_CONFIG,
   BAM_HW_DEP_CONFIG,
   BAM_APP_CONFIG,
   BAM_VALID_CONFIG,
   BAM_PSS_NAME,
   BAM_START_PARSE,
   BAM_APP_FILE,
   BAM_APP_NOT_PRES
}BAM_INFO_FLEX;

/******************************************************************************
 Logging offset indexes for Event logging 
 ******************************************************************************/
typedef enum bam_msg_flex
{
   BAM_DEF_MSG
}BAM_MSG_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum bam_flex_sets
{
   BAM_FC_HDLN,
   BAM_FC_MEMFAIL,
   BAM_FC_API,
   BAM_FC_MSG,
   BAM_FC_SVC_PRVDR,
   BAM_FC_INFO
}BAM_FLEX_SETS;

typedef enum bam_log_ids
{
   BAM_LID_HDLN,
   BAM_LID_MEMFAIL,
   BAM_LID_API,
   BAM_LID_MSG,
   BAM_LID_MSG_TIL,
   BAM_LID_MSG_TIC,
   BAM_LID_SVC_PRVDR,
   BAM_LID_MDS_SND,
   BAM_LID_MSG_TII,
   BAM_LID_INFO
} BAM_LOG_IDS;


EXTERN_C void bam_flx_log_reg(void);
EXTERN_C void bam_flx_log_dereg(void);

EXTERN_C uns32 bam_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
EXTERN_C uns32 bam_reg_strings (void);
EXTERN_C uns32 bam_unreg_strings (void);
EXTERN_C char *bam_concatenate_twin_string (char *string1,char *string2, char *o_str);
#if ( NCS_BAM_LOG == 1) 
EXTERN_C void bam_log_headline(uns8 hdln_id, uns8 sev);
EXTERN_C void bam_log_memfail(uns8 mf_id);
EXTERN_C void bam_log_api(uns8 api_id, uns8 sev);
EXTERN_C void bam_log_msg(uns8 msg_id, uns8 sev, uns32 node);
EXTERN_C void bam_log_svc_prvdr(uns8 sp_id, uns8 sev);
EXTERN_C void bam_log_TICL_fmt_2string(uns8 msg_id, uns8 sev, char *string1, char *string2);
EXTERN_C void bam_log_msg_TIL(uns8 msg_id, uns8 sev, uns32 info);
EXTERN_C void bam_log_msg_TIC(uns8 msg_id, uns8 sev, char* str);

#define m_LOG_BAM_HEADLINE(id, sev)        bam_log_headline(id,sev)
#define m_LOG_BAM_MEMFAIL(id)              bam_log_memfail(id)
#define m_LOG_BAM_API(id,sev)              bam_log_api(id,sev)
#define m_LOG_BAM_MSG(id,sev,node)         bam_log_msg(id,sev,node)
#define m_LOG_BAM_MSG_TICL(id,sev,string1,string2) bam_log_TICL_fmt_2string(id,sev,string1,string2)
#define m_LOG_BAM_MSG_TIL(id,sev,info) bam_log_msg_TIL(id,sev,info)
#define m_LOG_BAM_SVC_PRVDR(id, sev)       bam_log_svc_prvdr(id,sev)
#define m_LOG_BAM_MSG_TIC(id,sev,info) bam_log_msg_TIC(id,sev,info)
#define m_LOG_BAM_MDS_ERR(id, sev)     ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_MDS_SND, BAM_FC_SVC_PRVDR, \
                                                  NCSFL_LC_FUNC_RET_FAIL, sev, NCSFL_TYPE_TICL, \
                                                  id, __FILE__, __LINE__)
#define m_LOG_BAM_MSG_TII(idx1, idx2, sev) ncs_logmsg(NCS_SERVICE_ID_BAM, BAM_LID_MSG_TII, BAM_FC_INFO, \
                                           NCSFL_LC_FUNC_RET_FAIL, sev, NCSFL_TYPE_TII, idx1, idx2)
#else /*( NCS_BAM_LOG == 1)  */

#define m_LOG_BAM_HEADLINE(id, sev)        
#define m_LOG_BAM_MEMFAIL(id)             
#define m_LOG_BAM_API(id,sev)            
#define m_LOG_BAM_MSG(id,sev,node)      
#define m_LOG_BAM_MSG_TICL(id,sev,string1,string2) 
#define m_LOG_BAM_MSG_TIL(id,sev,info) 
#define m_LOG_BAM_SVC_PRVDR(id, sev)      
#define m_LOG_BAM_MSG_TIC(id,sev,string1)
#define m_LOG_BAM_MSG_TII(idx1, idx2, sev) 

#endif /*( (NCS_DTA == 1) && ( NCS_BAM_LOG == 1) ) */
#endif /* BAM_LOG_H */



