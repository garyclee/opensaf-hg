/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2010 The OpenSAF Foundation
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

#include <avd_cb.h>
#include <avd.h>
#include <avd_clm.h>
#include <saAis.h>
#include <logtrace.h>
#include <avd_node.h>

static SaVersionT clmVersion = { 'B', 4, 1 };

static void clm_node_get_cb(SaInvocationT invocation,
	const SaClmClusterNodeT_4 *clusterNode,
	SaAisErrorT error)
{
	TRACE_LEAVE2("Not Implemented");
	return;
}

static void clm_node_add(AVD_AVND *node, SaClmClusterNodeT_4 *clmNode)
{
	node->node_info.nodeId = clmNode->nodeId;
	node->node_info.member = clmNode->member;
	node->node_info.bootTimestamp = clmNode->bootTimestamp;
	node->node_info.initialViewNumber = clmNode->initialViewNumber;
	memcpy((void *)(&(node->node_info.nodeAddress)),
	       (void *)(&(clmNode->nodeAddress)), sizeof(SaClmNodeAddressT));
	memcpy((void *)(&(node->node_info.executionEnvironment)),
	       (void *)(&(clmNode->executionEnvironment)),sizeof(SaNameT));
	/* node_info.nodeName still contains SaAmfNode name */
	avd_node_add_nodeid(node);
}

static void clm_track_cb(const SaClmClusterNotificationBufferT_4 *notificationBuffer,
	SaUint32T numberOfMembers, SaInvocationT invocation,
	const SaNameT *rootCauseEntity, const SaNtfCorrelationIdsT *correlationIds,
	SaClmChangeStepT step, SaTimeT timeSupervision, SaAisErrorT error)
{
	int i;
	SaClmClusterNotificationT_4 *notifItem;
	AVD_AVND *node;

	TRACE_ENTER2("'%llu' '%u' '%u'", invocation, step, error);

	if (error != SA_AIS_OK) {
		LOG_ER("ClmTrackCallback received in error");
		goto done;
	}

	for (i = 0; i < notificationBuffer->numberOfItems; i++)
	{
		notifItem = &notificationBuffer->notification[i];
		switch(step) {
		case SA_CLM_CHANGE_VALIDATE:
			if(notifItem->clusterChange == SA_CLM_NODE_LEFT) {
				node = avd_node_find_nodeid(notifItem->clusterNode.nodeId);
				/* store the invocation for clm response */
				node->clm_pend_inv = invocation;
				/* Validate the node leaving cluster if not acceptable then
				   reject it through saClmResponse_4 */
				clm_node_exit_validate(node);
			}
			else /* All other Cluster changes are not valid in this step */
				LOG_ER("Invalid Cluster change in VALIDATE step");
			break;

		case SA_CLM_CHANGE_START:
			node = avd_node_find_nodeid(notifItem->clusterNode.nodeId);
			if ( notifItem->clusterChange == SA_CLM_NODE_LEFT ||
			     notifItem->clusterChange == SA_CLM_NODE_SHUTDOWN ) {
				/* invocation to be used by pending clm response */ 
				node->clm_pend_inv = invocation;
				clm_node_exit_start(node, notifItem->clusterChange);
			}
			else
				LOG_ER("Invalid Cluster change in START step");
			break;

		case SA_CLM_CHANGE_COMPLETED:
			if( (notifItem->clusterChange == SA_CLM_NODE_LEFT)||
			    (notifItem->clusterChange == SA_CLM_NODE_SHUTDOWN)) {
				node = avd_node_find_nodeid(notifItem->clusterNode.nodeId);
				if (node == NULL) {
					LOG_NO("Node not a member");
					goto done;
				}
				LOG_NO("Node has exited cluster '%x'", node->node_info.nodeId);
				/* make the node OO cluster */
				clm_node_exit_complete(node);
				/* Remove the node from the node_id tree. */
				avd_node_delete_nodeid(node);
			}
			else if(notifItem->clusterChange == SA_CLM_NODE_RECONFIGURED) {
				/* delete, reconfigure, re-add to the node-id db
				   incase if the node id has changed */
				node = avd_node_find_nodeid(notifItem->clusterNode.nodeId);
				avd_node_delete_nodeid(node);
				clm_node_add(node, &(notifItem->clusterNode));
			}
			else if(notifItem->clusterChange == SA_CLM_NODE_JOINED || 
			        notifItem->clusterChange == SA_CLM_NODE_NO_CHANGE) {
				/* This may be due to track flags set to 
				 SA_TRACK_CURRENT|CHANGES_ONLY and supply no buffer
				 in saClmClusterTrack call so update the local database */
				/* get the first node */
				SaClmClusterNodeT_4 *clmNode = &(notifItem->clusterNode);
				node = avd_node_getnext(NULL);
				while (node != NULL && 
					0 != strncmp(node->saAmfNodeClmNode.value,
						 notifItem->clusterNode.nodeName.value,
						 notifItem->clusterNode.nodeName.length))
				{
					node = avd_node_getnext(&node->node_info.nodeName);
				}
				if ( node != NULL ) {
					clm_node_add(node, clmNode);
					LOG_NO("Node Joined '%s' '%x'", node->node_info.nodeName.value, clmNode->nodeId);
				}
				if (node->node_state == AVD_AVND_STATE_PRESENT) {
					LOG_NO("Node already up and configured");
					/* now try to instantiate all the SUs that need to be */
					node_admin_unlock_instantiation(node);
				}
			}
			else
				LOG_NO("clmTrackCallback in COMPLETED::UNLOCK");
			break;

		case SA_CLM_CHANGE_ABORTED:
			/* Do nothing */
			LOG_NO("ClmTrackCallback ABORTED_STEP" );
			break;
		}
	}
done:
	TRACE_LEAVE();
}

