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
  DESCRIPTION:

  This file contains the main() routine for FM.

******************************************************************************/

#include <configmake.h>
#include <daemon.h>
#include <logtrace.h>

#include <nid_api.h>
#include "fm.h"

char *role_string[] = { "Undefined", "ACTIVE", "STANDBY", "QUIESCED",
	"ASSERTING", "YIELDING", "UNDEFINED"
};

/*****************************************************************
 *                                                               *
 *         Prototypes for static functions                       *
 *                                                               *
 *****************************************************************/

static uns32 fm_agents_startup(void);
static uns32 fm_agents_shutdown(void);
static uns32 fm_get_args(FM_CB *);
static uns32 fm_hpl_init(void);
static uns32 fm_hpl_finalize(void);
static uns32 fm_can_smh_sw_process(FM_CB *, FM_EVT *);
static uns32 fm_conv_shelf_slot_to_entity_path(uns8 *, uns8, uns8, uns8);
static uns32 fms_fma_node_reset_intimate(FM_CB *, uns32, uns32);
static uns32 fms_reset_peer(FM_CB *);
static uns32 fm_nid_notify(uns32);
static uns32 fm_tmr_start(FM_TMR *, SaTimeT);
static void fm_mbx_msg_handler(FM_CB *, FM_EVT *);
static void fm_tmr_stop(FM_TMR *);
static void fm_tmr_exp(void *);

uns32 gl_fm_hdl;

extern void ava_install_amf_down_cb(void (*cb)(void));

static void amf_down(void)
{
	ncs_reboot("AMF down");
}

