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
  
    
..............................................................................
      
DESCRIPTION:
        
This include file contains AMF interaction logic for health-check and other
stuff.
*******************************************************************************/
#include "eds.h"


/* HA AMF statemachine & State handler definitions */

#if 0
static eds_HAStateHandler eds_stateHandler[]={
     eds_invalid_state_handler,
     eds_active_state_handler,
     eds_standby_state_handler,
     eds_quiesced_state_handler,
     eds_quiescing_state_handler
}; /*Indexed by next states */
                                                                                                                       
static struct next_HAState validStates[]={
{SA_AMF_HA_ACTIVE,SA_AMF_HA_STANDBY}, /* From Current State Initial */
{SA_AMF_HA_QUIESCED,SA_AMF_HA_QUIESCING}, /*Current State active */
{SA_AMF_HA_ACTIVE,EDS_HA_INVALID}, /*Current State standby */
{SA_AMF_HA_ACTIVE,SA_AMF_HA_STANDBY}, /* Current State quiesced */
{SA_AMF_HA_ACTIVE,SA_AMF_HA_QUIESCED} /*Current State quiescing */
}; /*Indexed by current states */

/* HA state handler functions definitions */                                                                           

/****************************************************************************
 * Name          : eds_active_state_handler
 *
 * Description   : This function is called upon receving an active state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block. 
 *
 * Return Values : None
 *
 * Notes         : None 
 *****************************************************************************/

uns32 eds_active_state_handler(EDS_CB *cb,
                              SaInvocationT invocation)
{
   V_DEST_RL         mds_role;
#if 0
   V_CARD_QA         anchor;
#endif

   mds_role = V_DEST_RL_ACTIVE;
#if 0
   anchor   = V_DEST_QA_1;
#endif

   /** set the CB's anchor value & mds role */
#if 0
   cb->my_anc = anchor;
#endif
   cb->mds_role = mds_role;

   m_NCS_CONS_PRINTF("I AM HA AMF ACTIVE\n");
   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_standby_state_handler
 *
 * Description   : This function is called upon receving an standby state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

uns32 eds_standby_state_handler(EDS_CB *cb,
                              SaInvocationT invocation)
{
   V_DEST_RL         mds_role;
#if 0
   V_CARD_QA         anchor;
#endif

/*TBD: CHECK THE USAGE OF MDS_ROLE!!!!!!!!!!!!!!!!!!!!! */
   mds_role = V_DEST_RL_STANDBY;
#if 0
   anchor   = V_DEST_QA_2;
#endif

   /** set the CB's anchor value */
#if 0
   cb->my_anc = anchor;
#endif
   cb->mds_role = mds_role;

   m_NCS_CONS_PRINTF("I AM HA AMF STANDBY\n");

   return NCSCC_RC_SUCCESS;
}
#endif 
/****************************************************************************
 * Name          : eds_quiescing_state_handler
 *
 * Description   : This function is called upon receving an Quiescing state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

uns32 eds_quiescing_state_handler(EDS_CB *cb,
                              SaInvocationT invocation)
{
   SaAisErrorT error=SA_AIS_OK;
   error =  saAmfCSIQuiescingComplete( cb->amf_hdl,
                                        invocation,
                                        error);

   saAmfResponse(cb->amf_hdl, invocation, error);
   m_NCS_CONS_PRINTF("I AM IN HA AMF QUIESCING STATE\n");
   return NCSCC_RC_SUCCESS;
}
                                                                                                                       
/****************************************************************************
 * Name          : eds_quiesced_state_handler
 *
 * Description   : This function is called upon receving an Quiesced state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

uns32 eds_quiesced_state_handler(EDS_CB *cb,
                              SaInvocationT invocation)
{
   V_DEST_RL         mds_role;

   mds_role = V_DEST_RL_QUIESCED;

   /** set the CB's anchor value & mds role */

   cb->mds_role = mds_role;
   eds_mds_change_role(cb);
   cb->amf_invocation_id = invocation;
   
   cb->is_quisced_set = TRUE; 
   m_NCS_CONS_PRINTF("I AM IN HA AMF QUIESCED STATE\n");
   return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : eds_invalid_state_handler
 *
 * Description   : This function is called upon receving an invalid state(as
 *                 in the FSM) assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

uns32 eds_invalid_state_handler(EDS_CB *cb,
                              SaInvocationT invocation)
{
    saAmfResponse(cb->amf_hdl, invocation, SA_AIS_ERR_BAD_OPERATION);
    return NCSCC_RC_FAILURE;
}


/****************************************************************************
 * Name          : eds_amf_health_chk_callback
 *
 * Description   : This is the callback function which will be called 
 *                 when the AMF framework needs to health check for the component.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *                 checkType      - The type of healthcheck to be executed. 
 *
 * Return Values : None
 *
 * Notes         : At present we are just support a simple liveness check.
 *****************************************************************************/
void
eds_amf_health_chk_callback (SaInvocationT invocation,
                             const SaNameT *compName,
                             SaAmfHealthcheckKeyT *checkType)
{
   EDS_CB *eds_cb;
   SaAisErrorT error = SA_AIS_OK;
   
   if (NULL == (eds_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl)))
   {
       m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
       return;      
   } 
   else
   {
      saAmfResponse(eds_cb->amf_hdl,invocation, error);
      ncshm_give_hdl(gl_eds_hdl);
   }
   return;
}

