/*      -*- OpenSAF -*-
 *
 * Copyright (C) 2017, Oracle and/or its affiliates. All rights reserved.
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
 * Author(s): Oracle
 *
 */

#include "immnd.h"
#include "base/osaf_time.h"

/****************************************************************************
 * Name : immnd_clm_node_change
 *
 * Description :
 * 	When the clm clusterchange happens the function is called.
 *	
 * Arguments:
 *	left:  True if clm left the node else false 
 *          
 *  Return Values : 
 * 	NCSCC_RC_SUCCESS if change is completed sucessfully.  
 *
****************************************************************************/
uint32_t immnd_clm_node_change(bool left){
	IMMSV_EVT send_evt;
	TRACE_ENTER();
        uint32_t rc = NCSCC_RC_SUCCESS;
	if(left){
		immnd_cb->isClmNodeJoined = false;
		TRACE("isClmNodeJoined is set to false");
	} else {
		immnd_cb->isClmNodeJoined = true;
		TRACE("isClmNodeJoined is set to true");
	}

	SaImmHandleT clientHdl = 0;
	IMMND_IMM_CLIENT_NODE * client_node = NULL;
	send_evt.type = IMMSV_EVT_TYPE_IMMA;
	immnd_client_node_getnext(immnd_cb, 0, &client_node);
	while(client_node){
		memset(&send_evt, '\0', sizeof(IMMSV_EVT));
		if (client_node->version.minorVersion >= 0x12 &&
				client_node->version.majorVersion == 0x2 &&
				client_node->version.releaseCode == 'A') {
			if(immnd_cb->isClmNodeJoined){
				send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_CLM_NODE_JOINED;
				TRACE("Sending Node Joined message to client handle %llx", client_node->imm_app_hdl);
			} else {
				send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_CLM_NODE_LEFT;
				TRACE("Sending Node Left message to client handle %llx", client_node->imm_app_hdl);
			}

			if(immnd_mds_msg_send(immnd_cb, client_node->sv_id,
						client_node->agent_mds_dest, &send_evt) != NCSCC_RC_SUCCESS) {
				TRACE("Sending clm change to client id %llx failed", client_node->imm_app_hdl);
			}else {
				TRACE("immnd_mds_msg_send success");
			}
		}
			clientHdl = client_node->imm_app_hdl;
			immnd_client_node_getnext(immnd_cb, clientHdl, &client_node);
		}
	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
 * Name : immnd_clm_track_cbk
 *
 * Description :
 *         CLM callback for tracking the node membership status.
 *         Depending upon the membership status (joining/leaving cluster)
 *         of a node with node_id of present node clm_join or clm_left will 
 *         be broadcasted to all imma agents. 
 *
 *  Return Values : None.
 *
****************************************************************************/
static void immnd_clm_track_cbk(const SaClmClusterNotificationBufferT_4 *notificationBuffer,
        SaUint32T numberOfMembers, SaInvocationT invocation,
        const SaNameT *rootCauseEntity, const SaNtfCorrelationIdsT *correlationIds,
        SaClmChangeStepT step, SaTimeT timeSupervision, SaAisErrorT error)
{
	NCS_NODE_ID node_id;
        uint32_t i = 0;
	IMMND_CLM_NODE_LIST * imm_clm_node = NULL ;

        TRACE_ENTER2("Returned error value from callback is %d",error);

        if (error != SA_AIS_OK)
                return;

	for (i = 0; i < notificationBuffer->numberOfItems; i++) {
		switch(step) {
			case SA_CLM_CHANGE_COMPLETED:
				node_id = notificationBuffer->notification[i].clusterNode.nodeId;
				imm_clm_node = NULL;
                                immnd_clm_node_get(immnd_cb, node_id, &imm_clm_node);
				if(notificationBuffer->notification[i].clusterChange == SA_CLM_NODE_LEFT){
					if(imm_clm_node){
						if( node_id == immnd_cb->node_id){
							if (immnd_clm_node_change(true) != NCSCC_RC_SUCCESS) {
								TRACE_4(" immnd_proc_ckpt_clm_node_left failed");
							}
						}
						immnd_clm_node_delete(immnd_cb, imm_clm_node);
						TRACE("Node %x left the CLM membership", node_id);
					}else if(node_id == immnd_cb->node_id){
						TRACE("IMMND restarted when node is locked");
						if (immnd_clm_node_change(true) != NCSCC_RC_SUCCESS) {
							TRACE_4(" immnd_proc_ckpt_clm_node_left failed");
						}
					}
				} else if(!imm_clm_node){
					if ((notificationBuffer->notification[i].clusterChange == SA_CLM_NODE_NO_CHANGE) ||
							(notificationBuffer->notification[i].clusterChange == SA_CLM_NODE_JOINED) ||
							(notificationBuffer->notification[i].clusterChange == SA_CLM_NODE_RECONFIGURED)) {
						if( node_id == immnd_cb->node_id){
							if (immnd_clm_node_change(false) != NCSCC_RC_SUCCESS) {
								TRACE_4("immnd_proc_ckpt_clm_node_joined  failed");
							}
						}
						if (immnd_clm_node_add(immnd_cb, node_id) != NCSCC_RC_SUCCESS) {
							TRACE_4("immnd_clm_node_add failed");
						}

						TRACE("Node %x Joined the CLM membership", node_id);
					}
				}

				break;
			default:
				break;
		}
	}
	TRACE_LEAVE();
}

static SaVersionT clmVersion = { 'B', 0x04, 0x01 };
static const SaClmCallbacksT_4 clm_callbacks = {
	0,
	immnd_clm_track_cbk /*saClmClusterTrackCallback*/
};

/****************************************************************************
 * Name : immnd_clm_init_thread
 *
 * Description :
 *    Registers with the CLM service (B.04.01).
 *  Return Values : None.
 *
 ****************************************************************************/
void *immnd_clm_init_thread(void *cb)
{
	TRACE_ENTER();

	SaAisErrorT rc = saClmInitialize_4(&immnd_cb->clm_hdl, &clm_callbacks, &clmVersion);
	while ((rc == SA_AIS_ERR_TRY_AGAIN) || (rc == SA_AIS_ERR_TIMEOUT) ||
			(rc == SA_AIS_ERR_UNAVAILABLE)) {
		osaf_nanosleep(&kHundredMilliseconds);
		rc = saClmInitialize_4(&immnd_cb->clm_hdl, &clm_callbacks, &clmVersion);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("saClmInitialize failed with error: %d", rc);
		TRACE_LEAVE();
		exit(EXIT_FAILURE);
	}

	rc = saClmSelectionObjectGet(immnd_cb->clm_hdl, &immnd_cb->clmSelectionObject);
	if (rc != SA_AIS_OK) {
		LOG_ER("saClmSelectionObjectGet failed with error: %d", rc);
		TRACE_LEAVE();
		exit(EXIT_FAILURE);
	}
	rc = saClmClusterTrack_4(immnd_cb->clm_hdl, SA_TRACK_CURRENT|SA_TRACK_CHANGES, NULL);
	if (rc != SA_AIS_OK) {
		LOG_ER("saClmClusterTrack failed with error: %d", rc);
                TRACE_LEAVE();
                exit(EXIT_FAILURE);
        }
        TRACE("CLM Initialization SUCCESS......");
        TRACE_LEAVE();
        return NULL;
}
/****************************************************************************
 * Name : immnd_init_with_clm
 *
 * Description :
 *      initialize clm thread
 *
 * Return Values : None.
 *
****************************************************************************/
void immnd_init_with_clm(void)
{
        pthread_t thread;
        pthread_attr_t attr;
        TRACE_ENTER();

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&thread, &attr, immnd_clm_init_thread, immnd_cb) != 0) {
                LOG_ER("pthread_create FAILED: %s", strerror(errno));
                exit(EXIT_FAILURE);
        }
        pthread_attr_destroy(&attr);
        TRACE_LEAVE();
}

