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

/*****************************************************************************

  DESCRIPTION:

  This file contains EDSv sample application. It demonstrates the
  following:
  a) usage of EVT APIs.
  b) certain EDSv features:
      i)  publish/subscribe mechanism for events.
      ii) Retention timer usage with published event.
..............................................................................

******************************************************************************
*/

/* EDSv Toolkit header file */
#include "edsv_demo_app.h"


/*############################################################################
                            Global Variables
############################################################################*/
/* EVT Handle */
SaEvtHandleT          gl_evt_hdl = 0;

/* Channel created/opened in the demo */
SaNameT               gl_chan_name = {40, "safChnl=DemoChannel,safApp=safEvtService"};

/* Channel handle of the channel that'll be used */
SaEvtChannelHandleT   gl_chan_hdl = 0;

/* Event Handle of the event that'll be published */
SaEvtEventHandleT     gl_chan_pub_event_hdl = 0;

/* Event id of the to be published event */
SaEvtEventIdT         gl_chan_pub_event_id = 0;

/* subscription id of the installed subscription */
SaEvtSubscriptionIdT  gl_subid = 19428;

/* Name of the event publisher */
SaNameT               gl_pubname = {11,"EDSvDemokit"};

/* Data content of the event */
unsigned char gl_event_data[64] = "  I am a TRAP event";

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
static unsigned int alloc_pattern_array(SaEvtEventPatternArrayT **pattern_array,
                                 unsigned int num_patterns, 
                                 unsigned int  pattern_size);

/* Utility routine to free a pattern array */
static void free_pattern_array(SaEvtEventPatternArrayT *pattern_array);

/* Utility routine to dump a pattern array */
static void dump_event_patterns(SaEvtEventPatternArrayT *pattern_array);



/*#############################################################################
                           End Of Declarations
###############################################################################*/

