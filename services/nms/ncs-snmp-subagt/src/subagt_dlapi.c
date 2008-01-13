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
 
  DESCRIPTION: This file describes the DL SE APIs
                ncs_snmpsubagt_lib_req  - SE API to be exported
                ncs_snmpsubagt_create   - to create the SubAgent
                snmpsubagt_start        - SubAgent's thread start routine
                ncs_snmpsubagt_destroy  - to destroy the SubAgent
                snmpsubagt_destroy      - to destroy the SubAgent
                ncs_snmpsubagt_about    - to know more about SubAgent
                ncs_snmpsubagt_diagnose - to know Health of the SubAgent
                snmpsubagt_cleanup      - to cleanup things for 
                                          initialization failures
***************************************************************************/ 
#include "subagt.h"

/* static functions declarations */
/* initialization routine */
static uns32
snmpsubagt_svcs_init(NCSSA_CB *cb); 

/* start_routine for the SubAgent thread */
static void
snmpsubagt_start(SYSF_MBX   *subagt_mbx);

/* For cleanup during the initialization */
static  void
snmpsubagt_cleanup(NCSSA_CB *cb);

/* callback to free the message in subagt's mailbox. */
static NCS_BOOL snmpsubagt_leave_on_queue_cb (void *arg1, void *arg2);

/* SIGHUP Handler */ 
static void
snmpsubagt_sighup_handler(int i_sig_num); 

/* IR00061409 */
/* SIGUSR1 Handler */ 
static void
snmpsubagt_amf_sigusr1_handler(int i_sig_num); 

/* SIGUSR2 Handler */ 
static void
snmpsubagt_sigusr2_handler(int i_sig_num); 

/******************************************************************************
*  Name:          ncs_snmpsubagt_lib_req
*  Description:   To manage the NCS SNMP SubAgent Library
*
*  Arguments:     NCS_LIB_REQ_INFO - Library request info from the Caller 
*
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK
*                 NCSCC_RC_FAILURE   -  failure
*  Note:
******************************************************************************/
uns32
ncs_snmpsubagt_lib_req(NCS_LIB_REQ_INFO *io_lib_req)
{
    uns32   status = NCSCC_RC_FAILURE; 

    /* validate the inputs */
    if (io_lib_req == NULL)
    {
       m_NCS_SNMPSUBAGT_DBG_SINK(status);
       return status; 
    }

    /* service the request */
    switch(io_lib_req->i_op)
    {
        
#if 0
        /* SPA does not support this as of now */
        case NCS_LIB_REQ_INIT:
            /* Not supported */
            break; 
            
        case NCS_LIB_REQ_ABOUT:
            status = ncs_snmpsubagt_about(&io_lib_req.about);
            break;
            
        case NCS_LIB_REQ_MANAGE:
            /* not supported */
            break;

        case NCS_LIB_REQ_DIAGNOSE:
            status = ncs_snmpsubagt_diagnose();
            if (status != NCSCC_RC_SUCCESS)
            {   
                /* log the error */
            }
            break;
#endif 
            
        case NCS_LIB_REQ_CREATE:
            /* extract the startup parameters from the REQ struct */
            status = ncs_snmpsubagt_create(io_lib_req->info.create.argc, 
                                    (uns8**)io_lib_req->info.create.argv); 
            if (status != NCSCC_RC_SUCCESS)
            {
                /* write on the console */
                m_NCS_SNMPSUBAGT_DBG_SINK(status);
            }
            break;
        case NCS_LIB_REQ_DESTROY:
            /* Suicide */
            status = ncs_snmpsubagt_destroy();
            if (status != NCSCC_RC_SUCCESS)
            {
                /* write on the console */
                 m_NCS_SNMPSUBAGT_DBG_SINK(status);
            }
            break;
        default: 
            status = NCSCC_RC_FAILURE; 
            m_NCS_SNMPSUBAGT_DBG_SINK(status);
            break; 
    }
    return status;
}

