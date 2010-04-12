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
 * Author(s): Ericsson AB
 *
 */

/*****************************************************************************
  FILE NAME: immnd_proc.c

  DESCRIPTION: IMMND Processing routines

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include <libgen.h>
#include <signal.h>
#include <sys/types.h>

#include <configmake.h>
#include <nid_api.h>

#include "immnd.h"
#include "immsv_api.h"

#define SIG_TERM 15 

static const char *loaderBase = "immload";
static const char *pbeBase = BINDIR "/immdump";

void immnd_ackToNid(uns32 rc)
{
	if (immnd_cb->nid_started == 0)
		return;

	if (nid_notify("IMMND", rc, NULL) != NCSCC_RC_SUCCESS) {
		LOG_ER("nid_notify failed");
		rc = NCSCC_RC_FAILURE;
	}
}

/****************************************************************************
 * Name          : immnd_proc_immd_down 
 *
 * Description   : Function to handle Director going down
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *
 * Notes         : Policy used for handling immd down is to blindly cleanup 
 *                :immnd_cb 
 ****************************************************************************/
void immnd_proc_immd_down(IMMND_CB *cb)
{
	LOG_WA("immmnd_proc_immd_down - not implemented");
}

/****************************************************************************
 * Name          : immnd_proc_imma_discard_connection
 *
 * Description   : Function to handle Director going down
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_IMM_CLIENT_NODE *cl_node - a client node.
 *
 * Return Values : FALSE => connection was NOT completely removed (IMMD down)
 * Notes         : Policy used for handling immd down is to blindly cleanup 
 *                :immnd_cb 
 ****************************************************************************/
uns32 immnd_proc_imma_discard_connection(IMMND_CB *cb, IMMND_IMM_CLIENT_NODE *cl_node)
{
	SaUint32T client_id;
	SaUint32T node_id;
	IMMND_OM_SEARCH_NODE *sn = NULL;
	SaUint32T implId = 0;
	IMMSV_EVT send_evt;
	SaUint32T *idArr = NULL;
	SaUint32T arrSize = 0;

	TRACE_ENTER();

	client_id = m_IMMSV_UNPACK_HANDLE_HIGH(cl_node->imm_app_hdl);
	node_id = m_IMMSV_UNPACK_HANDLE_LOW(cl_node->imm_app_hdl);
	TRACE_5("Attempting discard connection id:%llx <n:%x, c:%u>", cl_node->imm_app_hdl, node_id, client_id);
	/* Corresponds to "discardConnection" in the older EVS based IMM 
	   implementation. */

	/* 1. Discard searchOps. */
	while (cl_node->searchOpList) {
		sn = cl_node->searchOpList;
		cl_node->searchOpList = sn->next;
		sn->next = NULL;
		TRACE_5("Discarding search op %u", sn->searchId);
		immModel_deleteSearchOp(sn->searchOp);
		sn->searchOp = NULL;
		free(sn);
	}

	/* 2. Discard any continuations (locally only) that are associated with 
	   the dead connection. */

	immModel_discardContinuations(cb, client_id);

	/* No need to broadcast the discarding of the connection (as compared
	   with the EVS based implementation. In the new implementation we 
	   always look up the connection in the client_tree. Any late arrivals
	   destined for this connection will simply not find it and be discarded.
	 */

	/* 3. Take care of (at most one!) implementer associated with the dead
	   connection. Broadcast the dissapearance of the impementer via IMMD. 
	 */

	implId =		/* Assumes there is at most one implementer/conn  */
	    immModel_getImplementerId(cb, client_id);

	if (implId) {
		TRACE_5("Discarding implementer id:%u for connection: %u", implId, client_id);
		memset(&send_evt, '\0', sizeof(IMMSV_EVT));
		send_evt.type = IMMSV_EVT_TYPE_IMMD;
		send_evt.info.immd.type = IMMD_EVT_ND2D_DISCARD_IMPL;
		send_evt.info.immd.info.impl_set.r.impl_id = implId;
		if (immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, &send_evt) != NCSCC_RC_SUCCESS) {
			if (immnd_is_immd_up(cb)) {
				LOG_ER("Discard implementer failed for implId:%u "
				       "but IMMD is up !? - case not handled. Client will be orphanded", implId);
			} else {
				LOG_WA("Discard implementer failed for implId:%u "
				       "(immd_down)- will retry later", implId);
			}
			cl_node->mIsStale = TRUE;
		}
		/*Discard the local implementer directly and redundantly to avoid 
		   race conditions using this implementer (ccb's causing abort upcalls).
		 */
		immModel_discardImplementer(cb, implId, SA_FALSE);
	}

	if (cl_node->mIsStale) {
		TRACE_LEAVE();
		return FALSE;
	}

	/* 4. Check for Ccbs that where originated from the dead connection.
	   Abort all such ccbs via broadcast over IMMD. 
	 */

	immModel_getCcbIdsForOrigCon(cb, client_id, &arrSize, &idArr);
	if (arrSize) {
		SaUint32T ix;
		memset(&send_evt, '\0', sizeof(IMMSV_EVT));
		send_evt.type = IMMSV_EVT_TYPE_IMMD;
		send_evt.info.immd.type = IMMD_EVT_ND2D_ABORT_CCB;
		for (ix = 0; ix < arrSize && !(cl_node->mIsStale); ++ix) {
			send_evt.info.immd.info.ccbId = idArr[ix];
			TRACE_5("Discarding Ccb id:%u originating at dead connection: %u", idArr[ix], client_id);
			if (immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id,
					       &send_evt) != NCSCC_RC_SUCCESS) {
				if (immnd_is_immd_up(cb)) {
					LOG_ER("Failure to broadcast discard Ccb for ccbId:%u "
					       "but IMMD is up !? - case not handled. Client will "
					       "be orphanded", implId);
				} else {
					LOG_WA("Failure to broadcast discard Ccb for ccbId:%u "
					       "(immd down)- will retry later", idArr[ix]);
				}
				cl_node->mIsStale = TRUE;
			}
		}
		free(idArr);
		idArr = NULL;
		arrSize = 0;
	}

	if (cl_node->mIsStale) {
		TRACE_LEAVE();
		return FALSE;
	}

	/* 5. 
	   Check for admin owners that where associated with the dead connection.
	   Finalize all such admin owners via broadcast over IMMD. */

	immModel_getAdminOwnerIdsForCon(cb, client_id, &arrSize, &idArr);
	if (arrSize) {
		SaUint32T ix;
		memset(&send_evt, '\0', sizeof(IMMSV_EVT));
		send_evt.type = IMMSV_EVT_TYPE_IMMD;
		send_evt.info.immd.type = IMMD_EVT_ND2D_ADMO_HARD_FINALIZE;
		for (ix = 0; ix < arrSize && !(cl_node->mIsStale); ++ix) {
			send_evt.info.immd.info.admoId = idArr[ix];
			TRACE_5("Hard finalize of AdmOwner id:%u originating at "
				"dead connection: %u", idArr[ix], client_id);
			if (immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id,
					       &send_evt) != NCSCC_RC_SUCCESS) {
				if (immnd_is_immd_up(cb)) {
					LOG_ER("Failure to broadcast discard admo0wner for ccbId:%u "
					       "but IMMD is up !? - case not handled. Client will "
					       "be orphanded", implId);
				} else {
					LOG_WA("Failure to broadcast discard admowner for id:%u "
					       "(immd down)- will retry later", idArr[ix]);
				}
				cl_node->mIsStale = TRUE;
			}
		}
		free(idArr);
		idArr = NULL;
		arrSize = 0;
	}

	if (!cl_node->mIsStale) {
		TRACE_5("Discard connection id:%llx succeeded", cl_node->imm_app_hdl);
	}

	TRACE_LEAVE();
	return !(cl_node->mIsStale);
}

