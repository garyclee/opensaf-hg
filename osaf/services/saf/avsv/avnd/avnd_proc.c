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

  DESCRIPTION:This module contains the array of function pointers, indexed by
  the events for the AvND. It also has the processing
  function that calls into one of these function pointers based on the event.

 
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <poll.h>
#include <sched.h>

#include <logtrace.h>

#include "avnd.h"
#include "avnd_mon.h"

#define FD_MBX   0
#define FD_USR1  1
#define FD_MBCSV 2

char *avnd_evt_type_name[] = {
	"AVND_EVT_INVALID",
	"AVND_EVT_AVD_NODE_UPDATE_MSG",
	"AVND_EVT_AVD_NODE_UP_MSG",
	"AVND_EVT_AVD_REG_HLT_MSG",
	"AVND_EVT_AVD_REG_SU_MSG",
	"AVND_EVT_AVD_REG_COMP_MSG",
	"AVND_EVT_AVD_INFO_SU_SI_ASSIGN_MSG",
	"AVND_EVT_AVD_NODE_ON_FOVER",
	"AVND_EVT_AVD_PG_TRACK_ACT_RSP_MSG",
	"AVND_EVT_AVD_PG_UPD_MSG",
	"AVND_EVT_AVD_OPERATION_REQUEST_MSG",
	"AVND_EVT_AVD_HB_INFO_MSG",
	"AVND_EVT_AVD_SU_PRES_MSG",
	"AVND_EVT_AVD_VERIFY_MSG",
	"AVND_EVT_AVD_ACK_MSG",
	"AVND_EVT_AVD_SHUTDOWN_APP_SU_MSG",
	"AVND_EVT_AVD_SET_LEDS_MSG",
	"AVND_EVT_AVD_COMP_VALIDATION_RESP_MSG",
	"AVND_EVT_AVD_ROLE_CHANGE_MSG",
	"AVND_EVT_AVD_ADMIN_OP_REQ_MSG",
	"AVND_EVT_AVA_FINALIZE",
	"AVND_EVT_AVA_COMP_REG",
	"AVND_EVT_AVA_COMP_UNREG",
	"AVND_EVT_AVA_PM_START",
	"AVND_EVT_AVA_PM_STOP",
	"AVND_EVT_AVA_HC_START",
	"AVND_EVT_AVA_HC_STOP",
	"AVND_EVT_AVA_HC_CONFIRM",
	"AVND_EVT_AVA_CSI_QUIESCING_COMPL",
	"AVND_EVT_AVA_HA_GET",
	"AVND_EVT_AVA_PG_START",
	"AVND_EVT_AVA_PG_STOP",
	"AVND_EVT_AVA_ERR_REP",
	"AVND_EVT_AVA_ERR_CLEAR",
	"AVND_EVT_AVA_RESP",
	"AVND_EVT_CLA_FINALIZE",
	"AVND_EVT_CLA_TRACK_START",
	"AVND_EVT_CLA_TRACK_STOP",
	"AVND_EVT_CLA_NODE_GET",
	"AVND_EVT_CLA_NODE_ASYNC_GET",
	"AVND_EVT_TMR_HC",
	"AVND_EVT_TMR_CBK_RESP",
	"AVND_EVT_TMR_SND_HB",
	"AVND_EVT_TMR_RCV_MSG_RSP",
	"AVND_EVT_TMR_CLC_COMP_REG",
	"AVND_EVT_TMR_SU_ERR_ESC",
	"AVND_EVT_TMR_NODE_ERR_ESC",
	"AVND_EVT_TMR_CLC_PXIED_COMP_INST",
	"AVND_EVT_TMR_CLC_PXIED_COMP_REG",
	"AVND_EVT_MDS_AVD_UP",
	"AVND_EVT_MDS_AVD_DN",
	"AVND_EVT_MDS_AVA_DN",
	"AVND_EVT_MDS_CLA_DN",
	"AVND_EVT_MDS_AVND_DN",
	"AVND_EVT_MDS_AVND_UP",
	"AVND_EVT_HA_STATE_CHANGE",
	"AVND_EVT_CLC_RESP",
	"AVND_EVT_AVND_AVND_MSG",
	"AVND_EVT_COMP_PRES_FSM_EV",
	"AVND_EVT_LAST_STEP_TERM",
	"AVND_EVT_PID_EXIT",
};

static NCS_SEL_OBJ usr1_sel_obj; /* Selection object for USR1 signal events */

static NCS_BOOL avnd_mbx_process(SYSF_MBX *);
static void avnd_evt_process(AVND_EVT *);

