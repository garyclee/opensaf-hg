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

  MODULE NAME: VDS_AMF.C
 
..............................................................................

  DESCRIPTION: This file contains VDS AMF integration specific routines

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/
#include "vds.h"
extern uns32 gl_vds_hdl;
extern uns32 vds_role_ack_to_state;

/****************************************************************************
                       Static Function Prototypes
****************************************************************************/
static uns32 vds_invalid_state_process(VDS_CB *vds, SaInvocationT invocation);
/* Go to ACTIVE State */ 
static uns32 vds_active_process(VDS_CB  *vds, SaInvocationT invocation);
/* Go to STAND-BY State */ 
static uns32 vds_standby_process(VDS_CB *vds, SaInvocationT invocation);
/* Go to QUIESCED State */
static uns32 vds_quiesced_process(VDS_CB  *vds, SaInvocationT invocation);
/* Go to QUIESCING State */
static uns32 vds_quiescing_process(VDS_CB  *vds, SaInvocationT  invocation);
/* Go to QUIESCED from QUIESCING State */
static uns32 vds_quiescing_quiesced_process(VDS_CB *vds, SaInvocationT invocation);
/* to initialize the AMF Callbacks */
static void vds_amf_callbacks_init(SaAmfCallbacksT *amf_call_backs);
/* Describes the health check of the VDS */
static void vds_amf_health_check_callback(SaInvocationT invocation, 
                                          const SaNameT *comp_name, 
                                          SaAmfHealthcheckKeyT *check_key);
/* HA State Handling (Failover and Switchover) */
static void vds_amf_csi_set_callback(SaInvocationT invocation, 
                                     const SaNameT *comp_name, 
                                     SaAmfHAStateT ha_state, 
                                     SaAmfCSIDescriptorT csi_descriptor);
/* Component Terminate callback */
static void vds_amf_component_terminate_callback(SaInvocationT invocation, 
                                                 const SaNameT *comp_name);
/* Component Service Instance Remove Callback */
static void vds_amf_csi_remove_callback(SaInvocationT  invocation, 
                                        const SaNameT  *comp_name, 
                                        const SaNameT  *csi_name, 
                                        SaAmfCSIFlagsT csi_flags);

/****************************************************************************
                 Initialize the HA state machine 
****************************************************************************/
const vds_readiness_ha_state_process
      vds_readiness_ha_state_dispatch[VDS_HA_STATE_MAX]
                                     [VDS_HA_STATE_MAX]=
{
   /* current HA State NULL -- Initial state of the VDS */
   {
      vds_invalid_state_process, /* should never be the case*/
      vds_active_process,        /* Go to ACTIVE State */ 
      vds_standby_process,       /* Go to STAND-BY State */ 
      vds_invalid_state_process, /* should never be the case*/
      vds_invalid_state_process, /* should never be the case*/
   },
   /* Current HA State ACTIVE */
   {
      vds_invalid_state_process, /* should never be the case*/
      vds_invalid_state_process, /* should never be the case*/
      vds_invalid_state_process, /* should never be the case*/
      vds_quiesced_process,      /* Go to QUIESCED State */
      vds_quiescing_process,     /* Go to QUIESCING State */
   },
   /* Current HA State STAND-BY */
   {
      vds_invalid_state_process, /* should never be the case*/
      vds_active_process,        /* Go to ACTIVE State */ 
      vds_invalid_state_process, /* should never be the case*/
      vds_invalid_state_process, /* should never be the case*/
      vds_invalid_state_process, /* should never be the case*/
   },
   /* current HA State QUIESCED */
   {
      vds_invalid_state_process, /* should never be the case*/
      vds_active_process,        /* Goto ACTIVE state */
      vds_standby_process,       /* Goto STAND-BY state */
      vds_invalid_state_process, /* never be the case*/
      vds_invalid_state_process, /* never be the case*/
   },
   /* current HA State QUIESCING */
   {
      vds_invalid_state_process, /* never be the case*/
      vds_active_process,        /* Goto ACTIVE state */
      vds_invalid_state_process, /* never be the case*/
      vds_quiescing_quiesced_process,/* Go to QUIESCED State */
      vds_invalid_state_process, /* never be the case*/
   }
};

