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

  This file contains functions related to the Event Handling
..............................................................................

  FUNCTIONS INCLUDED in this module:

   glnd_process_gla_register_agent .........Registers the agent info.
   glnd_process_gla_unreg_agent ............Unregisters the agent info.
   glnd_process_gla_client_initialize.......Initialize the client info.
   glnd_process_gla_client_finalize.........Delete the client info.
   glnd_process_gla_resource_open ..........Request for resource open.
   glnd_process_gla_resource_close..........Request for resource close.
   glnd_process_gla_resource_lock ..........Request for resource lock
   glnd_process_gla_resource_unlock ........Request for resource unlock.
   glnd_process_gla_resource_purge..........Request for resource purge.

   glnd_process_glnd_lck_req ...............Request from non-master glnd for lock.
   glnd_process_glnd_unlck_req .............Request from non-master glnd for unlock.
   glnd_process_glnd_lck_grant .............Response to the non-master glnd for lock grant.
   glnd_process_glnd_lck_block .............Response to the non-master glnd for lock blocked..
   glnd_process_glnd_lck_req_cancel.........Cancel the lock request from non-master.
   glnd_process_glnd_lck_req_orphan.........Orphan the lock request from non-master.
   glnd_process_glnd_lck_waiter_clbk........Notification to the lock holders on the resource block.
   glnd_process_glnd_lck_purge..............Request from non-master glnd for all orphan lock deletion.
   glnd_process_glnd_send_rsc_info..........Send all info regarding the resource.
   glnd_process_glnd_fwd_dd_probe...........Forward the deadlock detection probe.
   glnd_process_glnd_dd_probe ..............Initiate the deadlock detection probe.

   glnd_process_gld_resource_details .......Response to the resource details query.
   glnd_process_gld_resource_new_master ....Notification of the new master after election.
   glnd_process_gld_resource_master_detail..Update resource-master details after GLND restarts. 

   glnd_process_tmr_resource_req_timeout....... Timeout function for the resource open
   glnd_process_tmr_nm_resource_lock_req_timeout....... Timeout function for the resource lock open
   glnd_process_tmr_nm_resource_unlock_req_timeout... Timeout function for the resource lock close

   glnd_process_gld_non_master_status.......GLD has sent new status of non_master GLND
   glnd_process_glnd_cb_dump.................... Debug function to dump the cb.

  
******************************************************************************/

#include <logtrace.h>

#include "glnd.h"

/******************************************************************************/

/* This is the function prototype for event handling */
typedef uns32 (*GLND_EVT_HANDLER) (GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);