static uns32 avnd_evt_invalid_func(AVND_CB *cb, AVND_EVT *evt);

/* list of all the function pointers related to handling the events
*/
const AVND_EVT_HDLR g_avnd_func_list[AVND_EVT_MAX] = {
	/* invalid event */
	avnd_evt_invalid_func,	/* AVND_EVT_INVALID */

	/* AvD message event types */
	avnd_evt_avd_node_update_msg,	/* AVND_EVT_AVD_NODE_UPDATE_MSG */
	avnd_evt_avd_node_up_msg,	/* AVND_EVT_AVD_NODE_UP_MSG */
	NULL,				/* AVND_EVT_AVD_REG_HLT_MSG */
	avnd_evt_avd_reg_su_msg,	/* AVND_EVT_AVD_REG_SU_MSG */
	avnd_evt_avd_reg_comp_msg,	/* AVND_EVT_AVD_REG_COMP_MSG */
	avnd_evt_avd_info_su_si_assign_msg,	/* AVND_EVT_AVD_INFO_SU_SI_ASSIGN_MSG */
	avnd_evt_avd_clm_node_on_fover,	/* AVND_EVT_AVD_NODE_ON_FOVER */
	avnd_evt_avd_pg_track_act_rsp_msg,	/* AVND_EVT_AVD_PG_TRACK_ACT_RSP_MSG */
	avnd_evt_avd_pg_upd_msg,	/* AVND_EVT_AVD_PG_UPD_MSG */
	avnd_evt_avd_operation_request_msg,	/* AVND_EVT_AVD_OPERATION_REQUEST_MSG */
	avnd_evt_avd_hb_info_msg,	/* AVND_EVT_AVD_HB_INFO_MSG */
	avnd_evt_avd_su_pres_msg,	/* AVND_EVT_AVD_SU_PRES_MSG */
	avnd_evt_avd_verify_message,	/* AVND_EVT_AVD_VERIFY_MSG */
	avnd_evt_avd_ack_message,	/* AVND_EVT_AVD_ACK_MSG */
	avnd_evt_avd_shutdown_app_su_msg,	/* AVND_EVT_AVD_SHUTDOWN_APP_SU_MSG */
	avnd_evt_avd_set_leds_msg,	/* AVND_EVT_AVD_SET_LEDS_MSG */
	avnd_evt_avd_comp_validation_resp_msg,	/* AVND_EVT_AVD_COMP_VALIDATION_RESP_MSG */
	avnd_evt_avd_role_change_msg,	/* AVND_EVT_AVD_ROLE_CHANGE_MSG */
	avnd_evt_avd_admin_op_req_msg,       /*  AVND_EVT_AVD_ADMIN_OP_REQ_MSG */

	/* AvA event types */
	avnd_evt_ava_finalize,	/* AVND_EVT_AVA_AMF_FINALIZE */
	avnd_evt_ava_comp_reg,	/* AVND_EVT_AVA_COMP_REG */
	avnd_evt_ava_comp_unreg,	/* AVND_EVT_AVA_COMP_UNREG */
	avnd_evt_ava_pm_start,	/* AVND_EVT_AVA_PM_START */
	avnd_evt_ava_pm_stop,	/* AVND_EVT_AVA_PM_STOP */
	avnd_evt_ava_hc_start,	/* AVND_EVT_AVA_HC_START */
	avnd_evt_ava_hc_stop,	/* AVND_EVT_AVA_HC_STOP */
	avnd_evt_ava_hc_confirm,	/* AVND_EVT_AVA_HC_CONFIRM */
	avnd_evt_ava_csi_quiescing_compl,	/* AVND_EVT_AVA_CSI_QUIESCING_COMPL */
	avnd_evt_ava_ha_get,	/* AVND_EVT_AVA_HA_GET */
	avnd_evt_ava_pg_start,	/* AVND_EVT_AVA_PG_START */
	avnd_evt_ava_pg_stop,	/* AVND_EVT_AVA_PG_STOP */
	avnd_evt_ava_err_rep,	/* AVND_EVT_AVA_ERR_REP */
	avnd_evt_ava_err_clear,	/* AVND_EVT_AVA_ERR_CLEAR */
	avnd_evt_ava_resp,	/* AVND_EVT_AVA_RESP */

	/* CLA event types */
	avnd_evt_cla_finalize,	/* AVND_EVT_CLA_FINALIZE */
	avnd_evt_cla_track_start,	/* AVND_EVT_CLA_TRACK_START */
	avnd_evt_cla_track_stop,	/* AVND_EVT_CLA_TRACK_STOP */
	avnd_evt_cla_node_get,	/* AVND_EVT_CLA_NODE_GET */
	avnd_evt_cla_node_async_get,	/* AVND_EVT_CLA_NODE_ASYNC_GET */

	/* timer event types */
	avnd_evt_tmr_hc,	/* AVND_EVT_TMR_HC */
	avnd_evt_tmr_cbk_resp,	/* AVND_EVT_TMR_CBK_RESP */
	avnd_evt_tmr_snd_hb,	/* AVND_EVT_TMR_SND_HB */
	avnd_evt_tmr_rcv_msg_rsp,	/* AVND_EVT_TMR_RCV_MSG_RSP */
	avnd_evt_tmr_clc_comp_reg,	/* AVND_EVT_TMR_CLC_COMP_REG */
	avnd_evt_tmr_su_err_esc,	/* AVND_EVT_TMR_SU_ERR_ESC */
	avnd_evt_tmr_node_err_esc,	/* AVND_EVT_TMR_NODE_ERR_ESC */
	avnd_evt_tmr_clc_pxied_comp_inst,	/* AVND_EVT_TMR_CLC_PXIED_COMP_INST */
	avnd_evt_tmr_clc_pxied_comp_reg,	/* AVND_EVT_TMR_CLC_PXIED_COMP_REG */

	/* mds event types */
	avnd_evt_mds_avd_up,	/* AVND_EVT_MDS_AVD_UP */
	avnd_evt_mds_avd_dn,	/* AVND_EVT_MDS_AVD_DN */
	avnd_evt_mds_ava_dn,	/* AVND_EVT_MDS_AVA_DN */
	avnd_evt_mds_cla_dn,	/* AVND_EVT_MDS_CLA_DN */
	avnd_evt_mds_avnd_dn,	/* AVND_EVT_MDS_AVND_DN */
	avnd_evt_mds_avnd_up,	/* AVND_EVT_MDS_AVND_UP */

	/* HA State Change event types */
	avnd_evt_ha_state_change,	/* AVND_EVT_HA_STATE_CHANGE */

	/* clc event types */
	avnd_evt_clc_resp,	/* AVND_EVT_CLC_RESP */

	/* AvND to AvND Events. */
	avnd_evt_avnd_avnd_msg,	/* AVND_EVT_AVND_AVND_MSG */

	/* internal event types */
	avnd_evt_comp_pres_fsm_ev,	/* AVND_EVT_COMP_PRES_FSM_EV */
	avnd_evt_last_step_term,	/* AVND_EVT_LAST_STEP_TERM */
	avnd_evt_pid_exit_evt	/* AVND_EVT_PID_EXIT */
};

