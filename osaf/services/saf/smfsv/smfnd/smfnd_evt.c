/*       OpenSAF 
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
 * Author(s): Ericsson AB
 *
 */

#include <alloca.h>
#include <time.h>
#include <limits.h>

#include "smfnd.h"
#include "smfsv_defs.h"
#include "smfsv_evt.h"
#include "smfnd_evt.h"

static void proc_cmd_req(smfnd_cb_t * cb, SMFSV_EVT * evt);

/****************************************************************************
 * Name          : proc_cmd_req
 *
 * Description   : Handle a command request 
 *
 * Arguments     : cb  - SMFND control block  
 *                 evt - The CMD_REQ event
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void proc_cmd_req(smfnd_cb_t * cb, SMFSV_EVT * evt)
{
	int cmd_result;
	SMFSV_EVT result_evt;
	uns32 rc;

	TRACE_ENTER2("dest %" PRIx64, evt->fr_dest);

	if ((evt->info.smfnd.event.cmd_req.cmd_len == 0) || (evt->info.smfnd.event.cmd_req.cmd == NULL)) {
		LOG_ER("SMFND received empty command");
		goto err;
	}

	TRACE("Executing command: %s", evt->info.smfnd.event.cmd_req.cmd);
	cmd_result = system(evt->info.smfnd.event.cmd_req.cmd);

	memset(&result_evt, 0, sizeof(SMFSV_EVT));
	result_evt.type = SMFSV_EVT_TYPE_SMFD;
	result_evt.info.smfd.type = SMFD_EVT_CMD_RSP;
	result_evt.info.smfd.event.cmd_rsp.result = cmd_result;

	TRACE("Command %s responded %d", evt->info.smfnd.event.cmd_req.cmd, cmd_result);

	/* Send result back to sender */
	rc = smfsv_mds_send_rsp(cb->mds_handle,
				evt->mds_ctxt,
				evt->fr_svc, evt->fr_dest, NCSMDS_SVC_ID_SMFND, cb->mds_dest, &result_evt);
	if (rc != NCSCC_RC_SUCCESS) {
		LOG_ER("proc_cmd_req: send CMD RSP FAILED\n");
	}
 err:

	TRACE_LEAVE();
}

/****************************************************************************
 * Name          : smfvs_mds_msg_bcast
 * Description   : Broadcast MDS msg. SMFND uses this when broadcasting the 
 		   cbk evt to all the agents on this node.
 * Arguments     : mds_hdl - My mds handle.
 		   from_svc - My service id.
		   to_svc - To service id.
		   evt - Event to be broadcasted.
		   priority - Priority of the msg sent.
		   scope - Scope of sent.
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS. 
 *****************************************************************************/
