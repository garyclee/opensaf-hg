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

  MODULE NAME: IFSV_LOG.H

$Header: 
..............................................................................

  DESCRIPTION: Defines structures and defn of IFSV log file


******************************************************************************/

#ifndef IFSV_LOG_H
#define IFSV_LOG_H

/*****************************************************************************
 * Macros used to log the events/errors.
 *****************************************************************************/

/* Common Macro logging for IfSv */
#if (NCS_IFSV_LOG == 1)

#define  NCS_IFSV_SEV_INFO  NCSFL_SEV_INFO
#define  NCS_IFSV_SEV_DEBUG  NCSFL_SEV_DEBUG

/* By default all logs of IFSV will be of severity NCSFL_SEV_INFO. FLS doesn't
 * log this severity by default and enabling log for this severity via CLI is 
 * not possible right now. So, if user want to see the logs, then remove #if 0
 * and compile it again as FLS logs of severity NCSFL_SEV_NOTICE. The same stands
 * for VIP also.
 * */ 
#if 0
#define  NCS_IFSV_SEV_INFO  NCSFL_SEV_NOTICE
#define  NCS_IFSV_SEV_DEBUG NCSFL_SEV_NOTICE
#endif
#define m_IFSV_LOG_EVT_L(svc,indx,info1,info2)   ifsv_flx_log_TICL_fmt(svc,NCSFL_LC_EVENT,IFSV_LC_EVENT_L,NCS_IFSV_SEV_INFO,IFSV_LFS_EVENT_L,indx,info1,(uns32)info2)
#define m_IFSV_LOG_EVT_LL(svc,indx,info1,info2,info3)   ifsv_flx_log_TICLL_fmt(svc,NCSFL_LC_EVENT,IFSV_LC_EVENT_LL,NCS_IFSV_SEV_INFO,IFSV_LFS_EVENT_LL,indx,info1,(uns32)info2,(uns32)info3)
#define m_IFSV_LOG_EVT_LLL(svc,indx,info1,info2,info3,info4)   ifsv_flx_log_TICLLL_fmt(svc,NCSFL_LC_EVENT,IFSV_LC_EVENT_LLL,NCS_IFSV_SEV_INFO,IFSV_LFS_EVENT_LLL,indx,info1,(uns32)info2,(uns32)info3,(uns32)info4)
#define m_IFSV_LOG_EVT_INIT(svc,indx,info)        ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_EVENT,IFSV_LC_EVENT_INIT,NCS_IFSV_SEV_INFO,IFSV_LFS_EVENT_INIT,indx,(uns32)info)
#define m_IFSV_LOG_EVT_IFINDEX(svc,indx,info)        ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_EVENT,IFSV_LC_EVENT_IFINDEX,NCS_IFSV_SEV_INFO,IFSV_LFS_EVENT_IFINDEX,indx,(uns32)info)


#define m_IFSV_LOG_FUNC_ENTRY_L(svc,indx,info)   ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_FUNC_ENTRY,IFSV_LC_FUNC_ENTRY_L,NCS_IFSV_SEV_INFO,IFSV_LFS_FUNC_ENTRY_L,indx,(uns32)info)
#define m_IFSV_LOG_FUNC_ENTRY_LL(svc,indx,info1,info2)   ifsv_flx_log_TILL_fmt(svc,NCSFL_LC_FUNC_ENTRY,IFSV_LC_FUNC_ENTRY_LL,NCS_IFSV_SEV_INFO,IFSV_LFS_FUNC_ENTRY_LL,indx,(uns32)info1,(uns32)info2)
#define m_IFSV_LOG_FUNC_ENTRY_LLL(svc,indx,info1,info2,info3)   ifsv_flx_log_TILLL_fmt(svc,NCSFL_LC_FUNC_ENTRY,IFSV_LC_FUNC_ENTRY_LLL,NCS_IFSV_SEV_INFO,IFSV_LFS_FUNC_ENTRY_LLL,indx,(uns32)info1,(uns32)info2,(uns32)info3)
#define m_IFSV_LOG_FUNC_ENTRY_LLLL(svc,indx,info1,info2,info3,info4)   ifsv_flx_log_TILLLL_fmt(svc,NCSFL_LC_FUNC_ENTRY,IFSV_LC_FUNC_ENTRY_LLLL,NCS_IFSV_SEV_INFO,IFSV_LFS_FUNC_ENTRY_LLLL,indx,(uns32)info1,(uns32)info2,(uns32)info3,(uns32)info4)
#define m_IFSV_LOG_FUNC_ENTRY_LLLLL(svc,indx,info1,info2,info3,info4,info5)   ifsv_flx_log_TILLLLL_fmt(svc,NCSFL_LC_FUNC_ENTRY,IFSV_LC_FUNC_ENTRY_LLLLL,NCS_IFSV_SEV_INFO,IFSV_LFS_FUNC_ENTRY_LLLLL,indx,(uns32)info1,(uns32)info2,(uns32)info3,(uns32)info4,(uns32)info5)
#define m_IFSV_LOG_FUNC_ENTRY_LLLLLL(svc,indx,info1,info2,info3,info4,info5,info6)   ifsv_flx_log_TILLLLLL_fmt(svc,IFSV_LFS_FUNC_ENTRY_LLLLLL,IFSV_LC_FUNC_ENTRY_LLLLLL,NCSFL_LC_FUNC_ENTRY,NCS_IFSV_SEV_INFO,NCSFL_TYPE_TILLLLLL,indx,(uns32)info1,(uns32)info2,(uns32)info3,(uns32)info4,(uns32)info5,(uns32)info6)

#define m_IFSV_LOG_API_L(svc,indx,info)   ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_API,IFSV_LC_API_L,NCS_IFSV_SEV_INFO,IFSV_LFS_API_L,indx,(uns32)info)
#define m_IFSV_LOG_API_LL(svc,indx,info1,info2)   ifsv_flx_log_TILL_fmt(svc,NCSFL_LC_API,IFSV_LC_API_LL,NCS_IFSV_SEV_INFO,IFSV_LFS_API_LL,indx,(uns32)info1,(uns32)info2)
#define m_IFSV_LOG_API_LLL(svc,indx,info1,info2,info3)   ifsv_flx_log_TILLL_fmt(svc,NCSFL_LC_API,IFSV_LC_API_LLL,NCS_IFSV_SEV_INFO,IFSV_LFS_API_LLL,(uns32)indx,(uns32)info1,(uns32)info2,(uns32)info3)
#define m_IFSV_LOG_API_LLLL(svc,indx,info1,info2,info3,info4)   ifsv_flx_log_TILLLL_fmt(svc,NCSFL_LC_API,IFSV_LC_API_LLLL,NCS_IFSV_SEV_INFO,IFSV_LFS_API_LLLL,indx,(uns32)info1,(uns32)info2,(uns32)info3,(uns32)info4)
#define m_IFSV_LOG_API_LLLLL(svc,indx,info1,info2,info3,info4,info5)   ifsv_flx_log_TILLLLL_fmt(svc,NCSFL_LC_API,IFSV_LC_API_LLLLL,NCS_IFSV_SEV_INFO,IFSV_LFS_API_LLLLL,indx,(uns32)info1,(uns32)info2,(uns32)info3,(uns32)info4,(uns32)info5)


