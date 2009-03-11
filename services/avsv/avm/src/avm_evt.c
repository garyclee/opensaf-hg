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
 
  DESCRIPTION:This module deals with handling of all events in all states they
  occur. 
  ..............................................................................

  Function Included in this Module: Too many to list.
 
 
******************************************************************************
*/


#include "avm.h"


static uns32
avm_assign_state(
                  AVM_ENT_INFO_T    *ent_info,
                  AVM_FSM_STATES_T  next_state
                );


static uns32 avm_ins_pend(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_not_pres_inactive(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt);
static uns32 avm_not_pres_active(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_not_pres_avd_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_not_pres_adm_shutdown(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_not_pres_adm_lock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_not_pres_adm_unlock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_not_pres_resource_rest(AVM_CB_T *, AVM_ENT_INFO_T *, void *);
static uns32 avm_not_pres_ext_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt);

static uns32 avm_inactive_active(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_extracted(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_sensor_assert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_sensor_deassert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_inactive(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_adm_shutdown(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_adm_lock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_adm_unlock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_inactive_resource_fail(AVM_CB_T *, AVM_ENT_INFO_T *, void *);
static uns32 avm_inactive_resource_rest(AVM_CB_T *, AVM_ENT_INFO_T *, void *);
            
static uns32 avm_active_extract_pend(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
/*static uns32 avm_active_inactive(AVM_CB_T*, AVM_ENT_INFO_T*, void*);*/
static uns32 avm_active_extracted(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_inactive(AVM_CB_T*, AVM_ENT_INFO_T*, void*);

static uns32 avm_active_surp_extract(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_sensor_assert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_sensor_deassert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_handle_fw_progress_event(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_node_shutdown_resp(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_avd_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_adm_soft_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_adm_hard_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
/*static uns32 avm_active_failover_resp(AVM_CB_T*, AVM_ENT_INFO_T*, void*);*/
static uns32 avm_active_adm_shutdown(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_adm_lock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_adm_unlock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_active_resource_fail(AVM_CB_T *, AVM_ENT_INFO_T *, void *);
 
static uns32 avm_faulty_extract_pend(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_surp_extract(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_sensor_assert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_sensor_deassert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_node_failover_resp(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_adm_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_avd_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_adm_shutdown(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_faulty_adm_lock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);

static uns32 avm_reset_req_extract_pend(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_surp_extract(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_sensor_assert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_sensor_deassert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_node_shutdown_resp(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_node_failover_resp(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_avd_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_resource_fail(AVM_CB_T *, AVM_ENT_INFO_T *, void *);
static uns32 avm_ext_req_adm_soft_reset_req(AVM_CB_T *, AVM_ENT_INFO_T *, void *);
static uns32 avm_reset_req_adm_hard_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_adm_shutdown(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_adm_lock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_adm_unlock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_extracted(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_reset_req_inactive(AVM_CB_T*, AVM_ENT_INFO_T*, void*);

static uns32 avm_ext_req_active(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_surp_extract(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_sensor_assert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_sensor_deassert(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_node_shutdown_resp(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_avd_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_adm_soft_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_adm_hard_reset_req(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_adm_shutdown(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_adm_lock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_adm_unlock(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_inactive(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_extracted(AVM_CB_T*, AVM_ENT_INFO_T*, void*);
static uns32 avm_ext_req_resource_fail(AVM_CB_T *, AVM_ENT_INFO_T *, void *);

AVM_FSM_TABLE_T avm_fsm_table_g[AVM_ENT_MAX_STATE + 1][AVM_EVT_MAX + 1] = 
{
   {
      {AVM_ENT_NOT_PRESENT, AVM_EVT_FRU_INSTALL, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_INSERTION_PENDING, avm_ins_pend},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ENT_ACTIVE, avm_not_pres_active},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_EXTRACTION_PENDING, avm_not_pres_ext_req},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ENT_EXTRACTED, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ENT_INACTIVE,  avm_not_pres_inactive},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_SURP_EXTRACTION, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_SENSOR_ASSERT, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_SENSOR_DEASSERT,  avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_SENSOR_FW_PROGRESS,  avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_RESOURCE_FAIL,       avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT,  AVM_EVT_RESOURCE_REST,       avm_not_pres_resource_rest},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_FAULT_DOMAIN_REQ, avm_handle_ignore_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_NODE_SHUTDOWN_RESP, avm_handle_ignore_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_NODE_FAILOVER_RESP, avm_handle_ignore_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_AVD_RESET_REQ, avm_not_pres_avd_reset_req},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ADM_SOFT_RESET_REQ, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ADM_HARD_RESET_REQ, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_MIB_REQ, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_TMR_INIT_EDA_EXP, avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ADM_SHUTDOWN, avm_not_pres_adm_shutdown},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ADM_LOCK,   avm_not_pres_adm_lock},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_ADM_UNLOCK, avm_not_pres_adm_unlock},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_IGNORE,  avm_handle_invalid_event},
      {AVM_ENT_NOT_PRESENT, AVM_EVT_INVALID, avm_handle_invalid_event}
   },
   {
      {AVM_ENT_INACTIVE, AVM_EVT_FRU_INSTALL,  avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_INSERTION_PENDING,  avm_ins_pend   },
      {AVM_ENT_INACTIVE, AVM_EVT_ENT_ACTIVE,  avm_inactive_active   },
      {AVM_ENT_INACTIVE, AVM_EVT_EXTRACTION_PENDING, avm_handle_invalid_event },
      {AVM_ENT_INACTIVE, AVM_EVT_ENT_EXTRACTED,   avm_inactive_extracted },
      {AVM_ENT_INACTIVE, AVM_EVT_ENT_INACTIVE, avm_inactive_inactive  },
      {AVM_ENT_INACTIVE, AVM_EVT_SURP_EXTRACTION, avm_handle_ignore_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_SENSOR_ASSERT,  avm_inactive_sensor_assert  },
      {AVM_ENT_INACTIVE, AVM_EVT_SENSOR_DEASSERT, avm_inactive_sensor_deassert},
      {AVM_ENT_INACTIVE, AVM_EVT_SENSOR_FW_PROGRESS,  avm_handle_invalid_event},
      {AVM_ENT_INACTIVE, AVM_EVT_RESOURCE_FAIL,       avm_inactive_resource_fail},
      {AVM_ENT_INACTIVE, AVM_EVT_RESOURCE_REST,       avm_inactive_resource_rest},
      {AVM_ENT_INACTIVE, AVM_EVT_FAULT_DOMAIN_REQ, avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_NODE_SHUTDOWN_RESP, avm_handle_invalid_event},
      {AVM_ENT_INACTIVE, AVM_EVT_NODE_FAILOVER_RESP, avm_handle_invalid_event },
      {AVM_ENT_INACTIVE, AVM_EVT_AVD_RESET_REQ, avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_ADM_SOFT_RESET_REQ, avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_ADM_HARD_RESET_REQ, avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_MIB_REQ,  avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_TMR_INIT_EDA_EXP, avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_ADM_SHUTDOWN,  avm_inactive_adm_shutdown},
      {AVM_ENT_INACTIVE, AVM_EVT_ADM_LOCK,     avm_inactive_adm_lock},
      {AVM_ENT_INACTIVE, AVM_EVT_ADM_UNLOCK, avm_inactive_adm_unlock},
      {AVM_ENT_INACTIVE, AVM_EVT_IGNORE,   avm_handle_invalid_event  },
      {AVM_ENT_INACTIVE, AVM_EVT_INVALID,  avm_handle_invalid_event  }
   },
   {
      {AVM_ENT_ACTIVE,  AVM_EVT_FRU_INSTALL,      avm_handle_invalid_event  },
      {AVM_ENT_ACTIVE,  AVM_EVT_INSERTION_PENDING,  avm_handle_invalid_event  },
      {AVM_ENT_ACTIVE,  AVM_EVT_ENT_ACTIVE,        avm_handle_invalid_event  },
      {AVM_ENT_ACTIVE,  AVM_EVT_EXTRACTION_PENDING,   avm_active_extract_pend },
      {AVM_ENT_ACTIVE,  AVM_EVT_ENT_EXTRACTED,     avm_active_extracted  },
      {AVM_ENT_ACTIVE,  AVM_EVT_ENT_INACTIVE,      avm_active_inactive  },
      {AVM_ENT_ACTIVE,  AVM_EVT_SURP_EXTRACTION,   avm_active_surp_extract   },
      {AVM_ENT_ACTIVE,  AVM_EVT_SENSOR_ASSERT,    avm_active_sensor_assert  },
      {AVM_ENT_ACTIVE,  AVM_EVT_SENSOR_DEASSERT,  avm_active_sensor_deassert},
      {AVM_ENT_ACTIVE,  AVM_EVT_SENSOR_FW_PROGRESS,  avm_handle_fw_progress_event},
      {AVM_ENT_ACTIVE,  AVM_EVT_RESOURCE_FAIL,       avm_active_resource_fail},
      {AVM_ENT_ACTIVE,  AVM_EVT_RESOURCE_REST,       avm_handle_invalid_event},
      {AVM_ENT_ACTIVE,  AVM_EVT_FAULT_DOMAIN_REQ, avm_handle_invalid_event  },
      {AVM_ENT_ACTIVE,  AVM_EVT_NODE_SHUTDOWN_RESP, avm_active_node_shutdown_resp},
      {AVM_ENT_ACTIVE,  AVM_EVT_NODE_FAILOVER_RESP,   avm_handle_ignore_event },
      {AVM_ENT_ACTIVE,  AVM_EVT_AVD_RESET_REQ,  avm_active_avd_reset_req  },
      {AVM_ENT_ACTIVE,  AVM_EVT_ADM_SOFT_RESET_REQ,  avm_active_adm_soft_reset_req  },
      {AVM_ENT_ACTIVE,  AVM_EVT_ADM_HARD_RESET_REQ,  avm_active_adm_hard_reset_req  },
      {AVM_ENT_ACTIVE,  AVM_EVT_MIB_REQ,        avm_handle_invalid_event  },
      {AVM_ENT_ACTIVE,  AVM_EVT_TMR_INIT_EDA_EXP,  avm_handle_invalid_event  },
      {AVM_ENT_ACTIVE,   AVM_EVT_ADM_SHUTDOWN,      avm_active_adm_shutdown},
      {AVM_ENT_ACTIVE,   AVM_EVT_ADM_LOCK,          avm_active_adm_lock},
      {AVM_ENT_ACTIVE,   AVM_EVT_ADM_UNLOCK,        avm_active_adm_unlock},
      {AVM_ENT_ACTIVE,   AVM_EVT_IGNORE,        avm_handle_invalid_event  },
      {AVM_ENT_ACTIVE,  AVM_EVT_INVALID,       avm_handle_invalid_event  }
   },
   {
      {AVM_ENT_FAULTY, AVM_EVT_FRU_INSTALL,     avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_INSERTION_PENDING,  avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_ENT_ACTIVE,         avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_EXTRACTION_PENDING,  avm_faulty_extract_pend   },
      {AVM_ENT_FAULTY, AVM_EVT_ENT_EXTRACTED,    avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_ENT_INACTIVE,     avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_SURP_EXTRACTION,  avm_faulty_surp_extract   },
      {AVM_ENT_FAULTY, AVM_EVT_SENSOR_ASSERT,    avm_faulty_sensor_assert  },
      {AVM_ENT_FAULTY, AVM_EVT_SENSOR_DEASSERT,  avm_faulty_sensor_deassert},
      {AVM_ENT_FAULTY, AVM_EVT_SENSOR_FW_PROGRESS,  avm_handle_invalid_event},
      {AVM_ENT_FAULTY, AVM_EVT_RESOURCE_REST,       avm_handle_invalid_event},
      {AVM_ENT_FAULTY, AVM_EVT_RESOURCE_FAIL,       avm_handle_invalid_event},
      {AVM_ENT_FAULTY, AVM_EVT_FAULT_DOMAIN_REQ,  avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_NODE_SHUTDOWN_RESP, avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_NODE_FAILOVER_RESP, avm_faulty_node_failover_resp},
      {AVM_ENT_FAULTY, AVM_EVT_AVD_RESET_REQ,     avm_faulty_avd_reset_req  },
      {AVM_ENT_FAULTY, AVM_EVT_ADM_SOFT_RESET_REQ,     avm_faulty_adm_reset_req  },
      {AVM_ENT_FAULTY, AVM_EVT_ADM_HARD_RESET_REQ,     avm_faulty_adm_reset_req  },
      {AVM_ENT_FAULTY, AVM_EVT_MIB_REQ,           avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_TMR_INIT_EDA_EXP,  avm_handle_invalid_event  },
      {AVM_ENT_FAULTY,  AVM_EVT_ADM_SHUTDOWN,      avm_faulty_adm_shutdown},
      {AVM_ENT_FAULTY,  AVM_EVT_ADM_LOCK,          avm_faulty_adm_lock},
      {AVM_ENT_FAULTY,  AVM_EVT_ADM_UNLOCK,        avm_handle_invalid_event},
      {AVM_ENT_FAULTY,  AVM_EVT_IGNORE,            avm_handle_invalid_event  },
      {AVM_ENT_FAULTY, AVM_EVT_INVALID,           avm_handle_invalid_event  }
   },
   {
      {AVM_ENT_RESET_REQ, AVM_EVT_FRU_INSTALL,    avm_handle_invalid_event  },
      {AVM_ENT_RESET_REQ, AVM_EVT_INSERTION_PENDING, avm_handle_invalid_event  },
      {AVM_ENT_RESET_REQ, AVM_EVT_ENT_ACTIVE,  avm_handle_invalid_event  },
      {AVM_ENT_RESET_REQ, AVM_EVT_EXTRACTION_PENDING,   avm_reset_req_extract_pend},
      {AVM_ENT_RESET_REQ, AVM_EVT_ENT_EXTRACTED,  avm_reset_req_extracted  },
      {AVM_ENT_RESET_REQ, AVM_EVT_ENT_INACTIVE,   avm_reset_req_inactive  },
      {AVM_ENT_RESET_REQ, AVM_EVT_SURP_EXTRACTION, avm_reset_req_surp_extract},
      {AVM_ENT_RESET_REQ, AVM_EVT_SENSOR_ASSERT,   avm_reset_req_sensor_assert},
      {AVM_ENT_RESET_REQ, AVM_EVT_SENSOR_DEASSERT, avm_reset_req_sensor_deassert},
      {AVM_ENT_RESET_REQ, AVM_EVT_SENSOR_FW_PROGRESS,  avm_handle_invalid_event},
      {AVM_ENT_RESET_REQ, AVM_EVT_RESOURCE_FAIL,       avm_reset_req_resource_fail},
      {AVM_ENT_RESET_REQ, AVM_EVT_RESOURCE_REST,       avm_handle_invalid_event},
      {AVM_ENT_RESET_REQ, AVM_EVT_FAULT_DOMAIN_REQ, avm_handle_invalid_event  },
      {AVM_ENT_RESET_REQ, AVM_EVT_NODE_SHUTDOWN_RESP, avm_reset_req_node_shutdown_resp},
      {AVM_ENT_RESET_REQ, AVM_EVT_NODE_FAILOVER_RESP, avm_reset_req_node_failover_resp},
      {AVM_ENT_RESET_REQ, AVM_EVT_AVD_RESET_REQ, avm_reset_req_avd_reset_req},
      {AVM_ENT_RESET_REQ, AVM_EVT_ADM_SOFT_RESET_REQ, avm_handle_ignore_event  },
      {AVM_ENT_RESET_REQ, AVM_EVT_ADM_HARD_RESET_REQ, avm_reset_req_adm_hard_reset_req},
      {AVM_ENT_RESET_REQ, AVM_EVT_MIB_REQ,       avm_handle_invalid_event  },
      {AVM_ENT_RESET_REQ, AVM_EVT_TMR_INIT_EDA_EXP, avm_handle_invalid_event },
      {AVM_ENT_RESET_REQ, AVM_EVT_ADM_SHUTDOWN,  avm_reset_req_adm_shutdown},
      {AVM_ENT_RESET_REQ,  AVM_EVT_ADM_LOCK,     avm_reset_req_adm_lock},
      {AVM_ENT_RESET_REQ,  AVM_EVT_ADM_UNLOCK,   avm_reset_req_adm_unlock},
      {AVM_ENT_RESET_REQ, AVM_EVT_IGNORE,       avm_handle_invalid_event  },
      {AVM_ENT_RESET_REQ, AVM_EVT_INVALID,      avm_handle_invalid_event  }
   },
   {
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_FRU_INSTALL, avm_handle_invalid_event  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_INSERTION_PENDING, avm_handle_invalid_event  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ENT_ACTIVE, avm_ext_req_active  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_EXTRACTION_PENDING, avm_handle_invalid_event},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ENT_EXTRACTED,  avm_ext_req_extracted  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ENT_INACTIVE,   avm_ext_req_inactive  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_SURP_EXTRACTION, avm_ext_req_surp_extract},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_SENSOR_ASSERT, avm_ext_req_sensor_assert},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_SENSOR_DEASSERT, avm_ext_req_sensor_deassert},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_SENSOR_FW_PROGRESS,  avm_handle_invalid_event},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_RESOURCE_FAIL,       avm_ext_req_resource_fail},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_RESOURCE_REST,       avm_handle_invalid_event},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_FAULT_DOMAIN_REQ, avm_handle_invalid_event  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_NODE_SHUTDOWN_RESP, avm_ext_req_node_shutdown_resp},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_NODE_FAILOVER_RESP, avm_handle_ignore_event},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_AVD_RESET_REQ, avm_ext_req_avd_reset_req},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ADM_SOFT_RESET_REQ, avm_ext_req_adm_soft_reset_req },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ADM_HARD_RESET_REQ, avm_ext_req_adm_hard_reset_req  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_MIB_REQ,  avm_handle_invalid_event  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_TMR_INIT_EDA_EXP, avm_handle_invalid_event  },
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ADM_SHUTDOWN, avm_ext_req_adm_shutdown},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ADM_LOCK,   avm_ext_req_adm_lock},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_ADM_UNLOCK, avm_ext_req_adm_unlock},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_IGNORE,     avm_handle_invalid_event},
      {AVM_ENT_EXTRACT_REQ, AVM_EVT_INVALID,    avm_handle_invalid_event}
   },

   {
      {AVM_ENT_INVALID, AVM_EVT_FRU_INSTALL,    avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_INSERTION_PENDING, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_ENT_ACTIVE,    avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_EXTRACTION_PENDING, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_ENT_EXTRACTED, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_ENT_INACTIVE,  avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_SURP_EXTRACTION,   avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_SENSOR_ASSERT, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_SENSOR_DEASSERT,   avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_SENSOR_FW_PROGRESS,  avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_RESOURCE_FAIL,       avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_RESOURCE_REST,       avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_FAULT_DOMAIN_REQ,  avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_NODE_SHUTDOWN_RESP, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_NODE_FAILOVER_RESP, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_AVD_RESET_REQ, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_ADM_SOFT_RESET_REQ, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_ADM_HARD_RESET_REQ, avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_MIB_REQ,       avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_TMR_INIT_EDA_EXP,   avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_ADM_SHUTDOWN,   avm_handle_invalid_event}, 
      {AVM_ENT_INVALID, AVM_EVT_ADM_LOCK,       avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_ADM_UNLOCK,     avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_IGNORE,         avm_handle_invalid_event},
      {AVM_ENT_INVALID, AVM_EVT_INVALID,        avm_handle_invalid_event}
   }
};

static uns32 
avm_assign_state(
                  AVM_ENT_INFO_T    *ent_info,
                  AVM_FSM_STATES_T   next_state
                )
{
   ent_info->previous_state = ent_info->current_state;
   ent_info->current_state  = next_state;

   return NCSCC_RC_SUCCESS;
}  

extern uns32 
avm_reset_ent_info(AVM_ENT_INFO_T *ent_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 i;
   
   for(i = 0; i < AVM_MAX_SENSOR_COUNT; i++)
   {
      avm_sensor_event_initialize(ent_info, i);
   }
   
   ent_info->sensor_assert = NCSCC_RC_FAILURE;
   return rc;
}   

/***********************************************************************
*****
 * Name          : avm_comp_text_buffers
 *
 * Description   : This function is used to compare two text buffers
 *
 * Arguments     : SaHpiTextBufferT* Pointer to the source buffer  
 *                 SaHpitextBufferT* Ponter to the destination buffer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : The comparison criteria are both the string are suppo
 *                 sed to be exactly same.  
 ***********************************************************************
******/
  
static uns32
avm_comp_text_buffers(SaHpiTextBufferT *src, SaHpiTextBufferT *dst)
{
   
   if((dst->DataLength == src->DataLength) && (!m_NCS_MEMCMP(dst->Data, src->Data, src->DataLength)))
   {
      return NCSCC_RC_SUCCESS;
   }
   else
   {
      return NCSCC_RC_FAILURE;
   }      
}

/***********************************************************************
*****
 * Name          : avm_validate_fru
 *
 * Description   : This function is used to compare the Validation Info-
 *                 mation given in the Deployment BOM and the actual FRU
 *                 data received from HiSV through EDSv
 *
 * Arguments     : AVM_ENT_INFO - Pointer to the entity to which Insert
 *                 ion Pending Event has been received at AvM.
 *                 HPI_EVT_T    - Pointer to the HPI.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
******/

static uns32 
avm_validate_fru(AVM_ENT_INFO_T *ent_info, HPI_EVT_T *fsm_evt)
{
   /* if validation not required, return success */
   if (ent_info->act_policy == AVM_SELF_ACTIVATION)
      return NCSCC_RC_SUCCESS;
   
   if(NCSCC_RC_SUCCESS == avm_comp_text_buffers(&ent_info->valid_info->inv_data.product_name, &fsm_evt->inv_data.product_name))
   {
      if(NCSCC_RC_SUCCESS == avm_comp_text_buffers(&ent_info->valid_info->inv_data.product_version, &fsm_evt->inv_data.product_version))
      {
         return NCSCC_RC_SUCCESS;
      }else
      {
         m_AVM_LOG_GEN_EP_STR("Product Versions dont match:", ent_info->ep_str.name, NCSFL_SEV_WARNING);
      }
   }else
   {
      m_AVM_LOG_GEN_EP_STR("Product Names dont match:", ent_info->ep_str.name, NCSFL_SEV_WARNING);
   }
   return NCSCC_RC_FAILURE;             
}

/***********************************************************************
*****
 * Name          : avm_list_nodes_chgd
 *
 * Description   : This function is used to frame a list of nodes for  
 *                 which AvM has to send operstate to AvD
 *
 * Arguments     : AVM_ENT_INFO       - Entity, I/P argument
 *                 AVM_LIST_HEAD_T    - O/P argument .
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
******/
static uns32 
avm_list_nodes_chgd(AVM_ENT_INFO_T *ent_info, AVM_LIST_HEAD_T *head)
{
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_NODE_T *temp_node = AVM_LIST_NODE_NULL ;

   head->count = 0;
   head->node  = AVM_LIST_NODE_NULL;   

  /* Store the first node  */

  temp_node  = m_MMGR_ALLOC_AVM_AVD_LIST_NODE;
  if(AVM_LIST_NODE_NULL == temp_node)
    {
      m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return NCSCC_RC_FAILURE;
    }
   memcpy(&temp_node->node_name, &ent_info->node_name, sizeof(SaNameT));

 /* Incremnent the node counts  */
   head->count++;
   temp_node->next = AVM_LIST_NODE_NULL;
   head->node = temp_node ;

  /* Traverse through child entity and form the node name list  */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
   {
     if(TRUE == child->ent_info->is_node)
      {
       /* Child entity represent a Node , Add the node name to the list  */

         temp_node->next  = m_MMGR_ALLOC_AVM_AVD_LIST_NODE;
         if(AVM_LIST_NODE_NULL == temp_node->next)
         {
            m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
            return NCSCC_RC_FAILURE;
          }
      
          temp_node = temp_node->next; 
          memcpy(&temp_node->node_name, &child->ent_info->node_name, sizeof(SaNameT));
          head->count++;
          temp_node->next = AVM_LIST_NODE_NULL ;
       
      }
     
   }

   return NCSCC_RC_SUCCESS;
}

/*********************************************************************** ******
  * Name          : avm_ins_pend
  *
  * Description   : This function is invoked when a FRU is inserted in 
  *                 the Chassis. It validates the FRU and issues approp-
  *                 riate command(Activate or Inactivate) to Shelf Mana
  *                 ger through HPL
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Insert
  *                 ion Pending Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
*****************************************************************************/

static uns32 
avm_ins_pend(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{   
   uns32 rc = NCSCC_RC_SUCCESS;
   HISV_API_CMD api_cmd = HS_RESOURCE_ACTIVE_SET;

   HPI_EVT_T *hpi_evt;
   AVM_ENT_INFO_T *dep_ent_info;   

   uns8 str[AVM_LOG_STR_MAX_LEN];
   char *arch_type = NULL;

   hpi_evt = ((AVM_EVT_T*)fsm_evt)->evt.hpi_evt; 

   m_AVM_LOG_FUNC_ENTRY("avm_not_present_ins_pend"); 

   /*****************************************************************
       While IPMC upgrade is going on, this event is not supposed
       to come. If this event comes, stop the ipmc timer. And log
       CRITICAL error.
   *****************************************************************/ 
   str[0] = '\0';
   if(ent_info->ipmc_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);

      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }
   else if(ent_info->ipmc_mod_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_mod_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }

   arch_type = m_NCS_OS_PROCESS_GET_ENV_VAR("OPENSAF_TARGET_SYSTEM_ARCH");
   /* Start up the boot timer only if the target system architecture is ATCA */
   if (m_NCS_OS_STRCMP(arch_type, "ATCA") == 0) {
      m_AVM_SSU_BOOT_TMR_START(avm_cb, ent_info);
   }

   ent_info->power_cycle = FALSE;
   if(AVM_ADM_LOCK == ent_info->adm_lock)
   {
      m_AVM_LOG_GEN_EP_STR("Entity Admin Locked", ent_info->ep_str.name, NCSFL_SEV_ERROR);
      rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
      avm_assign_state(ent_info, AVM_ENT_INACTIVE);
      return rc;   
   }      

   if(ent_info->act_policy == AVM_SHELF_MGR_ACTIVATION)
   {
      m_AVM_LOG_GEN_EP_STR("Entity Act by Shelf Mgr", ent_info->ep_str.name, NCSFL_SEV_INFO);

      ent_info->previous_state = AVM_ENT_NOT_PRESENT;
      ent_info->current_state  = AVM_ENT_INACTIVE;

      /* Update the DHCP configuration here */
      if (((AVM_EVT_T*)fsm_evt)->src != AVM_EVT_SSU)
         avm_ssu_dhconf(avm_cb, ent_info, fsm_evt, 1);
      return NCSCC_RC_SUCCESS;
   }

   if(TRUE == ent_info->depends_on_valid)
   {
      dep_ent_info = avm_find_ent_str_info(avm_cb, &ent_info->dep_ep_str,TRUE);

      if(AVM_ENT_INFO_NULL == dep_ent_info)
      {
         m_AVM_LOG_GEN("Depends On: No Entity existing ", ent_info->depends_on.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_WARNING);
         avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
         return NCSCC_RC_SUCCESS;
      }else
      {   
         if(dep_ent_info->row_status != NCS_ROW_ACTIVE)
         {
            m_AVM_LOG_GEN_EP_STR("Depends On: Row Status Not Active", dep_ent_info->ep_str.name, NCSFL_SEV_WARNING);
            avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
            return NCSCC_RC_FAILURE;
         }

         if((dep_ent_info->current_state == AVM_ENT_ACTIVE) || (dep_ent_info->current_state == AVM_ENT_RESET_REQ))
         {
            m_AVM_LOG_GEN_EP_STR("Depends On: Activate Entity", ent_info->ep_str.name, NCSFL_SEV_INFO);
            api_cmd = HS_RESOURCE_ACTIVE_SET; 
         }else
         {
            m_AVM_LOG_GEN_EP_STR("Depends On: Dependee Entity not Yet Active", ent_info->ep_str.name, NCSFL_SEV_INFO);

            rc = avm_validate_fru(ent_info, hpi_evt);   
            if(NCSCC_RC_SUCCESS != rc)
            {     
               m_AVM_LOG_GEN_EP_STR("FRU validation Failure:", ent_info->ep_str.name, NCSFL_SEV_ERROR);
               api_cmd = HS_RESOURCE_INACTIVE_SET;
            }else
            {
               avm_add_dependent(ent_info, dep_ent_info);   
               ent_info->previous_state = ent_info->current_state;
               ent_info->current_state  = AVM_ENT_INACTIVE;
               m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_ADD_DEPENDENT);
              /* Update the DHCP configuration here */
             if (((AVM_EVT_T*)fsm_evt)->src != AVM_EVT_SSU)
             {
                avm_ssu_dhconf(avm_cb, ent_info, fsm_evt, 1);
             }  
             return NCSCC_RC_SUCCESS;   
            }
         }
      }
   }

   if(api_cmd == HS_RESOURCE_ACTIVE_SET)
   {
      rc = avm_validate_fru(ent_info, hpi_evt);   
      if(NCSCC_RC_SUCCESS != rc)
      {     
         m_AVM_LOG_GEN_EP_STR("FRU validation Failure:", ent_info->ep_str.name, NCSFL_SEV_ERROR);
         rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
      }
      else
      {
         /* Update the DHCP configuration here */
         if (((AVM_EVT_T*)fsm_evt)->src != AVM_EVT_SSU)
         {
            avm_ssu_dhconf(avm_cb, ent_info, fsm_evt, 1);
         } 
         m_AVM_LOG_GEN_EP_STR("FRU validation Successful:", ent_info->ep_str.name, NCSFL_SEV_INFO);
         rc = avm_hisv_api_cmd(ent_info, api_cmd, 0);
      }
   }else
   {
      rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
   }

   ent_info->previous_state = AVM_ENT_NOT_PRESENT;
   ent_info->current_state  = AVM_ENT_INACTIVE;
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_active
  *
  * Description   : This function is invoked after AvM sends Activate &
  *                 the hotswap state is changed to SAHPI_HS_ACTIVE.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
extern uns32
avm_active(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_LIST_HEAD_T      head; 
   AVM_LIST_NODE_T     *node;
   AVM_ENT_INFO_LIST_T *child;
   AVM_ENT_INFO_LIST_T *temp;
   HISV_API_CMD         api_cmd = HS_RESOURCE_ACTIVE_SET;   

   head.count = 0;
   head.node  = AVM_LIST_NODE_NULL;

   m_AVM_LOG_GEN_EP_STR("Entity Active:", ent_info->ep_str.name, NCSFL_SEV_INFO);

   if(AVM_ADM_LOCK == ent_info->adm_lock)
   {
      m_AVM_LOG_GEN_EP_STR("Entity Admin Locked ", ent_info->ep_str.name, NCSFL_SEV_ERROR);
      rc = avm_hisv_api_cmd(ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_EXTRACTION);
      avm_assign_state(ent_info, AVM_ENT_ACTIVE);
      return rc;   
   }      

   if(TRUE == ent_info->is_node)
   {
      head.node = m_MMGR_ALLOC_AVM_AVD_LIST_NODE;
      if(AVM_LIST_NODE_NULL == head.node)
      {
         m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
         return NCSCC_RC_FAILURE;
      }  

      memcpy(&head.node->node_name, &ent_info->node_name, sizeof(SaNameT));
      head.node->next    = AVM_LIST_NODE_NULL;
      head.count++;
      node                = head.node;

      rc = avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ENABLED);
   }

   avm_assign_state(ent_info, AVM_ENT_ACTIVE);

   for(child = ent_info->child; child != NULL; child = child->next)
     {
         if((child->ent_info->is_fru) && (AVM_ADM_LOCK != child->ent_info->adm_lock))
           {
              m_AVM_LOG_GEN_EP_STR("Child Entity insertion Requested", child->ent_info->ep_str.name, NCSFL_SEV_INFO);
              rc = avm_hisv_api_cmd(child->ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_INSERTION);
              if(NCSCC_RC_SUCCESS != rc)
                  m_AVM_LOG_GEN_EP_STR("Child Entity insertion Request Failed", child->ent_info->ep_str.name, NCSFL_SEV_INFO);
                 
              /* Reset the RC */
              rc = NCSCC_RC_SUCCESS;
                 
           }
     }

   for(temp = ent_info->dependents; temp != AVM_ENT_INFO_LIST_NULL; temp = ent_info->dependents)
   {
     rc =  avm_hisv_api_cmd(temp->ent_info, api_cmd, 0);
     if(NCSCC_RC_SUCCESS != rc)
     {
         m_AVM_LOG_GEN_EP_STR("Entity Activtion Failed:", temp->ent_info->ep_str.name, NCSFL_SEV_ERROR);
     }       
     ent_info->dependents = ent_info->dependents->next;
     m_MMGR_FREE_AVM_ENT_INFO_LIST(temp);
   }   

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_not_pres_active
  *
  * Description   : This function is invoked after AvM sends Activate &
  *                 the hotswap state is changed to SAHPI_HS_ACTIVE.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_not_pres_active(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if(AVM_ADM_LOCK == ent_info->adm_lock)
   {
   /* Not-Present to Active , Entity were already Active .
      Extraction flow will take care of shutting Down the child too */

      m_AVM_LOG_GEN_EP_STR("Entity Admin Locked ", ent_info->ep_str.name, NCSFL_SEV_ERROR); 
      m_AVM_LOG_INVALID_VAL_ERROR(ent_info->adm_lock);
      rc = avm_hisv_api_cmd(ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_EXTRACTION);
      avm_assign_state(ent_info, AVM_ENT_ACTIVE);
      return rc;   
   }

   if(AVM_SHELF_MGR_ACTIVATION != ent_info->act_policy)
   {
      m_AVM_LOG_GEN_EP_STR("Act Configured by NCS but Act by SHMM", ent_info->ep_str.name, NCSFL_SEV_ERROR); 
      m_AVM_LOG_INVALID_VAL_ERROR(ent_info->act_policy);      
      ent_info->power_cycle = TRUE;
      rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_POWER, HISV_RES_POWER_CYCLE);
      avm_assign_state(ent_info, AVM_ENT_ACTIVE);
      return rc;   
   }

   if(ent_info->act_policy == AVM_SHELF_MGR_ACTIVATION)
   {
      rc = avm_active(avm_cb, ent_info);
   }

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_not_pres_ext_req
  *
  * Description   : This function is invoked after AvM sends Activate &
  *                 the hotswap state is changed to SAHPI_HS_ACTIVE.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_not_pres_ext_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32            rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T  head;
   
   m_AVM_LOG_GEN_EP_STR("Ext Req in Not Present", ent_info->ep_str.name, NCSFL_SEV_NOTICE); 
   rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);

   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in inactive state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
    if((AVM_ENT_INACTIVE < (child->ent_info->current_state)) && (AVM_ENT_INVALID >(child->ent_info->current_state)))
        avm_assign_state(child->ent_info, AVM_ENT_INACTIVE);

   avm_assign_state(ent_info, AVM_ENT_INACTIVE);

   return rc;
}  


/***********************************************************************
 ******
  * Name          : avm_not_pres_inactive
  *
  * Description   : This function is invoked when a blade is put in
  *                 chassis.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_not_pres_inactive(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_LIST_HEAD_T      head; 
   AVM_LIST_NODE_T     *node;

   head.count = 0;
   head.node  = AVM_LIST_NODE_NULL;

   m_AVM_LOG_FUNC_ENTRY("avm_not_pres_inactive");
   m_AVM_LOG_GEN_EP_STR("Valid in A-Spec ", ent_info->ep_str.name, NCSFL_SEV_INFO);
  
   ent_info->previous_state = AVM_ENT_NOT_PRESENT;
   ent_info->current_state  = AVM_ENT_INACTIVE;

   head.node = m_MMGR_ALLOC_AVM_AVD_LIST_NODE;
   if(AVM_LIST_NODE_NULL == head.node)
   {
      m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return NCSCC_RC_FAILURE;
   }  

   memcpy(&head.node->node_name, &ent_info->node_name, sizeof(SaNameT));
   head.node->next    = AVM_LIST_NODE_NULL;
   head.count++;
   node               = head.node;
   rc = avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_not_pres_avd_reset_req
  *
  * Description   : This function is invoked when a blade is put in
  *                 chassis.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_not_pres_avd_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_not_pres_avd_reset_req");

   m_AVM_LOG_GEN_EP_STR("AvD Reset Req in Not Present:", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   
   rc =  avm_handle_avd_error(avm_cb, ent_info, (AVM_EVT_T*)fsm_evt); 

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_not_pres_adm_shutdown
  *
  * Description   : This function is invoked after AvM sends Activate &
  *                 the hotswap state is changed to SAHPI_HS_ACTIVE.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_not_pres_adm_shutdown(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_GEN_EP_STR("Shutdown issued in Not Present ", ent_info->ep_str.name, NCSFL_SEV_INFO);

   ent_info->adm_lock = AVM_ADM_LOCK;

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_not_pres_adm_lock
  *
  * Description   : This function is invoked after AvM sends Activate &
  *                 the hotswap state is changed to SAHPI_HS_ACTIVE.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_not_pres_adm_lock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   ent_info->adm_lock = AVM_ADM_LOCK;
   m_AVM_LOG_GEN_EP_STR("Lock issued in Not Present", ent_info->ep_str.name, NCSFL_SEV_INFO);

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_not_pres_adm_unlock
  *
  * Description   : This function is invoked after AvM sends Activate &
  *                 the hotswap state is changed to SAHPI_HS_ACTIVE.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_not_pres_adm_unlock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   ent_info->adm_lock = AVM_ADM_UNLOCK;
   m_AVM_LOG_GEN_EP_STR("Unlock issued in NOT_PRESENT state for", ent_info->ep_str.name, NCSFL_SEV_ERROR);

   return rc;
}

static uns32
avm_not_pres_resource_rest(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;

   
   m_AVM_LOG_GEN_EP_STR("HISv did not publish Prev HS state  ", ent_info->ep_str.name, NCSFL_SEV_ERROR); 

   if((AVM_ADM_LOCK == ent_info->adm_lock) || (ent_info->adm_shutdown))
   {
     /* Communication Restored : Perform extraction for child too.  */
 
      m_AVM_LOG_GEN_EP_STR("Entity Admin Locked ", ent_info->ep_str.name, NCSFL_SEV_ERROR); 
      m_AVM_LOG_INVALID_VAL_ERROR(ent_info->adm_lock);
      m_AVM_LOG_INVALID_VAL_ERROR(ent_info->adm_shutdown);
 
      for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
     {
         rc = avm_hisv_api_cmd(child->ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_EXTRACTION);
         rc = avm_hisv_api_cmd(child->ent_info, HS_RESOURCE_INACTIVE_SET, 0);
         avm_assign_state(child->ent_info, AVM_ENT_INACTIVE);
      
        if((AVM_ADM_LOCK == child->ent_info->adm_lock) || (child->ent_info->adm_shutdown))
          {
            child->ent_info->adm_shutdown = FALSE;
            child->ent_info->adm_lock     = AVM_ADM_LOCK;
          }
     }
 
      rc = avm_hisv_api_cmd(ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_EXTRACTION);
      rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
      avm_assign_state(ent_info, AVM_ENT_INACTIVE);
      ent_info->adm_shutdown = FALSE;
      ent_info->adm_lock     = AVM_ADM_LOCK;
      return rc;   
   }

   if(AVM_SHELF_MGR_ACTIVATION != ent_info->act_policy)
   {
      rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_POWER, HISV_RES_POWER_CYCLE);
   }

   return rc;
}