/****************************************************************************
 * Name          : immnd_proc_imma_down 
 *
 * Description   : Function to process agent going down    
 *                 from Applications. 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 MDS_DEST dest - Agent MDS_DEST
 *
 * Notes         : None.
 *****************************************************************************/
void immnd_proc_imma_down(IMMND_CB *cb, MDS_DEST dest, NCSMDS_SVC_ID sv_id)
{
	IMMND_IMM_CLIENT_NODE *cl_node = NULL;
	SaImmHandleT prev_hdl;
	unsigned int count = 0;
	unsigned int failed = 0;

	/* go through the client tree  */
	immnd_client_node_getnext(cb, 0, &cl_node);

	while (cl_node) {
		prev_hdl = cl_node->imm_app_hdl;

		if ((memcmp(&dest, &cl_node->agent_mds_dest, sizeof(MDS_DEST)) == 0) && sv_id == cl_node->sv_id) {
			if (immnd_proc_imma_discard_connection(cb, cl_node)) {
				TRACE_5("Removing client id:%llx sv_id:%u", cl_node->imm_app_hdl, cl_node->sv_id);
				immnd_client_node_del(cb, cl_node);
				memset(cl_node, '\0', sizeof(IMMND_IMM_CLIENT_NODE));
				free(cl_node);
				prev_hdl = 0LL;	/* Restart iteration */
				cl_node = NULL;
				++count;
			} else {
				TRACE_5("Stale marked client id:%llx sv_id:%u", cl_node->imm_app_hdl, cl_node->sv_id);
				++failed;
			}
		}
		immnd_client_node_getnext(cb, prev_hdl, &cl_node);
	}
	if (failed) {
		TRACE_5("Removed %u IMMA clients, %u STALE IMMA clients pending", count, failed);
	} else {
		TRACE_5("Removed %u IMMA clients", count);
	}
}

/****************************************************************************
 * Name          : immnd_proc_imma_discard_stales
 *
 * Description   : Function to garbage collect stale client nodes
 *                 and to send TRY_AGAIN to a sync process blocked
 *                 waiting for reply.
 *                 Only used at IMMD UP events (failover/switchover).
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *
 * Notes         : None.
 *****************************************************************************/

void immnd_proc_imma_discard_stales(IMMND_CB *cb)
{
	IMMND_IMM_CLIENT_NODE *cl_node = NULL;
	SaImmHandleT prev_hdl;
	unsigned int count = 0;
	unsigned int failed = 0;
	TRACE_ENTER();

	/* go through the client tree  */
	immnd_client_node_getnext(cb, 0, &cl_node);
	while (cl_node) {
		prev_hdl = cl_node->imm_app_hdl;
		if (cl_node->mIsStale) {
			cl_node->mIsStale = FALSE;
			if (immnd_proc_imma_discard_connection(cb, cl_node)) {
				TRACE_5("Removing client id:%llx sv_id:%u", cl_node->imm_app_hdl, cl_node->sv_id);
				immnd_client_node_del(cb, cl_node);
				memset(cl_node, '\0', sizeof(IMMND_IMM_CLIENT_NODE));
				free(cl_node);
				prev_hdl = 0LL;	/* Restart iteration */
				cl_node = NULL;
				++count;
			} else {
				TRACE_5("Stale marked (again) client id:%llx sv_id:%u",
					cl_node->imm_app_hdl, cl_node->sv_id);
				++failed;
				/*cl_node->mIsStale = TRUE; done in discard_connection */
			}
		} else if (cl_node->mIsSync && cl_node->mSyncBlocked) {
			IMMSV_EVT send_evt;
			LOG_NO("Detected a sync client waiting on reply, send TRY_AGAIN");
			/* Dont need any delay of this. Worst case that could happen
			   is a resend of a sync message (new fevs, not same) that has 
			   actually already been processed. But such a duplicate sync
			   message (class_create/sync_object) will be rejected and
			   cause the sync to fail. This logic at least gives the sync
			   a chance to succeed. The finalize sync message is replied
			   immediately to agent, before it is sent over fevs. If the
			   immd is down then the agent gets immediate reply. So blocking
			   on immd down is not relevant for finalize sync.
			 */
			memset(&send_evt, '\0', sizeof(IMMSV_EVT));
			send_evt.type = IMMSV_EVT_TYPE_IMMA;
			send_evt.info.imma.info.errRsp.error = SA_AIS_ERR_TRY_AGAIN;
			assert(cl_node->tmpSinfo.stype == MDS_SENDTYPE_SNDRSP);
			send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
			if (immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt) != NCSCC_RC_SUCCESS) {
				LOG_ER("Failed to send result to Sync Agent");
			}
			cl_node->mSyncBlocked = FALSE;
		} else {
			TRACE_5("No action client id :%llx sv_id:%u", cl_node->imm_app_hdl, cl_node->sv_id);
		}

		immnd_client_node_getnext(cb, prev_hdl, &cl_node);
	}
	if (failed) {
		TRACE_5("Removed %u STALE IMMA clients, %u STALE clients pending", count, failed);
	} else {
		TRACE_5("Removed %u STALE IMMA clients", count);
	}

	TRACE_LEAVE();
}

/**************************************************************************
 * Name     :  immnd_introduceMe
 *
 * Description   : Generate initial message to IMMD, providing epoch etc
 *                 allowing IMMND coord to begin IMM global load or sync
 *                 sequence. 
 *
 * Return Values : NONE
 *
 *****************************************************************************/
uns32 immnd_introduceMe(IMMND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	IMMSV_EVT send_evt;
	memset(&send_evt, '\0', sizeof(IMMSV_EVT));

	send_evt.type = IMMSV_EVT_TYPE_IMMD;
	send_evt.info.immd.type = IMMD_EVT_ND2D_INTRO;
	send_evt.info.immd.info.ctrl_msg.ndExecPid = cb->mMyPid;
	send_evt.info.immd.info.ctrl_msg.epoch = cb->mMyEpoch;
	send_evt.info.immd.info.ctrl_msg.refresh = cb->mIntroduced;
	send_evt.info.immd.info.ctrl_msg.pbeEnabled = 
		cb->mPbeFile && (cb->mRim == SA_IMM_KEEP_REPOSITORY);

	if (!immnd_is_immd_up(cb)) {
		return NCSCC_RC_FAILURE;
	}

	rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, &send_evt);
	if (cb->mNumNodes == 0) {
		/*Only set to 1 if this is the first time, (dont overwrite higher 
		   values). Incremented in immnd_evt_proc_intro_rsp. */
		cb->mNumNodes = 1;
		TRACE("immnd_introduceMe cb->mNumNodes:%u", cb->mNumNodes);
	}
	return rc;
}

