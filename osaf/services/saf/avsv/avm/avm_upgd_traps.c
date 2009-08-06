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

  DESCRIPTION:This module contain the functions for sending SNMP traps.
  ..............................................................................

  Function Included in this Module:

******************************************************************************
*/

#include "avm.h"

/* Trap pattern array len */
#define AVM_TRAP_PATTERN_ARRAY_LEN  1
#define AVM_EDA_EVT_PUBLISHER_NAME  "AVM"
#define AVM_TRAP_CHANNEL_OPEN_TIMEOUT 200000000
static SaEvtEventPatternT avm_trap_pattern_array[AVM_TRAP_PATTERN_ARRAY_LEN] = {
       {9, 9, (SaUint8T *) "SNMP_TRAP"}
};

/*****************************************************************************
  Name          :  avm_send_boot_upgd_trap

  Description   :  This function generates SNMP trap during boot 
                   up and upgrade.

  Arguments     :  avm_cb - Pointer to the AVM CB structure
                   ent_info - Pointer to the Entity Info struct
                   trap_id  - trap to be sent.

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE 

  Notes         :
*****************************************************************************/
uns32 avm_send_boot_upgd_trap(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, uns32 trap_id)
{
   NCS_TRAP           snmptm_avm_trap; 
   NCS_TRAP_VARBIND   *i_trap_varbind = NULL;
   NCS_TRAP_VARBIND   *temp_trap_varbind = NULL;
   uns32              status = NCSCC_RC_FAILURE;
   uns32              tlv_size = 0; 
   uns8*              encoded_buffer = NULL; 
   EDU_ERR            o_err = EDU_NORMAL;
   SaEvtEventHandleT  eventHandle;
   SaUint8T           priority = 1;
   SaTimeT            retentionTime = 9000000;
   SaEvtEventIdT      eventId;
   SaAisErrorT           saStatus = SA_AIS_OK;
   SaSizeT            eventDataSize;
   SaEvtEventPatternArrayT  patternArray;
   uns32               inst_ids[1000];
   uns32               i, length = 0;
   uns8               logstr[100];  

   /* Time to open Trap channnel */
   if (FALSE == avm_cb->trap_channel)
   {
      if (NCSCC_RC_SUCCESS != avm_open_trap_channel(avm_cb))
      {
         avm_cb->trap_channel = FALSE;
         return status;
      }
      else
      {
         avm_cb->trap_channel = TRUE;
         m_AVM_LOG_EDA(AVM_LOG_EDA_CHANNEL_OPEN, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_NOTICE);
      }
   }

   memset(&patternArray, 0, sizeof(SaEvtEventPatternArrayT)); 
   memset(&snmptm_avm_trap, 0, sizeof(NCS_TRAP)); 
   memset(&eventHandle, 0, sizeof(SaEvtEventHandleT)); 
   memset(&eventId, 0, sizeof(SaEvtEventIdT)); 

   /* Fill in the trap details */ 
   snmptm_avm_trap.i_trap_tbl_id = NCSMIB_TBL_AVM_TRAPS; 
   snmptm_avm_trap.i_trap_id = trap_id;
   snmptm_avm_trap.i_inform_mgr = TRUE; 
   snmptm_avm_trap.i_version = m_NCS_TRAP_VERSION; 

   inst_ids[0] = m_NCS_OS_NTOHS(ent_info->ep_str.length);
   length = inst_ids[0];
   for (i=1; i <= inst_ids[0]; i++)
   {
      inst_ids[i] = ent_info->ep_str.name[i-1];
   }

   if (trap_id == ncsAvmBootFailure_ID)
   {
      /* Allocate Memory */ 
      i_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB; 
      if (i_trap_varbind == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      
      memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
      
      /* Fill in the object ncsAvmEntityPath details */ 
      i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
      i_trap_varbind->i_param_val.i_param_id = ncsAvmEntNodeName_ID; 
      i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;

      i_trap_varbind->i_param_val.i_length = ent_info->node_name.length;
      i_trap_varbind->i_param_val.info.i_oct = ent_info->node_name.value; 

      i_trap_varbind->i_idx.i_inst_len = (length + 1);
      
      i_trap_varbind->i_idx.i_inst_ids = inst_ids;
      
      /* Add to the trap */
      snmptm_avm_trap.i_trap_vb = i_trap_varbind;
      temp_trap_varbind = snmptm_avm_trap.i_trap_vb;
   }
   else if ((trap_id == ncsAvmUpgradeSuccess_ID) || 
            (trap_id == ncsAvmCurrActiveLabelChange_ID))
   {
      /* Allocate Memory */ 
      i_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB; 
      if (i_trap_varbind == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      
      memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
      
      /* Fill in the object ncsAvmEntityPath details */ 
      i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
      
      i_trap_varbind->i_param_val.i_param_id = ncsAvmEntDHCPConfCurrActiveLabel_ID; 
      i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
      
      i_trap_varbind->i_param_val.i_length = 
         ent_info->dhcp_serv_conf.curr_act_label->name.length; 
      i_trap_varbind->i_param_val.info.i_oct = 
         ent_info->dhcp_serv_conf.curr_act_label->name.name; 
      
      i_trap_varbind->i_idx.i_inst_len = (length + 1);
      
      i_trap_varbind->i_idx.i_inst_ids = inst_ids;
      
      /* Add to the trap */
      snmptm_avm_trap.i_trap_vb = i_trap_varbind;
      temp_trap_varbind = snmptm_avm_trap.i_trap_vb;
   }
   else if (trap_id == ncsAvmBootProgress_ID) 
   {
      /* Allocate Memory */ 
      i_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB; 
      if (i_trap_varbind == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      
      memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
      
      /* Fill in the object ncsAvmEntityPath details */ 
      i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
      
      i_trap_varbind->i_param_val.i_param_id = ncsAvmBootProgressStatus_ID; 
      i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
      
      i_trap_varbind->i_param_val.i_length = sizeof(int);
      i_trap_varbind->i_param_val.info.i_int = ent_info->boot_prg_status;
      
      i_trap_varbind->i_idx.i_inst_len = (length + 1);
      
      i_trap_varbind->i_idx.i_inst_ids = inst_ids;

      /* Add to the trap */
      snmptm_avm_trap.i_trap_vb = i_trap_varbind;
   }
   else if ((trap_id == ncsAvmSwFwUpgradeFailure_ID) ||
            (trap_id == ncsAvmUpgradeProgress_ID))
   {
      /* Allocate Memory */ 
      i_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB; 
      if (i_trap_varbind == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      
      memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
      
      /* Fill in the object ncsAvmEntityPath details */ 
      i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
      i_trap_varbind->i_param_val.i_param_id = ncsAvmEntDHCPConfCurrActiveLabel_ID; 
      i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
      
      i_trap_varbind->i_param_val.i_length = 
         ent_info->dhcp_serv_conf.curr_act_label->name.length;
      i_trap_varbind->i_param_val.info.i_oct = 
         ent_info->dhcp_serv_conf.curr_act_label->name.name; 
      
      i_trap_varbind->i_idx.i_inst_len = (length + 1);
      
      i_trap_varbind->i_idx.i_inst_ids = inst_ids;
     
      snmptm_avm_trap.i_trap_vb = i_trap_varbind;
      temp_trap_varbind = snmptm_avm_trap.i_trap_vb; 
      
      /* Allocate Memory */ 
      i_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB; 
      if (i_trap_varbind == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      
      memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
      
      /* Fill in the object ncsAvmUpgradeModule details */ 
      i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_SUPP;
      i_trap_varbind->i_param_val.i_param_id = ncsAvmUpgradeModule_ID; 
      i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
      
      i_trap_varbind->i_param_val.i_length = sizeof(int); 
      i_trap_varbind->i_param_val.info.i_int = 
                                     avm_cb->upgrade_module; 
      /* add to the trap */
      temp_trap_varbind->next_trap_varbind = i_trap_varbind;
      temp_trap_varbind = temp_trap_varbind->next_trap_varbind; 
      
      /* Allocate Memory */ 
      i_trap_varbind = m_MMGR_ALLOC_AVM_TRAP_VB; 
      if (i_trap_varbind == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      
      memset(i_trap_varbind, 0, sizeof(NCS_TRAP_VARBIND)); 
    
      if (trap_id == ncsAvmSwFwUpgradeFailure_ID) 
      {
         /* Fill in the object ncsAvmUpgradeErrorType details */ 
         i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_SUPP;
         i_trap_varbind->i_param_val.i_param_id = ncsAvmUpgradeErrorType_ID; 
         i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
      
         i_trap_varbind->i_param_val.i_length = sizeof(int); 
         i_trap_varbind->i_param_val.info.i_int = 
                                     avm_cb->upgrade_error_type; 
      }
      else
      {
         /* Fill in the object ncsSundUpgradeProgressEvent details */ 
         i_trap_varbind->i_tbl_id = NCSMIB_TBL_AVM_SUPP;
         i_trap_varbind->i_param_val.i_param_id = ncsAvmUpgradeProgressEvent_ID; 
         i_trap_varbind->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
      
         i_trap_varbind->i_param_val.i_length = sizeof(int); 
         i_trap_varbind->i_param_val.info.i_int = 
                                     avm_cb->upgrade_prg_evt; 
      }  
      /* add to the trap */ 
      temp_trap_varbind->next_trap_varbind = i_trap_varbind;
   }  
   /* Encode the trap */ 
   /* 1. Get the length */ 
   tlv_size = ncs_tlvsize_for_ncs_trap_get(&snmptm_avm_trap); 

   /* Allocate the memory */
   if (NULL == (encoded_buffer = m_MMGR_ALLOC_AVM_EDA_TLV_SIZE(tlv_size)))
   {
      /* Probably need to call a function that free's the trap var bind list */
      status = NCSCC_RC_FAILURE;
      goto done1;
   }

   memset(encoded_buffer, '\0', tlv_size);

   /* call the EDU macro to encode with buffer pointer and size */
   status = 
      m_NCS_EDU_TLV_EXEC(&avm_cb->edu_hdl, ncs_edp_ncs_trap, encoded_buffer, 
                         tlv_size, EDP_OP_TYPE_ENC, &snmptm_avm_trap, &o_err);
   if (status != NCSCC_RC_SUCCESS)
   {
      snprintf(logstr , sizeof(logstr)-1, "%s %d", "\nTLV-EXEC failed,  error-value=", o_err);
      m_AVM_LOG_DEBUG(logstr,NCSFL_SEV_ERROR);
      /* Probably need to call a function that free's the trap var bind list */
      goto done;
   }

   eventDataSize = (SaSizeT)tlv_size;
   
   /* Allocate memory for the event */
   saStatus = saEvtEventAllocate(avm_cb->trap_chan_hdl, &eventHandle);
   if (saStatus != SA_AIS_OK)
   {
      m_AVM_LOG_DEBUG("\n saEvtEventAllocate failed",NCSFL_SEV_ERROR);
      /* Probably need to call a function that free's the trap var bind list */
      status = NCSCC_RC_FAILURE;
      goto done;
   }

   patternArray.patternsNumber = AVM_TRAP_PATTERN_ARRAY_LEN;
   patternArray.patterns = avm_trap_pattern_array;

   /* Set all the writeable event attributes */
   saStatus = saEvtEventAttributesSet(eventHandle,
                                      &patternArray, 
                                      priority,
                                      retentionTime,
                                      &avm_cb->publisherName);
   if (saStatus != SA_AIS_OK)
   {
      m_AVM_LOG_DEBUG("\n saEvtEventAttributesSet failed",NCSFL_SEV_ERROR);
      saEvtEventFree(eventHandle);
      /* Probably need to call a function that free's the trap var bind list */
      status = NCSCC_RC_FAILURE;
      goto done;
   }
   
   /* Publish an event on the channel */
   saStatus = saEvtEventPublish(eventHandle,
                                encoded_buffer,
                                eventDataSize,
                                &eventId);
   if (saStatus != SA_AIS_OK)
   {
      m_AVM_LOG_DEBUG("\n saEvtEventPublish failed",NCSFL_SEV_ERROR);
      saEvtEventFree(eventHandle);
      /* Probably need to call a function that free's the trap var bind list */
      status = NCSCC_RC_FAILURE;
      goto done;
   }

   /* Free the allocated memory for the event */
   saStatus = saEvtEventFree(eventHandle);
   if (saStatus != SA_AIS_OK)
   {
      m_AVM_LOG_DEBUG("\n saEvtEventFree failed",NCSFL_SEV_ERROR);
      /* Probably need to call a function that free's the trap var bind list */
      status = NCSCC_RC_FAILURE;
      goto done;
   }

   /* Free the memory allocated till now and return */
done:
   m_MMGR_FREE_AVM_EDA_TLV_SIZE(encoded_buffer);

done1:
   if ((trap_id == ncsAvmUpgradeSuccess_ID) || 
       (trap_id == ncsAvmUpgradeFailure_ID) ||
       (trap_id == ncsAvmBootFailure_ID)    || 
       (trap_id == ncsAvmCurrActiveLabelChange_ID))
   {
      m_MMGR_FREE_AVM_TRAP_VB(temp_trap_varbind);
   }

   if ((trap_id == ncsAvmUpgradeFailure_ID) ||
      (trap_id == ncsAvmBootProgress_ID))
   {
      m_MMGR_FREE_AVM_TRAP_VB(i_trap_varbind);
   }

   return status;
}


/*****************************************************************************
  Name          :  avm_open_trap_chanel

  Description   :  This function is used for creating a channel to send traps.

  Arguments     :  avm_cb - Pointer to the AVM CB structure

  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE 

  Notes         :
*****************************************************************************/
extern uns32
avm_open_trap_channel(AVM_CB_T *avm_cb)
{
   uns32           channelFlags = 0;
   SaAisErrorT     status = SA_AIS_OK;
   SaTimeT         timeout = AVM_TRAP_CHANNEL_OPEN_TIMEOUT;
   
   strcpy(avm_cb->publisherName.value, AVM_EDA_EVT_PUBLISHER_NAME);
   avm_cb->publisherName.length = strlen(avm_cb->publisherName.value);
   
   /* update the Channel Name */
   strcpy(avm_cb->trap_chan_name.value, m_SNMP_EDA_EVT_CHANNEL_NAME);
   avm_cb->trap_chan_name.length = strlen(avm_cb->trap_chan_name.value);
   
   /* Open event channel with remote server (EDS)  */
   channelFlags = SA_EVT_CHANNEL_CREATE | SA_EVT_CHANNEL_PUBLISHER;
   
   status = saEvtChannelOpen(avm_cb->eda_hdl,
      &avm_cb->trap_chan_name,
      channelFlags,
      timeout,
      &avm_cb->trap_chan_hdl);

   if (status != SA_AIS_OK)
   {
      m_AVM_LOG_EDA_VAL(status, AVM_LOG_EDA_CHANNEL_OPEN, AVM_LOG_EDA_FAILURE, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}
