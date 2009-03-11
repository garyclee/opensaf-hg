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
 
  DESCRIPTION:This module deals with registering AvM with EDSv for subscribing  
  HPI events from HISv.
  ..............................................................................

  Function Included in this Module:

  avm_eda_initialize       -
  avm_chan_open_callback   -
  avm_evt_callback         -
 
******************************************************************************
*/


#include "avm.h"
#include "avm_util.h"

uns32 g_fault_events_sub_id = 0;
uns32 g_non_fault_events_sub_id = 0;

/* Call Back declarations used by EDSv */
static void avm_chan_open_callback(
               SaInvocationT inv, 
               SaEvtChannelHandleT chan_hdl,
               SaAisErrorT rc
              );

static void avm_evt_callback(
            SaEvtSubscriptionIdT sub_id, 
            SaEvtEventHandleT event_hdl, 
            SaSizeT evt_data_size
             );
static uns32 
avm_alloc_pattern_array(
                        SaEvtEventPatternArrayT    **pattern_array, 
                        uns32                        num_patterns, 
                        uns32                        pattern_size
                       );

static uns32 
avm_free_pattern_array(
                         SaEvtEventPatternArrayT    *pattern_array
                      );

/* Pattern Array set for subscribing events */
static SaEvtEventFilterT avm_fault_filter_array[AVM_FAULT_FILTER_ARRAY_LEN]
 = { 
      {SA_EVT_EXACT_FILTER, 
         {
            SAHPI_CRITICAL_PATTERN_LEN, 
            SAHPI_CRITICAL_PATTERN_LEN, 
            (SaUint8T *) SAHPI_CRITICAL_PATTERN
         }
      },

      {SA_EVT_EXACT_FILTER, 
         {
            SAHPI_MAJOR_PATTERN_LEN, 
            SAHPI_MAJOR_PATTERN_LEN, 
            (SaUint8T *) SAHPI_MAJOR_PATTERN
         }
      }
   };

static SaEvtEventFilterT avm_non_fault_filter_array[AVM_NON_FAULT_FILTER_ARRAY_LEN]
 = {
       {SA_EVT_PASS_ALL_FILTER, 
         {
            SAHPI_MINOR_PATTERN_LEN, 
            SAHPI_MINOR_PATTERN_LEN, 
            (SaUint8T *) SAHPI_MINOR_PATTERN
         }
       },   

       {SA_EVT_PASS_ALL_FILTER, 
         {
            SAHPI_INFORMATIONAL_PATTERN_LEN, 
            SAHPI_INFORMATIONAL_PATTERN_LEN, 
            (SaUint8T *) SAHPI_INFORMATIONAL_PATTERN
         }
       },

       {SA_EVT_PASS_ALL_FILTER, 
         {
            SAHPI_OK_PATTERN_LEN,  
            SAHPI_OK_PATTERN_LEN,
            (SaUint8T *) SAHPI_OK_PATTERN
         }
      },

      {SA_EVT_PASS_ALL_FILTER, 
        {
           SAHPI_DEBUG_PATTERN_LEN, 
           SAHPI_DEBUG_PATTERN_LEN, 
           (SaUint8T *) SAHPI_DEBUG_PATTERN
        }
      }
};
/***********************************************************************
 ******
 * Name          : avm_eda_initialize
 *
 * Description   : This function  initiailizes eda interface 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
 ***********************************************************************
 *****/