/* Event Processing Prototypes */
static uns32 glnd_process_gla_register_agent(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_unreg_agent(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_client_initialize(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_client_finalize(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_resource_open(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_resource_close(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_register_agent(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_resource_lock(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_resource_unlock(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_resource_purge(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gla_client_info(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_lck_req(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_unlck_req(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_lck_rsp(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_unlck_rsp(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_lck_req_cancel(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_lck_req_orphan(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_lck_waiter_clbk(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_lck_purge(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_send_rsc_info(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_fwd_dd_probe(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_dd_probe(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);

static uns32 glnd_process_gld_resource_details(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gld_resource_master_state(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gld_resource_new_master(GLND_CB *glnd_cb, GLSV_EVT_GLND_NEW_MAST_INFO *new_master_info);
static uns32 glnd_process_tmr_resource_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_tmr_resource_lock_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_tmr_nm_resource_lock_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_tmr_nm_resource_unlock_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_tmr_agent_info_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gld_non_master_status(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_glnd_cb_dump(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_gld_resource_master_details(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt);
static uns32 glnd_process_non_master_lck_req_status(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_node,
						    GLND_RES_LOCK_LIST_INFO *lock_list_info,
						    GLND_EVT_GLND_NON_MASTER_STATUS *node_status);
static uns32 glnd_process_check_master_and_non_master_status(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_info);

/* GLND event dispatch table */
const GLSV_GLND_EVT_HANDLER glsv_glnd_evt_dispatch_tbl[GLSV_GLND_EVT_MAX - GLSV_GLND_EVT_BASE] = {
	glnd_process_gla_register_agent,
	glnd_process_gla_unreg_agent,
	glnd_process_gla_client_initialize,
	glnd_process_gla_client_finalize,
	glnd_process_gla_resource_open,
	glnd_process_gla_resource_close,
	glnd_process_gla_resource_lock,
	glnd_process_gla_resource_unlock,
	glnd_process_gla_resource_purge,
	glnd_process_gla_client_info,
	glnd_process_glnd_lck_req,
	glnd_process_glnd_unlck_req,
	glnd_process_glnd_lck_rsp,
	glnd_process_glnd_unlck_rsp,
	glnd_process_glnd_lck_req_cancel,
	glnd_process_glnd_lck_req_orphan,
	glnd_process_glnd_lck_waiter_clbk,
	glnd_process_glnd_lck_purge,
	glnd_process_glnd_send_rsc_info,
	glnd_process_glnd_fwd_dd_probe,
	glnd_process_glnd_dd_probe,
	glnd_process_gld_resource_details,
	glnd_process_gld_resource_master_state,
	glnd_process_gld_resource_master_details,
	glnd_process_tmr_resource_req_timeout,
	glnd_process_tmr_resource_lock_req_timeout,
	glnd_process_tmr_nm_resource_lock_req_timeout,
	glnd_process_tmr_nm_resource_unlock_req_timeout,
	glnd_process_tmr_agent_info_timeout,
	glnd_process_gld_non_master_status,
	glnd_process_glnd_cb_dump
};

/****************************************************************************
 * Name          : glnd_evt_destroy
 *
 * Description   : This is the function which is used to free all event 
 *                 pointer which it has received/Send by GLND.
 *
 * Arguments     : evt  - This is the pointer which holds thee  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glnd_evt_destroy(GLSV_GLND_EVT *evt)
{
	/* do any event specific incase of link lists */
	switch (evt->type) {
	case GLSV_GLND_EVT_REG_AGENT:
	case GLSV_GLND_EVT_UNREG_AGENT:
	case GLSV_GLND_EVT_INITIALIZE:
	case GLSV_GLND_EVT_FINALIZE:
	case GLSV_GLND_EVT_RSC_OPEN:
	case GLSV_GLND_EVT_RSC_CLOSE:
	case GLSV_GLND_EVT_RSC_LOCK:
	case GLSV_GLND_EVT_RSC_UNLOCK:
	case GLSV_GLND_EVT_RSC_PURGE:
	case GLSV_GLND_EVT_LCK_REQ:
	case GLSV_GLND_EVT_UNLCK_REQ:
	case GLSV_GLND_EVT_LCK_RSP:
	case GLSV_GLND_EVT_UNLCK_RSP:
	case GLSV_GLND_EVT_LCK_WAITER_CALLBACK:
	case GLSV_GLND_EVT_LCK_PURGE:
	case GLSV_GLND_EVT_RSC_GLD_DETAILS:
	case GLSV_GLND_EVT_RSC_NEW_MASTER:
	case GLSV_GLND_EVT_NON_MASTER_INFO:
	case GLSV_GLND_EVT_CB_DUMP:
		/* free the specific entry for this events */
		m_MMGR_FREE_GLND_EVT(evt);
		break;
	case GLSV_GLND_EVT_RSC_MASTER_INFO:
		if (evt->info.rsc_master_info.rsc_master_list) {
			m_MMGR_FREE_GLND_RES_MASTER_LIST_INFO(evt->info.rsc_master_info.rsc_master_list);
			evt->info.rsc_master_info.rsc_master_list = NULL;
		}
		m_MMGR_FREE_GLND_EVT(evt);
		break;
	case GLSV_GLND_EVT_FWD_DD_PROBE:
	case GLSV_GLND_EVT_DD_PROBE:
		/* free the dd list */
		{
			GLSV_GLND_DD_INFO_LIST *dd_list = evt->info.dd_probe_info.dd_info_list, *tmp_dd_list = NULL;
			while (dd_list) {
				tmp_dd_list = dd_list;
				dd_list = dd_list->next;
				m_MMGR_FREE_GLSV_GLND_DD_INFO_LIST(tmp_dd_list, NCS_SERVICE_ID_GLND);
			}

		}
		m_MMGR_FREE_GLND_EVT(evt);
		break;
	case GLSV_GLND_EVT_SND_RSC_INFO:
		/* need free the lock list */
		{
			GLND_LOCK_LIST_INFO *lck_list_info = evt->info.node_rsc_info.list_of_req, *tmp_list = NULL;
			while (lck_list_info) {
				tmp_list = lck_list_info;
				lck_list_info = lck_list_info->next;
				m_MMGR_FREE_GLSV_GLND_LOCK_LIST_INFO(tmp_list, NCS_SERVICE_ID_GLND);
			}
		}
		m_MMGR_FREE_GLND_EVT(evt);
		break;
	default:
		m_MMGR_FREE_GLND_EVT(evt);
		break;
	}
	return;
}

/****************************************************************************
 * Name          : glnd_retrieve_info_from_evt
 *
 * Description   :  
 *
 * Notes         : None.
 *****************************************************************************/
static void glnd_retrieve_info_from_evt(GLSV_GLND_EVT *evt,
					uns32 *node,
					SaLckHandleT *hdl_id, SaLckResourceIdT *rsc_id, SaLckLockIdT *lck_id)
{
	/* initialize the values */
	*node = 0;
	*hdl_id = 0;
	*rsc_id = 0;
	*lck_id = 0;

	switch (evt->type) {
	case GLSV_GLND_EVT_REG_AGENT:
	case GLSV_GLND_EVT_UNREG_AGENT:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.agent_info.agent_mds_dest);
		break;
	case GLSV_GLND_EVT_INITIALIZE:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.client_info.agent_mds_dest);
		break;
	case GLSV_GLND_EVT_FINALIZE:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.finalize_info.agent_mds_dest);
		break;
	case GLSV_GLND_EVT_RSC_OPEN:
	case GLSV_GLND_EVT_RSC_CLOSE:
	case GLSV_GLND_EVT_RSC_PURGE:
	case GLSV_GLND_EVT_LCK_PURGE:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.rsc_info.agent_mds_dest);
		*hdl_id = evt->info.rsc_info.client_handle_id;
		*rsc_id = evt->info.rsc_info.resource_id;
		break;
	case GLSV_GLND_EVT_RSC_LOCK:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.rsc_lock_info.agent_mds_dest);
		*hdl_id = evt->info.rsc_lock_info.client_handle_id;
		*rsc_id = evt->info.rsc_lock_info.resource_id;
		break;
	case GLSV_GLND_EVT_RSC_UNLOCK:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.rsc_unlock_info.agent_mds_dest);
		*hdl_id = evt->info.rsc_unlock_info.client_handle_id;
		*rsc_id = evt->info.rsc_unlock_info.resource_id;
		*lck_id = evt->info.rsc_unlock_info.lockid;
		break;
	case GLSV_GLND_EVT_CLIENT_INFO:
		break;

	case GLSV_GLND_EVT_LCK_REQ:
	case GLSV_GLND_EVT_UNLCK_REQ:
	case GLSV_GLND_EVT_LCK_RSP:
	case GLSV_GLND_EVT_LCK_WAITER_CALLBACK:
	case GLSV_GLND_EVT_UNLCK_RSP:
	case GLSV_GLND_EVT_LCK_REQ_CANCEL:
	case GLSV_GLND_EVT_LCK_REQ_ORPHAN:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.node_lck_info.glnd_mds_dest);
		*hdl_id = evt->info.node_lck_info.client_handle_id;
		*rsc_id = evt->info.node_lck_info.resource_id;
		*lck_id = evt->info.node_lck_info.lockid;
		break;
	case GLSV_GLND_EVT_SND_RSC_INFO:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.node_rsc_info.glnd_mds_dest);
		*rsc_id = evt->info.node_rsc_info.resource_id;
		*lck_id = (SaLckLockIdT)evt->info.node_rsc_info.num_requests;
		break;
	case GLSV_GLND_EVT_FWD_DD_PROBE:
	case GLSV_GLND_EVT_DD_PROBE:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.dd_probe_info.dest_id);
		*hdl_id = evt->info.dd_probe_info.hdl_id;
		*rsc_id = evt->info.dd_probe_info.rsc_id;
		*lck_id = evt->info.dd_probe_info.lck_id;
		break;
	case GLSV_GLND_EVT_RSC_GLD_DETAILS:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.rsc_gld_info.master_dest_id);
		*rsc_id = evt->info.rsc_gld_info.rsc_id;
		break;
	case GLSV_GLND_EVT_RSC_NEW_MASTER:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.new_master_info.master_dest_id);
		*rsc_id = evt->info.new_master_info.rsc_id;
		break;
	case GLSV_GLND_EVT_RSC_MASTER_INFO:
		break;

	case GLSV_GLND_EVT_RSC_OPEN_TIMEOUT:
	case GLSV_GLND_EVT_RSC_LOCK_TIMEOUT:
	case GLSV_GLND_EVT_NM_RSC_LOCK_TIMEOUT:
	case GLSV_GLND_EVT_NM_RSC_UNLOCK_TIMEOUT:
		break;
	case GLSV_GLND_EVT_NON_MASTER_INFO:
		*node = m_NCS_NODE_ID_FROM_MDS_DEST(evt->info.non_master_info.dest_id);
		break;

	default:
		break;
	}
	return;
}

/****************************************************************************
 * Name          : glnd_process_evt
 *
 * Description   : This is the function which is called when GLND receives any
 *                 event. 
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 glnd_process_evt(NCSCONTEXT cb, GLSV_GLND_EVT *evt)
{
	GLND_EVT_HANDLER glnd_evt_hdl = NULL;
	SaLckHandleT hdl_id = 0;
	SaLckResourceIdT rsc_id = 0;
	SaLckLockIdT lck_id = 0;
	uns32 node_id = 0;
	GLND_CB *glnd_cb = (GLND_CB *)cb;

	glnd_retrieve_info_from_evt(evt, &node_id, &hdl_id, &rsc_id, &lck_id);
	m_LOG_GLND_EVT(GLND_EVT_RECIEVED, __FILE__, __LINE__, evt->type, (uns32)node_id, (uns32)hdl_id, (uns32)rsc_id,
		       (uns32)lck_id);
	glnd_evt_hdl = glsv_glnd_evt_dispatch_tbl[evt->type - (GLSV_GLND_EVT_BASE + 1)];
	if (glnd_evt_hdl != NULL) {
		if (glnd_evt_hdl(glnd_cb, evt) != NCSCC_RC_SUCCESS) {
			/* Log the event for the failure */
			m_LOG_GLND_EVT(GLND_EVT_PROCESS_FAILURE, __FILE__, __LINE__, evt->type, 0, 0, 0, 0);
		}
	}
	glnd_evt_destroy(evt);
	return (NCSCC_RC_SUCCESS);
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_register_agent

  DESCRIPTION    : Process the GLSV_GLND_EVT_REG_AGENT event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_register_agent(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_AGENT_INFO *agent_info;

	agent_info = (GLSV_EVT_AGENT_INFO *)&evt->info.agent_info;
	/* add it to the agent tree */
	glnd_agent_node_add(glnd_cb, agent_info->agent_mds_dest, agent_info->process_id);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_unreg_agent

  DESCRIPTION    : Process the GLSV_GLND_EVT_UNREG_AGENT event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_unreg_agent(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_AGENT_INFO *agent_info;
	GLND_AGENT_INFO *del_node;

	agent_info = (GLSV_EVT_AGENT_INFO *)&evt->info.agent_info;

	del_node = glnd_agent_node_find(glnd_cb, agent_info->agent_mds_dest);

	if (del_node) {
		/* delete it from the agent tree and also clean up all the clients associated with it */
		glnd_agent_node_del(glnd_cb, del_node);
		return NCSCC_RC_SUCCESS;
	} else {
		m_LOG_GLND_API(GLND_AGENT_NODE_NOT_FOUND, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_client_initialize

  DESCRIPTION    : Process the GLSV_GLND_EVT_INITIALIZE event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_client_initialize(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_CLIENT_INFO *client_info;
	GLND_CLIENT_INFO *add_node;
	GLSV_GLA_EVT gla_evt;
	SaLckHandleT app_handle_id = 0;

	client_info = (GLSV_EVT_CLIENT_INFO *)&evt->info.client_info;

	memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
	gla_evt.type = GLSV_GLA_API_RESP_EVT;

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE || glnd_cb->gld_card_up != TRUE) {
		m_LOG_GLND_API(GLND_RSC_REQ_NODE_ADD_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		/* initialise the gla_evt */
		gla_evt.error = SA_AIS_ERR_TRY_AGAIN;
		gla_evt.type = GLSV_GLA_API_RESP_EVT;
		gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_INITIALIZE;

		/* send the evt */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, client_info->agent_mds_dest, &evt->mds_context);

		return NCSCC_RC_FAILURE;
	}

	/* verify that the agent exists */
	if (!glnd_agent_node_find(glnd_cb, client_info->agent_mds_dest)) {
		m_LOG_GLND_API(GLND_AGENT_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		goto err;
	}

	/* add the new client to the tree */
	add_node = glnd_client_node_add(glnd_cb, client_info->agent_mds_dest, app_handle_id);

	/* send back the handle id for the client */
	if (add_node) {
		/* update the fields */
		add_node->app_proc_id = client_info->client_proc_id;
		add_node->cbk_reg_info = client_info->cbk_reg_info;
		add_node->version = client_info->version;

		gla_evt.error = SA_AIS_OK;
		gla_evt.handle = add_node->app_handle_id;
		gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_INITIALIZE;
		gla_evt.info.gla_resp_info.param.lck_init.dummy = 0;

		/* send the message */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, add_node->agent_mds_dest, &evt->mds_context);
		return NCSCC_RC_SUCCESS;
	}

 err:
	gla_evt.error = SA_AIS_ERR_LIBRARY;
	gla_evt.handle = 0;
	gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_INITIALIZE;
	gla_evt.info.gla_resp_info.param.lck_init.dummy = 0;
	/* send the message */
	glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, client_info->agent_mds_dest, &evt->mds_context);
	return NCSCC_RC_FAILURE;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_client_finalize

  DESCRIPTION    : Process the GLSV_GLND_EVT_FINALIZE event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_client_finalize(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_FINALIZE_INFO *finalize_info;
	GLND_CLIENT_INFO *del_node;
	GLSV_GLA_EVT gla_evt;

	finalize_info = (GLSV_EVT_FINALIZE_INFO *)&evt->info.finalize_info;
	memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
	gla_evt.type = GLSV_GLA_API_RESP_EVT;

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE || glnd_cb->gld_card_up != TRUE) {
		m_LOG_GLND_API(GLND_RSC_REQ_NODE_ADD_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		/* initialise the gla_evt */
		gla_evt.error = SA_AIS_ERR_TRY_AGAIN;
		gla_evt.handle = finalize_info->handle_id;
		gla_evt.type = GLSV_GLA_API_RESP_EVT;
		gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_FINALIZE;

		/* send the evt */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, finalize_info->agent_mds_dest, &evt->mds_context);

		return NCSCC_RC_FAILURE;
	}

	/* verify that the agent exists */
	if ((del_node = glnd_client_node_find(glnd_cb, finalize_info->handle_id))) {
		/* delete the client from the tree */
		if (glnd_client_node_del(glnd_cb, del_node) == NCSCC_RC_SUCCESS) {
			gla_evt.error = SA_AIS_OK;
			gla_evt.handle = finalize_info->handle_id;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_FINALIZE;
			gla_evt.info.gla_resp_info.param.lck_fin.dummy = 0;
			/* send the message */
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, finalize_info->agent_mds_dest, &evt->mds_context);
			return NCSCC_RC_SUCCESS;
		}
	}
	gla_evt.error = SA_AIS_ERR_LIBRARY;
	gla_evt.handle = 0;
	gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_FINALIZE;
	gla_evt.info.gla_resp_info.param.lck_fin.dummy = 0;
	/* send the message */
	glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, finalize_info->agent_mds_dest, &evt->mds_context);
	return NCSCC_RC_FAILURE;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_resource_open

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_OPEN event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_resource_open(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_RSC_INFO *rsc_info;
	GLND_RESOURCE_INFO *resource_node;
	GLND_RESOURCE_REQ_LIST *res_req_node;
	GLSV_GLA_EVT gla_evt;
	GLSV_GLD_EVT gld_evt;

	rsc_info = (GLSV_EVT_RSC_INFO *)&evt->info.rsc_info;

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE || glnd_cb->gld_card_up != TRUE) {
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
		gla_evt.handle = rsc_info->client_handle_id;

		if (rsc_info->call_type == GLSV_SYNC_CALL) {
			m_LOG_GLND_API(GLND_RSC_REQ_NODE_ADD_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
			/* initialise the gla_evt */
			gla_evt.error = SA_AIS_ERR_TRY_AGAIN;
			gla_evt.type = GLSV_GLA_API_RESP_EVT;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_OPEN;

			/* send the evt */
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);

			return NCSCC_RC_FAILURE;
		} else {
			gla_evt.type = GLSV_GLA_CALLBK_EVT;
			gla_evt.info.gla_clbk_info.callback_type = GLSV_LOCK_RES_OPEN_CBK;
			gla_evt.info.gla_clbk_info.resourceId = rsc_info->lcl_resource_id;
			gla_evt.info.gla_clbk_info.params.res_open.resourceId = 0;
			gla_evt.info.gla_clbk_info.params.res_open.invocation = rsc_info->invocation;
			gla_evt.info.gla_clbk_info.params.res_open.error = SA_AIS_ERR_TRY_AGAIN;
			/* send the evt */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest);

			return NCSCC_RC_FAILURE;
		}

	}

	/* search locally for the resource */
	resource_node = glnd_resource_node_find_by_name(glnd_cb, &evt->info.rsc_info.resource_name);

	if (resource_node) {
		/* populate the evt to be sent to gla */
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		if (glnd_client_node_resource_add
		    (glnd_client_node_find(glnd_cb, rsc_info->client_handle_id), resource_node) == NCSCC_RC_SUCCESS) {

			/* inc the local reference */
			resource_node->lcl_ref_cnt++;
			glnd_restart_resource_info_ckpt_overwrite(glnd_cb, resource_node);
		}

		gla_evt.error = SA_AIS_OK;

		gla_evt.handle = rsc_info->client_handle_id;
		if (rsc_info->call_type == GLSV_SYNC_CALL) {
			gla_evt.type = GLSV_GLA_API_RESP_EVT;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_OPEN;
			gla_evt.info.gla_resp_info.param.res_open.resourceId = resource_node->resource_id;
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);
		} else {
			gla_evt.type = GLSV_GLA_CALLBK_EVT;
			gla_evt.info.gla_clbk_info.callback_type = GLSV_LOCK_RES_OPEN_CBK;
			gla_evt.info.gla_clbk_info.resourceId = rsc_info->lcl_resource_id;
			gla_evt.info.gla_clbk_info.params.res_open.resourceId = resource_node->resource_id;
			gla_evt.info.gla_clbk_info.params.res_open.invocation = rsc_info->invocation;
			gla_evt.info.gla_clbk_info.params.res_open.error = gla_evt.error;

			/* send the evt */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest);
		}
		/*no of users fix */
		memset(&gld_evt, 0, sizeof(GLSV_GLD_EVT));
		/* populate the evt to be sent to gld */
		gld_evt.evt_type = GLSV_GLD_EVT_RSC_OPEN;
		memcpy(&gld_evt.info.rsc_open_info.rsc_name, &rsc_info->resource_name, sizeof(SaNameT));
		gld_evt.info.rsc_open_info.flag = rsc_info->flag;
		glnd_mds_msg_send_gld(glnd_cb, &gld_evt, glnd_cb->gld_mdest_id);

	} else {
		/* create a local copy */
		/* create the request node */
		res_req_node =
		    glnd_resource_req_node_add(glnd_cb, rsc_info, &evt->mds_context, rsc_info->lcl_resource_id);

		if (!res_req_node) {
			m_LOG_GLND_API(GLND_RSC_REQ_NODE_ADD_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
			/* initialise the gla_evt */
			memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

			gla_evt.error = SA_AIS_ERR_NO_MEMORY;
			gla_evt.handle = rsc_info->client_handle_id;
			if (rsc_info->call_type == GLSV_SYNC_CALL) {
				gla_evt.type = GLSV_GLA_API_RESP_EVT;
				gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_OPEN;
				glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest,
							  &evt->mds_context);
			} else {
				gla_evt.type = GLSV_GLA_CALLBK_EVT;
				gla_evt.info.gla_clbk_info.callback_type = GLSV_LOCK_RES_OPEN_CBK;
				gla_evt.info.gla_clbk_info.resourceId = rsc_info->lcl_resource_id;
				gla_evt.info.gla_clbk_info.params.res_open.invocation = rsc_info->invocation;
				gla_evt.info.gla_clbk_info.params.res_open.error = SA_AIS_ERR_NO_MEMORY;
				/* send the evt */
				glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest);
			}

			return NCSCC_RC_FAILURE;
		} else {
			/* not found . send the request to the director */
			memset(&gld_evt, 0, sizeof(GLSV_GLD_EVT));
			/* populate the evt to be sent to gld */
			gld_evt.evt_type = GLSV_GLD_EVT_RSC_OPEN;
			memcpy(&gld_evt.info.rsc_open_info.rsc_name, &rsc_info->resource_name, sizeof(SaNameT));
			gld_evt.info.rsc_open_info.flag = rsc_info->flag;
			glnd_mds_msg_send_gld(glnd_cb, &gld_evt, glnd_cb->gld_mdest_id);
		}
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_client_info

  DESCRIPTION    : 

  ARGUMENTS      :
              

  RETURNS       : 

*****************************************************************************/
static uns32 glnd_process_gla_client_info(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLND_CLIENT_INFO *add_node;
	GLND_RESOURCE_INFO *res_node;
	GLSV_EVT_RESTART_CLIENT_INFO *restart_client_info = (GLSV_EVT_RESTART_CLIENT_INFO *)&evt->info.restart_client_info;

	
	if (restart_client_info->resource_id) {
		/* Add res_node to the glnd_res_tree of glnd_cb */
		res_node =
		    (GLND_RESOURCE_INFO *)glnd_restart_client_resource_node_add(glnd_cb,
										restart_client_info->resource_id);

		/* Add res_node to the client info */
		if (res_node)
			glnd_client_node_resource_add(glnd_client_node_find
						      (glnd_cb, restart_client_info->client_handle_id), res_node);
		else
			return NCSCC_RC_FAILURE;

	} else {

		/* add the new client to the tree */
		add_node = glnd_client_node_add(glnd_cb, restart_client_info->agent_mds_dest,
						restart_client_info->client_handle_id);
		if (add_node) {
			/* update the fields */
			add_node->app_proc_id = restart_client_info->app_proc_id;
			add_node->cbk_reg_info = restart_client_info->cbk_reg_info;
			add_node->version = restart_client_info->version;
		} else
			return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_resource_close

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_CLOSE event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_resource_close(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_RSC_INFO *rsc_info;
	GLND_RESOURCE_INFO *res_node;
	GLND_CLIENT_INFO *client_info;
	GLSV_GLA_EVT gla_evt;
	GLSV_GLD_EVT gld_evt;
	SaAisErrorT error = SA_AIS_ERR_LIBRARY;
	NCS_BOOL resource_del = FALSE, orphan = FALSE;
	SaLckLockModeT mode;

	rsc_info = (GLSV_EVT_RSC_INFO *)&evt->info.rsc_info;

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE || glnd_cb->gld_card_up != TRUE) {
		m_LOG_GLND_API(GLND_RSC_REQ_NODE_ADD_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		/* initialise the gla_evt */
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		gla_evt.error = SA_AIS_ERR_TRY_AGAIN;
		gla_evt.handle = rsc_info->client_handle_id;
		gla_evt.type = GLSV_GLA_API_RESP_EVT;
		gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_CLOSE;
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);
		return NCSCC_RC_SUCCESS;
	}

	/* get the client handle */
	client_info = glnd_client_node_find(glnd_cb, rsc_info->client_handle_id);
	if (!client_info) {
		/* initialise the gla_evt */
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		gla_evt.error = SA_AIS_ERR_BAD_HANDLE;
		gla_evt.handle = rsc_info->client_handle_id;
		gla_evt.type = GLSV_GLA_API_RESP_EVT;
		gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_CLOSE;
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);

		m_LOG_GLND_API(GLND_CLIENT_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

	/* find the resource node */
	res_node = glnd_resource_node_find(glnd_cb, rsc_info->resource_id);

	if (res_node) {
		glnd_set_orphan_state(glnd_cb, res_node);

		glnd_client_node_lcl_resource_del(glnd_cb, client_info, res_node, rsc_info->lcl_resource_id,
						  rsc_info->lcl_resource_id_count, &resource_del);
		/* decrement the resource count and if zero send the event to the gld */
		if (resource_del == TRUE) {
			if (res_node->lcl_ref_cnt != 0 && rsc_info->lcl_resource_id_count == 1) {
				res_node->lcl_ref_cnt--;
				glnd_restart_resource_info_ckpt_overwrite(glnd_cb, res_node);

				if (res_node->lcl_ref_cnt == 0
				    && glnd_resource_grant_list_orphan_locks(res_node, &mode) == FALSE) {
					glnd_resource_node_destroy(glnd_cb, res_node, orphan);
					res_node = NULL;	/* It is freed inside glnd_resource_node_destroy */
				}
			}
			if (res_node) {
				glnd_resource_master_lock_resync_grant_list(glnd_cb, res_node);

				memset(&gld_evt, 0, sizeof(GLSV_GLD_EVT));

				gld_evt.evt_type = GLSV_GLD_EVT_RSC_CLOSE;
				gld_evt.info.rsc_details.rsc_id = res_node->resource_id;
				gld_evt.info.rsc_details.lcl_ref_cnt = rsc_info->lcl_resource_id_count;
				glnd_mds_msg_send_gld(glnd_cb, &gld_evt, glnd_cb->gld_mdest_id);

			}

		}
		error = SA_AIS_OK;	/* NEED TO SEE */
	} else {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		error = SA_AIS_ERR_INVALID_PARAM;
	}

	gla_evt.error = error;
	gla_evt.type = GLSV_GLA_API_RESP_EVT;
	gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_CLOSE;
	glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_check_master_and_non_master_status

  DESCRIPTION    : Checks to see if any exclusive locks are present in the grant list
  ARGUMENTS      :
                  res_info      - ptr to the Resource Node.

  RETURNS        : TRUE/FALSE

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_check_master_and_non_master_status(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_info)
{
	GLND_RES_LOCK_LIST_INFO *lock_list_info = NULL;

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE || glnd_cb->gld_card_up != TRUE
	    || res_info->master_status != GLND_OPERATIONAL_STATE)
		return SA_AIS_ERR_TRY_AGAIN;

	for (lock_list_info = res_info->lck_master_info.grant_list; lock_list_info != NULL;
	     lock_list_info = lock_list_info->next) {
		if (lock_list_info->lock_info.lock_type == SA_LCK_EX_LOCK_MODE
		    && lock_list_info->non_master_status == GLND_RESTART_STATE)
			return SA_AIS_ERR_TRY_AGAIN;
	}
	return SA_AIS_OK;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_resource_lock

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_LOCK event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_resource_lock(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_RSC_LOCK_INFO *rsc_lock_info;
	GLND_RESOURCE_INFO *res_node;
	GLSV_LOCK_REQ_INFO lck_info;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;
	GLND_CLIENT_INFO *client_info;
	GLSV_GLA_EVT gla_evt;
	SaAisErrorT error = SA_AIS_ERR_LIBRARY;
	uns32 rc;

	memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
	memset(&lck_info, 0, sizeof(GLSV_LOCK_REQ_INFO));

	rsc_lock_info = (GLSV_EVT_RSC_LOCK_INFO *)&evt->info.rsc_lock_info;

	/* check for the resource node  */
	res_node = glnd_resource_node_find(glnd_cb, rsc_lock_info->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		error = SA_AIS_ERR_BAD_HANDLE;
		goto err;
	}

	rc = glnd_process_check_master_and_non_master_status(glnd_cb, res_node);
	if (rc == SA_AIS_ERR_TRY_AGAIN) {
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		if (rsc_lock_info->call_type == GLSV_SYNC_CALL) {
			m_LOG_GLND_API(GLND_RSC_REQ_NODE_ADD_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
			/* initialise the gla_evt */
			gla_evt.error = SA_AIS_ERR_TRY_AGAIN;
			gla_evt.handle = rsc_lock_info->client_handle_id;
			gla_evt.type = GLSV_GLA_API_RESP_EVT;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_SYNC_LOCK;

			/* send the evt to GLA */
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest, &evt->mds_context);
			return NCSCC_RC_FAILURE;
		} else {
			m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_ERR_TRY_AGAIN,
							     0,
							     rsc_lock_info->lcl_lockid,
							     rsc_lock_info->lock_type,
							     rsc_lock_info->lcl_resource_id,
							     rsc_lock_info->invocation,
							     0, rsc_lock_info->client_handle_id);

			/* send the evt to GLA */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest);
			return NCSCC_RC_FAILURE;

		}

	}

	/* get the client handle */
	client_info = glnd_client_node_find(glnd_cb, rsc_lock_info->client_handle_id);
	if (!client_info) {
		m_LOG_GLND_API(GLND_CLIENT_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	gla_evt.handle = rsc_lock_info->client_handle_id;

	if (res_node->status == GLND_RESOURCE_NOT_INITIALISED) {
		/* sleep for relection time and resend the event */
		uns32 tm = GLSV_GLND_MASTER_REELECTION_WAIT_TIME / 10000000;
		GLSV_GLND_EVT *resend_evt = m_MMGR_ALLOC_GLND_EVT;

		memcpy(resend_evt, evt, sizeof(GLSV_GLND_EVT));
		resend_evt->next = NULL;
		m_NCS_TASK_SLEEP(tm);
		glnd_evt_local_send(glnd_cb, resend_evt, NCS_IPC_PRIORITY_NORMAL);
		return NCSCC_RC_SUCCESS;
	}

	/* check to see if the request is for a duplicate ex */
	if (rsc_lock_info->lock_type == SA_LCK_EX_LOCK_MODE &&
	    glnd_client_node_resource_lock_find_duplicate_ex(client_info, res_node->resource_id,
							     rsc_lock_info->lcl_resource_id) == NCSCC_RC_FAILURE) {
		/* send back the duplicate status */
		if (rsc_lock_info->call_type == GLSV_SYNC_CALL) {
			m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_OK,
							    0,
							    SA_LCK_LOCK_DUPLICATE_EX, rsc_lock_info->client_handle_id);
			/* send the evt to GLA */
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest, &evt->mds_context);
		} else {
			m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_OK,
							     0,
							     rsc_lock_info->lcl_lockid,
							     rsc_lock_info->lock_type,
							     rsc_lock_info->lcl_resource_id,
							     rsc_lock_info->invocation,
							     SA_LCK_LOCK_DUPLICATE_EX, rsc_lock_info->client_handle_id);

			/* send the evt to GLA */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest);
		}
		return NCSCC_RC_SUCCESS;
	}

	lck_info.lcl_lockid = rsc_lock_info->lcl_lockid;
	lck_info.call_type = rsc_lock_info->call_type;
	lck_info.handleId = rsc_lock_info->client_handle_id;
	lck_info.invocation = rsc_lock_info->invocation;
	lck_info.lock_type = rsc_lock_info->lock_type;
	lck_info.lockFlags = rsc_lock_info->lockFlags;
	lck_info.timeout = rsc_lock_info->timeout;
	lck_info.agent_mds_dest = rsc_lock_info->agent_mds_dest;
	lck_info.waiter_signal = rsc_lock_info->waiter_signal;	/* added by Ravi */
	lck_info.lcl_lockid = rsc_lock_info->lcl_lockid;

	if (res_node->status == GLND_RESOURCE_ACTIVE_MASTER) {

		/* request for the lock */
		lck_list_info =
		    glnd_resource_master_process_lock_req(glnd_cb, res_node, lck_info, glnd_cb->glnd_mdest_id,
							  rsc_lock_info->lcl_resource_id, rsc_lock_info->lcl_lockid);

		if (!lck_list_info) {
			error = SA_AIS_ERR_NO_MEMORY;
			goto err;
		}
		/* add this resource lock to the client  */
		if (lck_list_info->lock_info.lockStatus != SA_LCK_LOCK_NOT_QUEUED
		    && lck_list_info->lock_info.lockStatus != SA_LCK_LOCK_ORPHANED)
			glnd_restart_res_lock_list_ckpt_write(glnd_cb, lck_list_info,
							      lck_list_info->res_info->resource_id, 0, 2);

		switch (lck_list_info->lock_info.lockStatus) {
		case SA_LCK_LOCK_GRANTED:
		case SA_LCK_LOCK_NOT_QUEUED:
		case SA_LCK_LOCK_ORPHANED:
			/* send the grant to the gla */
			if (lck_info.call_type == GLSV_SYNC_CALL) {
				m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_OK,
								    lck_list_info->lock_info.lockid,
								    lck_list_info->lock_info.lockStatus,
								    lck_list_info->lock_info.handleId);
				/* send the evt to GLA */
				glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest,
							  &evt->mds_context);
			} else {
				m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_OK,
								     lck_list_info->lock_info.lockid,
								     rsc_lock_info->lcl_lockid,
								     lck_list_info->lock_info.lock_type,
								     rsc_lock_info->lcl_resource_id,
								     lck_list_info->lock_info.invocation,
								     lck_list_info->lock_info.lockStatus,
								     lck_list_info->lock_info.handleId);

				/* send the evt to GLA */
				glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest);
			}

			if (lck_list_info->lock_info.lockStatus == SA_LCK_LOCK_NOT_QUEUED ||
			    lck_list_info->lock_info.lockStatus == SA_LCK_LOCK_ORPHANED) {
				/* free the lck_list info */
				glnd_client_node_resource_lock_req_find_and_del(client_info, res_node->resource_id,
										lck_list_info->lock_info.lockid,
										lck_list_info->lcl_resource_id);
				glnd_resource_lock_req_delete(res_node, lck_list_info);

			} else {
				if ((lck_info.lockFlags & SA_LCK_LOCK_ORPHAN) == SA_LCK_LOCK_ORPHAN) {
					glnd_resource_lock_req_set_orphan(glnd_cb, res_node, lck_info.lock_type);
				}
			}
			break;
		default:
			/* set the mds context info */
			lck_list_info->glnd_res_lock_mds_ctxt = evt->mds_context;
			break;
		}
	} else {		/* non- master */

		lck_list_info =
		    glnd_resource_non_master_lock_req(glnd_cb, res_node, lck_info, rsc_lock_info->lcl_resource_id,
						      rsc_lock_info->lcl_lockid);
		if (!lck_list_info) {
			error = SA_AIS_ERR_NO_RESOURCES;
			goto err;
		}
		/* set the mds context info */
		lck_list_info->glnd_res_lock_mds_ctxt = evt->mds_context;

		/* add this resource lock to the client  */
		glnd_client_node_resource_lock_req_add(client_info, res_node, lck_list_info);

	}
	return NCSCC_RC_SUCCESS;

 err:
	if (rsc_lock_info->call_type == GLSV_SYNC_CALL) {
		m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, error, 0, 0, rsc_lock_info->client_handle_id);
		/* send the evt to GLA */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest, &evt->mds_context);
	} else {
		m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt,
						     error, 0,
						     rsc_lock_info->lcl_lockid,
						     rsc_lock_info->lock_type,
						     rsc_lock_info->lcl_resource_id,
						     rsc_lock_info->invocation, 0, rsc_lock_info->client_handle_id);
		/* send the evt to GLA */
		glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_lock_info->agent_mds_dest);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_resource_unlock

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_UNLOCK event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt         - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_resource_unlock(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_RSC_UNLOCK_INFO *rsc_unlock_info;
	GLND_RESOURCE_INFO *res_node;
	GLSV_LOCK_REQ_INFO lck_info;
	GLND_RES_LOCK_LIST_INFO *m_info;
	GLSV_GLA_EVT gla_evt;
	GLND_CLIENT_INFO *client_info;
	SaAisErrorT error = SA_AIS_ERR_LIBRARY;
	SaLckLockModeT lock_type = SA_LCK_PR_LOCK_MODE;
	NCS_BOOL local_orphan_lock = FALSE;

	rsc_unlock_info = (GLSV_EVT_RSC_UNLOCK_INFO *)&evt->info.rsc_unlock_info;
	memset(&lck_info, 0, sizeof(GLSV_LOCK_REQ_INFO));

	/* get the resource node */
	res_node = glnd_resource_node_find(glnd_cb, rsc_unlock_info->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		error = SA_AIS_ERR_INVALID_PARAM;
		goto err;
	}

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE || glnd_cb->gld_card_up != TRUE
	    || res_node->master_status != GLND_OPERATIONAL_STATE) {
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		if (rsc_unlock_info->call_type == GLSV_SYNC_CALL) {
			/* initialise the gla_evt */
			gla_evt.error = SA_AIS_ERR_TRY_AGAIN;
			gla_evt.handle = rsc_unlock_info->client_handle_id;
			gla_evt.type = GLSV_GLA_API_RESP_EVT;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_SYNC_UNLOCK;

			/* send the evt to GLA */
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_unlock_info->agent_mds_dest,
						  &evt->mds_context);
			return NCSCC_RC_FAILURE;
		} else {
			/* initialise the gla_evt */
			m_GLND_RESOURCE_ASYNC_LCK_UNLOCK_FILL(gla_evt, SA_AIS_ERR_TRY_AGAIN,
							      rsc_unlock_info->invocation,
							      rsc_unlock_info->lcl_resource_id,
							      rsc_unlock_info->lcl_lockid, 0);
			gla_evt.handle = rsc_unlock_info->client_handle_id;

			/* send the evt to GLA */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_unlock_info->agent_mds_dest);
			return NCSCC_RC_FAILURE;

		}
	}

	/* get the client handle */
	client_info = glnd_client_node_find(glnd_cb, rsc_unlock_info->client_handle_id);
	if (!client_info) {
		m_LOG_GLND_API(GLND_CLIENT_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	if (res_node->status == GLND_RESOURCE_NOT_INITIALISED) {
		/* sleep for relection time and resend the event */
		uns32 tm = GLSV_GLND_MASTER_REELECTION_WAIT_TIME / 10000000;
		GLSV_GLND_EVT *resend_evt = m_MMGR_ALLOC_GLND_EVT;

		memcpy(resend_evt, evt, sizeof(GLSV_GLND_EVT));
		resend_evt->next = NULL;
		m_NCS_TASK_SLEEP(tm);
		glnd_evt_local_send(glnd_cb, resend_evt, NCS_IPC_PRIORITY_NORMAL);
		return NCSCC_RC_SUCCESS;
	}

	lck_info.call_type = rsc_unlock_info->call_type;
	lck_info.handleId = rsc_unlock_info->client_handle_id;
	lck_info.invocation = rsc_unlock_info->invocation;
	lck_info.timeout = rsc_unlock_info->timeout;
	lck_info.lockid = rsc_unlock_info->lockid;
	lck_info.lcl_lockid = rsc_unlock_info->lcl_lockid;
	lck_info.agent_mds_dest = rsc_unlock_info->agent_mds_dest;

	if (res_node->status == GLND_RESOURCE_ACTIVE_MASTER) {
		m_info = glnd_resource_master_unlock_req(glnd_cb, res_node, lck_info, glnd_cb->glnd_mdest_id,
							 rsc_unlock_info->lcl_resource_id);
		if (m_info) {
			/* send back the info to the GLA */
			if (rsc_unlock_info->call_type == GLSV_SYNC_CALL) {
				memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
				gla_evt.type = GLSV_GLA_API_RESP_EVT;
				gla_evt.error = SA_AIS_OK;
				gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_SYNC_UNLOCK;
				gla_evt.info.gla_resp_info.param.sync_unlock.dummy = 0;
				/* send the evt to GLA */
				glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, m_info->lock_info.agent_mds_dest,
							  &evt->mds_context);
			} else {
				m_GLND_RESOURCE_ASYNC_LCK_UNLOCK_FILL(gla_evt, SA_AIS_OK,
								      rsc_unlock_info->invocation,
								      m_info->lcl_resource_id,
								      rsc_unlock_info->lcl_lockid,
								      m_info->lock_info.lockStatus);
				/* send the evt to GLA */
				gla_evt.handle = rsc_unlock_info->client_handle_id;
				glnd_mds_msg_send_gla(glnd_cb, &gla_evt, m_info->lock_info.agent_mds_dest);
			}
			if ((m_info->lock_info.lockFlags & SA_LCK_LOCK_ORPHAN) == SA_LCK_LOCK_ORPHAN) {
				lock_type = m_info->lock_info.lock_type;
				local_orphan_lock = TRUE;
			}
			/* delete the lock node */
			if (glnd_client_node_resource_lock_req_find_and_del(client_info,
									    res_node->resource_id,
									    m_info->lock_info.lockid,
									    m_info->lcl_resource_id) ==
			    NCSCC_RC_SUCCESS) {
				glnd_resource_lock_req_delete(res_node, m_info);
			}

			if (local_orphan_lock == TRUE)
				glnd_resource_lock_req_unset_orphan(glnd_cb, res_node, lock_type);
			/* do the re sync of the grant list */
			glnd_resource_master_lock_resync_grant_list(glnd_cb, res_node);
		} else {
			/* the lock could be in the pending mode - so search the pending list */
			m_info = glnd_resource_pending_lock_req_find(res_node, lck_info, glnd_cb->glnd_mdest_id,
								     rsc_unlock_info->lcl_resource_id);
			if (m_info) {
				/* send back the info to the GLA */
				if (rsc_unlock_info->call_type == GLSV_SYNC_CALL) {
					memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
					gla_evt.type = GLSV_GLA_API_RESP_EVT;
					gla_evt.error = SA_AIS_OK;
					gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_SYNC_UNLOCK;
					gla_evt.info.gla_resp_info.param.sync_unlock.dummy = 0;
					/* send the evt to GLA */
					glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, m_info->lock_info.agent_mds_dest,
								  &evt->mds_context);
				} else {
					m_GLND_RESOURCE_ASYNC_LCK_UNLOCK_FILL(gla_evt, SA_AIS_OK,
									      rsc_unlock_info->invocation,
									      m_info->lcl_resource_id,
									      rsc_unlock_info->lcl_lockid, 0);
					/* send the evt to GLA */
					gla_evt.handle = rsc_unlock_info->client_handle_id;
					glnd_mds_msg_send_gla(glnd_cb, &gla_evt, m_info->lock_info.agent_mds_dest);
				}
				/* cancel the pending lock */
				if (glnd_client_node_resource_lock_req_find_and_del(client_info,
										    res_node->resource_id,
										    m_info->lock_info.lockid,
										    m_info->lcl_resource_id) ==
				    NCSCC_RC_SUCCESS) {
					if (m_info->lock_info.call_type == GLSV_ASYNC_CALL
					    && m_info->lock_info.lockStatus != SA_LCK_LOCK_DEADLOCK) {
						memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
						m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_OK,
										     m_info->lock_info.lockid,
										     m_info->lock_info.lcl_lockid,
										     m_info->lock_info.lock_type,
										     m_info->lcl_resource_id,
										     m_info->lock_info.invocation,
										     m_info->lock_info.lockStatus,
										     m_info->lock_info.handleId);
						/* send the evt to GLA */

						glnd_mds_msg_send_gla(glnd_cb, &gla_evt,
								      m_info->lock_info.agent_mds_dest);
					}

					glnd_resource_lock_req_delete(res_node, m_info);
				}

			} else {
				error = SA_AIS_ERR_NOT_EXIST;
				goto err;
			}
		}
	} else {
		/* non-master stuff */
		m_info =
		    glnd_resource_non_master_unlock_req(glnd_cb, res_node, lck_info, rsc_unlock_info->lcl_resource_id,
							rsc_unlock_info->lcl_lockid);
		if (!m_info) {
			error = SA_AIS_ERR_INVALID_PARAM;
			goto err;
		} else {
			m_info->glnd_res_lock_mds_ctxt = evt->mds_context;
			m_info->unlock_req_sent = TRUE;
			m_info->unlock_call_type = lck_info.call_type;
		}
	}
	return NCSCC_RC_SUCCESS;

 err:
	if (rsc_unlock_info->call_type == GLSV_SYNC_CALL) {
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
		gla_evt.type = GLSV_GLA_API_RESP_EVT;
		gla_evt.error = error;
		gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_SYNC_UNLOCK;
		gla_evt.info.gla_resp_info.param.sync_unlock.dummy = 0;
		/* send the evt to GLA */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_unlock_info->agent_mds_dest, &evt->mds_context);
	} else {
		m_GLND_RESOURCE_ASYNC_LCK_UNLOCK_FILL(gla_evt, error,
						      rsc_unlock_info->invocation,
						      rsc_unlock_info->lcl_resource_id,
						      rsc_unlock_info->lcl_lockid, SA_LCK_LOCK_NO_MORE);
		/* send the evt to GLA */
		glnd_mds_msg_send_gla(glnd_cb, &gla_evt, rsc_unlock_info->agent_mds_dest);
	}

	return NCSCC_RC_FAILURE;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gla_resource_purge

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_PURGE event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gla_resource_purge(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_RSC_INFO *rsc_info;
	GLND_RESOURCE_INFO *res_node = NULL;
	rsc_info = (GLSV_EVT_RSC_INFO *)&evt->info.rsc_info;
	GLSV_GLA_EVT gla_evt;
	SaAisErrorT error = SA_AIS_ERR_LIBRARY;

	memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
	gla_evt.type = GLSV_GLA_API_RESP_EVT;

	/* get the resource node */
	res_node = glnd_resource_node_find(glnd_cb, rsc_info->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		error = SA_AIS_ERR_INVALID_PARAM;
		goto err;
	}

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE || glnd_cb->gld_card_up != TRUE
	    || res_node->master_status != GLND_OPERATIONAL_STATE) {
		m_LOG_GLND_API(GLND_RSC_REQ_NODE_ADD_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		/* initialise the gla_evt */
		gla_evt.error = SA_AIS_ERR_TRY_AGAIN;
		gla_evt.handle = rsc_info->client_handle_id;
		gla_evt.type = GLSV_GLA_API_RESP_EVT;
		gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_PURGE;

		/* send the evt */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);

		return NCSCC_RC_FAILURE;
	}

	if (res_node->status == GLND_RESOURCE_ACTIVE_MASTER) {
		/* clear out all the orphaned requests */
		glnd_resource_master_lock_purge_req(glnd_cb, res_node, TRUE);
	} else if (res_node->status == GLND_RESOURCE_NOT_INITIALISED) {
		/* clear out all the orphaned requests */
		glnd_resource_master_lock_purge_req(glnd_cb, res_node, TRUE);
	} else {
		/* send the request to the master node director */
		GLSV_GLND_EVT glnd_evt;
		memset(&glnd_evt, 0, sizeof(GLSV_GLND_EVT));
		glnd_evt.type = GLSV_GLND_EVT_LCK_PURGE;
		glnd_evt.info.rsc_info.resource_id = res_node->resource_id;
		if (res_node->status == GLND_RESOURCE_ELECTION_IN_PROGESS) {
			/* send it to the backup queue */
			glnd_evt_backup_queue_add(glnd_cb, &glnd_evt);
		} else
			glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, res_node->master_mds_dest);
	}

	/* initialise the gla_evt */
	gla_evt.error = SA_AIS_OK;
	gla_evt.handle = rsc_info->client_handle_id;
	gla_evt.type = GLSV_GLA_API_RESP_EVT;
	gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_PURGE;

	/* send the evt */
	glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);

	return NCSCC_RC_SUCCESS;
 err:
	gla_evt.error = error;
	gla_evt.handle = rsc_info->client_handle_id;
	gla_evt.type = GLSV_GLA_API_RESP_EVT;
	gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_PURGE;

	/* send the evt */
	glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, rsc_info->agent_mds_dest, &evt->mds_context);

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_lck_req

  DESCRIPTION    : Process the GLSV_GLND_EVT_LCK_REQ event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : got a lock request from a non-master glnd 
