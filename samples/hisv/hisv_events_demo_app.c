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
 *            Hewlett-Packard Company
 */

/*****************************************************************************

  DESCRIPTION:

  This file contains the HISv events sample application. It demonstrates the
  following:
  a) reading HISv events from the EDSv "EVENTS" Channel.
  b) certain EDSv features:
      i)  subscribe mechanism for HISv events.
      ii) Retention timer usage with published event.
..............................................................................

******************************************************************************
*/

/* HISv Toolkit header file */
#include <opensaf/hpl_api.h>
#include <opensaf/hpl_msg.h>

#include <opensaf/mds_papi.h>
#include <opensaf/ncssysf_tmr.h>
#include <opensaf/ncssysf_lck.h>
/* FIXME: Shouldn't be using internal header for sample
#include "../../services/hisv/inc/hcd_mem.h"
#include "../../services/hisv/inc/hcd_util.h"
#include "../../services/hisv/inc/hisv_msg.h"
#include "../../services/hisv/inc/ham_cb.h"

#include "../../services/hisv/inc/hpl_cb.h"*/
#include "hisv_events_demo_app.h"

EXTERN_C int raw;

typedef struct hpi_evt_type
{
   SaHpiEventT            hpi_event;
   SaHpiEntityPathT       entity_path;
   HISV_INV_DATA          inv_data;
   uns32                  version;      /* versioning; see the macros defined in hpl_msg.h */
}HPI_EVT_T;


/*############################################################################
                            Global Variables
############################################################################*/
/* EVT Handle */
SaEvtHandleT          gl_evt_hdl = 0;

/* Channel created/opened in the demo */
SaNameT               gl_chan_name = {6,EVT_CHANNEL_NAME};

/* Channel handle of the channel that'll be used */
SaEvtChannelHandleT   gl_chan_hdl = 0;

/* Event Handle of the event that'll be published */
SaEvtEventHandleT     gl_chan_pub_event_hdl = 0;

/* Event id of the to be published event */
SaEvtEventIdT         gl_chan_pub_event_id = 0;

/* subscription id of the installed subscription */
SaEvtSubscriptionIdT  gl_subid = 19428;

/* Name of the event publisher */
SaNameT               gl_pubname = {11,"EDSvToolkit"};

/* Data content of the event */
uns8                  gl_event_data[64] = "  I am a TRAP event";

/* Description of the pattern by which the evt will be published */
#define TRAP_PATTERN_ARRAY_LEN 3
static SaEvtEventPatternT gl_trap_pattern_array[TRAP_PATTERN_ARRAY_LEN] = {
   {13, 13, (SaUint8T *) "trap xyz here"},
   {0,0,    (SaUint8T *) NULL},
   {5,5,    (SaUint8T *) "abcde"}
};

/* Description of the filter by which the subscription on the TRAP channel
 * will be installed.
 */
#define TRAP_FILTER_ARRAY_LEN 6
static SaEvtEventFilterT gl_trap_filter_array[TRAP_FILTER_ARRAY_LEN] = {
   {SA_EVT_PREFIX_FILTER,   {4,4,(SaUint8T *) "trap"}},
   {SA_EVT_SUFFIX_FILTER,   {0,0,  (SaUint8T *) NULL}},
   {SA_EVT_SUFFIX_FILTER,   {4,4,(SaUint8T *) "bcde"}},
   {SA_EVT_SUFFIX_FILTER,   {0,0,  (SaUint8T *) NULL}},
   {SA_EVT_PASS_ALL_FILTER, {4,4,(SaUint8T *) "fooy"}},
   {SA_EVT_PASS_ALL_FILTER, {0,0,  (SaUint8T *) NULL}}
};

/* Pattern Array set for subscribing events */
static SaEvtEventFilterT hpi_filter_array[TRAP_FILTER_ARRAY_LEN]
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
      },

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


/*############################################################################
                            Macro Definitions
############################################################################*/
/* Macro to retrieve the EVT version */
#define m_EDSV_EVT_VER_SET(ver) \
{ \
   ver.releaseCode = 'B'; \
   ver.majorVersion = 0x01; \
   ver.minorVersion  = 0x01; \
};

/*############################################################################
                       Static Function Decalarations
############################################################################*/
/* Channel open callback that is registered with EVT during saEvtInitialize()*/
static void edsv_chan_open_callback(SaInvocationT,  
                                    SaEvtChannelHandleT,
                                    SaAisErrorT);

/* Event deliver callback that is registered with EVT during saEvtInitialize()*/
static void edsv_evt_delv_callback(SaEvtSubscriptionIdT, 
                                   SaEvtEventHandleT,
                                   const SaSizeT);