/***********************************************************************
 ******
  * Name          : avm_inactive_adm_shutdown
  *
  * Description   : This function is invoked  admin issues shutdown in
  *                 in inactive state.  
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_inactive_adm_shutdown(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   ent_info->adm_lock = AVM_ADM_LOCK;

   m_AVM_LOG_GEN_EP_STR("Adm Shutdown issued in Inactive", ent_info->ep_str.name, NCSFL_SEV_INFO);

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_inactive_adm_lock
  *
  * Description   : This function is invoked  admin issues lock in
  *                 in inactive state.  
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_inactive_adm_lock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   ent_info->adm_lock = AVM_ADM_LOCK;
   m_AVM_LOG_GEN_EP_STR("Lock issued in INACTIVE state for", ent_info->ep_str.name, NCSFL_SEV_INFO);

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_inactive_adm_unlock
  *
  * Description   : This function is invoked  admin issues unlock in
  *                 in inactive state.  
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_inactive_adm_unlock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns8 str[AVM_LOG_STR_MAX_LEN];

   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "HISV returned error, so this entity could not be activated but has been unlocked in AVM db";

   m_AVM_LOG_GEN_EP_STR("Unlock issued in INACTIVE state for", ent_info->ep_str.name, NCSFL_SEV_INFO);

   /*****************************************************************
       While IPMC upgrade is going on, this event is not supposed
       to come. If this event comes, stop the ipmc timer. And log
       CRITICAL error.
   *****************************************************************/ 
   str[0] = '\0';     
   if(ent_info->ipmc_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }
   else if(ent_info->ipmc_mod_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_mod_tmr);
       sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }

   rc = avm_hisv_api_cmd(ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_INSERTION);

   if(NCSCC_RC_SUCCESS != rc)
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("SAHPI_HS_ACTION_INSERTION", ent_info->ep_str.name, NCSFL_SEV_INFO);
   }

   if((ent_info->adm_shutdown) || (ent_info->adm_lock != AVM_ADM_UNLOCK))
   {
     ent_info->adm_lock     = AVM_ADM_UNLOCK;
     ent_info->adm_shutdown = FALSE;
   }

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_inactive_active
  *
  * Description   : This function is invoked after AvM sends Activate &
  *                 the hotswap state is changed to SAHPI_HS_ACTIVE.
  *                 
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which Active
  *                 Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_inactive_active(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns8 str[AVM_LOG_STR_MAX_LEN];

   m_AVM_LOG_FUNC_ENTRY("avm_inactive_active");

   if(ent_info->previous_state == AVM_ENT_EXTRACT_REQ)
   {
      m_AVM_LOG_GEN_EP_STR("Latch Opened and closed without AvM resp", ent_info->ep_str.name, NCSFL_SEV_ERROR);
      return NCSCC_RC_SUCCESS;
   }

   /*****************************************************************
       While IPMC upgrade is going on, this event is not supposed
       to come. If this event comes, stop the ipmc timer. And log
       CRITICAL error.
   *****************************************************************/ 
   str[0] = '\0';     
   if(ent_info->ipmc_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
  
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }
   else if(ent_info->ipmc_mod_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_mod_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }

   rc = avm_active(avm_cb, ent_info);
   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_inactive_inactive
  *
  * Description   : This function is invoked after AvM sends Inctivate &
  *                 the hotswap state is changed to SAHPI_HS_INACTIVE.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which 
  *                 Inactive Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/

static uns32
avm_inactive_inactive(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_LIST_HEAD_T      head; 
   AVM_LIST_NODE_T     *node;
  
   m_AVM_LOG_GEN_EP_STR("Blade is inactivated", ent_info->ep_str.name, NCSFL_SEV_INFO);
   head.count = 0;
   head.node  = AVM_LIST_NODE_NULL;

   avm_assign_state(ent_info, AVM_ENT_INACTIVE);

   head.node = m_MMGR_ALLOC_AVM_AVD_LIST_NODE;
   if(AVM_LIST_NODE_NULL == head.node)
   {
      m_AVM_LOG_MEM(AVM_LOG_DEFAULT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return NCSCC_RC_FAILURE;
   }  

   memcpy(&head.node->node_name, &ent_info->node_name, sizeof(SaNameT));
   head.node->next    = AVM_LIST_NODE_NULL;
   head.count++;
   node               = head.node;
   rc = avm_avd_node_operstate(avm_cb, &head,AVM_NODES_ABSENT);

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_inactive_extracted
  *
  * Description   : This function is invoked after FRU is extracted from
  *                 Chassis.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Extracted Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None.
************************************************************************
*****/
static uns32
avm_inactive_extracted(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns8 str[AVM_LOG_STR_MAX_LEN];
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;

   m_AVM_LOG_GEN_EP_STR("FRU extracted", ent_info->ep_str.name, NCSFL_SEV_INFO); 

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   /*****************************************************************
       While IPMC upgrade is going on, this event is not supposed
       to come. If this event comes, stop the ipmc timer. And log
       CRITICAL error.
   *****************************************************************/ 
   /*this event will come when the IPMC is resetted after the ipmc upgrade is done
     The shmm will lose the communication with ipmc while its resetting and hence will
     send this hotswap event where it will render the blade as extracted. So this is a success
     scenario in case of IPMC upgrade and we should check if the blade is in adm lock state before
     rendering this event as an erroneous one. - change*/

   str[0] = '\0';
   if((ent_info->ipmc_tmr.status == AVM_TMR_RUNNING) &&
                 (((ent_info->dhcp_serv_conf.ipmc_upgd_state == IPMC_UPGD_DONE)||
                  (ent_info->dhcp_serv_conf.ipmc_upgd_state == 0))))
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);

      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }
   else if(ent_info->ipmc_mod_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_mod_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }

   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
       {
             avm_reset_ent_info(child->ent_info);
             avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT);
       }

   rc = avm_reset_ent_info(ent_info);
   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT); 

   return rc;
}  
static uns32
avm_inactive_resource_fail(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{

   uns32            rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T  head;
   uns8 str[AVM_LOG_STR_MAX_LEN];
   
   /*****************************************************************
       While IPMC upgrade is going on, this event is not supposed
       to come. If this event comes, stop the ipmc timer. And log
       CRITICAL error.
   *****************************************************************/ 
   str[0] = '\0';     
   if(ent_info->ipmc_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }
   else if(ent_info->ipmc_mod_tmr.status == AVM_TMR_RUNNING)
   {
      avm_stop_tmr(avm_cb, &ent_info->ipmc_mod_tmr);
      sprintf(str,"AVM-SSU: Payload blade %s: Unexpected Event received, while upgrading IPMC",
                     ent_info->ep_str.name);
      m_AVM_LOG_DEBUG(str,NCSFL_SEV_CRITICAL);
   }
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in not-present state */

   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
       avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT);

  /* Set parent entity in not-present state */
   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT);

   return rc;
}

