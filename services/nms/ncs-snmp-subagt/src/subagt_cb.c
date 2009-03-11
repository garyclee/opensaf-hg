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
  
  DESCRIPTION: This file describes the routines Control Block related
                routines.  
  
  ***************************************************************************/ 
#include "subagt.h"
#include "subagt_oid_db.h" 

/******************************************************************************
 *  Name:          snmpsubagt_cb_init
 *
 *  Description:   To allocate the memory for the CB and to intialize
 *                  the CB parameters to default values
 * 
 *  Arguments:     None
 *
 *  Returns:       pointer to the CB  - everything is OK
 *                 NULL   -  failure
 *  NOTE: 
 *****************************************************************************/
NCSSA_CB*
snmpsubagt_cb_init(void)
{
    NCS_PATRICIA_PARAMS tree_params;
    NCSSA_CB            *cb = NULL;
    uns32               status = NCSCC_RC_FAILURE;  

    /* allocate the place to live in */
    cb = m_MMGR_NCSSA_CB_ALLOC; 
    if (cb == NULL)
    {
        /* log the error, Interface with FLA is not yet inited */
        m_NCS_SNMPSUBAGT_DBG_SINK(NCSCC_RC_OUT_OF_MEM);

        /* Should the SubAgent inform SPA about these failures?? 
         * A return from here leads to thread termination - TBD */
        return NULL; 
    }

    /* Level it */
    memset(cb, 0, sizeof(NCSSA_CB)); 

    /* initialize the lock */ 
    status = m_NCS_LOCK_INIT(&cb->cb_lock); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_NCS_SNMPSUBAGT_DBG_SINK(status); 
        
        /* free the control block */
        m_MMGR_NCSSA_CB_FREE(cb); 

        return NULL; 
    }

    /* set the Service ID */
    cb->svc_id = NCS_SERVICE_ID_SNMPSUBAGT;

    /* SubAgent blocking/nonblocking mode? */
    cb->block = SNMPSUBAGT_BLOCK_MODE; 

    /* initilize the OID data base tree  */
    memset(&tree_params, 0, sizeof(NCS_PATRICIA_PARAMS));
    tree_params.key_size = sizeof(NCSSA_OID_DATABASE_KEY);
    tree_params.info_size = 0;
    if ((ncs_patricia_tree_init(&cb->oidDatabase, &tree_params))
               != NCSCC_RC_SUCCESS)
    { 
        /* log the error */
        m_NCS_SNMPSUBAGT_DBG_SINK(NCSCC_RC_OUT_OF_MEM);

        /* destroy the lock */ 
        status = m_NCS_LOCK_DESTROY(&cb->cb_lock); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the error */ 
            m_NCS_SNMPSUBAGT_DBG_SINK(status);

            /* should I return from here?? May lead to a memleak */ 
        }

        /* free the control block */
        m_MMGR_NCSSA_CB_FREE(cb); 
        return NULL; 
    }

    /* initialize the SAF version details */
    cb->safVersion.releaseCode = (char) 'B'; 
    cb->safVersion.majorVersion = 0x01;
    cb->safVersion.minorVersion = 0x01;

    /* set the dispatch flags to ALL */
    /* this means that, all the pending events will be called one after the
     * other in the same dispatch call 
     */
    cb->subagt_dispatch_flags = SA_DISPATCH_ONE; 

    /* Initialize the HA state to NULL */
    cb->haCstate = SNMPSUBAGT_HA_STATE_NULL;

    /* health check not yet started */ 
    cb->healthCheckStarted = FALSE; 
    
    /* Fill in the Health check key */ 
    strcpy(cb->healthCheckKey.key,m_SNMPSUBAGT_AMF_HELATH_CHECK_KEY);
    cb->healthCheckKey.keyLen=strlen(cb->healthCheckKey.key);

    /* Health check invocation type; for subagent, health check will be
     * driven by the AMF
     */
    cb->invocationType = m_SUBAGT_HB_INVOCATION_TYPE;

    /* Recommended recovery is to failover */ 
    cb->recommendedRecovery = m_SUBAGT_RECOVERY; 

    /* initialize the event callback routine */
    cb->evtCallback.saEvtEventDeliverCallback = 
        snmpsubagt_eda_callback;

    /* initialize the subscription id for the event Channel */
    cb->evtSubscriptionId = m_SNMPSUBAGT_EDA_SUBSCRIPTION_ID_TRAPS; 

    /* initialize the event filters */ 

    /* 1. Initialize the filter string */
    strcpy(cb->evtPatternStr, m_SNMPSUBAGT_EDA_EVT_FILTER_PATTERN);

    /* 2. Initialize the pattern structure */
    cb->evtFilter.filterType = SA_EVT_PASS_ALL_FILTER;
    
    cb->evtFilter.filter.pattern = cb->evtPatternStr;
    cb->evtFilter.filter.patternSize = m_SNMPSUBAGT_EDA_EVT_FILTER_PATTERN_LEN; 
   
    /* 3. initialize the pattern Array */ 
    cb->evtFilters.filtersNumber = 1; 
    cb->evtFilters.filters = &cb->evtFilter; 
    
    /* update the Channel Name */
    strcpy(cb->evtChannelName.value, m_SNMPSUBAGT_EDA_EVT_CHANNEL_NAME); 
    cb->evtChannelName.length = strlen(cb->evtChannelName.value);

    /* EDU_HDL init */
    m_NCS_EDU_HDL_INIT(&cb->edu_hdl);

    /* Initialize the list of pending registrations */
    cb->new_registrations.order = NCS_DBLIST_ANY_ORDER; 
    cb->new_registrations.free_cookie = snmpsubagt_pending_regs_free;   
    cb->new_registrations.cmp_cookie = snmpsubagt_pending_regs_cmp; 

    cb->edaInitStatus = m_SNMPSUBAGT_EDA_INIT_NOT_STARTED;
    cb->amfInitStatus = m_SNMPSUBAGT_AMF_INIT_NOT_STARTED;

    /* everything went well */    
    return cb;
} 