/****************************************************************************
  Name          : ncs_edsv_run
 
  Description   : This routine runs the EDSV toolkit demo.
 
  Arguments     : None.
 
  Return Values : 0/1
 
  Notes         : This demo involves a single process 
                  acting as a subscriber and a publisher.
                  The process first subscribes the publishes
                  and obatins the event. It then displays the
                  attributes obtained.
                  
******************************************************************************/
unsigned int  ncs_edsv_run(void)
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
   struct pollfd fds[1];

   /*#########################################################################
                     Demonstrating the usage of saEvtInitialize()
   #########################################################################*/

   /* Fill the callbacks that are to be registered with EVT */
   memset(&reg_callback_set, 0, sizeof(SaEvtCallbacksT));

   reg_callback_set.saEvtChannelOpenCallback  = edsv_chan_open_callback;
   reg_callback_set.saEvtEventDeliverCallback = edsv_evt_delv_callback;

   /* Fill the EVT version */
   m_EDSV_EVT_VER_SET(ver);

   if (SA_AIS_OK != (rc = saEvtInitialize(&gl_evt_hdl, &reg_callback_set, &ver)))
   {
      printf("\n EDSv: EDA: SaEvtInitialize() failed. rc=%d \n",rc);
      return (1);
   }

   printf(
      "\n EDSv: EDA: EVT Initialization Done !!! \n EvtHandle: %x \n", (unsigned int )gl_evt_hdl);

   /*#########################################################################
                  Demonstrating the usage of saEvtSelectionObjectGet()
   #########################################################################*/

   if (SA_AIS_OK != (rc = saEvtSelectionObjectGet(gl_evt_hdl, &evt_sel_obj)))
   {
      printf("\n EDSv: EDA: SaEvtSelectionObjectGet() failed. rc=%d \n",rc);
      return (1);
   }

   printf("\n EDSv: EDA: Obtained Selection Object Successfully !!! \n");

  /*#########################################################################
                  Demonstrating the usage of saEvtChannelOpen()
   #########################################################################*/

   sleep(1);

   chan_open_flags = 
      SA_EVT_CHANNEL_CREATE|SA_EVT_CHANNEL_SUBSCRIBER|SA_EVT_CHANNEL_PUBLISHER;

   if (SA_AIS_OK != 
         (rc = 
            saEvtChannelOpen(gl_evt_hdl, &gl_chan_name, chan_open_flags, timeout, &gl_chan_hdl)))
   {
      printf("\n EDSv: EDA: SaEvtChannelOpen() failed. rc=%d \n",rc);
      return (1);
   }

   printf("\n EDSv: Opened DEMO Channel: %s Successfully !!! \n",gl_chan_name.value);

  /*#########################################################################
                  Demonstrating the usage of SaEvtEventSubscribe()
   #########################################################################*/
   sleep(2);

   filter_array.filtersNumber = TRAP_FILTER_ARRAY_LEN;
   filter_array.filters = gl_trap_filter_array;
   if (SA_AIS_OK != (rc = saEvtEventSubscribe(gl_chan_hdl, &filter_array, gl_subid)))
   {
      printf("\n EDSv: EDA: SaEvtEventSubscribe() failed. rc=%d \n",rc);
      return (1);
   }

   printf("\n EDSv: Subscribed for events on DEMO channel as Both a Publisher and Subscriber !!! \n");

   /*#########################################################################
                  Demonstrating the usage of saEvtEventAllocate()
   #########################################################################*/
   sleep(2);

   if (SA_AIS_OK != (rc = saEvtEventAllocate(gl_chan_hdl, &gl_chan_pub_event_hdl)))
   {
      printf("\n EDSv: EDA: SaEvtEventAllocate() failed. rc=%d \n", rc);
      return (1);
   }

   printf("\n EDSv: Allocated an Event Successfully !!! \n");

   /*#########################################################################
                  Demonstrating the usage of saEvtEventAttributesSet()
   #########################################################################*/
   sleep(2);

   pattern_array.patternsNumber = TRAP_PATTERN_ARRAY_LEN;
   pattern_array.patterns = gl_trap_pattern_array;
   if (SA_AIS_OK != (rc = saEvtEventAttributesSet(gl_chan_pub_event_hdl, 
                                          &pattern_array,
                                          priority, 
                                          retention_time, 
                                          &gl_pubname)))
   {
      printf("\n EDSv: EDA: SaEvtEventAttributesSet() failed. rc=%d \n", rc);
      return (1);
   }

   printf("\n EDSv: Set the attributes for the event successfully !!! \n");

   /*#########################################################################
                  Demonstrating the usage of saEvtEventPublish()
   #########################################################################*/
   sleep(2);

   rc = saEvtEventPublish(gl_chan_pub_event_hdl,
                  &gl_event_data[0],
                  strlen((char *)gl_event_data),
                  &gl_chan_pub_event_id);

   if (rc != SA_AIS_OK)
      printf("\n EDSv: EDA: SaEvtEventPublish() failed. rc=%d\n", rc);

   printf(
      "\n EDSv: Published event on demo channel successfully !!!, event_id = %llu \n",gl_chan_pub_event_id);

   
   /***** Now wait (select) on EVT selction object *****/

   fds[0].fd = evt_sel_obj;
   fds[0].events = POLLIN;
 
   while (1)
   {
      int res = poll(fds,1,-1);
      if (res == -1) {
              if (errno == EINTR)
                      continue;
              else {
                     printf("Poll Failed - %s\n",strerror(errno));
                     exit(1);
              }
     }
      /* Process EDSv evt messages */
      if (fds[0].revents & POLLIN) {
         /* Dispatch all pending messages */
         printf("\n EDSv: EDA: Dispatching message received on demo channel\n");
         
         /*######################################################################
                        Demonstrating the usage of saEvtDispatch()
         ######################################################################*/
         rc = saEvtDispatch(gl_evt_hdl, SA_DISPATCH_ALL);

         if (rc != SA_AIS_OK)
            printf("\n EDSv: EDA: SaEvtDispatch() failed. rc=%d \n", rc);
         
         /* Rcvd the published event, now escape */
         break;
      }

   }

   /*######################################################################
                        Demonstrating the usage of saEvtEventFree()
   ########################################################################*/
   sleep(4);

   if (SA_AIS_OK != (rc = saEvtEventFree(gl_chan_pub_event_hdl)))
   {
      printf("\n EDSv: EDA: SaEvtEventFree() failed. rc=%d\n", rc);
      return (1);
   }

   printf("\n EDSv: Freed the event that I allocated as a Publisher !!! \n");

   /*##########################################################################
                        Demonstrating the usage of saEvtEventUnsubscribe()
   ############################################################################*/
   sleep(2);

   if (SA_AIS_OK != (rc = saEvtEventUnsubscribe(gl_chan_hdl, gl_subid)))
   {
      printf("\n EDSv: EDA: SaEvtEventUnsubscribe() failed. rc=%d \n",rc);
      return (1);
   }

   printf("\n EDSv: Unsubscribed for events Successfully !!! \n");

   /*###########################################################################
                        Demonstrating the usage of saEvtChannelClose()
   #############################################################################*/
   sleep(2);

   if (SA_AIS_OK != (rc = saEvtChannelClose(gl_chan_hdl)))
   {
      printf("\n EDSv: EDA: SaEvtChannelClose() failed. rc=%d\n",rc);
      return (1);
   }

   printf("\n EDSv: Closed DEMO channel Successfully !!! \n");

   /*###########################################################################
                        Demonstrating the usage of SaEvtFinalize()
   #############################################################################*/
   sleep(2);

   rc = saEvtFinalize(gl_evt_hdl);

   if (rc != SA_AIS_OK)
   {
      printf("\n EDSv: EDA: SaEvtFinalize() failed. rc=%d\n", rc);
      return (1);
   }

   printf("\n EDSv: Finalized with event service successfully !!! \n");

   return 0;

}