#define m_IFSV_LOG_MEMORY(svc,indx,info)        ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_MEMORY,IFSV_LC_MEMORY,NCS_IFSV_SEV_INFO,IFSV_LFS_MEMORY,indx,(uns32)info)
#define m_IFSV_LOG_LOCK(svc,indx,info)          ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_LOCKS,IFSV_LC_LOCKS,NCS_IFSV_SEV_INFO,IFSV_LFS_LOCKS,indx,(uns32)info)
#define m_IFSV_LOG_SYS_CALL_FAIL(svc,indx,info) ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_SYS_CALL_FAIL,IFSV_LC_SYS_CALL_FAIL,NCSFL_SEV_EMERGENCY,IFSV_LFS_SYS_CALL_FAIL,indx,(uns32)info)
#define m_IFSV_LOG_TMR(svc,indx,info)        ifsv_flx_log_TIL_fmt(svc,NCSFL_LC_TIMER,IFSV_LC_TIMER,NCS_IFSV_SEV_INFO,IFSV_LFS_TIMER,indx,(uns32)info)

/* To log success cases */
#define m_IFSV_LOG_NORMAL(svc,indx,info1,info2)  ifsv_flx_log_TILL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE,NCSFL_SEV_NOTICE,IFSV_LFS_HEADLINE,indx,(uns32)info1,(uns32)info2)



/* To log a string along with a long value */
#define m_IFSV_LOG_STR_NORMAL(svc,indx,info1,info2)   ifsv_flx_log_TICL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE,NCSFL_SEV_NOTICE,IFSV_LFS_LOG_STR_ERROR,indx,info1,(uns32)info2)

/* To log the message as error..*/
#define m_IFSV_LOG_ERROR(svc,indx,info1,info2)   ifsv_flx_log_TICL_fmt(svc,NCSFL_LC_HEADLINE,NCSFL_LC_HEADLINE,NCSFL_SEV_ERROR,IFSV_LFS_LOG_STR_ERROR,indx,info1,(uns32)info2)

/* To log two strings */
#define m_IFSV_LOG_STR_2_NORMAL(svc,indx,info1,info2)   ifsv_flx_log_TCIC_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE,NCSFL_SEV_NOTICE,IFSV_LFS_LOG_STR_2_ERROR,indx,info1,info2)

/* To log a string along with 7 long values */
#define m_IFSV_LOG_FUNC_ENTRY_INFO(svc,indx,info1,info2,info3,info4,info5,info6,info7,info8) ifsv_flx_log_TICLLLLLLL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE_CLLLLLLL,NCSFL_SEV_NOTICE,IFSV_LFS_REQ_ORIG_INFO,indx,info1,(uns32)info2,(uns32)info3,(uns32)info4,(uns32)info5,(uns32)info6,(uns32)info7,(uns32)info8) 
#define m_IFSV_LOG_FUNC_ENTRY_CRITICAL_INFO(svc,indx,info1,info2,info3,info4,info5,info6,info7,info8) ifsv_flx_log_TICLLLLLLL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE_CLLLLLLL,NCSFL_SEV_CRITICAL,IFSV_LFS_REQ_ORIG_INFO,indx,info1,(uns32)info2,(uns32)info3,(uns32)info4,(uns32)info5,(uns32)info6,(uns32)info7,(uns32)info8) 



/* Most of the error cases are under this macro. */
#define m_IFSV_LOG_HEAD_LINE(svc,indx,info1,info2)  ifsv_flx_log_TILL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE,NCSFL_SEV_NOTICE,IFSV_LFS_HEADLINE,indx,(uns32)info1,(uns32)info2)
#define m_IFSV_LOG_DEL_IF_REC(svc,indx,info1,info2)  ifsv_flx_log_TILL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE,NCSFL_SEV_NOTICE,IFSV_LFS_DEL_REC,indx,(uns32)info1,(uns32)info2)



#define m_IFSV_LOG_HEAD_LINE_NORMAL(svc,indx,info1,info2)  ifsv_flx_log_TILL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE,NCSFL_SEV_INFO,IFSV_LFS_HEADLINE,indx,(uns32)info1,(uns32)info2)
#define m_IFSV_LOG_ADD_MOD_IF_REC(svc,indx,info1,info2,info3,info4,info5,info6)  ifsv_flx_log_TICLLLLL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE_CLLLLL,NCS_IFSV_SEV_INFO,IFSV_LFS_ADD_MOD_REC,indx,info1,(uns32)info2,(uns32)info3,(uns32)info4,(uns32)info5,(uns32)info6)
#define m_IFSV_LOG_IF_STATS_INFO(svc,indx,info1,info2,info3,info4,info5,info6)  ifsv_flx_log_TICLLLLL_fmt(svc,NCSFL_LC_HEADLINE,IFSV_LC_HEADLINE_CLLLLL,NCS_IFSV_SEV_INFO,IFSV_LFS_IF_STAT_INFO,indx,info1,(uns32)info2,(uns32)info3,(uns32)info4,(uns32)info5,(uns32)info6)

/* New defines for VIP */
#if (NCS_VIP == 1)
#define m_IFSV_VIP_LOG_MESG(svc_id, str_id)  ncs_logmsg(svc_id, IFSV_VIP_FLC_API, IFSV_VIP_LC_API, IFSV_VIP_LC_API, NCS_IFSV_SEV_DEBUG, "TI", str_id)
#endif

#else
   /*** here log is diabled ***/

#define m_IFSV_LOG_EVT_L(svc,indx,info1,info2)   
#define m_IFSV_LOG_EVT_LL(svc,indx,info1,info2,info3)   
#define m_IFSV_LOG_EVT_LLL(svc,indx,info1,info2,info3,info4)   
#define m_IFSV_LOG_EVT_INIT(svc,indx,info)        
#define m_IFSV_LOG_EVT_IFINDEX(svc,indx,info)     


#define m_IFSV_LOG_FUNC_ENTRY_L(svc,indx,info)   
#define m_IFSV_LOG_FUNC_ENTRY_LL(svc,indx,info1,info2)   
#define m_IFSV_LOG_FUNC_ENTRY_LLL(svc,indx,info1,info2,info3)   
#define m_IFSV_LOG_FUNC_ENTRY_LLLL(svc,indx,info1,info2,info3,info4)   
#define m_IFSV_LOG_FUNC_ENTRY_LLLLL(svc,indx,info1,info2,info3,info4,info5)   
#define m_IFSV_LOG_FUNC_ENTRY_LLLLLL(svc,indx,info1,info2,info3,info4,info5,info6)   

#define m_IFSV_LOG_API_L(svc,indx,info)   
#define m_IFSV_LOG_API_LL(svc,indx,info1,info2)   
#define m_IFSV_LOG_API_LLL(svc,indx,info1,info2,info3)   
#define m_IFSV_LOG_API_LLLL(svc,indx,info1,info2,info3,info4)   
#define m_IFSV_LOG_API_LLLLL(svc,indx,info1,info2,info3,info4,info5)   


#define m_IFSV_LOG_MEMORY(svc,indx,info)        
#define m_IFSV_LOG_LOCK(svc,indx,info)          
#define m_IFSV_LOG_SYS_CALL_FAIL(svc,indx,info) 
#define m_IFSV_LOG_TMR(svc,indx,info)        
#define m_IFSV_LOG_HEAD_LINE(svc,indx,info1,info2)  
#define m_IFSV_LOG_HEAD_LINE_NORMAL(svc,indx,info1,info2)  
#define m_IFSV_LOG_DEL_IF_REC(svc,indx,info1,info2)  
#define m_IFSV_LOG_ADD_MOD_IF_REC(svc,indx,info1,info2,info3,info4,info5,info6)  
#define m_IFSV_LOG_IF_STATS_INFO(svc,indx,info1,info2,info3,info4,info5,info6)  

/* New defines for VIP */
#if (NCS_VIP == 1)
#define m_IFSV_VIP_LOG_MESG(svc_id, str_id)  
#endif

#endif

