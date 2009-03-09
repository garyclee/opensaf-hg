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

  DESCRIPTION: This module contain all the supporting functions for the
               checkpointing operation. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avm_enqueue_hpi_evt
  avm_dequeue_hpi_evt
  avm_ckpt_update_db
******************************************************************************
*/

#include "avm.h"

static uns32
avm_update_dependents(
            AVM_CB_T       *cb,   
                      AVM_ENT_INFO_T *dest,
            AVM_ENT_INFO_T *src
           );

static uns32
avm_enqueue_hpi_evt(
                     AVM_CB_T  *avm_cb,
                     AVM_EVT_T *hpi_evt,
                     uns32      event_id,
                     NCS_BOOL   bool
                   );
static uns32  
avm_dequeue_hpi_evt(
                    AVM_CB_T              *cb, 
                    AVM_EVT_Q_NODE_T      *evt_id
                  ); 

static uns32
avm_copy_inv_data(AVM_VALID_INFO_T *dest,
                  AVM_VALID_INFO_T *src
                 )
{
    uns32 rc = NCSCC_RC_SUCCESS;

    dest->inv_data.product_name.DataLength = src->inv_data.product_name.DataLength;
    m_NCS_MEMCPY(dest->inv_data.product_name.Data, src->inv_data.product_name.Data, src->inv_data.product_name.DataLength);
            
            
    dest->inv_data.product_version.DataLength = src->inv_data.product_version.DataLength;
    m_NCS_MEMCPY(dest->inv_data.product_version.Data, src->inv_data.product_version.Data, dest->inv_data.product_version.DataLength);
         
    return rc;   
}   

static uns32
avm_update_dependents(
                      AVM_CB_T       *cb,   
                      AVM_ENT_INFO_T *dest,
                      AVM_ENT_INFO_T *src
                    )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 status = NCSCC_RC_SUCCESS;

   AVM_ENT_INFO_T      *ent_info;
   AVM_ENT_INFO_LIST_T *temp_ent_info_list;
   AVM_ENT_INFO_LIST_T *t_ent_info_list;
   AVM_ENT_INFO_LIST_T *temp;
   

   dest->dependents = src->dependents;

   for(temp_ent_info_list = src->dependents; temp_ent_info_list != AVM_ENT_INFO_LIST_NULL; temp_ent_info_list = temp_ent_info_list->next)
   {

      ent_info = avm_find_ent_str_info(cb, &temp_ent_info_list->ent_info->ep_str ,FALSE);

      if(AVM_ENT_INFO_NULL == ent_info)   
      {
         if(cb->cold_sync)
         { 
            for(t_ent_info_list = src->dependents; t_ent_info_list != AVM_ENT_INFO_LIST_NULL;)
            {
               m_AVM_ENT_IS_VALID(t_ent_info_list->ent_info, status);
               if(!status)
               {
                  m_MMGR_FREE_AVM_ENT_INFO(t_ent_info_list->ent_info);
               }
               temp = t_ent_info_list;
               t_ent_info_list =  t_ent_info_list->next; 
               m_MMGR_FREE_AVM_ENT_INFO_LIST(temp); 
            }
            dest->dependents = AVM_ENT_INFO_LIST_NULL;
            m_AVM_LOG_INVALID_VAL_FATAL(0);
            rc = NCSCC_RC_FAILURE;
            break;
         } 
      }else
      {
         m_MMGR_FREE_AVM_ENT_INFO(temp_ent_info_list->ent_info);
         temp_ent_info_list->ent_info = ent_info;   
         temp_ent_info_list->ep_str   = &ent_info->ep_str;   
      }
   }
   
   return rc;
}      
         
