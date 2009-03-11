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
 
  DESCRIPTION:This module provides utility functions needed by AvM.
  ..............................................................................

  Function Included in this Module:

  avm_copy_list            -
  avm_map_hpi2fsm          - 
  avm_scope_fault          -  
  avm_scope_fault_children -
  avm_fault_criteria       -
  avm_handle_fault         -
  avm_is_this_entity_self  -
  avm_conv_phy_info_to_ent_path - 
  avm_compare_ent_paths         -
 
******************************************************************************
*/

#include "avm.h"

#define m_AVM_ADM_LOCK_ENT(avm_cb, ent_info)\
{\
   AVM_EVT_T fsm_evt;\
   if(AVM_ADM_LOCK == ent_info->adm_lock)\
   {\
      fsm_evt.fsm_evt_type = AVM_EVT_ADM_LOCK;\
      avm_fsm_handler(avm_cb, ent_info, &fsm_evt);\
   }\
}

#define m_AVM_ADM_UNLOCK_ENT(avm_cb, ent_info)\
{\
   AVM_EVT_T fsm_evt;\
   if(AVM_ADM_UNLOCK == ent_info->adm_lock)\
   {\
      fsm_evt.fsm_evt_type = AVM_EVT_ADM_UNLOCK;\
      avm_fsm_handler(avm_cb, ent_info, &fsm_evt);\
   }\
}
static uns32
avm_validate_loc_range(
                        AVM_ENT_INFO_T    *ent_info,
                        SaHpiEntityT       entity,
                        AVM_VALID_INFO_T  *valid_info
                      );
static uns32
avm_rmv_node_info(
                  AVM_CB_T       *avm_cb,
                  AVM_ENT_INFO_T *ent_info
                 );

static uns32
avm_rmv_frm_parent(
                   AVM_CB_T       *avm_cb,
                   AVM_ENT_INFO_T *ent_info
                  );

static uns32
avm_rmv_frm_dependee(
                     AVM_CB_T       *avm_cb,
                     AVM_ENT_INFO_T *ent_info
                    );

static uns32
ncs_avm_build_mib_idx(NCSMIB_IDX *mib_idx, AVM_ENT_PATH_STR_T *idx_val);

extern void
avm_sensor_event_initialize(
                              AVM_ENT_INFO_T *ent_info, 
                              uns32           n
                           )
{
   ent_info->sensors[n].SensorType = 0;
   ent_info->sensors[n].SensorNum  = 0;
   ent_info->sensors[n].sensor_assert = NCSCC_RC_FAILURE;   
}   


/***********************************************************************
 ******
 * Name          : avm_map_hpi2fsm
 *
 * Description   : This function maps hpi events to AvM fsm events
 *
 * Arguments     : AVM_AVD_MSG_T* -  pointer to destination.
 *                 AVM_AVD_MSG_T* -  pointer to source.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern uns32 
avm_map_hpi2fsm(
                  AVM_CB_T             *cb,
                  HPI_EVT_T            *evt, 
                  AVM_FSM_EVT_TYPE_T   *fsm_evt_type,
                  AVM_ENT_INFO_T       *ent_info
               )
{
   SaHpiEventT  hpi_event;

   m_AVM_LOG_FUNC_ENTRY("avm_map_hpi2fsm");
   hpi_event = evt->hpi_event;

   switch(hpi_event.EventType)
   {
      case SAHPI_ET_RESOURCE:
      {
         /* Log Resource Fail/Restore event */
         m_AVM_LOG_HPI_RES_EVT(AVM_LOG_EVT_RESOURCE, hpi_event.EventDataUnion.ResourceEvent.ResourceEventType, ent_info->ep_str.name);
         if(SAHPI_RESE_RESOURCE_FAILURE == hpi_event.EventDataUnion.ResourceEvent.ResourceEventType)
         {
            *fsm_evt_type = AVM_EVT_RESOURCE_FAIL;
         }else if(SAHPI_RESE_RESOURCE_RESTORED == hpi_event.EventDataUnion.ResourceEvent.ResourceEventType)
         {
            *fsm_evt_type = AVM_EVT_RESOURCE_REST;
         }else
         {
            *fsm_evt_type = AVM_EVT_IGNORE;
         }
      }
      break;

      case SAHPI_ET_HOTSWAP:
      {
         m_AVM_LOG_HPI_HS_EVT(AVM_LOG_EVT_HOTSWAP, hpi_event.EventDataUnion.HotSwapEvent.HotSwapState, hpi_event.EventDataUnion.HotSwapEvent.PreviousHotSwapState, ent_info->ep_str.name);
         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_INSERTION_PENDING)
         {
            *fsm_evt_type = AVM_EVT_INSERTION_PENDING;
             break;  
         }

#ifdef HPI_A
         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_ACTIVE_HEALTHY)
#else
         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_ACTIVE)
#endif
         {
            *fsm_evt_type = AVM_EVT_ENT_ACTIVE;
             break;  
         }

         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_EXTRACTION_PENDING)
         {
             *fsm_evt_type = AVM_EVT_EXTRACTION_PENDING;
              break; 
         }

         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_INACTIVE)
         {
            *fsm_evt_type = AVM_EVT_ENT_INACTIVE;
             break;
         }

         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_NOT_PRESENT)
         {
            if((hpi_event.EventDataUnion.HotSwapEvent.PreviousHotSwapState == SAHPI_HS_STATE_INACTIVE) || (ent_info->current_state == AVM_ENT_INACTIVE))
            {
               *fsm_evt_type = AVM_EVT_ENT_EXTRACTED;
                break;  
            }
            if(hpi_event.EventDataUnion.HotSwapEvent.PreviousHotSwapState != SAHPI_HS_STATE_INACTIVE)
            {
               *fsm_evt_type = AVM_EVT_SURP_EXTRACTION;
                break;
            }
         }
      }
      break;

      case SAHPI_ET_SENSOR:
      {
         m_AVM_LOG_HPI_HS_EVT(AVM_LOG_EVT_SENSOR, hpi_event.EventDataUnion.SensorEvent.Assertion, hpi_event.Severity, ent_info->ep_str.name);
         m_AVM_LOG_HPI_HS_EVT(AVM_LOG_EVT_SENSOR, hpi_event.EventType, hpi_event.EventDataUnion.SensorEvent.SensorType, ent_info->ep_str.name);

         /* 
          * Check for the Firmware progress event and then send the event to the 
          * FSM to handle it.
          */
         if (hpi_event.EventDataUnion.SensorEvent.SensorType == SAHPI_SYSTEM_FW_PROGRESS)
         {
            *fsm_evt_type = AVM_EVT_SENSOR_FW_PROGRESS;
         }else
         {
            *fsm_evt_type = AVM_EVT_IGNORE;
         }

         /* Break here since other sensor events are not yet supported. Can be 
          * removed later if required.
          */
         break;
       } 
      default:
      {
          m_AVM_LOG_GEN_EP_STR("AvM doesnt support the event from HPI for entity", ent_info->ep_str.name, NCSFL_SEV_INFO);
         *fsm_evt_type = AVM_EVT_INVALID;
      }
   }

    m_AVM_LOG_FSM_EVT(AVM_LOG_EVT_FSM, *fsm_evt_type, ent_info->current_state, NCSFL_SEV_INFO);

   return NCSCC_RC_SUCCESS;
}

extern uns32
avm_map_avd2fsm(
                  AVM_EVT_T            *avd_avm, 
                  AVM_AVD_MSG_TYPE_T    msg_type,
                  AVM_FSM_EVT_TYPE_T   *fsm_evt_type,
                  SaNameT              *node_name
               )
{

   m_AVM_LOG_FUNC_ENTRY("avm_map_avd2fsm");
   switch(msg_type)
   {
      case AVD_AVM_NODE_SHUTDOWN_RESP_MSG:
      {   
        *node_name = avd_avm->evt.avd_evt->avd_avm_msg.shutdown_resp.node_name;
         avd_avm->fsm_evt_type = AVM_EVT_NODE_SHUTDOWN_RESP;
      }
      break;

      case AVD_AVM_NODE_FAILOVER_RESP_MSG:
      {   
         *node_name = avd_avm->evt.avd_evt->avd_avm_msg.failover_resp.node_name;
         avd_avm->fsm_evt_type = AVM_EVT_NODE_FAILOVER_RESP;
      }
      break;
            
      case AVD_AVM_NODE_RESET_REQ_MSG:
      {   
         *node_name = avd_avm->evt.avd_evt->avd_avm_msg.reset_req.node_name;
         avd_avm->fsm_evt_type = AVM_EVT_AVD_RESET_REQ;
      }
      break;
   
      case AVD_AVM_FAULT_DOMAIN_REQ_MSG:
      {   
         /* Future */
      }
      break;

      default:
      {  
         *fsm_evt_type = AVM_EVT_INVALID;
      }
   }
   return NCSCC_RC_SUCCESS;
}

extern uns32
avm_find_chassis_id( 
                     SaHpiEntityPathT *entity_path,
                     uns32            *chassis_id
                   )
{
   uns32 i  = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_find_chassis_id");
   for(i = 0; i < SAHPI_MAX_ENTITY_PATH; i++)
   {
      if(entity_path->Entry[i].EntityType == SAHPI_ENT_ROOT)
      {
         break;
      }
   }

   if(0 == i)
   {
      m_NCS_DBG_PRINTF("\n No chassis in our system-only root");
      rc = NCSCC_RC_FAILURE;
   }else
   {
#ifdef HPI_A
      *chassis_id = entity_path->Entry[i-1].EntityInstance;
#else
      *chassis_id = entity_path->Entry[i-1].EntityLocation;
#endif
   }

   return rc;
}


/********************************************************************
 * Name          : avm_convert_entity_path_to_string
 *
 * Description   : This function converts the entity path structure
 *                 to a string
 *
 * Arguments     : entity_path   - SaHpiEntityPathT(Entity Path Structure)
 *                 str*          - pointer to string.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : This function invoking will automatically allocate
 *                 memory and hence have to free the memory for the 
 *                 allocated string.
 ********************************************************************/
