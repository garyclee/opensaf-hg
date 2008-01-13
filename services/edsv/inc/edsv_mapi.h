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
  MODULE NAME: EDSV_MAPI.H 
******************************************************************************/


#ifndef  EDSV_MAPI_H
#define  EDSV_MAPI_H

#ifdef  __cplusplus
    extern "C" {
#endif

/***********************************************************************
           Object ID enums for  "SAF-EVENT-SVC-MIB"  Module
***********************************************************************/

/* Object ID enums for the  "safEvtScalarObject"  table */
typedef enum saf_evt_scalar_object_id 
{
   safSpecVersion_ID = 1,
   safAgentVendor_ID = 2,
   safAgentVendorProductRev_ID = 3,
   safServiceStartEnabled_ID = 4,
   safServiceState_ID = 5,
   safEvtScalarObjectMax_ID 
} SAF_EVT_SCALAR_OBJECT_ID; 


/* Object ID enums for the  "saEvtChannelEntry"  table */
typedef enum sa_evt_channel_entry_id 
{
   saEvtChannelName_ID = 1,
   saEvtChannelCreationTimeStamp_ID = 2,
   saEvtChannelNumUsers_ID = 3,
   saEvtChannelNumSubscribers_ID = 4,
   saEvtChannelNumPublishers_ID = 5,
   saEvtChannelNumRetainedEvents_ID = 6,
   saEvtChannelLostEventsEventCount_ID = 7,
   saEvtChannelEntryMax_ID 
} SA_EVT_CHANNEL_ENTRY_ID; 


/* Object ID enums for the  "saEvtTrapObject"  table */
typedef enum sa_evt_trap_object_id 
{
   saEvtSvcUserName_ID = 1,
   saEvtErrorCode_ID = 2,
   saEvtTrapObjectMax_ID 
} SA_EVT_TRAP_OBJECT_ID; 


/* Object ID enums for the  "saEvtTraps"  table */
typedef enum sa_evt_traps_id 
{
   saEvtAlarmServiceImpaired_ID = 1,
   saEvtAlarmSubscriberOverloaded_ID = 2,
   saEvtAlarmEventChannelOverloaded_ID = 3,
   saEvtTrapsMax_ID 
} SA_EVT_TRAPS_ID; 

#ifdef  __cplusplus
}
#endif


#endif 