/**
 * USR1 signal is used when NID wants to stop OpenSAF (AMF).
 * 
 * @param sig
 */
static void sigusr1_handler(int sig)
{
	TRACE_ENTER();
	ncs_sel_obj_ind(usr1_sel_obj);
	signal(SIGUSR1, SIG_IGN);
}

/****************************************************************************
  Name          : avnd_main_process
 
  Description   : This routine is an entry point for the AvND task.
 
  Arguments     : -
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avnd_main_process(void)
{
	NCS_SEL_OBJ mbx_fd;
	NCS_BOOL avnd_exit = FALSE;
	struct pollfd fds[3];
	nfds_t nfds = 2;

	TRACE_ENTER();

	if (avnd_create() != NCSCC_RC_SUCCESS) {
		syslog(LOG_ERR, "ncspvt_svcs_startup failed\n");
		goto done;
	}

	/* Change scheduling class to real time. */
	{
		struct sched_param param = {.sched_priority = 79 };

		if (sched_setscheduler(0, SCHED_RR, &param) == -1)
			syslog(LOG_ERR, "Could not set scheduling class for avnd: %s",
				strerror(errno));
	}

	if (ncs_sel_obj_create(&usr1_sel_obj) != NCSCC_RC_SUCCESS) {
		LOG_ER("ncs_sel_obj_create failed");
		goto done;
	}

	if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
		LOG_ER("signal USR1 failed: %s", strerror(errno));
		goto done;
	}

	mbx_fd = ncs_ipc_get_sel_obj(&avnd_cb->mbx);
	fds[FD_MBX].fd = mbx_fd.rmv_obj;
	fds[FD_MBX].events = POLLIN;

	fds[FD_USR1].fd = usr1_sel_obj.rmv_obj;
	fds[FD_USR1].events = POLLIN;
#if FIXME
	if (avnd_cb->clmdb.type == AVSV_AVND_CARD_SYS_CON) {
		fds[FD_MBCSV].fd = avnd_cb->avnd_mbcsv_sel_obj;
		fds[FD_MBCSV].events = POLLIN;
		nfds++;
	}