/******************************************************************************
  Name          :  vds_amf_initialize

  Description   :  To intialize the session with AMF Agent
 
  Arguments     :  VDS_CB *vds - VDS control block 

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure
                   NCSCC_RC_AMF_INIT_FAILURE - Init Library failure 
                   NCSCC_RC_AMF_COMP_NAME_GET_FAILURE 
                   NCSCC_RC_AMF_COMP_REG_FAILURE 
                   NCSCC_RC_AMF_SEL_OBJ_FAILURE
                   NCSCC_RC_AMF_HEALTH_CHECK_FAILURE
  NOTE: 
******************************************************************************/
uns32 vds_amf_initialize(VDS_CB *vds) 
{ 
   SaAisErrorT status = SA_AIS_OK; 
   SaAmfCallbacksT amf_callbacks;          /* AMF Callbacks */
   SaVersionT      saf_version;
   
   VDS_TRACE1_ARG1("vds_amf_initialize\n"); 
   
   /* initialize the AMF Callbacks */
   vds_amf_callbacks_init(&amf_callbacks);  
    
   m_VDS_LOG_AMF(VDS_LOG_AMF_CBK_INIT,
                           VDS_LOG_AMF_SUCCESS,
                                     NCSFL_SEV_INFO, 0);    
   
   m_NCS_OS_MEMSET(&saf_version, 0,sizeof(SaVersionT));

   /* update the SAF version details */
   m_VDS_GET_AMF_VER(saf_version);

   /* Initialize the AMF Library */
   status = saAmfInitialize(&vds->amf.amf_handle,    /* AMF Handle */
                            &amf_callbacks, /* AMF Callbacks */ 
                            &saf_version);  /* Version of the AMF */
   if (status != SA_AIS_OK) 
   { 
         m_VDS_LOG_AMF(VDS_LOG_AMF_INIT,
                               VDS_LOG_AMF_FAILURE,
                                       NCSFL_SEV_CRITICAL, status);         
         
         return NCSCC_RC_FAILURE; 
   }   
       
   /* get the communication Handle */
   status = saAmfSelectionObjectGet(vds->amf.amf_handle, 
                                    &vds->amf.amf_sel_obj); 
   if (status != SA_AIS_OK)
   {     
      m_VDS_LOG_AMF(VDS_LOG_AMF_GET_SEL_OBJ,
                                VDS_LOG_AMF_FAILURE,
                                        NCSFL_SEV_CRITICAL, status);  
      
      return NCSCC_RC_FAILURE; 
   }
   
   /* get the component name */
   status = saAmfComponentNameGet(vds->amf.amf_handle,  /* input */
                                  &vds->amf.comp_name); /* output */
   if (status != SA_AIS_OK) 
   {   
      m_VDS_LOG_AMF(VDS_LOG_AMF_GET_COMP_NAME,
                                VDS_LOG_AMF_FAILURE,
                                        NCSFL_SEV_CRITICAL, status); 
      return NCSCC_RC_FAILURE;
   }

   /* Register the component with the Availability Agent */
   status = saAmfComponentRegister(vds->amf.amf_handle, &vds->amf.comp_name, NULL);
   if (status != SA_AIS_OK)
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_REG_COMP,
                                VDS_LOG_AMF_FAILURE,
                                        NCSFL_SEV_CRITICAL, status); 
      return NCSCC_RC_FAILURE;
   }

   
   m_NCS_OS_MEMSET(&vds->amf.health_check_key, 0,sizeof(SaAmfHealthcheckKeyT));

   vds->amf.health_check_key.keyLen = m_NCS_OS_STRLEN(VDS_AMF_HEALTH_CHECK_KEY);
   
   m_NCS_STRNCPY(vds->amf.health_check_key.key, 
                 VDS_AMF_HEALTH_CHECK_KEY,
                          vds->amf.health_check_key.keyLen);
   

   /* start the healthcheck */ 
   status = saAmfHealthcheckStart(vds->amf.amf_handle,
                                      &vds->amf.comp_name,
                                      &vds->amf.health_check_key,
                                      VDS_HEALTHCHECK_AMF_INVOKED,
                                      VDS_AMF_COMPONENT_FAILOVER);
   if (status != SA_AIS_OK) 
   {   
      m_VDS_LOG_AMF(VDS_LOG_AMF_START_HLTCHECK,
                                VDS_LOG_AMF_FAILURE,
                                        NCSFL_SEV_CRITICAL, status); 
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS; 
}


