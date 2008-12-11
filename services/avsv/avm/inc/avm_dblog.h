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

  Description: 

   This is the include file for AvM debug logging.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef __AVM_DBLOG_H__
#define __AVM_DBLOG_H__

/******************************************************************************
           Logging Offset indices for Headlines
 ******************************************************************************/
typedef enum avm_hdln_flex
{
  AVM_INVALID_VAL,
  AVM_UNKNOWN_MSG_RCVD,
  AVM_MSG_PROC_FAILED,
  AVM_ENTERED_FUNC, 
  AVM_RCVD_VAL   
}AVM_HDLN_FLEX;


/******************************************************************************
          Logging Offset indices for SE-API. 
 ******************************************************************************/
typedef enum  avm_seapi_flex 
{
   AVM_LOG_SEAPI_CREATE , 
   AVM_LOG_SEAPI_DESTROY,
   AVM_LOG_SEAPI_SUCCESS,
   AVM_LOG_SEAPI_FAILURE
}AVM_SEAPI_FLEX;

/******************************************************************************
          Logging Offset indices for CB
 ******************************************************************************/
typedef enum avm_cb_flex 
{
   AVM_LOG_CB_CREATE  ,     
   AVM_LOG_CB_DESTROY ,    
   AVM_LOG_CB_HDL_ASS_CREATE, 
   AVM_LOG_CB_HDL_ASS_REMOVE,
   AVM_LOG_CB_RETRIEVE,     
   AVM_LOG_CB_RETURN  ,    
   AVM_LOG_CB_SUCCESS ,   
   AVM_LOG_CB_FAILURE  
}AVM_CB_FLEX;

/******************************************************************************
        Logging Offset indices for RDE. 
 ******************************************************************************/
typedef enum avm_rde_flex 
{
   AVM_LOG_RDE_REG         , 
   AVM_LOG_RDE_SUCCESS     ,
   AVM_LOG_RDE_FAILURE     
}AVM_RDE_FLEX;

/******************************************************************************
        Logging Offset indices for MDS. 
 ******************************************************************************/
typedef enum avm_mds_flex 
{
   AVM_LOG_MDS_REG         , 
   AVM_LOG_MDS_INSTALL     , 
   AVM_LOG_MDS_UNINSTALL   ,   
   AVM_LOG_MDS_SUBSCRIBE   , 
   AVM_LOG_MDS_UNREG       , 
   AVM_LOG_MDS_ADEST_HDL   ,
   AVM_LOG_MDS_ADEST_REG   ,
   AVM_LOG_MDS_VDEST_CRT   ,
   AVM_LOG_MDS_VDEST_ROL   ,
   AVM_LOG_MDS_VDEST_REG   ,
   AVM_LOG_MDS_VDEST_DESTROY, 
   AVM_LOG_MDS_SEND        , 
   AVM_LOG_MDS_RCV_CBK     ,
   AVM_LOG_MDS_CPY_CBK     , 
   AVM_LOG_MDS_SVEVT_CBK   , 
   AVM_LOG_MDS_SUCCESS     ,
   AVM_LOG_MDS_FAILURE     
}AVM_MDS_FLEX;

/******************************************************************************
       Logging Offset indices for MAS 
 ******************************************************************************/
typedef enum avm_mas_flex 
{
   AVM_LOG_MIBLIB_REGISTER,  
   AVM_LOG_MIB_CPY,   
   AVM_LOG_OAC_REGISTER,
   AVM_LOG_OAC_REGISTER_TBL,       
   AVM_LOG_OAC_UNREGISTER_TBL,    
   AVM_LOG_OAC_REGISTER_ROW,       
   AVM_LOG_OAC_UNREGISTER_ROW,    
   AVM_LOG_PSS_REGISTER,   
   AVM_LOG_OAC_WARMBOOT,
   AVM_LOG_MAS_SKIP,         
   AVM_LOG_MAS_SUCCESS,     
   AVM_LOG_MAS_FAILURE    
   
}AVM_MAS_FLEX;