/******************************************************************************
*  Name:          ncs_snmpsubagt_create
*  Description:   To create the NCS SNMP SubAgent 
*                  - Starts a new thread for the SubAgent
*
*  Arguments:     param - startup parameters string as given in the BOM
*
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK
*                 NCSCC_RC_FAILURE   -  failure
*  Note:
******************************************************************************/
/* CREATE DL-SE API prototype */
uns32
ncs_snmpsubagt_create(int32 argc, uns8 **argv)
{
    FILE            *fp;
    uns32           status = NCSCC_RC_FAILURE;
    NCSSA_CB        *cb=NULL; 

    /* LOGGING is not yet enabled */

    /* Initialize the NCS SubAgent Control Block */
    cb = snmpsubagt_cb_init(); 
    if (cb == NULL)
    {
        /* write on the console */
        m_NCS_SNMPSUBAGT_DBG_SINK(status);
        return status; 
    }
    
    /* store the CB */
    m_SNMPSUBAGT_CB_SET(cb);

    /* Initialize the interface with Flex Log Service */
    status = snmpsubagt_dla_initialize(cb->svc_id);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* free the CB */
        snmpsubagt_cb_finalize(cb); 

        /* write on the console */
        m_NCS_SNMPSUBAGT_DBG_SINK(status);
        return status;
    }
    
    /* log that DLS init is success */
    m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_DTA_INIT_SUCCESS);

    /* extract the startup parameters, if at all any */
    status = snmpsubagt_cb_update(cb, argc, argv); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log that cb update failed  TBD */ 
        
        /* free the CB */
        snmpsubagt_cleanup(cb);

        return status;
    }
    /* initialize the other services */
    status = snmpsubagt_svcs_init(cb); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* free the CB */
        snmpsubagt_cleanup(cb);

        return status;
    }

    /* open the file to write the component name */
    fp = sysf_fopen(m_SUBAGT_PID_FILE, "w");/* /var/run/ncs_snmp_subagt.pid */
    if (fp == NULL)
    {
        return NCSCC_RC_FAILURE;
    }

    status = getpid();
    status = sysf_fprintf(fp, "%d", status);
    if(status < 1)
    {
        fclose(fp);
        return NCSCC_RC_FAILURE;
    }
    sysf_fclose(fp);

    /* install the signal handler */ 
    m_NCS_SIGNAL(SIGHUP, snmpsubagt_sighup_handler);

    /* create the selection object associated with SIGHUP signal handler */
    m_NCS_SEL_OBJ_CREATE(&cb->sighdlr_sel_obj);

    /* IR00061409-start */
    /* install the signal handler */ 
    m_NCS_SIGNAL(SIGUSR1, snmpsubagt_amf_sigusr1_handler);

    /* create the selection object associated with SIGUSR1 signal handler */
    m_NCS_SEL_OBJ_CREATE(&cb->sigusr1hdlr_sel_obj);
    /* IR00061409-end */

    /* install the signal handler */ 
    m_NCS_SIGNAL(SIGUSR2, snmpsubagt_sigusr2_handler);

    /* create the selection object associated with SIGUSR2 signal handler */
    m_NCS_SEL_OBJ_CREATE(&cb->sigusr2hdlr_sel_obj);

    /* create the mail box to communicate with the SubAgent thread */
    status = m_NCS_IPC_CREATE(&cb->subagt_mbx); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */

        /* clean the subagt */
        snmpsubagt_cleanup(cb);
        return status; 
    }

    /* log the mail-box id */
    
    /* Attach the IPC to the created thread */ 
    m_NCS_IPC_ATTACH(&cb->subagt_mbx); 
    
    /* Create the SubAgent's thread */
    status = m_NCS_TASK_CREATE((NCS_OS_CB)snmpsubagt_start, 
                        &cb->subagt_mbx, 
                        "NCS_SNMP_SUBAGT",
                        NCS_SNMPSUBAGT_PRIORITY, 
                        NCS_SNMPSUBAGT_STACK_SIZE,
                        &cb->subagt_task); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */

        /* Detach the IPC. */
        m_NCS_IPC_DETACH(&cb->subagt_mbx, NULL, NULL);

        /* release the IPC */ 
        m_NCS_IPC_RELEASE(&cb->subagt_mbx, snmpsubagt_leave_on_queue_cb); 

        /* clean up the CB */
        snmpsubagt_cleanup(cb);
        return status; 
    }
    
    /* Put the thread in the Active state */
    status = m_NCS_TASK_START(cb->subagt_task);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* kill the created task */
        m_NCS_TASK_RELEASE(cb->subagt_task); 
        
        /* Detach the IPC. */
        m_NCS_IPC_DETACH(&cb->subagt_mbx, NULL, NULL);

        /* release the IPC */ 
        m_NCS_IPC_RELEASE(&cb->subagt_mbx, snmpsubagt_leave_on_queue_cb); 

        /* clean up the CB */
        snmpsubagt_cleanup(cb);

        /* log the error */
        return status; 
    }

    return status; 
}
/******************************************************************************
*  Name:          snmpsubagt_svcs_init
*  Description:   Start routine for the NCS SNMP SubAgent thread
*                  - Initialize the interface with AMF, Eda, MAC, Agent
*
*  Arguments:     NCSSA_CB* - SubAgent's Control block 
*
*  Returns:       
*  Note:
******************************************************************************/
/* Function to initialize the SubAgent */
static uns32
snmpsubagt_svcs_init(NCSSA_CB *cb)
{
    uns32       status = NCSCC_RC_SUCCESS; 

    /* Log the function entry */ 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_SVCS_INIT);

    if (cb == NULL)
    {
       /* log that an invalid input param is received */
       m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
       return NCSCC_RC_FAILURE; 
    }

    /* LOG -- CB Create and Init success */
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_CREATE_SUCCESS);
 
