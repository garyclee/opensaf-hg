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

  This file contains logging offset indices that are used for logging AvND 
  information.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#ifndef AVND_LOG_H
#define AVND_LOG_H

#include "avsv_log.h"

/******************************************************************************
                    Logging offset indices for timer
 ******************************************************************************/
typedef enum avnd_log_tmr {
   /* type */
   AVND_LOG_TMR_HC,
   AVND_LOG_TMR_CBK_RESP,
   AVND_LOG_TMR_HB,
   AVND_LOG_TMR_MSG_RSP,
   AVND_LOG_TMR_CLC_COMP_REG,
   AVND_LOG_TMR_SU_ERR_ESC,
   AVND_LOG_TMR_NODE_ERR_ESC,
   AVND_LOG_TMR_CLC_PXIED_COMP_INST,
   AVND_LOG_TMR_CLC_PXIED_COMP_REG,

   /* operation */
   AVND_LOG_TMR_START,
   AVND_LOG_TMR_STOP,
   AVND_LOG_TMR_EXPIRY,

   /* status */
   AVND_LOG_TMR_SUCCESS,
   AVND_LOG_TMR_FAILURE,

   AVND_LOG_TMR_MAX
} AVND_LOG_TMR;


/******************************************************************************
                    Logging offset indices for events
 ******************************************************************************/
typedef enum avnd_log_evt {
   /* type */
   AVND_LOG_EVT_AVD_NODE_UPDATE_MSG,
   AVND_LOG_EVT_AVD_NODE_UP_MSG,
   AVND_LOG_EVT_AVD_REG_HLT_MSG,
   AVND_LOG_EVT_AVD_REG_SU_MSG,
   AVND_LOG_EVT_AVD_REG_COMP_MSG,
   AVND_LOG_EVT_AVD_INFO_SU_SI_ASSIGN_MSG,
   AVND_LOG_EVT_AVD_NODE_ON_FOVER,
   AVND_LOG_EVT_AVD_PG_TRACK_ACT_RSP_MSG,
   AVND_LOG_EVT_AVD_PG_UPD_MSG,
   AVND_LOG_EVT_AVD_OPERATION_REQUEST_MSG,
   AVND_LOG_EVT_AVD_HB_INFO_MSG,
   AVND_LOG_EVT_AVD_SU_PRES_MSG,
   AVND_LOG_EVT_AVD_VERIFY_MSG,
   AVND_LOG_EVT_AVD_ACK_MSG,
   AVND_LOG_EVT_AVD_SHUTDOWN_APP_SU_MSG,
   AVND_LOG_EVT_AVD_SET_LEDS_MSG,
   AVND_LOG_EVT_AVA_FINALIZE,
   AVND_LOG_EVT_AVA_COMP_REG,
   AVND_LOG_EVT_AVA_COMP_UNREG,
   AVND_LOG_EVT_AVA_PM_START,
   AVND_LOG_EVT_AVA_PM_STOP,
   AVND_LOG_EVT_AVA_HC_START,
   AVND_LOG_EVT_AVA_HC_STOP,
   AVND_LOG_EVT_AVA_HC_CONFIRM,
   AVND_LOG_EVT_AVA_CSI_QUIESCING_COMPL,
   AVND_LOG_EVT_AVA_HA_GET,
   AVND_LOG_EVT_AVA_PG_START,
   AVND_LOG_EVT_AVA_PG_STOP,
   AVND_LOG_EVT_AVA_ERR_REP,
   AVND_LOG_EVT_AVA_ERR_CLEAR,
   AVND_LOG_EVT_AVA_RESP,
   AVND_LOG_EVT_CLA_FINALIZE,
   AVND_LOG_EVT_CLA_TRACK_START,
   AVND_LOG_EVT_CLA_TRACK_STOP,
   AVND_LOG_EVT_CLA_NODE_GET,
   AVND_LOG_EVT_CLA_NODE_ASYNC_GET,
   AVND_LOG_EVT_TMR_HC,
   AVND_LOG_EVT_TMR_CBK_RESP,
   AVND_LOG_EVT_TMR_SND_HB,
   AVND_LOG_EVT_TMR_RCV_MSG_RSP,
   AVND_LOG_EVT_TMR_CLC_COMP_REG,
   AVND_LOG_EVT_TMR_SU_ERR_ESC,
   AVND_LOG_EVT_TMR_NODE_ERR_ESC,
   AVND_LOG_EVT_TMR_CLC_PXIED_COMP_INST,
   AVND_LOG_EVT_TMR_CLC_PXIED_COMP_REG,
   AVND_LOG_EVT_MDS_AVD_UP,
   AVND_LOG_EVT_MDS_AVD_DN,
   AVND_LOG_EVT_MDS_AVA_DN,
   AVND_LOG_EVT_MDS_CLA_DN,
   AVND_LOG_EVT_CLC_RESP,
   AVND_LOG_EVT_COMP_PRES_FSM_EV,
   AVND_LOG_EVT_LAST_STEP_TERM,

   /* operation */
   AVND_LOG_EVT_CREATE,
   AVND_LOG_EVT_DESTROY,
   AVND_LOG_EVT_SEND,

   /* status */
   AVND_LOG_EVT_SUCCESS,
   AVND_LOG_EVT_FAILURE,

   AVND_LOG_EVT_MAX
} AVND_LOG_EVT;