static uns32
avm_inactive_resource_rest(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
  uns32  rc = NCSCC_RC_SUCCESS;
  m_AVM_LOG_FUNC_ENTRY("avm_inactive_resource_rest");
  return rc;
}

static uns32
avm_inactive_sensor_assert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_FUNC_ENTRY("avm_inactive_sensor_assert");
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_ASSERT);
   return rc;
}  

static uns32
avm_inactive_sensor_deassert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_inactive_sensor_deassert");
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_DEASSERT);
   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_active_adm_shutdown
  *
  * Description   : This function is invoked when admin issues
  *                 shutdown 
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Extracton Pending Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected FRUs.
************************************************************************
*****/
static uns32
avm_active_adm_shutdown(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "HISV returned error, so entity could not be shutdown";

   m_AVM_LOG_FUNC_ENTRY("avm_active_adm_shutdown_req"); 

   if((AVM_ADM_LOCK != ent_info->adm_lock)  && (!ent_info->adm_shutdown))
   {
      rc = avm_hisv_api_cmd(ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_EXTRACTION);
      if(NCSCC_RC_SUCCESS != rc)
      {
         evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

         evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);
 
         m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

         memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

         m_AVM_LOG_GEN_EP_STR("SAHPI_HS_ACTION_EXTRACTION failed", ent_info->ep_str.name, NCSFL_SEV_ERROR);
      }else
      {
         ent_info->adm_shutdown = TRUE;
      }
   }

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_active_adm_lock
  *
  * Description   : This function is invoked when admin issues
  *                 lock
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Extracton Pending Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected FRUs.
************************************************************************
*****/
static uns32
avm_active_adm_lock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "HISV returned error, so entity could not be locked";

   m_AVM_LOG_FUNC_ENTRY("avm_active_adm_lock"); 


   if(ent_info->adm_shutdown)
   {
     ent_info->adm_shutdown = FALSE;
     ent_info->adm_lock = AVM_ADM_LOCK;
     return rc;
   }

   rc =  avm_hisv_api_cmd(ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_EXTRACTION);
   if(NCSCC_RC_SUCCESS != rc)
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("SAHPI_HS_ACTION_EXTRACTION failed", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   }else
   {
      ent_info->adm_lock = AVM_ADM_LOCK;
   }

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_active_adm_unlock
  *
  * Description   : This function is invoked when admin issues
  *                 unlock
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Extracton Pending Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected FRUs.
************************************************************************
*****/
static uns32
avm_active_adm_unlock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_FAILURE;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg1 = "Unlock can not be done because lock operation is already in progress, try after lock is done";
   uns8 *cli_msg2 = "Unlock operation on an active entity is not allowed";

   m_AVM_LOG_FUNC_ENTRY("avm_active_adm_unlock"); 

   if(AVM_ADM_LOCK == ent_info->adm_lock)
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg1);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg1, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("Lock not yet completed", ent_info->ep_str.name, NCSFL_SEV_INFO); 
   }else
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg2);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg2, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("Unlock not valid in Active", ent_info->ep_str.name, NCSFL_SEV_INFO); 
   }

   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_active_extract_pend
  *
  * Description   : This function is invoked when the latch is ejected 
  *                 and hotswap event with SAHPI_HS_EXTRACTION_PENDING 
  *                 as the current hotswap state. 
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Extracton Pending Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected FRUs.
************************************************************************
*****/
static uns32
avm_active_extract_pend(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;

   m_AVM_LOG_FUNC_ENTRY("avm_active_extract_pend"); 

   
   if(TRUE == ent_info->power_cycle)
   {
      m_AVM_LOG_GEN_EP_STR("Entity Power Cycled", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
      ent_info->power_cycle = FALSE;
      avm_assign_state(ent_info, AVM_ENT_EXTRACT_REQ);
      return rc;
   }
   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   if(AVM_ADM_LOCK != ent_info->adm_lock) 
   {
      if(ent_info->adm_shutdown)
      {
         ent_info->adm_lock = AVM_ADM_LOCK;
      }
      ent_info->adm_shutdown = FALSE;
      rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_EXTRACTION);
      avm_assign_state(ent_info, AVM_ENT_EXTRACT_REQ); 
 
   }else
   {
    
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
      if((AVM_ENT_INACTIVE < (child->ent_info->current_state)) && (AVM_ENT_INVALID >(child->ent_info->current_state)))
            avm_assign_state(child->ent_info, AVM_ENT_INACTIVE);

      rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
      avm_assign_state(ent_info, AVM_ENT_INACTIVE); 
   }
   return rc;
}  

