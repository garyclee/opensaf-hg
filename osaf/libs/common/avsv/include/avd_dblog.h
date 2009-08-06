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

  This module is the include file for Availability Directors debug logging.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_DBLOG_H
#define AVD_DBLOG_H

#include "avsv_log.h"

/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/
typedef enum avd_hdln_flex {
	AVD_INVALID_VAL,
	AVD_UNKNOWN_MSG_RCVD,
	AVD_MSG_PROC_FAILED,
	AVD_ENTERED_FUNC,
	AVD_RCVD_VAL
} AVD_HDLN_FLEX;

/******************************************************************************
 Logging offset indexes for Memory Fail logging 
 ******************************************************************************/

typedef enum avd_memfail_flex {
	AVD_CB_ALLOC_FAILED,
	AVD_MBX_ALLOC_FAILED,
	AVD_AVND_ALLOC_FAILED,
	AVD_COMP_ALLOC_FAILED,
	AVD_CSI_ALLOC_FAILED,
	AVD_COMPCSI_ALLOC_FAILED,
	AVD_SG_ALLOC_FAILED,
	AVD_SI_ALLOC_FAILED,
	AVD_SUSI_ALLOC_FAILED,
	AVD_SU_ALLOC_FAILED,
	AVD_PG_CSI_NODE_ALLOC_FAILED,
	AVD_PG_NODE_CSI_ALLOC_FAILED,
	AVD_EVT_ALLOC_FAILED,
	AVD_HLT_ALLOC_FAILED,
	AVD_DND_MSG_ALLOC_FAILED,
	AVD_D2D_MSG_ALLOC_FAILED,
	AVD_CSI_PARAM_ALLOC_FAILED,
	AVD_DND_MSG_INFO_ALLOC_FAILED,
	AVD_SG_OPER_ALLOC_FAILED,
	AVD_SU_PER_SI_RANK_ALLOC_FAILED,
	AVD_SG_SI_RANK_ALLOC_FAILED,
	AVD_SG_SU_RANK_ALLOC_FAILED,
	AVD_COMP_CS_TYPE_ALLOC_FAILED,
	AVD_CS_TYPE_PARAM_ALLOC_FAILED,
	AVD_TRAP_VAR_BIND_ALLOC_FAILED
} AVD_MEMFAIL_FLEX;

/******************************************************************************
 Logging offset indexes for Message logging 
 ******************************************************************************/
typedef enum avd_msg_flex {
	AVD_LOG_PROC_MSG,
	AVD_LOG_RCVD_MSG,
	AVD_LOG_SND_MSG,
	AVD_LOG_DND_MSG,
	AVD_LOG_D2D_MSG,
	AVD_DUMP_DND_MSG,
	AVD_DUMP_D2D_MSG
} AVD_MSG_FLEX;

/******************************************************************************
 Logging offset indexes for Proxy-Proxied logging
 ******************************************************************************/
typedef enum avd_log_pxy_pxd_flex {
	AVD_PXY_PXD_SUCC_INFO,
	AVD_PXY_PXD_ERR_INFO,
	AVD_PXY_PXD_ENTRY_INFO
} AVD_LOG_PXY_PXD_FLEX;

/******************************************************************************
 Logging offset indexes for event logging 
 ******************************************************************************/
typedef enum avd_evt_flex {
	AVD_SND_TMR_EVENT,
	AVD_SND_AVD_MSG_EVENT,
	AVD_SND_AVND_MSG_EVENT,
	AVD_SND_MAB_EVENT,
	AVD_RCVD_EVENT,
	AVD_RCVD_INVLD_EVENT
} AVD_EVT_FLEX;

/******************************************************************************
 Logging offset indexes for Checkpoint event logging 
 ******************************************************************************/