extern uns32
avm_convert_entity_path_to_string(
                                    SaHpiEntityPathT entity_path,
                                    uns8           **entity_path_str
                                 )
{
   uns32 len = 0;
   uns32 i; 
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 index;

   m_AVM_LOG_FUNC_ENTRY("avm_convert_entity_path_to_string");
   for(i = 0; i < SAHPI_MAX_ENTITY_PATH; i++)
   {
      if(entity_path.Entry[i].EntityType == SAHPI_ENT_ROOT)
      {
         break;
      }else
      {
#ifdef HPI_A
         if((SAHPI_ENT_UNSPECIFIED > entity_path.Entry[i].EntityType) ||
            (SAHPI_ENT_SUBBOARD_CARRIER_BLADE < entity_path.Entry[i].EntityType))
         {
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path.Entry[i].EntityType);
            rc =  NCSCC_RC_FAILURE;
            break;
         }
         if((SAHPI_ENT_UNSPECIFIED <= entity_path.Entry[i].EntityType) &&
            (SAHPI_ENT_BATTERY >= entity_path.Entry[i].EntityType))
         {
            index = entity_path.Entry[i].EntityType;
         }else
         {
            if((SAHPI_ENT_ROOT <= entity_path.Entry[i].EntityType) &&
               (SAHPI_ENT_SUBBOARD_CARRIER_BLADE >= entity_path.Entry[i].EntityType))
            {
               index = SAHPI_ENT_BATTERY + entity_path.Entry[i].EntityType - SAHPI_ENT_ROOT + 4;
            }else
            {
               if(SAHPI_ENT_CHASSIS_SPECIFIC == entity_path.Entry[i].EntityType)
               {
                  index = SAHPI_ENT_BATTERY + 1;
               }else 
               if(SAHPI_ENT_BOARD_SET_SPECIFIC == entity_path.Entry[i].EntityType)
               {
                  index = SAHPI_ENT_BATTERY + 2;
               }else
               if(SAHPI_ENT_OEM_SYSINT_SPECIFIC == entity_path.Entry[i].EntityType)
               {
                  index = SAHPI_ENT_BATTERY + 3;
               }else
               {
                  m_AVM_LOG_INVALID_VAL_FATAL(entity_path.Entry[i].EntityType);
                  rc = NCSCC_RC_FAILURE;
                  break;
               }
            }
         } 
#else
         if((SAHPI_ENT_UNSPECIFIED > entity_path.Entry[i].EntityType)
#ifdef HPI_B_02
             || (SAHPI_ENT_OEM < entity_path.Entry[i].EntityType))
#else
             || (SAHPI_ENT_PHYSICAL_SLOT < entity_path.Entry[i].EntityType))
#endif
         {
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path.Entry[i].EntityType);
            rc =  NCSCC_RC_FAILURE;
            break;
         }

         index = 0;
         do {
            if (gl_hpi_ent_type_list[index].etype_val == entity_path.Entry[i].EntityType) {
               break;
            }
            index++;
#ifdef HPI_B_02
         } while (gl_hpi_ent_type_list[index -1].etype_val != SAHPI_ENT_OEM);

         if (gl_hpi_ent_type_list[index -1].etype_val == SAHPI_ENT_OEM) {
            /* reached end of list and did not find entity type */
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path.Entry[i].EntityType);
            rc = NCSCC_RC_FAILURE;
            break;
         }
#else
         } while (gl_hpi_ent_type_list[index -1].etype_val != AMC_SUB_SLOT_TYPE);

         if (gl_hpi_ent_type_list[index -1].etype_val == AMC_SUB_SLOT_TYPE) {
            /* reached end of list and did not find entity type */
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path.Entry[i].EntityType);
            rc = NCSCC_RC_FAILURE;
            break;
         }
#endif
#endif
         len += m_NCS_STRLEN(gl_hpi_ent_type_list[index].etype_str);
         len += 7;
      }
   }

   if(NCSCC_RC_SUCCESS != rc)
   {
      return NCSCC_RC_FAILURE;
   }

   /* For the root entity the length will be zero and hence we dont want to
   ** allocate any memory and create issues during free
   */
   if(len != 0)
      *entity_path_str = m_MMGR_ALLOC_AVM_DEFAULT_VAL(len);
   else
      return NCSCC_RC_FAILURE;

   if(NULL == *entity_path_str)
   {
      m_NCS_DBG_PRINTF("\n Malloc failed");
      return NCSCC_RC_FAILURE;
   }

   rc = hpi_convert_entitypath_string(&entity_path, *entity_path_str);
   *((*entity_path_str) + len - 1) = '\0';
   return rc;
}

extern uns32
avm_refine_fault_domain(
                        AVM_ENT_INFO_T      *ent_info, 
                        AVM_FAULT_DOMAIN_T **fault_domain
                       )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_AVM_LOG_FUNC_ENTRY("avm_refine_fault_domain");

   *fault_domain = m_MMGR_ALLOC_AVM_FAULT_DOMAIN;
   memcpy(&(*fault_domain)->ent_path, &ent_info->entity_path, sizeof(SaHpiEntityPathT));
   (*fault_domain)->next = AVM_FAULT_DOMAIN_NULL;

   return rc;
}

extern uns32 
avm_scope_extract_criteria(AVM_ENT_INFO_T *ent_info ,uns32 **child_impact)
{
   
   uns32 node_insert = NCSCC_RC_FAILURE;   
   **child_impact = NCSCC_RC_FAILURE;   
   m_AVM_LOG_FUNC_ENTRY("avm_scope_extract_criteria");
   if((TRUE == ent_info->is_node) && (TRUE == ent_info->is_fru))
   {  
      switch(ent_info->current_state)
      {
         case AVM_ENT_RESET_REQ:
         {
            /* Lets once again send the shutdown request to AVD */ 
            ent_info->current_state = AVM_ENT_EXTRACT_REQ;
            node_insert = NCSCC_RC_SUCCESS;
            **child_impact = NCSCC_RC_SUCCESS;   
         }
         case AVM_ENT_NOT_PRESENT:
         case AVM_ENT_INACTIVE:
         {
            node_insert = NCSCC_RC_FAILURE;
            break;
         }
         case AVM_ENT_ACTIVE:
         case AVM_ENT_FAULTY:
         {
            ent_info->current_state = AVM_ENT_EXTRACT_REQ;
            node_insert = NCSCC_RC_SUCCESS;
            **child_impact = NCSCC_RC_SUCCESS;   
            break;
         }
         case AVM_ENT_INVALID:
         case AVM_ENT_EXTRACT_REQ:
         default:
         {
            node_insert = NCSCC_RC_FAILURE;
         }
      }   
   }
   return node_insert;
}

extern uns32 
avm_scope_reset_criteria(AVM_ENT_INFO_T *ent_info ,uns32 **child_impact)
{
   
   uns32 node_insert = NCSCC_RC_FAILURE;   
   **child_impact = NCSCC_RC_FAILURE; /* Software Fault  */
   m_AVM_LOG_FUNC_ENTRY("avm_scope_reset_criteria");
   if(TRUE == ent_info->is_node)
   {  
      switch(ent_info->current_state)
      {
         case AVM_ENT_RESET_REQ:
         case AVM_ENT_NOT_PRESENT:
         case AVM_ENT_INACTIVE:
         {
            node_insert = NCSCC_RC_FAILURE;
            break;
         }
         case AVM_ENT_ACTIVE:
         case AVM_ENT_FAULTY:
         {
            ent_info->current_state = AVM_ENT_RESET_REQ;
            node_insert = NCSCC_RC_SUCCESS;
            break;
         }
         case AVM_ENT_INVALID:
         case AVM_ENT_EXTRACT_REQ:
         default:
         {
            node_insert = NCSCC_RC_FAILURE;
         }
      }   
   }
   return node_insert;
}

extern uns32 
avm_scope_sensor_assert_criteria(AVM_ENT_INFO_T *ent_info, uns32 **child_impact)
{
   
   uns32 node_insert = NCSCC_RC_FAILURE; 
   **child_impact = NCSCC_RC_FAILURE; /* Software Fault  */
  
   m_AVM_LOG_FUNC_ENTRY("avm_scope_sensor_assert_criteria");
   if(TRUE == ent_info->is_node)
   {  
      switch(ent_info->current_state)
      {
         case AVM_ENT_FAULTY:
         case AVM_ENT_RESET_REQ:
         {
            ent_info->sensor_assert = NCSCC_RC_SUCCESS;
         }
         case AVM_ENT_NOT_PRESENT:
         case AVM_ENT_INACTIVE:
         {
            node_insert = NCSCC_RC_FAILURE;
            break;
         }
         case AVM_ENT_ACTIVE:
         {
            if(ent_info->sensor_assert != NCSCC_RC_SUCCESS)
            {
               ent_info->current_state = AVM_ENT_FAULTY;
               ent_info->previous_state = AVM_ENT_ACTIVE;
               ent_info->sensor_assert = NCSCC_RC_SUCCESS;
               node_insert = NCSCC_RC_SUCCESS; 
            }
            break;
         }
         default:
         {
            node_insert = NCSCC_RC_FAILURE;
         }         
      }   
   }
   return node_insert;
}

extern uns32 
avm_scope_sensor_deassert_criteria(AVM_ENT_INFO_T *ent_info ,uns32 **child_impact)
{
   
   uns32 node_insert = NCSCC_RC_FAILURE;   
   **child_impact = NCSCC_RC_FAILURE;/* Software Fault Domain , No impact no Child */
   m_AVM_LOG_FUNC_ENTRY("avm_scope_sensor_deassert_criteria");
   if(TRUE == ent_info->is_node)
   {  
      switch(ent_info->current_state)
      {
         case AVM_ENT_NOT_PRESENT:
         case AVM_ENT_INACTIVE:
         case AVM_ENT_EXTRACT_REQ:
         case AVM_ENT_RESET_REQ:
         case AVM_ENT_FAULTY:
         {
            ent_info->sensor_assert = NCSCC_RC_FAILURE;
            node_insert = NCSCC_RC_FAILURE;
            break;
         }
         case AVM_ENT_ACTIVE:
         {
            node_insert = NCSCC_RC_SUCCESS; 
            ent_info->sensor_assert = NCSCC_RC_FAILURE;
            break;
         }
         case AVM_ENT_INVALID:   
         default:
         {
            node_insert = NCSCC_RC_FAILURE;
         }
      }   
   }
   return node_insert;
}

