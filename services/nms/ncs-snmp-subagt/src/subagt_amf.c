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
  
  DESCRIPTION: This file describes the AMF Interface
        snmpsubagt_amf_initialize
            - To initialize the session with AMF Framework
            
        snmpsubagt_amf_finalize
            - To finalize the session with AMF
            
        snmpsubagt_amf_callbacks_init
            - To Init the AMF Callback functions 
            
        snmpsubagt_amf_health_check_callback
            - To HeartBeat with the AVA

        snmpsubagt_amf_component_terminate_callback
            - To handle the Component termination 

        snmpsubagt_amf_CSISet_callback
            - To Handle the HA State changes 

        snmpsubagt_amf_CSIRemove_callback
            - To remove the Component Service Instance
  ***************************************************************************/ 
#if (BBS_SNMPSUBAGT == 0)
#include "subagt.h"

#include "avsv_subagt_init.h"
#include "ifsv_subagt_init.h"
#include "ifsv_entp_subagt_init.h"
#include "ipxs_subagt_init.h"
#include "dtsv_subagt_init.h"
#include "avm_subagt_init.h"
#include "rda_papi.h"

typedef uns32 
(*SnmpSaReadinessAndHAStateProcess)(NCSSA_CB  *cb, 
                                    SaInvocationT   invocation);

static uns32 
snmpsubagt_invalid_state_process(NCSSA_CB    *cb, 
                                SaInvocationT       invocation);

/* IR00061409 */
/* Go to ACTIVE State from ACTIVE state*/ 
static uns32 
snmpsubagt_ACTIVE_ACTIVE_process (NCSSA_CB    *cb, 
                           SaInvocationT       invocation);


/* Go to QUIESCED State */
static uns32 
snmpsubagt_QUIESCED_process(NCSSA_CB    *cb, 
                           SaInvocationT       invocation);

/* Go to QUIESCING State */
static uns32 
snmpsubagt_QUIESCING_process(NCSSA_CB    *cb, 
                            SaInvocationT       invocation);

/* Go to QUIESCED from QUIESCING State */
static uns32 
snmpsubagt_QUIESCING_QUIESCED_process(NCSSA_CB         *cb, 
                            SaInvocationT       invocation);

static uns32 
snmpsubagt_STANDBY_STANDBY_process(NCSSA_CB         *cb, 
                           SaInvocationT    invocation);

/* to unregister the MIBs supported in NCS of all the services */
static uns32
snmpsubagt_appl_mibs_unregister(void);

/* IR00061409 */
static uns32
subagt_amf_componentize(NCSSA_CB *cb);

/* Initialize the HA state machine */
const SnmpSaReadinessAndHAStateProcess
      snmpsubagt_ReadinessAndHAStateDispatch[SNMPSUBAGT_HA_STATE_MAX]
                                            [SNMPSUBAGT_HA_STATE_MAX]=
{
    /* current HA State NULL -- Initial state of the SubAgent */
    {
        snmpsubagt_invalid_state_process, /* should never be the case*/
        snmpsubagt_ACTIVE_ACTIVE_process, /* Go to ACTIVE State */ /* IR00061409 */
        snmpsubagt_STANDBY_process, /* Go to STANDBY State */
        snmpsubagt_invalid_state_process, /* should never be the case*/
        snmpsubagt_invalid_state_process, /* should never be the case*/
    },
    /* Current HA State ACTIVE */
    {
        snmpsubagt_invalid_state_process,/* should never be the case*/
        snmpsubagt_ACTIVE_ACTIVE_process, /* Go to ACTIVE State */ /* IR00061409 */
        snmpsubagt_invalid_state_process,/* should never be the case*/
        snmpsubagt_QUIESCED_process,/* Go to QUIESCED State */
        snmpsubagt_QUIESCING_process,/* Go to QUIESCING State */
    },

    /* current HA State STANDBY */
    {
        snmpsubagt_invalid_state_process,/* should never be the case*/
        snmpsubagt_ACTIVE_process, /* Go to ACTIVE State */ 
        snmpsubagt_STANDBY_STANDBY_process, /* Go to STANDBY State */ /* IR00061409 */
        snmpsubagt_invalid_state_process,/* should never be the case*/
        snmpsubagt_invalid_state_process,/* should never be the case*/
    },
    /* current HA State QUIESCED */
    {
        snmpsubagt_invalid_state_process,/* should never be the case*/
        snmpsubagt_ACTIVE_process, /* Goto ACTIVE state */
        snmpsubagt_STANDBY_process, /* Goto STANDBY state */
        snmpsubagt_invalid_state_process,/* never be the case*/
        snmpsubagt_invalid_state_process,/* never be the case*/
    },
    /* current HA State QUIESCING */
    {
        snmpsubagt_invalid_state_process,/* never be the case*/
        snmpsubagt_ACTIVE_process, /* Goto ACTIVE state */
        snmpsubagt_invalid_state_process,/* never be the case*/
        snmpsubagt_QUIESCING_QUIESCED_process,/* Go to QUIESCED State */
        snmpsubagt_invalid_state_process,/* never be the case*/
    }
};

/* to initialize the AMF Callbacks */
static uns32
snmpsubagt_amf_callbacks_init(SaAmfCallbacksT     *amfCallbacks);

/* Describes the health check of the NCS SubAgent */
static void
snmpsubagt_amf_health_check_callback(SaInvocationT     invocation, 
                                    const SaNameT      *compName, 
                                    SaAmfHealthcheckKeyT  *checkKey);

/* HA State Handling (Failover and Switchover) */
static void
snmpsubagt_amf_CSISet_callback(SaInvocationT    invocation, 
                                const SaNameT   *compName, 
                                SaAmfHAStateT   haState, 
                          SaAmfCSIDescriptorT   csiDescriptor);

/* Component Terminate callback */
static void
snmpsubagt_amf_component_terminate_callback(SaInvocationT   invocation, 
                                        const SaNameT  *compName);

/* Component Service Instance Remove Callback */
static void
snmpsubagt_amf_CSIRemove_callback(SaInvocationT invocation, 
                                const SaNameT   *compName, 
                                const SaNameT   *csiName, 
                                SaAmfCSIFlagsT  csiFlags);

/******************************************************************************
 *  Name:          snmpsubagt_amf_initialize
 *
 *  Description:   To intialize the session with AMF Agent
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *                 NCSCC_RC_AMF_INIT_FAILURE   - Init Library failure 
 *                 NCSCC_RC_AMF_COMP_NAME_GET_FAILURE 
 *                 NCSCC_RC_AMF_COMP_REG_FAILURE 
 *                 NCSCC_RC_AMF_SEL_OBJ_FAILURE
 *                 NCSCC_RC_AMF_HEALTH_CHECK_FAILURE
 *  NOTE: 
 ******************************************************************************/