/****************************************************************************
 * Name          : eds_amf_CSI_set_callback
 *
 * Description   : AMF callback function called 
 *                 when there is any change in the HA state.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The 
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *                 haState        - The new HA state to be assumeb by the 
 *                                  component service instance identified by 
 *                                  csiName.
 *                 csiDescriptor - This will indicate whether or not the 
 *                                  component service instance for 
 *                                  ativeCompName went through quiescing.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
eds_amf_CSI_set_callback (SaInvocationT invocation,
                          const SaNameT  *compName,
                          SaAmfHAStateT  new_haState,
                          SaAmfCSIDescriptorT csiDescriptor)
{
   EDS_CB             *eds_cb;
   SaAisErrorT error  = SA_AIS_OK;
   SaAmfHAStateT      prev_haState;
   NCS_BOOL           role_change = TRUE;
   uns32              rc = NCSCC_RC_SUCCESS;
    
   if (NULL == (eds_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl)))
   {
       m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
       return;
   }
   else
   {
      /*
       *  Handle Active to Active role change.
       */
      m_LOG_EDSV_S(EDS_AMF_RCVD_CSI_SET_CLBK,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,eds_cb->ha_state,__FILE__,__LINE__,new_haState);
      prev_haState = eds_cb->ha_state;
     
      /* Invoke the appropriate state handler routine */
#if 0
      error = eds_stateHandler[VALIDATE_STATE(eds_cb->ha_state,new_haState)](eds_cb,invocation);
