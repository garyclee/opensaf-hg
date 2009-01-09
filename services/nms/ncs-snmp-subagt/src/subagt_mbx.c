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
  DESCRIPTION: This file describes the routines for SubAgent's mail box
               handling.
******************************************************************************/ 
#include "subagt.h"

/* to handle the destroy message */
static uns32
snmpsubagt_mbx_destroy_process(NCSSA_CB *cb); 

/* to handle the register/deregister message */
static uns32  
snmpsubagt_mbx_init_deinit_process(NCSSA_CB *cb, uns8*  init_deinit_routine);

/* to start the health check */
static uns32 
snmpsubagt_mbx_healthcheck_start(NCSSA_CB *cb); 

static void snmpsubagt_process_retry_eda_init_msg(NCSSA_CB *cb);

static void snmpsubagt_process_retry_amf_init_msg(NCSSA_CB *cb);

static void snmpsubagt_check_snmpd_and_subscribe_edsv(NCSSA_CB *cb);

static void snmpsubagt_process_retry_check_snmpd_msg(NCSSA_CB *cb);

static void snmpsubagt_unsubscribe_edsv(NCSSA_CB *cb);

extern netsnmp_session *main_session;

/*****************************************************************************\
 *  Name:          snmpsubagt_mbx_process                                     * 
 *                                                                            *
 *  Description:   To process the events on the subagent's mail box           *
 *                                                                            *
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block               *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32   
snmpsubagt_mbx_process(NCSSA_CB *cb)
{
    uns32               status = NCSCC_RC_FAILURE; 
    SNMPSUBAGT_MBX_MSG  *process_me= NULL; 

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_MBX_PROCESS);

    /* validate the inputs */
    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE;
    }

    /* read the message from mailbox */
    if ((process_me = (SNMPSUBAGT_MBX_MSG*) m_NCS_IPC_NON_BLK_RECEIVE(&cb->subagt_mbx, NULL)) != NULL)
    {
        /* process the message */
        switch(process_me->msg_type)
        {
            /* destroy message */
            case SNMPSUBAGT_MBX_MSG_DESTROY:
            status = snmpsubagt_mbx_destroy_process(cb); 
            if (status != NCSCC_RC_SUCCESS)
            {
                /* log that there is an error while destroying the subagent */
                m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MBX_DESTROY_FAILED, status, 0, 0);
            }
            break; 

            /* register or deregister message is been posted */
            case SNMPSUBAGT_MBX_MSG_MIB_INIT_OR_DEINIT:
            status = snmpsubagt_mbx_init_deinit_process(cb,
                                    process_me->init_or_deinit_routine);
            if (status != NCSCC_RC_SUCCESS)
            {
                /* log the failure */
                m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MBX_REG_DEREG_FAILED, status, 0, 0);
            }
            break; 

            /* start the health check */ 
            case SNMPSUBAGT_MBX_MSG_HEALTH_CHECK_START:
            status = snmpsubagt_mbx_healthcheck_start(cb); 
            if (status != NCSCC_RC_SUCCESS)
            {
                /* log the failure */
                m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MBX_HLTH_CHECK_START_FAILED, status, 0, 0);
            }
            break; 

#if (SUBAGT_AGT_MONITOR == 1)
            /* monitor the SNMP Agent */
            case SNMPSUBAGT_MBX_MSG_AGT_MONITOR:
            snmpsubagt_agt_monitor_same_task(cb); 
            break; 
#endif

            /* Retry the EDA Initialization */
            case SNMPSUBAGT_MBX_MSG_RETRY_EDA_INIT:
            snmpsubagt_process_retry_eda_init_msg(cb);
            break; 

            /* Retry the AMF Initialization */
            case SNMPSUBAGT_MBX_MSG_RETRY_AMF_INIT:
            snmpsubagt_process_retry_amf_init_msg(cb);
            break; 

            /* Retry the check for snmpd presence to subscribe with EDSv */
            case SNMPSUBAGT_MBX_MSG_RETRY_CHECK_SNMPD:
            snmpsubagt_process_retry_check_snmpd_msg(cb);
            break; 

            case SNMPSUBAGT_MBX_CONF_RELOAD:
            /* Unsubscribe with EDSv */
            snmpsubagt_unsubscribe_edsv(cb);

            /* unload the current libraries */
            snmpsubagt_mibs_unload(); 
            
            /* re-read the configuration file */ 
            snmpsubagt_spa_job(SNMPSUBAGT_LIB_CONF, 1); 
            
            break;
            
            default:
            /* log that message type is not good */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MBX_INVALID_MSG, status, 0, 0);
            status = NCSCC_RC_FAILURE; 
            break; 
        }
        /* free the message */
        m_MMGR_SNMPSUBAGT_MBX_MSG_FREE(process_me); 
        process_me = NULL; 
    }/* process the next message in the mail box */
    return status; 
}
        