/******************************************************************************
                    Logging offset indices for CLM database
 ******************************************************************************/
typedef enum avnd_log_clm_db {
   AVND_LOG_CLM_DB_CREATE,
   AVND_LOG_CLM_DB_DESTROY,
   AVND_LOG_CLM_DB_REC_ADD,
   AVND_LOG_CLM_DB_REC_DEL,
   AVND_LOG_CLM_DB_SUCCESS,
   AVND_LOG_CLM_DB_FAILURE,
   AVND_LOG_CLM_DB_MAX
} AVND_LOG_CLM_DB;


/******************************************************************************
                    Logging offset indices for PG database
 ******************************************************************************/
typedef enum avnd_log_pg_db {
   AVND_LOG_PG_DB_CREATE,
   AVND_LOG_PG_DB_DESTROY,
   AVND_LOG_PG_DB_REC_ADD,
   AVND_LOG_PG_DB_REC_DEL,
   AVND_LOG_PG_DB_SUCCESS,
   AVND_LOG_PG_DB_FAILURE,
   AVND_LOG_PG_DB_MAX
} AVND_LOG_PG_DB;


/******************************************************************************
                    Logging offset indices for SU database
 ******************************************************************************/
typedef enum avnd_log_su_db {
   AVND_LOG_SU_DB_CREATE,
   AVND_LOG_SU_DB_DESTROY,
   AVND_LOG_SU_DB_REC_ADD,
   AVND_LOG_SU_DB_REC_DEL,
   AVND_LOG_SU_DB_SI_ADD,
   AVND_LOG_SU_DB_SI_DEL,
   AVND_LOG_SU_DB_SI_ASSIGN,
   AVND_LOG_SU_DB_SI_REMOVE,
   AVND_LOG_SU_DB_SUCCESS,
   AVND_LOG_SU_DB_FAILURE,
   AVND_LOG_SU_DB_MAX
} AVND_LOG_SU_DB;


/******************************************************************************
                 Logging offset indices for Component database
 ******************************************************************************/
typedef enum avnd_log_comp_db {
   AVND_LOG_COMP_DB_CREATE,
   AVND_LOG_COMP_DB_DESTROY,
   AVND_LOG_COMP_DB_REC_ADD,
   AVND_LOG_COMP_DB_REC_DEL,
   AVND_LOG_COMP_DB_REC_LOOKUP,
   AVND_LOG_COMP_DB_CSI_ADD,
   AVND_LOG_COMP_DB_CSI_DEL,
   AVND_LOG_COMP_DB_CSI_ASSIGN,
   AVND_LOG_COMP_DB_CSI_REMOVE,
   AVND_LOG_COMP_DB_SUCCESS,
   AVND_LOG_COMP_DB_FAILURE,
   AVND_LOG_COMP_DB_MAX
} AVND_LOG_COMP_DB;


/******************************************************************************
                Logging offset indices for Health Check database
 ******************************************************************************/
typedef enum avnd_log_hc_db {
   AVND_LOG_HC_DB_CREATE,
   AVND_LOG_HC_DB_DESTROY,
   AVND_LOG_HC_DB_REC_ADD,
   AVND_LOG_HC_DB_REC_DEL,
   AVND_LOG_HC_DB_SUCCESS,
   AVND_LOG_HC_DB_FAILURE,
   AVND_LOG_HC_DB_MAX
} AVND_LOG_HC_DB;


/******************************************************************************
                Logging offset indices for SU FSM
 ******************************************************************************/
