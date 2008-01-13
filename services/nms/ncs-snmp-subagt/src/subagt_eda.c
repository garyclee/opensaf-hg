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
  
  DESCRIPTION: This file describes the EDA Intrerface 
  
  ***************************************************************************/ 
#include "subagt.h"
#include "subagt_oid_db.h"
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/library/snmp_impl.h"
#include "net-snmp/agent/agent_trap.h"

/* to compose the varbind (object id and its value) */
static netsnmp_variable_list*
snmpsubagt_eda_varbind_compose(NCSSA_OID_DATABASE_NODE  *i_oid_db_node, 
                               NCS_TRAP_VARBIND       *i_trap_varbind);

/* to Fill in the value in Varbind */
static uns32 
snmpsubagt_eda_trap_varbind_value_populate(netsnmp_variable_list *var_bind,
                         struct variable  *object_details,
                         NCSMIB_PARAM_VAL *i_param_val);

/* to get the complete OID (base_oid + param_id + instance) */
static oid*
snmpsubagt_eda_full_oid_populate(oid    *base_oid, 
                                 uns32  base_oid_len, 
                                 NCSMIB_PARAM_ID  i_param_id,
                                 uns32  *i_instids,
                                 uns32  i_inst_len,
                                 size_t *io_name_length);

/* to compose the Varbind for the snmpTrap.0 */
static netsnmp_variable_list*
snmpsubagt_eda_snmpTrapId_vb_populate(NCS_PATRICIA_TREE    *i_oid_db, 
                                       uns32                i_trap_tbl_id,
                                       uns32                i_trap_id);

/* to append to the list of varbinds */
static uns32 
snmpsubagt_eda_varbindlist_append(netsnmp_variable_list **add_to_me, 
                                  netsnmp_variable_list *to_be_added);

#if 0
/* to free the list of varbinds */
uns32
snmpsubagt_eda_trap_varbinds_free(NCS_TRAP_VARBIND *trap_varbinds);
#endif

static uns32 
snmpsubagt_eda_varbinds_free(netsnmp_variable_list *to_be_freed);


static int32
subagt_eda_object_get(struct variable   *object_details, 
                      uns32             number_of_objects,
                      NCSMIB_PARAM_ID   i_param_id);
uns32 
subagt_send_v2trap(NCS_PATRICIA_TREE    *oid_db, 
                   NCS_TRAP             *trap_header);


/* helper utility to LOG NCS TRAP DATA */ 
static void subagt_log_ncs_trap_data(NCS_TRAP *ncs_trap_data);

static void subagt_log_ncs_trap_vb(NCS_TRAP_VARBIND *trap_vb); 

typedef struct snmpsa_log_trap_tbl
 { 
       uns32 tbl_id; 
       uns32 param_id; 
       uns32 inform; 
 }SNMPSA_LOG_TRAP_TBL;

typedef struct snmpsa_log_trap_param_info
{
   uns32 param_id;
   uns32 fmt_id;
   uns32 i_len;
}SNMPSA_LOG_TRAP_PARAM_INFO;

   
/* Endof  Helper utility to LOG NCS TRAP DATA */ 

/****************************************************************************** 
*  Name:          snmpsubagt_eda_initialize 
*  Description:   To intialize the session with EDA Agent
*                   - To register the callbacks
*                   - Subscribe for the events of type TRAP 
*  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK 
*                 NCSCC_RC_FAILURE   -  failure                                
*                 NCSCC_RC_EDA_INTF_INIT_FAILURE
*                 NCSCC_RC_EDA_CHANNEL_OPEN_FAILURE
*                 NCSCC_RC_SUBSCRIBE_FAILURE
*                 NCSCC_RC_EDA_SEL_OBJ_FAILURE
******************************************************************************/
SaAisErrorT 
snmpsubagt_eda_initialize(NCSSA_CB     *pSacb)
{
    uns32       channelFlags = 0;   
    SaAisErrorT status = SA_AIS_OK; 
    SaTimeT     timeout_in_ns;  /* in nano seconds */ 

    /* Log the function entry */
   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_INIT);

    if (pSacb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return SA_AIS_ERR_INVALID_PARAM; 
    }

#if 0
    if(pSacb->edaInitStatus != m_SNMPSUBAGT_EDA_INIT_COMPLETED)
    {
#endif
       /* Interface with EDSv is not initialised. */

       m_NCS_MEMSET(&timeout_in_ns, 0, sizeof(SaTimeT));
       timeout_in_ns = m_SNMPSUBAGT_EDA_TIMEOUT;

       /* 1. Create the evt handle structure, set callbacks, etc */ 
       status = saEvtInitialize(&pSacb->evtHandle,  /* output */
                               &pSacb->evtCallback,/* input */
                               &pSacb->safVersion);/* input */ 
       if (status != SA_AIS_OK)
       {
           /* log the failure */
           m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_INIT_FAIL,  
                                  status,
                                  0, 0);
           return status; 
       } 
       /* 2. Create/Open event channel with remote server (EDS)  */ 
       /* To use the SA Event Service, a process must create an event channel.  
        * The process can then open the event channel to publish events and 
        * to subscribe to receive events.  
        *IMP: Publishers can also be subscribers on the same Channel 
        */ 
       channelFlags = SA_EVT_CHANNEL_CREATE | 
                    SA_EVT_CHANNEL_PUBLISHER | 
                    SA_EVT_CHANNEL_SUBSCRIBER; 
    
       status = saEvtChannelOpen(pSacb->evtHandle, /* input */
                               &pSacb->evtChannelName, /* TBD input  */
                               channelFlags,   /* input */
                               timeout_in_ns,
                               &pSacb->evtChannelHandle); /* i/o */
       if (status != SA_AIS_OK) 
       { 
           /* log the error */
           m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_CHNL_OPEN_FAIL, 
                               status,  
                               pSacb->evtHandle, 
                               channelFlags);

           saEvtFinalize(pSacb->evtHandle);
           pSacb->evtHandle = 0; 

           return status; 
       }

       /* 3. Get a communications handle */
       status = saEvtSelectionObjectGet(pSacb->evtHandle, /* input */
                                &pSacb->evtSelectionObject); /* input */ 
       if (status != SA_AIS_OK) 
       { 
           /* log the error */
           m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_SEL_OBJ_GET_FAILED, 
                               status, 
                               pSacb->evtHandle, 
                               (uns32)pSacb->evtSelectionObject);

           saEvtChannelClose(pSacb->evtChannelHandle);
           saEvtFinalize(pSacb->evtHandle);
           pSacb->evtHandle = 0; 
           pSacb->evtChannelHandle = 0; 

           return status; 
       }                     
       
       pSacb->edaInitStatus = m_SNMPSUBAGT_EDA_INIT_COMPLETED;

       /* 4. log the EDA Selection object */
       m_SNMPSUBAGT_DATA_LOG(SNMPSUBAGT_EDA_SEL_OBJ, (int)pSacb->evtSelectionObject);