/****************************************************************************\
 * Function: avm_ckpt_update_valid_db
 *
 * Purpose:  Add new AVM_VALID_INFO_T  entry if action is ADD, remove it  from 
 *           the tree if action is to remove and update data if request is 
 *           to update.
 *
 * Input: cb       - CB pointer.
 *        ent_info - Decoded struct.
 *        action   - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
extern uns32
avm_ckpt_update_valid_info_db( AVM_CB_T             *cb,
                               AVM_VALID_INFO_T     *valid_info,
                               NCS_MBCSV_ACT_TYPE    action)
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVM_VALID_INFO_T  *valid_info_lp;
   uns32 i = 0;
   uns32 j = 0;
   
   valid_info_lp = avm_find_valid_info(cb, &(valid_info->desc_name));
   
   switch(action)
   {
      case NCS_MBCSV_ACT_ADD:
      {
         if(valid_info_lp == AVM_VALID_INFO_NULL)
         {
            valid_info_lp = avm_add_valid_info(cb, &(valid_info->desc_name));
            if(valid_info_lp == AVM_VALID_INFO_NULL)
            {
               return NCSCC_RC_FAILURE;
            }
         }

         valid_info_lp->is_node = valid_info->is_node;

         avm_copy_inv_data(valid_info_lp, valid_info);  

         valid_info_lp->is_node     = valid_info->is_node;
         valid_info_lp->is_fru      = valid_info->is_fru;
         valid_info_lp->entity_type = valid_info->entity_type;
    
         valid_info_lp->parent_cnt = valid_info->parent_cnt;
   
         for(i = 0; i < valid_info->parent_cnt; i++)
         {
            valid_info_lp->location[i].parent.length = valid_info->location[i].parent.length; 
            if(valid_info_lp->location[i].parent.length > NCS_MAX_INDEX_LEN)
            { 
               valid_info_lp->location[i].parent.length = NCS_MAX_INDEX_LEN; 
            }
         
            m_NCS_MEMCPY(valid_info_lp->location[i].parent.name, valid_info->location[i].parent.name, valid_info_lp->location[i].parent.length);

            for(j = 0; j < MAX_POSSIBLE_LOC_RANGES; j++)
            {
               valid_info_lp->location[i].range[j].min = valid_info->location[i].range[j].min; 
               valid_info_lp->location[i].range[j].max = valid_info->location[i].range[j].max; 
            }
         }   
      }
      break;
      
      case NCS_MBCSV_ACT_UPDATE:
      case NCS_MBCSV_ACT_RMV:
      default:
      {
         m_AVM_LOG_INVALID_VAL_FATAL(action);
         status = NCSCC_RC_FAILURE;
      }
      break;
   }   
   
   return status;
      
}


/****************************************************************************\
 * Function: avm_ckpt_update_ent_db
 *
 * Purpose:  Add new AVM_ENT_INFO_T  entry if action is ADD, remove it  from 
 *           the tree if action is to remove and update data if request is 
 *           to update.
 *
 * Input: cb       - CB pointer.
 *        ent_info - Decoded struct.
 *        action   - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
uns32 avm_ckpt_update_ent_db( 
                              AVM_CB_T              *cb,
                              AVM_ENT_INFO_T        *ent_info,
                              NCS_MBCSV_ACT_TYPE     action,
                              AVM_ENT_INFO_T       **o_ent_info
                            )
{
   uns32 status = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_T     *ent_info_lp;
   AVM_NODE_INFO_T    *node_info;
   AVM_ENT_PATH_STR_T ep;
   uns32 i;   
   
   ent_info_lp = avm_find_ent_info(cb, &ent_info->entity_path);
   
   switch(action)
   {
      case NCS_MBCSV_ACT_ADD:
      {
         if(ent_info_lp == AVM_ENT_INFO_NULL)
         {
            ent_info_lp = avm_add_ent_info(cb, &ent_info->entity_path);
            if(ent_info_lp == AVM_ENT_INFO_NULL)
            {
               return NCSCC_RC_FAILURE;
            }
            
            ep.length = m_NCS_OS_NTOHS(ent_info->ep_str.length);
            m_NCS_MEMCPY(ep.name, ent_info->ep_str.name, AVM_MAX_INDEX_LEN);
            avm_add_ent_str_info(cb, ent_info_lp, &ep);
         }
         
         (*o_ent_info) = ent_info_lp;

         ent_info_lp->is_node          = ent_info->is_node;
         ent_info_lp->is_fru           = ent_info->is_fru;
         ent_info_lp->act_policy       = ent_info->act_policy;
         ent_info_lp->node_name.length = ent_info->node_name.length;

         ent_info_lp->adm_lock      = ent_info->adm_lock;
         ent_info_lp->adm_shutdown  = ent_info->adm_shutdown;
         ent_info_lp->adm_req       = ent_info->adm_req;

         m_NCS_MEMCPY(ent_info_lp->node_name.value, ent_info->node_name.value, SA_MAX_NAME_LENGTH);
    
         if(ent_info_lp->is_node)
         {    
            node_info = avm_add_node_name_info(cb, ent_info_lp->node_name);
          
            if(AVM_NODE_INFO_NULL == node_info) 
            {
               m_AVM_LOG_INVALID_VAL_FATAL(0);   
            }else
            {
               node_info->ent_info = ent_info_lp;
            }
         }  
        
         ent_info_lp->entity_type      = ent_info->entity_type;
         ent_info_lp->depends_on_valid = ent_info->depends_on_valid;
         
         ent_info_lp->dep_ep_str.length = ent_info->dep_ep_str.length;
         m_NCS_MEMCPY(ent_info_lp->dep_ep_str.name, ent_info->dep_ep_str.name, AVM_MAX_INDEX_LEN);
 
         ent_info_lp->desc_name.length = ent_info->desc_name.length;
         m_NCS_MEMCPY(ent_info_lp->desc_name.name, ent_info->desc_name.name, NCS_MAX_INDEX_LEN);
    
         ent_info_lp->parent_desc_name.length = ent_info->parent_desc_name.length;
         m_NCS_MEMCPY(ent_info_lp->parent_desc_name.name, ent_info->parent_desc_name.name, NCS_MAX_INDEX_LEN);
 
         ent_info_lp->valid_info = (AVM_VALID_INFO_T*)ncs_patricia_tree_get(&cb->db.valid_info_anchor, ent_info->desc_name.name);
         ent_info_lp->parent_valid_info = (AVM_VALID_INFO_T*)ncs_patricia_tree_get(&cb->db.valid_info_anchor, ent_info->parent_desc_name.name);
 
         ent_info_lp->current_state  = ent_info->current_state;
         ent_info_lp->previous_state = ent_info->previous_state;
         ent_info_lp->previous_diff_state = ent_info->previous_diff_state;
 
         for(i = 0; i < AVM_MAX_SENSOR_COUNT; i++)
         {
            ent_info_lp->sensors[i].SensorType = ent_info->sensors[i].SensorType;
            ent_info_lp->sensors[i].SensorNum  = ent_info->sensors[i].SensorNum;
            ent_info_lp->sensors[i].sensor_assert = ent_info->sensors[i].sensor_assert;
         }
 
         ent_info_lp->row_status          = ent_info->row_status;  
         
         if(ent_info->dependents)  
         { 
            avm_update_dependents(cb, ent_info_lp, ent_info);   
         }

         /* Update DHCP configuration */
         avm_decode_ckpt_dhcp_conf_update(&ent_info_lp->dhcp_serv_conf,
            &ent_info->dhcp_serv_conf);

         avm_decode_ckpt_dhcp_state_update(&ent_info_lp->dhcp_serv_conf,
            &ent_info->dhcp_serv_conf);

         avm_decode_ckpt_upgd_state_update(&ent_info_lp->dhcp_serv_conf,
            &ent_info->dhcp_serv_conf);

         avm_populate_ent_info(cb, ent_info_lp);
      } 
      break;
      
      case NCS_MBCSV_ACT_UPDATE:
      {
          m_AVM_LOG_INVALID_VAL_FATAL(action);
          return NCSCC_RC_FAILURE;
      }
      break;

      case NCS_MBCSV_ACT_RMV:
      {
         if(AVM_ENT_INFO_NULL != ent_info_lp)
         {  
            avm_rmv_ent_info(cb, ent_info_lp);
            avm_delete_ent_str_info(cb, ent_info_lp);
            avm_delete_ent_info(cb, ent_info_lp);
            m_MMGR_FREE_AVM_ENT_INFO(ent_info_lp);
         }
      }
      break;

      default:
      {
         m_AVM_LOG_INVALID_VAL_FATAL(action);
         status = NCSCC_RC_FAILURE;
      }
      break;
   }   

   return status;
}

