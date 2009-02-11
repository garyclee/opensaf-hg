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
  FILE NAME: IFD_SAF.C

  DESCRIPTION: IfD SAF callback routines.

  FUNCTIONS INCLUDED in this module:
  ifd_saf_readiness_state_callback ........... IfD SAF readiness callback.
  ifd_saf_CSI_set_callback.................... IfD SAF HA state callback.
  ifd_saf_pend_oper_confirm_callback.......... IfD SAF oper confirm callback.
  ifd_saf_health_chk_callback................. IfD SAF Health Check callback.  
  ifd_saf_CSI_rem_callback   ................. IfD SAF CSI remove callback.  
  ifd_saf_comp_terminate_callback............. IfD SAF Terminate comp callback.  

******************************************************************************/

#include "ifd.h"
uns32 ifd_process_quisced_state(IFSV_CB *ifsv_cb,SaInvocationT invocation,SaAmfHAStateT haState);

/****************************************************************************
 * Name          : ifd_saf_CSI_set_callback
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
void ifd_saf_CSI_set_callback(SaInvocationT invocation,
                         const SaNameT *compName,
                         SaAmfHAStateT haState,
                         SaAmfCSIDescriptorT csiDescriptor)
{
   IFSV_CB *ifsv_cb;
   SaAisErrorT error = SA_AIS_OK;
   IFSV_EVT_INIT_DONE_INFO init_done;
   V_DEST_RL   mds_role;
   NCSVDA_INFO vda_info;
   IFSV_EVT *evt = IFSV_NULL;
   NCS_IPC_PRIORITY priority = NCS_IPC_PRIORITY_HIGH;

   m_IFSV_GET_CB(ifsv_cb,compName->value);
   
   if (ifsv_cb != IFSV_NULL)   
   {
     if((ifsv_cb->ha_state == SA_AMF_HA_ACTIVE) &&
         (haState == SA_AMF_HA_QUIESCED))
     {
        m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFSV_HA_QUIESCED_MSG,haState,0);
        ifd_process_quisced_state(ifsv_cb,invocation,haState);
        m_IFSV_GIVE_CB(ifsv_cb);
        return;
     }

     if((ifsv_cb->ha_state == SA_AMF_HA_STANDBY) &&
         (haState == SA_AMF_HA_ACTIVE))
     {
        if(ifsv_cb->cold_or_warm_sync_on == TRUE)
        {
          /* If cold sync is going on then we will not send response
             to AMF, we will send the response after cold sync is complete. */
          m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFSV_HA_ACTIVE_IN_WARM_COLD_MSG,haState,0);
          ifsv_cb->amf_req_in_cold_syn.amf_req = TRUE; 
          ifsv_cb->amf_req_in_cold_syn.invocation = invocation; 
          m_IFSV_GIVE_CB(ifsv_cb);
          return;
        }
        if ((ifsv_cb->ifnd_rec_flush_tmr != IFSV_NULL ) &&
              (ifsv_cb->ifnd_rec_flush_tmr->is_active == TRUE))
        {
           m_NCS_CONS_PRINTF("Stopping the timer state : Stdby to Active\n");
           ifsv_tmr_stop(ifsv_cb->ifnd_rec_flush_tmr, ifsv_cb);
           m_MMGR_FREE_IFSV_TMR(ifsv_cb->ifnd_rec_flush_tmr);
           ifsv_cb->ifnd_rec_flush_tmr = IFSV_NULL;
        }
         
        m_NCS_CONS_PRINTF("State Change : Stdby to Active. Forming TMR EXP EVT \n"); 
        evt = m_MMGR_ALLOC_IFSV_EVT;
        m_NCS_MEMSET(evt,0,sizeof(IFSV_EVT));
        evt->vrid = ifsv_cb->vrid;
        evt->cb_hdl = ifsv_cb->cb_hdl;
        evt->type = IFD_EVT_TMR_EXP;
        evt->info.ifd_evt.info.tmr_exp.tmr_type = NCS_IFSV_IFD_IFND_REC_FLUSH_TMR;

        m_NCS_CONS_PRINTF("Successfully formed timer expiry event \n");

        /* Put the event in mail box */
        m_NCS_IPC_SEND(&ifsv_cb->mbx, evt, priority);

     }

      ifsv_cb->ha_state = haState;

      if ((ifsv_cb->init_done == FALSE) && 
         (ifsv_cb->ha_state != 0))
      {
         init_done.init_done = TRUE;
         /* sends a message to itself */
         ifd_evt_send((NCSCONTEXT)&init_done, IFD_EVT_INIT_DONE, ifsv_cb);
      }
      
      /** Change the MDS role **/
      if (ifsv_cb->ha_state == SA_AMF_HA_ACTIVE)
      {
         mds_role = V_DEST_RL_ACTIVE;
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFSV_HA_ACTIVE_MSG,haState,0);
         printf("****** IFD IN ACTIVE STATE********* \n");
      } else
      {
         mds_role = V_DEST_RL_STANDBY;
         m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFSV_HA_STDBY_MSG,haState,0);
         printf("****** IFD IN STANDBY STATE********* \n");
      }
      m_NCS_OS_MEMSET(&vda_info, 0, sizeof(vda_info));
      
      vda_info.req = NCSVDA_VDEST_CHG_ROLE;
      vda_info.info.vdest_chg_role.i_vdest = ifsv_cb->my_dest;
      vda_info.info.vdest_chg_role.i_new_role = mds_role;
      if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
      {
        m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsvda_api() returned failure"," ");
         m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         m_IFSV_GIVE_CB(ifsv_cb);
         return;
      }
 
      if(ifd_mbcsv_chgrole(ifsv_cb)!=NCSCC_RC_SUCCESS)
      {
       /*  m_LOG_CPD_HEADLINE(CPD_MBCSV_CHGROLE_FAILED,NCSFL_SEV_ERROR); */
      }

      /** set the CB's anchor value */
      saAmfResponse(ifsv_cb->amf_hdl,invocation, error);
      m_IFSV_GIVE_CB(ifsv_cb);
      m_IFD_LOG_API_L(IFSV_LOG_AMF_HA_STATE_CHNG,haState);
   }
   else
   {
     m_IFD_LOG_HEAD_LINE(IFSV_LOG_IFSV_CB_NULL,0,0);
   }
   
   return;
}