/****************************************************************************
  Name          : edsv_chan_open_callback
 
  Description   : This routine is a callback to notify
                  about channel opens. It is specified as a part of EVT initialization.
                  
 
  Arguments     : inv             - particular invocation of this callback 
                                    function
                  chan_hdl        - hdl of the channel which was opened.
                  rc              - return code.
 
  Return Values : 0/1
 
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
 
  Return Values : 0/1
 
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
   unsigned int                      rc;
   unsigned int                      num_patterns;
   unsigned int                      pattern_size;
   

   /* Prepare an appropriate-sized data buffer.
    */
   data_len = event_data_size;
   p_data = malloc(data_len+1);
   if (p_data == NULL)
      return;
   memset(&publisher_name,'\0',sizeof(SaNameT));
   memset(p_data, 0, (size_t)data_len+1);

   /* Create an empty patternArray to be filled in by EDA */
   num_patterns = 8;
   pattern_size = 128;

   if (0 != 
         (rc = alloc_pattern_array(&pattern_array, num_patterns, pattern_size)))
   {
      printf("\n EDSv: EDA: SaEvtEventAttributesGet() failed. rc=%d\n",rc);
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
      printf("\n EDSv: EDA: SaEvtEventAttributesGet() failed. rc=%d\n",rc);
      return;
   }

   printf("\n EDSv: Got Attributes of the Events Successfully !!! \n");

  /*#############################################################################
                 Demonstrating the usage of SaEvtEventDataGet()
   #############################################################################*/

   /* Get the event data */
   rc = saEvtEventDataGet(event_hdl, p_data, &data_len);
   if (rc != SA_AIS_OK)
   {
      printf("\n EDSv: EDA: SaEvtEventDataGet() failed. rc=%d\n",rc);
      return;
   }

   printf("\n EDSv: Got Data from the received event successfully !!! \n");


   /* Say what we received */
   printf("\n-------------- Received Event --------------------------\n");
   printf(" publisherName  =    %s\n", publisher_name.value);
   printf(" patternArray\n");
   dump_event_patterns(pattern_array);
   printf(" priority       =    %d\n", priority);
   printf(" publishTime    =    %llu\n", (unsigned long long)publish_time);
   printf(" retentionTime  =    %llu\n", (unsigned long long)retention_time);
   printf(" eventId        =    %llu\n", (unsigned long long)event_id);
   printf(" dataLen        =    %d\n", (unsigned int )data_len);
   printf(" data           =    %s\n", p_data);
   printf("---------------------------------------------------------\n\n");

  /*#############################################################################
               Demonstrating the usage of saEvtEventRetentionTimeClear()
   #############################################################################*/
   printf("Press <Enter> key to continue\n");	
   getchar();
   /* Test clearing the retention timer */
   if (0 != retention_time)
   {
      rc = saEvtEventRetentionTimeClear(gl_chan_hdl, event_id);
      if (rc != SA_AIS_OK)
      {
         printf("\n EDSv: EDA: saEvtEventRetentionTimeClear() failed. rc=%d\n",rc);
         goto done;
      }

      printf("\n EDSv: Cleared retention timer Successfully !!! \n");
   }