SaAisErrorT 
snmpsubagt_amf_initialize(NCSSA_CB *cb) 
{ 
    uns32   ret_code = NCSCC_RC_SUCCESS; 
    SaAisErrorT status = SA_AIS_OK; 
    SaVersionT          safVersion; 
 
    /* Log the entry into this function */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_AMF_INIT);

    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
        return SA_AIS_ERR_INVALID_PARAM; 
    }

    /* initialize the AMF Callbacks */
    ret_code = snmpsubagt_amf_callbacks_init(&cb->amfCallbacks);
    if (ret_code != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_CALLBACKS_INIT_FAILED, 
                                ret_code, 0, 0);
        return SA_AIS_ERR_INVALID_PARAM; 
    }


    /* initialize the SAF version details */
    safVersion.releaseCode = 'B';
    safVersion.majorVersion = 0x01;
    safVersion.minorVersion = 0x01;
    
    /* Initialize the AMF Library */
    status = saAmfInitialize(&cb->amfHandle, /* AMF Handle */
                    &cb->amfCallbacks, /* AMF Callbacks */ 
                    &safVersion /*Version of the AMF*/);
    if (status != SA_AIS_OK) 
    { 
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_INIT_FAILED, 
                            cb->amfHandle,status, 0);
        return status; 
    }
    
    /* get the communication Handle */
    status = saAmfSelectionObjectGet(cb->amfHandle, 
                                &cb->amfSelectionObject); 
    if (status != SA_AIS_OK)
    {
       /* log the error */ 
       m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_SEL_OBJ_GET_FAILED, 
                                cb->amfHandle,status, 0);
       saAmfFinalize(cb->amfHandle);
       cb->amfHandle = 0; 
       return status; 
    }

    /* log the selection object, if possible */
    m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_AMF_SEL_OBJ, cb->amfSelectionObject);

    /* get the component name */
    status = saAmfComponentNameGet(cb->amfHandle,  /* input */
                              &cb->compName); /* output */
    if (status != SA_AIS_OK) 
    { 
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_COMP_NAME_GET_FAILED, 
                                cb->amfHandle,status, 0);
        saAmfFinalize(cb->amfHandle);
        cb->amfHandle = 0; 
        return status;
    }

    cb->amfInitStatus = m_SNMPSUBAGT_AMF_INIT_COMPLETED;

    return SA_AIS_OK; 
}

 /*****************************************************************************\
 *  Name:          snmpsubagt_amf_finalize
 *
 *  Description:   To Finalize the session with the AMF 
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 \*****************************************************************************/
/* Unregistration and Finalization with AMF Library */
uns32
snmpsubagt_amf_finalize(NCSSA_CB *cb) 
{ 
    SaAisErrorT status = SA_AIS_OK; 

    /* log the function entry  */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_AMF_FINALIZE);

    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
        return NCSCC_RC_FAILURE; 
    }
    /* stop the timer. */
    if(cb->amf_timer.timer_id != TMR_T_NULL)
    {
       m_NCS_TMR_STOP(cb->amf_timer.timer_id);
    }

    /* destroy the timer. */
    m_NCS_TMR_DESTROY(cb->amf_timer.timer_id);


    /* delete the fd from the select list */ 
    m_NCS_MEMSET(&cb->amfSelectionObject, 0, sizeof(SaSelectionObjectT));

    /* Disable the health monitoring */ 
    status = saAmfHealthcheckStop(cb->amfHandle, 
                                &cb->compName, 
                                &cb->healthCheckKey); 
    if (status != SA_AIS_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_HLTH_CHK_STOP_FAILED, 
                                status, 0, 0);      
        /* continue finalization */
    }

    /* Unregister with AMF */ 
    status = saAmfComponentUnregister(cb->amfHandle, 
                                    &cb->compName, 
                                    NULL); 
    if (status != SA_AIS_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_COMP_UNREG_FAILED, status, 0, 0);      
        /* continue finalization */
    } 
    
    /* Finalize */ 
    status = saAmfFinalize(cb->amfHandle); 
    if (status != SA_AIS_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_FINALIZE_FAILED, status, 0, 0);      
        /* continue finalization */
    } 
    return NCSCC_RC_SUCCESS; 
}

/******************************************************************************
 *  Name:          snmpsubagt_amf_callbacks_init
 *
 *  Description:   To Initialize the AMF callbacks
 * 
 *  Arguments:     SaAmfCallbacksT *  - structure pointer to be filled in with
 *                                      the AMF callbacks
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
/* To initialize the AMF Callbacks */
static uns32
snmpsubagt_amf_callbacks_init(SaAmfCallbacksT     *amfCallbacks)
{
    uns32 status = NCSCC_RC_SUCCESS; 

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_AMF_CB_INIT);      
  
    /* do the checks on the validity of the pointer */
    if (amfCallbacks == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_CB_NULL); 
        return NCSCC_RC_FAILURE; 
    }

    /* Initialize all the callbacks to NULL */ 
    m_NCS_MEMSET(amfCallbacks, 0, sizeof(SaAmfCallbacksT)); 

    /* healthcheck callback */ 
    amfCallbacks->saAmfHealthcheckCallback = 
        snmpsubagt_amf_health_check_callback; 

    /* Component Terminate Callback */
    amfCallbacks->saAmfComponentTerminateCallback = 
        snmpsubagt_amf_component_terminate_callback;

    /* HA state Handling callback */ 
    amfCallbacks->saAmfCSISetCallback = snmpsubagt_amf_CSISet_callback;

    /* CSI Remove callback */ 
    amfCallbacks->saAmfCSIRemoveCallback = snmpsubagt_amf_CSIRemove_callback; 
    
    return status; 
}

/******************************************************************************
 *  Name:          snmpsubagt_amf_health_check_callback
 *
 *  Description:  For the Health Check of the SubAgent 
 * 
 *  Arguments:     SaInvocationT     invocation, - Invocation Id
 *                 SaNameT           *compName   - Component Name
 *                 SaAmfHealthcheckKeyT  *healthCheckKey - Key
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
/* Describes the health check of the NCS SubAgent */
static void
snmpsubagt_amf_health_check_callback(SaInvocationT     invocation, 
                                const SaNameT           *compName,
                                SaAmfHealthcheckKeyT  *healthCheckKey)
{ 
    SaAisErrorT    status = SA_AIS_OK; 

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_AMF_HLTH_CHK_CB);      
  
    /* get the control block */
    const NCSSA_CB *cb = m_SNMPSUBAGT_CB_GET; 
    if (cb == NULL)
    {
        /* log the error */
  
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_CB_NULL); 
  
        return; 
    }
    /* do the sanity check ?? But what all to check ? *?*/ 
    /* send the SA_AIS_OK response to the AVA */ 
    status = saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_amf_CSISet_callback
 *
 *  Description:   To handle the HA state transitions
 * 
 *  Arguments:     SaInvocationT    - Ref given by the AMF
 *                 compName         - Component Name  
 *                 haState          - HA State, destination
 *                 csiDescriptor    - CSI/worlkload description
 *
 *  Returns:      Nothing 
 *  NOTE: 
 *****************************************************************************/
