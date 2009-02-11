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
*  MODULE NAME:  hsm_init.c                                                  *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality for initializing HPI Session       *
*  Manager. It initializes the HSM control block and event distribution      *
*  channels used to publish events on EDSv. Also contains the 'finalizing'   *
*  routines used to finalize HSM, destroy control block and un-register the  *
*  the EDA channels.                                                         *
*                                                                            *
*****************************************************************************/

#include "hcd.h"
#include "hpl_msg.h"

/* global cb handle */
uns32 gl_hsm_hdl = 0;

/* local function declarations */
static void hsm_chan_open_callback(SaInvocationT inv, SaEvtChannelHandleT chan_hdl,
                                   SaAisErrorT rc);
static void
hsm_event_callback(SaEvtSubscriptionIdT sub_id, SaEvtEventHandleT event_hdl,
                   const SaSizeT event_data_size);

extern uns32 hsm_encode_hisv_evt (HPI_HISV_EVT_T *evt_data, uns8 **evt_publish, uns32 version);

/* pattern array set for publishing fault events */
static SaEvtEventPatternT hpi_fault_pattern_array[HPI_FAULT_PATTERN_ARRAY_LEN] = {
   {SAHPI_CRITICAL_PATTERN_LEN, SAHPI_CRITICAL_PATTERN_LEN, (SaUint8T *) SAHPI_CRITICAL_PATTERN},
   {SAHPI_MAJOR_PATTERN_LEN, SAHPI_MAJOR_PATTERN_LEN, (SaUint8T *) SAHPI_MAJOR_PATTERN}
};

/* pattern array set for publishing non fault events */
static SaEvtEventPatternT hpi_non_fault_pattern_array[HPI_NON_FAULT_PATTERN_ARRAY_LEN] = {
   {SAHPI_MINOR_PATTERN_LEN, SAHPI_MINOR_PATTERN_LEN, (SaUint8T *) SAHPI_MINOR_PATTERN},
   {SAHPI_INFORMATIONAL_PATTERN_LEN, SAHPI_INFORMATIONAL_PATTERN_LEN, (SaUint8T *) SAHPI_INFORMATIONAL_PATTERN},
   {SAHPI_OK_PATTERN_LEN, SAHPI_OK_PATTERN_LEN, (SaUint8T *) SAHPI_OK_PATTERN},
   {SAHPI_DEBUG_PATTERN_LEN, SAHPI_DEBUG_PATTERN_LEN, (SaUint8T *) SAHPI_DEBUG_PATTERN}
};


/****************************************************************************
 * Name          : hsm_initialize
 *
 * Description   : This function initializes the HSM control block and creates
 *                 HSM control block handle. Also initializes the EDA channel
 *                 and creates the event handles to publish fault, non-fault
 *                 events and inventory data to the EDSv. It creates different
 *                 pattern sets to distinguish fault and non-fault events
 *                 published to EDSv
 *
 * Arguments     : pointer HPI_SESSION_ARGS which holds session-id, domain-id
 *                 of the HPI session.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hsm_initialize(HPI_SESSION_ARGS *args)
{
   HSM_CB *hsm_cb = NULL;
   uns32  rc = NCSCC_RC_SUCCESS;

   /* allocate HSM cb */
   if ( NULL == (hsm_cb = m_MMGR_ALLOC_HSM_CB))
   {
      /* free HPI session arguments */
      m_MMGR_FREE_HPI_SESSION_ARGS(args);
      /* reset the global cb handle */
      gl_hsm_hdl = 0;
      return NCSCC_RC_FAILURE;
   }
   m_NCS_OS_MEMSET(hsm_cb, 0, sizeof(HSM_CB));

   /* assign the HSM pool-id (used by hdl-mngr) */
   hsm_cb->pool_id = NCS_HM_POOL_ID_COMMON;
   /* initialize the session arguments */
   hsm_cb->args = args;
   /* init event timestamp */
   hsm_cb->evt_time = 0;
   hsm_cb->eds_init_success = 0;

   /* create the association with hdl-mngr */
   if ( 0 == (hsm_cb->cb_hdl = ncshm_create_hdl(hsm_cb->pool_id, NCS_SERVICE_ID_HCD,
                                        (NCSCONTEXT)hsm_cb)) )
   {
      /* log */
      m_LOG_HISV_DEBUG("Error create hsm_cb handle\n");
      hsm_cb->cb_hdl = 0;
      rc = NCSCC_RC_FAILURE;
      goto error;
   }
   /* store the cb hdl in the global variable */
   gl_hsm_hdl = hsm_cb->cb_hdl;

   /* get the process id */
   hsm_cb->prc_id = m_NCS_OS_PROCESS_GET_ID();

   /* Initialize the EDA channels used for publishing events */
   if ( (NCSCC_RC_SUCCESS != hsm_eda_chan_initialize(hsm_cb)))
   {
      /* log */
      m_LOG_HISV_DEBUG("No EDS. Failed to initialize EDA channel\n");
      rc = NCSCC_RC_FAILURE;
      hsm_cb->eds_init_success = 0;
   /*   goto error; */
   }
   else
      hsm_cb->eds_init_success = 1;
   return NCSCC_RC_SUCCESS;