#define m_IFA_LOG_STR_NORMAL(indx,info1,info2) m_IFSV_LOG_STR_NORMAL(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_ERROR(indx,info1,info2) m_IFSV_LOG_ERROR(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_STR_2_NORMAL(indx,info1,info2) m_IFSV_LOG_STR_2_NORMAL(NCS_SERVICE_ID_IFA,indx,info1,info2)

#define m_IFA_LOG_EVT_L(indx,info1,info2)   m_IFSV_LOG_EVT_L(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_EVT_LL(indx,info1,info2,info3)   m_IFSV_LOG_EVT_LL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3)
#define m_IFA_LOG_EVT_LLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_EVT_LLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4)
#define m_IFA_LOG_EVT_INIT(indx,info)        m_IFSV_LOG_EVT_INIT(NCS_SERVICE_ID_IFA,indx,info)
#define m_IFA_LOG_EVT_IFINDEX(indx,info)        m_IFSV_LOG_EVT_IFINDEX(NCS_SERVICE_ID_IFA,indx,info)


#define m_IFA_LOG_FUNC_ENTRY_L(indx,info)    m_IFSV_LOG_FUNC_ENTRY_L(NCS_SERVICE_ID_IFA,indx,info)
#define m_IFA_LOG_FUNC_ENTRY_LL(indx,info1,info2)   m_IFSV_LOG_FUNC_ENTRY_LL(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_FUNC_ENTRY_LLL(indx,info1,info2,info3)   m_IFSV_LOG_FUNC_ENTRY_LLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3)
#define m_IFA_LOG_FUNC_ENTRY_LLLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_FUNC_ENTRY_LLLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4)
#define m_IFA_LOG_FUNC_ENTRY_LLLLL(indx,info1,info2,info3,info4,info5)   m_IFSV_LOG_FUNC_ENTRY_LLLLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4,info5)
#define m_IFA_LOG_FUNC_ENTRY_LLLLLL(indx,info1,info2,info3,info4,info5,info6)   m_IFSV_LOG_FUNC_ENTRY_LLLLLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4,info5,info6)

#define m_IFA_LOG_API_L(indx,info)   m_IFSV_LOG_API_L(NCS_SERVICE_ID_IFA,indx,info)
#define m_IFA_LOG_API_LL(indx,info1,info2)   m_IFSV_LOG_API_LL(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_API_LLL(indx,info1,info2,info3)   m_IFSV_LOG_API_LLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3)
#define m_IFA_LOG_API_LLLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_API_LLLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4)
#define m_IFA_LOG_API_LLLLL(indx,info1,info2,info3,info4,info5)   m_IFSV_LOG_API_LLLLL(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4,info5)



#define m_IFA_LOG_MEMORY(indx,info)        m_IFSV_LOG_MEMORY(NCS_SERVICE_ID_IFA,indx,info)
#define m_IFA_LOG_LOCK(indx,info)          m_IFSV_LOG_LOCK(NCS_SERVICE_ID_IFA,indx,info)
#define m_IFA_LOG_SYS_CALL_FAIL(indx,info) m_IFSV_LOG_SYS_CALL_FAIL(NCS_SERVICE_ID_IFA,indx,info)
#define m_IFA_LOG_TMR(indx,info)        m_IFSV_LOG_TMR(NCS_SERVICE_ID_IFA,indx,info)
#define m_IFA_LOG_HEAD_LINE(indx,info1,info2)  m_IFSV_LOG_HEAD_LINE(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_HEAD_LINE_NORMAL(indx,info1,info2)  m_IFSV_LOG_HEAD_LINE_NORMAL(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_DEL_IF_REC(indx,info1,info2)  m_IFSV_LOG_DEL_IF_REC(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFA_LOG_ADD_MOD_IF_REC(indx,info1,info2,info3,info4,info5,info6)  m_IFSV_LOG_ADD_MOD_IF_REC(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4,info5,info6)
#define m_IFA_LOG_IF_STATS_INFO(indx,info1,info2,info3,info4,info5,info6)  m_IFSV_LOG_IF_STATS_INFO(NCS_SERVICE_ID_IFA,indx,info1,info2,info3,info4,info5,info6)


/* Macros used to log all IFND events */
#define m_IFND_LOG_STR_NORMAL(indx,info1,info2) m_IFSV_LOG_STR_NORMAL(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_ERROR(indx,info1,info2) m_IFSV_LOG_ERROR(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_STR_2_NORMAL(indx,info1,info2) m_IFSV_LOG_STR_2_NORMAL(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_FUNC_ENTRY_INFO(indx,info1,info2,info3,info4,info5,info6,info7,info8) m_IFSV_LOG_FUNC_ENTRY_INFO(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4,info5,info6,info7,info8)


#define m_IFND_LOG_EVT_L(indx,info1,info2)   m_IFSV_LOG_EVT_L(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_EVT_LL(indx,info1,info2,info3)   m_IFSV_LOG_EVT_LL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3)
#define m_IFND_LOG_EVT_LLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_EVT_LLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4)
#define m_IFND_LOG_EVT_INIT(indx,info)        m_IFSV_LOG_EVT_INIT(NCS_SERVICE_ID_IFND,indx,info)
#define m_IFND_LOG_EVT_IFINDEX(indx,info)        m_IFSV_LOG_EVT_IFINDEX(NCS_SERVICE_ID_IFND,indx,info)


#define m_IFND_LOG_FUNC_ENTRY_L(indx,info)    m_IFSV_LOG_FUNC_ENTRY_L(NCS_SERVICE_ID_IFND,indx,info)
#define m_IFND_LOG_FUNC_ENTRY_LL(indx,info1,info2)   m_IFSV_LOG_FUNC_ENTRY_LL(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_FUNC_ENTRY_LLL(indx,info1,info2,info3)   m_IFSV_LOG_FUNC_ENTRY_LLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3)
#define m_IFND_LOG_FUNC_ENTRY_LLLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_FUNC_ENTRY_LLLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4)
#define m_IFND_LOG_FUNC_ENTRY_LLLLL(indx,info1,info2,info3,info4,info5)   m_IFSV_LOG_FUNC_ENTRY_LLLLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4,info5)
#define m_IFND_LOG_FUNC_ENTRY_LLLLLL(indx,info1,info2,info3,info4,info5,info6)   m_IFSV_LOG_FUNC_ENTRY_LLLLLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4,info5,info6)

#define m_IFND_LOG_API_L(indx,info)   m_IFSV_LOG_API_L(NCS_SERVICE_ID_IFND,indx,info)
#define m_IFND_LOG_API_LL(indx,info1,info2)   m_IFSV_LOG_API_LL(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_API_LLL(indx,info1,info2,info3)   m_IFSV_LOG_API_LLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3)
#define m_IFND_LOG_API_LLLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_API_LLLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4)
#define m_IFND_LOG_API_LLLLL(indx,info1,info2,info3,info4,info5)   m_IFSV_LOG_API_LLLLL(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4,info5)



#define m_IFND_LOG_MEMORY(indx,info)        m_IFSV_LOG_MEMORY(NCS_SERVICE_ID_IFND,indx,info)
#define m_IFND_LOG_LOCK(indx,info)          m_IFSV_LOG_LOCK(NCS_SERVICE_ID_IFND,indx,info)
#define m_IFND_LOG_SYS_CALL_FAIL(indx,info) m_IFSV_LOG_SYS_CALL_FAIL(NCS_SERVICE_ID_IFND,indx,info)
#define m_IFND_LOG_TMR(indx,info)        m_IFSV_LOG_TMR(NCS_SERVICE_ID_IFND,indx,info)
#define m_IFND_LOG_HEAD_LINE(indx,info1,info2)  m_IFSV_LOG_HEAD_LINE(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_HEAD_LINE_NORMAL(indx,info1,info2)  m_IFSV_LOG_HEAD_LINE_NORMAL(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_DEL_IF_REC(indx,info1,info2)  m_IFSV_LOG_DEL_IF_REC(NCS_SERVICE_ID_IFND,indx,info1,info2)
#define m_IFND_LOG_ADD_MOD_IF_REC(indx,info1,info2,info3,info4,info5,info6)  m_IFSV_LOG_ADD_MOD_IF_REC(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4,info5,info6)
#define m_IFND_LOG_IF_STATS_INFO(indx,info1,info2,info3,info4,info5,info6)  m_IFSV_LOG_IF_STATS_INFO(NCS_SERVICE_ID_IFND,indx,info1,info2,info3,info4,info5,info6)




