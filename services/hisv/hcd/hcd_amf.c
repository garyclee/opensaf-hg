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
  FILE NAME: hcd_amf.c

  DESCRIPTION: HCD AMF callback routines.

  FUNCTIONS INCLUDED in this module:
******************************************************************************/


#include "hcd.h"
#include "hcd_amf.h"



/****************************************************************************
 * Name          : hcd_amf_CSI_set_callback
 *
 * Description   : This function SAF callback function which will be called
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
 *                 csiName        - A pointer to the name of the new component
 *                                  service instance to be supported by the
 *                                  component or of an alreadt supported
 *                                  component service instance whose HA state
 *                                  is to be changed.
 *                 csiFlags       - A value of the choiceflag type which
 *                                  indicates whether the HA state change must
 *                                  be applied to a new component service
 *                                  instance or to all component service
 *                                  instance currently supported by the
 *                                  component.
 *                 haState        - The new HA state to be assumeb by the
 *                                  component service instance identified by
 *                                  csiName.
 *                 activeCompName - A pointer to the name of the component that
 *                                  currently has the active state or had the
 *                                  active state for this component serivce
 *                                  insance previously.
 *                 transitionDesc - This will indicate whether or not the
 *                                  component service instance for
 *                                  ativeCompName went through quiescing.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
hcd_amf_CSI_set_callback (SaInvocationT invocation,
                          const SaNameT  *compName,
                          SaAmfHAStateT  haState,
                          SaAmfCSIDescriptorT csiDescriptor)

{
   HCD_CB            *hcd_cb;
   HAM_CB            *ham_cb;
   V_DEST_RL         mds_role;

   hcd_cb = m_HISV_HCD_RETRIEVE_HCD_CB;
   if (hcd_cb != NULL)
   {
      saAmfResponse(hcd_cb->amf_hdl, invocation, SA_AIS_OK);
      /* Call into MDS to set the role TBD. */
      if (haState == SA_AMF_HA_ACTIVE)
      {
         mds_role = V_DEST_RL_ACTIVE;
      } else
      if (haState == SA_AMF_HA_QUIESCED)
      {
         mds_role = V_DEST_RL_QUIESCED;
      } else
      {
         mds_role = V_DEST_RL_STANDBY;
      }
      /** set the CB's anchor value */
      hcd_cb->mds_role = mds_role;
      if (NULL != (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
      {
         ham_cb->mds_role = mds_role;
         ham_mds_change_role(ham_cb);
         ncshm_give_hdl(gl_ham_hdl);
      }
      if (haState != SA_AMF_HA_QUIESCED)
      {
         /* In case we need to rediscover, set this to 1 */
         hcd_cb->args->rediscover = 1;
         /* Update control block */
         hcd_cb->ha_state = haState;
      }

      
      if (haState == SA_AMF_HA_STANDBY)
      {
          HSM_CB *local_hsm_cb;
          local_hsm_cb = (HSM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hsm_hdl);
          saHpiUnsubscribe(local_hsm_cb->args->session_id);
          saHpiSessionClose(local_hsm_cb->args->session_id);
#ifdef HPI_A
          saHpiFinalize();
#endif
          local_hsm_cb->args->session_valid = 0;
          ncshm_give_hdl(gl_hsm_hdl);
      }

      /*Give handles */
      m_HISV_HCD_GIVEUP_HCD_CB;
   }
   else
      m_NCS_CONS_PRINTF("HCD Control block found NULL in CSI Assignment\n");
}


