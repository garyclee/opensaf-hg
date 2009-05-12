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

  DESCRIPTION: This module contains interface with the fault management agent.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avm_fma_initialize
  avm_rcv_fma_node_reset_cb
  avm_rcv_fma_switchover_req_cb
  avm_notify_fm_hb_evt
  avm_fm_can_switchover_proceed

******************************************************************************
*/

/* Include header files */
#include "avm.h"

/* Function prototypes */
extern void avm_rcv_fma_node_reset_cb(SaInvocationT inv, 
                  SaHpiEntityPathT ent_path);

extern void avm_rcv_fma_switchover_req_cb(SaInvocationT inv);

/* Function(s) definition */
/*****************************************************************************
 * Name          : avm_fma_initialize
 *
 * Description   : This function does the following:
 *                 1. Initialize FMA library connection
 *                 2. Initialize with FMA (supplying callbacks, getting
 *                                             handle and version)
 *                 3. Get the selection object for FMA callbacks
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ******************************************************************************/
uns32
avm_fma_initialize(AVM_CB_T *cb)
{
   SaAisErrorT rc_ais = SA_AIS_OK;
   fmHandleT temp_fm_hdl;
   fmCallbacksT fm_cbks;
   SaVersionT ver; 
   SaSelectionObjectT temp_sel_obj;

   m_AVM_LOG_FUNC_ENTRY("avm_fma_initialize");

   /* Initialize with FMA */
   fm_cbks.fmNodeResetIndCallback = avm_rcv_fma_node_reset_cb;
   fm_cbks.fmSysManSwitchReqCallback = avm_rcv_fma_switchover_req_cb;
   ver.releaseCode  = 'B'; /* Hardcoding version as no alternative such as API available */
   ver.majorVersion = 1;
   ver.minorVersion = 1;
   rc_ais = fmInitialize(&temp_fm_hdl, &fm_cbks, &ver); 
   if(rc_ais != SA_AIS_OK)   /* Other error codes TBD */
   {
      return NCSCC_RC_FAILURE;
   }

   cb->fm_hdl = temp_fm_hdl;

   /* Get the selection object for FMA callbacks */
   rc_ais = fmSelectionObjectGet(cb->fm_hdl, &temp_sel_obj);

   if(rc_ais != SA_AIS_OK)   /* Other error codes TBD */
   {
      /* Do both finalize and lib-destroy before returning */
      fmFinalize(cb->fm_hdl);
      return NCSCC_RC_FAILURE; 
   }
 
   cb->fma_sel_obj = temp_sel_obj;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Name          : avm_rcv_fma_node_reset_cb
 *
 * Description   : This function is called by FMA to notify reset of a node
 *                 which is done by FM.
 *
 * Arguments     : SaInvocationT    - Invocation ID for callback
 *                 SaHpiEntityPathT - Entity path of node being reset
 *
 * Return Values : None.
 *
 * Notes         : None.
 ******************************************************************************/
void avm_rcv_fma_node_reset_cb(SaInvocationT inv,
                  SaHpiEntityPathT ent_path)
{
   AVM_ENT_INFO_T *ent_info;
   AVM_CB_T *avm_cb = NULL;

   m_AVM_LOG_FUNC_ENTRY("avm_rcv_fma_node_reset_cb");

   if (AVM_CB_NULL == (avm_cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      fmResponse(avm_cb->fm_hdl, inv, SA_AIS_ERR_NO_RESOURCES); 
      return;
   }

   if(AVM_ENT_INFO_NULL == (ent_info = avm_find_ent_info(avm_cb, &ent_path)))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      syslog(LOG_ERR, "Error - Cannot find entity path, wrong configuration?");
      fmResponse(avm_cb->fm_hdl, inv, SA_AIS_ERR_INVALID_PARAM);
      return;
   }
      
   m_AVM_LOG_FM_NODE_RESET(AVM_LOG_FM_NODE_RESET, ent_info->ep_str.name); 

   /* Send the node reset information to AvD */
   avm_avd_node_reset_resp(avm_cb, AVM_NODE_RESET_SUCCESS, ent_info->node_name);

   /* Send response back to FM */
   fmResponse(avm_cb->fm_hdl, inv, SA_AIS_OK);
}


/*****************************************************************************
 * Name          : avm_rcv_fma_switchover_req_cb
 *
 * Description   : This function is called by FMA to request a switchover.
 *
 * Arguments     : SaInvocationT    - Invocation ID for callback
 *
 * Return Values : None.
 *
 * Notes         : As the switchover request is from FM itself, we don't need 
 *                 to check with FM again before proceeding.
 ******************************************************************************/
void avm_rcv_fma_switchover_req_cb(SaInvocationT inv)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_CB_T *avm_cb = NULL;

   m_AVM_LOG_FUNC_ENTRY("avm_rcv_fma_switchover_req_cb");
   m_AVM_LOG_FM_INFO(AVM_LOG_FM_ISSUED_SWITCHOVER, NCSFL_SEV_NOTICE, NCSFL_LC_HEADLINE);

   if (AVM_CB_NULL == (avm_cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl)))
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      fmResponse(avm_cb->fm_hdl, inv, SA_AIS_ERR_NO_RESOURCES);
      return;
   }

   if(avm_cb->adm_switch == TRUE)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(avm_cb->adm_switch);
      fmResponse(avm_cb->fm_hdl, inv, SA_AIS_ERR_FAILED_OPERATION);
      return;
   }
   else
   {
      /* Treat this as Admin switchover and process */
      avm_cb->adm_switch = TRUE;

      /* AVM informs RDE about the switchover by telling it
       * to go quaisced. As per current RDE implementation
       * it will not send any ACK, so we should proceed with
       * switchover.
       */

      if(NCSCC_RC_SUCCESS != (rc = avm_notify_rde_set_role(avm_cb,
                                                           SA_AMF_HA_QUIESCED)))
      {
         m_NCS_DBG_PRINTF("\n rde_set->Quiesced Failed on Active SCXB, hence cancelling switch Triggered by FM");
         m_AVM_LOG_INVALID_VAL_FATAL(rc);
         avm_cb->adm_switch =  FALSE;
         avm_adm_switch_trap(avm_cb, ncsAvmSwitch_ID, AVM_SWITCH_FAILURE);
         fmResponse(avm_cb->fm_hdl, inv, SA_AIS_ERR_FAILED_OPERATION);
         return;
       }

       rc = avm_avd_role(avm_cb, SA_AMF_HA_QUIESCED, AVM_ADMIN_SWITCH_OVER);
       fmResponse(avm_cb->fm_hdl, inv, SA_AIS_OK);
   }
}