error:
   if (hsm_cb != NULL)
   {
      /* remove the association with hdl-mngr */
      if (hsm_cb->cb_hdl != 0)
         ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, hsm_cb->cb_hdl);

      /* free HPI session arguments */
      m_MMGR_FREE_HPI_SESSION_ARGS(hsm_cb->args);

      /* free the control block */
      m_MMGR_FREE_HSM_CB(hsm_cb);

      /* reset the global cb handle */
      gl_hsm_hdl = 0;
   }
   return rc;
}


/****************************************************************************
 * Name          : hsm_finalize
 *
 * Description   : This function un-registers the EDA channels used by HSM,
 *                 frees the allocated memory and destroys the HSM control
 *                 block and associated handle.
 *
 * Arguments     : destroy_info - ptr to the NCS destroy information
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/

uns32 hsm_finalize ()
{
   HSM_CB *hsm_cb = 0;

   /* retrieve HSM CB */
   hsm_cb = (HSM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_hsm_hdl);
   if(!hsm_cb)
      return NCSCC_RC_FAILURE;

   /* finalize HPI session */
   saHpiSessionClose(hsm_cb->args->session_id);

#ifdef HPI_A
   /* finalize the HPI session */
   saHpiFinalize();
#endif

   /* unregister with EDA channels */
   hsm_eda_chan_finalize(hsm_cb);

   /* return HSM CB */
   ncshm_give_hdl(gl_hsm_hdl);

   /* remove the association with hdl-mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, hsm_cb->cb_hdl);

   /* stop & kill the created task */
   m_NCS_TASK_STOP(hsm_cb->task_hdl);
   m_NCS_TASK_RELEASE(hsm_cb->task_hdl);

   /* free HPI session arguments */
   m_MMGR_FREE_HPI_SESSION_ARGS(hsm_cb->args);

   /* free the control block */
   m_MMGR_FREE_HSM_CB(hsm_cb);

   /* reset the global cb handle */
   gl_hsm_hdl = 0;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : my_chan_open_callback
 *
 * Description   : EDA call back routine which gets invoked when 'channel
 *                 specific event' occurs.
 *
 * Arguments     : inv - invocation data
 *                 chan_hdl - the associated channel handle.
 *                 rc - channel status.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

static void
hsm_chan_open_callback(SaInvocationT inv, SaEvtChannelHandleT chan_hdl,
                      SaAisErrorT rc)
{
   if (NCSCC_RC_SUCCESS == rc)
      m_LOG_HISV_DEBUG("Async Channel open Successful inv\n");
   else
      m_LOG_HISV_DEBUG("Async Channel open Failed\n");
   return;
}


