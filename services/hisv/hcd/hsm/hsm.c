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
*                                                                            *
*  MODULE NAME:  hsm.c                                                       *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality of HPI Session Manager.            *
*  HSM is component of HCD and it blocks to receive the HPI events on open   *
*  HPI session. It publishes the received HPI events on to EDSv              *
*                                                                            *
*****************************************************************************/

#include "hcd.h"

/* forward declarations of local functions */
static uns32 publish_inspending(HSM_CB *hsm_cb, SaHpiRptEntryT *RptEntry);
static uns32 publish_active_healty(HSM_CB *hsm_cb, SaHpiRptEntryT *RptEntry);
static uns32 publish_curr_hs_state_evt(HSM_CB *hsm_cb, SaHpiRptEntryT *RptEntry);
static uns32 publish_extracted(HSM_CB *hsm_cb, uns8 *node_state);
#if 0
static uns32 publish_fwprog_events(HSM_CB *hsm_cb, SaHpiRptEntryT *RptEntry);
static uns32 hsm_clear_sel(HSM_CB *hsm_cb);
#endif 
static uns32 hsm_rediscover(HCD_CB *hcd_cb, HSM_CB *hsm_cb, SaHpiSessionIdT *session_id);
static uns32 dispatch_hotswap(HSM_CB *hsm_cb);
static uns32 hsm_inv_data_proc(SaHpiSessionIdT session_id, SaHpiDomainIdT domain_id,
                  SaHpiRdrT *Rdr, SaHpiRptEntryT  *RptEntry, uns8 **event_data,
                  SaHpiUint32T   *actual_size, SaHpiUint32T   *invdata_size, uns32 len);
static uns32 hsm_ham_resend_chassis_id(void);

/****************************************************************************
 * Name          : hcd_hsm
 *
 * Description   : This is actual HSM thread. It gets as argument, the
 *                 session and domain identifiers of HPI session it manages.
 *                 It blocks on HPI to receive HPI event and publishes the
 *                 received events to EDSv
 *
 * Arguments     : Pointer argument strucutre contains session & domain id.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hcd_hsm()
{
   SaHpiEventT     event;
   SaHpiRdrT   Rdr;
   SaHpiRptEntryT  RptEntry;
   SaHpiEntityPathT  epath;

   SaHpiDomainIdT *domain_id;
   SaHpiSessionIdT *session_id;
   SaErrorT val, hotswap_err, autoextract_err;
   SaErrorT policy_err=0;
   SaHpiUint32T   actual_size, invdata_size=sizeof(HISV_INV_DATA);
   HSM_CB *hsm_cb = 0;
   SIM_CB *sim_cb = 0;
   HCD_CB *hcd_cb = 0;
   uns8 *event_data, severity;
   HSM_EVT_TYPE evt_type = HSM_FAULT_EVT;
   uns32 evt_len = sizeof(SaHpiEventT), epath_len = sizeof(SaHpiEntityPathT);
   uns32 i, rc, min_evt_len = HISV_MIN_EVT_LEN; /* minimum message length does not include inventory data */
#ifndef HPI_A
   uns32 slot_ind;
#endif

   /* retrieve HSM CB */
   hsm_cb = (HSM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hsm_hdl);
   sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
   m_LOG_HISV_DTS_CONS("hcd_hsm: HSM thread initializing\n");
   if ((!hsm_cb) || (!sim_cb))
   {
      /* Can't do anything without control block.. */
      m_LOG_HISV_DTS_CONS("hcd_hsm: failed to take HSM handle\n");
      return NCSCC_RC_FAILURE;
   }
   /* retrieve HCD and ShIM CB */
   hcd_cb = (HCD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hcd_hdl);
   if (hcd_cb == NULL)
   {
      m_LOG_HISV_DTS_CONS("hcd_hsm: Error getting HCD CB handle\n");
      goto error;
   }

   /* collect the domain-id and session-id of HPI session */
   domain_id = &hsm_cb->args->domain_id;
   session_id = &hsm_cb->args->session_id;

   /* wait till we get CSI assignment from AMF */
   while (hcd_cb->ha_state != SA_AMF_HA_ACTIVE)
   {
      /* m_LOG_HISV_DTS_CONS("hcd_hsm: SA_AMF_HA_STANDBY assigned role\n"); */
      m_NCS_TASK_SLEEP(3000);
   }
   m_LOG_HISV_DTS_CONS("hcd_hsm: got SA_AMF_HA_ACTIVE assigned role\n");

   /* rediscover with CSI assignment */
   if (hcd_cb->args->rediscover)
      hsm_rediscover(hcd_cb, hsm_cb, session_id);
   else
   {
     /* subscribe to receive events on this HPI session */
#ifdef HPI_A
      val = saHpiSubscribe(*session_id, SAHPI_FALSE);
#else
      val = saHpiSubscribe(*session_id);
#endif
      if (val != SA_OK)
      {
         m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiSubscribe has failed \n");
         hsm_cb->args->session_valid = 0;
         hsm_rediscover(hcd_cb, hsm_cb, session_id);
      }
      else
      {
         /* process the outstanding insertion pending events */
         if (dispatch_hotswap(hsm_cb) != NCSCC_RC_SUCCESS)
         {
            m_LOG_HISV_DTS_CONS("hcd_hsm: Error in dispatch_hotswap\n");
            hsm_cb->args->session_valid = 0;
            if (hsm_rediscover(hcd_cb, hsm_cb, session_id) != NCSCC_RC_SUCCESS)
            {
               m_LOG_HISV_DTS_CONS("hcd_hsm: again there was error in hsm_rediscover\n");
            }
         }
      }
   }
   /* just for HAM to get ready */
   m_NCS_TASK_SLEEP(2000);

   m_LOG_HISV_DTS_CONS("hcd_hsm: HSM blocking to receive events\n");
   while (TRUE)
   {
      /* block to receive event on HPI session */
#ifdef HPI_A
      if ((rc = saHpiEventGet(*session_id, SAHPI_TIMEOUT_BLOCK, &event, &Rdr,
                        &RptEntry)) != SA_OK)
#else
      if ((rc = saHpiEventGet(*session_id, SAHPI_TIMEOUT_BLOCK, &event, &Rdr,
                        &RptEntry, NULL)) != SA_OK)
#endif
      {
         /* return the health-check status. Actually saHpiEventGet
          * should not block with SAHPI_TIMEOUT_BLOCK.
          */
         m_NCS_SEM_GIVE(hcd_cb->p_s_handle);

#ifdef HPI_A
         /* wait for sometime before looking for next event */
         if (rc != SA_ERR_HPI_TIMEOUT)
         {
            /* m_LOG_HISV_DTS_CONS ("hcd_hsm: Error in saHpiEventGet\n"); */
            if (hcd_cb->ha_state == SA_AMF_HA_ACTIVE)
            {
               hsm_cb->args->session_valid = 0;
               hsm_rediscover(hcd_cb, hsm_cb, session_id);
            }
            else
               m_NCS_TASK_SLEEP(3500);
         }
#else
         rc = saHpiUnsubscribe(*session_id);
         rc = saHpiSessionClose(*session_id);
         hsm_cb->args->session_valid = 0;
         hsm_rediscover(hcd_cb, hsm_cb, session_id);
#endif
         if ((hcd_cb->ha_state == SA_AMF_HA_ACTIVE) && (hcd_cb->args->rediscover))
         {
            m_LOG_HISV_DTS_CONS("Invalid HPI Session: Re-Initializing...\n");
            hsm_cb->args->session_valid = 0;
            hsm_rediscover(hcd_cb, hsm_cb, session_id);
         }
         continue;
      }
      /* return the health-check status */
      m_NCS_SEM_GIVE(hcd_cb->p_s_handle);

      if (hcd_cb->ha_state != SA_AMF_HA_ACTIVE)
         continue;

#ifndef HPI_A
      if(!((RptEntry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SWITCH_BLADE)||
         (RptEntry.ResourceEntity.Entry[0].EntityType == ((SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 1)))||
         (RptEntry.ResourceEntity.Entry[0].EntityType == ((SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)))))
         {
           /* don't care about this event */
          continue;
         }

       m_NCS_CONS_PRINTF(" HPI event from resource_id=%d\n", RptEntry.ResourceId);

      if(RptEntry.ResourceEntity.Entry[0].EntityType == ((SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)))
         {
             /* Amc case,Check Slot range check */
            if(!((RptEntry.ResourceEntity.Entry[3].EntityLocation > 0) &&
                (RptEntry.ResourceEntity.Entry[3].EntityLocation <= MAX_NUM_SLOTS)))
                 continue; 

          }
      else
          {  
              if(!((RptEntry.ResourceEntity.Entry[1].EntityLocation > 0) &&
              (RptEntry.ResourceEntity.Entry[1].EntityLocation <= MAX_NUM_SLOTS)  &&
              ((RptEntry.ResourceEntity.Entry[1].EntityType == SAHPI_ENT_SYSTEM_CHASSIS) ||
              (RptEntry.ResourceEntity.Entry[1].EntityType == SAHPI_ENT_PHYSICAL_SLOT)) &&
              (
#ifdef HPI_B_02
              (RptEntry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_PICMG_FRONT_BLADE) ||
              (RptEntry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_BLADE) ||
#endif
              (RptEntry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SWITCH_BLADE))))
              {
                /* don't care about this event */
                m_LOG_HISV_DTS_CONS("Discarding Event, not from Controller or Payload\n");
                continue;
              }
          }

#endif
      /* check if it is system firmware progress event, publish it to ShIM. */
      if ((event.EventType == SAHPI_ET_SENSOR) &&
          (event.EventDataUnion.SensorEvent.SensorType == SAHPI_SYSTEM_FW_PROGRESS))
      {
         SIM_EVT  *sim_evt;
         sim_evt = (SIM_EVT *)m_MMGR_ALLOC_SIM_EVT;
         sim_evt->evt_type = HCD_SIM_FIRMWARE_PROGRESS;
         m_NCS_MEMCPY(&sim_evt->msg.fp_evt, (uns8 *)&event, evt_len);
         m_NCS_MEMCPY(&sim_evt->msg.epath, (uns8 *)&RptEntry.ResourceEntity, epath_len);

         if(m_NCS_IPC_SEND(&sim_cb->mbx, sim_evt, NCS_IPC_PRIORITY_NORMAL)
            == NCSCC_RC_FAILURE)
         {
            m_MMGR_FREE_SIM_EVT(sim_evt);
            m_LOG_HISV_DTS_CONS("hcd_hsm: failed to deliver msg on ShIM mail-box\n");
         }
         /* proceed as AvM needs this events now */
         /* continue; */
      }

      /* select the appropriate event handle for publishing this event */
      switch (event.Severity)
      {
         case SAHPI_CRITICAL:
         case SAHPI_MAJOR:
            evt_type = HSM_FAULT_EVT;
            severity = NCSFL_SEV_ALERT;
            break;
         default:
            evt_type = HSM_NON_FAULT_EVT;
            severity = NCSFL_SEV_INFO;
            break;
      }
      if (event.EventType != SAHPI_ET_HOTSWAP) 
#if 0
         || 
          (RptEntry.ResourceEntity.Entry[2].EntityType > MAX_HPI_ENTITY_TYPE))