/***********************************************************************
 ******
  * Name          : avm_active_adm_soft_reset_req
  *
  * Description   : This function is invoked when the Operator issues 
  *                 a Soft Reset Request.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Reset Req Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected nodes
************************************************************************
*****/

static uns32
avm_active_adm_soft_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{

/*  Soft reset case , Child will not be reseted , Parent-Child in different fault domain  */
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "Either lock or shutdown of this entity is in progress as well as a locked entity can not be reset";

   m_AVM_LOG_FUNC_ENTRY("avm_active_adm_reset_req"); 

   if((!ent_info->adm_shutdown) && (AVM_ADM_LOCK != ent_info->adm_lock))
   {
      rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_RESET);
      avm_assign_state(ent_info, AVM_ENT_RESET_REQ);
   }else
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

      rc = NCSCC_RC_FAILURE;
   }   
   return  rc;
}

/***********************************************************************
 ******
  * Name          : avm_active_adm_reset_req
  *
  * Description   : This function is invoked when the Operator issues 
  *                 a Reset Request.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Reset Req Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected nodes
************************************************************************
*****/

static uns32
avm_active_adm_hard_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_NODE_RESET_RESP_T resp;

   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg1 = "HISV resource reset returned error.";
   uns8 *cli_msg2 = "Either lock or shutdown of this entity is in progress as well as a locked entity can not be reset.";
   uns8 *cli_msg3 = "AvM failed to reset entity using HISV but AvD will attempt NCS Reboot.";

   m_AVM_LOG_FUNC_ENTRY("avm_active_adm_reset_req"); 

   if((!ent_info->adm_shutdown) && (AVM_ADM_LOCK != ent_info->adm_lock))
   {
      /* Mark that this is a hard reset case and we should not send
      ** a node reset message to FM so that FM will act on the 
      ** loss of heart beat loss and lhc failure
      **
      ** Also reset the flag after the processing is done.
      */
      rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_COLD_RESET);
      resp = (NCSCC_RC_SUCCESS == rc) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;

      if(NCSCC_RC_SUCCESS != rc)
      {
        if(avm_is_this_entity_self(avm_cb, ent_info->entity_path) == TRUE)
        {
           evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg3);

         evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

         m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

           memcpy(evt->evt.mib_req->rsp.add_info, cli_msg3, evt->evt.mib_req->rsp.add_info_len);

           m_AVM_LOG_GEN_EP_STR("AvM failed to reset using HISV but AvD will attempt NCS Reboot of entity:",
                                    ent_info->ep_str.name, NCSFL_SEV_ERROR);
         }
         else
         {
            evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg1);

            evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

            m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

            memcpy(evt->evt.mib_req->rsp.add_info, cli_msg1, evt->evt.mib_req->rsp.add_info_len);

            m_AVM_LOG_GEN_EP_STR("Cold/Adm Hard Reset Failure", ent_info->ep_str.name, NCSFL_SEV_ERROR);
         }
      }else
      { 
         m_AVM_LOG_GEN_EP_STR("Cold/Adm Hard Reset Success", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
      } 
      avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);  
   }else
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg2);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg2, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("Two un-compatible admin Operations", ent_info->ep_str.name, NCSFL_SEV_INFO); 
      rc = NCSCC_RC_FAILURE;
   }
   return  rc;
}

