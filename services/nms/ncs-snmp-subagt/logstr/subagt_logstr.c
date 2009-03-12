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

  
  .....................................................................  
  
  DESCRIPTION: This file has the canned strings for the SUBAGT Logs
  ***************************************************************************/ 
#include "ncsgl_defs.h"  
#include "t_suite.h"
#include "subagt_log.h"
#include "dts_papi.h"  

extern uns32 subagt_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);

const NCSFL_STR subagt_log_hdln_str[] =
{
   { SNMPSUBAGT_CB_NULL,                 "Subagt CB Null"}, 
   { SNMPSUBAGT_ARG_NULL,                "Subagt Arguments Null"}, 
   { SNMPSUBAGT_CB_CREATE_SUCCESS,       "Subagt CB Successfully Created"},
   { SNMPSUBAGT_DESTROY_SUCCESS,         "Subagt Successfully Destroyed"},
   { SNMPSUBAGT_CB_SET_NULL,             "Subagt CB set to Null"}, 
   { SNMPSUBAGT_AMF_CB_NULL,             "Subagt AMF CB found Null"}, 
   { SNMPSUBAGT_AMF_STATE_CHNGE_SUCCESS, "Subagt AMF State Change Success"}, 
   { NETSNMP_DS_SET_BOOLEAN,             "Netsnmp lib func ds set bool invoked"},
   { INIT_SNMP_DONE,                     "Init Snmp lib func invoked"},  
   { INIT_AGENT_DONE,                    "Init Snmp agent lib func invoked"}, 
   { SNMP_SHUTDOWN_DONE,                 "Subagt shut down succesfull"}, 
   { SNMPSUBAGT_AMF_FD_CLR,              "Subagt AMF FD Sel Obj Cleared"}, 
   { SNMPSUBAGT_EDA_FD_CLR,              "Subagt EDA FD Sel Obj Cleared"}, 
   { SNMPSUBAGT_RUN_ALARMS,              "Run Alarms lib Func invoked"}, 
   { SNMPSUBAGT_OID_LEN_NULL,            "Subagt OID lenght null received"}, 
   { SNMPSUBAGT_OID_BASE_NULL,           "Subagt OID base found Null"}, 
   { SNMPSUBAGT_MEM_ALLOC_PAT_NODE_FAIL, "Subagt Mem Failure while PAT Node Alloc"}, 
   { SNMPSUBAGT_OID_DB_NODE_NULL,        "Subagt OID db Node Null"}, 
   { SNMPSUBAGT_OID_DB_NODE_DEL_FAIL,    "Subagt OID DB Node Delete Ops failed"}, 
   { SNMPSUBAGT_OID_DB_NULL,             "Subagt OID DB found Null"}, 
   { SNMPSUBAGT_ALARM_UNREG_DONE,        "Subagt unregistered the PING Alarm"}, 
   { SNMPSUBAGT_CLOSE_SESSIONS_DONE,     "Subagt Closed the session with Agent"}, 
   { SNMPSUBAGT_THREAD_EXIT_BEING_INVOKED,"Subagt thread exited..."}, 
   { SNMPSUBAGT_EVT_DATA_NULL,            "Subagt Event data null"}, 
   { SNMPSUBAGT_DONOT_INFORM_TRAP,        "Subagt inform trap set to false"},
   { SNMPSUBAGT_TRAP_VARBIND_NULL,        "Subagt trap varbind is NULL"},
   { SNMPSUBAGT_TRAP_VAR_ELEM_NULL,       "Subagt trap var element is NULL"},
   { SNMPSUBAGT_MIB_PARAM_VAL_NULL,       "Subagt MIB Param Val is NULL"},
   { SNMPSUBAGT_OBJ_DETAILS_NULL,         "Subagt Object details are  NULL"},
   { SNMPSUBAGT_MBX_REG_DEREG_NULL,       "Subagt Mailbox init/deinit process() received a NULL string"},
   { SNMPSUBAGT_LOCK_TAKEN,               "Subagt CB is LOCKed"},
   { SNMPSUBAGT_LOCK_RELEASED,            "Subagt CB is UNLOCKed"},
   { SNMPSUBAGT_AMF_HLTH_CHECK_STARTED,   "Subagt Health Check Started"},  
   { SNMPSUBAGT_AMF_HLTH_CHECK_STOPPED,   "Subagt Health Check Stopped"},  
   { SNMPSUBAGT_AMF_HLTH_CHECK_STARTED_ALREADY, "Subagt Health Check already started"},
   { SNMPSUBAGT_AGT_MONITORING_STARTED,   "Subagt started monitoring the SNMP Agent"},
   { SNMPSUBAGT_AGT_RESTART_THRSLD_CROSSED,"SNMP Agent restart crossed the threshold. Recommending the AMF Framework for a switchover"},
   { SNMPSUBAGT_AGT_RESTART,               "Restarting the SNMP Agent"},
   { SNMPSUBAGT_AGT_MONITOR_FAILED,        "SNMP Agent Monitoring did NOT kicked off..."},
   { SNMPSUBAGT_AGT_NOT_MONITORED,        "SNMP Agent is NOT Monitored by the SubAgent..."},
   { SNMPSUBAGT_AGT_MONITORED,            "SNMP Agent is being Monitored by the SubAgent..."},
   { SNMPSUBAGT_JUMP_TO_NEXT_COLUMN,       "Jumping to next column"},
   {0,0}
};