/**************************************************************************
 * Name     :  immnd_iAmCoordinator
 *
 * Description   : Determines if this IMMND can take on the role of
 *                 coordinator at this moment in time.
 *
 * Return Values : NONE
 *
 * Notes         : 
 *****************************************************************************/
static int32 immnd_iAmCoordinator(IMMND_CB *cb)
{
	if (!cb->mIntroduced)
		return (-1);

	if (cb->mIsCoord) {
		if (cb->mRulingEpoch > cb->mMyEpoch) {
			LOG_ER("%u > %u", cb->mRulingEpoch, cb->mMyEpoch);
			exit(1);
		}
		return 1;
	}

	return 0;
}

static int32 immnd_iAmLoader(IMMND_CB *cb)
{
	int coord = immnd_iAmCoordinator(cb);
	if (coord == -1) {
		LOG_IN("Loading is not possible, no coordinator");
		return (-1);
	}

	if (cb->mMyEpoch != cb->mRulingEpoch) {
		/*We are joining the cluster, need to sync this IMMND. */
		return (-2);
	}

	/*Loading is possible.
	   Allow all fevs messages to be handled by this node. */
	cb->mAccepted = TRUE;

	if (coord == 1) {
		if ((cb->mRulingEpoch != 0) || (cb->mMyEpoch != 0)) {
			return (-1);
		}
		return (cb->mMyEpoch + 1);
	}

	return 0;
}

static uns32 immnd_announceLoading(IMMND_CB *cb, int32 newEpoch)
{
	TRACE_ENTER();
	uns32 rc = NCSCC_RC_SUCCESS;
	IMMSV_EVT send_evt;
	memset(&send_evt, '\0', sizeof(IMMSV_EVT));

	send_evt.type = IMMSV_EVT_TYPE_IMMD;
	send_evt.info.immd.type = IMMD_EVT_ND2D_ANNOUNCE_LOADING;
	send_evt.info.immd.info.ctrl_msg.ndExecPid = cb->mMyPid;
	send_evt.info.immd.info.ctrl_msg.epoch = newEpoch;
	send_evt.info.immd.info.ctrl_msg.pbeEnabled = 
		cb->mPbeFile && (cb->mRim == SA_IMM_KEEP_REPOSITORY);

	if (immnd_is_immd_up(cb)) {
		rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, &send_evt);
	} else {
		rc = NCSCC_RC_FAILURE;
	}

	if (rc != NCSCC_RC_SUCCESS) {
		LOG_ER("Coord failed to send start of load to IMMD");
		/*Revert state back to IMM_SERVER_CLUSTER_WAITING.
		   cb->mState = IMM_SERVER_CLUSTER_WAITING; */
	} else {
		cb->mMyEpoch = newEpoch;
	}
	TRACE_LEAVE();
	return rc;
}

static uns32 immnd_requestSync(IMMND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	IMMSV_EVT send_evt;
	memset(&send_evt, '\0', sizeof(IMMSV_EVT));

	send_evt.type = IMMSV_EVT_TYPE_IMMD;
	send_evt.info.immd.type = IMMD_EVT_ND2D_REQ_SYNC;
	send_evt.info.immd.info.ctrl_msg.ndExecPid = cb->mMyPid;
	send_evt.info.immd.info.ctrl_msg.epoch = cb->mMyEpoch;
	send_evt.info.immd.info.ctrl_msg.pbeEnabled = 
		cb->mPbeFile && (cb->mRim == SA_IMM_KEEP_REPOSITORY);
	if (immnd_is_immd_up(cb)) {
		rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, &send_evt);
	} else {
		rc = NCSCC_RC_FAILURE;
	}
	return (rc == NCSCC_RC_SUCCESS);
}

void immnd_announceDump(IMMND_CB *cb)
{
	uns32 rc = NCSCC_RC_FAILURE;
	IMMSV_EVT send_evt;
	memset(&send_evt, '\0', sizeof(IMMSV_EVT));

	send_evt.type = IMMSV_EVT_TYPE_IMMD;
	send_evt.info.immd.type = IMMD_EVT_ND2D_ANNOUNCE_DUMP;
	send_evt.info.immd.info.ctrl_msg.ndExecPid = cb->mMyPid;
	send_evt.info.immd.info.ctrl_msg.epoch = cb->mMyEpoch;
	send_evt.info.immd.info.ctrl_msg.pbeEnabled = 
		cb->mPbeFile && (cb->mRim == SA_IMM_KEEP_REPOSITORY);
	if (immnd_is_immd_up(cb)) {
		rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, &send_evt);
	}
	/* Failure not handled. Should not be a problem. */
}

static SaInt32T immnd_syncNeeded(IMMND_CB *cb)
{
	/*if(immnd_iAmCoordinator(cb) != 1) {return 0;} Only coord can start sync
	   The fact that we are IMMND coord has laready been checked. */

	if (cb->mSyncRequested) {
		cb->mSyncRequested = 0;	/*reset sync request marker */
		return (cb->mMyEpoch + 1);
	}

	return 0;
}

static uns32 immnd_announceSync(IMMND_CB *cb, SaUint32T newEpoch)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	IMMSV_EVT send_evt;
	memset(&send_evt, '\0', sizeof(IMMSV_EVT));
	LOG_IN("Announce sync, epoch:%u", newEpoch);

	send_evt.type = IMMSV_EVT_TYPE_IMMD;
	send_evt.info.immd.type = IMMD_EVT_ND2D_SYNC_START;
	send_evt.info.immd.info.ctrl_msg.ndExecPid = cb->mMyPid;
	send_evt.info.immd.info.ctrl_msg.epoch = newEpoch;
	send_evt.info.immd.info.ctrl_msg.pbeEnabled = 
		cb->mPbeFile && (cb->mRim == SA_IMM_KEEP_REPOSITORY);

	if (immnd_is_immd_up(cb)) {
		rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, &send_evt);
	} else {
		rc = NCSCC_RC_FAILURE;
	}

	if (rc != NCSCC_RC_SUCCESS) {
		LOG_ER("Coord failed to send start of sync to IMMD");
		return 0;
	}

	cb->mPendSync = 1;	//Avoid starting the sync before everyone is readonly

	/*cb->mMyEpoch = newEpoch; */
	/* Wait, let immd decide if sync will be started */

	return 1;
}

void immnd_adjustEpoch(IMMND_CB *cb)
{
	TRACE_ENTER();
	/*Correct epoch for counter loaded from backup/sync
	   First check that current members agree ?? */

	int newEpoch = immModel_adjustEpoch(cb, cb->mMyEpoch);
	if (newEpoch != cb->mMyEpoch) {
		/*This case only relevant when persistent epoch overrides
		   last epoch, i.e. after reload at cluster start. */
		TRACE_5("Adjusting epoch to:%u", newEpoch);
		cb->mMyEpoch = newEpoch;
		if (cb->mRulingEpoch != newEpoch) {
			assert(cb->mRulingEpoch < newEpoch);
			cb->mRulingEpoch = newEpoch;
		}

	}
	assert(immnd_introduceMe(cb) == NCSCC_RC_SUCCESS);
	/* Convert to a test and postpone intro if we can note & do it later. */

	TRACE_LEAVE();
}