/****************************************************************************\
 * Function: avm_enqueue_hpi_evt
 *
 * Purpose:  Thie function is used to enqueue events at standby.  
 * Input: cb       - CB pointer.
 *        hpi_evt  - HPI event received.
 *        event_id - event id  
 *        bool     - bool to know wheher the event is from Active or
 *                   standby 
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
extern uns32
avm_enqueue_hpi_evt(
                     AVM_CB_T  *avm_cb,
                     AVM_EVT_T *hpi_evt, 
                     uns32      event_id,
                     NCS_BOOL   bool
                   )
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if(AVM_EVT_Q_NODE_NULL == avm_cb->evt_q.front)
   {
      if(AVM_EVT_Q_NODE_NULL == (avm_cb->evt_q.evt_q = m_MMGR_ALLOC_AVM_EVT_Q_NODE))
      {
         m_AVM_LOG_INVALID_VAL_FATAL(0);
         return NCSCC_RC_FAILURE;
      }else
      {
         m_NCS_DBG_PRINTF("\n Event Id of event enqueued in beginning is %u\n", event_id);
         m_AVM_LOG_CKPT_EVT_Q(AVM_LOG_EVT_Q_ENQ, event_id, bool, NCSFL_SEV_INFO);
         m_NCS_MEMSET((uns8*)avm_cb->evt_q.evt_q, 0, sizeof(AVM_EVT_Q_NODE_T));

         avm_cb->evt_q.evt_q->evt_id  = event_id;
         avm_cb->evt_q.evt_q->is_proc = bool;
         avm_cb->evt_q.evt_q->evt     = hpi_evt;
         avm_cb->evt_q.rear = avm_cb->evt_q.front  = avm_cb->evt_q.evt_q;
         avm_cb->evt_q.evt_q->next = AVM_EVT_Q_NODE_NULL;
      }
   }else
   {
      if(AVM_EVT_Q_NODE_NULL != (avm_cb->evt_q.rear->next = m_MMGR_ALLOC_AVM_EVT_Q_NODE))
      {
         m_AVM_LOG_CKPT_EVT_Q(AVM_LOG_EVT_Q_ENQ, event_id, bool, NCSFL_SEV_INFO);
         avm_cb->evt_q.rear           = avm_cb->evt_q.rear->next;
         m_NCS_MEMSET((uns8*)avm_cb->evt_q.rear, 0, sizeof(AVM_EVT_Q_NODE_T));

         avm_cb->evt_q.rear->evt_id   = event_id;
         avm_cb->evt_q.rear->is_proc = bool;
         avm_cb->evt_q.rear->evt     = hpi_evt;
         avm_cb->evt_q.rear->next     = AVM_EVT_Q_NODE_NULL;
      }else
      {
         m_AVM_LOG_INVALID_VAL_FATAL(0);  
         return NCSCC_RC_FAILURE;
      }
   }

   return rc;
}

/****************************************************************************\
 * Function: avm_dequeue_hpi_evt
 *
 * Purpose:  Add new AVM_ENT_INFO_T  entry if action is ADD, remove it
 *           the tree if action is to remove and update data if request
 *           to update.
 *
 * Input: cb     - CB pointer.
 *         deq   - Event to be dequeued fro standby
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/

extern uns32
avm_dequeue_hpi_evt(AVM_CB_T            *cb,
                    AVM_EVT_Q_NODE_T    *deq)
{
   AVM_EVT_Q_NODE_T *node = AVM_EVT_Q_NODE_NULL;
   AVM_EVT_Q_NODE_T *temp = AVM_EVT_Q_NODE_NULL;

   m_AVM_LOG_CKPT_EVT_Q(AVM_LOG_EVT_Q_DEQ, deq->evt_id, deq->is_proc, NCSFL_SEV_INFO);

   for(node = cb->evt_q.front; deq != node; )
   {
      temp = node->next;
      m_AVM_LOG_CKPT_EVT_Q(AVM_LOG_EVT_Q_DEQ, deq->evt_id, deq->is_proc, NCSFL_SEV_INFO);

      if(node->evt != AVM_EVT_NULL)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(node->evt->evt.hpi_evt);
         m_MMGR_FREE_AVM_EVT(node->evt);
      }
      m_MMGR_FREE_AVM_EVT_Q_NODE(node);
      node = temp;
   }
  
   if(AVM_EVT_Q_NODE_NULL != node)
   {
      cb->evt_q.front = node->next;
      if(node->evt != AVM_EVT_NULL)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(node->evt->evt.hpi_evt);
         m_MMGR_FREE_AVM_EVT(node->evt);
      }
      m_MMGR_FREE_AVM_EVT_Q_NODE(node);
      if(AVM_EVT_Q_NODE_NULL == cb->evt_q.front)
      { 
        cb->evt_q.rear  = cb->evt_q.front;
        cb->evt_q.evt_q = AVM_EVT_Q_NODE_NULL;
      }
            
   }else
   {
      cb->evt_q.front = AVM_EVT_Q_NODE_NULL;
      cb->evt_q.rear  = AVM_EVT_Q_NODE_NULL;
   }
   return NCSCC_RC_SUCCESS;
}
/**************************************************************\
 * Function: avm_updt_evt_q
 *
 * Purpose:  This function is used at to update the event queue
 *           After proc HPI event, active sends ack to standby.
 *           standby enqueues it.
 *                
 *
 *
 * Input: cb  - CB pointer.
 *        sg - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
 \***********************************************************/