/* IR00061409 */
#if 0
#if (BBS_SNMPSUBAGT == 0)
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
       /* log that AMF Interface is init is success */
       m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_INIT_SUCCESS);
#endif
#endif
     
    /* Initialize the interface with EDSv */
    status = snmpsubagt_eda_initialize(cb);
    if (status != SA_AIS_OK)
    {  
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_INIT_FAILED,status,0,0);
        if(status == SA_AIS_ERR_TRY_AGAIN)
        {
           /* saEvt api returned error to try again. set the status and start the timer. */
           cb->edaInitStatus = m_SNMPSUBAGT_EDA_INIT_RETRY;
           m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_INIT_RETRY);
           if(snmpsubagt_timer_start(cb, SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT) == NCSCC_RC_FAILURE)
           {
              /* timer couldn't be started. return failure. */
              m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TMR_FAIL, NCSCC_RC_FAILURE, 0, 0);
              return NCSCC_RC_FAILURE;
           }
        }
        else /* error is something other than TRY_AGAIN */
           return NCSCC_RC_FAILURE; 
    }
    else
       /* log that EDA Interface is init is success */
       m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_INIT_SUCCESS);

    /* Initialize the session with the Agent */
    status = snmpsubagt_netsnmp_lib_initialize(cb);
    if (status != NCSCC_RC_SUCCESS)
    {  
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AGT_INIT_FAILED,status,0,0);
  
        return status; 
    }

    /* log that SNMP Agent Interface is init is success */
    m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AGENT_INIT_SUCCESS);
    
    /* Perform init for Subagent's EDPs */
    status = snmpsubagt_edu_edp_initialize(&cb->edu_hdl);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error from EDU */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_REGISTER_FAILED,status,0,0);
  
        return status; 
    }

    /* Initialize the interface with Checkpoint Service */

    /* initialize the interface with the MAC */
    status = snmpsubagt_mac_initialize();
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error from MAC initialization */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MAC_INIT_FAILED,status,0,0);
        
        return status; 
    }
    
    /* log that MAC Interface is init is success */
    m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_MAC_INIT_SUCCESS);
    
    return status;
}