#endif
      {
#ifdef HPI_A
         m_NCS_CONS_PRINTF("Non hotswap HPI event from entity_type=%d,%d slot=%d event_type=%d, resource_id=%d\n",
                            RptEntry.ResourceEntity.Entry[2].EntityType,
                            RptEntry.ResourceEntity.Entry[0].EntityType,
                            RptEntry.ResourceEntity.Entry[2].EntityInstance, event.EventType, RptEntry.ResourceId);

         m_LOG_HISV_DEBUG_VAL(HCD_NON_HOTSWAP_TYPE, RptEntry.ResourceEntity.Entry[2].EntityType);
#else
         m_NCS_CONS_PRINTF("Non hotswap HPI event from entity_type=%d slot=%d event_type=%d, resource_id=%d\n",
                            RptEntry.ResourceEntity.Entry[1].EntityType,
                            RptEntry.ResourceEntity.Entry[1].EntityLocation, event.EventType, RptEntry.ResourceId);

         m_LOG_HISV_DEBUG_VAL(HCD_NON_HOTSWAP_TYPE, RptEntry.ResourceEntity.Entry[1].EntityType);
#endif

         /* if it is a OEM event for resource restore, publish outstanding hotswap event */
        if (event.EventType == SAHPI_ET_RESOURCE)
        {
           if(event.EventDataUnion.ResourceEvent.ResourceEventType == SAHPI_RESE_RESOURCE_RESTORED)
           {
              /* m_NCS_CONS_PRINTF("Resource restored event received\n"); */
              publish_curr_hs_state_evt(hsm_cb, &RptEntry);
           }
        }
        /* IR00085909: If the event_type is SAHPI_ET_SENSOR and sensor_type is SAHPI_ENTITY_PRESENCE then 
            we need to query the HotSwap state of the resource and publish it (During the IPMC upgrade it is not possible 
            to query the HotSwap state of the Resource, instead it generates the above sensor event after IPMC upgrade
            completes) */
        if (event.EventType == SAHPI_ET_SENSOR)
        {
            m_NCS_CONS_PRINTF ("Sensor event: Type=%d, Cat=%d, state=%d, prevstate=%d\n", 
                                                        event.EventDataUnion.SensorEvent.SensorType,
                                                        event.EventDataUnion.SensorEvent.EventCategory,
                                                        event.EventDataUnion.SensorEvent.EventState,
                                                        event.EventDataUnion.SensorEvent.PreviousState);

           if(event.EventDataUnion.SensorEvent.SensorType == SAHPI_ENTITY_PRESENCE)
           {
              m_NCS_CONS_PRINTF("Entity presence sensor event received\n");
#ifdef HPI_A
              if (hsm_cb->node_state[RptEntry.ResourceEntity.Entry[2].EntityInstance] == NODE_HS_STATE_NOT_PRESENT)
#else
              if (hsm_cb->node_state[RptEntry.ResourceEntity.Entry[1].EntityLocation] == NODE_HS_STATE_NOT_PRESENT)
#endif
              {
                  m_NCS_CONS_PRINTF("Invoking publish_curr_hs_state_evt to check the HS state of resource\n");
                  publish_curr_hs_state_evt(hsm_cb, &RptEntry);
              }

           }
        }
         /* continue; */
      }
      else
      {
#ifdef HPI_A
         print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                        event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                        RptEntry.ResourceEntity.Entry[2].EntityInstance,
                        RptEntry.ResourceEntity.Entry[2].EntityType);

         if ((RptEntry.ResourceEntity.Entry[2].EntityInstance > 0) && 
             (RptEntry.ResourceEntity.Entry[2].EntityInstance <= MAX_NUM_SLOTS) &&
             (RptEntry.ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) &&
             (RptEntry.ResourceEntity.Entry[0].EntityType == 160))
         {
             /* Change the state of the resourece in the local list */
             switch (event.EventDataUnion.HotSwapEvent.HotSwapState)
             {
               case SAHPI_HS_STATE_INACTIVE:
                 hsm_cb->node_state[RptEntry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_INACTIVE;
                 break;
               case SAHPI_HS_STATE_INSERTION_PENDING:
                 hsm_cb->node_state[RptEntry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_INSERTION_PENDING;
                 break;
               case SAHPI_HS_STATE_ACTIVE_HEALTHY:
                 hsm_cb->node_state[RptEntry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_ACTIVE_HEALTHY;
                 break;
               case SAHPI_HS_STATE_ACTIVE_UNHEALTHY:
                 hsm_cb->node_state[RptEntry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_ACTIVE_UNHEALTHY;
                 break;
               case SAHPI_HS_STATE_EXTRACTION_PENDING:
                 hsm_cb->node_state[RptEntry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_EXTRACTION_PENDING;
                 break;
               case SAHPI_HS_STATE_NOT_PRESENT:
                 hsm_cb->node_state[RptEntry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_NOT_PRESENT;
                 break;
             }
         }
#else
         /* Allow for the case where blades are ATCA or non-ATCA.                           */
         if (RptEntry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_BLADE)
           print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                          event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                          RptEntry.ResourceEntity.Entry[0].EntityLocation,
                          RptEntry.ResourceEntity.Entry[0].EntityType);
         else
         print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                        event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                        RptEntry.ResourceEntity.Entry[1].EntityLocation,
                        RptEntry.ResourceEntity.Entry[1].EntityType);
         switch (event.EventDataUnion.HotSwapEvent.HotSwapState)
         {
            case SAHPI_HS_STATE_INACTIVE:
               hsm_cb->node_state[RptEntry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_INACTIVE;
               break;
            case SAHPI_HS_STATE_INSERTION_PENDING:
               hsm_cb->node_state[RptEntry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_INSERTION_PENDING;
               break;
            case SAHPI_HS_STATE_ACTIVE:
               hsm_cb->node_state[RptEntry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_ACTIVE_HEALTHY;
               break;
            case SAHPI_HS_STATE_EXTRACTION_PENDING:
               hsm_cb->node_state[RptEntry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_EXTRACTION_PENDING;
               break;
            case SAHPI_HS_STATE_NOT_PRESENT:
               hsm_cb->node_state[RptEntry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_NOT_PRESENT;
               break;
         }
#endif
      }
      event_data = NULL;
      actual_size = 0;

      if ((event.EventType == SAHPI_ET_HOTSWAP) &&
          (event.EventDataUnion.HotSwapEvent.HotSwapState ==
           SAHPI_HS_STATE_EXTRACTION_PENDING))
      {
         /* take the control of extraction */
#ifdef HPI_A
         if (saHpiHotSwapControlRequest(*session_id, RptEntry.ResourceId) != SA_OK)
#else
         policy_err = saHpiHotSwapPolicyCancel(*session_id, RptEntry.ResourceId);

         if (policy_err != SA_OK)
#endif
         {
            if (policy_err == SA_ERR_HPI_INVALID_REQUEST)
            {
               /* Allow for the case where the hotswap policy cannot be canceled because it */
               /* has already begun to execute.                                             */
               m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiHotSwapPolicyCancel cannot cancel hotswap policy\n");
            }
            m_NCS_CONS_PRINTF("Extraction: Error taking control of resource Error  %d , Resouce ID %d \n",policy_err,RptEntry.ResourceId );
            policy_err=SA_OK;
         }
      }
      /* if it is hotswap event transitioning to Insertion Pending state */
      /* check if the resource contains inventory data record */
      if ((event.EventType == SAHPI_ET_HOTSWAP) &&
          (event.EventDataUnion.HotSwapEvent.HotSwapState ==
           SAHPI_HS_STATE_INSERTION_PENDING)
#if 0
          &&
          (RptEntry.ResourceCapabilities & SAHPI_CAPABILITY_INVENTORY_DATA)
           && (RptEntry.ResourceCapabilities & SAHPI_CAPABILITY_RDR)
#endif /* 0 */
         )

      {
         SaHpiTimeoutT auto_exttime = SAHPI_TIMEOUT_BLOCK;

#ifdef HPI_A
         if (saHpiHotSwapControlRequest(*session_id, RptEntry.ResourceId) != SA_OK)
#else
         hotswap_err = saHpiHotSwapPolicyCancel(*session_id, RptEntry.ResourceId);

         if (hotswap_err != SA_OK)
#endif
         {
            if (hotswap_err == SA_ERR_HPI_INVALID_REQUEST)
            {
               /* Allow for the case where the hotswap policy cannot be canceled because it */
               /* has already begun to execute.                                             */
               m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiHotSwapPolicyCancel cannot cancel hotswap policy\n");
            }
            else
            {
               m_LOG_HISV_DTS_CONS("hcd_hsm: Error taking control of resource\n");
            }
         }
         autoextract_err = saHpiAutoExtractTimeoutSet(*session_id, RptEntry.ResourceId, auto_exttime);

         if (autoextract_err != SA_OK)
         {
            if (autoextract_err == SA_ERR_HPI_READ_ONLY)
            {
               /* Allow for the case where the extraction timeout is read-only. */
               m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiAutoExtractTimeoutSet is read-only\n");
            }
            else
            {
               m_LOG_HISV_DTS_CONS("hcd_hsm: Error in setting Auto Extraction timeout\n");
            }
         }
         if (hsm_inv_data_proc(*session_id, *domain_id, &Rdr, &RptEntry, &event_data, &actual_size, 
             &invdata_size, min_evt_len) == NCSCC_RC_FAILURE)
         {
            m_LOG_HISV_DTS_CONS("hcd_hsm: Inventory data not found\n");
         }
      }
      else {
        if (event.EventType == SAHPI_ET_HOTSWAP) {
           /* A hotswap state other than INSERTION_PENDING.              */
           /* Try to get the inventory info - and provide inv data       */
           /* in the published event.                                    */
           if (hsm_inv_data_proc(*session_id, *domain_id, &Rdr, &RptEntry, &event_data, &actual_size, 
               &invdata_size, min_evt_len) == NCSCC_RC_FAILURE)
           {
              m_LOG_HISV_DTS_CONS("hcd_hsm: Inventory data not found\n");
           }
         }
      }
      /* if inventory data not available, just publish event and entity path */
      if (event_data == NULL)
      {
         event_data = m_MMGR_ALLOC_HPI_INV_DATA(min_evt_len + invdata_size);
         if (event_data == NULL)
         {
            m_NCS_CONS_PRINTF("hcd_hsm: memory error\n");
            continue;
         }
         /* invdata_size = 0; */
      }

      m_NCS_MEMCPY(event_data, (uns8 *)&event, evt_len);

      /* remove the grouping elements of entity path */
      m_NCS_OS_MEMSET(&epath, 0, epath_len);
#ifdef HPI_A
      for (i=2; i < SAHPI_MAX_ENTITY_PATH; i++)
         epath.Entry[i-2] = RptEntry.ResourceEntity.Entry[i];

      /* IR00060725: temporary fix till we migrate to HPI-B spec entity path mechanism */
      if ((RptEntry.ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) && 
          (RptEntry.ResourceEntity.Entry[0].EntityType != 160))
         epath.Entry[0].EntityType = RptEntry.ResourceEntity.Entry[0].EntityType;
#else
         if(RptEntry.ResourceEntity.Entry[0].EntityType == ((SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)))
        {
           /* Special Case of AMC-Sub Slot board */  
           epath.Entry[0]= RptEntry.ResourceEntity.Entry[1];    
           slot_ind = 2;
            for ( i=1; (slot_ind + i)<SAHPI_MAX_ENTITY_PATH; i++ )
            {
             epath.Entry[i] = RptEntry.ResourceEntity.Entry[slot_ind + i];
             if (RptEntry.ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_ROOT)
              {
            /* don't need to copy anymore */
               break;
              }
            }
        }
        
      else if ((RptEntry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_BLADE) ||
          (RptEntry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SWITCH_BLADE)) {
         /* NON-ATCA: Do not strip off leaf entry. */
         slot_ind = 0;
      }
      else {
         /* ATCA: Strip off leaf entry. */
         slot_ind = 1;
      }
      for ( i=0; (slot_ind + i)<SAHPI_MAX_ENTITY_PATH; i++ ) {
         /* For NON-ATCA, strip the SAHPI_ENT_RACK entry. */
         if (RptEntry.ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_RACK) {
            slot_ind = 1;
            i--;
         }
         else
            epath.Entry[i] = RptEntry.ResourceEntity.Entry[slot_ind + i];

         /* ATCA */ 
         if (RptEntry.ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_ROOT) {
            /* don't need to copy anymore */
            break;
         }
      }         
#endif

      m_NCS_MEMCPY(event_data+evt_len, (uns8 *)&epath, epath_len);

      /* Publish the consolidate event message */
      rc = hsm_eda_event_publish (evt_type, event_data, min_evt_len+invdata_size, hsm_cb);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_HISV_DTS_CONS("hcd_hsm: SaEvtEventPublish() failed.\n");
         hsm_cb->eds_init_success = 0;
         /* in case of failure, wait for sometime before looking for next event */
         m_NCS_TASK_SLEEP(100);
      }
      else
      {
         m_LOG_HISV_DTS_CONS("hcd_hsm: saEvtEventPublish Done\n");
      }
      m_MMGR_FREE_HPI_INV_DATA(event_data);

      /* re-discover if CSI set needs it */
      if (hcd_cb->args->rediscover)
      {
         m_LOG_HISV_DTS_CONS("Role change may need re-discover of resources...\n");
         hsm_cb->args->session_valid = 0;
         hsm_rediscover(hcd_cb, hsm_cb, session_id);
      }
   }
   /* release the control blocks */
   ncshm_give_hdl(gl_hcd_hdl);
   ncshm_give_hdl(gl_hsm_hdl);
   ncshm_give_hdl(gl_sim_hdl);

error:
   /* unsubscribe the reception of events on HPI session */
   saHpiUnsubscribe(*session_id);

   /* finalize HPI session */
   saHpiSessionClose(*session_id);

#ifdef HPI_A
   /* finalize the HPI session */
   saHpiFinalize();
#endif

   /* unregister with EDA channels */
   hsm_eda_chan_finalize(hsm_cb);

   /* remove the association with hdl-mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, hsm_cb->cb_hdl);

   /* free HPI session arguments */
   m_MMGR_FREE_HPI_SESSION_ARGS(hsm_cb->args);

   /* free the control block */
   m_MMGR_FREE_HSM_CB(hsm_cb);

   /* reset the global cb handle */
   gl_hsm_hdl = 0;
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : dispatch_hotswap
 *
 * Description   : This function publishes the outstanding insertion-pending
 *                 events which apparently happened before the HISv is
 *                 service is started.
 *
 * Arguments     : Pointer to HSM control block structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
dispatch_hotswap(HSM_CB *hsm_cb)
{
   SaHpiHsStateT  state;
   SaErrorT       err, hpi_rc, policy_err, hotswap_err, autoextract_err;
#ifdef HPI_A
   SaHpiRptInfoT   rpt_info_before;
#endif
   SaHpiEntryIdT   current;
   SaHpiEntryIdT   next;

   SaHpiDomainIdT  domain_id;
   SaHpiSessionIdT session_id;
   SaHpiRptEntryT  entry;
   uns32  rc = NCSCC_RC_SUCCESS, i;

   m_LOG_HISV_DTS_CONS("dispatch_hotswap: dispatching outstanding hotwap events\n");
   m_NCS_MEMSET(hsm_cb->node_state, 0, sizeof(hsm_cb->node_state));

   /* collect the domain-id and session-id of HPI session */
   domain_id = hsm_cb->args->domain_id;
   session_id = hsm_cb->args->session_id;

#ifdef HPI_A
   /* grab copy of the update counter before traversing RPT */
   err = saHpiRptInfoGet(session_id, &rpt_info_before);
   if (SA_OK != err)
   {
      m_LOG_HISV_DTS_CONS("dispatch_hotswap: saHpiRptInfoGet\n");
      return NCSCC_RC_FAILURE;
   }
#endif
   /** process the list of resource pointer tables on this session.
    ** verifies the existence of RPT entries.
    **/
   next = SAHPI_FIRST_ENTRY;
   do
   {
      current = next;
      /* get the HPI RPT entry */
      err = saHpiRptEntryGet(session_id, current, &next, &entry);
      if (SA_OK != err)
      {
         /* error getting RPT entry */
         if (current != SAHPI_FIRST_ENTRY)
         {
            m_LOG_HISV_DTS_CONS("dispatch_hotswap: Error saHpiRptEntryGet\n");
            m_NCS_CONS_PRINTF ("dispatch_hotswap: saHpiRptEntryGet, HPI error code = %d\n", err);
            return NCSCC_RC_FAILURE;
         }
         else
         {
            m_LOG_HISV_DTS_CONS("dispatch_hotswap: Empty RPT\n");
            m_NCS_CONS_PRINTF ("dispatch_hotswap: saHpiRptEntryGet, HPI error code = %d\n", err);
            rc = NCSCC_RC_FAILURE;
            break;
         }
      }

      /* clear the SEL if it supports; in order to avoid duplicate playback of events */
#ifdef HPI_A
      if (entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL)
#else
      if (entry.ResourceCapabilities & SAHPI_CAPABILITY_EVENT_LOG)
#endif
      {
         saHpiEventLogClear(session_id, entry.ResourceId);
      }
      /* if it is a FRU and hotswap managable then publish outstanding inspending events */
       if ((entry.ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) && 
          (entry.ResourceCapabilities & SAHPI_CAPABILITY_FRU))
      {

         hpi_rc = saHpiHotSwapStateGet(session_id, entry.ResourceId, &state);
         if (hpi_rc != SA_OK)
         {
            /* m_LOG_HISV_DTS_CONS("failed to get hotswap state\n"); */
#ifdef HPI_A
             m_NCS_CONS_PRINTF("Failed to get hotswap state from entity_type=%d,%d slot=%d, resource_id=%d: Error=%d\n",
                    entry.ResourceEntity.Entry[2].EntityType,
                    entry.ResourceEntity.Entry[0].EntityType,
                    entry.ResourceEntity.Entry[2].EntityInstance, entry.ResourceId, hpi_rc);
#else
             m_NCS_CONS_PRINTF("Failed to get hotswap state from entity_type=%d slot=%d, resource_id=%d: Error=%d\n",
                    entry.ResourceEntity.Entry[1].EntityType,
                    entry.ResourceEntity.Entry[1].EntityLocation, entry.ResourceId, hpi_rc);
#endif
         }
         else
         {

          SaHpiTimeoutT auto_exttime = HPI_AUTO_HS_STOP_TMR; /* Old Style */
#if 0
FIXME: This was suggested by mibis, validate it.
          SaHpiTimeoutT auto_exttime = SAHPI_TIMEOUT_BLOCK;  /* Enhancement */
#endif

#ifdef HPI_A
          autoextract_err = saHpiAutoExtractTimeoutSet(session_id, entry.ResourceId, auto_exttime);
          
          if (autoextract_err != SA_OK)
          {
             if (autoextract_err == SA_ERR_HPI_READ_ONLY)
             {
                /* Allow for the case where the extraction timeout is read-only. */
                m_LOG_HISV_DTS_CONS("dispatch_hotswap: saHpiAutoExtractTimeoutSet is read-only\n");
             }
             else
             {
                m_LOG_HISV_DTS_CONS("dispatch_hotswap: Error in setting Auto Extraction timeout\n");
             }
          }

#ifdef HPI_A
        hotswap_err = saHpiHotSwapControlRequest(session_id, entry.ResourceId);
#else
        hotswap_err = saHpiHotSwapPolicyCancel(session_id, entry.ResourceId);
#endif

         if (hotswap_err != SA_OK)
         {
            if (hotswap_err == SA_ERR_HPI_INVALID_REQUEST)
            {
               /* Allow for the case where the hotswap policy cannot be canceled because it */
               /* has already begun to execute.                                             */
               m_LOG_HISV_DTS_CONS("dispatch_hotswap: saHpiHotSwapPolicyCancel cannot cancel hotswap policy\n");
            }
            else
            {
               m_LOG_HISV_DTS_CONS("dispatch_hotswap: Error taking control of resource\n");
            }
         }   
            /* fill hotswap states */

            if ((entry.ResourceEntity.Entry[2].EntityInstance > 0) && 
                (entry.ResourceEntity.Entry[2].EntityInstance <= MAX_NUM_SLOTS) &&
                (entry.ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) &&
                (entry.ResourceEntity.Entry[0].EntityType == 160))

            {
               if (state == SAHPI_HS_STATE_INACTIVE)
                  hsm_cb->node_state[entry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_INACTIVE;
               else if (state == SAHPI_HS_STATE_EXTRACTION_PENDING)
                  hsm_cb->node_state[entry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_EXTRACTION_PENDING;
            }

            if (state == SAHPI_HS_STATE_INSERTION_PENDING)
#if 0   
                 && ((entry.ResourceEntity.Entry[2].EntityInstance > 0)
                 && (entry.ResourceEntity.Entry[2].EntityInstance < 15)
                 && (entry.ResourceEntity.Entry[2].EntityType <= MAX_HPI_ENTITY_TYPE)))
#endif
            {
               m_LOG_HISV_DTS_CONS("Publishing the outstanding Insertion Pending hotswap event\n");
               publish_inspending(hsm_cb, &entry);
               if ((entry.ResourceEntity.Entry[2].EntityInstance > 0) && 
                   (entry.ResourceEntity.Entry[2].EntityInstance <= MAX_NUM_SLOTS))
               {
                  if ((entry.ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) &&
                      (entry.ResourceEntity.Entry[0].EntityType == 160))
                     hsm_cb->node_state[entry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_INSERTION_PENDING;
               }
            }
            /* publish outstanding active healthy events and fwprog events */
            if (state == SAHPI_HS_STATE_ACTIVE_HEALTHY)
#if 0
               &&
               ((entry.ResourceEntity.Entry[2].EntityInstance == 1) ||
               (entry.ResourceEntity.Entry[2].EntityInstance == 2)))
#endif
            {
               m_LOG_HISV_DTS_CONS("Publishing the outstanding Active Healthy hotswap event\n");
               publish_active_healty(hsm_cb, &entry);
               if ((entry.ResourceEntity.Entry[2].EntityInstance > 0) && 
                   (entry.ResourceEntity.Entry[2].EntityInstance <= MAX_NUM_SLOTS))
               {
                  if ((entry.ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) &&
                      (entry.ResourceEntity.Entry[0].EntityType == 160))
                     hsm_cb->node_state[entry.ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_ACTIVE_HEALTHY;
               }
               /* publish_fwprog_events(hsm_cb, &entry); */
            }
#else
            /* Allow for the case where blades are ATCA or non-ATCA.                        */
        if(!((entry.ResourceEntity.Entry[0].EntityType == (SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 1))  ||
           (entry.ResourceEntity.Entry[0].EntityType == (SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)) ||
            ( 
#ifdef HPI_B_02
       (entry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_PICMG_FRONT_BLADE) ||
       (entry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_BLADE) ||
#endif
           (entry.ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SWITCH_BLADE))))

          {
          /* don't care about this event */
           continue;
          }

         if(entry.ResourceEntity.Entry[0].EntityType == ((SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)))
          {
             /* Amc case,Check Slot range check */
            if(!((entry.ResourceEntity.Entry[3].EntityLocation > 0) &&
                (entry.ResourceEntity.Entry[3].EntityLocation <= MAX_NUM_SLOTS)))
                 continue;
           }
          else
           { 

            if(!((entry.ResourceEntity.Entry[1].EntityLocation > 0) &&
                (entry.ResourceEntity.Entry[1].EntityLocation <= MAX_NUM_SLOTS)))
                 continue;
           }

          if ((policy_err=saHpiAutoExtractTimeoutSet(session_id, entry.ResourceId, auto_exttime)) != SA_OK)
          {
             policy_err=SA_OK;
          }

          if ((policy_err=saHpiHotSwapPolicyCancel(session_id, entry.ResourceId)) != SA_OK)
          {
             policy_err=SA_OK;
          }

            if (state == SAHPI_HS_STATE_INACTIVE)
            {
               hsm_cb->node_state[entry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_INACTIVE;
            }
            else if (state == SAHPI_HS_STATE_EXTRACTION_PENDING)
            {
               hsm_cb->node_state[entry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_EXTRACTION_PENDING;
            }
            else if (state == SAHPI_HS_STATE_INSERTION_PENDING)
            {
               m_LOG_HISV_DTS_CONS("Publishing the outstanding Insertion Pending hotswap event\n");
               publish_inspending(hsm_cb, &entry);
               hsm_cb->node_state[entry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_INSERTION_PENDING;
            }
            else if (state == SAHPI_HS_STATE_ACTIVE)
            {
               m_LOG_HISV_DTS_CONS("Publishing the outstanding Active hotswap event\n");
               publish_active_healty(hsm_cb, &entry);
               hsm_cb->node_state[entry.ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_ACTIVE_HEALTHY;
            }
#endif
         }
      }

   } while (next != SAHPI_LAST_ENTRY);

   /* check if all are not present to sense something wrong with discovery 
    (IR00085091,IR00084254) */
   for (i=1; i<=MAX_NUM_SLOTS; i++)
   {
      if (hsm_cb->node_state[i] != NODE_HS_STATE_NOT_PRESENT)
         break;
   }
   if (i > MAX_NUM_SLOTS)
   {
      m_LOG_HISV_DTS_CONS("no blades found. possibly discovery problem\n");
      return NCSCC_RC_FAILURE;
   }
   /* publish the dummy suprise extraction events of boards which are not present */
   publish_extracted(hsm_cb, hsm_cb->node_state);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : publish_inspending
 *
 * Description   : This function retrieves and publishes the outstanding
 *                 insertion pending events and associated inventory data.
 *
 * Arguments     : Pointer to HSM control block structure.
 *                 Pointer to RptEntry structure;
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
publish_inspending(HSM_CB *hsm_cb, SaHpiRptEntryT *RptEntry)
{
   SaHpiEventT     event;
   SaHpiRdrT   Rdr;
   SaHpiDomainIdT domain_id;
   SaHpiSessionIdT session_id;
   SaHpiEntityPathT  epath;
   SaHpiUint32T   actual_size, invdata_size=sizeof(HISV_INV_DATA);
   uns8 *event_data;
   uns32 evt_len = sizeof(SaHpiEventT), epath_len = sizeof(SaHpiEntityPathT);
   uns32 i, rc, min_evt_len = HISV_MIN_EVT_LEN; /* minimum message length does not include inventory data */
#ifndef HPI_A
   uns32 slot_ind = 0;
#endif

   /* collect the domain-id and session-id of HPI session */
   domain_id = hsm_cb->args->domain_id;
   session_id = hsm_cb->args->session_id;

   event.Severity = SAHPI_CRITICAL;
   event.EventType = SAHPI_ET_HOTSWAP;
   event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_INSERTION_PENDING;
   event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_INACTIVE;

   event_data = NULL;
   actual_size = 0;

   /* check if the required RDR is found */
   if (hsm_inv_data_proc(session_id, domain_id, &Rdr, RptEntry, &event_data,
                   &actual_size, &invdata_size, min_evt_len) == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("publish_inspending: Inventory data not found\n");
      if (event_data != NULL)
      {
         m_MMGR_FREE_HPI_INV_DATA(event_data);
      }
      return NCSCC_RC_FAILURE;
   }
   /* if inventory data not available, just publish event and entity path */
   if (event_data == NULL)
   {
      event_data = m_MMGR_ALLOC_HPI_INV_DATA(min_evt_len + invdata_size);
      /* invdata_size = 0; */
   }
   m_NCS_MEMCPY(event_data, (uns8 *)&event, evt_len);

#ifdef HPI_A
   /* remove the grouping elements of entity path */
   m_NCS_OS_MEMSET(&epath, 0, epath_len);
   for (i=2; i < SAHPI_MAX_ENTITY_PATH; i++)
      epath.Entry[i-2] = RptEntry->ResourceEntity.Entry[i];

   /* IR00060725: temporary fix till we migrate to HPI-B spec entity path mechanism */
   if ((RptEntry->ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) && 
       (RptEntry->ResourceEntity.Entry[0].EntityType != 160))
        epath.Entry[0].EntityType = RptEntry->ResourceEntity.Entry[0].EntityType;

   m_NCS_MEMCPY(event_data+evt_len, (uns8 *)&epath, epath_len);
   print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                  event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                  RptEntry->ResourceEntity.Entry[2].EntityInstance,
                  RptEntry->ResourceEntity.Entry[2].EntityType);
#else
   for ( i=0; i<SAHPI_MAX_ENTITY_PATH; i++ ) {
      /* Allow for the case where blades are ATCA or non-ATCA.                        */
      if ((RptEntry->ResourceEntity.Entry[i].EntityType == SAHPI_ENT_PHYSICAL_SLOT) ||
          (RptEntry->ResourceEntity.Entry[i].EntityType == SAHPI_ENT_SYSTEM_BLADE)  ||
          (RptEntry->ResourceEntity.Entry[i].EntityType == SAHPI_ENT_SWITCH_BLADE)) {
         slot_ind = i;
         break;
      }
   }

   m_NCS_OS_MEMSET(&epath, 0, epath_len);

   if( RptEntry->ResourceEntity.Entry[0].EntityType == ((SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)) ) 
     {
      epath.Entry[0] = RptEntry->ResourceEntity.Entry[1];
      slot_ind=2; 

     for ( i=1; (slot_ind + i)<SAHPI_MAX_ENTITY_PATH; i++ ) 
       {
        epath.Entry[i] = RptEntry->ResourceEntity.Entry[slot_ind + i];
        if (RptEntry->ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_ROOT)
          {
         /* don't need to copy anymore */
          break;
          }
        }
     }
   else
    {  
       if ((RptEntry->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_BLADE) ||
           (RptEntry->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SWITCH_BLADE)) {
          /* NON-ATCA: Do not strip off leaf entry. */
          slot_ind = 0;
       }
     for ( i=0; (slot_ind + i)<SAHPI_MAX_ENTITY_PATH; i++ ) 
       {
      /* For NON-ATCA, strip the SAHPI_ENT_RACK entry. */
      if (RptEntry->ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_RACK) {
         slot_ind = 1;
         i--;
      }
      else {
         epath.Entry[i] = RptEntry->ResourceEntity.Entry[slot_ind + i];
      }
        if (RptEntry->ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_ROOT)
          {
         /* don't need to copy anymore */
          break;
          }
      }
   }
   m_NCS_MEMCPY(event_data+evt_len, (uns8 *)&epath, epath_len);
   print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                  event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                  RptEntry->ResourceEntity.Entry[slot_ind].EntityLocation,
                  RptEntry->ResourceEntity.Entry[slot_ind].EntityType);
#endif

   /* Publish the consolidate event message */
   rc = hsm_eda_event_publish (HSM_FAULT_EVT, event_data, min_evt_len+invdata_size, hsm_cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DTS_CONS("publish_inspending: SaEvtEventPublish() failed.\n");
      hsm_cb->eds_init_success = 0;
      m_MMGR_FREE_HPI_INV_DATA(event_data);
      return NCSCC_RC_FAILURE;
   }
   else
   {
      m_LOG_HISV_DTS_CONS("publish_inspending: saEvtEventPublish Done\n");
   }
   m_MMGR_FREE_HPI_INV_DATA(event_data);
   return NCSCC_RC_SUCCESS;
}



/****************************************************************************
 * Name          : publish_active_healty
 *
 * Description   : This function retrieves and publishes the outstanding
 *                 Active healthy events (for SCXBs).
 *
 * Arguments     : Pointer to HSM control block structure.
 *                 Pointer to RptEntry structure;
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
publish_active_healty(HSM_CB *hsm_cb, SaHpiRptEntryT *RptEntry)
{
   SaHpiEventT     event;
   SaHpiDomainIdT domain_id;
   SaHpiSessionIdT session_id;
   SaHpiEntityPathT  epath;
   SaHpiUint32T   invdata_size=sizeof(HISV_INV_DATA);
   uns8 *event_data;
   uns32 evt_len = sizeof(SaHpiEventT), epath_len = sizeof(SaHpiEntityPathT);
   uns32 i, rc, min_evt_len = HISV_MIN_EVT_LEN; /* minimum message length does not include inventory data */
#ifndef HPI_A
   uns32 slot_ind = 0;
#endif
   /* collect the domain-id and session-id of HPI session */
   domain_id = hsm_cb->args->domain_id;
   session_id = hsm_cb->args->session_id;

   event.Severity = SAHPI_CRITICAL;
   event.EventType = SAHPI_ET_HOTSWAP;
#ifdef HPI_A
   event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_ACTIVE_HEALTHY;
#else
   event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_ACTIVE;
#endif
   event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_INSERTION_PENDING;

   event_data = NULL;

   /* if inventory data not available, just publish event and entity path */
   if (event_data == NULL)
   {
      event_data = m_MMGR_ALLOC_HPI_INV_DATA(min_evt_len + invdata_size);
      /* invdata_size = 0; */
   }

   m_NCS_MEMCPY(event_data, (uns8 *)&event, evt_len);

#ifdef HPI_A
   /* remove the grouping elements of entity path */
   m_NCS_OS_MEMSET(&epath, 0, epath_len);
   for (i=2; i < SAHPI_MAX_ENTITY_PATH; i++)
      epath.Entry[i-2] = RptEntry->ResourceEntity.Entry[i];

   /* IR00060725: temporary fix till we migrate to HPI-B spec entity path mechanism */
   if ((RptEntry->ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) && 
       (RptEntry->ResourceEntity.Entry[0].EntityType != 160))
        epath.Entry[0].EntityType = RptEntry->ResourceEntity.Entry[0].EntityType;

   m_NCS_MEMCPY(event_data+evt_len, (uns8 *)&epath, epath_len);

   print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                  event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                  RptEntry->ResourceEntity.Entry[2].EntityInstance,
                  RptEntry->ResourceEntity.Entry[2].EntityType);
#else
   for ( i=0; i<SAHPI_MAX_ENTITY_PATH; i++ ) {
      /* Allow for the case where blades are ATCA or non-ATCA.                        */
      if ((RptEntry->ResourceEntity.Entry[i].EntityType == SAHPI_ENT_PHYSICAL_SLOT) ||
          (RptEntry->ResourceEntity.Entry[i].EntityType == SAHPI_ENT_SYSTEM_BLADE)  ||
          (RptEntry->ResourceEntity.Entry[i].EntityType == SAHPI_ENT_SWITCH_BLADE)) {
         slot_ind = i;
         break;
      }
   }

   m_NCS_OS_MEMSET(&epath, 0, epath_len);

   if( RptEntry->ResourceEntity.Entry[0].EntityType == ((SaHpiEntityTypeT)(SAHPI_ENT_PHYSICAL_SLOT + 4)) )
     {
      epath.Entry[0] = RptEntry->ResourceEntity.Entry[1];
      slot_ind=2; 

     for ( i=1; (slot_ind + i)<SAHPI_MAX_ENTITY_PATH; i++ ) 
       {
        epath.Entry[i] = RptEntry->ResourceEntity.Entry[slot_ind + i];
        if (RptEntry->ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_ROOT)
          {
         /* don't need to copy anymore */
          break;
          }
        }
     }
   else
    {  
       if ((RptEntry->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SYSTEM_BLADE) ||
           (RptEntry->ResourceEntity.Entry[0].EntityType == SAHPI_ENT_SWITCH_BLADE)) {
          /* NON-ATCA: Do not strip off leaf entry. */
          slot_ind = 0;
       }
     for ( i=0; (slot_ind + i)<SAHPI_MAX_ENTITY_PATH; i++ ) 
       {
          /* For NON-ATCA, strip the SAHPI_ENT_RACK entry. */
          if (RptEntry->ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_RACK) {
             slot_ind = 1;
             i--;
          }
          else {
             epath.Entry[i] = RptEntry->ResourceEntity.Entry[slot_ind + i];
          }
        if (RptEntry->ResourceEntity.Entry[slot_ind + i].EntityType == SAHPI_ENT_ROOT)
          {
         /* don't need to copy anymore */
          break;
          }
       }
    }

   m_NCS_MEMCPY(event_data+evt_len, (uns8 *)&epath, epath_len);

   print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                  event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                  RptEntry->ResourceEntity.Entry[slot_ind].EntityLocation,
                  RptEntry->ResourceEntity.Entry[slot_ind].EntityType);
#endif

   /* Publish the consolidate event message */
   rc = hsm_eda_event_publish (HSM_FAULT_EVT, event_data, min_evt_len+invdata_size, hsm_cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DTS_CONS("publish_active_healty: SaEvtEventPublish() failed.\n");
      hsm_cb->eds_init_success = 0;
      m_MMGR_FREE_HPI_INV_DATA(event_data);
      return NCSCC_RC_FAILURE;
   }
   else
   {
      m_LOG_HISV_DTS_CONS("publish_active_healty: saEvtEventPublish Done\n");
   }
   m_MMGR_FREE_HPI_INV_DATA(event_data);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : publish_extracted
 *
 * Description   : This function retrieves and publishes the events
 *                 for missing boards.
 *
 * Arguments     : Pointer to HSM control block structure.
 *                 pointer to node state
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
publish_extracted(HSM_CB *hsm_cb, uns8 *node_state)
{
   SaHpiEventT     event;
   SaHpiDomainIdT domain_id;
   SaHpiSessionIdT session_id;
   SaHpiEntityPathT  epath;
   SaHpiUint32T   invdata_size=sizeof(HISV_INV_DATA);
   uns8 *event_data;
   uns32 evt_len = sizeof(SaHpiEventT), epath_len = sizeof(SaHpiEntityPathT);
   uns32 i, rc, min_evt_len = HISV_MIN_EVT_LEN; /* minimum message length does not include inventory data */

   m_LOG_HISV_DTS_CONS("publish_extracted: Invoked\n");

   /* collect the domain-id and session-id of HPI session */
   domain_id = hsm_cb->args->domain_id;
   session_id = hsm_cb->args->session_id;

   event.Severity = SAHPI_CRITICAL;
   event.EventType = SAHPI_ET_HOTSWAP;
   event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_NOT_PRESENT;
#ifdef HPI_A
   event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE_HEALTHY;
#else
   event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
#endif

   event_data = NULL;

   /* if inventory data not available, just publish event and entity path */
   if (event_data == NULL)
   {
      event_data = m_MMGR_ALLOC_HPI_INV_DATA(min_evt_len + invdata_size);
      /* invdata_size = 0; */
   }
   /* remove the grouping elements of entity path */
   m_NCS_OS_MEMSET(&epath, 0, epath_len);

#ifdef HPI_A
   epath.Entry[0].EntityType = SAHPI_ENT_SYSTEM_BOARD;
   epath.Entry[1].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
   epath.Entry[1].EntityInstance = hsm_cb->args->chassis_id;
#else
   epath.Entry[0].EntityType = SAHPI_ENT_PHYSICAL_SLOT;
   epath.Entry[1].EntityType = SAHPI_ENT_ADVANCEDTCA_CHASSIS;
   epath.Entry[1].EntityLocation = hsm_cb->args->chassis_id;
#endif
   epath.Entry[2].EntityType = SAHPI_ENT_ROOT;

   /* publish the outstanding extracted events */
   for (i=1; i<=MAX_NUM_SLOTS; i++)
   {
       /* Bug fix :  IR00085657 */

       /* if ((node_state[i] != NODE_HS_STATE_INACTIVE) && (node_state[i] != NODE_HS_STATE_NOT_PRESENT)
           && (node_state[i] != NODE_HS_STATE_EXTRACTION_PENDING)) */
      switch(node_state[i])
      {
         case NODE_HS_STATE_INACTIVE:
            event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_INACTIVE;
            event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_EXTRACTION_PENDING;
            break;
         case NODE_HS_STATE_EXTRACTION_PENDING:
            event.EventDataUnion.HotSwapEvent.HotSwapState = SAHPI_HS_STATE_EXTRACTION_PENDING;
#ifdef HPI_A
            event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE_HEALTHY;
#else
            event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_ACTIVE;
#endif
            break;
         case NODE_HS_STATE_NOT_PRESENT:
         case NODE_HS_STATE_INSERTION_PENDING:
         case NODE_HS_STATE_ACTIVE_HEALTHY:
         default:
            continue;

      }

      m_NCS_MEMCPY(event_data, (uns8 *)&event, evt_len);

#ifdef HPI_A
      epath.Entry[0].EntityInstance = i;
#else
      epath.Entry[0].EntityLocation = i;
#endif
      m_NCS_MEMCPY(event_data+evt_len, (uns8 *)&epath, epath_len);
      /* print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                     event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                     epath.Entry[0].EntityInstance,
                     epath.Entry[0].EntityType); */

      /* Publish the consolidate event message */
      rc = hsm_eda_event_publish (HSM_FAULT_EVT, event_data, min_evt_len+invdata_size, hsm_cb);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_HISV_DTS_CONS("publish_extracted: SaEvtEventPublish() failed.\n");
         hsm_cb->eds_init_success = 0;
      }
   }
   m_MMGR_FREE_HPI_INV_DATA(event_data);
   return NCSCC_RC_SUCCESS;
}


#if 0
/****************************************************************************
 * Name          : publish_fwprog_events
 *
 * Description   : This function retrieves and publishes the outstanding
 *                 system firmware progress events (for SCXBs).
 *
 * Arguments     : Pointer to HSM control block structure.
 *                 Pointer to RptEntry structure;
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
publish_fwprog_events(HSM_CB *hsm_cb, SaHpiRptEntryT *RptEntry)
{
   SaHpiEventT     event;
   SaHpiRdrT   Rdr;
   SaHpiDomainIdT domain_id;
   SaHpiSessionIdT session_id;
   SaHpiEntryIdT  current_rdr, next_rdr;
   SaErrorT err;
   SIM_CB *sim_cb = 0;
   SIM_EVT  *sim_evt;
   SaHpiSensorEvtEnablesT Enables;
   uns32 evt_len = sizeof(SaHpiEventT), epath_len = sizeof(SaHpiEntityPathT);
   /*
   m_NCS_CONS_PRINTF("publish_fwprog_events: Publishing outstanding fwprog events for board %d",
                      RptEntry->ResourceEntity.Entry[2].EntityInstance);
   */
   /* retrieve HSM CB */
   sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
   if (!sim_cb)
   {
      /* Can't do anything without control block.. */
      m_LOG_HISV_DTS_CONS("publish_fwprog_events: failed to take SIM handle\n");
      return NCSCC_RC_FAILURE;
   }
   /* collect the domain-id and session-id of HPI session */
   domain_id = hsm_cb->args->domain_id;
   session_id = hsm_cb->args->session_id;

   /* find the RDR which has got sensor data associated with it */
   next_rdr = SAHPI_FIRST_ENTRY;
   current_rdr = next_rdr;
   do
   {
      err = saHpiRdrGet(session_id, RptEntry->ResourceId, current_rdr,
                        &next_rdr, &Rdr);
      if (SA_OK != err)
      {
         if (current_rdr == SAHPI_FIRST_ENTRY)
            m_LOG_HISV_DTS_CONS("publish_fwprog_events: Empty RDR table\n")
         else
            m_LOG_HISV_DTS_CONS("publish_fwprog_events: saHpiRdrGet error");
         break;
      }
      if ((Rdr.RdrType == SAHPI_SENSOR_RDR) &&
          (Rdr.RdrTypeUnion.SensorRec.Type == SAHPI_SYSTEM_FW_PROGRESS))
      {
         if ((Rdr.RdrTypeUnion.SensorRec.Category != SAHPI_EC_GENERIC) &&
             (Rdr.RdrTypeUnion.SensorRec.Category != SAHPI_EC_USER))
         {
            m_LOG_HISV_DTS_CONS("Non OEM firmware progress event\n");
            current_rdr = next_rdr;
            continue;
         }
         if (saHpiSensorEventEnablesGet(session_id, RptEntry->ResourceId,
                        Rdr.RdrTypeUnion.SensorRec.Num, &Enables) != SA_OK)
         {
            m_LOG_HISV_DTS_CONS("saHpiSensorEventEnablesGet error\n");
            current_rdr = next_rdr;
            continue;
         }
         /* detect the outstanding firmware progress events; publish it to ShIM. */
         event.EventType = SAHPI_ET_SENSOR;
         event.EventDataUnion.SensorEvent.SensorType = SAHPI_SYSTEM_FW_PROGRESS;
         event.EventDataUnion.SensorEvent.EventState = (Enables.SensorStatus & 0x0f) << 1;
         /* event.EventDataUnion.SensorEvent.OptionalDataPresent = SAHPI_SOD_OEM; */
         event.EventDataUnion.SensorEvent.Oem = Rdr.RdrTypeUnion.SensorRec.Oem;

         sim_evt = (SIM_EVT *)m_MMGR_ALLOC_SIM_EVT;
         sim_evt->evt_type = HCD_SIM_FIRMWARE_PROGRESS;
         m_NCS_MEMCPY(&sim_evt->msg.fp_evt, (uns8 *)&event, evt_len);
         m_NCS_MEMCPY(&sim_evt->msg.epath, (uns8 *)&RptEntry->ResourceEntity, epath_len);

         if(m_NCS_IPC_SEND(&sim_cb->mbx, sim_evt, NCS_IPC_PRIORITY_NORMAL)
            == NCSCC_RC_FAILURE)
         {
            m_MMGR_FREE_SIM_EVT(sim_evt);
            m_LOG_HISV_DTS_CONS("publish_fwprog_events: failed to deliver msg on ShIM mail-box\n");
         }
      }
      current_rdr = next_rdr;
   }
   while (next_rdr != SAHPI_LAST_ENTRY);

   /* release the control blocks */
   ncshm_give_hdl(gl_sim_hdl);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : hsm_clear_sel
 *
 * Description   : This functions clears SEL during switchover.
 *                 Currently this is Not required.
 *
 * Arguments     : Pointers required for inventory data functionality
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
hsm_clear_sel(HSM_CB *hsm_cb)
{
   SaErrorT        err;
   SaHpiRptInfoT   rpt_info_before;
   SaHpiEntryIdT   current;
   SaHpiEntryIdT   next;

   SaHpiDomainIdT  domain_id;
   SaHpiSessionIdT session_id;
   SaHpiRptEntryT  entry;
   uns32  rc = NCSCC_RC_SUCCESS;

   m_LOG_HISV_DTS_CONS("hsm_clear_sel: clearing SEL\n");

   /* collect the domain-id and session-id of HPI session */
   domain_id = hsm_cb->args->domain_id;
   session_id = hsm_cb->args->session_id;

   /* grab copy of the update counter before traversing RPT */
   err = saHpiRptInfoGet(session_id, &rpt_info_before);
   if (SA_OK != err)
   {
      m_LOG_HISV_DTS_CONS("hsm_clear_sel: saHpiRptInfoGet\n");
      return NCSCC_RC_FAILURE;
   }
   /** process the list of resource pointer tables on this session.
    ** verifies the existence of RPT entries.
    **/
   next = SAHPI_FIRST_ENTRY;
   do
   {
      current = next;
      /* get the HPI RPT entry */
      err = saHpiRptEntryGet(session_id, current, &next, &entry);
      if (SA_OK != err)
      {
         /* error getting RPT entry */
         if (current != SAHPI_FIRST_ENTRY)
         {
            m_LOG_HISV_DTS_CONS("hsm_clear_sel: Error saHpiRptEntryGet\n");
            return NCSCC_RC_FAILURE;
         }
         else
         {
            m_LOG_HISV_DTS_CONS("hsm_clear_sel: Empty RPT\n");
            rc = NCSCC_RC_FAILURE;
            break;
         }
      }
#if 1
      /* clear the SEL if it supports; in order to avoid duplicate playback of events */
      if (entry.ResourceCapabilities & SAHPI_CAPABILITY_SEL)
      {
         saHpiEventLogClear(session_id, entry.ResourceId);
      }
#endif /* 1 */
   } while (next != SAHPI_LAST_ENTRY);
   return NCSCC_RC_SUCCESS;
}
#endif 


/****************************************************************************
 * Name          : hsm_rediscover
 *
 * Description   : This functions re-initializes the HPI session after
 *                 re-discover the resources.
 *
 * Arguments     : Pointer to HCD, HSM control block structure.
 *                 Pointer to session_id pointer;
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
hsm_rediscover(HCD_CB *hcd_cb, HSM_CB *hsm_cb, SaHpiSessionIdT *session_id)
{
   SaErrorT           val, err,session_err;
#ifdef HPI_A
   SaHpiVersionT      version;
#endif
   uns32 retry ;
   uns32 rc = NCSCC_RC_SUCCESS;

   if (!hsm_cb->args->session_valid)
   {
      /* restart the session */
      if((session_err=saHpiSessionClose(*session_id))!= SA_OK);
          m_NCS_CONS_PRINTF("hsm_rediscover:Close session return error :- %d:\n",session_err);
#ifdef HPI_A 
      saHpiFinalize();
#endif
      retry = 0;
      /* first step in openhpi */
      m_LOG_HISV_DTS_CONS("hsm_rediscover: HPI Re-Initialization...\n");

      while (1)
      {
#ifdef HPI_A     
         err = saHpiInitialize(&version);
         if (SA_OK != err)
#else
         if(hcd_cb->ha_state != SA_AMF_HA_ACTIVE)
#endif
         {
            rc = NCSCC_RC_FAILURE;
            m_NCS_TASK_SLEEP(2500);
            continue;
         }
         else
            break;
      }
      if (retry >= HPI_INIT_MAX_RETRY)
      {

         rc = NCSCC_RC_FAILURE;
         /* m_MMGR_FREE_HPI_SESSION_ARGS(dom_args); */
      }
      else
      {
         /* Every domain requires a new session */
         rc = NCSCC_RC_SUCCESS;
         m_LOG_HISV_DTS_CONS("hsm_rediscover: Re-Opening HPI Session...\n");
#ifdef HPI_A
         err = saHpiSessionOpen(SAHPI_DEFAULT_DOMAIN_ID, session_id, NULL);
#else
         err = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, session_id, NULL);
#endif
         if (SA_OK != err)
         {
          m_NCS_CONS_PRINTF("hsm_rediscover:open session return error code :- %d:\n",err);
            m_LOG_HISV_DTS_CONS("hsm_rediscover: saHpiSessionReOpen\n");
            rc = NCSCC_RC_FAILURE;
         }
         /* store the HPI session information */
         hsm_cb->args->session_id = *session_id;
#ifdef HPI_A
         hsm_cb->args->domain_id = SAHPI_DEFAULT_DOMAIN_ID;
#else
         hsm_cb->args->domain_id = SAHPI_UNSPECIFIED_DOMAIN_ID;
#endif
      }
   }
   if (rc != NCSCC_RC_FAILURE)
   {
      /* discover the HPI resources */
      if (NCSCC_RC_FAILURE == discover_domain(hsm_cb->args))
      {
         m_LOG_HISV_DTS_CONS("hisv_hcd_init: discover_domain error\n");
         rc = NCSCC_RC_FAILURE;
      }
      else
      {
         hsm_cb->args->session_valid = 1;
         hcd_cb->args->rediscover = 0;
         /* subscribe to receive events on this HPI session */
#ifdef HPI_A
         if ((val = saHpiSubscribe(*session_id, SAHPI_FALSE)) != SA_OK)
#else
         if ((val = saHpiSubscribe(*session_id)) != SA_OK)
#endif
         {
            m_LOG_HISV_DTS_CONS("hsm_rediscover: saHpiReSubscribe has failed \n");
            m_NCS_CONS_PRINTF("hsm_rediscover:saHpiSubscribe  session return error code :- %d:\n",val);

            hsm_cb->args->session_valid = 0;
            hcd_cb->args->rediscover = 1;
            rc = NCSCC_RC_FAILURE;
            /* return NCSCC_RC_FAILURE; */
         }
      }
   }
   if (rc != NCSCC_RC_FAILURE)
   {
      SIM_CB *sim_cb = 0;
      /* can dispatch outstanding hotswap events */
      hsm_cb->args->session_valid = 1;
      hcd_cb->args->rediscover = 0;
      hsm_cb->evt_time = 0;
      /* suggest HAM to re-send chassis-id */
      hsm_ham_resend_chassis_id();
      m_NCS_TASK_SLEEP(1000);
      sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
      if (sim_cb != NULL)
      {
         m_NCS_MEMSET(sim_cb->fwprog_done, 0, MAX_NUM_SLOTS);
         ncshm_give_hdl(gl_sim_hdl);
      }
      if (dispatch_hotswap(hsm_cb) != NCSCC_RC_SUCCESS)
      { 
         /* IR00085091 IR00084254 */
         hcd_cb->args->rediscover = 1;
         hsm_cb->args->session_valid = 0;
         m_LOG_HISV_DTS_CONS("hsm_rediscover: dispatch_hotswap failed\n");
         rc = NCSCC_RC_FAILURE;
         m_NCS_TASK_SLEEP(1000);
      }
      else
      {
         m_LOG_HISV_DTS_CONS("hsm_rediscover: Re-Discover Done\n");
      }
   }
   else
   {
      hcd_cb->args->rediscover = 1;
      hsm_cb->args->session_valid = 0;
      m_LOG_HISV_DTS_CONS("hsm_rediscover: Re-Discover Failed\n");
      m_NCS_TASK_SLEEP(1000);
   }
   return rc;
}



/****************************************************************************
 * Name          : hsm_inv_data_proc
 *
 * Description   : This functions processes the inventory data functionality
 *
 * Arguments     : Pointers required for inventory data functionality
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
hsm_inv_data_proc(SaHpiSessionIdT session_id, SaHpiDomainIdT domain_id,
                  SaHpiRdrT *Rdr, SaHpiRptEntryT  *RptEntry, uns8 **event_data,
                  SaHpiUint32T   *actual_size, SaHpiUint32T   *invdata_size,
                  uns32 evt_epathlen)
{
#ifdef HPI_A
   uns32 rc, j, i, k;
   uns32 retry=0;
   SaHpiEntryIdT  current_rdr, next_rdr;
   SaErrorT err;
   HISV_INV_DATA *inv_var;
   SaHpiInventoryDataT *inv_data;

CHECK:

   /* find the RDR which has got inventory data associated with it */
   next_rdr = SAHPI_FIRST_ENTRY;
   current_rdr = next_rdr;
   do
   {
      err = saHpiRdrGet(session_id, RptEntry->ResourceId, current_rdr,
                        &next_rdr, Rdr);
      if (SA_OK != err)
      {
         if (current_rdr == SAHPI_FIRST_ENTRY)
            m_LOG_HISV_DTS_CONS("hcd_hsm: Empty RDR table\n")
         else
            m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiRdrGet error\n");
         break;
      }
      if (Rdr->RdrType == SAHPI_INVENTORY_RDR)
         break;
      current_rdr = next_rdr;
   }
   while (next_rdr != SAHPI_LAST_ENTRY);

   /* check if the required RDR is found */
   if (Rdr->RdrType != SAHPI_INVENTORY_RDR)
   {
      /* discover and try again */
      saHpiResourcesDiscover(session_id);
      next_rdr = SAHPI_FIRST_ENTRY;
      do
      {
         current_rdr = next_rdr;
         err = saHpiRdrGet(session_id, RptEntry->ResourceId, current_rdr,
                           &next_rdr, Rdr);
         if (SA_OK != err)
         {
            if (current_rdr == SAHPI_FIRST_ENTRY)
               m_LOG_HISV_DTS_CONS("hcd_hsm :  Empty RDR table\n")
            else
               m_LOG_HISV_DTS_CONS("hcd_hsm :  saHpiRdrGet error\n");
            break;
         }
         if (Rdr->RdrType == SAHPI_INVENTORY_RDR)
            break;
      }
      while (next_rdr != SAHPI_LAST_ENTRY);
   }

   *invdata_size = sizeof(HISV_INV_DATA);
   *event_data = m_MMGR_ALLOC_HPI_INV_DATA(*invdata_size+evt_epathlen);
   m_NCS_MEMSET(*event_data, 0, (*invdata_size+evt_epathlen)); 


   if (Rdr->RdrType == SAHPI_INVENTORY_RDR)
   {
      m_LOG_HISV_DTS_CONS("hcd_hsm: Retrieving inventory data...\n");
      /* read the associated inventory data of resource */
#if 0
      /* getting actual size is not supported in HPI 0.1 */
      rc = saHpiEntityInventoryDataRead(session_id, RptEntry->ResourceId,
                                        Rdr->RdrTypeUnion.InventoryRec.EirId,
                                        0,
                                        (SaHpiInventoryDataT *)NULL,
                                        actual_size);
#endif
      *actual_size = HISV_MAX_INV_DATA_SIZE;
      inv_var = &((HPI_HISV_EVT_T *)*event_data)->inv_data;
      inv_data = (SaHpiInventoryDataT *)(m_MMGR_ALLOC_HPI_INV_DATA(*actual_size));

      /* read the associated inventory data of resource */
      rc = saHpiEntityInventoryDataRead(session_id, RptEntry->ResourceId,
                                        Rdr->RdrTypeUnion.InventoryRec.EirId,
                                        *actual_size,
                                        (SaHpiInventoryDataT *)inv_data,
                                        actual_size);

      /* look for SAHPI_INVENT_RECTYPE_PRODUCT_INFO */
      k = 0;
      if (rc == SA_OK)
      while (inv_data->DataRecords[k] != NULL)
      {
         if (inv_data->DataRecords[k]->RecordType == SAHPI_INVENT_RECTYPE_PRODUCT_INFO) 
            break;
         k++;
      }

      if ((rc != SA_OK) || (inv_data->DataRecords[k] == NULL))
      {
         m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiEntityInventoryDataRead() failed.\n");
         m_LOG_HISV_DTS_CONS("hcd_hsm: Product Info in the FRU is not configured at right offset.\n");
         /*RDR wasn't updated , try once more retreiving the Rdr */
         if(retry <= 1)
         { 
            retry ++;
            m_MMGR_FREE_HPI_INV_DATA(*event_data);
            *actual_size = 0;
            *invdata_size = 0;
            m_NCS_OS_MEMSET(Rdr, 0, sizeof(SaHpiRdrT));
            goto CHECK;
         }
#if 0
         m_MMGR_FREE_HPI_INV_DATA(*event_data);
         *event_data = NULL;
         *actual_size = 0;
         *invdata_size = 0;
#endif /* 0 */
      }
      else
      {
         MAC_OEM_REC_TYPE mac_rec_type;
         /* copy the data to HPL inventory structure, to publish it */
         /* fill product name */
         inv_var->product_name.DataLength = 0;
         if (inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName != NULL)
         {
            SaHpiTextTypeT data_type;
            data_type = inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->DataType;
            /* if DataLength look unsually long, try re-compute it */
            if (inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->DataLength > HISV_MAX_INV_STR_LEN)
               inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->DataLength = 
                  m_NCS_STRLEN(inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->Data);
            if (data_type != SAHPI_TL_TYPE_LANGUAGE)
            {
               int ret_len;
               inv_var->product_name.DataLength = 0;
               ret_len = hpi_decode_to_ascii(data_type,
                         inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->Data,
                         inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->DataLength,
                         inv_var->product_name.Data);
               if (ret_len == -1)
               {
                  m_LOG_HISV_DTS_CONS("Unknown encoding for Product Name may result in FRU Invalidation\n");
               }
               else
                  inv_var->product_name.DataLength = ret_len;
            }
            else
            {
               inv_var->product_name.DataLength = inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->DataLength;

                m_NCS_MEMCPY(inv_var->product_name.Data,
                             inv_data->DataRecords[k]->RecordData.ProductInfo.ProductName->Data,
                             inv_var->product_name.DataLength);
                inv_var->product_name.Data[inv_var->product_name.DataLength] = '\0';
            }
         }

         /* fill product version */
         inv_var->product_version.DataLength = 0;
         if (inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion != NULL)
         {
            SaHpiTextTypeT data_type;
            /* if DataLength look unsually long, try re-compute it */
            if (inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->DataLength > HISV_MAX_INV_STR_LEN)
               inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->DataLength = 
                  m_NCS_STRLEN(inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->Data);

            data_type = inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->DataType;
            if (data_type != SAHPI_TL_TYPE_LANGUAGE)
            {
               int ret_len;
               inv_var->product_version.DataLength = 0;
               ret_len = hpi_decode_to_ascii(data_type,
                         inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->Data,
                         inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->DataLength,
                         inv_var->product_version.Data);
               if (ret_len == -1)
               {
                  m_LOG_HISV_DTS_CONS("Unknown encoding for Product Version may result in FRU Invalidation\n");
               }
               else
                  inv_var->product_version.DataLength = ret_len;
            }
            else
            {
               inv_var->product_version.DataLength = inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->DataLength;

               m_NCS_MEMCPY(inv_var->product_version.Data,
                            inv_data->DataRecords[k]->RecordData.ProductInfo.ProductVersion->Data,
                            inv_var->product_version.DataLength);
               inv_var->product_version.Data[inv_var->product_version.DataLength] = '\0';
            }
         }
#if 0
         /* fill part number */
         inv_var->part_number.DataLength = 0;
         if (inv_data->DataRecords[k]->RecordData.ProductInfo.PartNumber != NULL)
         {
            /* if DataLength look unsually long, try re-compute it */
            if (inv_data->DataRecords[k]->RecordData.ProductInfo.PartNumber->DataLegth > HISV_MAX_INV_STR_LEN)
               inv_data->DataRecords[k]->RecordData.ProductInfo.PartNumber->DataLength = 
                  m_NCS_STRLEN(inv_data->DataRecords[k]->RecordData.ProductInfo.PartNumber->Data);
            inv_var->part_number.DataLength = inv_data->DataRecords[k]->RecordData.ProductInfo.PartNumber->DataLength;

            m_NCS_MEMCPY(inv_var->part_number.Data,
                         inv_data->DataRecords[k]->RecordData.ProductInfo.PartNumber->Data,
                         inv_var->part_number.DataLength);
            inv_var->part_number.Data[inv_var->part_number.DataLength] = '\0';
         }
#endif /* 0 */

         /* look for OEM Mac Data */
         k = 0;
         mac_rec_type = MAC_OEM_UNKNOWN_TYPE;
         while (inv_data->DataRecords[k] != NULL)
         {
            if (inv_data->DataRecords[k]->RecordType == SAHPI_INVENT_RECTYPE_OEM)
            {
               /* check if it is a OpenSAF specific MAC OEM Record */
               if (((inv_data->DataRecords[k]->RecordData.OemData.MId == HISV_MAC_ADDR_MOT_OEM_MID)|| 
                   (inv_data->DataRecords[k]->RecordData.OemData.MId == HISV_MAC_ADDR_EMERSON_OEM_MID)) &&
                   (inv_data->DataRecords[k]->RecordData.OemData.Data[0] == HISV_MAC_ADDR_MOT_OEM_REC_ID))
               {
                  mac_rec_type = MAC_OEM_MOT_TYPE;
                  break;
               }
               /* check if it is a Force specific MAC OEM Record */
               if ((inv_data->DataRecords[k]->RecordData.OemData.MId == HISV_MAC_ADDR_FORCE_OEM_MID) &&
                   (inv_data->DataRecords[k]->RecordData.OemData.Data[0] == HISV_MAC_ADDR_FORCE_OEM_REC_ID))
               {
                  mac_rec_type = MAC_OEM_FORCE_TYPE;
                  break;
               }
            }
            k++;
         }

         /* if we have got OEM MAC address data, it should be at least 13 bytes */
         if (inv_data->DataRecords[k] == NULL)
         {
            m_LOG_HISV_DTS_CONS("Mac address data is missing in OEM record , It is Null \n");
            inv_var->oem_inv_data.num_mac_entries = 0;
         }
         else
         if (inv_data->DataRecords[k]->DataLength < 13)
         {
            m_LOG_HISV_DTS_CONS("Mac address data is missing in OEM record\n");
            inv_var->oem_inv_data.num_mac_entries = 0;
         }
         else
         {
#if 0
            if (inv_data->DataRecords[k]->DataLength !=
               ((inv_data->DataRecords[k]->RecordData.OemData.Data[2] * 7) + 7))
               m_LOG_HISV_DTS_CONS("Mismatch in Mac data-len and data in OEM record\n");
#endif /* 0 */
            inv_var->oem_inv_data.type = SAHPI_INVENT_RECTYPE_OEM;
            inv_var->oem_inv_data.mId = inv_data->DataRecords[k]->RecordData.OemData.MId;
            inv_var->oem_inv_data.mot_oem_rec_id = inv_data->DataRecords[k]->RecordData.OemData.Data[0];
            inv_var->oem_inv_data.rec_format_ver = inv_data->DataRecords[k]->RecordData.OemData.Data[1];
            inv_var->oem_inv_data.num_mac_entries = inv_data->DataRecords[k]->RecordData.OemData.Data[2];
            /* copy the MAC address entries */
            j = 0;
            /* force record type has two formats. Retrieve MAC addresses based on format */
            if (mac_rec_type == MAC_OEM_FORCE_TYPE)
            {
               if (inv_data->DataRecords[k]->DataLength <= 13)
               {
                  if (inv_var->oem_inv_data.num_mac_entries == 0)
                  {
                     m_LOG_HISV_DTS_CONS("Mac address data is missing in Force OEM record\n");
                  }
                  for (i=0; i < inv_var->oem_inv_data.num_mac_entries; i++)
                  {
                     m_NCS_MEMCPY(inv_var->oem_inv_data.interface_mac_addr[j++],
                                   &inv_data->DataRecords[k]->RecordData.OemData.Data[2 + (0*MAC_DATA_LEN) + 1],
                                   MAC_DATA_LEN);
                     inv_var->oem_inv_data.interface_mac_addr[j-1][5] = inv_var->oem_inv_data.interface_mac_addr[0][5] + i; 
                     if (j >= MAX_MAC_ENTRIES) break;
                  }
               }
               else 
               {
                  inv_var->oem_inv_data.num_mac_entries = (inv_data->DataRecords[k]->DataLength - 5) / MAC_DATA_LEN;
                  for (i=0; i < inv_var->oem_inv_data.num_mac_entries; i++)
                  {
                     m_NCS_MEMCPY(inv_var->oem_inv_data.interface_mac_addr[j++],
                                   &inv_data->DataRecords[k]->RecordData.OemData.Data[1 + (i*MAC_DATA_LEN) + 1],
                                   MAC_DATA_LEN);
                     if (j >= MAX_MAC_ENTRIES) break;
                  }
               }
            }
            else
            {
               for (i=0; i < inv_var->oem_inv_data.num_mac_entries; i++)
               {
                  /* copy base Mac Addresses only */
                  if (inv_data->DataRecords[k]->RecordData.OemData.Data[2 + (i*MAC_DATA_LEN) + (i+1)] == 0x01)
                     m_NCS_MEMCPY(inv_var->oem_inv_data.interface_mac_addr[j++],
                                   &inv_data->DataRecords[k]->RecordData.OemData.Data[2 + (i*MAC_DATA_LEN) + (i+2)],
                                   MAC_DATA_LEN);
                  if (j >= MAX_MAC_ENTRIES) break;
               }
            }
            inv_var->oem_inv_data.num_mac_entries = j;
         }
         print_invdata(inv_var);
      }
      m_MMGR_FREE_HPI_INV_DATA(inv_data);
   }
   else
   {
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
#else
   SaHpiEntryIdT current_rdr, next_rdr;
   SaHpiEntryIdT  next_area,areaId;
   SaHpiEntryIdT  next_field , fieldId;
   SaHpiIdrInfoT idrInfo;
   SaHpiIdrIdT idrId;
   SaHpiIdrAreaHeaderT areaInfo ;
   SaHpiManufacturerIdT  MId;
   SaHpiIdrFieldTypeT   fieldType;
   SaHpiIdrFieldT product_name, product_version, addr_field;
   SaErrorT err,rv_field;
   HISV_INV_DATA *inv_var;
   SaHpiUint32T numFields = 0;
   MAC_OEM_REC_TYPE mac_rec_type;
   uns32 j, i;
   int rediscover_flag;

   /* find the RDR which has got inventory data associated with it */
   current_rdr = next_rdr = SAHPI_FIRST_ENTRY;
   rediscover_flag = 1;
   do
   {
      err = saHpiRdrGet(session_id, RptEntry->ResourceId, current_rdr,
                        &next_rdr, Rdr);
      if (SA_OK != err)
      {
         if (current_rdr == SAHPI_FIRST_ENTRY)
         {
            m_LOG_HISV_DTS_CONS("hcd_hsm: Empty RDR table\n");
         }
         else
         {
            m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiRdrGet error\n");
         }

         if ( rediscover_flag == 1 )
         {
            saHpiDiscover(session_id);
            current_rdr = next_rdr = SAHPI_FIRST_ENTRY;
            rediscover_flag = 0;
         }
      }
      else
      {
         if (Rdr->RdrType == SAHPI_INVENTORY_RDR)
         {
            /* found RDR Inventory Data, break out of the loop */
            break;
         }
         current_rdr = next_rdr;
      }
   }
   while (next_rdr != SAHPI_LAST_ENTRY);

   /* event_data is allocated here, it is freed by calling function */
   *invdata_size = sizeof(HISV_INV_DATA);
   *event_data = m_MMGR_ALLOC_HPI_INV_DATA(*invdata_size+evt_epathlen);
   inv_var = &((HPI_HISV_EVT_T *)*event_data)->inv_data;
   m_NCS_MEMSET(*event_data, 0, (*invdata_size+evt_epathlen));

   if (Rdr->RdrType != SAHPI_INVENTORY_RDR) {
      m_LOG_HISV_DTS_CONS("hcd_hsm: Could not find RDR record\n");
      return(NCSCC_RC_FAILURE);
   }

   /* get the IDR header information */
   idrId = Rdr->RdrTypeUnion.InventoryRec.IdrId;
   err = saHpiIdrInfoGet(session_id, RptEntry->ResourceId, idrId, &idrInfo);
   if ( err != SA_OK )
   {
      m_LOG_HISV_DTS_CONS("hcd_hsm: call to saHpiIdrInfoGet failed\n");
      return(NCSCC_RC_FAILURE);
   }

   /* get the IDR Area info for the PRODUCT_INFO area */
   err = saHpiIdrAreaHeaderGet(session_id,
                               RptEntry->ResourceId,
                               idrId,
                               SAHPI_IDR_AREATYPE_PRODUCT_INFO,
                               SAHPI_FIRST_ENTRY,
                               &next_area,
                               &areaInfo);

   if ( err != SA_OK )
   {
      m_LOG_HISV_DTS_CONS("hcd_hsm: call to saHpiIdrAreaHeaderGet failed\n");
      return(NCSCC_RC_FAILURE);
   }

   err = saHpiIdrFieldGet(session_id,
                          RptEntry->ResourceId,
                          idrId,
                          areaInfo.AreaId,
                          SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
                          SAHPI_FIRST_ENTRY,
                          &next_field,
                          &product_name);

   if ( err != SA_OK )
   {
      m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiIdrFieldGet failed to get product name\n");
   }
   else
   {
      /* copy product_name into event structure */
      if (product_name.Field.DataLength  > HISV_MAX_INV_STR_LEN)
         product_name.Field.DataLength = m_NCS_STRLEN(product_name.Field.Data);

      inv_var->product_name.DataLength = product_name.Field.DataLength;
      m_NCS_MEMCPY(inv_var->product_name.Data, product_name.Field.Data, inv_var->product_name.DataLength);
   }

   err = saHpiIdrFieldGet(session_id,
                          RptEntry->ResourceId,
                          idrId,
                          areaInfo.AreaId,
                          SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION,
                          SAHPI_FIRST_ENTRY,
                          &next_field,
                          &product_version);

   if ( err != SA_OK )
   {
      m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiIdrFieldGet failed to get product version\n");
   }
   else
   {
      /* copy product_version into event structure */
      if (product_version.Field.DataLength  > HISV_MAX_INV_STR_LEN)
         product_version.Field.DataLength = m_NCS_STRLEN(product_version.Field.Data);

      inv_var->product_version.DataLength = product_version.Field.DataLength;
      m_NCS_MEMCPY(inv_var->product_version.Data, product_version.Field.Data, inv_var->product_version.DataLength);
   }

   areaId = SAHPI_FIRST_ENTRY;
   m_NCS_MEMSET(&areaInfo, 0, sizeof(SaHpiIdrAreaHeaderT));
   err=SA_OK;

   while ((err == SA_OK) && (areaId != SAHPI_LAST_ENTRY))
   {
      err = saHpiIdrAreaHeaderGet(session_id,
                                  RptEntry->ResourceId,
                                  idrId,
                                  SAHPI_IDR_AREATYPE_OEM,
                                  areaId,
                                  &next_area,
                                  &areaInfo);

    if(SA_OK == err)
    {
      numFields=areaInfo.NumFields;

      fieldType = SAHPI_IDR_FIELDTYPE_UNSPECIFIED;
      fieldId=SAHPI_FIRST_ENTRY;
      rv_field=SA_OK;
     
         
              while ((rv_field == SA_OK) && (fieldId != SAHPI_LAST_ENTRY))
                {
                        rv_field = saHpiIdrFieldGet( session_id,
                                                    RptEntry->ResourceId,
                                                    idrId,
                                                    areaInfo.AreaId,
                                                    fieldType,
                                                    fieldId,
                                                    &next_field,
                                                    &addr_field);
                      if(rv_field == SA_OK )
                       {
        
                          if(1 == addr_field.FieldId) /* Manufactur ID field */
                                 MId = addr_field.Field.Data[2] | (addr_field.Field.Data[1] << 8) | (addr_field.Field.Data[0] << 16);

                         if(2 == addr_field.FieldId ) /*  Record ID field*/
                           {
                             if(( HISV_MAC_ADDR_MOT_OEM_REC_ID == addr_field.Field.Data[0]) && 
                                 ( (MId ==HISV_MAC_ADDR_EMERSON_OEM_MID  ) || ( MId == HISV_MAC_ADDR_MOT_OEM_MID)))
                              {
                                       mac_rec_type=MAC_OEM_MOT_TYPE;
                                  inv_var->oem_inv_data.type=SAHPI_IDR_AREATYPE_OEM;                           
                                  inv_var->oem_inv_data.mId = MId;
                                  inv_var->oem_inv_data.mot_oem_rec_id = addr_field.Field.Data[0];
                                  inv_var->oem_inv_data.rec_format_ver = addr_field.Field.Data[1];
                                  inv_var->oem_inv_data.num_mac_entries = addr_field.Field.Data[2];
                               
                              }
                              else
                               if((HISV_MAC_ADDR_FORCE_OEM_REC_ID == addr_field.Field.Data[0]) &&( MId == HISV_MAC_ADDR_FORCE_OEM_MID))
                               {
                                      mac_rec_type=MAC_OEM_FORCE_TYPE;
                                  inv_var->oem_inv_data.type=SAHPI_IDR_AREATYPE_OEM;
                                  inv_var->oem_inv_data.mId = MId;
                                  inv_var->oem_inv_data.mot_oem_rec_id = addr_field.Field.Data[0];
                                  inv_var->oem_inv_data.rec_format_ver = addr_field.Field.Data[1];
                               }
                            }
                         
                            if( MAC_OEM_FORCE_TYPE == mac_rec_type)
                             {
                               if(numFields > 5  )
                               {
                                 /* 6 Mac Address specified , First two are Base address  */
                                   if(4 == addr_field.FieldId)
                                     {
                                      memcpy(inv_var->oem_inv_data.interface_mac_addr[0],&addr_field.Field.Data,MAC_DATA_LEN);
                                      inv_var->oem_inv_data.num_mac_entries = 1;
                                     }
                                   if(5 == addr_field.FieldId)
                                    {
                                      memcpy(inv_var->oem_inv_data.interface_mac_addr[1],&addr_field.Field.Data,MAC_DATA_LEN);
                                      inv_var->oem_inv_data.num_mac_entries ++;
                                    }
                                   
                                }
                                else
                                {
                                  /* Only one base MAC address */
                                   if(5 == addr_field.FieldId)
                                   {
                                     inv_var->oem_inv_data.num_mac_entries = 1;
                                       memcpy(inv_var->oem_inv_data.interface_mac_addr[0],&addr_field.Field.Data,MAC_DATA_LEN);
                                   }

                                }
                             }
                             else if ( MAC_OEM_MOT_TYPE == mac_rec_type)
                             {
                               j=0;
                                for(i=0 ; i<inv_var->oem_inv_data.num_mac_entries;i++ )
                               {
                                 if( addr_field.Field.Data[2+ (i*MAC_DATA_LEN) + (i+1) ] == 0x01 ) /* Pick only base address */
                                   memcpy(inv_var->oem_inv_data.interface_mac_addr[j++],&addr_field.Field.Data[2 + (i*MAC_DATA_LEN) + (i+2)],MAC_DATA_LEN );

                                 if(j >= MAX_MAC_ENTRIES) break;
                                }
                                 inv_var->oem_inv_data.num_mac_entries=j;
 
                             }
                       }
                       else
                       {
                          m_LOG_HISV_DTS_CONS("hcd_hsm: saHpiIdrFieldGet failed to get OEM Data\n");
                       }

                   fieldId=next_field;
               }/*End Fields */

    }
    else
    {
      m_LOG_HISV_DTS_CONS("hcd_hsm: call to saHpiIdrAreaHeaderGet failed\n");
      return(NCSCC_RC_FAILURE);
        /* Log an err Area header Get issue */
    }
   areaId = next_area; 
   } /* while areas */
   

   print_invdata(inv_var);
   return NCSCC_RC_SUCCESS;
#endif
}

/****************************************************************************
 * Name          : hsm_ham_resend_chassis_id
 *
 * Description   : Suggest HAM to resend chassisid
 *
 * Arguments     : pointer to struct ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
hsm_ham_resend_chassis_id()
{
   HISV_EVT * hisv_evt;
   HAM_CB *ham_cb;

   /* allocate the event */
   if (NULL == (hisv_evt = m_MMGR_ALLOC_HISV_EVT))
   {
      m_LOG_HISV_DTS_CONS("hsm_ham_resend_chassis_id: memory error\n");
      return NCSCC_RC_FAILURE;
   }
   if (NULL == (ham_cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_ham_hdl)))
   {
      m_LOG_HISV_DTS_CONS("hsm_ham_resend_chassis_id: Failed to get ham_cb handle\n");
      hisv_evt_destroy(hisv_evt);
      return NCSCC_RC_FAILURE;
   }

   /* populate the message with command to resend the chassis-id */
   m_HAM_HISV_LOG_CMD_MSG_FILL(hisv_evt->msg, HISV_RESEND_CHASSIS_ID);

   /* send the request to HAM mailbox */
   if(m_NCS_IPC_SEND(&ham_cb->mbx, hisv_evt, NCS_IPC_PRIORITY_NORMAL) == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DTS_CONS("hsm_ham_resend_chassis_id: failed to deliver msg on mail-box\n");
      hisv_evt_destroy(hisv_evt);
      ncshm_give_hdl(gl_ham_hdl);
      return NCSCC_RC_FAILURE;
   }
   ncshm_give_hdl(gl_ham_hdl);
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : publish_curr_hs_state_evt
 *
 * Description   : This function retrieves and publishes the outstanding
 *                 hotswap events before OEM restored event.
 *
 * Arguments     : Pointer to HSM control block structure.
 *                 Pointer to RptEntry structure;
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
publish_curr_hs_state_evt(HSM_CB *hsm_cb, SaHpiRptEntryT *entry)
{
   SaHpiEventT     event;
   SaErrorT        hpi_rc, hotswap_err;
   SaHpiHsStateT   state;
   SaHpiDomainIdT domain_id;
   SaHpiSessionIdT session_id;
   SaHpiEntityPathT  epath;
   SaHpiUint32T   invdata_size=sizeof(HISV_INV_DATA);
   uns8 *event_data;
   uns32 evt_len = sizeof(SaHpiEventT), epath_len = sizeof(SaHpiEntityPathT);
   uns32 i, rc, min_evt_len = HISV_MIN_EVT_LEN; /* minimum message length does not include inventory data */

   m_NCS_CONS_PRINTF("publish_curr_hs_state_evt ()\n");

   /* collect the domain-id and session-id of HPI session */
   domain_id = hsm_cb->args->domain_id;
   session_id = hsm_cb->args->session_id;

   /* if it is a FRU and hotswap managable then publish outstanding inspending events */
   if ((entry->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP) && 
       (entry->ResourceCapabilities & SAHPI_CAPABILITY_FRU))
   {
/*  Nitin : Why it is required here */
#ifdef HPI_A
      if (saHpiHotSwapControlRequest(session_id, entry->ResourceId) != SA_OK)
#else
      hotswap_err = saHpiHotSwapPolicyCancel(session_id, entry->ResourceId);

      if (hotswap_err != SA_OK)
#endif
      {
         if (hotswap_err == SA_ERR_HPI_INVALID_REQUEST)
         {
            /* Allow for the case where the hotswap policy cannot be canceled because it */
            /* has already begun to execute.                                             */
            m_LOG_HISV_DTS_CONS("publish_curr_hs_state_evt: saHpiHotSwapPolicyCancel cannot cancel hotswap policy\n");
         }
         else
         {
         m_LOG_HISV_DTS_CONS("publish_curr_hs_state_evt: Error taking control of resource \n");
         }
      }
      hpi_rc = saHpiHotSwapStateGet(session_id, entry->ResourceId, &state);
      if (hpi_rc != SA_OK)
      {
         m_LOG_HISV_DTS_CONS("failed to get hotswap state here\n");
         m_NCS_CONS_PRINTF("HPI return code=%d\n", hpi_rc);

      }
      else
      {
         /* fill hotswap states */
#ifdef HPI_A
         if ((entry->ResourceEntity.Entry[2].EntityInstance > 0) && 
             (entry->ResourceEntity.Entry[2].EntityInstance <= MAX_NUM_SLOTS) &&
             (entry->ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) &&
             (entry->ResourceEntity.Entry[0].EntityType == 160))
         {
#endif
            /* collect the domain-id and session-id of HPI session */
            domain_id = hsm_cb->args->domain_id;
            session_id = hsm_cb->args->session_id;

            event.Severity = SAHPI_CRITICAL;
            event.EventType = SAHPI_ET_HOTSWAP;
            event.EventDataUnion.HotSwapEvent.HotSwapState = state;
            event.EventDataUnion.HotSwapEvent.PreviousHotSwapState = SAHPI_HS_STATE_NOT_PRESENT;

            /* Change the state of the resourece in the local list */
            switch (state)
            {
#ifdef HPI_A
              case SAHPI_HS_STATE_INACTIVE:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_INACTIVE;
                 break;
              case SAHPI_HS_STATE_INSERTION_PENDING:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_INSERTION_PENDING;
                 break;
              case SAHPI_HS_STATE_ACTIVE_HEALTHY:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_ACTIVE_HEALTHY;
                 break;
              case SAHPI_HS_STATE_ACTIVE_UNHEALTHY:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_ACTIVE_UNHEALTHY;
                 break;
              case SAHPI_HS_STATE_EXTRACTION_PENDING:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_EXTRACTION_PENDING;
                 break;
              case SAHPI_HS_STATE_NOT_PRESENT:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[2].EntityInstance] = NODE_HS_STATE_NOT_PRESENT;
                 break;
#else
              case SAHPI_HS_STATE_INACTIVE:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_INACTIVE;
                 break;
              case SAHPI_HS_STATE_INSERTION_PENDING:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_INSERTION_PENDING;
                 break;
              case SAHPI_HS_STATE_ACTIVE:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_ACTIVE_HEALTHY;
                 break;
              case SAHPI_HS_STATE_EXTRACTION_PENDING:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_EXTRACTION_PENDING;
                 break;
              case SAHPI_HS_STATE_NOT_PRESENT:
                 hsm_cb->node_state[entry->ResourceEntity.Entry[1].EntityLocation] = NODE_HS_STATE_NOT_PRESENT;
                 break;
#endif
            }

            event_data = NULL;

            /* if inventory data not available, just publish event and entity path */
            if (event_data == NULL)
            {
               event_data = m_MMGR_ALLOC_HPI_INV_DATA(min_evt_len + invdata_size);
               /* invdata_size = 0; */
            }
            m_NCS_MEMCPY(event_data, (uns8 *)&event, evt_len);

            /* remove the grouping elements of entity path */
            m_NCS_OS_MEMSET(&epath, 0, epath_len);
#ifdef HPI_A
            for (i=2; i < SAHPI_MAX_ENTITY_PATH; i++)
               epath.Entry[i-2] = entry->ResourceEntity.Entry[i];

            /* IR00060725: temporary fix till we migrate to HPI-B spec entity path mechanism */
            if ((entry->ResourceEntity.Entry[2].EntityType == SAHPI_ENT_SYSTEM_BOARD) && 
               (entry->ResourceEntity.Entry[0].EntityType != 160))
               epath.Entry[0].EntityType = entry->ResourceEntity.Entry[0].EntityType;
#else
            for (i=1; i < SAHPI_MAX_ENTITY_PATH; i++)
            {
               epath.Entry[i-1] = entry->ResourceEntity.Entry[i];
               if ( entry->ResourceEntity.Entry[i].EntityType == SAHPI_ENT_ROOT)
                  break;
            }
#endif

            m_NCS_MEMCPY(event_data+evt_len, (uns8 *)&epath, epath_len);

#ifdef HPI_A
            print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                      event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                      entry->ResourceEntity.Entry[2].EntityInstance,
                      entry->ResourceEntity.Entry[2].EntityType);
#else
            print_hotswap (event.EventDataUnion.HotSwapEvent.HotSwapState,
                      event.EventDataUnion.HotSwapEvent.PreviousHotSwapState,
                      entry->ResourceEntity.Entry[1].EntityLocation,
                      entry->ResourceEntity.Entry[1].EntityType);
#endif

            /* Publish the consolidate event message */
            rc = hsm_eda_event_publish (HSM_FAULT_EVT, event_data, min_evt_len+invdata_size, hsm_cb);
            if (rc != NCSCC_RC_SUCCESS)
            {
               m_LOG_HISV_DTS_CONS("publish_curr_hs_state_evt: SaEvtEventPublish() failed.\n");
               hsm_cb->eds_init_success = 0;
               m_MMGR_FREE_HPI_INV_DATA(event_data);
               return NCSCC_RC_FAILURE;
            }
            else
            {
               m_LOG_HISV_DTS_CONS("publish_curr_hs_state_evt: saEvtEventPublish Done\n");
            }
            m_MMGR_FREE_HPI_INV_DATA(event_data);
            return NCSCC_RC_SUCCESS;
#ifdef HPI_A
        } /* End ifdef A_01 */
#endif
      } /* End else */
   }
   return NCSCC_RC_SUCCESS;
}
