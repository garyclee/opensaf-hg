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
  FILE NAME: IFND_SAF.C

  DESCRIPTION: IfND SAF callback routines.

  FUNCTIONS INCLUDED in this module:
  ifnd_saf_readiness_state_callback ........... IfND SAF readiness callback.
  ifnd_saf_pend_oper_confirm_callback.......... IfND SAF oper confirm callback.
  ifnd_saf_health_chk_callback................. IfND SAF Health Check callback.
  ifnd_saf_CSI_rem_callback   ................. IfND SAF CSI remove callback.  
  ifind_saf_comp_terminate_callback............. IfiND SAF Term comp callback.  

******************************************************************************/

#include "ifnd.h"


/****************************************************************************\
 PROCEDURE NAME : ifnd_saf_csi_set_cb

 DESCRIPTION    : This function SAF callback function which will be called
                  when there is any change in the HA state.

 ARGUMENTS      : invocation     - This parameter designated a particular
                                  invocation of this callback function. The
                                  invoke process return invocation when it
                                  responds to the Avilability Management
                                  FrameWork using the saAmfResponse()
                                  function.
                 compName       - A pointer to the name of the component
                                  whose readiness stae the Availability
                                  Management Framework is setting.
                 csiName        - A pointer to the name of the new component
                                  service instance to be supported by the
                                  component or of an alreadt supported
                                  component service instance whose HA state
                                  is to be changed.
                 csiFlags       - A value of the choiceflag type which
                                  indicates whether the HA state change must
                                  be applied to a new component service
                                  instance or to all component service
                                  instance currently supported by the
                                  component.
                 haState        - The new HA state to be assumeb by the
                                  component service instance identified by
                                  csiName.
                 activeCompName - A pointer to the name of the component that
                                  currently has the active state or had the
                                  active state for this component serivce
                                  insance previously.
                 transitionDesc - This will indicate whether or not the
                                  component service instance for
                                  ativeCompName went through quiescing.
 RETURNS       : None.
\*****************************************************************************/

void ifnd_saf_csi_set_cb(SaInvocationT invocation,
                         const SaNameT *compName,
                         SaAmfHAStateT haState,
                         SaAmfCSIDescriptorT csiDescriptor)

{
   IFSV_CB      *cb = 0;
   SaAisErrorT    saErr = SA_AIS_OK;

   m_IFSV_GET_CB(cb,compName->value);

   if(cb == NULL)
      return;

   if(cb) {
      cb->ha_state = haState; /* Set the HA State */

      saAmfResponse(cb->amf_hdl, invocation, saErr);
      m_IFSV_GIVE_CB(cb);
   }
   return;
} /* End of cpnd_saf_csi_set_cb() */


/****************************************************************************
 * Name          : ifnd_saf_health_chk_callback
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
ifnd_saf_health_chk_callback (SaInvocationT invocation,
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
      m_IFND_LOG_API_L(IFSV_LOG_AMF_HEALTH_CHK,(long)checkType->key); 
   }   
   return;
}
/****************************************************************************
 * Name          : ifnd_saf_CSI_rem_callback
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
void ifnd_saf_CSI_rem_callback (SaInvocationT invocation,
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
 * Name          : ifnd_saf_comp_terminate_callback
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
 *                 compName       - A pointer to the name of the component. 
 *
 * Return Values : None
 *
 * Notes         : This is not completed - TBD.
 *****************************************************************************/
void ifnd_saf_comp_terminate_callback (SaInvocationT invocation,
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