typedef enum avd_ckpt_flex {
	/* AVD Role change events */
	AVD_ROLE_CHANGE_ATOS,
	AVD_ROLE_CHANGE_STOA,
	AVD_ROLE_CHANGE_ATOQ,
	AVD_ROLE_CHANGE_QTOS,
	AVD_ROLE_CHANGE_QTOA,
	AVD_MBCSV_MSG_ASYNC_UPDATE,
	AVD_COLD_SYNC_REQ_RCVD,
	AVD_MBCSV_MSG_COLD_SYNC_RESP,
	AVD_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE,
	AVD_MBCSV_MSG_WARM_SYNC_REQ,
	AVD_MBCSV_MSG_WARM_SYNC_RESP,
	AVD_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE,
	AVD_MBCSV_MSG_DATA_REQ,
	AVD_MBCSV_MSG_DATA_RESP,
	AVD_MBCSV_MSG_DATA_RESP_COMPLETE,
	AVD_MBCSV_ERROR_IND,
	/* Some failure events now */
	AVD_MBCSV_MSG_WARM_SYNC_RESP_FAILURE,
	AVD_MBCSV_MSG_DATA_RSP_DECODE_FAILURE,
	AVD_MBCSV_MSG_DISPATCH_FAILURE,
	AVD_STBY_UNAVAIL_FOR_RCHG,
	AVD_ROLE_CHANGE_FAILURE,
	AVD_HB_MSG_SND_FAILURE,

	/* AVD HB loss event */
	AVD_HB_MISS_WITH_PEER,

} AVD_CKPT_FLEX;

/******************************************************************************
 Logging offset inexes for traps gen.
 ******************************************************************************/
typedef enum avd_trap_flex {
	AVD_TRAP_AMF,
	AVD_TRAP_CLM,
	AVD_TRAP_CLUSTER,
	AVD_TRAP_UNASSIGNED,
	AVD_TRAP_JOINED,
	AVD_TRAP_EXITED,
	AVD_TRAP_RECONFIGURED,
	AVD_TRAP_EDA_INIT_FAILED,
	AVD_TRAP_EDA_CHNL_OPEN_FAILED,
	AVD_TRAP_EDA_EVT_PUBLISH_FAILED,
	AVD_TRAP_NCS_INIT_SUCCESS,

} AVD_TRAP_FLEX;

/******************************************************************************
 Logging offset inexes for oper state
 ******************************************************************************/
typedef enum avd_oper_state_flex {
	AVD_TRAP_OPER_STATE_MIN,
	AVD_TRAP_OPER_STATE_ENABLE,
	AVD_TRAP_OPER_STATE_DISABLE
} AVD_OPER_STATE_FLEX;

/******************************************************************************
 Logging offset inexes for admin state
 ******************************************************************************/
typedef enum avd_admin_state_flex {
	AVD_TRAP_ADMIN_STATE_MIN,
	AVD_TRAP_ADMIN_STATE_LOCK,
	AVD_TRAP_ADMIN_STATE_UNLOCK,
	AVD_TRAP_ADMIN_STATE_SHUTDOWN
} AVD_ADMIN_STATE_FLEX;

typedef enum avd_ha_state_flex {
	AVD_TRAP_HA_NONE,
	AVD_TRAP_HA_ACTIVE,
	AVD_TRAP_HA_STANDBY,
	AVD_TRAP_HA_QUIESCED,
	AVD_TRAP_HA_QUIESCING,

} AVD_HA_STATE_FLEX;

/******************************************************************************
 Logging offset indexes for shutdown failure trap
 ******************************************************************************/

typedef enum avd_shutdown_failure_flex {
	AVD_TRAP_NODE_ACTIVE_SYS_CTRL,
	AVD_TRAP_SUS_SAME_SG,
	AVD_TRAP_SG_UNSTABLE,
} AVD_SHUTDOWN_FAILURE_FLEX;

