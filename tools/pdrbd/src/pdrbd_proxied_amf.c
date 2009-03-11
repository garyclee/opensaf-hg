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
 */

#include "pdrbd.h"



/****************************************************************************
 * Name          : pdrbdProxiedAmfInitialise
 *
 * Description   : This function registers the proxied components' callbacks
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
uns32 pdrbdProxiedAmfInitialise()
{
   SaAmfCallbacksT amfCallbacks;
   SaVersionT amfVersion;
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   amfCallbacks.saAmfCSISetCallback = pdrbdProxiedAmfCsiSetCallback;

   /* Let the pseudo (main) component do the health-check; so no need to register */
   amfCallbacks.saAmfCSIRemoveCallback = (SaAmfCSIRemoveCallbackT) pdrbdProxiedAmfCsiRemoveCallback;
   amfCallbacks.saAmfComponentTerminateCallback = pdrbdProxiedAmfTerminateCallback;
   amfCallbacks.saAmfProxiedComponentInstantiateCallback = pdrbdProxiedAmfInstantiateCallback;
   amfCallbacks.saAmfProxiedComponentCleanupCallback = pdrbdProxiedAmfCleanupCallback;

   m_PSEUDO_GET_AMF_VER(amfVersion);

   if((error = saAmfInitialize(&pseudoCB.proxiedAmfHandle, &amfCallbacks, &amfVersion)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_CBK_INIT_FAILED, NCSFL_SEV_ERROR, NULL);
   }

   else
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_CBK_INIT_SUCCESS, NCSFL_SEV_NOTICE, NULL);
   }

   return(ret);
}


/*******************************************************************************
 * Name          : pdrbdProxiedAmfUninitialise
 *
 * Description   : This function un-registers the proxied components' callbacks
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *******************************************************************************/
uns32 pdrbdProxiedAmfUninitialise()
{
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   if((error = saAmfFinalize(pseudoCB.proxiedAmfHandle)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_CBK_UNINIT_FAILED, NCSFL_SEV_ERROR, NULL);
   }

   else
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_CBK_UNINIT_SUCCESS, NCSFL_SEV_NOTICE, NULL);
   }

   return(ret);
}


/********************************************************************************
 * Name          : pdrbdProxiedAmfRegister
 *
 * Description   : This function registers the proxied component's name with AMF
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *********************************************************************************/
uns32 pdrbdProxiedAmfRegister(uns32 pxNum)
{
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   /* Register the proxied (sub) component */
   if((error = saAmfComponentRegister(pseudoCB.proxiedAmfHandle, &pseudoCB.proxied_info[pxNum].compName,
                                       &pseudoCB.compName)) != SA_AIS_OK)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_NAME_REG_FAILED, NCSFL_SEV_WARNING, NULL);
      pseudoCB.proxied_info[pxNum].regDone = FALSE;
      ret = NCSCC_RC_FAILURE;
   }

   else
   {
      pseudoCB.proxied_info[pxNum].regDone = TRUE;
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_NAME_REG_SUCCESS, NCSFL_SEV_NOTICE, NULL);
   }

   return(ret);
}


/***********************************************************************************
 * Name          : pdrbdProxiedAmfUnregister
 *
 * Description   : This function un-registers the proxied component's name with AMF
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 ***********************************************************************************/
uns32 pdrbdProxiedAmfUnregister(uns32 pxNum)
{
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   /* Un-register the proxied (sub) component */
   if((error = saAmfComponentUnregister(pseudoCB.proxiedAmfHandle, &pseudoCB.proxied_info[pxNum].compName,
                                          (SaNameT *) NULL)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_NAME_UNREG_FAILED, NCSFL_SEV_ERROR, NULL);
   }

   else
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_NAME_UNREG_SUCCESS, NCSFL_SEV_NOTICE, NULL);
   }

   return(ret);
}


