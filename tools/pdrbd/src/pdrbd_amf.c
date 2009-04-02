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

extern uns32 avsv_cpy_node_DN_from_DN(SaNameT *d_node_dn,
                              SaNameT *s_dn_name);



/****************************************************************************
 * Name          : pseudoAmfInitialise
 *
 * Description   : This function initialises AMF and registers callbacks
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
uns32 pseudoAmfInitialise()
{
   SaAmfCallbacksT amfCallbacks;
   SaVersionT amfVersion;
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   amfCallbacks.saAmfCSISetCallback = pseudoAmfCsiSetCallback;
   amfCallbacks.saAmfHealthcheckCallback = (SaAmfHealthcheckCallbackT) pseudoAmfHealthcheckCallback;
   amfCallbacks.saAmfCSIRemoveCallback = (SaAmfCSIRemoveCallbackT) pseudoAmfCsiRemoveCallback;
   amfCallbacks.saAmfComponentTerminateCallback = pseudoAmfTerminateCallback;

   m_PSEUDO_GET_AMF_VER(amfVersion);

   if((error = saAmfInitialize(&pseudoCB.amfHandle, &amfCallbacks, &amfVersion)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_INSTALL_FAILURE, NCSFL_SEV_ERROR, 0);
   }

   else
   {
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_INSTALL_SUCCESS, NCSFL_SEV_NOTICE, 0);
   }

   return(ret);
}


/****************************************************************************
 * Name          : pseudoAmfUninitialise
 *
 * Description   : This function un-initialises AMF
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
uns32 pseudoAmfUninitialise()
{
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   if((error = saAmfFinalize(pseudoCB.amfHandle)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_UNINSTALL_FAILURE, NCSFL_SEV_ERROR, 0);
   }

   else
   {
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_UNINSTALL_SUCCESS, NCSFL_SEV_NOTICE, 0);
   }

   return(ret);
}


/****************************************************************************
 * Name          : pseudoAmfRegister
 *
 * Description   : This function registers Pseudo with AMF
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
uns32 pseudoAmfRegister()
{
   SaNameT nodeId;
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   memset(&nodeId, 0, sizeof(nodeId));

   /* Get the component name */
   if((error = saAmfComponentNameGet(pseudoCB.amfHandle, &pseudoCB.compName)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_NAME_GET_FAILURE, NCSFL_SEV_ERROR, 0);
      goto parRet;
   }

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_NAME_GET_SUCCESS, NCSFL_SEV_NOTICE, 0);

   /* Register the proxy (main) component */
   if((error = saAmfComponentRegister(pseudoCB.amfHandle, &pseudoCB.compName, (SaNameT *) NULL)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_REGN_FAILURE, NCSFL_SEV_ERROR, 0);
      goto parRet;
   }

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_REGN_SUCCESS, NCSFL_SEV_NOTICE, 0);

   /* Find the node ID */
   if(avsv_cpy_node_DN_from_DN(&nodeId, &pseudoCB.compName) != NCSCC_RC_SUCCESS)
   {
      m_LOG_PDRBD_MISC(PDRBD_MISC_UNABLE_TO_RETRIEVE_NODE_ID, NCSFL_SEV_WARNING, pseudoCB.compName.value);
      ret = NCSCC_RC_FAILURE;
      goto parRet;
   }

   memcpy((uns8 *) &pseudoCB.nodeId, &nodeId.value, nodeId.length);

parRet:
   return(ret);
}


/****************************************************************************
 * Name          : pseudoAmfUnregister
 *
 * Description   : This function unregisters Pseudo with AMF
 *
 * Arguments     : None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/
uns32 pseudoAmfUnregister()
{
   SaAisErrorT error;
   uns32 ret=NCSCC_RC_SUCCESS;

   /* Get the component name */
   if((error = saAmfComponentNameGet(pseudoCB.amfHandle, &pseudoCB.compName)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_NAME_GET_FAILURE, NCSFL_SEV_ERROR, 0);
      goto paurRet;
   }

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_NAME_GET_SUCCESS, NCSFL_SEV_NOTICE, 0);

   /* Un-register the proxy (main) component */
   if((error = saAmfComponentUnregister(pseudoCB.amfHandle, &pseudoCB.compName, (SaNameT *) NULL)) != SA_AIS_OK)
   {
      ret = NCSCC_RC_FAILURE;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_UNREGN_FAILURE, NCSFL_SEV_ERROR, 0);
      goto paurRet;
   }

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_COMP_UNREGN_SUCCESS, NCSFL_SEV_NOTICE, 0);

paurRet:
   return(ret);
}