/****************************************************************************
 * Name          : hcd_amf_health_chk_callback
 *
 * Description   : This function SAF callback function which will be called
 *                 when the AMF framework needs to health for the component.
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
hcd_amf_health_chk_callback (SaInvocationT invocation,
                             const SaNameT *compName,
                             SaAmfHealthcheckKeyT *checkType)
{
   HCD_CB *hcd_cb;
   SaAisErrorT error = SA_AIS_OK;

   hcd_cb = m_HISV_HCD_RETRIEVE_HCD_CB;

   if (hcd_cb != NULL)
   {
      saAmfResponse(hcd_cb->amf_hdl,invocation, error);
      m_HISV_HCD_GIVEUP_HCD_CB;
   }
   return;
}

/****************************************************************************
 * Name          : hcd_amf_comp_terminate_callback
 *
 * Description   : This function SAF callback function which will be called
 *                 when the AMF framework needs to terminate HISV. This does
 *                 all required to destroy HISV(except to unregister from AMF)
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
hcd_amf_comp_terminate_callback(SaInvocationT invocation,
                                const SaNameT *compName)
{
   HCD_CB *hcd_cb;
   SaAisErrorT error = SA_AIS_OK;

   hcd_cb = m_HISV_HCD_RETRIEVE_HCD_CB;

   if (hcd_cb != NULL)
   {
      saAmfResponse(hcd_cb->amf_hdl,invocation, error);


      m_LOG_HISV_SVC_PRVDR(HCD_AMF_RCVD_HEALTHCHK,NCSFL_SEV_INFO);
   }
   m_NCS_TASK_SLEEP(100000000);
   exit(0);

   return;
}


/****************************************************************************
 * Name          : hcd_amf_csi_rmv_callback
 *
 * Description   : TBD
 *
 *
 * Return Values : None
 *****************************************************************************/
void
hcd_amf_csi_rmv_callback(SaInvocationT invocation,
                         const SaNameT *compName,
                         const SaNameT *csiName,
                         SaAmfCSIFlagsT *csiFlags)
{
   HCD_CB *hcd_cb;
   SaAisErrorT error = SA_AIS_OK;

   hcd_cb = m_HISV_HCD_RETRIEVE_HCD_CB;

   if (hcd_cb != NULL)
   {
      saAmfResponse(hcd_cb->amf_hdl,invocation, error);
      m_HISV_HCD_GIVEUP_HCD_CB;
      m_LOG_HISV_SVC_PRVDR(HCD_AMF_RCVD_HEALTHCHK,NCSFL_SEV_INFO);
   }
   return;
}

/****************************************************************************
 * Name          : hcd_amf_init
 *
 * Description   : HCD initializes AMF for involking process and registers
 *                 the various callback functions.
 *
 * Arguments     : hcd_cb  - Ifsv control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
hcd_amf_init (HCD_CB *hcd_cb)
{
   SaAmfCallbacksT amfCallbacks;
   SaVersionT      amf_version;
   SaAisErrorT        error;
   uns32           res = NCSCC_RC_SUCCESS;

   memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   amfCallbacks.saAmfHealthcheckCallback = hcd_amf_health_chk_callback;
   amfCallbacks.saAmfCSISetCallback      = hcd_amf_CSI_set_callback;
   amfCallbacks.saAmfComponentTerminateCallback
                                         = hcd_amf_comp_terminate_callback;
   amfCallbacks.saAmfCSIRemoveCallback   = (SaAmfCSIRemoveCallbackT)hcd_amf_csi_rmv_callback;

   m_HISV_GET_AMF_VER(amf_version);

   error = saAmfInitialize(&hcd_cb->amf_hdl, &amfCallbacks, &amf_version);

   if (error != SA_AIS_OK)
   {
      m_LOG_HISV_SVC_PRVDR(HCD_AMF_INIT_ERROR,NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   /* get the component name */
   error = saAmfComponentNameGet(hcd_cb->amf_hdl,&hcd_cb->comp_name);
   if (error != SA_AIS_OK)
   {
      m_LOG_HISV_SVC_PRVDR(HCD_AMF_INIT_ERROR,NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }

   m_LOG_HISV_SVC_PRVDR(HCD_AMF_INIT_SUCCESS,NCSFL_SEV_INFO);
   return (res);
}