#if 0
    }
#endif

#if 0
    /* Call the API to subscribe for the events if active state assignment is done for subagt. */
    if (pSacb->haCstate == SNMPSUBAGT_HA_STATE_ACTIVE)
    {
      status = saEvtEventSubscribe(pSacb->evtChannelHandle, /* input */
                                   &pSacb->evtFilters,/* input */
                                   pSacb->evtSubscriptionId);/* input */
      if(status != SA_AIS_OK)
      {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_EVT_SUBSCRIBE_FAIL,
                               status, pSacb->evtSubscriptionId, 0);

        return status; 
      }
      /* log the subscription status */
      m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_SUBSCRIBE_SUCCESS);
   }
#endif

   /* log the end of the function */
    return SA_AIS_OK; 
}

/******************************************************************************
 *  Name:          snmpsubagt_eda_finalize
 *
 *  Description:   To Finalize the session with the EDA 
 * 
 *  Arguments:     NCSSA_CB *pSacb - SNMP SubAgent's control block 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32   
snmpsubagt_eda_finalize(NCSSA_CB *pSacb)
{
    SaAisErrorT    status = SA_AIS_OK; 

    /* Log the function entry */

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_FINALIZE);

    /* validate the inputs */
    if (pSacb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return NCSCC_RC_FAILURE; 
    }
     
    /* stop the timer. */
    if(pSacb->eda_timer.timer_id != TMR_T_NULL)
    {
       m_NCS_TMR_STOP(pSacb->eda_timer.timer_id);

       /* destroy the timer. */
       m_NCS_TMR_DESTROY(pSacb->eda_timer.timer_id);
       pSacb->eda_timer.timer_id = TMR_T_NULL;
    }

    /* 1. unsubscribe for the event from EDS
     *
     * All the pending events to be delivered to the SubAgent will be
     * purged by the EDA before delivering them, once this unsubscribing 
     * is done. 
     *  -- coutesy SAF documentation 
     */
    status = saEvtEventUnsubscribe(pSacb->evtChannelHandle, /* input */
         pSacb->evtSubscriptionId);/* input */
    if (status != SA_AIS_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_UNSUBSCRIBE_FAIL,  
                            status, 
                            pSacb->evtSubscriptionId, 
                            pSacb->evtChannelHandle);
                         
        return NCSCC_RC_FAILURE; 
    }
    /* log the unsubscription status */
    m_SNMPSUBAGT_INTF_INIT_STATE_LOG(SNMPSUBAGT_EDA_UNSUBSCRIBE_SUCCESS);
    
    /* 2. terminate the communication channels 
     *
     * Channel Close causes to delete the event channel from the 
     * cluster name space
     *    -- courtesy SAF documentation 
     */
    status = saEvtChannelClose(pSacb->evtChannelHandle/* input */);
    if (status != SA_AIS_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_CHNL_CLOSE_FAIL, 
                            status, pSacb->evtChannelHandle, 0);
        return NCSCC_RC_FAILURE; 
    }
    
    /* 3. Close all the associations with EDA, close the deal */
    status = saEvtFinalize(pSacb->evtHandle);
    if (status != SA_AIS_OK)
    {

        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_FINALIZE_FAILED,
                            status, 
                            pSacb->evtHandle, 0);

        return NCSCC_RC_FAILURE; 
    }

    return NCSCC_RC_SUCCESS; 
}

/******************************************************************************
 *  Name:          snmpsubagt_eda_callback
 *
 *  Description:   To handle the events from EDA 
 *                  - to get the data from the Event
 *                  - Log the Details of the Event 
 *                  - Send the event to NET-SNMP Agent
 * 
 *  Arguments:    subscriptionid - What is the Subscription ID
 *                eventHandle    - Event Handle to get the event data 
 *                eventDataSize  - Event Data size
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
void 
snmpsubagt_eda_callback(SaEvtSubscriptionIdT   subscriptionid, /* input */
                        SaEvtEventHandleT     eventHandle, /* input */
                        const SaSizeT         eventDataSize)/* input */
{
    SaSizeT             evtSize;
    SaAisErrorT         status = SA_AIS_OK;
    NCSSA_CB            *cb = NULL; 
    uns32               ret_code = NCSCC_RC_SUCCESS; 
    
    /* memory for the event data */
    uns8                *eventData = NULL; 
    
#if 0    
    /* To store the details about the received event */
    SaEvtEventPatternArrayT     patternArray;
    SaUint8T                    priority = 0;
    SaTimeT                     retentionTime;
    SaNameT                     publisherName;
    SaTimeT                     publishTime;
    SaEvtEventIdT               eventId;
#endif

    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_CB);

    /* get the control block */
    cb = m_SNMPSUBAGT_CB_GET;
    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL);
        return ; 
    }

    /* validate the inputs */
    if (subscriptionid != cb->evtSubscriptionId)/* mismatch in subscription */
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_SUBSCRIP_MISMATCH, 
                            subscriptionid, 
                            cb->evtSubscriptionId, 0);
        return ;
    }
