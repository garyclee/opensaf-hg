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

  This file contains AvND initialization & destroy routines.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avnd.h"
#include "ncs_main_pvt.h"
#include "avsv_d2nedu.h"
#include "avsv_n2avaedu.h"
#include "avsv_n2claedu.h"
#include "avsv_nd2ndmsg.h"

/* global cb handle */
uns32 gl_avnd_hdl = 0;

/* global task handle */
NCSCONTEXT gl_avnd_task_hdl = 0;

/* static function declarations */

static uns32 avnd_create(NCS_LIB_CREATE *create_info);

static AVND_CB *avnd_cb_create(void);

static uns32 avnd_mbx_create(AVND_CB *);

static uns32 avnd_ext_intf_create(AVND_CB *);

static uns32 avnd_task_create(void);

static uns32 avnd_mbx_destroy(AVND_CB *);

static uns32 avnd_ext_intf_destroy(AVND_CB *);

static uns32 avnd_task_destroy(void);

static NCS_BOOL avnd_mbx_clean(NCSCONTEXT, NCSCONTEXT);

static void avnd_sigusr1_handler(void);

/****************************************************************************
  Name          : avnd_lib_req
 
  Description   : This routine is exported to the external entities & is used
                  to create & destroy AvND.
 
  Arguments     : req_info - ptr to the request info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		rc = avnd_create(&req_info->info.create);
		if (NCSCC_RC_SUCCESS == rc) {
			m_AVND_LOG_SEAPI(AVSV_LOG_SEAPI_CREATE, AVSV_LOG_SEAPI_SUCCESS, NCSFL_SEV_INFO);
		} else {
			m_AVND_LOG_SEAPI(AVSV_LOG_SEAPI_CREATE, AVSV_LOG_SEAPI_FAILURE, NCSFL_SEV_CRITICAL);
		}
		break;

	case NCS_LIB_REQ_DESTROY:
		avnd_sigusr1_handler();
		break;

	default:
		break;
	}

	return rc;
}

/****************************************************************************
  Name          : avnd_create
 
  Description   : This routine creates & initializes the PWE of AvND. It does
                  the following:
                  a) create & initialize AvND control block.
                  b) create & attach AvND mailbox.
                  c) initialize external interfaces (logging service being the
                     exception).
                  d) create & start AvND task.
 
  Arguments     : create_info - ptr to the create info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_create(NCS_LIB_CREATE *create_info)
{
	AVND_CB *cb = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* register with dtsv */
#if (NCS_AVND_LOG == 1)
	rc = avnd_log_reg();
