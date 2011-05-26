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

  This file contains the routines to handle the callback queue mgmt
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include <logtrace.h>

#include "gla.h"
#define NCS_SAF_MIN_ACCEPT_TIME  10

/****************************************************************************
  Name          : saLckInitialize
  
  Description   : This function initializes the Global Lock Service for the 
                  invoking process and registers the various callback functions.
 
  Arguments     : client_info - pointer to the client info
                   
  Return Values : status operation
 
  Notes         : None
******************************************************************************/
SaAisErrorT saLckInitialize(SaLckHandleT *lckHandle, const SaLckCallbacksT *lckCallbacks, SaVersionT *version)
{
	GLSV_GLND_EVT initialize_evt;
	GLA_CB *gla_cb = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	uns32 ret;
	uns16 reg_info = 0;
	GLSV_GLA_EVT *out_evt = NULL;
	GLA_CLIENT_INFO *client_info = NULL;

	TRACE_ENTER();

	rc = ncs_agents_startup();
	if (rc != SA_AIS_OK)
		return SA_AIS_ERR_LIBRARY;

	rc = ncs_gla_startup();
	if (rc != SA_AIS_OK) {
		ncs_agents_shutdown();
		return SA_AIS_ERR_LIBRARY;
	}

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto err;
	}

	/* validation of input */
	if (lckHandle == NULL || version == NULL) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto err;
	}

	/* validate the version */
	if (!m_GLA_VER_IS_VALID(version)) {
		m_LOG_GLA_HEADLINE(GLA_VERSION_INCOMPATIBLE, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		rc = SA_AIS_ERR_VERSION;
		goto err;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		rc = SA_AIS_ERR_TRY_AGAIN;
		goto err;
	}

	/* initialise the handle */
	*lckHandle = 0;

	/* compute the reg_info */
	if (lckCallbacks != NULL) {
		reg_info |= (uns16)((lckCallbacks->saLckResourceOpenCallback) ? GLSV_LOCK_OPEN_CBK_REG : 0);
		reg_info |= (uns16)((lckCallbacks->saLckLockGrantCallback) ? GLSV_LOCK_GRANT_CBK_REG : 0);
		reg_info |= (uns16)((lckCallbacks->saLckLockWaiterCallback) ? GLSV_LOCK_WAITER_CBK_REG : 0);
		reg_info |= (uns16)((lckCallbacks->saLckResourceUnlockCallback) ? GLSV_LOCK_UNLOCK_CBK_REG : 0);
	}

	/* populate the structure */
	memset(&initialize_evt, 0, sizeof(GLSV_GLND_EVT));
	initialize_evt.type = GLSV_GLND_EVT_INITIALIZE;
	initialize_evt.info.client_info.agent_mds_dest = gla_cb->gla_mds_dest;
	initialize_evt.info.client_info.client_proc_id = getpid();
	initialize_evt.info.client_info.version = *version;
	initialize_evt.info.client_info.cbk_reg_info = reg_info;

	/* send the request to the GLND */
	if ((ret = gla_mds_msg_sync_send(gla_cb, &initialize_evt, &out_evt, GLA_API_RESP_TIME)) != NCSCC_RC_SUCCESS) {
		if (ret == NCSCC_RC_REQ_TIMOUT)
			rc = SA_AIS_ERR_TIMEOUT;
		else
			m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
					    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest),
					    GLSV_GLND_EVT_INITIALIZE);
		goto err;
	}
	rc = out_evt->error;
	if (rc == SA_AIS_OK) {
		/* create the client node and populate it */
		client_info = gla_client_tree_find_and_add(gla_cb, out_evt->handle, TRUE);
		if (client_info == NULL) {
			m_LOG_GLA_HEADLINE(GLA_CLIENT_TREE_ADD_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
			rc = SA_AIS_ERR_NO_RESOURCES;
			goto err;
		}

		/* copy the callbacks */
		if (lckCallbacks)
			memcpy((void *)&client_info->lckCallbk, (void *)lckCallbacks, sizeof(SaLckCallbacksT));
		*lckHandle = out_evt->handle;
		m_MMGR_FREE_GLA_EVT(out_evt);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		version->releaseCode = REQUIRED_RELEASECODE;
		version->majorVersion = REQUIRED_MAJORVERSION;
		version->minorVersion = REQUIRED_MINORVERSION;
		TRACE_LEAVE();
		return rc;
	}

 err:
	if (version) {
		/* Note: The logic to highest and least release code support can go in future 
		   as of now - there is only support for B.01.03 */
		version->releaseCode = REQUIRED_RELEASECODE;
		version->majorVersion = REQUIRED_MAJORVERSION;
		version->minorVersion = REQUIRED_MINORVERSION;
	}

	if (rc == SA_AIS_ERR_TRY_AGAIN)
		m_LOG_GLA_API(GLA_API_LCK_INITIALIZE_FAIL, NCSFL_SEV_INFO, __FILE__, __LINE__);
	else
		m_LOG_GLA_API(GLA_API_LCK_INITIALIZE_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);

	/* free the client node */
	if (client_info) {
		m_NCS_LOCK(&gla_cb->cb_lock, NCS_LOCK_WRITE);
		gla_client_tree_delete_node(gla_cb, client_info, FALSE);
		m_NCS_UNLOCK(&gla_cb->cb_lock, NCS_LOCK_WRITE);
	}

	/* clear up the out evt */
	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	/* return GLA CB */
	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	TRACE("saLckInitialize Failed");

	ncs_gla_shutdown();
	ncs_agents_shutdown();

	return rc;
}