typedef enum avnd_log_su_fsm {
   /* state */
   AVND_LOG_SU_FSM_ST_UNINSTANTIATED,
   AVND_LOG_SU_FSM_ST_INSTANTIATING,
   AVND_LOG_SU_FSM_ST_INSTANTIATED,
   AVND_LOG_SU_FSM_ST_TERMINATING,
   AVND_LOG_SU_FSM_ST_RESTARTING,
   AVND_LOG_SU_FSM_ST_INSTANTIATIONFAILED,
   AVND_LOG_SU_FSM_ST_TERMINATIONFAILED,

   /* event */
   AVND_LOG_SU_FSM_EV_NONE,
   AVND_LOG_SU_FSM_EV_INST,
   AVND_LOG_SU_FSM_EV_TERM,
   AVND_LOG_SU_FSM_EV_RESTART,
   AVND_LOG_SU_FSM_EV_COMP_INSTANTIATED,
   AVND_LOG_SU_FSM_EV_COMP_INST_FAIL,
   AVND_LOG_SU_FSM_EV_COMP_RESTARTING,
   AVND_LOG_SU_FSM_EV_COMP_TERM_FAIL,
   AVND_LOG_SU_FSM_EV_COMP_UNINSTANTIATED,
   AVND_LOG_SU_FSM_EV_COMP_TERMINATING,

   AVND_LOG_SU_FSM_ENTER,
   AVND_LOG_SU_FSM_EXIT,

   AVND_LOG_SU_FSM_MAX
} AVND_LOG_SU_FSM;


/******************************************************************************
                Logging offset indices for Comp FSM
 ******************************************************************************/
typedef enum avnd_log_comp_fsm {
   /* state */
   AVND_LOG_COMP_FSM_ST_UNINSTANTIATED,
   AVND_LOG_COMP_FSM_ST_INSTANTIATING,
   AVND_LOG_COMP_FSM_ST_INSTANTIATED,
   AVND_LOG_COMP_FSM_ST_TERMINATING,
   AVND_LOG_COMP_FSM_ST_RESTARTING,
   AVND_LOG_COMP_FSM_ST_INSTANTIATIONFAILED,
   AVND_LOG_COMP_FSM_ST_TERMINATIONFAILED,
   AVND_LOG_COMP_FSM_ST_ORPHANED,

   /* event */
   AVND_LOG_COMP_FSM_EV_NONE,
   AVND_LOG_COMP_FSM_EV_INST,
   AVND_LOG_COMP_FSM_EV_INST_SUCC,
   AVND_LOG_COMP_FSM_EV_INST_FAIL,
   AVND_LOG_COMP_FSM_EV_TERM,
   AVND_LOG_COMP_FSM_EV_TERM_SUCC,
   AVND_LOG_COMP_FSM_EV_TERM_FAIL,
   AVND_LOG_COMP_FSM_EV_CLEANUP,
   AVND_LOG_COMP_FSM_EV_CLEANUP_SUCC,
   AVND_LOG_COMP_FSM_EV_CLEANUP_FAIL,
   AVND_LOG_COMP_FSM_EV_RESTART,
   AVND_LOG_COMP_FSM_EV_ORPH,

   AVND_LOG_COMP_FSM_ENTER,
   AVND_LOG_COMP_FSM_EXIT,

   AVND_LOG_COMP_FSM_MAX
} AVND_LOG_COMP_FSM;


/******************************************************************************
                Logging offset indices for error report, recovery & escalation
 ******************************************************************************/
typedef enum avnd_log_err {
   /* component error source */
   AVND_LOG_ERR_SRC_REP,     /* comp reports an error */
   AVND_LOG_ERR_SRC_HC,
   AVND_LOG_ERR_SRC_PM,
   AVND_LOG_ERR_SRC_AM,
   AVND_LOG_ERR_SRC_CMD_FAILED,
   AVND_LOG_ERR_SRC_CBK_HC_TIMEOUT,
   AVND_LOG_ERR_SRC_CBK_HC_FAILED,
   AVND_LOG_ERR_SRC_AVA_DN,
   AVND_LOG_ERR_SRC_PXIED_REG_TIMEOUT,
   AVND_LOG_ERR_SRC_CBK_CSI_SET_TIMEOUT,
   AVND_LOG_ERR_SRC_CBK_CSI_REM_TIMEOUT,
   AVND_LOG_ERR_SRC_CBK_CSI_SET_FAILED,
   AVND_LOG_ERR_SRC_CBK_CSI_REM_FAILED,

   /* recovery specified in SaAmfRecommendedRecoveryT */
   AVSV_LOG_AMF_NO_RECOMMENDATION,
   AVSV_LOG_AMF_COMPONENT_RESTART,
   AVSV_LOG_AMF_COMPONENT_FAILOVER,
   AVSV_LOG_AMF_NODE_SWITCHOVER,
   AVSV_LOG_AMF_NODE_FAILOVER,
   AVSV_LOG_AMF_NODE_FAILFAST,
   AVSV_LOG_AMF_CLUSTER_RESET,

   /* escalated recovery */
   AVSV_LOG_ERR_RCVR_SU_RESTART,
   AVSV_LOG_ERR_RCVR_SU_FAILOVER,
   AVSV_LOG_ERR_RCVR_MAX
} AVND_LOG_ERR;

/******************************************************************************
             Logging offset indices for miscellaneous AvND events
 ******************************************************************************/
