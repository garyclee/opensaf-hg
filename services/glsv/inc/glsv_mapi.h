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
  MODULE NAME: GLSV_MAPI.H 
******************************************************************************/


#ifndef  GLSV_MAPI_H
#define  GLSV_MAPI_H

#ifdef  __cplusplus
     extern "C" {
#endif

/***********************************************************************
           Object ID enums for  "SAF-LCK-SVC-MIB"  Module
***********************************************************************/

/* Object ID enums for the  "safLckScalarObject"  table */
typedef enum saf_lck_scalar_object_id 
{
   safSpecVersion_ID = 1,
   safAgentVendor_ID = 2,
   safAgentVendorProductRev_ID = 3,
   safServiceStartEnabled_ID = 4,
   safServiceState_ID = 5,
   safLckScalarObjectMax_ID 
} SAF_LCK_SCALAR_OBJECT_ID; 


/* Object ID enums for the  "saLckResourceEntry"  table */
typedef enum sa_lck_resource_entry_id 
{
   saLckResourceResourceName_ID = 1,
   saLckResourceCreationTimeStamp_ID = 2,
   saLckResourceNumUsers_ID = 3,
   saLckResourceIsOrphaned_ID = 4,
   saLckResourceStrippedCount_ID = 5,
   saLckResourceEntryMax_ID 
} SA_LCK_RESOURCE_ENTRY_ID; 

#ifdef  __cplusplus
}
#endif


#endif 