#if 0
    /* Get event header info */
    status = saEvtEventAttributesGet(eventHandle, &patternArray, 
                        &priority, &retentionTime, 
                        &publisherName, &publishTime, 
                        &eventId);
    if (status != SA_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_ATTR_GET_FAIL, 
                                status,
                                eventHandle,
                                eventId);
        /* Mahesh -- TBD, I think we can continue */
    }
    else
    {
        /* log the above details of the event */
    }
#endif

    /* Prepare an appropriate-sized data buffer.
     */
     evtSize = eventDataSize;
     eventData = (uns8*)malloc((uns32)evtSize+1);
     if (eventData == NULL)
        return;
     m_NCS_MEMSET(eventData, 0, (size_t)evtSize+1);
                              

    /* Get the event data  */
    status = saEvtEventDataGet(eventHandle, /* input */
                        (void*)eventData, /* in/out */
                        &evtSize); /* in/out */
    if (status != SA_AIS_OK)
    {
        /* log the error */
        /* 
         * An event buffer of size SA_EVT_DATA_MAX_LEN bytes or more 
         * will always be able to contain the largest possible event
         * data associated with an event 
         *      --courtesy SAF documentation 
         * As per the above stmt, we shall never get the error of
         * SA_ERR_NO_SPACE from this call. Hence it is not handled here.
         */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_EVT_DATA_GET_FAIL, 
                            status,0,0);
        /* free the eventData */
        free(eventData);
        return; 
    }

    /* convert the data into the SNMP - Trap and send it to the 
     * Agentx Agent 
     */
    ret_code = snmpsubagt_eda_trapevt_to_agentxtrap_populate(&cb->edu_hdl,
                                                &cb->oidDatabase,
                                                eventData, 
                                                eventDataSize);
    if (ret_code == NCSCC_RC_FAILURE)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TE_AGENTXTRAP_FAILED, 
                               ret_code,0,0);
    }
    
    /* even if there is any error in sending the trap to the agent,
     * it will be notified to the caller from here */

    free(eventData);

    status = saEvtEventFree(eventHandle);
    if (status != SA_AIS_OK)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EVT_FREE_FAIL,
                               status,0,0);
    }

    return; 
}


/******************************************************************************
 *  Name:          snmpsubagt_eda_trapevt_to_agentxtrap_populate
 *
 *  Description:   To send the trap to the SNMP Agent 
 *                  - Decode the event data 
 *                  - Get the base oids from the data base
 *                  - Compose the varbinds 
 *                  - Send the trap 
 * 
 *  Arguments:     NCSSA_CB *cb - SNMP SubAgent's control block 
 *                 SaInvocationT    - Ref given by the AMF
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32 
snmpsubagt_eda_trapevt_to_agentxtrap_populate(
                        EDU_HDL           *edu_hdl, 
                        NCS_PATRICIA_TREE *oid_db,/*in */
                        uns8    *evtData, /* in */  
                        uns32   evtDataSize) /* in */
{
    NCS_TRAP                trap_header;
    uns32                   status = NCSCC_RC_FAILURE; 

    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_TE_TO_AX);

    /* validate the inputs */  
    if (oid_db == NULL) 
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_DB_NULL); 
        return NCSCC_RC_FAILURE; 
    }
    if ((evtData == NULL) ||
        (evtDataSize == 0))
    {
        /* log that there is no data in the received event. 
         * there is nothing to send to the Agent */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_EVT_DATA_NULL); 
        return NCSCC_RC_FAILURE; 
    }

    m_NCS_MEMSET(&trap_header, 0, sizeof(NCS_TRAP));

    /* decode the number of varbinds from the event data */
    status = subagt_edu_ncs_trap_decode(edu_hdl, evtData,
                                        evtDataSize,  &trap_header);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log that unable to decode the trap buffer */
       m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TRAP_DECODE_FAIL,status,0,0); 
        return status; 
    }

   /* Logging the NCS TRAP data - before shotting it of to TRAP receiver */ 
    subagt_log_ncs_trap_data(&trap_header);

    status = subagt_send_v2trap(oid_db, &trap_header);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log that unable to send the trap */
       m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TRAP_SEND_FAIL,status,trap_header.i_trap_tbl_id,trap_header.i_trap_id); 
       return status; 
    }

    return status; 
}