typedef enum avnd_log_misc {
   AVND_LOG_MISC_AVD_UP,
   AVND_LOG_MISC_AVD_DN,
   AVND_LOG_MISC_AVA_UP,
   AVND_LOG_MISC_AVA_DN,

   AVND_LOG_COMP_SCRIPT_INSTANTIATE,
   AVND_LOG_COMP_SCRIPT_TERMINATE,
   AVND_LOG_COMP_SCRIPT_CLEANUP,
   AVND_LOG_COMP_SCRIPT_AMSTART,
   AVND_LOG_COMP_SCRIPT_AMSTOP,

   /* keep adding misc logs here */

   AVND_LOG_MISC_MAX
} AVND_LOG_MISC;

/******************************************************************************
             Logging offset indices for fail-over AvND events
 ******************************************************************************/
typedef enum avnd_log_fovr
{
   AVND_LOG_FOVR_CLM_NODE_INFO,
   AVND_LOG_FOVR_COMP_UPDT,
   AVND_LOG_FOVR_COMP_UPDT_FAIL,
   AVND_LOG_FOVR_SU_UPDT,
   AVND_LOG_FOVR_SU_UPDT_FAIL,
   AVND_LOG_FOVR_HLT_UPDT,
   AVND_LOG_FOVR_HLT_UPDT_FAIL,
   AVND_LOG_MSG_ID_MISMATCH,
   AVND_LOG_FOVR_VERIFY_MSG_RCVD,
   AVND_LOG_FOVR_AVD_SND_ID_CNT,
   AVND_LOG_FOVR_AVND_RCV_ID_CNT,
   AVND_LOG_FOVR_AVD_RCV_ID_CNT,
   AVND_LOG_FOVR_AVND_SND_ID_CNT,
   AVND_LOG_FOVR_REC_SENT,
   AVND_LOG_FOVR_REC_DEL,
   AVND_LOG_FOVR_REC_NOT_FOUND,
   AVND_LOG_FOVR_MSG_ID_ACK_NACK,
   AVND_LOG_FOVR_VNUM_ACK_NACK,

   AVND_LOG_FOVR_MAX

}AVND_LOG_FOVR;

typedef enum avnd_log_trap
{
   AVND_LOG_TRAP_INSTANTIATION,
   AVND_LOG_TRAP_TERMINATION,
   AVND_LOG_TRAP_CLEANUP,
   AVND_LOG_TRAP_OPER_STATE,
   AVND_LOG_TRAP_PRES_STATE,
   AVND_LOG_TRAP_ORPHANED,
   AVND_LOG_TRAP_FAILED,
   AVND_LOG_EDA_INIT_FAILED,   
   AVND_LOG_EDA_CHNL_OPEN_FAILED,
   AVND_LOG_EDA_EVT_PUBLISH_FAILED,

   AVND_LOG_TRAP_MAX

}AVND_LOG_TRAP;

typedef enum avnd_log_oper_state
{
   AVND_LOG_TRAP_OPER_STATE_MIN,
   AVND_LOG_TRAP_OPER_STATE_ENABLE,
   AVND_LOG_TRAP_OPER_STATE_DISABLE,

   AVND_LOG_TRAP_OPER_STATE_MAX

}AVND_LOG_OPER_STATE;

typedef enum avnd_log_pres_state
{
   AVND_LOG_TRAP_PRES_STATE_MIN                ,
   AVND_LOG_TRAP_PRES_STATE_UNINSTANTIATED     ,
   AVND_LOG_TRAP_PRES_STATE_INSTANTIATING      ,
   AVND_LOG_TRAP_PRES_STATE_INSTANTIATED       ,
   AVND_LOG_TRAP_PRES_STATE_TERMINATING        ,
   AVND_LOG_TRAP_PRES_STATE_RESTARTING         ,
   AVND_LOG_TRAP_PRES_STATE_INSTANTIATIONFAILED,
   AVND_LOG_TRAP_PRES_STATE_TERMINATIONFAILED  ,
   AVND_LOG_TRAP_PRES_STATE_ORPHANED           ,

   AVND_LOG_TRAP_PRES_STATE_MAX

}AVND_LOG_PRES_STATE;

/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/
typedef enum avnd_log_hdln_flex
{
   AVND_INVALID_VAL,
   AVND_UNKNOWN_MSG_RCVD,
   AVND_MSG_PROC_FAILED,
   AVND_ENTERED_FUNC,
   AVND_RCVD_VAL
} AVND_LOG_HDLN_FLEX;

/******************************************************************************
 Logging offset indexes for AvND-AvND logging
 ******************************************************************************/
typedef enum avnd_log_avnd_avnd_flex
{
   AVND_AVND_SUCC_INFO,
   AVND_AVND_ERR_INFO,
   AVND_AVND_ENTRY_INFO,
   AVND_AVND_DEBUG_INFO
} AVND_LOG_AVND_AVND_FLEX;

