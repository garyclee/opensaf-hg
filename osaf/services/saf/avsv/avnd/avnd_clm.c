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

  This file contains cluster membership routines.
 
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avnd.h"
#include "mds_pvt.h"
#include "nid_api.h"

/****************************************************************************
  Name          : avnd_evt_cla_finalize
 
  Description   : 
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_cla_finalize(AVND_CB *cb, AVND_EVT *evt)
{
	SaClmHandleT hdl = 0;
	AVND_CLA_EVT cla_evt;
	AVSV_CLM_API_INFO *info;
	AVND_CLM_TRK_INFO *trk_info;
	AVSV_NDA_CLA_MSG *msg;

	cla_evt = evt->info.cla;
	msg = cla_evt.msg;

	/* compare the type */
	if (msg->type != AVSV_CLA_API_MSG)
		return NCSCC_RC_FAILURE;

	info = &msg->info.api_info;

	/* compare the type */
	if (info->type != AVSV_CLM_FINALIZE)
		return NCSCC_RC_FAILURE;

	hdl = info->param.finalize.hdl;

	trk_info = avnd_clm_trkinfo_list_del(cb, hdl, &cla_evt.mds_dest);

	/* Free the node */
	if (trk_info)
		m_MMGR_FREE_AVND_CLM_TRK_INFO(trk_info);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avnd_evt_cla_track_start
 
  Description   : 
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_cla_track_start(AVND_CB *cb, AVND_EVT *evt)
{
	AVND_CLA_EVT cla_evt;
	AVSV_CLM_API_INFO *api_info;
	AVND_CLM_TRK_INFO *trk_info;
	uns32 rc = NCSCC_RC_SUCCESS;

	cla_evt = evt->info.cla;

	/* compare the type */
	if (cla_evt.msg->type != AVSV_CLA_API_MSG)
		return NCSCC_RC_FAILURE;

	api_info = &cla_evt.msg->info.api_info;

	/* compare the type */
	if (api_info->type != AVSV_CLM_TRACK_START)
		return NCSCC_RC_FAILURE;

	trk_info = avnd_clm_trkinfo_list_find(cb, api_info->param.track_start.hdl, &cla_evt.mds_dest);
	if (trk_info) {
		/* Entry found. Adjust the flags and check if a response need to 
		 * be sent right away. 
		 */
		trk_info->track_flag = api_info->param.track_start.flags;
		goto mds_send_check;
	}

	/* The Entry doesnt exist. Create and Add to the list */
	/* alloc the memory, initialize/fill the node and add to the list */
	trk_info = m_MMGR_ALLOC_AVND_CLM_TRK_INFO;
	if (trk_info) {
		memset(trk_info, 0, sizeof(AVND_CLM_TRK_INFO));

		trk_info->req_hdl = api_info->param.track_start.hdl;
		trk_info->track_flag = api_info->param.track_start.flags;
		trk_info->mds_dest = cla_evt.mds_dest;

		rc = avnd_clm_trkinfo_list_add(cb, trk_info);
	} else {
		/* Log an error */
		return NCSCC_RC_FAILURE;
	}

 mds_send_check:
	if (m_AVND_CLM_IS_TRACK_CURRENT(trk_info)) {
		/* send the entire list of nodes to that mds_dest */
		avnd_clm_track_current_resp(cb, trk_info, &evt->mds_ctxt, api_info->is_sync_api);
	}
	return rc;
}