/*****************************************************************************\
 *  Name:          snmpsubagt_mbx_destroy_process                             * 
 *                                                                            *
 *  Description:   To initialize the session with the Agentx Agent            *
 *                  To do the above job NetSnmp Supplied APIs are used        *
 *                                                                            *
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block               *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static uns32   
snmpsubagt_mbx_destroy_process(NCSSA_CB *cb) 
{
    uns32       status = NCSCC_RC_FAILURE; 
    NCSCONTEXT task_id = NULL;
    
    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_MBX_DESTROY_PROCESS);

    /* validate the inputs */
    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE; 
    }

    /* destroy the subagent */ 
    /* As of now, SPA thread will post to the subagt mail-box 
     * to destroy the subagent 
     */
    
    task_id = cb->subagt_task;

    status = snmpsubagt_destroy(cb, TRUE); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log on to the console */
        m_NCS_SNMPSUBAGT_DBG_SINK(status); 
    }

    m_NCS_TASK_STOP(task_id);
    m_NCS_TASK_RELEASE(task_id);

    return status;
}

/*****************************************************************************\
 *  Name:          snmpsubagt_mbx_init_deinit_process                         * 
 *                                                                            *
 *  Description:   To load the init/deinit routine of a library               *
 *                                                                            *
 *  Arguments:     uns8*    - init/deinit routine to be called                *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static uns32   
snmpsubagt_mbx_init_deinit_process(NCSSA_CB *cb, uns8*  reg_dereg_routine)
{
    uns32   status = NCSCC_RC_FAILURE; 
    uns32    (*init_deinit_routine)() = NULL; 
    NCS_OS_DLIB_HDL     *lib_hdl = NULL;
    int8    *dl_error = NULL; 
    /* SNMPSUBAGT_PENDING_REGS *pending_reg = NULL;*/ 
    
    /* log the function entry */ 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_MBX_INIT_DEINIT_PROCESS); 

    if (reg_dereg_routine == NULL)
    {
        /* log the error */ 
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_MBX_REG_DEREG_NULL);
        return status; 
    }

    /* reset the error */
    dl_error =  m_NCS_OS_DLIB_ERROR();

    /* re-load the libraries */ 
    lib_hdl = m_NCS_OS_DLIB_LOAD(NULL, m_NCS_OS_DLIB_ATTR); 
    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
    {
        /* log the error returned from dlopen() */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_DLIB_LOAD_NULL_FAILED, dl_error, 0, 0); 
        return status;
    }
    if (lib_hdl == NULL)
    {
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_DLIB_LOAD_NULL_FAILED, status, 0, 0);
        return status; 
    }

    /* get the function pointer for init/deinit */ 
    init_deinit_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, 
                                               reg_dereg_routine); 
    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
    {
        /* log the error returned from dlopen() */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_DLIB_SYM_FAILED, dl_error, 0, 0); 
        return status;
    }
    if (init_deinit_routine == NULL)
    {
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_DLIB_SYM_FAILED, reg_dereg_routine, 0, 0); 

        return status; 
    }

#if 0
    /* if 0 because, init_snmp() is called at the time of initilizing the session with Agent. 
     * Session with Agent is done as part of the init unlike after getting the CSI State
     * assignment. 
     */
    /* If the session with the master agent is established, perform the registration */
    if ((cb->haCstate == SNMPSUBAGT_HA_STATE_ACTIVE) ||  /* register/init the MIBs */
        (cb->haCstate == SNMPSUBAGT_HA_STATE_QUISCED))   /* Unregister/deinit the MIBs */
    {
#endif
        /* do the INIT/DEINIT now... */
        status = (*init_deinit_routine)();
        if (status != NCSCC_RC_SUCCESS)
            m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_REG_DEREG_FAILED, reg_dereg_routine, 0, 0);
        else
            m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_REG_DEREG_SUCCESS, reg_dereg_routine, 0, 0);