/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum avd_flex_sets {
	AVD_FC_HDLN,
	AVD_FC_MEMFAIL,
	AVD_FC_MSG,
	AVD_FC_EVT,
	AVSV_FC_SEAPI,
	AVSV_FC_MDS,
	AVSV_FC_LOCK,
	AVSV_FC_MBX,
	AVSV_FC_CKPT,
	AVD_FC_TRAP,
	AVD_FC_OPER,
	AVD_FC_ADMIN,
	AVD_FC_SUSI_HA,
	AVD_FC_PXY_PXD,
	AVD_FC_SHUTDOWN_FAILURE,
	AVD_FC_GENLOG,
} AVD_FLEX_SETS;

typedef enum avd_log_ids {
	AVD_LID_HDLN,
	AVD_LID_HDLN_VAL,
	AVD_LID_HDLN_VAL_NAME,
	AVD_LID_MEMFAIL,
	AVD_LID_MEMFAIL_LOC,
	AVD_LID_MSG_INFO,
	AVD_LID_MSG_DND_DTL,
	AVD_LID_FUNC_RETVAL,
	AVD_LID_EVT_VAL,
	AVD_LID_EVT_CKPT,
	AVD_LID_ADMIN,
	AVD_LID_SI_UNASSIGN,
	AVD_LID_OPER,
	AVD_LID_SUSI_HA,
	AVD_LID_CLM,
	AVD_LID_TRAP_EVT,
	AVD_LID_TRAP_NCS_SUCC,
	AVD_LID_SUSI_HA_CHG_START,
	AVD_LID_HDLN_SVAL,
	AVD_PXY_PXD,
	AVD_LID_SHUTDOWN_FAILURE,
	AVD_LID_GENLOG,
} AVD_LOG_IDS;

#if (NCS_AVD_LOG == 1)

#define avd_log(severity, format, args...) _avd_log((severity), __FUNCTION__, (format), ##args)
#define avd_trace(format, args...) _avd_trace(__FILE__, __LINE__, (format), ##args)

#define m_AVD_LOG_FUNC_ENTRY(func_name) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN, AVD_FC_HDLN, NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_DEBUG, NCSFL_TYPE_TIC,AVD_ENTERED_FUNC,func_name)

#define m_AVD_LOG_INVALID_VAL_ERROR(data) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, NCSFL_TYPE_TICLL,AVD_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVD_LOG_INVALID_VAL_FATAL(data) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TICLL,AVD_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVD_LOG_INVALID_SVAL_ERROR(data) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_SVAL, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, "TICLC",AVD_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVD_LOG_INVALID_SVAL_FATAL(data) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_SVAL, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_EMERGENCY, "TICLC",AVD_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVD_LOG_INVALID_NAME_VAL_ERROR(addrs,lent) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = lent;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL_NAME, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, "TICLP",AVD_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVD_LOG_INVALID_NAME_VAL_FATAL(addrs,lent) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = lent;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL_NAME, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_EMERGENCY, "TICLP",AVD_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(addrs,lent_net) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = m_NCS_OS_NTOHS(lent_net);\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL_NAME, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, "TICLP",AVD_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(addrs,lent_net) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = m_NCS_OS_NTOHS(lent_net);\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL_NAME, AVD_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_EMERGENCY, "TICLP",AVD_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVD_LOG_MEM_FAIL(memtype) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_MEMFAIL, AVD_FC_MEMFAIL, NCSFL_LC_MEMORY, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TI,memtype)

#define m_AVD_LOG_MEM_FAIL_LOC(memtype) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_MEMFAIL_LOC, AVD_FC_MEMFAIL,  NCSFL_LC_MEMORY, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TICL,memtype, __FILE__, __LINE__)

#define m_AVD_LOG_SEAPI_INIT_INFO(flag) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_SEAPI, NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG, NCSFL_TYPE_TII,AVSV_LOG_SEAPI_CREATE, flag)

#define m_AVD_LOG_SEAPI_DSTR_INFO(flag) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_SEAPI, NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG, NCSFL_TYPE_TII,AVSV_LOG_SEAPI_DESTROY, flag)

#define m_AVD_LOG_MDS_ERROR(func) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_MDS, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSFL_TYPE_TII,func,AVSV_LOG_MDS_FAILURE)