/****************************************************************************
  Name          : avnd_evt_cla_track_stop
 
  Description   : 
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_cla_track_stop(AVND_CB *cb, AVND_EVT *evt)
{
	AVND_CLA_EVT cla_evt;
	AVSV_CLM_API_INFO *api_info;
	AVND_CLM_TRK_INFO *trk_info;
	AVND_MSG msg;
	AVSV_NDA_CLA_MSG *cla_msg = 0;

	cla_msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG;
	if (!cla_msg) {
		/* Log an error . This is low memory situation need not continue */
		return NCSCC_RC_FAILURE;
	}
	memset(cla_msg, 0, sizeof(AVSV_NDA_CLA_MSG));

	cla_evt = evt->info.cla;

	/* compare the type */
	if (cla_evt.msg->type != AVSV_CLA_API_MSG)
		return NCSCC_RC_FAILURE;

	api_info = &cla_evt.msg->info.api_info;

	/* compare the type */
	if (api_info->type != AVSV_CLM_TRACK_STOP)
		return NCSCC_RC_FAILURE;

	trk_info = avnd_clm_trkinfo_list_find(cb, api_info->param.track_start.hdl, &cla_evt.mds_dest);
	if ((trk_info) && (m_AVND_CLM_IS_TRACK_CHANGES(trk_info) || m_AVND_CLM_IS_TRACK_CHANGES_ONLY(trk_info))
	    ) {
		/* Entry found. Adjust the flags and return */
		trk_info->track_flag = 0;

		cla_msg->type = AVSV_AVND_CLM_API_RESP_MSG;
		cla_msg->info.api_resp_info.type = AVSV_CLM_TRACK_STOP;
		cla_msg->info.api_resp_info.rc = SA_AIS_OK;
	} else
	{
		cla_msg->type = AVSV_AVND_CLM_API_RESP_MSG;
		cla_msg->info.api_resp_info.type = AVSV_CLM_TRACK_STOP;
		cla_msg->info.api_resp_info.rc = SA_AIS_ERR_NOT_EXIST;
	}

   /*** send the resp message to CLA ***/
	memset(&msg, 0, sizeof(AVND_MSG));
	msg.type = AVND_MSG_CLA;
	msg.info.cla = cla_msg;

	return avnd_mds_send(cb, &msg, &evt->info.cla.mds_dest, &evt->mds_ctxt);
}

/****************************************************************************
  Name          : avnd_evt_cla_node_get
 
  Description   : 
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_cla_node_get(AVND_CB *cb, AVND_EVT *evt)
{
	AVND_CLA_EVT cla_evt;
	AVSV_CLM_API_INFO *api_info;
	AVND_CLM_REC *rec = NULL;
	SaClmNodeIdT node_id;
	SaClmClusterNodeT clusterNode;
	AVND_MSG msg;
	AVSV_NDA_CLA_MSG *cla_msg = 0;

	cla_msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG;
	if (!cla_msg) {
		/* Log an error . This is low memory situation need not continue */
		return NCSCC_RC_FAILURE;
	}
	memset(cla_msg, 0, sizeof(AVSV_NDA_CLA_MSG));

	cla_evt = evt->info.cla;

	/* compare the type */
	if (cla_evt.msg->type != AVSV_CLA_API_MSG)
		return NCSCC_RC_FAILURE;

	api_info = &cla_evt.msg->info.api_info;

	/* compare the type */
	if (api_info->type != AVSV_CLM_NODE_GET)
		return NCSCC_RC_FAILURE;

	node_id = api_info->param.node_get.node_id;

	if (node_id == SA_CLM_LOCAL_NODE_ID)
		node_id = m_NCS_NODE_ID_FROM_MDS_DEST(cb->avnd_dest);

	if (node_id)
		rec = avnd_clmdb_rec_get(cb, node_id);

	if (rec) {
		clusterNode.nodeId = rec->info.node_id;
		clusterNode.nodeAddress = rec->info.node_address;
		clusterNode.nodeName = rec->info.node_name_net;
		clusterNode.nodeName.length = m_NCS_OS_NTOHS(rec->info.node_name_net.length);
		clusterNode.member = rec->info.member;
		clusterNode.bootTimestamp = rec->info.boot_timestamp;
		clusterNode.initialViewNumber = rec->info.view_number;

		/* stick the notification buffer into the message */
		/* Fill the cla msg and make sure everything is linked */
		cla_msg->type = AVSV_AVND_CLM_API_RESP_MSG;
		cla_msg->info.api_resp_info.type = AVSV_CLM_NODE_GET;
		cla_msg->info.api_resp_info.rc = SA_AIS_OK;
		cla_msg->info.api_resp_info.param.node_get = clusterNode;
	} else {
		cla_msg->type = AVSV_AVND_CLM_API_RESP_MSG;
		cla_msg->info.api_resp_info.type = AVSV_CLM_NODE_GET;
		cla_msg->info.api_resp_info.rc = SA_AIS_ERR_INVALID_PARAM;
	}

   /*** send the callback message to CLA ***/
	memset(&msg, 0, sizeof(AVND_MSG));
	msg.type = AVND_MSG_CLA;
	msg.info.cla = cla_msg;

	return avnd_mds_send(cb, &msg, &evt->info.cla.mds_dest, &evt->mds_ctxt);
}