/****************************************************************************
  Name          : saLckSelectionObjectGet
 
  Description   : This function creates & returns the operating system handle 
                  associated with the lock Handle.
 
  Arguments     : hdl       - LCK handle
                  o_sel_obj - ptr to the selection object
 
  Return Values : Error status
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckSelectionObjectGet(SaLckHandleT lckHandle, SaSelectionObjectT *o_sel_obj)
{
	GLA_CB *gla_cb = 0;
	GLA_CLIENT_INFO *client_info = NULL;
	SaAisErrorT rc = SA_AIS_OK;

	TRACE("saLckSelectionObjectGet Called with Handle %" PRIu64, (uns64)lckHandle);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* validate the input */
	if (o_sel_obj == NULL) {
		/* shashi --- #7 changed  from ERR_LIBRARY to INVALID param */
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, lckHandle, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* everything's fine.. pass the sel obj to the appl */
	*o_sel_obj = (SaSelectionObjectT)m_GET_FD_FROM_SEL_OBJ(m_NCS_IPC_GET_SEL_OBJ(&client_info->callbk_mbx));

 done:
	/* return GLA CB */
	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK || *o_sel_obj <= 0)
		m_LOG_GLA_API(GLA_API_LCK_SELECTION_OBJECT_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);

	if (rc == SA_AIS_OK)
		TRACE("saLckSelectionObjectGet SUCCESS");
	else
		TRACE("saLckSelectionObjectGet FAILURE");
	return rc;
}

/****************************************************************************
  Name          : saLckOptionCheck
 
  Description   : This function returns the optional features supported
                  by the Global Lock Service
 
  Arguments     : hdl - Lck handle
                  lckOptions - Locks options support.
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckOptionCheck(SaLckHandleT hdl, SaLckOptionsT *lckOptions)
{
	GLA_CB *gla_cb = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	GLA_CLIENT_INFO *client_info = NULL;

	TRACE("saLckOptionCheck Called with Handle %" PRIu64, (uns64)hdl);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}

	/* shashi --- #3 check added for lckOption Invalid param */

	if (NULL == lckOptions) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;

	}
	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, hdl, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* populate the options - as this implementation support both Deadlock and orphan ,
	   set the values */
	*lckOptions = SA_LCK_OPT_ORPHAN_LOCKS | SA_LCK_OPT_DEADLOCK_DETECTION;
	rc = SA_AIS_OK;

 done:
	/* return GLA CB */

	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK) {
		m_LOG_GLA_API(GLA_API_LCK_OPTIONS_CHECK_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);
	}

	if (rc == SA_AIS_OK)
		TRACE("saLckOptionCheck SUCCESS");
	else
		TRACE("saLckOptionCheck FAILURE");
	return rc;
}

/****************************************************************************
  Name          : saLckDispatch
 
  Description   : This function invokes, in the context of the calling thread,
                  the next pending callback for the LCK handle.
 
  Arguments     : hdl   - LCK handle
                  flags - flags that specify the callback execution behavior
                  of the saLckDispatch() function,
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckDispatch(SaLckHandleT lckHandle, const SaDispatchFlagsT flags)
{
	GLA_CB *gla_cb = 0;
	GLA_CLIENT_INFO *client_info = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;

	TRACE("saLckDispatch Called with Handle %" PRIu64, (uns64)lckHandle);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}
	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, lckHandle, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	switch (flags) {
	case SA_DISPATCH_ONE:
		rc = gla_hdl_callbk_dispatch_one(gla_cb, client_info);
		break;

	case SA_DISPATCH_ALL:
		rc = gla_hdl_callbk_dispatch_all(gla_cb, client_info);
		break;

	case SA_DISPATCH_BLOCKING:
		rc = gla_hdl_callbk_dispatch_block(gla_cb, client_info);
		break;

	default:
		rc = SA_AIS_ERR_INVALID_PARAM;
		break;
	}			/* switch */

 done:
	/* return GLA CB */
	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK)
		m_LOG_GLA_API(GLA_API_LCK_DISPATCH_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);

	if (rc == SA_AIS_OK)
		TRACE("saLckDispatch Called SUCCESS");
	else
		TRACE("saLckDispatch Called FAILURE");
	return rc;
}

/****************************************************************************
  Name          : saLckFinalize
 
  Description   : This function closes the association, represented by the 
                  LCK handle, between the invoking process and the LCK.
 
  Arguments     : hdl - Lck handle
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckFinalize(SaLckHandleT hdl)
{
	GLA_CB *gla_cb = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	GLSV_GLND_EVT finalize_evt;
	GLSV_GLA_EVT *out_evt = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	uns32 ret;

	TRACE("SaLckFinalize Called with Handle %" PRIu64, (uns64)hdl);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		return rc;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, hdl, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	/* populate the structure */
	memset(&finalize_evt, 0, sizeof(GLSV_GLND_EVT));
	finalize_evt.type = GLSV_GLND_EVT_FINALIZE;
	finalize_evt.info.finalize_info.agent_mds_dest = gla_cb->gla_mds_dest;
	finalize_evt.info.finalize_info.handle_id = hdl;

	/* send the request to the GLND */
	if ((ret = gla_mds_msg_sync_send(gla_cb, &finalize_evt, &out_evt, GLA_API_RESP_TIME)) != NCSCC_RC_SUCCESS) {
		if (ret == NCSCC_RC_REQ_TIMOUT)
			rc = SA_AIS_ERR_TIMEOUT;
		else
			m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
					    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_FINALIZE);
		goto done;
	}
	rc = out_evt->error;

	if (rc == SA_AIS_OK) {
		/* cleanup all the client info */
		m_NCS_LOCK(&gla_cb->cb_lock, NCS_LOCK_WRITE);

		/* delete the client resource tree */
		gla_client_res_tree_destroy(client_info);

		gla_client_tree_delete_node(gla_cb, client_info, TRUE);

		/* clean up all the resource handles that are linked with the client */
		gla_res_tree_cleanup_client_down(gla_cb, hdl);
		gla_lock_tree_cleanup_client_down(gla_cb, hdl);
		client_info = NULL;
		m_NCS_UNLOCK(&gla_cb->cb_lock, NCS_LOCK_WRITE);
	}

 done:

	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	/* return GLA CB */
	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK) {
		m_LOG_GLA_API(GLA_API_LCK_FINALIZE_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);
	}

	if (rc == SA_AIS_OK)
		TRACE("SaLckFinalize SUCCESS");
	else
		TRACE("SaLckFinalize FAILURE");

	if (rc == SA_AIS_OK) {
		ncs_gla_shutdown();
		ncs_agents_shutdown();
	}

	return rc;
}