extern uns32 
avm_scope_surp_extract_criteria(AVM_ENT_INFO_T *ent_info , uns32 **child_impact)
{
   
   uns32 node_insert = NCSCC_RC_FAILURE;
   **child_impact = NCSCC_RC_FAILURE;
   
   m_AVM_LOG_FUNC_ENTRY("avm_scope_surp_extract_criteria");
   if((TRUE == ent_info->is_node) && (TRUE == ent_info->is_fru))
   {
      switch(ent_info->current_state)
      {
         case AVM_ENT_NOT_PRESENT:
         {
            m_NCS_DBG_PRINTF("\n This event in this state is atrocious");
            break;
         }
         case AVM_ENT_EXTRACT_REQ:
         case AVM_ENT_FAULTY:
         case AVM_ENT_RESET_REQ:
         case AVM_ENT_INACTIVE:
         case AVM_ENT_ACTIVE:
         {
           /* Child Should Be impacted */
            avm_reset_ent_info(ent_info);
            ent_info->current_state = AVM_ENT_NOT_PRESENT;
            node_insert = NCSCC_RC_SUCCESS; 
            **child_impact = NCSCC_RC_SUCCESS;
            break;
         }
         case AVM_ENT_INVALID:
         default:
         {
            node_insert = NCSCC_RC_FAILURE;
         }
      }   
   }
   return node_insert;
}

extern uns32 
avm_scope_active_criteria(
                           AVM_CB_T       *avm_cb,  
                           AVM_ENT_INFO_T *ent_info,
                           uns32          **child_impact
                         )
{
   uns32 node_insert =  NCSCC_RC_FAILURE;   
   uns32 rc = NCSCC_RC_SUCCESS;
   **child_impact = NCSCC_RC_FAILURE; /* Recovery from hardware Fault , Bring back child too */ 

   AVM_NODE_RESET_RESP_T resp;

   m_AVM_LOG_FUNC_ENTRY("avm_scope_active_criteria");
   if(TRUE == ent_info->is_node)
   {  
      switch(ent_info->current_state)
      {
         case AVM_ENT_RESET_REQ:
         case AVM_ENT_NOT_PRESENT:
         case AVM_ENT_INACTIVE:
         case AVM_ENT_ACTIVE:
         {
            node_insert = NCSCC_RC_FAILURE;
            break;
         }

         case AVM_ENT_EXTRACT_REQ:
         {
            node_insert = NCSCC_RC_SUCCESS;
            **child_impact = NCSCC_RC_SUCCESS;  

            rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_GRACEFUL_REBOOT);
            resp = ((NCSCC_RC_SUCCESS == rc) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE);
            avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);
            ent_info->current_state = AVM_ENT_ACTIVE;
            ent_info->previous_state = AVM_ENT_EXTRACT_REQ;
 
            break;
         }

         case AVM_ENT_INVALID:   
         default:
         {
            node_insert = NCSCC_RC_FAILURE;
         }
   
      }   
   }
   return node_insert;
}

extern uns32 
avm_scope_fault_domain_req_criteria(AVM_ENT_INFO_T *ent_info , uns32 **child_impact)
{
   
   uns32 node_insert =  NCSCC_RC_FAILURE;   
   **child_impact = NCSCC_RC_FAILURE;
   m_AVM_LOG_FUNC_ENTRY("avm_scope_fault_domain_req_criteria");

   if(TRUE == ent_info->is_node)
   {  
      switch(ent_info->current_state)
      {
         case AVM_ENT_NOT_PRESENT:
         case AVM_ENT_INACTIVE:
         {
            node_insert = NCSCC_RC_FAILURE;
         }
         break;

         case AVM_ENT_RESET_REQ:
         case AVM_ENT_ACTIVE:
         case AVM_ENT_EXTRACT_REQ:
         {
            node_insert = NCSCC_RC_SUCCESS;
            **child_impact = NCSCC_RC_SUCCESS;
         }
         break;

         case AVM_ENT_INVALID:
         default:
         {
            node_insert = NCSCC_RC_FAILURE;
         } 
      }   
   }
   return node_insert;
}
extern uns32 
avm_scope_fault_criteria(
                           AVM_CB_T            *avm_cb,
                           AVM_ENT_INFO_T      *ent_info, 
                           AVM_SCOPE_EVENTS_T  fault_type,
                           uns32               *impact
                        )
{
   uns32 rc;

   m_AVM_LOG_FUNC_ENTRY("avm_scope_fault_criteria");
   switch(fault_type)
   {
      case AVM_SCOPE_EXTRACTION:
      {
         rc = avm_scope_extract_criteria(ent_info,&impact);
      }  
      break;

      case AVM_SCOPE_RESET:
      {
         rc = avm_scope_reset_criteria(ent_info,&impact); 
      }   
      break;

      case AVM_SCOPE_SENSOR_ASSERT:
      {
         rc = avm_scope_sensor_assert_criteria(ent_info,&impact);
      }
      break;

      case AVM_SCOPE_SENSOR_DEASSERT:
      {
         rc = avm_scope_sensor_deassert_criteria(ent_info,&impact);
      }
      break;

      case AVM_SCOPE_SURP_EXTRACTION:
      {
         rc = avm_scope_surp_extract_criteria(ent_info,&impact);
      }
      break;

      case AVM_SCOPE_ACTIVE:
      {
         rc = avm_scope_active_criteria(avm_cb, ent_info,&impact);
      }
      break;

      case AVM_SCOPE_FAULT_DOMAIN_REQ:
      {
         rc = avm_scope_fault_domain_req_criteria(ent_info,&impact);
      }
      break;

      default:
      {
         m_NCS_DBG_PRINTF("\n Fault Event not mapped perfectly");
         rc = NCSCC_RC_FAILURE; 
         break;
      }

   }
   return rc; 
}

extern uns32
avm_scope_fault_children(
                           AVM_CB_T                *avm_cb, 
                           AVM_ENT_INFO_T          *ent_info, 
                           AVM_LIST_HEAD_T         *head, 
                           AVM_LIST_NODE_T         *fault_node, 
                           AVM_SCOPE_EVENTS_T       fault_type
                        )
{
   uns32 rc;
   uns32 child_impact;

   AVM_ENT_INFO_LIST_T  *child;
   AVM_LIST_NODE_T      *temp_node; 
   
   m_AVM_LOG_FUNC_ENTRY("avm_scope_fault_children");
   rc = avm_scope_fault_criteria(avm_cb, ent_info, fault_type, &child_impact);   
   if(NCSCC_RC_SUCCESS == rc)
   {
      temp_node =  m_MMGR_ALLOC_AVM_AVD_LIST_NODE;
      if(AVM_LIST_NODE_NULL == temp_node) 
      {
         m_NCS_DBG_PRINTF("\n Malloc failed for AvM List Node");
         return NCSCC_RC_FAILURE;
      }
      memcpy(&temp_node->node_name, &ent_info->node_name, sizeof(SaNameT));
      
      temp_node->next  = AVM_LIST_NODE_NULL;
      if(head->node    == AVM_LIST_NODE_NULL)
      {
         head->node = temp_node;
         fault_node = head->node; 
         head->count++;
      }else
      {
         fault_node->next = temp_node;
         fault_node       = fault_node->next;
         head->count++;
      }

   if(NCSCC_RC_SUCCESS == child_impact)
     {
       for(child =  ent_info->child; child != NULL; child = child->next)
       {
         rc = avm_scope_fault_children(avm_cb, child->ent_info, head, fault_node, fault_type);
         if(NCSCC_RC_FAILURE == rc)
         {
            m_NCS_DBG_PRINTF("\n AvM Scope Extract Children failed");
            break;
         }   
       }
     }
   }
   return rc;   
}

extern uns32
avm_handle_fault(
                  AVM_CB_T                *avm_cb, 
                  AVM_LIST_HEAD_T         *head, 
                  AVM_ENT_INFO_T          *ent_info, 
                  AVM_SCOPE_EVENTS_T       fault_type
                )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_handle_fault");
   switch(fault_type)
   {
      case AVM_SCOPE_RESET:
      case AVM_SCOPE_EXTRACTION:
      {
         rc = avm_avd_node_shutdown_req(avm_cb, head);
         break; 
      }
      case AVM_SCOPE_SENSOR_ASSERT:
      {
         rc = avm_avd_node_failover_req(avm_cb, head, AVM_NODES_DISABLED);
         break;
      }
      
      case AVM_SCOPE_SENSOR_DEASSERT:
      {
         rc = avm_avd_node_operstate(avm_cb, head, AVM_NODES_ENABLED);
         break;
      }
      case AVM_SCOPE_SURP_EXTRACTION:
      {
         rc = avm_avd_node_operstate(avm_cb, head, AVM_NODES_ABSENT);
         break;
      }

      case AVM_SCOPE_ACTIVE:
      {
         /* rc = avm_avd_node_reset_resp(avm_cb, head, NCSCC_RC_SUCCESS); */
         break;
      }
      case AVM_SCOPE_FAULT_DOMAIN_REQ:
      {
         rc = avm_avd_fault_domain_resp(avm_cb, head, ent_info->node_name);
         break;
      }

      default:
      {
         m_NCS_DBG_PRINTF("\n AvM cant understand the Fault Type mentioned");
         rc = NCSCC_RC_FAILURE;
      }
   }
   return rc;
}   

extern uns32 
avm_scope_fault(
                  AVM_CB_T             *avm_cb, 
                  AVM_ENT_INFO_T       *ent_info, 
                  AVM_SCOPE_EVENTS_T   fault_type
               )
{

   AVM_LIST_HEAD_T head;
   AVM_ENT_INFO_T  *fault_ent_info;

   AVM_FAULT_DOMAIN_T *fault_domain =  AVM_FAULT_DOMAIN_NULL;
   AVM_FAULT_DOMAIN_T *fault_node;
   AVM_FAULT_DOMAIN_T *temp1        =  AVM_FAULT_DOMAIN_NULL;
   AVM_FAULT_DOMAIN_T *temp2        =  AVM_FAULT_DOMAIN_NULL; 

   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_scope_fault");
   head.count = 0;
   head.node  = AVM_LIST_NODE_NULL;      

   rc = avm_refine_fault_domain(ent_info, &fault_domain);

   if(AVM_FAULT_DOMAIN_NULL == fault_domain)
   {
      m_NCS_DBG_PRINTF("Memory Failure in avm refine fault domain");
      return NCSCC_RC_FAILURE;
   } 

   for(fault_node = fault_domain; fault_node != NULL; fault_node = fault_node->next)
   {
      fault_ent_info = (AVM_ENT_INFO_T*) avm_find_ent_info(avm_cb, &fault_node->ent_path);

      if(AVM_ENT_INFO_NULL == fault_ent_info)
      {
         m_AVM_LOG_GEN("Entity not found:", fault_node->ent_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_ERROR);
      }else
      {
         rc = avm_scope_fault_children(avm_cb, fault_ent_info, &head, AVM_LIST_NODE_NULL, fault_type);
      }
   }

   rc = avm_handle_fault(avm_cb, &head, ent_info, fault_type);   
   
   temp1 = fault_domain;
   while(AVM_FAULT_DOMAIN_NULL != temp1)
   {
      temp2 = temp1;
      temp1 = temp1->next;
      m_MMGR_FREE_AVM_FAULT_DOMAIN(temp2);
    
   }
   return rc;   
}