/****************************************************************************
  Name          : avnd_evt_cla_node_async_get
 
  Description   : 
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_cla_node_async_get(AVND_CB *cb, AVND_EVT *evt)
{
	AVND_CLA_EVT cla_evt;
	AVSV_CLM_API_INFO *api_info;
	AVND_CLM_REC *rec = NULL;
	SaClmNodeIdT node_id;
	SaClmClusterNodeT clusterNode;
	AVND_MSG msg;
	AVSV_NDA_CLA_MSG *cla_msg = 0;

	cla_msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG;
	if (!cla_msg) {
		/* Log an error . This is low memory situation need not continue */
		return NCSCC_RC_FAILURE;
	}
	memset(cla_msg, 0, sizeof(AVSV_NDA_CLA_MSG));

	cla_evt = evt->info.cla;

	/* compare the type */
	if (cla_evt.msg->type != AVSV_CLA_API_MSG)
		return NCSCC_RC_FAILURE;

	api_info = &cla_evt.msg->info.api_info;

	/* compare the type */
	if (api_info->type != AVSV_CLM_NODE_ASYNC_GET)
		return NCSCC_RC_FAILURE;

	node_id = api_info->param.node_async_get.node_id;

	if (node_id == SA_CLM_LOCAL_NODE_ID)
		node_id = m_NCS_NODE_ID_FROM_MDS_DEST(cb->avnd_dest);

	if (node_id)
		rec = avnd_clmdb_rec_get(cb, node_id);

	if (rec) {
		clusterNode.nodeId = rec->info.node_id;
		clusterNode.nodeAddress = rec->info.node_address;
		clusterNode.nodeName = rec->info.node_name_net;
		clusterNode.nodeName.length = m_NCS_OS_NTOHS(rec->info.node_name_net.length);
		clusterNode.member = rec->info.member;
		clusterNode.bootTimestamp = rec->info.boot_timestamp;
		clusterNode.initialViewNumber = rec->info.view_number;

		/* stick the notification buffer into the message */
		/* Fill the cla msg and make sure everything is linked */
		cla_msg->type = AVSV_AVND_CLM_CBK_MSG;
		cla_msg->info.cbk_info.hdl = api_info->param.node_async_get.hdl;
		cla_msg->info.cbk_info.type = AVSV_CLM_CBK_NODE_ASYNC_GET;
		cla_msg->info.cbk_info.param.node_get.err = SA_AIS_OK;
		cla_msg->info.cbk_info.param.node_get.node = clusterNode;
		cla_msg->info.cbk_info.param.node_get.inv = api_info->param.node_async_get.inv;
	} else {
		cla_msg->type = AVSV_AVND_CLM_CBK_MSG;
		cla_msg->info.cbk_info.hdl = api_info->param.node_async_get.hdl;
		cla_msg->info.cbk_info.type = AVSV_CLM_CBK_NODE_ASYNC_GET;
		cla_msg->info.cbk_info.param.node_get.err = SA_AIS_ERR_INVALID_PARAM;
		cla_msg->info.cbk_info.param.node_get.inv = api_info->param.node_async_get.inv;
	}

   /*** send the callback message to CLA ***/
	memset(&msg, 0, sizeof(AVND_MSG));
	msg.type = AVND_MSG_CLA;
	msg.info.cla = cla_msg;

	return avnd_mds_send(cb, &msg, &evt->info.cla.mds_dest, 0);
}

