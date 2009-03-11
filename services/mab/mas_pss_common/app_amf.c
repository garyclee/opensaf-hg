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
  
  DESCRIPTION: This file describes AMF Interface implementation common to 
               MAS and PSS
*****************************************************************************/ 

#include "app_amf.h"
#include "dta_papi.h"
#include "mab_log.h"
#include "rda_papi.h"

void ncs_app_amf_init_timer_cb (void* amf_init_timer);

/*****************************************************************************\
 *  Name:          ncs_app_signal_install                                     * 
 *                                                                            *
 *  Description:   to install a singnal handler for a given signal            *
 *                                                                            *
 *  Arguments:     int i_sig_num - for which signal?                          *
 *                 SIG_HANDLR    - Handler to hanlde the signal               *
 *                                                                            * 
 *  Returns:       0  - everything is OK                                      *
 *                 -1 - failure                                               *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* install the signal handler */ 
int32
ncs_app_signal_install(int i_sig_num, SIG_HANDLR i_sig_handler)
{
    struct sigaction sig_act; 

    if ((i_sig_num <= 0) ||
        (i_sig_num > 32) || /* not the real time signals */ 
        (i_sig_handler == NULL))
        return -1; 

    /* now say that, we are interested in this signal */ 
    m_NCS_MEMSET(&sig_act, 0, sizeof(struct sigaction)); 
    sig_act.sa_handler = i_sig_handler; 
    return sigaction(i_sig_num, &sig_act, NULL); 

}

/*****************************************************************************\
 *  Name:          ncs_app_amf_invalid_state_process                          * 
 *                                                                            *
 *  Description:   Junk CSI state transition                                  *
 *                                                                            *
 *  Arguments:     void              *data      - List of CSIs                * 
 *                 struct ncs_app_amf_attribs   *amf_attribs - MAS attribs    *
 *                 SaAmfCSIDescriptorT   csiDescriptor - CSI details          *
 *                 SaInvocationT         invocation - For AMF ro correlate    *
 *                                                                            * 
 *  Returns:       NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* generic invalid state transition handler */     
uns32 
ncs_app_amf_invalid_state_process(void *data, PW_ENV_ID  envid, 
                                  SaAmfCSIDescriptorT csiDesc)
{
    return NCSCC_RC_FAILURE;
}

/*****************************************************************************\
 *  Name:          ncs_app_amf_attribs_init                                   * 
 *                                                                            *
 *  Description:   To initialize the AMF attributes                           *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_ATTRIBS            *attribs - AMF attributes(o)*  
 *                 int8                           *health_key - HLTH Key      *
 *                 SaDispatchFlagsT               dispatch_flags              *  
 *                 SaAmfHealthcheckInvocationT    hc_inv_type                 *  
 *                 SaAmfRecommendedRecoveryT      recovery                    *  
 *                 NCS_APP_AMF_HA_STATE_HANDLERS  *state_handler              * 
 *                 SaAmfCallbacksT                *amfCallbacks               *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* to initialize the AMF attributes of an application */ 