/* Macros used to Log all IFD events */
#define m_IFD_LOG_STR_NORMAL(indx,info1,info2) m_IFSV_LOG_STR_NORMAL(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFD_LOG_ERROR(indx,info1,info2) m_IFSV_LOG_ERROR(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFD_LOG_STR_2_NORMAL(indx,info1,info2) m_IFSV_LOG_STR_2_NORMAL(NCS_SERVICE_ID_IFA,indx,info1,info2)
#define m_IFD_LOG_FUNC_ENTRY_INFO(indx,info1,info2,info3,info4,info5,info6,info7,info8) m_IFSV_LOG_FUNC_ENTRY_INFO(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4,info5,info6,info7,info8)
#define m_IFD_LOG_FUNC_ENTRY_CRITICAL_INFO(indx,info1,info2,info3,info4,info5,info6,info7,info8) m_IFSV_LOG_FUNC_ENTRY_CRITICAL_INFO(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4,info5,info6,info7,info8)


#define m_IFD_LOG_EVT_L(indx,info1,info2)   m_IFSV_LOG_EVT_L(NCS_SERVICE_ID_IFD,indx,info1,info2)
#define m_IFD_LOG_EVT_LL(indx,info1,info2,info3)   m_IFSV_LOG_EVT_LL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3)
#define m_IFD_LOG_EVT_LLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_EVT_LLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4)
#define m_IFD_LOG_EVT_INIT(indx,info)        m_IFSV_LOG_EVT_INIT(NCS_SERVICE_ID_IFD,indx,info)
#define m_IFD_LOG_EVT_IFINDEX(indx,info)        m_IFSV_LOG_EVT_IFINDEX(NCS_SERVICE_ID_IFD,indx,info)


#define m_IFD_LOG_FUNC_ENTRY_L(indx,info)    m_IFSV_LOG_FUNC_ENTRY_L(NCS_SERVICE_ID_IFD,indx,info)
#define m_IFD_LOG_FUNC_ENTRY_LL(indx,info1,info2)   m_IFSV_LOG_FUNC_ENTRY_LL(NCS_SERVICE_ID_IFD,indx,info1,info2)
#define m_IFD_LOG_FUNC_ENTRY_LLL(indx,info1,info2,info3)   m_IFSV_LOG_FUNC_ENTRY_LLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3)
#define m_IFD_LOG_FUNC_ENTRY_LLLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_FUNC_ENTRY_LLLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4)
#define m_IFD_LOG_FUNC_ENTRY_LLLLL(indx,info1,info2,info3,info4,info5)   m_IFSV_LOG_FUNC_ENTRY_LLLLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4,info5)
#define m_IFD_LOG_FUNC_ENTRY_LLLLLL(indx,info1,info2,info3,info4,info5,info6)   m_IFSV_LOG_FUNC_ENTRY_LLLLLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4,info5,info6)

#define m_IFD_LOG_API_L(indx,info)   m_IFSV_LOG_API_L(NCS_SERVICE_ID_IFD,indx,info)
#define m_IFD_LOG_API_LL(indx,info1,info2)   m_IFSV_LOG_API_LL(NCS_SERVICE_ID_IFD,indx,info1,info2)
#define m_IFD_LOG_API_LLL(indx,info1,info2,info3)   m_IFSV_LOG_API_LLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3)
#define m_IFD_LOG_API_LLLL(indx,info1,info2,info3,info4)   m_IFSV_LOG_API_LLLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4)
#define m_IFD_LOG_API_LLLLL(indx,info1,info2,info3,info4,info5)   m_IFSV_LOG_API_LLLLL(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4,info5)


#define m_IFD_LOG_MEMORY(indx,info)        m_IFSV_LOG_MEMORY(NCS_SERVICE_ID_IFD,indx,info)
#define m_IFD_LOG_LOCK(indx,info)          m_IFSV_LOG_LOCK(NCS_SERVICE_ID_IFD,indx,info)
#define m_IFD_LOG_SYS_CALL_FAIL(indx,info) m_IFSV_LOG_SYS_CALL_FAIL(NCS_SERVICE_ID_IFD,indx,info)
#define m_IFD_LOG_TMR(indx,info)        m_IFSV_LOG_TMR(NCS_SERVICE_ID_IFD,indx,info)
#define m_IFD_LOG_HEAD_LINE(indx,info1,info2)  m_IFSV_LOG_HEAD_LINE(NCS_SERVICE_ID_IFD,indx,info1,info2)
#define m_IFD_LOG_HEAD_LINE_NORMAL(indx,info1,info2)  m_IFSV_LOG_HEAD_LINE_NORMAL(NCS_SERVICE_ID_IFD,indx,info1,info2)
#define m_IFD_LOG_DEL_IF_REC(indx,info1,info2)  m_IFSV_LOG_DEL_IF_REC(NCS_SERVICE_ID_IFD,indx,info1,info2)
#define m_IFD_LOG_ADD_MOD_IF_REC(indx,info1,info2,info3,info4,info5,info6)  m_IFSV_LOG_ADD_MOD_IF_REC(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4,info5,info6)
#define m_IFD_LOG_IF_STATS_INFO(indx,info1,info2,info3,info4,info5,info6)  m_IFSV_LOG_IF_STATS_INFO(NCS_SERVICE_ID_IFD,indx,info1,info2,info3,info4,info5,info6)




/*****************************************************************************
 * Bit mask for IfSv logging.
 *****************************************************************************/
#define IFSV_FLEX_LOG_UNREGISTER    (0x00)
#define IFSV_FLEX_LOG_REGISTER      (0x01)


/*****************************************************************************
 * Enumerated type all the IfSv related log sets.
 *****************************************************************************/
typedef enum ifsv_log_sets
{
   IFSV_LC_EVENT_L,   
   IFSV_LC_EVENT_LL,
   IFSV_LC_EVENT_LLL,
   IFSV_LC_EVENT_INIT,
   IFSV_LC_EVENT_IFINDEX,
   IFSV_LC_MEMORY,
   IFSV_LC_LOCKS,
   IFSV_LC_TIMER,
   IFSV_LC_SYS_CALL_FAIL,   
   IFSV_LC_HEADLINE,
   IFSV_LC_HEADLINE_CL,
   IFSV_LC_HEADLINE_CLLLLL,
   IFSV_LC_FUNC_ENTRY_L,
   IFSV_LC_FUNC_ENTRY_LL,
   IFSV_LC_FUNC_ENTRY_LLL,
   IFSV_LC_FUNC_ENTRY_LLLL,
   IFSV_LC_FUNC_ENTRY_LLLLL,
   IFSV_LC_FUNC_ENTRY_LLLLLL,
   IFSV_LC_API_L,
   IFSV_LC_API_LL,
   IFSV_LC_API_LLL,
   IFSV_LC_API_LLLL,
   IFSV_LC_API_LLLLL,
#if (NCS_VIP == 1)
   IFSV_VIP_LC_API,
#endif
   IFSV_LC_HEADLINE_CLLLLLLL   
}IFSV_LOG_SETS;