/****************************************************************************
  Name          : avnd_evt_avd_clm_node_on_fover
 
  Description   : This routine processes the node update mesage from AvD. 
                  This message contains cluster membership nodes information
                  which needs to be updated at AVND.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_clm_node_on_fover(AVND_CB *cb, AVND_EVT *evt)
{
	AVSV_D2N_CLM_NODE_ON_FOVER *info;
	AVND_CLM_REC *rec = 0, t_rec;
	AVSV_CLM_INFO_MSG *curr = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	m_AVND_LOG_FOVER_EVTS(NCSFL_SEV_NOTICE, AVND_LOG_FOVR_CLM_NODE_INFO, NCSCC_RC_SUCCESS);

	/* dont process unless AvD is up */
	if (!m_AVND_CB_IS_AVD_UP(cb))
		goto done;

	info = &evt->info.avd->msg_info.d2n_clm_node_fover;

	for (curr = info->list_of_nodes; curr; curr = curr->next) {
		rec = avnd_clmdb_rec_get(cb, curr->clm_info.node_id);
		/* if the update is for this node, dont process it */
		if (curr->clm_info.node_id == cb->clmdb.node_info.nodeId)
			continue;

		if (NULL == rec) {
			rec = avnd_clmdb_rec_add(cb, &info->list_of_nodes->clm_info);
			avnd_clm_snd_track_changes(cb, rec, SA_CLM_NODE_JOINED);
		} else {
			rec->avd_updt_flag = TRUE;
		}
	}

	/* send clm track cbk to all the processes for which tracking is on */
	rec = 0;
	for (rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_FIRST(&cb->clmdb.clm_list);
	     rec; rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->clm_dll_node)) {
		t_rec = *rec;
		if (rec->avd_updt_flag == TRUE) {
			rec->avd_updt_flag = FALSE;
			continue;
		}

		avnd_clm_snd_track_changes(cb, rec, SA_CLM_NODE_LEFT);
		rc = avnd_clmdb_rec_del(cb, rec->info.node_id);
		rec = &t_rec;
	}

	/* update the latest view number */
	cb->clmdb.curr_view_num = info->view_number;

 done:
	return rc;
}

/****************************************************************************
  Name          : avnd_evt_avd_node_update_msg
 
  Description   : This routine processes the node update mesage from AvD. 
                  This message contains cluster membership changes & is 
                  generated when a node joins / leaves the cluster.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_node_update_msg(AVND_CB *cb, AVND_EVT *evt)
{
	AVSV_D2N_CLM_NODE_UPDATE_MSG_INFO *info;
	AVND_CLM_REC *rec = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* dont process unless AvD is up */
	if (!m_AVND_CB_IS_AVD_UP(cb))
		goto done;

	info = &evt->info.avd->msg_info.d2n_clm_node_update;

	/* if the update is for this node, dont process it */
	if (info->clm_info.node_id == cb->clmdb.node_info.nodeId) {
		if (SA_FALSE == info->clm_info.member) {
			/* we got a node down for ourself, reboot this node */
			ncs_reboot("This node exited the cluster");
		}
		goto done;
	}

	/* update the latest view number */
	cb->clmdb.curr_view_num = info->clm_info.view_number;

	/* update the clm database */
	if (SA_TRUE == info->clm_info.member) {	/* => node addition */
		rec = avnd_clmdb_rec_add(cb, &info->clm_info);
		if (rec)
			avnd_clm_snd_track_changes(cb, rec, SA_CLM_NODE_JOINED);
	} else {		/* => node deletion */

		mds_node_link_reset(info->clm_info.node_id);
		rec = avnd_clmdb_rec_get(cb, info->clm_info.node_id);

/*************************   Section  1 Starts Here **************************/

/* We have a Node down message, time to check whether we have any internode
   component related to this Node on this node.*/
		if (rec) {
			AVND_COMP *comp = NULL;
			SaNameT name_net;
			AVND_COMP_PXIED_REC *pxd_rec = 0, *curr_rec = 0;
			memset(&name_net, 0, sizeof(SaNameT));

			for (comp = m_AVND_COMPDB_REC_GET_NEXT(cb->internode_avail_comp_db, name_net);
			     comp; comp = m_AVND_COMPDB_REC_GET_NEXT(cb->internode_avail_comp_db, name_net)) {
				name_net = comp->name_net;
				/* Check the node id */
				if (comp->node_id == rec->info.node_id) {
					if (m_AVND_COMP_TYPE_IS_PROXIED(comp)) {
						/* We need to delete the proxied component and remove
						   this component from the proxy's pxied_list */
						/* Remove the proxied component from pxied_list  */
						m_AVND_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp,
										 AVND_CKPT_COMP_PROXY_PROXIED_DEL);
						rc = avnd_comp_proxied_del(cb, comp, comp->pxy_comp, FALSE, NULL);
						/* Delete the proxied component */
						m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVND_CKPT_COMP_CONFIG);
						rc = avnd_internode_comp_del(cb, &(cb->internode_avail_comp_db),
									     &(comp->name_net));
					} /* if(m_AVND_COMP_TYPE_IS_PROXIED(comp)) */
					else if (m_AVND_COMP_TYPE_IS_PROXY(comp)) {
						/* We need to delete the proxy component and make
						   all its proxied component as orphan */
						/* look in each proxied for this handle */
						pxd_rec =
						    (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_FIRST(&comp->pxied_list);
						while (pxd_rec) {
							curr_rec = pxd_rec;
							pxd_rec =
							    (AVND_COMP_PXIED_REC *)m_NCS_DBLIST_FIND_NEXT(&pxd_rec->
													  comp_dll_node);
							rc = avnd_comp_unreg_prc(cb, curr_rec->pxied_comp, comp);

							if (NCSCC_RC_SUCCESS != rc) {
								m_AVND_AVND_ERR_LOG
								    ("avnd_evt_avd_node_update_msg:Unreg failed:Comp is",
								     &curr_rec->pxied_comp->name_net, 0, 0, 0, 0);
							}
						}	/* while */
					}	/* else if(m_AVND_COMP_TYPE_IS_PROXY(comp)) */
				}	/* if(comp->node_id == rec->info.node_id)  */
			}	/* for(comp = m_AVND_COMPDB_REC_GET_NEXT()  */

		}

		/* if(rec) */
 /*************************   Section  1 Ends  Here **************************/
		if (rec)
			avnd_clm_snd_track_changes(cb, rec, SA_CLM_NODE_LEFT);
		rc = avnd_clmdb_rec_del(cb, info->clm_info.node_id);
	}

 done:
	return rc;
}