/*****************************************************************************
 * Name          : pseudoAmfCsiSetCallback
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
void pseudoAmfCsiSetCallback(SaInvocationT invocation, const SaNameT *compName, SaAmfHAStateT haState,
                                SaAmfCSIDescriptorT csiDescriptor)
{
   SaAisErrorT error=SA_AIS_OK;

   switch(haState)
   {
   case SA_AMF_HA_ACTIVE:

      /* Save the HA state */
      pseudoCB.haState = haState;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_ACTIVE_ROLE_ASSIGN, NCSFL_SEV_NOTICE, 0);
      break;

   /* With the current scheme, pseudo (main) component should only get Active role */
   case SA_AMF_HA_STANDBY:
   case SA_AMF_HA_QUIESCED:   /* for DRBD Quiesced state is same as Secondary */
   default:

      /* Save the HA state */
      pseudoCB.haState = haState;
      error=SA_AIS_ERR_FAILED_OPERATION;
      m_LOG_PDRBD_AMF(PDRBD_SP_AMF_INVALID_ROLE_ASSIGN, NCSFL_SEV_WARNING, 0);
      break;
   };

   saAmfResponse(pseudoCB.amfHandle, invocation, error);
   return;
}


/*****************************************************************************
 * Name          : pseudoAmfCsiRemoveCallback
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
void pseudoAmfCsiRemoveCallback(SaInvocationT invocation, const SaNameT *compName)
{
   SaAisErrorT error=SA_AIS_OK;

   /* Save the HA role as null or void */
   pseudoCB.haState = PSEUDO_DRBD_DEFAULT_ROLE;
   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_REMOVE_CB_INVOKE, NCSFL_SEV_NOTICE, 0);

   saAmfResponse(pseudoCB.amfHandle, invocation, error);
   return;
}


/****************************************************************************
 * Name          : pseudoAmfHealthcheckCallback
 *
 * Description   : This function is an SAF callback function which will be
 *                 called when the AMF framework needs to do health check
 *                 for the component
 *
 * Arguments     : invocation     - This parameter designates a particular
 *                                  invocation of this callback function. The
 *                                  invoked process returns invocation when it
 *                                  responds to the AMF using saAmfResponse()
 *
 *                 compName       - A pointer to the name of the component
 *                                  whose readiness state the AMF is setting
 *
 *                 checkType      - The type of healthcheck to be executed
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
void pseudoAmfHealthcheckCallback(SaInvocationT invocation, const SaNameT *compName,
                                    SaAmfHealthcheckKeyT *checkType)
{
   uns32 ret, i;
   char script[250]={0},*ptr;
   SaAisErrorT error=SA_AIS_OK;

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_HLTH_CHK_CB_INVOKE, NCSFL_SEV_INFO, 0);

   /* If the proxied (sub) components are not ready yet, then return success to AMF */
   if(pseudoCB.compsReady == TRUE)
   {
      ptr = script;
      ptr += sprintf(script, "%s ", PSEUDO_STS_SCRIPT_NAME);

      for(i=0; i<pseudoCB.noOfProxied; i++)
      {
         /* Skip the component if its AMF regn has failed */
         if(pseudoCB.proxied_info[i].regDone == TRUE)
         {
            ptr += sprintf(ptr, "%d %s %s ", i, pseudoCB.proxied_info[i].resName, pseudoCB.proxied_info[i].devName);
         }
      }

      ret = pdrbd_script_execute(&pseudoCB, script, PDRBD_SCRIPT_TIMEOUT_HLTH_CHK, PDRBD_HEALTH_CHECK_CB,
                                    PDRBD_MAX_PROXIED);

      if(ret == NCSCC_RC_FAILURE || pseudoCB.hcTimeOutCnt >= HC_SCRIPT_TIMEOUT_MAX_COUNT)
      {
         error=SA_AIS_ERR_FAILED_OPERATION;
         m_LOG_PDRBD_MISC(PDRBD_MISC_PSEUDO_DRBD_HCK_SRC_EXEC_FAILED, NCSFL_SEV_ERROR, NULL );
      }
   }

   saAmfResponse(pseudoCB.amfHandle, invocation, error);
   return;
}


/*****************************************************************************
 * Name          : pseudoAmfCsiTerminateCallback
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
void pseudoAmfTerminateCallback(SaInvocationT invocation, const SaNameT *compName)
{
   SaAisErrorT error=SA_AIS_OK;

   m_LOG_PDRBD_AMF(PDRBD_SP_AMF_TERMINATE_CB_INVOKE, NCSFL_SEV_NOTICE, 0);
   saAmfResponse(pseudoCB.amfHandle, invocation, error);
   return;
}