uns32 ifd_process_quisced_state(IFSV_CB *ifsv_cb,SaInvocationT invocation,SaAmfHAStateT haState)
{
    uns32 rc= NCSCC_RC_SUCCESS;
    V_DEST_RL   mds_role; 
    NCSVDA_INFO vda_info;

    ifsv_cb->invocation = invocation;
    ifsv_cb->is_quisced_set = TRUE;
    m_NCS_OS_MEMSET(&vda_info, 0, sizeof(vda_info));

    mds_role     = V_DEST_RL_QUIESCED;

    vda_info.req = NCSVDA_VDEST_CHG_ROLE;
    vda_info.info.vdest_chg_role.i_vdest = ifsv_cb->my_dest;
    vda_info.info.vdest_chg_role.i_new_role = mds_role;
    rc = ncsvda_api(&vda_info);
    if(NCSCC_RC_SUCCESS != rc) 
    {
       m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       m_IFSV_GIVE_CB(ifsv_cb);
       return rc;
    }
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : ifd_saf_health_chk_callback
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
ifd_saf_health_chk_callback (SaInvocationT invocation,
                             const SaNameT *compName,
                             SaAmfHealthcheckKeyT *checkType)
{
   IFSV_CB *ifsv_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_IFSV_GET_CB(ifsv_cb,compName->value);
   if (ifsv_cb != IFSV_NULL)   
   {
      saAmfResponse(ifsv_cb->amf_hdl,invocation, error);
      m_IFSV_GIVE_CB(ifsv_cb);
   }  
   m_IFD_LOG_API_L(IFSV_LOG_AMF_HEALTH_CHK,(long)checkType->key); 
   return;
}
/****************************************************************************
 * Name          : ifd_saf_CSI_rem_callback
 *
 * Description   : This function SAF callback function which will be called 
 *                 when the AMF framework needs remove the CSI assignment.
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
 *
 * Return Values : None
 *
 * Notes         : This is not completed - TBD.
 *****************************************************************************/
void ifd_saf_CSI_rem_callback (SaInvocationT invocation,
                               const SaNameT *compName,
                               const SaNameT *csiName,
                               const SaAmfCSIFlagsT csiFlags)
{
   IFSV_CB *ifsv_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_IFSV_GET_CB(ifsv_cb,compName->value);
   if (ifsv_cb != IFSV_NULL)   
   {
      saAmfResponse(ifsv_cb->amf_hdl,invocation, error);
      m_IFSV_GIVE_CB(ifsv_cb);
   }  
   return;
}

/****************************************************************************
 * Name          : ifd_saf_comp_terminate_callback
 *
 * Description   : This function SAF callback function which will be called 
 *                 when the AMF needs to terminate the component.
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
 * Notes         : This is not completed - TBD.
 *****************************************************************************/
void ifd_saf_comp_terminate_callback (SaInvocationT invocation,
                                      const SaNameT *compName)
{
   IFSV_CB *ifsv_cb;
   SaAisErrorT error = SA_AIS_OK;

   m_IFSV_GET_CB(ifsv_cb,compName->value);
   if (ifsv_cb != IFSV_NULL)   
   {
      saAmfResponse(ifsv_cb->amf_hdl,invocation, error);
      m_IFSV_GIVE_CB(ifsv_cb);
   }  
   exit(0);
   return;
}
