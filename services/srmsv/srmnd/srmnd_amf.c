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

  MODULE NAME: SRMND_AMF.C
 
..............................................................................

  DESCRIPTION: This file contains SRMA CB specific routines

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/
#include "srmnd.h"

extern uns32 gl_srmnd_hdl;

/****************************************************************************
                       Static Function Prototypes
****************************************************************************/
static uns32 srmnd_invalid_state_process(SRMND_CB *srmnd, SaInvocationT invocation);
/* Go to ACTIVE State */ 
static uns32 srmnd_active_process(SRMND_CB  *srmnd, SaInvocationT invocation);
/* Go to QUIESCED State */
static uns32 srmnd_quiesced_process(SRMND_CB  *srmnd, SaInvocationT invocation);
/* Go to QUIESCING State */
static uns32 srmnd_quiescing_process(SRMND_CB  *srmnd, SaInvocationT  invocation);
/* Go to QUIESCED from QUIESCING State */
static uns32 srmnd_quiescing_quiesced_process(SRMND_CB *srmnd, SaInvocationT invocation);
/* Describes the health check of the SRMND */
static void srmnd_amf_health_check_callback(SaInvocationT invocation, 
                                            const SaNameT *comp_name, 
                                            SaAmfHealthcheckKeyT *check_key);
/* HA State Handling (Failover and Switchover) */
static void srmnd_amf_csi_set_callback(SaInvocationT invocation, 
                                       const SaNameT *comp_name, 
                                       SaAmfHAStateT ha_state, 
                                       SaAmfCSIDescriptorT csi_descriptor);
/* Component Terminate callback */
static void srmnd_amf_component_terminate_callback(SaInvocationT invocation, 
                                                   const SaNameT *comp_name);
/* Component Service Instance Remove Callback */
static void srmnd_amf_csi_remove_callback(SaInvocationT  invocation, 
                                          const SaNameT  *comp_name, 
                                          const SaNameT  *csi_name, 
                                          SaAmfCSIFlagsT csi_flags);

/****************************************************************************
                 Initialize the HA state machine 
****************************************************************************/
const srmnd_readiness_ha_state_process
      srmnd_readiness_ha_state_dispatch[SRMND_HA_STATE_MAX]
                                       [SRMND_HA_STATE_MAX]=
{
   /* current HA State NULL -- Initial state of the SRMND */
   {
      srmnd_invalid_state_process, /* should never be the case*/
      srmnd_active_process,        /* Go to ACTIVE State */ 
      srmnd_invalid_state_process, /* should never be the case*/
      srmnd_invalid_state_process, /* should never be the case*/
   },
   /* Current HA State ACTIVE */
   {
      srmnd_invalid_state_process, /* should never be the case*/
      srmnd_invalid_state_process, /* should never be the case*/
      srmnd_quiesced_process,      /* Go to QUIESCED State */
      srmnd_quiescing_process,     /* Go to QUIESCING State */
   },
   /* current HA State QUIESCED */
   {
      srmnd_invalid_state_process, /* should never be the case*/
      srmnd_active_process,        /* Goto ACTIVE state */
      srmnd_invalid_state_process, /* never be the case*/
      srmnd_invalid_state_process, /* never be the case*/
   },
   /* current HA State QUIESCING */
   {
      srmnd_invalid_state_process, /* never be the case*/
      srmnd_active_process,        /* Goto ACTIVE state */
      srmnd_quiescing_quiesced_process,/* Go to QUIESCED State */
      srmnd_invalid_state_process, /* never be the case*/
   }
};