extern uns32 
avm_add_dependent(
                  AVM_ENT_INFO_T *dependent,
                  AVM_ENT_INFO_T *dependee
                 )   
{
   AVM_ENT_INFO_LIST_T *temp;
   
   m_AVM_LOG_FUNC_ENTRY("avm_add_dependent");
   if((AVM_ENT_INFO_NULL == dependee) || (AVM_ENT_INFO_NULL == dependent))
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;
   }
   
   if(dependee->dependents == AVM_ENT_INFO_LIST_NULL)
   {    
      temp = m_MMGR_ALLOC_AVM_ENT_INFO_LIST;
      if(AVM_ENT_INFO_LIST_NULL == temp)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(0);
         return NCSCC_RC_FAILURE;
      }    
      
      temp->ep_str         = &dependent->ep_str;
      temp->ent_info       = dependent;
      temp->next           = dependee->dependents;   
      dependee->dependents = temp;
  }else
  {
     for(temp = dependee->dependents; temp != AVM_ENT_INFO_LIST_NULL; temp = temp->next)
     {
        if(!m_NCS_MEMCMP(temp->ent_info->ep_str.name, dependent->ep_str.name, AVM_MAX_INDEX_LEN))
        {
           m_AVM_LOG_GEN_EP_STR("Entity in the list", dependee->dependents->ent_info->ep_str.name, NCSFL_SEV_INFO);
           return NCSCC_RC_SUCCESS;   
        }
     }   
     temp = m_MMGR_ALLOC_AVM_ENT_INFO_LIST;
     if(AVM_ENT_INFO_LIST_NULL == temp)
     {
        m_AVM_LOG_INVALID_VAL_FATAL(0);
        return NCSCC_RC_FAILURE;
     }    

     temp->ep_str         = &dependent->ep_str;
     temp->ent_info       = dependent;
     temp->next           = dependee->dependents;   
     dependee->dependents = temp;
     return NCSCC_RC_SUCCESS;   
  }     
  
  return NCSCC_RC_SUCCESS; 
}   

extern uns32 
avm_hisv_api_cmd(
                 AVM_ENT_INFO_T *ent_info,
                 HISV_API_CMD    api_cmd,
                 uns32           arg
                )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 chassis_id;
   uns8  *entity_path     = NULL;
   AVM_CB_T  *avm_cb = NULL;
   uns8  bootbank_number;

   m_AVM_LOG_FUNC_ENTRY("avm_hisv_api_cmd");
   rc = avm_convert_entity_path_to_string(ent_info->entity_path, &entity_path);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_GEN("Cant convert the entity path to string:", ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   rc = avm_find_chassis_id(&ent_info->entity_path, &chassis_id);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_GEN_EP_STR("Cant get the Chassis id:", ent_info->ep_str.name,  NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;       
   }

   switch(api_cmd)
   {
      case HS_ACTION_REQUEST:
      case HS_RESOURCE_INACTIVE_SET:
      case HS_RESOURCE_ACTIVE_SET:
      {
         rc = hpl_manage_hotswap(chassis_id, entity_path, api_cmd, arg);
      }
      break;


      case HISV_RESOURCE_RESET:
      {
           /* If AvM succeeds in doing this, let the management know about this
           ** by sending a TRAP. This is now achieved by sending a reset indication
           ** to fault management and it will do the honors.
           */


        /* dt:20-07-06
        ** When the reset is issued for self sometimes this bcast message
        ** will not reach the intended party esp. FM and if this message 
        ** is not received by the fm instead of ignoring the HB loss due to
        ** node going down it will process and power down the card which is
        ** undesired.
        **
        ** Hence sending the node reset ind each time irrespective of the
        ** API return status.
        ** 
        ** Note that AvM will inform AVD that the node is reset irrespective
        ** of the return status of the api and AVD will do the exit cluster
        */

        if(1)
        {
           avm_cb = (AVM_CB_T*)ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);
           if(AVM_CB_NULL == avm_cb)
           {
              m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, 
                                                NCSFL_SEV_CRITICAL);
              rc = NCSCC_RC_FAILURE;
           }
           else 
           {
              avm_notify_fm_node_reset(avm_cb, &ent_info->entity_path);
              ncshm_give_hdl(g_avm_hdl);
           }
           rc = hpl_resource_reset(chassis_id, entity_path, arg);
        }
      }
      break;
      
      case HISV_RESOURCE_POWER:
      {
         rc = hpl_resource_power_set(chassis_id, entity_path, arg);
      }
      break;

      case HISV_PAYLOAD_BIOS_SWITCH:
      {
         avm_cb = (AVM_CB_T*)ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);
         if(AVM_CB_NULL == avm_cb)
         {
            m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE,NCSFL_SEV_CRITICAL);
            rc = NCSCC_RC_FAILURE;
            break;
         }
         rc = hpl_bootbank_get (chassis_id, entity_path, &bootbank_number);
         if (rc != NCSCC_RC_SUCCESS)
         {
            ncshm_give_hdl(g_avm_hdl);
            break;
         }
         if (bootbank_number == 0)
         {
            ent_info->dhcp_serv_conf.bios_upgd_state = 
                    (ent_info->dhcp_serv_conf.bios_upgd_state == BIOS_TMR_STOPPED) ?  BIOS_STOP_BANK_0 : BIOS_EXP_BANK_0;
            m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);  
            ncshm_give_hdl(g_avm_hdl);
            rc = hpl_bootbank_set (chassis_id, entity_path, 1);
         }
         else if (bootbank_number == 1)
         {
            ent_info->dhcp_serv_conf.bios_upgd_state = 
                    (ent_info->dhcp_serv_conf.bios_upgd_state == BIOS_TMR_STOPPED) ?  BIOS_STOP_BANK_1 : BIOS_EXP_BANK_1;
            m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);  
            ncshm_give_hdl(g_avm_hdl);
            rc = hpl_bootbank_set (chassis_id, entity_path, 0);
         }  
         else
         {
            ncshm_give_hdl(g_avm_hdl);
            rc = NCSCC_RC_FAILURE;
            break;
         }
         /* Reset the payload */
      /*   rc = hpl_resource_reset(chassis_id, entity_path, arg); - not required always - TBD - JPL */
      }
      break;

      default:
      {
         m_AVM_LOG_INVALID_VAL_FATAL(api_cmd);
         rc = NCSCC_RC_FAILURE;
      } 
   }
   
   if(NULL != entity_path)
   {
      m_MMGR_FREE_AVM_DEFAULT_VAL(entity_path);
   }

   return rc;
}

EXTERN_C uns32
avm_get_hpi_hs_state(
                 AVM_ENT_INFO_T *ent_info,
                 uns32           *arg
                )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 chassis_id;
   uns8  *entity_path     = NULL;

   m_AVM_LOG_FUNC_ENTRY("avm_get_hpi_hs_state");
   rc = avm_convert_entity_path_to_string(ent_info->entity_path, &entity_path);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_GEN("Cant convert the entity path to string:", ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   rc = avm_find_chassis_id(&ent_info->entity_path, &chassis_id);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_GEN_EP_STR("Cant get the Chassis id:", ent_info->ep_str.name,  NCSFL_SEV_ERROR);
      if(NULL != entity_path)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(entity_path);
      }

      return NCSCC_RC_FAILURE;       
   }

   rc = hpl_config_hs_state_get(chassis_id, entity_path, arg);
   
   if(NULL != entity_path)
   {
      m_MMGR_FREE_AVM_DEFAULT_VAL(entity_path);
   }

   return rc;
}

extern uns32
avm_bld_validation_info(
                          AVM_CB_T *cb,
                          NCS_HW_ENT_TYPE_DESC *ent_desc_info
                       )
{
   uns32                    rc;   
   uns32                    i,j;
   AVM_ENT_DESC_NAME_T      desc_name;
   AVM_VALID_INFO_T        *valid_info; 
   NCS_HW_ENT_TYPE_DESC    *ptr; 
 
   m_AVM_LOG_FUNC_ENTRY("avm_bld_validation_info");
   
   if(ent_desc_info == NCS_HW_ENT_TYPE_DESC_NULL)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;   
   }      
   
   for(ptr = ent_desc_info; NCS_HW_ENT_TYPE_DESC_NULL != ptr; ptr = ptr->next)
   {
      memset(desc_name.name, '\0', NCS_MAX_INDEX_LEN);
      
      desc_name.length = m_NCS_STRLEN(ptr->entity_name);
      memcpy(desc_name.name, ptr->entity_name, desc_name.length);

      valid_info = avm_add_valid_info(cb, &desc_name);
      if(AVM_VALID_INFO_NULL == valid_info)
      {
         m_AVM_LOG_INVALID_VAL_FATAL(0);
         return NCSCC_RC_FAILURE;
      }   
        
      valid_info->entity_type = ptr->entity_type;
      valid_info->is_node     = ptr->isNode;
      valid_info->is_fru      = ptr->is_fru;

      valid_info->inv_data.product_name.DataLength =  m_NCS_STRLEN(ptr->fru_product_name);
      memcpy(valid_info->inv_data.product_name.Data, ptr->fru_product_name, valid_info->inv_data.product_name.DataLength);
      
      valid_info->inv_data.product_version.DataLength =  m_NCS_STRLEN(ptr->fru_product_version);
      memcpy(valid_info->inv_data.product_version.Data, ptr->fru_product_version, valid_info->inv_data.product_version.DataLength);
    
      valid_info->parent_cnt = ptr->num_possible_parents;
   
      for(i = 0; i < ptr->num_possible_parents; i++)
      {
         valid_info->location[i].parent.length = m_NCS_STRLEN(ptr->location_range[i].parent_ent); 
         if(valid_info->location[i].parent.length > NCS_MAX_INDEX_LEN)
         { 
            valid_info->location[i].parent.length = NCS_MAX_INDEX_LEN; 
         }
         
         memcpy(valid_info->location[i].parent.name, ptr->location_range[i].parent_ent, valid_info->location[i].parent.length);

         for(j = 0; j < MAX_POSSIBLE_LOC_RANGES; j++)
         {
            valid_info->location[i].range[j].min = ptr->location_range[i].valid_location.min[j]; 
            valid_info->location[i].range[j].max = ptr->location_range[i].valid_location.max[j]; 
         }
      }   
   }

   rc = avm_bld_valid_info_parent_child_relation(cb);

   return NCSCC_RC_SUCCESS;
} 