/******************************************************************************
        Logging Offset Indices for EDA
 ******************************************************************************/
typedef enum  avm_eda_flex 
{
   AVM_LOG_EDA_CREATE,
   AVM_LOG_EDA_EVT_INIT     ,   
   AVM_LOG_EDA_EVT_FINALIZE ,
   AVM_LOG_EDA_CHANNEL_OPEN ,    
   AVM_LOG_EDA_CHANNEL_CLOSE,
   AVM_LOG_EDA_EVT_SUBSCRIBE,    
   AVM_LOG_EDA_EVT_UNSUBSCRIBE,
   AVM_LOG_EDA_EVT_SELOBJ,
   AVM_LOG_EDA_EVT_CBK      ,   
   AVM_LOG_EDA_SUCCESS      ,  
   AVM_LOG_EDA_FAILURE      
}AVM_EDA_FLEX;

/******************************************************************************
          Logging Offset Indices for MBCSv 
 ******************************************************************************/
typedef enum avm_mbcsv_flex 
{
  AVM_MBCSV_MSG_WARM_SYNC_REQ,
  AVM_MBCSV_MSG_WARM_SYNC_RESP,
  AVM_MBCSV_MSG_DATA_RESP,
  AVM_MBCSV_MSG_DATA_RESP_COMPLETE,
  AVM_MBCSV_MSG_DATA_REQ,
  AVM_MBCSV_MSG_ENC,
  AVM_MBCSV_MSG_DEC,
  AVM_MBCSV_MSG_COLD_ENC,
  AVM_MBCSV_MSG_COLD_DEC,
  AVM_MBCSV_MSG_ASYNC_ENC,
  AVM_MBCSV_MSG_ASYNC_DEC,
  AVM_MBCSV_RES_BASE  = AVM_MBCSV_MSG_ASYNC_DEC,
  AVM_MBCSV_SUCCESS, 
  AVM_MBCSV_FAILURE,
  AVM_MBCSV_IGNORE
}AVM_MBCSV_FLEX;

/******************************************************************************
           Logging Offset Indices for HPL 
 ******************************************************************************/
typedef enum avm_hpl_flex
{
   AVM_LOG_HPL_INIT,          
   AVM_LOG_HPL_DESTROY,      
   AVM_LOG_HPL_API,         
   AVM_LOG_HPL_SUCCESS,    
   AVM_LOG_HPL_FAILURE   
}AVM_HPL_FLEX;

/******************************************************************************
           Logging Offset Indices for Mailbox 
 ******************************************************************************/
typedef enum  avm_mbx_flex 
{
   AVM_LOG_MBX_CREATE , 
   AVM_LOG_MBX_ATTACH ,
   AVM_LOG_MBX_DESTROY, 
   AVM_LOG_MBX_DETACH ,
   AVM_LOG_MBX_SEND   ,
   AVM_LOG_MBX_SUCCESS,
   AVM_LOG_MBX_FAILURE,
  
}AVM_MBX_FLEX;

/******************************************************************************
         Logging Offset Indices for Task      
 ******************************************************************************/
typedef enum avm_task_flex 
{
   AVM_LOG_TASK_CREATE , 
   AVM_LOG_TASK_START  , 
   AVM_LOG_TASK_RELEASE, 
   AVM_LOG_TASK_SUCCESS, 
   AVM_LOG_TASK_FAILURE
}AVM_TASK_FLEX;

/******************************************************************************
          Logging Offset Indices for Patricia Tree
 ******************************************************************************/
typedef enum avm_pat_flex 
{
   AVM_LOG_PAT_INIT   , 
   AVM_LOG_PAT_ADD    , 
   AVM_LOG_PAT_DEL    ,
   AVM_LOG_PAT_SUCCESS,
   AVM_LOG_PAT_FAILURE
}AVM_PAT_FLEX;

/******************************************************************************
           Logging offset Indices for TMR
 ******************************************************************************/