#endif
	(void) nid_notify("AMFND", NCSCC_RC_SUCCESS, NULL);

	/* now wait forever */
	while (1) {
		int ret = poll(fds, nfds, -1);

		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}

			syslog(LOG_ERR, "%s: poll failed - %s", __FUNCTION__, strerror(errno));
			break;
		}

		if (fds[FD_MBX].revents & POLLIN) {
			avnd_exit = avnd_mbx_process(&avnd_cb->mbx);
			if (TRUE == avnd_exit) {
				if (avnd_cb->clmdb.type == AVSV_AVND_CARD_SYS_CON) {
					/*
					* FIXME: this is a hack until NID is removed
					* Kill amfd otherwise it will reboot the system.
					*/
					LOG_IN("Killing amfd...");
					system("killall opensaf_amfd");
				}
				break;
			}
		}

		if (fds[FD_USR1].revents & POLLIN)
			avnd_sigusr1_handler();
#if FIXME
		if ((avnd_cb->clmdb.type == AVSV_AVND_CARD_SYS_CON) &&
		    (fds[FD_MBCSV].revents & POLLIN)) {
			if (NCSCC_RC_SUCCESS != avnd_mbcsv_dispatch(avnd_cb, SA_DISPATCH_ALL)) {
				; /* continue with our loop. */
			}
		}
#endif
	}

	if (FALSE == avnd_exit)
		syslog(LOG_ERR, "%s: Thread Exited", __FUNCTION__);

done:
	/* Give some time for cleanup and reboot scipts */
	sleep(5);
	exit(0);
}

/****************************************************************************
  Name          : avnd_mbx_process
 
  Description   : This routine dequeues & processes the events from the avnd
                  mailbox.
 
  Arguments     : mbx - ptr to the mailbox
 
  Return Values : TRUE - avnd needs to be destroyed
                  FALSE - normal mbx processing
 
  Notes         : None
******************************************************************************/
NCS_BOOL avnd_mbx_process(SYSF_MBX *mbx)
{
	AVND_EVT *evt;

	/* process each event */
	while (0 != (evt = (AVND_EVT *)ncs_ipc_non_blk_recv(mbx))) {
		avnd_evt_process(evt);

		if (avnd_cb->destroy == TRUE)
			return TRUE;
	}

	return FALSE;
}

/****************************************************************************
  Name          : avnd_evt_process
 
  Description   : This routine processes the AvND event.
 
  Arguments     : mbx - ptr to the mailbox
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avnd_evt_process(AVND_EVT *evt)
{
	AVND_CB *cb = avnd_cb;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* nothing to process */
	if (!evt)
		goto done;

	TRACE_ENTER2("%s", avnd_evt_type_name[evt->type]);

	/* validate the event type */
	if ((evt->type <= AVND_EVT_INVALID) || (evt->type >= AVND_EVT_MAX)) {
		/* log */
		goto done;
	}

	/* Temp: AvD Down Handling */
	if (TRUE == cb->is_avd_down)
		goto done;

	/* log the event reception */
	m_AVND_LOG_EVT(evt->type, 0, 0, NCSFL_SEV_INFO);

	/* acquire cb write lock */
	m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);

	/* invoke the event handler */
	rc = g_avnd_func_list[evt->type] (cb, evt);

	if ((SA_AMF_HA_ACTIVE == cb->avail_state_avnd) || (SA_AMF_HA_QUIESCED == cb->avail_state_avnd)) {
		m_AVND_SEND_CKPT_UPDT_SYNC(cb, NCS_MBCSV_ACT_UPDATE, 0);
	}

	/* release cb write lock */
	m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);

	/* log the result of event processing */
	m_AVND_LOG_EVT(evt->type, 0,
		       (rc == NCSCC_RC_SUCCESS) ? AVND_LOG_EVT_SUCCESS : AVND_LOG_EVT_FAILURE, NCSFL_SEV_INFO);

done:
	TRACE_LEAVE2("%s", avnd_evt_type_name[evt->type]);

	/* free the event */
	if (evt)
		avnd_evt_destroy(evt);
}

/*****************************************************************************
 * Function: avnd_evt_invalid_func
 *
 * Purpose:  This function is the handler for invalid events. It just
 * dumps the event to the debug log and returns.
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static uns32 avnd_evt_invalid_func(AVND_CB *cb, AVND_EVT *evt)
{

	/* This function should never be called
	 * log the event to the debug log and return
	 */
	assert(0);

	return NCSCC_RC_SUCCESS;
}