/******************************************************************************
  Name          :  vds_amf_finalize

  Description   :  To Finalize the session with the AMF 
 
  Arguments     :  VDS_CB *vds - VDS control block 

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure
  NOTE          : 
******************************************************************************/
uns32 vds_amf_finalize(VDS_CB *vds) 
{ 
   SaAisErrorT status = SA_AIS_OK; 


   VDS_TRACE1_ARG1("vds_amf_finalize\n"); 
   
   /* amf_handle is 0, implies VDS still not integrated with AMF
      so nothing to do here.. return back */
   if (!(vds->amf.amf_handle))
      return NCSCC_RC_SUCCESS;

   /* delete the fd from the select list */ 
   m_NCS_OS_MEMSET(&vds->amf.amf_sel_obj, 0, sizeof(SaSelectionObjectT));

   /* Disable the health monitoring */ 
   status = saAmfHealthcheckStop(vds->amf.amf_handle, 
                                 &vds->amf.comp_name, 
                                 &vds->amf.health_check_key); 
   if (status != SA_AIS_OK)
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_STOP_HLTCHECK,
                                VDS_LOG_AMF_FAILURE,
                                        NCSFL_SEV_CRITICAL, status); 
      /* continue finalization */
   } 

   m_VDS_LOG_AMF(VDS_LOG_AMF_UNREG_COMP,
                               VDS_LOG_AMF_SUCCESS,
                                      NCSFL_SEV_NOTICE, 0); 

   /* Unregister with AMF */   
   /* Added by vishal for IR00085041 : replacing unregister with error report */
   status  = saAmfComponentErrorReport(vds->amf.amf_handle, &vds->amf.comp_name,
                                   0, SA_AMF_COMPONENT_RESTART, 0);
   
   if (status != SA_AIS_OK)
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_UNREG_COMP,
                               VDS_LOG_AMF_FAILURE,
                                      NCSFL_SEV_CRITICAL, status); 
      
      /* continue finalization */
   } 
       
   /* Finalize */ 
   status = saAmfFinalize(vds->amf.amf_handle); 
   if (status != SA_AIS_OK)
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_FIN,
                               VDS_LOG_AMF_FAILURE,
                                       NCSFL_SEV_CRITICAL, status); 
      /* continue finalization */
   } 
   
   return NCSCC_RC_SUCCESS; 
}

/******************************************************************************
  Name          :  vds_amf_callbacks_init

  Description   :  To Initialize the AMF callbacks
 
  Arguments     :  SaAmfCallbacksT *  - structure pointer to be filled in with
                                        the AMF callbacks

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure
  NOTE          : 
******************************************************************************/
void vds_amf_callbacks_init(SaAmfCallbacksT *amf_call_backs)
{  

   VDS_TRACE1_ARG1("vds_amf_callbacks_init\n");
    
   /* Initialize all the callbacks to NULL */ 
   m_NCS_OS_MEMSET(amf_call_backs, 0, sizeof(SaAmfCallbacksT)); 

   /* healthcheck callback */ 
   amf_call_backs->saAmfHealthcheckCallback = vds_amf_health_check_callback; 

   /* Component Terminate Callback */
   amf_call_backs->saAmfComponentTerminateCallback =
                                     vds_amf_component_terminate_callback;

   /* HA state Handling callback */ 
   amf_call_backs->saAmfCSISetCallback = vds_amf_csi_set_callback;

   /* CSI Remove callback */ 
   amf_call_backs->saAmfCSIRemoveCallback = vds_amf_csi_remove_callback; 
    
   return; 
}