typedef enum avm_tmr_flex 
{
   AVM_LOG_TMR_CREATED ,     
   AVM_LOG_TMR_STARTED ,    
   AVM_LOG_TMR_EXPIRED ,   
   AVM_LOG_TMR_STOPPED ,  
   AVM_LOG_TMR_DESTROYED,
   AVM_LOG_TMR_SUCCESS , 
   AVM_LOG_TMR_FAILURE
}AVM_TMR_FLEX;

/******************************************************************************
         Logging Offset Indices for Memfail
 ******************************************************************************/
typedef enum avm_mem_flex
{
   AVM_LOG_DEFAULT_ALLOC,
   AVM_LOG_EVT_ALLOC,
   AVM_LOG_ENTITY_PATH_ALLOC,     
   AVM_LOG_INVENTORY_DATA_REC_ALLOC, 
   AVM_LOG_FAULT_DOMAIN_ALLOC,      
   AVM_LOG_ENT_INFO_ALLOC,        
   AVM_LOG_VALID_INFO_ALLOC,
   AVM_LOG_NODE_INFO_ALLOC, 
   AVM_LOG_LIST_NODE_INFO_ALLOC,     
   AVM_LOG_ENT_INFO_LIST_ALLOC, 
   AVM_LOG_PATTERN_ARRAY_ALLOC,
   AVM_LOG_PATTERN_ALLOC,     
   AVM_LOG_MEM_ALLOC_SUCCESS,
   AVM_LOG_MEM_ALLOC_FAILURE 
   
}AVM_MEM_FLEX;

/******************************************************************************
         Logging Offset Indices for Role 
 ******************************************************************************/
typedef enum avm_role_flex 
{
   AVM_LOG_RDA_REG             , 
   AVM_LOG_RDA_GET_ROLE        , 
   AVM_LOG_RDA_UNINSTALL       , 
   AVM_LOG_RDA_SET_ROLE        , 
   AVM_LOG_RDA_AVM_SET_ROLE    , 
   AVM_LOG_RDA_AVM_ROLE        , 
   AVM_LOG_RDA_HB, 
   AVM_LOG_RDA_CBK         , 
   AVM_LOG_SND_ROLE_CHG    ,
   AVM_LOG_SND_ROLE_RSP    , 
   AVM_LOG_RCV_ROLE_RSP    , 
   AVM_LOG_SWOVR_FAILURE   ,
   AVM_LOG_ROLE_CHG    ,      
   AVM_LOG_AVD_ROLE_RSP    ,
   AVM_LOG_ROLE_QUIESCED,
   AVM_LOG_ROLE_ACTIVE,
   AVM_LOG_ROLE_STANDBY,
   AVM_LOG_RDA_SUCCESS     , 
   AVM_LOG_RDA_FAILURE     , 
   AVM_LOG_RDA_IGNORE
}AVM_ROLE_FLEX;

/******************************************************************************
   Logging Offset Indices for Events
******************************************************************************/
typedef enum avm_evt_flex
{
   AVM_LOG_EVT_SRC,                          
   AVM_LOG_EVT_HOTSWAP,                 
   AVM_LOG_EVT_RESOURCE,                 
   AVM_LOG_EVT_SENSOR,                   
   AVM_LOG_EVT_SAME_SENSOR,              
   AVM_LOG_EVT_IGNORE_SENSOR_ASSERT,      
   AVM_LOG_EVT_SENSOR_ASSERT,            
   AVM_LOG_EVT_IGNORE_SENSOR_DEASSERT,   
   AVM_LOG_EVT_SHUTDOWN_RESP,            
   AVM_LOG_EVT_FAILOVER_RESP,           
   AVM_LOG_EVT_RESET_REQ,
   AVM_LOG_EVT_FSM,
   AVM_LOG_EVT_INVALID, 
   AVM_LOG_EVT_IGNORE,
   AVM_LOG_EVT_NONE

}AVM_EVT_FLEX; 

/******************************************************************************
         Logging Offset Indices for Evt Queue 
 ******************************************************************************/