#define m_AVD_LOG_MDS_CRITICAL(func) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_MDS, NCSFL_LC_HEADLINE, NCSFL_SEV_CRITICAL, NCSFL_TYPE_TII,func,AVSV_LOG_MDS_FAILURE)

#define m_AVD_LOG_MDS_SUCC(func) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_MDS, NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG, NCSFL_TYPE_TII,func,AVSV_LOG_MDS_SUCCESS)

#define m_AVD_LOG_LOCK_ERROR(func) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_LOCK, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSFL_TYPE_TII,func,AVSV_LOG_LOCK_FAILURE)

#define m_AVD_PXY_PXD_SUCC_LOG(info,comp,info1,info2,info3,info4) \
      avd_pxy_pxd_log(NCSFL_SEV_NOTICE,AVD_PXY_PXD_SUCC_INFO,info,comp,info1,info2,info3,info4)

#define m_AVD_PXY_PXD_ERR_LOG(info,comp,info1,info2,info3,info4) \
      avd_pxy_pxd_log(NCSFL_SEV_ERROR,AVD_PXY_PXD_ERR_INFO,info,comp,info1,info2,info3,info4)

#define m_AVD_PXY_PXD_ENTRY_LOG(info,comp,info1,info2,info3,info4) \
      avd_pxy_pxd_log(NCSFL_SEV_DEBUG,AVD_PXY_PXD_ENTRY_INFO,info,comp,info1,info2,info3,info4)

#define m_AVD_LOG_LOCK_SUCC(func) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_LOCK, NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG, NCSFL_TYPE_TII,func,AVSV_LOG_LOCK_SUCCESS)

#define m_AVD_LOG_MBX_ERROR(func) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_MBX, NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, NCSFL_TYPE_TII,func,AVSV_LOG_MBX_FAILURE)

#define m_AVD_LOG_MBX_SUCC(func) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_FUNC_RETVAL, AVSV_FC_MBX, NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG, NCSFL_TYPE_TII,func,AVSV_LOG_MBX_SUCCESS)

#define m_AVD_LOG_EVT_INFO(evttyp,evtval) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_EVT_VAL, AVD_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_DEBUG, NCSFL_TYPE_TIL,evttyp,evtval)

#define m_AVD_LOG_EVT_INVAL(evtval) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_EVT_VAL, AVD_FC_EVT, NCSFL_LC_EVENT, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TIL,AVD_RCVD_INVLD_EVENT, evtval)

#define m_AVD_LOG_MSG_DND_SND_INFO(msgtype,node) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_MSG_INFO, AVD_FC_MSG, NCSFL_LC_EVENT, NCSFL_SEV_DEBUG, NCSFL_TYPE_ILTIL, AVD_LOG_SND_MSG, node, AVD_LOG_DND_MSG, msgtype)

#define m_AVD_LOG_MSG_DND_RCV_INFO(act,msgptr,node) \
{\
   if (msgptr != NULL) \
      ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_MSG_INFO, AVD_FC_MSG, NCSFL_LC_EVENT, NCSFL_SEV_DEBUG, NCSFL_TYPE_ILTIL, act, node, AVD_LOG_DND_MSG, msgptr->msg_type);\
   else \
      ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_MSG_INFO, AVD_FC_MSG, NCSFL_LC_EVENT, NCSFL_SEV_DEBUG, NCSFL_TYPE_ILTIL, act, node, AVD_LOG_DND_MSG, 0);\
}
#define m_AVD_LOG_MSG_DND_DUMP(sev,addrs,lent,mem) \
{\
   NCSFL_MEM dmp_val;\
   dmp_val.len = lent;\
   dmp_val.addr = (char *)addrs;\
   dmp_val.dump = (char *)mem;\
   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_MSG_DND_DTL, AVD_FC_MSG, NCSFL_LC_DATA, sev, NCSFL_TYPE_TID, AVD_DUMP_DND_MSG, dmp_val);\
}