uns32
avm_eda_initialize(AVM_CB_T *cb)
{
   uns32                    rc                = NCSCC_RC_SUCCESS;
   SaVersionT               saf_version       = AVM_SAF_VERSION; 
   SaTimeT                  timeout           = AVM_HPI_EVT_RETENTION_TIME;
   SaEvtChannelOpenFlagsT   open_flags        = 0;   
   SaEvtCallbacksT          avm_eda_callbacks = {
                                                  &avm_chan_open_callback,
                                                  &avm_evt_callback
                                                };       

   rc = saEvtInitialize(&cb->eda_hdl, &avm_eda_callbacks, &saf_version);
   if(SA_AIS_OK != rc)
   {
      m_AVM_LOG_EDA_VAL(rc, AVM_LOG_EDA_EVT_INIT, AVM_LOG_EDA_FAILURE, NCSFL_SEV_ERROR);
      return AVM_EDA_FAILURE;
   }   
   m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_INIT, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);

   open_flags = SA_EVT_CHANNEL_CREATE | SA_EVT_CHANNEL_SUBSCRIBER;
   cb->event_chan_name.length = m_NCS_STRLEN(AVM_HPI_EVT_CHANNEL_NAME);
   strcpy(cb->event_chan_name.value, AVM_HPI_EVT_CHANNEL_NAME);
  
   rc = saEvtSelectionObjectGet(cb->eda_hdl, &cb->eda_sel_obj);
   if(SA_AIS_OK != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_SELOBJ, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
   m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_SELOBJ, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);

   rc = saEvtChannelOpen(cb->eda_hdl, &cb->event_chan_name, open_flags, timeout, &cb->event_chan_hdl);
   if(SA_AIS_OK != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_CHANNEL_OPEN, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
      saEvtFinalize(cb->eda_hdl);
      return NCSCC_RC_FAILURE;
   }
   m_AVM_LOG_EDA(AVM_LOG_EDA_CHANNEL_OPEN, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);
   
   cb->fault_filter.filtersNumber = AVM_FAULT_FILTER_ARRAY_LEN;
   cb->fault_filter.filters       = avm_fault_filter_array;
   g_fault_events_sub_id = AVM_FAULT_EVENTS_SUB_ID;
   rc = saEvtEventSubscribe(cb->event_chan_hdl, &cb->fault_filter, g_fault_events_sub_id);
   if(SA_AIS_OK != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_SUBSCRIBE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
      saEvtChannelClose(cb->event_chan_hdl);
      saEvtFinalize(cb->eda_hdl);
      return NCSCC_RC_FAILURE;
   } 
   m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_SUBSCRIBE, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);

   cb->non_fault_filter.filtersNumber = AVM_NON_FAULT_FILTER_ARRAY_LEN;
   cb->non_fault_filter.filters       = avm_non_fault_filter_array;
   g_non_fault_events_sub_id          = AVM_NON_FAULT_EVENTS_SUB_ID;   
   rc = saEvtEventSubscribe(cb->event_chan_hdl, &cb->non_fault_filter, g_non_fault_events_sub_id);
   if(SA_AIS_OK != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_SUBSCRIBE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
      saEvtEventUnsubscribe(cb->event_chan_hdl, g_non_fault_events_sub_id);
      saEvtChannelClose(cb->event_chan_hdl);
      saEvtFinalize(cb->eda_hdl);
      return NCSCC_RC_FAILURE;
   } 

   cb->trap_channel = TRUE;

   /* Time to open Trap channnel */
   if (NCSCC_RC_SUCCESS != avm_open_trap_channel(cb))
   {
      cb->trap_channel = FALSE;
   } 
   else
      m_AVM_LOG_EDA(AVM_LOG_EDA_CHANNEL_OPEN, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);

   return NCSCC_RC_SUCCESS;
} 

/***********************************************************************
 ******
 * Name          : avm_eda_finalize
 *
 * Description   : This function  destroys eda interface 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
 ***********************************************************************
 *****/
extern uns32
avm_eda_finalize(AVM_CB_T *cb)
{
   uns32               rc = NCSCC_RC_SUCCESS;
  
   rc = saEvtEventUnsubscribe(cb->event_chan_hdl, g_fault_events_sub_id);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_UNSUBSCRIBE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
   }
   m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_UNSUBSCRIBE, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);

   rc = saEvtEventUnsubscribe(cb->event_chan_hdl, g_non_fault_events_sub_id);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_UNSUBSCRIBE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
   }
   m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_UNSUBSCRIBE, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);

   rc = saEvtChannelClose(cb->event_chan_hdl);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_CHANNEL_CLOSE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
   }
   m_AVM_LOG_EDA(AVM_LOG_EDA_CHANNEL_CLOSE, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);

   rc = saEvtFinalize(cb->eda_hdl);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_FINALIZE, AVM_LOG_EDA_FAILURE, NCSFL_SEV_CRITICAL);
   }
   m_AVM_LOG_EDA(AVM_LOG_EDA_EVT_FINALIZE, AVM_LOG_EDA_SUCCESS, NCSFL_SEV_INFO);
   return rc;
} 
       