const NCSFL_STR subagt_log_intf_init_state_str[] =
{
   { SNMPSUBAGT_AMF_INIT_SUCCESS,         "Subagt AMF interface initialization success"},
   { SNMPSUBAGT_EDA_INIT_SUCCESS,         "Subagt EDA interface initialization success"},
   { SNMPSUBAGT_DTA_INIT_SUCCESS,         "Subagt DTSv interface initialization success"},
   { SNMPSUBAGT_AGENT_INIT_SUCCESS,       "Subagt Agentx Master Agent interface initialized successfully"},
   { SNMPSUBAGT_MAC_INIT_SUCCESS,         "Subagt MAC interface initialization success ..."},
   { SNMPSUBAGT_INIT_SUCCESS,             "Subagt intialized successfully, waiting for the requests..."},
   { SNMPSUBAGT_AMF_FINALIZE_SUCCESS,     "Subagt AMF interface finalization success"},
   { SNMPSUBAGT_EDA_FINALIZE_SUCCESS,     "Subagt EDA interface finalization success"},
   { SNMPSUBAGT_DTA_FINALIZE_SUCCESS,     "Subagt DTSv interface finalization success"},
   { SNMPSUBAGT_AGENT_FINALIZE_SUCCESS,   "Subagt Agentx Master Agent interface finalization successfully"},
   { SNMPSUBAGT_MAC_FINALIZE_SUCCESS,     "Subagt MAC interface finalization success ..."},
   { SNMPSUBAGT_AMF_INIT_RETRY,            "AMF Initialisation is in retry mode"},
   { SNMPSUBAGT_AMF_COMP_REG_RETRY,        "Subagt component registration with AMF is in retry mode"},
   { SNMPSUBAGT_AMF_COMP_REG_COMPLETED,    "Subagt component registration with AMF is completed"},
   { SNMPSUBAGT_EDA_INIT_RETRY,            "EDA Initialisation is in retry mode"},
   { SNMPSUBAGT_EDA_SUBSCRIBE_SUCCESS,    "Subagt EDA Subscription success"},
   { SNMPSUBAGT_EDA_UNSUBSCRIBE_SUCCESS,  "Subagt EDA Unsubscription success"},
   { SNMPSUBAGT_PROCESSING_SIGHUP,          "Subagt received SIGHUP and processing it"},
   { SNMPSUBAGT_PROCESSING_SIGUSR2,          "Subagt received SIGUSR2 and processing it"},
   {0,0}
};