/******************************************************************************
 *  Name:          subagt_send_v2trap
 *
 *  Description:   To convert the NCS_TRAP format into native SNMP Trap format
 *                 and send it to the SNMP Agent. 
 * 
 *  Arguments:     NCS_PATRICIA_TREE    *oid_db - OID to table id mapping
                   NCS_TRAP             *trap_header - trap data to be converted
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32 
subagt_send_v2trap(NCS_PATRICIA_TREE    *oid_db, 
                   NCS_TRAP             *trap_header)
{
    uns32                   status = NCSCC_RC_SUCCESS; 
    uns32                   varbinds_count = 0;
    NCSSA_OID_DATABASE_KEY  oid_db_key;
    NCSSA_OID_DATABASE_NODE *oid_db_node = NULL;
    NCS_TRAP_VARBIND        *trap_varbinds = NULL;
    NCS_TRAP_VARBIND        *trap_varbind = NULL;
    
    netsnmp_variable_list   *var_trap = NULL;
    netsnmp_variable_list   *var_trap_elem = NULL;
    NCSSA_CB            *cb = NULL;

    /* log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_SEND_V2_TRAP);

    if ((oid_db == NULL)||
        (trap_header == NULL))
    {
        /* log the error */ 
        /* Free the trap_header data structure if trap_header is non null */
        if(trap_header != NULL) 
               status = ncs_trap_eda_trap_varbinds_free(trap_header->i_trap_vb);
        
        return NCSCC_RC_FAILURE; 
    }
    
    /* Shall we convert this event to SNMP Trap?? */
    if (trap_header->i_inform_mgr == FALSE)
    {
        /* log that notification generator does not want this trap to send
         * to the SNMP Manager
         */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_DONOT_INFORM_TRAP); 

        /* free the decoded trap data */ 
        status = ncs_trap_eda_trap_varbinds_free(trap_header->i_trap_vb);
        return status; 
    }
            
    /* Get the trap varbinds */
    trap_varbind = trap_varbinds = trap_header->i_trap_vb; 
    if (trap_varbind == NULL)
    {
        /* log the error 
         * may be we can increment the count saying there are no trap varbinds
         *  --TBD for Phase - 2
         * Can there be a chance of having a trap with no varbinds at all ??? 
         * TBD Mahesh  
         * if the above case is true, we should not return from here 
         */ 
       
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_TRAP_VARBIND_NULL); 
        return NCSCC_RC_FAILURE; 
    }


    /* Compose the varbind(name+value) for snmpTrapId.0 */
    var_trap_elem = snmpsubagt_eda_snmpTrapId_vb_populate(oid_db, 
                                                    trap_header->i_trap_tbl_id, 
                                                    trap_header->i_trap_id);
    if (var_trap_elem == NULL)
    {

        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TRAPID_VB_POP_FAIL, trap_header->i_trap_tbl_id,
                                                              trap_header->i_trap_id, 0);

        /* free the trap_varbind list */
        ncs_trap_eda_trap_varbinds_free(trap_varbinds);
        return NCSCC_RC_FAILURE;
    }

    /* Append to the list of varbinds */
    status = snmpsubagt_eda_varbindlist_append(&var_trap, var_trap_elem);
    if (status == NCSCC_RC_FAILURE)
    {
        /* log the error */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_VAR_BIND_APPEND_FAIL,status,0,0);
        
        /* free the trap_varbind list */
        snmpsubagt_eda_varbinds_free(var_trap_elem);
        ncs_trap_eda_trap_varbinds_free(trap_varbinds);
        return NCSCC_RC_FAILURE; 
    }

    /* go to the next varbind */
    varbinds_count++;

    while (trap_varbind)
    {
        /* Decode the trap_table_id */
        oid_db_key.i_table_id = trap_varbind->i_tbl_id;
        
        /* Get the base oid based on the trap_table_id */
        oid_db_node = (NCSSA_OID_DATABASE_NODE *)ncs_patricia_tree_get(oid_db,
                                                    (uns8*)&oid_db_key);
        if (oid_db_node == NULL)
        {
            /* log the error, table-id not found in the DB */
            m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_DB_NODE_NULL);

            snmpsubagt_eda_varbinds_free(var_trap);
            ncs_trap_eda_trap_varbinds_free(trap_varbinds);
            return NCSCC_RC_FAILURE; 
        }

        /* Compose the varbind list */
        var_trap_elem = snmpsubagt_eda_varbind_compose(oid_db_node, 
                                                trap_varbind);
        if (var_trap_elem == NULL)
        {
            /* there is an error */
            m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_TRAP_VAR_ELEM_NULL);

            snmpsubagt_eda_varbinds_free(var_trap);
            ncs_trap_eda_trap_varbinds_free(trap_varbinds);
            return NCSCC_RC_FAILURE;
        }

        /* Append to the list of varbinds */
        status = snmpsubagt_eda_varbindlist_append(&var_trap, var_trap_elem);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_TRAP_VARBIND_LIST_APPEND_FAIL, status, 0,0);
            
            snmpsubagt_eda_varbinds_free(var_trap);
            snmpsubagt_eda_varbinds_free(var_trap_elem);
            ncs_trap_eda_trap_varbinds_free(trap_varbinds);
            return NCSCC_RC_FAILURE; 
        }

        /* get to the next trap varbind */
        varbinds_count++;
        trap_varbind = trap_varbind->next_trap_varbind;
    }/* while */

    /* send the trap to the Agentx Agent */
    /*
     * A net-snmp supplied api to send a trap to the Agent is being used
     * here.  This routine does not reurn any value. 
     */

    /* get the control block */
    cb = m_SNMPSUBAGT_CB_GET;
    if (cb->haCstate == SNMPSUBAGT_HA_STATE_ACTIVE)
       send_v2trap(var_trap); 

    /* Free the varbind list */
    status = snmpsubagt_eda_varbinds_free(var_trap);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_EDA_VARBINDS_FREE_FAIL);
    }

    /* Free the trap varbinds list */
    status = ncs_trap_eda_trap_varbinds_free(trap_varbinds);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_EDA_TRAP_VARBINDS_FREE_FAIL);
    }
    return status; 
}
        
/******************************************************************************
 *  Name:          snmpsubagt_eda_varbind_compose
 *
 *  Description:   To compose the Varbind 
 * 
 *  Arguments:     i_oid_db_node - OID Database node
 *                 i_trap_varbind - Trap Varbind
 *
 *  Returns:       netsnmp_variable_list* - everything is OK
 *                 NULL   -  failure
 *  NOTE: 
 *****************************************************************************/