/****************************************************************************
 * Name          : hsm_event_callback
 *
 * Description   : EDS subscription call-back routine. It is not used by
 *                 HSM because HSM is a publisher of events and not the
 *                 subscriber to receive the events from EDSv.
 *
 * Arguments     : sub_id - subscription id.
 *                 event_hdl - the associated channel event handle.
 *                 event_data_size - maximum event data size
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

static void
hsm_event_callback(SaEvtSubscriptionIdT sub_id, SaEvtEventHandleT event_hdl,
                  const SaSizeT event_data_size)
{
   return;
}



/****************************************************************************
 * Name          : hsm_eda_chan_initialize
 *
 * Description   : This function initializes the EDA channel used for
 *                 publishing events on EDSv. It creates separate channels
 *                 for publishing fault, non-fault events and resource
 *                 inventory data. Again to distinguish between the published
 *                 fault, non-fault events, it assigns unique pattern sets for
 *                 publishing fault, non-fault events.
 *
 * Arguments     : hsm_cb - pointer to HSM control block structure.
 *
 * Return Values : NCSCC_RC_SUCCESS, if init success else NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hsm_eda_chan_initialize(HSM_CB *hsm_cb)
{
   uns32    rc, count = 0;
   SaSelectionObjectT       evt_sel_obj;
   SaTimeT                  timeout = 0xffffffff;
   SaVersionT         saf_version = {'B', 1, 1};
   SaEvtChannelOpenFlagsT   open_flags = 0;
   /* event callback routines */
   SaEvtCallbacksT    callbacks = {&hsm_chan_open_callback,
                                   &hsm_event_callback};

   /* intialize EDA library */
   /* m_LOG_HISV_DEBUG("Initializing eda library\n"); */

   /* initialize SAF EVT library */
   do
   {
      rc = saEvtInitialize(&hsm_cb->eda_handle, &callbacks , &saf_version);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_HISV_DEBUG("SaEvtInitialize() failed.\n");
         m_NCS_TASK_SLEEP(3000);
      }
      else
         break;
   }
   while (count++ <= 3);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;

   /* get a selection object */
   /* m_LOG_HISV_DEBUG("Getting a selection object\n"); */
   rc = saEvtSelectionObjectGet(hsm_cb->eda_handle, &evt_sel_obj);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DEBUG("SaEvtSelectionObjectGet() failed.\n");
      return(NCSCC_RC_FAILURE);
   }

   /** open the event channel for publishing fault/non-fault events
    **/
   /* m_LOG_HISV_DEBUG("Opening event channel for publishing events\n"); */
   open_flags = SA_EVT_CHANNEL_CREATE | SA_EVT_CHANNEL_PUBLISHER;

   hsm_cb->event_chan_name.length = m_NCS_STRLEN(EVT_CHANNEL_NAME);
   m_NCS_STRCPY(hsm_cb->event_chan_name.value, EVT_CHANNEL_NAME);

   rc = saEvtChannelOpen(hsm_cb->eda_handle, &hsm_cb->event_chan_name, open_flags,
                         timeout, &hsm_cb->chan_evt);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DEBUG("SaEvtChannelOpen() failed.\n");
      return(NCSCC_RC_FAILURE);
   }

   hsm_cb->eds_init_success = 1;
   return(NCSCC_RC_SUCCESS);
}


/****************************************************************************
 * Name          : hsm_eda_chan_finalize
 *
 * Description   : This function cleans-up the channel used for publishing
 *                 events on EDSv
 *
 * Arguments     : hsm_cb - pointer for HSM control block structure.
 *
 * Return Values : NCSCC_RC_SUCCESS, if clean success else NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hsm_eda_chan_finalize(HSM_CB *hsm_cb)
{
   uns32    rc;

   hsm_cb->eds_init_success = 0;
   /* close fault/non-fault event channel */
   /* m_LOG_HISV_DEBUG("Closing fault/non-fault event channel\n"); */
   rc = saEvtChannelClose(hsm_cb->chan_evt);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DEBUG("SaEvtChannelClose() failed.\n");
      return(NCSCC_RC_FAILURE);
   }

   /* finalize usage of EDA */
   /* m_LOG_HISV_DEBUG("Finalizing\n"); */
   rc = saEvtFinalize(hsm_cb->eda_handle);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DEBUG("SaEvtFinalize() failed.\n");
      return(NCSCC_RC_FAILURE);
   }

   return(NCSCC_RC_SUCCESS);
}