const NCSFL_STR subagt_log_func_entry_str[] =
{
    { SNMPSUBAGT_FUNC_ENTRY_SUBAGT_START,         "snmpsubagt_start()"}, /* 1 */
    { SNMPSUBAGT_FUNC_ENTRY_SVCS_INIT,            "snmpsubagt_svcs_init()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_CB_FINALISE,          "snmpsubagt_cb_finalise()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_CB_UPDATE,            "snmpsubagt_cb_update()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_DESTROY,              "ncs_snmpsubagt_destroy()"},
    { SNMPSUBAGT_FUNC_ENTRY_SUBAGT_DESTROY,       "snmpsubagt_destroy()"},
    { SNMPSUBAGT_FUNC_ENTRY_CLEANUP,              "snmpsubagt_cleanup()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_AMF_INIT,             "snmpsubagt_amf_initialize()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_AMF_FINALIZE,         "snmpsubagt_amf_finalize()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_AMF_CB_INIT,          "snmpsubagt_amf_callbacks_init()"}, /* 10 */
    { SNMPSUBAGT_FUNC_ENTRY_AMF_HLTH_CHK_CB,      "snmpsubagt_amf_health_check_callback()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_CSISET_CB,            "snmpsubagt_amf_CSISet_callback()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_INVALID_STATE_PROCESS,"snmpsubagt_invalid_state_process()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_ACTIVE_STATE_PROCESS,"snmpsubagt_ACTIVE_process()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_STANDBY_STATE_PROCESS,"snmpsubagt_STANDBY_process()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_QTOQ_STATE_PROCESS,   "snmpsubagt_QUIESCING_QUIESCED_process()"},
    { SNMPSUBAGT_FUNC_ENTRY_QUIESCED_STATE_PROCESS,"snmpsubagt_QUIESCED_process()"},
    { SNMPSUBAGT_FUNC_ENTRY_QUIESCING_STATE_PROCESS,  "snmpsubagt_QUIESCING_process()"},
    { SNMPSUBAGT_FUNC_ENTRY_AMF_TERMNAT_CB,       "snmpsubagt_amf_component_terminate_callback()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_AMF_CSIRMV_CB,        "snmpsubagt_amf_CSIRemove_callback()"}, /* 20 */
    { SNMPSUBAGT_FUNC_ENTRY_NETSNMP_LIB_INIT,     "snmpsubagt_netsnmp_lib_initialize()"},
    { SNMPSUBAGT_FUNC_ENTRY_NETSNMP_LIB_FINALIZE, "snmpsubagt_netsnmp_lib_finalize()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_NETSNMP_LIB_DEINIT,   "snmpsubagt_netsnmp_lib_deinit()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_RQST_PROCESS,         "snmpsubagt_request_process()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_TABLE_OID_ADD,        "snmpsubagt_table_oid_add()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_TABLE_OID_DEL,        "snmpsubagt_table_oid_del()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_TABLE_OID_DESTROY,    "snmpsubagt_table_oid_destroy()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_MBX_PROCESS ,         "snmpsubagt_mbx_process()"},
    { SNMPSUBAGT_FUNC_ENTRY_MBX_DESTROY_PROCESS,  "snmpsubagt_mbx_destroy_process()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_MBX_INIT_DEINIT_PROCESS, "snmpsubagt_mbx_reg_dereg_process()"}, /* 30 */
    { SNMPSUBAGT_FUNC_ENTRY_MAB_MAC_MSG_SEND,     "snmpsubagt_mab_mac_msg_send()"},
    { SNMPSUBAGT_FUNC_ENTRY_EDA_INIT,             "snmpsubagt_eda_initialize()"},
    { SNMPSUBAGT_FUNC_ENTRY_EDA_FINALIZE,         "snmpsubagt_eda_finalize()"},
    { SNMPSUBAGT_FUNC_ENTRY_EDA_CB,               "snmpsubagt_eda_callback()"},
    { SNMPSUBAGT_FUNC_ENTRY_TE_TO_AX,             "snmpsubagt_eda_trapevt_to_agentxtrap_populate()"},
    { SNMPSUBAGT_FUNC_ENTRY_SEND_V2_TRAP,         "subagt_send_v2trap()"}, 
    { SNMPSUBAGT_VAR_BIND_COMPOSE,                "snmpsubagt_eda_varbind_compose()"},
    { SNMPSUBAGT_FUNC_ENTRY_EDA_OBJ_GET,          "subagt_eda_object_get()"},
    { SNMPSUBAGT_FUNC_ENTRY_EDA_TRAP_VB_FREE,     "snmpsubagt_eda_trap_varbinds_free()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_EDA_VB_FREE,          "snmpsubagt_eda_varbinds_free()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_EDA_VB_LIST_APPEND,   "snmpsubagt_eda_varbindlist_append()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_EDA_TRAP_VB_POP,      "snmpsubagt_eda_trap_varbind_value_populate()"},
    { SNMPSUBAGT_FUNC_ENTRY_EDA_FULL_OID_POP,     "snmpsubagt_eda_full_oid_populate()"},
    { SNMPSUBAGT_FUNC_ENTRY_SNMP_TRAPID_VB_POP,   "snmpsubagt_eda_snmpTrapId_vb_populate()"},
    { SNMPSUBAGT_FUNC_INIT_DEINIT_MSG_POST,       "ncs_snmpsubagt_init_deinit_msg_post()"},
    { SNMPSUBAGT_FUNC_ENTRY_MAC_INITIALIZE,       "snmpsubagt_mac_initialize()"},
    { SNMPSUBAGT_FUNC_ENTRY_MAC_FINALIZE,         "snmpsubagt_mac_finalize()"},
    { SNMPSUBAGT_FUNC_ENTRY_HLTH_CHECK_START_POST, "snmpsubagt_amf_healthcheck_start_msg_post()"},
    { SNMPSUBAGT_FUNC_ENTRY_MBX_HEALTHCHECK_START, "snmpsubagt_mbx_healthcheck_start()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_APPL_MIBS_REG,         "snmpsubagt_appl_mibs_register()"}, /* 50 */
    { SNMPSUBAGT_FUNC_ENTRY_SPA_JOB,               "snmpsubagt_spa_job()"},
    { SNMPSUBAGT_FUNC_ENTRY_PENDING_LST_CMP,       "snmpsubagt_pending_regs_cmp()"},
    { SNMPSUBAGT_FUNC_ENTRY_PENDING_REGS_FREE,     "snmpsubagt_pending_regs_free()"},
    { SNMPSUBAGT_FUNC_ENTRY_PEND_REGS_PROC,        "snmpsubagt_pending_registrations_do()"},
    { SNMPSUBAGT_FUNC_ENTRY_PEND_LIST_FREE,        "snmpsubagt_pending_registrations_free()"},
    { SNMPSUBAGT_FUNC_ENTRY_PING_INT_CONF,         "snmpsubagt_netsnmp_agentx_pingInterval()"},
    { SNMPSUBAGT_FUNC_ENTRY_PARAM_FILL,            "snmpsubagt_mab_mib_param_fill()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_PARAM_TYPE_GET,        "snmpsubagt_mab_param_type_get()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_INDEX_EXTRACT,         "snmpsubagt_mab_index_extract()"},  
    { SNMPSUBAGT_FUNC_ENTRY_OID_COMPOSE,           "snmpsubagt_mab_oid_compose()"},  /* 60 */
    { SNMPSUBAGT_FUNC_ENTRY_RESP_PROCESS,          "snmpsubagt_mab_mibarg_resp_process()"},
    { SNMPSUBAGT_FUNC_ENTRY_ERROR_CODE_MAP,        "snmpsubagt_mab_error_code_map()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_UNREGISTER_MIB,        "snmpsubagt_mab_unregister_mib()"},
    { SNMPSUBAGT_FUNC_ENTRY_APPL_MIBS_UNREG,       "snmpsubagt_appl_mibs_unregister()"},
    { SNMPSUBAGT_FUNC_ENTRY_AGT_MONITOR_KICKOFF,   "snmpsubagt_agt_monitor_kickoff()"},
    { SNMPSUBAGT_FUNC_ENTRY_AGT_MONITOR,           "snmpsubagt_agt_monitor()"},
    { SNMPSUBAGT_FUNC_ENTRY_AGT_PING,              "snmpsubagt_agt_ping()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_AGT_MONITOR_STOP,      "snmpsubagt_agt_monitor_stop()"},/* 68*/
    { SNMPSUBAGT_FUNC_ENTRY_AMF_COMPONENTIZE,      "subagt_amf_componentize()"}, 
    { SNMPSUBAGT_FUNC_ENTRY_ACTIVE_ACTIVE_PROCESS,       "snmpsubagt_ACTIVE_ACTIVE_process"}, 
    { SNMPSUBAGT_FUNC_ENTRY_SIGUSR1_SIGNAL,        "subagt_process_sig_usr1_signal"}, 
    { SNMPSUBAGT_FUNC_ENTRY_RDA_INIT_ROLE_GET,     "subagt_rda_init_role_get"}, 
    {0,0}
};
const NCSFL_STR subagt_mem_fail_str[] =
{
    { SNMPSUBAGT_MEM_ALLOC,                         "Subagt Mem Alloc Invoked"}, 
    { SNMPSUBAGT_MEM_FREE,                          "Subagt Mem Free  Invoked"}, 
    { SNMPSUBAGT_MEM_FAIL,                          "Subagt Mem Alloc Failed"}, 
    { SNMPSUBAGT_EDA_VARBINDS_FREE_FAIL,            "Subagt Eda varbinds free Failed"}, 
    { SNMPSUBAGT_EDA_TRAP_VARBINDS_FREE_FAIL,       "Subagt Trap varbinds free Failed"}, 
    { SNMPSUBAGT_VAR_BIND_ALLOC_FAIL,               "Subagt Mem Alloc for Var bind Failed "}, 
    { SNMPSUBAGT_OID_ALLOC_FAIL,                    "Subagt Mem Alloc for OID Failed "}, 
    { SNMPSUBAGT_VAR_BIND_NAME_ALLOC_FAIL,          "Subagt Mem Alloc for vb  Name Failed "}, 
    { SNMPSUBAGT_PEND_REG_NODE_ALLOC_FAILED,        "Subagt Mem Alloc Pending Registrations failed"},
    {0,0}
};

const NCSFL_STR subagt_log_state_str[] =
{
   { SNMPSUBAGT_HA_STATE_CHNGE,            "Subagt HA State Changed"}, 
   { SNMPSUBAGT_HA_STATE_CHNGE_INVALID,    "Subagt HA State Change Invalid"},
   { SNMPSUBAGT_HA_STATE_INVALID,          "Subagt HA State Invalid"}, 

    {0,0}
}; 

const NCSFL_STR subagt_log_errors_str[] =
{
    { SNMPSUBAGT_NO_MBX,                     "Subagt mailbox is NULL"}, 
    { SNMPSUBAGT_AGT_INIT_FAILED,            "Subagt Agent init Failed"}, 
    { SNMPSUBAGT_MAC_INIT_FAILED,            "Subagt MAC init Failed"}, 
    { SNMPSUBAGT_INVALID_INPUT,              "Subagt Invalid Input "}, 
    { SNMPSUBAGT_AMF_INIT_FAILED,            "Subagt AMF Init Failed"}, 
    { SNMPSUBAGT_EDA_INIT_FAILED,            "Subagt EDA Init Failed"},
    { SNMPSUBAGT_EDA_REGISTER_FAILED,        "Subagt EDA Regsiter Failed"}, 
    { SNMPSUBAGT_EDA_DSPATCH_ERR,            "Subagt EDA Dispatch Failed"}, 
    { SNMPSUBAGT_RQST_PROCESS_FAILED,        "Subagt Rqst Process Failed"}, 
    { SNMPSUBAGT_FATAL_ERROR,                "Subagt Fatal Error"}, /* 10 */
    { SNMPSUBAGT_NETSNMP_LIB_FINALIZE_FAILED,"Subagt Net Snmp Finalise Failed"}, 
    { SNMPSUBAGT_EDA_FINALIZE_FAILED,        "Subagt EDA Finalise Failed"}, 
    { SNMPSUBAGT_MAC_FINALIZE_FAILED,        "Subagt MAC Finalise Failed"}, 
    { SNMPSUBAGT_CB_FINALIZE_FAILED,         "Subagt CB Finalise Failed"}, 
    { SNMPSUBAGT_DTA_FINALIZE_FAILED,        "Subagt DTA Finalise Failed"}, 
    { SNMPSUBAGT_DESTROY_FAILED,             "Subagt Destroy Failed"}, 
    { SNMPSUBAGT_AMF_CALLBACKS_INIT_FAILED,  "Subagt AMF CB Init Failed" }, 
    { SNMPSUBAGT_AMF_COMP_NAME_GET_FAILED,   "Subagt AMF Comp Name Get Failed"}, 
    { SNMPSUBAGT_AMF_COMP_REG_FAILED,        "Subagt Comp Register Failed"}, 
    { SNMPSUBAGT_AMF_SEL_OBJ_GET_FAILED,     "Subagt AMF Sel Obj get Failed"}, 
    { SNMPSUBAGT_AMF_DSPATCH_ERR,            "Subagt amf dispatch error"}, /* 20 */
    { SNMPSUBAGT_AMF_HLTH_CHK_STRT_FAILED,   "Subagt AMF HLTH CHK Start Failed"}, 
    { SNMPSUBAGT_AMF_HLTH_CHK_STOP_FAILED,   "Subagt AMF HLTH CHK Stop Failed"}, 
    { SNMPSUBAGT_AMF_STOP_FAILED,            "Subagt AMF Stopping Failed"}, 
    { SNMPSUBAGT_AMF_COMP_UNREG_FAILED,      "Subagt AMF Comp Unreg Failed"}, 
    { SNMPSUBAGT_AMF_FINALIZE_FAILED,        "Subagt AMF Finalize Failed"}, 
    { SNMPSUBAGT_STATE_CHNGE_FAILED,         "Subagt State Change Failed"}, 
    { SNMPSUBAGT_NETSNMP_LIB_DEINIT_FAILED,  "Subagt NETSNMP LIB Deinit Failed"}, 
    { SNMPSUBAGT_NETSNMP_LIB_INIT_FAILED,    "Subagt NETSNMP LIB Init Failed"}, 
    { SNMPSUBAGT_OID_DB_NODE_MALLOC_FAIL,    "Subagt OID DB NODE Malloc Failed"}, 
    { SNMPSUBAGT_MBX_MSG_MALLOC_FAIL,        "Subagt mbx message Malloc Failed"},/* 30 */ 
    { SNMPSUBAGT_EVT_CHNL_OPEN_FAIL,         "Subagt Evt Channel Open Failed"}, 
    { SNMPSUBAGT_EDA_EVT_SUBSCRIBE_FAIL,     "Subagt Evt Subscription Failed"}, 
    { SNMPSUBAGT_EDA_SEL_OBJ_GET_FAILED,     "Subagt Evt Selection object Get Failed"}, 
    { SNMPSUBAGT_EVT_INIT_FAIL,              "Subagt Evt Initialization Failed"}, 
    { SNMPSUBAGT_EVT_UNSUBSCRIBE_FAIL,       "Subagt Evt UnSubscribe Fail"},
    { SNMPSUBAGT_EVT_CHNL_CLOSE_FAIL,        "Subagt Evt Channel Close Failed"},
    { SNMPSUBAGT_EDA_EVT_DATA_GET_FAIL,      "Subagt Evt Data Get Failed"},
    { SNMPSUBAGT_EVT_SUBSCRIP_MISMATCH,      "Subagt Evt Subscription Mismatch"},
    { SNMPSUBAGT_EVT_ATTR_GET_FAIL,          "Subagt Evt Attribute get failed"}, 
    { SNMPSUBAGT_EVT_FREE_FAIL,              "Subagt Evt Free failed"},
    { SNMPSUBAGT_TE_AGENTXTRAP_FAILED,       "Subagt_eda_trapevt_to_agentxtrap_populate_failed"}, /* 40 */
    { SNMPSUBAGT_TRAP_DECODE_FAIL,           "Subagt_trap_decode_failed"}, 
    { SNMPSUBAGT_TRAP_SEND_FAIL,             "Subagt_trap_send_failed"}, 
    { SNMPSUBAGT_TRAPID_VB_POP_FAIL,         "Subagt TRAP-ID vb composing failed"},
    { SNMPSUBAGT_VAR_BIND_APPEND_FAIL,       "Subagt_trap_var_bind_append_failed"}, 
    { SNMPSUBAGT_TRAP_VARBIND_LIST_APPEND_FAIL,   "Subagt_trap_var_bind_list_append_failed"}, 
    { SNMPSUBAGT_EDA_OBJ_GET_FAIL,           "Subagt Unable to get the Selection object for EDSv"},   
    { SNMPSUBAGT_MBX_DESTROY_FAILED,         "Subagt destroy failed"}, 
    { SNMPSUBAGT_MBX_REG_DEREG_FAILED,       "Subagt reg/dereg of an OID failed"}, 
    { SNMPSUBAGT_MBX_INVALID_MSG,            "Subagt Mailbox received an illegal message"},
    { SNMPSUBAGT_MBX_IPC_SEND_FAILED,        "Unable to post the message to Subagt's mail box"}, /* 50 */ 
    { SNMPSUBAGT_CB_LOCK_FAILED,             "CB Lock failed"},
    { SNMPSUBAGT_CB_UNLOCK_FAILED,           "CB UNLock failed"},
    { SNMPSUBAGT_MBX_HLTH_CHECK_START_FAILED, "Mbx processing: healthcheck start failed"},
    { SNMPSUBAGT_HLTHCHECK_POST_FAILED,      "Unable to post HEALTHCHECK_START message"}, 
    { SNMPSUBAGT_PEND_REG_DLIST_ADD_FAILED,  "Pending registration addition to linked list failed"},
    { SNMPSUBAGT_PEND_REG_PROCESS_FAILED,    "Pending registration processing failed"},
    { SNMPSUBAGT_PEND_DEL_FAILED,            "Pending reg del failed"},
    { SNMPSUBAGT_AGT_MONITOR_TASK_CREATE_FAILED,"Agent Monitor task creation failed with error... "},
    { SNMPSUBAGT_AGT_MONITOR_TASK_START_FAILED,"Agent Monitor task start failed with error... "},
    { SNMPSUBAGT_AGT_MONITOR_OPEN_FAILED,"Unable to open the session with Agent to monitor..."},
    { SNMPSUBAGT_AGT_MONITOR_STOP_FAILED,"SNMP Agent Monitor STOP failed "},
    { SNMPSUBAGT_AGT_MONITOR_AMF_REPORT_FAILED,"Escalation to AMF about Agent Failed "},
    { SNMPSUBAGT_AGT_MONITOR_FLAG_INVALID,"motAgentMonitor tag holds an invalid value"},
    { SNMPSUBAGT_AGT_MONITOR_INTERVAL_INVALID,"motAgentMonitorInterval tag holds an invalid value"},
    { SNMPSUBAGT_AGT_MONITOR_MAX_TIMEOUT_INVALID,"motAgentMaxTimeOutCounter tag holds an invalid value"},
    { SNMPSUBAGT_AGT_MONITOR_MAX_RETRIES_INVALID,"motAgentMaxRestartCounter tag holds an invalid value"},
    { SNMPSUBAGT_TMR_FAIL,"Timer create failed."},
    { SNMPSUBAGT_MIB_REGISTER_FAIL,"Application MIBs register failed."},
    { SNMPSUBAGT_PENDING_REGISTER_FAIL,"pending registrations failed."},
    { SNMPSUBAGT_RDA_INIT_ROLE_GET_FAILED, "subagent rda Init role get failed"}, 
    { PCS_RDA_REQUEST_FAIL, "pcs rda request failed"}, 
    { SNMPSUBAGT_TIME_TO_WAIT_DISCREPANCY, "NCS SSA: snmp_select_info() returned more time to block than ping interval."}, 
    {0,0}
};

const NCSFL_STR subagt_log_error_strs_str[] = 
{
   { SNMPSUBAGT_DLIB_LOAD_NULL_FAILED,      "Subagt Reg/Dereg dl_load with NULL failed -- "},
   { SNMPSUBAGT_DLIB_SYM_FAILED,            "Subagt Reg/Dereg symbol lookup failed -- "},
   { SNMPSUBAGT_REG_DEREG_FAILED,           "Subagt OID Reg/Dereg failed -- "},
   { SNMPSUBAGT_REG_DEREG_SUCCESS,          "Subagt OID Reg/Dereg SUCCESS  -- "},
   { SNMPSUBAGT_PENDING_REG,                "Pending Registration -- "},
   { SNMPSUBAGT_REG_COMPLETED,              "All Registrations completed, last registration msg is -- "},
   { SNMPSUBAGT_SUBSYS_REG_FAILED,          "Subsystem Registration failed -- "},
   { SNMPSUBAGT_SUBSYS_UNREG_FAILED,        "Subsystem Unregistration failed -- "},
   { SNMPSUBAGT_AGT_MONITOR_NO_RSTRT_SCP,   "motAgentRestartScript is not present -- "},
   { SNMPSUBAGT_AGT_MONITOR_RSTRT_SCP_LARGE,"motAgentRestartScript is larger in length -- "},
   { SNMPSUBAGT_AGT_MONITOR_RSTRT_SCP_PERMISSIONS,"motAgentRestartScript  does not have executable permissions -- "},
   { 0,0}
};

const NCSFL_STR subagt_log_data_dump_str[] =
{
    { SNMPSUBAGT_AMF_SEL_OBJ,      "AMF Sel Obj"}, 
    { SNMPSUBAGT_EDA_SEL_OBJ,      "EDA Sel Obj"}, 
    { SNMPSUBAGT_IPC_MBX_SEL_OBJ,  "IPC Mailbox Sel Obj"}, 
    { SNMPSUBAGT_NUM_FDS,          "Number of FDs in Select "}, 
    { SNMPSUBAGT_AMF_FD_ISSET,     "AMF FD is SET"},  
    { SNMPSUBAGT_NETSNMP_FD_ISSET, "NETSNMP FD is SET"}, 
    { SNMPSUBAGT_EDA_FD_ISSET,     "EDA FD is SET"}, 
    { SNMPSUBAGT_IPC_MBX_FD_ISSET, "message recv: IPC Mailbox FD"}, 
    { SNMPSUBAGT_SPL_FD_PROCESS,   "select() returned..."}, 
    { SNMPSUBAGT_AGT_TOUT_THRSLD,   "SNMP Agent timed out count: "},
    { SNMPSUBAGT_AGT_RESTART_THRSLD,"SNMP Agent Restart count: "},
    {0,0}
};

const NCSFL_STR subagt_log_memdump_str[] =
{ 
   { SNMPSUBAGT_NCSMIB_ARG_TBL_DUMP ,      "NCSMIB_ARG TBL DATA "}, 
   { SNMPSUBAGT_NCSTRAP_DUMP,              "NCSTRAP data dump"}, 
   { SNMPSUBAGT_NCSMIB_ARG_INSTID_DUMP,    "Instance_ids "}, 
   { SNMPSUBAGT_NCSMIB_ARG_PARAM_INFO_DUMP, "Param_info"}, 
   { SNMPSUBAGT_NCSMIB_ARG_INT_INFO_DUMP,   "Int_param_val"},
   { SNMPSUBAGT_NCSMIB_ARG_OCT_INFO_DUMP,   "Oct_param_val"},
   { SNMPSUBAGT_NCSTRAP_TBL_DUMP,           "Trap_Tbl_info"},
   { SNMPSUBAGT_NCSTRAP_VB_TBL_DUMP,        "Trap_varbind_info"},
 
   {0,0}
}; 

const NCSFL_STR subagt_log_generated_strs[] =
{ 
    {0,""},
    {0,0}
}; 

const NCSFL_STR subagt_log_table_id_str[] =
{ 
    { SNMPSUBAGT_NODE_ADDED_IN_OID_DB,"Table-id added: "},
    { SNMPSUBAGT_NODE_DEL_FROM_OID_DB,"Table-id deleted:"},
    {0,0}
}; 

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
******************************************************************************/
NCSFL_SET subagt_log_str_set[] = 
{
    { SNMPSUBAGT_FS_HDLN,         0, (NCSFL_STR*) subagt_log_hdln_str},
    { SNMPSUBAGT_FS_FUNC_ENTRY,   0, (NCSFL_STR*) subagt_log_func_entry_str},
    { SNMPSUBAGT_FS_MEM,          0, (NCSFL_STR*) subagt_mem_fail_str},
    { SNMPSUBAGT_FS_DATA_DUMP,    0, (NCSFL_STR*) subagt_log_data_dump_str},
    { SNMPSUBAGT_FS_STATE,        0, (NCSFL_STR*) subagt_log_state_str},
    { SNMPSUBAGT_FS_ERRORS,       0, (NCSFL_STR*) subagt_log_errors_str}, 
    { SNMPSUBAGT_FS_MEMDUMP,      0, (NCSFL_STR*) subagt_log_memdump_str}, 
    { SNMPSUBAGT_FS_ERROR_STRS,   0, (NCSFL_STR*) subagt_log_error_strs_str},
    { SNMPSUBAGT_FS_GEN_STR,      0, (NCSFL_STR*) subagt_log_generated_strs},
    { SNMPSUBAGT_FS_GEN_ERR,      0, (NCSFL_STR*) subagt_log_generated_strs},
    { SNMPSUBAGT_FS_GEN_OID_LOG,  0, (NCSFL_STR*) subagt_log_generated_strs},
    { SNMPSUBAGT_FS_INTF_INIT_STATE, 0, (NCSFL_STR*) subagt_log_intf_init_state_str},
    { SNMPSUBAGT_FS_LOG_TABLE_ID, 0, (NCSFL_STR*) subagt_log_table_id_str},
    { 0,0,0 }
};

NCSFL_FMAT subagt_fmat_set[] = 
{
    { SNMPSUBAGT_FMTID_HDLN,        NCSFL_TYPE_TI   ,  "SNMP SubAgt %s HEADLINE    : %s \n"         },
    { SNMPSUBAGT_FMTID_FUNC_ENTRY,  NCSFL_TYPE_TI   ,  "SNMP SubAgt %s FUNC        : %s \n"         },
    { SNMPSUBAGT_FMTID_MEM,         NCSFL_TYPE_TI   ,  "SNMP Subagt %s MEM         : %s \n"         },
    { SNMPSUBAGT_FMTID_DATA_DUMP,   NCSFL_TYPE_TIL  ,  "SNMP Subagt %s DATA DUMP   : %s [TABLE/FD] %ld \n"}, 
    { SNMPSUBAGT_FMTID_STATE,       NCSFL_TYPE_TILL ,  "SNMP Subagt %s STATE       : %s Svalue %ld %ld \n"},
    { SNMPSUBAGT_FMTID_ERRORS,      NCSFL_TYPE_TILLL,  "SNMP Subagt %s ERROR       : %s Evalue %ld %ld %ld \n"},
    { SNMPSUBAGT_FMTID_MEMDUMP,     NCSFL_TYPE_TID,    "SNMP Subagt %s MEMDUMP\n %s\n %s\n"},  
    { SNMPSUBAGT_FMTID_ERROR_STRS,  NCSFL_TYPE_TICLL,  "SNMP Subagt %s INFORM      : %s String  %s %ld %ld \n"},
    { SNMPSUBAGT_FMTID_GEN_STR,     NCSFL_TYPE_TIC,  "SNMP Subagt %s GEN           : %s %s \n"},
    { SNMPSUBAGT_FMTID_GEN_ERR,     NCSFL_TYPE_TICL,  "SNMP Subagt %s GEN-ERR      : %s %s %ld \n"},
    { SNMPSUBAGT_FMTID_GEN_OID_LOG,  NCSFL_TYPE_TID,  "SNMP Subagt %s OID\n %s %s\n"},
    { SNMPSUBAGT_FMTID_INTF_INIT_STATE, NCSFL_TYPE_TI,  "SNMP SubAgt %s Interface Init status    : %s \n"         },
    { SNMPSUBAGT_FMTID_LOG_TABLE_ID, NCSFL_TYPE_TIL  ,  "SNMP Subagt %s TABLE ID    : %s %ld \n"         },
    { 0, 0, 0 } 
};

NCSFL_ASCII_SPEC    snmpsubagt_str_set=
{
    NCS_SERVICE_ID_SNMPSUBAGT,      /* Service ID     */
    SNMPSUBAGT_LOG_VERSION,         /* Revision ID    */
    "SUBAGT",
    (NCSFL_SET*)subagt_log_str_set,  /* Log string set */
    (NCSFL_FMAT*)subagt_fmat_set,    /* Log format set */
    0,                               /* Placeholder for str_set  count */
    0                                /* Placeholder for fmat_set count */
};

/****************************************************************************
 * Name          : subagt_log_str_lib_req
 *
 * Description   : Library entry point for subagt log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
subagt_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = snmpsubagt_canned_str_reg(NCS_SERVICE_ID_SNMPSUBAGT);
         break;

      case NCS_LIB_REQ_DESTROY:
         res = snmpsubagt_canned_str_dereg(NCS_SERVICE_ID_SNMPSUBAGT);
         break;

      default:
         break;
   }
   return (res);
}

/******************************************************************************
 *  Name:          snmpsubagt_canned_str_reg
 *  Description:   Register the canned strings and the formats 
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note:
 ******************************************************************************/
uns32 
snmpsubagt_canned_str_reg(NCS_SERVICE_ID i_svc_id)
{
    uns32       status = NCSCC_RC_FAILURE;

    NCS_DTSV_REG_CANNED_STR     canned_str; 

    /* Level the ground */
    memset(&canned_str, 0, sizeof(NCS_DTSV_REG_CANNED_STR));

    /* register the canned string set with DTSV */ 
    canned_str.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
    canned_str.info.reg_ascii_spec.spec = &snmpsubagt_str_set;
    status = ncs_dtsv_ascii_spec_api(&canned_str);
    if (status != NCSCC_RC_SUCCESS)
    {
         m_NCS_CONS_PRINTF ("IN SNMPSUBAGT_DBG_SINK: line %d, file %s\n",
                            __LINE__,__FILE__);
    }
        
    return status; 
}

/******************************************************************************
 *  Name:          snmpsubagt_canned_str_dereg
 *  Description:   De-Register the canned strings and the formats 
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note:
 ******************************************************************************/
uns32 
snmpsubagt_canned_str_dereg(NCS_SERVICE_ID i_svc_id)
{
    uns32                       status = NCSCC_RC_FAILURE;
    NCS_DTSV_REG_CANNED_STR     canned_str; 

    /* Level the ground */
    memset(&canned_str, 0, sizeof(NCS_DTSV_REG_CANNED_STR));
 
    /* register the canned string set with DTSV */ 
    canned_str.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
    canned_str.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_SNMPSUBAGT;
    canned_str.info.dereg_ascii_spec.version = SNMPSUBAGT_LOG_VERSION;

    status = ncs_dtsv_ascii_spec_api(&canned_str);
    if (status != NCSCC_RC_SUCCESS)
    {
         m_NCS_CONS_PRINTF ("IN SNMPSUBAGT_DBG_SINK: line %d, file %s\n",
                            __LINE__,__FILE__);
    }
        
    return status; 
}