static void
snmpsubagt_amf_CSISet_callback(SaInvocationT    invocation, 
                        const SaNameT           *compName, 
                        SaAmfHAStateT           haState, 
                        SaAmfCSIDescriptorT     csiDescriptor) 
{
    uns32           status = NCSCC_RC_SUCCESS; 

    /* Function entry log */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_CSISET_CB);
 
    /* get the Control Block */
    NCSSA_CB  *cb = m_SNMPSUBAGT_CB_GET;

    if (cb == NULL)
    {
        /* log the error*/
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_CB_NULL); 
  
        return;
    }

    /* Process the callback, based on the state */  
    status = snmpsubagt_ReadinessAndHAStateDispatch[cb->haCstate][haState](cb, 
                                                                invocation); 
    if (status == NCSCC_RC_SUCCESS)
    {
        /* log the state change */
        m_SNMPSUBAGT_STATE_LOG(SNMPSUBAGT_HA_STATE_CHNGE, cb->haCstate, haState);
        
        /* update the HA state */
        cb->haCstate= haState; 
    }
    else
    {
        /* log the state change failure */
        m_SNMPSUBAGT_STATE_LOG(SNMPSUBAGT_HA_STATE_INVALID, cb->haCstate, haState);
    }
    
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_invalid_state_process
 *
 *  Description:   To log the invalid state transition    
 *                      Same routine is being used for the readiness state 
 *                      as well as HA state transitions.
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block (i)
 *                 SaInvocationT - Ref. number given by AMF (i)
 *
 *  Returns:       NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
static uns32 
snmpsubagt_invalid_state_process(NCSSA_CB    *cb, 
                                SaInvocationT       invocation)
{
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_INVALID_STATE_PROCESS);

    saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION); 
    return NCSCC_RC_FAILURE; 

}

/* Go to ACTIVE State */ 
uns32 
snmpsubagt_ACTIVE_process(NCSSA_CB          *cb, 
                          SaInvocationT     invocation)
{
    uns32       status = NCSCC_RC_FAILURE; 
#if 0
    SaAisErrorT saf_status = SA_AIS_OK; 
#endif

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_ACTIVE_STATE_PROCESS);

#if 0 /* IR00061409 */
    if(cb->edaInitStatus == m_SNMPSUBAGT_EDA_INIT_COMPLETED)
    {
       /* EDSv Interface Initialisation is completed. Now subscribe for the events. */

       /* 
        * Subscribe for the events of our interest
        *
        * SubscriptionId -- An identifier that uniquely identifies a 
        * subscription on an event channel  and that is used as a 
        * parameter of saEvtEventDeliverCallback()
        *
        * Filtering criteria was set during the initialization of the CB 
        * (for "NCS_TRAP" pattern)
        */
       saf_status = saEvtEventSubscribe(cb->evtChannelHandle, /* input */
                                    &cb->evtFilters,/* input */
                                    cb->evtSubscriptionId);/* input */
       if (saf_status != SA_AIS_OK)
       {
           /* log the error */
           m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_EVT_SUBSCRIBE_FAIL,
                                  saf_status, cb->evtSubscriptionId, 0);

           if(saf_status == SA_AIS_ERR_TRY_AGAIN)
           {
              /* Subscribe api returned TRY_AGAIN, start the timer. */
              if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT) == NCSCC_RC_FAILURE)
              {
                 /* timer couldn't be started. log the failure. */
                 m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);

                 /* send error response to AMF. */
                     saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION);
                 return NCSCC_RC_FAILURE;
              }
           }
           else
           {
              /* Error returned is something other than TRY_AGAIN, send failure response to AMF. */
                  saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION);
              return NCSCC_RC_FAILURE;
           }
       }
       /* log the status SNMPSUBAGT_EDA_SUBSCRIBE_SUCCESS*/
       m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_SUBSCRIBE_SUCCESS);
    }

#if 0
    /* for unit testing */
    if (cb->haCstate != 0)
    {
        printf("\n======= I am assigned Active, I am going to sleep====\n");
        m_NCS_TASK_SLEEP(2000); 
        printf("\n======= I WOKE UP -----------------------------------\n");
    }
#endif

#endif  /* IR00061409 */
    
    /* 
    We are here as a result of one of the following cases.
    1. EDA initialisation is in retry mode 
    2. EDA initialisation is completed and subscribe is in retry mode. 
    3. EDA initialisation and subscribe are completed. 
    
    As Active state is assigned by AMF, now application mibs can be registered 
    */

    /* register the application MIBs with the Agent */ 
    status = snmpsubagt_appl_mibs_register();
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MIB_REGISTER_FAIL, status, 0, 0);

      #if 0  /* IR00061409 */

        /*
        Check edaInitStatus. If it is completed, then subscribe might have been success or it is in retry mode.
        If subscribe is success, then unsubscribe now. If subscribe is in retry mode, then stop the timer. 
        In both the cases, send error response to AMF as mibs register is failed. 
        */

        if (cb->edaInitStatus == m_SNMPSUBAGT_EDA_INIT_COMPLETED) 
        {
           if(saf_status == SA_AIS_ERR_TRY_AGAIN)
           {
              /* stop the timer. */
              m_NCS_TMR_STOP(cb->eda_timer.timer_id);
           }
           else
           {
              /* UnSubscribe to the NCS_TRAPs, not to service traps */
              saEvtEventUnsubscribe(cb->evtChannelHandle, /* input */
                                      cb->evtSubscriptionId);/* input */
              m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_UNSUBSCRIBE_SUCCESS);
           }
        }

      #endif /* IR00061409 */

        /* send error response to AMF and return status. */
        if(invocation != 0) /* IR00061409 */
            saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION); 
        return status;
    }