extern uns32 
avm_updt_evt_q(AVM_CB_T   *cb,
               AVM_EVT_T  *hpi_evt,
               uns32       event_id,
               NCS_BOOL    bool)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   
   AVM_EVT_Q_NODE_T *node;
   
   for(node = cb->evt_q.front; node != AVM_EVT_Q_NODE_NULL; node = node->next)
   {
      if(node->evt_id == event_id)
      {
         /* node->is_proc != bool would be sufficient but to
            avoid any further crashes  */

         /* If active has already sent ack, dequeue the event */
         if( ((TRUE == node->is_proc) && (FALSE == bool)) || 
             ((FALSE == node->is_proc) && (TRUE == bool)))
         {
            avm_dequeue_hpi_evt(cb, node);
            if(AVM_EVT_NULL != hpi_evt)
            {
               m_MMGR_FREE_AVM_DEFAULT_VAL(hpi_evt->evt.hpi_evt);
               m_MMGR_FREE_AVM_EVT(hpi_evt);
            }  
            return NCSCC_RC_SUCCESS;
         }else
         {
            m_NCS_DBG_PRINTF("\n most unlikely to happen");
            return NCSCC_RC_SUCCESS;
         }
      }
   }

   /* If active has already not yest sent ack, enqueue the event */
   rc = avm_enqueue_hpi_evt(cb, hpi_evt,  event_id, bool);
   return rc;
} 