/****************************************************************************
  Name          : saLckResourceOpen
 
  Description   : This function creates the resources for lock operations 
 
  Arguments     : hdl   - LCK handle
                  lockname - name of the lock resource.
                  timeout - timeout
                  resourceid - value returned as the output.
 
  Return Values : Refer to SAI-AIS specification for various return values.
 
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckResourceOpen(SaLckHandleT lckHandle,
			      const SaNameT *lockResourceName,
			      SaLckResourceOpenFlagsT resourceFlags,
			      SaTimeT timeout, SaLckResourceHandleT *lockResourceHandle)
{
	GLA_CB *gla_cb = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	GLSV_GLND_EVT res_open_evt;
	GLSV_GLA_EVT *out_evt = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	SaTimeT gla_timeout;
	uns32 ret;
	GLA_CLIENT_RES_INFO *client_res_info = NULL;

	/* validate the inputs */
	if (lockResourceName == NULL || lockResourceHandle == NULL) {
		/* shashi --- #6 Added Invalid error code */
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* SA_AIS_ERR_INVALID_PARAM, bullet 5 in SAI-AIS-LCK-A.02.01 
	   Section 3.5.1 saLckResourceOpen() and saLckResourceOpenAsync(), Return Values */
	if (strncmp((const char *)lockResourceName->value, "safLock=", 8) != 0) {
		TRACE("\"safLock=\" is missing in ResourceName");
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	m_GLSV_MEMSET_SANAME(lockResourceName);

	if (!(resourceFlags == SA_LCK_RESOURCE_CREATE || resourceFlags == 0)) {
		rc = SA_AIS_ERR_BAD_FLAGS;
		goto done;
	}

	TRACE("saLckResourceOpen Called with Handle %" PRIu64 " and Name %.7s", (uns64)lckHandle, lockResourceName->value);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}
	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, lckHandle, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	/* populate the evt */
	memset(&res_open_evt, 0, sizeof(GLSV_GLND_EVT));
	res_open_evt.type = GLSV_GLND_EVT_RSC_OPEN;
	res_open_evt.info.rsc_info.client_handle_id = lckHandle;
	res_open_evt.info.rsc_info.call_type = GLSV_SYNC_CALL;
	res_open_evt.info.rsc_info.resource_name.length = lockResourceName->length;
	memcpy(&res_open_evt.info.rsc_info.resource_name.value, &lockResourceName->value, lockResourceName->length);
	res_open_evt.info.rsc_info.agent_mds_dest = gla_cb->gla_mds_dest;
	res_open_evt.info.rsc_info.timeout = timeout;
	res_open_evt.info.rsc_info.flag = resourceFlags;

	/* convert the timeout to 10 ms value and add it to the sync send timeout */
	gla_timeout = m_GLSV_CONVERT_SATIME_TEN_MILLI_SEC(timeout);
	if (gla_timeout < NCS_SAF_MIN_ACCEPT_TIME) {
		rc = SA_AIS_ERR_TIMEOUT;
		goto done;
	}

	/* send the event */
	if ((ret = gla_mds_msg_sync_send(gla_cb, &res_open_evt, &out_evt, (uns32)gla_timeout)) != NCSCC_RC_SUCCESS) {
		if (ret == NCSCC_RC_REQ_TIMOUT)
			rc = SA_AIS_ERR_TIMEOUT;
		else
			m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
					    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_RSC_OPEN);
		goto done;
	}

	/* got the reply... do the needful */
	rc = out_evt->error;
	*lockResourceHandle = out_evt->info.gla_resp_info.param.res_open.resourceId;
	if (rc == SA_AIS_OK) {
		GLA_RESOURCE_ID_INFO *res_id_node;
		/* allocate the local resource node and add the values */
		res_id_node = gla_res_tree_find_and_add(gla_cb, 0, TRUE);
		if (res_id_node) {
			res_id_node->gbl_res_id = *lockResourceHandle;
			res_id_node->lock_handle_id = lckHandle;
			*lockResourceHandle = res_id_node->lcl_res_id;

			client_res_info = gla_client_res_tree_find_and_add(client_info, res_id_node->gbl_res_id, FALSE);
			if (client_res_info)
				client_res_info->lcl_res_cnt++;
			else {
				client_res_info =
				    gla_client_res_tree_find_and_add(client_info, res_id_node->gbl_res_id, TRUE);
				if (client_res_info)
					client_res_info->lcl_res_cnt++;
			}

		}
	}

 done:
	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	/* return GLA CB */
	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK) {
		if (rc == SA_AIS_ERR_TRY_AGAIN)
			m_LOG_GLA_API(GLA_API_LCK_RESOURCE_OPEN_SYNC_FAIL, NCSFL_SEV_INFO, __FILE__, __LINE__);
		else
			m_LOG_GLA_API(GLA_API_LCK_RESOURCE_OPEN_SYNC_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);
	}
	if (rc == SA_AIS_OK)
		TRACE("saLckResourceOpen SUCCESS Res_id %" PRIu64, (uns64)*lockResourceHandle);
	else
		TRACE("saLckResourceOpen FAILURE");
	return rc;
}