/******************************************************************************
 Logging offset indices for canned constant strings for the ASCII SPEC
 ******************************************************************************/
typedef enum avnd_flex_sets {
   AVND_FC_SEAPI,
   AVND_FC_MDS,
   AVND_FC_SRM,
   AVND_FC_EDU,
   AVND_FC_LOCK,
   AVND_FC_CB,
   AVND_FC_MBX,
   AVND_FC_TASK,
   AVND_FC_TMR,
   AVND_FC_EVT,
   AVND_FC_CLM_DB,
   AVND_FC_PG_DB,
   AVND_FC_SU_DB,
   AVND_FC_COMP_DB,
   AVND_FC_HC_DB,
   AVND_FC_SU_FSM,
   AVND_FC_COMP_FSM,
   AVND_FC_ERR,
   AVND_FC_MISC,
   AVND_FC_FOVR,
   AVND_FC_TRAP,
   AVND_FC_OPER,
   AVND_FC_PRES,
   AVND_FC_HDLN,
   AVND_FC_AVND_MSG
} AVND_FLEX_SETS;

typedef enum avnd_log_ids {
   AVND_LID_SEAPI,
   AVND_LID_MDS,
   AVND_LID_SRM,
   AVND_LID_EDU,
   AVND_LID_LOCK,
   AVND_LID_CB,
   AVND_LID_MBX,
   AVND_LID_TASK,
   AVND_LID_TMR,
   AVND_LID_EVT1,
   AVND_LID_EVT2,
   AVND_LID_EVT3,
   AVND_LID_CLM_DB,
   AVND_LID_PG_DB,
   AVND_LID_SU_DB1,
   AVND_LID_SU_DB2,
   AVND_LID_COMP_DB1,
   AVND_LID_COMP_DB2,
   AVND_LID_HC_DB,
   AVND_LID_SU_FSM,
   AVND_LID_COMP_FSM,
   AVND_LID_ERR,
   AVND_LID_MISC1,
   AVND_LID_MISC2,
   AVND_LID_FOVER,
   AVND_LID_TRAP_CLC,
   AVND_LID_TRAP_OPER_STAT,
   AVND_LID_TRAP_PRES_STAT,
   AVND_LID_TRAP_PROXIED,
   AVND_LID_TRAP_EVT,
   AVND_LID_TRAP_COMP_FAIL,
   AVND_LID_HDLN,
   AVND_LID_HDLN_VAL,
   AVND_LID_HDLN_VAL_NAME,
   AVND_LID_HDLN_STRING,
   AVND_AVND_MSG,
} AVND_LOG_IDS;


/*****************************************************************************
                          Macros for Logging
*****************************************************************************/

#if ( NCS_AVND_LOG == 1 )

#define m_AVND_LOG_SEAPI(op, st, sev)       avnd_log_seapi(op, st, sev)
#define m_AVND_LOG_MDS(op, st, sev)         avnd_log_mds(op, st, sev)
#define m_AVND_LOG_SRM(op, st, sev)         avnd_log_srm(op, st, sev)
#define m_AVND_LOG_EDU(op, st, sev)         avnd_log_edu(op, st, sev)
#define m_AVND_LOG_LOCK(op, st, sev)        avnd_log_lock(op, st, sev)
#define m_AVND_LOG_CB(op, st, sev)          avnd_log_cb(op, st, sev)
#define m_AVND_LOG_MBX(op, st, sev)         avnd_log_mbx(op, st, sev)
#define m_AVND_LOG_TASK(op, st, sev)        avnd_log_task(op, st, sev)
#define m_AVND_LOG_TMR(ty, op, st, sev)     avnd_log_tmr(ty, op, st, sev)
#define m_AVND_LOG_EVT(ty, op, st, sev)     avnd_log_evt(ty, op, st, sev)
#define m_AVND_LOG_CLM_DB(op, st, nid, sev) avnd_log_clm_db(op, st, nid, sev)
#define m_AVND_LOG_PG_DB(op, st, csin, sev) avnd_log_pg_db(op, st, csin, sev)
#define m_AVND_LOG_HC_DB(op, st, key, sev)  avnd_log_hc_db(op, st, key, sev)
#define m_AVND_LOG_MISC(op, name, sev)      avnd_log_misc(op, name, sev)

#define m_AVND_LOG_SU_DB(op, st, sun, sin, sev) \
                                       avnd_log_su_db(op, st, sun, sin, sev)

#define m_AVND_LOG_COMP_DB(op, st, cn, csin, sev) \
                                       avnd_log_comp_db(op, st, cn, csin, sev)