/**************************************************************\
 * Function: avm_proc_evt_q
 *
 * Purpose:  This function is used at to process all the 
 *           unprocessed events at standby once it switches
 *           to become active 
 *           
 *                
 * Input: cb  - CB pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
 \***********************************************************/
extern uns32
avm_proc_evt_q(AVM_CB_T *cb)
{
   uns32 rc  = NCSCC_RC_SUCCESS;

   AVM_EVT_Q_NODE_T *node;
   AVM_EVT_Q_NODE_T *temp;
   AVM_EVT_ID_T      evt_id;
   
   for(node = cb->evt_q.front; node != AVM_EVT_Q_NODE_NULL; )
   {
      if(node->is_proc != TRUE)
      {
         node->evt->ssu_evt_id.evt_id = node->evt_id;
         node->evt->ssu_evt_id.src    = AVM_EVT_HPI;

         rc = avm_msg_handler(cb, node->evt);
        
         m_NCS_DBG_PRINTF("\n SSU tmr status is %d", cb->ssu_tmr.status); 
         if(AVM_TMR_EXPIRED == cb->ssu_tmr.status)
         {
            evt_id.evt_id = node->evt_id;
            evt_id.src    = AVM_EVT_HPI;
            m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(cb, &evt_id, AVM_CKPT_EVT_ID);
         }
         m_MMGR_FREE_AVM_DEFAULT_VAL(node->evt->evt.hpi_evt);
         m_MMGR_FREE_AVM_EVT(node->evt);
      }

      temp = node->next;
      m_MMGR_FREE_AVM_EVT_Q_NODE(node);
      node = temp;
   }

   cb->evt_q.front = cb->evt_q.rear = cb->evt_q.evt_q = AVM_EVT_Q_NODE_NULL;
   return rc;
}