/******************************************************************************
*  Name:          snmpsubagt_start
*  Description:   Start routine for the NCS SNMP SubAgent thread
*                  - Initializes the control block 
*                  - Update the startup parameters 
*                  - Initialize the interface with AMF and Eda
*                  - Wait and Service the requests from AMF and EDA and Agent
*
*  Arguments:     SYSF_MBX* - SubAgent's mail box 
*
*  Returns:       NULL - Something went wrong.           
*                 Will Never return, if everything is going well. 
*  Note:
******************************************************************************/
/* Function to initialize the SubAgent */
static void
snmpsubagt_start(SYSF_MBX   *subagt_mbx)
{
    uns32       status = NCSCC_RC_SUCCESS; 
#if 0
    SaAisErrorT saf_status = SA_AIS_OK; 
#endif
    NCSSA_CB    *cb = NULL; 

    /* Log the function entry */ 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_SUBAGT_START);

    /* validate the input */
    if (subagt_mbx == NULL)
    {
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_NO_MBX, 0,0,0);
        
        snmpsubagt_cleanup(m_SNMPSUBAGT_CB_GET);
        return; 
    }

    /* Get the CB */
    cb = m_SNMPSUBAGT_CB_GET;
    if (cb == NULL)
    {
       /* log that an invalid input param is received */
       m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
       return; 
    }

    /* IR00061409 */
    status = subagt_rda_init_role_get(cb);
    if(status != NCSCC_RC_SUCCESS)
    {
        /* log the error & return */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_RDA_INIT_ROLE_GET_FAILED, status,0,0);
        /* free the CB */
        snmpsubagt_cb_finalize(cb); 
        m_NCS_SNMPSUBAGT_DBG_SINK(status);
        return ;
    }
    if(cb->haCstate ==  SNMPSUBAGT_HA_STATE_ACTIVE)
    {
        m_SNMPSUBAGT_STATE_LOG(SNMPSUBAGT_HA_STATE_CHNGE,SNMPSUBAGT_HA_STATE_NULL,cb->haCstate);
        /*call the snmpsubagt_ACTIVE_process with required inputs(cb & invocation) */
        status = snmpsubagt_ACTIVE_process(cb,0); 
        if(status != NCSCC_RC_SUCCESS)
        {
            /* free the CB */
            snmpsubagt_cb_finalize(cb); 
            m_NCS_SNMPSUBAGT_DBG_SINK(status);
            return ;
        }
    }
    else if(cb->haCstate == SNMPSUBAGT_HA_STATE_STANDBY)
    {
        m_SNMPSUBAGT_STATE_LOG(SNMPSUBAGT_HA_STATE_CHNGE,SNMPSUBAGT_HA_STATE_NULL,cb->haCstate);
        status = snmpsubagt_STANDBY_process(cb,0);
    }

/* IR00061409 */
#if 0
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

       #if (BBS_SNMPSUBAGT == 0)    
       /* Register the component with the Availability Agent */
       saf_status = saAmfComponentRegister(cb->amfHandle, &cb->compName, NULL);
       if (saf_status != SA_AIS_OK)
       {
           /* log the error */
           m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_COMP_REG_FAILED,
               cb->amfHandle, saf_status,0/*  cb->compName TBD */);
           if(saf_status == SA_AIS_ERR_TRY_AGAIN)
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
       #endif
    }
#endif

    /* log that Subagent init is success */
    m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_INIT_SUCCESS);

    /* Wait for the requests from all the above mentioned interfaces */
    while (1)
    {
        status = snmpsubagt_request_process(cb); 
        if (status != NCSCC_RC_SUCCESS)    
        {
           /* Log the error */
           /* On repeated failures, does the SubAgent need to inform 
            * AMF framework through 'saAmfErrorReport()' ?? TBD -- Mahesh 
            */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_RQST_PROCESS_FAILED,status,0,0);

            if (status == NCSCC_RC_INVALID_PING_TIMEOUT) return;
  
        }
    }
    m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_FATAL_ERROR,0,0,0);
  
    return; 
}

/******************************************************************************
*  Name:          ncs_snmpsubagt_destroy
*  Description:   Destroy routine for the NCS SNMP SubAgent thread
*                 - Post a dummy message to the SubAgent's thread for destroy
*
*  Arguments:     Nothing
*
*  Returns:       NCSCC_RC_SUCCESS - All the interfaces were down properly
*                 NCSCC_RC_FAILURE - Something went wrong.           
*  Note:
******************************************************************************/
/* DESTROY DL-SE API prototype */
uns32
ncs_snmpsubagt_destroy()
{
    uns32               status = NCSCC_RC_FAILURE; 
    NCSSA_CB            *cb = NULL;
    SNMPSUBAGT_MBX_MSG  *post_me = NULL; 

    /* Log the Function Entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_DESTROY);

    /* get the CB */
    cb = m_SNMPSUBAGT_CB_GET;
    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE; 
    }

    /* fill in the message details to destroy the subagent */
    post_me = m_MMGR_SNMPSUBAGT_MBX_MSG_ALLOC;
    if (post_me == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_MBX_MSG_MALLOC_FAIL);
        return status; 
    }
    m_NCS_MEMSET(post_me, 0, sizeof(SNMPSUBAGT_MBX_MSG)); 
    post_me->msg_type = SNMPSUBAGT_MBX_MSG_DESTROY; 

    /* post a message to the SubAgent's thread */ 
    status = m_NCS_IPC_SEND(&cb->subagt_mbx, 
                            (NCS_IPC_MSG*)post_me, 
                            NCS_IPC_PRIORITY_HIGH); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
    }

    return status; 
}

