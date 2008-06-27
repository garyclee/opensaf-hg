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

  $Header:

  ..............................................................................

  DESCRIPTION:
  This file containts the control block structure of SIM

******************************************************************************/

#ifndef SIM_CB_H
#define SIM_CB_H


/* structure for ShIM control block */
typedef struct his_sim_info
{
   uns32                cb_hdl;         /* CB hdl returned by hdl mngr */
   uns8                 pool_id;        /* pool-id used by hdl mngr */
   uns32                prc_id;         /* process identifier */
   NCSCONTEXT           task_hdl;       /* ShIM task handle */
   SYSF_MBX             mbx;            /* ShIM's mailbox       */
   NCS_LOCK             cb_lock;        /* Lock for this control Block */
   uns8                 fwprog_done[MAX_NUM_SLOTS+1]; /* firmware progress state */

} SIM_CB;

/* ShIM event types */
typedef enum sim_event_types
{
   HCD_SIM_HEALTH_CHECK_REQ,
   HCD_SIM_HEALTH_CHECK_RESP,
   HCD_SIM_FIRMWARE_PROGRESS,
   HCD_SIM_MAX_EVENT_TYPES

} HCD_SIM_EVT_TYPES;

/* message structure for ShIM mail box */
typedef struct sim_msg_tag
{
   SaHpiEventT         fp_evt;          /* firmware progress event */
   SaHpiEntityPathT    epath;           /* entity path of the resource */

} SIM_MSG;

/* event structure for ShIM mail box */
typedef struct sim_evt_tag
{
   /* NCS_IPC_MSG       next; */
   struct sim_evt_tag *next;
   uns32             cb_hdl;
   uns32     evt_type;
   SIM_MSG   msg;

} SIM_EVT;


/**************************************************************************
 * function and extern variable declarations
 */
EXTERN_C uns32 sim_process_evt(SIM_EVT *evt);
EXTERN_C uns32 sim_evt_destroy(SIM_EVT *sim_evt);

/* global control block handle for SIM */
EXTERN_C uns32 gl_sim_hdl;

#endif /* SIM_CB_H */