uns32
ncs_app_amf_attribs_init(NCS_APP_AMF_ATTRIBS            *attribs, 
                         int8                           *health_key, 
                         SaDispatchFlagsT               dispatch_flags, 
                         SaAmfHealthcheckInvocationT    hc_inv_type, 
                         SaAmfRecommendedRecoveryT      recovery, 
                         NCS_APP_AMF_HA_STATE_HANDLERS  *state_handler, 
                         SaAmfCallbacksT                *amfCallbacks)
{
    NCS_APP_AMF_HA_STATE i, j; 
    
    if (attribs == NULL)
        return NCSCC_RC_FAILURE; 

    if (health_key == NULL)
        return NCSCC_RC_FAILURE; 

    if (amfCallbacks == NULL)
        return NCSCC_RC_FAILURE; 

    /* initialize the SAF version details */
    attribs->safVersion.releaseCode = (char) 'B';
    attribs->safVersion.majorVersion = 0x01;
    attribs->safVersion.minorVersion = 0x01;

    attribs->dispatchFlags = dispatch_flags; 
    attribs->recommendedRecovery = recovery; 
    
    /* Fill in the Health check key */
    m_NCS_STRCPY(attribs->healthCheckKey.key,health_key);
    attribs->healthCheckKey.keyLen=m_NCS_STRLEN(attribs->healthCheckKey.key);
    attribs->invocationType = hc_inv_type; 

    /* some applications may do AMF interface for the purpose of health check */ 
    if (state_handler != NULL)
    {
        /* initialize the state machine */ 
        for (i=NCS_APP_AMF_HA_STATE_NULL; i<NCS_APP_AMF_HA_STATE_MAX; i++)
        {
            for (j=NCS_APP_AMF_HA_STATE_NULL; j<NCS_APP_AMF_HA_STATE_MAX; j++)
                attribs->ncs_app_amf_hastate_dispatch[i][j] = 
                                            state_handler->invalidTrans; 
        }

        /* go to HA state from NULL */  
        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_NULL]
                [NCS_APP_AMF_HA_STATE_ACTIVE] = state_handler->initActive; 

        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_NULL]
                [NCS_APP_AMF_HA_STATE_STANDBY] = state_handler->initStandby; 

        /* from ACTIVE */ 
        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_ACTIVE]
                [NCS_APP_AMF_HA_STATE_QUISCED] = state_handler->quiescedTrans; 

        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_ACTIVE]
                [NCS_APP_AMF_HA_STATE_QUISCING] = state_handler->quiescingTrans; 
        
        /* from STANDBY */
        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_STANDBY]
                [NCS_APP_AMF_HA_STATE_ACTIVE] = state_handler->activeTrans; 

        /* from QUIESCED */ 
        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_QUISCED]
                [NCS_APP_AMF_HA_STATE_ACTIVE] = state_handler->activeTrans; 
        
        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_QUISCED]
                [NCS_APP_AMF_HA_STATE_STANDBY] = state_handler->standbyTrans; 

        /* from QUIESCING */ 
        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_QUISCING]
                [NCS_APP_AMF_HA_STATE_ACTIVE] = state_handler->activeTrans; 
        
        attribs->ncs_app_amf_hastate_dispatch[NCS_APP_AMF_HA_STATE_QUISCING]
                [NCS_APP_AMF_HA_STATE_QUISCED] = state_handler->quiescingToQuiesced; 
    }

    /* update the AMF callbacks */ 
    memcpy(&attribs->amfCallbacks, 
                    amfCallbacks, 
                    sizeof(SaAmfCallbacksT)); 

    /* Set the time period to retry amf interface initialisation */
    attribs->amf_init_timer.period = m_NCS_AMF_INIT_TIMEOUT_IN_SEC * SYSF_TMR_TICKS;
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
 *  Name:          ncs_app_amf_initialize                                     * 
 *                                                                            *
 *  Description:   To initialize with AMF                                     *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_ATTRIBS *amf_attribs  - AMF attributes         * 
 *                                                                            * 
 *  Returns:       SaAisErrorT                                                *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* routine to intialize the AMF */
SaAisErrorT
ncs_app_amf_initialize(NCS_APP_AMF_ATTRIBS *amf_attribs)
{
    SaAisErrorT status = SA_AIS_OK; 
 
    /* Initialize the AMF Library */
    status = saAmfInitialize(&amf_attribs->amfHandle, /* AMF Handle */
                    &amf_attribs->amfCallbacks, /* AMF Callbacks */ 
                    &amf_attribs->safVersion /*Version of the AMF*/);
    if (status != SA_AIS_OK) 
    { 
        /* log the error */ 
        return status; 
    }
    
    /* get the communication Handle */
    status = saAmfSelectionObjectGet(amf_attribs->amfHandle, 
                                &amf_attribs->amfSelectionObject); 
    if (status != SA_AIS_OK)
    {
       /* log the error */ 
       saAmfFinalize(amf_attribs->amfHandle);
       amf_attribs->amfHandle = 0;
       return status; 
    }

    /* get the component name */
    status = saAmfComponentNameGet(amf_attribs->amfHandle,  /* input */
                              &amf_attribs->compName); /* output */
    if (status != SA_AIS_OK) 
    { 
        /* log the error */ 
        saAmfFinalize(amf_attribs->amfHandle);
        amf_attribs->amfHandle = 0;
        m_NCS_MEMSET(&amf_attribs->amfSelectionObject, 0, sizeof(SaSelectionObjectT));

        return status;
    }

    /* register the component with the AMF */ 
    status = saAmfComponentRegister(amf_attribs->amfHandle, 
                                    &amf_attribs->compName, /* output */
                                    NULL); 
    if (status != SA_AIS_OK)
    {
        /* log the error */ 
        saAmfFinalize(amf_attribs->amfHandle);
        amf_attribs->amfHandle = 0;
        m_NCS_MEMSET(&amf_attribs->amfSelectionObject, 0, sizeof(SaSelectionObjectT));
        m_NCS_MEMSET(&amf_attribs->compName, 0, sizeof(SaNameT));
        return status; 
    }

    /* start health check */
    status = saAmfHealthcheckStart(amf_attribs->amfHandle,
                                   &amf_attribs->compName,
                                   &amf_attribs->healthCheckKey,
                                   amf_attribs->invocationType,
                                   amf_attribs->recommendedRecovery);

    if (status != SA_AIS_OK)
    {
        /* log the error */ 
        /* Unregister with AMF */ 
        saAmfComponentUnregister(amf_attribs->amfHandle, 
                                 &amf_attribs->compName, 
                                 NULL); 
        saAmfFinalize(amf_attribs->amfHandle);
        amf_attribs->amfHandle = 0;
        m_NCS_MEMSET(&amf_attribs->amfSelectionObject, 0, sizeof(SaSelectionObjectT));
        m_NCS_MEMSET(&amf_attribs->compName, 0, sizeof(SaNameT));
        return status; 
    }

    return status; 
}