#define m_AVD_LOG_RCVD_VAL(data) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL, AVD_FC_HDLN, NCSFL_LC_DATA, NCSFL_SEV_DEBUG, NCSFL_TYPE_TICLL,AVD_RCVD_VAL,__FILE__, __LINE__,data)

#define m_AVD_LOG_RCVD_NAME_VAL(addrs,lent) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = lent;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL_NAME, AVD_FC_HDLN, NCSFL_LC_DATA, NCSFL_SEV_DEBUG, "TICLP",AVD_RCVD_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVD_LOG_RCVD_NAME_NET_VAL(addrs,lent_net) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = m_NCS_OS_NTOHS(lent_net);\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_HDLN_VAL_NAME, AVD_FC_HDLN, NCSFL_LC_DATA, NCSFL_SEV_DEBUG, "TICLP",AVD_RCVD_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVD_LOG_CKPT_EVT(evt, sev, v) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_EVT_CKPT, AVSV_FC_CKPT, NCSFL_LC_MSG_CKPT, sev, NCSFL_TYPE_TIL, evt, v)

#define m_AVD_LOG_ADMIN_STATE_TRAPS(state, name_net) \
        avd_log_admin_state_traps(state, name_net, NCSFL_SEV_NOTICE)

#define m_AVD_LOG_SI_UNASSIGNED_TRAP( name_net) \
        avd_log_si_unassigned_trap(AVD_TRAP_UNASSIGNED, name_net, NCSFL_SEV_NOTICE)

#define m_AVD_LOG_OPER_STATE_TRAPS(state, name_net) \
        avd_log_oper_state_traps(state, name_net, NCSFL_SEV_NOTICE)

#define m_AVD_LOG_SUSI_HA_TRAPS(state, su_name_net, si_name_net, is_state_changed) \
        avd_log_susi_ha_traps(state, su_name_net, si_name_net, NCSFL_SEV_NOTICE, is_state_changed)

#define m_AVD_LOG_CLM_NODE_TRAPS(name_net, op) \
        avd_log_clm_node_traps(AVD_TRAP_CLUSTER, op, name_net,  NCSFL_SEV_NOTICE)

#define m_AVD_LOG_TRAP_EVT(evt, err) ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_TRAP_EVT, AVD_FC_TRAP, NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, "TIL", evt, err)

#define m_AVD_LOG_NCS_INIT_TRAP(node_id) \
        ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_TRAP_NCS_SUCC, AVD_FC_TRAP, NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, "TIL", AVD_TRAP_NCS_INIT_SUCCESS, node_id)

#define m_AVD_LOG_SHUTDOWN_FAILURE_TRAP(node_name, errcode) \
        avd_log_shutdown_failure(node_name, NCSFL_SEV_NOTICE, errcode)