extern uns32
avm_bld_valid_info_parent_child_relation(AVM_CB_T *cb)
{
   uns32   rc = NCSCC_RC_SUCCESS;
   uns32   i;
   
   AVM_ENT_DESC_NAME_T    desc_name;
   AVM_ENT_DESC_NAME_T    parent_desc_name;
   AVM_VALID_INFO_T *valid_info; 
   AVM_VALID_INFO_T *parent_valid_info; 

   m_AVM_LOG_FUNC_ENTRY("avm_bld_valid_info_parent_child_relation");
   memset(desc_name.name, '\0', NCS_MAX_INDEX_LEN);

   for(valid_info = (AVM_VALID_INFO_T*) ncs_patricia_tree_getnext(&cb->db.valid_info_anchor, desc_name.name);
       valid_info != AVM_VALID_INFO_NULL; 
       valid_info = (AVM_VALID_INFO_T*) ncs_patricia_tree_getnext(&cb->db.valid_info_anchor, desc_name.name) 
      )
   {
      for(i = 0; i < valid_info->parent_cnt; i++)
      {
    memset(parent_desc_name.name, '\0', NCS_MAX_INDEX_LEN);
    memcpy(parent_desc_name.name, valid_info->location[i].parent.name, NCS_MAX_INDEX_LEN);
         parent_valid_info = (AVM_VALID_INFO_T*)ncs_patricia_tree_get(&cb->db.valid_info_anchor, parent_desc_name.name);
         valid_info->parents[i] = parent_valid_info;
     }

     memset(desc_name.name, '\0', NCS_MAX_INDEX_LEN);
     memcpy(desc_name.name, valid_info->desc_name.name, valid_info->desc_name.length);
   }   

   return rc;
}

static uns32
avm_validate_loc_range(
                        AVM_ENT_INFO_T     *ent_info,
                        SaHpiEntityT        entity,
                        AVM_VALID_INFO_T   *valid_info
                      )
{
   uns32 i, j;

   m_AVM_LOG_FUNC_ENTRY("avm_validate_loc_range");
   if(entity.EntityType == valid_info->entity_type)
   {
      for(i = 0; i < valid_info->parent_cnt; i++)
      {
         if(ent_info->parent_valid_info == valid_info->parents[i])
         {
            for(j = 0; j < MAX_POSSIBLE_LOC_RANGES; j++)
#ifdef HPI_A
            if((entity.EntityInstance >= valid_info->location[i].range[j].min) &&
               (entity.EntityInstance <= valid_info->location[i].range[j].max))   
#else
            if((entity.EntityLocation >= valid_info->location[i].range[j].min) &&
               (entity.EntityLocation <= valid_info->location[i].range[j].max))   
#endif
            {
               return NCSCC_RC_SUCCESS;
            }    
         }
      } 
   }

   return NCSCC_RC_FAILURE;   
}

extern uns32
avm_add_dependent_list(
                        AVM_ENT_INFO_T *ent_info, 
                        AVM_ENT_INFO_T *dep_ent_info
                      )
{
   m_AVM_LOG_FUNC_ENTRY("avm_add_dependent_list");
   return NCSCC_RC_SUCCESS;      
}

extern uns32
avm_check_deployment(AVM_CB_T *cb)
{

   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 status = NCSCC_RC_SUCCESS;
   uns32 i;
   AVM_ENT_PATH_STR_T ep_str;
   AVM_ENT_INFO_T     *ent_info  = AVM_ENT_INFO_NULL;
   AVM_NODE_INFO_T    *node_info = AVM_NODE_INFO_NULL;  

   m_AVM_LOG_FUNC_ENTRY("avm_check_deployment");
   memset(ep_str.name, '\0', AVM_MAX_INDEX_LEN);
   ep_str.length = 0; 
   
   for(ent_info = (AVM_ENT_INFO_T*) avm_find_next_ent_str_info(cb, &ep_str);
      ent_info != AVM_ENT_INFO_NULL;
      ent_info = (AVM_ENT_INFO_T*) avm_find_next_ent_str_info(cb, &ep_str)
     )
   {

      memset(ep_str.name, '\0', AVM_MAX_INDEX_LEN);
      ep_str.length = m_NCS_OS_NTOHS(ent_info->ep_str.length);
      memcpy(ep_str.name, ent_info->ep_str.name, ep_str.length);
    
      if(SAHPI_ENT_ROOT == ent_info->entity_type)
      { 
         ent_info->row_status = NCS_ROW_ACTIVE;
         continue;
      }
    
      if(NCS_ROW_ACTIVE != ent_info->row_status)
      {
         continue;
      }   
      
      ent_info->row_status =  NCS_ROW_NOT_READY;
 
      ent_info->valid_info = (AVM_VALID_INFO_T*)ncs_patricia_tree_get(&cb->db.valid_info_anchor, ent_info->desc_name.name);
      ent_info->parent_valid_info = (AVM_VALID_INFO_T*)ncs_patricia_tree_get(&cb->db.valid_info_anchor, ent_info->parent_desc_name.name);

      if(AVM_VALID_INFO_NULL != ent_info->valid_info)
      {
         ent_info->is_node = ent_info->valid_info->is_node;
         ent_info->is_fru  = ent_info->valid_info->is_fru;
      }    

      rc = avm_populate_ent_info(cb, ent_info);

      if(NCSCC_RC_SUCCESS != rc)
      {
         status = NCSCC_RC_FAILURE;
         continue;
      } 

      if((TRUE == ent_info->is_node) && (0 == ent_info->node_name.length)) 
      {
         status = NCSCC_RC_FAILURE;
         avm_rmv_frm_parent(cb, ent_info);
         continue;
      }

      node_info = avm_add_node_name_info(cb, ent_info->node_name);
      if(AVM_NODE_INFO_NULL != node_info)
      {
    node_info->ent_info = ent_info;
      }else
      {
    m_AVM_LOG_GEN(" Node already in DB", ent_info->node_name.value, SA_MAX_NAME_LENGTH, NCSFL_SEV_WARNING);
    status = NCSCC_RC_FAILURE;
         avm_rmv_frm_parent(cb, ent_info);
         continue;
      }

      if(ent_info->act_policy == AVM_SHELF_MGR_ACTIVATION)
      {
          if(NCS_ROW_ACTIVE != ent_info->parent->row_status)
          {
             status = NCSCC_RC_FAILURE;
             avm_rmv_frm_parent(cb, ent_info);
             avm_rmv_node_info(cb, ent_info);
             continue;
          }

          m_AVM_LOG_GEN_EP_STR("Row Made Active", ent_info->ep_str.name, NCSFL_SEV_INFO);
          ent_info->row_status = NCS_ROW_ACTIVE;
    
          m_AVM_ADM_LOCK_ENT(cb, ent_info);
          m_AVM_ADM_UNLOCK_ENT(cb, ent_info);
          continue;
      }   

      if(ent_info->valid_info == AVM_VALID_INFO_NULL) 
      { 
         status = NCSCC_RC_FAILURE;
         avm_rmv_frm_parent(cb, ent_info);
         avm_rmv_node_info(cb, ent_info);
         continue;
      }

     if((ent_info->parent->row_status == NCS_ROW_ACTIVE) &&
        (ent_info->parent->entity_type == SAHPI_ENT_ROOT))  
     { 
        m_AVM_LOG_GEN_EP_STR("Row Made Active", ent_info->ep_str.name, NCSFL_SEV_INFO);
        ent_info->row_status =  NCS_ROW_ACTIVE;
        m_AVM_ADM_LOCK_ENT(cb, ent_info);
        m_AVM_ADM_UNLOCK_ENT(cb, ent_info);
        continue;
     }  

     if(ent_info->parent_valid_info == AVM_VALID_INFO_NULL)
     {
        status = NCSCC_RC_FAILURE;
        avm_rmv_frm_parent(cb, ent_info);
        avm_rmv_node_info(cb, ent_info);
        continue;
     }

     rc =  avm_validate_loc_range(ent_info, ent_info->entity_path.Entry[0], ent_info->valid_info);

     if(NCSCC_RC_SUCCESS != rc)
     { 
        status = NCSCC_RC_FAILURE;
        avm_rmv_frm_parent(cb, ent_info);
        avm_rmv_node_info(cb, ent_info);
        continue;
     } 

     if(ent_info->parent_valid_info->entity_type == ent_info->entity_path.Entry[1].EntityType)
     {
        for (i = 0; i < MAX_POSSIBLE_PARENTS; i ++)
        {
           if(ent_info->parent_valid_info == ent_info->valid_info->parents[i])   
           {  
              if(ent_info->parent->row_status == NCS_ROW_ACTIVE)
               {   
                  m_AVM_LOG_GEN_EP_STR("Row Made Active", ent_info->ep_str.name, NCSFL_SEV_INFO);
                  ent_info->row_status = NCS_ROW_ACTIVE;
                  m_AVM_ADM_LOCK_ENT(cb, ent_info);
                  m_AVM_ADM_UNLOCK_ENT(cb, ent_info);
               }else
               {
                  status = NCSCC_RC_FAILURE;
                  avm_rmv_frm_parent(cb, ent_info);
                  avm_rmv_node_info(cb, ent_info);
               } 
               break;
           }     
        }
     }else
     {
        avm_rmv_frm_parent(cb, ent_info);
        avm_rmv_node_info(cb, ent_info);
        status = NCSCC_RC_FAILURE;
     }
   }
   return status;
}