#if 0
    /* IR00061409-start */ 
    if(cb->edaInitStatus == m_SNMPSUBAGT_EDA_INIT_COMPLETED)
    {
       /* EDSv Interface Initialisation is completed. Now subscribe for the events. */

       /* 
        * Subscribe for the events of our interest
        *
        * SubscriptionId -- An identifier that uniquely identifies a 
        * subscription on an event channel  and that is used as a 
        * parameter of saEvtEventDeliverCallback()
        *
        * Filtering criteria was set during the initialization of the CB 
        * (for "NCS_TRAP" pattern)
        */
       saf_status = saEvtEventSubscribe(cb->evtChannelHandle, /* input */
                                    &cb->evtFilters,/* input */
                                    cb->evtSubscriptionId);/* input */
       if (saf_status != SA_AIS_OK)
       {
           /* log the error */
           m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_EVT_SUBSCRIBE_FAIL,
                                  saf_status, cb->evtSubscriptionId, 0);

           if(saf_status == SA_AIS_ERR_TRY_AGAIN)
           {
              /* Subscribe api returned TRY_AGAIN, start the timer. */
              if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT) == NCSCC_RC_FAILURE)
              {
                 /* timer couldn't be started. log the failure. */
                 m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);

                 /* send error response to AMF. */
                 if (invocation != 0) /* IR00061409 */
                     saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION);
                 return NCSCC_RC_FAILURE;
              }
           }
           else
           {
              /* Error returned is something other than TRY_AGAIN, send failure response to AMF. */
              if(invocation != 0) /* IR00061409 */
                  saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION);
              return NCSCC_RC_FAILURE;
           }
       }
       /* log the status SNMPSUBAGT_EDA_SUBSCRIBE_SUCCESS*/
       m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_SUBSCRIBE_SUCCESS);
    }
#endif

#if 0
    /* for unit testing */
    if (cb->haCstate != 0)
    {
        printf("\n======= I am assigned Active, I am going to sleep====\n");
        m_NCS_TASK_SLEEP(2000); 
        printf("\n======= I WOKE UP -----------------------------------\n");
    }
#endif
  /* IR00061409-end */

    /* send success response to AMF. */
    if(invocation != 0) /* IR00061409 */
        saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 

    return NCSCC_RC_SUCCESS; 
}

/* IR00061409 */
/* Go to ACTIVE State from ACTIVE state*/ 
static uns32 
snmpsubagt_ACTIVE_ACTIVE_process(NCSSA_CB          *cb, 
                          SaInvocationT     invocation)
{
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_ACTIVE_ACTIVE_PROCESS);
    if(cb->haCstate ==  SNMPSUBAGT_HA_STATE_ACTIVE)
    {
         saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 
         return NCSCC_RC_SUCCESS; 
    }
    else
         return NCSCC_RC_FAILURE;
}

/* Go to STANDBY State */
static uns32 
snmpsubagt_STANDBY_STANDBY_process(NCSSA_CB         *cb, 
                           SaInvocationT    invocation)
{
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_STANDBY_STATE_PROCESS);
    
    if (invocation != 0) /* IR00061409 */
        saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 
    return NCSCC_RC_SUCCESS; 
}

/* Go to STANDBY State */
uns32 
snmpsubagt_STANDBY_process(NCSSA_CB         *cb, 
                           SaInvocationT    invocation)
{
    SaAisErrorT saf_status = SA_AIS_OK; 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_STANDBY_STATE_PROCESS);
    
    if (invocation != 0) /* IR00061409 */
    {
        /* UnSubscribe to the NCS_TRAPs, not to service traps */
        saf_status = saEvtEventUnsubscribe(cb->evtChannelHandle, /* input */
                       cb->evtSubscriptionId);/* input */
        if (saf_status != SA_AIS_OK)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_UNSUBSCRIBE_FAIL,
                               saf_status, cb->evtSubscriptionId, cb->evtChannelHandle); 
            saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION); 
            return NCSCC_RC_FAILURE;
        }
        /* log the unsubscribe status */
        m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_UNSUBSCRIBE_SUCCESS);

        saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 
    }
    return NCSCC_RC_SUCCESS; 
}

/* Go to QUIESCED from QUIESCING State */
static uns32 
snmpsubagt_QUIESCING_QUIESCED_process(NCSSA_CB         *cb, 
                           SaInvocationT    invocation)
{
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_QTOQ_STATE_PROCESS);

    saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 
    return NCSCC_RC_SUCCESS; 
}

/* Go to QUIESCING State */
static uns32 
snmpsubagt_QUIESCED_process(NCSSA_CB         *cb, 
                           SaInvocationT    invocation)
{
    uns32       status = NCSCC_RC_FAILURE; 

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_QUIESCED_STATE_PROCESS);

    /* unregister the applications MIBs with the Agent */ 
    status = snmpsubagt_appl_mibs_unregister();
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_NETSNMP_LIB_DEINIT_FAILED,
                               status, 0, 0);
        saAmfResponse(cb->amfHandle, invocation, SA_AIS_ERR_FAILED_OPERATION); 
        return status;
    }

    saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 
    return NCSCC_RC_SUCCESS; 
}

/* Go to QUIESCING State */
static uns32 
snmpsubagt_QUIESCING_process(NCSSA_CB        *cb, 
                            SaInvocationT   invocation)
{
    uns32       status = NCSCC_RC_FAILURE; 
    SaAisErrorT saf_status = SA_AIS_OK; 
    
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_QUIESCING_STATE_PROCESS);

    /* complete processing the Pending SNMP Requests and Traps */ 

    /* send the response to AMF immediately */ 
    saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK); 

    /* de-register the MIBs, and unsubscribe to the Traps */ 
    status = snmpsubagt_QUIESCED_process(cb, invocation); 

    /* QUIESCING is complete */ 
    saf_status = saAmfCSIQuiescingComplete(cb->amfHandle, invocation, 
                   (status == NCSCC_RC_SUCCESS)?SA_AIS_OK:SA_AIS_ERR_FAILED_OPERATION); 
    if (saf_status != SA_AIS_OK)
    {
        /* log the failure */
    }

    return status; 
}
    
/******************************************************************************
 *  Name:          snmpsubagt_amf_component_terminate_callback
 *
 *  Description:   To handle the Component Terminate request from AMF 
 *                  - All the sessions will be closed and Subagent will Suicide
 * 
 *  Arguments:     SaInvocationT    - Ref given by the AMF
 *                 SaNameT          - Name of the Component
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
/* Component Terminate Callback */
static void
snmpsubagt_amf_component_terminate_callback(SaInvocationT   invocation, 
                                        const SaNameT  *compName)
{ 
    NCSSA_CB *cb = m_SNMPSUBAGT_CB_GET;
#if 0
    NCSCONTEXT task_id = NULL;
#endif

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_AMF_TERMNAT_CB);
    
    /* send response to AMF. */
    saAmfResponse(cb->amfHandle, invocation, SA_AIS_OK);
 
#if 0
    task_id = cb->subagt_task;
#endif

    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_THREAD_EXIT_BEING_INVOKED); 

    /* close all the sessions */ 
    snmpsubagt_destroy(cb, FALSE);