static const SaClmCallbacksT_4 clm_callbacks = {
	.saClmClusterNodeGetCallback = clm_node_get_cb,
	.saClmClusterTrackCallback = clm_track_cb
};

SaAisErrorT avd_clm_init()
{
        SaAisErrorT error = SA_AIS_OK;

	TRACE_ENTER();
	error = saClmInitialize_4(&avd_cb->clmHandle, &clm_callbacks, &clmVersion);
	if (SA_AIS_OK != error) {
		LOG_ER("Failed to initialize with CLM %u", error);
		goto done;
	}
	error = saClmSelectionObjectGet(avd_cb->clmHandle, &avd_cb->clm_sel_obj);
	if (SA_AIS_OK != error) { 
		LOG_ER("Failed to get selection object from CLM %u", error);
		goto done;
	}
	LOG_NO("Successfully initialized CLM");

done:
	TRACE_LEAVE();
	return error;
}

SaAisErrorT avd_clm_track_start()
{
        SaAisErrorT error = SA_AIS_OK;
	SaUint8T trackFlags = SA_TRACK_CURRENT|SA_TRACK_CHANGES_ONLY|SA_TRACK_VALIDATE_STEP; 
        
	TRACE_ENTER();
	error = saClmClusterTrack_4(avd_cb->clmHandle, trackFlags, NULL);
        if (SA_AIS_OK != error)
                LOG_ER("Failed to start cluster tracking");

        TRACE_LEAVE();
	return error;
}

SaAisErrorT avd_clm_track_stop()
{
        SaAisErrorT error = SA_AIS_OK;

        TRACE_ENTER();
	error = saClmClusterTrackStop(avd_cb->clmHandle);
        if (SA_AIS_OK != error) 
                LOG_ER("Failed to stop cluster tracking");
	else
		LOG_NO("Sucessfully stops cluster tracking");

        TRACE_LEAVE();
        return error;
}

SaAisErrorT avd_clm_finalize()
{
        SaAisErrorT error = SA_AIS_OK;

        TRACE_ENTER();
	error = saClmFinalize(avd_cb->clmHandle);
        if (SA_AIS_OK != error)
                LOG_ER("Failed to finalize with CLM");
	else
                LOG_NO("Sucessfully finalizes with CLM");
		
	TRACE_LEAVE();
	return error;
}

/* validating this node for a graceful exit */ 
void clm_node_exit_validate(AVD_AVND *node)
{
	AVD_SU *su;
	AVD_SU_SI_REL *susi;
	SaBoolT reject = SA_FALSE;

	/* now go through each SU to determine whether
	 any SI assigned becomes unassigned due to node exit*/
	su = node->list_of_su;
	while (su != NULL) {
		susi = su->list_of_susi;
		/* now evalutate each SI that is assigned to this SU */
		while (susi != NULL) {
			if ((susi->state == SA_AMF_HA_ACTIVE) &&
			    (susi->si->saAmfSINumCurrActiveAssignments == 1) &&
			    (susi->si->saAmfSINumCurrStandbyAssignments == 0) &&
			    (su->sg_of_su->sg_redundancy_model != SA_AMF_NO_REDUNDANCY_MODEL)) {
				/* there is only one active assignment w.r.t this SUSI */
				reject = SA_TRUE;
				goto done;
			}
			susi = susi->su_next;
		}
	}

done:
	if(reject == SA_FALSE) { 
		saClmResponse_4(avd_cb->clmHandle, node->clm_pend_inv,
						SA_CLM_CALLBACK_RESPONSE_OK);
		LOG_NO("CLM track callback VALIDATE::ACCEPTED");
	}
	else { 
		saClmResponse_4(avd_cb->clmHandle, node->clm_pend_inv,
						SA_CLM_CALLBACK_RESPONSE_REJECTED);
		LOG_NO("CLM track callback VALIDATE::REJECTED");
	}

	node->clm_pend_inv = 0;
	TRACE_LEAVE();
} 