static uns32
avm_active_resource_fail(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T head;
 
/* This is communication Lost state information , Child will be impacted too  */  
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in not-present state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
       avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT);

   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT);
   
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_active_sensor_assert 
  *
  * Description   : This function is invoked when a sensor event with 
  *                 severeity Critical is received.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Sensor Assert Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected nodes
************************************************************************
*****/
static uns32
avm_active_sensor_assert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_active_sensor_assert");
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_ASSERT);

   return rc;   
}

/***********************************************************************
 ******
  * Name          : avm_active_sensor_deassert 
  *
  * Description   : This function is invoked when a sensor deassert is 
  *                 received. 
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Sensor Assert Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : Scope the event to all the infected nodes
************************************************************************
*****/
static uns32
avm_active_sensor_deassert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   m_AVM_LOG_FUNC_ENTRY("avm_active_sensor_deassert");
   rc  = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_DEASSERT);
   return rc;
}

static uns32
avm_active_node_shutdown_resp(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   m_AVM_LOG_FUNC_ENTRY("avm_active_node_shutdown_resp");
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
  * Name          : avm_active_avd_reset_req
  *
  * Description   : This function is invoked when the AvD issues
  *                 a Reset Request.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 Reset Req Event has been received
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/

static uns32
avm_active_avd_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_NODE_RESET_RESP_T resp = AVM_NODE_RESET_SUCCESS;

   rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_GRACEFUL_REBOOT);

   resp = (NCSCC_RC_SUCCESS == rc) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_GEN_EP_STR("AvD Issued Reset/Cold Failure", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   }else
   { 
      m_AVM_LOG_GEN_EP_STR("AvD Issued Reset/Cold Success", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
   }

   avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);     
   
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_active_surp_extract 
  *
  * Description   : This function is invoked when the operator extracts
  *                 the FRU without any prior notice to AvM.  
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event with current state SAHPI_HS_NOT_PRES
  *                 ENT received.    
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_active_surp_extract(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_active_surp_extract");
 
   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   if(ent_info->adm_shutdown)
   {
      ent_info->adm_lock     = AVM_ADM_LOCK;
      ent_info->adm_shutdown = FALSE;
   }
   rc = avm_scope_fault(avm_cb, ent_info,AVM_SCOPE_SURP_EXTRACTION);    
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_active_extracted
  *
  * Description   : This function is invoked when the blade is extracted
  *                 and blade went back to active after AvD rejected 
  *                 shutdown request
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event with current state SAHPI_HS_NOT_PRES
  *                 ENT received.    
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_active_extracted(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{

   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_LIST_HEAD_T head;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;

   m_AVM_LOG_FUNC_ENTRY("avm_active_extracted");

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   if(ent_info->adm_shutdown)
   {
      ent_info->adm_lock     = AVM_ADM_LOCK;
      ent_info->adm_shutdown = FALSE;
   }

   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
     avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT); 

  /*All child traversed now the parent one  */
   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT);

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_active_inactive
  *
  * Description   : This function is invoked when the blade is 
  *                 inactivated and blade went back to active after 
  *                 AvD rejected shutdown request
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event with current state SAHPI_HS_NOT_PRES
  *                 ENT received.    
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_active_inactive(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{

   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T head;
   

   m_AVM_LOG_FUNC_ENTRY("avm_active_extracted");
 
   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   if(ent_info->adm_shutdown)
   {
      ent_info->adm_lock     = AVM_ADM_LOCK;
      ent_info->adm_shutdown = FALSE;
   }

   /*Select all the nodes which have got impacted */
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in inactive state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
     if((AVM_ENT_INACTIVE < (child->ent_info->current_state)) && (AVM_ENT_INVALID >(child->ent_info->current_state)))
        avm_assign_state(child->ent_info, AVM_ENT_INACTIVE);

  /* Inactive state for the parent one */
   avm_assign_state(ent_info, AVM_ENT_INACTIVE);

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_faulty_node_failover_resp
  *
  * Description   : This function is invoked when AvD sends failover re
  *                 -sp to AvM.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the node to which
  *                 a Failover resp has been received from AvD.
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/

static uns32
avm_faulty_node_failover_resp(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   AVM_LIST_HEAD_T   head;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_faulty_node_failover_resp");

   head.count = 0;
   head.node  = AVM_LIST_NODE_NULL;
   if(NCSCC_RC_SUCCESS == ent_info->sensor_assert)
   {
      m_AVM_LOG_GEN("Node is not yet deasserted ", ent_info->node_name.value, SA_MAX_NAME_LENGTH, NCSFL_SEV_INFO);
   }else
   {
      head.node = m_MMGR_ALLOC_AVM_AVD_LIST_NODE;
      if(AVM_LIST_NODE_NULL  == head.node)
      {
         m_AVM_LOG_MEM(AVM_LOG_LIST_NODE_INFO_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
         return NCSCC_RC_FAILURE;
      }else
      {
         memcpy(&head.node->node_name, &ent_info->node_name, sizeof(SaNameT));
         head.count++;
         head.node->next = AVM_LIST_NODE_NULL;
         avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ENABLED);
      }  
   }      
   avm_assign_state(ent_info, AVM_ENT_ACTIVE);
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_faulty_sensor_deaasert
  *
  * Description   : This function is invoked when HiSV issues a sensor
  *                 event with asserted flag false.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which 
  *                 the event has been received from HiSV. 
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/

static uns32
avm_faulty_sensor_deassert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_faulty_sensor_deassert");
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_DEASSERT);   
   return rc;
}

static uns32
avm_faulty_adm_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_faulty_adm_reset_req");
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_RESET); 
   return rc; 
   
}