/*****************************************************************************
 * Name          : pdrbdProxiedAmfCsiSetCallback
 *
 * Description   : This function is an SAF callback function which will be
 *                 called whenever there is a change in the HA state
 *
 * Arguments     : invocation    - This parameter designates a particular
 *                                 invocation of this callback function. The
 *                                 invoked process returns invocation when it
 *                                 responds to the AMF using saAmfResponse()
 *
 *                 compName      - A pointer to the name of the component
 *                                 whose readiness state the AMF is setting
 *
 *                 csiName       - A pointer to the name of the new component
 *                                 service instance to be supported by the
 *                                 component or of an alreadt supported
 *                                 component service instance whose HA state
 *                                 is to be changed
 *                 haState       - The new HA state to be assumed by the
 *                                 component service instance identified by
 *                                 csiName
 *
 *                 csiDescriptor - This will indicate whether or not the
 *                                 component service instance for the
 *                                 compName went through quiescing
 *
 * Return Values : None
 *
 * Notes:
 ******************************************************************************/
void pdrbdProxiedAmfCsiSetCallback(SaInvocationT invocation, const SaNameT *compName, SaAmfHAStateT haState,
                                    SaAmfCSIDescriptorT csiDescriptor)
{
   uns32 ret,compNo;
   char script[250] = {0};
   SaAisErrorT error=SA_AIS_OK;

   for(compNo=0; compNo<pseudoCB.noOfProxied; compNo++)
   {
      if(0 == m_NCS_STRCMP(compName->value, pseudoCB.proxied_info[compNo].compName.value))
         break;
   }

   switch(haState)
   {
   case SA_AMF_HA_ACTIVE:

      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_RCVD_ACT_ROLE, NCSFL_SEV_NOTICE,
                           pseudoCB.proxied_info[compNo].compName.value);

      /* Save the HA state */
      pseudoCB.proxied_info[compNo].haState = haState;

      /* Save the invocation context for use with aynchronous response from processing thread */
      pseudoCB.proxied_info[compNo].script_status_array[PDRBD_PRI_ROLE_ASSIGN_CB].invocation = invocation;

      /* Put the DRBD in Primary state */
      sysf_sprintf(script, "%s %s %d %s %s %s %s %s", PSEUDO_CTRL_SCRIPT_NAME, "set_pri", compNo,
                     pseudoCB.proxied_info[compNo].resName, pseudoCB.proxied_info[compNo].devName,
                     pseudoCB.proxied_info[compNo].mountPnt, pseudoCB.proxied_info[compNo].dataDisk,
                     pseudoCB.proxied_info[compNo].metaDisk);

      ret = pdrbd_script_execute(&pseudoCB, script, PDRBD_SCRIPT_TIMEOUT_ACTIVE, PDRBD_PRI_ROLE_ASSIGN_CB, compNo);

      if(ret == NCSCC_RC_FAILURE)
      {
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_ACT_CSI_SCRIPT_EXEC_FAILED, NCSFL_SEV_ERROR, NULL);
      }

      else
      {
         if(pseudoCB.compsReady == FALSE)
            pseudoCB.compsReady = TRUE;
      }

      break;

   case SA_AMF_HA_STANDBY:

      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_RCVD_STDBY_ROLE, NCSFL_SEV_NOTICE,
                           pseudoCB.proxied_info[compNo].compName.value);

      /* Save the HA state */
      pseudoCB.proxied_info[compNo].haState = haState;

      /* Save the invocation context for use with aynchronous response from processing thread */
      pseudoCB.proxied_info[compNo].script_status_array[PDRBD_SEC_ROLE_ASSIGN_CB].invocation = invocation;

      /* Put the DRBD in Secondary state */
      sysf_sprintf(script, "%s %s %d %s %s %s %s %s", PSEUDO_CTRL_SCRIPT_NAME, "set_sec", compNo,
                     pseudoCB.proxied_info[compNo].resName, pseudoCB.proxied_info[compNo].devName,
                     pseudoCB.proxied_info[compNo].mountPnt, pseudoCB.proxied_info[compNo].dataDisk,
                     pseudoCB.proxied_info[compNo].metaDisk);

      ret = pdrbd_script_execute(&pseudoCB, script, PDRBD_SCRIPT_TIMEOUT_STANDBY, PDRBD_SEC_ROLE_ASSIGN_CB, compNo);

      if(ret == NCSCC_RC_FAILURE)
      {
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_STDBY_CSI_SCRIPT_EXEC_FAILED, NCSFL_SEV_ERROR, NULL);
      }

      else
      {
         if(pseudoCB.compsReady == FALSE)
            pseudoCB.compsReady = TRUE;
      }

      break;

   case SA_AMF_HA_QUIESCED:   /* for DRBD Quiesced state is same as Secondary */

      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_RCVD_QUISCED_ROLE, NCSFL_SEV_NOTICE,
                           pseudoCB.proxied_info[compNo].compName.value);

      /* Save the HA state */
      pseudoCB.proxied_info[compNo].haState = haState;

      /* Save the invocation context for use with aynchronous response from processing thread */
      pseudoCB.proxied_info[compNo].script_status_array[PDRBD_QUI_ROLE_ASSIGN_CB].invocation = invocation;

      /* Put the DRBD in Secondary state */
      sysf_sprintf(script, "%s %s %d %s %s %s %s %s", PSEUDO_CTRL_SCRIPT_NAME, "set_qui", compNo,
                     pseudoCB.proxied_info[compNo].resName, pseudoCB.proxied_info[compNo].devName,
                     pseudoCB.proxied_info[compNo].mountPnt, pseudoCB.proxied_info[compNo].dataDisk,
                     pseudoCB.proxied_info[compNo].metaDisk);

      ret = pdrbd_script_execute(&pseudoCB, script, PDRBD_SCRIPT_TIMEOUT_QUIESCED, PDRBD_QUI_ROLE_ASSIGN_CB, compNo);

      if(ret == NCSCC_RC_FAILURE)
      {
         error = SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_QSED_CSI_SCRIPT_EXEC_FAILED, NCSFL_SEV_ERROR, NULL);
      }

      break;

   default:
      error = SA_AIS_ERR_FAILED_OPERATION;
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_RCVD_INVALID_ROLE, NCSFL_SEV_ERROR, NULL);
      break;
   };

   if(error == SA_AIS_ERR_FAILED_OPERATION)
   {
      saAmfResponse(pseudoCB.proxiedAmfHandle, invocation, error);
   }

   return;
}


