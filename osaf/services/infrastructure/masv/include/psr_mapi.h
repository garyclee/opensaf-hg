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

  DESCRIPTION:

 PSR MIB Tables Definitions

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef PSR_MAPI_H
#define PSR_MAPI_H

/* This enum is for the param id of the scalar TriggerSave */
typedef enum psr_sclr_obj_id {
	ncsPSSvExistingProfile_ID = 1,
	ncsPSSvNewProfile_ID,
	ncsPSSvTriggerOperation_ID,
	ncsPSSvCurrentProfile_ID,
	ncsPSSvScalarsMax_ID
} PSR_SCLR_OBJ_ID;

/* This enum defines the possible values that the above-defined scalar can take. */
typedef enum psr_trigger_values {
	ncsPSSvTriggerNoop = 0,
	ncsPSSvTriggerLoad,
	ncsPSSvTriggerSave,
	ncsPSSvTriggerCopy,
	ncsPSSvTriggerRename,
	ncsPSSvTriggerReplace,
	ncsPSSvTriggerReloadLibConf,
	ncsPSSvTriggerReloadSpcnList
} PSR_TRIGGER_VALUES;

/* This enum defines the parameter ids for the table NCSMIB_TBL_PSR_PROFILES */
typedef enum psr_profile_tbl_obj_id {
	ncsPSSvProfileName_ID = 1,
	ncsPSSvProfileDesc_ID,
	ncsPSSvProfileTableStatus_ID,
	ncsPSSvProfileTableEntryMax_ID
} PSR_PROFILE_TBL_OBJ_ID;

typedef enum {
	PSS_CMD_TBL_CMD_NUM_SET_XML_OPTION,
	PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB,
	PSS_CMD_TBL_CMD_NUM_DISPLAY_PROFILE,
	PSS_CMD_TBL_CMD_NUM_MAX
} PSS_CMD_TBL_CMD_NUM;

#endif   /* PSR_MAPI_H */