/****************************************************************************
  Name          : saLckResourceOpenAsync

  Description   : This function creates the resources for lock operations 
  
  Arguments     : hdl   - LCK handle
                  invocation - Value to identify the call.
                  lockname - name of the lock resource.
    
  Return Values : Refer to SAI-AIS specification for various return values.
       
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckResourceOpenAsync(SaLckHandleT lckHandle,
				   SaInvocationT invocation,
				   const SaNameT *lockResourceName, SaLckResourceOpenFlagsT resourceFlags)
{
	GLA_CB *gla_cb = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	GLSV_GLND_EVT res_open_evt;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;

	/* validate the inputs */
	if (lockResourceName == NULL) {
		/* shashi --- #8 INVALID PARAM */
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* SA_AIS_ERR_INVALID_PARAM, bullet 5 in SAI-AIS-LCK-A.02.01 
	   Section 3.5.1 saLckResourceOpen() and saLckResourceOpenAsync(), Return Values */
	if (strncmp((const char *)lockResourceName->value, "safLock=", 8) != 0) {
		TRACE("\"safLock=\" is missing in ResourceName");
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	if (!(resourceFlags == SA_LCK_RESOURCE_CREATE || resourceFlags == 0)) {
		rc = SA_AIS_ERR_BAD_FLAGS;
		goto done;
	}

	TRACE("saLckResourceOpenAsync Called with Handle %" PRIu64 " and Name %s", (uns64)lckHandle, lockResourceName->value);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}
	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, lckHandle, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check to see if the grant callback was registered */
	if (!client_info->lckCallbk.saLckResourceOpenCallback) {
		rc = SA_AIS_ERR_INIT;
		goto done;
	}
	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	GLA_RESOURCE_ID_INFO *res_id_node;
	/*Allocate the local resource id node and add the values */
	res_id_node = gla_res_tree_find_and_add(gla_cb, 0, TRUE);
	if (!res_id_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	res_id_node->lock_handle_id = lckHandle;

	res_id_node->res_async_tmr.client_hdl = lckHandle;
	res_id_node->res_async_tmr.clbk_info.callback_type = GLSV_LOCK_RES_OPEN_CBK;
	res_id_node->res_async_tmr.clbk_info.resourceId = res_id_node->lcl_res_id;;
	res_id_node->res_async_tmr.clbk_info.invocation = invocation;

	/* populate the evt */
	memset(&res_open_evt, 0, sizeof(GLSV_GLND_EVT));
	res_open_evt.type = GLSV_GLND_EVT_RSC_OPEN;
	res_open_evt.info.rsc_info.client_handle_id = lckHandle;
	res_open_evt.info.rsc_info.lcl_resource_id = res_id_node->lcl_res_id;
	res_open_evt.info.rsc_info.call_type = GLSV_ASYNC_CALL;
	memcpy(&res_open_evt.info.rsc_info.resource_name, lockResourceName, sizeof(SaNameT));
	res_open_evt.info.rsc_info.agent_mds_dest = gla_cb->gla_mds_dest;
	res_open_evt.info.rsc_info.invocation = invocation;
	res_open_evt.info.rsc_info.flag = resourceFlags;

	/* start the timer anyway */
	gla_start_tmr(&res_id_node->res_async_tmr);

	/* send the event */
	if (gla_mds_msg_async_send(gla_cb, &res_open_evt) != NCSCC_RC_SUCCESS) {
		gla_stop_tmr(&res_id_node->res_async_tmr);
		m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
				    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_RSC_OPEN);
		goto done;
	}
	rc = SA_AIS_OK;

 done:
	/* return GLA CB */
	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK) {
		if (rc == SA_AIS_ERR_TRY_AGAIN)
			m_LOG_GLA_API(GLA_API_LCK_RESOURCE_OPEN_ASYNC_FAIL, NCSFL_SEV_INFO, __FILE__, __LINE__);
		else
			m_LOG_GLA_API(GLA_API_LCK_RESOURCE_OPEN_ASYNC_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);
	}

	if (rc == SA_AIS_OK)
		TRACE("saLckResourceOpenAsync SUCCESS");
	else
		TRACE("saLckResourceOpenAsync FAILURE");

	return rc;
}

/****************************************************************************
  Name          : saLckResourceClose

  Description   : This function closes the resources 
  
  Arguments     : hdl   - LCK handle
                  resourceId - identifier of the resource.
    
  Return Values : Refer to SAI-AIS specification for various return values.
       
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckResourceClose(SaLckResourceHandleT lockResourceHandle)
{
	GLA_CB *gla_cb = NULL;
	GLSV_GLND_EVT res_close_evt;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	GLSV_GLA_EVT *out_evt = NULL;
	GLA_RESOURCE_ID_INFO *res_id_info = NULL;
	uns32 ret;
	GLA_CLIENT_INFO *client_info = NULL;
	GLA_CLIENT_RES_INFO *client_res_info = NULL;

	TRACE("saLckResourceClose Called with Res_handle %" PRIu64, (uns64)lockResourceHandle);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (res_id_info = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lockResourceHandle))) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, res_id_info->lock_handle_id, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(lockResourceHandle);
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	/* populate the evt */
	memset(&res_close_evt, 0, sizeof(GLSV_GLND_EVT));
	res_close_evt.type = GLSV_GLND_EVT_RSC_CLOSE;
	res_close_evt.info.rsc_info.client_handle_id = res_id_info->lock_handle_id;
	res_close_evt.info.rsc_info.resource_id = res_id_info->gbl_res_id;
	res_close_evt.info.rsc_info.lcl_resource_id = res_id_info->lcl_res_id;
	res_close_evt.info.rsc_info.call_type = GLSV_SYNC_CALL;
	res_close_evt.info.rsc_info.agent_mds_dest = gla_cb->gla_mds_dest;

	client_res_info = gla_client_res_tree_find_and_add(client_info, res_id_info->gbl_res_id, FALSE);
	if (client_res_info)
		res_close_evt.info.rsc_info.lcl_resource_id_count = client_res_info->lcl_res_cnt;

	/* send the event */
	if ((ret = gla_mds_msg_sync_send(gla_cb, &res_close_evt, &out_evt, GLA_API_RESP_TIME)) != NCSCC_RC_SUCCESS) {
		if (ret == NCSCC_RC_REQ_TIMOUT)
			rc = SA_AIS_ERR_TIMEOUT;
		m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
				    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_RSC_CLOSE);
		goto done;
	}

	rc = out_evt->error;	/* got the reply... do the needful */

	if (rc == SA_AIS_OK) {

		if (client_res_info) {
			client_res_info->lcl_res_cnt--;

			if (!client_res_info->lcl_res_cnt) {
				ncs_patricia_tree_del(&client_info->client_res_tree, &client_res_info->patnode);

				/* free the mem */
				m_MMGR_FREE_GLA_CLIENT_RES_INFO(client_res_info);

			}

		}

		gla_res_lock_tree_cleanup_client_down(gla_cb, res_id_info, res_id_info->lock_handle_id);

		/* delete the resource node */
		gla_res_tree_delete_node(gla_cb, res_id_info);
		/* Make the res_id_info pointer to NULL to make sure that this has been deleted */
		res_id_info = NULL;
	}

 done:
	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	/* return GLA CB */
	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (res_id_info)
		ncshm_give_hdl(lockResourceHandle);

	if (rc != SA_AIS_OK) {
		if (rc == SA_AIS_ERR_TRY_AGAIN)
			m_LOG_GLA_API(GLA_API_LCK_RESOURCE_CLOSE_FAIL, NCSFL_SEV_INFO, __FILE__, __LINE__);
		else
			m_LOG_GLA_API(GLA_API_LCK_RESOURCE_CLOSE_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);
	}

	if (rc == SA_AIS_OK)
		TRACE("saLckResourceClose SUCCESS");
	else
		TRACE("saLckResourceClose FAILURE");
	return rc;
}