/******************************************************************************
*  Name:          snmpsubagt_destroy
*  Description:   Destroy routine for the NCS SNMP SubAgent thread
*                  - Finalize the session with the Agent, EDA and AMF
*                  - Finatializes the control block 
*
*  Arguments:     SubAgent's Control Block
*                 Flag to indicate whether component has to be unregistered with amf.
*
*  Returns:       NCSCC_RC_SUCCESS - All the interfaces were down properly
*                 NCSCC_RC_FAILURE - Something went wrong.           
*  Note:
******************************************************************************/
uns32
snmpsubagt_destroy(NCSSA_CB     *cb, NCS_BOOL comp_unreg)
{
    uns32   status = NCSCC_RC_FAILURE; 
    
    /* Log the Function Entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_SUBAGT_DESTROY);

    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE; 
    }

    /* de-initialize the NET-SNMP Library to interface with Agent */
    status = snmpsubagt_netsnmp_lib_finalize(cb); 
    if (status != NCSCC_RC_SUCCESS)
    {
 
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_NETSNMP_LIB_FINALIZE_FAILED, status,0,0);
 
        /* continue shutdown */
    }
    else
    {
        m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AGENT_FINALIZE_SUCCESS);
    }
    
    /* stop the snmpd timer if running. */
    if(cb->snmpd_timer.timer_id != TMR_T_NULL)
    {
       m_NCS_TMR_STOP(cb->snmpd_timer.timer_id);

       /* destroy the timer. */
       m_NCS_TMR_DESTROY(cb->snmpd_timer.timer_id);
       cb->snmpd_timer.timer_id = TMR_T_NULL;
    }

    /* de-nitialize the interface with EDSv */
    status = snmpsubagt_eda_finalize(cb);
    if (status != NCSCC_RC_SUCCESS)
    {
 
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_FINALIZE_FAILED, status,0,0);
 
        /* continue shutdown */
    }
    else
    {
        m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_FINALIZE_SUCCESS);
    }
    
#if (BBS_SNMPSUBAGT == 0)
    if(comp_unreg == TRUE)
    {
        /* de-nitialize the interface with Availability Management Framework */
        status = snmpsubagt_amf_finalize(cb); 
        if (status != NCSCC_RC_SUCCESS)
        {
 
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_FINALIZE_FAILED, status,0,0);
 
            /* continue shutdown */
        } 
        else
        {
            m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_FINALIZE_SUCCESS);
        }
    }
    else
    {
        /* stop the timer. */
        if(cb->amf_timer.timer_id != TMR_T_NULL)
        {
           m_NCS_TMR_STOP(cb->amf_timer.timer_id);
        }

        /* destroy the timer. */
        m_NCS_TMR_DESTROY(cb->amf_timer.timer_id);

        status = saAmfFinalize(cb->amfHandle);
        if(status != SA_AIS_OK)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_FINALIZE_FAILED, status,0,0);
        }
        else
        {
            m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_AMF_FINALIZE_SUCCESS);
        }
    }
#endif

    /* Finalize the MAC Interface */
    status = snmpsubagt_mac_finalize();
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MAC_FINALIZE_FAILED,status, 0, 0);
        /* continue shutdown process */
    }
    else
    {
        m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_MAC_FINALIZE_SUCCESS);
    }

    /* Detach the IPC. */
    m_NCS_IPC_DETACH(&cb->subagt_mbx, NULL, NULL);

    /* release the IPC */ 
    m_NCS_IPC_RELEASE(&cb->subagt_mbx, snmpsubagt_leave_on_queue_cb); 

    /* finalize the interface with DLA */
    status = snmpsubagt_dla_finalize(cb->svc_id);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_DTA_FINALIZE_FAILED,status, 0, 0);
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_DESTROY_FAILED,0,0,0);     
    }
    
    /* de-initialize the NCS SubAgent Control Block */
    status = snmpsubagt_cb_finalize(cb); 
    if (status != NCSCC_RC_SUCCESS)
    { 
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_CB_FINALIZE_FAILED, status,0,0);
 
    }
     
    return status; 
}