static netsnmp_variable_list*
snmpsubagt_eda_varbind_compose(NCSSA_OID_DATABASE_NODE  *i_oid_db_node, 
                               NCS_TRAP_VARBIND       *i_trap_varbind)
{
    uns32                   status = NCSCC_RC_SUCCESS;
    netsnmp_variable_list   *var_bind = NULL;
    int32                    param_location = -1; 
    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_VAR_BIND_COMPOSE);

    /* validate the inputs */
    if (i_oid_db_node == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_DB_NODE_NULL);
        return NULL; 
    }
    if (i_trap_varbind == NULL)
    {

        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_TRAP_VARBIND_NULL);
        return NULL; 
    }

    /* Firstly, find out whether object-id belongs to this table */
    param_location = subagt_eda_object_get(i_oid_db_node->objects_details, 
                                        i_oid_db_node->number_of_objects,
                                        i_trap_varbind->i_param_val.i_param_id);
    if (param_location == -1 )
    {
        /* log that an invalid param id is received, hence 
         * not able to get the object details 
         */
        m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_EDA_OBJ_GET_FAIL, param_location, 0,0);
        return NULL; 
    }


    /* allocate the memory for the varbind */
    /* I can not use our memory mgmt macros for the netsnmp library structs */
    var_bind = (netsnmp_variable_list*)malloc(sizeof(netsnmp_variable_list));
    if (var_bind == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_VAR_BIND_ALLOC_FAIL);
        return NULL;
    }
    m_NCS_MEMSET(var_bind, 0, sizeof(netsnmp_variable_list));

    /* get the complete OID */
    var_bind->name = snmpsubagt_eda_full_oid_populate(i_oid_db_node->base_oid, 
                                        i_oid_db_node->base_oid_len, 
                                        i_trap_varbind->i_param_val.i_param_id,
                                        (uns32 *)i_trap_varbind->i_idx.i_inst_ids,
                                        i_trap_varbind->i_idx.i_inst_len,
                                        &var_bind->name_length);
    if (var_bind->name == NULL)
    {
        /* log the error */
        free(var_bind); 
        return NULL; 
    }
    
    /* Now compose the value part of the varbind */
    /* assign the value and type */
    status = snmpsubagt_eda_trap_varbind_value_populate(var_bind, 
                                           &i_oid_db_node->objects_details[param_location],
                                           &i_trap_varbind->i_param_val);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */
        free(var_bind->name); 
        free(var_bind);
        var_bind = NULL; 
    }
    return var_bind; 
}

/******************************************************************************
 *  Name:          snmpsubagt_eda_trap_varbind_value_populate
 *
 *  Description:   To populate the value in the varbind
 * 
 *  Arguments:     var_bind     - Where the value needs to be filled in 
 *                 struct variable *    - object details of this table
 *                 i_param_val  - Value to be filled in var_bind
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
static uns32 
snmpsubagt_eda_trap_varbind_value_populate(netsnmp_variable_list *var_bind,
                             struct variable  *object_details,
                             NCSMIB_PARAM_VAL *i_param_val)
{
    uns32   status = NCSCC_RC_SUCCESS; 

   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_TRAP_VB_POP);
    /* validate the inputs */
    if (var_bind == NULL) 
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_TRAP_VARBIND_NULL);
        return NCSCC_RC_FAILURE; 
    }
    if (i_param_val == NULL) 
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_MIB_PARAM_VAL_NULL);
        return NCSCC_RC_FAILURE; 
    }

    if (object_details == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OBJ_DETAILS_NULL);
        return NCSCC_RC_FAILURE; 
    }  

    /* assign the type */
    var_bind->type = object_details->type; 
    
    /* Now compose the value */
    switch (var_bind->type)
    {
        case ASN_INTEGER:
        case ASN_COUNTER:
        case ASN_GAUGE:
        case ASN_TIMETICKS:
        var_bind->val_len = (size_t)sizeof(uns32);
        var_bind->val.integer = (long*)&i_param_val->info.i_int;
        break; 
        
        case ASN_BIT_STR:
        case ASN_OCTET_STR:
        case ASN_IPADDRESS:
        case ASN_OPAQUE:
        var_bind->val.string = (u_char*)i_param_val->info.i_oct;
        var_bind->val_len = (size_t)i_param_val->i_length;
        break; 

        case ASN_OBJECT_ID:
        {
            uns8    *temp; 
            uns32   *p_int; 
            uns32   i; 

            temp = (uns8 *)i_param_val->info.i_oct; 
            p_int = (uns32*)i_param_val->info.i_oct; 

            /* put in the host order */ 
            for (i = 0; i<(i_param_val->i_length/4); i++)
                p_int[i] = ncs_decode_32bit(&temp);

            var_bind->val_len = i_param_val->i_length;
            var_bind->val.objid = (oid *)i_param_val->info.i_oct;
        }
        break; 

        case ASN_COUNTER64:
        {
            long long llong = 0; 
            struct counter64    lc64; 

            if (i_param_val->i_length != sizeof(struct counter64))
                return NCSCC_RC_FAILURE; 
                
            m_NCS_MEMSET(&lc64, 0, sizeof (struct counter64)); 
            
            llong = m_NCS_OS_NTOHLL_P(i_param_val->info.i_oct);
            lc64.low = (u_long)(llong)&(0xFFFFFFFF);
            lc64.high = (u_long)(llong>>32)&(0xFFFFFFFF);
            memcpy((uns8*)&lc64, i_param_val->info.i_oct, sizeof(struct counter64));

            var_bind->val_len = (size_t)i_param_val->i_length;
            var_bind->val.counter64 = (struct counter64 *)i_param_val->info.i_oct; 
        }
        break; 

        default:
        status = NCSCC_RC_FAILURE;
        break; 
    }
    return status; 
}