/*****************************************************************************\
 *  Name:          ncs_app_amf_finalize                                       * 
 *                                                                            *
 *  Description:   To finalize with AMF                                       *
 *                  * stops the health check                                  *
 *                  * finalize interface with AMF                             *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_ATTRIBS *amf_attribs  - AMF attributes         * 
 *                                                                            * 
 *  Returns:       SaAisErrorT                                                *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Unregistration and Finalization with AMF Library */
SaAisErrorT
ncs_app_amf_finalize(NCS_APP_AMF_ATTRIBS *amf_attribs)
{
    SaAisErrorT status = SA_AIS_OK; 

    /* Destroy the amf initialisation timer if it was created */
    if(amf_attribs->amf_init_timer.timer_id != TMR_T_NULL)
    {
       m_NCS_TMR_DESTROY(amf_attribs->amf_init_timer.timer_id);
       amf_attribs->amf_init_timer.timer_id = TMR_T_NULL;
    }

    /* delete the fd from the select list */ 
    m_NCS_MEMSET(&amf_attribs->amfSelectionObject, 0, 
                    sizeof(SaSelectionObjectT));

    /* Disable the health monitoring */ 
    status = saAmfHealthcheckStop(amf_attribs->amfHandle, 
                                &amf_attribs->compName, 
                                &amf_attribs->healthCheckKey); 
    if (status != SA_AIS_OK)
    {
        /* continue finalization */
    }

    /* Unregister with AMF */ 
    status = saAmfComponentUnregister(amf_attribs->amfHandle, 
                                    &amf_attribs->compName, 
                                    NULL); 
    if (status != SA_AIS_OK)
    {
        /* continue finalization */
    } 
    
    /* Finalize */ 
    status = saAmfFinalize(amf_attribs->amfHandle); 
    return status; 
}

/*****************************************************************************\
 *  Name:          ncs_app_amf_init_start_timer                               * 
 *                                                                            *
 *  Description:   Starts the timer to retry amf interface initialisation     *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_INIT_TIMER *amf_init_timer                     * 
 *                 timer_id is updated as output parameter in amf_init_timer  *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS on success                                *
 *                 NCSCC_RC_FAILURE on failure                                *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32 ncs_app_amf_init_start_timer(NCS_APP_AMF_INIT_TIMER* amf_init_timer)
{

   /* Create the timer if it is not yet created */
   if (amf_init_timer->timer_id == TMR_T_NULL)
   {
      /* create the timer. */
      m_NCS_TMR_CREATE(amf_init_timer->timer_id,
                       amf_init_timer->period,
                       ncs_app_amf_init_timer_cb,
                       (void*)(amf_init_timer));
   }

   /* start the timer. */
   m_NCS_TMR_START(amf_init_timer->timer_id,
                   amf_init_timer->period,
                   ncs_app_amf_init_timer_cb,
                   (void*)(amf_init_timer));

   /* If timer is not started, return failure. */
   if(amf_init_timer->timer_id == TMR_T_NULL)
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
 *  Name:          ncs_app_amf_init_timer_cb                                  * 
 *                                                                            *
 *  Description:   Timer callback function - posts message as specified in    *
 *                 msg_type and mbx of timer_data                             *
 *                                                                            *
 *  Arguments:     void* timer_data                                           * 
 *                                                                            * 
 *  Returns:       void                                                       *
 *  NOTE:                                                                     * 