#if 0  
/******************************************************************************
*  Name:          ncs_snmpsubagt_about
*  Description:   About the NCS SNMP SubAgent
*
*  Arguments:     arg - NCS SNMP SubAgent's Startup parameters from BOM
*
*  Returns;             NCSCC_RC_FAILURE  - Something went wrong
*                       NCSCC_RC_SUCCESS  - Everything went well 
*  Note:
*****************************************************************************/
/* ABOUT DL-SE API prototype */
uns32
ncs_snmpsubagt_about(NCS_LIB_ABOUT *about)
{
    uns32   status = NCSCC_RC_SUCCESS; 

    /* Log the Function Entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_ABOUT);

    /* validate the input */
    if (about == NULL)
        return NCSCC_RC_FAILURE; 

    /* Service Name - An ASCII text version of the subsystem 
     * that this component is a part. 
     */
    m_NCS_OS_STRCPY(about->name, "NCS SNMP SUBAGENT\n");

    /* Component Name  An ASCII text version of the components name. */
    m_NCS_OS_STRCPYS(about->comp_name, "MAHESH -- TBD I do not Know/NMS\n"); 

    /* Version - a revision identifier noting what release this software is. */
    m_NCS_OS_STRCPYstrcpy(about->version, "NCS SSNMP SUBAGENT Ver 1.0\n");

    /* Description  a high level ASCII string description of this service.  */
    m_NCS_OS_STRCPY(about->descry, 
        "NCS SNMP SubAgent, Converts the SNMP requests to MAB request and vice versa...\n"); 

    /* Options  an inventory of the (significant) compile flags that this 
     * module was built with. 
     */
    m_NCS_OS_STRCPY(about->options, "Significant Compile time flags\n");

    return status; 
}

/******************************************************************************
*  Name:          ncs_snmpsubagt_diagnose
*  Description:   Diagnose API of the SubAgent
*
*  Arguments:     
*
*  Returns:       
*  Note:
*****************************************************************************/
/* DIAGNOSE  DL-SE API prototype */
uns32
ncs_snmpsubagt_diagnose()
{
    /* TBD */
    /* Following things can be done as identified in the LLD */ 
    /* Implementation left for the Phase - 2 */
    
    /* Following Things can be dumped 
       1. CB Contents
       2. Statistcs(Traps, Agentx requests etc...,
       3. Number of Role Changes
       4. SRM details (like CPU and Mem usages)
       5. Health of the SubAgent
     */
    return NCSCC_RC_SUCCESS; 
}
  
#endif

/******************************************************************************
*  Name:          snmpsubagt_cleanup
*  Description:   Error Handling/clean up job during the init time
*  Arguments:     NCSSA_CB * - SubAgent's Control Block
*  Returns:       Nothing 
*  Note:
*****************************************************************************/
static  void
snmpsubagt_cleanup(NCSSA_CB *cb)
{
    SaAmfHandleT    nullAmfHandle; 
    SaEvtHandleT    nullEvtHandle;    
    uns32           status = NCSCC_RC_SUCCESS; 

 
    /* Log the Function Entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_CLEANUP);

    if (cb == NULL)
    {
        /* log that an invalid input param is received */
       m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
       return; 
    }

    m_NCS_MEMSET(&nullAmfHandle, 0 , sizeof(SaAmfHandleT));
    m_NCS_MEMSET(&nullEvtHandle, 0 , sizeof(SaEvtHandleT));

#if (BBS_SNMPSUBAGT == 0)
    /* Free the AMF related data */
    if (m_NCS_MEMCMP(&cb->amfHandle, &nullAmfHandle, sizeof(SaAmfHandleT))!= 0)
    {
        status = snmpsubagt_amf_finalize(cb);
    }
#endif

    /* Free the EVT related data */
    if (m_NCS_MEMCMP(&cb->evtHandle, &nullEvtHandle, sizeof(SaEvtHandleT))!= 0)
    {
        status = snmpsubagt_eda_finalize(cb);
    }

    /* unregister with the AGENTX Master */
    status = snmpsubagt_netsnmp_lib_finalize(cb); 

    /* unregister the MAC Interface, if created already */
    status = snmpsubagt_mac_finalize(); 

    /* Un register with FLA */ 
    status = snmpsubagt_dla_finalize(cb->svc_id); 

    /* Free the allocated CB  */
    status = snmpsubagt_cb_finalize(cb); 
        
    /* precautionary */
    m_SNMPSUBAGT_CB_SET(NULL);
 #if 0 
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_SET_NULL); 
#endif
  
    return; 
}