/**************************************************************\
 * Function: avm_empty_evt_q
 *
 * Purpose:  This function is used at to empty all the evevt q.
 *                
 * Input: cb  - CB pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
 \***********************************************************/
extern uns32
avm_empty_evt_q(AVM_CB_T *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   AVM_EVT_Q_NODE_T *node;
   AVM_EVT_Q_NODE_T *temp;
   
   for(node = cb->evt_q.front; node != AVM_EVT_Q_NODE_NULL; )
   {
      if(node->is_proc != TRUE)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(node->evt->evt.hpi_evt);
         m_MMGR_FREE_AVM_EVT(node->evt);
      }

      temp = node->next;
      m_MMGR_FREE_AVM_EVT_Q_NODE(node);
      node = temp;
   }

   cb->evt_q.front = cb->evt_q.rear = cb->evt_q.evt_q = AVM_EVT_Q_NODE_NULL;
   return rc;
}

/****************************************************************************\
 * Function: avm_cleanup_db
 *
 * Purpose:  This function cleans up AvM DB
 *
 * Input: cb       - CB pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
\**************************************************************************/
extern uns32 avm_cleanup_db(AVM_CB_T   *cb)
{
   uns32 rc =  NCSCC_RC_SUCCESS;
  
   AVM_ENT_INFO_T      *ent_info;
   AVM_VALID_INFO_T    *valid_info; 
   AVM_ENT_DESC_NAME_T desc_name;
   
   SaHpiEntityPathT   entity_path;
  
   m_NCS_MEMSET(entity_path.Entry, 0, sizeof(SaHpiEntityPathT));  
   for(ent_info = avm_find_next_ent_info(cb, &entity_path); 
       ent_info != AVM_ENT_INFO_NULL; 
       ent_info = avm_find_next_ent_info(cb, &entity_path))
   {
      m_NCS_MEMCPY(entity_path.Entry, ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT));

      avm_rmv_ent_info(cb, ent_info);
      avm_delete_ent_info(cb, ent_info);
      avm_delete_ent_str_info(cb, ent_info);

      m_MMGR_FREE_AVM_ENT_INFO(ent_info);
      ent_info =  AVM_ENT_INFO_NULL;
   }

   m_NCS_MEMSET(desc_name.name, 0, NCS_MAX_INDEX_LEN);  
 
   for(valid_info  = avm_find_next_valid_info(cb, &desc_name);
       valid_info != AVM_VALID_INFO_NULL;
       valid_info  = avm_find_next_valid_info(cb, &desc_name))
   {
      
      m_NCS_MEMCPY(desc_name.name, valid_info->desc_name.name, valid_info->desc_name.length);
      avm_delete_valid_info(cb, valid_info);
      m_MMGR_FREE_AVM_VALID_INFO(valid_info);
      valid_info = AVM_VALID_INFO_NULL; 
   } 

   return rc;
}