/******************************************************************************
  Name          :  srmnd_amf_initialize

  Description   :  To intialize the session with AMF Agent
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block 

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure
                   NCSCC_RC_AMF_INIT_FAILURE - Init Library failure 
                   NCSCC_RC_AMF_COMP_NAME_GET_FAILURE 
                   NCSCC_RC_AMF_COMP_REG_FAILURE 
                   NCSCC_RC_AMF_SEL_OBJ_FAILURE
                   NCSCC_RC_AMF_HEALTH_CHECK_FAILURE
  NOTE: 
******************************************************************************/
uns32 srmnd_amf_initialize(SRMND_CB *srmnd) 
{ 
   SaAisErrorT     status = SA_AIS_OK; 
   SaAmfCallbacksT amf_callbacks;
   SaVersionT      saf_version;

   /* Update the SRMSv specific AMF callback functions */
   m_SRMSV_UPDATE_AMF_CBKS(amf_callbacks);

   /* update the SAF version details */
   m_SRMSV_GET_AMF_VER(saf_version);

   /* Initialize the AMF Library */
   status = saAmfInitialize(&srmnd->amf_handle,  /* AMF Handle */
                            &amf_callbacks,      /* AMF Callbacks */ 
                            &saf_version);       /* Version of the AMF */
   if (status != SA_AIS_OK) 
   { 
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_INITIALIZE,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE; 
   }
    
   /* get the communication Handle */
   status = saAmfSelectionObjectGet(srmnd->amf_handle, 
                                    &srmnd->amf_sel_obj); 
   if (status != SA_AIS_OK)
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_GET_SEL_OBJ,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL);       
      srmnd->amf_handle = 0; 
      return NCSCC_RC_FAILURE; 
   }
   
   /* get the component name */
   status = saAmfComponentNameGet(srmnd->amf_handle,  /* input */
                                  &srmnd->comp_name); /* output */
   if (status != SA_AIS_OK) 
   { 
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_GET_COMP_NAME,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL); 
      srmnd->amf_handle = 0; 
      return NCSCC_RC_FAILURE;
   }

   /* Register the component with the Availability Agent */
   status = saAmfComponentRegister(srmnd->amf_handle, &srmnd->comp_name, NULL);
   if (status != SA_AIS_OK)
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_REG_COMP,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL);     
      return NCSCC_RC_FAILURE;
   }

   /* start the health check if it is not started already */
   status = srmnd_healthcheck_start(srmnd); 
   if (status != SA_AIS_OK)
      return NCSCC_RC_FAILURE;

#if 0
   /* start the health check if it is not started already */
   if (srmnd->health_check_started == FALSE)
   {
      /* post a message to subagt mailbox to start the health check */
      if (srmnd_amf_healthcheck_start_msg_post(srmnd) != NCSCC_RC_SUCCESS)
         return NCSCC_RC_FAILURE;     
   }
#endif

   return NCSCC_RC_SUCCESS; 
}


/******************************************************************************
  Name          :  srmnd_amf_finalize

  Description   :  To Finalize the session with the AMF 
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block 

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure
  NOTE          : 
******************************************************************************/
uns32 srmnd_amf_finalize(SRMND_CB *srmnd) 
{ 
   SaAisErrorT status = SA_AIS_OK; 

   /* amf_handle is 0, implies SRMND still not integrated with AMF
      so nothing to do here.. return back */
   if (!(srmnd->amf_handle))
      return NCSCC_RC_SUCCESS;

   /* delete the fd from the select list */ 
   m_NCS_OS_MEMSET(&srmnd->amf_sel_obj, 0, sizeof(SaSelectionObjectT));

   /* Disable the health monitoring */ 
   status = saAmfHealthcheckStop(srmnd->amf_handle, 
                                 &srmnd->comp_name, 
                                 &srmnd->health_check_key); 
   if (status != SA_AIS_OK)
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_STOP_HLTCHECK,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL); 
      /* continue finalization */
   }

   /* Unregister with AMF */ 
   status = saAmfComponentUnregister(srmnd->amf_handle, 
                                     &srmnd->comp_name, 
                                     NULL); 
   if (status != SA_AIS_OK)
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_UNREG_COMP,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL); 
      /* continue finalization */
   } 
    
   /* Finalize */ 
   status = saAmfFinalize(srmnd->amf_handle); 
   if (status != SA_AIS_OK)
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_FINALIZE,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL); 
      /* continue finalization */
   } 
   
   return NCSCC_RC_SUCCESS; 
}