/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_EVENT".
 *****************************************************************************/
typedef enum ifsv_evt_flex
{
   /* IfD Card Received Events */
   IFSV_LOG_IFD_EVT_INTF_CREATE_RCV,
   IFSV_LOG_IFD_EVT_INTF_DESTROY_RCV,
   IFSV_LOG_IFD_EVT_INIT_DONE_RCV,   
   IFSV_LOG_IFD_EVT_IFINDEX_REQ_RCV,
   IFSV_LOG_IFD_EVT_IFINDEX_CLEANUP_RCV,
   IFSV_LOG_IFD_EVT_AGING_TMR_RCV,
   IFSV_LOG_IFD_EVT_RET_TMR_RCV,
   IFSV_LOG_IFD_EVT_REC_SYNC_RCV,
   IFSV_LOG_IFD_EVT_SVC_UPD_RCV,
   IFSV_LOG_IFD_EVT_SVC_UPD_FAILURE,
   IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_RCV,
   IFSV_LOG_IFD_EVT_A2S_SYNC_RESP_RCV,
   /* IfD Card Send Events*/
   IFSV_LOG_IFD_EVT_INTF_CREATE_SND,
   IFSV_LOG_IFD_EVT_INTF_DESTROY_SND,
   IFSV_LOG_IFD_EVT_INIT_DONE_SND,
   IFSV_LOG_IFD_EVT_AGING_TMR_SND,
   IFSV_LOG_IFD_EVT_IFINDEX_RESP_SND,
   IFSV_LOG_IFD_EVT_ALL_REC_SND,
   IFSV_LOG_IFD_EVT_A2S_ASYNC_UPDATE_SND,

   /* IfND Card Receive Events */
   IFSV_LOG_IFND_EVT_INTF_CREATE_RCV,
   IFSV_LOG_IFND_EVT_INTF_DESTROY_RCV,
   IFSV_LOG_IFND_EVT_INIT_DONE_RCV,   
   IFSV_LOG_IFND_EVT_IFINDEX_RESP_RCV,   
   IFSV_LOG_IFND_EVT_INTF_INFO_GET_RCV,
   IFSV_LOG_IFND_EVT_STATS_RESP_RCV,
   IFSV_LOG_IFND_EVT_AGING_TMR_RCV,
   IFSV_LOG_IFND_EVT_MDS_DEST_GET_TMR_RCV,
   IFSV_LOG_IFND_EVT_SVCD_UPD_FROM_IFA_RCV,
   IFSV_LOG_IFND_EVT_SVCD_UPD_FROM_IFD_RCV,
   IFSV_LOG_IFND_EVT_SVCD_GET_REQ_RCV,
   IFSV_LOG_IFND_EVT_IFD_UP_RCV,
   /* IfND Card Send Events */
   IFSV_LOG_IFND_EVT_INTF_CREATE_SND,
   IFSV_LOG_IFND_EVT_INTF_DESTROY_SND,
   IFSV_LOG_IFND_EVT_INIT_DONE_SND, 
   IFSV_LOG_IFND_EVT_IFINDEX_REQ_SND,
   IFSV_LOG_IFND_EVT_IFINDEX_CLEANUP_SND,
   IFSV_LOG_IFND_EVT_AGING_TMR_SND,
   IFSV_LOG_IFND_EVT_REC_SYNC_SND,
   IFSV_LOG_IFND_EVT_IDIM_HEALTH_CHK_SND,
   IFSV_LOG_IFND_EVT_GET_HW_STATS_SND,
   IFSV_LOG_IFND_EVT_SET_HW_PARAM_SND,
   /* IDIM Event receive */
   IFSV_LOG_IDIM_EVT_HW_STATS_RCV,
   IFSV_LOG_IDIM_EVT_HW_PORT_REG_RCV,
   IFSV_LOG_IDIM_EVT_HW_PORT_STATUS_RCV,
   IFSV_LOG_IDIM_EVT_GET_HW_STATS_RCV,
   IFSV_LOG_IDIM_EVT_SET_HW_PARAM_RCV,
   IFSV_LOG_IDIM_EVT_HEALTH_CHK_RCV,
   /* IDIM Event Send */
   IFSV_LOG_IDIM_EVT_GET_HW_DRV_STATS_SND,
   IFSV_LOG_IDIM_EVT_SET_HW_DRV_PARAM_SND,
   IFSV_LOG_IDIM_EVT_IFND_UP_SND,
   IFSV_LOG_IDIM_EVT_INTF_CREATE_SND,
   IFSV_LOG_IDIM_EVT_INTF_DESTROY_SND,
   IFSV_LOG_IDIM_EVT_INTF_STATS_RESP_SND,
   IFSV_LOG_MBCSV_MSG,   
   /* IFA Event */
   IFSV_LOG_IFA_EVT_INFO,
   IFSV_LOG_IFA_IPXS_EVT_INFO,
   IFSV_LOG_IFND_IPXS_EVT_INFO,
   IFSV_LOG_IFD_IPXS_EVT_INFO,
   IFSV_LOG_UNKNOWN_EVENT,
   IFSV_LOG_IFSV_CREATE_MOD_MSG,
   IFSV_LOG_IFD_MSG,
   IFSV_LOG_IFND_EVT_LOG_MAX   
}IFSV_EVT_FLEX;

/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_MEMORY".
 *****************************************************************************/
typedef enum ifsv_mem_flex
{
   /* memory failure*/   
   IFSV_LOG_MEM_ALLOC,
   IFSV_LOG_MEM_FREE,   
   IFSV_MEM_FAIL_LOG_MAX
} IFSV_MEM_FLEX;

/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_LOCKS".
 *****************************************************************************/
typedef enum ifsv_lock_fail_flex
{
   /* memory failure*/
   IFSV_LOG_LOCK_CREATE,
   IFSV_LOG_LOCK_DESTROY,
   IFSV_LOG_LOCK_LOCKED,
   IFSV_LOG_LOCK_UNLOCKED,   
   IFSV_LOG_MEM_FAIL_LOG_MAX
} IFSV_LOCK_FAIL_FLEX;

/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_SYS_CALL_FAIL".
 *****************************************************************************/
typedef enum ifsv_sys_call_fail_flex
{  
   IFSV_LOG_MSG_QUE_CREATE_FAIL,
   /* message queue send failure */
   IFSV_LOG_MSG_QUE_SEND_FAIL,
   /* message queue receive failure */
   IFSV_LOG_MSG_QUE_RCV_FAIL,
   /* task create failure */
   IFSV_LOG_TASK_CREATE_FAIL,
   /* Semaphore create failure*/
   IFSV_LOG_SEM_CREATE_FAIL,
   IFSV_LOG_SEM_TAKE_FAIL,
   IFSV_LOG_SEM_GIVE_FAIL,
   IFSV_LOG_LOCK_LOCKED_FAIL,
   IFSV_LOG_LOCK_UNLOCKED_FAIL,
   IFSV_LOG_LOCK_CREATE_FAIL,
   IFSV_LOG_LOCK_DESTROY_FAIL,    
   IFSV_LOG_MEM_ALLOC_FAIL,
   IFSV_LOG_MEM_FREE_FAIL,
   IFSV_LOG_HDL_MGR_FAIL,
   IFSV_RESOURCE_FAIL_LOG_MAX
} IFSV_sys_call_fail_FLEX;


/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_TIMER".
 *****************************************************************************/
