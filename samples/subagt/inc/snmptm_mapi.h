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
  MODULE NAME: SNMPTM_MAPI.H 
******************************************************************************/


#ifndef  SNMPTM_MAPI_H
#define  SNMPTM_MAPI_H

#ifdef  __cplusplus
    extern "C" {
#endif

/***********************************************************************
           Object ID enums for  "MCG-NCS-TEST-MIB"  Module
***********************************************************************/

/* Object ID enums for the  "ncsTestScalars"  table */
typedef enum ncs_test_scalars_id 
{
   ncsTestScalarsUnsigned32_ID = 1,
   ncsTestScalarsInteger32_ID = 2,
   ncsTestScalarsIpAddress_ID = 3,
   ncsTestScalarsObsolete_ID = 4,
   ncsTestScalarsForNotify_ID = 5,
   ncsTestScalarsDepricated_ID = 6,
   ncsTestScalarsNotAccessible_ID = 7,
   ncsTestNotificationScalarsObjs_ID = 8,
   ncsTestScalarsMax_ID 
} NCS_TEST_SCALARS_ID; 


/* Object ID enums for the  "ncsTestTableOneEntry"  table */
typedef enum ncs_test_table_one_entry_id 
{
   ncsTestTableOneDisplayString_ID = 1,
   ncsTestTableOnePhysAddress_ID = 2,
   ncsTestTableOneMacAddress_ID = 3,
   ncsTestTableOneTestAndIncr_ID = 4,
   ncsTestTableOneIpAddress_ID = 5,
   ncsTestTableOneGauge32Obsolete_ID = 6,
   ncsTestTableOneCounter32_ID = 7,
   ncsTestTableOneDiscrete_ID = 8,
   ncsTestTableOneRowStatus_ID = 9,
   ncsTestTableOneCounter64_ID = 10,
   ncsTestTableOneGauge32Discrete_ID = 11,
   ncsTestTableOneOctetStringFixedLen_ID = 12,
   ncsTestTableOneOctetStringVarLen_ID = 13,
   ncsTestTableOneOctetStringDiscrete_ID = 14,
   ncsTestTableOneObjectIdentifier_ID = 15,
   ncsTestTableOneEntryMax_ID 
} NCS_TEST_TABLE_ONE_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableTwoEntry"  table */
typedef enum ncs_test_table_two_entry_id 
{
   ncsTestTableTwoUnsigned_ID = 1,
   ncsTestTableTwoRowStatus_ID = 2,
   ncsTestTableTwoCounter64_ID = 3,
   ncsTestTableTwoObjId_ID = 4,
   ncsTestTableTwoEntryMax_ID 
} NCS_TEST_TABLE_TWO_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableThreeEntry"  table */
typedef enum ncs_test_table_three_entry_id 
{
   ncsTestTableThreeIpAddress_ID = 1,
   ncsTestTableThreeRowStatus_ID = 2,
   ncsTestTableThreeEntryMax_ID 
} NCS_TEST_TABLE_THREE_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableFourEntry"  table */
typedef enum ncs_test_table_four_entry_id 
{
   ncsTestTableFourIndex_ID = 1,
   ncsTestTableFourUInteger_ID = 2,
   ncsTestTableFourRowStatus_ID = 3,
   ncsTestTableFourEntryMax_ID 
} NCS_TEST_TABLE_FOUR_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableFiveEntry"  table */
typedef enum ncs_test_table_five_entry_id 
{
   ncsTestTableFiveIpAddress_ID = 1,
   ncsTestTableFiveRowStatus_ID = 2,
   ncsTestTableFiveEntryMax_ID 
} NCS_TEST_TABLE_FIVE_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableSixEntry"  table */
typedef enum ncs_test_table_six_entry_id 
{
   ncsTestTableSixCount_ID = 1,
   ncsTestTableSixData_ID = 2,
   ncsTestTableSixName_ID = 3,
   ncsTestTableSixEntryMax_ID 
} NCS_TEST_TABLE_SIX_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableSevenEntry"  table */
typedef enum ncs_test_table_seven_entry_id 
{
   ncsTestTableSevenStringOne_ID = 1,
   ncsTestTableSevenStringTwo_ID = 2,
   ncsTestTableSevenStringLength_ID = 3,
   ncsTestTableSevenEntryMax_ID 
} NCS_TEST_TABLE_SEVEN_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableEightEntry"  table */
typedef enum ncs_test_table_eight_entry_id 
{
   ncsTestTableEightUnsigned32_ID = 1,
   ncsTestTableEightRowStatus_ID = 2,
   ncsTestTableEightEntryMax_ID 
} NCS_TEST_TABLE_EIGHT_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableNineEntry"  table */
typedef enum ncs_test_table_nine_entry_id 
{
   ncsTestTableNineUnsigned32_ID = 1,
   ncsTestTableNineEntryMax_ID 
} NCS_TEST_TABLE_NINE_ENTRY_ID; 


/* Object ID enums for the  "ncsTestTableTenEntry"  table */
typedef enum ncs_test_table_ten_entry_id 
{
   ncsTestTableTenUnsigned32_ID = 1,
   ncsTestTableTenInt_ID = 2,
   ncsTestTableTenEntryMax_ID 
} NCS_TEST_TABLE_TEN_ENTRY_ID; 


/* Object ID enums for the  "ncsTestNotifications"  table */
typedef enum ncs_test_notifications_id 
{
   ncsTestNotificationTableObjs_ID = 1,
   ncsTestNotificationCombo_ID = 2,
   ncsTestNotificationObsolete_ID = 3,
   ncsTestNotificationsMax_ID 
} NCS_TEST_NOTIFICATIONS_ID; 

#ifdef  __cplusplus
}
#endif


#endif 