/****************************************************************************
  Name          : saLckResourceLock

  Description   : This function locks the resources synchronously.
  
  Arguments     : hdl   - LCK handle
                  invocation - to identify the future waitercallbacks
                  resourceId - identifier of the resource.
                  lockId - the output of the call.
                  lockMode - the lock mode requested.
                  lockFlags - Flags for the lock reuqest.
                  timeout - timeout for this block call.
                  lockStatus - the status of the lock operation.

  Return Values : Refer to SAI-AIS specification for various return values.
       
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckResourceLock(SaLckResourceHandleT lockResourceHandle,
			      SaLckLockIdT *lockId,
			      SaLckLockModeT lockMode,
			      SaLckLockFlagsT lockFlags,
			      SaLckWaiterSignalT waiterSignal, SaTimeT timeout, SaLckLockStatusT *lockStatus)
{
	GLA_CB *gla_cb = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	GLSV_GLND_EVT res_lock_evt;
	GLSV_GLA_EVT *out_evt = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	SaTimeT gla_timeout = 0;
	uns32 ret;
	GLA_RESOURCE_ID_INFO *res_id_info = NULL;
	GLA_LOCK_ID_INFO *lock_id_node = NULL;

	TRACE("saLckResourceLock Called with Resource Handle %d", (uns32)lockResourceHandle);

	if (!(lockMode == SA_LCK_PR_LOCK_MODE || lockMode == SA_LCK_EX_LOCK_MODE)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* validate the inputs */
	if (lockId == NULL || lockStatus == NULL) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	if (((SA_LCK_LOCK_NO_QUEUE | SA_LCK_LOCK_ORPHAN) & lockFlags) != lockFlags) {
		rc = SA_AIS_ERR_BAD_FLAGS;
		goto done;
	}

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}

	/* retrieve Resorce hdl record */
	if (NULL == (res_id_info = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lockResourceHandle))) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, res_id_info->lock_handle_id, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(lockResourceHandle);
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	/* allocate the local lock node and add the values */
	lock_id_node = gla_lock_tree_find_and_add(gla_cb, 0, TRUE);

	if (NULL == (GLA_LOCK_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lock_id_node->lcl_lock_id)) {
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	/* populate the evt */
	memset(&res_lock_evt, 0, sizeof(GLSV_GLND_EVT));
	res_lock_evt.type = GLSV_GLND_EVT_RSC_LOCK;
	res_lock_evt.info.rsc_lock_info.client_handle_id = res_id_info->lock_handle_id;
	res_lock_evt.info.rsc_lock_info.waiter_signal = waiterSignal;
	res_lock_evt.info.rsc_lock_info.agent_mds_dest = gla_cb->gla_mds_dest;
	res_lock_evt.info.rsc_lock_info.resource_id = res_id_info->gbl_res_id;
	res_lock_evt.info.rsc_lock_info.lcl_resource_id = res_id_info->lcl_res_id;
	res_lock_evt.info.rsc_lock_info.lcl_lockid = lock_id_node->lcl_lock_id;
	res_lock_evt.info.rsc_lock_info.lock_type = lockMode;
	res_lock_evt.info.rsc_lock_info.lockFlags = lockFlags;
	res_lock_evt.info.rsc_lock_info.timeout = timeout;
	res_lock_evt.info.rsc_lock_info.call_type = GLSV_SYNC_CALL;

	gla_timeout = m_GLSV_CONVERT_SATIME_TEN_MILLI_SEC(timeout);
	if (gla_timeout < NCS_SAF_MIN_ACCEPT_TIME) {
		rc = SA_AIS_ERR_TIMEOUT;
		gla_lock_tree_delete_node(gla_cb, lock_id_node);
		lock_id_node = NULL;
		goto done;
	}
	/* Incrementing mds timer value so that before mds timer expires at agent,
	   lock request timer at glnd will be expired and send the response to agent */
	gla_timeout = gla_timeout + LCK_TIMEOUT_LATENCY;

	/* send the event */
	if ((ret = gla_mds_msg_sync_send(gla_cb, &res_lock_evt, &out_evt, (uns32)gla_timeout)) != NCSCC_RC_SUCCESS) {
		if (ret == NCSCC_RC_REQ_TIMOUT) {
			/* delete the lock node */
			gla_lock_tree_delete_node(gla_cb, lock_id_node);
			lock_id_node = NULL;
			rc = SA_AIS_ERR_TIMEOUT;
		} else
			m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
					    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_RSC_LOCK);
		goto done;
	}

	/* got the reply... do the needful */
	rc = out_evt->error;
	*lockId = out_evt->info.gla_resp_info.param.sync_lock.lockId;
	*lockStatus = out_evt->info.gla_resp_info.param.sync_lock.lockStatus;

	if (rc == SA_AIS_OK && *lockStatus == SA_LCK_LOCK_GRANTED) {
		lock_id_node->gbl_res_id = res_id_info->gbl_res_id;
		lock_id_node->lcl_res_id = res_id_info->lcl_res_id;
		lock_id_node->lock_handle_id = res_id_info->lock_handle_id;
		lock_id_node->gbl_lock_id = *lockId;
		lock_id_node->mode = lockMode;
		*lockId = lock_id_node->lcl_lock_id;

	} else {
		gla_lock_tree_delete_node(gla_cb, lock_id_node);
		lock_id_node = NULL;

	}
 done:
	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	if (lock_id_node)
		ncshm_give_hdl(lock_id_node->lcl_lock_id);

	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (res_id_info)
		ncshm_give_hdl(lockResourceHandle);

	/* return GLA CB */
	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK) {
		m_LOG_GLA_API(GLA_API_LCK_RESOURCE_LOCK_SYNC_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);
	}
	if (rc == SA_AIS_OK)
		TRACE("saLckResourceLock SUCCESS Lock id %d, Status %d", (uns32)*lockId, (uns32)*lockStatus);
	else
		TRACE("saLckResourceLock FAILURE %d", rc);

	return rc;
}