typedef enum avm_evt_q_flex 
{
   AVM_LOG_EVT_Q_ENQ,
   AVM_LOG_EVT_Q_DEQ,
   AVM_LOG_EVT_Q_SUCCESS , 
   AVM_LOG_EVT_Q_FAILURE , 
   AVM_LOG_EVT_Q_IGNORE
}AVM_EVT_Q_FLEX;


/******************************************************************************
 Lopgging Offset Indices for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum avm_flex_sets 
{
   AVM_FC_HDLN,           
   AVM_FC_SEAPI,         
   AVM_FC_CB,          
   AVM_FC_MBX,        
   AVM_FC_PATRICIA, 
   AVM_FC_TASK,     
   AVM_FC_RDE,     
   AVM_FC_MDS,     
   AVM_FC_MAS,    
   AVM_FC_EDA,  
   AVM_FC_TMR,  
   AVM_FC_MBCSV,
   AVM_FC_HPL,  
   AVM_FC_MEM, 
   AVM_FC_ROLE,
   AVM_FC_EVT,
   AVM_FC_EVT_Q
}AVM_FLEX_SETS;

typedef enum avm_log_ids
{
   AVM_LID_HDLN,           
   AVM_LID_HDLN_VAL,      
   AVM_LID_HDLN_STR,  
   AVM_LID_PATRICIA_VAL,  
   AVM_LID_PATRICIA_VAL_STR,  
   AVM_LID_MAS,
   AVM_LID_EDA,
   AVM_LID_TMR,
   AVM_LID_TMR_IGN,              
   AVM_LID_MEM,       
   AVM_LID_ROLE,
   AVM_LID_ROLE_SRC,
   AVM_LID_MBCSV,
   AVM_LID_HPL_API,       
   AVM_LID_FUNC_RETVAL,    
   AVM_LID_HPI_HS_EVT,
   AVM_LID_HPI_RES_EVT,
   AVM_LID_EVT_AVD_DTL,
   AVM_LID_EVT,
   AVM_LID_ENT_INFO,
   AVM_LID_GEN_INFO1,
   AVM_LID_GEN_INFO2,
   AVM_LID_GEN_INFO3,
   AVM_LID_EVT_Q
/*  AVM_LID_EVT_HPI_HS_DTL,     
   AVM_LID_EVT_HPI_SENSOR_DTL, */
}AVM_LOG_IDS;


/* DTSv versioning support */
#define AVM_LOG_VERSION 2

#if (NCS_AVM_LOG == 1)

#define m_AVM_LOG_FUNC_ENTRY(func_name) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN, AVM_FC_HDLN, NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_DEBUG, NCSFL_TYPE_TIC, AVM_ENTERED_FUNC, func_name)

#define m_AVM_LOG_INVALID_VAL_ERROR(data) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN_VAL, AVM_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, NCSFL_TYPE_TICLL, AVM_INVALID_VAL, __FILE__, __LINE__,data)

#define m_AVM_LOG_INVALID_VAL_CRITICAL(data) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN_VAL, AVM_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_CRITICAL, NCSFL_TYPE_TICLL, AVM_INVALID_VAL, __FILE__, __LINE__,data)

#define m_AVM_LOG_INVALID_VAL_FATAL(data) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN_VAL, AVM_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TICLL, AVM_INVALID_VAL, __FILE__, __LINE__,data)


#define m_AVM_LOG_INVALID_STR_ERROR(addrs, length) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = length;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN_STR, AVM_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, "TICLP", AVM_INVALID_VAL, __FILE__, __LINE__,nam_val);\
}

#define m_AVM_LOG_INVALID_STR_FATAL(addrs, length) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = length;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN_STR, AVM_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_EMERGENCY, "TICLP", AVM_INVALID_VAL, __FILE__, __LINE__,nam_val);\
}