static void immnd_abortLoading(IMMND_CB *cb)
{
	TRACE_ENTER();
	/* Here we should send a fevs message informing all parties that the 
	   loading failed. */
	TRACE_LEAVE();
}

SaBoolT immnd_syncComplete(IMMND_CB *cb, SaBoolT coordinator, SaUint32T step)
{				/*Invoked by sync-coordinator and sync-clients.
				   Other old-member nodes do not invoke.
				*/

#if 0   /* Enable this code only to test logic for handling coord crash in
	   sync */
	if (coordinator && cb->mMyEpoch == 4) {
		LOG_NO("FAULT INJECTION crash during sync");
		exit(1);
	}
#endif

	SaBoolT completed;
	assert(cb->mSync || coordinator);
	assert(!(cb->mSync) || !coordinator);
	completed = immModel_syncComplete(cb);
	if (!completed) {
		/*Possibly a timeout with the help of step.
		   But sync failure is aborted from coord. */

	} else {		/*completed */
		if (cb->mSync) {
			cb->mSync = SA_FALSE;;

			/*cb->mAccepted = SA_TRUE; */
			/*BUGFIX this is too late! We arrive here not on fevs basis, 
			   but on timing basis from immnd_proc. */
			assert(cb->mAccepted);
		}
	}
	return completed;
}

static void immnd_abortSync(IMMND_CB *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	IMMSV_EVT send_evt;
	uns16 retryCount = 0;
	memset(&send_evt, '\0', sizeof(IMMSV_EVT));
	TRACE_ENTER();
	TRACE("ME:%u RE:%u", cb->mMyEpoch, cb->mRulingEpoch);
	assert(cb->mIsCoord);
	cb->mPendSync = 0;
	immModel_abortSync(cb);

	if (cb->mRulingEpoch == cb->mMyEpoch + 1) {
		cb->mMyEpoch = cb->mRulingEpoch;
	} else if (cb->mRulingEpoch != cb->mMyEpoch) {
		LOG_ER("immnd_abortSync not clean on epoch: RE:%u ME:%u", cb->mRulingEpoch, cb->mMyEpoch);
	}

	while (!immnd_is_immd_up(cb) && (retryCount++ < 20)) {
		LOG_WA("Coord blocked in sending ABORT_SYNC because IMMD is DOWN %u", retryCount);
		sleep(1);
	}

	immnd_adjustEpoch(cb);
	send_evt.type = IMMSV_EVT_TYPE_IMMD;
	send_evt.info.immd.type = IMMD_EVT_ND2D_SYNC_ABORT;
	send_evt.info.immd.info.ctrl_msg.ndExecPid = cb->mMyPid;
	send_evt.info.immd.info.ctrl_msg.epoch = cb->mMyEpoch;
	send_evt.info.immd.info.ctrl_msg.pbeEnabled = 
		cb->mPbeFile && (cb->mRim == SA_IMM_KEEP_REPOSITORY);

	LOG_NO("Coord broadcasting ABORT_SYNC, epoch:%u", cb->mRulingEpoch);
	rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, &send_evt);
	if (rc != NCSCC_RC_SUCCESS) {
		LOG_ER("Coord failed to send ABORT_SYNC over MDS err:%u", rc);
	}
	TRACE_LEAVE();
}

static void immnd_cleanTheHouse(IMMND_CB *cb, SaBoolT iAmCoordNow)
{
	SaInvocationT *admReqArr = NULL;
	SaUint32T admReqArrSize = 0;
	SaInvocationT *searchReqArr = NULL;
	SaUint32T searchReqArrSize = 0;
	SaUint32T *ccbIdArr = NULL;
	SaUint32T ccbIdArrSize = 0;

	SaInvocationT inv;
	SaUint32T reqConn = 0;
	SaUint32T dummyImplConn = 0;
	SaUint64T reply_dest = 0LL;
	unsigned int ix;
	SaImmHandleT tmp_hdl = 0LL;
	IMMND_IMM_CLIENT_NODE *cl_node = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;
	/*TRACE_ENTER(); */

	if (!immnd_is_immd_up(cb)) {
		return;
	}

	if(cb->fevs_out_list) {
		/* Push out any lingering buffered asyncronous feves messages. 
		   Normally pushed by arrival of reply over fevs.
		*/
		TRACE("cleanTheHouse push out buffered async fevs");
		dequeue_outgoing(cb); /* function body in immnd_evt.c */
	}

	immModel_cleanTheBasement(cb,
				  cb->mTimer,
				  &admReqArr,
				  &admReqArrSize,
				  &searchReqArr, &searchReqArrSize, &ccbIdArr, &ccbIdArrSize, iAmCoordNow);

	if (admReqArrSize) {
		/* TODO: Correct for explicit continuation handling in the 
		   new IMM standard. */
		IMMSV_EVT send_evt;
		memset(&send_evt, 0, sizeof(IMMSV_EVT));
		for (ix = 0; ix < admReqArrSize; ++ix) {
			inv = admReqArr[ix];
			reqConn = 0;
			immModel_fetchAdmOpContinuations(cb, inv, SA_FALSE, &dummyImplConn, &reqConn, &reply_dest);
			assert(reqConn);
			tmp_hdl = m_IMMSV_PACK_HANDLE(reqConn, cb->node_id);

			immnd_client_node_get(cb, tmp_hdl, &cl_node);
			if (cl_node == NULL || cl_node->mIsStale) {
				LOG_WA("IMMND - Client went down so no response");
				continue;
			}
			send_evt.type = IMMSV_EVT_TYPE_IMMA;
			send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
			send_evt.info.imma.info.admOpRsp.invocation = inv;
			send_evt.info.imma.info.errRsp.error = SA_AIS_ERR_TIMEOUT;

			SaInt32T subinv = m_IMMSV_UNPACK_HANDLE_LOW(inv);
			if (subinv < 0) {	//async-admin-op
				LOG_WA("Timeout on asyncronous admin operation %i", subinv);
				rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OM, cl_node->agent_mds_dest, &send_evt);
			} else {	//sync-admin-op
				LOG_WA("Timeout on syncronous admin operation %i", subinv);
				rc = immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt);
			}

			if (rc != NCSCC_RC_SUCCESS) {
				LOG_WA("Failure in sending reply for admin-op over MDS");
			}
		}

		free(admReqArr);
	}

	if (searchReqArrSize) {
		IMMSV_OM_RSP_SEARCH_REMOTE reply;
		memset(&reply, 0, sizeof(IMMSV_OM_RSP_SEARCH_REMOTE));
		reply.requestNodeId = cb->node_id;
		for (ix = 0; ix < searchReqArrSize; ++ix) {
			inv = searchReqArr[ix];
			reqConn = 0;
			/*Fetch originating request continuation */
			immModel_fetchSearchReqContinuation(cb, inv, &reqConn);
			assert(reqConn);

			reply.searchId = m_IMMSV_UNPACK_HANDLE_LOW(inv);

			reply.result = SA_AIS_ERR_TIMEOUT;
			LOG_WA("Timeout on search op waiting on implementer");
			search_req_continue(cb, &reply, reqConn);
		}
		free(searchReqArr);
	}

	if (ccbIdArrSize) {
		IMMSV_EVT send_evt;
		memset(&send_evt, '\0', sizeof(IMMSV_EVT));
		send_evt.type = IMMSV_EVT_TYPE_IMMD;
		send_evt.info.immd.type = IMMD_EVT_ND2D_ABORT_CCB;
		for (ix = 0; ix < ccbIdArrSize; ++ix) {
			send_evt.info.immd.info.ccbId = ccbIdArr[ix];
			LOG_WA("Timeout while waiting for implementer, aborting ccb:%u", ccbIdArr[ix]);

			if (immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id,
				    &send_evt) != NCSCC_RC_SUCCESS) {
				LOG_ER("Failure to broadcast discard Ccb for ccbId:%u", ccbIdArr[ix]);
				/* assert(0); */
			}
		}
		free(ccbIdArr);
	}

	/*TRACE_LEAVE(); */
}