/******************************************************************************
 *  Name:          snmpsubagt_eda_full_oid_populate
 *
 *  Description:   To Compose the complete OID
 * 
 *  Arguments:     base_oid   -  Base OID 
 *                 base_oid_len - base OID Length 
 *                  i_param_id   - Param ID
 *                  i_instids    - Instance Ids
 *                  i_inst_len   - Instance ID length
 *                  io_name_length - Complete OID length (output)
 *
 *  Returns:       oid* - Pointer to the complete OID
 *                 NULL   -  failure
 *  NOTE: 
 *****************************************************************************/
static oid*
snmpsubagt_eda_full_oid_populate(oid    *base_oid, 
                                 uns32  base_oid_len, 
                                 NCSMIB_PARAM_ID  i_param_id,
                                 uns32  *i_inst_ids,
                                 uns32  i_inst_len,
                                 size_t *io_name_length)
{
    uns32   i = 0;
    uns32   *src_oid_uns32 = NULL;
    oid     *new_oid = NULL;
    oid     *dest_oid = NULL;
    oid     *src_oid = NULL;
    size_t  subid_num = 0;      

   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_FULL_OID_POP);
    /* allocate the memory to hold the Complete OID */
    /* base_oid.param_id.instance */
    subid_num = (size_t)base_oid_len + 1 + ((i_inst_len==0)?1:i_inst_len);
                
    new_oid = (oid*)malloc(subid_num * sizeof(oid));
    if (new_oid == NULL)
    {
        /* log the error */ 
      
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_OID_ALLOC_FAIL);
        return NULL; 
    }

    memset(new_oid, 0, subid_num * sizeof(oid));

    /* Now, prepare the OID */

    if (base_oid != NULL)
    {
        /* base oid */
        dest_oid = new_oid;
        src_oid =  base_oid; 
        for (i = 0; i<base_oid_len; i++)
        {
            *dest_oid++ = *src_oid++;
        }
    }

    /* param-id */
    *dest_oid++ = i_param_id;

    /* instance */
    if (i_inst_ids != NULL)
    {
        src_oid_uns32 = i_inst_ids;
        for (i=0; i<i_inst_len; i++)
        {
            *dest_oid = (oid)(*src_oid_uns32);
            dest_oid++;
            src_oid_uns32++;
        }
    }

    /* update the number of sub-identifiers */
    *io_name_length = subid_num;

    /* completed composing the Complete OID */
    return new_oid;
}

/******************************************************************************
 *  Name:          snmpsubagt_eda_snmpTrapId_vb_populate
 *
 *  Description:   To generate the Varbind for the snmpTrapId  
 *                  This Varind is a bit differnt because, the value
 *                  of this object is an Object Identifier.  So only a 
 *                  new routine is being used for this. 
 * 
 *  Arguments:     NCS_PATRICIA_TREE* - OID Data Base 
 *                 NCS_TRAP_VARBIND   - Trap varbind from the decoded buffer
 *
 *  Returns:       netsnmp_varible_list - new varbind
 *                 NULL                 -  failure
 *  NOTE:        
 *****************************************************************************/
static netsnmp_variable_list*
snmpsubagt_eda_snmpTrapId_vb_populate(NCS_PATRICIA_TREE    *i_oid_db, 
                                       uns32                i_trap_tbl_id,
                                       uns32                i_trap_id)
{
    netsnmp_variable_list    *var_bind = NULL;
    /* snmpTrapId.0 */
    oid             objid_snmptrap[] = { SNMP_OID_SNMPMODULES, 1, 1, 4, 1, 0 };
    NCSSA_OID_DATABASE_KEY    oid_db_key; 
    NCSSA_OID_DATABASE_NODE   *oid_db_node = NULL; 


   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_SNMP_TRAPID_VB_POP);
    /* validate the inputs */
    if ((i_oid_db == NULL)||(i_trap_id == 0)) /* table_id can be zero as per ncs_mtbl.h */
    {
        /* log the error */
        return NULL;
    }

    m_NCS_MEMSET(&oid_db_key, 0, sizeof(NCSSA_OID_DATABASE_KEY));

    /* trap_table_id */
    oid_db_key.i_table_id = i_trap_tbl_id;
        
    /* Get the base oid based on the trap_table_id */
    oid_db_node = (NCSSA_OID_DATABASE_NODE *)ncs_patricia_tree_get(i_oid_db,
                                                    (uns8*)&oid_db_key);
    if (oid_db_node == NULL)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_DB_NODE_NULL);
        return NULL; 
    }

    /* allocate the memory for the varbind */
    var_bind = (netsnmp_variable_list*)malloc(sizeof(netsnmp_variable_list));
    if (var_bind == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_VAR_BIND_ALLOC_FAIL);
        return NULL; 
    }
    m_NCS_MEMSET(var_bind, 0, sizeof(netsnmp_variable_list));

    /* allocate the memory for the snmpTrapOID.0 */
    var_bind->name_length = sizeof(objid_snmptrap)/sizeof(oid);
    var_bind->name = (oid*)malloc(sizeof(objid_snmptrap));
    if (var_bind->name == NULL)
    {
        /* log the error */
        /* free the var_bind */
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_VAR_BIND_NAME_ALLOC_FAIL);
        free(var_bind);  
        return NULL; 
    }

    /* get the complete oid */
    m_NCS_OS_MEMCPY(var_bind->name, 
                    objid_snmptrap, 
                    (var_bind->name_length)*sizeof(oid));

    /* assign the value */
    var_bind->type = ASN_OBJECT_ID; 
    var_bind->val.objid = snmpsubagt_eda_full_oid_populate(
                                        oid_db_node->base_oid, 
                                        oid_db_node->base_oid_len, 
                                        i_trap_id,
                                        0,0, &var_bind->val_len);
    if (var_bind->val.objid == NULL)
    {
        /* free the memory */ 
        free(var_bind->name); 
        free(var_bind); 
        return NULL; 
    }
    /* Discuss this with Kamesh -- TBD Mahesh */ 
    /* Following assignment must be there for all the objects of type OID. 
     * Length of the value (not the actual object) should be interms no.of bytes. 
     * I can not put this inside the above routine, as it will be used to 
     * compute the complete 
     */
    var_bind->val_len *= sizeof(oid);     
    
    return var_bind;
}