#define m_AVM_LOG_SEAPI(option1, option2, severity)    ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_SEAPI, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_CB(option1, option2, severity)       ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_CB, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_MBX(option1, option2, severity)      ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_MBX, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_TASK(option1, option2, severity)     ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_TASK, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_PATRICIA(option1, option2, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_PATRICIA, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_PATRICIA_OP(option1, addrs, length, option2, severity)\
{\
   NCSFL_PDU nam_val;\
   nam_val.len = length;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_PATRICIA_VAL, AVM_FC_PATRICIA, NCSFL_LC_HEADLINE, severity, "TIIP", option1, option2, nam_val);\
}

#define m_AVM_LOG_PATRICIA_OP_STR(option1, addrs, option2, severity)\
{\
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_PATRICIA_VAL_STR, AVM_FC_PATRICIA, NCSFL_LC_HEADLINE, severity, "TIIC", option1, option2, addrs);\
}

#define m_AVM_LOG_MDS(option1, option2, severity)      ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_MDS, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_RDE(option1, option2, severity)      ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_RDE, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_MAS(option1, option2, severity)      ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_MAS, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_MAS_VAL(id, option1, option2, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_MAS, AVM_FC_MAS, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TIIL, option1, option2)

#define m_AVM_LOG_EDA(option1, option2, severity)      ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_EDA, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_HPL(option1, option2, severity)      ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_HPL, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_EVT_SRC(src, severity)               ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN, AVM_FC_EVT, NCSFL_LC_MISC, severity, NCSFL_TYPE_TIC, AVM_LOG_EVT_SRC, src) 

#define m_AVM_LOG_EVT_IGN(option, val1, val2, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_EVT, AVM_FC_EVT, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TILL, option, val1, val2)

#define m_AVM_LOG_EVT_INVALID(option, val1, val2, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_EVT, AVM_FC_EVT, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TILL, option, val1, val2)

#define m_AVM_LOG_EVT(option, str, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN, AVM_FC_EVT, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TIC, option, str);

#define m_AVM_LOG_FSM_EVT(option, val1, val2, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_EVT, AVM_FC_EVT, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TILL, option, val1, val2);

#define m_AVM_LOG_HPI_HS_EVT(option, val1, val2, addrs) \
{\
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HPI_HS_EVT, AVM_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TICLL", option, addrs, val1, val2); \
}

#define m_AVM_LOG_HPI_RES_EVT(option, val1, addrs) \
{\
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HPI_RES_EVT, AVM_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TCIL", addrs, option, val1); \
}

#define m_AVM_LOG_GEN(str, addrs, length, severity) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = length;\
   nam_val.dump = (char *)addrs; \
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_GEN_INFO2, AVM_FC_EVT, NCSFL_LC_FUNC_RET_FAIL, severity, "TCP", str, nam_val);\
}

#define m_AVM_LOG_GEN_EP_STR(str, addrs, severity) \
{\
   ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_GEN_INFO3, AVM_FC_EVT, NCSFL_LC_FUNC_RET_FAIL, severity, "TCC", str, addrs);\
}

#define m_AVM_LOG_DEBUG(str, severity)                    ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_GEN_INFO1, AVM_FC_EVT, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TC, str)

#define m_AVM_LOG_TMR(option1, period, option2, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_TMR, AVM_FC_TMR, NCSFL_LC_TIMER, severity, NCSFL_TYPE_TIIL, option1, option2, period)

#define m_AVM_LOG_TMR_IGN(option, str, severity)          ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_HDLN, AVM_FC_TMR, NCSFL_LC_TIMER, severity, NCSFL_TYPE_TIC, option, str)

#define m_AVM_LOG_EDA(option1, option2, severity)         ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_EDA, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_EDA_VAL(ret_val, option1, option2, severity) ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_EDA, AVM_FC_EDA, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TIIL, option1, option2, ret_val)

#define m_AVM_LOG_MEM(option1, option2, severity)              ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_MEM, AVM_FC_MEM, NCSFL_LC_MEMORY, severity, NCSFL_TYPE_TIICL, option1, option2, __FILE__, __LINE__)