/* perform required operations on this node for a graceful exit */ 
void clm_node_exit_start(AVD_AVND *node, SaClmClusterChangesT change)
{
	TRACE_ENTER();

	if (node->saAmfNodeOperState == SA_AMF_OPERATIONAL_DISABLED) {
		saClmResponse_4(avd_cb->clmHandle, node->clm_pend_inv, 
			SA_CLM_CALLBACK_RESPONSE_OK);
		node->clm_pend_inv = 0; 
		goto done;
	}

	if (change == SA_CLM_NODE_SHUTDOWN)
		/* call with NULL invocation to differentiate it with AMF node shutdown */
		node_admin_lock_unlock_shutdown(node, 0, SA_AMF_ADMIN_SHUTDOWN); 
	else /* SA_CLM_NODE_LEFT case */
		node_admin_lock_unlock_shutdown(node, 0, SA_AMF_ADMIN_LOCK);

	if (node->su_cnt_admin_oper == 0 && node->clm_pend_inv != 0) {
		clm_node_terminate(node);
	}
	/* else wait for the su's on this node that are undergoing the required operation */
done:
	TRACE_LEAVE();
}

void clm_node_exit_complete(AVD_AVND *node)
{
	/*TODO*/
	/*AvND will terminate the local components*/
	/*Currently since middleware components are considered to be up
	 you cannot mark the node as absent :- TBD*/
	avd_node_oper_state_set(node, SA_AMF_OPERATIONAL_DISABLED);
	/* stop the heart beat timer */
	avd_stop_tmr(avd_cb, &(node->heartbeat_rcv_avnd));
}

void clm_node_join_complete(AVD_AVND *node)
{
	AVD_SU *su;

	TRACE_ENTER();
	/* For each of the SUs calculate the readiness state. 
	 ** call the SG FSM with the new readiness state.
	 */

	avd_node_oper_state_set(node, SA_AMF_OPERATIONAL_ENABLED);
	su = node->list_of_su;
	while (su != NULL) {

		if (m_AVD_APP_SU_IS_INSVC(su, node)) {
			avd_su_readiness_state_set(su, SA_AMF_READINESS_IN_SERVICE);
			switch (su->sg_of_su->sg_redundancy_model) {
			case SA_AMF_2N_REDUNDANCY_MODEL:
				avd_sg_2n_su_insvc_func(avd_cb, su);
				break;

			case SA_AMF_N_WAY_REDUNDANCY_MODEL:
				avd_sg_nway_su_insvc_func(avd_cb, su);
				break;

			case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
				avd_sg_nacvred_su_insvc_func(avd_cb, su);
				break;

			case SA_AMF_NPM_REDUNDANCY_MODEL:
				avd_sg_npm_su_insvc_func(avd_cb, su);
				break;

			case SA_AMF_NO_REDUNDANCY_MODEL:
				avd_sg_nored_su_insvc_func(avd_cb, su);
				break;
			}

			/* Since an SU has come in-service re look at the SG to see if other
			 * instantiations or terminations need to be done.
			 */
			avd_sg_app_su_inst_func(avd_cb, su->sg_of_su);
		}
		/* get the next SU on the node */
		su = su->avnd_list_su_next;
	}

	TRACE_LEAVE();
}

void clm_node_terminate(AVD_AVND *node)
{

	if (NCSCC_RC_SUCCESS != node_admin_lock_instantiation(node)) { 
		saClmResponse_4(avd_cb->clmHandle, node->clm_pend_inv, SA_CLM_CALLBACK_RESPONSE_ERROR);
		LOG_NO("Pending Response sent for CLM track callback::ERROR");
		node->clm_pend_inv = 0;
	} /* NCSCC_RC_SUCCESS means that either you had no SUs terminate or you sent
	     all the SUs on this node the termination message */
	else if (node->su_cnt_admin_oper == 0) { 
		saClmResponse_4(avd_cb->clmHandle, node->clm_pend_inv, SA_CLM_CALLBACK_RESPONSE_OK);
		LOG_NO("Pending Response sent for CLM track callback::OK");
		node->clm_pend_inv = 0;
	}
	/* else wait for the SU presence state updates */
}