/******************************************************************************
  Name          :  srmnd_amf_health_check_callback

  Description   :  For the Health Check of the SRMND 
 
  Arguments     :  invocation - Invocation Id
                   *comp_name - Component Name
                   *health_check_key - Key

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure

  NOTE          : 
******************************************************************************/
static void srmnd_amf_health_check_callback(SaInvocationT invocation, 
                                            const SaNameT *comp_name,
                                            SaAmfHealthcheckKeyT *health_check_key)
{ 
   SRMND_CB *srmnd = NULL;
   SRMND_EVT *srmnd_evt = NULL;

   /* retrieve srmnd cb */
   if ((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                           (uns32)gl_srmnd_hdl)) == NULL)
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return;      
   }

   /* Post the event to SRMND about timer expiration */
   srmnd_evt = srmnd_evt_create(srmnd->cb_hdl,
                                NULL,
                                &invocation,
                                SRMND_AMF_EVT_TYPE);
   if (srmnd_evt)
      m_NCS_IPC_SEND(&srmnd->mbx, srmnd_evt, NCS_IPC_PRIORITY_HIGH);
      
   ncshm_give_hdl((uns32)gl_srmnd_hdl);

   return; 
}


/******************************************************************************
  Name          :  srmnd_amf_csi_set_callback

  Description   :  To handle the HA state transitions
 
  Arguments     :  SaInvocationT  - Ref given by the AMF
                   comp_name      - Component Name  
                   ha_state        - HA State, destination
                   csi_descriptor - CSI/worlkload description

  Return Values :  Nothing 

  NOTE          :  
******************************************************************************/
static void srmnd_amf_csi_set_callback(SaInvocationT  invocation, 
                                       const SaNameT  *comp_name, 
                                       SaAmfHAStateT  ha_state, 
                                       SaAmfCSIDescriptorT csi_descriptor) 
{
   uns32    status = NCSCC_RC_SUCCESS; 
   SRMND_CB *srmnd = NULL;   

   /* retrieve srmnd cb */
   if ((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                           (uns32)gl_srmnd_hdl)) == NULL)
   {
      /* No association record?? :-( ok log it & return */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return;      
   }

   /* Process the callback, based on the state */  
   status = srmnd_readiness_ha_state_dispatch[srmnd->ha_cstate][ha_state](srmnd, 
                                                                invocation); 
   if (status == NCSCC_RC_SUCCESS)
   {
      /* update the HA state */
      srmnd->ha_cstate = ha_state; 
   }
   
   ncshm_give_hdl((uns32)gl_srmnd_hdl);

   return; 
}


/******************************************************************************
  Name          :  srmnd_invalid_state_process
 
  Description   :  To log the invalid state transition. Same routine is being
                   used for the readiness state as well as HA state transitions.
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_FAILURE -  failure

  NOTE: 
******************************************************************************/
static uns32 srmnd_invalid_state_process(SRMND_CB *srmnd, SaInvocationT invocation)
{
   saAmfResponse(srmnd->amf_handle, invocation, SA_AIS_ERR_BAD_OPERATION); 
   return NCSCC_RC_FAILURE; 
}


/******************************************************************************
  Name          :  srmnd_active_process
 
  Description   :  Active process routine
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_FAILURE - failure
                   NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
static uns32 srmnd_active_process(SRMND_CB *srmnd, SaInvocationT invocation)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Make SRMND oper status UP/active */
   rc = srmnd_active(srmnd);  
   if (rc == NCSCC_RC_SUCCESS)
      saAmfResponse(srmnd->amf_handle, invocation, SA_AIS_OK); 

   return rc; 
}


