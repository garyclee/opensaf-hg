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
  
  DESCRIPTION: This file describes the  Control Block realted functions
  
*****************************************************************************/ 
#ifndef SUBAGT_CB_H
#define SUBAGT_CB_H

#include "net-snmp/library/asn1.h"
#include "net-snmp/library/snmp.h"
#include "net-snmp/library/snmp_api.h"
#include "net-snmp/library/int64.h"

/* SubAgent thread priority */
#define NCS_SNMPSUBAGT_PRIORITY     NCS_TASK_PRIORITY_4

/* Task Size... In future a common macro may need to be used */ 
#define NCS_SNMPSUBAGT_STACK_SIZE   NCS_STACKSIZE_HUGE

typedef enum
{
    SNMPSUBAGT_TMR_MSG_AGT_MONITOR=1,
    SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT,
    SNMPSUBAGT_TMR_MSG_RETRY_AMF_INIT,
    SNMPSUBAGT_TMR_MSG_RETRY_CHECK_SNMPD,
    SNMPSUBAGT_TMR_MSG_MAX
}SNMPSUBAGT_TMR_MSG_TYPE;

typedef struct snmpsubagt_timer
{
    tmr_t                       timer_id;
    uns32                       period; /* in seconds */
    NCS_BOOL                    is_active;
    SNMPSUBAGT_TMR_MSG_TYPE     msg_type;
    void                        *cb;
}SNMPSUBAGT_TIMER;

typedef SNMPSUBAGT_TIMER SNMPSUBAGT_AGT_TIMER;

/* A doubly linked list, which holds the list of MIBs registerred
 * from the configuration file 
 */
typedef struct mibs_reg_list
{
    /* pointer to the next node */ 
    NCS_DB_LINK_LIST_NODE   node;

    /* unregister function pointer for this MIB */ 
    uns32    (*deinit_routine)();

    /* library handle for this MIB */
    NCS_OS_DLIB_HDL     *lib_hdl;
}SNMPSUBAGT_MIBS_REG_LIST;

/* NCS SubAgent Control Block */
typedef struct ncsSa_cb
{   
    /* lock to protect the CB data structure */
    NCS_LOCK            cb_lock; 
     
    /* Service Id (for logging, memory mgmt etc..,) */
    NCS_SERVICE_ID      svc_id;

    /* SubAgent's mail box */
    SYSF_MBX            subagt_mbx;

    /* task handle to process the SubAgent's events */
    NCSCONTEXT          subagt_task; 

#if 0
    /* Agent monitoring code is being removed, so these members are not required. 
    If agent monitoring is done by subagent, then they have to be defined */

    /* timer to monitor the Agent */
    SNMPSUBAGT_AGT_TIMER    agt_monitor_timer;

    /* SNMP Agent Monitor attributes */
    SNMPSUBAGT_AGT_MONITOR_ATTRIBS  agt_mntr_attrib;
#endif
    
    /* MDS Handle (for MAC purposes) */
    uns32               mds_hdl; 

    /* EDU handle for performing encode/decode operations */
    EDU_HDL         edu_hdl;

    /* SubAgent Blocking or NonBlocking mode */
    /* 0 - NonBlocking mode; 1 - Blocking mode */
    int             block; 

    /* NCS_MTBL_ID to base OID table mapping database */
    NCS_PATRICIA_TREE     oidDatabase;

    /* version info of the SAF implementation */
    SaVersionT          safVersion; 

    /* SubAgent  Flags */ 
    SaDispatchFlagsT   subagt_dispatch_flags;

    /* component Name */
    SaNameT             compName;
                                            
    /* HA Current state */ 
    SNMPSUBAGT_HA_STATE         haCstate; 

    /* Details about the standby peer  -- Is this required for us? 
    * I dont think so
    */ 
    SaAmfProtectionGroupMemberT *peerDetails; 
    
    /* AMF Callbacks */ 
    SaAmfCallbacksT             amfCallbacks; 

    /* healthcheck start flag */
    NCS_BOOL                     healthCheckStarted;
    
    /* Health Check Key, How do we get this information?? */ 
    SaAmfHealthcheckKeyT        healthCheckKey; 
    
    /* Health Check Invocation Type, subagent driven or AMF driven?? */ 
    SaAmfHealthcheckInvocationT invocationType; 
    
    /* Health Check Failure Recommended Recovery, 
     * How do we get this information 
     */ 
    SaAmfRecommendedRecoveryT       recommendedRecovery; 
     
    /* AMF Handle */ 
    SaAmfHandleT        amfHandle;
    
    /* for AMF communication */ 
    SaSelectionObjectT       amfSelectionObject;
    
    /* timer to retry the AMF Initialization */
    SNMPSUBAGT_TIMER    amf_timer;

    /* Initialisation status can take values - 
    m_SNMPSUBAGT_AMF_INIT_NOT_STARTED, m_SNMPSUBAGT_AMF_INIT_RETRY, m_SNMPSUBAGT_AMF_INIT_COMPLETED,
    m_SNMPSUBAGT_AMF_COMP_REG_RETRY, m_SNMPSUBAGT_AMF_COMP_REG_COMPLETED  */
    uns8       amfInitStatus;

    /*  EDA Handle */ 
    SaEvtHandleT        evtHandle;
    
    /* EDA subscription ID on this channel */
    SaEvtSubscriptionIdT    evtSubscriptionId; 
    
    /* EDA callback routine place holder */ 
    SaEvtCallbacksT     evtCallback; 
    
    /* EDA Channel Name */ 
    SaNameT             evtChannelName;
    
    /* EDA Channel Handle */
    SaEvtChannelHandleT evtChannelHandle; 

    /* Event Filtering Pattern string */
    SaUint8T            evtPatternStr[m_SNMPSUBAGT_EDA_EVT_FILTER_PATTERN_LEN+1];

    /* Event Pattern */
    SaEvtEventFilterT      evtFilter;
    
    /* EDA Filter Array */ 
    SaEvtEventFilterArrayT  evtFilters; 
    
    /* timer to retry the EDA Initialization */
    SNMPSUBAGT_TIMER    eda_timer;

    /* Initialisation status can take values - 
    m_SNMPSUBAGT_EDA_INIT_NOT_STARTED, m_SNMPSUBAGT_EDA_INIT_RETRY, m_SNMPSUBAGT_EDA_INIT_COMPLETED  */
    uns8       edaInitStatus;

    /* place holder to store the socket handler of the 
     * SubAgent thread 
     */ 
    SaSelectionObjectT       evtSelectionObject; 

    /* List of registered MIBs */
    NCS_DB_LINK_LIST         new_registrations;

    /* selection object to get the SIGHUP */
    NCS_SEL_OBJ      sighdlr_sel_obj;

    /* selection object to get the SIGUSR1 */ /* IR00061409 */
    NCS_SEL_OBJ      sigusr1hdlr_sel_obj;

    /* To hold the last registration message string */
    uns8             lastRegMsg[255];

    /* timer to retry the check for snmpd presence  */
    SNMPSUBAGT_TIMER    snmpd_timer;

    /* selection object to get the SIGUSR2 */
    NCS_SEL_OBJ      sigusr2hdlr_sel_obj;


}NCSSA_CB; 

