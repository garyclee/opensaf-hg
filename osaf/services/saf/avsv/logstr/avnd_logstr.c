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

#if (NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "avsv_log.h"
#include "avnd_log.h"
#include "avsv_logstr.h"

uns32 avnd_str_reg(void);
uns32 avnd_str_unreg(void);

/******************************************************************************
                      Canned string for AvND Timers
 ******************************************************************************/
const NCSFL_STR avnd_tmr_set[] = {
	{AVND_LOG_TMR_HC, "HC Tmr"},
	{AVND_LOG_TMR_CBK_RESP, "Cbk Rsp Tmr"},
	{AVND_LOG_TMR_HB, "HB Tmr"},
	{AVND_LOG_TMR_MSG_RSP, "AvD Msg Rsp Tmr"},
	{AVND_LOG_TMR_CLC_COMP_REG, "CLC Comp Reg Tmr"},
	{AVND_LOG_TMR_SU_ERR_ESC, "SU Err Esc Tmr"},
	{AVND_LOG_TMR_NODE_ERR_ESC, "Node Err Esc Tmr"},
	{AVND_LOG_TMR_CLC_PXIED_COMP_INST, "CLC proxied Comp Inst Tmr"},
	{AVND_LOG_TMR_CLC_PXIED_COMP_REG, "CLC Proxied Comp Reg Tmr"},
	{AVND_LOG_TMR_START, "Start"},
	{AVND_LOG_TMR_STOP, "Stop"},
	{AVND_LOG_TMR_EXPIRY, "Expiry"},
	{AVND_LOG_TMR_SUCCESS, "Success"},
	{AVND_LOG_TMR_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for AvND Event
 ******************************************************************************/
const NCSFL_STR avnd_evt_set[] = {
	{AVND_LOG_EVT_AVD_NODE_UPDATE_MSG, "AvD Node Upd Evt"},
	{AVND_LOG_EVT_AVD_NODE_UP_MSG, "AvD Node Up Evt"},
	{AVND_LOG_EVT_AVD_REG_HLT_MSG, "AvD Healthcheck Evt"},
	{AVND_LOG_EVT_AVD_REG_SU_MSG, "AvD SU Evt"},
	{AVND_LOG_EVT_AVD_REG_COMP_MSG, "AvD Comp Evt"},
	{AVND_LOG_EVT_AVD_INFO_SU_SI_ASSIGN_MSG, "AvD SU-SI Assign Evt"},
	{AVND_LOG_EVT_AVD_NODE_ON_FOVER, "AvD Node On Failover Evt"},
	{AVND_LOG_EVT_AVD_PG_TRACK_ACT_RSP_MSG, "AvD PG Track Act Rsp Evt"},
	{AVND_LOG_EVT_AVD_PG_UPD_MSG, "AvD PG Upd Evt"},
	{AVND_LOG_EVT_AVD_OPERATION_REQUEST_MSG, "AvD Oper Req Evt"},
	{AVND_LOG_EVT_AVD_HB_INFO_MSG, "AvD HB Info Evt"},
	{AVND_LOG_EVT_AVD_SU_PRES_MSG, "AvD SU Pres Evt"},
	{AVND_LOG_EVT_AVD_VERIFY_MSG, "AvD Verify Evt"},
	{AVND_LOG_EVT_AVD_ACK_MSG, "AvD Ack Evt"},
	{AVND_LOG_EVT_AVD_SHUTDOWN_APP_SU_MSG, "AvD Shut Down Evt"},
	{AVND_LOG_EVT_AVD_SET_LEDS_MSG, "AvD LED Set Evt"},
	{AVND_LOG_EVT_AVA_FINALIZE, "AvA Finalize Evt"},
	{AVND_LOG_EVT_AVA_COMP_REG, "AvA Comp Reg Evt"},
	{AVND_LOG_EVT_AVA_COMP_UNREG, "AvA Comp Unreg Evt"},
	{AVND_LOG_EVT_AVA_PM_START, "AvA PM Start Evt"},
	{AVND_LOG_EVT_AVA_PM_STOP, "AvA PM Stop Evt"},
	{AVND_LOG_EVT_AVA_HC_START, "AvA HC Start Evt"},
	{AVND_LOG_EVT_AVA_HC_STOP, "AvA HC Stop Evt"},
	{AVND_LOG_EVT_AVA_HC_CONFIRM, "AvA HC Confirm Evt"},
	{AVND_LOG_EVT_AVA_CSI_QUIESCING_COMPL, "AvA CSI Quiescing Compl Evt"},
	{AVND_LOG_EVT_AVA_HA_GET, "AvA HA Get Evt"},
	{AVND_LOG_EVT_AVA_PG_START, "AvA PG Start Evt"},
	{AVND_LOG_EVT_AVA_PG_STOP, "AvA PG Stop Evt"},
	{AVND_LOG_EVT_AVA_ERR_REP, "AvA Err Rep Evt"},
	{AVND_LOG_EVT_AVA_ERR_CLEAR, "AvA Err Clear Evt"},
	{AVND_LOG_EVT_AVA_RESP, "AvA Resp Evt"},
	{AVND_LOG_EVT_CLA_FINALIZE, "CLA Finalize Evt"},
	{AVND_LOG_EVT_CLA_TRACK_START, "CLA Track Start Evt"},
	{AVND_LOG_EVT_CLA_TRACK_STOP, "CLA Track Stop Evt"},
	{AVND_LOG_EVT_CLA_NODE_GET, "CLA Node Get Evt"},
	{AVND_LOG_EVT_CLA_NODE_ASYNC_GET, "CLA Node Async Get Evt"},
	{AVND_LOG_EVT_TMR_HC, "HC Tmr Evt"},
	{AVND_LOG_EVT_TMR_CBK_RESP, "Cbk Tmr Evt"},
	{AVND_LOG_EVT_TMR_SND_HB, "HB Tmr Evt"},
	{AVND_LOG_EVT_TMR_RCV_MSG_RSP, "AvD Msg Rsp Tmr Evt"},
	{AVND_LOG_EVT_TMR_CLC_COMP_REG, "Clc Comp Reg Tmr Evt"},
	{AVND_LOG_EVT_TMR_SU_ERR_ESC, "SU Err Esc Tmr Evt"},
	{AVND_LOG_EVT_TMR_NODE_ERR_ESC, "Node Err Esc Tmr Evt"},
	{AVND_LOG_EVT_TMR_CLC_PXIED_COMP_INST, "Clc pxied Comp inst Tmr Evt"},
	{AVND_LOG_EVT_TMR_CLC_PXIED_COMP_REG, "Clc pxied Comp Reg Tmr Evt"},
	{AVND_LOG_EVT_MDS_AVD_UP, "AvD Up Evt"},
	{AVND_LOG_EVT_MDS_AVD_DN, "AvD Dn Evt"},
	{AVND_LOG_EVT_MDS_AVA_DN, "AvA Dn Evt"},
	{AVND_LOG_EVT_MDS_CLA_DN, "CLA Dn Evt"},
	{AVND_LOG_EVT_CLC_RESP, "CLC Resp Evt"},
	{AVND_LOG_EVT_COMP_PRES_FSM_EV, "Comp Pres FSM Evt"},
	{AVND_LOG_EVT_LAST_STEP_TERM, "Last Step Term Evt"},
	{AVND_LOG_EVT_CREATE, "Create"},
	{AVND_LOG_EVT_DESTROY, "Destroy"},
	{AVND_LOG_EVT_SEND, "Send"},
	{AVND_LOG_EVT_SUCCESS, "Success"},
	{AVND_LOG_EVT_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for CLM Database
 ******************************************************************************/
const NCSFL_STR avnd_clm_db_set[] = {
	{AVND_LOG_CLM_DB_CREATE, "Clm Db Create"},
	{AVND_LOG_CLM_DB_DESTROY, "Clm Db Destroy"},
	{AVND_LOG_CLM_DB_REC_ADD, "Clm Db Add"},
	{AVND_LOG_CLM_DB_REC_DEL, "Clm Db Del"},
	{AVND_LOG_CLM_DB_SUCCESS, "Success"},
	{AVND_LOG_CLM_DB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for PG Database
 ******************************************************************************/
const NCSFL_STR avnd_pg_db_set[] = {
	{AVND_LOG_PG_DB_CREATE, "PG Db Create"},
	{AVND_LOG_PG_DB_DESTROY, "PG Db Destroy"},
	{AVND_LOG_PG_DB_REC_ADD, "PG Db Add"},
	{AVND_LOG_PG_DB_REC_DEL, "PG Db Del"},
	{AVND_LOG_PG_DB_SUCCESS, "Success"},
	{AVND_LOG_PG_DB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for SU Database
 ******************************************************************************/
const NCSFL_STR avnd_su_db_set[] = {
	{AVND_LOG_SU_DB_CREATE, "SU Db Create"},
	{AVND_LOG_SU_DB_DESTROY, "SU Db Destroy"},
	{AVND_LOG_SU_DB_REC_ADD, "SU Db Add"},
	{AVND_LOG_SU_DB_REC_DEL, "SU Db Del"},
	{AVND_LOG_SU_DB_SI_ADD, "SI Add"},
	{AVND_LOG_SU_DB_SI_DEL, "SI Del"},
	{AVND_LOG_SU_DB_SI_ASSIGN, "SI Assign"},
	{AVND_LOG_SU_DB_SI_REMOVE, "SI Remove"},
	{AVND_LOG_SU_DB_SUCCESS, "Success"},
	{AVND_LOG_SU_DB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for Component Database
 ******************************************************************************/
const NCSFL_STR avnd_comp_db_set[] = {
	{AVND_LOG_COMP_DB_CREATE, "Comp Db Create"},
	{AVND_LOG_COMP_DB_DESTROY, "Comp Db Destroy"},
	{AVND_LOG_COMP_DB_REC_ADD, "Comp Db Add"},
	{AVND_LOG_COMP_DB_REC_DEL, "Comp Db Del"},
	{AVND_LOG_COMP_DB_REC_LOOKUP, "Comp Db Lookup"},
	{AVND_LOG_COMP_DB_CSI_ADD, "CSI Add"},
	{AVND_LOG_COMP_DB_CSI_DEL, "CSI Del"},
	{AVND_LOG_COMP_DB_CSI_ASSIGN, "CSI Assign"},
	{AVND_LOG_COMP_DB_CSI_REMOVE, "CSI Remove"},
	{AVND_LOG_COMP_DB_SUCCESS, "Success"},
	{AVND_LOG_COMP_DB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for HealthCheck Database
 ******************************************************************************/
const NCSFL_STR avnd_hc_db_set[] = {
	{AVND_LOG_HC_DB_CREATE, "HC Db Create"},
	{AVND_LOG_HC_DB_DESTROY, "HC Db Destroy"},
	{AVND_LOG_HC_DB_REC_ADD, "HC Db Add"},
	{AVND_LOG_HC_DB_REC_DEL, "HC Db Del"},
	{AVND_LOG_HC_DB_SUCCESS, "Success"},
	{AVND_LOG_HC_DB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                      Canned string for SU FSM
 ******************************************************************************/
const NCSFL_STR avnd_su_fsm_set[] = {
	{AVND_LOG_SU_FSM_ST_UNINSTANTIATED, "Uninstantiated"},
	{AVND_LOG_SU_FSM_ST_INSTANTIATING, "Instantiating"},
	{AVND_LOG_SU_FSM_ST_INSTANTIATED, "Instantiated"},
	{AVND_LOG_SU_FSM_ST_TERMINATING, "Terminating"},
	{AVND_LOG_SU_FSM_ST_RESTARTING, "Restarting"},
	{AVND_LOG_SU_FSM_ST_INSTANTIATIONFAILED, "Inst Failed"},
	{AVND_LOG_SU_FSM_ST_TERMINATIONFAILED, "Term Failed"},
	{AVND_LOG_SU_FSM_EV_NONE, "None"},
	{AVND_LOG_SU_FSM_EV_INST, "Instantiate"},
	{AVND_LOG_SU_FSM_EV_TERM, "Terminate"},
	{AVND_LOG_SU_FSM_EV_RESTART, "Restart"},
	{AVND_LOG_SU_FSM_EV_COMP_INSTANTIATED, "Comp Instantiated"},
	{AVND_LOG_SU_FSM_EV_COMP_INST_FAIL, "Comp Inst Failed"},
	{AVND_LOG_SU_FSM_EV_COMP_RESTARTING, "Comp Restarting"},
	{AVND_LOG_SU_FSM_EV_COMP_TERM_FAIL, "Comp Term Failed"},
	{AVND_LOG_SU_FSM_EV_COMP_UNINSTANTIATED, "Comp Uninstantiated"},
	{AVND_LOG_SU_FSM_EV_COMP_TERMINATING, "Comp Terminating"},
	{AVND_LOG_SU_FSM_ENTER, "Enter"},
	{AVND_LOG_SU_FSM_EXIT, "Exit"},
	{0, 0}
};

/******************************************************************************
                      Canned string for Comp FSM
 ******************************************************************************/
const NCSFL_STR avnd_comp_fsm_set[] = {
	{AVND_LOG_COMP_FSM_ST_UNINSTANTIATED, "Uninstantiated"},
	{AVND_LOG_COMP_FSM_ST_INSTANTIATING, "Instantiating"},
	{AVND_LOG_COMP_FSM_ST_INSTANTIATED, "Instantiated"},
	{AVND_LOG_COMP_FSM_ST_TERMINATING, "Terminating"},
	{AVND_LOG_COMP_FSM_ST_RESTARTING, "Restarting"},
	{AVND_LOG_COMP_FSM_ST_INSTANTIATIONFAILED, "Inst Failed"},
	{AVND_LOG_COMP_FSM_ST_TERMINATIONFAILED, "Term Failed"},
	{AVND_LOG_COMP_FSM_ST_ORPHANED, "Orphaned"},
	{AVND_LOG_COMP_FSM_EV_NONE, "None"},
	{AVND_LOG_COMP_FSM_EV_INST, "Instantiate"},
	{AVND_LOG_COMP_FSM_EV_INST_SUCC, "Inst Succ"},
	{AVND_LOG_COMP_FSM_EV_INST_FAIL, "Inst Fail"},
	{AVND_LOG_COMP_FSM_EV_TERM, "Terminate"},
	{AVND_LOG_COMP_FSM_EV_TERM_SUCC, "Term Succ"},
	{AVND_LOG_COMP_FSM_EV_TERM_FAIL, "Term Fail"},
	{AVND_LOG_COMP_FSM_EV_CLEANUP, "Cleanup"},
	{AVND_LOG_COMP_FSM_EV_CLEANUP_SUCC, "Cleanup Succ"},
	{AVND_LOG_COMP_FSM_EV_CLEANUP_FAIL, "Cleanup Fail"},
	{AVND_LOG_COMP_FSM_EV_RESTART, "Restart"},
	{AVND_LOG_COMP_FSM_EV_ORPH, "Orphan"},
	{AVND_LOG_COMP_FSM_ENTER, "Enter"},
	{AVND_LOG_COMP_FSM_EXIT, "Exit"},
	{0, 0}
};

/******************************************************************************
                      Canned string for Error Processing
 ******************************************************************************/
const NCSFL_STR avnd_err_set[] = {
	{AVND_LOG_ERR_SRC_REP, "Error Reported "},
	{AVND_LOG_ERR_SRC_HC, " HC "},
	{AVND_LOG_ERR_SRC_PM, " PM "},
	{AVND_LOG_ERR_SRC_AM, " AM  "},
	{AVND_LOG_ERR_SRC_CMD_FAILED, " CMD Failed "},
	{AVND_LOG_ERR_SRC_CBK_HC_TIMEOUT, " HC CBK Timed out "},
	{AVND_LOG_ERR_SRC_CBK_HC_FAILED, " HC CBK Failed "},
	{AVND_LOG_ERR_SRC_AVA_DN, " AVA Down   "},
	{AVND_LOG_ERR_SRC_PXIED_REG_TIMEOUT, " Pxied Reg Failed"},
	{AVND_LOG_ERR_SRC_CBK_CSI_SET_TIMEOUT, " CSI-SET CBK Timed out "},
	{AVND_LOG_ERR_SRC_CBK_CSI_REM_TIMEOUT, " CSI-REM CBK Timed out "},
	{AVND_LOG_ERR_SRC_CBK_CSI_SET_FAILED, " CSI-SET CBK Failed "},
	{AVND_LOG_ERR_SRC_CBK_CSI_REM_FAILED, " CSI-REM CBK Failed "},
	{AVSV_LOG_AMF_NO_RECOMMENDATION, " NO RECOMMEND "},
	{AVSV_LOG_AMF_COMPONENT_RESTART, " Comp Restart "},
	{AVSV_LOG_AMF_COMPONENT_FAILOVER, " Comp Failover "},
	{AVSV_LOG_AMF_NODE_SWITCHOVER, " Node Switchover "},
	{AVSV_LOG_AMF_NODE_FAILOVER, " Node Failover "},
	{AVSV_LOG_AMF_NODE_FAILFAST, " Node Failfast "},
	{AVSV_LOG_AMF_CLUSTER_RESET, " Cluster Reset"},
	{AVSV_LOG_ERR_RCVR_SU_RESTART, " SU Restart "},
	{AVSV_LOG_ERR_RCVR_SU_FAILOVER, " SU Failover "},
	{0, 0}
};

/******************************************************************************
                      Canned string for Miscellaneous Events
 ******************************************************************************/
const NCSFL_STR avnd_misc_set[] = {
	{AVND_LOG_MISC_AVD_UP, "AvD Up"},
	{AVND_LOG_MISC_AVD_DN, "AvD Dn"},
	{AVND_LOG_MISC_AVA_UP, "AvA Up"},
	{AVND_LOG_MISC_AVA_DN, "AvA Dn"},
	{AVND_LOG_COMP_SCRIPT_INSTANTIATE, "Instantiate"},
	{AVND_LOG_COMP_SCRIPT_TERMINATE, "Terminate"},
	{AVND_LOG_COMP_SCRIPT_CLEANUP, "Cleanup"},
	{AVND_LOG_COMP_SCRIPT_AMSTART, "AMStart"},
	{AVND_LOG_COMP_SCRIPT_AMSTOP, "AMStop"},
	{0, 0}
};

/******************************************************************************
                      Canned string for Miscellaneous Events
 ******************************************************************************/
const NCSFL_STR avnd_fovr_set[] = {
	{AVND_LOG_FOVR_CLM_NODE_INFO, "CLM Node Info received on fail-over"},
	{AVND_LOG_FOVR_COMP_UPDT, "Comp updates are received on fail-over"},
	{AVND_LOG_FOVR_COMP_UPDT_FAIL, "Comp Update Failure"},
	{AVND_LOG_FOVR_SU_UPDT, "SU updates are received on fail-over"},
	{AVND_LOG_FOVR_SU_UPDT_FAIL, "SU Update Failure"},
	{AVND_LOG_FOVR_HLT_UPDT, "HLT updates are received on fail-over"},
	{AVND_LOG_FOVR_HLT_UPDT_FAIL, "HLT update Failure"},
	{AVND_LOG_MSG_ID_MISMATCH, "Message ID Mismatch"},
	{AVND_LOG_FOVR_VERIFY_MSG_RCVD, "Verify Message received by AVND"},
	{AVND_LOG_FOVR_AVD_SND_ID_CNT, "AVD send ID count"},
	{AVND_LOG_FOVR_AVND_RCV_ID_CNT, "AVND Receive ID count"},
	{AVND_LOG_FOVR_AVD_RCV_ID_CNT, "AVD receive ID count"},
	{AVND_LOG_FOVR_AVND_SND_ID_CNT, "AVND send ID count"},
	{AVND_LOG_FOVR_REC_SENT, "AVND record sent on fail-over"},
	{AVND_LOG_FOVR_REC_DEL, "AVND record deleted on fail-over"},
	{AVND_LOG_FOVR_REC_NOT_FOUND, "AVND record Not found"},
	{AVND_LOG_FOVR_MSG_ID_ACK_NACK, "Message ID ACK NACK"},
	{AVND_LOG_FOVR_VNUM_ACK_NACK, "View Number ACK NACK"},

	{0, 0}
};

/******************************************************************************
                      Canned string for Ntfs
 ******************************************************************************/
const NCSFL_STR avnd_ntf_set[] = {
	{AVND_LOG_NTFS_INSTANTIATION, "Instantiation"},
	{AVND_LOG_NTFS_TERMINATION, "Termination"},
	{AVND_LOG_NTFS_CLEANUP, "Cleanup"},
	{AVND_LOG_NTFS_OPER_STATE, "Oper State"},
	{AVND_LOG_NTFS_PRES_STATE, "Pres State"},
	{AVND_LOG_NTFS_ORPHANED, "Orphaned"},
	{AVND_LOG_NTFS_FAILED, "Failed"},

	{0, 0}
};

/******************************************************************************
                      Canned string for Oper State
 ******************************************************************************/
const NCSFL_STR avnd_oper_set[] = {
	{AVND_LOG_NTF_OPER_STATE_MIN, "do not use"},
	{AVND_LOG_NTF_OPER_STATE_ENABLE, "Enable"},
	{AVND_LOG_NTF_OPER_STATE_DISABLE, "Disable"},

	{0, 0}
};

/******************************************************************************
                      Canned string for Pres State 

 ******************************************************************************/
const NCSFL_STR avnd_pres_set[] = {
	{AVND_LOG_NTF_PRES_STATE_MIN, "do not use"},
	{AVND_LOG_NTF_PRES_STATE_UNINSTANTIATED, "Uninstantiated"},
	{AVND_LOG_NTF_PRES_STATE_INSTANTIATING, "Instantiating"},
	{AVND_LOG_NTF_PRES_STATE_INSTANTIATED, "Instantiated"},
	{AVND_LOG_NTF_PRES_STATE_TERMINATING, "Terminating"},
	{AVND_LOG_NTF_PRES_STATE_RESTARTING, "Restarting"},
	{AVND_LOG_NTF_PRES_STATE_INSTANTIATIONFAILED, "Instantiationfailed"},
	{AVND_LOG_NTF_PRES_STATE_TERMINATIONFAILED, "Terminationfailed"},
	{AVND_LOG_NTF_PRES_STATE_ORPHANED, "Orphaned"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for AvND-AvND messages 
 ******************************************************************************/
const NCSFL_STR avnd_avnd_set[] = {
	{AVND_AVND_SUCC_INFO, "AvND-AvND SUCC"},
	{AVND_AVND_ERR_INFO, "AvND-AvND ERROR"},
	{AVND_AVND_ENTRY_INFO, "AvND-AvND Entry Info"},
	{AVND_AVND_DEBUG_INFO, "AvND-AvND Info"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Headlines
 ******************************************************************************/
const NCSFL_STR avnd_hdln_set[] = {
	{AVND_INVALID_VAL, "AN INVALID DATA VALUE"},
	{AVND_UNKNOWN_MSG_RCVD, "UNKNOWN EVENT RCVD"},
	{AVND_MSG_PROC_FAILED, "EVENT PROCESSING FAILED"},
	{AVND_ENTERED_FUNC, "ENTRED THE FUNCTION"},
	{AVND_RCVD_VAL, "RECEIVED THE VALUE"},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/
NCSFL_SET avnd_str_set[] = {
	{AVND_FC_SEAPI, 0, (NCSFL_STR *)avsv_seapi_set},
	{AVND_FC_MDS, 0, (NCSFL_STR *)avsv_mds_set},
	{AVND_FC_EDU, 0, (NCSFL_STR *)avsv_edu_set},
	{AVND_FC_LOCK, 0, (NCSFL_STR *)avsv_lock_set},
	{AVND_FC_CB, 0, (NCSFL_STR *)avsv_cb_set},
	{AVND_FC_MBX, 0, (NCSFL_STR *)avsv_mbx_set},
	{AVND_FC_TASK, 0, (NCSFL_STR *)avsv_task_set},
	{AVND_FC_TMR, 0, (NCSFL_STR *)avnd_tmr_set},
	{AVND_FC_EVT, 0, (NCSFL_STR *)avnd_evt_set},
	{AVND_FC_CLM_DB, 0, (NCSFL_STR *)avnd_clm_db_set},
	{AVND_FC_PG_DB, 0, (NCSFL_STR *)avnd_pg_db_set},
	{AVND_FC_SU_DB, 0, (NCSFL_STR *)avnd_su_db_set},
	{AVND_FC_COMP_DB, 0, (NCSFL_STR *)avnd_comp_db_set},
	{AVND_FC_HC_DB, 0, (NCSFL_STR *)avnd_hc_db_set},
	{AVND_FC_SU_FSM, 0, (NCSFL_STR *)avnd_su_fsm_set},
	{AVND_FC_COMP_FSM, 0, (NCSFL_STR *)avnd_comp_fsm_set},
	{AVND_FC_ERR, 0, (NCSFL_STR *)avnd_err_set},
	{AVND_FC_MISC, 0, (NCSFL_STR *)avnd_misc_set},
	{AVND_FC_FOVR, 0, (NCSFL_STR *)avnd_fovr_set},
	{AVND_FC_NTF, 0, (NCSFL_STR *)avnd_ntf_set},
	{AVND_FC_OPER, 0, (NCSFL_STR *)avnd_oper_set},
	{AVND_FC_PRES, 0, (NCSFL_STR *)avnd_pres_set},
	{AVND_FC_HDLN, 0, (NCSFL_STR *)avnd_hdln_set},
	{AVND_FC_AVND_MSG, 0, (NCSFL_STR *)avnd_avnd_set},

	{0, 0, 0}
};

NCSFL_FMAT avnd_fmat_set[] = {
	/* <SE-API Create/Destroy> <Success/Failure> */
	{AVND_LID_SEAPI, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <MDS Register/Install/...> <Success/Failure> */
	{AVND_LID_MDS, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <EDU Init/Finalize> <Success/Failure> */
	{AVND_LID_EDU, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <Lck Init/Finalize/Take/Give> <Success/Failure> */
	{AVND_LID_LOCK, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <CB Create/Destroy/...> <Success/Failure> */
	{AVND_LID_CB, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <Mbx Create/Attach/...> <Success/Failure> */
	{AVND_LID_MBX, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <Task Create/Start/...> <Success/Failure> */
	{AVND_LID_TASK, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* Processed <tmr-type>; Operation: <Start/Stop/...>; Status: <Success/...> */
	{AVND_LID_TMR, NCSFL_TYPE_TIII, "[%s] Processed %s Operation: %s  Status: %s\n"},

	/* Rcvd <evt-type> */
	{AVND_LID_EVT1, NCSFL_TYPE_TI, "[%s] Rcvd %s\n"},

	/* Processed <evt-type> <Success/Failure> */
	{AVND_LID_EVT2, NCSFL_TYPE_TII, "[%s] Processed %s %s\n"},

	/* <evt-type> <Create/Destroy/Send> <Success/Failure> */
	{AVND_LID_EVT3, NCSFL_TYPE_TIII, "[%s] %s %s %s\n"},

	/* <Clm Db Create/Destroy/..> <Success/Failure>; NodeId: <node-id> */
	{AVND_LID_CLM_DB, NCSFL_TYPE_TIIL, "[%s] %s %s; NodeId: %ld\n"},

	/* <PG Db Create/Destroy/..> <Success/Failure>; CSI: <csi-name> */
	{AVND_LID_PG_DB, NCSFL_TYPE_TIIC, "[%s] %s %s; CSI: %s\n"},

	/* <SU Db Create/Destroy/..> <Success/Failure>; SU: <su-name> */
	{AVND_LID_SU_DB1, NCSFL_TYPE_TIIC, "[%s] %s %s; SU: %s\n"},

	/* <SI Add/Del/Assign/Remove> <Success/Failure>; SU: <su-name>; SI: <si-name> */
	{AVND_LID_SU_DB2, NCSFL_TYPE_TIICC, "[%s] %s %s; SU: %s; SI: %s\n"},

	/* <Comp Db Create/Destroy/..> <Success/Failure>; Comp: <comp-name> */
	{AVND_LID_COMP_DB1, NCSFL_TYPE_TIIC, "[%s] %s %s; Comp: %s\n"},

	/* <CSI Add/Del/Assign/Remove> <Success/Failure>; Comp: <comp-name>; CSI: <csi-name> */
	{AVND_LID_COMP_DB2, NCSFL_TYPE_TIICC, "[%s] %s %s; Comp: %s; CSI: %s\n"},

	/* <HC Db Create/Destroy/..> <Success/Failure>; HCKey: <hc-key> */
	{AVND_LID_HC_DB, NCSFL_TYPE_TIIC, "[%s] %s %s; HCKey: %s\n"},

	/* SU FSM <Enter/Exit>;  St: <state> Ev: <event>; SU: <su-name> */
	{AVND_LID_SU_FSM, NCSFL_TYPE_TIIIC, "[%s] SU FSM %s;  St: %s Ev: %s; SU: %s\n"},

	/* Comp FSM <Enter/Exit>;  St: <state> Ev: <event>; Comp: <comp-name> */
	{AVND_LID_COMP_FSM, NCSFL_TYPE_TIIIC, "[%s] Comp FSM %s;  St: %s Ev: %s; Comp: %s\n"},

	/* Error Rcvr  src: <HC/PM/AM/..> ; Rcvr:<NO RECOMEND/...>; Comp: <comp-name> */
	{AVND_LID_ERR, NCSFL_TYPE_TIIC, "[%s] Error Rcvr Src: %s; Rcvr: %s; Comp: %s\n"},

	/* <AvD Up/Down...> */
	{AVND_LID_MISC1, NCSFL_TYPE_TI, "[%s] %s\n"},

	/* Invoked <comp-instantiate/terminate/...> Cmd for Comp: <comp-name> */
	{AVND_LID_MISC2, NCSFL_TYPE_TIC, "[%s] Invoked %s Script for Comp: %s\n"},

	/* Failover Events at AVND on AVD fail-over */
	{AVND_LID_FOVER, NCSFL_TYPE_TICLL, "[%s] Role Change Event %s %s %ld %ld\n"},

	/* COMPONENT /Instantiation/Termination/Cleanup Failed */
	{AVND_LID_NTFS_CLC, "TCII", "[%s] %s %s %s\n"},

	/* SU Oper State Changed to .. */
	{AVND_LID_NTFS_OPER_STAT, "TCI", "[%s] %s Oper State Changed To %s\n"},

	/* SU Pres State Changed to .. */
	{AVND_LID_NTFS_PRES_STAT, "TCI", "[%s] %s Pres State Changed To %s\n"},

	/* Proxy Component Failed Proxied Component Orphaned */
	{AVND_LID_NTFS_PROXIED, "TCICI", "[%s] %s %s %s %s\n"},

	{AVND_LID_NTFS_COMP_FAIL, "TCLI", "[%s] %s Failed on Node Id:  0x%08lx Error Src: %s\n"},
	{AVND_LID_HDLN, "TIC", "[%s] TRACE INFO : %s %s\n"},
	{AVND_LID_HDLN_VAL, "TICLL", "[%s] TRACE INFO : %s at %s:%ld val %ld\n"},
	{AVND_LID_HDLN_VAL_NAME, "TICLP", "[%s] TRACE INFO : %s at %s:%ld val %s\n"},
	{AVND_LID_HDLN_STRING, "TICLC", "[%s] TRACE INFO : %s at %s:%ld val %s\n"},
	{AVND_AVND_MSG, "TICCLLLL", "[%s]: %s : %s: (%s, %ld, %ld, %ld, %ld)\n"},
	{AVND_LID_GENLOG, "TC", "%s %s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC avnd_ascii_spec = {
	NCS_SERVICE_ID_AVND,	/* AvND service id */
	AVSV_LOG_VERSION,	/* AvND revision id */
	"AvND",			/* used for generating log filename */
	(NCSFL_SET *)avnd_str_set,	/* AvND const strings ref by index */
	(NCSFL_FMAT *)avnd_fmat_set,	/* AvND string format info */
	0,			/* placeholder for str_set cnt */
	0			/* placeholder for fmat_set cnt */
};

/****************************************************************************
  Name          : avnd_str_reg
 
  Description   : This routine registers the AvND canned strings with DTS.
                  
  Arguments     : None
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 avnd_str_reg(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &avnd_ascii_spec;

	rc = ncs_dtsv_ascii_spec_api(&arg);

	return rc;
}

/****************************************************************************
  Name          : avnd_str_unreg
 
  Description   : This routine unregisters the AvND canned strings from DTS.
                  
  Arguments     : None
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 avnd_str_unreg(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_AVND;
	arg.info.dereg_ascii_spec.version = AVSV_LOG_VERSION;

	rc = ncs_dtsv_ascii_spec_api(&arg);

	return rc;
}

#endif   /* (NCS_DTS == 1) */