static void 
avm_chan_open_callback(
         SaInvocationT       inv, 
         SaEvtChannelHandleT    chan_hdl, 
         SaAisErrorT       rc
                      )
{
   m_NCS_DBG_PRINTF("\n avm_chan_open_callback is called.....");
}

/***********************************************************************
******
* Name          : avm_evt_callback
*
* Description   : This function is called when an evt is received at AvM
*                 from EDSv. 
*
* Arguments     : AVM_CB_T*    - Pointer to AvM CB
*
* Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*
* Notes         : None
***********************************************************************
*****/


static void 
avm_evt_callback(
        SaEvtSubscriptionIdT sub_id, 
        SaEvtEventHandleT    event_hdl, 
        SaSizeT        evt_data_size
      )
{
   AVM_CB_T                *avm_cb        = AVM_CB_NULL;
   SaEvtEventPatternArrayT *pattern_array = NULL;
   AVM_EVT_T               *avm_evt       = AVM_EVT_NULL;
   HPI_EVT_T               *hpi_event     = NULL;
   SaUint8T                priority = 1;
   SaNameT                 publisher_name;
   SaTimeT                 retention_time = 90000000;
   SaTimeT                 publish_time;
   SaSizeT                 data_len;
   uns32                   rc;
   uns32                   num_patterns;
   uns32                   pattern_size;
   SaEvtEventIdT           event_id;
   SaUint8T                *p_data =  NULL;
   AVM_EVT_ID_T             evt_id;

   m_AVM_LOG_FUNC_ENTRY("avm_evt_callback");

   data_len = evt_data_size;

   p_data   = m_MMGR_ALLOC_AVM_DEFAULT_VAL(data_len + 1); 
   if(NULL == p_data)
   {
      m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return;   
   }
   m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_SUCCESS, NCSFL_SEV_INFO);

   memset(&publisher_name, '\0', sizeof(SaNameT));
   memset(p_data, 0, (size_t)data_len + 1);
   publisher_name.length = SA_MAX_NAME_LENGTH;

   num_patterns = AVM_MAX_PATTERNS;
   pattern_size = AVM_MAX_PATTERN_SIZE;

   rc = avm_alloc_pattern_array(&pattern_array, num_patterns, pattern_size);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(rc);

      /* Free the event */
      rc =  saEvtEventFree(event_hdl);

      /* Free Data Storage */
      if(NULL != p_data)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
      }
      return;
   }

   rc = saEvtEventAttributesGet(event_hdl, pattern_array, &priority,
                     &retention_time, &publisher_name, &publish_time, &event_id);
   if(SA_AIS_OK != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(rc);

      /* Free the event */
      rc =  saEvtEventFree(event_hdl);

      /* Free Data Storage */
      if(NULL != p_data)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
      }
   
      /* Free the pattern array */
      if(NULL != pattern_array)
      {
         pattern_array->patternsNumber = num_patterns;
         avm_free_pattern_array(pattern_array);
      }
      return;
   }

   if(!m_NCS_STRNCMP(pattern_array->patterns->pattern, "SA_EVT_LOST", 11))
   {
      m_AVM_LOG_DEBUG("Received LOST Event from EDS", NCSFL_SEV_NOTICE);

      /* Free the event */
      rc =  saEvtEventFree(event_hdl);

      /* Free Data Storage */
      if(NULL != p_data)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
      }
   
      /* Free the pattern array */
      if(NULL != pattern_array)
      {
         pattern_array->patternsNumber = num_patterns;
         avm_free_pattern_array(pattern_array);
      }
      return;
   }
   
   rc = saEvtEventDataGet(event_hdl, p_data, &data_len);
   if(SA_AIS_OK != rc)
   {
   
      m_AVM_LOG_INVALID_VAL_ERROR(rc);

      /* Free the event */
      rc =  saEvtEventFree(event_hdl);

      /* Free Data Storage */
      if(NULL != p_data)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
      }
   
      /* Free the pattern array */
      if(NULL != pattern_array)
      {
         pattern_array->patternsNumber = num_patterns;
         avm_free_pattern_array(pattern_array);
      }
      return;
   }

   m_AVM_LOG_GEN_EP_STR("Publisher Name  : ", publisher_name.value, NCSFL_SEV_INFO);

   hpi_event = m_MMGR_ALLOC_AVM_DEFAULT_VAL(sizeof(HPI_EVT_T)); 
   if(NULL == hpi_event)
   {
      m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);

      /* Free the event */
      rc =  saEvtEventFree(event_hdl);

      /* Free Data Storage */
      if(NULL != p_data)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
      }
   
      /* Free the pattern array */
      if(NULL != pattern_array)
      {
         pattern_array->patternsNumber = num_patterns;
         avm_free_pattern_array(pattern_array);
      }

      return;   
   }
   
   m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_SUCCESS, NCSFL_SEV_INFO);
   memset(hpi_event, '\0', sizeof(HPI_EVT_T));

   /* decode the event here */
   if ((rc = hpl_decode_hisv_evt ((HPI_HISV_EVT_T *)hpi_event, p_data, data_len, (HISV_SW_VERSION | HISV_EDS_INF_VERSION))) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(rc);
      /* Free the event */
      rc =  saEvtEventFree(event_hdl);

      /* Free Data Storage */
      if(NULL != p_data)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
      }
   
      /* Free the pattern array */
      if(NULL != pattern_array)
      {
         pattern_array->patternsNumber = num_patterns;
         avm_free_pattern_array(pattern_array);
      }

      /* Free hpi_event  */
      if(NULL != hpi_event)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(hpi_event);
      }

      return;
   }

   avm_evt = (AVM_EVT_T*) m_MMGR_ALLOC_AVM_EVT;
   if(avm_evt == AVM_EVT_NULL)
   {
      m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      m_NCS_DBG_PRINTF("\n Malloc failed for AvM Evt");

      /* Free the event */
      rc =  saEvtEventFree(event_hdl);

      /* Free Data Storage */
      if(NULL != p_data)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
      }
   
      /* Free the pattern array */
      if(NULL != pattern_array)
      {
         pattern_array->patternsNumber = num_patterns;
         avm_free_pattern_array(pattern_array);
      }

      /* Free hpi_event  */
      if(NULL != hpi_event)
      {
         m_MMGR_FREE_AVM_DEFAULT_VAL(hpi_event);
      }

      return;
   }
   m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_SUCCESS, NCSFL_SEV_INFO);

   avm_evt->src         = AVM_EVT_HPI;
   avm_evt->evt.hpi_evt = hpi_event; 
   avm_evt->data_len = data_len;

   avm_cb = (AVM_CB_T*) ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);
   if(AVM_CB_NULL == avm_cb) 
   {
      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
   }else
   {
      if((avm_cb->ha_state == SA_AMF_HA_STANDBY) || (avm_cb->ha_state == SA_AMF_HA_QUIESCING) || (avm_cb->ha_state == SA_AMF_HA_QUIESCED) || (TRUE == avm_cb->adm_switch))
      {

          /*  boot_succ_tmr for the corresponding entity
            Needs to be started/stopped at Active & Switchover to fix the 
            case for Switchover/Failover - Only the Active will act on the 
            Expiry of this timer. */
         {
              AVM_ENT_INFO_T       *ent_info;
              AVM_FSM_EVT_TYPE_T   fsm_evt_type;
              
              ent_info = avm_find_ent_info(avm_cb, &avm_evt->evt.hpi_evt->entity_path);

              if(ent_info)
              {
                 avm_standby_map_hpi2fsm(avm_cb, avm_evt->evt.hpi_evt, &fsm_evt_type, ent_info);
                 if((fsm_evt_type == AVM_EVT_INSERTION_PENDING) || (fsm_evt_type == AVM_EVT_SENSOR_FW_PROGRESS))
                 {
                     avm_standby_boot_succ_tmr_handler(avm_cb, avm_evt,ent_info,fsm_evt_type);
                 }
              }
         } /* End of 91104 fix */

         rc = avm_updt_evt_q(avm_cb, avm_evt, event_id, FALSE);
         /* Free the event */
         rc =  saEvtEventFree(event_hdl);
         if (SA_AIS_OK != rc)
         {
            m_AVM_LOG_INVALID_VAL_ERROR(rc);
         }

          /* Free Data Storage */
         if(NULL != p_data)
         {
            m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
         }
  
         /* Free the pattern array */
         if(NULL != pattern_array)
         {
            pattern_array->patternsNumber = num_patterns;
            avm_free_pattern_array(pattern_array);
         }

         return ;
      }

      m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

      avm_evt->ssu_evt_id.evt_id = event_id;
      avm_evt->ssu_evt_id.src    = AVM_EVT_HPI;

      avm_msg_handler(avm_cb, avm_evt);

      evt_id.evt_id = event_id;
      evt_id.src    = AVM_EVT_HPI;
      
      /* check if message got queued for SSU processing before sending ack */
      if (avm_cb->ssu_tmr.status == AVM_TMR_EXPIRED)
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, &evt_id, AVM_CKPT_EVT_ID); 
      ncshm_give_hdl(g_avm_hdl);
   }
  
   /* Free Data Storage */
   if(NULL != p_data)
   {
      m_MMGR_FREE_AVM_DEFAULT_VAL(p_data);
   }

   if(AVM_EVT_NULL != avm_evt)
   {
      m_MMGR_FREE_AVM_EVT(avm_evt);
   }

   /* Free the event */
   rc =  saEvtEventFree(event_hdl);
   if (SA_AIS_OK != rc)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(rc);
   }
   
   /* Free the pattern array */
   if(NULL != pattern_array)
   {
      pattern_array->patternsNumber = num_patterns;
      avm_free_pattern_array(pattern_array);
   }
   /* Free hpi_event  */
   if(NULL != hpi_event)
   {
      m_MMGR_FREE_AVM_DEFAULT_VAL(hpi_event);
   }

   return;
}