/*****************************************************************************

  PROCEDURE NAME:       main

  DESCRIPTION:          Main routine for FM

*****************************************************************************/
int main(int argc, char *argv[])
{
	FM_CB *fm_cb = NULL;
	NCS_SEL_OBJ mbx_sel_obj, pipe_sel_obj, amf_sel_obj, highest_sel_obj;
	NCS_SEL_OBJ_SET sel_obj_set;
	FM_EVT *fm_mbx_evt = NULL;

	daemonize(argc, argv);

	ava_install_amf_down_cb(amf_down);

	if (fm_agents_startup() != NCSCC_RC_SUCCESS) {
		/* notify the NID */
		printf("\nfm_agents_startup() failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_agents_startup_failed;
	}

	/* Allocate memory for FM_CB. */
	fm_cb = m_MMGR_ALLOC_FM_CB;
	if (NULL == fm_cb) {
		/* notify the NID */
		printf("\nCB Allocation failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_agents_startup_failed;
	}

	memset(fm_cb, 0, sizeof(FM_CB));

	/* Create CB handle */
	gl_fm_hdl = ncshm_create_hdl(NCS_HM_POOL_ID_COMMON, NCS_SERVICE_ID_GFM, (NCSCONTEXT)fm_cb);

	/* Take CB handle */
	ncshm_take_hdl(NCS_SERVICE_ID_GFM, gl_fm_hdl);

	if (fm_get_args(fm_cb) != NCSCC_RC_SUCCESS) {
		/* notify the NID */
		printf("\nfm_get_args() failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_get_args_failed;
	}

	/* Set the is_platform = FALSE. 
	 * Will be set to true when HISv MDS up is recieved 
	 */
	fm_cb->is_platform = FALSE;

	/* Create MBX. */
	if (m_NCS_IPC_CREATE(&fm_cb->mbx) != NCSCC_RC_SUCCESS) {
		printf("\nm_NCS_IPC_CREATE() failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_get_args_failed;
	}

	/* Attach MBX */
	if (m_NCS_IPC_ATTACH(&fm_cb->mbx) != NCSCC_RC_SUCCESS) {
		printf("\nm_NCS_IPC_ATTACH() failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_mbx_attach_failure;
	}

	/* MDS initialization */
	if (fm_mds_init(fm_cb) != NCSCC_RC_SUCCESS) {
		printf("\nfm_mds_init() failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_mds_init_failed;
	}

	/* RDA initialization */
	if (fm_rda_init(fm_cb) != NCSCC_RC_SUCCESS) {
		printf("\nfm_rda_init() failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_rda_init_failed;
	}

	/* HPL initialization */
	if (fm_hpl_init() != NCSCC_RC_SUCCESS) {
		printf("\nfm_hpl_init() failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_hpl_lib_init_failed;
	}

	/* Open FM pipe for receiving AMF up intimation */
	if (fm_amf_open(&fm_cb->fm_amf_cb) != NCSCC_RC_SUCCESS) {
		printf("\nfm pipe open failed (avm) failed.");
		fm_nid_notify((uns32)NCSCC_RC_FAILURE);
		goto fm_hpl_lib_init_failed;
	}

	/* Get mailbox selection object */
	mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&fm_cb->mbx);

	/* Get pipe selection object */
	m_SET_FD_IN_SEL_OBJ(fm_cb->fm_amf_cb.pipe_fd, pipe_sel_obj);

	/* Give CB hdl */
	ncshm_give_hdl(gl_fm_hdl);

	/* notify the NID */
	fm_nid_notify(NCSCC_RC_SUCCESS);

	/* clear selection object set */
	m_NCS_SEL_OBJ_ZERO(&sel_obj_set);

	/* Set only one pipe eelection object in set */
	m_NCS_SEL_OBJ_SET(pipe_sel_obj, &sel_obj_set);

	highest_sel_obj = pipe_sel_obj;

	/* Wait for pipe selection object */
	if (m_NCS_SEL_OBJ_SELECT(highest_sel_obj, &sel_obj_set, NULL, NULL, NULL) != -1) {
		/* following if will be true only first time */
		if (m_NCS_SEL_OBJ_ISSET(pipe_sel_obj, &sel_obj_set)) {
			/* Take handle */
			ncshm_take_hdl(NCS_SERVICE_ID_GFM, gl_fm_hdl);

			/* Process the message received on pipe */
			fm_amf_pipe_process_msg(&fm_cb->fm_amf_cb);

			/* Get amf selection object */
			m_SET_FD_IN_SEL_OBJ(fm_cb->fm_amf_cb.amf_fd, amf_sel_obj);

			/* Release handle */
			ncshm_give_hdl(gl_fm_hdl);
		}
	}

	/* clear selection object set */
	m_NCS_SEL_OBJ_ZERO(&sel_obj_set);
	m_NCS_SEL_OBJ_SET(amf_sel_obj, &sel_obj_set);
	m_NCS_SEL_OBJ_SET(mbx_sel_obj, &sel_obj_set);
	highest_sel_obj = m_GET_HIGHER_SEL_OBJ(amf_sel_obj, mbx_sel_obj);

	/* Wait infinitely on  */
	while ((m_NCS_SEL_OBJ_SELECT(highest_sel_obj, &sel_obj_set, NULL, NULL, NULL) != -1)) {
		if (m_NCS_SEL_OBJ_ISSET(mbx_sel_obj, &sel_obj_set)) {
			/* Take handle */
			ncshm_take_hdl(NCS_SERVICE_ID_GFM, gl_fm_hdl);

			fm_mbx_evt = (FM_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(&fm_cb->mbx, msg);
			if (fm_mbx_evt) {
				fm_mbx_msg_handler(fm_cb, fm_mbx_evt);
			}
			/* Release handle */
			ncshm_give_hdl(gl_fm_hdl);
		}

		if (m_NCS_SEL_OBJ_ISSET(amf_sel_obj, &sel_obj_set)) {
			/* Take handle */
			ncshm_take_hdl(NCS_SERVICE_ID_GFM, gl_fm_hdl);

			/* Process the message received on amf selection object */
			fm_amf_process_msg(&fm_cb->fm_amf_cb);

			/* Release handle */
			ncshm_give_hdl(gl_fm_hdl);
		}

		m_NCS_SEL_OBJ_SET(amf_sel_obj, &sel_obj_set);
		m_NCS_SEL_OBJ_SET(mbx_sel_obj, &sel_obj_set);
		highest_sel_obj = m_GET_HIGHER_SEL_OBJ(amf_sel_obj, mbx_sel_obj);
	}

	fm_amf_close(&fm_cb->fm_amf_cb);
	fm_hpl_finalize();

 fm_hpl_lib_init_failed:

	fm_rda_finalize(fm_cb);

 fm_rda_init_failed:

	fm_mds_finalize(fm_cb);

 fm_mds_init_failed:

	m_NCS_IPC_DETACH(&fm_cb->mbx, NULL, NULL);

 fm_mbx_attach_failure:

	m_NCS_IPC_RELEASE(&fm_cb->mbx, NULL);

 fm_get_args_failed:

	ncshm_destroy_hdl(NCS_SERVICE_ID_GFM, gl_fm_hdl);
	gl_fm_hdl = 0;
	m_MMGR_FREE_FM_CB(fm_cb);

 fm_agents_startup_failed:

	fm_agents_shutdown();

	return 1;
}

/****************************************************************************
 * Name          : fm_agents_startup
 *
 * Description   : Starts NCS agents. 
 *
 * Arguments     :  None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_agents_startup(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* Start agents */
	rc = ncs_agents_startup();
	if (rc != NCSCC_RC_SUCCESS) {
		printf("ncs core agent startup failed\n ");
		return rc;
	}

	return rc;
}

/****************************************************************************
 * Name          : fm_agents_shutdown
 *
 * Description   :  Shutdown NCS agents
 *
 * Arguments     :  None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_agents_shutdown(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* Start agents */
	rc = ncs_agents_shutdown();
	if (rc != NCSCC_RC_SUCCESS) {
		printf("ncs core agent shutdown failed\n ");
		return rc;
	}

	return rc;
}

/****************************************************************************
 * Name          : fm_hpl_init 
 *
 * Description   :  Calls Hpl library for initialization.
 *
 * Arguments     :  None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_hpl_init(void)
{
	NCS_LIB_REQ_INFO req_info;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* Initialize with HPL. */
	/*memset(&req_info, '\0', sizeof(req_info));
	req_info.i_op = NCS_LIB_REQ_CREATE;
	rc = ncs_hpl_lib_req(&req_info);
	if (rc != NCSCC_RC_SUCCESS) {
		printf("hpl lib init failed\n ");
		return rc;
	}*/

	return rc;
}

/****************************************************************************
 * Name          : fm_hpl_finalize
 *
 * Description   :  Calls Hpl library for finalize
 *
 * Arguments     :  None
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_hpl_finalize(void)
{
	NCS_LIB_REQ_INFO req_info;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* Initialize with HPL. */
	/*memset(&req_info, '\0', sizeof(req_info));
	req_info.i_op = NCS_LIB_REQ_DESTROY;
	rc = ncs_hpl_lib_req(&req_info);*/

	return rc;
}

/****************************************************************************
 * Name          : fm_get_args
 *
 * Description   :  Parses configuration and store the values in Data str.
 *
 * Arguments     : Pointer to Control block 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_get_args(FM_CB *fm_cb)
{
	char *value;
	uns32 temp_slot_1 = 0, temp_slot_2 = 0;
	uns32 temp_sub_slot_1 = 0, temp_sub_slot_2 = 0;
	NCS_NODE_ID node_id;

	value = getenv("FM_CONTROLLER1_SLOT");
	if (value != NULL) {
		temp_slot_1 = atoi(value);
		if (!temp_slot_1) {
			syslog(LOG_ERR, "Invalid FM_CONTROLLER1_SLOT set in " PKGSYSCONFDIR "/fmd.conf");
			return NCSCC_RC_FAILURE;
		}
	}

	value = getenv("FM_CONTROLLER1_SUBSLOT");
	if (value != NULL) {
		temp_sub_slot_1 = atoi(value);
		if (!temp_sub_slot_1) {
			syslog(LOG_ERR, "Invalid FM_CONTROLLER1_SUBSLOT set in " PKGSYSCONFDIR "/fmd.conf");
			return NCSCC_RC_FAILURE;
		}
	}

	value = getenv("FM_CONTROLLER2_SLOT");
	if (value != NULL) {
		temp_slot_2 = atoi(value);
		if (!temp_slot_2) {
			syslog(LOG_ERR, "Invalid FM_CONTROLLER2_SLOT set in " PKGSYSCONFDIR "/fmd.conf");
			return NCSCC_RC_FAILURE;
		}
	}

	value = getenv("FM_CONTROLLER2_SUBSLOT");
	if (value != NULL) {
		temp_sub_slot_2 = atoi(value);
		if (!temp_sub_slot_2) {
			syslog(LOG_ERR, "Invalid FM_CONTROLLER2_SUBSLOT set in " PKGSYSCONFDIR "/fmd.conf");
			return NCSCC_RC_FAILURE;
		}
	}

	/* Update fm_cb configuration fields */
	node_id = m_NCS_GET_NODE_ID;
	m_NCS_GET_PHYINFO_FROM_NODE_ID(node_id, &fm_cb->shelf, &fm_cb->slot, &fm_cb->sub_slot);

	fm_cb->peer_shelf = fm_cb->shelf;
	fm_cb->peer_slot = (fm_cb->slot == temp_slot_1) ? temp_slot_2 : temp_slot_1;
	fm_cb->peer_sub_slot = (fm_cb->sub_slot == temp_sub_slot_1) ? temp_sub_slot_2 : temp_sub_slot_1;
	fm_cb->active_promote_tmr_val = 500;
	fm_cb->reset_retry_tmr_val = 300;

	/* Set timer variables */
	fm_cb->promote_active_tmr.type = FM_TMR_PROMOTE_ACTIVE;
	fm_cb->reset_retry_tmr.type = FM_TMR_RESET_RETRY;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : fm_mbx_msg_handler
 *
 * Description   : Processes Mail box messages between FM. 
 *
 * Arguments     :  Pointer to Control block and Mail box event 
 *
 * Return Values : None.
 * 
 * Notes         : None. 
 *****************************************************************************/
static void fm_mbx_msg_handler(FM_CB *fm_cb, FM_EVT *fm_mbx_evt)
{
	uns8 entity_path[128] = { 0 };
	uns32 status;

	switch (fm_mbx_evt->evt_code) {
	case FM_EVT_HB_LOSS:
		syslog(LOG_INFO,
		       "Role: %s, FM_EVT_HB_LOSS: for slot_id: %d, subslot_id: %d",
		       role_string[fm_cb->role], fm_mbx_evt->slot, fm_mbx_evt->sub_slot);
		if (fm_cb->role == PCS_RDA_STANDBY) {
			if ((fm_mbx_evt->slot == fm_cb->peer_slot) && (fm_mbx_evt->sub_slot == fm_cb->peer_sub_slot)) {
				/* Start Promote active timer */
				fm_tmr_start(&fm_cb->promote_active_tmr, fm_cb->active_promote_tmr_val);
				syslog(LOG_INFO, "Promote active timer started\n");
			}
		} else if (fm_cb->role == PCS_RDA_ACTIVE) {
			/* Shelf id of Peer and Host are same */
			fm_conv_shelf_slot_to_entity_path(entity_path, fm_cb->peer_shelf,
							  fm_mbx_evt->slot, fm_mbx_evt->sub_slot);
			if (fm_cb->is_platform == TRUE)
				syslog(LOG_INFO, "FIXME: Removed HISv integration\n");
				/*hpl_resource_reset(fm_cb->peer_shelf, entity_path, HISV_RES_GRACEFUL_REBOOT);*/
			else {
				if (fm_mbx_evt->slot == fm_cb->peer_slot &&
				    fm_mbx_evt->sub_slot == fm_cb->peer_sub_slot) {
					/* I got hb loss for standby controller. Try to reboot it */
					fms_reset_peer(fm_cb);
				} else {
					/* We got hb loss for payload. 
					   Payload reset might be supported in future */
				}
			}
			fms_fma_node_reset_intimate(fm_cb, fm_mbx_evt->slot, fm_mbx_evt->sub_slot);
		}
		break;

	case FM_EVT_HB_RESTORE:
		syslog(LOG_INFO,
		       "Role: %s, FM_EVT_HB_RESTORE: for slot_id: %d, subslot_id: %d",
		       role_string[fm_cb->role], fm_mbx_evt->slot, fm_mbx_evt->sub_slot);
		break;

	case FM_EVT_CAN_SMH_SW:
		/* Send reply to FMA if current card role is active */
		fm_can_smh_sw_process(fm_cb, fm_mbx_evt);
		break;

	case FM_EVT_AVM_NODE_RESET_IND:
		syslog(LOG_INFO,
		       "Role: %s, NODE_RESET_IND: for slot_id: %d, subslot_id: %d",
		       role_string[fm_cb->role], fm_mbx_evt->slot, fm_mbx_evt->sub_slot);

		if (fm_cb->role == PCS_RDA_STANDBY) {
			if (fm_cb->peer_slot == fm_mbx_evt->slot && fm_cb->peer_sub_slot == fm_mbx_evt->sub_slot) {
				/* I got node reset indication for peer(active) */
				/* Set RDA role to active */
				fm_rda_set_role(fm_cb, PCS_RDA_ACTIVE);
			}
		}
		/* I don'thave to do anything if I'm active */
		break;

	case FM_EVT_TMR_EXP:
		/* Timer Expiry event posted */
		if (fm_mbx_evt->info.fm_tmr->type == FM_TMR_PROMOTE_ACTIVE) {
			/* Now. Try resetting other blade */
			fm_cb->role = PCS_RDA_ACTIVE;

			/* Timer started only in case of reset of peer FMS, so getting slot and shelf id from cb */
			fm_conv_shelf_slot_to_entity_path(entity_path, fm_cb->peer_shelf, fm_cb->peer_slot,
							  fm_cb->peer_sub_slot);
			syslog(LOG_INFO, "Promote active timer expired." " Reseting peer controller slot_id=%u\n",
			       fm_cb->peer_slot);

			if (fm_cb->is_platform == TRUE)
				syslog(LOG_INFO, "FIXME: Removed HISv integration\n");
				/*status = hpl_resource_reset(fm_cb->peer_shelf, entity_path, HISV_RES_GRACEFUL_REBOOT);*/
			else
				status = fms_reset_peer(fm_cb);

			if (status != NCSCC_RC_SUCCESS) {
				syslog(LOG_INFO,
				       "Reseting peer controller slot_id=%d failed. Starting reset retry timer\n",
				       fm_cb->peer_slot);
				fm_tmr_start(&fm_cb->reset_retry_tmr, fm_cb->reset_retry_tmr_val);
			}

			if (( fm_cb->is_platform != TRUE ) && ( status == NCSCC_RC_SUCCESS )) {
				/* Wait for the other node  to go down, case of SCAP kill on active controller */
				sleep(5);
			}

			/* Set RDA role to active */
			fm_rda_set_role(fm_cb, PCS_RDA_ACTIVE);
		} else if (fm_mbx_evt->info.fm_tmr->type == FM_TMR_RESET_RETRY) {
			fm_conv_shelf_slot_to_entity_path(entity_path, fm_cb->peer_shelf, fm_cb->peer_slot,
							  fm_cb->peer_sub_slot);
			syslog(LOG_INFO, "Reset retry timer expired." " Reseting peer controller slot_id=%u failed.\n",
			       fm_cb->peer_slot);

			if (fm_cb->is_platform == TRUE)
				syslog(LOG_INFO, "FIXME: Removed HISv integration\n");
				/* Don't care what is return status */
				/*hpl_resource_reset(fm_cb->peer_shelf, entity_path, HISV_RES_GRACEFUL_REBOOT);*/
			else
				fms_reset_peer(fm_cb);

			/* Intimate FMA about reset */
			fms_fma_node_reset_intimate(fm_cb, fm_cb->peer_slot, fm_cb->peer_sub_slot);
		}
		break;

	default:
		break;
	}

	/* Free the event. */
	if (fm_mbx_evt != NULL) {
		m_MMGR_FREE_FM_EVT(fm_mbx_evt);
		fm_mbx_evt = NULL;
	}

	return;
}

/****************************************************************************
 * Name          : fm_tmr_start
 *
 * Description   : Starts timer with the given period 
 *
 * Arguments     :  Pointer to TImer Data Str
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
uns32 fm_tmr_start(FM_TMR *tmr, SaTimeT period)
{
	uns32 tmr_period;

	tmr_period = (uns32)period;

	if (tmr->tmr_id == NULL) {
		m_NCS_TMR_CREATE(tmr->tmr_id, tmr_period, fm_tmr_exp, (void *)tmr);
	}

	if (tmr->status == FM_TMR_RUNNING) {
		return NCSCC_RC_FAILURE;
	}

	m_NCS_TMR_START(tmr->tmr_id, tmr_period, fm_tmr_exp, (void *)tmr);
	tmr->status = FM_TMR_RUNNING;

	if (TMR_T_NULL == tmr->tmr_id) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : fm_tmr_exp
 *
 * Description   : Timer Expiry function 
 *
 * Arguments     : Pointer to timer data structure 
 *
 * Return Values : None.
 * 
 * Notes         : None. 
 *****************************************************************************/
void fm_tmr_exp(void *fm_tmr)
{
	FM_CB *fm_cb = NULL;
	FM_TMR *tmr = (FM_TMR *)fm_tmr;
	FM_EVT *evt = NULL;

	if (tmr == NULL) {
		return;
	}

	/* Take handle */
	fm_cb = ncshm_take_hdl(NCS_SERVICE_ID_GFM, gl_fm_hdl);

	if (fm_cb == NULL) {
		syslog(LOG_ERR, "Taking handle failed in timer expiry \n");
		return;
	}

	if (FM_TMR_STOPPED == tmr->status) {
		return;
	}
	tmr->status = FM_TMR_STOPPED;

	/* Create & send the timer event to the FM MBX. */
	evt = m_MMGR_ALLOC_FM_EVT;

	if (evt != NULL) {
		memset(evt, '\0', sizeof(FM_EVT));
		evt->evt_code = FM_EVT_TMR_EXP;
		evt->info.fm_tmr = tmr;

		if (m_NCS_IPC_SEND(&fm_cb->mbx, evt, NCS_IPC_PRIORITY_HIGH) != NCSCC_RC_SUCCESS) {
			syslog(LOG_ERR, "IPC send failed in timer expiry \n");
			m_MMGR_FREE_FM_EVT(evt);
		}
	}

	/* Give handle */
	ncshm_give_hdl(gl_fm_hdl);
	fm_cb = NULL;

	return;
}

/****************************************************************************
 * Name          : fm_tmr_stop
 *
 * Description   :  Stops the timer
 *
 * Arguments     :  Pointer to Timer Data Str
 *
 * Return Values : None.
 * 
 * Notes         : None. 
 *****************************************************************************/
void fm_tmr_stop(FM_TMR *tmr)
{
	if (FM_TMR_TYPE_MAX <= tmr->type) {
		return;
	}

	if (FM_TMR_RUNNING == tmr->status) {
		tmr->status = FM_TMR_STOPPED;
		m_NCS_TMR_STOP(tmr->tmr_id);
	}

	if (TMR_T_NULL != tmr->tmr_id) {
		m_NCS_TMR_DESTROY(tmr->tmr_id);
		tmr->tmr_id = TMR_T_NULL;
	}

	return;
}

/****************************************************************************
 * Name          : fm_can_smh_sw_process 
 *
 * Description   : Sends a message to Svc in Sync 
 *
 * Arguments     : Pointer to Control block and Mail box Event. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_can_smh_sw_process(FM_CB *fm_cb, FM_EVT *fm_mbx_evt)
{
	FMA_FM_MSG msg;
	uns32 return_val;

	memset(&msg, 0, sizeof(FMA_FM_MSG));

	msg.msg_type = FMA_FM_EVT_SMH_SW_RESP;

	if (fm_cb->role == PCS_RDA_ACTIVE) {
		msg.info.response = FMA_FM_SMH_CAN_SW;
	} else {
		msg.info.response = FMA_FM_SMH_CANNOT_SW;
	}

	return_val = fm_mds_sync_send(fm_cb, (NCSCONTEXT)&msg, NCSMDS_SVC_ID_FMA,
				      MDS_SEND_PRIORITY_MEDIUM,
				      MDS_SENDTYPE_RSP, &fm_mbx_evt->fr_dest, &fm_mbx_evt->mds_ctxt);

	return return_val;
}

/****************************************************************************
 * Name          : fm_conv_shelf_slot_to_entity_path  
 *
 * Description   :  Forms Entity path
 *
 * Arguments     :  Pointer to Entity path, shelf, slot, sub slot.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_conv_shelf_slot_to_entity_path(uns8 *o_ent_path, uns8 shelf, uns8 slot, uns8 sub_slot)
{
	char *arch_type = NULL;
	arch_type = getenv("FM_TARGET_SYSTEM_ARCH");

	if (NULL == arch_type)
		return NCSCC_RC_FAILURE;

	if ((sub_slot == 0) || (sub_slot == 15)) {
#ifdef HAVE_HPI_A01
		sprintf((char *)o_ent_path, "{{SYSTEM_BOARD,%d},{SYSTEM_CHASSIS,%d}}", slot, shelf);
#else
		if (strcmp(arch_type, "ATCA") == 0)
			sprintf((char *)o_ent_path, "{{PHYSICAL_SLOT,%d},{ADVANCEDTCA_CHASSIS,%d}}", slot, shelf);
		else
			sprintf((char *)o_ent_path, "{{SYSTEM_BLADE,%d},{SYSTEM_CHASSIS,%d}}", slot, shelf);
#endif
	} else {
		/* Embedding subslot into the entity path in string format */
		if (strcmp(arch_type, "ATCA") == 0)
			sprintf((char *)o_ent_path,
				"{{AMC_SUB_SLOT, %d},{PHYSICAL_SLOT,%d},{ADVANCEDTCA_CHASSIS,%d}}",
				sub_slot, slot, shelf);
		else
			sprintf((char *)o_ent_path,
				"{{AMC_SUB_SLOT, %d},{SYSTEM_BLADE,%d},{SYSTEM_CHASSIS,%d}}", sub_slot, slot, shelf);
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : fms_fma_node_reset_intimate 
 *
 * Description   : Sends node reset indication.
 *
 * Arguments     :  Pointer to Control Block, Slot Id, Subslot Id
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fms_fma_node_reset_intimate(FM_CB *fm_cb, uns32 slot_id, uns32 sub_slot_id)
{
	FMA_FM_MSG fma_msg;

	memset(&fma_msg, 0, sizeof(FMA_FM_MSG));

	fma_msg.msg_type = FMA_FM_EVT_NODE_RESET_IND;
	fma_msg.info.phy_addr.slot = slot_id;
	fma_msg.info.phy_addr.site = sub_slot_id;

	if (NCSCC_RC_SUCCESS != fm_mds_async_send(fm_cb, (NCSCONTEXT)&fma_msg,
						  NCSMDS_SVC_ID_FMA, MDS_SEND_PRIORITY_MEDIUM,
						  MDS_SENDTYPE_BCAST, 0, NCSMDS_SCOPE_INTRANODE)) {
		syslog(LOG_ERR, "Intimation node reset to FMA failed\n");
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : fms_reset_peer 
 *
 * Description   : Sends a message to reset its peer. 
 *
 * Arguments     : Pointer to Control Block. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fms_reset_peer(FM_CB *fm_cb)
{
	GFM_GFM_MSG gfm_msg;

	if (fm_cb->peer_adest != 0) {
		/* peer fms present */
		memset(&gfm_msg, 0, sizeof(GFM_GFM_MSG));
		gfm_msg.msg_type = GFM_GFM_EVT_NODE_RESET_IND;
		gfm_msg.info.node_reset_ind_info.shelf = fm_cb->peer_shelf;
		gfm_msg.info.node_reset_ind_info.slot = fm_cb->peer_slot;
		gfm_msg.info.node_reset_ind_info.sub_slot = fm_cb->peer_sub_slot;

		if (NCSCC_RC_SUCCESS != fm_mds_async_send(fm_cb, (NCSCONTEXT)&gfm_msg,
							  NCSMDS_SVC_ID_GFM, MDS_SEND_PRIORITY_MEDIUM,
							  0, fm_cb->peer_adest, 0)) {
			syslog(LOG_ERR, "Sending node-reset message to peer fms failed\n");
			return NCSCC_RC_FAILURE;
		}

		return NCSCC_RC_SUCCESS;
	}

	return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : fm_nid_notify
 *
 * Description   : Sends notification to NID
 *
 * Arguments     :  Error Type
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 * 
 * Notes         : None. 
 *****************************************************************************/
static uns32 fm_nid_notify(uns32 nid_err)
{
	uns32 error;
	uns32 nid_stat_code;

	if (nid_err > NCSCC_RC_SUCCESS)
		nid_err = NCSCC_RC_FAILURE;

	nid_stat_code = nid_err;
	return nid_notify("HLFM", nid_stat_code, &error);
}