/****************************************************************************
  Name          : saLckResourceLockAsync

  Description   : This function locks the resources Asynchronously.
  
  Arguments     : hdl   - LCK handle
                  invocation - to identify the future waitercallbacks
                  resourceId - identifier of the resource.
                  lockId - the output of the call.
                  lockMode - the lock mode requested.
                  lockFlags - Flags for the lock reuqest.
                  timeout - timeout for this block call.
  
  Return Values : Refer to SAI-AIS specification for various return values.
       
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckResourceLockAsync(SaLckResourceHandleT lockResourceHandle,
				   SaInvocationT invocation,
				   SaLckLockIdT *lockId,
				   SaLckLockModeT lockMode, SaLckLockFlagsT lockFlags, SaLckWaiterSignalT waiterSignal)
{
	GLA_CB *gla_cb = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	GLSV_GLND_EVT res_lock_evt;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	GLSV_GLA_EVT *out_evt = NULL;
	GLA_RESOURCE_ID_INFO *res_id_info = NULL;
	GLA_LOCK_ID_INFO *lock_id_node = NULL;
	uns32 ret;

	TRACE("saLckResourceLockAsync Called with Res_id %d", (uns32)lockResourceHandle);

	/* validate the parameters */
	if (!(lockMode == SA_LCK_PR_LOCK_MODE || lockMode == SA_LCK_EX_LOCK_MODE)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	if (((SA_LCK_LOCK_NO_QUEUE | SA_LCK_LOCK_ORPHAN) & lockFlags) != lockFlags) {
		rc = SA_AIS_ERR_BAD_FLAGS;
		goto done;
	}

	if (lockId == NULL) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (res_id_info = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lockResourceHandle))) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, res_id_info->lock_handle_id, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check to see if the grant callback was registered */
	if (!client_info->lckCallbk.saLckLockGrantCallback) {

		/* shashi --- #4 changed from ERR_NOT_EXIST to ERR_INIT */
		rc = SA_AIS_ERR_INIT;
		goto done;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(lockResourceHandle);
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	/* put it in the lock id tree */
	lock_id_node = gla_lock_tree_find_and_add(gla_cb, 0, TRUE);
	if (NULL == (GLA_LOCK_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lock_id_node->lcl_lock_id)) {
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	lock_id_node->gbl_res_id = res_id_info->gbl_res_id;
	lock_id_node->lcl_res_id = res_id_info->lcl_res_id;
	lock_id_node->lock_handle_id = res_id_info->lock_handle_id;
	lock_id_node->mode = lockMode;
	*lockId = lock_id_node->lcl_lock_id;

	lock_id_node->lock_async_tmr.client_hdl = res_id_info->lock_handle_id;
	lock_id_node->lock_async_tmr.clbk_info.callback_type = GLSV_LOCK_GRANT_CBK;
	lock_id_node->lock_async_tmr.clbk_info.resourceId = res_id_info->lcl_res_id;
	lock_id_node->lock_async_tmr.clbk_info.lcl_lockId = lock_id_node->lcl_lock_id;
	lock_id_node->lock_async_tmr.clbk_info.invocation = invocation;

	/* populate the evt */
	memset(&res_lock_evt, 0, sizeof(GLSV_GLND_EVT));
	res_lock_evt.type = GLSV_GLND_EVT_RSC_LOCK;
	res_lock_evt.info.rsc_lock_info.client_handle_id = res_id_info->lock_handle_id;
	res_lock_evt.info.rsc_lock_info.invocation = invocation;
	res_lock_evt.info.rsc_lock_info.agent_mds_dest = gla_cb->gla_mds_dest;
	res_lock_evt.info.rsc_lock_info.resource_id = res_id_info->gbl_res_id;
	res_lock_evt.info.rsc_lock_info.lcl_resource_id = res_id_info->lcl_res_id;
	res_lock_evt.info.rsc_lock_info.lcl_lockid = lock_id_node->lcl_lock_id;
	res_lock_evt.info.rsc_lock_info.lock_type = lockMode;
	res_lock_evt.info.rsc_lock_info.lockFlags = lockFlags;
	res_lock_evt.info.rsc_lock_info.timeout = GLSV_LOCK_DEFAULT_TIMEOUT;
	res_lock_evt.info.rsc_lock_info.call_type = GLSV_ASYNC_CALL;
	res_lock_evt.info.rsc_lock_info.waiter_signal = waiterSignal;

	gla_start_tmr(&lock_id_node->lock_async_tmr);

	/* send the event */
	ret = gla_mds_msg_async_send(gla_cb, &res_lock_evt);
	if (ret != NCSCC_RC_SUCCESS) {
		gla_stop_tmr(&lock_id_node->lock_async_tmr);
		m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
				    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_RSC_LOCK);
		goto done;
	}

	rc = SA_AIS_OK;

 done:
	/* return GLA CB */

	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	if (lock_id_node)
		ncshm_give_hdl(lock_id_node->lcl_lock_id);

	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (res_id_info)
		ncshm_give_hdl(lockResourceHandle);

	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK)
		m_LOG_GLA_API(GLA_API_LCK_RESOURCE_LOCK_ASYNC_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);

	if (rc == SA_AIS_OK)
		TRACE("saLckResourceLockAsync SUCCESS");
	else
		TRACE("saLckResourceLockAsync FAILURE %d", rc);

	return rc;
}