static uns32 
avm_alloc_pattern_array(
                         SaEvtEventPatternArrayT    **pattern_array,
                         uns32                        num_patterns,
                         uns32                        pattern_size
                       )
{
   SaEvtEventPatternArrayT    *array_pattern_ptr;
   SaEvtEventPatternT         *pattern_ptr;
   uns32                       i;

   array_pattern_ptr = m_MMGR_ALLOC_AVM_EVENT_PATTERN_ARRAY;
   
   if((SaEvtEventPatternArrayT*)NULL == array_pattern_ptr)
   {
      m_AVM_LOG_MEM(AVM_LOG_PATTERN_ARRAY_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return NCSCC_RC_FAILURE;
   }

   array_pattern_ptr->patterns = m_MMGR_ALLOC_AVM_EVENT_PATTERNS(num_patterns);
   if((SaEvtEventPatternT*)NULL == array_pattern_ptr->patterns)
   {
      m_AVM_LOG_MEM(AVM_LOG_PATTERN_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return NCSCC_RC_FAILURE;
   }

   pattern_ptr = array_pattern_ptr->patterns;
   for(i = 0; i < num_patterns; i++)
   {
      pattern_ptr->pattern = m_MMGR_ALLOC_AVM_DEFAULT_VAL(pattern_size);
      if(NULL == pattern_ptr->pattern)
      {
         m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
         return NCSCC_RC_FAILURE;
      }
      pattern_ptr->patternSize = pattern_size;
      pattern_ptr->allocatedSize = pattern_size;
      pattern_ptr++;
   }

   array_pattern_ptr->patternsNumber  = num_patterns;
   array_pattern_ptr->allocatedNumber = num_patterns;

   *pattern_array = array_pattern_ptr;

   return NCSCC_RC_SUCCESS;
}

static uns32 
avm_free_pattern_array(
                         SaEvtEventPatternArrayT    *pattern_array
                      )
{
   SaEvtEventPatternT         *pattern_ptr;
   uns32                       i;

   pattern_ptr = pattern_array->patterns;
   for(i = 0; i < pattern_array->patternsNumber; i++)
   {
      if(NULL != pattern_ptr->pattern)
      {   
         m_MMGR_FREE_AVM_DEFAULT_VAL(pattern_ptr->pattern);
      }
      pattern_ptr++;
   }

   if(NULL != pattern_array->patterns)
   {
      m_MMGR_FREE_AVM_EVENT_PATTERNS(pattern_array->patterns);
   }
   m_MMGR_FREE_AVM_EVENT_PATTERN_ARRAY(pattern_array);
   return NCSCC_RC_SUCCESS;
}