extern uns32
avm_check_dynamic_ent(
                       AVM_CB_T       *cb,
                       AVM_ENT_INFO_T *ent_info
                     )
{

   uns32 rc     = NCSCC_RC_SUCCESS;
   uns32 i;
   AVM_NODE_INFO_T   *node_info = AVM_NODE_INFO_NULL;  

   m_AVM_LOG_FUNC_ENTRY("avm_check_dynamic_ent");
   ent_info->valid_info = (AVM_VALID_INFO_T*)ncs_patricia_tree_get(&cb->db.valid_info_anchor, ent_info->desc_name.name);
   ent_info->parent_valid_info = (AVM_VALID_INFO_T*)ncs_patricia_tree_get(&cb->db.valid_info_anchor, ent_info->parent_desc_name.name);

   if(AVM_VALID_INFO_NULL != ent_info->valid_info)
   {
      ent_info->is_node = ent_info->valid_info->is_node;
      ent_info->is_fru  = ent_info->valid_info->is_fru;
   }    
   rc = avm_populate_ent_info(cb, ent_info);

   if(NCSCC_RC_SUCCESS != rc)
   {
      return NCSCC_RC_FAILURE;
   } 

   if((TRUE == ent_info->is_node) && (0 == ent_info->node_name.length))
   {
      avm_rmv_frm_parent(cb, ent_info);
      return NCSCC_RC_FAILURE;
   }

   node_info = avm_add_node_name_info(cb, ent_info->node_name);
   if(AVM_NODE_INFO_NULL != node_info)
   {
      node_info->ent_info = ent_info;
   }else
   {
      avm_rmv_frm_parent(cb, ent_info);
      m_AVM_LOG_GEN(" Node already in DB", ent_info->node_name.value, SA_MAX_NAME_LENGTH, NCSFL_SEV_WARNING);
      return NCSCC_RC_FAILURE;
   }

   if(ent_info->entity_type != SAHPI_ENT_ROOT)
   {
      if(ent_info->act_policy == AVM_SHELF_MGR_ACTIVATION)  
      {
         if(NCS_ROW_ACTIVE != ent_info->parent->row_status)
         {
            avm_rmv_frm_parent(cb, ent_info);
            avm_rmv_node_info(cb, ent_info);
            return NCSCC_RC_FAILURE;
         }
         m_AVM_LOG_GEN_EP_STR("Row Made Active", ent_info->ep_str.name, NCSFL_SEV_INFO);
         ent_info->row_status = NCS_ROW_ACTIVE;
         return NCSCC_RC_SUCCESS;
      }  

      if(ent_info->valid_info == AVM_VALID_INFO_NULL) 
      {
         avm_rmv_frm_parent(cb, ent_info);
         avm_rmv_node_info(cb, ent_info);
         return NCSCC_RC_FAILURE;
      }
   
      if((ent_info->parent->row_status == NCS_ROW_ACTIVE) &&
       (ent_info->parent->entity_type == SAHPI_ENT_ROOT))  
      {
         ent_info->row_status =  NCS_ROW_ACTIVE;
         return NCSCC_RC_SUCCESS;
      }  

      if(ent_info->parent_valid_info == AVM_VALID_INFO_NULL)
      {
         avm_rmv_frm_parent(cb, ent_info);
         avm_rmv_node_info(cb, ent_info);
         return NCSCC_RC_FAILURE;
      }

      rc =  avm_validate_loc_range(ent_info, ent_info->entity_path.Entry[0], ent_info->valid_info);

      if(NCSCC_RC_SUCCESS != rc)
      {
         avm_rmv_frm_parent(cb, ent_info);
         avm_rmv_node_info(cb, ent_info);
         return NCSCC_RC_FAILURE;
      }

      if(ent_info->parent_valid_info->entity_type == ent_info->entity_path.Entry[1].EntityType)
      {
         for (i = 0; i < MAX_POSSIBLE_PARENTS; i ++)
         {
            if(ent_info->parent_valid_info == ent_info->valid_info->parents[i])   
            {
               if(ent_info->parent->row_status == NCS_ROW_ACTIVE)
               {    
                  ent_info->row_status = NCS_ROW_ACTIVE;
               }else
               {
                  ent_info->row_status = NCS_ROW_NOT_READY;
                  avm_rmv_frm_parent(cb, ent_info);
                  avm_rmv_node_info(cb, ent_info);
                  return NCSCC_RC_FAILURE;
               }
               break;
            }     
         }
      }else
      {
         ent_info->row_status = NCS_ROW_NOT_READY;      
         avm_rmv_frm_parent(cb, ent_info);
         avm_rmv_node_info(cb, ent_info);
         return NCSCC_RC_FAILURE;
      }
   }else
   {
     ent_info->row_status = NCS_ROW_ACTIVE;
   }

   return NCSCC_RC_SUCCESS;
}

extern uns32
avm_add_root(AVM_CB_T *cb)
{
   AVM_ENT_PATH_STR_T   ep_str;
   SaHpiEntityPathT     ep;

   AVM_ENT_INFO_T     *ent_info;

   m_AVM_LOG_FUNC_ENTRY("avm_add_root");
   memset(ep_str.name, '\0', AVM_MAX_INDEX_LEN); 
   sprintf(ep_str.name, "{{%d,0}}", SAHPI_ENT_ROOT);
   ep_str.length = m_NCS_STRLEN(ep_str.name);
   
   memset(ep.Entry, '\0', sizeof(SaHpiEntityPathT));
   ep.Entry[0].EntityType     = SAHPI_ENT_ROOT;
#ifdef HPI_A
   ep.Entry[0].EntityInstance = 0;
#else
   ep.Entry[0].EntityLocation = 0;
#endif
   
   ent_info = avm_add_ent_info(cb, &ep);
   avm_add_ent_str_info(cb, ent_info, &ep_str); 
   
   ent_info->entity_type = SAHPI_ENT_ROOT;
   ent_info->parent      = AVM_ENT_INFO_NULL;

   return NCSCC_RC_SUCCESS;
}

extern uns32 
avm_remove_dependents(
                       AVM_ENT_INFO_T *dependee
                     )   
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T *temp;
   AVM_ENT_INFO_LIST_T *del_ent;

   m_AVM_LOG_FUNC_ENTRY("avm_remove_dependents");
   
   if(AVM_ENT_INFO_NULL == dependee) 
   {
      return rc;
   }
   
   for(temp = dependee->dependents; temp != AVM_ENT_INFO_LIST_NULL; )
   {
        del_ent = temp;
        temp    = temp->next;
        m_MMGR_FREE_AVM_ENT_INFO_LIST(del_ent);
   }   

   dependee->dependents = AVM_ENT_INFO_LIST_NULL;
  
  return NCSCC_RC_SUCCESS; 
}

static uns32
avm_rmv_frm_dependee(
                     AVM_CB_T       *avm_cb,
                     AVM_ENT_INFO_T *ent_info
                    )
{
   uns32                     rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_T           *dependee;
   AVM_ENT_INFO_LIST_T      *del_rec;
   AVM_ENT_INFO_LIST_T      *temp;
   
   m_AVM_LOG_FUNC_ENTRY("avm_rmv_frm_dependee");
   if(TRUE == ent_info->depends_on_valid)
   {
      dependee = avm_find_ent_str_info(avm_cb, &ent_info->dep_ep_str,TRUE);

      if((AVM_ENT_INFO_NULL != dependee) && (AVM_ENT_INFO_LIST_NULL != dependee->dependents)) 
      {
         if(ent_info == dependee->dependents->ent_info)
         {
            del_rec = dependee->dependents;
            dependee->dependents =  dependee->dependents->next;
            m_MMGR_FREE_AVM_ENT_INFO_LIST(del_rec);
        }else
        {
           for(temp = dependee->dependents; 
               (temp->next != AVM_ENT_INFO_LIST_NULL) && (ent_info != temp->next->ent_info) ; 
               temp = temp->next);
           if(AVM_ENT_INFO_LIST_NULL != temp->next)
           {
              del_rec = temp->next;
              temp->next = temp->next->next;
              m_MMGR_FREE_AVM_ENT_INFO_LIST(del_rec);
           }                        
         }
      }
   }   
   return rc;
}   


static uns32
avm_rmv_frm_parent(
                   AVM_CB_T       *avm_cb,
                   AVM_ENT_INFO_T *ent_info
                  )
{
   uns32                     rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T      *del_rec;
   AVM_ENT_INFO_LIST_T      *temp;
   
   m_AVM_LOG_FUNC_ENTRY("avm_rmv_frm_parent");
   if(AVM_ENT_INFO_NULL != ent_info->parent)
   {
      if(ent_info == ent_info->parent->child->ent_info)
      {
          del_rec = ent_info->parent->child;
          ent_info->parent->child =  ent_info->parent->child->next;
          m_MMGR_FREE_AVM_ENT_INFO_LIST(del_rec);
      }else
      {
         for(temp = ent_info->parent->child; (temp->next != AVM_ENT_INFO_LIST_NULL) && (ent_info != temp->next->ent_info) ; temp = temp->next);

         if(AVM_ENT_INFO_LIST_NULL != temp->next)
         {
            del_rec = temp->next;
            temp->next = temp->next->next;
            m_MMGR_FREE_AVM_ENT_INFO_LIST(del_rec);
         }                        
      }
   }

   return rc;
}   

               
static uns32
avm_rmv_node_info(
                  AVM_CB_T       *avm_cb,
                  AVM_ENT_INFO_T *ent_info
                )
{
   uns32                     rc = NCSCC_RC_SUCCESS;
   AVM_NODE_INFO_T          *node_info;
   
   m_AVM_LOG_FUNC_ENTRY("avm_rmv_node_info");
   node_info = avm_find_node_name_info(avm_cb, ent_info->node_name);
   if(AVM_NODE_INFO_NULL != node_info)
   {
      if(ent_info == node_info->ent_info)
      { 
         avm_delete_node_name_info(avm_cb, node_info);
         m_MMGR_FREE_AVM_NODE_INFO(node_info);
      } 
   }  

   return rc;
}   

extern uns32 
avm_deact_dependents(
                       AVM_CB_T       *cb,
                       AVM_ENT_INFO_T *dependee
                    )   
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T *temp;
   
   m_AVM_LOG_FUNC_ENTRY("avm_deact_dependents");
   if(AVM_ENT_INFO_NULL == dependee) 
   {
      return rc;
   }
   
   for(temp = dependee->dependents; temp != AVM_ENT_INFO_LIST_NULL; temp = temp->next)
   {
      m_AVM_LOG_GEN_EP_STR("Entity Deactivated", temp->ent_info->ep_str.name, NCSFL_SEV_NOTICE);
      avm_hisv_api_cmd(temp->ent_info, HS_RESOURCE_INACTIVE_SET, 0);
   }   

  return NCSCC_RC_SUCCESS; 
}