#if 0
    m_NCS_TASK_STOP(task_id);
    m_NCS_TASK_RELEASE(task_id);
#endif
  
    /* Suicide!!! */ 
    exit(0); 
} 

/******************************************************************************
 *  Name:          snmpsubagt_amf_CSIRemove_callback
 *
 *  Description:   To remove the instance of a component 
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block 
 *                 SaInvocationT    - Ref given by the AMF
 *                 compNam          - Name of the component (i)
 *                 csiName          - Name of the CSI (i)
 *                 csiFlags         - Flags...
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
/* Component Service Instance Remove Callback */
static void
snmpsubagt_amf_CSIRemove_callback(SaInvocationT   invocation, 
                                  const SaNameT   *compName, 
                                  const SaNameT   *csiName,
                                  SaAmfCSIFlagsT  csiFlags)
{
    uns32   status = NCSCC_RC_SUCCESS; 
 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_AMF_CSIRMV_CB);
 
    NCSSA_CB *cb = m_SNMPSUBAGT_CB_GET;

    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_CB_NULL); 
        return; 
    }

#if 0
    status = snmpsubagt_QUIESCED_process(cb, invocation);
    
    /* Rest the HA State */ 
    if (status == NCSCC_RC_SUCCESS)
#endif

    cb->haCstate = SNMPSUBAGT_HA_STATE_NULL; 
    
    saAmfResponse(cb->amfHandle, invocation, 
                  (status == NCSCC_RC_SUCCESS)?SA_AIS_OK:SA_AIS_ERR_FAILED_OPERATION); 

    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_amf_healthcheck_start_msg_post
 *
 *  Description:   posts a message to the subagt thread to start the 
 *                 health check 
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32
snmpsubagt_amf_healthcheck_start_msg_post(NCSSA_CB *cb) 
{
    uns32   status = NCSCC_RC_FAILURE; 
    SNMPSUBAGT_MBX_MSG  *post_me = NULL;

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_HLTH_CHECK_START_POST);

    /* update the message info */
    post_me = m_MMGR_SNMPSUBAGT_MBX_MSG_ALLOC;
    if (post_me == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_MBX_MSG_MALLOC_FAIL);
        return NCSCC_RC_OUT_OF_MEM;
    }
    
    m_NCS_MEMSET(post_me, 0, sizeof(SNMPSUBAGT_MBX_MSG));
    post_me->msg_type = SNMPSUBAGT_MBX_MSG_HEALTH_CHECK_START;

    /* post a message to the SubAgent's thread */
    status = m_NCS_IPC_SEND(&cb->subagt_mbx,
                            (NCS_IPC_MSG*)post_me,
                            NCS_IPC_PRIORITY_HIGH);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MBX_IPC_SEND_FAILED, status,0,0);
        return status;
    }
    return status;
}

/*****************************************************************************\
 *  Name:          snmpsubagt_appl_mibs_register                              * 
 *                                                                            *
 *  Description:   To register the MIBs supported in NCS 1.0                  * 
 *                                                                            *
 *  Arguments:     None                                                       * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
\*****************************************************************************/
uns32
snmpsubagt_appl_mibs_register() 
{
    uns32   status = NCSCC_RC_FAILURE;

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_APPL_MIBS_REG);
    
    /* register AVSV subsystem */ 
    status = subagt_register_avsv_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_REG_FAILED,
                                   "subagt_register_avsv_subsys()", 0, 0); 
    }

    /* register AvM subsystem */ 
    status = subagt_register_avm_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_REG_FAILED,
                                   "subagt_register_avm_subsys()", 0, 0); 
    }

    /* register IFSV subsystem */ 
    status = subagt_register_ifsv_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_REG_FAILED,
                                   "subagt_register_ifsv_subsys()", 0, 0); 
    }

    /* register IFSV Enterprise MIB */
    status = subagt_register_ifsv_entp_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_REG_FAILED,
                                   "subagt_register_ifsv_entp_subsys()", 0, 0); 
    }

    /* register IPXS subsystem */
    status = subagt_register_ipxs_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_REG_FAILED,
                                   "subagt_register_ipxs_subsys()", 0, 0); 
    }

    /* register DTSV subsystem */
    status = subagt_register_dtsv_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_REG_FAILED,
                                   "subagt_register_dtsv_subsys()", 0, 0); 
    }
   
    /* register test-mib - to be deleted once SPSV Integration is ready */
    status = snmpsubagt_spa_job(SNMPSUBAGT_LIB_CONF/* SYSCONFDIR "subagt_lib_conf" */, 1 /* REGISTER */);     
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
    }
     
    return status; 
}
/*****************************************************************************\
 *  Name:          snmpsubagt_appl_mibs_unregister                            * 
 *                                                                            *
 *  Description:   To unregister the MIBs supported                           *
 *                                                                            *
 *  Arguments:     None                                                       * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
\*****************************************************************************/
static uns32
snmpsubagt_appl_mibs_unregister() 
{
    uns32   status = NCSCC_RC_FAILURE;

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_APPL_MIBS_UNREG);
    
    /* unregister AVSV subsystem */ 
    status = subagt_unregister_avsv_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_UNREG_FAILED,
                                   "subagt_unregister_avsv_subsys()", 0, 0); 
    }

    /* unregister AvM subsystem */ 
    status = subagt_unregister_avm_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_UNREG_FAILED,
                                   "subagt_unregister_avm_subsys()", 0, 0); 
    }

    /* unregister IFSV subsystem */ 
    status = subagt_unregister_ifsv_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_UNREG_FAILED,
                                   "subagt_unregister_ifsv_subsys()", 0, 0); 
    }

    /* unregister IFSV Enterprise MIB */
    status = subagt_unregister_ifsv_entp_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_UNREG_FAILED,
                                   "subagt_unregister_ifsv_entp_subsys()", 0, 0); 
    }

    /* unregister IPXS subsystem */
    status = subagt_unregister_ipxs_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_UNREG_FAILED,
                                   "subagt_unregister_ipxs_subsys()", 0, 0); 
    }

    /* unregister DTSV subsystem */
    status = subagt_unregister_dtsv_subsys();  
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_SUBSYS_UNREG_FAILED,
                                   "subagt_unregister_dtsv_subsys()", 0, 0); 
    }
    
    /* unregister test-mib - to be deleted once SPSV Integration is ready */
    status = snmpsubagt_spa_job(SNMPSUBAGT_LIB_CONF/* SYSCONFDIR "subagt_lib_conf" */, 0 /* UNREGISTER */);     
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
    }
    
    /* Remove the nodes having unregister function pointers from the linked list in cb. */
    status = snmpsubagt_mibs_unload();

    return status; 
}