#if 0
    }
    else /* If the session with the master agent is not yet established, add to the pending list */
    {
        /* allocate the memory for the pending registraion node */
        pending_reg = m_MMGR_SNMPSUBAGT_PENDING_REGS_ALLOC;
        if (pending_reg == NULL)
        {
            /* MEMFAILURE log: the memory failure error */
            m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_PEND_REG_NODE_ALLOC_FAILED);
            return NCSCC_RC_FAILURE;
        }
        /* initialize */
        m_NCS_MEMSET(pending_reg, 0, sizeof(SNMPSUBAGT_PENDING_REGS));
        pending_reg->init_deinit_routine = init_deinit_routine;
        pending_reg->node.key = (uns8*)pending_reg->init_deinit_routine;
        
        /* add to the list */
        status = ncs_db_link_list_add(&cb->pending_registrations,
                                      (NCS_DB_LINK_LIST_NODE*)pending_reg);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* ERROR log: adding to the list failed */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_PEND_REG_DLIST_ADD_FAILED,
                                            status, 0, 0);
            m_MMGR_SNMPSUBAGT_PENDING_REGS_FREE(pending_reg);
            return status;
        }
        
        /*  INFORM LOG: an event that a registration is pending */
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_PENDING_REG, reg_dereg_routine, 0, 0); 
     }
#endif

     if (strcmp(cb->lastRegMsg, reg_dereg_routine) == 0)
     {
        m_SNMPSUBAGT_ERROR_STR_LOG(SNMPSUBAGT_REG_COMPLETED, reg_dereg_routine, 0, 0); 
        /* Set this to indicate the completion of registrations. */
        m_NCS_MEMSET(cb->lastRegMsg, 0, 255);
        strcpy(cb->lastRegMsg, "REG_COMPLETED");

        /* All the registrations are completed. Now check for edsv interface init status and snmpd.*/
        if (cb->edaInitStatus == m_SNMPSUBAGT_EDA_INIT_COMPLETED)
        {
           snmpsubagt_check_snmpd_and_subscribe_edsv(cb);
        }

     }

     return status; 
}

/*****************************************************************************\
 *  Name:          snmpsubagt_mbx_healthcheck_start                           * 
 *                                                                            *
 *  Description:   To start the health check                                  *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\******************************************************************************/
static uns32 
snmpsubagt_mbx_healthcheck_start(NCSSA_CB *cb) 
{
#if (BBS_SNMPSUBAGT == 0)
    SaAisErrorT     saf_status = SA_AIS_OK; 
    
    /* log the function entry */ 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_MBX_HEALTHCHECK_START); 

    if (cb->healthCheckStarted == TRUE)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_HLTH_CHECK_STARTED_ALREADY);
        return NCSCC_RC_SUCCESS; 
    }

    /* start the healthcheck */ 
    saf_status = saAmfHealthcheckStart(cb->amfHandle,
                                      &cb->compName,
                                      &cb->healthCheckKey,
                                      cb->invocationType,
                                      cb->recommendedRecovery);
    if (saf_status != SA_AIS_OK) 
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_HLTH_CHK_STRT_FAILED,
                               cb->amfHandle,saf_status, 0);
        return NCSCC_RC_FAILURE;
    }
   
    cb->healthCheckStarted = TRUE; 

    /* log that health check started */
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_HLTH_CHECK_STARTED);
#endif /* #if (BBS_SNMPSUBAGT == 0) */

    return NCSCC_RC_SUCCESS; 
}    