/****************************************************************************
  Name          : avnd_evt_avd_node_up_msg
 
  Description   : This routine processes the node-up response message from 
                  AvD. It contains a list of current nodes in the cluster.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_avd_node_up_msg(AVND_CB *cb, AVND_EVT *evt)
{
	AVSV_D2N_CLM_NODE_UP_MSG_INFO *info;
	AVSV_CLM_INFO_MSG *curr = 0;
	AVND_CLM_REC *rec = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	info = &evt->info.avd->msg_info.d2n_clm_node_up;

   /*** update this node with the supplied parameters ***/
	cb->clmdb.type = info->node_type;
	cb->su_failover_max = info->su_failover_max;
	cb->su_failover_prob = info->su_failover_prob;
	cb->hb_intv = info->snd_hb_intvl;

	/* update the latest view number */
	cb->clmdb.curr_view_num = info->list_of_nodes->clm_info.view_number;

	/* update the clm db */
	for (curr = info->list_of_nodes; curr; curr = curr->next) {
		rec = avnd_clmdb_rec_add(cb, &curr->clm_info);
		if (!rec)
			break;

		/* update the latest view number */
		if (rec->info.node_id == cb->clmdb.node_info.nodeId) {
			cb->clmdb.curr_view_num = curr->clm_info.view_number;
			cb->clmdb.node_info.nodeName = curr->clm_info.node_name_net;
			cb->clmdb.node_info.nodeName.length = m_NCS_OS_NTOHS(cb->clmdb.node_info.nodeName.length);
		}
	}

	/* 
	 * if all the records aren't added, delete those 
	 * records that were successfully added
	 */
	if (curr) {		/* => add operation stopped in the middle */
		for (curr = info->list_of_nodes; curr; curr = curr->next)
			avnd_clmdb_rec_del(cb, curr->clm_info.node_id);
	}

	/* send clm track cbk to all the processes for which tracking is on */
	rec = 0;
	for (rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_FIRST(&cb->clmdb.clm_list);
	     rec; rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->clm_dll_node)) {
		avnd_clm_snd_track_changes(cb, rec, SA_CLM_NODE_JOINED);
	}

	/* initiate heartbeat process */
	rc = avnd_di_hb_send(cb);
	if (NCSCC_RC_SUCCESS != rc)
		goto done;

	/* set the AvD up flag */
	m_AVND_CB_AVD_UP_SET(cb);

	/* register this row with mab */
	rc = avnd_mab_reg_tbl_rows(cb, NCSMIB_TBL_AVSV_NCS_NODE_STAT, 0, 0,
				   &cb->clmdb.node_info.nodeId, &cb->mab_node_hdl, cb->mab_hdl);

 done:
	return rc;
}