#define m_AVND_AVND_SUCC_LOG(info,comp,info1,info2,info3,info4) \
      avnd_pxy_pxd_log(NCSFL_SEV_NOTICE,AVND_AVND_SUCC_INFO,info,comp,info1,info2,info3,info4)

#define m_AVND_AVND_ERR_LOG(info,comp,info1,info2,info3,info4) \
      avnd_pxy_pxd_log(NCSFL_SEV_ERROR,AVND_AVND_ERR_INFO,info,comp,info1,info2,info3,info4)

#define m_AVND_AVND_ENTRY_LOG(info,comp,info1,info2,info3,info4) \
      avnd_pxy_pxd_log(NCSFL_SEV_INFO,AVND_AVND_ENTRY_INFO,info,comp,info1,info2,info3,info4)

#define m_AVND_AVND_DEBUG_LOG(info,comp,info1,info2,info3,info4) \
      avnd_pxy_pxd_log(NCSFL_SEV_DEBUG,AVND_AVND_DEBUG_INFO,info,comp,info1,info2,info3,info4)

#define m_AVND_LOG_SU_FSM(op, st, ev, name, sev) \
                                       avnd_log_su_fsm(op, st, ev, name, sev)

#define m_AVND_LOG_COMP_FSM(op, st, ev, name, sev) \
                                       avnd_log_comp_fsm(op, st, ev, name, sev)
#define m_AVND_LOG_ERR(src, rec, name, sev) \
                                       avnd_log_err(src -1, AVSV_LOG_AMF_NO_RECOMMENDATION + rec -1, name, sev)

#define m_AVND_LOG_FOVER_EVTS(sev, ev, val) \
   ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_FOVER, AVND_FC_FOVR, \
                 NCSFL_LC_EVENT, sev, NCSFL_TYPE_TICLL,  \
                 ev,__FILE__, __LINE__, val)

#define m_AVND_LOG_CLC_TRAPS(op, cn) \
        avnd_log_clc_traps(op, AVND_LOG_TRAP_FAILED, cn, NCSFL_SEV_NOTICE)

#define m_AVND_LOG_PROXIED_ORPHANED_TRAP(cn, pxy_cn) \
        avnd_log_proxied_orphaned_trap(AVND_LOG_TRAP_ORPHANED, AVND_LOG_TRAP_FAILED, cn, pxy_cn, NCSFL_SEV_NOTICE)

#define m_AVND_LOG_SU_OPER_STATE_TRAP(state, sn) \
        avnd_log_su_oper_state_trap(state, sn, NCSFL_SEV_NOTICE)

#define m_AVND_LOG_SU_PRES_STATE_TRAP(state, sn) \
        avnd_log_su_pres_state_trap(state, sn, NCSFL_SEV_NOTICE)

#define m_AVND_LOG_TRAP_EVT(evt, err)\
        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_TRAP_EVT, AVND_FC_TRAP,\
              NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, "TIL", evt, err)


#define m_AVND_LOG_COMP_FAIL_TRAP(node_id, comp_name, errSrc)\
        avnd_log_comp_failed_trap(node_id, comp_name, errSrc-1, NCSFL_SEV_NOTICE)

/* debugging logs */
#define m_AVND_LOG_INVALID_VAL_ERROR(data)\
        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL, AVND_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL,\
              NCSFL_SEV_ERROR, "TICLL", AVND_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVND_LOG_INVALID_VAL_FATAL(data)\
        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL, AVND_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL,\
              NCSFL_SEV_EMERGENCY, "TICLL", AVND_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVND_LOG_INVALID_STRING_ERROR(data)\
        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_STRING, AVND_FC_HDLN, NCSFL_LC_HEADLINE,\
              NCSFL_SEV_ERROR, "TICLC", AVND_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVND_LOG_INVALID_STRING_FATAL(data)\
        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_STRING, AVND_FC_HDLN, NCSFL_LC_HEADLINE,\
              NCSFL_SEV_EMERGENCY, "TICLC", AVND_INVALID_VAL,__FILE__, __LINE__,data)

#define m_AVND_LOG_INVALID_NAME_VAL_ERROR(addrs,lent) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = lent;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL_NAME, AVND_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL,\
         NCSFL_SEV_ERROR, "TICLP",AVND_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVND_LOG_INVALID_NAME_VAL_FATAL(addrs,lent) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = lent;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL_NAME, AVND_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL,\
         NCSFL_SEV_EMERGENCY, "TICLP",AVND_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVND_LOG_INVALID_NAME_NET_VAL_ERROR(addrs,lent_net) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = m_NCS_OS_NTOHS(lent_net);\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL_NAME, AVND_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL,\
         NCSFL_SEV_ERROR, "TICLP",AVND_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVND_LOG_INVALID_NAME_NET_VAL_FATAL(addrs,lent_net) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = m_NCS_OS_NTOHS(lent_net);\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL_NAME, AVND_FC_HDLN, NCSFL_LC_FUNC_RET_FAIL,\
         NCSFL_SEV_EMERGENCY, "TICLP",AVND_INVALID_VAL,__FILE__, __LINE__,nam_val);\
}