/*****************************************************************************\
 *  Name:          snmpsubagt_process_retry_eda_init_msg                      * 
 *                                                                            *
 *  Description:   To process the msg received in the subagt's mailbox        *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       Nothing                                                    *
 *  NOTE:                                                                     * 
\******************************************************************************/
static void snmpsubagt_process_retry_eda_init_msg(NCSSA_CB *cb)
{
   uns32 status; 
   if (cb->edaInitStatus != m_SNMPSUBAGT_EDA_INIT_COMPLETED)
   {
      status = snmpsubagt_eda_initialize(cb); 
      if (status != SA_AIS_OK)
      {  
         /* log the error */
         m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_INIT_FAILED,status,0,0);
         if(status == SA_AIS_ERR_TRY_AGAIN)
         {
            /* start the timer as error returned is TRY_AGAIN. */
            if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT)==NCSCC_RC_FAILURE)
            {
               /* timer couldn't be started. log the failure. */
               m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);
   
               /* Notify AMF abt the timer start failure, if subagt has registered with it. */
               if(cb->amfHandle)
                  saAmfComponentErrorReport(cb->amfHandle,
                                          &cb->compName,
                                          0, /* errorDetectionTime; AvNd will provide this value */
                                          SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
            }
         }
         else
         {
            /* error returned is something other than TRY_AGAIN. Notify AMF abt error if ACTIVE state is assigned to subagt. */
            if(cb->haCstate == SNMPSUBAGT_HA_STATE_ACTIVE)
            {
               /* escalate the error, with FAILOVER as recovery */
               saAmfComponentErrorReport(cb->amfHandle,
                                         &cb->compName,
                                         0, /* errorDetectionTime; AvNd will provide this value */
                                         SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
            }
         }
         return;
      }
      else
        /* log that EDA Interface init is success */
        m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_INIT_SUCCESS);
   }

   if (cb->haCstate == SNMPSUBAGT_HA_STATE_ACTIVE)
   {
      /* If all the registrations are completed, then try to subscribe to EDSV */
      if (strcmp(cb->lastRegMsg, "REG_COMPLETED") == 0)
      {
         /* Check if snmpd is up now. If it is, then subscribe to EDSv interface. */
         snmpsubagt_check_snmpd_and_subscribe_edsv(cb);
      }

   }
   return;
}

/*****************************************************************************\
 *  Name:          snmpsubagt_process_retry_amf_init_msg                      * 
 *                                                                            *
 *  Description:   To process the msg received in the subagt's mailbox        *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       Nothing                                                    *
 *  NOTE:                                                                     * 
\******************************************************************************/
static void snmpsubagt_process_retry_amf_init_msg(NCSSA_CB *cb)
{
   uns32 status; 
  
   if(cb->amfInitStatus == m_SNMPSUBAGT_AMF_INIT_RETRY)
   {
      /* AMF Interface initialisation is not yet completed. Call the API to do the initialisation. */
      status = snmpsubagt_amf_initialize(cb); 
      if (status != SA_AIS_OK)
      {  
         /* log the error */
         m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_INIT_FAILED,status,0,0);
         if(status == SA_AIS_ERR_TRY_AGAIN)
         {
            /* start the timer as error returned is TRY_AGAIN. */
            if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_AMF_INIT) == NCSCC_RC_FAILURE)
            {
               /* timer couldn't be started. log the failure. */
               m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);
               return;
            }
         }
      }
      else
      {
         /* AMF Interface initialisation is completed. set the status. */
         cb->amfInitStatus = m_SNMPSUBAGT_AMF_INIT_COMPLETED;
         /* log the status. */
         m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_INIT_SUCCESS); 
      }
   }

   /* We are here for one of the following reasons.
      1. AMF interface initialisation is completed.
      2. Subagt component registration with AMF is being retried.
      
      In both the cases, start the health check, if it was not started, register component with AMF.
    */
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
               return;
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

                 return ;
              }
           }
           else
              return ;
       }
       else
       {
          /* Component registration with AMF is completed. set the status. */
          cb->amfInitStatus = m_SNMPSUBAGT_AMF_COMP_REG_COMPLETED;
          /* log the status */
          m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_COMP_REG_COMPLETED); 
       }
   }
   return;
}

/*****************************************************************************\
 *  Name:          snmpsubagt_process_retry_check_snmpd_msg                   * 
 *                                                                            *
 *  Description:   To process the msg received in the subagt's mailbox        *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       Nothing                                                    *
 *  NOTE:                                                                     * 
\******************************************************************************/
static void snmpsubagt_process_retry_check_snmpd_msg(NCSSA_CB *cb)
{
   /* Check if snmpd is up now. If it is, then subscribe to EDSv interface. */
   snmpsubagt_check_snmpd_and_subscribe_edsv(cb);
}