/****************************************************************************
 * Name          : hisv_hcd_health_check
 *
 * Description   : This function is used to do the health check of HAM, ShIM
 *                 and HSM and inform the status to AMF.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32
hisv_hcd_health_check(SYSF_MBX *mbx)
{
   SaAisErrorT    amf_error;
   SaAmfHealthcheckKeyT  Healthy;
   int8*   health_key, rc;
   NCS_SEL_OBJ    mbx_fd;
   NCS_SEL_OBJ_SET     all_sel_obj;
   NCS_SEL_OBJ         amf_ncs_sel_obj;
   NCS_SEL_OBJ         high_sel_obj;
   SaSelectionObjectT  amf_sel_obj;
   SaAmfHandleT        amf_hdl;
   HSM_CB *hsm_cb;
   SIM_CB *sim_cb;
   HAM_CB *ham_cb;
   HCD_CB *hcd_cb;

   /* retrieve HAM CB */
   ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl);
      /* retrieve HSM CB */
   hsm_cb = (HSM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hsm_hdl);
      /* retrieve HSM CB */
   sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
      /* retrieve HAM CB */
   hcd_cb = (HCD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl);

   if ((mbx == NULL) || (ham_cb == NULL) || (hsm_cb == NULL)
                     || (hcd_cb == NULL) || (sim_cb == NULL))
   {
      m_LOG_HISV_HEADLINE(HCD_TAKE_HANDLE_FAILED, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
      /** start the AMF health check **/
   memset(&Healthy,0,sizeof(Healthy));
   health_key = m_NCS_OS_PROCESS_GET_ENV_VAR("HISV_ENV_HEALTHCHECK_KEY");
   if(health_key == NULL)
   {
      strcpy(Healthy.key,"F6C7");
      m_LOG_HISV_HEADLINE(HCD_HEALTH_KEY_DEFAULT_SET, NCSFL_SEV_INFO);
   }
   else
   {
      strcpy(Healthy.key,health_key);
   }
   Healthy.keyLen=m_NCS_STRLEN(Healthy.key);

   amf_error = saAmfHealthcheckStart(hcd_cb->amf_hdl,&hcd_cb->comp_name,&Healthy,
      SA_AMF_HEALTHCHECK_AMF_INVOKED,SA_AMF_COMPONENT_RESTART);
   if (amf_error != SA_AIS_OK)
      m_LOG_HISV_SVC_PRVDR(HCD_AMF_HLTH_CHK_START_FAIL,NCSFL_SEV_CRITICAL);
   else
      m_LOG_HISV_SVC_PRVDR(HCD_AMF_HLTH_CHK_START_DONE,NCSFL_SEV_INFO);

   mbx_fd  = m_NCS_IPC_GET_SEL_OBJ(mbx);

   amf_hdl = hcd_cb->amf_hdl;
   ncshm_give_hdl(gl_hcd_hdl);

   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);

   amf_error = saAmfSelectionObjectGet(amf_hdl, &amf_sel_obj);

   if (amf_error != SA_AIS_OK)
   {
      m_LOG_HISV_SVC_PRVDR(HCD_AMF_SEL_OBJ_GET_ERROR,NCSFL_SEV_CRITICAL);
      /* return CB handles */
      ncshm_give_hdl(gl_ham_hdl);
      ncshm_give_hdl(gl_hsm_hdl);
      ncshm_give_hdl(gl_sim_hdl);
      return NCSCC_RC_FAILURE;
   }
   m_SET_FD_IN_SEL_OBJ(amf_sel_obj,amf_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);

   high_sel_obj = m_GET_HIGHER_SEL_OBJ(amf_ncs_sel_obj,mbx_fd);

   while ((rc = m_NCS_SEL_OBJ_SELECT(high_sel_obj,&all_sel_obj,0,0,0)))
   {
      if(rc == -1)
      {
         /* m_NCS_SEL_OBJ_SELECT unblocks and returns -1, if thread receives a signal */
         m_LOG_HISV_DTS_CONS("hisv_hcd_health_check: error m_NCS_SEL_OBJ_SELECT: CONT...\n");
         m_NCS_TASK_SLEEP(1000);
         continue;
      }
      /* process all the AMF messages */
      if (m_NCS_SEL_OBJ_ISSET(amf_ncs_sel_obj,&all_sel_obj))
      {
         HISV_EVT * hisv_evt, * evt;
         SIM_EVT  * sim_evt, * sim_rsp;
         HISV_MSG * msg;

         /* do the fd set for the select obj */
         m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
         m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);

         /* check the health of HAM */
         /* allocate the event */
         if (NULL == (hisv_evt = m_MMGR_ALLOC_HISV_EVT))
         {
            m_LOG_HISV_DTS_CONS("hisv_hcd_health_check: error m_MMGR_ALLOC_HISV_EVTs\n");
            continue;
         }
         /* for initial testing just dispatch */
         goto dispatch;
         msg = &hisv_evt->msg;
         msg->info.api_info.cmd = HISV_HAM_HEALTH_CHECK;

         /* send the request to HAM mailbox */
         if(m_NCS_IPC_SEND(&ham_cb->mbx, hisv_evt, NCS_IPC_PRIORITY_VERY_HIGH)
                           == NCSCC_RC_FAILURE)
         {
            m_LOG_HISV_DTS_CONS("hisv_hcd_health_check: failed to deliver msg on mail-box\n");
            m_MMGR_FREE_HISV_EVT(hisv_evt);
            continue;
         }
         /* expect the response from HAM */
         if (!m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
            continue;

         /* receive the message delivered on mailbox and process it */
         if (NULL != (evt = (HISV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt)))
         {
            m_MMGR_FREE_HISV_EVT(evt);
         }
         else
         {
            /* message null */
            m_LOG_HISV_DTS_CONS("hisv_hcd_health_check: received NULL message\n");
            continue;
         }

         /* check the health of SIM */
         /* allocate the event */
         if (NULL == (sim_evt = m_MMGR_ALLOC_SIM_EVT))
         {
            m_LOG_HISV_DTS_CONS("hisv_hcd_health_check: error m_MMGR_ALLOC_SIM_EVTs\n");
            continue;
         }
         sim_evt->evt_type = HCD_SIM_HEALTH_CHECK_REQ;

         /* send the request to SIM mailbox */
         if(m_NCS_IPC_SEND(&sim_cb->mbx, sim_evt, NCS_IPC_PRIORITY_VERY_HIGH)
                           == NCSCC_RC_FAILURE)
         {
            m_LOG_HISV_DTS_CONS("hisv_hcd_health_check: failed to deliver msg on mail-box\n");
            m_MMGR_FREE_SIM_EVT(sim_evt);
            continue;
         }
         /* expect the response from SIM */
         if (!m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
            continue;

         /* receive the message delivered on mailbox and process it */
         if (NULL != (sim_rsp = (SIM_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx, sim_rsp)))
         {
            m_MMGR_FREE_SIM_EVT(sim_rsp);
         }
         else
         {
            /* message null */
            m_LOG_HISV_DTS_CONS("hisv_hcd_health_check: received NULL message\n");
            continue;
         }

         /* check the health of HSM */
         m_NCS_SEM_TAKE(hcd_cb->p_s_handle);

dispatch:
         /* dispatch all the AMF pending function */
         amf_error = saAmfDispatch(amf_hdl, SA_DISPATCH_ALL);
         if (amf_error != SA_AIS_OK)
         {
            m_LOG_HISV_SVC_PRVDR(HCD_AMF_DISPATCH_ERROR,NCSFL_SEV_CRITICAL);
         }
         m_MMGR_FREE_HISV_EVT(hisv_evt);
      }
      /* do the fd set for the select obj */
      m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
      m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
   }
   /* return CB handles */
   ncshm_give_hdl(gl_ham_hdl);
   ncshm_give_hdl(gl_hsm_hdl);
   ncshm_give_hdl(gl_sim_hdl);
   return NCSCC_RC_SUCCESS;
}