/****************************************************************************
  Name          : saLckResourceUnlock

  Description   : This function unlocks the resources synchronously.
  
  Arguments     : hdl   - LCK handle
                  resourceId - identifier of the resource.
                  lockId - the output of the call.
                  timeout - timeout for this block call.

  Return Values : Refer to SAI-AIS specification for various return values.
       
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckResourceUnlock(SaLckLockIdT lockId, SaTimeT timeout)
{
	GLA_CB *gla_cb = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	GLSV_GLND_EVT res_unlock_evt;
	GLSV_GLA_EVT *out_evt = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	SaTimeT gla_timeout;
	uns32 ret;
	GLA_LOCK_ID_INFO *lock_id_info = NULL;
	GLA_RESOURCE_ID_INFO *res_id_info = NULL;
	SaLckResourceHandleT res_hdl = 0;

	TRACE("saLckResourceUnlock Called with lock_id %d", (uns32)lockId);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}
	/* retrieve Lock hdl record */
	if (NULL == (lock_id_info = (GLA_LOCK_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lockId))) {
		rc = SA_AIS_ERR_NOT_EXIST;
		goto done;
	}

	/*Added to fix LCL_RESOURCE_ID prob */
	/* get the handle and global resource id */
	res_hdl = lock_id_info->lcl_res_id;
	/* retrieve Resorce hdl record */
	if (NULL == (res_id_info = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, res_hdl))) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, lock_id_info->lock_handle_id, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(lockId);
		ncshm_give_hdl(res_hdl);
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	/* populate the evt */
	memset(&res_unlock_evt, 0, sizeof(GLSV_GLND_EVT));
	res_unlock_evt.type = GLSV_GLND_EVT_RSC_UNLOCK;
	res_unlock_evt.info.rsc_unlock_info.client_handle_id = lock_id_info->lock_handle_id;
	res_unlock_evt.info.rsc_unlock_info.agent_mds_dest = gla_cb->gla_mds_dest;
	res_unlock_evt.info.rsc_unlock_info.resource_id = lock_id_info->gbl_res_id;
	res_unlock_evt.info.rsc_unlock_info.lcl_resource_id = res_id_info->lcl_res_id;
	res_unlock_evt.info.rsc_unlock_info.lcl_lockid = lock_id_info->lcl_lock_id;
	res_unlock_evt.info.rsc_unlock_info.timeout = timeout;
	res_unlock_evt.info.rsc_unlock_info.call_type = GLSV_SYNC_CALL;

	gla_timeout = m_GLSV_CONVERT_SATIME_TEN_MILLI_SEC(timeout);
	if (gla_timeout < NCS_SAF_MIN_ACCEPT_TIME) {
		rc = SA_AIS_ERR_TIMEOUT;
		goto done;
	}
	/* Incrementing mds timer value so that before mds timer expires at agent,
	   lock request timer at glnd will be expired and send the response to agent */
	gla_timeout = gla_timeout + LCK_TIMEOUT_LATENCY;

	/* send the event */
	if ((ret = gla_mds_msg_sync_send(gla_cb, &res_unlock_evt, &out_evt, (uns32)gla_timeout)) != NCSCC_RC_SUCCESS) {
		if (ret == NCSCC_RC_REQ_TIMOUT)
			rc = SA_AIS_ERR_TIMEOUT;
		else
			m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
					    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest),
					    GLSV_GLND_EVT_RSC_UNLOCK);
		goto done;
	}
	rc = out_evt->error;

	if (rc == SA_AIS_OK) {
		gla_lock_tree_delete_node(gla_cb, lock_id_info);
		lock_id_info = NULL;

	}
 done:
	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (lock_id_info)
		ncshm_give_hdl(lockId);

	if (res_id_info)
		ncshm_give_hdl(res_hdl);

	/* return GLA CB */
	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK)
		m_LOG_GLA_API(GLA_API_LCK_RESOURCE_UNLOCK_SYNC_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);

	if (rc == SA_AIS_OK)
		TRACE("saLckResourceUnlock SUCCESS");
	else
		TRACE("saLckResourceUnlock FAILURE %d", rc);

	return rc;
}