/*****************************************************************************
 * Name          : avm_notify_fm_hb_evt
 *
 * Description   : This function notifies FM about HB loss/restore of 
 *                 AVD/AvND.
 *
 * Arguments     : AVM_CB_T*               - Pointer to AvM CB
 *                 SaNameT                 - Node's name for which HB event 
 *                                           has occured. 
 *                 AVM_ROLE_FSM_EVT_TYPE_T - HB event type 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Valid values for evt_type are:
 *                 AVM_ROLE_EVT_AVD_HB, AVM_ROLE_EVT_AVND_HB,
 *                 AVM_ROLE_EVT_AVD_HB_RESTORE, AVM_ROLE_EVT_AVND_HB_RESTORE
 ******************************************************************************/
uns32 avm_notify_fm_hb_evt(AVM_CB_T *cb, 
       SaNameT node_name, AVM_ROLE_FSM_EVT_TYPE_T evt_type)
{
   SaAisErrorT rc_ais = SA_AIS_OK;
   AVM_NODE_INFO_T  *node_info = AVM_NODE_INFO_NULL;
   
   m_AVM_LOG_FUNC_ENTRY("avm_notify_fm_hb_evt");

   /* Construct entity path */
   node_info  = avm_find_node_name_info(cb, node_name);
   if(AVM_NODE_INFO_NULL == node_info)
   {
      m_AVM_LOG_GEN("No Entity in the DB with Node Name for HB loss", node_name.value, 
                         SA_MAX_NAME_LENGTH, NCSFL_SEV_WARNING);
      return NCSCC_RC_FAILURE;
   }

   /* Decide the HB event type and send it to FM */
   switch(evt_type)
   {
   case AVM_ROLE_EVT_AVD_HB:
      rc_ais = fmNodeHeartbeatInd(cb->fm_hdl, fmHeartbeatLost, node_info->ent_info->entity_path);
      if(rc_ais == SA_AIS_OK)
         m_AVM_LOG_NOTIFIED_FM(AVM_LOG_NOTIFIED_FM_AVD_HB, AVM_LOG_NOTIFIED_FM_LOSS,
                node_info->ent_info->ep_str.name); 
      break;

   case AVM_ROLE_EVT_AVND_HB:
      rc_ais = fmNodeHeartbeatInd(cb->fm_hdl, fmHeartbeatLost, node_info->ent_info->entity_path);
      if(rc_ais == SA_AIS_OK)
         m_AVM_LOG_NOTIFIED_FM(AVM_LOG_NOTIFIED_FM_AVND_HB, AVM_LOG_NOTIFIED_FM_LOSS,
                node_info->ent_info->ep_str.name); 
      break;

   case AVM_ROLE_EVT_AVD_HB_RESTORE:
      rc_ais = fmNodeHeartbeatInd(cb->fm_hdl, fmHeartbeatRestore, node_info->ent_info->entity_path);
      if(rc_ais == SA_AIS_OK)
         m_AVM_LOG_NOTIFIED_FM(AVM_LOG_NOTIFIED_FM_AVD_HB, AVM_LOG_NOTIFIED_FM_RESTORE,
                node_info->ent_info->ep_str.name); 
      break;

   case AVM_ROLE_EVT_AVND_HB_RESTORE:
      rc_ais = fmNodeHeartbeatInd(cb->fm_hdl, fmHeartbeatRestore, node_info->ent_info->entity_path);
      if(rc_ais == SA_AIS_OK)
         m_AVM_LOG_NOTIFIED_FM(AVM_LOG_NOTIFIED_FM_AVND_HB, AVM_LOG_NOTIFIED_FM_RESTORE,
                node_info->ent_info->ep_str.name); 
      break;

   default:
      return NCSCC_RC_FAILURE;
   }

   if(rc_ais != SA_AIS_OK)
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Name          : avm_notify_fm_node_reset
 *
 * Description   : This function notifies FM about reset of a node.
 *
 * Arguments     : AVM_CB_T*        - Pointer to AvM CB
 *                 SaHpiEntityPathT - Entity path of node
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : ent_path is assumed to contain physical details.
 ******************************************************************************/
uns32 avm_notify_fm_node_reset(AVM_CB_T *avm_cb,
                   SaHpiEntityPathT *ent_path)
{
   SaAisErrorT rc_ais;
   SaHpiEntityPathT temp_ent_path;

   m_AVM_LOG_FUNC_ENTRY("avm_notify_fm_node_reset");

   memset(&temp_ent_path, 0, sizeof(SaHpiEntityPathT));
   memcpy(&temp_ent_path, ent_path, sizeof(SaHpiEntityPathT));

   /* Send notification to FM */
   rc_ais = fmNodeResetInd(avm_cb->fm_hdl, temp_ent_path);

   if(rc_ais != SA_AIS_OK)
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Name          : avm_fm_can_switchover_proceed
 *
 * Description   : This function queries FM about whether to proceed with
 *                 switchover or not.
 *
 * Arguments     : AVM_CB_T*  -  Pointer to AvM CB
 *
 * Return Values : TRUE/FALSE.
 *
 * Notes         : None. 
 ******************************************************************************/
NCS_BOOL avm_fm_can_switchover_proceed(AVM_CB_T *cb)
{
   SaBoolT can_proceed_flag;
   SaAisErrorT rc_ais;

   m_AVM_LOG_FUNC_ENTRY("avm_fm_can_switchover_proceed");

   /* Send message to FM via FMA */
   rc_ais = fmCanSwitchoverProceed(cb->fm_hdl, &can_proceed_flag);

   if((rc_ais != SA_AIS_OK) || (can_proceed_flag == SA_FALSE)) 
      return FALSE;         /* other err codes TBD */
   
   return TRUE;
}
