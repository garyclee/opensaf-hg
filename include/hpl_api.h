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
  This file containts the structures and prototype declarations HPI Private
  Library APIs.

******************************************************************************
*/
#ifndef HPL_API_H
#define HPL_API_H

#include <SaHpi.h> 

#include "ncsgl_defs.h"
#include "ncs_lib.h"
#include "hpl_msg.h"

/* HPL command arguments */
typedef enum hisv_power_args
{
   HISV_RES_POWER_OFF,     /* power OFF the resource */
   HISV_RES_POWER_ON,      /* power ON the resource */
   HISV_RES_POWER_CYCLE    /* power cycle the resource */
} HISV_PWR_ARGS;

typedef enum hisv_reset_args
{
   HISV_RES_COLD_RESET = 0,   /* reset type - cold */
   HISV_RES_WARM_RESET,       /* reset type - warm */
   HISV_RES_RESET_ASSERT,     /* reset type - assertive */
   HISV_RES_RESET_DEASSERT,   /* reset type - de-assertive */
   HISV_RES_GRACEFUL_REBOOT   /* reboot the resource gracefully */
} HISV_RESET_ARGS;

/* HPL API commands */
typedef enum hisv_api_cmd
{
   HISV_RESOURCE_POWER,       /* change the power status of resource */
   HISV_RESOURCE_RESET,     /* reset the resource */
   HISV_CLEAR_SEL,            /* clear the HPI system event log */
   HISV_CHASSIS_ID_GET,        /* get the chassis id of chassis managed by HAM */

   /* HPL Hot swap configuration commands */
   HS_AUTO_INSERT_TIMEOUT_GET,   /* hot swap auto insert timeout get */
   HS_AUTO_INSERT_TIMEOUT_SET,   /* hot swap auto insert timeout set */
   HS_AUTO_EXTRACT_TIMEOUT_GET,  /* hot swap auto extract timeout get */
   HS_AUTO_EXTRACT_TIMEOUT_SET,  /* hot swap auto extract timeout set */
   HS_CURRENT_HS_STATE_GET,      /* hot swap current state get */
   HS_INDICATOR_STATE_GET,       /* hot swap indicator state get */
   HS_INDICATOR_STATE_SET,        /* hot swap indicator state set */

   /* HPL Hot swap manage commands */
   HS_POLICY_CANCEL,             /* hot swap policy cancel */
   HS_RESOURCE_ACTIVE_SET,       /* hot swap activate the resource */
   HS_RESOURCE_INACTIVE_SET,     /* hot swap inactivate the resource */
   HS_ACTION_REQUEST,            /* perform hot swap action request */

   /* HPI Alarm commands */
   HISV_ALARM_ADD,               /* Add Alarm in HPI DAT */
   HISV_ALARM_GET,               /* Get Alarm from HPI DAT */
   HISV_ALARM_DELETE,            /* Delete Alarm from HPI DAT */

   EVENTLOG_TIMEOUT_GET,         /* get the event log time of resource */
   EVENTLOG_TIMEOUT_SET,         /* set the event log time of resource */

   HISV_HAM_HEALTH_CHECK,        /* command to health check the HAM */
   HISV_TMR_SEL_CLR,             /* clear the SEL timely */
   HISV_RESEND_CHASSIS_ID,       /* send the chassis-id after re-discovery */
   HISV_ENTITYPATH_LOOKUP,       /* Entity mapping lookup */
   HISV_BOOTBANK_GET,
   HISV_BOOTBANK_SET,
  /* vivek_hisv */
   HISV_PAYLOAD_BIOS_SWITCH,     /* send the command to switch bios bank   */
   HISV_MAX_API_CMD              /* last in HISV API commands */

} HISV_API_CMD;

/* structure for holding information of each HAM Instance */

typedef struct his_ham_info {
   MDS_DEST  ham_dest;   /* HAM virtual destination address */
   uns32     chassis_id; /* Id of chassis on which HAM instance runs */
   struct his_ham_info *ham_next; /* for making list of HAM instance info */

}HAM_INFO;

/**************************************************************************
 * function declarations
 */

EXTERN_C uns32 ncs_hpl_lib_req (NCS_LIB_REQ_INFO *req_info);
EXTERN_C uns32 hpl_initialize(NCS_LIB_CREATE *create_info);
EXTERN_C uns32 hpl_finalize(NCS_LIB_DESTROY *destroy_info);
EXTERN_C uns32 hpl_resource_reset(uns32 chassis_id, uns8 *entity_path,
                                    uns32 reset_type);
EXTERN_C uns32 hpl_resource_power_set(uns32 chassis_id, uns8 *entity_path,
                                        uns32 power_state);
EXTERN_C uns32 hpl_sel_clear(uns32 chassis_id);
EXTERN_C uns32 hpl_config_hotswap(uns32 chassis_id,
                          HISV_API_CMD hs_config_cmd, uns64 *arg);
EXTERN_C uns32 hpl_config_hs_state_get(uns32 chassis_id, uns8 *entity_path, uns32 *arg);
EXTERN_C uns32 hpl_config_hs_indicator(uns32 chassis_id, uns8 *entity_path,
                                        HISV_API_CMD hs_ind_cmd, uns32 *arg);
EXTERN_C uns32 hpl_config_hs_autoextract(uns32 chassis_id, uns8 *entity_path,
                         HISV_API_CMD hs_config_cmd, uns64 *arg);
EXTERN_C uns32 hpl_manage_hotswap(uns32 chassis_id, uns8 *entity_path,
                                  HISV_API_CMD hs_manage_cmd, uns32 arg);
EXTERN_C uns32 hpl_alarm_add(uns32 chassis_id, HISV_API_CMD alarm_cmd,
                         uns16 arg_len, uns8* arg);
EXTERN_C uns32 hpl_alarm_get(uns32 chassis_id, HISV_API_CMD alarm_cmd, uns32 alarm_id,
                         uns16 arg_len, uns8* arg);
EXTERN_C uns32 hpl_alarm_delete(uns32 chassis_id, HISV_API_CMD alarm_cmd, uns32 alarm_id,
                         uns32 alarm_severity);
EXTERN_C uns32 hpl_event_log_time(uns32 chassis_id, uns8 *entity_path,
                         HISV_API_CMD evlog_time_cmd, uns64 *arg);
EXTERN_C uns32 hpl_chassi_id_get(uns32 *arg);
EXTERN_C uns32 hpl_decode_hisv_evt (HPI_HISV_EVT_T *evt_struct, uns8 *evt_data, uns32 data_len, uns32 version);

EXTERN_C uns32 hpl_bootbank_get (uns32 chassis_id, uns8 *entity_path, uns8 *o_bootbank_number);
EXTERN_C uns32 hpl_bootbank_set (uns32 chassis_id, uns8 *entity_path, uns8 i_bootbank_number);
EXTERN_C uns32 hpl_entity_path_lookup(uns32 flag, uns32 chassis_id, uns32 blade_id, uns8 *entity_path);

#endif /* HPL_API_H */