/******************************************************************************
 *  Name:          snmpsubagt_eda_varbindlist_append
 *
 *  Description:   To make the list of Varbinds to be sent in the TRAP PDU
 *                  This routine adds the new varbind at the end of the list
 * 
 *  Arguments:    netsnmp_variable_list ** - Header of the list
 *                netsnmp_variable_list *  - Varbind to be added 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
static uns32 
snmpsubagt_eda_varbindlist_append(netsnmp_variable_list **add_to_me, 
                                  netsnmp_variable_list *to_be_added)
{
    netsnmp_variable_list *temp = NULL; 
    
   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_VB_LIST_APPEND);
    /* validate the inputs */
    if ((add_to_me == NULL) || (to_be_added == NULL))
    {
        /* log the error */
        return NCSCC_RC_SUCCESS; 
    }

    if (*add_to_me == NULL)
    {
        /* add at the begining */
        *add_to_me = to_be_added; 
        return NCSCC_RC_SUCCESS; 
    }

    temp = *add_to_me; 
    while (temp->next_variable)
        temp = temp->next_variable; 

    /* add at the end */
    temp->next_variable = to_be_added; 

    return NCSCC_RC_SUCCESS; 
}
    
/******************************************************************************
 *  Name:          snmpsubagt_eda_varbinds_free
 *
 *  Description:   To handle the state transition to Out Of Service
 * 
 *  Arguments:     netsnmp_variable_list * - List of varbinds to be freed
 *                  - Free the OID 
 *                  - Free the value -- Need to be enhanced, once MAB
 *                      supports the ASN_OBJECT_ID type. 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
static uns32 
snmpsubagt_eda_varbinds_free(netsnmp_variable_list *to_be_freed)
{
    netsnmp_variable_list   *prev, *current; 

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_VB_FREE);
    
    if (to_be_freed == NULL)
    {
        return NCSCC_RC_SUCCESS; 
    }
    
    prev = current = to_be_freed; 
    free (prev->val.objid); /* Added by suresh */
    while(current)
    {
        /* free the oid */
        prev = current; 
        if (prev->name) 
           free(prev->name);
        current = current->next_variable;
        free(prev);
    }
    return NCSCC_RC_SUCCESS; 
}

#if 0
/******************************************************************************
 *  Name:          snmpsubagt_eda_trap_varbinds_free
 *
 *  Description:   To free the trap varbinds recieved from the event 
 *                  buffer. 
 *                      - Free the memory allocated by the decode routines
 *                      - Handle care with memory for 
 *                        the ASN_OBJECT_ID type
 *                          - Mahesh TBD, MAB Enhancement 
 * 
 *  Arguments:     NCS_TRAP_VARBIND* -List of varbinds to be freed 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   - failure
 *  NOTE: 
 *****************************************************************************/
uns32
snmpsubagt_eda_trap_varbinds_free(NCS_TRAP_VARBIND *trap_varbinds)
{
    NCS_TRAP_VARBIND     *prev, *current; 

   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_TRAP_VB_FREE);
    /* validate the inputs */
    if (trap_varbinds == NULL)
    {
        /* log the error, but nothing to free */
        return NCSCC_RC_SUCCESS; 
    }

    current = trap_varbinds; 

    while (current)
    {
        prev = current; 
       
        /* free the Octets */
        switch(prev->i_param_val.i_fmat_id)
        {
            case NCSMIB_FMAT_INT:
            /* Nothing to do */
            break; 
            case NCSMIB_FMAT_OCT:
            if (prev->i_param_val.info.i_oct != NULL)
            {
                m_NCS_MEM_FREE(prev->i_param_val.info.i_oct,
                               NCS_MEM_REGION_PERSISTENT,
                               NCS_SERVICE_ID_COMMON,2);
            }
            break; 
            default:
            /* log the error */
            break; 
        }

        if (prev->i_idx.i_inst_ids != NULL)
        {
            /* free the index */
            m_NCS_MEM_FREE(prev->i_idx.i_inst_ids,
                           NCS_MEM_REGION_PERSISTENT,
                           NCS_SERVICE_ID_COMMON,4);
        }

        /* go to the next node */
        current = current->next_trap_varbind; 
        
        /* free the varbind now */
        m_MMGR_NCS_TRAP_VARBIND_FREE(prev);
    } /* while */
    return NCSCC_RC_SUCCESS; 
}
#endif

/******************************************************************************
 *  Name:          subagt_eda_object_get
 *
 *  Description:   To find the location of the object in the list
 *                 variables generated by SMIDUMP
 * 
 *  Arguments:     objct_details    - pointer to the list of objects
 *                 number_of_objects - Number of objects in this group
 *                 i_param_id       - Object which needs to be located
 *
 *  Returns:       -1 - There is no such object in the list
 *                 param_location   -  if found in the list
 *  NOTE: 
 *****************************************************************************/