/*****************************************************************************\
 *  Name:          ncs_snmpsubagt_init_deinit_msg_post                        * 
 *                                                                            *
 *  Description:   To send the name of the function to be sent to the         * 
 *                 subagent's thread to load the init or deinit routine       * 
 *                 in the subagent thread.                                    *
 *                                                                            *
 *  Arguments:     uns8* - Name of the function to be posted                  *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
\*****************************************************************************/
uns32
ncs_snmpsubagt_init_deinit_msg_post(uns8* init_deinit_routine)
{
    uns32               status = NCSCC_RC_FAILURE; 
    SNMPSUBAGT_MBX_MSG  *post_me = NULL;
    NCSSA_CB            *cb = NULL; 

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_INIT_DEINIT_MSG_POST);

    if (init_deinit_routine == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_ARG_NULL);

        return status; 
    }
   
    /* get the control block */
    cb = m_SNMPSUBAGT_CB_GET;
    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);

        return status; 
    }
    
    /* take the lock before posting the message to the subagent's thread */ 
    /* As there is a chance that subagent is in the processing of destroying
     * itself. 
     */ 
    status = m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_CB_LOCK_FAILED, status,0,0);

        return status; 
    }

    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_TAKEN);
    
    /* update the message info */
    post_me = m_MMGR_SNMPSUBAGT_MBX_MSG_ALLOC;
    if (post_me == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_MBX_MSG_MALLOC_FAIL);
        
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_RELEASED);
        return status; 
    }
    m_NCS_MEMSET(post_me, 0, sizeof(SNMPSUBAGT_MBX_MSG)); 
    post_me->msg_type = SNMPSUBAGT_MBX_MSG_MIB_INIT_OR_DEINIT;
    m_NCS_STRCPY(post_me->init_or_deinit_routine, 
                init_deinit_routine);

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

    /* Store the message in the last reg message */
    if (init_deinit_routine[2] == 'r')
    {
       m_NCS_MEMSET (cb->lastRegMsg, 0, 255);
       strcpy (cb->lastRegMsg, init_deinit_routine);
    }
    
    /* release the lock */ 
    status = m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_CB_UNLOCK_FAILED, status,0,0);
    }
    
    m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_RELEASED);
    return status; 
}

static void
snmpsubagt_sighup_handler(int i_sig_num)
{
    NCSSA_CB    *cb = m_SNMPSUBAGT_CB_GET; 

    m_NCS_SEL_OBJ_IND(cb->sighdlr_sel_obj);

    return;
}
/* IR00061409 */
static void
snmpsubagt_amf_sigusr1_handler(int i_sig_num)
{
    NCSSA_CB    *cb = m_SNMPSUBAGT_CB_GET; 

    m_NCS_SEL_OBJ_IND(cb->sigusr1hdlr_sel_obj);

    return;
}

static void
snmpsubagt_sigusr2_handler(int i_sig_num)
{
    NCSSA_CB    *cb = m_SNMPSUBAGT_CB_GET; 

    m_NCS_SEL_OBJ_IND(cb->sigusr2hdlr_sel_obj);

    return;
}

/****************************************************************************\
*  Name:          snmpsubagt_leave_on_queue_cb                               * 
*                                                                            *
*  Description:   This is callback function to free the pending message      *
*                 in the mailbox.                                            *
*                                                                            *
*  Arguments:     arg2 - Pointer to the message to be freed.                 * 
*                                                                            * 
*  Returns:       TRUE  - Everything went well                               *
*  NOTE:                                                                     * 
\****************************************************************************/
static NCS_BOOL snmpsubagt_leave_on_queue_cb (void *arg1, void *arg2)
{
    SNMPSUBAGT_MBX_MSG *msg;

    if ((msg = (SNMPSUBAGT_MBX_MSG *)arg2) != NULL)
    {
       m_MMGR_SNMPSUBAGT_MBX_MSG_FREE(msg);
    }
    return TRUE;
}