void immnd_proc_global_abort_ccb(IMMND_CB *cb, SaUint32T ccbId)
{
	IMMSV_EVT send_evt;
	memset(&send_evt, '\0', sizeof(IMMSV_EVT));
	send_evt.type = IMMSV_EVT_TYPE_IMMD;
	send_evt.info.immd.type = IMMD_EVT_ND2D_ABORT_CCB;
	send_evt.info.immd.info.ccbId = ccbId;
	if (immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id,
		    &send_evt) != NCSCC_RC_SUCCESS) {
		LOG_ER("Failure to broadcast abort Ccb for ccbId:%u", ccbId);
	}
}


static SaBoolT immnd_ccbsTerminated(IMMND_CB *cb, SaUint32T step)
{
	assert(cb->mIsCoord);
	if (cb->mPendSync) {
		return SA_FALSE;
	}

	if (immModel_ccbsTerminated(cb)) {
		return SA_TRUE;
	}

	if (!(step % 50)) {	//Preemtively abort non critical Ccbs. 
		IMMSV_EVT send_evt;	//One try should be enough.
		SaUint32T *ccbIdArr = NULL;
		SaUint32T ccbIdArrSize = 0;
		memset(&send_evt, '\0', sizeof(IMMSV_EVT));
		send_evt.type = IMMSV_EVT_TYPE_IMMD;
		send_evt.info.immd.type = IMMD_EVT_ND2D_ABORT_CCB;
		int ix;
		immModel_getNonCriticalCcbs(cb, &ccbIdArr, &ccbIdArrSize);

		for (ix = 0; ix < ccbIdArrSize; ++ix) {
			send_evt.info.immd.info.ccbId = ccbIdArr[ix];
			LOG_WA("Aborting ccbId  %u to start sync", ccbIdArr[ix]);
			if (immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id,
					       &send_evt) != NCSCC_RC_SUCCESS) {
				LOG_ER("Failure to broadcast abort Ccb for ccbId:%u", ccbIdArr[ix]);
				break;	/* out of forloop to free ccbIdArr & return SA_FALSE */
			}
		}
		free(ccbIdArr);
	}

	return SA_FALSE;
}

static int immnd_forkLoader(IMMND_CB *cb)
{
	char loaderName[1024];
	const char *base = basename(cb->mProgName);
	int myLen = strlen(base);
	int myAbsLen = strlen(cb->mProgName);
	int loaderBaseLen = strlen(loaderBase);
	int pid = (-1);
	int i, j;

	TRACE_ENTER();
	if (!cb->mDir && !cb->mFile) {
		LOG_WA("No directory and no file-name=>IMM coming up empty");
		return (-1);
	}

	if ((myAbsLen - myLen + loaderBaseLen) > 1023) {
		LOG_ER("Pathname too long: %u max is 1023", myAbsLen - myLen + loaderBaseLen);
		return (-1);
	}

	for (i = 0; myAbsLen > myLen; ++i, --myAbsLen) {
		loaderName[i] = cb->mProgName[i];
	}
	for (j = 0; j < loaderBaseLen; ++i, ++j) {
		loaderName[i] = loaderBase[j];
	}
	loaderName[i] = 0;
	pid = fork();		/*posix fork */
	if (pid == (-1)) {
		LOG_ER("%s failed to fork, error %u", base, errno);
		return (-1);
	}
	if (pid == 0) {		/*Child */
		/* TODO Should close file-descriptors ... */
		char *ldrArgs[4] = { loaderName, (char *)(cb->mDir ? cb->mDir : ""),
			(char *)(cb->mFile ? cb->mFile : "imm.xml"), 0
		};

		TRACE_5("EXEC %s %s %s", ldrArgs[0], ldrArgs[1], ldrArgs[2]);
		execvp(loaderName, ldrArgs);
		LOG_ER("%s failed to exec, error %u", base, errno);
		exit(1);
	}
	TRACE_5("Parent %s, successfully forked loader, pid:%d", base, pid);
	TRACE_LEAVE();
	return pid;
}

static int immnd_forkSync(IMMND_CB *cb)
{
	char loaderName[1024];
	const char *base = basename(cb->mProgName);
	int myLen = strlen(base);
	int myAbsLen = strlen(cb->mProgName);
	int loaderBaseLen = strlen(loaderBase);
	int pid = (-1);
	int i, j;

	TRACE_ENTER();
	if ((myAbsLen - myLen + loaderBaseLen) > 1023) {
		LOG_ER("Pathname too long: %u max is 1023", myAbsLen - myLen + loaderBaseLen);
		return (-1);
	}

	for (i = 0; myAbsLen > myLen; ++i, --myAbsLen) {
		loaderName[i] = cb->mProgName[i];
	}

	for (j = 0; j < loaderBaseLen; ++i, ++j) {
		loaderName[i] = loaderBase[j];
	}
	loaderName[i] = 0;

	pid = fork();		/*posix fork */
	if (pid == (-1)) {
		LOG_ER("%s failed to fork sync-agent, error %u", base, errno);
		return (-1);
	}

	if (pid == 0) {		/*child */
		/* TODO: Should close file-descriptors ... */
		char *ldrArgs[5] = { loaderName, "", "", "sync", 0 };
		execvp(loaderName, ldrArgs);
		LOG_ER("%s failed to exec sync, error %u", base, errno);
		exit(1);
	}
	TRACE_5("Parent %s, successfully forked sync-agent, pid:%d", base, pid);
	TRACE_LEAVE();
	return pid;
}

static int immnd_forkPbe(IMMND_CB *cb)
{
	const char *base = basename(cb->mProgName);
	char pbePath[1024];
	int pid = (-1);
	int dirLen = strlen(cb->mDir);
	int pbeLen = strlen(cb->mPbeFile);
	int i, j;
	TRACE_ENTER();

	for (i = 0; i < dirLen; ++i) {
           pbePath[i] = cb->mDir[i];
	}

	pbePath[i++] = '/';

	for(j = 0; j < pbeLen; ++i, ++j) {
            pbePath[i] = cb->mPbeFile[j];
	}
	pbePath[i] = '\0';


	TRACE("pbe-file-path:%s", pbePath);

	pid = fork();		/*posix fork */
	if (pid == (-1)) {
		LOG_ER("%s failed to fork, error %u", base, errno);
		return (-1);
	}

	if (pid == 0) {		/*child */
		/* TODO: Should close file-descriptors ... */
		/*char * const pbeArgs[5] = { (char *) pbeBase, "--daemon", "--pbe", pbePath, 0 };*/
		char * pbeArgs[5];
		pbeArgs[0] = (char *) pbeBase;
		pbeArgs[1] =  "--daemon";
		pbeArgs[2] = "--pbe";
		pbeArgs[3] = pbePath;
		pbeArgs[4] =  0;

		LOG_IN("Trying to exec: %s %s %s %s", pbeArgs[0], pbeArgs[1], pbeArgs[2], pbeArgs[3]);
		execvp(pbeBase, pbeArgs);
		LOG_ER("%s failed to exec '%s -pbe', error %u", base, pbeBase, errno);
		exit(1);
	}
	TRACE_5("Parent %s, successfully forked %s, pid:%d", base, pbePath, pid);
	TRACE_LEAVE();
	return pid;
}