extern uns32
avm_rmv_ent_info(
                  AVM_CB_T       *avm_cb,
                  AVM_ENT_INFO_T *ent_info
                )
{
   uns32                     rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_rmv_ent_info");
   avm_rmv_frm_dependee(avm_cb, ent_info);
   avm_rmv_frm_parent(avm_cb, ent_info);
   avm_rmv_node_info(avm_cb, ent_info);
   avm_remove_dependents(ent_info);

   return rc;
}
extern SaHpiHsStateT
avm_map_hs_to_hpi_hs(AVM_FSM_STATES_T  hs_state)
{
  SaHpiHsStateT hpi_hs_state;

  m_AVM_LOG_FUNC_ENTRY("avm_map_hs_to_hpi_hs");
  switch(hs_state)
  {
     case  AVM_ENT_INACTIVE:
     {
       hpi_hs_state = SAHPI_HS_STATE_INACTIVE;
     }
     break;

     case  AVM_ENT_ACTIVE:
     {
#ifdef HPI_A
       hpi_hs_state = SAHPI_HS_STATE_ACTIVE_HEALTHY;
#else
       hpi_hs_state = SAHPI_HS_STATE_ACTIVE;
#endif
     }
     break;


     case  AVM_ENT_RESET_REQ:
     {
#ifdef HPI_A
       hpi_hs_state = SAHPI_HS_STATE_ACTIVE_HEALTHY;
#else
       hpi_hs_state = SAHPI_HS_STATE_ACTIVE;
#endif
     }
     break;

     case  AVM_ENT_EXTRACT_REQ:
     {
       hpi_hs_state = SAHPI_HS_STATE_EXTRACTION_PENDING;
     }
     break;

     case  AVM_ENT_NOT_PRESENT:
     default:
     {
       hpi_hs_state = SAHPI_HS_STATE_NOT_PRESENT;
     }
     break; 
  }

  return hpi_hs_state;
}



/***********************************************************************
 ******
 * Name          : ncs_avm_build_mib_idx 
 *
 * Description   : This function creates index for a mibset.
 *
 * Arguments     : NCSMIB_IDX *mib_idx        -   Pointer to mib index
 *                 AVM_ENT_PATH_STR_T *idx_val -  Pointer to Index value
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/

static uns32
ncs_avm_build_mib_idx(NCSMIB_IDX *mib_idx, AVM_ENT_PATH_STR_T *idx_val)
{
   uns32 *ptr = NULL;
   uns32 inst_len=0;
   uns32 count = 0;

   m_AVM_LOG_FUNC_ENTRY("ncs_avm_build_mib_idx");
   inst_len = m_NCS_OS_NTOHS(idx_val->length);

   mib_idx->i_inst_ids = (uns32 *)m_MMGR_ALLOC_AVM_DEFAULT_VAL((inst_len + 1) * sizeof(uns32));

   if(mib_idx->i_inst_ids == NULL)
   {
      m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return NCSCC_RC_FAILURE;
   }
   ptr = (uns32 *)mib_idx->i_inst_ids;

   mib_idx->i_inst_len = inst_len+1;
   *ptr++ = inst_len;

   while(count < inst_len)
   {
      *ptr++ = (uns32)idx_val->name[count++];
   }

   return NCSCC_RC_SUCCESS;
}


/***********************************************************************
 ******
 * Name          : avm_push_admin_mib_set_to_psr 
 *
 * Description   : This function push admin state as unlock for an entity into pss.
 *
 * Arguments     : AVM_CB_T *cb              -  Pointer to AvM CB
 *                 AVM_ENT_INFO_T  *ent_info -  Pointer to entity info
 *                 AVM_ADM_OP_T adm_req      -  Admin value to be push
 *
 * Return Values : NCSCC_RC_SUCCESS.
 *
 * Notes         : None.
 ***********************************************************************
 ******/

uns32
avm_push_admin_mib_set_to_psr(AVM_CB_T *cb, AVM_ENT_INFO_T  *ent_info, AVM_ADM_OP_T adm_req)
{
   NCSMIB_ARG      mib_arg;
   uns32           status = NCSCC_RC_SUCCESS;
   NCSOAC_SS_ARG   mab_arg;
   NCSMIB_IDX      mib_idx;

   m_AVM_LOG_FUNC_ENTRY("avm_push_admin_mib_set_to_psr");
   if(ent_info == NULL)
      return status;

   if(ncs_avm_build_mib_idx(&mib_idx, &(ent_info->ep_str)) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(&mib_arg, 0, sizeof(NCSMIB_ARG));

   ncsmib_init(&mib_arg);

   mib_arg.i_idx.i_inst_ids  = (uns32*)(mib_idx.i_inst_ids);
   mib_arg.i_idx.i_inst_len  =  mib_idx.i_inst_len;
   mib_arg.i_tbl_id          = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
   mib_arg.rsp.info.set_rsp.i_param_val.i_param_id = ncsAvmEntAdmReq_ID;
   mib_arg.rsp.info.set_rsp.i_param_val.i_length = 4;
   mib_arg.rsp.info.set_rsp.i_param_val.info.i_int = adm_req;
   mib_arg.rsp.info.set_rsp.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   mib_arg.rsp.i_status = NCSCC_RC_SUCCESS;


   memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));

   mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
   mab_arg.i_oac_hdl = cb->mab_hdl;
   mab_arg.i_tbl_id  = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;

   mib_arg.i_op   = NCSMIB_OP_RSP_SET;

   mab_arg.info.push_mibarg_data.arg = &mib_arg;

   if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
   {
       status = NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVM_DEFAULT_VAL(mib_idx.i_inst_ids);

   return status;
}

/***********************************************************************
 ******
 * Name          :  avm_standby_boot_succ_tmr_handler
 *
 * Description   :  This is to Fix for boot_succ_tmr for the corresponding entity
                    Needs to be started/stopped at Active & Switchover to fix the 
                    case for Switchover/Failover - Only the Active will act on the 
                    Expiry of this timer. 

 *
 * Arguments     : AVM_CB_T *avm_cb              -  Pointer to AvM CB
                   AVM_EVT_T *evt
 *
 * Return Values : NCSCC_RC_SUCCESS.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern 
uns32 avm_standby_boot_succ_tmr_handler(AVM_CB_T *avm_cb,AVM_EVT_T *hpi_evt,AVM_ENT_INFO_T *ent_info,AVM_FSM_EVT_TYPE_T   fsm_evt_type)
{

   /* start the boot_succ_tmr if it is a AVM_EVT_INSERTION_PENDING event  */
   if(((ent_info->current_state == AVM_ENT_NOT_PRESENT) || (ent_info->current_state == AVM_ENT_INACTIVE))
      && (fsm_evt_type == AVM_EVT_INSERTION_PENDING))
   {
      m_AVM_SSU_BOOT_TMR_START(avm_cb, ent_info);
   }
   /* stop the boot_succ_tmr if it is a HPI_FWPROG_BOOT_SUCCESS event */
   if(ent_info->current_state == AVM_ENT_ACTIVE && fsm_evt_type == AVM_EVT_SENSOR_FW_PROGRESS)
   {
        uns32 evt_state = 0, evt_type = 0;
        SaHpiEventT *hpi_event;

        hpi_event = &((AVM_EVT_T*)hpi_evt)->evt.hpi_evt->hpi_event;

        evt_state = (uns8)hpi_event->EventDataUnion.SensorEvent.EventState;
        evt_state = (evt_state >> 1);

        /* Get the Firmware progress event type */
        if (hpi_event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_SENSOR_SPECIFIC)
        {
            evt_type = hpi_event->EventDataUnion.SensorEvent.SensorSpecific;
        }
        else
        if (hpi_event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_OEM)
        {
            evt_type = (hpi_event->EventDataUnion.SensorEvent.Oem) >> 8;
        }
        else
        {
           /* Log it ?? */
           return NCSCC_RC_FAILURE;
        }

        if(evt_type == HPI_FWPROG_BOOT_SUCCESS)
        {
           if(ent_info->boot_succ_tmr.status == AVM_TMR_RUNNING)
           {
              avm_stop_tmr(avm_cb, &ent_info->boot_succ_tmr);
           }
        }

     }
     return NCSCC_RC_SUCCESS;

}

/***********************************************************************
 ******
 * Name          : avm_standby_map_hpi2fsm
 *
 * Description   : This function maps hpi events to AvM fsm events
 *
 * Arguments     : AVM_AVD_MSG_T* -  pointer to destination.
 *                 AVM_AVD_MSG_T* -  pointer to source.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern uns32
avm_standby_map_hpi2fsm(
                  AVM_CB_T             *cb,
                  HPI_EVT_T            *evt,
                  AVM_FSM_EVT_TYPE_T   *fsm_evt_type,
                  AVM_ENT_INFO_T       *ent_info
               )
{
   SaHpiEventT  hpi_event;

   hpi_event = evt->hpi_event;

   switch(hpi_event.EventType)
   {
      case SAHPI_ET_RESOURCE:
      {
         if(SAHPI_RESE_RESOURCE_FAILURE == hpi_event.EventDataUnion.ResourceEvent.ResourceEventType)
         {
            *fsm_evt_type = AVM_EVT_RESOURCE_FAIL;
         }else if(SAHPI_RESE_RESOURCE_RESTORED == hpi_event.EventDataUnion.ResourceEvent.ResourceEventType)
         {
            *fsm_evt_type = AVM_EVT_RESOURCE_REST;
         }else
         {
            *fsm_evt_type = AVM_EVT_IGNORE;
         }
      }
      break;

      case SAHPI_ET_HOTSWAP:
      {
         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_INSERTION_PENDING)
         {
            *fsm_evt_type = AVM_EVT_INSERTION_PENDING;
             break;
         }

#ifdef HPI_A
         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_ACTIVE_HEALTHY)
#else
         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_ACTIVE)
#endif
         {
            *fsm_evt_type = AVM_EVT_ENT_ACTIVE;
             break;
         }

         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_EXTRACTION_PENDING)
         {
             *fsm_evt_type = AVM_EVT_EXTRACTION_PENDING;
              break;
         }

         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_INACTIVE)
         {
            *fsm_evt_type = AVM_EVT_ENT_INACTIVE;
             break;
         }

         if(hpi_event.EventDataUnion.HotSwapEvent.HotSwapState == SAHPI_HS_STATE_NOT_PRESENT)
         {
            if((hpi_event.EventDataUnion.HotSwapEvent.PreviousHotSwapState == SAHPI_HS_STATE_INACTIVE) || (ent_info->current_state == AVM_ENT_INACTIVE))
            {
               *fsm_evt_type = AVM_EVT_ENT_EXTRACTED;
                break;
            }
            if(hpi_event.EventDataUnion.HotSwapEvent.PreviousHotSwapState != SAHPI_HS_STATE_INACTIVE)
            {
               *fsm_evt_type = AVM_EVT_SURP_EXTRACTION;
                break;
            }
         }
      }
      break;

      case SAHPI_ET_SENSOR:
      {
         /* 
          * Check for the Firmware progress event and then send the event to the 
          * FSM to handle it.
          */
         if (hpi_event.EventDataUnion.SensorEvent.SensorType == SAHPI_SYSTEM_FW_PROGRESS)
         {
            *fsm_evt_type = AVM_EVT_SENSOR_FW_PROGRESS;
         }else
         {
            *fsm_evt_type = AVM_EVT_IGNORE;
         }

         /* Break here since other sensor events are not yet supported. Can be 
          * removed later if required.
          */
         break;
       }
      default:
      {
         *fsm_evt_type = AVM_EVT_INVALID;
      }
   }
   return NCSCC_RC_SUCCESS;
}