/***********************************************************************
 ******
  * Name          : avm_faulty_avd_reset_req
  *
  * Description   : This function is invoked when AvD issues a reset
  *                 request o AvM.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the node to which
  *                 a Reset request has been received from AvD.
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/

static uns32
avm_faulty_avd_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_NODE_RESET_RESP_T resp;

   rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_GRACEFUL_REBOOT);

   resp = (NCSCC_RC_SUCCESS == rc)? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_GEN_EP_STR("Reset Failure", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   }else
   { 
      m_AVM_LOG_GEN_EP_STR("Reset Successful", ent_info->ep_str.name, NCSFL_SEV_INFO);
   }

   avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);     
   avm_assign_state(ent_info, AVM_ENT_ACTIVE);

   return rc;
}


static uns32
avm_faulty_sensor_assert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   m_AVM_LOG_GEN("Entity is already under failover", ent_info->entity_path.Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_INFO);   
   return NCSCC_RC_SUCCESS;
}

static uns32
avm_faulty_extract_pend(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void*fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_faulty_extract_pend");

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   rc = avm_scope_fault(avm_cb, ent_info,AVM_SCOPE_EXTRACTION); 
   return rc;
}   

static uns32
avm_faulty_surp_extract(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void*fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   m_AVM_LOG_FUNC_ENTRY("avm_faulty_surp_extract");
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SURP_EXTRACTION);    
   return rc;
}

static uns32
avm_faulty_adm_shutdown(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void*fsm_evt)
{
   return NCSCC_RC_SUCCESS;
}

static uns32
avm_faulty_adm_lock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void*fsm_evt)
{
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_extract_pend
  *
  * Description   : This function is invoked when the latch is ejected.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap 
  *                 state as SAHPI_HS_EXTRACTION_PENDING 
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/

static uns32
avm_reset_req_extract_pend(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T *child = AVM_ENT_INFO_LIST_NULL ;

   m_AVM_LOG_FUNC_ENTRY("avm_reset_req_extract_pend");
  
   if(AVM_ADM_LOCK != ent_info->adm_lock) 
   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   if(AVM_ADM_LOCK != ent_info->adm_lock)
   {
      if(ent_info->adm_shutdown)
      {
         ent_info->adm_shutdown = FALSE;
         ent_info->adm_lock = AVM_ADM_LOCK; 
      }
     /* Hardware Fault : Include the child in fault */
     rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_EXTRACTION);
     avm_assign_state(ent_info, AVM_ENT_EXTRACT_REQ);

   }else
   {

      rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);

    for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
       if((AVM_ENT_INACTIVE < (child->ent_info->current_state)) && (AVM_ENT_INVALID >(child->ent_info->current_state)))
           avm_assign_state(ent_info, AVM_ENT_INACTIVE);

      avm_assign_state(ent_info, AVM_ENT_INACTIVE);
   }
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_node_shutdown_resp
  *
  * Description   : This function is invoked when AvD is done with 
  *                 shutdown.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_node_shutdown_resp(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_NODE_RESET_RESP_T   resp;
   AVM_EVT_T *avd_avm;

   avd_avm = (AVM_EVT_T*)fsm_evt;
   if(AVM_EVT_NULL == avd_avm)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;
   }
   
   if(AVM_ADM_LOCK == ent_info->adm_lock)
   {
      m_AVM_LOG_GEN_EP_STR("Entity Locked, Ignore the Soft Reset by admin", ent_info->ep_str.name, NCSFL_SEV_WARNING);
      rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
      return rc;
   }

   if(NCSCC_RC_FAILURE == avd_avm->evt.avd_evt->avd_avm_msg.shutdown_resp.recovery_status)
   {
      m_AVM_LOG_GEN("AvD rejected Reset request for the node", ent_info->node_name.value, SA_MAX_NAME_LENGTH, NCSFL_SEV_ERROR);
      avm_assign_state(ent_info, AVM_ENT_ACTIVE);
      return NCSCC_RC_SUCCESS;
   }

   if(TRUE == ent_info->is_node)
   {
      rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_GRACEFUL_REBOOT);
      resp = (rc == NCSCC_RC_SUCCESS) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;

      if(NCSCC_RC_SUCCESS != rc)
      {
         m_AVM_LOG_GEN_EP_STR("Admin Soft/Warm Reset Fail", ent_info->ep_str.name, NCSFL_SEV_ERROR);
      }else
      { 
         m_AVM_LOG_GEN_EP_STR("Admin Soft/Warm Reset Success", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
      }     
   }else
   {
      m_AVM_LOG_GEN_EP_STR("Shutdown Response for a non-node entity", ent_info->ep_str.name, NCSFL_SEV_ERROR); 
   }

   avm_assign_state(ent_info, AVM_ENT_ACTIVE);
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_node_failover_resp
  *
  * Description   : This function is invoked when blade receibs failover resp
  *                 in reset_req state  
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a Reset Req has been issued from AvD.
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_node_failover_resp(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   m_AVM_LOG_GEN_EP_STR("Failover Response in reset_req state ", ent_info->ep_str.name, NCSFL_SEV_INFO); 
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_surp_extract
  *
  * Description   : This function is invoked when AvD issues a reset req
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a Reset Req has been issued from AvD.
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_surp_extract(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_GEN_EP_STR("Surprise extraction in reset_req state ", ent_info->ep_str.name, NCSFL_SEV_INFO); 

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SURP_EXTRACTION);
   
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_avd_node_reset_req
  *
  * Description   : This function is invoked when AvD issues a reset req
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a Reset Req has been issued from AvD.
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_avd_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   
   uns32                   rc = NCSCC_RC_SUCCESS;
   AVM_NODE_RESET_RESP_T   resp;

   m_AVM_LOG_FUNC_ENTRY("avm_reset_req_avd_reset_req");

   if(TRUE == ent_info->is_node)
   {
      rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_GRACEFUL_REBOOT);

      resp = (NCSCC_RC_SUCCESS == rc) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;

      if(NCSCC_RC_SUCCESS != rc)
      {
         m_AVM_LOG_GEN_EP_STR("AvD Issued Reset/Cold Fail", ent_info->ep_str.name,  NCSFL_SEV_ERROR);
      }else
      { 
         m_AVM_LOG_GEN_EP_STR("AvD Issued Reset/Cold Successful", ent_info->ep_str.name,  NCSFL_SEV_NOTICE);
      }     
      avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);

   }else
   {
      m_AVM_LOG_GEN_EP_STR("Avd Reset Req for a non-node entity:", ent_info->ep_str.name, NCSFL_SEV_ERROR); 
   }

   avm_assign_state(ent_info, AVM_ENT_ACTIVE);
   return rc;
}

static uns32
avm_reset_req_resource_fail(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T head;
   
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in not present state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
      avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT);

   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT);
   return rc;

}

/***********************************************************************
 ******
  * Name          : avm_reset_req_sensor_ssert
  *
  * Description   : This function is invoked when balde receives 
  *                 sensor assert while in reset 
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_sensor_assert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc;

   m_AVM_LOG_FUNC_ENTRY("avm_reset_req_sensor_assert");   
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_ASSERT);
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_sensor_dessert
  *
  * Description   : This function is invoked when balde receives 
  *                 sensor deassert while in reset 
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_sensor_deassert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc;   

   m_AVM_LOG_FUNC_ENTRY("avm_reset_req_sensor_assert");   
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_DEASSERT);
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_adm_hard_reset
  *
  * Description   : This function is invoked when AvD is done with 
  *                 shutdown.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_adm_hard_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_NODE_RESET_RESP_T resp;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg1 = "HISV resource reset returned error.";
   uns8 *cli_msg2 = "Either lock or shutdown of this entity is in progress as well as a locked entity can not be reset.";
   uns8 *cli_msg3 = "AvM failed to reset entity using HISV but AvD will attempt NCS Reboot.";
   
   if((AVM_ADM_LOCK == ent_info->adm_lock) || (ent_info->adm_shutdown))
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg2);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg2, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("Entity Locked, Ignore the Hard Reset by admin", ent_info->ep_str.name, NCSFL_SEV_WARNING);
      return NCSCC_RC_FAILURE;
   } 

   rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_COLD_RESET);
   if(NCSCC_RC_SUCCESS != rc)
   {
      if(avm_is_this_entity_self(avm_cb, ent_info->entity_path) == TRUE)
      {
         evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg3);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

         memcpy(evt->evt.mib_req->rsp.add_info, cli_msg3, evt->evt.mib_req->rsp.add_info_len);

         m_AVM_LOG_GEN_EP_STR("AvM failed to reset using HISV but AvD will attempt NCS Reboot of entity:",
                                    ent_info->ep_str.name, NCSFL_SEV_ERROR);
      }
      else
      {
         evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg1);

         evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

         m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

         memcpy(evt->evt.mib_req->rsp.add_info, cli_msg1, evt->evt.mib_req->rsp.add_info_len);

         m_AVM_LOG_GEN_EP_STR("Hard/Cold Reset failed", ent_info->ep_str.name, NCSFL_SEV_ERROR);
      }
   }else
   {
      m_AVM_LOG_GEN_EP_STR("Hard/Cold Reset Successful", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   }
   
   resp = (NCSCC_RC_SUCCESS == rc) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;
   avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);
   avm_assign_state(ent_info, AVM_ENT_ACTIVE);

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_adm_shutdown
  *
  * Description   : This function is invoked when balde receives 
  *                 shutdown from adminn EXTRACTION_PENDING.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_adm_shutdown(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_FAILURE;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "Shutdown is not allowed while entity reset is in progress";

   evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

   evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

   m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

   memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

   m_AVM_LOG_GEN_EP_STR("Shutdown while Reset not valid", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_adm_lock
  *
  * Description   : This function is invoked when AvD is done with 
  *                 shutdown.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_adm_lock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "HISV returned error, so entity could not be locked";

   m_AVM_LOG_FUNC_ENTRY("avm_reset_req_adm_lock");

   rc = avm_hisv_api_cmd(ent_info, HS_ACTION_REQUEST, SAHPI_HS_ACTION_EXTRACTION);
   if(NCSCC_RC_SUCCESS != rc)
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("SAHPI_HS_ACTION_EXTRACTION failed", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   }else
   {
      m_AVM_LOG_GEN_EP_STR("Entity Locked", ent_info->ep_str.name, NCSFL_SEV_INFO);
      ent_info->adm_lock = AVM_ADM_LOCK;
   }

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_adm_unlock
  *
  * Description   : This function is invoked when AvD is done with 
  *                 shutdown.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_adm_unlock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_FAILURE;

   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "Unlock operation is rejected because entity reset is in progress.";
  
   m_AVM_LOG_FUNC_ENTRY("avm_reset_req_adm_unlock");

   evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

   evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

   m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

   memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);
 
   m_AVM_LOG_GEN_EP_STR("Adm Unock invalid in Reset Req state", ent_info->ep_str.name, NCSFL_SEV_ERROR);

   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_extracted
  *
  * Description   : This function is invoked when AvM is still waiting for
  *                 shutdown response and the operator issued shutdown 
  *                 request and a switchover occured because of which there
  *                 are event losses and the slot becomes unmanageable so
  *                 AvM has to update its database whenever it gets infor-
  *                 mation that the blade is being removed from the chassis
  *                 
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_extracted(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void*fsm_evt)
{
   AVM_LIST_HEAD_T head;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);
   
   if(ent_info->adm_shutdown)
   {
      ent_info->adm_lock = AVM_ADM_LOCK;
      ent_info->adm_shutdown = FALSE;
   }
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in not present state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
      avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT);

   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT);
   
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_reset_req_inactive
  *
  * Description   : This function is invoked when AvM is still waiting for
  *                 shutdown response and the operator issued shutdown 
  *                 request and a switchover occured because of which there
  *                 are event losses and the slot becomes unmanageable so
  *                 AvM has to update its database whenever it gets infor-
  *                 mation that the blade is inactivated. 
  *                 
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event is received with current hotswap
  *                 state as SAHPI_HS_EXTRACTION_PENDING
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_reset_req_inactive(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void*fsm_evt)
{
   AVM_LIST_HEAD_T head;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);
   
   if(ent_info->adm_shutdown)
   {
      ent_info->adm_lock = AVM_ADM_LOCK;
      ent_info->adm_shutdown = FALSE;
   }
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in inactive state */
  /* Parent Dead , Child will follow  */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
     if((AVM_ENT_INACTIVE < (child->ent_info->current_state)) && (AVM_ENT_INVALID >(child->ent_info->current_state)))
       avm_assign_state(child->ent_info, AVM_ENT_INACTIVE);

   avm_assign_state(ent_info, AVM_ENT_INACTIVE);
   
   return rc;
}