/**************************************************************************
 * Name     :  immnd_proc_server
 *
 * Description   : Function for IMMND internal periodic processing
 *
 * Arguments     : timeout IN/OUT - Current timeout period in milliseconds, can
 *                                 be altered by this function.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : Error return => IMMND task should end.
 *****************************************************************************/
uns32 immnd_proc_server(uns32 *timeout)
{
	IMMND_CB *cb = immnd_cb;
	uns32 rc = NCSCC_RC_SUCCESS;
	int32 coord, newEpoch;
	int32 printFrq = (*timeout > 100) ? 5 : 50;
	/*TRACE_ENTER(); */

	if ((cb->mTimer % printFrq) == 0) {
		TRACE_5("tmout:%u ste:%u ME:%u RE:%u crd:%u rim:%s",
			*timeout, cb->mState, cb->mMyEpoch, cb->mRulingEpoch, cb->mIsCoord,
			(cb->mRim==SA_IMM_KEEP_REPOSITORY)?
			"SA_IMM_KEEP_REPOSITORY":"SA_IMM_INIT_FROM_FILE");
	}

	if (cb->mState < IMM_SERVER_DUMP) {
		*timeout = 100;	/* 0.1 sec */
	} else {
		*timeout = 1000;	/* 1 sec */
	}

	switch (cb->mState) {
	case IMM_SERVER_ANONYMOUS:	/*send introduceMe msg. */
		/*TRACE_5("IMM_SERVER_ANONYMOUS");*/
		if (immnd_introduceMe(cb) == NCSCC_RC_SUCCESS) {
			cb->mTimer = 0;
			LOG_NO("SERVER STATE: IMM_SERVER_ANONYMOUS --> IMM_SERVER_CLUSTER_WAITING");
			cb->mState = IMM_SERVER_CLUSTER_WAITING;
		}

		if(cb->is_immd_up) {
			TRACE("IMMD IS UP");
		}

		break;

	case IMM_SERVER_CLUSTER_WAITING:
		TRACE_5("IMM_CLUSTER_WAITING");
		coord = immnd_iAmCoordinator(cb);
		if (coord != (-1)) {	/* (-1) Not ready; (1) coord; (0) non-coord */
			if (coord) {
				if ((cb->mNumNodes >= cb->mExpectedNodes) || (cb->mTimer >= 10 * (cb->mWaitSecs))) {
					cb->mState = IMM_SERVER_LOADING_PENDING;
					LOG_NO("SERVER STATE: IMM_SERVER_CLUSTER_WAITING -->"
					       " IMM_SERVER_LOADING_PENDING");
					cb->mTimer = 0;
				} else {
					TRACE_5("Nodes %u < %u || %u < %u", cb->mNumNodes,
						cb->mExpectedNodes, cb->mTimer, 10 * (cb->mWaitSecs));
				}
			} else {
				/*Non coordinator goes directly to loading pending */
				cb->mState = IMM_SERVER_LOADING_PENDING;
				LOG_NO("SERVER STATE: IMM_SERVER_CLUSTER_WAITING --> IMM_SERVER_LOADING_PENDING");
				cb->mTimer = 0;
			}
		} else {	/*We are not ready to start loading yet */
			if (!(cb->mIntroduced) && !(cb->mTimer % 100)) {
				LOG_WA("Resending introduce-me - problems with MDS ?");
				immnd_introduceMe(cb);
			}
		}

		if (cb->mTimer > 200) {	/* No progress in 20 secs */
			LOG_ER("Failed to load/sync. Giving up after %u seconds, restarting..", (cb->mTimer) / 10);
			rc = NCSCC_RC_FAILURE;	/*terminate. */
			immnd_ackToNid(rc);
		}
		break;

	case IMM_SERVER_LOADING_PENDING:
		TRACE_5("IMM_SERVER_LOADING_PENDING");
		newEpoch = immnd_iAmLoader(cb);
		if (newEpoch < 0) {	/*Loading is not possible */
			if (immnd_iAmCoordinator(cb) == 1) {
				LOG_ER("LOADING NOT POSSIBLE, CLUSTER DOES NOT AGREE ON EPOCH.. TERMINATING");
				rc = NCSCC_RC_FAILURE;
				immnd_ackToNid(rc);
			} else {
				/*Epoch == -2 means coord is available but epoch
				   missmatch => go to synch immediately.
				   Epoch == -1 means still waiting for coord. 
				   Be patient, give the coord 3 seconds to come up. */

				if (newEpoch == (-2) || cb->mTimer > 300) {
					/*Request to be synched instead. */
					LOG_IN("REQUESTING SYNC");
					if (immnd_requestSync(cb)) {
						LOG_NO("SERVER STATE: IMM_SERVER_LOADING_PENDING "
						       "--> IMM_SERVER_SYNC_PENDING");
						cb->mState = IMM_SERVER_SYNC_PENDING;
						cb->mTimer = 0;
					}
				}
			}
		} else {	/*Loading is possible */
			if (newEpoch) {
				/*Locks out stragglers */
				if (immnd_announceLoading(cb, newEpoch) == NCSCC_RC_SUCCESS) {
					LOG_NO("SERVER STATE: IMM_SERVER_LOADING_PENDING "
					       "--> IMM_SERVER_LOADING_SERVER");
					cb->mState = IMM_SERVER_LOADING_SERVER;
					cb->mTimer = 0;
				}
			} else {
				/*Non loader goes directly to loading client */
				LOG_NO("SERVER STATE: IMM_SERVER_LOADING_PENDING --> IMM_SERVER_LOADING_CLIENT");
				cb->mState = IMM_SERVER_LOADING_CLIENT;
				cb->mTimer = 0;
			}
		}
		break;

	case IMM_SERVER_SYNC_PENDING:
		if (cb->mSync) {
			/* Sync has started */
			cb->mTimer = 0;
			LOG_NO("SERVER STATE: IMM_SERVER_SYNC_PENDING --> IMM_SERVER_SYNC_CLIENT");
			cb->mState = IMM_SERVER_SYNC_CLIENT;
		} else {
			/* Sync has not started yet. */
			if (cb->mTimer > 110) {	/*This timeout value should not be
						   hard-coded. */
				LOG_WA("This node failed to be sync'ed. "
				       "Giving up after %u seconds, restarting..", cb->mTimer / 10);
				rc = NCSCC_RC_FAILURE;	/*terminate. */
				immnd_ackToNid(rc);
			}

			if (!(cb->mTimer % 60)) {
				LOG_IN("REQUESTING SYNC AGAIN %u", cb->mTimer);
				immnd_requestSync(cb);
			}

			if (!(cb->mTimer % 10)) {
				LOG_IN("This node still waiting to be sync'ed after %u seconds", cb->mTimer / 10);
			}
		}
		break;

	case IMM_SERVER_LOADING_SERVER:
		TRACE_5("IMM_SERVER_LOADING_SERVER");
		if (cb->mMyEpoch > cb->mRulingEpoch) {
			TRACE_5("Wait for permission to start loading "
				"My epoch:%u Ruling epoch:%u", cb->mMyEpoch, cb->mRulingEpoch);
			if (cb->mTimer > 150) {
				LOG_WA("MDS problem-2, giving up");
				rc = NCSCC_RC_FAILURE;	/*terminate. */
				immnd_ackToNid(rc);
			}
		} else {
			if (cb->loaderPid == (-1) && immModel_readyForLoading(cb)) {
				TRACE_5("START LOADING");
				cb->loaderPid = immnd_forkLoader(cb);
				if (cb->loaderPid > 0) {
					immModel_setLoader(cb, cb->loaderPid);
				} else {
					LOG_ER("Failed to fork loader :%u", cb->loaderPid);
					/* broadcast failure to load?
					   Not clear what I should do in this case.
					   Probably best to restart immnd
					   immModel_setLoader(cb, 0);
					 */
					immnd_abortLoading(cb);
					rc = NCSCC_RC_FAILURE;	/*terminate. */
					immnd_ackToNid(rc);
				}
			} else if (immModel_getLoader(cb) == 0) {	/*Success in loading */
				immnd_adjustEpoch(cb);
				cb->mState = IMM_SERVER_READY;
				immnd_ackToNid(NCSCC_RC_SUCCESS);
				LOG_NO("SERVER STATE: IMM_SERVER_LOADING_SERVER --> IMM_SERVER_READY");
				if (cb->mPbeFile) {/* Pbe enabled */
					cb->mRim = immModel_getRepositoryInitMode(cb);

					TRACE("RepositoryInitMode: %s", (cb->mRim==SA_IMM_KEEP_REPOSITORY)?
						"SA_IMM_KEEP_REPOSITORY":"SA_IMM_INIT_FROM_FILE");
				}
				/*Dont zero cb->loaderPid yet. 
				   It is used to control waiting for loader child. */
			} else {
				int status = 0;
				/* posix waitpid & WEXITSTATUS */
				if (waitpid(cb->loaderPid, &status, WNOHANG) > 0) {
					LOG_ER("LOADING APPARENTLY FAILED status:%i", WEXITSTATUS(status));
					cb->loaderPid = 0;
					/*immModel_setLoader(cb, 0); */
					immnd_abortLoading(cb);
					rc = NCSCC_RC_FAILURE;	/*terminate. */
					immnd_ackToNid(rc);
				}
			}
		}
		if (cb->mTimer > 600) {	/*Waited for loading for a whole minute */
			LOG_ER("LOADING IS APPARENTLY HUNG");
			cb->loaderPid = 0;
			/*immModel_setLoader(cb, 0); */
			immnd_abortLoading(cb);
			rc = NCSCC_RC_FAILURE;	/*terminate. */
			immnd_ackToNid(rc);
		}

		break;

	case IMM_SERVER_LOADING_CLIENT:
		TRACE_5("IMM_SERVER_LOADING_CLIENT");
		if (cb->mTimer > (cb->mWaitSecs ? ((cb->mWaitSecs) * 10 + 150) : 150)) {
			LOG_WA("Loading client timed out, waiting to be loaded - terminating");
			cb->mTimer = 0;
			/*immModel_setLoader(cb,0); */
			/*cb->mState = IMM_SERVER_ANONYMOUS; */
			rc = NCSCC_RC_FAILURE;	/*terminate. */
			immnd_ackToNid(rc);
		}
		if (immModel_getLoader(cb) == 0) {
			immnd_adjustEpoch(cb);
			immnd_ackToNid(NCSCC_RC_SUCCESS);
			cb->mState = IMM_SERVER_READY;
			LOG_NO("SERVER STATE: IMM_SERVER_LOADING_CLIENT --> IMM_SERVER_READY");
			if (cb->mPbeFile) {/* Pbe enabled */
				cb->mRim = immModel_getRepositoryInitMode(cb);

				TRACE("RepositoryInitMode: %s", (cb->mRim==SA_IMM_KEEP_REPOSITORY)?
					"SA_IMM_KEEP_REPOSITORY":"SA_IMM_INIT_FROM_FILE");
			}
			/*
			   This code case duplicated in immnd_evt.c
			   Search for: "ticket:#598"
			 */
		}
		break;

	case IMM_SERVER_SYNC_CLIENT:
		TRACE_5("IMM_SERVER_SYNC_CLIENT");
		if (immnd_syncComplete(cb, SA_FALSE, cb->mTimer)) {
			cb->mTimer = 0;
			cb->mState = IMM_SERVER_READY;
			immnd_ackToNid(NCSCC_RC_SUCCESS);
			LOG_NO("SERVER STATE: IMM_SERVER_SYNC_CLIENT --> IMM SERVER READY");
			if (cb->mPbeFile) {/* Pbe enabled */
				cb->mRim = immModel_getRepositoryInitMode(cb);

				TRACE("RepositoryInitMode: %s", (cb->mRim==SA_IMM_KEEP_REPOSITORY)?
					"SA_IMM_KEEP_REPOSITORY":"SA_IMM_INIT_FROM_FILE");
			}
			/*
			   This code case duplicated in immnd_evt.c
			   Search for: "ticket:#599"
			 */
		} else if (immnd_iAmCoordinator(cb) == 1) {
			/*This case is weird. It can only happen if the last
			   operational immexec crashed during this sync. 
			   In this case we may becomme the coordinator even though
			   we are not loaded or synced. We should in this case becomme
			   the loader. */
			LOG_WA("Coordinator apparently crashed during sync. Aborting the sync.");
			rc = NCSCC_RC_FAILURE;	/*terminate. */
			immnd_ackToNid(rc);
		}
		break;

	case IMM_SERVER_SYNC_SERVER:
		if (!immnd_ccbsTerminated(cb, cb->mTimer)) {
			/*Phase 1 */
			if (!(cb->mTimer % 10)) {
				LOG_IN("Sync Phase-1, waiting for existing "
				       "Ccbs to terminate, seconds waited: %u", cb->mTimer / 10);
			}
			if (cb->mTimer > 200) {
				LOG_IN("Still waiting for existing Ccbs to terminate "
				       "after %u seconds. Aborting this sync attempt", cb->mTimer / 10);
				immnd_abortSync(cb);
				assert(cb->syncPid == 0);
				cb->mTimer = 0;
				cb->mState = IMM_SERVER_READY;
				LOG_NO("SERVER STATE: IMM_SERVER_SYNC_SERVER --> IMM SERVER READY");
			}
		} else {
			/*Phase 2 */
			if (cb->syncPid <= 0) {
				/*Fork sync-agent */
				cb->syncPid = immnd_forkSync(cb);
				if (cb->syncPid <= 0) {
					LOG_ER("Failed to fork sync process");
					cb->syncPid = 0;
					cb->mTimer = 0;
					cb->mState = IMM_SERVER_READY;
					immnd_abortSync(cb);
					LOG_NO("SERVER STATE: IMM_SERVER_SYNC_SERVER --> IMM SERVER READY");
				} else {
					LOG_IN("Sync Phase-2: Ccbs are terminated, IMM in "
					       "read-only mode, forked sync process pid:%u", cb->syncPid);
				}
			} else {
				/*Phase 3 */
				if (!(cb->mTimer % 10)) {
					LOG_IN("Sync Phase-3 time:%u", cb->mTimer/10);
				}
				if (immnd_syncComplete(cb, TRUE, cb->mTimer)) {
					cb->mTimer = 0;
					cb->mState = IMM_SERVER_READY;
					LOG_NO("SERVER STATE: IMM_SERVER_SYNC_SERVER --> IMM SERVER READY");
				} else {
					int status = 0;
					if (waitpid(cb->syncPid, &status, WNOHANG) > 0) {
						LOG_ER("SYNC APPARENTLY FAILED status:%i", WEXITSTATUS(status));
						cb->syncPid = 0;
						cb->mTimer = 0;
						LOG_NO("-SERVER STATE: IMM_SERVER_SYNC_SERVER --> IMM_SERVER_READY");
						cb->mState = IMM_SERVER_READY;
						immnd_abortSync(cb);
					}
				}
			}
		}
		break;

	case IMM_SERVER_READY:	/*Normal stable service state. */
		/* Do some periodic housekeeping */

		if (cb->loaderPid > 0) {
			int status = 0;
			if (waitpid(cb->loaderPid, &status, WNOHANG) > 0) {
				/*Expected termination of loader process.
				   Should check status.
				   But since loading apparently succeeded, we dont care. */
				cb->loaderPid = 0;
			}
		}

		if (cb->syncPid > 0) {
			int status = 0;
			if (waitpid(cb->syncPid, &status, WNOHANG) > 0) {
				/*Expected termination of sync process.
				   Should check status.
				   But since sync apparently succeeded, we dont care. */
				cb->syncPid = 0;
			}
		}

		if (cb->pbePid > 0) {
			int status = 0;
			if (waitpid(cb->pbePid, &status, WNOHANG) > 0) {
				LOG_WA("Persistent back-end process has apparently died.");
				cb->pbePid = 0;
			}
		}

		coord = immnd_iAmCoordinator(cb);

		immnd_cleanTheHouse(cb, coord == 1);

		if ((coord == 1) && (cb->mTimer > 1)) {
			/*Every sec but first time after 2 secs. */
			if (immModel_immNotWritable(cb)) {
				/*Ooops we have apparently taken over the role of IMMND
				  coordinator during an uncompleted sync. Probably due 
				  to coordinator crash. Abort the sync. */
				LOG_WA("ABORTING UNCOMPLETED SYNC - COORDINATOR MUST HAVE CRASHED");
				immnd_abortSync(cb);
			} else {
				newEpoch = immnd_syncNeeded(cb);
				if (newEpoch) {
					if (cb->syncPid > 0) {
						LOG_WA("Will not start new sync when previous "
							"sync process (%u) has not terminated", 
							cb->syncPid);
					} else {
						if (immnd_announceSync(cb, newEpoch)) {
							LOG_NO("SERVER STATE: IMM_SERVER_READY -->"
								" IMM_SERVER_SYNC_SERVER");
							cb->mState = IMM_SERVER_SYNC_SERVER;
							cb->mTimer = 0;
							*timeout = 100;	/* 0.1 sec */
						}
					}
				}
			}
			/* Independently of aborting or coordinating sync, 
			   check if we should be starting/stopping persistent back-end.*/
			if (cb->mPbeFile) {/* Pbe enabled */
				/* TODO more efficient, only check if pbePid is zero. 
				   Move rim to cb and only fetch after ccb-apply. 
				 */
				/*cb->mRim = immModel_getRepositoryInitMode(cb); Should not be needed */

				if (cb->pbePid == 0) { /* Pbe is NOT running */
					if (cb->mRim == SA_IMM_KEEP_REPOSITORY) {/* Pbe SHOULD run. */
						LOG_NO("STARTING persistent back end process.");
						cb->pbePid = immnd_forkPbe(cb);
					}
				} else {
					assert(cb->pbePid > 0); /* Pbe is running. */
					if (cb->mRim == SA_IMM_INIT_FROM_FILE) {/* Pbe should NOT run.*/
						LOG_NO("STOPPING persistent back end process.");
						kill(cb->pbePid, SIG_TERM);
					}
				}
			}
		} /* if((coord == 1)...*/
		break;

	default:
		LOG_ER("Illegal state in ImmServer:%u", cb->mState);
		assert(0);
		break;
	}

	++(cb->mTimer);

	/*TRACE_LEAVE(); */
	return rc;
}