/*****************************************************************************
 * Name          : pdrbdProxiedAmfCsiRemoveCallback
 *
 * Description   : This function is an SAF callback function which will be
 *                 called to put the component in pre-CSI-Set state
 *
 * Arguments     : invocation    - This parameter designates a particular
 *                                 invocation of this callback function. The
 *                                 invoked process returns invocation when it
 *                                 responds to the AMF using saAmfResponse()
 *
 *                 compName      - A pointer to the name of the component
 *                                 whose readiness state the AMF is setting
 *
 * Return Values : None
 *
 * Notes         : None
 ******************************************************************************/
void pdrbdProxiedAmfCsiRemoveCallback(SaInvocationT invocation, const SaNameT *compName)
{
   uns32 ret,compNo;
   char script[250] = {0};
   SaAisErrorT error=SA_AIS_OK;

   for(compNo=0; compNo<pseudoCB.noOfProxied; compNo++)
   {
      if(0 == m_NCS_STRCMP(compName->value, pseudoCB.proxied_info[compNo].compName.value))
         break;
   }

   /* Save the HA role as null or void */
   pseudoCB.proxied_info[compNo].haState = PSEUDO_DRBD_DEFAULT_ROLE;

   m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_RCVD_RMV_CBK, NCSFL_SEV_ERROR,
                        pseudoCB.proxied_info[compNo].compName.value );

   /* Save the invocation context for use with aynchronous response from processing thread */
   pseudoCB.proxied_info[compNo].script_status_array[PDRBD_REMOVE_CB].invocation = invocation;

   /* Terminate (cleanup) the DRBD */
   sysf_sprintf(script, "%s %s %d %s %s %s %s %s", PSEUDO_CTRL_SCRIPT_NAME, "remove", compNo,
                  pseudoCB.proxied_info[compNo].resName, pseudoCB.proxied_info[compNo].devName,
                  pseudoCB.proxied_info[compNo].mountPnt, pseudoCB.proxied_info[compNo].dataDisk,
                  pseudoCB.proxied_info[compNo].metaDisk);

   ret = pdrbd_script_execute(&pseudoCB, script, PDRBD_SCRIPT_TIMEOUT_REMOVE, PDRBD_REMOVE_CB, compNo);

   if(ret == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_RMV_SCRIPT_EXEC_FAILED, NCSFL_SEV_ERROR,
                           pseudoCB.proxied_info[compNo].compName.value );
      error = SA_AIS_ERR_FAILED_OPERATION;
      saAmfResponse(pseudoCB.proxiedAmfHandle, invocation, error);
   }

   return;
}