/***********************************************************************
 ******
 * Name          : avm_ext_req_node_shutdown_resp
 *
 * Description   : This function is invoked when AvD sends a shutdown 
 *                 response.   
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : Only two levels of FRUs have been assumed.It implies that
 *                 if a shtdown is finished on one FRU, a check has to be done
 *                 if its children are ready to be extracted.Only one level of 
 *                 check is needed.     
 *****************************************************************************
 ******/

static uns32
avm_ext_req_node_shutdown_resp(
                                    AVM_CB_T        *avm_cb, 
                                    AVM_ENT_INFO_T  *ent_info, 
                                    void            *fsm_evt
                              )
{
   AVM_ENT_INFO_LIST_T *child = AVM_ENT_INFO_LIST_NULL;
   uns32 rc      = NCSCC_RC_SUCCESS;
   AVM_EVT_T *avd_avm;

   avd_avm = (AVM_EVT_T*)fsm_evt;
   if(AVM_EVT_NULL == avd_avm)
   {
      m_AVM_LOG_INVALID_VAL_FATAL(0);
      return NCSCC_RC_FAILURE;
   }
   
   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_node_shutdown_resp");

   if(NCSCC_RC_FAILURE == avd_avm->evt.avd_evt->avd_avm_msg.shutdown_resp.recovery_status)
   {
      m_AVM_LOG_GEN("AvD rejected shutdown request for the node", ent_info->node_name.value, SA_MAX_NAME_LENGTH, NCSFL_SEV_ERROR);
      
      ent_info->adm_lock = AVM_ADM_UNLOCK;
      ent_info->adm_shutdown = FALSE;

      /* Push the unlock mib set to PSS store */
      avm_push_admin_mib_set_to_psr(avm_cb, ent_info, AVM_ADM_UNLOCK);

    for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
      if(AVM_ENT_EXTRACT_REQ == child->ent_info->current_state)
        {
         /* The Sub-slot will also move in M5 or M6 state as the slot extraction is
                requested ,We need to reverse it too  */
          avm_hisv_api_cmd(child->ent_info, HS_RESOURCE_ACTIVE_SET, 0);
          avm_assign_state(child->ent_info, AVM_ENT_ACTIVE);
        }

      avm_assign_state(ent_info, AVM_ENT_ACTIVE);
      m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_ADM_OP);
      avm_hisv_api_cmd(ent_info, HS_RESOURCE_ACTIVE_SET, 0);

      return NCSCC_RC_SUCCESS;
   }
/* Success Case : If it is  parent entity bring all the Child Enitiy to Inactive state too */

  if(ent_info->is_fru)  /* Managed FRU  */
  {
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
     {
      if(AVM_ENT_EXTRACT_REQ == child->ent_info->current_state)
       {
          rc = avm_hisv_api_cmd(child->ent_info, HS_RESOURCE_INACTIVE_SET, 0);
          if(NCSCC_RC_SUCCESS != rc)
         { 
               m_AVM_LOG_GEN_EP_STR("Entity Inactivation failure received  ", child->ent_info->ep_str.name, NCSFL_SEV_NOTICE);
              /* If the above call failed, generate a trap to notify this */
                 avm_entity_inactive_hisv_ret_error_trap(avm_cb, child->ent_info);
        }
          avm_assign_state(child->ent_info, AVM_ENT_INACTIVE);
        /* Send a trap to notify that the entity has been locked */
         if(NCSCC_RC_SUCCESS == rc)
            avm_entity_locked_trap(avm_cb, child->ent_info);
       }
     }

     rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
     if(NCSCC_RC_SUCCESS != rc)
    {
           m_AVM_LOG_GEN_EP_STR("Entity Inactivation failure received  ", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
          /* If the above call failed, generate a trap to notify this */
            avm_entity_inactive_hisv_ret_error_trap(avm_cb, ent_info);
     }
      avm_assign_state(ent_info, AVM_ENT_INACTIVE);
   /* Send a trap to notify that the entity has been locked */
         if(NCSCC_RC_SUCCESS == rc)
            avm_entity_locked_trap(avm_cb, ent_info);
   }
  return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_ext_req_sensor_deassert
 *
 * Description   : This function is invoked when a blade receives 
 *                 sensor assert in EXTRACTION_PENDING state
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************
 ******/
static uns32 avm_ext_req_sensor_assert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
  
   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_sensor_assert"); 
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_ASSERT);
   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_ext_req_sensor_deassert
 *
 * Description   : This function is invoked when a blade receives 
 *                 sensor deassert in EXTRACTION_PENDING state
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************
 ******/
static uns32 avm_ext_req_sensor_deassert(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_sensor_deassert"); 
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SENSOR_DEASSERT);
   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_ext_req_surp_extract
 *
 * Description   : This function is invoked when a blade is
                   extracted without notice.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************
 ******/
static uns32 avm_ext_req_surp_extract(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_surp_extract"); 
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_SURP_EXTRACTION);
   return rc;
}

/***********************************************************************
 ******
 * Name          : avm_ext_req_active
 *
 * Description   : This function is invoked when  a latch is closed with
 *                 out waiting for the response from AvM for extraction
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************
 ******/
static uns32
avm_ext_req_active(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_active");
   rc = avm_scope_fault(avm_cb, ent_info, AVM_SCOPE_ACTIVE);
   return rc;

}

/***********************************************************************
 ******
 * Name          : avm_ext_req_inactive
 *
 * Description   : This function is invoked when  a blade moves to Inactive 
 *                 with out waiting for the response from AvM for 
 *                 extraction
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************
 ******/
static uns32
avm_ext_req_inactive(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T      head; 


   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_inactive");

   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

   /* Set child entity in inactive state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
     if((AVM_ENT_INACTIVE < (child->ent_info->current_state)) && (AVM_ENT_INVALID >(child->ent_info->current_state)))
          avm_assign_state(child->ent_info, AVM_ENT_INACTIVE);

   avm_assign_state(ent_info, AVM_ENT_INACTIVE);

   return rc;

}

/***********************************************************************
 ******
 * Name          : avm_ext_req_avd_reset_req
 *
 * Description   : This function is invoked when AvD issues q reset req
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************
 ******/
static uns32
avm_ext_req_avd_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_NODE_RESET_RESP_T resp;

   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_avd_reset_req");

   rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_GRACEFUL_REBOOT);

   resp = (rc == NCSCC_RC_SUCCESS) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_AVM_LOG_GEN_EP_STR("AvD issued Reset/Cold Fail", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   }else
   { 
     m_AVM_LOG_GEN_EP_STR("AvD issued Reset/Cold Successful", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
   }

   avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);     
   
   return rc;
}

static uns32
avm_ext_req_adm_soft_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_FAILURE;

   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "Soft reset of an entity in extraction-pending state is not valid";
  
   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_adm_soft_reset_req");

   evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

   evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

   m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

   memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len); 

   return rc;
}

static uns32
avm_ext_req_adm_hard_reset_req(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc;
   AVM_NODE_RESET_RESP_T resp;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "HISV resource reset returned error.";
   uns8 *cli_msg1 = "AvM failed to reset entity using HISV but AvD will attempt NCS Reboot.";
  
   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_adm_hard_reset_req"); 

   rc = avm_hisv_api_cmd(ent_info, HISV_RESOURCE_RESET, HISV_RES_COLD_RESET);
   if(NCSCC_RC_SUCCESS != rc)
   {
      if(avm_is_this_entity_self(avm_cb, ent_info->entity_path) == TRUE)
      {
         evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg1);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

         memcpy(evt->evt.mib_req->rsp.add_info, cli_msg1, evt->evt.mib_req->rsp.add_info_len);

         m_AVM_LOG_GEN_EP_STR("AvM failed to reset using HISV but AvD will attempt NCS Reboot of entity:",
                                    ent_info->ep_str.name, NCSFL_SEV_ERROR);
      }
      else
      {
         evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

         evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

         m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

         memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

         m_AVM_LOG_GEN_EP_STR("Adm Hard Reset/Cold Fail", ent_info->ep_str.name, NCSFL_SEV_ERROR);
      }
   }else
   { 
     m_AVM_LOG_GEN_EP_STR("Adm Hard Reset/Cold Successful", ent_info->ep_str.name, NCSFL_SEV_INFO);
   }
   
   resp = (rc == NCSCC_RC_SUCCESS) ? AVM_NODE_RESET_SUCCESS : AVM_NODE_RESET_FAILURE;
   avm_avd_node_reset_resp(avm_cb, resp, ent_info->node_name);

   return rc;
}

static uns32
avm_ext_req_adm_shutdown(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc  = NCSCC_RC_SUCCESS;
   
   m_AVM_LOG_GEN_EP_STR("Adm shutdown issued in Ext-Req", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
   ent_info->adm_lock = AVM_ADM_LOCK; 
   return rc;
}

static uns32
avm_ext_req_adm_lock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc;
   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;

   uns8 *cli_msg = "HISV returned error, so entity could not be locked";
  
   m_AVM_LOG_FUNC_ENTRY("avm_ext_req_adm_lock");

   rc = avm_hisv_api_cmd(ent_info, HS_RESOURCE_INACTIVE_SET, 0);
   if(NCSCC_RC_SUCCESS != rc)
   {  
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("Adm Lock Failed in Ext-Req", ent_info->ep_str.name, NCSFL_SEV_ERROR);
   }else
   {
      for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
        if((AVM_ENT_INACTIVE < (child->ent_info->current_state)) && (AVM_ENT_INVALID >(child->ent_info->current_state)))
            avm_assign_state(ent_info, AVM_ENT_INACTIVE);

      m_AVM_LOG_GEN_EP_STR("Adm Lock Successful in Ext-Req", ent_info->ep_str.name, NCSFL_SEV_NOTICE);
      ent_info->adm_lock = AVM_ADM_LOCK; 
      avm_assign_state(ent_info, AVM_ENT_INACTIVE);
   }
   return rc;
}

static uns32
avm_ext_req_adm_unlock(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc =  NCSCC_RC_FAILURE;

   AVM_EVT_T *evt = (AVM_EVT_T*)fsm_evt;

   uns8 *cli_msg = "Unlock can not be done on the entity in extraction pending state.";
 
   evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

   evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

   m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

   memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);
  
   m_AVM_LOG_GEN_EP_STR("AdM Unlock invalid in Ext-Req", ent_info->ep_str.name, NCSFL_SEV_INFO);
   return rc;
}

/***************************************************************************
 ******
 * Name          : avm_ext_req_extracted
 *
 * Description   : This function is invoked when  a blade is waiting for
 *                 AvDs response to be deactivated and and Event Loss of 
 *                 its transition to SAHPI_HS_STATE_INACTIVE happenned and
 *                 blade state is SAHPI_HS_STATE_NOT_PRESENT 
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 a Reset Req has been issued from AvD.
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***************************************************************************
 ******/
static uns32
avm_ext_req_extracted(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T head;

   /* stop the upgrade progress timer */
   if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);

   if(ent_info->adm_shutdown)
   {
      ent_info->adm_lock = AVM_ADM_LOCK;
      ent_info->adm_shutdown = FALSE;
   }
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity in not present state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
      avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT);

   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT);

   return rc;
}

static uns32
avm_ext_req_resource_fail(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   AVM_ENT_INFO_LIST_T   *child = AVM_ENT_INFO_LIST_NULL ;
   AVM_LIST_HEAD_T head;
   
   avm_list_nodes_chgd(ent_info, &head);
   avm_avd_node_operstate(avm_cb, &head, AVM_NODES_ABSENT);

  /* Set child entity  in not present state */
   for(child = ent_info->child; child != AVM_ENT_INFO_LIST_NULL; child = child->next)
        avm_assign_state(child->ent_info, AVM_ENT_NOT_PRESENT);

   avm_assign_state(ent_info, AVM_ENT_NOT_PRESENT);
   return rc;
}