/****************************************************************************
 * Name          : immnd_cb_dump
 *
 * Description   : This is the function dumps the contents of the control block. *
 * Arguments     : immnd_cb     -  Pointer to the control block
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
void immnd_cb_dump(void)
{
	IMMND_CB *cb = immnd_cb;
	IMMND_IMM_CLIENT_NODE *cl_node = NULL;

	TRACE_5("************ IMMND CB Details *************** ");

	TRACE_5("**** Global Cb Details ***************\n");
	TRACE_5("IMMND MDS dest - %x", m_NCS_NODE_ID_FROM_MDS_DEST(cb->immnd_mdest_id));
	if (cb->is_immd_up) {
		TRACE_5("IMMD is UP  ");
		TRACE_5("IMMD MDS dest - %x", m_NCS_NODE_ID_FROM_MDS_DEST(cb->immd_mdest_id));
	} else
		TRACE_5("IMMD is DOWN ");

	TRACE_5("***** Start of Client Details ***************\n");
	immnd_client_node_getnext(cb, 0, &cl_node);
	/*
	   while ( cl_node != NULL)
	   {
	   SaImmHandleT prev_cl_hdl;

	   immnd_dump_client_info(cl_node);

	   while(imm_list != NULL) 
	   {
	   immnd_dump_ckpt_info(imm_list->cnode);
	   imm_list=imm_list->next;
	   }
	   prev_cl_hdl = cl_node->imm_app_hdl;
	   immnd_client_node_getnext(cb,prev_cl_hdl,&cl_node);
	   }
	 */
	TRACE_5("***** End of Client Details ***************\n");
}

/****************************************************************************
 * Name          : immnd_dump_client_info
 *
 * Description   : This is the function dumps the Ckpt Client info
 * Arguments     : cl_node  -  Pointer to the IMMND_IMM_CLIENT_NODE
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
#if 0				/*Only for debug */
static
void immnd_dump_client_info(IMMND_IMM_CLIENT_NODE *cl_node)
{
	TRACE_5("++++++++++++++++++++++++++++++++++++++++++++++++++");

	TRACE_5("Client_hdl  %u\t MDS DEST %x ",
		(uns32)(cl_node->imm_app_hdl), (uns32)m_NCS_NODE_ID_FROM_MDS_DEST(cl_node->agent_mds_dest));
	TRACE_5("++++++++++++++++++++++++++++++++++++++++++++++++++");
}

#endif
