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

#include <immutil.h>
#include <logtrace.h>

#include "avnd.h"
#include "ncs_main_pvt.h"
#include "avsv_d2nedu.h"
#include "avsv_n2avaedu.h"
#include "avsv_n2claedu.h"
#include "avsv_nd2ndmsg.h"
#include "avnd_mon.h"

/* global task handle */
NCSCONTEXT gl_avnd_task_hdl = 0;

/* control block and global pointer to it  */
static AVND_CB _avnd_cb;
AVND_CB *avnd_cb = &_avnd_cb;

/* static function declarations */

static AVND_CB *avnd_cb_create(void);

static uns32 avnd_mbx_create(AVND_CB *);

static uns32 avnd_ext_intf_create(AVND_CB *);

static uns32 avnd_mbx_destroy(AVND_CB *);

static uns32 avnd_ext_intf_destroy(AVND_CB *);

static NCS_BOOL avnd_mbx_clean(NCSCONTEXT, NCSCONTEXT);

static int get_node_type(void)
{
        size_t bytes;
        int type;
        char buf[32];
        FILE *f = fopen(PKGSYSCONFDIR "node_type", "r");

        if (!f) {
                LOG_ER("Could not open file %s - %s", PKGSYSCONFDIR "node_type", strerror(errno));
                return AVSV_AVND_CARD_PAYLOAD;
        }

        if ((bytes = fscanf(f, "%s", buf)) > 0) {
                if (strncmp(buf, "controller", sizeof(buf)) == 0) {
			TRACE("Node type: controller");
                        type = AVSV_AVND_CARD_SYS_CON;
		}
                else if (strncmp(buf, "payload", sizeof(buf)) == 0) {
			TRACE("Node type: payload");
                        type = AVSV_AVND_CARD_PAYLOAD;
		}
                else {
                        syslog(LOG_ERR, "Unknown node type %s", buf);
                        type = AVSV_AVND_CARD_PAYLOAD;
                }
        } else {
		LOG_ER("fscanf FAILED for %s - %s", PKGSYSCONFDIR "node_type", strerror(errno));
                type = AVSV_AVND_CARD_PAYLOAD;
        }

        (void)fclose(f);

	return type;
}


/****************************************************************************
  Name          : avnd_create
 
  Description   : This routine creates & initializes the PWE of AvND. It does
                  the following:
                  a) create & initialize AvND control block.
                  b) create & attach AvND mailbox.
                  c) initialize external interfaces (logging service being the
                     exception).
 
  Arguments     : -
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_create(void)
{
	AVND_CB *cb = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* register with dtsv */
	rc = avnd_log_reg();

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
	AVND_CB *cb = avnd_cb;
	uns32 rc = NCSCC_RC_SUCCESS;

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
	/* unregister with DTSv */
	rc = avnd_log_unreg();

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
	AVND_CB *cb = avnd_cb;
	uns32 rc = NCSCC_RC_SUCCESS;
	SaVersionT ntfVersion = { 'A', 0x01, 0x01 };
	SaNtfCallbacksT ntfCallbacks = { NULL, NULL };
	SaVersionT immVersion = { 'A', 2, 1 };

	m_AVND_LOG_CB(AVSV_LOG_CB_CREATE, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	/* assign the AvND pool-id (used by hdl-mngr) */
	cb->pool_id = NCS_HM_POOL_ID_COMMON;

	/* assign the default states */
	cb->admin_state = SA_AMF_ADMIN_UNLOCKED;
	cb->oper_state = SA_AMF_OPERATIONAL_ENABLED;
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

	immutil_saImmOmInitialize(&cb->immOmHandle, NULL, &immVersion);

	/*** initialize avnd dbs ***/

	/* initialize su db */
	if (NCSCC_RC_SUCCESS != avnd_sudb_init(cb))
		goto err;

	/* initialize comp db */
	if (NCSCC_RC_SUCCESS != avnd_compdb_init(cb))
		goto err;

	/* initialize healthcheck db */
	avnd_hcdb_init(cb);

	/* initialize clm db */
	if (NCSCC_RC_SUCCESS != avnd_clmdb_init(cb))
		goto err;

	avnd_cb->clmdb.type = get_node_type();

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

	/* NTFA Initialization */
	rc = saNtfInitialize(&cb->ntfHandle, &ntfCallbacks, &ntfVersion);
	if (rc != SA_AIS_OK) {
		/* log the error code here */
		avnd_log(NCSFL_SEV_ERROR, "saNtfInitialize Failed (%u)", rc);
		goto err;
	}

	immutil_saImmOmInitialize(&cb->immOmHandle, NULL, &immVersion);

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
#if FIXME
	if (cb->clmdb.type == AVSV_AVND_CARD_SYS_CON) {
		rc = avnd_mds_mbcsv_reg(cb);
		if (NCSCC_RC_SUCCESS != rc) {
			m_AVND_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
			goto err;
		}
	}
#endif
	m_AVND_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

	return rc;

 err:
	/* destroy external interfaces */
	avnd_ext_intf_destroy(cb);

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

	/* detach & destroy AvND mailbox */
	rc = avnd_mbx_destroy(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

	cb = NULL;
	m_AVND_LOG_CB(AVSV_LOG_CB_DESTROY, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

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

	/* NTFA Finalize */
	rc = saNtfFinalize(cb->ntfHandle);
	if (rc != SA_AIS_OK) {
		avnd_log(NCSFL_SEV_ERROR, "saNtfFinalize Failed (%u)", rc);
	}
 done:
	return rc;
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

   Arguments     : 
  
   Return Values : TRUE/FALSE
  
   Notes         : None.
 *****************************************************************************/
void avnd_sigusr1_handler(void)
{
	AVND_EVT *evt = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER();

	/* create the evt with evt type indicating last step of termination */
	evt = avnd_evt_create(avnd_cb, AVND_EVT_LAST_STEP_TERM, NULL, NULL, NULL, 0, 0);
	if (!evt) {
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	/* send the event */
	rc = avnd_evt_send(avnd_cb, evt);

done:
	/* free the event */
	if (NCSCC_RC_SUCCESS != rc && evt)
		avnd_evt_destroy(evt);
}