#define m_AVM_LOG_ROLE(option1, option2, severity)              ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_ROLE, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_ROLE_OP(option1, option2, severity)              ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_ROLE, AVM_FC_ROLE, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TIL, option1, option2)

#define m_AVM_LOG_ROLE_SRC(option1, option2, option3, severity)              ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_ROLE_SRC, AVM_FC_ROLE, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TILL, option1, option2, option3)

#define m_AVM_LOG_CKPT_OP(option1, option2, severity)              ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_FUNC_RETVAL, AVM_FC_MBCSV, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TII, option1, option2)

#define m_AVM_LOG_CKPT_EVT(option1, option2, option3, severity)              ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_MBCSV, AVM_FC_MBCSV, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TIIL, option1, option2, option3)

#define m_AVM_LOG_CKPT_EVT_Q(option1, option2, option3, severity)              ncs_logmsg(NCS_SERVICE_ID_AVM, AVM_LID_EVT_Q, AVM_FC_EVT_Q, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TILL, option1, option2, option3)

#else /* NCS_AVM_LOG == 1 */

#define m_AVM_LOG_FUNC_ENTRY(func_name)
#define m_AVM_LOG_INVALID_VAL_ERROR(data)
#define m_AVM_LOG_INVALID_VAL_CRITICAL(data)
#define m_AVM_LOG_INVALID_VAL_FATAL(data)
#define m_AVM_LOG_INVALID_STR_ERROR(addrs, length) 
#define m_AVM_LOG_INVALID_STR_FATAL(addrs, length) 
#define m_AVM_LOG_SEAPI(option1, option2, severity)
#define m_AVM_LOG_CB(option1, option2, severity) 
#define m_AVM_LOG_MBX(option1, option2, severity)
#define m_AVM_LOG_TASK(option1, option2, severity)
#define m_AVM_LOG_PATRICIA(option1, option2, severity)
#define m_AVM_LOG_PATRICIA_OP(option1, addrs, length, option2, severity)
#define m_AVM_LOG_MDS(option1, option2, severity)
#define m_AVM_LOG_MAS(option1, option2, severity)
#define m_AVM_LOG_MAS_VAL(id, option1, option2, severity)
#define m_AVM_LOG_EDA(option1, option2, severity)
#define m_AVM_LOG_EDA_VAL(ret_val, option1, option2, severity)
#define m_AVM_LOG_HPL(option1, option2, severity)
#define m_AVM_LOG_EVT_SRC(src, severity)
#define m_AVM_LOG_EVT_IGN(option, val1, val2, severity)
#define m_AVM_LOG_EVT_INVALID(option, val1, val2, severity)
#define m_AVM_LOG_EVT(option, str, severity)
#define m_AVM_LOG_FSM_EVT(option, val1, val2, severity)
#define m_AVM_LOG_HPI_HS_EVT(option, val1, val2, addrs)
#define m_AVM_LOG_HPI_RES_EVT(option, addrs, val1)
#define m_AVM_LOG_GEN(str, addrs, length, severity)
#define m_AVM_LOG_DEBUG(str, severity)
#define m_AVM_LOG_TMR(option1, period, option2, severity)
#define m_AVM_LOG_TMR_IGN(option, str, severity)
#define m_AVM_LOG_MEM(option1, option2, severity)
#define m_AVM_LOG_ROLE(option1, option2, severity)
#define m_AVM_LOG_ROLE_OP(option1, option2, severity)
#define m_AVM_LOG_CKPT_OP(option1, option2, severity) 
#define m_AVM_LOG_CKPT_EVT(option1, option2, option3, severity)  

#endif /* NCS_AVM_LOG == 1 */


#if (NCS_AVM_LOG == 1)

/* registers the AvM logging with DTA */
EXTERN_C void avm_flx_log_reg(void);

/* unregisters the AvM logging with DTS */
EXTERN_C void avm_flx_log_dereg(void);

#endif /* (NCS_AVM_LOG == 1) */


#endif /* __AVM_DB_LOG_H__ */