/*****************************************************************************\
 *  Name:          snmpsubagt_check_snmpd_and_subscribe_edsv                  * 
 *                                                                            *
 *  Description:   Checks for presence of snmpd and subscribes to edsv        *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       Nothing                                                    *
 *  NOTE:                                                                     * 
\******************************************************************************/
static void snmpsubagt_check_snmpd_and_subscribe_edsv(NCSSA_CB *cb)
{
    SaAisErrorT saf_status = SA_AIS_OK;

    /* SNMPD is running now. Subscribe with EDSv */
    saf_status = saEvtEventSubscribe(cb->evtChannelHandle, /* input */
                                     &cb->evtFilters,/* input */
                                     cb->evtSubscriptionId);/* input */
    if(saf_status != SA_AIS_OK)
    {
          /* log the error */
          m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_EVT_SUBSCRIBE_FAIL,
                                      saf_status, cb->evtSubscriptionId, 0);
          if(saf_status == SA_AIS_ERR_TRY_AGAIN)
          {
             /* start the timer as error returned is TRY_AGAIN. */
             if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT)==NCSCC_RC_FAILURE)
             {
                /* timer couldn't be started. log the failure. */
                m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);
         
                /* Notify AMF abt the timer start failure, if subagt has registered with it. */
                if(cb->amfHandle)
                    saAmfComponentErrorReport(cb->amfHandle,
                                              &cb->compName,
                                              0, /* errorDetectionTime; AvNd will provide this value */
                                              SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
             }
         }
         else if(saf_status != SA_AIS_ERR_EXIST)
         {
             /* error returned is something other than TRY_AGAIN. Notify AMF abt error if ACTIVE state is assigned to subagt. */
             if(cb->amfHandle)
             {
                    /* escalate the error, with FAILOVER as recovery */
                    saAmfComponentErrorReport(cb->amfHandle,
                                              &cb->compName,
                                              0, /* errorDetectionTime; AvNd will provide this value */
                                              SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
             }
         }
         return;
    }
    /* log the subscription status */
    m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_SUBSCRIBE_SUCCESS);

    /* Delete the timers and set the timer-ids to NULL as subscribe is successful. */
    if (cb->eda_timer.timer_id != TMR_T_NULL)
    {
         m_NCS_TMR_DESTROY(cb->eda_timer.timer_id);
         cb->eda_timer.timer_id = TMR_T_NULL;
    }

    return;
}

/*****************************************************************************\
 *  Name:          snmpsubagt_unsubscribe_edsv                                * 
 *                                                                            *
 *  Description:   Unsubscribes to edsv if already subscribed                 *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       Nothing                                                    *
 *  NOTE:                                                                     * 
\******************************************************************************/
static void snmpsubagt_unsubscribe_edsv(NCSSA_CB *cb)
{
   SaAisErrorT status = SA_AIS_OK;


   /* Need to unsubscribe only if subscribed.. This can be known if eda_timer is running. */
   if (cb->edaInitStatus != m_SNMPSUBAGT_EDA_INIT_COMPLETED)
   {
      /* Subscribe didn't happen. No need to unsubscribe. */
      return;
   }
   /* Here, subscribe may be in try again mode or snmpd timer may be running or subscribe is completed */

   if (cb->eda_timer.timer_id != NULL)
   {
      m_NCS_TMR_STOP(cb->eda_timer.timer_id);
   }
   else /* Subscription happened earlier. So do unsubscribe now. */
   {
       /* If the  traps are buffered and snmpd is up, send the buffered traps */
       snmpsubagt_check_and_flush_buffered_traps(cb);

       status = saEvtEventUnsubscribe(cb->evtChannelHandle, /* input */
                                      cb->evtSubscriptionId);/* input */
       if (status != SA_AIS_OK)
       {
           /* log the error */
           m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_UNSUBSCRIBE_FAIL,
                               status,
                               cb->evtSubscriptionId,
                               cb->evtChannelHandle);
       }
       /* log the unsubscription status */
       m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_UNSUBSCRIBE_SUCCESS);
   }
   return;
}