#endif 
      switch(new_haState)
      { 
        case SA_AMF_HA_ACTIVE :
             eds_cb->mds_role = V_DEST_RL_ACTIVE;
             m_NCS_CONS_PRINTF("I AM HA AMF ACTIVE\n");
             break;
        case SA_AMF_HA_STANDBY :
             eds_cb->mds_role = V_DEST_RL_STANDBY;
             m_NCS_CONS_PRINTF("I AM HA AMF STANDBY\n");
             break;
        case SA_AMF_HA_QUIESCED :
             eds_quiesced_state_handler(eds_cb,invocation);
             break;
        case SA_AMF_HA_QUIESCING : 
             eds_quiescing_state_handler(eds_cb,invocation);
             break;
        default :
             m_LOG_EDSV_S(EDS_AMF_CSI_SET_HA_STATE_INVALID,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,eds_cb->ha_state,__FILE__,__LINE__,new_haState);
             eds_invalid_state_handler(eds_cb,invocation);
      } 
                     
      if(new_haState == SA_AMF_HA_QUIESCED)
        return ;

      
      /* Update control block */
      eds_cb->ha_state = new_haState;
      
      if (eds_cb->csi_assigned == FALSE)
      {
          eds_cb->csi_assigned = TRUE;
         /* We shall open checkpoint only once in our life time. currently doing at lib init  */
      }
      else if ((new_haState == SA_AMF_HA_ACTIVE) || (new_haState == SA_AMF_HA_STANDBY))
      {  /* It is a switch over */
         eds_cb->ckpt_state = COLD_SYNC_IDLE;
       }
       if((prev_haState == SA_AMF_HA_ACTIVE) && (new_haState == SA_AMF_HA_ACTIVE))
       {
           role_change = FALSE;
       }

       /*
        * Handle Stby to Stby role change.
        */
       if((prev_haState == SA_AMF_HA_STANDBY) && (new_haState == SA_AMF_HA_STANDBY))
       {
          role_change = FALSE;
       }

       if(role_change == TRUE)
       { 
           if ( (rc = eds_mds_change_role(eds_cb) ) != NCSCC_RC_SUCCESS )
           {
               m_LOG_EDSV_S(EDS_MDS_CSI_ROLE_CHANGE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,eds_cb->mds_role);
               m_NCS_CONS_PRINTF(" eds_amf_CSI_set_callback: MDS role change to %d FAILED\n",eds_cb->mds_role);
           }
           else
           {
               m_LOG_EDSV_S(EDS_MDS_CSI_ROLE_CHANGE_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__,eds_cb->mds_role);
               m_NCS_CONS_PRINTF(" eds_amf_CSI_set_callback: MDS role change to %d SUCCESS\n",eds_cb->mds_role);
           }
       }
       /* Inform MBCSV of HA state change */
       if (NCSCC_RC_SUCCESS != (error=eds_mbcsv_change_HA_state(eds_cb)))
       {
           m_LOG_EDSV_S(EDS_MBCSV_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,error,__FILE__,__LINE__,new_haState);
           m_NCS_CONS_PRINTF(" eds_amf_CSI_set_callback: MBCSV state change to (in CSI AssignMent Cbk): %d FAILED \n",eds_cb->ha_state);
       }
       else
       {
           m_NCS_CONS_PRINTF(" eds_amf_CSI_set_callback: MBCSV state change to (in CSI Assignment Cbk): %d SUCCESS \n",eds_cb->ha_state);
           m_LOG_EDSV_S(EDS_MBCSV_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,1,__FILE__,__LINE__,1);
       }
       /* Send the response to AMF */ 
       saAmfResponse(eds_cb->amf_hdl, invocation, SA_AIS_OK);  
        
       /* Do some OverHead Processing At New Active For EDA Down Events Cleanup if those reccords are present*/ 
       if( eds_cb->ha_state == SA_AMF_HA_ACTIVE )
       { 
           EDA_DOWN_LIST *eda_down_rec =NULL;
           EDA_DOWN_LIST *temp_eda_down_rec =NULL;
           
           eda_down_rec = eds_cb->eda_down_list_head;
           while(eda_down_rec)
           {
             /*Remove the EDA DOWN REC from the EDA_DOWN_LIST */
             /* Free the EDA_DOWN_REC */
             /* Remove this EDA entry from our processing lists */
             temp_eda_down_rec = eda_down_rec;
             eds_remove_regid_by_mds_dest(eds_cb, eda_down_rec->mds_dest);
             eda_down_rec = eda_down_rec->next;
             m_MMGR_FREE_EDA_DOWN_LIST(temp_eda_down_rec);
           }     
           eds_cb->eda_down_list_head  = NULL;     
           eds_cb->eda_down_list_tail  = NULL;     
       }
       /*Give handles */
       ncshm_give_hdl(gl_eds_hdl);
   }   
}/* End CSI set callback */


/****************************************************************************
 * Name          : eds_amf_comp_terminate_callback
 *
 * Description   : This is the callback function which will be called 
 *                 when the AMF framework needs to terminate EDS. This does
 *                 all required to destroy EDS(except to unregister from AMF)
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *
 * Return Values : None
 *
 * Notes         : At present we are just support a simple liveness check.
 *****************************************************************************/