/******************************************************************************
  Name          :  vds_amf_health_check_callback

  Description   :  For the Health Check of the VDS 
 
  Arguments     :  invocation - Invocation Id
                   *comp_name - Component Name
                   *health_check_key - Key

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure

  NOTE          : 
******************************************************************************/
void vds_amf_health_check_callback(SaInvocationT invocation, 
                                   const SaNameT *comp_name,
                                   SaAmfHealthcheckKeyT *health_check_key)
{

   VDS_EVT *vds_evt = NULL;
   uns32   rc = NCSCC_RC_SUCCESS;
   VDS_CB *vds = NULL;

   VDS_TRACE1_ARG1("vds_amf_health_check_callback\n");

   /* retrieve vds cb */
   if ((vds = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS,
                                       (uns32)gl_vds_hdl)) == NULL)
   {
       m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                                 VDS_LOG_HDL_FAILURE,
                                         NCSFL_SEV_CRITICAL);
      /* No association record?? :-( ok log it & return error */
      return;
   }

   /* Create an event for VDS */
   vds_evt = vds_evt_create((NCSCONTEXT)&invocation,
                            VDS_AMF_HEALTH_CHECK_EVT_TYPE);
   /* send the event */
   if (vds_evt)
      rc = m_NCS_IPC_SEND(&vds->mbx, vds_evt, NCS_IPC_PRIORITY_HIGH);

   /* if failure, free the event */
   if ((rc != NCSCC_RC_SUCCESS) && (vds_evt))
      vds_evt_destroy(vds_evt);

   ncshm_give_hdl((uns32)gl_vds_hdl);

   return; 
}


/******************************************************************************
  Name          :  vds_amf_csi_set_callback

  Description   :  To handle the HA state transitions
 
  Arguments     :  SaInvocationT  - Ref given by the AMF
                   comp_name      - Component Name  
                   ha_state       - HA State, destination
                   csi_descriptor - CSI/worlkload description

  Return Values :  Nothing 

  NOTE          :  
******************************************************************************/
void vds_amf_csi_set_callback(SaInvocationT  invocation, 
                              const SaNameT  *comp_name, 
                              SaAmfHAStateT  ha_state, 
                              SaAmfCSIDescriptorT csi_descriptor) 
{
   uns32  status = NCSCC_RC_SUCCESS; 
   VDS_CB *vds = NULL;   
   char *fsm[5] ={"NULL State","ACTIVE State","STAND-BY state",
                 "QUIESCED State","QUiescing state"};

   VDS_TRACE1_ARG1("vds_amf_csi_set_callback\n"); 

   m_VDS_LOG_AMF(VDS_LOG_AMF_SET_CALLBACK,
                               VDS_LOG_AMF_NOTHING,
                                      NCSFL_SEV_NOTICE, 0);

   /* retrieve vds cb */
   if ((vds = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS,
                                           (uns32)gl_vds_hdl)) == NULL)
   {
      /* No association record?? :-( ok log it & return */      
       m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                                 VDS_LOG_HDL_FAILURE,
                                         NCSFL_SEV_CRITICAL);
      return;      
   }

   /* Process the callback, based on the state */  
   m_VDS_LOG_AMF_STATE(fsm[vds->amf.ha_cstate],fsm[ha_state]);

   status = vds_readiness_ha_state_dispatch[vds->amf.ha_cstate][ha_state](vds, 
                                                                invocation); 
   if (status == NCSCC_RC_SUCCESS) 
   {

      if (ha_state == VDS_HA_STATE_QUIESCING)
      {
         ncshm_give_hdl((uns32)gl_vds_hdl);
         
         return;
      }
      else
      if (ha_state != VDS_HA_STATE_QUIESCED)
      {
         vds->amf.ha_cstate = ha_state; 
      }
      
   }

   if(vds_destroying != TRUE)
     ncshm_give_hdl((uns32)gl_vds_hdl);

   return; 
}