#else				/* (NCS_AVD_LOG == 1) */
#define m_AVD_LOG_FUNC_ENTRY(func_name)
#define m_AVD_LOG_INVALID_VAL_ERROR(data)
#define m_AVD_LOG_INVALID_VAL_FATAL(data)
#define m_AVD_LOG_INVALID_SVAL_ERROR(data)
#define m_AVD_LOG_INVALID_SVAL_FATAL(data)
#define m_AVD_LOG_INVALID_NAME_VAL_ERROR(addrs,lent)
#define m_AVD_LOG_INVALID_NAME_VAL_FATAL(addrs,lent)
#define m_AVD_LOG_INVALID_NAME_NET_VAL_ERROR(addrs,lent_net)
#define m_AVD_LOG_INVALID_NAME_NET_VAL_FATAL(addrs,lent_net)
#define m_AVD_LOG_MEM_FAIL(memtype)
#define m_AVD_LOG_MEM_FAIL_LOC(memtype)
#define m_AVD_LOG_SEAPI_INIT_INFO(flag)
#define m_AVD_LOG_SEAPI_DSTR_INFO(flag)
#define m_AVD_LOG_MDS_ERROR(func)
#define m_AVD_LOG_MDS_CRITICAL(func)
#define m_AVD_LOG_MDS_SUCC(func)
#define m_AVD_LOG_LOCK_ERROR(func)
#define m_AVD_PXY_PXD_SUCC_LOG(info,comp,info1,info2,info3,info4)
#define m_AVND_PXY_PXD_ERR_LOG(info,comp,info1,info2,info3,info4)
#define m_AVND_PXY_PXD_ENTRY_LOG(info,comp,info1,info2,info3,info4)
#define m_AVD_LOG_LOCK_SUCC(func)
#define m_AVD_LOG_MBX_ERROR(func)
#define m_AVD_LOG_MBX_SUCC(func)
#define m_AVD_LOG_EVT_INFO(evttyp,evtval)
#define m_AVD_LOG_EVT_INVAL(evtval)
#define m_AVD_LOG_MSG_DND_SND_INFO(msgtype,node)
#define m_AVD_LOG_MSG_DND_RCV_INFO(act,msgptr,node)
#define m_AVD_LOG_MSG_DND_DUMP(sev,addrs,lent,mem)
#define m_AVD_LOG_RCVD_VAL(data)
#define m_AVD_LOG_RCVD_NAME_VAL(addrs,lent)
#define m_AVD_LOG_RCVD_NAME_NET_VAL(addrs,lent_net)
#define m_AVD_LOG_CKPT_EVT(evt, sev, v)
#define m_AVD_LOG_ADMIN_STATE_TRAPS(state, name_net)
#define m_AVD_LOG_SI_UNASSIGNED_TRAP( name_net)
#define m_AVD_LOG_OPER_STATE_TRAPS(state, name_net)
#define m_AVD_LOG_SUSI_HA_TRAPS(state, su_name_net, si_name_net, is_state_changed)
#define m_AVD_LOG_CLM_NODE_TRAPS(name_net, op)
#define m_AVD_LOG_TRAP_EVT(evt, err)
#define m_AVD_LOG_NCS_INIT_TRAP(node_id)
#define m_AVD_LOG_SHUTDOWN_FAILURE_TRAP(node_name, errcode)
#endif   /* (NCS_AVD_LOG == 1) */

#if (NCS_AVD_LOG == 1)
/* registers the AVD logging with FLA. */
EXTERN_C void avd_flx_log_reg(void);
/* unregisters the AVD logging with FLA. */
EXTERN_C void avd_flx_log_dereg(void);
#endif   /* (NCS_AVD_LOG == 1) */

EXTERN_C void avd_log_shutdown_failure(SaNameT *node_name_net, uns8 sev, AVD_SHUTDOWN_FAILURE_FLEX errcode);

EXTERN_C void avd_log_admin_state_traps(AVD_ADMIN_STATE_FLEX state, SaNameT *name_net, uns8 sev);

EXTERN_C void avd_log_si_unassigned_trap(AVD_TRAP_FLEX state, SaNameT *name_net, uns8 sev);

EXTERN_C void avd_log_oper_state_traps(AVD_OPER_STATE_FLEX state, SaNameT *name_net, uns8 sev);

EXTERN_C void avd_log_clm_node_traps(AVD_TRAP_FLEX cl, AVD_TRAP_FLEX op, SaNameT *name_net, uns8 sev);

EXTERN_C void avd_log_susi_ha_traps(AVD_HA_STATE_FLEX state, SaNameT *su_name_net,
				    SaNameT *si_name_net, uns8 sev, NCS_BOOL isStateChanged);
void avd_pxy_pxd_log(uns32 sev, uns32 index, uns8 *info, SaNameT *comp_name,
		     uns32 info1, uns32 info2, uns32 info3, uns32 info4);

extern void _avd_log(uns8 severity, const char *function, const char *format, ...);
extern void _avd_trace(const char *file, unsigned int line, const char *format, ...);

#endif   /* AVD_DBLOG_H */