*****************************************************************************/
static uns32 glnd_process_glnd_lck_req(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_LCK_INFO *lck_req;
	GLND_RESOURCE_INFO *res_node;
	GLSV_LOCK_REQ_INFO lck_info;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;
	GLSV_GLND_EVT glnd_evt;
	uns32 rc;

	lck_req = (GLSV_EVT_GLND_LCK_INFO *)&evt->info.node_lck_info;
	memset(&lck_info, 0, sizeof(GLSV_LOCK_REQ_INFO));

	/* check for the resource node  */
	res_node = glnd_resource_node_find(glnd_cb, lck_req->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		goto err;
	}
	rc = glnd_process_check_master_and_non_master_status(glnd_cb, res_node);
	if (rc == SA_AIS_ERR_TRY_AGAIN) {
		/* send the status to the non-master glnd */
		m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt, GLSV_GLND_EVT_LCK_RSP,
						   res_node->resource_id, lck_req->lcl_resource_id,
						   lck_req->client_handle_id, lck_req->lockid, lck_req->lock_type,
						   lck_req->lockFlags, 0, 0, 0, SA_AIS_ERR_TRY_AGAIN,
						   lck_req->lcl_lockid, 0);

		glnd_evt.info.node_lck_info.glnd_mds_dest = glnd_cb->glnd_mdest_id;

		/* send the response evt to GLND */
		glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, lck_req->glnd_mds_dest);
	}

	if (res_node->status == GLND_RESOURCE_NOT_INITIALISED) {
		/* sleep for relection time and resend the event */
		uns32 tm = GLSV_GLND_MASTER_REELECTION_WAIT_TIME / 10000000;
		GLSV_GLND_EVT *resend_evt = m_MMGR_ALLOC_GLND_EVT;

		memcpy(resend_evt, evt, sizeof(GLSV_GLND_EVT));
		resend_evt->next = NULL;
		m_NCS_TASK_SLEEP(tm);
		glnd_evt_local_send(glnd_cb, resend_evt, NCS_IPC_PRIORITY_NORMAL);
		return NCSCC_RC_SUCCESS;

	}

	lck_info.call_type = GLSV_ASYNC_CALL;
	lck_info.handleId = lck_req->client_handle_id;
	lck_info.lock_type = lck_req->lock_type;
	lck_info.lockFlags = lck_req->lockFlags;
	lck_info.lockid = lck_req->lockid;
	lck_info.lcl_lockid = lck_req->lcl_lockid;
	lck_info.waiter_signal = lck_req->waiter_signal;

	if (res_node->status == GLND_RESOURCE_ACTIVE_MASTER) {
		/* request for the lock */
		lck_list_info =
		    glnd_resource_master_process_lock_req(glnd_cb, res_node, lck_info, lck_req->glnd_mds_dest,
							  lck_req->lcl_resource_id, lck_req->lcl_lockid);
		if (lck_list_info) {
			/* Checkpointing if the lock is granted or in waitlist */
			if (lck_list_info->lock_info.lockStatus != SA_LCK_LOCK_NOT_QUEUED
			    && lck_list_info->lock_info.lockStatus != SA_LCK_LOCK_ORPHANED)
				glnd_restart_res_lock_list_ckpt_write(glnd_cb, lck_list_info,
								      lck_list_info->res_info->resource_id, 0, 2);

			switch (lck_list_info->lock_info.lockStatus) {
			case SA_LCK_LOCK_GRANTED:
			case SA_LCK_LOCK_NOT_QUEUED:
			case SA_LCK_LOCK_ORPHANED:
				/* send the status to the non-master glnd */
				m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt, GLSV_GLND_EVT_LCK_RSP,
								   res_node->resource_id, lck_req->lcl_resource_id,
								   lck_info.handleId, lck_list_info->lock_info.lockid,
								   lck_info.lock_type, lck_info.lockFlags,
								   lck_list_info->lock_info.lockStatus, 0, 0, SA_AIS_OK,
								   lck_req->lcl_lockid, 0);

				glnd_evt.info.node_lck_info.glnd_mds_dest = glnd_cb->glnd_mdest_id;

				/* send the response evt to GLND */
				glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, lck_req->glnd_mds_dest);

				if (lck_list_info->lock_info.lockStatus == SA_LCK_LOCK_NOT_QUEUED ||
				    lck_list_info->lock_info.lockStatus == SA_LCK_LOCK_ORPHANED) {
					/* free the lck_list info */
					glnd_resource_lock_req_delete(res_node, lck_list_info);
				}
				break;
			default:
				break;
			}
		}
		return NCSCC_RC_SUCCESS;
	}

 err:
	/* send back the non-master glnd about the failure */
	m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt, GLSV_GLND_EVT_LCK_RSP,
					   lck_req->resource_id,
					   lck_req->lcl_resource_id,
					   lck_req->client_handle_id,
					   lck_req->lockid, lck_req->lock_type, lck_req->lockFlags,
					   /* shashi --- #10 under master election need to tell app to try it again */