/******************************************************************************
  Name          :  vds_invalid_state_process
 
  Description   :  To log the invalid state transition. Same routine is being
                   used for the readiness state as well as HA state transitions.
 
  Arguments     :  VDS_CB *vds - VDS control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_FAILURE -  failure

  NOTE: 
******************************************************************************/
uns32 vds_invalid_state_process(VDS_CB *vds, SaInvocationT invocation)
{
   
   VDS_TRACE1_ARG1("vds_invalid_state_process\n"); 
   
   SaAisErrorT status = SA_AIS_OK; 


   if((status = saAmfResponse(vds->amf.amf_handle, invocation, SA_AIS_ERR_FAILED_OPERATION))!= SA_AIS_OK) 
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                                  VDS_LOG_AMF_FAILURE,
                                          NCSFL_SEV_CRITICAL, status);
   }

   m_VDS_LOG_AMF(VDS_LOG_AMF_INVALID,
                               VDS_LOG_AMF_NOTHING,
                                         NCSFL_SEV_NOTICE, 0);

   return NCSCC_RC_FAILURE; 
}


/******************************************************************************
  Name          :  vds_active_process
 
  Description   :  Active process routine
 
  Arguments     :  VDS_CB *vds - VDS control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_FAILURE - failure
                   NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
uns32 vds_active_process(VDS_CB *vds, SaInvocationT invocation)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   SaAisErrorT status = SA_AIS_OK; 

   V_DEST_RL role;

   VDS_TRACE1_ARG1("vds_active_process\n"); 

   /* Make VDS oper status UP/active */
   role = VDS_HA_STATE_ACTIVE;
  
   /* Update MDS about the ROLE change */
   rc = vds_role_agent_callback(vds, role);

   if (rc == NCSCC_RC_SUCCESS)
   {        
      if((status = saAmfResponse(vds->amf.amf_handle, invocation, SA_AIS_OK)) != SA_AIS_OK) 
      { 
                     
        m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                                  VDS_LOG_AMF_FAILURE,
                                          NCSFL_SEV_CRITICAL, status);
      }
   }   
   else
   {
      VDS_TRACE1_ARG1("vds_active_process: FAILURE\n"); 
      return NCSCC_RC_FAILURE;
   }

   /* Read the checkpoint data */
   /* Condition deleted by vishal : IR00084570 */
   vds_ckpt_read(vds);

   
   m_VDS_LOG_AMF(VDS_LOG_AMF_ACTIVE,
                               VDS_LOG_AMF_NOTHING,
                                      NCSFL_SEV_NOTICE, 0);
   return rc; 
}


/******************************************************************************
  Name          :  vds_standby_process
 
  Description   :  Standby process routine
 
  Arguments     :  VDS_CB *vds - VDS control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_FAILURE - failure
                   NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
uns32 vds_standby_process(VDS_CB *vds, SaInvocationT invocation)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   V_DEST_RL role;
   SaAisErrorT status = SA_AIS_OK; 

   VDS_TRACE1_ARG1("vds_standby_process\n"); 
   
   /* Make VDS oper status UP/active */
   role = VDS_HA_STATE_STDBY;
  
   /* Update MDS about the ROLE change */
   rc = vds_role_agent_callback(vds, role);

   if (rc == NCSCC_RC_SUCCESS)
   {        
      if((status = saAmfResponse(vds->amf.amf_handle, invocation, SA_AIS_OK)) != SA_AIS_OK)
      {
         m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                               VDS_LOG_AMF_FAILURE,
                                      NCSFL_SEV_CRITICAL, status);
      }  
   }   
   else
   {
      VDS_TRACE1_ARG1("vds_stanby_process: FAILURE\n"); 
      return NCSCC_RC_FAILURE;

   }

    m_VDS_LOG_AMF(VDS_LOG_AMF_STANDBY,
                               VDS_LOG_AMF_NOTHING,
                                       NCSFL_SEV_NOTICE, 0);
   
   return rc; 
}