/****************************************************************************
  Name          : saLckResourceUnlockAsync

  Description   : This function unlocks the resources synchronously.
  
  Arguments     : lckHandle   - LCK handle
                  resourceId - identifier of the resource.
                  lockId - the output of the call.
                  invocation - indentifer for the callback.

  Return Values : Refer to SAI-AIS specification for various return values.
       
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckResourceUnlockAsync(SaInvocationT invocation, SaLckLockIdT lockId)
{
	GLA_CB *gla_cb = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	GLSV_GLND_EVT res_unlock_evt;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	GLA_LOCK_ID_INFO *lock_id_info = NULL;

	TRACE("saLckResourceUnlockAsync Called with lock_id %d", (uns32)lockId);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		goto done;
	}

	/* retrieve Lock hdl record */
	if (NULL == (lock_id_info = (GLA_LOCK_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lockId))) {
		rc = SA_AIS_ERR_NOT_EXIST;
		goto done;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, lock_id_info->lock_handle_id, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check to see if the grant callback was registered */
	if (!client_info->lckCallbk.saLckResourceUnlockCallback) {
		rc = SA_AIS_ERR_INIT;
		goto done;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(lockId);
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	lock_id_info->unlock_async_tmr.client_hdl = lock_id_info->lock_handle_id;
	lock_id_info->unlock_async_tmr.clbk_info.callback_type = GLSV_LOCK_UNLOCK_CBK;
	lock_id_info->unlock_async_tmr.clbk_info.resourceId = lock_id_info->lcl_res_id;
	lock_id_info->unlock_async_tmr.clbk_info.lcl_lockId = lock_id_info->lcl_lock_id;
	lock_id_info->unlock_async_tmr.clbk_info.invocation = invocation;

	/* populate the evt */
	memset(&res_unlock_evt, 0, sizeof(GLSV_GLND_EVT));
	res_unlock_evt.type = GLSV_GLND_EVT_RSC_UNLOCK;
	res_unlock_evt.info.rsc_unlock_info.client_handle_id = lock_id_info->lock_handle_id;
	res_unlock_evt.info.rsc_unlock_info.agent_mds_dest = gla_cb->gla_mds_dest;
	res_unlock_evt.info.rsc_unlock_info.resource_id = lock_id_info->gbl_res_id;
	res_unlock_evt.info.rsc_unlock_info.lcl_resource_id = lock_id_info->lcl_res_id;
	res_unlock_evt.info.rsc_unlock_info.lockid = lock_id_info->gbl_lock_id;
	res_unlock_evt.info.rsc_unlock_info.lcl_lockid = lock_id_info->lcl_lock_id;
	res_unlock_evt.info.rsc_unlock_info.invocation = invocation;
	res_unlock_evt.info.rsc_unlock_info.call_type = GLSV_ASYNC_CALL;
	res_unlock_evt.info.rsc_unlock_info.timeout = GLSV_LOCK_DEFAULT_TIMEOUT;

	/* start the timer anyway */
	gla_start_tmr(&lock_id_info->unlock_async_tmr);

	/* send the event */
	if (gla_mds_msg_async_send(gla_cb, &res_unlock_evt) != NCSCC_RC_SUCCESS) {
		gla_stop_tmr(&lock_id_info->unlock_async_tmr);
		m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
				    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_RSC_UNLOCK);
		goto done;
	}
	rc = SA_AIS_OK;

 done:
	/* return GLA CB */
	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (lock_id_info)
		ncshm_give_hdl(lockId);

	if (rc != SA_AIS_OK)
		m_LOG_GLA_API(GLA_API_LCK_RESOURCE_UNLOCK_ASYNC_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);

	if (rc == SA_AIS_OK)
		TRACE("saLckResourceUnlockAsync SUCCESS");
	else
		TRACE("saLckResourceUnlockAsync FAILURE %d", rc);

	return rc;
}

/****************************************************************************
  Name          : saLckLockPurge

  Description   : This function purges all the orphan locks.
  
  Arguments     : lckHandle   - LCK handle
                  resourceId - identifier of the resource.

  Return Values : Refer to SAI-AIS specification for various return values.
       
  Notes         : None.
******************************************************************************/
SaAisErrorT saLckLockPurge(SaLckResourceHandleT lockResourceHandle)
{
	GLA_CB *gla_cb = NULL;
	GLA_CLIENT_INFO *client_info = NULL;
	GLSV_GLND_EVT res_purge_evt;
	GLSV_GLA_EVT *out_evt = NULL;
	SaAisErrorT rc = SA_AIS_ERR_LIBRARY;
	GLA_RESOURCE_ID_INFO *res_id_info = NULL;
	uns32 ret;

	TRACE("saLckLockPurge Called with Res Handle %" PRIu64, (uns64)lockResourceHandle);

	/* retrieve GLA CB */
	gla_cb = (GLA_CB *)m_GLSV_GLA_RETRIEVE_GLA_CB;
	if (!gla_cb) {
		m_LOG_GLA_HEADLINE(GLA_CB_RETRIEVAL_FAILED, NCSFL_SEV_INFO, __FILE__, __LINE__);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (res_id_info = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, lockResourceHandle))) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* get the client_info */
	client_info = gla_client_tree_find_and_add(gla_cb, res_id_info->lock_handle_id, FALSE);
	if (!client_info) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}
	/* retrieve Resorce hdl record */
	if (NULL == (GLA_CLIENT_INFO *)ncshm_take_hdl(NCS_SERVICE_ID_GLA, client_info->lcl_lock_handle_id)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto done;
	}

	/* check whether GLND is up or not */
	if (!gla_cb->glnd_svc_up) {
		ncshm_give_hdl(lockResourceHandle);
		ncshm_give_hdl(client_info->lcl_lock_handle_id);
		m_GLSV_GLA_GIVEUP_GLA_CB;
		return SA_AIS_ERR_TRY_AGAIN;
	}

	/* populate the evt */
	memset(&res_purge_evt, 0, sizeof(GLSV_GLND_EVT));
	res_purge_evt.type = GLSV_GLND_EVT_RSC_PURGE;
	res_purge_evt.info.rsc_info.client_handle_id = res_id_info->lock_handle_id;
	res_purge_evt.info.rsc_info.call_type = GLSV_SYNC_CALL;
	res_purge_evt.info.rsc_info.agent_mds_dest = gla_cb->gla_mds_dest;
	res_purge_evt.info.rsc_info.resource_id = res_id_info->gbl_res_id;

	/* send the event */
	if ((ret = gla_mds_msg_sync_send(gla_cb, &res_purge_evt, &out_evt, GLA_API_RESP_TIME)) != NCSCC_RC_SUCCESS) {
		if (ret == NCSCC_RC_REQ_TIMOUT)
			rc = SA_AIS_ERR_TIMEOUT;
		else
			m_LOG_GLA_DATA_SEND(GLA_MDS_SEND_FAILURE, __FILE__, __LINE__,
					    m_NCS_NODE_ID_FROM_MDS_DEST(gla_cb->gla_mds_dest), GLSV_GLND_EVT_RSC_PURGE);
		goto done;
	}

	/* got the reply... do the needful */
	rc = out_evt->error;

 done:
	if (out_evt)
		m_MMGR_FREE_GLA_EVT(out_evt);

	if (client_info)
		ncshm_give_hdl(client_info->lcl_lock_handle_id);

	if (res_id_info)
		ncshm_give_hdl(lockResourceHandle);

	/* return GLA CB */
	if (gla_cb)
		m_GLSV_GLA_GIVEUP_GLA_CB;

	if (rc != SA_AIS_OK)
		m_LOG_GLA_API(GLA_API_LCK_RESOURCE_PURGE_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__);

	if (rc == SA_AIS_OK)
		TRACE("saLckLockPurge SUCCESS");
	else
		TRACE("saLckLockPurge FAILURE %d", rc);

	return rc;
}