/****************************************************************************
  Name          : avnd_evt_mds_cla_dn
 
  Description   : This routine processes the CLA down event from MDS. It 
                  scans the process list to determine the process in which 
                  the failed CLA resides. It then clears the CLM specific 
                  info from the process & deletes the process. It may so 
                  happen that AvA is still up in which case only CLM info is 
                  cleared & process is not deleted.
 
  Arguments     : cb  - ptr to the AvND control block
                  evt - ptr to the AvND event
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_evt_mds_cla_dn(AVND_CB *cb, AVND_EVT *evt)
{
	AVND_CLM_TRK_INFO *i_ptr = NULL, *prev_ptr = NULL;
	MDS_DEST dest = evt->info.mds.mds_dest;
	AVND_CLM_DB *db = &cb->clmdb;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (evt->type != AVND_EVT_MDS_CLA_DN)
		return NCSCC_RC_FAILURE;

	/* Get all the trk_info structures with the matching MDS_DEST and clean the 
	 *  list 
	 */

	if (!db->clm_trk_info) {
		/* Nothing to be done list Empty. */
		return NCSCC_RC_SUCCESS;
	}

	i_ptr = db->clm_trk_info;
	prev_ptr = i_ptr;
	while (i_ptr != NULL) {
		if (memcmp(&i_ptr->mds_dest, &dest, sizeof(MDS_DEST)) == 0) {
			/* found the trk_info. Now clean it. 
			 * 
			 * We dont want to call list_del function coz, that
			 * function will start at first node and loop thru to 
			 * get to this node.
			 */

			if (i_ptr == db->clm_trk_info) {
				/* first node */
				db->clm_trk_info = i_ptr->next;
				m_MMGR_FREE_AVND_CLM_TRK_INFO(i_ptr);
				i_ptr = db->clm_trk_info;
				prev_ptr = i_ptr;
				continue;
			} else {
				prev_ptr->next = i_ptr->next;
				m_MMGR_FREE_AVND_CLM_TRK_INFO(i_ptr);
				i_ptr = prev_ptr->next;
				continue;
			}
		}
		prev_ptr = i_ptr;
		i_ptr = i_ptr->next;
	}
	return rc;
}