/******************************************************************************
  Name          :  srmnd_quiescing_quiesced_process

  Description   :  Quiescing & Quiesced process routine
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
static uns32 srmnd_quiescing_quiesced_process(SRMND_CB *srmnd, 
                                              SaInvocationT invocation)
{
   saAmfResponse(srmnd->amf_handle, invocation, SA_AIS_OK); 
   return NCSCC_RC_SUCCESS; 
}


/******************************************************************************
  Name          :  srmnd_quiesced_process

  Description   :  To go to the Quiesced state
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
static uns32 srmnd_quiesced_process(SRMND_CB *srmnd, SaInvocationT invocation)
{ 
   uns32 rc = NCSCC_RC_SUCCESS;
   
   /* Deactive SRMND */
   rc = srmnd_deactive(srmnd);
  
   saAmfResponse(srmnd->amf_handle, invocation, 
                (rc == NCSCC_RC_SUCCESS)?SA_AIS_OK:SA_AIS_ERR_FAILED_OPERATION); 

   return rc; 
}


/******************************************************************************
  Name          :  srmnd_quiescing_process

  Description   :  To go to the Quiesced state
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
static uns32 srmnd_quiescing_process(SRMND_CB *srmnd, SaInvocationT invocation)
{
   uns32       status = NCSCC_RC_FAILURE; 
   SaAisErrorT saf_status = SA_AIS_OK; 
      
   /* make SRMND operation status to DOWN state */ 
   status = srmnd_quiesced_process(srmnd, invocation); 

   /* QUIESCING is complete */ 
   saf_status = saAmfCSIQuiescingComplete(srmnd->amf_handle, invocation, 
                (status == NCSCC_RC_SUCCESS)?SA_AIS_OK:SA_AIS_ERR_FAILED_OPERATION); 
   if (saf_status != SA_AIS_OK)
   {
      status = NCSCC_RC_FAILURE; 
   }

   return status; 
}
   
 
/******************************************************************************
  Name          :  srmnd_amf_component_terminate_callback

  Description   :  To handle the Component Terminate request from AMF 
                   - All the sessions will be closed and Subagent will Suicide
 
  Arguments     :  SaInvocationT    - Ref given by the AMF
                   SaNameT          - Name of the Component

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure
  NOTE          : 
******************************************************************************/
static void srmnd_amf_component_terminate_callback(SaInvocationT invocation, 
                                                   const SaNameT *comp_name)
{
   SRMND_CB *srmnd = NULL;
 
   /* retrieve srmnd cb */
   if ((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                       (uns32)gl_srmnd_hdl)) == SRMND_CB_NULL)
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return;      
   }

   ncshm_give_hdl((uns32)gl_srmnd_hdl);    
       
   saAmfResponse(srmnd->amf_handle, invocation, SA_AIS_OK); 

   /* Destroy the SRMND process */ 
   if (srmnd_destroy(TRUE) == NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_MISC(SRMND_LOG_MISC_ND_DESTROY,
                       SRMND_LOG_MISC_SUCCESS,
                       NCSFL_SEV_INFO);
   }
   else
   {
      m_SRMND_LOG_MISC(SRMND_LOG_MISC_ND_DESTROY,
                       SRMND_LOG_MISC_FAILED,
                       NCSFL_SEV_CRITICAL);
   }
 
   /* Suicide :-( */ 
   exit(0); 
} 


/******************************************************************************
  Name          :  srmnd_amf_csi_remove_callback

  Description   :  To remove the instance of a component 
 
  Arguments     :  SRMND_CB *srmnd   - SRMND control block 
                   SaInvocationT  - Ref given by the AMF
                   compNam        - Name of the component (i)
                   csi_name        - Name of the CSI (i)
                   csi_flags       - Flags...

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure

  NOTE          : 
******************************************************************************/
static void srmnd_amf_csi_remove_callback(SaInvocationT  invocation, 
                                          const SaNameT  *comp_name, 
                                          const SaNameT  *csi_name,
                                          SaAmfCSIFlagsT csi_flags)
{
   uns32    status = NCSCC_RC_SUCCESS; 
   SRMND_CB *srmnd = NULL;

   /* retrieve srmnd cb */
   if ((srmnd = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                       (uns32)gl_srmnd_hdl)) == NULL)
   {
      /* No association record?? :-( ok log it & return */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return;      
   }

   /* If SRMND is in active state only then do the quiesced process */
   if (srmnd->ha_cstate == SA_AMF_HA_ACTIVE)
      status = srmnd_quiesced_process(srmnd, invocation);  
   
   if (status == NCSCC_RC_SUCCESS)
      srmnd->ha_cstate = SRMND_HA_STATE_NULL; /* Rest the HA State */ 
   
   saAmfResponse(srmnd->amf_handle, invocation, SA_AIS_OK); 

   ncshm_give_hdl((uns32)gl_srmnd_hdl);           

   return; 
}