extern uns32
avm_update_sensors(AVM_ENT_INFO_T *dest,
                   AVM_ENT_INFO_T *src
                  )
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 index;

   for(index = 0; index < AVM_MAX_SENSOR_COUNT; index++)
   {
      dest->sensors[index].SensorType = src->sensors[index].SensorType;
      dest->sensors[index].SensorNum  = src->sensors[index].SensorNum;
      dest->sensors[index].sensor_assert = src->sensors[index].sensor_assert;
   }
   
   return rc;
}


/**************************************************************\
 * Function: avm_dhcp_conf_per_label_update
 *
 * Purpose:  This function updates per label configuration
 *                
 * Input: cb  - AVM per label configuration.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
 \***********************************************************/

extern uns32
avm_dhcp_conf_per_label_update(
                               AVM_PER_LABEL_CONF *dhcp_label,
                               AVM_PER_LABEL_CONF *temp_label
                              )
{
   m_NCS_MEMCPY(&dhcp_label->name, &temp_label->name, sizeof(AVM_DHCP_CONF_NAME_TYPE));
   m_NCS_DBG_PRINTF("\n Label Name = %s",dhcp_label->name.name);
   m_NCS_MEMCPY(&dhcp_label->file_name, &temp_label->file_name, sizeof(AVM_DHCP_CONF_NAME_TYPE));
   m_NCS_MEMCPY(&dhcp_label->sw_version, &temp_label->sw_version, sizeof(AVM_DHCP_CONF_NAME_TYPE));
   m_NCS_MEMCPY(&dhcp_label->install_time, &temp_label->install_time, AVM_TIME_STR_LEN);
   dhcp_label->conf_chg = temp_label->conf_chg;

   return NCSCC_RC_SUCCESS;
}


/**************************************************************\
 * Function: avm_decode_ckpt_dhcp_conf_update
 *
 * Purpose:  This function updates SSU DHCP configuration
 *                
 * Input: cb  - AVM SSU, DHCP configuration.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
 \***********************************************************/

extern uns32
avm_decode_ckpt_dhcp_conf_update(
                        AVM_ENT_DHCP_CONF *dhcp_conf,
                        AVM_ENT_DHCP_CONF *temp_conf
                        )
{
   uns32 i;
   /* copy the mac addresses */
   for (i=0; i<AVM_MAX_MAC_ENTRIES; i++)
      m_NCS_MEMCPY(dhcp_conf->mac_address[i], temp_conf->mac_address[i], AVM_MAC_DATA_LEN);

   /* netboot and tftpserver */
   dhcp_conf->net_boot = temp_conf->net_boot;
   m_NCS_MEMCPY(dhcp_conf->tftp_serve_ip, temp_conf->tftp_serve_ip, 4);

   /* preferred label, label1 and label2 */
   m_NCS_MEMCPY(&dhcp_conf->pref_label, &temp_conf->pref_label, sizeof(AVM_DHCP_CONF_NAME_TYPE));
   avm_dhcp_conf_per_label_update(&dhcp_conf->label1, &temp_conf->label1);
   avm_dhcp_conf_per_label_update(&dhcp_conf->label2, &temp_conf->label2);

   return NCSCC_RC_SUCCESS;
}