static int32
subagt_eda_object_get(struct variable   *object_details, 
                      uns32             number_of_objects,
                      NCSMIB_PARAM_ID   i_param_id)
{
    int32   param_location = -1;     
    uns32   i = 0;

   m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_EDA_OBJ_GET);
    
    if (object_details == NULL)
    {
        return param_location; 
    }

    if (number_of_objects == 0)
    {
        return param_location; 
    }

    /* search the list */
    for (i = 0; i < number_of_objects; i++)
    {
        if (i_param_id == object_details[i].magic)
        {
            param_location = i; 
            break; 
        }
    }

    return param_location; 
}



/******************************************************************************
 *  Name:          subagt_log_ncs_trap_data
 *
 *  Description:    Helper function to LOG NCS TRAP DATA 
 * 
 *  Arguments:      NCS_TRAP 
 *                
 *
 *  Returns:       NONE 
 ******************************************************************************/

static void subagt_log_ncs_trap_data(NCS_TRAP *ncs_trap_data)
{ 
  NCSFL_MEM                  log_trap_data;
  SNMPSA_LOG_TRAP_TBL        log_trap_tbl; 
  NCS_TRAP_VARBIND           *trap_vb; 

  if (ncs_trap_data == NULL)
  {
      /* there is nothing to log */
      return; 
  }

  m_NCS_MEMSET(&log_trap_data, 0, sizeof(NCSFL_MEM));
  m_NCS_MEMSET(&log_trap_tbl,  0, sizeof(SNMPSA_LOG_TRAP_TBL));

  /* LOG the TRAP TBL information */ 
  log_trap_tbl.tbl_id   =  (uns32)ncs_trap_data->i_trap_tbl_id; 
  log_trap_tbl.param_id =  (uns32)ncs_trap_data->i_trap_id; 
  log_trap_tbl.inform   =  (uns32) ncs_trap_data->i_inform_mgr;
   
  log_trap_data.len= sizeof(SNMPSA_LOG_TRAP_TBL); 
  log_trap_data.addr = log_trap_data.dump = (char *)&log_trap_tbl;
  m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSTRAP_TBL_DUMP, log_trap_data); 

  /* LOG the TRAP VB information */ 
  if((trap_vb = ncs_trap_data->i_trap_vb) != NULL) 
   { 
     while(trap_vb != NULL) 
       { 
           /* LOG the TRAP VARBIND data */ 
            subagt_log_ncs_trap_vb(trap_vb); 
            trap_vb = trap_vb->next_trap_varbind;
        }
  }

} 

/******************************************************************************
 *  Name:          subagt_log_ncs_trap_vb
 *
 *  Description:    Helper function to LOG NCS TRAP VAR BIND DATA 
 * 
 *  Arguments:      NCS_TRAP_VARBIND 
 *                
 *
 *  Returns:       NONE 
 ******************************************************************************/

static void subagt_log_ncs_trap_vb(NCS_TRAP_VARBIND *trap_vb)
{

  NCSFL_MEM                     log_trap_data;
  SNMPSA_LOG_TRAP_PARAM_INFO    param_info; 
   
  m_NCS_MEMSET(&log_trap_data, 0, sizeof(NCSFL_MEM));
  m_NCS_MEMSET(&param_info, 0, sizeof(SNMPSA_LOG_TRAP_PARAM_INFO));

  if (trap_vb == NULL)
  {
      /* nothing to dump */
      return;
  }
 
 /* LOG Varbind table Id */ 
   
  log_trap_data.len= sizeof(uns32); 
  log_trap_data.addr = log_trap_data.dump = (char *)&trap_vb->i_tbl_id;
  m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSTRAP_VB_TBL_DUMP, log_trap_data);
  
  /* LOG the Instance IDs */ 
  if (trap_vb->i_idx.i_inst_len != 0)
  {
      m_NCS_MEMSET(&log_trap_data, 0, sizeof(NCSFL_MEM));
      log_trap_data.len    = (trap_vb->i_idx.i_inst_len)*sizeof(uns32);
      log_trap_data.dump   = log_trap_data.addr = (char *)trap_vb->i_idx.i_inst_ids;
      m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_INSTID_DUMP, log_trap_data);
  }

  /* LOG the Paramater Information  */ 

  m_NCS_MEMSET(&log_trap_data, 0, sizeof(NCSFL_MEM));
  param_info.param_id = trap_vb->i_param_val.i_param_id; 
  param_info.fmt_id   = trap_vb->i_param_val.i_fmat_id; 
  param_info.i_len    = trap_vb->i_param_val.i_length; 

  log_trap_data.len = sizeof(SNMPSA_LOG_TRAP_PARAM_INFO);
  log_trap_data.dump = log_trap_data.addr = (char *)&param_info;
  m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_PARAM_INFO_DUMP,
                           log_trap_data);

  /* LOG the INT Parameter value */
  m_NCS_MEMSET(&log_trap_data, 0, sizeof(NCSFL_MEM));
  if (trap_vb->i_param_val.i_fmat_id == NCSMIB_FMAT_INT)
      {
        log_trap_data.len = sizeof(uns32);
        log_trap_data.dump = log_trap_data.addr = (char *)&trap_vb->i_param_val.info.i_int;
        m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_INT_INFO_DUMP,
                                 log_trap_data);
      }

 /* LOG the OCTET Parameter value */
  m_NCS_MEMSET(&log_trap_data, 0, sizeof(NCSFL_MEM));
  if (trap_vb->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
      {
        log_trap_data.len = trap_vb->i_param_val.i_length;
        log_trap_data.dump = log_trap_data.addr = (char *)trap_vb->i_param_val.info.i_oct;
        m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_OCT_INFO_DUMP, log_trap_data);
      }

} 
    
