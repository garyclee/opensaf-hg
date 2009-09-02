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
 
  DESCRIPTION:This file contains data definitions for common data types
  ..............................................................................

 
******************************************************************************
*/

#ifndef __AVM_DEFS_H__
#define __AVM_DEFS_H__

#define NCS_AVM_LOG       1
#define AVM_VCARD_ID      1
#define AVM_EDA_FAILURE   3

/*Pattern Array length for Fault, Non Fault handle */
#define AVM_FAULT_FILTER_ARRAY_LEN        2
#define AVM_NON_FAULT_FILTER_ARRAY_LEN    4
#define MAX_INVENTORY_DATA_BUFFER         2000

#define NCS_AVM_NAME       "AVM"
#define NCS_AVM_PRIORITY   0
#define NCS_AVM_STACK_SIZE NCS_STACKSIZE_HUGE

#define AVM_EDA_DONE       1
#define AVM_EDA_NOT_DONE   2
#define AVM_EDA_FAILURE    3

#define AVM_MAX_PATTERNS         10
#define AVM_MAX_PATTERN_SIZE    128

#define AVM_ENT_IS_NODE_INVALID       2
#define AVM_ENT_IS_NODE_TRUE          1
#define AVM_ENT_IS_NODE_FALSE         0

#define AVM_ENT_IS_RESET_INVALID     2
#define AVM_ENT_IS_RESET_TRUE        1
#define AVM_ENT_IS_RESET_FALSE       0

#define AVM_MAX_SENSOR_COUNT         10
#define AVM_MAX_INDEX_LEN            256
#define AVM_MAX_EP_OCTET_STRING      100
#define AVM_HPI_EVT_RETENTION_TIME   ((SaTimeT)9000000000LL)
#define AVM_SWITCH_TRAP_RETENTION_TIME ((SaTimeT)3000000000LL)
#define AVM_HPI_EVT_CHANNEL_NAME     "safChnl=saChnlEvents,safApp=safEvtService"
#define AVM_SAF_VERSION              ((SaVersionT){'B', 1, 1})

#define AVM_FAULT_EVENTS_SUB_ID      13
#define AVM_NON_FAULT_EVENTS_SUB_ID  23
#define AVM_MAX_LOCATION_RANGE       14
#define AVM_MAX_ENTITIES             14
#define AVM_DISPLAY_TIME_LENGTH       8

typedef enum avm_config_state_type {
	AVM_CONFIG_DONE = 1,
	AVM_CONFIG_NOT_DONE
} AVM_CONFIG_STATE_T;

typedef enum avm_valid_state_type {
	AVM_VALID_INFO_DONE = 0,
	AVM_VALID_INFO_NOT_DONE
} AVM_VALID_INFO_STATE_T;

typedef enum cold_sync_state {
	AVM_COLD_SYNC_DONE = 0,
	AVM_COLD_SYNC_NOT_DONE
} AVM_COLD_SYNC_T;

typedef enum avm_ha_state_type {
	AVM_SA_AMF_HA_QUIECING = 0,
	AVM_SA_AMF_HA_INVALID
} AVM_HA_TRANSIT_STATE_T;

typedef enum avm_destroy_type {
	AVM_DESTROY_CB,
	AVM_DESTROY_PATRICIA,
	AVM_DESTROY_MBX,
	AVM_DESTROY_TASK,
	AVM_DESTROY_MDS,
	AVM_DESTROY_MAB,
	AVM_DESTROY_EDA,
	AVM_DESTROY_HPL
} AVM_DESTROY_T;

typedef enum {
	AVM_TBL_RANK_ENT_DEPLOY = 12,
	AVM_TBL_RANK_MAX
} AVM_TBL_RANK;

typedef enum {
	AVM_SWITCH_SUCCESS = 1,
	AVM_SWITCH_FAILURE = 2
} AVM_SWITCH_STATUS_T;

typedef struct avm_ent_path_str_type {
	uns32 length;
	uns8 name[AVM_MAX_INDEX_LEN];
} AVM_ENT_PATH_STR_T;

typedef struct avm_cb AVM_CB_T;
typedef struct avm_ent AVM_ENT_INFO_T;
typedef struct avm_node AVM_NODE_INFO_T;
typedef struct avm_tmr AVM_TMR_T;
typedef struct avm_evt AVM_EVT_T;

#endif   /*__AVM_DEFS_H__ */