#define m_AVND_LOG_RCVD_VAL(data)\
        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL, AVND_FC_HDLN, NCSFL_LC_DATA,\
              NCSFL_SEV_DEBUG, NCSFL_TYPE_TICLL,AVND_RCVD_VAL,__FILE__, __LINE__,data)

#define m_AVND_LOG_RCVD_NAME_VAL(addrs,lent) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = lent;\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL_NAME, AVND_FC_HDLN, NCSFL_LC_DATA,\
         NCSFL_SEV_DEBUG, "TICLP",AVND_RCVD_VAL,__FILE__, __LINE__,nam_val);\
}

#define m_AVND_LOG_RCVD_NAME_NET_VAL(addrs,lent_net) \
{\
   NCSFL_PDU nam_val;\
   nam_val.len = m_NCS_OS_NTOHS(lent_net);\
   nam_val.dump = (char *)addrs;\
   ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN_VAL_NAME, AVND_FC_HDLN, NCSFL_LC_DATA,\
         NCSFL_SEV_DEBUG, "TICLP",AVND_RCVD_VAL,__FILE__, __LINE__,nam_val);\
}


#define m_AVND_LOG_FUNC_ENTRY(func_name)\
        ncs_logmsg(NCS_SERVICE_ID_AVND, AVND_LID_HDLN, AVND_FC_HDLN, NCSFL_LC_FUNC_ENTRY,\
              NCSFL_SEV_DEBUG, NCSFL_TYPE_TIC,AVND_ENTERED_FUNC,func_name)


#else /* NCS_AVND_LOG == 1 */

#define m_AVND_LOG_SEAPI(op, st, sev)
#define m_AVND_LOG_MDS(op, st, sev)
#define m_AVND_LOG_SRM(op, st, sev)
#define m_AVND_LOG_EDU(op, st, sev)
#define m_AVND_LOG_LOCK(op, st, sev)
#define m_AVND_LOG_CB(op, st, sev)
#define m_AVND_LOG_MBX(op, st, sev)
#define m_AVND_LOG_TASK(op, st, sev)
#define m_AVND_LOG_TMR(ty, op, st, sev)
#define m_AVND_LOG_EVT(ty, op, st, sev)
#define m_AVND_LOG_CLM_DB(op, st, nid, sev)
#define m_AVND_LOG_PG_DB(op, st, nid, sev)
#define m_AVND_LOG_SU_DB(op, st, sun, sin, sev)
#define m_AVND_LOG_HC_DB(op, st, key, sev)
#define m_AVND_LOG_MISC(op, name, sev)
#define m_AVND_LOG_COMP_DB(op, st, cn, csin, sev)
#define m_AVND_AVND_SUCC_LOG(info,comp,info1,info2,info3,info4) 
#define m_AVND_AVND_ERR_LOG(info,comp,info1,info2,info3,info4) 
#define m_AVND_AVND_ENTRY_LOG(info,comp,info1,info2,info3,info4) 
#define m_AVND_AVND_DEBUG_LOG(info,comp,info1,info2,info3,info4) 
#define m_AVND_LOG_SU_FSM(op, st, ev, name, sev)
#define m_AVND_LOG_COMP_FSM(op, st, ev, name, sev)
#define m_AVND_LOG_FOVER_EVTS(sev, ev, val)
#define m_AVND_LOG_ERR(name, src, rec, sev)
#define m_AVND_LOG_CLC_TRAPS(op, cn)
#define m_AVND_LOG_PROXIED_ORPHANED_TRAP(cn, pxy_cn)
#define m_AVND_LOG_SU_OPER_STATE_TRAP(state, sn)
#define m_AVND_LOG_SU_PRES_STATE_TRAP(state, sn)
#define m_AVND_LOG_TRAP_EVT(evt, err)
#define m_AVND_LOG_COMP_FAIL_TRAP(node_id, comp_name, errSrc)
#define m_AVND_LOG_INVALID_VAL_ERROR(data)
#define m_AVND_LOG_INVALID_VAL_FATAL(data)
#define m_AVND_LOG_INVALID_STRING_ERROR(data)
#define m_AVND_LOG_INVALID_STRING_FATAL(data)
#define m_AVND_LOG_INVALID_NAME_VAL_ERROR(addrs,lent)
#define m_AVND_LOG_INVALID_NAME_VAL_FATAL(addrs,lent)
#define m_AVND_LOG_INVALID_NAME_NET_VAL_ERROR(addrs,lent_net)
#define m_AVND_LOG_INVALID_NAME_NET_VAL_FATAL(addrs,lent_net)
#define m_AVND_LOG_RCVD_VAL(data)
#define m_AVND_LOG_RCVD_NAME_VAL(addrs,lent)
#define m_AVND_LOG_RCVD_NAME_NET_VAL(addrs,lent_net)
#define m_AVND_LOG_FUNC_ENTRY(func_name)