/****************************************************************************
 * Name          : hsm_eda_event_publish
 *
 * Description   : This function publishes the events on EDSv
 *
 * Arguments     : type of event, data, size and event id.
 *
 * Return Values : NCSCC_RC_SUCCESS, if publish success else NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hsm_eda_event_publish (HSM_EVT_TYPE event_type, uns8 *evt_data,
                             uns32 evt_data_size, HSM_CB *hsm_cb)
{
   SaEvtEventIdT event_id;
   SaUint8T      priority = 1;
   uns32         rc;
   SaTimeT       retention_time = 0;
   uns8 * evt_publish = NULL;

   /* check if eds is not yet initialized? */
   if (hsm_cb->eds_init_success == 0)
   {
      m_LOG_HISV_DTS_CONS("EDS was not ready\n");
      if (hsm_eda_chan_initialize(hsm_cb) != NCSCC_RC_SUCCESS)
      {
         m_LOG_HISV_DTS_CONS("EDS could not be initialized\n");
         return NCSCC_RC_FAILURE;
      }

   }
   /* fill the version here */
   ((HPI_HISV_EVT_T *)(evt_data))->version = (HISV_SW_VERSION | HISV_EDS_INF_VERSION);
   /* allocate the fault event handle for fault/non-fault event channe */
   /* m_LOG_HISV_DEBUG("Allocating an event for fault, non-fault event channel\n");*/
   rc = saEvtEventAllocate(hsm_cb->chan_evt, &hsm_cb->fault_evt_handle);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_HISV_DEBUG("SaEvtEventAllocate() failed.\n");
      return(NCSCC_RC_FAILURE);
   }

   if (event_type == HSM_FAULT_EVT)
   {
      /* set the fault channel event pattern attributes */
      /* m_LOG_HISV_DEBUG("Setting the fault channel event attributes\n");*/
      hsm_cb->fault_pattern.patternsNumber = HPI_FAULT_PATTERN_ARRAY_LEN;
      hsm_cb->fault_pattern.patterns = hpi_fault_pattern_array;

      rc = saEvtEventAttributesSet(hsm_cb->fault_evt_handle, &hsm_cb->fault_pattern,
                                   priority, retention_time, &hsm_cb->event_chan_name);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_HISV_DEBUG("SaEvtEventAttributesSet() failed 1.\n");
         return(NCSCC_RC_FAILURE);
      }
   }
   else
   {
      /* set the non-fault event pattern attributes */
      /* m_LOG_HISV_DEBUG("Setting the non fault channel event attributes\n");*/
      hsm_cb->non_fault_pattern.patternsNumber = HPI_NON_FAULT_PATTERN_ARRAY_LEN;
      hsm_cb->non_fault_pattern.patterns = hpi_non_fault_pattern_array;
      rc = saEvtEventAttributesSet(hsm_cb->fault_evt_handle, &hsm_cb->non_fault_pattern,
                                   priority, retention_time, &hsm_cb->event_chan_name);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_HISV_DEBUG("SaEvtEventAttributesSet() failed.\n");
         return(NCSCC_RC_FAILURE);
      }

   }
   /* encode the message before publishing to EDSv */
   hsm_encode_hisv_evt ((HPI_HISV_EVT_T *)evt_data, &evt_publish, ((HPI_HISV_EVT_T *)evt_data)->version);

   /* Publish the consolidated event message */
   if (evt_publish != NULL)
   {
      rc = saEvtEventPublish(hsm_cb->fault_evt_handle, evt_publish, evt_data_size, &event_id);
      m_MMGR_FREE_HPI_INV_DATA(evt_publish);
   }

   /* free the fault event handle */
   if (NCSCC_RC_SUCCESS != saEvtEventFree(hsm_cb->fault_evt_handle))
   {
      m_LOG_HISV_DEBUG("SaEvtEventFree() failed.\n");
   }
   return rc;
}