uns32 smfvs_mds_msg_bcast(uns32 mds_hdl,uns32 from_svc, uns32 to_svc, NCSCONTEXT evt, uns32 priority,uns32 scope)
{
	NCSMDS_INFO   info;
	uns32 rc;
	TRACE_ENTER();

	memset(&info, 0, sizeof(info));
	info.i_mds_hdl = mds_hdl;
	info.i_op = MDS_SEND;
	info.i_svc_id = from_svc;

	info.info.svc_send.i_msg = evt;
	info.info.svc_send.i_priority = priority;
	info.info.svc_send.i_sendtype = MDS_SENDTYPE_BCAST;
	info.info.svc_send.i_to_svc   = to_svc;
	info.info.svc_send.info.bcast.i_bcast_scope = scope; 
	
	rc = ncsmds_api(&info);
	
	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
 * Name          : smfnd_cbk_req_proc
 * Description   : This function is called when SMFND receives SMFND_EVT_CBK_RSP 
 		   from SMFD. If there is no agent registered in this node, then
		   return OK to SMFD immediately. Otherwise bcast the cbk evt
		   to all the agents.
 * Arguments     : cb  - SMFND control block.
 		   evt - Received from SMFD.
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS. 
 *****************************************************************************/
uns32 smfnd_cbk_req_proc(smfnd_cb_t * cb, SMFSV_EVT *evt)
{
	SMFND_SMFA_ADEST_INVID_MAP *inv_node;
	SMFSV_EVT cbk_resp_evt;
	uns32 rc = NCSCC_RC_SUCCESS;
	TRACE_ENTER();

	/* If there are no agents in this node, then resp OK to SMFD now.*/
	if (!cb->agent_cnt){
		TRACE_2("SMFND: No agents on this ND. Send OK resp to SMFD.");
		memset(&cbk_resp_evt,0,sizeof(SMFSV_EVT));
		cbk_resp_evt.type = SMFSV_EVT_TYPE_SMFD;
		/*resp_evt.info.smfd.type = SMFA_EV:sT_RSP;*/ /* chnaging to below line */
		cbk_resp_evt.info.smfd.type = SMFD_EVT_CBK_RSP;/* chnaging to below line */
		cbk_resp_evt.info.smfd.event.cbk_rsp.evt_type = SMF_RSP_EVT;
		cbk_resp_evt.info.smfd.event.cbk_rsp.evt.resp_evt.inv_id = 
		evt->info.smfnd.event.cbk_req_rsp.evt.cbk_evt.inv_id;
		
		cbk_resp_evt.info.smfd.event.cbk_rsp.evt.resp_evt.err = SA_AIS_OK;

		rc = smfsv_mds_msg_send(cb->mds_handle,NCSMDS_SVC_ID_SMFD,cb->smfd_dest,
		NCSMDS_SVC_ID_SMFND,&cbk_resp_evt);
		TRACE_LEAVE();
		return rc;
	}

	/* Broadcast the cbk evt to all the agents on this node.*/
	cbk_resp_evt.type = SMFSV_EVT_TYPE_SMFA;	
	cbk_resp_evt.info.smfa.type = SMFA_EVT_CBK;
	cbk_resp_evt.info.smfa.event.cbk_req_rsp = evt->info.smfnd.event.cbk_req_rsp;
	cbk_resp_evt.info.smfa.event.cbk_req_rsp.evt_type = SMF_CLBK_EVT;

	if (NCSCC_RC_SUCCESS != smfvs_mds_msg_bcast(cb->mds_handle,NCSMDS_SVC_ID_SMFND,NCSMDS_SVC_ID_SMFA,
	&cbk_resp_evt,MDS_SEND_PRIORITY_MEDIUM,NCSMDS_SCOPE_INTRANODE)){
		LOG_ER("SMFND: Cbk broadcast FAILED.");
		return NCSCC_RC_FAILURE;
	}
	
	TRACE_2("SMFND: CBK msg is sent to %d no of agents.",cb->agent_cnt);

	inv_node = (SMFND_SMFA_ADEST_INVID_MAP *)calloc(1, sizeof(SMFND_SMFA_ADEST_INVID_MAP));
	if (NULL == inv_node){
		LOG_ER("SMFND: calloc failed. Err: %s",strerror(errno));
		assert(1);
	}
	inv_node->inv_id = evt->info.smfnd.event.cbk_req_rsp.evt.resp_evt.inv_id;
	inv_node->no_of_cbk = cb->agent_cnt;
	
	/* Add the inv node to the cb->cbk_list at the head.*/
	inv_node->next_invid = cb->cbk_list;
	cb->cbk_list = inv_node;

	TRACE_LEAVE();
	return rc;
}
/****************************************************************************
 * Name          : smfnd_cbk_resp_err_proc
 * Description   : Process the err response from the agent. As the resp is
 		   is not ok, clean up the database related to the inv_id
		   and send the err resp to SMFD.
 * Arguments     : cb  - SMFND control block.
 		   inv_id - invocation id.
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS. 
 *****************************************************************************/
uns32 smfnd_cbk_resp_err_proc(smfnd_cb_t *cb, SaInvocationT inv_id)
{
	SMFND_SMFA_ADEST_INVID_MAP *inv_node = cb->cbk_list;
	SMFND_SMFA_ADEST_INVID_MAP *prev_inv = inv_node;
	uns32 rc = NCSCC_RC_SUCCESS;
	SMFSV_EVT resp_evt;

	TRACE_ENTER2("SMFND: Received ERR resp for the inv_id: %llu.",inv_id);
	while (inv_node){
		if (inv_id == inv_node->inv_id){
			if (prev_inv->inv_id == inv_node->inv_id){
				/* Head node is deleted.*/
				cb->cbk_list = inv_node->next_invid;
			}else{
				prev_inv->next_invid = inv_node->next_invid;
			}
			free(inv_node);
			/* Send resp to SMFD.*/
			{
				memset(&resp_evt,0,sizeof(SMFSV_EVT));
				resp_evt.type = SMFSV_EVT_TYPE_SMFD;
				resp_evt.info.smfd.type = SMFD_EVT_CBK_RSP;
				resp_evt.info.smfd.event.cbk_rsp.evt_type = SMF_RSP_EVT;
				resp_evt.info.smfd.event.cbk_rsp.evt.resp_evt.inv_id = inv_id;
				resp_evt.info.smfd.event.cbk_rsp.evt.resp_evt.err = SA_AIS_ERR_FAILED_OPERATION;

				rc = smfsv_mds_msg_send(cb->mds_handle,NCSMDS_SVC_ID_SMFD,cb->smfd_dest,NCSMDS_SVC_ID_SMFND,
				&resp_evt);
			}
			break;
		}
		prev_inv = inv_node;
		inv_node = inv_node->next_invid;
	}
	TRACE_LEAVE();
	return rc;
}
/****************************************************************************
 * Name          : smfnd_cbk_resp_ok_proc
 * Description   : Process the ok response from the agent. If this is the
 		   last response SMFND is waiting, then send the resp to 
		   SMFD. Otherwise wait for the other agents to respond.
 * Arguments     : cb  - SMFND control block.
 		   inv_id - invocation id.
		   resp - resp sent by agent.
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS. 
 *****************************************************************************/
uns32 smfnd_cbk_resp_ok_proc(smfnd_cb_t *cb, SaInvocationT inv_id, uns32 resp)
{
	SMFND_SMFA_ADEST_INVID_MAP *inv_node = cb->cbk_list;
	SMFND_SMFA_ADEST_INVID_MAP *prev_inv = inv_node;
	uns32 rc = NCSCC_RC_SUCCESS;
	SMFSV_EVT resp_evt;
	
	TRACE_ENTER2("SMFND: Received OK resp for the ind_id: %llu.",inv_id);
	while (inv_node){
		if (inv_id == inv_node->inv_id){
			inv_node->no_of_cbk--;
			/* Last resp.*/
			if (0 == inv_node->no_of_cbk){
				if (prev_inv->inv_id == inv_node->inv_id){
					/* Head node is deleted.*/
					cb->cbk_list = inv_node->next_invid;
				}else{
					prev_inv->next_invid = inv_node->next_invid;
				}
				free(inv_node);
				{
					memset(&resp_evt,0,sizeof(SMFSV_EVT));
					resp_evt.type = SMFSV_EVT_TYPE_SMFD;
					resp_evt.info.smfd.type = SMFD_EVT_CBK_RSP;
					resp_evt.info.smfd.event.cbk_rsp.evt_type = SMF_RSP_EVT;
					resp_evt.info.smfd.event.cbk_rsp.evt.resp_evt.inv_id = inv_id;
					resp_evt.info.smfd.event.cbk_rsp.evt.resp_evt.err = resp;

					rc = smfsv_mds_msg_send(cb->mds_handle,NCSMDS_SVC_ID_SMFD,cb->smfd_dest,
					NCSMDS_SVC_ID_SMFND, &resp_evt);
				}
				/* Send resp to SMFD and break.*/
				break;
			}
			/* More responses expected.*/
			break;
		}
		prev_inv = inv_node;
		inv_node = inv_node->next_invid;
	}
	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
 * Name          : smfnd_cbk_resp_proc
 * Description   : Process the cbk response from the agent. 
 * Arguments     : cb  - SMFND control block  
 *                 evt - The CBK_RESP event received from agent. 
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS. 
 *****************************************************************************/
uns32 smfnd_cbk_resp_proc(smfnd_cb_t *cb, SMFSV_EVT *evt)
{
	uns32 rc;
	TRACE_ENTER();

	if (SA_AIS_ERR_FAILED_OPERATION == evt->info.smfnd.event.cbk_req_rsp.evt.resp_evt.err){
		rc = smfnd_cbk_resp_err_proc(cb,evt->info.smfnd.event.cbk_req_rsp.evt.resp_evt.inv_id);
	}else{
		/* Any response other than FAILED_OPERATION is considered as OK.*/
		rc = smfnd_cbk_resp_ok_proc(cb,evt->info.smfnd.event.cbk_req_rsp.evt.resp_evt.inv_id,
		evt->info.smfnd.event.cbk_req_rsp.evt.resp_evt.err); 
	}
	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
 * Name          : proc_cmd_req
 *
 * Description   : Handle a command request 
 *
 * Arguments     : cb  - SMFND control block  
 *                 evt - The CMD_REQ event
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void proc_cbk_req_rsp(smfnd_cb_t * cb, SMFSV_EVT * evt)
{
	TRACE_ENTER();
	switch (evt->info.smfnd.event.cbk_req_rsp.evt_type) {
		case SMF_CLBK_EVT:
		{
			smfnd_cbk_req_proc(cb, evt);
			break;
		}
		case SMF_RSP_EVT:
		{
			smfnd_cbk_resp_proc(cb, evt);
			break;
		}
		default:
		{
			LOG_ER("SMFND received unknown event %d", evt->info.smfnd.event.cbk_req_rsp.evt_type);
			break;
		}
	}
	TRACE_LEAVE();
}

/****************************************************************************
 * Name          : smfnd_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 SMFND 
 *
 * Arguments     : mbx  - This is the mail box pointer on which SMFND is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void smfnd_process_mbx(SYSF_MBX * mbx)
{
	SMFSV_EVT *evt = (SMFSV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt);

	if (evt != NULL) {
		if (evt->type != SMFSV_EVT_TYPE_SMFND) {
			LOG_ER("SMFND received wrong event type %d", evt->type);
			goto done;
		}

		switch (evt->info.smfnd.type) {
		case SMFND_EVT_CMD_REQ:
			{
				proc_cmd_req(smfnd_cb, evt);
				break;
			}
		case SMFND_EVT_CBK_RSP:
			{
				proc_cbk_req_rsp(smfnd_cb, evt);
				break;
			}
		default:
			{
				LOG_ER("SMFND received unknown event %d", evt->info.smfnd.type);
				break;
			}
		}
	}

 done:
	smfsv_evt_destroy(evt);
}