/******************************************************************************
  Name          :  vds_quiescing_quiesced_process

  Description   :  Quiescing & Quiesced process routine
 
  Arguments     :  VDS_CB *vds - VDS control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
uns32 vds_quiescing_quiesced_process(VDS_CB *vds, 
                                     SaInvocationT invocation)
{
   VDS_TRACE1_ARG1("vds_quiescing_quiesced_process\n"); 
   SaAisErrorT status = SA_AIS_OK; 

   if((status = saAmfResponse(vds->amf.amf_handle, invocation, SA_AIS_OK))!= SA_AIS_OK) 
   {
    
      m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                                  VDS_LOG_AMF_FAILURE,
                                          NCSFL_SEV_CRITICAL, status);
   }

   m_VDS_LOG_AMF(VDS_LOG_AMF_QUIESCING_QUIESCED,
                                  VDS_LOG_AMF_NOTHING,
                                            NCSFL_SEV_NOTICE, 0);
   return NCSCC_RC_SUCCESS; 
}


/******************************************************************************
  Name          :  vds_quiesced_process

  Description   :  To go to the Quiesced state
 
  Arguments     :  VDS_CB *vds - VDS control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
uns32 vds_quiesced_process(VDS_CB *vds, SaInvocationT invocation)
{ 
   V_DEST_RL role;
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_quiesced_process\n"); 
   
   vds->amf.invocation = invocation;

   role = VDS_HA_STATE_QUIESCED; 

   vds_role_ack_to_state = VDS_HA_STATE_QUIESCED;
  
   /* Update MDS about the ROLE change */
   rc = vds_role_agent_callback(vds, role);

   if (rc != NCSCC_RC_SUCCESS)
   {
      VDS_TRACE1_ARG1("vds_quiesced_process: Failed in role change\n"); 
      return NCSCC_RC_FAILURE;
   }

   m_VDS_LOG_AMF(VDS_LOG_AMF_QUIESCED,
                               VDS_LOG_AMF_NOTHING,
                                      NCSFL_SEV_NOTICE, 0);   
   return rc;
}


/******************************************************************************
  Name          :  vds_quiescing_process

  Description   :  To go to the Quiesced state
 
  Arguments     :  VDS_CB *vds - VDS control block (i)
                   SaInvocationT - Ref. number given by AMF (i)
 
  Return Values :  NCSCC_RC_SUCCESS - success

  NOTE: 
******************************************************************************/
uns32 vds_quiescing_process(VDS_CB *vds, SaInvocationT invocation)
{
   V_DEST_RL role;
   uns32 rc = NCSCC_RC_SUCCESS;
   SaAisErrorT status = SA_AIS_OK; 

   VDS_TRACE1_ARG1("vds_quiescing_process\n"); 

   vds->amf.invocation = invocation;

   role = VDS_HA_STATE_QUIESCED;
 
   vds_role_ack_to_state = VDS_HA_STATE_QUIESCING;

   /* Update MDS about the ROLE change */
   rc = vds_role_agent_callback(vds, role);

   if (rc != NCSCC_RC_SUCCESS)
   {
      VDS_TRACE1_ARG1("vds_quiescing_process: Failed in role change\n");
      return NCSCC_RC_FAILURE;
   }
   else
   {
     /* send the response to AMF immediately */ 
     if ((status = saAmfResponse(vds->amf.amf_handle, invocation, SA_AIS_OK)) != SA_AIS_OK)
     { 

        VDS_TRACE1_ARG1("vds_quiescing_process: Failed in SA_AIS_OK response\n"); 
        m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                             VDS_LOG_AMF_FAILURE,
                                   NCSFL_SEV_CRITICAL, status);
     }   
   }
 
   m_VDS_LOG_AMF(VDS_LOG_AMF_QUIESCING,
                               VDS_LOG_AMF_NOTHING,
                                        NCSFL_SEV_NOTICE, 0);
   return rc; 
}
   
 
/******************************************************************************
  Name          :  vds_amf_component_terminate_callback

  Description   :  To handle the Component Terminate request from AMF 
                   - All the sessions will be closed and Subagent will Suicide
 
  Arguments     :  SaInvocationT - Ref given by the AMF
                   SaNameT       - Name of the Component

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure
  NOTE          : 
******************************************************************************/
void vds_amf_component_terminate_callback(SaInvocationT invocation, 
                                          const SaNameT *comp_name)
{ 
   VDS_CB *vds = NULL;
   SaAisErrorT status = SA_AIS_OK; 

   VDS_TRACE1_ARG1("vds_amf_compoent_terminate_callback\n"); 

   /* retrieve vds cb */
   if ((vds = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS,
                                           (uns32)gl_vds_hdl)) == NULL)
   {
      /* No association record?? :-( ok log it & return */
      m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                                 VDS_LOG_HDL_FAILURE,
                                         NCSFL_SEV_CRITICAL);
      return;
   }

   if((status = saAmfResponse(vds->amf.amf_handle, invocation, SA_AIS_OK))!= SA_AIS_OK)
   {
     m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                             VDS_LOG_AMF_FAILURE,
                                   NCSFL_SEV_CRITICAL, status);
   }
   
   ncshm_give_hdl((uns32)gl_vds_hdl);
 
   m_VDS_LOG_AMF(VDS_LOG_AMF_TERM_CALLBACK,
                               VDS_LOG_AMF_NOTHING,
                                      NCSFL_SEV_NOTICE, 0);
   /* Destroy the VDS process */ 
   if (vds_destroy(VDS_DONT_CANCEL_THREAD) == NCSCC_RC_SUCCESS)
   {
       /* Continue to exit  */
   }

   /* Suicide :-( */ 
    
   exit(0); 
} 