\*****************************************************************************/
void ncs_app_amf_init_timer_cb (void* timer_data)
{
   MAB_MSG* post_me = NULL;
   uns32 status = NCSCC_RC_SUCCESS;
   NCS_APP_AMF_INIT_TIMER* amf_init_timer = (NCS_APP_AMF_INIT_TIMER*)timer_data;

   /* Stop the timer. */
   m_NCS_TMR_STOP(amf_init_timer->timer_id);

   /* post message to mailbox as specified in amf_init_timer */
   post_me = m_MMGR_ALLOC_MAB_MSG;
   if(post_me == NULL)
   {
      /* No memory available. Log the error. */
      /* m_MAB_DBG_SINK(NCSCC_RC_OUT_OF_MEM);*/
      return;
   }

   m_NCS_MEMSET(post_me, 0, sizeof(MAB_MSG));
   post_me->op = amf_init_timer->msg_type;

   status = m_NCS_IPC_SEND(amf_init_timer->mbx,
                           (NCS_IPC_MSG*)post_me,
                           NCS_IPC_PRIORITY_HIGH);
   if (status != NCSCC_RC_SUCCESS)
   {
      /*m_MAB_DBG_SINK(status);*/
      return;
   }

   return;
}

/****************************************************************************\
*  Name:          app_rda_init_role_get                                      * 
*                                                                            *
*  Description:   Gets the initial role from RDA                             *
*                                                                            *
*  Arguments:     *o_init_role - Store the initial role                      * 
*                                                                            * 
*  Returns:       NCSCC_RC_FAILURE - RDA interface failure, a JUNK state is  *
*                                    is returned.                            *
*                 NCSCC_RC_SUCCESS - initial role is available in            *
*                                    '*o_init_role'                          *                      
\****************************************************************************/
uns32
app_rda_init_role_get(NCS_APP_AMF_HA_STATE *o_init_role) 
{
    uns32       rc; 
    uns32       status = NCSCC_RC_SUCCESS;
    PCS_RDA_REQ app_rda_req;

    /* validate the output location */ 
    if (o_init_role == NULL)
    {
        /* log that invalid output param location */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_ERR_RDA_INVALID_OP_LOC, 0);
        return NCSCC_RC_FAILURE; 
    }
    
    /* initialize the RDA Library */ 
    m_NCS_MEMSET(&app_rda_req, 0, sizeof(PCS_RDA_REQ)); 
    app_rda_req.req_type = PCS_RDA_LIB_INIT;
    rc = pcs_rda_request (&app_rda_req);
    if (rc  != PCSRDA_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_ERR_RDA_LIB_INIT_FAILED, rc);
        return NCSCC_RC_FAILURE; 
    }
    
    /* get the role */ 
    m_NCS_MEMSET(&app_rda_req, 0, sizeof(PCS_RDA_REQ)); 
    app_rda_req.req_type = PCS_RDA_GET_ROLE;
    rc = pcs_rda_request(&app_rda_req);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_ERR_RDA_API_GET_ROLE_FAILED, rc);

        /* set the error code to be returned */ 
        status = NCSCC_RC_FAILURE; 
        
        /* finalize */ 
        goto finalize; 
    }
    
    /* update the output parameter */ 
    if (app_rda_req.info.io_role == PCS_RDA_ACTIVE)
        *o_init_role = NCS_APP_AMF_HA_STATE_ACTIVE; 
    else if (app_rda_req.info.io_role == PCS_RDA_STANDBY)
        *o_init_role = NCS_APP_AMF_HA_STATE_STANDBY; 
    else
    {
        /* log the state returned by RDA */
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_ERR_RDA_INVALID_ROLE, app_rda_req.info.io_role);
        
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
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_ERR_RDA_LIB_FIN_FAILED, rc);
        
        return NCSCC_RC_FAILURE;
    }

    /* return the final status */ 
    return status; 
}
