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

  This module is the main module for Availability Director. This module
  deals with the initialisation of AvD.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_init_proc - entry function to AvD thread or task.
  avd_initialize - creation and starting of AvD task/thread.
  avd_tmr_cl_init_func - the event handler for AMF cluster timer expiry.
  avd_destroy - the destruction of the AVD module.
  avd_lib_req - DL SE API for init and destroy of the AVD module

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <poll.h>
#include <saImmOm.h>
#include <immutil.h>
#include <logtrace.h>
#include <avd.h>
#include <avd_hlt.h>
#include <avd_imm.h>
#include <avd_cluster.h>

/* 
** The singleton AVD Cluster Control Block. Statically allocated which is
** good for debugging core dumps
*/
static AVD_CL_CB _avd_cb;

/* A handy global reference to the control block */
AVD_CL_CB *avd_cb = &_avd_cb;

/*****************************************************************************
 * Function: avd_hb_proc 
 *
 * Purpose: This is the infinite loop (for hb_thread) in which both the active
 * and standby AvDs execute waiting for heartbeat events to happen. When woken
 * up due to an event, based on the HA state it moves to either the active
 * or standby processing modules. Even in Init state the same arrays are used.
 *
 * Input: -
 *
 * Returns: NONE.
 *
 * NOTES: This function will never return execept in case of init errors.
 *
 * 
 **************************************************************************/

static void avd_hb_proc(uns32 *null)
{
	struct pollfd fds[1];
	AVD_CL_CB *cb = avd_cb;
	AVD_EVT *evt;
	NCS_SEL_OBJ mbx_fd;

	TRACE_ENTER();

	mbx_fd = ncs_ipc_get_sel_obj(&cb->avd_hb_mbx);

	fds[0].fd = mbx_fd.rmv_obj;
	fds[0].events = POLLIN;

	while (1) {
		int ret = poll(fds, 1, -1);

		if (ret == -1) {
			if (errno == EINTR)
				continue;

			avd_log(NCSFL_SEV_ERROR, "AVD-HB poll failed - %s", strerror(errno));
			break;
		}

		if (fds[0].revents & POLLIN) {
			evt = (AVD_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(&cb->avd_hb_mbx, msg);

			if ((evt != AVD_EVT_NULL) && (evt->rcv_evt > AVD_EVT_INVALID) && (evt->rcv_evt < AVD_EVT_MAX)) {
				/* We will get only timer expiry and MDS msg  
				 */
				if ((evt->rcv_evt == AVD_EVT_TMR_SND_HB) ||
				    (evt->rcv_evt == AVD_EVT_HEARTBEAT_MSG) || (evt->rcv_evt == AVD_EVT_D_HB)) {
					/* Process the event */
					avd_process_hb_event(cb, evt);
				} else
					avd_log(NCSFL_SEV_EMERGENCY, "Invalid event: %u", evt->rcv_evt);
			}
		}
	}

	syslog(LOG_CRIT, "Avd-HB Thread Failed");
	sleep(3);		/* Allow logs to be printed */
	exit(EXIT_FAILURE);
}

/*****************************************************************************
 * Function: avd_hb_task_create 
 *
 * Purpose: This routine will create another thread which has another mailbox
 *          and will process only AVD-AVD & AVD-AVND heart beat msg.
 *
 * Returns: None. 
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static int avd_hb_task_create()
{
	void *avd_hb_task_hdl;

	/* create and start the AvD HB thread */
	if (m_NCS_TASK_CREATE((NCS_OS_CB) avd_hb_proc,
			      NULL,
			      NCS_AVD_HB_NAME_STR,
			      NCS_AVD_HB_PRIORITY, NCS_AVD_HB_STCK_SIZE, &avd_hb_task_hdl) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "task create FAILED");
		return -1;
	}

	if (m_NCS_TASK_START(avd_hb_task_hdl) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "task start FAILED");
		return -1;
	}

	return 0;
}