/***********************************************************************
 ******
 * Name          : avm_is_this_entity_self
 *
 * Description   : This function checks whether the entity path passed
 *                 to it is same as entity path of self i.e. active
 *                 system controller.
 *
 * Arguments     : AVM_CB_T*, -  Pointer to AVM's CB.
 *                 SaHpiEntityPathT -  Entity path which needs to be 
 *                                     checked.
 *
 * Return Values : TRUE / FALSE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern NCS_BOOL
avm_is_this_entity_self(AVM_CB_T *avm_cb, SaHpiEntityPathT ep)
{
   MDS_DEST tmp_mds_to_node_id;
   NCS_CHASSIS_ID chassis_id;
   NCS_PHY_SLOT_ID phy_slot_id;
   NCS_SUB_SLOT_ID subslot_id;
   SaHpiEntityPathT self_ep;
   NCS_BOOL rc = FALSE;

   m_AVM_LOG_FUNC_ENTRY("avm_is_this_entity_self");

   /* Get node ID of self and convert it to entity path*/
   tmp_mds_to_node_id = avm_cb->adest;
   tmp_mds_to_node_id = m_NCS_NODE_ID_FROM_MDS_DEST(tmp_mds_to_node_id);
   m_NCS_GET_PHYINFO_FROM_NODE_ID(tmp_mds_to_node_id, &chassis_id, &phy_slot_id, &subslot_id);
   avm_conv_phy_info_to_ent_path(chassis_id, phy_slot_id, subslot_id, &self_ep);

   /* Compare entity path of self and given entity */
   rc = avm_compare_ent_paths(self_ep, ep);   
   
   return rc;
}

 
/***********************************************************************
 ******
 * Name          : avm_conv_phy_info_to_ent_path
 *
 * Description   : This function converts physical info, chassis id, 
 *                 slot id and subslot id, to entity path 
 *
 * Arguments     : NCS_CHASSIS_ID    - Chassis ID
 *                 NCS_PHY_SLOT_ID   - Physical Slot ID
 *                 NCS_SUB_SLOT_ID   - Sub-slot ID
 *                 SaHpiEntityPathT* - Entity path reference which will
 *                                    have the resulting entity path.
 *
 * Return Values : None.
 *
 * Notes         : This function treats both 0x00 and 0x0F as invalid 
 *                 subslot values.
 ***********************************************************************
 ******/
extern void 
avm_conv_phy_info_to_ent_path(NCS_CHASSIS_ID chassis_id, NCS_PHY_SLOT_ID phy_slot_id, 
                                    NCS_SUB_SLOT_ID subslot_id, SaHpiEntityPathT *ep)
{
   char *arch_type = NULL;

   arch_type = m_NCS_OS_PROCESS_GET_ENV_VAR("OPENSAF_TARGET_SYSTEM_ARCH");

   /* Initialize entity path */
   memset(ep->Entry, 0, sizeof(SaHpiEntityPathT));

   /* Depending on HPI version construct entity path */

#ifdef HPI_A

   ep->Entry[2].EntityType  = SAHPI_ENT_ROOT; 
   ep->Entry[2].EntityInstance = 0;
   ep->Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
   ep->Entry[1].EntityInstance = chassis_id; 
   ep->Entry[0].EntityType = SAHPI_ENT_SYSTEM_BOARD; 
   ep->Entry[0].EntityInstance = phy_slot_id;

#elif  HPI_B_02

   /* If Sub-slot ID is 0x00 or 0x0F then entity path doesn't
    * have the sub-slot */
   if ((subslot_id != 0) && (subslot_id !=15))
   {
      ep->Entry[3].EntityType  = SAHPI_ENT_ROOT; 
      ep->Entry[3].EntityLocation = 0;
      if (m_NCS_OS_STRCMP(arch_type, "ATCA") == 0)
      {
         ep->Entry[2].EntityType = SAHPI_ENT_ADVANCEDTCA_CHASSIS;
         ep->Entry[2].EntityLocation = chassis_id; 
         ep->Entry[1].EntityType = SAHPI_ENT_PHYSICAL_SLOT; 
         ep->Entry[1].EntityLocation = phy_slot_id;                          
      }
      else
      {
         ep->Entry[2].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
         ep->Entry[2].EntityLocation = chassis_id; 
         ep->Entry[1].EntityType = SAHPI_ENT_SYSTEM_BLADE; 
         ep->Entry[1].EntityLocation = phy_slot_id;                          
      }
      ep->Entry[0].EntityType = AMC_SUB_SLOT_TYPE; 
      ep->Entry[0].EntityLocation = subslot_id;                          
   }
   else 
   {
      ep->Entry[2].EntityType  = SAHPI_ENT_ROOT; 
      ep->Entry[2].EntityLocation = 0;
      if (m_NCS_OS_STRCMP(arch_type, "ATCA") == 0)
      {
         ep->Entry[1].EntityType = SAHPI_ENT_ADVANCEDTCA_CHASSIS;
         ep->Entry[1].EntityLocation = chassis_id; 
         ep->Entry[0].EntityType = SAHPI_ENT_PHYSICAL_SLOT; 
         ep->Entry[0].EntityLocation = phy_slot_id;                          
      }
      else
      {
         ep->Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
         ep->Entry[1].EntityLocation = chassis_id; 
         ep->Entry[0].EntityType = SAHPI_ENT_SYSTEM_BLADE; 
         ep->Entry[0].EntityLocation = phy_slot_id;                          
      }
   }

#endif
}   


/***********************************************************************
 ******
 * Name          : avm_compare_ent_paths
 *
 * Description   : This function compares two entity paths.
 *
 * Arguments     : SaHpiEntityPathT - Entity Path 1
 *                 SaHpiEntityPathT - Entity path 2
 *
 * Return Values : TRUE/FALSE.
 *
 * Notes         : 
 ***********************************************************************
 ******/
extern NCS_BOOL
avm_compare_ent_paths(SaHpiEntityPathT ent_path1, SaHpiEntityPathT ent_path2)
{
   uns8 ent_path1_lev, ent_path2_lev;
   NCS_CHASSIS_ID chassis_id1, chassis_id2;
   NCS_PHY_SLOT_ID phy_slot1, phy_slot2;
   NCS_SUB_SLOT_ID subslot1, subslot2;
   uns8 i=0;
   char *arch_type = NULL;
 
   arch_type = m_NCS_OS_PROCESS_GET_ENV_VAR("OPENSAF_TARGET_SYSTEM_ARCH");

   /* Extract values from two entity paths. */ 
   while(ent_path1.Entry[i].EntityType  != SAHPI_ENT_ROOT)
   {
   ent_path1_lev++;

#ifdef HPI_A
   if(ent_path1.Entry[i].EntityType == SAHPI_ENT_SYSTEM_BOARD)
      phy_slot1 = ent_path1.Entry[i].EntityInstance;
   if(ent_path1.Entry[i].EntityType == SAHPI_ENT_SYSTEM_CHASSIS)
      chassis_id1 = ent_path1.Entry[i].EntityInstance;
#elif  HPI_B_02
   if(ent_path1.Entry[i].EntityType == AMC_SUB_SLOT_TYPE)
      subslot1 = ent_path1.Entry[i].EntityLocation;
   if (m_NCS_OS_STRCMP(arch_type, "ATCA") == 0)
   {
      if(ent_path1.Entry[i].EntityType == SAHPI_ENT_PHYSICAL_SLOT)
         phy_slot1 = ent_path1.Entry[i].EntityLocation;
      if(ent_path1.Entry[i].EntityType == SAHPI_ENT_ADVANCEDTCA_CHASSIS)
         chassis_id1 = ent_path1.Entry[i].EntityLocation;
   }
   else
   {
      if(ent_path1.Entry[i].EntityType == SAHPI_ENT_SYSTEM_BLADE)
         phy_slot1 = ent_path1.Entry[i].EntityLocation;
      if(ent_path1.Entry[i].EntityType == SAHPI_ENT_SYSTEM_CHASSIS)
         chassis_id1 = ent_path1.Entry[i].EntityLocation;
   }
#endif

   i++;
   }

   i=0;
   while(ent_path2.Entry[i].EntityType  != SAHPI_ENT_ROOT)
   {
   ent_path2_lev++;

#ifdef HPI_A
   if(ent_path2.Entry[i].EntityType == SAHPI_ENT_SYSTEM_BOARD)
      phy_slot2 = ent_path2.Entry[i].EntityInstance;
   if(ent_path2.Entry[i].EntityType == SAHPI_ENT_SYSTEM_CHASSIS)
      chassis_id2 = ent_path2.Entry[i].EntityInstance;
#elif  HPI_B_02
   if(ent_path2.Entry[i].EntityType == AMC_SUB_SLOT_TYPE)
      subslot2 = ent_path2.Entry[i].EntityLocation;
   if (m_NCS_OS_STRCMP(arch_type, "ATCA") == 0)
   {
      if(ent_path2.Entry[i].EntityType == SAHPI_ENT_PHYSICAL_SLOT)
         phy_slot2 = ent_path2.Entry[i].EntityLocation;
      if(ent_path2.Entry[i].EntityType == SAHPI_ENT_ADVANCEDTCA_CHASSIS)
         chassis_id2 = ent_path2.Entry[i].EntityLocation;
   }
   else
   {
      if(ent_path2.Entry[i].EntityType == SAHPI_ENT_SYSTEM_BLADE)
         phy_slot2 = ent_path2.Entry[i].EntityLocation;
      if(ent_path2.Entry[i].EntityType == SAHPI_ENT_SYSTEM_CHASSIS)
         chassis_id2 = ent_path2.Entry[i].EntityLocation;
   }
#endif

   i++;
   }

   if(ent_path1_lev != ent_path2_lev)
      return FALSE;

   if((ent_path1_lev == 2) && (chassis_id1 == chassis_id2) && (phy_slot1 == phy_slot2))
      return TRUE;

   if((ent_path1_lev == 3) && (chassis_id1 == chassis_id2) && (phy_slot1 == phy_slot2) && (subslot1 == subslot2))
      return TRUE;

   return FALSE;
}

 