/***********************************************************************
 ******
  * Name          : avm_handle_fw_progress_event
  *
  * Description   : This function is invoked when the Firmware progress
  *                 sensor event is received from the HISv.
  *
  * Arguments     : AVM_CB_T*    - Pointer to AvM CB
  *                 AVM_ENT_INFO - Pointer to the entity to which
  *                 a hotswap event with current state SAHPI_HS_NOT_PRES
  *                 ENT received.
  *                 void*        - AvM Event.
  *
  * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  *
  * Notes         : None
************************************************************************
*****/
static uns32
avm_handle_fw_progress_event(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 evt_state = 0, evt_type = 0;
   SaHpiEventT *hpi_event;
   uns8 logbuf[500];
   uns32     flag=0;
   hpi_event = &((AVM_EVT_T*)fsm_evt)->evt.hpi_evt->hpi_event;
   ent_info->boot_prg_status = 0;

   m_AVM_LOG_FUNC_ENTRY("avm_handle_fw_progress_event");

   evt_state = (uns8)hpi_event->EventDataUnion.SensorEvent.EventState;
   evt_state = (evt_state >> 1);

   /* Get the Firmware progress event type */
   if (hpi_event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_SENSOR_SPECIFIC)
   {
      evt_type = hpi_event->EventDataUnion.SensorEvent.SensorSpecific;
      ent_info->boot_prg_status = (evt_state << 16) | (evt_type << 8) | 0xff;
   }
   else
   if (hpi_event->EventDataUnion.SensorEvent.OptionalDataPresent & SAHPI_SOD_OEM)
   {
      evt_type = (hpi_event->EventDataUnion.SensorEvent.Oem) >> 8;
      ent_info->boot_prg_status = (evt_state << 16) | evt_type | 0xfd00;
   }
   else
   {
      /* Log it ?? */
      return NCSCC_RC_SUCCESS;
   }


   /* Boot Progress trap */
   avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmBootProgress_ID);

   if(evt_type == HPI_FWPROG_BOOT_SUCCESS)
   {
      avm_stop_tmr(avm_cb, &ent_info->boot_succ_tmr);
   } 
   /* Check for Success or failure of the firmware progress event */
   if (evt_state == HPI_EVT_FWPROG_PROG)
   {
      /* 
       * If Node Init Success is received and if upgrade is in progress, 
       * then stop the upgrade success timer allow commit operations. 
       * Send SNMP trap.
       */
      if (evt_type == HPI_FWPROG_NODE_INIT_SUCCESS)
      {
         if (NULL != ent_info->dhcp_serv_conf.curr_act_label->name.name)
         { 
            logbuf[0] = '\0'; 
            sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Came up with %s",ent_info->ep_str.name,ent_info->dhcp_serv_conf.curr_act_label->name.name);
            m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
         }

         if(ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
         {
            /* Handle node Init success */
            avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);
            avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmUpgradeSuccess_ID);
            ent_info->dhcp_serv_conf.upgd_prgs = FALSE;
            sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Upgraded successfully with %s",
                         ent_info->ep_str.name,ent_info->dhcp_serv_conf.curr_act_label->name.name);
            m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
         } 
      }
      /* Reception of this progress code implies 7221 phoenix bios */
      /* was upgrade and 7221 was reset. We would start a timer to */
      /* check if the new bios boots up successfully               */
      else if (evt_type == HPI_FWPROG_PHOENIXBIOS_UPGRADE_SUCCESS)
      {

        if(avm_check_config(ent_info->ep_str.name,&flag)==NCSCC_RC_FAILURE)
        {
          sysf_sprintf(logbuf, "AVM-SSU: ssuHepler.conf file open has failed %s ","ssuHelper.conf" );
          m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
          return NCSCC_RC_FAILURE;
        }
        else
        {
          if(flag==1)
          {
            /* flag=1 suggest that present blade is not 7221 its 7150 which has */
            /*has ipmc support of bios rollback if upgraded bios is currupted*/
          sysf_sprintf(logbuf, "AVM-SSU: present blade is 7150  %s ", " ");
          m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
          }
          else
          {
            sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Bios Upgraded successfully", ent_info->ep_str.name);
            m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
            m_AVM_SSU_BIOS_UPGRADE_TMR_START(avm_cb, ent_info);
            ent_info->dhcp_serv_conf.bios_upgd_state = BIOS_TMR_STARTED;
            m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
          }
        }
      }

      /* Reception of this progress code is taken as indication  */
      /* of successful bios boot up. Hence stop the bios upgrade */ 
      /* timer                                                   */
      else if (evt_type == HPI_FWPROG_SYS_BOOT)
      {
         sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Bios Boot Success", ent_info->ep_str.name);
         m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);

         avm_stop_tmr(avm_cb, &ent_info->bios_upgrade_tmr);
         ent_info->dhcp_serv_conf.bios_upgd_state = 0;  /* reset the bios_upgd_state */
         m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
      }
   } /* consider upgrade failure only if it is OEM specific error event       */
     /* For 7221's even OEM sent errors come as "SAHPI_SOD_SENSOR_SPECIFIC"   */
     /* Hence making the handling based on error rather than SENSOR OR OEM    */
     /* Further any error condition should result in role back effort, rather */
     /* than only OEM specific                                                */
   else if ((evt_state == HPI_EVT_FWPROG_ERROR) || (evt_state == HPI_EVT_FWPROG_HANG))   
   {
      if (ent_info->dhcp_serv_conf.upgd_prgs == TRUE)
      {
         avm_stop_tmr(avm_cb, &ent_info->upgd_succ_tmr);
         /* 
          * Upgrade Error. Rollback to old label.
          */

         /* If Bios upgrade timer is running, it should be stopped */
         /* and bios bank should be switched                       */
         if(ent_info->bios_upgrade_tmr.status == AVM_TMR_RUNNING)
         {
/* Means upgraded Bios is faulty. We would defer the rollback action  */
/* till Bios upgrade timer expiry.                                    */
/* Going for above because during failover scenarion, when we process */
/* System firmware error code, we wont be able to switch bios bank    */
/* unless openhpi infrastructure is UP. That range could be 1-->3 mins*/
/* Hence handling bios boot failure case using timer expiry only      */
/* It helps us avoid infrastructure constraint caused by openhpi      */
/* Once above openhpi constraint is removed, below code can be made   */
/* functional                                                         */

            /* async update */
            sysf_sprintf(logbuf, "AVM-SSU: Payload %s : Upgrade Failed : BIOS UPGRADE TIMER RUNNING ", ent_info->ep_str.name);
            m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);

            m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);
            return rc;
         } 
         sysf_sprintf(logbuf, "AVM-SSU: Payload blade %s : Upgrade Failed with %s, BootProgressStatus=%x",
                    ent_info->ep_str.name,ent_info->dhcp_serv_conf.curr_act_label->name.name,ent_info->boot_prg_status);
         m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
        avm_cb->upgrade_error_type = GEN_ERROR;
        avm_cb->upgrade_module = NCS_BIOS;
        avm_send_boot_upgd_trap(avm_cb, ent_info, ncsAvmSwFwUpgradeFailure_ID);

         avm_ssu_dhcp_rollback(avm_cb, ent_info);
         sysf_sprintf(logbuf, "AVM-SSU: Payloadblade %s : Rolling back to %s",
                               ent_info->ep_str.name,ent_info->dhcp_serv_conf.curr_act_label->name.name);
         m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_NOTICE);
         
         ent_info->dhcp_serv_conf.upgd_prgs = FALSE;
      }
      else
      {
         if(ent_info->bios_upgrade_tmr.status == AVM_TMR_RUNNING)
         {
            avm_stop_tmr(avm_cb, &ent_info->bios_upgrade_tmr);
            ent_info->dhcp_serv_conf.bios_upgd_state = 0;
            m_AVM_SEND_CKPT_UPDT_SYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_UPGD_STATE_CHG);
            logbuf[0] = '\0';
            sysf_sprintf(logbuf, "AVM-SSU: Payload %s : BIOS UPGRADE TIMER RUNNING : Rollback Failed", ent_info->ep_str.name);
            m_AVM_LOG_DEBUG(logbuf,NCSFL_SEV_CRITICAL);
            return NCSCC_RC_FAILURE;
         } 
      }
   }
   else
   {
      /* Some junk state we have got, log it */
   }
   /* async update */
   m_AVM_SEND_CKPT_UPDT_ASYNC_UPDT(avm_cb, ent_info, AVM_CKPT_ENT_DHCP_STATE_CHG);

   return rc;
}


/***********************************************************************
 ******
 * Name          : avm_handle_ignore_event
 *
 * Description   : This function is invoked when  an event is received 
 *                 which does not have implications on FSM.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received. 
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************
 ******/
extern uns32 
avm_handle_ignore_event(
                        AVM_CB_T          *avm_cb, 
                        AVM_ENT_INFO_T    *ent_info, 
                        void              *fsm_evt
                       )
{
   m_AVM_LOG_EVT_IGN(AVM_LOG_EVT_IGNORE, ((AVM_EVT_T*)fsm_evt)->fsm_evt_type, ent_info->current_state, NCSFL_SEV_INFO);
   return NCSCC_RC_SUCCESS;
}
   

/***********************************************************************
 ******
 * Name          : avm_handle_invalid_event
 *
 * Description   : This function is invoked when  an event is received 
 *                 which is invalid.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *                 AVM_ENT_INFO - Pointer to the entity to which
 *                 the event has been received. 
 *                 void*        - AvM Event.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         :  May the event itself is invalid or a particular
 *                  event is invalid in a particular state. 
 ************************************************************************
 ******/
extern uns32 
avm_handle_invalid_event(
                           AVM_CB_T           *avm_cb,
                           AVM_ENT_INFO_T     *ent_info,
                           void               *fsm_evt
                        )
{

   AVM_EVT_T *evt;
   uns32 rc = NCSCC_RC_FAILURE;

   evt = (AVM_EVT_T*)fsm_evt;
   uns8 *cli_msg = "Soft/Hard reset issued in Not Persent or Inactive state";

   if( (evt->fsm_evt_type == AVM_EVT_ADM_SOFT_RESET_REQ)|| (evt->fsm_evt_type == AVM_EVT_ADM_HARD_RESET_REQ) )
   {
      evt->evt.mib_req->rsp.add_info_len = m_NCS_OS_STRLEN(cli_msg);

      evt->evt.mib_req->rsp.add_info = m_MMGR_ALLOC_MIB_OCT(evt->evt.mib_req->rsp.add_info_len + 1);

      m_NCS_OS_MEMSET(evt->evt.mib_req->rsp.add_info, 0, evt->evt.mib_req->rsp.add_info_len + 1);

      memcpy(evt->evt.mib_req->rsp.add_info, cli_msg, evt->evt.mib_req->rsp.add_info_len);

      m_AVM_LOG_GEN_EP_STR("SAHPI_HS_ACTION_INSERTION", ent_info->ep_str.name, NCSFL_SEV_INFO);
      return rc;
   }

   if(evt->src == AVM_EVT_AVD)
   {
      rc = avm_handle_avd_error(avm_cb, ent_info, evt);
   }

   m_AVM_LOG_EVT_INVALID(AVM_LOG_EVT_INVALID, ((AVM_EVT_T*)fsm_evt)->fsm_evt_type, ent_info->current_state, NCSFL_SEV_WARNING);

   return rc;
}   

extern uns32
avm_handle_avd_error(
                        AVM_CB_T       *avm_cb, 
                        AVM_ENT_INFO_T *ent_info, 
                        AVM_EVT_T       *evt
                     )
{

   AVM_NODE_RESET_RESP_T resp = AVM_NODE_INVALID_REQ;
   uns32 rc = NCSCC_RC_SUCCESS;

   if(AVM_ENT_INFO_NULL == ent_info)
   {
      resp = AVM_NODE_INVALID_NAME;
   }

   switch(evt->fsm_evt_type)
   {
      case AVM_EVT_AVD_RESET_REQ:
      {
         rc = avm_avd_node_reset_resp(avm_cb, resp, evt->evt.avd_evt->avd_avm_msg.reset_req.node_name);
      }
      break;
      default:
      {
         return NCSCC_RC_FAILURE;
      }
      break;
   }
   
   return rc;
}