/* Utilty routine to allocate an empty pattern array */
static uns32 alloc_pattern_array(SaEvtEventPatternArrayT **pattern_array,
                                 uns32 num_patterns, 
                                 uns32 pattern_size);

/* Utility routine to free a pattern array */
static void free_pattern_array(SaEvtEventPatternArrayT *pattern_array);

/* Utility routine to dump a pattern array */
static void dump_event_patterns(SaEvtEventPatternArrayT *pattern_array);



/*#############################################################################
                           End Of Declarations
###############################################################################*/

/****************************************************************************
  Name          : ncs_hisv_run
 
  Description   : This routine runs the HISv sample demo.
                  It is based on the original EDSv demo, but instead of
                  creating a new channel and publishing events to it,
                  this demo listens for HISv events on the EDSv "EVENTS"
                  channel - and prints out the event information in a raw
                  or human-readable format.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : This demo involves a single process 
                  acting as a subscriber and a publisher.
                  The process first subscribes the publishes
                  and obatins the event. It then displays the
                  attributes obtained.
                  
******************************************************************************/
uns32 ncs_hisv_run(void)
{

   SaAisErrorT                 rc;
   SaTimeT                  timeout = 100000000000ll; /* In nanoseconds */
   SaTimeT                  retention_time = 10000000000ll; /* 10 sec in NanoSeconds */
   SaUint8T                 priority = SA_EVT_HIGHEST_PRIORITY;
   SaEvtChannelOpenFlagsT   chan_open_flags = 0;
   SaVersionT               ver;
   SaEvtEventPatternArrayT  pattern_array;
   SaEvtEventFilterArrayT   filter_array;
   SaEvtCallbacksT          reg_callback_set; 
   SaSelectionObjectT       evt_sel_obj;
   NCS_SEL_OBJ              evt_ncs_sel_obj;
   NCS_SEL_OBJ_SET          wait_sel_obj;


   /* this is to allow to establish MDS session with EDSv */
   m_NCS_TASK_SLEEP(3000);

   /*#########################################################################
                     Demonstrating the usage of saEvtInitialize()
   #########################################################################*/

   /* Fill the callbacks that are to be registered with EVT */
   m_NCS_MEMSET(&reg_callback_set, 0, sizeof(SaEvtCallbacksT));

   reg_callback_set.saEvtChannelOpenCallback  = edsv_chan_open_callback;
   reg_callback_set.saEvtEventDeliverCallback = edsv_evt_delv_callback;

   /* Fill the EVT version */
   m_EDSV_EVT_VER_SET(ver);

   if (SA_AIS_OK != (rc = saEvtInitialize(&gl_evt_hdl, &reg_callback_set, &ver)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtInitialize() failed. rc=%d \n",rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF(
      "\n HISv: EDA: EVT Initialization Done !!! \n EvtHandle: %x \n", (uns32)gl_evt_hdl);

   /*#########################################################################
                  Demonstrating the usage of saEvtSelectionObjectGet()
   #########################################################################*/

   if (SA_AIS_OK != (rc = saEvtSelectionObjectGet(gl_evt_hdl, &evt_sel_obj)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtSelectionObjectGet() failed. rc=%d \n",rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n HISv: EDA: Obtained Selection Object successfully !!! \n");

  /*#########################################################################
                  Demonstrating the usage of saEvtChannelOpen()
   #########################################################################*/

   m_NCS_TASK_SLEEP(2000);

   chan_open_flags = 
      SA_EVT_CHANNEL_SUBSCRIBER;

   if (SA_AIS_OK != 
         (rc = 
            saEvtChannelOpen(gl_evt_hdl, &gl_chan_name, chan_open_flags, timeout, &gl_chan_hdl)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtChannelOpen() failed. rc=%d \n",rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n HISv: Opened HISv EVENTS - Channel successfully !!! \n");

  /*#########################################################################
                  Demonstrating the usage of SaEvtEventSubscribe()
   #########################################################################*/
   m_NCS_TASK_SLEEP(2000);

   filter_array.filtersNumber = TRAP_FILTER_ARRAY_LEN;
   filter_array.filters = hpi_filter_array;
   if (SA_AIS_OK != (rc = saEvtEventSubscribe(gl_chan_hdl, &filter_array, gl_subid)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtEventSubscribe() failed. rc=%d \n",rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n HISv: Subscribed for events on HISv EVENTS channel successfully !!! \n");

   m_NCS_TASK_SLEEP(2000);

   /***** Now wait (select) on EVT selction object *****/

   /* Reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_obj);

   /* derive the fd for EVT selection object */
   m_SET_FD_IN_SEL_OBJ((uns32)evt_sel_obj, evt_ncs_sel_obj);

   /* Set the EVT select object on which HISv sample demo waits */
   m_NCS_SEL_OBJ_SET(evt_ncs_sel_obj, &wait_sel_obj);

   m_NCS_CONS_PRINTF("\n HISv: Waiting on event from the HISv EVENTS channel... \n");
   while (m_NCS_SEL_OBJ_SELECT(evt_ncs_sel_obj,&wait_sel_obj,NULL,NULL,NULL) != -1)
   {
      /* Process HISv evt messages */
      if (m_NCS_SEL_OBJ_ISSET(evt_ncs_sel_obj, &wait_sel_obj))
      {
         /* Dispatch all pending messages */
         if (raw) {
            m_NCS_CONS_PRINTF("\n HISv: EDA: Dispatching message received on HISv EVENTS channel\n");
         }
         
         /*######################################################################
                        Demonstrating the usage of saEvtDispatch()
         ######################################################################*/
         rc = saEvtDispatch(gl_evt_hdl, SA_DISPATCH_ALL);

         if (rc != SA_AIS_OK)
            m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtDispatch() failed. rc=%d \n", rc);
         
         /* Rcvd the published event, now loop back to wait on next event. */
         /* break; */
      }

      /* Again set EVT select object to wait for another callback */
      m_NCS_SEL_OBJ_SET(evt_ncs_sel_obj, &wait_sel_obj);
   }

   /*######################################################################
                        Demonstrating the usage of saEvtEventFree()
   ########################################################################*/
   m_NCS_TASK_SLEEP(4000);

   if (SA_AIS_OK != (rc = saEvtEventFree(gl_chan_pub_event_hdl)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtEventFree() failed. rc=%d\n", rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n HISv: Freed the event successfully !!! \n");

   /*##########################################################################
                        Demonstrating the usage of saEvtEventUnsubscribe()
   ############################################################################*/
   m_NCS_TASK_SLEEP(2000);

   if (SA_AIS_OK != (rc = saEvtEventUnsubscribe(gl_chan_hdl, gl_subid)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtEventUnsubscribe() failed. rc=%d \n",rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n HISv: Unsubscribed for events successfully !!! \n");

   /*###########################################################################
                        Demonstrating the usage of saEvtChannelClose()
   #############################################################################*/
   m_NCS_TASK_SLEEP(2000);

   if (SA_AIS_OK != (rc = saEvtChannelClose(gl_chan_hdl)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtChannelClose() failed. rc=%d\n",rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n HISv: Closed HISv EVENTS channel successfully !!! \n");

   /*###########################################################################
                        Demonstrating the usage of SaEvtFinalize()
   #############################################################################*/
   m_NCS_TASK_SLEEP(2000);

   rc = saEvtFinalize(gl_evt_hdl);

   if (rc != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtFinalize() failed. rc=%d\n", rc);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n HISv: Finalized with event service successfully !!! \n");

   return NCSCC_RC_SUCCESS;

}

/****************************************************************************
  Name          : edsv_chan_open_callback
 
  Description   : This routine is a callback to notify
                  about channel opens. It is specified as a part of EVT initialization.
                  
 
  Arguments     : inv             - particular invocation of this callback 
                                    function
                  chan_hdl        - hdl of the channel which was opened.
                  rc              - return code.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : NONE
******************************************************************************/

static void 
edsv_chan_open_callback(SaInvocationT inv, 
                        SaEvtChannelHandleT chan_hdl,
                        SaAisErrorT rc)
{
   return;
}

/****************************************************************************
  Name          : edsv_evt_delv_callback
 
  Description   : This routine is a callback to deliver events. 
                  It is specified as a part of EVT initialization.
                  It demonstrates the use of following EVT APIs:
                  a) SaEvtEventAttributesGet() b) SaEvtEventDataGet().
                 
 
  Arguments     : sub_id          - subscription id of the rcvd. event.
                  event_hdl       - hdl of the received evt.
                  event_data_size - data size of the received event.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : NONE
******************************************************************************/

static void 
edsv_evt_delv_callback(SaEvtSubscriptionIdT sub_id,
                       SaEvtEventHandleT event_hdl,
                       const SaSizeT event_data_size)
{
   SaEvtEventPatternArrayT  *pattern_array;
   SaUint8T                  priority;
   SaTimeT                   retention_time;
   SaNameT                   publisher_name;
   SaTimeT                   publish_time;
   SaEvtEventIdT             event_id;
   SaUint8T                 *p_data = NULL;
   SaSizeT                   data_len;
   uns32                     rc;
   uns32                     num_patterns;
   uns32                     pattern_size;
   HPI_EVT_T                *hpi_event     = NULL;
   SaUint8T                  hpi_event_type_string[50];
   SaUint8T                  hpi_event_sev_string[50];
   SaUint8T                  hpi_hs_state_string[50];
   SaUint8T                  hpi_sensor_type_string[50];
   SaUint8T                  hpi_sensor_es_string[50];
   SaUint8T                  hpi_entity_path_buffer[100];
   SaUint8T                  hpipower_string[100];
   SaUint8T                  hpi_entity_path[8][50];
   uns32                     hpi_entity_path_depth = 0;
   uns32                     hpi_entity_path_max = 8;
   SaInt32T		     i;
   SaInt32T		     hpi_event_slot;
   uns32                     hisv_power_res = 0;
   SaUint8T                  hpi_entity_path2[300];
   NCS_LIB_REQ_INFO          req_info;
   HPL_CB                    *hpl_cb;
   NCS_LIB_CREATE            hisv_create_info;

   /* Prepare an appropriate-sized data buffer.  */
   data_len = event_data_size;
   p_data = m_MMGR_ALLOC_EDSVTM_EVENT_DATA((uns32)data_len+1);
   if (p_data == NULL)
      return;
   m_NCS_MEMSET(&publisher_name,'\0',sizeof(SaNameT));
   m_NCS_MEMSET(p_data, 0, (size_t)data_len+1);

   /* Create an empty patternArray to be filled in by EDA */
   num_patterns = 8;
   pattern_size = 128;

   if (NCSCC_RC_SUCCESS != 
         (rc = alloc_pattern_array(&pattern_array, num_patterns, pattern_size)))
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtEventAttributesGet() failed. rc=%d\n",rc);
      return;
   }

  /*###########################################################################
                  Demonstrating the usage of saEvtEventAttributesGet()
   #############################################################################*/

   /* Get the event attributes */
   pattern_array->allocatedNumber=8;
   rc = saEvtEventAttributesGet(event_hdl, pattern_array,
                                &priority, &retention_time,
                                &publisher_name, &publish_time,
                                &event_id);
   if (rc != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtEventAttributesGet() failed. rc=%d\n",rc);
      return;
   }

   if (raw) {
      m_NCS_CONS_PRINTF("\n HISv: Got attributes of the HISv event successfully !!! \n");
   }

  /*#############################################################################
                 Demonstrating the usage of SaEvtEventDataGet()
   #############################################################################*/

   /* Get the event data */
   rc = saEvtEventDataGet(event_hdl, p_data, &data_len);
   if (rc != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtEventDataGet() failed. rc=%d\n",rc);
      return;
   }

   if (raw) {
      m_NCS_CONS_PRINTF("\n HISv: Got data from the HISv event successfully !!! \n");
   }


   /* Say what we received */
   m_NCS_CONS_PRINTF("\n-------------- Received HISv Event ---------------------\n");

   if (raw) {
      m_NCS_CONS_PRINTF(" publisherName  =    %s\n", publisher_name.value);
      m_NCS_CONS_PRINTF(" patternArray\n");
      dump_event_patterns(pattern_array);
      m_NCS_CONS_PRINTF(" priority       =    %d\n", priority);
      m_NCS_CONS_PRINTF(" publishTime    =    %llu\n", (uns64)publish_time);
      m_NCS_CONS_PRINTF(" retentionTime  =    %llu\n", (uns64)retention_time);
      m_NCS_CONS_PRINTF(" eventId        =    %llu\n", (uns64)event_id);
      m_NCS_CONS_PRINTF(" dataLen        =    %d\n", (uns32)data_len);
      m_NCS_CONS_PRINTF(" data           =    %s\n", p_data);
   }

      /* hpi_event = m_MMGR_ALLOC_AVM_DEFAULT_VAL(sizeof(HPI_EVT_T)); */
      hpi_event = malloc(sizeof(HPI_EVT_T));
      memset(hpi_event, '\0', sizeof(HPI_EVT_T));
      rc = hpl_decode_hisv_evt((HPI_HISV_EVT_T *)hpi_event, p_data, data_len, (HISV_SW_VERSION | HISV_EDS_INF_VERSION));

      /* m_NCS_CONS_PRINTF(" hpi rc         =    %d\n", rc); */
   if (raw) {
      m_NCS_CONS_PRINTF(" hpi prod namel =    %d\n", hpi_event->inv_data.product_name.DataLength);
      if ((hpi_event->inv_data.product_name.DataLength != 0) && (hpi_event->inv_data.product_name.DataLength != 255))
         m_NCS_CONS_PRINTF(" hpi prod name  =    %s\n", hpi_event->inv_data.product_name.Data);
      else
         m_NCS_CONS_PRINTF(" hpi prod name  = \n");
      m_NCS_CONS_PRINTF(" hpi prod versl =    %d\n", hpi_event->inv_data.product_version.DataLength);
      if ((hpi_event->inv_data.product_version.DataLength != 0) && (hpi_event->inv_data.product_version.DataLength != 255))
         m_NCS_CONS_PRINTF(" hpi prod vers  =    %s\n", hpi_event->inv_data.product_version.Data);
      else
         m_NCS_CONS_PRINTF(" hpi prod vers  = \n");
      m_NCS_CONS_PRINTF(" hpi event res  =    %d\n", hpi_event->hpi_event.Source);
      m_NCS_CONS_PRINTF(" hpi event type =    %d\n", hpi_event->hpi_event.EventType);
      m_NCS_CONS_PRINTF(" hpi event sev  =    %d\n", hpi_event->hpi_event.Severity);
      m_NCS_CONS_PRINTF(" hotswap state  =    %d\n", hpi_event->hpi_event.EventDataUnion.HotSwapEvent.HotSwapState);
      m_NCS_CONS_PRINTF(" prev hs state  =    %d\n", hpi_event->hpi_event.EventDataUnion.HotSwapEvent.PreviousHotSwapState);
      m_NCS_CONS_PRINTF(" entity type 0  =    %d\n", hpi_event->entity_path.Entry[0].EntityType);
      m_NCS_CONS_PRINTF(" entity loc  0  =    %d\n", hpi_event->entity_path.Entry[0].EntityLocation);
      m_NCS_CONS_PRINTF(" entity type 1  =    %d\n", hpi_event->entity_path.Entry[1].EntityType);
      m_NCS_CONS_PRINTF(" entity loc  1  =    %d\n", hpi_event->entity_path.Entry[1].EntityLocation);
      m_NCS_CONS_PRINTF(" entity type 2  =    %d\n", hpi_event->entity_path.Entry[2].EntityType);
      m_NCS_CONS_PRINTF(" entity loc  2  =    %d\n", hpi_event->entity_path.Entry[2].EntityLocation);
      m_NCS_CONS_PRINTF(" entity type 3  =    %d\n", hpi_event->entity_path.Entry[3].EntityType);
      m_NCS_CONS_PRINTF(" entity loc  3  =    %d\n", hpi_event->entity_path.Entry[3].EntityLocation);
   }
   else {
      switch (hpi_event->hpi_event.EventType) {
         case 0:
            strcpy (hpi_event_type_string, "SAHPI_ET_RESOURCE");
            break;
         case 1:
            strcpy (hpi_event_type_string, "SAHPI_ET_DOMAIN");
            break;
         case 2:
            strcpy (hpi_event_type_string, "SAHPI_ET_SENSOR");
            break;
         case 3:
            strcpy (hpi_event_type_string, "SAHPI_ET_SENSOR_ENABLE_CHANGE");
            break;
         case 4:
            strcpy (hpi_event_type_string, "SAHPI_ET_HOTSWAP");
            break;
         case 5:
            strcpy (hpi_event_type_string, "SAHPI_ET_WATCHDOG");
            break;
         case 6:
            strcpy (hpi_event_type_string, "SAHPI_ET_HPI_SW");
            break;
         case 7:
            strcpy (hpi_event_type_string, "SAHPI_ET_OEM");
            break;
         case 8:
            strcpy (hpi_event_type_string, "SAHPI_ET_USER");
            break;
         case 9:
            strcpy (hpi_event_type_string, "SAHPI_ET_DIMI");
            break;
         case 10:
            strcpy (hpi_event_type_string, "SAHPI_ET_DIMI_UPDATE");
            break;
         case 11:
            strcpy (hpi_event_type_string, "SAHPI_ET_FUMI");
            break;
         default:
            strcpy (hpi_event_type_string, "UNKNOWN");
            break;
      }
      m_NCS_CONS_PRINTF("       Event Type : %s\n", hpi_event_type_string);
      switch (hpi_event->hpi_event.Severity) {
         case 0:
            strcpy (hpi_event_sev_string, "SAHPI_CRITICAL");
            break;
         case 1:
            strcpy (hpi_event_sev_string, "SAHPI_MAJOR");
            break;
         case 2:
            strcpy (hpi_event_sev_string, "SAHPI_MINOR");
            break;
         case 3:
            strcpy (hpi_event_sev_string, "SAHPI_INFORMATIONAL");
            break;
         case 4:
            strcpy (hpi_event_sev_string, "SAHPI_OK");
            break;
         case 240:
            strcpy (hpi_event_sev_string, "SAHPI_DEBUG");
            break;
         case 255:
            strcpy (hpi_event_sev_string, "SAHPI_ALL_SEVERITIES");
            break;
         default:
            strcpy (hpi_event_sev_string, "UNKNOWN");
            break;
      }
      m_NCS_CONS_PRINTF("   Event Severity : %s\n", hpi_event_sev_string);
      if (hpi_event->hpi_event.EventType == SAHPI_ET_HOTSWAP) {
         switch (hpi_event->hpi_event.EventDataUnion.HotSwapEvent.HotSwapState) {
            case 0:
               strcpy (hpi_hs_state_string, "SAHPI_HS_STATE_INACTIVE");
               break;
            case 1:
               strcpy (hpi_hs_state_string, "SAHPI_HS_STATE_INSERTION_PENDING");
               break;
            case 2:
               strcpy (hpi_hs_state_string, "SAHPI_HS_STATE_ACTIVE");
               break;
            case 3:
               strcpy (hpi_hs_state_string, "SAHPI_HS_STATE_EXTRACTION_PENDING");
               break;
            case 4:
               strcpy (hpi_hs_state_string, "SAHPI_HS_STATE_NOT_PRESENT");
               break;
            default:
               strcpy (hpi_hs_state_string, "UNKNOWN");
               break;
         } 
  
         m_NCS_CONS_PRINTF("    HotSwap State : %s\n", hpi_hs_state_string);
      }
      if (hpi_event->hpi_event.EventType == SAHPI_ET_SENSOR) {
         switch (hpi_event->hpi_event.EventDataUnion.SensorEvent.SensorType) {
            case 1:
               strcpy (hpi_sensor_type_string, "SAHPI_TEMPERATURE");
               break;
            case 2:
               strcpy (hpi_sensor_type_string, "SAHPI_VOLTAGE");
               break;
            case 3:
               strcpy (hpi_sensor_type_string, "SAHPI_CURRENT");
               break;
            case 4:
               strcpy (hpi_sensor_type_string, "SAHPI_FAN");
               break;
            case 5:
               strcpy (hpi_sensor_type_string, "SAHPI_PHYSICAL_SECURITY");
               break;
            case 6:
               strcpy (hpi_sensor_type_string, "SAHPI_PLATFORM_VIOLATION");
               break;
            case 7:
               strcpy (hpi_sensor_type_string, "SAHPI_PROCESSOR");
               break;
            case 8:
               strcpy (hpi_sensor_type_string, "SAHPI_POWER_SUPPLY");
               break;
            case 9:
               strcpy (hpi_sensor_type_string, "SAHPI_POWER_UNIT");
               break;
            case 10:
               strcpy (hpi_sensor_type_string, "SAHPI_COOLING_DEVICE");
               break;
            default:
               strcpy (hpi_sensor_type_string, "UNKNOWN");
               break;
         } 
  
         m_NCS_CONS_PRINTF("      Sensor Type : %s\n", hpi_sensor_type_string);
      }
      if (hpi_event->hpi_event.EventType == SAHPI_ET_SENSOR) {
         switch (hpi_event->hpi_event.EventDataUnion.SensorEvent.EventState) {
            case 1:
               strcpy (hpi_sensor_es_string, "SAHPI_ES_LOWER_MINOR");
               break;
            case 2:
               strcpy (hpi_sensor_es_string, "SAHPI_ES_LOWER_MAJOR");
               break;
            case 4:
               strcpy (hpi_sensor_es_string, "SAHPI_ES_LOWER_CRIT");
               break;
            case 8:
               strcpy (hpi_sensor_es_string, "SAHPI_ES_UPPER_MINOR");
               break;
            case 16:
               strcpy (hpi_sensor_es_string, "SAHPI_ES_UPPER_MAJOR");
               break;
            case 32:
               strcpy (hpi_sensor_es_string, "SAHPI_ES_UPPER_CRIT");
               break;
            default:
               strcpy (hpi_sensor_es_string, "UNKNOWN");
               break;
         } 
  
         m_NCS_CONS_PRINTF(" Sensor Evt State : %s\n", hpi_sensor_es_string);
      }
      for (i = 0; i<hpi_entity_path_max; i++) {
         hpi_entity_path_depth++;
         if (hpi_event->entity_path.Entry[i].EntityType == SAHPI_ENT_ROOT)
            break; 
      }

      for (i = 0; i < hpi_entity_path_depth; i++) {
         switch (hpi_event->entity_path.Entry[i].EntityType) {
            case SAHPI_ENT_ROOT:
            sprintf(hpi_entity_path_buffer, "{ROOT,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            case SAHPI_ENT_RACK:
            sprintf(hpi_entity_path_buffer, "{RACK,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            case SAHPI_ENT_ADVANCEDTCA_CHASSIS:
            sprintf(hpi_entity_path_buffer, "{ADVANCEDTCA_CHASSIS,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            case SAHPI_ENT_SYSTEM_CHASSIS:
            sprintf(hpi_entity_path_buffer, "{SYSTEM_CHASSIS,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            case SAHPI_ENT_PHYSICAL_SLOT:
            sprintf(hpi_entity_path_buffer, "{PHYSICAL_SLOT,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            case SAHPI_ENT_PICMG_FRONT_BLADE:
            sprintf(hpi_entity_path_buffer, "{PICMG_FRONT_BLADE,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            case SAHPI_ENT_SYSTEM_BLADE:
            sprintf(hpi_entity_path_buffer, "{SYSTEM_BLADE,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            case SAHPI_ENT_SWITCH_BLADE:
            sprintf(hpi_entity_path_buffer, "{SWITCH_BLADE,%d}", hpi_event->entity_path.Entry[i].EntityLocation); 
            break;

            default:
            sprintf(hpi_entity_path_buffer, "{UNKNOWN,0}"); 
            break; 
         }
         strcpy(hpi_entity_path[i], hpi_entity_path_buffer);
      }

      m_NCS_CONS_PRINTF("Entity Path Depth : %d\n", hpi_entity_path_depth);
      m_NCS_CONS_PRINTF("      Entity Path : ");
      for (i = (hpi_entity_path_depth - 1); i > -1; i--) {
         if (i == 0)
            m_NCS_CONS_PRINTF("%s\n", hpi_entity_path[i]);
         else
            m_NCS_CONS_PRINTF("%s", hpi_entity_path[i]);
      }
      hpi_event_slot = hpi_event->entity_path.Entry[0].EntityLocation;
      m_NCS_CONS_PRINTF("    Slot Location : %d\n", hpi_event_slot);

      /* Make sure we have a good data length before printing product name and version.  */
      if ((hpi_event->inv_data.product_name.DataLength != 0) && (hpi_event->inv_data.product_name.DataLength != 255))
         m_NCS_CONS_PRINTF("     Product Name : %s\n", hpi_event->inv_data.product_name.Data);
      if ((hpi_event->inv_data.product_version.DataLength != 0) && (hpi_event->inv_data.product_version.DataLength != 255))
         m_NCS_CONS_PRINTF("  Product Version : %s\n", hpi_event->inv_data.product_version.Data);

      /* Now test to see if we shutdown a blade. Use system call to OpenHPI to shutdown the blade. */
      if (hpi_event->hpi_event.EventType == SAHPI_ET_SENSOR) {
         if ((strcmp(hpi_sensor_type_string, "SAHPI_TEMPERATURE") == 0) &&
             (strcmp(hpi_event_sev_string, "SAHPI_CRITICAL") == 0)) {
            m_NCS_CONS_PRINTF("\n*****  TEMP TRIGGER EVENT - SHUTTING DOWN BLADE: %d  *****\n", hpi_event_slot);


            /* Uncomment these 2 lines to shut off the blade using OpenHPI, or ... */
            /*
            sprintf(hpipower_string, "/usr/bin/hpipower -d -b %d > /dev/null", hpi_event_slot);
            system(hpipower_string);
            */


            /* ... uncomment this section of lines to shut off the blade using HISv. */
            /*
            rc = hpl_initialize(&hisv_create_info);
            m_NCS_CONS_PRINTF("\n***** hpl_initialize rc = %d\n", rc);
            */

            /* Need to sleep for a few seconds before calling the HISv APIs. */
            /*
            sleep(3);

            sprintf(hpi_entity_path2, "{{SYSTEM_BLADE,%d},{SYSTEM_CHASSIS,2}}", hpi_event_slot);
            m_NCS_CONS_PRINTF("\n***** Shutting down resource: %s\n", hpi_entity_path2);
            hisv_power_res = hpl_resource_power_set(2, hpi_entity_path2, HISV_RES_POWER_OFF);
            m_NCS_CONS_PRINTF("\n***** hisv_power_res = %d\n", hisv_power_res);
            if (hisv_power_res == NCSCC_RC_SUCCESS)
               m_NCS_CONS_PRINTF("\n*****          BLADE SUCCESSFULLY SHUTDOWN          *****\n");
            else
               m_NCS_CONS_PRINTF("\n*****          BLADE COULD NOT BE SHUTDOWN          *****\n");
            */
         }
      }
   }

   m_NCS_CONS_PRINTF("--------------------------------------------------------\n\n");

   free(hpi_event);


  /*#############################################################################
               Demonstrating the usage of saEvtEventRetentionTimeClear()
   #############################################################################*/

   /* Test clearing the retention timer */
   if (0 != retention_time)
   {
      rc = saEvtEventRetentionTimeClear(gl_chan_hdl, event_id);
      if (rc != SA_AIS_OK)
      {
         m_NCS_CONS_PRINTF("\n HISv: EDA: saEvtEventRetentionTimeClear() failed. rc=%d\n",rc);
         goto done;
      }

      m_NCS_CONS_PRINTF("\n HISv: Cleared retention timer successfully !!! \n");
   }

done:
   
   /* Free data storage */
   if (p_data != NULL)
      m_MMGR_FREE_EDSVTM_EVENT_DATA(p_data);

   /* Free the rcvd event */
   rc =  saEvtEventFree(event_hdl);
   if (rc != SA_AIS_OK)
      m_NCS_CONS_PRINTF("\n HISv: EDA: SaEvtEventFree() failed. rc=%d\n",rc);
   else
      if (raw) {
         m_NCS_CONS_PRINTF("\n HISv: Freed the Event successfully \n");
      }

   /* Free the pattern_array */
   pattern_array->patternsNumber = 8;
   free_pattern_array(pattern_array);

   return;
}

/****************************************************************************
  Name          : alloc_pattern_array
 
  Description   : This routine initializes and clears an empty
                  pattern array. 
                  
               
  Arguments     : pattern_array    - pattern array to be initialized.
                  num_patterns     - number of patterns in the array.
                  pattern_size     - size of the patterns in th array.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : NONE
******************************************************************************/
static uns32
alloc_pattern_array(SaEvtEventPatternArrayT **pattern_array,
                    uns32 num_patterns,
                    uns32 pattern_size)
{
   uns32   x;
   SaEvtEventPatternArrayT  *array_ptr;
   SaEvtEventPatternT       *pattern_ptr;

   array_ptr = m_MMGR_ALLOC_EDSVTM_EVENT_PATTERN_ARRAY;
   if (array_ptr == NULL)
      return(NCSCC_RC_FAILURE);

   array_ptr->patterns = m_MMGR_ALLOC_EDSVTM_EVENT_PATTERNS(num_patterns);
   if (array_ptr->patterns == NULL)
      return(NCSCC_RC_FAILURE);

   pattern_ptr = array_ptr->patterns;
   for (x=0; x<num_patterns; x++)
   {
      pattern_ptr->pattern = m_MMGR_ALLOC_EDSVTM_EVENT_DATA(pattern_size);
      if (pattern_ptr->pattern == NULL)
         return(NCSCC_RC_FAILURE);

      pattern_ptr->patternSize = pattern_size;
      pattern_ptr->allocatedSize = pattern_size;
      pattern_ptr++;
   }

   array_ptr->patternsNumber = num_patterns;
   *pattern_array = array_ptr;
   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
  Name          : free_pattern_array
 
  Description   : This routine frees a pattern_array.
  
  Arguments     : pattern_array - pointer to the pattern_array to be freed.
                   
  Return Values : NONE
 
  Notes         : NONE
******************************************************************************/
static void
free_pattern_array(SaEvtEventPatternArrayT *pattern_array)
{
   uns32   x;
   SaEvtEventPatternT *pattern_ptr;

   /* Free all the pattern buffers */
   pattern_ptr = pattern_array->patterns;
   for (x=0; x<pattern_array->patternsNumber; x++)
   {
      m_MMGR_FREE_EDSVTM_EVENT_DATA(pattern_ptr->pattern);
      pattern_ptr++;
   }

   /* Now free the pattern structs */
   m_MMGR_FREE_EDSVTM_EVENT_PATTERNS(pattern_array->patterns);
   m_MMGR_FREE_EDSVTM_EVENT_PATTERN_ARRAY(pattern_array);
}

/****************************************************************************
  Name          : dump_event_patterns
 
  Description   : Debug routine to dump contents of
                  an EVT patternArray.
                 
  Arguments     : pattern_array - pointer to the pattern_array to be dumped.
                   
  Return Values : NONE
 
  Notes         : NONE
******************************************************************************/
static void
dump_event_patterns(SaEvtEventPatternArrayT *patternArray)
{
   SaEvtEventPatternT  *pEventPattern;
   int32   x = 0;
   int8    buf[256];

   if (patternArray == NULL)
      return;
   if (patternArray->patterns == 0)
      return;

   pEventPattern = patternArray->patterns; /* Point to first pattern */
   for (x=0; x<(int32)patternArray->patternsNumber; x++) {
      m_NCS_MEMCPY(buf, pEventPattern->pattern, (uns32)pEventPattern->patternSize);
      buf[pEventPattern->patternSize] = '\0';
      m_NCS_CONS_PRINTF("     pattern[%ld] =    {%2u, \"%s\"}\n",
             x, (uns32)pEventPattern->patternSize, buf);
      pEventPattern++;
   }
}