/****************************************************************************
  Name          : avnd_clm_track_current_resp 
 
  Description   : This routine sends the CLM notify callback parameters to the
                  process that had requested for it earlier. The information 
                  sent depends on the track flags.
 
  Arguments     : cb          - ptr to the AvND control block
                  trk_info    - ptr to track info
                  ctxt        - pointer to MDS context.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : The only errors that happen in this routine are memory 
                  errors and we cant send a CBK message with error indication 
                  for such errors

******************************************************************************/
uns32 avnd_clm_track_current_resp(AVND_CB *cb,
				  AVND_CLM_TRK_INFO *trk_info, MDS_SYNC_SND_CTXT *ctxt, NCS_BOOL is_sync_api)
{
	SaClmClusterNotificationT *notify = NULL;
	AVND_MSG msg;
	AVSV_NDA_CLA_MSG *cla_msg = 0;
	uns32 num = 0;
	AVND_CLM_REC *rec = NULL;
	uns32 i = 0, rc = NCSCC_RC_SUCCESS;

	/* determine the number of items */
	num = cb->clmdb.clm_list.n_nodes;

	/* alloc the notify buffer if num is 0 dont return we can send a response 
	 * with mem_num field as 0
	 */
	if (num) {
		notify = (SaClmClusterNotificationT *)
		    m_MMGR_ALLOC_AVSV_CLA_DEFAULT_VAL(num * sizeof(SaClmClusterNotificationT));

		if (!notify) {
			rc = NCSCC_RC_FAILURE;
			goto error;
		}
	} else {
		rc = NCSCC_RC_FAILURE;
		goto error;
	}
	memset(notify, 0, num * sizeof(SaClmClusterNotificationT));

	/* Fill the notify buffer with the node info */
	rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_FIRST(&cb->clmdb.clm_list);
	if (rec != NULL) {
		do {
			notify[i].clusterNode.nodeId = rec->info.node_id;
			notify[i].clusterNode.nodeAddress = rec->info.node_address;
			notify[i].clusterNode.nodeName = rec->info.node_name_net;
			notify[i].clusterNode.nodeName.length = m_NCS_OS_NTOHS(rec->info.node_name_net.length);
			notify[i].clusterNode.member = rec->info.member;
			notify[i].clusterNode.bootTimestamp = rec->info.boot_timestamp;
			notify[i].clusterNode.initialViewNumber = rec->info.view_number;

			notify[i].clusterChange = SA_CLM_NODE_NO_CHANGE;

			i++;
			rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->clm_dll_node);
		}
		while ((rec != NULL) && (i < num));
	}

	/* stick the notification buffer into the message */
	cla_msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG;
	if (!cla_msg) {
		rc = NCSCC_RC_FAILURE;
		goto error;
	}
	memset(cla_msg, 0, sizeof(AVSV_NDA_CLA_MSG));

	/* Fill the cla msg and make sure everything is linked */
	if (is_sync_api == TRUE) {
		/* This is a sync call back, fill the api_resp msg */
		cla_msg->type = AVSV_AVND_CLM_API_RESP_MSG;
		cla_msg->info.api_resp_info.type = AVSV_CLM_TRACK_START;
		cla_msg->info.api_resp_info.param.track.num = num;
		cla_msg->info.api_resp_info.param.track.notify = notify;
		cla_msg->info.api_resp_info.rc = NCSCC_RC_SUCCESS;
	} else {
		cla_msg->type = AVSV_AVND_CLM_CBK_MSG;
		cla_msg->info.cbk_info.hdl = trk_info->req_hdl;
		cla_msg->info.cbk_info.type = AVSV_CLM_CBK_TRACK;
		cla_msg->info.cbk_info.param.track.mem_num = num;
		cla_msg->info.cbk_info.param.track.err = SA_AIS_OK;
		cla_msg->info.cbk_info.param.track.notify.viewNumber = cb->clmdb.curr_view_num;
		cla_msg->info.cbk_info.param.track.notify.numberOfItems = num;
		cla_msg->info.cbk_info.param.track.notify.notification = notify;
	}

   /*** send the callback message to CLA ***/
	memset(&msg, 0, sizeof(AVND_MSG));
	msg.type = AVND_MSG_CLA;
	msg.info.cla = cla_msg;

	if (is_sync_api == TRUE)
		rc = avnd_mds_send(cb, &msg, &trk_info->mds_dest, ctxt);
	else
		rc = avnd_mds_send(cb, &msg, &trk_info->mds_dest, NULL);

	return rc;

 error:
	if (cla_msg)
		m_MMGR_FREE_AVSV_NDA_CLA_MSG(cla_msg);

	if (notify)
		m_MMGR_FREE_AVSV_CLA_DEFAULT_VAL(notify);

	return rc;
}