void
eds_amf_comp_terminate_callback(SaInvocationT invocation,
                                const SaNameT *compName)
{
   EDS_CB   *eds_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_LOG_EDSV_S(EDS_AMF_TERMINATE_CALLBACK_CALLED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,1,__FILE__,__LINE__,1);
   if (NULL == (eds_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl)))
   {
       m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
       return;      
   } 
   else   
   {
      saAmfResponse(eds_cb->amf_hdl,invocation, error);
      /* Clean up all internal structures */
      eds_remove_reglist_entry(eds_cb, 0, TRUE);
      
      /* Destroy the cb */
      eds_cb_destroy(eds_cb);
      /* Give back the handle */
      ncshm_give_hdl(gl_eds_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl); 
      
      /* Detach from IPC */
      m_NCS_IPC_DETACH(&eds_cb->mbx,NULL,eds_cb);
      
      /* Disconnect from MDS */
      eds_mds_finalize(eds_cb);
      m_MMGR_FREE_EDS_CB(eds_cb);
      sleep(1);
      exit(0);
      
   }

   return;
}


/****************************************************************************
 * Name          : eds_amf_csi_rmv_callback
 *
 * Description   : This callback routine is invoked by AMF during a
 *                 CSI set removal operation. 
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 compName       - A pointer to the name of the component
 *                                  whose readiness stae the Availability
 *                                  Management Framework is setting.
 *                 csiName        - A const pointer to csiName
 *                 csiFlags       - csi Flags
 * Return Values : None 
 *****************************************************************************/
void
eds_amf_csi_rmv_callback(SaInvocationT invocation,
                         const SaNameT *compName,
                         const SaNameT *csiName,
                         const SaAmfCSIFlagsT csiFlags)
{
   EDS_CB   *eds_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_LOG_EDSV_S(EDS_REMOVE_CALLBACK_CALLED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,1,__FILE__,__LINE__,1);
   if (NULL == (eds_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl)))
   {
       m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
       return;      
   }
   else
   {
      saAmfResponse(eds_cb->amf_hdl,invocation, error);
      ncshm_give_hdl(gl_eds_hdl);
   }
   return;
}

/*****************************************************************************\
 *  Name:          eds_healthcheck_start                           * 
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
eds_healthcheck_start(EDS_CB *eds_cb)
{
   SaAisErrorT    error;
   SaAmfHealthcheckKeyT  Healthy;
   int8*       health_key=0;
   
    if (eds_cb->healthCheckStarted == TRUE)
    {
        return NCSCC_RC_SUCCESS;
    }

   /** start the AMF health check **/
   m_NCS_MEMSET(&Healthy,0,sizeof(Healthy));
   health_key = m_NCS_OS_PROCESS_GET_ENV_VAR("EDSV_ENV_HEALTHCHECK_KEY");
   if(health_key == NULL)
   {
      m_NCS_STRCPY(Healthy.key,"E5F6");
      /* Log it */
   }
   else
   {
      m_NCS_STRCPY(Healthy.key,health_key);
   }
   Healthy.keyLen=strlen(Healthy.key);

   error = saAmfHealthcheckStart(eds_cb->amf_hdl,&eds_cb->comp_name,&Healthy,
      SA_AMF_HEALTHCHECK_AMF_INVOKED,SA_AMF_COMPONENT_FAILOVER);
   
   
   if (error != SA_AIS_OK)
   {
       return NCSCC_RC_FAILURE;
   }
   else
   {
      eds_cb->healthCheckStarted = TRUE;
      return NCSCC_RC_SUCCESS;
   }
}
  