/**************************************************************\
 * Function: avm_decode_ckpt_dhcp_state_update
 *
 * Purpose:  This function updates SSU DHCP configuration state
 *                
 * Input: cb  - AVM SSU, DHCP configuration.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
 \***********************************************************/

extern uns32
avm_decode_ckpt_dhcp_state_update(
                        AVM_ENT_DHCP_CONF *dhcp_conf,
                        AVM_ENT_DHCP_CONF *temp_conf
                        )
{
   /* set default label */
   if (temp_conf->def_label_num == AVM_DEFAULT_LABEL_1)
   {
      dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_1;
      dhcp_conf->default_label = &dhcp_conf->label1;
   }
   else
   if (temp_conf->def_label_num == AVM_DEFAULT_LABEL_2)
   {
      dhcp_conf->def_label_num = AVM_DEFAULT_LABEL_2;
      dhcp_conf->default_label = &dhcp_conf->label2;
   }
   else
   {
      dhcp_conf->def_label_num  = AVM_DEFAULT_LABEL_NULL;
      dhcp_conf->default_label = NULL;
   }

   dhcp_conf->default_chg = temp_conf->default_chg;
   dhcp_conf->upgd_prgs = temp_conf->upgd_prgs;

   /* set current active label */
   if (temp_conf->cur_act_label_num == AVM_CUR_ACTIVE_LABEL_1)
   {
      dhcp_conf->cur_act_label_num = AVM_CUR_ACTIVE_LABEL_1;
      dhcp_conf->curr_act_label = &dhcp_conf->label1;
   }
   else
   if (temp_conf->cur_act_label_num == AVM_CUR_ACTIVE_LABEL_2)
   {
      dhcp_conf->cur_act_label_num = AVM_CUR_ACTIVE_LABEL_2;
      dhcp_conf->curr_act_label = &dhcp_conf->label2;
   }
   else
   {
      dhcp_conf->cur_act_label_num = AVM_CUR_ACTIVE_LABEL_NULL;
      dhcp_conf->curr_act_label = NULL;
   }

   /* update the status */
   dhcp_conf->label1.status = temp_conf->label1.status;
   dhcp_conf->label2.status = temp_conf->label2.status;

   return NCSCC_RC_SUCCESS;
}

/**************************************************************\
 * Function: avm_decode_ckpt_upgd_state_update
 *
 * Purpose:  This function updates SSU DHCP configuration state
 *                
 * Input: cb  - AVM SSU, DHCP configuration.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 *
 \***********************************************************/

extern uns32
avm_decode_ckpt_upgd_state_update(
                        AVM_ENT_DHCP_CONF *dhcp_conf,
                        AVM_ENT_DHCP_CONF *temp_conf
                        )
{
   m_NCS_MEMCPY(&dhcp_conf->ipmc_helper_node, &temp_conf->ipmc_helper_node, sizeof(AVM_DHCP_CONF_NAME_TYPE));

   dhcp_conf->ipmc_helper_ent_path.length = temp_conf->ipmc_helper_ent_path.length;
   m_NCS_MEMCPY(dhcp_conf->ipmc_helper_ent_path.name, temp_conf->ipmc_helper_ent_path.name, AVM_MAX_INDEX_LEN);

   dhcp_conf->upgrade_type        = temp_conf->upgrade_type;
   dhcp_conf->ipmb_addr           = temp_conf->ipmb_addr;
   dhcp_conf->ipmc_upgd_state     = temp_conf->ipmc_upgd_state;
   dhcp_conf->pld_bld_ipmc_status = temp_conf->pld_bld_ipmc_status;
   dhcp_conf->pld_rtm_ipmc_status = temp_conf->pld_rtm_ipmc_status; 
   dhcp_conf->bios_upgd_state     = temp_conf->bios_upgd_state;

   return NCSCC_RC_SUCCESS;
}