/* chg      SA_LCK_LOCK_NO_MORE,
      0,error); */
					   0, 0, 0, SA_AIS_ERR_TRY_AGAIN, lck_req->lcl_lockid, 0);
	glnd_evt.info.node_lck_info.glnd_mds_dest = glnd_cb->glnd_mdest_id;
	/* send the response evt to GLND */
	glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, lck_req->glnd_mds_dest);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_unlck_req

  DESCRIPTION    : Process the GLSV_GLND_EVT_UNLCK_REQ event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_unlck_req(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_LCK_INFO *lck_req;
	GLND_RESOURCE_INFO *res_node;
	GLSV_LOCK_REQ_INFO lck_info;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;
	GLSV_GLND_EVT glnd_evt;
	SaAisErrorT error = SA_AIS_ERR_LIBRARY;

	lck_req = (GLSV_EVT_GLND_LCK_INFO *)&evt->info.node_lck_info;
	memset(&lck_info, 0, sizeof(GLSV_LOCK_REQ_INFO));

	/* check for the resource node  */
	res_node = glnd_resource_node_find(glnd_cb, lck_req->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_SUCCESS;

	}

	if (res_node->status == GLND_RESOURCE_NOT_INITIALISED) {
		/* sleep for relection time and resend the event */
		uns32 tm = GLSV_GLND_MASTER_REELECTION_WAIT_TIME / 10000000;
		GLSV_GLND_EVT *resend_evt = m_MMGR_ALLOC_GLND_EVT;

		memcpy(resend_evt, evt, sizeof(GLSV_GLND_EVT));
		resend_evt->next = NULL;
		m_NCS_TASK_SLEEP(tm);
		glnd_evt_local_send(glnd_cb, resend_evt, NCS_IPC_PRIORITY_NORMAL);
		return NCSCC_RC_SUCCESS;
	}

	lck_info.call_type = GLSV_ASYNC_CALL;
	lck_info.handleId = lck_req->client_handle_id;
	lck_info.lockid = lck_req->lockid;
	lck_info.lcl_lockid = lck_req->lcl_lockid;

	/* request for the unlock */
	lck_list_info =
	    glnd_resource_master_unlock_req(glnd_cb, res_node, lck_info, lck_req->glnd_mds_dest,
					    lck_req->lcl_resource_id);

	if (lck_list_info) {

		/* send the status to the non-mater glnd */
		m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt, GLSV_GLND_EVT_UNLCK_RSP,
						   res_node->resource_id,
						   lck_req->lcl_resource_id,
						   lck_list_info->lock_info.handleId,
						   lck_list_info->lock_info.lockid,
						   lck_list_info->lock_info.lock_type,
						   lck_list_info->lock_info.lockFlags,
						   lck_list_info->lock_info.lockStatus,
						   0, 0, SA_AIS_OK, lck_req->lcl_lockid, lck_req->invocation);

		glnd_evt.info.node_lck_info.glnd_mds_dest = glnd_cb->glnd_mdest_id;
		/* send the response evt to GLND */
		glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, lck_list_info->req_mdest_id);

		/* free the lck_list info */
		glnd_resource_lock_req_delete(res_node, lck_list_info);

		/* do the re sync of the grant list */
		glnd_resource_master_lock_resync_grant_list(glnd_cb, res_node);

		return NCSCC_RC_SUCCESS;
	} else {
		/* could be a pending lock so delete it */
		lck_list_info = glnd_resource_pending_lock_req_find(res_node, lck_info,
								    lck_req->glnd_mds_dest, lck_req->lcl_resource_id);

		if (lck_list_info) {
			/* send the status to the non-mater glnd */
			m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt, GLSV_GLND_EVT_UNLCK_RSP,
							   res_node->resource_id,
							   lck_req->lcl_resource_id,
							   lck_list_info->lock_info.handleId,
							   lck_list_info->lock_info.lockid,
							   lck_list_info->lock_info.lock_type,
							   lck_list_info->lock_info.lockFlags,
							   0,
							   0, 0, SA_AIS_OK, lck_req->lcl_lockid, lck_req->invocation);

			glnd_evt.info.node_lck_info.glnd_mds_dest = glnd_cb->glnd_mdest_id;
			/* send the response evt to GLND */
			glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, lck_list_info->req_mdest_id);

			/* delete the pending lock */
			glnd_resource_lock_req_delete(res_node, lck_list_info);
		}
	}