/****************************************************************************
  Name          : avnd_clm_snd_track_changes
 
  Description   : This routine searches the list of track info nodes and
                  based on the flag sends corresponding info.
 
  Arguments     : cb          - ptr to the AvND control block
                  current_rec - ptr to current CLM record
                  change      - the change on the node.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : 

         Rough algorithm: 

         for each trk_info node check the track flags
         if(track flag == change only)
            send only that rec to the mds_dest in the trk_info
         else if(track flag == changes)
            get each record and check if it matches the passed "rec" argument
            if(matches)
               mark the trachchanges field as passed argument "change" 
            else
               mark as NO_CHANGE
         else
           nothing
         end loop

         if change = SA_CLM_NODE_LEFT
            add the rec to the notify buffer and mark change 

******************************************************************************/
void avnd_clm_snd_track_changes(AVND_CB *cb, AVND_CLM_REC *current_rec, SaClmClusterChangesT change)
{
	SaClmClusterNotificationT *notify = NULL;
	AVND_MSG msg;
	AVSV_NDA_CLA_MSG *cla_msg = 0;
	uns32 num = 0;
	AVND_CLM_REC *rec = NULL;
	uns32 i = 0;
	AVND_CLM_TRK_INFO *trk_info;

	/* determine the number of items */
	num = cb->clmdb.clm_list.n_nodes;

	trk_info = cb->clmdb.clm_trk_info;

	while (trk_info) {
		if (m_AVND_CLM_IS_TRACK_CHANGES_ONLY(trk_info)) {
			/* Allocate memory for just one record, the rec which got changed ONLY */
			notify = m_MMGR_ALLOC_AVSV_CLA_DEFAULT_VAL(sizeof(SaClmClusterNotificationT));
			if (!notify) {
				/* Log an error . This is low memory situation need not continue */
				return;
			}
			notify->clusterNode.nodeId = current_rec->info.node_id;
			notify->clusterNode.nodeAddress = current_rec->info.node_address;
			notify->clusterNode.nodeName = current_rec->info.node_name_net;
			notify->clusterNode.nodeName.length = m_NCS_OS_NTOHS(current_rec->info.node_name_net.length);

			/* If a node leaves the cluster, member field should be false */
			if (change == SA_CLM_NODE_LEFT)
				notify->clusterNode.member = SA_FALSE;
			else
				notify->clusterNode.member = current_rec->info.member;

			notify->clusterNode.bootTimestamp = current_rec->info.boot_timestamp;
			notify->clusterNode.initialViewNumber = current_rec->info.view_number;
			notify->clusterChange = change;
		} else if (m_AVND_CLM_IS_TRACK_CHANGES(trk_info)) {
			/* alloc the notify buffer */
			if (num)
				notify = m_MMGR_ALLOC_AVSV_CLA_DEFAULT_VAL(num * sizeof(SaClmClusterNotificationT));
			if (!notify) {
				/* Log an error . This is low memory situation need not continue */
				return;
			}
			memset(notify, 0, num * sizeof(SaClmClusterNotificationT));

			i = 0;
			rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_FIRST(&cb->clmdb.clm_list);
			if (rec != NULL) {
				do {
					if (rec == current_rec)
						notify[i].clusterChange = change;
					else
						notify[i].clusterChange = SA_CLM_NODE_NO_CHANGE;

					notify[i].clusterNode.nodeId = rec->info.node_id;
					notify[i].clusterNode.nodeAddress = rec->info.node_address;
					notify[i].clusterNode.nodeName = rec->info.node_name_net;
					notify[i].clusterNode.nodeName.length =
					    m_NCS_OS_NTOHS(rec->info.node_name_net.length);

					/* If a node leaves the cluster, member field should be false */
					if ((change == SA_CLM_NODE_LEFT) && (rec == current_rec))
						notify[i].clusterNode.member = SA_FALSE;
					else
						notify[i].clusterNode.member = rec->info.member;

					notify[i].clusterNode.bootTimestamp = rec->info.boot_timestamp;
					notify[i].clusterNode.initialViewNumber = rec->info.view_number;

					i++;
					rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_NEXT(&rec->clm_dll_node);
				}
				while ((rec != NULL) && (i < num));
			}
		} else {
			trk_info = trk_info->next;
			continue;
		}

		/* stick the notification buffer into the message */
		cla_msg = m_MMGR_ALLOC_AVSV_NDA_CLA_MSG;
		if (!cla_msg) {
			if (notify)
				m_MMGR_FREE_AVSV_CLA_DEFAULT_VAL(notify);
			continue;
		}
		memset(cla_msg, 0, sizeof(AVSV_NDA_CLA_MSG));

		/* Fill the cla msg and make sure everything is linked */
		cla_msg->type = AVSV_AVND_CLM_CBK_MSG;
		cla_msg->info.cbk_info.hdl = trk_info->req_hdl;
		cla_msg->info.cbk_info.type = AVSV_CLM_CBK_TRACK;

		/* If a node leaves the cluster, curr view number will be one less */
		if (change == SA_CLM_NODE_LEFT)
			cla_msg->info.cbk_info.param.track.mem_num = num - 1;
		else
			cla_msg->info.cbk_info.param.track.mem_num = num;

		cla_msg->info.cbk_info.param.track.err = SA_AIS_OK;
		cla_msg->info.cbk_info.param.track.notify.viewNumber = cb->clmdb.curr_view_num;

		/* If CHANGES_ONLY then we will have only one record */
		if (m_AVND_CLM_IS_TRACK_CHANGES_ONLY(trk_info))
			cla_msg->info.cbk_info.param.track.notify.numberOfItems = 1;
		else
			cla_msg->info.cbk_info.param.track.notify.numberOfItems = num;

		cla_msg->info.cbk_info.param.track.notify.notification = notify;

      /*** send the callback message to CLA ***/
		memset(&msg, 0, sizeof(AVND_MSG));
		msg.type = AVND_MSG_CLA;
		msg.info.cla = cla_msg;

		avnd_mds_send(cb, &msg, &trk_info->mds_dest, 0);

		/* DONE sending the message for one node move on to next TRK_INFO */
		trk_info = trk_info->next;
	}

	return;
}