#if 0
/******************************************************************************
  Name          :  srmnd_amf_healthcheck_start_msg_post

  Description   :  posts a message to the SRMND thread to start the health
                   check 
 
  Arguments     :  SRMND_CB *srmnd - SRMND control block 

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure

  NOTE          : 
******************************************************************************/
uns32 srmnd_amf_healthcheck_start_msg_post(SRMND_CB *srmnd) 
{
   uns32     status = NCSCC_RC_SUCCESS; 
   SRMND_EVT *post_me = NULL;
    
   /* update the message info */
   post_me = m_MMGR_ALLOC_SRMND_EVT;
   if (post_me == NULL)
   {
      m_SRMND_LOG_MEM(SRMND_MEM_EVENT,
                      SRMND_MEM_ALLOC_FAILED,
                      NCSFL_SEV_CRITICAL);
      return NCSCC_RC_OUT_OF_MEM;
   }
    
   m_NCS_OS_MEMSET(post_me, 0, sizeof(SRMND_EVT));

   /* Update event data */
   post_me->evt_type  = SRMND_HC_START_EVT_TYPE;
   post_me->cb_handle = srmnd->cb_hdl;

   /* post a message to the SRMND's thread */
   status = m_NCS_IPC_SEND(&srmnd->mbx,
                           (NCS_IPC_MSG*)post_me,
                           NCS_IPC_PRIORITY_HIGH);
   if (status != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_IPC_SEND,
                      SRMSV_LOG_TIM_FAILURE,
                      NCSFL_SEV_CRITICAL);    
      return status;
   }
   
   return status;
}
#endif

/******************************************************************************
  Name          :  srmnd_healthcheck_start                            
                                                                            
  Description   :  To start the health check                                  
                                                                            
  Arguments     :  SRMND_CB* - Control Block                                   
                                                                             
  Return Values :  NCSCC_RC_SUCCESS - everything is OK                      
                   NCSCC_RC_FAILURE - failure                              

  NOTE          :                                                                      
*******************************************************************************/
uns32 srmnd_healthcheck_start(SRMND_CB *cb) 
{
    SaAisErrorT  saf_status = SA_AIS_OK;    

    /* If health check already started, no need to process this function 
       once again */
    if (cb->health_check_started == TRUE)
       return NCSCC_RC_SUCCESS; 
   
    m_NCS_STRCPY(cb->health_check_key.key, SRMND_AMF_HEALTH_CHECK_KEY);
    cb->health_check_key.keyLen = m_NCS_OS_STRLEN(cb->health_check_key.key);
    
    /* start the healthcheck */ 
    saf_status = saAmfHealthcheckStart(cb->amf_handle,
                                       &cb->comp_name,
                                       &cb->health_check_key,
                                       SA_AMF_HEALTHCHECK_AMF_INVOKED,
                                       SA_AMF_COMPONENT_RESTART);
    if (saf_status != SA_AIS_OK) 
    {
       m_SRMND_LOG_AMF(SRMND_LOG_AMF_START_HLTCHECK,
                       SRMND_LOG_AMF_FAILED,
                       NCSFL_SEV_CRITICAL); 
       return NCSCC_RC_FAILURE;
    }
   
    cb->health_check_started = TRUE; 

    m_SRMND_LOG_AMF(SRMND_LOG_AMF_START_HLTCHECK,
                    SRMND_LOG_AMF_SUCCESS,
                    NCSFL_SEV_INFO); 
    
    return NCSCC_RC_SUCCESS; 
}    