/*   err: */
	/* send back the non-master glnd about the failure */
	m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt, GLSV_GLND_EVT_UNLCK_RSP,
					   lck_req->resource_id,
					   lck_req->lcl_resource_id,
					   lck_req->client_handle_id,
					   lck_req->lockid,
					   0, 0, SA_LCK_LOCK_NO_MORE, 0, 0, error, lck_req->lcl_lockid, 0);

	glnd_evt.info.node_lck_info.glnd_mds_dest = glnd_cb->glnd_mdest_id;
	/* send the response evt to GLND */
	glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, lck_req->glnd_mds_dest);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_lck_rsp

  DESCRIPTION    : Process the GLSV_GLND_EVT_LCK_RSP event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : Got the response for the lock request sent
*****************************************************************************/
static uns32 glnd_process_glnd_lck_rsp(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_LCK_INFO *lck_rsp;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;
	GLND_RESOURCE_INFO *res_node;
	GLSV_GLA_EVT gla_evt;
	GLND_CLIENT_INFO *client_info;
	SaAisErrorT return_val = SA_AIS_ERR_TRY_AGAIN;

	lck_rsp = (GLSV_EVT_GLND_LCK_INFO *)&evt->info.node_lck_info;

	/* check for the resource node */
	res_node = glnd_resource_node_find(glnd_cb, lck_rsp->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	/* get the lock node */
	lck_list_info =
	    glnd_resource_local_lock_req_find(res_node, lck_rsp->lcl_lockid, lck_rsp->client_handle_id,
					      lck_rsp->lcl_resource_id);
	if (!lck_list_info) {
		m_LOG_GLND_API(GLND_RSC_LOCAL_LOCK_REQ_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	if (lck_list_info->lock_info.lockStatus == lck_rsp->lockStatus && lck_rsp->lockStatus == SA_LCK_LOCK_GRANTED) {
		return NCSCC_RC_SUCCESS;
	}

	/* get the client handle */
	client_info = glnd_client_node_find(glnd_cb, lck_list_info->lock_info.handleId);

	if (lck_rsp->error == SA_AIS_ERR_TRY_AGAIN) {
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		/* send back the message to GLA */
		if (lck_list_info->lock_info.call_type == GLSV_SYNC_CALL) {
			m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_ERR_TRY_AGAIN,
							    lck_list_info->lock_info.lockid,
							    lck_list_info->lock_info.lockStatus,
							    lck_list_info->lock_info.handleId);
			/* send the evt to GLA */
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest,
						  &lck_list_info->glnd_res_lock_mds_ctxt);
		} else {
			m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_ERR_TRY_AGAIN,
							     lck_list_info->lock_info.lockid,
							     lck_list_info->lock_info.lcl_lockid,
							     lck_list_info->lock_info.lock_type,
							     lck_list_info->lcl_resource_id,
							     lck_list_info->lock_info.invocation,
							     lck_list_info->lock_info.lockStatus,
							     lck_list_info->lock_info.handleId);

			/* send the evt to GLA */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest);
		}

		/* delete the request from the client wait list */
		if (glnd_client_node_resource_lock_req_find_and_del(client_info,
								    res_node->resource_id,
								    lck_list_info->lock_info.lockid,
								    lck_list_info->lcl_resource_id) ==
		    NCSCC_RC_SUCCESS) {
			/* destroy the lock req */
			glnd_resource_lock_req_delete(res_node, lck_list_info);
		}
		return NCSCC_RC_SUCCESS;

	}

	if (lck_rsp->lockStatus == SA_LCK_LOCK_GRANTED || lck_rsp->lockStatus == SA_LCK_LOCK_NOT_QUEUED
	    || lck_rsp->lockStatus == SA_LCK_LOCK_ORPHANED) {
		lck_list_info->lock_info.lockStatus = lck_rsp->lockStatus;
		return_val = SA_AIS_OK;
	}

	/* cancel the timer */
	glnd_stop_tmr(&lck_list_info->timeout_tmr);

	/* send back the message to GLA */
	if (lck_list_info->lock_info.call_type == GLSV_SYNC_CALL) {
		m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, return_val,
						    lck_list_info->lock_info.lockid,
						    lck_list_info->lock_info.lockStatus,
						    lck_list_info->lock_info.handleId);
		/* send the evt to GLA */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest,
					  &lck_list_info->glnd_res_lock_mds_ctxt);
	} else {
		m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, return_val,
						     lck_list_info->lock_info.lockid,
						     lck_list_info->lock_info.lcl_lockid,
						     lck_list_info->lock_info.lock_type,
						     lck_list_info->lcl_resource_id,
						     lck_list_info->lock_info.invocation,
						     lck_list_info->lock_info.lockStatus,
						     lck_list_info->lock_info.handleId);

		/* send the evt to GLA */
		glnd_mds_msg_send_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest);
	}

	if (lck_rsp->lockStatus == SA_LCK_LOCK_GRANTED)
		glnd_restart_res_lock_list_ckpt_overwrite(glnd_cb, lck_list_info, lck_list_info->res_info->resource_id,
							  0, 1);

	/* destroy the request */
	if (lck_rsp->lockStatus != SA_LCK_LOCK_GRANTED) {
		/* delete the request from the client wait list */
		if (glnd_client_node_resource_lock_req_find_and_del(client_info,
								    res_node->resource_id,
								    lck_list_info->lock_info.lockid,
								    lck_list_info->lcl_resource_id) ==
		    NCSCC_RC_SUCCESS) {
			/* destroy the lock req */
			glnd_resource_lock_req_delete(res_node, lck_list_info);
		}

	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_unlck_rsp

  DESCRIPTION    : Process the GLSV_GLND_EVT_UNLCK_RSP event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_unlck_rsp(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_LCK_INFO *lck_rsp;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;
	GLND_RESOURCE_INFO *res_node;
	GLSV_GLA_EVT gla_evt;
	GLND_CLIENT_INFO *client_info;
	lck_rsp = (GLSV_EVT_GLND_LCK_INFO *)&evt->info.node_lck_info;

	/* check for the resource node */
	res_node = glnd_resource_node_find(glnd_cb, lck_rsp->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	/* get the lock node */
	lck_list_info = glnd_resource_local_lock_req_find(res_node, lck_rsp->lcl_lockid, lck_rsp->client_handle_id,
							  lck_rsp->lcl_resource_id);
	if (!lck_list_info) {
		if (res_node->lck_master_info.pr_orphaned) {
			res_node->lck_master_info.pr_orphaned = FALSE;
			return NCSCC_RC_SUCCESS;
		} else {
			m_LOG_GLND_API(GLND_RSC_LOCAL_LOCK_REQ_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
			return NCSCC_RC_FAILURE;
		}
	}

	/* lock was released */
	lck_list_info->lock_info.lockStatus = lck_rsp->lockStatus;

	/* cancel the timer */
	glnd_stop_tmr(&lck_list_info->timeout_tmr);

	/* send back the message to GLA */
	if (lck_list_info->unlock_call_type == GLSV_SYNC_CALL) {
		m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, lck_rsp->error,
						    lck_list_info->lock_info.lockid,
						    lck_list_info->lock_info.lockStatus,
						    lck_list_info->lock_info.handleId);
		/* send the evt to GLA */
		glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest,
					  &lck_list_info->glnd_res_lock_mds_ctxt);
	} else {
		/* filling the unlock response event */
		m_GLND_RESOURCE_ASYNC_LCK_UNLOCK_FILL(gla_evt, lck_rsp->error,
						      lck_rsp->invocation,
						      lck_list_info->lcl_resource_id,
						      lck_list_info->lock_info.lcl_lockid,
						      lck_list_info->lock_info.lockStatus);
		gla_evt.handle = lck_rsp->client_handle_id;

		/* send the evt to GLA */
		glnd_mds_msg_send_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest);
	}

	/* get the client handle */
	client_info = glnd_client_node_find(glnd_cb, lck_list_info->lock_info.handleId);

	/* delete the request from the client */
	if (glnd_client_node_resource_lock_req_find_and_del(client_info,
							    res_node->resource_id, lck_list_info->lock_info.lockid,
							    lck_list_info->lcl_resource_id) == NCSCC_RC_SUCCESS) {
		/* destroy the lock req */
		glnd_resource_lock_req_delete(res_node, lck_list_info);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_lck_req_cancel

  DESCRIPTION    : Process the GLSV_GLND_EVT_LCK_REQ_CANCEL event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_lck_req_cancel(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_LCK_INFO *lck_req_cancel;
	GLND_RESOURCE_INFO *res_node;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;
	lck_req_cancel = (GLSV_EVT_GLND_LCK_INFO *)&evt->info.node_lck_info;

	/* check for the resource node  */
	res_node = glnd_resource_node_find(glnd_cb, lck_req_cancel->resource_id);

	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	/* find the lock request */
	lck_list_info = glnd_resource_remote_lock_req_find(res_node, lck_req_cancel->lcl_lockid,
							   lck_req_cancel->client_handle_id,
							   lck_req_cancel->glnd_mds_dest,
							   lck_req_cancel->lcl_resource_id);

	if (lck_list_info) {
		/* free the lck_list info */
		glnd_resource_lock_req_delete(res_node, lck_list_info);

		/* do the re sync of the grant list */
		glnd_resource_master_lock_resync_grant_list(glnd_cb, res_node);
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_lck_req_orphan

  DESCRIPTION    : Process the GLSV_GLND_EVT_LCK_REQ_ORPHAN event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_lck_req_orphan(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_LCK_INFO *lck_req_orphan;
	GLND_RESOURCE_INFO *res_node;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;

	lck_req_orphan = (GLSV_EVT_GLND_LCK_INFO *)&evt->info.node_lck_info;

	/* check for the resource node  */
	res_node = glnd_resource_node_find(glnd_cb, lck_req_orphan->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	/* find the lock request */
	lck_list_info = glnd_resource_remote_lock_req_find(res_node, lck_req_orphan->lockid,
							   lck_req_orphan->client_handle_id,
							   lck_req_orphan->glnd_mds_dest,
							   lck_req_orphan->lcl_resource_id);

	if (lck_list_info) {
		if (lck_list_info->lock_info.lockStatus == SA_LCK_LOCK_GRANTED) {
			if (lck_list_info->lock_info.lockFlags & SA_LCK_LOCK_ORPHAN) {
				if (lck_list_info->lock_info.lock_type == SA_LCK_EX_LOCK_MODE)
					res_node->lck_master_info.ex_orphaned = TRUE;
				if (lck_list_info->lock_info.lock_type == SA_LCK_PR_LOCK_MODE)
					res_node->lck_master_info.pr_orphaned = TRUE;
			}
			/* free the lck_list info */
			glnd_resource_lock_req_delete(res_node, lck_list_info);

			/* do the re sync of the grant list */
			glnd_resource_master_lock_resync_grant_list(glnd_cb, res_node);

		}
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_lck_waiter_clbk

  DESCRIPTION    : Process the GLSV_GLND_EVT_LCK_WAITER_CALLBACK event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_lck_waiter_clbk(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_LCK_INFO *waiter_clbk;
	GLND_RESOURCE_INFO *res_node;
	GLSV_GLA_EVT gla_evt;
	GLND_CLIENT_INFO *client_info;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;

	waiter_clbk = (GLSV_EVT_GLND_LCK_INFO *)&evt->info.node_lck_info;

	res_node = glnd_resource_node_find(glnd_cb, waiter_clbk->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	/* get the lock node */
	lck_list_info =
	    glnd_resource_local_lock_req_find(res_node, waiter_clbk->lcl_lockid, waiter_clbk->client_handle_id,
					      waiter_clbk->lcl_resource_id);
	if (!lck_list_info) {
		m_LOG_GLND_API(GLND_RSC_LOCAL_LOCK_REQ_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	client_info = glnd_client_node_find(glnd_cb, lck_list_info->lock_info.handleId);

	if (client_info) {
		if ((client_info->cbk_reg_info & GLSV_LOCK_WAITER_CBK_REG) == GLSV_LOCK_WAITER_CBK_REG) {
			m_GLND_RESOURCE_ASYNC_LCK_WAITER_FILL(gla_evt,
							      lck_list_info->lock_info.lockid,
							      lck_list_info->lcl_resource_id,
							      lck_list_info->lock_info.invocation,
							      lck_list_info->lock_info.lock_type,
							      waiter_clbk->lock_type,
							      lck_list_info->lock_info.handleId,
							      waiter_clbk->waiter_signal,
							      lck_list_info->lock_info.lcl_lockid);

			/* send it to GLA */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest);
		}
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_lck_purge

  DESCRIPTION    : Process the GLSV_GLND_EVT_LCK_PURGE event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : Evt from the non-master to the master node for resource purge
*****************************************************************************/
static uns32 glnd_process_glnd_lck_purge(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_RSC_INFO *rsc_info;
	GLND_RESOURCE_INFO *res_node;

	rsc_info = (GLSV_EVT_RSC_INFO *)&evt->info.rsc_info;

	/* get the resource node */
	res_node = glnd_resource_node_find(glnd_cb, rsc_info->resource_id);

	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	/* clear out all the orphaned requests */
	glnd_resource_master_lock_purge_req(glnd_cb, res_node, TRUE);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_send_rsc_info

  DESCRIPTION    : Process the GLSV_GLND_EVT_SND_RSC_INFO event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : Got the lock req from non-masters cause of the new master selection.
*****************************************************************************/
static uns32 glnd_process_glnd_send_rsc_info(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_RSC_INFO *rsc_info;
	GLND_LOCK_LIST_INFO *lock_list;
	GLND_RESOURCE_INFO *res_node;

	rsc_info = (GLSV_EVT_GLND_RSC_INFO *)&evt->info.node_rsc_info;

	/* get the resource node */
	res_node = glnd_resource_node_find(glnd_cb, rsc_info->resource_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	for (lock_list = rsc_info->list_of_req; lock_list != NULL; lock_list = lock_list->next) {
		/* process each lock node info */
		glnd_resource_master_process_resend_lock_req(glnd_cb, res_node,
							     lock_list->lock_info, rsc_info->glnd_mds_dest);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_fwd_dd_probe

  DESCRIPTION    : Process the GLSV_GLND_EVT_FWD_DD_PROBE event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_fwd_dd_probe(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_DD_PROBE_INFO *dd_probe = &evt->info.dd_probe_info;
	GLND_RESOURCE_INFO *rsc_info = NULL;
	GLND_RES_LOCK_LIST_INFO *wait_list = NULL, *grant_list = NULL;
	GLSV_GLND_EVT glnd_evt;
	GLSV_GLND_DD_INFO_LIST *fwd_dd_info_list = NULL;
	GLSV_GLND_DD_INFO_LIST *dd_info_list = NULL;

	/* First check if the request exists on the blocked list */
	rsc_info = glnd_resource_node_find(glnd_cb, dd_probe->rsc_id);

	if (rsc_info == NULL) {
		/*GLSV_ADD_LOG_HERE - Deadlock detection of a non existent resource */
		return NCSCC_RC_SUCCESS;
	}

	if (rsc_info->status != GLND_RESOURCE_ACTIVE_MASTER) {
		/* The DD probe has been fwded to someone who is not the master
		   Ignore the probe */
		return NCSCC_RC_SUCCESS;
	}

	/* First make sure that the <rsc-id,hdl-id, mds-dest> is present on the blocked
	   list */
	wait_list = rsc_info->lck_master_info.wait_exclusive_list;
	while (wait_list != NULL) {
		if ((memcmp(&wait_list->req_mdest_id, &dd_probe->dest_id,
			    sizeof(MDS_DEST)) == 0) &&
		    (dd_probe->hdl_id == wait_list->lock_info.handleId) &&
		    (dd_probe->lck_id == wait_list->lock_info.lockid))
			break;
		wait_list = wait_list->next;
	}

	if (wait_list == NULL) {
		/* If it was not present on EX list, look for the req on SH list */
		wait_list = rsc_info->lck_master_info.wait_read_list;
		while (wait_list != NULL) {
			if ((memcmp(&wait_list->req_mdest_id, &dd_probe->dest_id,
				    sizeof(MDS_DEST)) == 0) &&
			    (dd_probe->hdl_id == wait_list->lock_info.handleId) &&
			    (dd_probe->lck_id == wait_list->lock_info.lockid))
				break;
			wait_list = wait_list->next;
		}
	}

	if (wait_list == NULL) {
		/* GLSV_ADD_LOG_HERE - Dropping dd probe, 
		   Could happen bcos the request moved to granted state
		   or the request was called off */
		return NCSCC_RC_SUCCESS;
	}

	/* Now Send this probe message to all the guys have been granted access to
	   this resource */
	memset(&glnd_evt, '\0', sizeof(GLSV_GLND_EVT));
	grant_list = rsc_info->lck_master_info.grant_list;
	while (grant_list != NULL) {
		GLSV_GLND_DD_INFO_LIST *tmp_dd_info_list;

		glnd_evt.type = GLSV_GLND_EVT_DD_PROBE;
		glnd_evt.info.dd_probe_info.dest_id = grant_list->req_mdest_id;
		glnd_evt.info.dd_probe_info.hdl_id = grant_list->lock_info.handleId;
		glnd_evt.info.dd_probe_info.lck_id = grant_list->lock_info.lockid;
		glnd_evt.info.dd_probe_info.rsc_id = rsc_info->resource_id;
		glnd_evt.info.dd_probe_info.lcl_rsc_id = grant_list->lcl_resource_id;

		fwd_dd_info_list = dd_probe->dd_info_list;
		if (fwd_dd_info_list == NULL)
			return NCSCC_RC_FAILURE;

		dd_info_list = glnd_evt.info.dd_probe_info.dd_info_list =
		    m_MMGR_ALLOC_GLSV_GLND_DD_INFO_LIST(sizeof(GLSV_GLND_DD_INFO_LIST), NCS_SERVICE_ID_GLND);
		if (dd_info_list == NULL) {
			/* GLSV_ADD_LOG_HERE - memory failure */
			return NCSCC_RC_FAILURE;
		} else
			memset(dd_info_list, 0, sizeof(GLSV_GLND_DD_INFO_LIST));

		while (fwd_dd_info_list != NULL) {
			dd_info_list->blck_dest_id = fwd_dd_info_list->blck_dest_id;
			dd_info_list->blck_hdl_id = fwd_dd_info_list->blck_hdl_id;
			dd_info_list->lck_id = fwd_dd_info_list->lck_id;
			dd_info_list->rsc_id = fwd_dd_info_list->rsc_id;

			if (fwd_dd_info_list->next != NULL) {
				fwd_dd_info_list = fwd_dd_info_list->next;
				dd_info_list->next =
				    m_MMGR_ALLOC_GLSV_GLND_DD_INFO_LIST(sizeof(GLSV_GLND_DD_INFO_LIST),
									NCS_SERVICE_ID_GLND);
				if (dd_info_list->next == NULL) {
					dd_info_list = glnd_evt.info.dd_probe_info.dd_info_list;
					while (dd_info_list != NULL) {
						tmp_dd_info_list = dd_info_list;
						dd_info_list = dd_info_list->next;
						m_MMGR_FREE_GLSV_GLND_DD_INFO_LIST(tmp_dd_info_list,
										   NCS_SERVICE_ID_GLND);

					}
					return NCSCC_RC_FAILURE;
				}
				dd_info_list = dd_info_list->next;
				memset(dd_info_list, 0, sizeof(GLSV_GLND_DD_INFO_LIST));

			} else
				break;
		}
		/* The probe hops along its way... */
		if (!memcmp(&glnd_cb->glnd_mdest_id, &grant_list->req_mdest_id, sizeof(MDS_DEST))) {
			GLSV_GLND_EVT *tmp_glnd_evt;

			tmp_glnd_evt = m_MMGR_ALLOC_GLND_EVT;
			if (tmp_glnd_evt == NULL) {
				m_LOG_GLND_MEMFAIL(GLND_EVT_ALLOC_FAILED, __FILE__, __LINE__);
				return NCSCC_RC_FAILURE;
			}
			memset(tmp_glnd_evt, 0, sizeof(GLSV_GLND_EVT));
			*tmp_glnd_evt = glnd_evt;
			glnd_evt_local_send(glnd_cb, tmp_glnd_evt, MDS_SEND_PRIORITY_MEDIUM);
		} else {
			if (glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, grant_list->req_mdest_id)
			    != NCSCC_RC_SUCCESS) {
				/* GLSV_ADD_LOG_HERE MDS send failed */
			}
			dd_info_list = glnd_evt.info.dd_probe_info.dd_info_list;
			while (dd_info_list) {
				tmp_dd_info_list = dd_info_list;
				dd_info_list = dd_info_list->next;
				m_MMGR_FREE_GLSV_GLND_DD_INFO_LIST(tmp_dd_info_list, NCS_SERVICE_ID_GLND);
			}
			glnd_evt.info.dd_probe_info.dd_info_list = NULL;

		}

		grant_list = grant_list->next;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_dd_probe

  DESCRIPTION    : Process the GLSV_GLND_EVT_DD_PROBE event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_dd_probe(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_DD_PROBE_INFO *dd_probe;
	NCS_BOOL granted = FALSE;
	GLND_CLIENT_INFO *client_info = NULL;
	GLND_CLIENT_LIST_RESOURCE *client_res_list = NULL;
	GLND_CLIENT_LIST_RESOURCE_LOCK_REQ *lck_req_info = NULL;
	GLSV_GLND_EVT snd_probe_evt;
	GLSV_GLND_DD_INFO_LIST *dd_info_list = NULL;
	GLSV_GLND_DD_INFO_LIST *fwd_dd_info_list = NULL;
	GLSV_GLND_DD_INFO_LIST *tmp_dd_info_list = NULL;

	dd_probe = (GLSV_EVT_GLND_DD_PROBE_INFO *)&evt->info.dd_probe_info;

	client_info = (GLND_CLIENT_INFO *)glnd_client_node_find(glnd_cb, dd_probe->hdl_id);

	if (client_info == NULL) {
		/* GLSV_ADD_LOG_HERE - Dd probe dropped */
		return NCSCC_RC_SUCCESS;
	}

	client_res_list = client_info->res_list;

	while (client_res_list != NULL) {
		if (client_res_list->rsc_info->resource_id == dd_probe->rsc_id) {
			lck_req_info = client_res_list->lck_list;
			while (lck_req_info != NULL) {
				if ((memcmp(&lck_req_info->lck_req->req_mdest_id, &dd_probe->dest_id,
					    sizeof(MDS_DEST)) == 0) &&
				    (dd_probe->hdl_id == lck_req_info->lck_req->lock_info.handleId) &&
				    (dd_probe->lck_id == lck_req_info->lck_req->lock_info.lockid)) {
					granted = TRUE;
					break;
				}
				lck_req_info = lck_req_info->next;
			}
			break;
		}
		client_res_list = client_res_list->next;
	}

	if (granted == FALSE) {
		/* GLSV_ADD_LOG_HERE - DD probe dropped */
		return NCSCC_RC_SUCCESS;
	}

	/* Now check if a deadlock is detected */
	/* Find client_info pointer here */
	if (glnd_deadlock_detect(glnd_cb, client_info, dd_probe) == TRUE) {
		/* GLSV_ADD_LOG_HERE - Deadlock has been detected or we are dropping the 
		   probe */
		return NCSCC_RC_SUCCESS;
	}
	/* We need to forward the dd probe to the master of those resources on which
	   we are blocked */
	client_res_list = client_info->res_list;
	while (client_res_list != NULL) {
		lck_req_info = client_res_list->lck_list;
		while (lck_req_info != NULL) {
			if (lck_req_info->lck_req->lock_info.lockStatus != SA_LCK_LOCK_GRANTED) {
				memset(&snd_probe_evt, 0, sizeof(GLSV_GLND_EVT));
				snd_probe_evt.type = GLSV_GLND_EVT_FWD_DD_PROBE;
				fwd_dd_info_list = evt->info.dd_probe_info.dd_info_list;
				if (fwd_dd_info_list == NULL)
					return NCSCC_RC_FAILURE;

				dd_info_list = snd_probe_evt.info.dd_probe_info.dd_info_list =
				    m_MMGR_ALLOC_GLSV_GLND_DD_INFO_LIST(sizeof(GLSV_GLND_DD_INFO_LIST),
									NCS_SERVICE_ID_GLND);
				if (dd_info_list == NULL) {
					/* GLSV_ADD_LOG_HERE - memory failure */
					return NCSCC_RC_FAILURE;
				} else
					memset(dd_info_list, 0, sizeof(GLSV_GLND_DD_INFO_LIST));

				while (fwd_dd_info_list != NULL) {
					dd_info_list->blck_dest_id = fwd_dd_info_list->blck_dest_id;
					dd_info_list->blck_hdl_id = fwd_dd_info_list->blck_hdl_id;
					dd_info_list->lck_id = fwd_dd_info_list->lck_id;
					dd_info_list->rsc_id = fwd_dd_info_list->rsc_id;

					if (fwd_dd_info_list->next != NULL) {
						fwd_dd_info_list = fwd_dd_info_list->next;
						dd_info_list->next =
						    m_MMGR_ALLOC_GLSV_GLND_DD_INFO_LIST(sizeof(GLSV_GLND_DD_INFO_LIST),
											NCS_SERVICE_ID_GLND);
						if (dd_info_list->next == NULL) {
							dd_info_list = snd_probe_evt.info.dd_probe_info.dd_info_list;
							while (dd_info_list != NULL) {
								tmp_dd_info_list = dd_info_list;
								dd_info_list = dd_info_list->next;
								m_MMGR_FREE_GLSV_GLND_DD_INFO_LIST(tmp_dd_info_list,
												   NCS_SERVICE_ID_GLND);

							}
							return NCSCC_RC_FAILURE;
						}
						dd_info_list = dd_info_list->next;
						memset(dd_info_list, 0, sizeof(GLSV_GLND_DD_INFO_LIST));

					} else
						break;
				}
				dd_info_list->next =
				    m_MMGR_ALLOC_GLSV_GLND_DD_INFO_LIST(sizeof(GLSV_GLND_DD_INFO_LIST),
									NCS_SERVICE_ID_GLND);
				if (dd_info_list->next == NULL) {
					dd_info_list = snd_probe_evt.info.dd_probe_info.dd_info_list;
					while (dd_info_list != NULL) {
						tmp_dd_info_list = dd_info_list;
						dd_info_list = dd_info_list->next;
						m_MMGR_FREE_GLSV_GLND_DD_INFO_LIST(tmp_dd_info_list,
										   NCS_SERVICE_ID_GLND);

					}
					return NCSCC_RC_FAILURE;
				}

				dd_info_list = dd_info_list->next;
				memset(dd_info_list, 0, sizeof(GLSV_GLND_DD_INFO_LIST));

				dd_info_list->rsc_id = snd_probe_evt.info.dd_probe_info.rsc_id
				    = client_res_list->rsc_info->resource_id;
				dd_info_list->blck_dest_id = snd_probe_evt.info.dd_probe_info.dest_id
				    = lck_req_info->lck_req->req_mdest_id;
				dd_info_list->blck_hdl_id = snd_probe_evt.info.dd_probe_info.hdl_id
				    = lck_req_info->lck_req->lock_info.handleId;
				dd_info_list->lck_id = snd_probe_evt.info.dd_probe_info.lck_id
				    = lck_req_info->lck_req->lock_info.lockid;

				/* The probe hops along its way... */
				if (!memcmp(&glnd_cb->glnd_mdest_id, &client_res_list->rsc_info->master_mds_dest,
					    sizeof(MDS_DEST))) {
					GLSV_GLND_EVT *tmp_glnd_evt;

					tmp_glnd_evt = m_MMGR_ALLOC_GLND_EVT;
					if (tmp_glnd_evt == NULL) {
						m_LOG_GLND_MEMFAIL(GLND_EVT_ALLOC_FAILED, __FILE__, __LINE__);
						return NCSCC_RC_FAILURE;
					}
					memset(tmp_glnd_evt, 0, sizeof(GLSV_GLND_EVT));
					*tmp_glnd_evt = snd_probe_evt;
					glnd_evt_local_send(glnd_cb, tmp_glnd_evt, MDS_SEND_PRIORITY_MEDIUM);
				} else {
					if (glnd_mds_msg_send_glnd
					    (glnd_cb, &snd_probe_evt,
					     client_res_list->rsc_info->master_mds_dest) != NCSCC_RC_SUCCESS) {
						/* GLSV_ADD_LOG_HERE MDS send failed */
					}
					dd_info_list = snd_probe_evt.info.dd_probe_info.dd_info_list;
					while (dd_info_list != NULL) {
						tmp_dd_info_list = dd_info_list;
						dd_info_list = dd_info_list->next;
						m_MMGR_FREE_GLSV_GLND_DD_INFO_LIST(tmp_dd_info_list,
										   NCS_SERVICE_ID_GLND);

					}
					snd_probe_evt.info.dd_probe_info.dd_info_list = NULL;
				}
			}

			lck_req_info = lck_req_info->next;
		}
		client_res_list = client_res_list->next;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gld_resource_details

  DESCRIPTION    : Process the GLSV_EVT_GLND_RSC_GLD_INFO event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : Director has sent the resource details for the resource open
                  request.
*****************************************************************************/
static uns32 glnd_process_gld_resource_details(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_RSC_GLD_INFO *rsc_gld_info;
	GLND_RESOURCE_REQ_LIST *res_list_node;
	GLND_RESOURCE_INFO *res_info;
	NCS_BOOL is_master = FALSE;

	rsc_gld_info = (GLSV_EVT_GLND_RSC_GLD_INFO *)&evt->info.rsc_gld_info;
	/* get the node from the list */
	res_list_node = glnd_resource_req_node_find(glnd_cb, &rsc_gld_info->rsc_name);
	if (res_list_node != NULL && (rsc_gld_info->error != SA_AIS_OK)) {
		GLSV_GLA_EVT gla_evt;
		/* populate the evt to be sent to gla */
		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		gla_evt.error = rsc_gld_info->error;
		gla_evt.handle = res_list_node->client_handle_id;
		if (res_list_node->call_type == GLSV_SYNC_CALL) {
			gla_evt.type = GLSV_GLA_API_RESP_EVT;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_OPEN;
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, res_list_node->agent_mds_dest,
						  &res_list_node->glnd_res_mds_ctxt);
		} else {
			gla_evt.type = GLSV_GLA_CALLBK_EVT;
			gla_evt.info.gla_clbk_info.callback_type = GLSV_LOCK_RES_OPEN_CBK;
			gla_evt.info.gla_clbk_info.resourceId = res_list_node->lcl_resource_id;
			gla_evt.info.gla_clbk_info.params.res_open.resourceId = rsc_gld_info->rsc_id;
			gla_evt.info.gla_clbk_info.params.res_open.invocation = res_list_node->invocation;
			gla_evt.info.gla_clbk_info.params.res_open.error = rsc_gld_info->error;
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, res_list_node->agent_mds_dest);
		}
		/* delete the list node */
		glnd_resource_req_node_del(glnd_cb, res_list_node->res_req_hdl_id);
		return NCSCC_RC_SUCCESS;
	}

	if (res_list_node) {
		do {
			if (m_GLND_IS_LOCAL_NODE(&glnd_cb->glnd_mdest_id, &rsc_gld_info->master_dest_id) == 0)
				is_master = TRUE;
			/* create the new resource node */
			res_info = glnd_resource_node_add(glnd_cb, rsc_gld_info->rsc_id, &rsc_gld_info->rsc_name,
							  is_master, rsc_gld_info->master_dest_id);
			if (res_info) {
				GLSV_GLA_EVT gla_evt;
				/* sent the ack back to the GLA */
				/* inc the local reference */
				res_info->lcl_ref_cnt++;

				if (res_info->status == GLND_RESOURCE_ACTIVE_MASTER) {
					if (rsc_gld_info->can_orphan == TRUE) {
						if (rsc_gld_info->orphan_mode == SA_LCK_EX_LOCK_MODE)
							res_info->lck_master_info.ex_orphaned = TRUE;
						else
							res_info->lck_master_info.pr_orphaned = TRUE;
					}
				}
				/* Checkpoint resource_info */
				glnd_restart_resource_info_ckpt_write(glnd_cb, res_info);

				/* populate the evt to be sent to gla */
				memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

				gla_evt.error = SA_AIS_OK;
				gla_evt.handle = res_list_node->client_handle_id;
				if (res_list_node->call_type == GLSV_SYNC_CALL) {
					gla_evt.type = GLSV_GLA_API_RESP_EVT;
					gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_OPEN;
					gla_evt.info.gla_resp_info.param.res_open.resourceId = res_info->resource_id;
					glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, res_list_node->agent_mds_dest,
								  &res_list_node->glnd_res_mds_ctxt);
				} else {
					gla_evt.type = GLSV_GLA_CALLBK_EVT;
					gla_evt.info.gla_clbk_info.callback_type = GLSV_LOCK_RES_OPEN_CBK;
					gla_evt.info.gla_clbk_info.resourceId = res_list_node->lcl_resource_id;
					gla_evt.info.gla_clbk_info.params.res_open.resourceId = res_info->resource_id;
					gla_evt.info.gla_clbk_info.params.res_open.invocation =
					    res_list_node->invocation;
					gla_evt.info.gla_clbk_info.params.res_open.error = SA_AIS_OK;
					glnd_mds_msg_send_gla(glnd_cb, &gla_evt, res_list_node->agent_mds_dest);
				}
				/* add it to the client node */
				glnd_client_node_resource_add(glnd_client_node_find
							      (glnd_cb, res_list_node->client_handle_id), res_info);

			}
			/* delete the list node */
			glnd_resource_req_node_del(glnd_cb, res_list_node->res_req_hdl_id);

			/* search for any more pending callbacks */
			res_list_node = glnd_resource_req_node_find(glnd_cb, &rsc_gld_info->rsc_name);
		} while (res_list_node);
	} else {
		GLSV_GLD_EVT gld_evt;

		/* send the timeout error message to GLD to cancel out the resource request */
		memset(&gld_evt, 0, sizeof(GLSV_GLD_EVT));
		gld_evt.evt_type = GLSV_GLD_EVT_RSC_CLOSE;
		gld_evt.info.rsc_details.rsc_id = rsc_gld_info->rsc_id;

		if (glnd_mds_msg_send_gld(glnd_cb, &gld_evt, glnd_cb->gld_mdest_id) != NCSCC_RC_SUCCESS) {
			return NCSCC_RC_FAILURE;
		}
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME :glnd_process_gld_resource_master_state 

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_NEW_MASTER event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gld_resource_master_state(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_NEW_MAST_INFO *new_master_info;
	GLND_RESOURCE_INFO *res_node;

	if (glnd_cb->node_state != GLND_OPERATIONAL_STATE)
		return NCSCC_RC_FAILURE;

	new_master_info = (GLSV_EVT_GLND_NEW_MAST_INFO *)&evt->info.rsc_gld_info;

	/* check for the resource node  */
	res_node = glnd_resource_node_find(glnd_cb, new_master_info->rsc_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_SUCCESS;
	} else {

		if (new_master_info->status == GLND_RESOURCE_ELECTION_IN_PROGESS ||
		    new_master_info->status == GLND_RESOURCE_ELECTION_COMPLETED) {
			glnd_process_gld_resource_new_master(glnd_cb, new_master_info);
			return NCSCC_RC_SUCCESS;
		} else {
			if (new_master_info->status == GLND_RESOURCE_MASTER_RESTARTED) {
				res_node->master_status = GLND_RESTART_STATE;
			} else {
				res_node->master_status = GLND_OPERATIONAL_STATE;
				res_node->master_mds_dest = new_master_info->master_dest_id;

			}
		}
		glnd_restart_resource_info_ckpt_overwrite(glnd_cb, res_node);

		return NCSCC_RC_SUCCESS;
	}
}

/*****************************************************************************
  PROCEDURE NAME :glnd_process_gld_resource_master_details

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_NEW_MASTER event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gld_resource_master_details(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_EVT_GLND_RSC_MASTER_INFO *res_master_info;
	GLND_RESOURCE_INFO *res_node = NULL;
	GLSV_GLND_RSC_MASTER_INFO_LIST *rsc_master_list = NULL;
	uns32 index = 0;

	res_master_info = (GLSV_EVT_GLND_RSC_MASTER_INFO *)&evt->info.rsc_master_info;
	rsc_master_list = res_master_info->rsc_master_list;
	glnd_cb->node_state = GLND_OPERATIONAL_STATE;

	if (rsc_master_list == NULL)
		return NCSCC_RC_FAILURE;

	for (index = 0; index < res_master_info->no_of_res;) {
		/* check for the resource node  */
		res_node = glnd_resource_node_find(glnd_cb, rsc_master_list[index].rsc_id);
		if (!res_node) {
			m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		} else {
			if (memcmp(&glnd_cb->glnd_mdest_id, &rsc_master_list[index].master_dest_id, sizeof(MDS_DEST))) {
				res_node->master_mds_dest = rsc_master_list[index].master_dest_id;
				res_node->master_status = rsc_master_list[index].master_status;

				glnd_restart_resource_info_ckpt_overwrite(glnd_cb, res_node);
			}
		}
		index++;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gld_resource_new_maste

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_NEW_MASTER event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gld_resource_new_master(GLND_CB *glnd_cb, GLSV_EVT_GLND_NEW_MAST_INFO *new_master_info)
{
	GLND_RESOURCE_INFO *res_node;

	/* check for the resource node  */
	res_node = glnd_resource_node_find(glnd_cb, new_master_info->rsc_id);
	if (!res_node) {
		m_LOG_GLND_API(GLND_RSC_NODE_FIND_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_SUCCESS;
	}

	if (m_GLND_IS_LOCAL_NODE(&glnd_cb->glnd_mdest_id, &new_master_info->master_dest_id) == 0) {
		if (new_master_info->status == GLND_RESOURCE_ELECTION_IN_PROGESS) {
			if (res_node->status == GLND_RESOURCE_ACTIVE_NON_MASTER) {
				TRACE("GLND becoming MASTER for res %d", (uns32)res_node->resource_id);
				m_LOG_GLND_HEADLINE_TI(GLND_NEW_MASTER_RSC, __FILE__, __LINE__,
						       (uns32)res_node->resource_id);

				/* convert the non master info to the master info */
				glnd_resource_convert_nonmaster_to_master(glnd_cb, res_node);

				/* change the status to active master */
				res_node->status = GLND_RESOURCE_NOT_INITIALISED;

				/* set the orphan locks if any */
				if (new_master_info->orphan == TRUE
				    && new_master_info->orphan_lck_mode == SA_LCK_PR_LOCK_MODE)
					res_node->lck_master_info.pr_orphaned = TRUE;
				else if (new_master_info->orphan == TRUE
					 && new_master_info->orphan_lck_mode == SA_LCK_EX_LOCK_MODE)
					res_node->lck_master_info.ex_orphaned = TRUE;

				/* set the master */
				res_node->master_mds_dest = new_master_info->master_dest_id;
			}
		} else {
			res_node->status = GLND_RESOURCE_ACTIVE_MASTER;
			res_node->master_status = GLND_OPERATIONAL_STATE;
			glnd_resource_master_lock_resync_grant_list(glnd_cb, res_node);
		}
	} else {
		if (new_master_info->status == GLND_RESOURCE_ELECTION_IN_PROGESS) {
			/* set status in election */
			res_node->status = GLND_RESOURCE_ELECTION_IN_PROGESS;

			TRACE("GLND electing new MASTER for res %d as Node %d",
			      (uns32)res_node->resource_id,
			      (uns32)m_NCS_NODE_ID_FROM_MDS_DEST(res_node->master_mds_dest));
			m_LOG_GLND_HEADLINE_TIL(GLND_MASTER_ELECTION_RSC, __FILE__, __LINE__,
						(uns32)res_node->resource_id,
						(uns32)m_NCS_NODE_ID_FROM_MDS_DEST(res_node->master_mds_dest));

			/* set the master */
			res_node->master_mds_dest = new_master_info->master_dest_id;
			/* resend all the nm_info */
			glnd_resource_resend_nonmaster_info_to_newmaster(glnd_cb, res_node);
		} else {
			res_node->status = GLND_RESOURCE_ACTIVE_NON_MASTER;
			res_node->master_status = GLND_OPERATIONAL_STATE;

		}

	}

	glnd_restart_resource_info_ckpt_overwrite(glnd_cb, res_node);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_tmr_resource_req_timeout

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_OPEN_TIMEOUT event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_tmr_resource_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	uns32 res_req_hdl = (uns32)evt->info.tmr.opq_hdl;
	GLSV_GLA_EVT gla_evt;
	GLND_RESOURCE_REQ_LIST *res_req_info;

	res_req_info = (GLND_RESOURCE_REQ_LIST *)ncshm_take_hdl(NCS_SERVICE_ID_GLND, res_req_hdl);
	if (res_req_info) {
		/* send the timeout error message to the gla */

		memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));

		gla_evt.error = SA_AIS_ERR_TIMEOUT;
		gla_evt.handle = res_req_info->client_handle_id;

		/* send the evt */
		if (res_req_info->call_type == GLSV_SYNC_CALL) {
			gla_evt.type = GLSV_GLA_API_RESP_EVT;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_RES_OPEN;
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, res_req_info->agent_mds_dest,
						  &res_req_info->glnd_res_mds_ctxt);
		} else {
			gla_evt.type = GLSV_GLA_CALLBK_EVT;
			gla_evt.info.gla_clbk_info.callback_type = GLSV_LOCK_RES_OPEN_CBK;
			gla_evt.info.gla_clbk_info.resourceId = res_req_info->lcl_resource_id;
			gla_evt.info.gla_clbk_info.params.res_open.invocation = res_req_info->invocation;
			gla_evt.info.gla_clbk_info.params.res_open.error = SA_AIS_ERR_TIMEOUT;

			/* send the evt */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, res_req_info->agent_mds_dest);
		}

	} else
		return NCSCC_RC_SUCCESS;

	ncshm_give_hdl(res_req_hdl);

	/* remove it from the list */
	glnd_resource_req_node_del(glnd_cb, res_req_hdl);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_tmr_agent_info_timeout

  DESCRIPTION    : 

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_tmr_agent_info_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLSV_GLD_EVT gld_evt;
	uns32 ret;

	/* change the glnd status to */
	glnd_cb->node_state = GLND_RESTART_STATE;

	/* build the database */
	ret = glnd_restart_build_database(glnd_cb);
	if (ret == NCSCC_RC_SUCCESS) {
		memset(&gld_evt, 0, sizeof(GLSV_GLD_EVT));
		/* populate the evt to be sent to gld */
		gld_evt.evt_type = GLSV_GLD_EVT_GLND_OPERATIONAL;
		gld_evt.info.glnd_mds_info.mds_dest_id = glnd_cb->glnd_mdest_id;

		glnd_mds_msg_send_gld(glnd_cb, &gld_evt, glnd_cb->gld_mdest_id);

		return NCSCC_RC_SUCCESS;
	}
	return NCSCC_RC_FAILURE;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_tmr_resource_lock_req_timeout

  DESCRIPTION    : Process the GLSV_GLND_EVT_RSC_LOCK_TIMEOUT event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_tmr_resource_lock_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	uns32 res_lock_req_hdl = (uns32)evt->info.tmr.opq_hdl;
	GLSV_GLA_EVT gla_evt;
	GLND_CLIENT_INFO *client_info = NULL;
	GLND_RES_LOCK_LIST_INFO *m_info = NULL;

	m_info = (GLND_RES_LOCK_LIST_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLND, res_lock_req_hdl);
	if (m_info) {
		m_info->lock_info.lockStatus = 0;
		if (m_info->lock_info.call_type == GLSV_SYNC_CALL) {
			m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_ERR_TIMEOUT,
							    m_info->lock_info.lockid, m_info->lock_info.lockStatus,
							    m_info->lock_info.handleId);
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, m_info->lock_info.agent_mds_dest,
						  &m_info->glnd_res_lock_mds_ctxt);
		} else {
			m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_ERR_TIMEOUT,
							     m_info->lock_info.lockid, m_info->lock_info.lcl_lockid,
							     m_info->lock_info.lock_type,
							     m_info->lcl_resource_id,
							     m_info->lock_info.invocation,
							     m_info->lock_info.lockStatus, m_info->lock_info.handleId);

			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, m_info->lock_info.agent_mds_dest);

		}
	} else
		return NCSCC_RC_SUCCESS;

	/* delete the lock from client node */
	client_info = glnd_client_node_find(glnd_cb, m_info->lock_info.handleId);
	glnd_client_node_resource_lock_req_find_and_del(client_info, m_info->res_info->resource_id,
							m_info->lock_info.lockid, m_info->lcl_resource_id);

	ncshm_give_hdl(res_lock_req_hdl);

	/* delete the lock list info */
	glnd_resource_lock_req_delete(m_info->res_info, m_info);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_tmr_nm_resource_lock_req_timeout

  DESCRIPTION    : Process the GLSV_GLND_EVT_NM_RSC_LOCK_TIMEOUT event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_tmr_nm_resource_lock_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	uns32 res_lock_req_hdl = (uns32)evt->info.tmr.opq_hdl;
	GLSV_GLA_EVT gla_evt;
	GLND_CLIENT_INFO *client_info;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;
	GLSV_GLND_EVT glnd_evt;

	lck_list_info = (GLND_RES_LOCK_LIST_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLND, res_lock_req_hdl);

	if (lck_list_info) {
		if (lck_list_info->lock_info.call_type == GLSV_SYNC_CALL) {
			m_GLND_RESOURCE_SYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_ERR_TIMEOUT,
							    lck_list_info->lock_info.lockid, 0,
							    /* lck_list_info->lock_info.lockStatus, */
							    lck_list_info->lock_info.handleId);
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest,
						  &lck_list_info->glnd_res_lock_mds_ctxt);

		} else {

			m_GLND_RESOURCE_ASYNC_LCK_GRANT_FILL(gla_evt, SA_AIS_ERR_TIMEOUT,
							     lck_list_info->lock_info.lockid,
							     lck_list_info->lock_info.lcl_lockid,
							     lck_list_info->lock_info.lock_type,
							     lck_list_info->lcl_resource_id,
							     lck_list_info->lock_info.invocation,
							     lck_list_info->lock_info.lockStatus,
							     lck_list_info->lock_info.handleId);

			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest);
		}
	} else
		return NCSCC_RC_SUCCESS;

	/* clean up the evt backup queue */
	if (glnd_evt_backup_queue_delete_lock_req
	    (glnd_cb, lck_list_info->lock_info.handleId, lck_list_info->res_info->resource_id,
	     lck_list_info->lock_info.lock_type) != NCSCC_RC_SUCCESS) {

		/* send a cancel to the master node director */
		m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt, GLSV_GLND_EVT_LCK_REQ_CANCEL,
						   lck_list_info->res_info->resource_id,
						   lck_list_info->lcl_resource_id,
						   lck_list_info->lock_info.handleId,
						   lck_list_info->lock_info.lockid,
						   lck_list_info->lock_info.lock_type,
						   lck_list_info->lock_info.lockFlags,
						   0, 0, 0, 0, lck_list_info->lock_info.lcl_lockid, 0);

		glnd_evt.info.node_lck_info.glnd_mds_dest = glnd_cb->glnd_mdest_id;

		/* send the response evt to GLND */
		if (lck_list_info->res_info->status != GLND_RESOURCE_ELECTION_IN_PROGESS)
			glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt, lck_list_info->res_info->master_mds_dest);
		else
			glnd_evt_backup_queue_add(glnd_cb, &glnd_evt);	/* send it to the backup queue */
	}

	client_info = glnd_client_node_find(glnd_cb, lck_list_info->lock_info.handleId);
	glnd_client_node_resource_lock_req_find_and_del(client_info, lck_list_info->res_info->resource_id,
							lck_list_info->lock_info.lockid,
							lck_list_info->lcl_resource_id);

	ncshm_give_hdl(res_lock_req_hdl);

	/* delete the lock list info */
	glnd_resource_lock_req_delete(lck_list_info->res_info, lck_list_info);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_tmr_nm_resource_unlock_req_timeout

  DESCRIPTION    : Process the GLSV_GLND_EVT_NM_RSC_UNLOCK_TIMEOUT event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_tmr_nm_resource_unlock_req_timeout(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	uns32 res_lock_req_hdl = (uns32)evt->info.tmr.opq_hdl;
	GLSV_GLA_EVT gla_evt;
	GLND_RES_LOCK_LIST_INFO *lck_list_info;

	lck_list_info = (GLND_RES_LOCK_LIST_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLND, res_lock_req_hdl);

	/* TBD: delete the unlock request */

	/* send the timeout to the GLA */
	if (lck_list_info) {
		if (lck_list_info->lock_info.call_type == GLSV_SYNC_CALL) {
			memset(&gla_evt, 0, sizeof(GLSV_GLA_EVT));
			gla_evt.type = GLSV_GLA_API_RESP_EVT;
			gla_evt.error = SA_AIS_ERR_TIMEOUT;
			gla_evt.info.gla_resp_info.type = GLSV_GLA_LOCK_SYNC_UNLOCK;
			gla_evt.info.gla_resp_info.param.sync_unlock.dummy = 0;
			/* send the evt to GLA */
			glnd_mds_msg_send_rsp_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest,
						  &lck_list_info->glnd_res_lock_mds_ctxt);
		} else {
			m_GLND_RESOURCE_ASYNC_LCK_UNLOCK_FILL(gla_evt, SA_AIS_ERR_TIMEOUT,
							      lck_list_info->lock_info.invocation,
							      lck_list_info->lcl_resource_id,
							      lck_list_info->lock_info.lockid,
							      lck_list_info->lock_info.lockStatus);
			/* send the evt to GLA */
			glnd_mds_msg_send_gla(glnd_cb, &gla_evt, lck_list_info->lock_info.agent_mds_dest);
		}

		ncshm_give_hdl(res_lock_req_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_gld_non_master_status

  DESCRIPTION    : Process the GLSV_GLND_EVT_GLND_NODE_DOWN event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_gld_non_master_status(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	GLND_EVT_GLND_NON_MASTER_STATUS *node_status = (GLND_EVT_GLND_NON_MASTER_STATUS *)&evt->info.non_master_info;
	GLND_RESOURCE_INFO *res_node = NULL;
	SaLckResourceIdT res_id = 0;
	GLND_RES_LOCK_LIST_INFO *lock_info = NULL, *temp_info = NULL;
	uns32 node_id;
	GLSV_GLND_EVT glnd_evt;

	node_id = m_NCS_NODE_ID_FROM_MDS_DEST(node_status->dest_id);

	/* search thru all the resource tree and if it is the master then go about orphaning 
	   the locks */
	res_node = (GLND_RESOURCE_INFO *)ncs_patricia_tree_getnext(&glnd_cb->glnd_res_tree, (uint8_t *)&res_id);
	while (res_node) {
		res_id = res_node->resource_id;
		if (res_node->status == GLND_RESOURCE_ACTIVE_MASTER) {
			/* go and find all the locks that match the going down glnd */
			for (lock_info = res_node->lck_master_info.grant_list; lock_info != NULL;) {
				temp_info = lock_info;
				lock_info = lock_info->next;
				if (node_id == m_NCS_NODE_ID_FROM_MDS_DEST(temp_info->req_mdest_id)) {
					if (node_status->status == GLND_OPERATIONAL_STATE) {
						if (temp_info->non_master_status == GLND_RESTART_STATE) {
							memset(&glnd_evt, 0, sizeof(GLSV_GLND_EVT));

							/* send notification back to non-master glnd */
							m_GLND_RESOURCE_NODE_LCK_INFO_FILL(glnd_evt,
											   GLSV_GLND_EVT_LCK_RSP,
											   res_node->resource_id,
											   temp_info->lcl_resource_id,
											   temp_info->
											   lock_info.handleId,
											   temp_info->lock_info.lockid,
											   temp_info->
											   lock_info.lock_type,
											   temp_info->
											   lock_info.lockFlags,
											   temp_info->
											   lock_info.lockStatus, 0, 0,
											   SA_AIS_OK,
											   temp_info->
											   lock_info.lcl_lockid, 0);
							glnd_evt.info.node_lck_info.glnd_mds_dest =
							    glnd_cb->glnd_mdest_id;

							/* send the response evt to GLND */
							glnd_mds_msg_send_glnd(glnd_cb, &glnd_evt,
									       node_status->dest_id);

						}

					}
					glnd_process_non_master_lck_req_status(glnd_cb, res_node, temp_info,
									       node_status);
				}
			}
			for (lock_info = res_node->lck_master_info.wait_exclusive_list; lock_info != NULL;) {
				temp_info = lock_info;
				lock_info = lock_info->next;
				if (node_id == m_NCS_NODE_ID_FROM_MDS_DEST(temp_info->req_mdest_id))
					glnd_process_non_master_lck_req_status(glnd_cb, res_node, temp_info,
									       node_status);
			}
			for (lock_info = res_node->lck_master_info.wait_read_list; lock_info != NULL;) {
				temp_info = lock_info;
				lock_info = lock_info->next;
				if (node_id == m_NCS_NODE_ID_FROM_MDS_DEST(temp_info->req_mdest_id))
					glnd_process_non_master_lck_req_status(glnd_cb, res_node, temp_info,
									       node_status);
			}
		}
		res_node = (GLND_RESOURCE_INFO *)ncs_patricia_tree_getnext(&glnd_cb->glnd_res_tree, (uint8_t *)&res_id);
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_non_master_lck_req_status

  DESCRIPTION    : 

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_non_master_lck_req_status(GLND_CB *glnd_cb, GLND_RESOURCE_INFO *res_node,
						    GLND_RES_LOCK_LIST_INFO *lock_list_info,
						    GLND_EVT_GLND_NON_MASTER_STATUS *node_status)
{
	if (node_status->status == GLND_DOWN_STATE) {
		if (lock_list_info->lock_info.lockStatus == SA_LCK_LOCK_GRANTED) {
			/* delete the lock */
			glnd_resource_lock_req_delete(res_node, lock_list_info);
			glnd_resource_master_lock_resync_grant_list(glnd_cb, res_node);
		} else
			glnd_resource_lock_req_delete(res_node, lock_list_info);
	} else {
		lock_list_info->req_mdest_id = node_status->dest_id;

		if (node_status->status == GLND_OPERATIONAL_STATE)
			lock_list_info->non_master_status = 0;
		else
			lock_list_info->non_master_status = node_status->status;

		glnd_restart_res_lock_list_ckpt_overwrite(glnd_cb, lock_list_info,
							  lock_list_info->res_info->resource_id, 0, 2);
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : glnd_process_glnd_cb_dump

  DESCRIPTION    : Process the GLSV_GLND_EVT_CB_DUMP event

  ARGUMENTS      :glnd_cb      - ptr to the GLND control block
                  evt          - ptr to the event.

  RETURNS        :NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS

  NOTES         : None
*****************************************************************************/
static uns32 glnd_process_glnd_cb_dump(GLND_CB *glnd_cb, GLSV_GLND_EVT *evt)
{
	glnd_dump_cb();
	return NCSCC_RC_SUCCESS;
}
