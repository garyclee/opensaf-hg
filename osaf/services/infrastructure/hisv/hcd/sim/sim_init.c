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
*                                                                            *
*  MODULE NAME:  sim_init.c                                                  *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality for initializing HPI Shelf Init    *
*  Manager. It initializes the SIM control block and messages queue on which *
*  it receive firware progress events. Also contains the 'finalizing'        *
*  routines used to finalize SIM, destroy control block and delete queue.    *
*                                                                            *
*****************************************************************************/

#include "hcd.h"

/* global cb handle */
uns32 gl_sim_hdl = 0;

/****************************************************************************
 * Name          : sim_initialize
 *
 * Description   : This function initializes the SIM control block and creates
 *                 SIM control block handle. Also initializes the IPC message
 *                 queue on which it receives sensor firmware progress events.
 *
 * Arguments     : pointer HPI_SESSION_ARGS which holds session-id, domain-id
 *                 of the HPI session.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 sim_initialize()
{
	SIM_CB *sim_cb = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	m_LOG_HISV_DTS_CONS("sim_initialize: Initializing ShIM...\n");
	/* allocate SIM cb */
	if (NULL == (sim_cb = m_MMGR_ALLOC_SIM_CB)) {
		/* reset the global cb handle */
		gl_sim_hdl = 0;
		m_LOG_HISV_DTS_CONS("sim_initialize: error allocating sim_cb\n");
		return NCSCC_RC_FAILURE;
	}
	memset(sim_cb, 0, sizeof(SIM_CB));

	/* assign the SIM pool-id (used by hdl-mngr) */
	sim_cb->pool_id = NCS_HM_POOL_ID_COMMON;

	/* create the association with hdl-mngr */
	if (0 == (sim_cb->cb_hdl = ncshm_create_hdl(sim_cb->pool_id, NCS_SERVICE_ID_HCD, (NCSCONTEXT)sim_cb))) {
		/* log */
		m_LOG_HISV_DTS_CONS("sim_initialize: error taking sim_cb handle\n");
		rc = NCSCC_RC_FAILURE;
		sim_cb->cb_hdl = 0;
		goto error;
	}
	/* store the cb hdl in the global variable */
	gl_sim_hdl = sim_cb->cb_hdl;

	/* get the process id */
	sim_cb->prc_id = getpid();

	m_LOG_HISV_DTS_CONS("sim_initialize: creating ShIM mbx\n");

	memset(sim_cb->fwprog_done, 0, MAX_NUM_SLOTS);
	/* create the mbx to communicate with the SIM thread */
	if (NCSCC_RC_SUCCESS != (rc = m_NCS_IPC_CREATE(&sim_cb->mbx))) {
		/* Destroy the hdl for this CB */
		ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
		gl_sim_hdl = 0;
		m_LOG_HISV_DTS_CONS("sim_initialize: error creating ShIM mbx\n");
		/* Free the control block */
		m_MMGR_FREE_SIM_CB(sim_cb);
		return NCSCC_RC_FAILURE;
	}
	/* Attach the IPC to the created thread */
	m_NCS_IPC_ATTACH(&sim_cb->mbx);

	/* initialize the SIM cb lock */
	m_NCS_LOCK_INIT(&sim_cb->cb_lock);
	m_LOG_HISV_DTS_CONS("sim_initialize: Done with Initializing ShIM\n");
	return NCSCC_RC_SUCCESS;

 error:
	if (sim_cb != NULL) {
		/* release the IPC */
		m_NCS_IPC_RELEASE(&sim_cb->mbx, NULL);

		/* destroy the lock */
		m_NCS_LOCK_DESTROY(&sim_cb->cb_lock);

		/* remove the association with hdl-mngr */
		if (sim_cb->cb_hdl != 0)
			ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, sim_cb->cb_hdl);

		/* free the control block */
		m_MMGR_FREE_SIM_CB(sim_cb);
		m_LOG_HISV_DTS_CONS("sim_initialize: Error with Initializing ShIM\n");

		/* reset the global cb handle */
		gl_sim_hdl = 0;
	}
	return rc;
}

/****************************************************************************
 * Name          : sim_clear_mbx
 *
 * Description   : This is the function which deletes all the messages from
 *                 the SIM mail-box. This is called as a callback from IPC
 *                 detach function
 *
 * Arguments     : arg     - argument to be passed.
 *                 msg     - Event start pointer.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/

static NCS_BOOL sim_clear_mbx(NCSCONTEXT arg, NCSCONTEXT msg)
{
	SIM_EVT *pEvt = (SIM_EVT *)msg;
	SIM_EVT *pnext;
	pnext = pEvt;
	m_LOG_HISV_DTS_CONS("sim_clear_mbx: Clearing ShIM mbx\n");

	while (pnext) {
		pnext = ( /* (SIM_EVT *)& */ (pEvt->next));
		m_MMGR_FREE_SIM_EVT(pEvt);
		pEvt = pnext;
	}
	return TRUE;
}

/****************************************************************************
 * Name          : sim_finalize
 *
 * Description   : This function deletes the resources used by ShIM,
 *                 frees the allocated memory and destroys the ShIM control
 *                 block and associated handle.
 *
 * Arguments     : destroy_info - ptr to the NCS destroy information
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None
 *****************************************************************************/

uns32 sim_finalize()
{
	SIM_CB *sim_cb = 0;

	/* retrieve SIM CB */
	sim_cb = (SIM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, gl_sim_hdl);
	m_LOG_HISV_DTS_CONS("sim_finalize: Finalizing ShIM\n");

	if (!sim_cb) {
		m_LOG_HISV_DTS_CONS("sim_finalize: error taking sim_cb handle in finalizing ShIM\n");
		return NCSCC_RC_FAILURE;
	}
	/* return SIM CB */
	ncshm_give_hdl(gl_sim_hdl);

	/* remove the association with hdl-mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_HCD, sim_cb->cb_hdl);

	/* stop & kill the created task */
	m_NCS_TASK_STOP(sim_cb->task_hdl);
	m_NCS_TASK_RELEASE(sim_cb->task_hdl);

	/* Detach from IPC */
	m_NCS_IPC_DETACH(&sim_cb->mbx, sim_clear_mbx, sim_cb);

	/* release the IPC */
	m_NCS_IPC_RELEASE(&sim_cb->mbx, NULL);

	/* destroy the lock */
	m_NCS_LOCK_DESTROY(&sim_cb->cb_lock);

	/* free the control block */
	m_MMGR_FREE_SIM_CB(sim_cb);

	/* reset the global cb handle */
	gl_sim_hdl = 0;
	m_LOG_HISV_DTS_CONS("sim_finalize: Done with Finalizing ShIM\n");

	return NCSCC_RC_SUCCESS;
}