/******************************************************************************
  Name          :  vds_amf_csi_remove_callback

  Description   :  To remove the instance of a component 
 
  Arguments     :
                   SaInvocationT - Ref given by the AMF
                   compNam       - Name of the component (i)
                   csi_name      - Name of the CSI (i)
                   csi_flags     - Flags...

  Return Values :  NCSCC_RC_SUCCESS - everything is OK
                   NCSCC_RC_FAILURE -  failure

  NOTE          : 
******************************************************************************/
void vds_amf_csi_remove_callback(SaInvocationT  invocation, 
                                 const SaNameT  *comp_name, 
                                 const SaNameT  *csi_name,
                                 SaAmfCSIFlagsT csi_flags)
{
   uns32  rc = NCSCC_RC_SUCCESS; 
   SaAisErrorT status = SA_AIS_OK; 
   VDS_CB *vds = NULL;

   VDS_TRACE1_ARG1("vds_amf_csi_remove_callback\n"); 
   
   /* retrieve vds cb */
   if ((vds = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS,
                                       (uns32)gl_vds_hdl)) == NULL)
   {
      m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                              VDS_LOG_HDL_FAILURE,
                                      NCSFL_SEV_CRITICAL);
      /* No association record?? :-( ok log it & return */      
      return;      
   }
   
   if (vds->amf.ha_cstate == VDS_HA_STATE_ACTIVE)
   {
      rc = vds_quiesced_process(vds, invocation);   
   }
   else
   { 
      if((status = saAmfResponse(vds->amf.amf_handle, invocation, SA_AIS_OK))!= SA_AIS_OK)
      {

         m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                             VDS_LOG_AMF_FAILURE,
                                   NCSFL_SEV_CRITICAL, status);
      }
    }

   if (rc == NCSCC_RC_SUCCESS)
      vds->amf.ha_cstate = VDS_HA_STATE_NULL; /* Rest the HA State */ 

   ncshm_give_hdl((uns32)gl_vds_hdl);     
   
   m_VDS_LOG_AMF(VDS_LOG_AMF_REMOVE_CALLBACK,
                               VDS_LOG_AMF_NOTHING,
                                      NCSFL_SEV_NOTICE, 0);      
   return; 
}