/****************************************************************************
 * Name          : eds_amf_init
 *
 * Description   : EDS initializes AMF for invoking process and registers 
 *                 the various callback functions.
 *
 * Arguments     : EDS_CB - EDS control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
eds_amf_init(EDS_CB *eds_cb)
{

   SaAmfCallbacksT amfCallbacks;
   SaVersionT      amf_version;   
   uns32           rc = NCSCC_RC_SUCCESS;
   char    comp_name[256] = {0};
   char    compfilename[EDS_COMP_FILE_NAME_LEN] = {0};
   FILE    *fp;


   /* Read the component name file now, AMF should have populated it by now */
   m_NCS_OS_ASSERT(sprintf(compfilename, "%s", m_EDS_COMP_NAME_FILE) < sizeof(compfilename));

   fp = fopen(compfilename, "r");/*/var/opt/opensaf/ncs_eds_comp_name */
   if(fp == NULL)
   {   
      m_LOG_EDSV_S(EDS_AMF_COMP_FILE_OPEN_FOR_READ_FAIL,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      m_NCS_CONS_PRINTF("  eds_amf_init: eds_amf_init: Unable to open compname file FAILED\n");
      return NCSCC_RC_FAILURE; 
   }
   if(fscanf(fp, "%s", comp_name) != 1)
   {
      fclose(fp);
      m_LOG_EDSV_S(EDS_AMF_COMP_FILE_READ_FAIL,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      m_NCS_CONS_PRINTF("  eds_amf_init: Unable to retrieve component name FAILED\n");
      return NCSCC_RC_FAILURE; 
   }

   if (setenv("SA_AMF_COMPONENT_NAME", comp_name, 1) == -1)
   {
      fclose(fp);
      m_LOG_EDSV_S(EDS_AMF_ENV_NAME_SET_FAIL,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      m_NCS_CONS_PRINTF("  eds_amf_init: Unable to set SA_AMF_COMPONENT_NAME enviroment variable\n");
      return NCSCC_RC_FAILURE; 
   }
   fclose(fp);
   fp = NULL;

   /* Initialize amf callbacks */
   m_NCS_MEMSET(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   amfCallbacks.saAmfHealthcheckCallback = eds_amf_health_chk_callback;
   amfCallbacks.saAmfCSISetCallback      = eds_amf_CSI_set_callback;
   amfCallbacks.saAmfComponentTerminateCallback
                                         = eds_amf_comp_terminate_callback;
   amfCallbacks.saAmfCSIRemoveCallback   = eds_amf_csi_rmv_callback;

   m_EDSV_GET_AMF_VER(amf_version);

   /*Initialize the amf library */

   rc = saAmfInitialize(&eds_cb->amf_hdl, &amfCallbacks, &amf_version);

   if (rc != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("  eds_amf_init: saAmfInitialize() AMF initialization FAILED\n");
      m_LOG_EDSV_S(EDS_AMF_INIT_ERROR,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_EDSV_S(EDS_AMF_INIT_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   
   /* Obtain the amf selection object to wait for amf events */
   if (SA_AIS_OK != (rc = saAmfSelectionObjectGet(eds_cb->amf_hdl, &eds_cb->amfSelectionObject)))
   {
      m_NCS_CONS_PRINTF("  eds_amf_init: saAmfSelectionObjectGet() FAILED\n");
      m_LOG_EDSV_S(EDS_AMF_GET_SEL_OBJ_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      return rc;
   }
   m_LOG_EDSV_S(EDS_AMF_GET_SEL_OBJ_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);

   /* get the component name */  
   rc = saAmfComponentNameGet(eds_cb->amf_hdl,&eds_cb->comp_name);
   if (rc != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("  eds_amf_init: saAmfComponentNameGet() FAILED\n");
      m_LOG_EDSV_S(EDS_AMF_COMP_NAME_GET_FAIL,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_EDSV_S(EDS_AMF_COMP_NAME_GET_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);

   return rc;

} /*End eds_amf_init */


/**************************************************************************
 Function: eds_amf_register

 Purpose:  Function which registers EDS with AMF.  

 Input:    Pointer to the EDS control block. 

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  Here we call eds_amf_init after reading the component name file and
         setting the environment varaiable in our own context.
         Proceed to register with AMF, since it has come up. 
**************************************************************************/
uns32 eds_amf_register(EDS_CB *eds_cb)
{

   SaAisErrorT    error;
   uns32          rc = NCSCC_RC_SUCCESS;

   m_NCS_LOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);

   /* Initialize amf framework for hc interface*/
   if ((rc = eds_amf_init(eds_cb) ) != NCSCC_RC_SUCCESS)
   {
      m_LOG_EDSV_S(EDS_AMF_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
      m_NCS_CONS_PRINTF(" eds_amf_register: eds_amf_init() FAILED\n");
      return NCSCC_RC_FAILURE;
   }
   m_LOG_EDSV_S(EDS_AMF_INIT_OK,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,1,__FILE__,__LINE__,1);
   
   m_NCS_CONS_PRINTF(" eds_amf_register: eds_amf_init() SUCCESS\n");

      /* register EDS component with AvSv */
   error = saAmfComponentRegister(eds_cb->amf_hdl, 
                     &eds_cb->comp_name,
                     (SaNameT*)NULL);
   if (error != SA_AIS_OK)
   {
      m_LOG_EDSV_S(EDS_AMF_COMP_REGISTER_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,error,__FILE__,__LINE__,0);
      m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
      m_NCS_CONS_PRINTF(" eds_amf_register: saAmfComponentRegister() FAILED\n");
      return NCSCC_RC_FAILURE;
   }
   m_LOG_EDSV_S(EDS_AMF_COMP_REGISTER_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,1,__FILE__,__LINE__,1);
   m_NCS_CONS_PRINTF(" eds_amf_register: saAmfComponentRegister() SUCCESS \n");
   if (NCSCC_RC_SUCCESS != (rc = eds_healthcheck_start(eds_cb) ) )
   {
      m_LOG_EDSV_S(EDS_AMF_HLTH_CHK_START_FAIL,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      m_NCS_CONS_PRINTF(" eds_amf_register: saAmfHealthcheckStart FAILED \n");
      m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
      return NCSCC_RC_FAILURE;
   }
   m_LOG_EDSV_S(EDS_AMF_HLTH_CHK_START_DONE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,1,__FILE__,__LINE__,1);
   m_NCS_CONS_PRINTF(" eds_amf_register: saAmfHealthcheckStart SUCCESS\n");
   m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);

  return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
 *  Name:          eds_amf_sigusr1_handler                                    * 
 *                                                                            *
 *  Description:   Raise selection object to register with AMF      *
 *                                                                            *
 *  Arguments:     int i_sug_num -  Signal received                           *
 *                                                                            * 
 *  Returns:       Nothing                                                    *  
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* signal from AMF, time to register with AMF */ 
void
eds_amf_sigusr1_handler(int i_sig_num)
{
   EDS_CB* eds_cb; 
   if (NULL == (eds_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl)))
   {  
       m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
       m_NCS_CONS_PRINTF("eds_amf_sigusr1_handler : eds_cb take FAILED...... \n");
       return;
   }
   else
   {
      /* uninstall the signal handler */
      m_NCS_SIGNAL(SIGUSR1, SIG_IGN); 
      m_NCS_SEL_OBJ_IND(eds_cb->sighdlr_sel_obj);
      ncshm_give_hdl(gl_eds_hdl);
   }
   return;
}


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

/**************************************************************************
 * Function: eds_clm_init
 *
 * Purpose:  Function to register with the CLM service (Currently B01.01). 
 * Input  :    None
 *
 * Returns: SaAisErrorT  type
 *
 * Notes  : The CLM registration is done only after AMF registration is
 * done and not during the pre-amf period.
 **************************************************************************/
                                                                                             
SaAisErrorT eds_clm_init(EDS_CB *cb)
{
   SaVersionT         clm_version;
   SaClmCallbacksT  clm_cbk;
   SaClmClusterNodeT  cluster_node;
   SaClmClusterNotificationBufferT  notify_buff;
   SaAisErrorT        rc = SA_AIS_OK;
   SaTimeT            timeout = EDSV_CLM_TIMEOUT;
     
   m_NCS_OS_MEMSET(&clm_version,0,sizeof(SaVersionT));
   m_NCS_OS_MEMSET(&cluster_node,0,sizeof(SaClmClusterNodeT));
   m_NCS_OS_MEMSET(&notify_buff,0,sizeof(SaClmClusterNotificationBufferT));

   /* Fill version */
   m_EDSV_GET_CLM_VER(clm_version);

   clm_cbk.saClmClusterNodeGetCallback = NULL;
   clm_cbk.saClmClusterTrackCallback = eds_clm_cluster_track_cbk;

   /* Say Hello */ 
   rc = saClmInitialize (&cb->clm_hdl,&clm_cbk,&clm_version);
   if (rc != SA_AIS_OK)
   {
      m_LOG_EDSV_S(EDS_CLM_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      return rc;
   }
   /* Get the FD */
   if (SA_AIS_OK != (rc = saClmSelectionObjectGet(cb->clm_hdl, &cb->clm_sel_obj)))
   {
      m_LOG_EDSV_S(EDS_CLM_SEL_OBJ_GET_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
     /* Log it */
     return rc;
   }

   rc = saClmClusterNodeGet (cb->clm_hdl,SA_CLM_LOCAL_NODE_ID,timeout,&cluster_node);
   if (rc != SA_AIS_OK)
   {
      m_LOG_EDSV_S(EDS_CLM_NODE_GET_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      return rc;
   }
   /* Get my node_id. Not sure if i need this at all */
   cb->node_id = cluster_node.nodeId;
   
   notify_buff.notification = NULL;
   rc  = saClmClusterTrack (cb->clm_hdl,(SA_TRACK_CURRENT|SA_TRACK_CHANGES),&notify_buff);
   if (rc != SA_AIS_OK)
      m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);

  return rc;

}

/**************************************************************************
 * Function: eds_clm_cluster_track_cbk
 *
 * Purpose: The cluster track callback invoked by CLM whenever thereis a 
 *          change in the status of a node in the CLUSTER. 
 *          This callback is invoked whenever any of the following events
 *          occur on a cluster node:
 *          - SA_CLM_NODE_JOINED
 *          - SA_CLM_NODE_EXITED
 *          - SA_CLM_NODE_NO_CHANGE
 *          - SA_CLM_NODE_RECONFIGURED
 * Input  : none 
 * Output : NotificationBuffer that contains information of all nodes in the 
 *          cluster and the corresponding change that occured on that node.
 * Returns:   None 
 *
 * Notes  : Upon invocation of this callback, if a node joined or left the cluster,
 * EDS shall send an update to all the EDAs on that particular node.
 * This function updates the nodes list (in the cluster) accordingly
 * This list is checked while processing init messages from EDAs. All current 
 * member nodes of the cluster are maintained in this list. SA_AIS_ERR_UNAVAILABLE 
 * is retured for an saEvtInitialize() originating from a non member node.
 **************************************************************************/

void eds_clm_cluster_track_cbk (const SaClmClusterNotificationBufferT *notificationBuffer, \
                                                SaUint32T numberOfMembers, SaAisErrorT error)
{
   EDS_CB                 *cb = NULL;
   NODE_ID                node_id;
   SaClmClusterChangesT   cluster_change;
   uns32                  counter = 0;

   if (error == SA_AIS_OK)
   {
      /* Get EDS CB Handle. */
      if (NULL == (cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl)))
      {
         m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_CBK_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,error,__FILE__,__LINE__,1);
         return;
      }

      if (notificationBuffer != NULL)
      {
         for (counter=0;counter < notificationBuffer->numberOfItems;counter++)
         {
             /* Send the update to EDA clients */
             cluster_change = notificationBuffer->notification[counter].clusterChange;
             if ((cluster_change == SA_CLM_NODE_JOINED) || (cluster_change == SA_CLM_NODE_LEFT))
             {
                 node_id = notificationBuffer->notification[counter].clusterNode.nodeId;
                 update_node_db(cb, node_id, cluster_change);
                 /* Send to all EDAs on node_id */
                 m_NCS_CONS_PRINTF("Sending ClusterChange Update to Agents on NodeId = %d\n",node_id);
                 send_clm_status_change(cb, cluster_change, node_id);
             }
          }
      }
 
      ncshm_give_hdl(gl_eds_hdl);
      m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_CBK_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,error,__FILE__,__LINE__,0);
   }
   else
       m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_CBK_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,error,__FILE__,__LINE__,1);

   return;
 
}

/**************************************************************************
 * Function: update_node_db 
 *
 * Purpose: The cluster track callback invoked by CLM whenever thereis a
 *          change in the status of a node in the CLUSTER.
 * Input  : none
 * Output : NotificationBuffer that contains information of all nodes in the
 *          cluster and the corresponding change that occured on that node.
 * Returns:   None
 *
 * Notes  : Upon invocation of this callback, if a node joined or left the cluster,
 *  EDS shall send an update to all the EDAs on that particular node.
 **************************************************************************/

void update_node_db(EDS_CB *cb, NODE_ID node_id, SaClmClusterChangesT cluster_change)
{
   NODE_INFO *prev= NULL, *cur = NULL, *new = NULL;

   prev=cur=cb->cluster_node_list; /* Get the list head */
   
   /* If list is null, there is nothing to delete! */
   if ((cur == NULL) && (cluster_change == SA_CLM_NODE_LEFT))
      return;
   /* See if node is already in the Cluster */
   while (cur != NULL)
   {
         if (node_id == cur->node_id)
         {
            if (cluster_change == SA_CLM_NODE_JOINED)
               return;
            else if (cluster_change == SA_CLM_NODE_LEFT)
            {
               prev = cur->next;
               m_MMGR_FREE_CLUSTER_NODE_LIST(cur);
               return;
            }
         }
      prev = cur;
      cur = cur->next;
   }
   /* Its a new node to the cluster, add to the list */
   if (cluster_change == SA_CLM_NODE_JOINED)
   {
     /* Add a new node to the list after prev*/
     new = m_MMGR_ALLOC_CLUSTER_NODE_LIST(sizeof(NODE_INFO));
     m_NCS_MEMSET(new,0,sizeof(NODE_INFO));
     new->node_id = node_id;
     new->next = NULL;
     if (prev == NULL)
        prev = new; /* first node to join */
     else
        prev->next = new; /* Add to the end */
   }
}

/**************************************************************************
 * Function: send_clm_status_change 
 *
 * Purpose: The cluster track callback invoked by CLM whenever thereis a
 *          change in the status of a node in the CLUSTER.
 * Input  : pointer to eds CB, cluster change and the node_id for this change.
 * Returns:   None
 * Notes  : Sends an asynch update message to all EDAs residing on the nodes
 * that have had a change in their cluster membership ((Re)joined/left).
 **************************************************************************/

void send_clm_status_change(EDS_CB *cb, SaClmClusterChangesT cluster_change, NODE_ID node_id)
{
   EDSV_MSG       msg;
   EDA_REG_REC    *reg_rec = NULL;
   uns32          rc = NCSCC_RC_SUCCESS;

   m_EDS_EDSV_CLM_STATUS_CB_MSG_FILL(msg,cluster_change);
   reg_rec=(EDA_REG_REC *)ncs_patricia_tree_getnext(&cb->eda_reg_list,(uns8 *)0);
   while (reg_rec != NULL)
   {
      if (node_id == (m_EDS_GET_NODE_ID_FROM_ADEST(reg_rec->eda_client_dest)))
      {
         rc = eds_mds_msg_send(cb, &msg, &reg_rec->eda_client_dest, NULL,
                                         MDS_SEND_PRIORITY_MEDIUM);
         if (rc != NCSCC_RC_SUCCESS)
            m_LOG_EDSV_S(EDS_CLUSTER_CHANGE_NOTIFY_SEND_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,1,__FILE__,__LINE__,1);
        else
            m_NCS_CONS_PRINTF("ClusterNodeUpdate = %d send to %d Success\n",cluster_change, node_id);
      }
      reg_rec=(EDA_REG_REC *)ncs_patricia_tree_getnext(&cb->eda_reg_list,(uns8*)&reg_rec->reg_id_Net);
   }
   
}

/**************************************************************************
 * Function: is_node_a_member
 *
 * Purpose : This function walks through the cluster_node_list to see if
 * the input node_id is present in the list or not.
 *
 * Input   : pointer to eds CB, the node_id.
 * Returns : TRUE/FALSE. 
 **************************************************************************/

NCS_BOOL is_node_a_member (EDS_CB *cb, NODE_ID node_id)
{
  NODE_INFO *tmp_rec=NULL;
  tmp_rec=cb->cluster_node_list;
  while ( tmp_rec != NULL)
  {
     if(tmp_rec->node_id == node_id)
        return TRUE;
     tmp_rec = tmp_rec->next;
  }
  return FALSE;
}