typedef enum ifsv_tmr_flex
{
   IFSV_LOG_TMR_CREATE,
   IFSV_LOG_TMR_START,
   IFSV_LOG_TMR_STOP,
   IFSV_LOG_TMR_DELETE,
   IFSV_LOG_TMR_EXPIRY,
   IFSV_LOG_TMR_FLEX_LOG_MAX
} IFSV_TMR_FLEX;

/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_HEADLINE".
 *****************************************************************************/
typedef enum ifsv_headline_flex
{
   IFSV_LOG_IF_TBL_CREATED,
   IFSV_LOG_IF_TBL_DESTROYED,
   IFSV_LOG_IF_MAP_TBL_CREATED,
   IFSV_LOG_IFND_NODE_ID_TBL_CREATED,
   IFSV_LOG_IF_MAP_TBL_DESTROYED,   
   IFSV_LOG_IF_TBL_CREATE_FAILURE,
   IFSV_LOG_IF_MAP_TBL_CREATE_FAILURE,
   IFSV_LOG_IFND_NODE_ID_TBL_CREATE_FAILURE,
   IFSV_LOG_IF_TBL_DESTROY_FAILURE,
   IFSV_LOG_IF_MAP_TBL_DESTROY_FAILURE,   
   IFSV_LOG_IF_TBL_ADD_SUCCESS,
   IFSV_LOG_IF_TBL_UPDATE,
   IFSV_LOG_IF_MAP_TBL_ADD_SUCCESS,   
   IFSV_LOG_IF_TBL_ADD_FAILURE,
   IFSV_LOG_IF_MAP_TBL_ADD_FAILURE,
   IFSV_LOG_IF_TBL_DEL_SUCCESS,
   IFSV_LOG_IF_MAP_TBL_DEL_SUCCESS,
   IFSV_LOG_IF_MAP_TBL_DEL_ALL_SUCCESS,
   IFSV_LOG_IF_TBL_DEL_FAILURE,
   IFSV_LOG_IF_MAP_TBL_DEL_FAILURE,
   IFSV_LOG_IF_TBL_DEL_ALL_SUCCESS,
   IFSV_LOG_IF_TBL_DEL_ALL_FAILURE,
   IFSV_LOG_IF_TBL_LOOKUP_SUCCESS,      
   IFSV_LOG_IF_TBL_ALREADY_EXIST,      
   IFSV_LOG_IF_MAP_TBL_LOOKUP_SUCCESS,
   IFSV_LOG_IF_TBL_LOOKUP_FAILURE,   
   IFSV_LOG_SPT_TO_INDEX_LOOKUP_FAILURE,
   IFSV_LOG_SPT_TO_INDEX_LOOKUP_SUCCESS,
   IFSV_LOG_IF_MAP_TBL_LOOKUP_FAILURE,   
   IFSV_LOG_IFINDEX_ALLOC_SUCCESS,
   IFSV_LOG_IFINDEX_ALLOC_FAILURE,
   IFSV_LOG_IFINDEX_FREE_SUCCESS,
   IFSV_LOG_IFINDEX_FREE_FAILURE,
   IFSV_LOG_ADD_REC_TO_IF_RES_QUE,
   IFSV_LOG_REM_REC_TO_IF_RES_QUE,
   IFSV_LOG_ADD_MOD_INTF_REC,
   IFSV_LOG_DEL_INTF_REC,
   IFSV_LOG_IF_STATS_INFO,
   IFSV_LOG_MDS_SVC_UP,
   IFSV_LOG_IFD_RED_DOWN,
   IFSV_LOG_IFD_RED_DOWN_EVENT,
   IFSV_LOG_MDS_SVC_DOWN,
   IFSV_LOG_MDS_RCV_MSG,
   IFSV_LOG_MDS_SEND_MSG,
   IFSV_LOG_MDS_DEC_SUCCESS,
   IFSV_LOG_MDS_DEC_FAILURE,
   IFSV_LOG_MDS_ENC_SUCCESS,
   IFSV_LOG_MDS_ENC_FAILURE,
   IFSV_LOG_STATE_INFO,
   IFSV_LOG_MDS_DEST_TBL_ADD_FAILURE,
   IFSV_LOG_MDS_DEST_TBL_DEL_FAILURE,
   IFSV_LOG_SVCD_UPD_SUCCESS,
   IFSV_LOG_NODE_ID_TBL_ADD_FAILURE,
   IFSV_LOG_NODE_ID_TBL_DEL_FAILURE,
   IFSV_LOG_NODE_ID_TBL_ADD_SUCCESS,
   IFSV_LOG_NODE_ID_TBL_DEL_SUCCESS,
   IFSV_LOG_IFIP_NODE_ID_TBL_ADD_SUCCESS,
   IFSV_LOG_IFIP_NODE_ID_TBL_DEL_SUCCESS,
   IFSV_LOG_IFIP_NODE_ID_TBL_ADD_FAILURE,
   IFSV_LOG_IFIP_NODE_ID_TBL_DEL_FAILURE,
   IFSV_LOG_IP_TBL_ADD_SUCCESS,
   IFSV_LOG_IP_TBL_ADD_FAILURE,
   IFSV_LOG_IP_TBL_DEL_SUCCESS,
   IFSV_LOG_IFSV_CREATE_MSG,
   IFSV_LOG_IFSV_CB_NULL,
   IFSV_LOG_IFSV_HA_QUIESCED_MSG,
   IFSV_LOG_IFSV_HA_ACTIVE_IN_WARM_COLD_MSG,
   IFSV_LOG_IFSV_HA_ACTIVE_MSG,
   IFSV_LOG_IFSV_HA_STDBY_MSG,
   IFSV_WARM_COLD_SYNC_START,
   IFSV_WARM_COLD_SYNC_STOP,
   IFSV_LOG_FUNC_RET_FAIL,
   IFSV_AMF_RESP_AFTER_COLD_SYNC,
   IFSV_LOG_HEADLINE_FLEX_LOG_MAX
} IFSV_HEADLINE_FLEX;

/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_FUNC_ENTRY".
 *****************************************************************************/
typedef enum ifsv_func_entry
{
   IFSV_LOG_FUNC_ENTERED,
   IFSV_LOG_FUNC_ENTRY_MAX
} IFSV_FUNC_ENTRY;

/*****************************************************************************
 * Different logging events for the LOG mask "NCSFL_LC_API".
 *****************************************************************************/
typedef enum ifsv_api_flex
{
   IFSV_LOG_SE_INIT_DONE,
   IFSV_LOG_SE_INIT_FAILURE,
   IFSV_LOG_SE_DESTROY_DONE,
   IFSV_LOG_SE_DESTROY_FAILURE,
   IFSV_LOG_CB_INIT_DONE,
   IFSV_LOG_CB_INIT_FAILURE,
   IFSV_LOG_CB_DESTROY_DONE,
   IFSV_LOG_CB_DESTROY_FAILURE,
   IFSV_LOG_MDS_INIT_DONE,
   IFSV_LOG_MDS_INIT_FAILURE,
   IFSV_LOG_MDS_DESTROY_DONE,
   IFSV_LOG_MDS_DESTROY_FAILURE,
   IFSV_LOG_RMS_INIT_DONE,
   IFSV_LOG_RMS_INIT_FAILURE,
   IFSV_LOG_RMS_DESTROY_DONE,
   IFSV_LOG_RMS_DESTROY_FAILURE,
   IFSV_LOG_AMF_INIT_DONE,
   IFSV_LOG_AMF_INIT_FAILURE,
   IFSV_LOG_AMF_REG_DONE,
   IFSV_LOG_AMF_REG_FAILURE,
   IFSV_LOG_AMF_HEALTH_CHK_START_DONE,
   IFSV_LOG_AMF_HEALTH_CHK_START_FAILURE,
   IFSV_LOG_AMF_HA_STATE_CHNG,
   IFSV_LOG_AMF_READY_STATE_CHNG,
   IFSV_LOG_AMF_HEALTH_CHK,
   IFSV_LOG_AMF_CONF_OPER,
   IFSV_LOG_AMF_GET_OBJ_FAILURE,
   IFSV_LOG_AMF_DISP_FAILURE,
   IFSV_LOG_IDIM_INIT_DONE,
   IFSV_LOG_IDIM_INIT_FAILURE,
   IFSV_LOG_IDIM_DESTROY_DONE,
   IFSV_LOG_IDIM_DESTROY_FAILURE,
   IFSV_LOG_DRV_INIT_DONE,
   IFSV_LOG_DRV_INIT_FAILURE,
   IFSV_LOG_DRV_DESTROY_DONE,
   IFSV_LOG_DRV_DESTROY_FAILURE,
   IFSV_LOG_DRV_PORT_REG_DONE,
   IFSV_LOG_DRV_PORT_REG_FAILURE,
   IFSV_LOG_API_LOG_MAX
} IFSV_API_FLEX;

