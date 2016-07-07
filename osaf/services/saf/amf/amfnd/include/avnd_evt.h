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
  
  AvND event definitions.
    
******************************************************************************
*/

#ifndef AVND_EVT_H
#define AVND_EVT_H

/* event type enums */
typedef enum avnd_evt_type {
	AVND_EVT_INVALID = 0,

	/* AvD message event types */
	AVND_EVT_AVD_NODE_UP_MSG,
	AVND_EVT_AVD_REG_SU_MSG,
	AVND_EVT_AVD_REG_COMP_MSG,
	AVND_EVT_AVD_INFO_SU_SI_ASSIGN_MSG,
	AVND_EVT_AVD_PG_TRACK_ACT_RSP_MSG,
	AVND_EVT_AVD_PG_UPD_MSG,
	AVND_EVT_AVD_OPERATION_REQUEST_MSG,
	AVND_EVT_AVD_SU_PRES_MSG,
	AVND_EVT_AVD_VERIFY_MSG,
	AVND_EVT_AVD_ACK_MSG,
	AVND_EVT_AVD_SHUTDOWN_APP_SU_MSG,
	AVND_EVT_AVD_SET_LEDS_MSG,
	AVND_EVT_AVD_COMP_VALIDATION_RESP_MSG,
	AVND_EVT_AVD_ROLE_CHANGE_MSG,
	AVND_EVT_AVD_ADMIN_OP_REQ_MSG,
	AVND_EVT_AVD_HEARTBEAT_MSG,
	AVND_EVT_AVD_REBOOT_MSG,
	AVND_EVT_AVD_MAX,

	/* AvA event types */
	AVND_EVT_AVA_FINALIZE = AVND_EVT_AVD_MAX,
	AVND_EVT_AVA_COMP_REG,
	AVND_EVT_AVA_COMP_UNREG,
	AVND_EVT_AVA_PM_START,
	AVND_EVT_AVA_PM_STOP,
	AVND_EVT_AVA_HC_START,
	AVND_EVT_AVA_HC_STOP,
	AVND_EVT_AVA_HC_CONFIRM,
	AVND_EVT_AVA_CSI_QUIESCING_COMPL,
	AVND_EVT_AVA_HA_GET,
	AVND_EVT_AVA_PG_START,
	AVND_EVT_AVA_PG_STOP,
	AVND_EVT_AVA_ERR_REP,
	AVND_EVT_AVA_ERR_CLEAR,
	AVND_EVT_AVA_RESP,
	AVND_EVT_AVA_MAX,

	/* timer event types */
	AVND_EVT_TMR_HC = AVND_EVT_AVA_MAX,
	AVND_EVT_TMR_CBK_RESP,
	AVND_EVT_TMR_RCV_MSG_RSP,
	AVND_EVT_TMR_CLC_COMP_REG,
	AVND_EVT_TMR_SU_ERR_ESC,
	AVND_EVT_TMR_NODE_ERR_ESC,
	AVND_EVT_TMR_CLC_PXIED_COMP_INST,
	AVND_EVT_TMR_CLC_PXIED_COMP_REG,
	AVND_EVT_TMR_HB_DURATION,
	AVND_EVT_TMR_MAX,

	/* mds event types */
	AVND_EVT_MDS_AVD_UP = AVND_EVT_TMR_MAX,
	AVND_EVT_MDS_AVD_DN,
	AVND_EVT_MDS_AVA_DN,
	AVND_EVT_MDS_AVND_DN,
	AVND_EVT_MDS_AVND_UP,
	AVND_EVT_MDS_MAX,

	/* HA State change event types */
	AVND_EVT_HA_STATE_CHANGE = AVND_EVT_MDS_MAX,
	AVND_EVT_HA_STATE_CHANGE_MAX,

	/* clc event types */
	AVND_EVT_CLC_RESP = AVND_EVT_HA_STATE_CHANGE_MAX,
	AVND_EVT_CLC_MAX,

	/* AvND to AvND event types */
	AVND_EVT_AVND_AVND_MSG = AVND_EVT_CLC_MAX,
	AVND_EVT_AVND_AVND_MAX,

	/* internal events (generated by AvND itself) */
	AVND_EVT_COMP_PRES_FSM_EV = AVND_EVT_AVND_AVND_MAX,
	AVND_EVT_LAST_STEP_TERM,
	AVND_EVT_PID_EXIT,
	AVND_EVT_TMR_QSCING_CMPL,
	AVND_EVT_IR,

	AVND_EVT_MAX
} AVND_EVT_TYPE;

/* AvA event definition */
typedef struct avnd_ava_evt {
	MDS_DEST mds_dest;
	AVSV_NDA_AVA_MSG *msg;
} AVND_AVA_EVT;

/* timer event definition */
typedef struct avnd_tmr_evt {
	uint32_t opq_hdl;
} AVND_TMR_EVT;

/* mds event definition */
typedef struct avnd_mds_evt {
	MDS_DEST mds_dest;	/* mds address */
	NODE_ID node_id;
} AVND_MDS_EVT;

/* HA STATE change event definition */
typedef struct avnd_ha_state_change_evt {
	SaAmfHAStateT ha_state;
} AVND_HA_STATE_CHANGE_EVT;

/* clc event definition */
typedef struct avnd_clc_evt {
	NCS_EXEC_HDL exec_ctxt;	/* execution context */
	NCS_OS_PROC_EXEC_STATUS_INFO exec_stat;	/* cmd execution status */
	SaNameT comp_name;	/* comp-name */
	AVND_COMP_CLC_CMD_TYPE cmd_type;	/* cmd-type */
} AVND_CLC_EVT;

typedef struct avnd_comp_fsm_evt {
	SaNameT comp_name;	/* comp-name */
	uint32_t ev;		/* comp fsm event */
} AVND_COMP_FSM_EVT;

/* Event record to send PID exit event */
typedef struct avnd_pm_mon_evt {
	SaNameT comp_name;
	SaUint64T pid;
	AVND_COMP_PM_REC *pm_rec;
} AVND_PM_MON_EVT;

typedef struct avnd_ir_evt {
	SaNameT su_name;      /* su-name */
	bool status;	/* Result of Imm read, true is succ. */
} AVND_IR_EVT;

/* AVND top-level event structure */
typedef struct avnd_evt_tag {
	struct avnd_evt_tag *next;
	MDS_SYNC_SND_CTXT mds_ctxt;
	MDS_CLIENT_MSG_FORMAT_VER msg_fmt_ver;
	NCS_IPC_PRIORITY priority;
	AVND_EVT_TYPE type;

	union {
		AVSV_DND_MSG *avd;
		AVSV_ND2ND_AVND_MSG *avnd;
		AVND_AVA_EVT ava;
		AVND_TMR_EVT tmr;
		AVND_MDS_EVT mds;
		AVND_HA_STATE_CHANGE_EVT ha_state_change;
		AVND_CLC_EVT clc;
		AVND_COMP_FSM_EVT comp_fsm;
		AVND_PM_MON_EVT pm_evt;
		AVND_IR_EVT ir_evt;
	} info;

} AVND_EVT;

/*** Extern function declarations ***/

struct avnd_cb_tag;

AVND_EVT *avnd_evt_create(struct avnd_cb_tag *, AVND_EVT_TYPE,
				   MDS_SYNC_SND_CTXT *, MDS_DEST *, void *, AVND_CLC_EVT *, AVND_COMP_FSM_EVT *);

void avnd_evt_destroy(AVND_EVT *);

uint32_t avnd_evt_send(struct avnd_cb_tag *, AVND_EVT *);

#endif   /* !AVND_EVT_H */