/*************************************************************************
 * Function:  hsm_encode_hisv_evt
 *
 * Purpose:   program handler for HPI_HISV_EVT_T. This is invoked when HISv
 *            has to perform ENCODE on HPI_HISV_EVT_T before publishing
 *
 * Input    :
 *          : input buffer and structure to be encoded
 * Output   :
 * Return Value : NCSCC_RC_SUCCESS/ NCSCC_RC_FAILURE
 *
 * NOTES    :
 *
 ************************************************************************/
uns32
hsm_encode_hisv_evt (HPI_HISV_EVT_T *evt_data, uns8 **evt_publish, uns32 version)
{
   uns32 rc = NCSCC_RC_SUCCESS, i, mac_len = MAX_MAC_ENTRIES * MAC_DATA_LEN;
   NCS_UBAID    uba;
   uns8         *p8;

   m_NCS_OS_MEMSET(&uba, '\0', sizeof(uba));
   ncs_enc_init_space(&uba);

   /* encode HPI event */
   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->hpi_event.Source);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->hpi_event.EventType);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->hpi_event.Timestamp);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->hpi_event.Severity);
   ncs_enc_claim_space(&uba, 4);

   ncs_encode_n_octets_in_uba(&uba, (uns8 *)&evt_data->hpi_event.EventDataUnion,
                              (int32)sizeof(SaHpiEventUnionT));
   /* encode entity path */
   for (i=0; i<SAHPI_MAX_ENTITY_PATH; i++)
   {
      p8 = ncs_enc_reserve_space(&uba, 4);
      ncs_encode_32bit(&p8, evt_data->entity_path.Entry[i].EntityType);
      ncs_enc_claim_space(&uba, 4);

      p8 = ncs_enc_reserve_space(&uba, 4);
#ifdef HPI_A
      ncs_encode_32bit(&p8, evt_data->entity_path.Entry[i].EntityInstance);
#else
      ncs_encode_32bit(&p8, evt_data->entity_path.Entry[i].EntityLocation);
#endif
      ncs_enc_claim_space(&uba, 4);
   }

   /* encode inventory data */
   /* product name */
   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.product_name.DataType);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.product_name.Language);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 1);
   ncs_encode_8bit(&p8, evt_data->inv_data.product_name.DataLength);
   ncs_enc_claim_space(&uba, 1);

   ncs_encode_n_octets_in_uba(&uba, (uns8 *)&evt_data->inv_data.product_name.Data,
                                    SAHPI_MAX_TEXT_BUFFER_LENGTH);

   /* product version */
   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.product_version.DataType);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.product_version.Language);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 1);
   ncs_encode_8bit(&p8, evt_data->inv_data.product_version.DataLength);
   ncs_enc_claim_space(&uba, 1);


   ncs_encode_n_octets_in_uba(&uba, (uns8 *)&evt_data->inv_data.product_version.Data,
                                    SAHPI_MAX_TEXT_BUFFER_LENGTH);

   /* encode OEM inventory data */
   p8 = ncs_enc_reserve_space(&uba, 1);
   ncs_encode_8bit(&p8, evt_data->inv_data.oem_inv_data.type);
   ncs_enc_claim_space(&uba, 1);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.oem_inv_data.mId);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.oem_inv_data.mot_oem_rec_id);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.oem_inv_data.rec_format_ver);
   ncs_enc_claim_space(&uba, 4);

   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->inv_data.oem_inv_data.num_mac_entries);
   ncs_enc_claim_space(&uba, 4);

   ncs_encode_n_octets_in_uba(&uba, (uns8 *)&evt_data->inv_data.oem_inv_data.interface_mac_addr, mac_len);

   /* encode version */
   p8 = ncs_enc_reserve_space(&uba, 4);
   ncs_encode_32bit(&p8, evt_data->version);
   ncs_enc_claim_space(&uba, 4);

   /* get the stuff back to user buffer for publishing */
   *evt_publish = m_MMGR_ALLOC_HPI_INV_DATA(uba.ttl);
   ncs_decode_n_octets_from_uba(&uba, *evt_publish, uba.ttl);

   return rc;
}

