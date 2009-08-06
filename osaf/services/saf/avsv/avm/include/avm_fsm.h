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
 
  DESCRIPTION:This file contains definitions for AvM FSM events.
  ..............................................................................

 
******************************************************************************
*/

#ifndef __NCS_AVM_FSM_H__
#define __NCS_AVM_FSM_H__

typedef struct avm_fsm_evt_type AVM_FSM_EVT_T;

typedef uns32
 (*AVM_FSM_FUNC_PTR_T) (AVM_CB_T *, AVM_ENT_INFO_T *, void *);

typedef uns32
 (*AVM_ROLE_FSM_FUNC_PTR_T) (AVM_CB_T *, AVM_EVT_T *);

/* AvM FSM states */

typedef enum avm_act_policy_type {
	AVM_SHELF_MGR_ACTIVATION = 1,
	AVM_SELF_VALIDATION_ACTIVATION,
	AVM_SELF_ACTIVATION,
	AVM_MAX_ACT_POLICY_TYPE
} AVM_ACT_POLICY_T;

typedef enum avm_adm_op_type {
	AVM_ADM_SHUTDOWN = 1,
	AVM_ADM_LOCK,
	AVM_ADM_UNLOCK,
} AVM_ADM_OP_T;

typedef enum avm_adm_op_reset_type {
	AVM_ENT_STABLE = 0,
	AVM_ADM_SOFT_RESET,
	AVM_ADM_HARD_RESET
} AVM_ADM_RESET_OP_T;

typedef enum avm_fsm_state {
	AVM_ENT_NOT_PRESENT,	/* FRU is in hand */
	AVM_ENT_INACTIVE,	/* Resource is in chassis and not yet powered */
	AVM_ENT_ACTIVE,		/* Resource is active */
	AVM_ENT_FAULTY,		/* Node is Faulty */
	AVM_ENT_RESET_REQ,	/* Node is in the process of getting reset */
	AVM_ENT_EXTRACT_REQ,	/* FRU is waiting to be extracted */
	AVM_ENT_INVALID,
	AVM_ENT_MAX_STATE = AVM_ENT_INVALID
} AVM_FSM_STATES_T;

/* AvM scope event types */
typedef enum avm_scope_hpi_event_type {
	AVM_SCOPE_EXTRACTION = 0,
	AVM_SCOPE_SURP_EXTRACTION,
	AVM_SCOPE_SENSOR_ASSERT,
	AVM_SCOPE_SENSOR_DEASSERT,
	AVM_SCOPE_ACTIVE,
	AVM_SCOPE_FAULT_DOMAIN_REQ,
	AVM_SCOPE_RESET
} AVM_SCOPE_EVENTS_T;

typedef enum avm_role_fsm_events_type {
	AVM_ROLE_EVT_AVM_CHG,	/* This is used by Active AvM to inform Standby AvM to change its role */
	AVM_ROLE_EVT_AVM_RSP,	/* This is used by Standby AvM to respond to Active AvM's AVM_ROLE_EVT_AVM_CHG req */
	AVM_ROLE_EVT_RDE_SET,	/* This used by RDE to set role to AvM */
	AVM_ROLE_EVT_AVD_ACK,	/* This is used by AvD to inform AvM about role acknowledgement */
	AVM_ROLE_EVT_AVD_HB,	/* AvD to inform AvD Heart Beat Lost to AvM */
	AVM_ROLE_EVT_AVND_HB,	/* AvD to inform AvND Heart Beat Lost to AvM */
	AVM_ROLE_EVT_ADM_SWITCH,
	AVM_ROLE_EVT_AVD_UP,
	AVM_ROLE_EVT_MDS_QUIESCED_ACK,
	AVM_ROLE_EVT_AVD_HB_RESTORE,	/* AvD to inform AvD Heart Beat is Restored to AvM */
	AVM_ROLE_EVT_AVND_HB_RESTORE,	/* AvD to inform AvND Heart Beat is Restored to AvM */
	AVM_ROLE_EVT_MAX = AVM_ROLE_EVT_AVND_HB_RESTORE
} AVM_ROLE_FSM_EVT_TYPE_T;