#endif /* NCS_AVND_LOG == 1 */

/*****************************************************************************
                       Extern Function Declarations
*****************************************************************************/

#if ( NCS_AVND_LOG == 1 )

EXTERN_C uns32 avnd_log_reg (void);
EXTERN_C uns32 avnd_log_unreg (void);

EXTERN_C void avnd_log_seapi (AVSV_LOG_SEAPI, AVSV_LOG_SEAPI, uns8);
EXTERN_C void avnd_log_mds (AVSV_LOG_MDS, AVSV_LOG_MDS, uns8);
EXTERN_C void avnd_log_srm (AVSV_LOG_SRM, AVSV_LOG_SRM, uns8);
EXTERN_C void avnd_log_edu (AVSV_LOG_EDU, AVSV_LOG_EDU, uns8);
EXTERN_C void avnd_log_lock (AVSV_LOG_LOCK, AVSV_LOG_LOCK, uns8);
EXTERN_C void avnd_log_cb (AVSV_LOG_CB, AVSV_LOG_CB, uns8);
EXTERN_C void avnd_log_mbx (AVSV_LOG_MBX, AVSV_LOG_MBX, uns8);
EXTERN_C void avnd_log_task (AVSV_LOG_TASK, AVSV_LOG_TASK, uns8);
EXTERN_C void avnd_log_tmr (AVND_LOG_TMR, AVND_LOG_TMR, 
                            AVND_LOG_TMR, uns8);
EXTERN_C void avnd_log_evt (AVND_LOG_EVT, AVND_LOG_EVT, 
                            AVND_LOG_EVT, uns8);
EXTERN_C void avnd_log_clm_db (AVND_LOG_CLM_DB, AVND_LOG_CLM_DB, 
                               SaClmNodeIdT, uns8);
EXTERN_C void avnd_log_pg_db (AVND_LOG_PG_DB, AVND_LOG_PG_DB, 
                               SaNameT *, uns8);
EXTERN_C void avnd_log_su_db (AVND_LOG_SU_DB, AVND_LOG_SU_DB, 
                              SaNameT *, SaNameT *, uns8);
EXTERN_C void avnd_log_comp_db (AVND_LOG_COMP_DB, AVND_LOG_COMP_DB, 
                                SaNameT *, SaNameT *, uns8);
EXTERN_C void avnd_pxy_pxd_log (uns32, uns32, uns8 *, SaNameT *, uns32, uns32, 
                             uns32, uns32 );

EXTERN_C void avnd_log_hc_db (AVND_LOG_HC_DB, AVND_LOG_HC_DB, 
                              SaAmfHealthcheckKeyT *, uns8);
EXTERN_C void avnd_log_su_fsm (AVND_LOG_SU_FSM , NCS_PRES_STATE ,
                               AVND_SU_PRES_FSM_EV , SaNameT  *, uns8 );
EXTERN_C void avnd_log_comp_fsm (AVND_LOG_COMP_FSM , NCS_PRES_STATE ,
                                 AVND_COMP_CLC_PRES_FSM_EV , SaNameT  *, uns8 );
EXTERN_C void avnd_log_err (AVND_LOG_ERR, AVND_LOG_ERR, SaNameT *, uns8);
EXTERN_C void avnd_log_misc (AVND_LOG_MISC, SaNameT *, uns8);

EXTERN_C void avnd_log_clc_traps (AVND_LOG_TRAP op, AVND_LOG_TRAP status,
                                SaNameT *comp_name, uns8 sev);

EXTERN_C void avnd_log_su_oper_state_trap (AVND_LOG_OPER_STATE state,
                                           SaNameT *su_name, uns8 sev);

EXTERN_C void avnd_log_su_pres_state_trap (AVND_LOG_PRES_STATE state,
                                           SaNameT *su_name, uns8 sev);

EXTERN_C void avnd_log_proxied_orphaned_trap (AVND_LOG_TRAP status, 
                                     AVND_LOG_TRAP pxy_status, SaNameT *comp_name,
                                     SaNameT *pxy_comp_name,uns8 sev);

EXTERN_C void  avnd_log_comp_failed_trap(uns32 node_id,
                               SaNameT *name_net,
                               AVND_LOG_ERR   errSrc,
                               uns8    sev);

#endif /* NCS_AVND_LOG == 1 */

 
#endif /* !AVND_LOG_H */