done:
   
   /* Free data storage */
   if (p_data != NULL)
      free(p_data);

   /* Free the rcvd event */
   rc =  saEvtEventFree(event_hdl);
   if (rc != SA_AIS_OK)
      printf("\n EDSv: EDA: SaEvtEventFree() failed. rc=%d\n",rc);
   else
      printf("\n EDSv: Freed the Event that I recieved as a Subscriber \n");

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
 
  Return Values : 0/1
 
  Notes         : NONE
******************************************************************************/
static unsigned int 
alloc_pattern_array(SaEvtEventPatternArrayT **pattern_array,
                    unsigned int  num_patterns,
                    unsigned int  pattern_size)
{
   unsigned int    x;
   SaEvtEventPatternArrayT  *array_ptr;
   SaEvtEventPatternT       *pattern_ptr;

   array_ptr = (SaEvtEventPatternArrayT *)malloc(sizeof(SaEvtEventPatternArrayT));
   if (array_ptr == NULL)
      return(1);

   array_ptr->patterns = (SaEvtEventPatternT *)malloc(num_patterns*sizeof(SaEvtEventPatternT));
   if (array_ptr->patterns == NULL)
      return(1);

   pattern_ptr = array_ptr->patterns;
   for (x=0; x<num_patterns; x++)
   {
      pattern_ptr->pattern = malloc(pattern_size);
      if (pattern_ptr->pattern == NULL)
         return(1);

      pattern_ptr->patternSize = pattern_size;
      pattern_ptr->allocatedSize = pattern_size;
      pattern_ptr++;
   }

   array_ptr->patternsNumber = num_patterns;
   *pattern_array = array_ptr;
   return(0);
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
   unsigned int    x;
   SaEvtEventPatternT *pattern_ptr;

   /* Free all the pattern buffers */
   pattern_ptr = pattern_array->patterns;
   for (x=0; x<pattern_array->patternsNumber; x++)
   {
      free(pattern_ptr->pattern);
      pattern_ptr++;
   }

   /* Now free the pattern structs */
   free(pattern_array->patterns);
   free(pattern_array);
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
   int   x = 0;
   char    buf[256];

   if (patternArray == NULL)
      return;
   if (patternArray->patterns == 0)
      return;

   pEventPattern = patternArray->patterns; /* Point to first pattern */
   for (x=0; x<(int)patternArray->patternsNumber; x++) {
      memcpy(buf, pEventPattern->pattern, (unsigned int )pEventPattern->patternSize);
      buf[pEventPattern->patternSize] = '\0';
      printf(" pattern[%d] =    {%2u, \"%s\"}\n",
             x, (unsigned int )pEventPattern->patternSize, buf);
      pEventPattern++;
   }
}