/* AvM events received from HPI, AVD and Admin */

typedef enum avm_fsm_events_type {
	AVM_EVT_FRU_INSTALL = 0,	/* FRU is installed in the shelf */
	AVM_EVT_INSERTION_PENDING,	/* Insertion Criteria are met and FRU is waiting to be validated by AvM */
	AVM_EVT_ENT_ACTIVE,	/* FRU is in active state */
	AVM_EVT_EXTRACTION_PENDING,	/*Extractiona criteria are met and FRU is waiting for graceful switch over to be done */
	AVM_EVT_ENT_EXTRACTED,	/* FRU is in hand */
	AVM_EVT_ENT_INACTIVE,	/* FRU is in the chassis but its not powered on */
	AVM_EVT_SURP_EXTRACTION,	/* FRU is extracted without prior notice to AvM */
	AVM_EVT_SENSOR_ASSERT,	/* Sensor has reached thresh hold levels */
	AVM_EVT_SENSOR_DEASSERT,	/* Sensor levels have been brought down to accpetable levels */
	AVM_EVT_SENSOR_FW_PROGRESS,	/* Firmware progress sensor event */
	AVM_EVT_RESOURCE_FAIL,
	AVM_EVT_RESOURCE_REST,
	AVM_EVT_FAULT_DOMAIN_REQ,	/* AvD has detected a a fault and want to scope it to all the effected nodes, so is the event */
	AVM_EVT_NODE_SHUTDOWN_RESP,	/* Shutdown Response from AvD */
	AVM_EVT_NODE_FAILOVER_RESP,	/* Failover response from AvD */
	AVM_EVT_AVD_RESET_REQ,
	AVM_EVT_ADM_RESET_REQ_BASE,
	AVM_EVT_ADM_SOFT_RESET_REQ = AVM_EVT_ADM_RESET_REQ_BASE,
	AVM_EVT_ADM_HARD_RESET_REQ,
	AVM_EVT_ADM_RESET_REQ_END = AVM_EVT_ADM_HARD_RESET_REQ,
	AVM_EVT_MIB_REQ,
	AVM_EVT_TMR_INIT_EDA_EXP,
	AVM_EVT_ADM_OP_BASE,
	AVM_EVT_ADM_SHUTDOWN = AVM_EVT_ADM_OP_BASE,
	AVM_EVT_ADM_LOCK,
	AVM_EVT_ADM_UNLOCK,
	AVM_EVT_ADM_OP_END = AVM_EVT_ADM_UNLOCK,
	AVM_EVT_IGNORE,
	AVM_EVT_INVALID,
	AVM_EVT_MAX = AVM_EVT_INVALID
} AVM_FSM_EVT_TYPE_T;

/* FSM table struct */
typedef struct avm_fsm_table_type {
	AVM_FSM_STATES_T current_state;
	AVM_FSM_EVT_TYPE_T evt;
	AVM_FSM_FUNC_PTR_T f_ptr;

} AVM_FSM_TABLE_T;

extern AVM_FSM_TABLE_T avm_fsm_table_g[AVM_ENT_MAX_STATE + 1][AVM_EVT_MAX + 1];

extern AVM_ROLE_FSM_FUNC_PTR_T avm_role_fsm_table_g[4][AVM_ROLE_EVT_MAX + 1];

extern uns32 avm_role_fsm_handler(AVM_CB_T *cb, AVM_EVT_T *fsm_evt);

extern uns32 avm_proc_ipmc_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T *avm_cb);

extern uns32 avm_proc_ipmc_mod_upgd_tmr_exp(AVM_EVT_T *my_evt, AVM_CB_T *avm_cb);

#endif   /*__NCS_AVM_FSM_H__*/