/*****************************************************************************
 * Function: avd_init_proc
 *
 * Purpose: This is AvD thread initializtion.
 *
 * Input: -
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_init_proc(void)
{
	AVD_CL_CB *cb = avd_cb;
	NCS_PATRICIA_PARAMS patricia_params;
	int rc = NCSCC_RC_FAILURE;
        SaVersionT ntfVersion = { 'A', 0x01, 0x01 };

	TRACE_ENTER();

	memset(&patricia_params, 0, sizeof(NCS_PATRICIA_PARAMS));

	/* Initialize all the locks and trees in the CB */

	if (NCSCC_RC_FAILURE == m_AVD_CB_LOCK_INIT(cb)) {
		avd_log(NCSFL_SEV_ERROR, "cb lock init FAILED");
		goto done;
	}

	if (NCSCC_RC_FAILURE == m_AVD_CB_AVND_TBL_LOCK_INIT(cb)) {
		avd_log(NCSFL_SEV_ERROR, "lock init FAILED");
		goto done;
	}

	cb->init_state = AVD_INIT_BGN;
	cb->rcv_hb_intvl = AVSV_RCV_HB_INTVL;
	cb->snd_hb_intvl = AVSV_SND_HB_INTVL;
	cb->role_switch = SA_FALSE;
	cb->stby_sync_state = AVD_STBY_IN_SYNC;
	cb->sync_required = TRUE;
	cb->avd_hrt_beat_rcvd = FALSE;

	patricia_params.key_size = sizeof(SaClmNodeIdT);
	if (ncs_patricia_tree_init(&cb->node_list, &patricia_params) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_patricia_tree_init FAILED");
		goto done;
	}

	patricia_params.key_size = sizeof(AVD_SUS_PER_SI_RANK_INDX);
	if (ncs_patricia_tree_init(&cb->su_per_si_rank_anchor, &patricia_params) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_patricia_tree_init FAILED");
		goto done;
	}

	patricia_params.key_size = sizeof(AVD_SI_SI_DEP_INDX);
	if (ncs_patricia_tree_init(&cb->si_dep.spons_anchor, &patricia_params) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_patricia_tree_init FAILED");
		goto done;
	}

	patricia_params.key_size = sizeof(AVD_SI_SI_DEP_INDX);
	if (ncs_patricia_tree_init(&cb->si_dep.dep_anchor, &patricia_params) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_patricia_tree_init FAILED");
		goto done;
	}

	/* Initialise MDS */
	if (avd_mds_reg_def(cb) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "avd_mds_reg_def FAILED");
		goto done;
	}

	/* get the node id of the node on which the AVD is running. */
	cb->node_id_avd = m_NCS_GET_NODE_ID;

	/* Initialise MDS */
	if (avd_mds_reg(cb) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "avd_mds_reg FAILED");
		goto done;
	}

	if (NCSCC_RC_FAILURE == avsv_mbcsv_register(cb)) {
		avd_log(NCSFL_SEV_EMERGENCY, "avsv_mbcsv_register FAILED");
		goto done;
	}

	if (avd_imm_init(cb) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_EMERGENCY, "avd_imm_init FAILED");
		goto done;
	}

	if (avd_hb_task_create() != 0)
		goto done;

        rc = saNtfInitialize(&cb->ntfHandle, NULL, &ntfVersion);
        if (rc != SA_AIS_OK) {
                avd_log(NCSFL_SEV_ERROR,"saNtfInitialize Failed (%u)", rc);
                goto done;
        }

	rc = NCSCC_RC_SUCCESS;

 done:
	TRACE_LEAVE2("%u", rc);
	return rc;
}

/*****************************************************************************
 * Function: avd_initialize
 *
 * Purpose: This is the routine that does the creation and starting of
 * AvD task/thread.
 * 
 *
 * Input: -
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

static uns32 avd_initialize(void)
{
	AVD_CL_CB *cb = avd_cb;
	void *avd_task_hdl;

	TRACE_ENTER();

	/* run the class constructors */
	avd_app_constructor();
	avd_comp_constructor();
	avd_ctcstype_constructor();
	avd_comptype_constructor();
	avd_cluster_constructor();
	avd_csi_constructor();
	avd_hc_constructor();
	avd_node_constructor();
	avd_sg_constructor();
	avd_si_constructor();
	avd_sidep_constructor();
	avd_su_constructor();

	/* Register with Logging subsystem. This is an agent call and
	 ** could succeed even if the DTS server is not available 
	 */
	avd_flx_log_reg();

	if ((cb->cb_handle = ncshm_create_hdl(NCS_HM_POOL_ID_COMMON, NCS_SERVICE_ID_AVD, cb)) == 0) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncshm_create_hdl FAILED");
		return NCSCC_RC_FAILURE;
	}

	if (ncs_ipc_create(&cb->avd_mbx) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_ipc_create FAILED");
		return NCSCC_RC_FAILURE;
	}

	if (ncs_ipc_attach(&cb->avd_mbx) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_ipc_attach FAILED");
		return NCSCC_RC_FAILURE;
	}

	/* create a mailbox for heart beat thread. */
	if (ncs_ipc_create(&cb->avd_hb_mbx) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_ipc_create FAILED");
		return NCSCC_RC_FAILURE;
	}

	if (ncs_ipc_attach(&cb->avd_hb_mbx) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_ipc_attach FAILED");
		return NCSCC_RC_FAILURE;
	}

	/* create and start the AvD thread */
	if (ncs_task_create((NCS_OS_CB) avd_main_proc, NULL, NCS_AVD_NAME_STR,
		NCS_AVD_PRIORITY, NCS_AVD_STCK_SIZE, &avd_task_hdl) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_task_create FAILED");
		return NCSCC_RC_FAILURE;
	}

	if (ncs_task_start(avd_task_hdl) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_EMERGENCY, "ncs_task_start FAILED");
		return NCSCC_RC_FAILURE;
	}

	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_lib_req
 *
 * Purpose: This is the routine that is exposed to the outside world
 * for both initialization and destruction of the AVD module. This
 * will inturn call the initialisation and destroy routines of AVD.
 * 
 *
 * Input: req_info - pointer to the DL SE API request structure.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	return avd_initialize();
}