/*****************************************************************************
 * Name          : pdrbdProxiedAmfCsiTerminateCallback
 *
 * Description   : This function is an SAF callback function which will be
 *                 called when the AMF needs to terminate the component
 *
 * Arguments     : invocation    - This parameter designates a particular
 *                                 invocation of this callback function. The
 *                                 invoked process returns invocation when it
 *                                 responds to the AMF using saAmfResponse()
 *
 *                 compName      - A pointer to the name of the component
 *                                 whose readiness state the AMF is setting
 *
 * Return Values : None
 *
 * Notes         : None
 ******************************************************************************/
void pdrbdProxiedAmfTerminateCallback(SaInvocationT invocation, const SaNameT *compName)
{
   uns32 ret,compNo;
   char script[250] = {0};
   SaAisErrorT error=SA_AIS_OK;

   for(compNo=0; compNo<pseudoCB.noOfProxied; compNo++)
   {
      if(0 == m_NCS_STRCMP(compName->value, pseudoCB.proxied_info[compNo].compName.value))
         break;
   }

   m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_RCVD_TERM_CBK, NCSFL_SEV_NOTICE,
                        pseudoCB.proxied_info[compNo].compName.value);

   /* Save the invocation context for use with aynchronous response from processing thread */
   pseudoCB.proxied_info[compNo].script_status_array[PDRBD_TERMINATE_CB].invocation = invocation;

   /* Terminate (cleanup) the DRBD */
   sysf_sprintf(script, "%s %s %d %s %s %s %s %s", PSEUDO_CTRL_SCRIPT_NAME, "terminate", compNo,
                  pseudoCB.proxied_info[compNo].resName, pseudoCB.proxied_info[compNo].devName,
                  pseudoCB.proxied_info[compNo].mountPnt, pseudoCB.proxied_info[compNo].dataDisk,
                  pseudoCB.proxied_info[compNo].metaDisk);

   ret = pdrbd_script_execute(&pseudoCB, script, PDRBD_SCRIPT_TIMEOUT_TERMINATE, PDRBD_TERMINATE_CB, compNo);

   if(ret == NCSCC_RC_FAILURE)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_AMF_COMP_TERM_SCRIPT_EXEC_FAILED, NCSFL_SEV_ERROR, NULL);
      error = SA_AIS_ERR_FAILED_OPERATION;
      saAmfResponse(pseudoCB.proxiedAmfHandle, invocation, error);
   }

   return;
}


void pdrbdProxiedAmfInstantiateCallback(SaInvocationT invocation, const SaNameT *compName)
{
   SaAisErrorT error=SA_AIS_OK;

   saAmfResponse(pseudoCB.proxiedAmfHandle, invocation, error);
}


void pdrbdProxiedAmfCleanupCallback(SaInvocationT invocation, const SaNameT *compName)
{
   SaAisErrorT error=SA_AIS_OK;

   saAmfResponse(pseudoCB.proxiedAmfHandle, invocation, error);
}