/*****************************************************************************
 * Different logging events for the LOG mask "IFSV_VIP_LC_API".
 *****************************************************************************/
#if (NCS_VIP == 1)
typedef enum ifsv_vip_api_flex
{
  IFSV_VIP_NULL_POOL_HDL,
  IFSV_VIP_NULL_APPL_NAME,
  IFSV_INTERFACE_REQUESTED_ADMIN_DOWN,
  IFSV_VIP_INVALID_INTERFACE_RECEIVED,
  IFSV_VIP_MDS_SEND_FAILURE,
  IFSV_VIP_VIPDC_DATABASE_CREATED,
  IFSV_VIP_VIPD_DATABASE_CREATED,
  IFSV_VIP_VIPDC_DATABASE_CREATE_FAILED,
  IFSV_VIP_VIPD_DATABASE_CREATE_FAILED,
  IFSV_VIP_VIPD_LOOKUP_SUCCESS,
  IFSV_VIP_VIPD_LOOKUP_FAILED,
  IFSV_VIP_VIPDC_LOOKUP_SUCCESS,
  IFSV_VIP_VIPDC_LOOKUP_FAILED,
  IFSV_VIP_ADDED_IP_NODE_TO_VIPDC,
  IFSV_VIP_ADDED_IP_NODE_TO_VIPD,
  IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPDC,
  IFSV_VIP_ADDED_INTERFACE_NODE_TO_VIPD,
  IFSV_VIP_ADDED_OWNER_NODE_TO_VIPDC,
  IFSV_VIP_ADDED_OWNER_NODE_TO_VIPD,

  IFSV_VIP_RECORD_ADDITION_TO_VIPDC_SUCCESS,
  IFSV_VIP_RECORD_ADDITION_TO_VIPDC_FAILURE,
  IFSV_VIP_RECORD_ADDITION_TO_VIPD_SUCCESS,
  IFSV_VIP_RECORD_ADDITION_TO_VIPD_FAILURE,

  IFSV_VIP_RECEIVED_REQUEST_FROM_SAME_IFA,
  IFSV_VIP_RECEIVED_VIP_EVENT,
  IFSV_VIP_INTERNAL_ERROR,
  IFSV_VIP_INSTALLING_VIP_AND_SENDING_GARP,

  IFSV_VIP_VIPDC_CLEAR_FAILED,
  IFSV_VIP_VIPD_CLEAR_FAILED,

  IFSV_VIP_LINK_LIST_NODE_DELETION_FAILED,
  IFSV_VIP_RECORD_DELETION_FAILED,
  IFSV_VIP_REQUESTED_VIP_ALREADY_EXISTS,
  IFSV_VIP_MEM_ALLOC_FAILED,

  IFSV_VIP_EXISTS_ON_OTHER_INTERFACE,
  IFSV_VIP_EXISTS_FOR_OTHER_HANDLE,
  IFSV_VIP_IPXS_TABLE_LOOKUP_FAILED,

  IFSV_VIP_INVALID_PARAM,


/*
  VIP MIB LOW LEVEL ROUTINES RELATED
*/
  IFSV_VIP_GET_NEXT_INITIALLY_NO_RECORD_FOUND,
  IFSV_VIP_GET_NEXT_INITIALLY_RECORD_FOUND,
  IFSV_VIP_GET_NEXT_NO_RECORD_FOUND,
  IFSV_VIP_GET_NEXT_RECORD_FOUND,
  IFSV_VIP_GET_NEXT_INVALID_INDEX_RECEIVED,
  IFSV_VIP_GET_NO_RECORD_FOUND,
  IFSV_VIP_GET_RECORD_FOUND,
  IFSV_VIP_GET_INVALID_INDEX_RECEIVED,
  IFSV_VIP_GET_NULL_INDEX_RECEIVED,


/*
  VIP CLI RELATED
*/
  IFSV_VIP_CLI_MIB_SYNC_REQ_CALL_FAILED,
  IFSV_VIP_CLI_SINGLE_ENTRY_DISPLAY,
  IFSV_VIP_CLI_DISPLAY_ALL_ENTRIES,
  IFSV_VIP_CLI_INVALID_INDEX_RECEIVED,
  IFSV_VIP_CLI_SHOW_ALL_FAILED,
  IFSV_VIP_CLI_DONE_FAILURE,
  IFSV_VIP_CLI_INVALID_APPL_NAME_INPUT_TO_CLI,
  IFSV_VIP_CLI_INVALID_HANDLE_INPUT_TO_CLI,
  IFSV_VIP_SINGLE_ENTRY_SHOW_CEF_FAILED,
  IFSV_VIP_CLI_MODE_CHANGE_CEF_SUCCESS,

/*
  VIP MIBLIB && OAC Registration RELATED
*/
  IFSV_VIP_MIBLIB_REGISTRATION_SUCCESS,
  IFSV_VIP_MIBLIB_REGISTRATION_FAILURE,
  IFSV_VIP_OAC_REGISTRATION_SUCCESS,
  IFSV_VIP_OAC_REGISTRATION_FAILURE,
  IFSV_VIP_MIB_TABLE_REGISTRATION_SUCCESS,
  IFSV_VIP_MIB_TABLE_REGISTRATION_FAILURE,

  IFSV_VIP_VIRTUAL_IP_ADDRESS_INSTALL_FAILED,
  IFSV_VIP_VIRTUAL_IP_ADDRESS_INSTALL_SUCCESS,
  IFSV_VIP_VIRTUAL_IP_ADDRESS_FREE_FAILED,
  IFSV_VIP_VIRTUAL_IP_ADDRESS_FREE_SUCCESS,
  IFSV_VIP_RECIVED_VIP_INSTALL_REQUEST,
  IFSV_VIP_SENDING_IFA_VIPD_INFO_ADD_REQ,
  IFSV_VIP_RECEIVED_ERROR_RESP_FOR_INSTALL,
  IFSV_VIP_RECEIVED_ERROR_RESP_FOR_FREE,
  IFSV_VIP_SENDING_IFA_VIP_FREE_REQ,
  IFSV_VIP_RECEIVED_IFA_VIP_FREE_REQ,
  IFSV_VIP_RECEIVED_IFA_VIPD_INFO_ADD_REQ,
  IFSV_VIP_SENDING_IFND_VIPD_INFO_ADD_REQ,
  IFSV_VIP_RECEIVED_IFND_VIPD_INFO_ADD_REQ,
  IFSV_VIP_RECEIVED_ERROR_RESP_FOR_VIPD_INFO_ADD,
  IFSV_VIP_SENDING_IPXS_ADD_IP_REQ,
  IFSV_VIP_SENDING_IPXS_INC_REFCNT_REQ,
  IFSV_VIP_RECEIVED_REQUEST_FROM_DIFFERENT_IFA,
  IFSV_VIP_SENDING_IFND_VIPD_INFO_ADD_REQ_RESP,
  IFSV_VIP_SENDING_IFND_VIP_FREE_REQ,
  IFSV_VIP_RECEIVED_IFND_VIP_FREE_REQ,
  IFSV_VIP_SENDING_IFND_VIP_FREE_REQ_RESP,
  IFSV_VIP_RECEIVED_IFA_CRASH_MSG,
  IFSV_VIP_RECORD_DELETION_SUCCESS,
  IFSV_VIP_SENDING_IFD_VIP_FREE_REQ_RESP,
  IFSV_VIP_SENDING_IFD_VIPD_INFO_ADD_REQ_RESP,

  IFSV_VIP_CREATED_IFND_IFND_VIP_DEL_VIPD_EVT,
  IFSV_VIP_RECEIVED_IFND_IFND_VIP_DEL_VIPD_EVT,
  IFSV_VIP_VIPDC_RECORD_DELETION_FAILED,
  IFSV_VIP_VIPD_RECORD_DELETION_FAILED,
  IFSV_VIP_VIPDC_RECORD_DELETION_SUCCESS,
  IFSV_VIP_VIPD_RECORD_DELETION_SUCCESS,
  IFSV_VIP_SENDING_IPXS_DEL_IP_REQ,
  IFSV_VIP_SENDING_IPXS_DEC_REFCNT_REQ,
  IFSV_VIP_INTF_REC_NOT_CREATED,
  IFSV_VIP_IP_EXISTS_ON_OTHER_INTERFACE,
  IFSV_VIP_IP_EXISTS_FOR_OTHER_HANDLE,
  IFSV_VIP_IP_EXISTS_BUT_NOT_VIP,
  IFSV_VIP_UNINSTALLING_VIRTUAL_IP,

  IFSV_VIP_API_LOG_MAX

} IFSV_VIP_API_FLEX;
#endif