#endif

	/* create & initialize AvND cb */
	cb = avnd_cb_create();
	if (!cb) {
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* create & attach AvND mailbox */
	rc = avnd_mbx_create(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* initialize external interfaces */
	rc = avnd_ext_intf_create(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* create & start AvND task */
	rc = avnd_task_create();
	if (NCSCC_RC_SUCCESS != rc) {
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

 done:
	/* if failed, perform the cleanup */
	if (NCSCC_RC_SUCCESS != rc)
		avnd_destroy();

	return rc;
}

/****************************************************************************
  Name          : avnd_destroy
 
  Description   : This routine destroys the PWE of AvND. It does the following:
                  a) destroy external interfaces (logging service being the
                     exception).
                  b) detach & destroy AvND mailbox.
                  c) destroy AvND control block.
                  d) destroy AvND task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_destroy()
{
	AVND_CB *cb = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* retrieve avnd cb */
	if (0 == (cb = (AVND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, (uns32)gl_avnd_hdl))) {
		m_AVND_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* destroy external interfaces */
	rc = avnd_ext_intf_destroy(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

	/* destroy AvND control block */
	rc = avnd_cb_destroy(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

	/* destroy AvND task */
	/*rc = avnd_task_destroy();
	 *if ( NCSCC_RC_SUCCESS != rc ) goto done;
	 */

 done:
	/* return avnd cb */
	if (cb)
		ncshm_give_hdl((uns32)gl_avnd_hdl);

	/* unregister with DTSv */
#if (NCS_AVND_LOG == 1)
	rc = avnd_log_unreg();
#endif

	return rc;
}

/****************************************************************************
  Name          : avnd_cb_create
 
  Description   : This routine creates & initializes AvND control block.
 
  Arguments     : None.
 
  Return Values : if successfull, ptr to AvND control block
                  else, 0
 
  Notes         : None
******************************************************************************/
AVND_CB *avnd_cb_create()
{
	AVND_CB *cb = 0;
	uns32 rc = NCSCC_RC_SUCCESS;
	SaVersionT ntfVersion = { 'A', 0x01, 0x01 };
	SaNtfCallbacksT ntfCallbacks = { NULL, NULL };

	/* allocate AvND cb */
	if ((0 == (cb = m_MMGR_ALLOC_AVND_CB))) {
		m_AVND_LOG_CB(AVSV_LOG_CB_CREATE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
	m_AVND_LOG_CB(AVSV_LOG_CB_CREATE, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	memset(cb, 0, sizeof(AVND_CB));

	/* assign the AvND pool-id (used by hdl-mngr) */
	cb->pool_id = NCS_HM_POOL_ID_COMMON;

	/* assign the default states */
	cb->admin_state = NCS_ADMIN_STATE_UNLOCK;
	cb->oper_state = NCS_OPER_STATE_ENABLE;
	cb->term_state = AVND_TERM_STATE_UP;
	cb->led_state = AVND_LED_STATE_RED;
	cb->destroy = FALSE;
	cb->stby_sync_state = AVND_STBY_IN_SYNC;

	/* assign the default timeout values (in nsec) */
	cb->msg_resp_intv = AVND_AVD_MSG_RESP_TIME * 1000000;

	/* initialize the AvND cb lock */
	m_NCS_LOCK_INIT(&cb->lock);
	m_AVND_LOG_LOCK(AVSV_LOG_LOCK_INIT, AVSV_LOG_LOCK_SUCCESS, NCSFL_SEV_INFO);

	/* initialize the PID monitor lock */
	m_NCS_LOCK_INIT(&cb->mon_lock);

	/* iniialize the error escaltion paramaets */
	cb->node_err_esc_level = AVND_ERR_ESC_LEVEL_0;

	/* create the association with hdl-mngr */
	if ((0 == (cb->cb_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVND, (NCSCONTEXT)cb)))) {
		m_AVND_LOG_CB(AVSV_LOG_CB_HDL_ASS_CREATE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
	m_AVND_LOG_CB(AVSV_LOG_CB_HDL_ASS_CREATE, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

   /*** initialize avnd dbs ***/

	/* initialize su db */
	if (NCSCC_RC_SUCCESS != avnd_sudb_init(cb))
		goto err;

	/* initialize comp db */
	if (NCSCC_RC_SUCCESS != avnd_compdb_init(cb))
		goto err;

	/* initialize healthcheck db */
	if (NCSCC_RC_SUCCESS != avnd_hcdb_init(cb))
		goto err;

	/* initialize clm db */
	if (NCSCC_RC_SUCCESS != avnd_clmdb_init(cb))
		goto err;

	/* initialize pg db */
	if (NCSCC_RC_SUCCESS != avnd_pgdb_init(cb))
		goto err;

	/* initialize pid_mon list */
	avnd_pid_mon_list_init(cb);

	/* initialize nodeid to mdsdest mapping db */
	if (NCSCC_RC_SUCCESS != avnd_nodeid_to_mdsdest_map_db_init(cb))
		goto err;

	/* initialize available internode components db */
	if (NCSCC_RC_SUCCESS != avnd_internode_avail_comp_db_init(cb))
		goto err;

	/* everything went off well.. store the cb hdl in the global variable */
	gl_avnd_hdl = cb->cb_hdl;

	/* NTFA Initialization */
	rc = saNtfInitialize(&cb->ntfHandle, &ntfCallbacks, &ntfVersion);
	if (rc != SA_AIS_OK) {
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR, "saNtfInitialize Failed (%u)", rc);
		goto err;
	}

	return cb;

 err:
	if (cb)
		avnd_cb_destroy(cb);

	return 0;
}

/****************************************************************************
  Name          : avnd_mbx_create
 
  Description   : This routine creates & attaches AvND mailbox.
 
  Arguments     : cb - ptr to AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_mbx_create(AVND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* create the mail box */
	rc = m_NCS_IPC_CREATE(&cb->mbx);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_MBX(AVSV_LOG_MBX_CREATE, AVSV_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
	m_AVND_LOG_MBX(AVSV_LOG_MBX_CREATE, AVSV_LOG_MBX_SUCCESS, NCSFL_SEV_INFO);

	/* attach the mail box */
	rc = m_NCS_IPC_ATTACH(&cb->mbx);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_MBX(AVSV_LOG_MBX_ATTACH, AVSV_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
	m_AVND_LOG_MBX(AVSV_LOG_MBX_ATTACH, AVSV_LOG_MBX_SUCCESS, NCSFL_SEV_INFO);

	return rc;

 err:
	/* destroy the mailbox */
	if (cb->mbx)
		avnd_mbx_destroy(cb);

	return rc;
}

/****************************************************************************
  Name          : avnd_ext_intf_create
 
  Description   : This routine initialize external interfaces (logging 
                  service being the exception).
 
  Arguments     : cb - ptr to AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_ext_intf_create(AVND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	EDU_ERR err = EDU_NORMAL;

	/* EDU initialisation */
	m_NCS_EDU_HDL_INIT(&cb->edu_hdl);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_INIT, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_dnd_msg, &err);

	if (rc != NCSCC_RC_SUCCESS) {
		/* Log ERROR */

		goto err;
	}

	m_NCS_EDU_HDL_INIT(&cb->edu_hdl_avnd);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_INIT, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl_avnd, avsv_edp_ndnd_msg, &err);

	if (rc != NCSCC_RC_SUCCESS) {
		/* Log ERROR */

		goto err;
	}

	m_NCS_EDU_HDL_INIT(&cb->edu_hdl_ava);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_INIT, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl_ava, avsv_edp_nda_msg, &err);

	if (rc != NCSCC_RC_SUCCESS) {
		/* Log ERROR */

		goto err;
	}

	m_NCS_EDU_HDL_INIT(&cb->edu_hdl_cla);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_INIT, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl_cla, avsv_edp_nd_cla_msg, &err);
	if (rc != NCSCC_RC_SUCCESS) {
		/* Log ERROR */

		goto err;
	}

	/* MDS registration */
	rc = avnd_mds_reg(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
	m_AVND_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

#ifdef NCS_AVND_MBCSV_CKPT
	/* MDS registration for VDEST */
	rc = avnd_mds_mbcsv_reg(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
#endif
	m_AVND_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

	/* MIB-LIB initialisation */
	rc = avnd_miblib_init(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		/* log */
		goto err;
	}

	/* MAB registration */
	rc = avnd_tbls_reg_with_mab(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		/* log */
		goto err;
	}
#ifdef NCS_AVND_MBCSV_CKPT
	/* MAB registration */
	rc = avnd_tbls_reg_with_mab_for_vdest(cb);
	if (NCSCC_RC_SUCCESS != rc) {
		/* log */
		goto err;
	}
#endif

	/* TBD Later */
	/* Initialise HPI */
	/* Initialise EDSv */

	return rc;

 err:
	/* destroy external interfaces */
	avnd_ext_intf_destroy(cb);

	return rc;
}

/****************************************************************************
  Name          : avnd_task_create
 
  Description   : This routine creates & starts AvND task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_task_create()
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* create avnd task */
	rc = m_NCS_TASK_CREATE((NCS_OS_CB)avnd_main_process, (void *)&gl_avnd_hdl, "AVND",
			       m_AVND_TASK_PRIORITY, m_AVND_STACKSIZE, &gl_avnd_task_hdl);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_TASK(AVSV_LOG_TASK_CREATE, AVSV_LOG_TASK_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
	m_AVND_LOG_TASK(AVSV_LOG_TASK_CREATE, AVSV_LOG_TASK_SUCCESS, NCSFL_SEV_INFO);

	/* now start the task */
	rc = m_NCS_TASK_START(gl_avnd_task_hdl);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_TASK(AVSV_LOG_TASK_START, AVSV_LOG_TASK_FAILURE, NCSFL_SEV_CRITICAL);
		goto err;
	}
	m_AVND_LOG_TASK(AVSV_LOG_TASK_START, AVSV_LOG_TASK_SUCCESS, NCSFL_SEV_INFO);

	return rc;

 err:
	/* destroy the task */
	if (gl_avnd_task_hdl)
		avnd_task_destroy();

	return rc;
}

/****************************************************************************
  Name          : avnd_cb_destroy
 
  Description   : This routine destroys AvND control block.
 
  Arguments     : cb - ptr to AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_cb_destroy(AVND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;

   /*** destroy all databases ***/

	/* We should delete external SU-SI first */
#ifdef NCS_AVND_MBCSV_CKPT
	if (NCSCC_RC_SUCCESS != (rc = avnd_ext_comp_data_clean_up(cb, TRUE)))
		goto done;
#endif
	/* destroy comp db */
	if (NCSCC_RC_SUCCESS != (rc = avnd_compdb_destroy(cb)))
		goto done;

	/* destroy su db */
	if (NCSCC_RC_SUCCESS != (rc = avnd_sudb_destroy(cb)))
		goto done;

	/* destroy healthcheck db */
	if (NCSCC_RC_SUCCESS != (rc = avnd_hcdb_destroy(cb)))
		goto done;

	/* destroy clm db */
	if (NCSCC_RC_SUCCESS != (rc = avnd_clmdb_destroy(cb)))
		goto done;

	/* destroy pg db */
	if (NCSCC_RC_SUCCESS != (rc = avnd_pgdb_destroy(cb)))
		goto done;

	/* Clean PID monitoring list */
	avnd_pid_mon_list_destroy(cb);

	/* destroy nodeid to mds dest db */
	if (NCSCC_RC_SUCCESS != (rc = avnd_nodeid_to_mdsdest_map_db_destroy(cb)))
		goto done;

	/* destroy available internode comp db */
	if (NCSCC_RC_SUCCESS != (rc = avnd_internode_avail_comp_db_destroy(cb)))
		goto done;

	/* destroy DND list */
	avnd_dnd_list_destroy(cb);

	/* destroy the lock */
	m_NCS_LOCK_DESTROY(&cb->lock);
	m_AVND_LOG_LOCK(AVSV_LOG_LOCK_FINALIZE, AVSV_LOG_LOCK_SUCCESS, NCSFL_SEV_INFO);

	/* destroy the PID monitor lock */
	m_NCS_LOCK_DESTROY(&cb->mon_lock);

	/* return AvND CB */
	ncshm_give_hdl(gl_avnd_hdl);

	/* remove the association with hdl-mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, cb->cb_hdl);
	m_AVND_LOG_CB(AVSV_LOG_CB_HDL_ASS_REMOVE, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	/* detach & destroy AvND mailbox */
	rc = avnd_mbx_destroy(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

	/* free the control block */
	m_MMGR_FREE_AVND_CB(cb);
	m_AVND_LOG_CB(AVSV_LOG_CB_DESTROY, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	/* reset the global cb handle */
	gl_avnd_hdl = 0;

 done:
	if (NCSCC_RC_SUCCESS != rc)
		m_AVND_LOG_INVALID_VAL_FATAL(rc);
	return rc;
}

/****************************************************************************
  Name          : avnd_mbx_destroy
 
  Description   : This routine destroys & detaches AvND mailbox.
 
  Arguments     : cb - ptr to AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_mbx_destroy(AVND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* detach the mail box */
	rc = m_NCS_IPC_DETACH(&cb->mbx, avnd_mbx_clean, cb);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_MBX(AVSV_LOG_MBX_DETACH, AVSV_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
		goto done;
	}
	m_AVND_LOG_MBX(AVSV_LOG_MBX_DETACH, AVSV_LOG_MBX_SUCCESS, NCSFL_SEV_INFO);

	/* delete the mail box */
	rc = m_NCS_IPC_RELEASE(&cb->mbx, 0);
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_MBX(AVSV_LOG_MBX_DESTROY, AVSV_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
		goto done;
	}
	m_AVND_LOG_MBX(AVSV_LOG_MBX_DESTROY, AVSV_LOG_MBX_SUCCESS, NCSFL_SEV_INFO);

 done:
	return rc;
}

/****************************************************************************
  Name          : avnd_ext_intf_destroy
 
  Description   : This routine destroys external interfaces (logging service
                  being the exception).
 
  Arguments     : cb - ptr to AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_ext_intf_destroy(AVND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* MDS unregistration */
	rc = avnd_mds_unreg(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

	/* EDU cleanup */
	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_FINALIZE, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl_avnd);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_FINALIZE, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl_ava);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_FINALIZE, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl_cla);
	m_AVND_LOG_EDU(AVSV_LOG_EDU_FINALIZE, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	/* Unregister all the avnd tables */
	/*rc = avnd_mab_unreg_rows(cb);
	 *if ( NCSCC_RC_SUCCESS != rc ) goto done;
	 */

	/* MAB unregistration */
	rc = avnd_tbls_unreg_with_mab(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

#ifdef NCS_AVND_MBCSV_CKPT
	/* MAB unregistration */
	rc = avnd_tbls_unreg_with_mab_for_vdest(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;
#endif
	/* TBD Later */
	/* unregister HPI */
	/* NTFA Finalize */
	rc = saNtfFinalize(cb->ntfHandle);
	if (rc != SA_AIS_OK) {
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR, "saNtfFinalize Failed (%u)", rc);
	}

 done:
	return rc;
}

/****************************************************************************
  Name          : avnd_task_destroy
 
  Description   : This routine destroys the AvND task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_task_destroy()
{
	/* release the task */
	m_NCS_TASK_RELEASE(gl_avnd_task_hdl);
	m_AVND_LOG_TASK(AVSV_LOG_TASK_RELEASE, AVSV_LOG_TASK_SUCCESS, NCSFL_SEV_INFO);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
   Name          : avnd_mbx_clean
  
   Description   : This routine dequeues & deletes all the events from the 
                   mailbox. It is invoked when mailbox is detached.
  
   Arguments     : arg - argument to be passed
                   msg - ptr to the 1st event in the mailbox
  
   Return Values : TRUE/FALSE
  
   Notes         : None.
 *****************************************************************************/
NCS_BOOL avnd_mbx_clean(NCSCONTEXT arg, NCSCONTEXT msg)
{
	AVND_EVT *curr;
	AVND_EVT *temp;

	/* clean the entire mailbox */
	for (curr = (AVND_EVT *)msg; curr;) {
		temp = curr;
		curr = curr->next;
		avnd_evt_destroy(temp);
	}

	return TRUE;
}

/****************************************************************************
   Name          : avnd_sigusr1_handler
  
   Description   : This routine handles the USR1 signal sent by NID script.
                   This routine posts the message to mailbox to clean 
                   all the NCS components also. This is the signal to perform 
                   the last step of termination including db clean-up.

   Arguments     : arg - argument to be passed
                   msg - ptr to the 1st event in the mailbox
  
   Return Values : TRUE/FALSE
  
   Notes         : None.
 *****************************************************************************/
static void avnd_sigusr1_handler(void)
{
	AVND_CB *cb = 0;
	AVND_EVT *evt = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* retrieve avnd cb */
	if (0 == (cb = (AVND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVND, (uns32)gl_avnd_hdl))) {
		m_AVND_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* create the evt with evt type indicating last step of termination */
	evt = avnd_evt_create(cb, AVND_EVT_LAST_STEP_TERM, NULL, NULL, NULL, 0, 0);
	if (!evt) {
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* send the event */
	rc = avnd_evt_send(cb, evt);

 done:
	/* free the event */
	if (NCSCC_RC_SUCCESS != rc && evt)
		avnd_evt_destroy(evt);

	if (cb)
		ncshm_give_hdl((uns32)gl_avnd_hdl);

	return;
}