/* define the Global Control Block */
NCSSA_CB *g_snmpsubagt_cb; 

/* set the Global Control Block */
#define m_SNMPSUBAGT_CB_SET(cb)\
     g_snmpsubagt_cb = cb

/* return the Global Control Block */
#define m_SNMPSUBAGT_CB_GET g_snmpsubagt_cb

#define m_SUBAGT_PID_FILE "/var/run/ncs_snmp_subagt.pid"

/* A valid address in which the data is present shall be
 * returned to the NET-SNMP library as part of the 
 * SNMP request servicing.  NET-SNMP Library reads the 
 * data from this location, fills in the GET_RESP SNMP PDU
 * and sends it back to the Agent.  So, it is very important
 * to have this variable.  
 *
 * In the current implementation of SYNC with MAC, since only one
 * request can be serviced at a time, there will not be any issue
 * in using this global variable without locks.  In future, if we migrate
 * to ASYNC communication with the MAC, this data structure may need to 
 * be protected with the LOCKS to avoid the data inconsistency/overwrite. 
 */
/* NOTE: 
   This varaible will not be used if the subagent uses the new api. 
   Once subagent is stabilised, this global variable can be deleted
   */   
NCSMIB_PARAM_VAL    rsp_param_val; 

/* This mac handle should have been a part of the CB itself. 
 * By having this variable as a global outside of DB, we can have
 * subagt_mab.h and subagt_mab.c independent of the Control Block. 
 * Otherwise, applications will have to include the subagt_cb.h
 * in their appl-mib-lib.so file. 
 */
uns32               g_subagt_mac_hdl;

/* to support counter64  values.  NCSMIB_PARAM_VAl can not hold a 64bit value */
U64                 g_counter64;

/* Function to allocate and initialize the Contorl Block */
NCSSA_CB*
snmpsubagt_cb_init(void);

/* Function to free the controlblock */
uns32
snmpsubagt_cb_finalize(NCSSA_CB *cb); 

/* Function to update the startup parameters to the CB */
EXTERN_C uns32
snmpsubagt_cb_update(NCSSA_CB   *cb,
                     int32      argc,
                     uns8       **arg);

EXTERN_C void snmpsubagt_timer_cb(void *arg);

EXTERN_C uns32 snmpsubagt_timer_start (NCSSA_CB *pSacb, SNMPSUBAGT_TMR_MSG_TYPE msgType);

#endif


