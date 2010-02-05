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
  This file containts the control block structure of HSM

******************************************************************************/

#ifndef HSM_CB_H
#define HSM_CB_H

#include <saEvt.h>

/* pattern array lengths for fault, non-fault events */
#define HPI_FAULT_PATTERN_ARRAY_LEN       2
#define HPI_NON_FAULT_PATTERN_ARRAY_LEN   4

#define HPI_EVT_GET_TIMEOUT           0x77359400
#define HISV_MAX_INV_DATA_SIZE        5120
#define HPI_DUPLICATE_EVENT_INTERVAL  12000000000
#define MAX_NUM_SLOTS                 16

/* outstanding hotswap states */
typedef enum node_hotswap_states {
	NODE_HS_STATE_NOT_PRESENT = 0,
	NODE_HS_STATE_INACTIVE,
	NODE_HS_STATE_INSERTION_PENDING,
	NODE_HS_STATE_ACTIVE_HEALTHY,
	NODE_HS_STATE_ACTIVE_UNHEALTHY,
	NODE_HS_STATE_EXTRACTION_PENDING
} NODE_HOTSWAP_STATES;

/* event types */
typedef enum hsm_evt_type {
	HSM_FAULT_EVT,
	HSM_NON_FAULT_EVT
} HSM_EVT_TYPE;

/* structure for HSM control block */

typedef struct his_hsm_info {
	uns32 cb_hdl;		/* CB hdl returned by hdl mngr */
	uns8 pool_id;		/* pool-id used by hdl mngr */
	uns32 prc_id;		/* process identifier */
	HPI_SESSION_ARGS *args;	/* HPI session arguments */
	NCSCONTEXT task_hdl;	/* HSM task handle */
	SaEvtHandleT eda_handle;	/* EDA initialization handle */
	SaEvtChannelHandleT chan_evt;	/* EDSv channel for publishing events */
	SaEvtEventHandleT fault_evt_handle;	/* Event handle for fault events */
	/* SaEvtEventHandleT    non_fault_evt_handle; */	/* Event handle for non fault events */
	SaNameT event_chan_name;	/* name of the event channel */
	SaEvtEventPatternArrayT fault_pattern;	/* pattern arrays */
	SaEvtEventPatternArrayT non_fault_pattern;
	SaHpiTimeT evt_time;	/* event time stamp */
	uns32 eds_init_success;	/* tracks the eds init status */
	uns8 node_state[MAX_NUM_SLOTS + 1];

} HSM_CB;

/**************************************************************************
 * function and extern variable declarations
 */

/* global control block handle for HSM */
EXTERN_C uns32 gl_hsm_hdl;
EXTERN_C uns32 hsm_eda_event_publish(HSM_EVT_TYPE event_type, uns8 *evt_data, uns32 evt_data_size, HSM_CB *hsm_cb);

#endif   /* HSM_CB_H */