/******************************************************************************
 *  Name:          snmpsubagt_cb_finalize
 *
 *  Description:   To free the control block contents
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
/* Function to free the controlblock */
uns32
snmpsubagt_cb_finalize(NCSSA_CB *cb)
{
    uns32       status = NCSCC_RC_FAILURE;    

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_CB_FINALISE); 

    if (cb == NULL)
    {
        /* there is nothing to free */
        return NCSCC_RC_SUCCESS; 
    }

    /* take the lock to avoid others using the CB from NOW...*/
    status = m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE);   
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the lock error */ 
        return status; 
    }

    /* EDU_HDL reset */
    m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);

    if(cb->eda_timer.timer_id != TMR_T_NULL)
    {
       m_NCS_TMR_STOP(cb->eda_timer.timer_id);

       /* destroy the timer. */
       m_NCS_TMR_DESTROY(cb->eda_timer.timer_id);
       cb->eda_timer.timer_id = TMR_T_NULL;
    }

    if(cb->amf_timer.timer_id != TMR_T_NULL)
    {
       m_NCS_TMR_STOP(cb->amf_timer.timer_id);

       /* destroy the timer. */
       m_NCS_TMR_DESTROY(cb->amf_timer.timer_id);
       cb->amf_timer.timer_id = TMR_T_NULL;
    }

    if(cb->snmpd_timer.timer_id != TMR_T_NULL)
    {
       m_NCS_TMR_STOP(cb->snmpd_timer.timer_id);

       /* destroy the timer. */
       m_NCS_TMR_DESTROY(cb->snmpd_timer.timer_id);
       cb->snmpd_timer.timer_id = TMR_T_NULL;
    }

    /* de-initialize the OID database */
    snmpsubagt_table_oid_destroy(&cb->oidDatabase); 
    ncs_patricia_tree_destroy(&cb->oidDatabase);

    /* destroy the pending registrations list, if there are any pending */
    snmpsubagt_pending_registrations_free(&cb->new_registrations);

    /* destroy the lock */ 
    status = m_NCS_LOCK_DESTROY(&cb->cb_lock); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        /* should I return from here?? May lead to a memleak */ 
    }

    /* free the control block */
    m_MMGR_NCSSA_CB_FREE(cb); 
    
    return NCSCC_RC_SUCCESS; 
}

/******************************************************************************
 *  Name:          snmpsubagt_cb_update
 *
 *  Description:  To update the Control Block with the startup parameters 
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block 
 *                 arg *        - startup arguments 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32   
snmpsubagt_cb_update(NCSSA_CB   *cb, 
                     int32      argc,
                     uns8       **argv)
{
    uns32       status = NCSCC_RC_FAILURE; 

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_CB_UPDATE); 
    
    /* validate the inputs */
    if (cb == NULL)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
        return NCSCC_RC_FAILURE; 
    }

    /* parse the startup parameters as supported in NET-SNMP package */
    /* 
     * For the time being we are supporting only NET-SNMP Supported 
     * features 
     */
    status = snmpsubagt_agt_startup_params_process(argc, argv);
    if (status != NCSCC_RC_SUCCESS)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_ARG_NULL); 
        return NCSCC_RC_FAILURE; 
    }

    return status; 
}

/******************************************************************************
 *  Name:          snmpsubagt_timer_cb
 *
 *  Description:   Callback function for timeout of EDA/AMF Initialization timer.
 * 
 *  Arguments:      void *arg 
 *                
 *  Returns:       NONE 
 ******************************************************************************/