/*****************************************************************************\
 *  Name:          snmpsubagt_spa_job                                         * 
 *                                                                            *
 *  Description:   To load/unload  the MIB libraries mentioned in the         *  
 *                 configuration file                                         * 
 *                                                                            *
 *  Arguments:     uns8* - Name of the Configuration File                     *
 *                 uns32 - what_to_do                                         *
 *                          1 - REGISTER the MIB                              *
 *                          0 - UNREGISTER the MIB                            *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
\*****************************************************************************/
uns32 
snmpsubagt_spa_job(uns8 *file_name, uns32 what_to_do)
{
    FILE    *fp = NULL; 
    int8   lib_name[255] = {0};
    int8   appl_name[255] = {0};
    int8   func_name[255] = {0};
    int32   nargs = 0;
    uns32   status  = NCSCC_RC_FAILURE;
    uns32    (*reg_unreg_routine)() = NULL; 
    uns32    (*unreg_routine)() = NULL; 
    NCS_OS_DLIB_HDL     *lib_hdl = NULL;
    int8    *dl_error = NULL; 
    SNMPSUBAGT_MIBS_REG_LIST *new_reg = NULL;

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_SPA_JOB);

    /* open the file */
    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        /* inform that there is no such file, and return */
        m_NCS_SNMPSUBAGT_DBG_SINK(NCSCC_RC_FAILURE);
        return NCSCC_RC_SUCCESS; 
    }
    
    /* continue till the you reach the end of the file */
    while (((nargs=fscanf(fp,"%s %s",lib_name, appl_name))==2) && 
          (nargs != EOF))
    {
        /* Load the library if REGISTRATION is to be peformed */
        if (what_to_do == 1)
        {
            lib_hdl = m_NCS_OS_DLIB_LOAD(lib_name, m_NCS_OS_DLIB_ATTR); 
            if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
            {
                /* log the error returned from dlopen() */
                m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_DLIB_LOAD_NULL_FAILED, dl_error, 0, 0); 

                /* reset the local variables */ 
                reg_unreg_routine = NULL; 
                lib_hdl = NULL;
                m_NCS_MEMSET(appl_name, 0, 255);
                m_NCS_MEMSET(func_name, 0, 255);
                m_NCS_MEMSET(lib_name, 0, 255);

                continue; 
            }
            /* get the function pointer for init/deinit */ 
            sprintf(func_name, "subagt_register_%s_subsys",appl_name);
        }
        else
        {
            /* de-register the subsystem */
            sprintf(func_name, "subagt_unregister_%s_subsys",appl_name);
        }
        
        reg_unreg_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, func_name); 
        if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
        {
            /* log the error returned from dlopen() */
            m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_DLIB_SYM_FAILED, dl_error, 0, 0); 
            if (lib_hdl != NULL)
               m_NCS_OS_DLIB_CLOSE(lib_hdl); 

            /* reset the local variables */ 
            reg_unreg_routine = NULL; 
            lib_hdl = NULL;
            m_NCS_MEMSET(appl_name, 0, 255);
            m_NCS_MEMSET(func_name, 0, 255);
            m_NCS_MEMSET(lib_name, 0, 255);

            continue; 
        }

        /* do the INIT/DEINIT now... */
        if (reg_unreg_routine != NULL)
        {
            status = (*reg_unreg_routine)(); 
            if (status != NCSCC_RC_SUCCESS)
            {
                m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_REG_DEREG_FAILED, func_name, 0, 0); 
            }
            else
            {
                m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_REG_DEREG_SUCCESS, func_name, 0, 0); 

                if (what_to_do == 1)
                {
                    NCSSA_CB    *cb = m_SNMPSUBAGT_CB_GET; 
                    
                    m_NCS_MEMSET(func_name, 0, 255);
                    sprintf(func_name, "subagt_unregister_%s_subsys",appl_name);
                    unreg_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, func_name); 
                    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
                    {
                        /* log the error returned from dlopen() */
                        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_DLIB_SYM_FAILED, dl_error, 0, 0); 
                        m_NCS_OS_DLIB_CLOSE(lib_hdl); 

                        /* reset the local variables */ 
                        reg_unreg_routine = NULL; 
                        lib_hdl = NULL;
                        m_NCS_MEMSET(appl_name, 0, 255);
                        m_NCS_MEMSET(func_name, 0, 255);
                        m_NCS_MEMSET(lib_name, 0, 255);

                        continue; 
                    }
                    
                    /* allocate the memory for the pending registraion node */
                    new_reg = m_MMGR_SNMPSUBAGT_MIBS_REG_LIST_ALLOC;
                    if (new_reg == NULL)
                    {
                        /* MEMFAILURE log: the memory failure error */
                        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_PEND_REG_NODE_ALLOC_FAILED);
                        m_NCS_OS_DLIB_CLOSE(lib_hdl); 
        
                        /* reset the local variables */ 
                        reg_unreg_routine = NULL; 
                        lib_hdl = NULL;
                        m_NCS_MEMSET(appl_name, 0, 255);
                        m_NCS_MEMSET(func_name, 0, 255);
                        m_NCS_MEMSET(lib_name, 0, 255);
                        
                        continue; 
                    }
                    
                    /* initialize */
                    m_NCS_MEMSET(new_reg, 0, sizeof(SNMPSUBAGT_MIBS_REG_LIST));

                    /* copy the unregister routine */ 
                    new_reg->deinit_routine = unreg_routine;
                    new_reg->node.key = (uns8*)new_reg->deinit_routine;
                    
                    /* store the name library handle */ 
                    new_reg->lib_hdl = lib_hdl;
                    
                    /* add to the list */
                    status = ncs_db_link_list_add(&cb->new_registrations,
                                                  (NCS_DB_LINK_LIST_NODE*)new_reg);
                    if (status != NCSCC_RC_SUCCESS)
                    {
                        /* ERROR log: adding to the list failed */
                        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_PEND_REG_DLIST_ADD_FAILED,
                                                        status, 0, 0);
                        m_MMGR_SNMPSUBAGT_MIBS_REG_LIST_FREE(new_reg);
                        m_NCS_OS_DLIB_CLOSE(lib_hdl); 

                        /* reset the local variables */ 
                        reg_unreg_routine = NULL; 
                        lib_hdl = NULL;
                        m_NCS_MEMSET(appl_name, 0, 255);
                        m_NCS_MEMSET(func_name, 0, 255);
                        m_NCS_MEMSET(lib_name, 0, 255);
                        
                        continue; 
                    }

                    /* log message TBD MAHESH */ 

                } /* if (what_to_do == 1) */ 
            } 
        }

        /* reset the local variables */ 
        reg_unreg_routine = NULL; 
        lib_hdl = NULL;
        m_NCS_MEMSET(appl_name, 0, 255);
        m_NCS_MEMSET(func_name, 0, 255);
        m_NCS_MEMSET(lib_name, 0, 255);
    } /* for all the libraries */

    /* close the file */
    fclose(fp);
     
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          snmpsubagt_mibs_reload                                     * 
*                                                                            *
*  Description:   Reload the subagent library configuration file.            * 
*                                                                            *
*  Arguments:     NCSSA_CB  *cb- Control Bloack of SNMP SubAgent             * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 snmpsubagt_mibs_reload(NCSSA_CB *cb)
{
    NCS_DB_LINK_LIST_NODE   *current = NULL; 
    uns32                   status = NCSCC_RC_FAILURE;     
    SNMPSUBAGT_MBX_MSG      *post_me = NULL;
    
    current = m_NCS_DBLIST_FIND_FIRST(&cb->new_registrations);
    while (current)   
    {
        /* unregister this MIB */ 
        status = ((SNMPSUBAGT_MIBS_REG_LIST*)current)->deinit_routine(); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* ERROR LOG: free failed */
            /* continue to the next node */
        }    
        
        /* go to the next registration */
        current = m_NCS_DBLIST_FIND_NEXT(current); 
    }

    /* As there is a chance that subagent is in the processing of destroying
     * itself. 
     */ 
    m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE); 
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_TAKEN);
    
    /* update the message info */
    post_me = m_MMGR_SNMPSUBAGT_MBX_MSG_ALLOC;
    if (post_me == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_MBX_MSG_MALLOC_FAIL);
        
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_RELEASED);
        return NCSCC_RC_OUT_OF_MEM; 
    }

    m_NCS_MEMSET(post_me, 0, sizeof(SNMPSUBAGT_MBX_MSG)); 
    post_me->msg_type = SNMPSUBAGT_MBX_CONF_RELOAD;

    /* post a message to the SubAgent's thread */ 
    status = m_NCS_IPC_SEND(&cb->subagt_mbx, 
                            (NCS_IPC_MSG*)post_me, 
                            NCS_IPC_PRIORITY_HIGH); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MBX_IPC_SEND_FAILED, status,0,0);

        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_RELEASED);
        return status; 
    }
    
    /* release the lock */ 
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_RELEASED);

    return status; 
}