/*****************************************************************************
 * Different logging format ID's.
 *****************************************************************************/
typedef enum ifsv_frmt_ids
{
   IFSV_LFS_EVENT_L,
   IFSV_LFS_EVENT_LL,
   IFSV_LFS_EVENT_LLL,
   IFSV_LFS_EVENT_INIT,
   IFSV_LFS_EVENT_IFINDEX,
   IFSV_LFS_MEMORY,
   IFSV_LFS_LOCKS,
   IFSV_LFS_TIMER,
   IFSV_LFS_SYS_CALL_FAIL,
   IFSV_LFS_HEADLINE,
   IFSV_LFS_ADD_MOD_REC,
   IFSV_LFS_DEL_REC,
   IFSV_LFS_IF_STAT_INFO,
   IFSV_LFS_FUNC_ENTRY_L,
   IFSV_LFS_FUNC_ENTRY_LL,
   IFSV_LFS_FUNC_ENTRY_LLL,
   IFSV_LFS_FUNC_ENTRY_LLLL,
   IFSV_LFS_FUNC_ENTRY_LLLLL,
   IFSV_LFS_FUNC_ENTRY_LLLLLL,
   IFSV_LFS_API_L,
   IFSV_LFS_API_LL,
   IFSV_LFS_API_LLL,
   IFSV_LFS_API_LLLL,
   IFSV_LFS_API_LLLLL,
   IFSV_VIP_FLC_API,
   IFSV_LFS_LOG_STR_2_ERROR,
   IFSV_LFS_LOG_STR_ERROR,
   IFSV_LFS_REQ_ORIG_INFO,  
} IFSV_FRMT_IDS;



/*****************************************************************************
 * Structure which holds the logging information.
 *****************************************************************************/
typedef struct ifsv_log_info
{
   struct ifsv_cb  *ifsv_cb;   /* Pointer to Ifsv control block */
   char            *file;      /* Log File */
   char            *pStr;      /* string to be passed */
   uns32           mask;       /* Log Mask */
   uns32           comp;       /* PIM Component */
   uns32           val;        /* numeric value to be logged */
   uns32           cat;        /* Logging category of that component */
   uns8            sev;        /* Severity */
   uns8            evt;        /* Event type */   
} IFSV_LOG_INFO;

void 
ifsv_flx_log_evt(uns32 scv_id, uns8 log_frmt, NCSFL_TYPE log_frmt_type, ...);
void 
ifsv_flx_log_tmr(uns32 scv_id, uns8 log_frmt, NCSFL_TYPE log_frmt_type, ...);
void 
ifsv_flx_log_head_line(uns32 scv_id, uns8 log_frmt, NCSFL_TYPE log_frmt_type, ...);
void 
ifsv_flx_log_func_entry(uns32 scv_id, uns8 log_frmt, NCSFL_TYPE log_frmt_type, ...);
void 
ifsv_flx_log_api(uns32 scv_id, uns8 log_frmt, NCSFL_TYPE log_frmt_type, ...);
void 
ifsv_flx_log_lock(uns32 scv_id, uns32 indx, uns32 lock_info);
void 
ifsv_flx_log_sys_call_fail(uns32 scv_id, uns32 indx, uns32 sys_call_info);
void 
ifsv_flx_log_mem(uns32 scv_id, uns32 indx, uns32 mem_info);

void
ifsv_flx_log_TCIC_fmt (uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
             uns32 fmt_id, uns32 indx, uns8 *info1, uns8 *info2);

void
ifsv_flx_log_TICL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                      uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2);

void
ifsv_flx_log_TICLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                       uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                       uns32 info3);

void
ifsv_flx_log_TICLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                        uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                        uns32 info3, uns32 info4);

void
ifsv_flx_log_TICLLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                          uns32 fmt_id, uns32 indx, uns8 *info1, uns32 info2,
                          uns32 info3, uns32 info4, uns32 info5, uns32 info6);


void
ifsv_flx_log_TIL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                     uns32 fmt_id, uns32 indx, uns32 info);

void
ifsv_flx_log_TILL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, 
                      uns32 sev, uns32 fmt_id, uns32 indx, uns32 info1,
                      uns32 info2);

void
ifsv_flx_log_TILLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                       uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                       uns32 info3);

void
ifsv_flx_log_TILLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                        uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                        uns32 info3, uns32 info4);

void
ifsv_flx_log_TILLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                         uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                         uns32 info3, uns32 info4, uns32 info5);

void
ifsv_flx_log_TILLLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                         uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                         uns32 info3, uns32 info4, uns32 info5, uns32 info6);

void
ifsv_flx_log_TILLLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                         uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2, 
                         uns32 info3, uns32 info4, uns32 info5, uns32 info6);
void
ifsv_flx_log_TILLLLLLL_fmt(uns32 scv_id, uns32 cat, uns32 str_set, uns32 sev, 
                          uns32 fmt_id, uns32 indx, uns32 info1, uns32 info2,
                          uns32 info3, uns32 info4, uns32 info5, uns32 info6,
                          uns32 info7);


uns32 ifsv_flx_log_reg (uns32 comp_type);

void ifsv_flx_log_dereg (uns32 comp_type);

NCS_SERVICE_ID 
ifsv_log_svc_id_from_comp_type (uns32 comp_type);

char *
ifsv_log_spt_string (NCS_IFSV_SPT spt, char *o_spt_str);

void m_IFSV_LOG_SPT_INFO(uns32 index, NCS_IFSV_SPT_MAP *spt_map,uns8 *info2);


/* DTSv versioning support */
#define IFSV_LOG_VERSION 1

#endif /*#ifndef IFSV_LOG_H*/