void snmpsubagt_timer_cb(void *arg)
{
        SNMPSUBAGT_TIMER         *timer_data = (SNMPSUBAGT_TIMER *) arg;
        NCSSA_CB                 *pSacb = (NCSSA_CB *) timer_data->cb;
        SNMPSUBAGT_MBX_MSG       *post_me = NULL;
        uns32                    status;

        /* send a mesg to subagt's mailbox. */
        /* stop the timer */
        m_NCS_TMR_STOP(timer_data->timer_id);

        /* allocate the memory for the message */
        post_me = m_MMGR_SNMPSUBAGT_MBX_MSG_ALLOC;
        if (post_me == NULL)
        {
            /* log the error */
            m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_MBX_MSG_MALLOC_FAIL);
            return ;
        }

        /* compose the message to post to the subagent's mail box*/
        memset(post_me, 0, sizeof(SNMPSUBAGT_MBX_MSG));
        if(timer_data->msg_type == SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT)
           post_me->msg_type = SNMPSUBAGT_MBX_MSG_RETRY_EDA_INIT;
        else if(timer_data->msg_type == SNMPSUBAGT_TMR_MSG_RETRY_AMF_INIT)
           post_me->msg_type = SNMPSUBAGT_MBX_MSG_RETRY_AMF_INIT;
        else if(timer_data->msg_type == SNMPSUBAGT_TMR_MSG_RETRY_CHECK_SNMPD)
           post_me->msg_type = SNMPSUBAGT_MBX_MSG_RETRY_CHECK_SNMPD;
        else
           return;

        /* take the lock */
        status = m_NCS_LOCK(&pSacb->cb_lock, NCS_LOCK_WRITE);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_CB_LOCK_FAILED, status,0,0);
            return ;
        }

        /* post a message to the SubAgent's thread */
        status = m_NCS_IPC_SEND(&pSacb->subagt_mbx,
                                (NCS_IPC_MSG*)post_me,
                                NCS_IPC_PRIORITY_HIGH);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_MBX_IPC_SEND_FAILED, status,0,0);

            m_NCS_UNLOCK(&pSacb->cb_lock, NCS_LOCK_WRITE);
            m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_LOCK_RELEASED);
            return ;
        }

        /* release the lock */
        status = m_NCS_UNLOCK(&pSacb->cb_lock, NCS_LOCK_WRITE);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_CB_UNLOCK_FAILED, status,0,0);
            return;
        }
        return;
}
/******************************************************************************
 *  Name:          snmpsubagt_timer_start
 *
 *  Description:   Function to start timer for EDA/AMF Initialization 
 * 
 *  Arguments:      NCSSA_CB *pSacb, SNMPSUBAGT_TMR_MSG_TYPE 
 *                
 *  Returns:       NCSCC_RC_SUCCESS
 *                 NCSCC_RC_FAILURE 
 ******************************************************************************/

uns32 snmpsubagt_timer_start (NCSSA_CB *pSacb, SNMPSUBAGT_TMR_MSG_TYPE msgType)
{
   SNMPSUBAGT_TIMER *timer_data = NULL;

   /* check the msgType and accordingly initialise the timer data structure in cb. */
   if(msgType == SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT)
   {
      pSacb->eda_timer.msg_type = SNMPSUBAGT_TMR_MSG_RETRY_EDA_INIT;
      pSacb->eda_timer.period = m_SNMPSUBAGT_EDA_TIMEOUT_IN_SEC * SYSF_TMR_TICKS;
      pSacb->eda_timer.cb = (void *) pSacb;
      timer_data = &(pSacb->eda_timer); 
   }
   else if(msgType == SNMPSUBAGT_TMR_MSG_RETRY_AMF_INIT)
   {
      pSacb->amf_timer.msg_type = SNMPSUBAGT_TMR_MSG_RETRY_AMF_INIT;
      pSacb->amf_timer.period = m_SNMPSUBAGT_AMF_TIMEOUT_IN_SEC * SYSF_TMR_TICKS;
      pSacb->amf_timer.cb = (void *) pSacb;
      timer_data = &(pSacb->amf_timer); 
   }
   else if(msgType == SNMPSUBAGT_TMR_MSG_RETRY_CHECK_SNMPD)
   {
      pSacb->snmpd_timer.msg_type = SNMPSUBAGT_TMR_MSG_RETRY_CHECK_SNMPD;
      pSacb->snmpd_timer.period = m_SNMPSUBAGT_SNMPD_TIMEOUT_IN_SEC * SYSF_TMR_TICKS;
      pSacb->snmpd_timer.cb = (void *) pSacb;
      timer_data = &(pSacb->snmpd_timer); 
   }
   else 
      return NCSCC_RC_FAILURE;


   if (timer_data->timer_id == TMR_T_NULL)
   {
      /* create the timer. */
      m_NCS_TMR_CREATE(timer_data->timer_id,
                       timer_data->period,
                       snmpsubagt_timer_cb,
                       (void*)(timer_data));
   }
   /* start the timer.  */
   m_NCS_TMR_START(timer_data->timer_id,
                   timer_data->period,
                   snmpsubagt_timer_cb,
                   (void*)(timer_data));

   if(timer_data->timer_id == TMR_T_NULL)
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}