/****************************************************************************\
*  Name:          snmpsubagt_mibs_unload                                     * 
*                                                                            *
*  Description:   Unloads the subagent integration libraries                 * 
*                                                                            *
*  Arguments:     Nothing                                                    *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    * 
\****************************************************************************/
uns32 snmpsubagt_mibs_unload()
{
    NCS_DB_LINK_LIST_NODE   *current = NULL; 
    NCS_DB_LINK_LIST_NODE   *next = NULL; 
    SNMPSUBAGT_MIBS_REG_LIST *reg_info;
    NCSSA_CB    *cb = m_SNMPSUBAGT_CB_GET; 

    current = m_NCS_DBLIST_FIND_FIRST(&cb->new_registrations);
    while (current)   
    {
        next = m_NCS_DBLIST_FIND_NEXT(current); 

        reg_info = (SNMPSUBAGT_MIBS_REG_LIST*)current;

        /* unload this library */ 
        m_NCS_OS_DLIB_CLOSE(reg_info->lib_hdl); 

        /* delete the current node from the list */ 
        ncs_db_link_list_del(&cb->new_registrations, current->key); 

        /* next library */ 
        current = next; 
    }
    
    return NCSCC_RC_SUCCESS; 
}

/*****************************************************************************\
 *  Name:          snmpsubagt_pending_regs_cmp                                * 
 *                                                                            *
 *  Description:   To compare the KEYs                                        *
 *                                                                            *
 *  Arguments:     uns8* - Name of the function to be posted                  *
 *                                                                            * 
 *  Returns:       0    -  if the keys are same                               *
 *                 0xff - otherwise                                           *
\*****************************************************************************/
uns32 snmpsubagt_pending_regs_cmp(uns8 *key1, uns8 *key2)
{
   /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_PENDING_LST_CMP);
   if (key1 == key2)
   {
      return 0;
   }
   else
   {
      return 0xff;
   }
}

/*****************************************************************************\
 *  Name:          snmpsubagt_pending_regs_free                               * 
 *                                                                            *
 *  Description:   To send the name of the function to be sent to the         * 
 *                 subagent's thread to load the init or deinit routine       * 
 *                 in the subagent thread.                                    *
 *                                                                            *
 *  Arguments:     NCS_DB_LINK_LIST_NODE * - Node to be freed                 *
 *                                                                            * 
 *  Returns:       Nothing                                                    *
\*****************************************************************************/
uns32  snmpsubagt_pending_regs_free(NCS_DB_LINK_LIST_NODE *free_node)
{
    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_PENDING_REGS_FREE);

    /** Call the macro to free the node **/
    m_MMGR_SNMPSUBAGT_MIBS_REG_LIST_FREE(free_node);

    return NCSCC_RC_SUCCESS; 
}

/*****************************************************************************\
 *  Name:          snmpsubagt_pending_registrations_free                      * 
 *                                                                            *
 *  Description:   To free the list of pending registrationsn                 * 
 *                                                                            *
 *  Arguments:     NCS_DB_LINK_LIST * - List of pending registrations         *
 *                                                                            * 
 *  Returns:       Nothing                                                    *
\*****************************************************************************/
void  
snmpsubagt_pending_registrations_free(NCS_DB_LINK_LIST *registrations)
{
    NCS_DB_LINK_LIST_NODE   *temp = NULL; 
    NCS_DB_LINK_LIST_NODE   *prev = NULL; 
    uns32                   status = NCSCC_RC_FAILURE;     
    
    /* log the function entry */ 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_PEND_LIST_FREE);

    prev = temp = m_NCS_DBLIST_FIND_FIRST(registrations);
    
    while (temp)   
    {
        temp = m_NCS_DBLIST_FIND_NEXT(temp); 

        /* delete the node */ 
        status = ncs_db_link_list_del(registrations, prev->key);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* ERROR LOG: free failed */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_PEND_DEL_FAILED, 
                                   status, 0, 0);
            /* continue to the next node */
        }    

        /* go to the next registration */
        prev = temp;
    }
    return; 
}
/********************* IR00061409 ***********************************/
/*****************************************************************************
* Name:          subagt_rda_init_role_get                                    *
*                                                                            * 
* Description:   Gets the initial role from RDA                              *
*                                                                            *
* Arguments:     *cb - Store the initial role                                *
* Returns:       NCSCC_RC_FAILURE - RDA interface failure, a JUNK state is   *
*                                    returned.                               *
*                 NCSCC_RC_SUCCESS - initial role is available in            *
*                                    cb->haCstate                            *
*****************************************************************************/
EXTERN_C uns32
subagt_rda_init_role_get(struct ncsSa_cb *cb)
{
    uns32       rc;
    uns32       status = NCSCC_RC_SUCCESS;
    PCS_RDA_REQ app_rda_req;
    
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_RDA_INIT_ROLE_GET);
 
    /* initialize the RDA Library */
    m_NCS_MEMSET(&app_rda_req, 0, sizeof(PCS_RDA_REQ));
    app_rda_req.req_type = PCS_RDA_LIB_INIT;
    rc = pcs_rda_request (&app_rda_req);
    if (rc  != PCSRDA_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(PCS_RDA_REQUEST_FAIL, rc, 0, 0);
        return NCSCC_RC_FAILURE;
    }
    /* get the role */
    m_NCS_MEMSET(&app_rda_req, 0, sizeof(PCS_RDA_REQ));
    app_rda_req.req_type = PCS_RDA_GET_ROLE;
    rc = pcs_rda_request(&app_rda_req);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(PCS_RDA_REQUEST_FAIL, rc, 0, 0);

        /* set the error code to be returned */
        status = NCSCC_RC_FAILURE;
        /* finalize */
        goto finalize;
    }
    /* update the output parameter */
    if (app_rda_req.info.io_role == PCS_RDA_ACTIVE)
        cb->haCstate = SNMPSUBAGT_HA_STATE_ACTIVE;
    else if (app_rda_req.info.io_role == PCS_RDA_STANDBY)
        cb->haCstate = SNMPSUBAGT_HA_STATE_STANDBY;
    else
    {
        /* log the state returned by RDA */
        m_SNMPSUBAGT_STATE_LOG(SNMPSUBAGT_HA_STATE_INVALID, SNMPSUBAGT_HA_STATE_NULL, cb->haCstate);
        /* set the error code to be returned */
        status = NCSCC_RC_FAILURE;
        goto finalize;
    }
    /* finalize the library */
finalize:
       m_NCS_MEMSET(&app_rda_req, 0, sizeof(PCS_RDA_REQ));
       app_rda_req.req_type = PCS_RDA_LIB_DESTROY;
       rc = pcs_rda_request(&app_rda_req);
       if (rc != PCSRDA_RC_SUCCESS)
       {
            /* log the error */
            return NCSCC_RC_FAILURE;
       }
       /* return the final status */
       return status;
}

/* IR00061409 */
/*****************************************************************************
*  Name:          subagt_amf_componentize                                    *
*                                                                            *
*  Description:   Registers the following with AMF framework                 *
*                 - AMF callbacks                                            *
*                 - Start the health check                                   *
*  Arguments:     NCSSA_CB  * control block of subagt                        *
*                                                                            *
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
*                  NCSCC_RC_FAILURE   -  failure                             * 
******************************************************************************/
static uns32
subagt_amf_componentize(NCSSA_CB *cb)
{
    uns32       status = NCSCC_RC_SUCCESS;
    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_AMF_COMPONENTIZE);
    if (cb == NULL)
    {
        /* log that an invalid input param is received */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE;
    }
    /* Initialize the interface with Availability Management Framework */
    status = snmpsubagt_amf_initialize(cb);
    if (status != SA_AIS_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_INIT_FAILED, status,0,0);
        if(status == SA_AIS_ERR_TRY_AGAIN)
        {
            /* saAmf api returned error to try again. set the status and start the timer. */
            cb->amfInitStatus = m_SNMPSUBAGT_AMF_INIT_RETRY;
            m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_INIT_RETRY);
            if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_AMF_INIT) == NCSCC_RC_FAILURE)
            {
                /* timer couldn't be started. return failure. */
                m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);
                return NCSCC_RC_FAILURE;
            }
        }
        else 
            return NCSCC_RC_FAILURE;
    }
    else 
    {
        /* log that AMF Interface init is success */
        m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_INIT_SUCCESS);
    }

    if((cb->amfInitStatus == m_SNMPSUBAGT_AMF_INIT_COMPLETED) || (cb->amfInitStatus == m_SNMPSUBAGT_AMF_COMP_REG_RETRY))
    {
        /* start the health check if it is not started already */
        if (cb->healthCheckStarted == FALSE)
        {
            /* post a message to subagt mailbox to start the health check */
            status = snmpsubagt_amf_healthcheck_start_msg_post(cb);
            if (status != NCSCC_RC_SUCCESS)
            {
                m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_HLTHCHECK_POST_FAILED, status,0,0);
                return status;
            }
        }
        /* Register the component with the Availability Agent */
        status = saAmfComponentRegister(cb->amfHandle, &cb->compName, NULL);
        if (status != SA_AIS_OK)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_COMP_REG_FAILED,
                        cb->amfHandle, status,0/*  cb->compName TBD */);
            if(status == SA_AIS_ERR_TRY_AGAIN)
            {
                /* saAmf api returned error to try again. set the status and start the timer. */
                cb->amfInitStatus = m_SNMPSUBAGT_AMF_COMP_REG_RETRY;
                if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_AMF_INIT) == NCSCC_RC_FAILURE)
                {
                    /* timer couldn't be started. return failure. */
                    m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);
                    saAmfComponentErrorReport(cb->amfHandle,
                                              &cb->compName,
                                              0, /* errorDetectionTime; AvNd will provide this value */
                                              SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
                    return NCSCC_RC_FAILURE;
                }
            }
            else
                return status;
        }
        else
        {
            /* Component registration with AMF is completed. set the status. */
            cb->amfInitStatus = m_SNMPSUBAGT_AMF_COMP_REG_COMPLETED;
            /* log the status */
            m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_COMP_REG_COMPLETED);
            return  status;    
        }
    }
    return status;
}

/* IR00061409 */
EXTERN_C void 
subagt_process_sig_usr1_signal(NCSSA_CB *cb)
{
    char    comp_name[256] = {0};
    FILE    *fp = NULL;
    uns32 status = NCSCC_RC_SUCCESS;
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_SIGUSR1_SIGNAL);

    fp = fopen(m_SUBAGENT_COMP_NAME_FILE, "r");/* LOCALSTATEDIR/ncs_subagent_comp_name */
    if (fp == NULL)
    {
        /* log that, there is no component name file */
        return;
    }
    if(fscanf(fp, "%s", comp_name) != 1)
    {
        fclose(fp);
        return;
    }
    if (setenv("SA_AMF_COMPONENT_NAME", comp_name, 1) == -1)
    {
        fclose(fp);
        return;
    }
    fclose(fp);
    status = subagt_amf_componentize(cb);
    return;
}

#endif /* #if (BBS_SNMPSUBAGT == 0) */


