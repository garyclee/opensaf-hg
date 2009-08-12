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
  DESCRIPTION:
  
  This file contains the IMMSv SAF API definitions.
*****************************************************************************/

#define _GNU_SOURCE
#include <string.h>

#include "imma.h"
#include "immsv_api.h"

#define NCS_SAF_MIN_ACCEPT_TIME 10

static const char *immLoaderName = IMMSV_LOADERNAME;	/*Defined in immsv_evt.h */

/* TODO: ABT move these to cb ? OR stop using the cb*/
static SaInt32T immInvocations = 0;
static SaBoolT immOmIsLoader = SA_FALSE;

static const char *sysaClName = SA_IMM_ATTR_CLASS_NAME;
static const char *sysaAdmName = SA_IMM_ATTR_ADMIN_OWNER_NAME;
static const char *sysaImplName = SA_IMM_ATTR_IMPLEMENTER_NAME;

/**
 * Validate a value type
 * @param theType
 * 
 * @return int
 */
static int is_valid_type(const SaImmValueTypeT theType)
{
	switch (theType) {
	case SA_IMM_ATTR_SAINT32T:
	case SA_IMM_ATTR_SAUINT32T:
	case SA_IMM_ATTR_SAINT64T:
	case SA_IMM_ATTR_SAUINT64T:
	case SA_IMM_ATTR_SATIMET:
	case SA_IMM_ATTR_SAFLOATT:
	case SA_IMM_ATTR_SADOUBLET:
	case SA_IMM_ATTR_SANAMET:
	case SA_IMM_ATTR_SASTRINGT:
	case SA_IMM_ATTR_SAANYT:
		return 1;
	default:
		return 0;
	}
}

/**
 * Validate admin op params
 * @param params
 * 
 * @return int
 */
static int is_adminop_params_valid(const SaImmAdminOperationParamsT_2 **params)
{
	int i = 0;

	/* Validate type for parameters */
	while (params[i] != NULL) {
		if (params[i]->paramName == NULL) {
			TRACE_1("no name for param: %d", i);
			return 0;
		}

		if (!is_valid_type(params[i]->paramType)) {
			TRACE_1("wrong type for param: %s", params[i]->paramName);
			return 0;
		}

		if (params[i]->paramBuffer == NULL) {
			TRACE_1("no value for param: %d", i);
			return 0;
		}

		i++;
	}

	return 1;
}

/****************************************************************************
  Name          :  SaImmOmInitialize
 
  Description   :  This function initializes the ImmOm Service for the
                   invoking process and registers the callback functions.
                   The handle 'immHandle' is returned as the reference to this
                   association between the process and  the ImmOm Service.
                   
 
  Arguments     :  immHandle -  A pointer to the handle designating this 
                                particular initialization of the IMM OM
                                service that is to be returned by the 
                                ImmOm Service.
                   immCallbacks  - Pointer to a SaImmCallbacksT structure, 
                                containing the callback functions of the
                                process that the ImmOm Service may invoke.
                   version    - Is a pointer to the version of the Imm
                                Service that the invoking process is using.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOmInitialize(SaImmHandleT *immHandle, const SaImmCallbacksT *immCallbacks, SaVersionT *version)
{
	IMMA_CB *cb = &imma_cb;
	SaAisErrorT rc = SA_AIS_OK;
	IMMSV_EVT init_evt;
	IMMSV_EVT *out_evt = NULL;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CLIENT_NODE *cl_node = 0;
	NCS_BOOL locked = TRUE;

	TRACE_ENTER();

	proc_rc = imma_startup(NCSMDS_SVC_ID_IMMA_OM);
	if (NCSCC_RC_SUCCESS != proc_rc) {
		TRACE_1("imma startup failed:%u", proc_rc);
		TRACE_LEAVE();
		return SA_AIS_ERR_LIBRARY;
	}

	if ((!immHandle) || (!version)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto end;
	}

	*immHandle = 0;

	/* Draft Validations : Version */
	rc = imma_version_validate(version);
	if (rc != SA_AIS_OK) {
		TRACE_2("Version validation failed");
		goto end;
	}

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	/* Alloc the client info data structure & put it in the Pat tree */
	cl_node = (IMMA_CLIENT_NODE *)calloc(1, sizeof(IMMA_CLIENT_NODE));

	if (cl_node == NULL) {
		TRACE_1("IMMA_CLIENT_NODE alloc failed");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto cnode_alloc_fail;
	}

	/* Store the callback functions, if set */
	if (immCallbacks) {
		cl_node->o.mCallbk = *immCallbacks;
		proc_rc = imma_callback_ipc_init(cl_node);
		if (proc_rc != NCSCC_RC_SUCCESS) {
			rc = SA_AIS_ERR_LIBRARY;
			/* ALready log'ed by imma_callback_ipc_init */
			goto ipc_init_fail;
		}

	}

	/* populate the EVT structure */
	memset(&init_evt, 0, sizeof(IMMSV_EVT));
	init_evt.type = IMMSV_EVT_TYPE_IMMND;
	init_evt.info.immnd.type = IMMND_EVT_A2ND_IMM_INIT;
	init_evt.info.immnd.info.initReq.version = *version;
	init_evt.info.immnd.info.initReq.client_pid = cb->process_id;

	/* Release the CB lock Before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/* IMMND GOES DOWN */
	if (FALSE == cb->is_immnd_up) {
		rc = SA_AIS_ERR_TRY_AGAIN;
		TRACE_2("IMMND is DOWN");
		goto mds_fail;
	}

	/* send the request to the IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, &init_evt, &out_evt, IMMSV_WAIT_TIME);

	/* Error Handling */
	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;
	case NCSCC_RC_REQ_TIMOUT:
		rc = SA_AIS_ERR_TIMEOUT;
		goto mds_fail;
	default:
		TRACE_1("Mds returned unexpected error code: %u", proc_rc);
		rc = SA_AIS_ERR_LIBRARY;
		goto mds_fail;
	}

	if (out_evt) {
		rc = out_evt->info.imma.info.initRsp.error;
		if (rc != SA_AIS_OK) {
			goto rsp_not_ok;
		}

		/* Take the CB lock after MDS Send */
		if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
			TRACE_1("Lock failed");
			rc = SA_AIS_ERR_LIBRARY;
			goto lock_fail1;
		} else {
			locked = TRUE;
		}

		if (FALSE == cb->is_immnd_up) {
			/*IMMND went down during THIS call! */
			rc = SA_AIS_ERR_TRY_AGAIN;
			TRACE_2("IMMND is DOWN");
			goto rsp_not_ok;
		}

		cl_node->handle = out_evt->info.imma.info.initRsp.immHandle;
		cl_node->isOm = TRUE;

		proc_rc = imma_client_node_add(&cb->client_tree, cl_node);
		if (proc_rc != NCSCC_RC_SUCCESS) {
			IMMA_CLIENT_NODE *stale_node = NULL;
			(void)imma_client_node_get(&cb->client_tree, &(cl_node->handle), &stale_node);

			if ((stale_node != NULL) && stale_node->stale) {
				TRACE_2("Removing stale client");
				imma_finalize_proc(cb, stale_node);
				proc_rc = imma_shutdown(NCSMDS_SVC_ID_IMMA_OM);
                if(proc_rc != NCSCC_RC_SUCCESS) {
                    TRACE_2("Call to imma_shutdown FAILED");
                    return SA_AIS_ERR_LIBRARY;
                }

				TRACE_2("Retrying add of client node");
				proc_rc = imma_client_node_add(&cb->client_tree, cl_node);
			}

			if (proc_rc != NCSCC_RC_SUCCESS) {
				rc = SA_AIS_ERR_LIBRARY;
				TRACE_1("client_node_add failed");
				goto node_add_fail;
			}
		}
		assert(rc == SA_AIS_OK);
	} else {
		TRACE_1("Empty reply received");
		rc = SA_AIS_ERR_NO_RESOURCES;
	}

	/*Error handling */
 node_add_fail:
 lock_fail1:
	if (rc != SA_AIS_OK) {
		IMMSV_EVT finalize_evt, *out_evt1;

		out_evt1 = NULL;
		memset(&finalize_evt, 0, sizeof(IMMSV_EVT));
		finalize_evt.type = IMMSV_EVT_TYPE_IMMND;
		finalize_evt.info.immnd.type = IMMND_EVT_A2ND_IMM_FINALIZE;
		finalize_evt.info.immnd.info.finReq.client_hdl = cl_node->handle;

		if (locked) {
			m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
			locked = FALSE;
		}

		/* send the request to the IMMND */
		proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl,
						 &(cb->immnd_mds_dest), &finalize_evt, &out_evt1, IMMSV_WAIT_TIME);
		if (out_evt1) {
			free(out_evt1);
		}
	}

 rsp_not_ok:
 mds_fail:

	/* Free the IPC initialized for this client */
	if (rc != SA_AIS_OK)
		imma_callback_ipc_destroy(cl_node);

 ipc_init_fail:
	if (rc != SA_AIS_OK)
		free(cl_node);

 cnode_alloc_fail:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	if (out_evt) {
		free(out_evt);
	}

	if (rc == SA_AIS_OK) {
		/* Went well, return immHandle to the application */
		*immHandle = cl_node->handle;
	}

 end:
	if (rc != SA_AIS_OK)
		if(NCSCC_RC_SUCCESS != imma_shutdown(NCSMDS_SVC_ID_IMMA_OM)) {
            /* Oh boy. Failure in imma_shutdown when we already have
               some other problem. */
            TRACE_1("Call to imma_shutdown failed, prior error %u", rc);
            rc = SA_AIS_ERR_LIBRARY;
        }

	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
  Name          :  saImmOmSelectionObjectGet
 
  Description   :  This function returns the operating system handle 
                   associated with the immHandle.
 
  Arguments     :  immHandle - Imm OM service handle.
                   selectionObject - Pointer to the operating system handle.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOmSelectionObjectGet(SaImmHandleT immHandle, SaSelectionObjectT *selectionObject)
{
	SaAisErrorT rc = SA_AIS_OK;
	IMMA_CB *cb = &imma_cb;
	IMMA_CLIENT_NODE *cl_node = 0;
	uns32 proc_rc = NCSCC_RC_FAILURE;

	if (!selectionObject)
		return SA_AIS_ERR_INVALID_PARAM;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	*selectionObject = (-1);	/* Ensure non valid descriptor in case of failure. */

	/* Take the CB lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	proc_rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);

	if (!(cl_node && cl_node->isOm)) {
		TRACE_2("Bad handle %llu", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto node_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	if (cl_node->o.mCallbk.saImmOmAdminOperationInvokeCallback == NULL) {
		TRACE_2("saImmOmSelectionObjectGet not allowed when saImmOmAdminOperationInvokeCallback is NULL");
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto no_callback;
	}

	*selectionObject = (SaSelectionObjectT)
	    m_GET_FD_FROM_SEL_OBJ(m_NCS_IPC_GET_SEL_OBJ(&cl_node->callbk_mbx));

 no_callback:
 stale_handle:
 node_not_found:
	/* Unlock CB */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

/****************************************************************************
  Name          :  saImmOmDispatch
 
  Description   :  This function invokes, in the context of the calling
                   thread. pending callbacks for the handle immHandle in a 
                   way that is specified by the dispatchFlags parameter.
 
  Arguments     :  immHandle - IMM OM Service handle
                   dispatchFlags - Flags that specify the callback execution
                                   behaviour of this function.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOmDispatch(SaImmHandleT immHandle, SaDispatchFlagsT dispatchFlags)
{
	SaAisErrorT rc = SA_AIS_OK;
	IMMA_CB *cb = &imma_cb;
	IMMA_CLIENT_NODE *cl_node = 0;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto fail;
	}

	/* get the client_info */
	imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		TRACE_2("client-node_get failed");
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
		goto fail;
	}

	if (cl_node->stale) {
		TRACE_2("Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
		goto fail;
	}

	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

	switch (dispatchFlags) {
	case SA_DISPATCH_ONE:
		rc = imma_hdl_callbk_dispatch_one(cb, immHandle);
		break;

	case SA_DISPATCH_ALL:
		rc = imma_hdl_callbk_dispatch_all(cb, immHandle);
		break;

	case SA_DISPATCH_BLOCKING:
		rc = imma_hdl_callbk_dispatch_block(cb, immHandle);
		break;

	default:
		rc = SA_AIS_ERR_INVALID_PARAM;
		break;
	}			/* switch */

 fail:

	return rc;
}

/****************************************************************************
  Name          :  saImmOmFinalize
 
  Description   :  This function closes the association, represented by
                   immHandle, between the invoking process and the IMM OM
                   Service.
 
  Arguments     :  immHandle - IMM OM Service handle.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOmFinalize(SaImmHandleT immHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT finalize_evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = 0;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = TRUE;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	TRACE_ENTER();

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	/* get the client_info */
	proc_rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		TRACE_2("client_node_get failed");
		goto node_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("Handle %llu is stale", immHandle);
		rc = SA_AIS_OK;	/*Dont punish the client for closing stale handle */
		goto stale_handle;
	}

	/* populate the structure */
	memset(&finalize_evt, 0, sizeof(IMMSV_EVT));
	finalize_evt.type = IMMSV_EVT_TYPE_IMMND;
	finalize_evt.info.immnd.type = IMMND_EVT_A2ND_IMM_FINALIZE;
	finalize_evt.info.immnd.info.finReq.client_hdl = cl_node->handle;

	/* Unlock before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	if (cb->is_immnd_up == FALSE) {
		rc = SA_AIS_ERR_TRY_AGAIN;
		TRACE_2("IMMND is DOWN");
		goto mds_send_fail;
	}

	/* send the request to the IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &(cb->immnd_mds_dest),
					 &finalize_evt, &out_evt, IMMSV_WAIT_TIME);
	/* MDS error handling */
	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;
	case NCSCC_RC_REQ_TIMOUT:
		rc = SA_AIS_ERR_TIMEOUT;
		goto mds_send_fail;
	default:
		TRACE_1("Mds returned unexpected error code: %u", proc_rc);
		rc = SA_AIS_ERR_LIBRARY;
		goto mds_send_fail;
	}

	/* Read the received error (if any)  */
	if (out_evt) {
		rc = out_evt->info.imma.info.errRsp.error;
		free(out_evt);
	} else {
		TRACE_1("Received empty reply from server");
		rc = SA_AIS_ERR_NO_RESOURCES;
	}

 stale_handle:
	/* Do the finalize processing at IMMA */
	if (rc == SA_AIS_OK) {
		/* Take the CB lock  */
		if (!locked && m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
			rc = SA_AIS_ERR_LIBRARY;
			TRACE_1("Lock failed");
			goto lock_fail1;
		}

		locked = TRUE;

		imma_finalize_proc(cb, cl_node);
	}

 lock_fail1:
 mds_send_fail:
 node_not_found:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	if (rc == SA_AIS_OK) {
		if(NCSCC_RC_SUCCESS != imma_shutdown(NCSMDS_SVC_ID_IMMA_OM)) {
            TRACE_1("Call to imma_shutdown failed");
            rc = SA_AIS_ERR_LIBRARY;
        }
	}
	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
  Name          :  saImmOmAdminOwnerInitialize
 
  Description   :  Initialize an admin owner handle.
                   This a blocking syncronous call.

                   
  Arguments     :  immHandle - IMM OM handle
                   adminOwnerName - The name of the admin owner.
                   releaseOwnershipOnFinalize - se the IMM spec.
                   adminOwnerHandle - The handle that is returned.

  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         : 
******************************************************************************/
SaAisErrorT saImmOmAdminOwnerInitialize(SaImmHandleT immHandle,
					const SaImmAdminOwnerNameT adminOwnerName,
					SaBoolT releaseOwnershipOnFinalize, SaImmAdminOwnerHandleT *adminOwnerHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	SaUint32T proc_rc = NCSCC_RC_SUCCESS;
	SaBoolT locked = SA_TRUE;
	SaBoolT isLoaderName = SA_FALSE;
	SaUint32T nameLen = 0;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((adminOwnerHandle == NULL) || (adminOwnerName == NULL) || (nameLen = strlen(adminOwnerName)) == 0) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (releaseOwnershipOnFinalize && (releaseOwnershipOnFinalize != 1)) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (nameLen >= SA_MAX_NAME_LENGTH) {
		TRACE_2("Admin owner name too long, size: %u max:%u", nameLen, SA_MAX_NAME_LENGTH);
		return SA_AIS_ERR_INVALID_PARAM;
	}

	isLoaderName = (strncmp(adminOwnerName, immLoaderName, nameLen) == 0);

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}

	/*locked == TRUE already */

	rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* Allocate the IMMA_ADMIN_OWNER_NODE & Populate */
	ao_node = (IMMA_ADMIN_OWNER_NODE *)calloc(1, sizeof(IMMA_ADMIN_OWNER_NODE));
	if (ao_node == NULL) {
		rc = SA_AIS_ERR_NO_MEMORY;
		TRACE_1("Memory allocation error");
		goto ao_node_alloc_fail;
	}

	memset(ao_node, 0, sizeof(IMMA_ADMIN_OWNER_NODE));
	ao_node->admin_owner_hdl = (SaImmAdminOwnerHandleT)m_NCS_GET_TIME_NS;
	/*This is the external handle that the application uses.
	   Internally we use the gloablId provided by the director. */

	ao_node->mImmHandle = immHandle;

	proc_rc = imma_admin_owner_node_add(&cb->admin_owner_tree, ao_node);

	if (proc_rc != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Failed to add node to the admin owner tree");
		goto ao_node_add_fail;
	}

	/* Populate & Send the Open Event to IMMND */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_IMM_ADMINIT;
	evt.info.immnd.info.adminitReq.client_hdl = immHandle;
	evt.info.immnd.info.adminitReq.i.adminOwnerName.length = nameLen;
	memcpy(evt.info.immnd.info.adminitReq.i.adminOwnerName.value, adminOwnerName, nameLen + 1);
	evt.info.immnd.info.adminitReq.i.releaseOwnershipOnFinalize = releaseOwnershipOnFinalize;

	/* Unlock before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/* IMMND GOES DOWN */
	if (cb->is_immnd_up == FALSE) {
		rc = SA_AIS_ERR_TRY_AGAIN;
		TRACE_2("IMMND is DOWN");
		goto mds_send_fail;
	}

	/* Send the evt to IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &(cb->immnd_mds_dest), &evt, &out_evt, IMMSV_WAIT_TIME);
	/* Generate rc from proc_rc */
	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;
	case NCSCC_RC_REQ_TIMOUT:
		rc = SA_AIS_ERR_TIMEOUT;
		goto mds_send_fail;
	default:
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("MDS returned unexpected error code %u", proc_rc);
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the received Event */
		rc = out_evt->info.imma.info.admInitRsp.error;
		ao_node->mAdminOwnerId = out_evt->info.imma.info.admInitRsp.ownerId;

		/* Take the CB lock */
		if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
			rc = SA_AIS_ERR_LIBRARY;
			TRACE_1("Lock failed");
			free(out_evt);
			goto lock_fail1;
		}

		locked = TRUE;

		free(out_evt);

		if (rc != SA_AIS_OK) {
			/* Free the Admin Owner Node */
			goto admin_owner_node_free;
		}
	}

	/* TODO: to be really safe we should look up the ao_node again. */
	*adminOwnerHandle = ao_node->admin_owner_hdl;

	if (isLoaderName) {
		immOmIsLoader = SA_TRUE;
	}

	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

	return SA_AIS_OK;

	/* Error Handling */
 lock_fail1:

	/* Call the Admin Owner close Processing */
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	return rc;

 mds_send_fail:
	if (!locked) {
		/* Take the CB lock */
		if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
			rc = SA_AIS_ERR_LIBRARY;
			TRACE_1("Lock failed");
		} else {
			locked = TRUE;
		}
	}

 admin_owner_node_free:
	if (locked) {
		imma_admin_owner_node_delete(cb, ao_node);
		ao_node = NULL;
	}

 ao_node_add_fail:
	if (ao_node != NULL) {
		free(ao_node);
	}

 ao_node_alloc_fail:
	/* Do Nothing */

 client_not_found:
 stale_handle:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

/*******************************************************************
 * imma_newCcbId internal function
 *
 * NOTE: The CB must be LOCKED on entry of this function!!
 *       It will usually be unlocked on exit, as reflected in the 'locked' 
 *       parameter.
 *******************************************************************/
static SaAisErrorT imma_newCcbId(IMMA_CB *cb, IMMA_CCB_NODE *ccb_node, IMMA_ADMIN_OWNER_NODE *ao_node, NCS_BOOL *locked)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaUint32T proc_rc = NCSCC_RC_SUCCESS;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;

	assert(locked && *locked);
	assert(ccb_node->mTerminated);
	ccb_node->mCcbId = 0;

	/* Populate & Send the Open Event to IMMND */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_CCBINIT;
	evt.info.immnd.info.ccbinitReq.client_hdl = ao_node->mImmHandle;
	evt.info.immnd.info.ccbinitReq.adminOwnerId = ao_node->mAdminOwnerId;
	evt.info.immnd.info.ccbinitReq.ccbFlags = ccb_node->mCcbFlags;

	/* Unlock before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	*locked = FALSE;

	/* IMMND GOES DOWN */
	if (cb->is_immnd_up == FALSE) {
		rc = SA_AIS_ERR_NO_RESOURCES;
		TRACE_2("IMMND is DOWN");
		goto mds_send_fail;
	}

	/* Send the evt to IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &(cb->immnd_mds_dest), &evt, &out_evt, IMMSV_WAIT_TIME);
	/* Generate rc from proc_rc */
	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;
	case NCSCC_RC_REQ_TIMOUT:
		rc = SA_AIS_ERR_TIMEOUT;
		goto mds_send_fail;
	default:
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("MDS returned unexpected error code %u", proc_rc);
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the received Event */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_CCBINIT_RSP);
		rc = out_evt->info.imma.info.ccbInitRsp.error;

		/* Take the CB lock */
		if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
			rc = SA_AIS_ERR_LIBRARY;
			TRACE_1("Lock failed");
			free(out_evt);
			goto lock_fail;
		}

		*locked = TRUE;

		if (rc == SA_AIS_OK) {	/* TODO: should really look up ccb_node again. */
			ccb_node->mCcbId = out_evt->info.imma.info.ccbInitRsp.ccbId;
		}

		/* Free the out event */
		free(out_evt);
	}

 mds_send_fail:
 lock_fail:
	if (rc == SA_AIS_OK) {
		ccb_node->mTerminated = SA_FALSE;
		TRACE("CcbId:%u\n", ccb_node->mCcbId);
	}

	return rc;
}

/****************************************************************************
  Name          :  saImmOmCcbInitialize
 
  Description   :  Initialize an admin owner handle.
                   This a blocking syncronous call.

                   
  Arguments     :  adminOwnerHandle - The handle for the admin owner to
                                      which the Ccb will be associated.
                   ccbFlags         - Refer to the SA_AIS_IMM specification
                                      for explanation.
                   ccbHandle        - The address of a Ccb handle which
                                      will be initialized. 

  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         : 
******************************************************************************/
SaAisErrorT saImmOmCcbInitialize(SaImmAdminOwnerHandleT adminOwnerHandle,
				 SaImmCcbFlagsT ccbFlags, SaImmCcbHandleT *ccbHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaUint32T proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CB *cb = &imma_cb;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CCB_NODE *ccb_node = NULL;
	SaBoolT locked = SA_FALSE;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((ccbHandle == NULL) || (ccbFlags && (ccbFlags != SA_IMM_CCB_REGISTERED_OI))) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}

	locked = SA_TRUE;

	rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &adminOwnerHandle, &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto admowner_not_found;
	}

	/*Look up client node also simply to verify that the client handle
	   is still active. */

	rc = imma_client_node_get(&cb->client_tree, &(ao_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No client associated with Admin Owner");
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ao_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* Allocate the IMMA_CCB_NODE & Populate */

	ccb_node = (IMMA_CCB_NODE *)calloc(1, sizeof(IMMA_CCB_NODE));
	if (ccb_node == NULL) {
		rc = SA_AIS_ERR_NO_MEMORY;
		TRACE_1("Memory allocation failure");
		goto ccb_node_alloc_fail;
	}

	ccb_node->ccb_hdl = (SaImmCcbHandleT)m_NCS_GET_TIME_NS;
	/*This is the external handle that the application uses.
	   Internally we use the gloablId provided by the Director. */

	proc_rc = imma_ccb_node_add(&cb->ccb_tree, ccb_node);

	if (proc_rc != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Faild to add ccb-node to ccb-tree");
		goto ccb_node_add_fail;
	}

	ccb_node->mCcbFlags = ccbFlags;	/*Save flags in client for repeated init */
	ccb_node->mImmHandle = ao_node->mImmHandle;
	ccb_node->mAdminOwnerHdl = adminOwnerHandle;
	ccb_node->mTerminated = SA_TRUE;

	rc = imma_newCcbId(cb, ccb_node, ao_node, &locked);

	if (rc == SA_AIS_OK) {
		*ccbHandle = ccb_node->ccb_hdl;
		goto success;
	} else {

		if (!locked) {
			if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
				rc = SA_AIS_ERR_LIBRARY;
				TRACE_1("Lock failed");
				goto lock_fail;	/*will leak memory, but this is a strange error */
			}
			locked = SA_TRUE;
		}

		imma_ccb_node_delete(cb, ccb_node);	/*Remove node from tree and free it. */
		ccb_node = NULL;
	}

 ccb_node_add_fail:
	if (ccb_node != NULL) {
		free(ccb_node);
	}

 success:
 ccb_node_alloc_fail:
 admowner_not_found:
 client_not_found:
 stale_handle:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

/****************************************************************************
  Name          :  saImmOmCcbObjectCreate/_2
 
  Description   :  Creates a config object in the IMM.
                   This a blocking syncronous call.

                   
  Arguments     :  ccbHandle - Ccb Handle
                   className - A pointer to the name of the class of the object
                   parentName - A pointer to the name of the parent.
                   attrValues - Initial values for some attributes.

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
#ifdef IMM_A_01_01
SaAisErrorT saImmOmCcbObjectCreate(SaImmCcbHandleT ccbHandle,
				   const SaImmClassNameT className,
				   const SaNameT *parentName, const SaImmAttrValuesT ** attrValues)
{
	return saImmOmCcbObjectCreate_2(ccbHandle, className, parentName, (const SaImmAttrValuesT_2 **)attrValues);
}
#endif

SaAisErrorT saImmOmCcbObjectCreate_2(SaImmCcbHandleT ccbHandle,
				     const SaImmClassNameT className,
				     const SaNameT *parentName, const SaImmAttrValuesT_2 **attrValues)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_CCB_NODE *ccb_node = NULL;
	NCS_BOOL locked = FALSE;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	TRACE_ENTER();

	if (className == NULL) {
		TRACE_2("classname is NULL");
		TRACE_LEAVE();
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (attrValues == NULL) {
		TRACE_2("attrValues is NULL");
		TRACE_LEAVE();
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	locked = TRUE;

	/* Get the CCB info */
	proc_rc = imma_ccb_node_get(&cb->ccb_tree, &ccbHandle, &ccb_node);
	if (!ccb_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		TRACE_2("Ccb handle not valid");
		goto ccb_not_found;
	}

	/* Get the Admin Owner info  */
	proc_rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &(ccb_node->mAdminOwnerHdl), &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No Amin-Owner associated with Ccb");
		goto ao_not_found;
	}

	/*Look up client node also, to verify that the client handle
	   is still active. */

	proc_rc = imma_client_node_get(&cb->client_tree, &(ccb_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No valid SaImmHandleT associated with Ccb");
		goto client_not_found;
	}

	assert(ccb_node->mImmHandle == ao_node->mImmHandle);

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ccb_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	if (ccb_node->mTerminated) {
		rc = imma_newCcbId(cb, ccb_node, ao_node, &locked);
		if (rc != SA_AIS_OK) {
			goto ccb_init_fail;
		}
	}

	/* Populate the Object-Create event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_OBJ_CREATE;

	evt.info.immnd.info.objCreate.adminOwnerId = ao_node->mAdminOwnerId;
	evt.info.immnd.info.objCreate.ccbId = ccb_node->mCcbId;

	evt.info.immnd.info.objCreate.className.size = strlen(className) + 1;

	/*alloc-1 */
	evt.info.immnd.info.objCreate.className.buf = malloc(evt.info.immnd.info.objCreate.className.size);
	if (evt.info.immnd.info.objCreate.className.buf == NULL) {
		rc = SA_AIS_ERR_NO_MEMORY;
		goto mds_send_fail;
	}
	strncpy(evt.info.immnd.info.objCreate.className.buf, className, evt.info.immnd.info.objCreate.className.size);

	if (parentName && parentName->length) {
		evt.info.immnd.info.objCreate.parentName.size = strlen((char *)parentName->value) + 1;

		if (parentName->length + 1 < evt.info.immnd.info.objCreate.parentName.size) {
			evt.info.immnd.info.objCreate.parentName.size = parentName->length + 1;
		}

		if (evt.info.immnd.info.objCreate.parentName.size > SA_MAX_NAME_LENGTH) {
			rc = SA_AIS_ERR_INVALID_PARAM;
			TRACE_2("Parent name too long for SaNameT: %u", evt.info.immnd.info.objCreate.parentName.size);
			goto mds_send_fail;

		}

		/*alloc-2 */
		evt.info.immnd.info.objCreate.parentName.buf = malloc(evt.info.immnd.info.objCreate.parentName.size);
		if (evt.info.immnd.info.objCreate.parentName.buf == NULL) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto mds_send_fail;
		}
		strncpy(evt.info.immnd.info.objCreate.parentName.buf,
			(char *)parentName->value, evt.info.immnd.info.objCreate.parentName.size);
		evt.info.immnd.info.objCreate.parentName.buf[evt.info.immnd.info.objCreate.parentName.size - 1] = '\0';
	} else {
		evt.info.immnd.info.objCreate.parentName.size = 0;
		evt.info.immnd.info.objCreate.parentName.buf = NULL;
	}

	assert(evt.info.immnd.info.objCreate.attrValues == NULL);
	const SaImmAttrValuesT_2 *attr;
	int i;
	for (i = 0; attrValues[i]; ++i) {
		attr = attrValues[i];
		TRACE("attr:%s \n", attr->attrName);

		/*Check that the user does not set value for System attributes. */

		if (strcmp(attr->attrName, sysaClName) == 0) {
			if (immOmIsLoader) {
				/*I am loader => will allow the classname attribute to be defaulted */
				continue;
			}
			/*Non loaders are not allowed to explicitly assign className attribute */
			rc = SA_AIS_ERR_INVALID_PARAM;
			TRACE_2("Not allowed to set attribute %s ", sysaClName);
			goto mds_send_fail;
		} else if (strcmp(attr->attrName, sysaAdmName) == 0) {
			if (immOmIsLoader) {
				/*Loader => clear admName attribute, not others */
				/*This is controversial! The standard is not explicit on this. */
				/* Removing curent admin owner name allows the imm to set IMMLOADER */
				continue;
			}
			rc = SA_AIS_ERR_INVALID_PARAM;
			TRACE_2("Not allowed to set attribute %s", sysaAdmName);
			goto mds_send_fail;
		} else if ((strcmp(attr->attrName, sysaImplName) == 0) && (!immOmIsLoader)) {
			/*Loader allowed to explicitly assign implName attribute, not others
			   The only point of allowing this is that a class/object implementer 
			   set with identical implementer name may be faster after cluster
			   restart because ImmAttrValue::setValueC_st checks for equality
			   before overwrite.
			 */

			rc = SA_AIS_ERR_INVALID_PARAM;
			TRACE_2("Not allowed to set attribute %s", sysaImplName);
			goto mds_send_fail;
		} else if (attr->attrValuesNumber == 0) {
			TRACE("CcbObjectCreate ignoring attribute %s with no values", attr->attrName);
			continue;
		}

		/*alloc-3 */
		IMMSV_ATTR_VALUES_LIST *p = calloc(1, sizeof(IMMSV_ATTR_VALUES_LIST));

		p->n.attrName.size = strlen(attr->attrName) + 1;
		if (p->n.attrName.size >= SA_MAX_NAME_LENGTH) {
			TRACE_2("Attribute name too long");
			rc = SA_AIS_ERR_INVALID_PARAM;
			free(p);
			goto mds_send_fail;
		}

		/*alloc-4 */
		p->n.attrName.buf = malloc(p->n.attrName.size);

		strncpy(p->n.attrName.buf, attr->attrName, p->n.attrName.size);

		p->n.attrValuesNumber = attr->attrValuesNumber;
		p->n.attrValueType = attr->attrValueType;

		const SaImmAttrValueT *avarr = attr->attrValues;
		/*alloc-5 */
		imma_copyAttrValue(&(p->n.attrValue), attr->attrValueType, avarr[0]);

		if (attr->attrValuesNumber > 1) {
			unsigned int numAdded = attr->attrValuesNumber - 1;
			unsigned int i;
			for (i = 1; i <= numAdded; ++i) {
				/*alloc-6 */
				IMMSV_EDU_ATTR_VAL_LIST *al = calloc(1, sizeof(IMMSV_EDU_ATTR_VAL_LIST));

				/*alloc-7 */
				imma_copyAttrValue(&(al->n), attr->attrValueType, avarr[i]);
				al->next = p->n.attrMoreValues;
				p->n.attrMoreValues = al;
			}
		}

		p->next = evt.info.immnd.info.objCreate.attrValues;	/*NULL initially. */
		evt.info.immnd.info.objCreate.attrValues = p;
	}

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, FALSE);

	TRACE("objectCreate send RETURNED:%u", rc);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is after a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here */

		/* Free the out event */
		/*Will free the root event only, not any pointer structures. */
		free(out_evt);
	}

 mds_send_fail:
	/*We may be un-locked here but this should not matter.
	   We are freing heap objects that should only be vissible from this
	   thread. */

	if (evt.info.immnd.info.objCreate.className.buf) {	/*free-1 */
		free(evt.info.immnd.info.objCreate.className.buf);
		evt.info.immnd.info.objCreate.className.buf = NULL;
	}

	if (evt.info.immnd.info.objCreate.parentName.buf) {	/*free-2 */
		free(evt.info.immnd.info.objCreate.parentName.buf);
		evt.info.immnd.info.objCreate.parentName.buf = NULL;
	}

	while (evt.info.immnd.info.objCreate.attrValues) {
		IMMSV_ATTR_VALUES_LIST *p = evt.info.immnd.info.objCreate.attrValues;
		evt.info.immnd.info.objCreate.attrValues = p->next;
		p->next = NULL;
		if (p->n.attrName.buf) {	/*free-4 */
			free(p->n.attrName.buf);
			p->n.attrName.buf = NULL;
		}

		immsv_evt_free_att_val(&(p->n.attrValue), p->n.attrValueType);	/*free-5 */

		while (p->n.attrMoreValues) {
			IMMSV_EDU_ATTR_VAL_LIST *al = p->n.attrMoreValues;
			p->n.attrMoreValues = al->next;
			al->next = NULL;
			immsv_evt_free_att_val(&(al->n), p->n.attrValueType);	/*free-7 */
			free(al);	/*free-6 */
		}

		p->next = NULL;
		free(p);	/*free-3 */
	}

 ccb_not_found:
 ao_not_found:
 client_not_found:
 stale_handle:
 ccb_init_fail:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
  Name          :  saImmOmCcbObjectModify/_2
 
  Description   :  Modifies a config object in the IMM.
                   This a blocking syncronous call.

                   
  Arguments     :  ccbHandle - Ccb Handle
                   objectName - A pointer to the name of the object.
                   attrMods - The attribute modifications (see the standard)

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
#ifdef IMM_A_01_01
SaAisErrorT saImmOmCcbObjectModify(SaImmCcbHandleT ccbHandle,
				   const SaNameT *objectName, const SaImmAttrModificationT ** attrMods)
{

	return saImmOmCcbObjectModify_2(ccbHandle, objectName, (const SaImmAttrModificationT_2 **)
					attrMods);
}
#endif

SaAisErrorT saImmOmCcbObjectModify_2(SaImmCcbHandleT ccbHandle,
				     const SaNameT *objectName, const SaImmAttrModificationT_2 **attrMods)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_CCB_NODE *ccb_node = NULL;
	NCS_BOOL locked = FALSE;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (objectName == NULL) {
		TRACE_2("objectName is NULL");
		TRACE_LEAVE();
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (attrMods == NULL) {
		TRACE_2("attrMods is NULL");
		TRACE_LEAVE();
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	locked = TRUE;

	/* Get the CCB info */
	proc_rc = imma_ccb_node_get(&cb->ccb_tree, &ccbHandle, &ccb_node);
	if (!ccb_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		TRACE_2("Ccb handle not valid");
		goto ccb_not_found;
	}

	/* Get the Admin Owner info  */
	proc_rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &(ccb_node->mAdminOwnerHdl), &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No Amin-Owner associated with Ccb");
		goto ao_not_found;
	}

	/*Look up client node also, to verify that the client handle
	   is still active. */

	proc_rc = imma_client_node_get(&cb->client_tree, &(ccb_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No valid SaImmHandleT associated with Ccb");
		goto client_not_found;
	}

	assert(ccb_node->mImmHandle == ao_node->mImmHandle);

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ccb_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	if (ccb_node->mTerminated) {
		rc = imma_newCcbId(cb, ccb_node, ao_node, &locked);
		if (rc != SA_AIS_OK) {
			goto ccb_init_fail;
		}
	}

	/* Populate the Object-Modify event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_OBJ_MODIFY;

	evt.info.immnd.info.objModify.adminOwnerId = ao_node->mAdminOwnerId;
	evt.info.immnd.info.objModify.ccbId = ccb_node->mCcbId;

	if (objectName->length) {
		evt.info.immnd.info.objModify.objectName.size = strlen((char *)objectName->value) + 1;

		if (objectName->length + 1 < evt.info.immnd.info.objModify.objectName.size) {
			evt.info.immnd.info.objModify.objectName.size = objectName->length + 1;
		}

		/*alloc-1 */
		evt.info.immnd.info.objModify.objectName.buf = malloc(evt.info.immnd.info.objModify.objectName.size);
		if (evt.info.immnd.info.objModify.objectName.buf == NULL) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto mds_send_fail;
		}
		strncpy(evt.info.immnd.info.objModify.objectName.buf,
			(char *)objectName->value, evt.info.immnd.info.objModify.objectName.size);
		evt.info.immnd.info.objModify.objectName.buf[evt.info.immnd.info.objModify.objectName.size - 1] = '\0';
	} else {
		evt.info.immnd.info.objModify.objectName.size = 0;
		evt.info.immnd.info.objModify.objectName.buf = NULL;
	}

	assert(evt.info.immnd.info.objModify.attrMods == NULL);

	const SaImmAttrModificationT_2 *attrMod;
	int i;
	for (i = 0; attrMods[i]; ++i) {
		attrMod = attrMods[i];

		/*NOTE: Check that user does not set values for System attributes. */
		/*alloc-2 */
		IMMSV_ATTR_MODS_LIST *p = calloc(1, sizeof(IMMSV_ATTR_MODS_LIST));
		p->attrModType = attrMod->modType;
		p->attrValue.attrName.size = strlen(attrMod->modAttr.attrName) + 1;

		/* alloc 3 */
		p->attrValue.attrName.buf = malloc(p->attrValue.attrName.size);
		strncpy(p->attrValue.attrName.buf, attrMod->modAttr.attrName, p->attrValue.attrName.size);

		p->attrValue.attrValuesNumber = attrMod->modAttr.attrValuesNumber;
		p->attrValue.attrValueType = attrMod->modAttr.attrValueType;

		if (attrMod->modAttr.attrValuesNumber) {	/*At least one value */
			const SaImmAttrValueT *avarr = attrMod->modAttr.attrValues;
			/*alloc-4 */
			imma_copyAttrValue(&(p->attrValue.attrValue), attrMod->modAttr.attrValueType, avarr[0]);

			if (attrMod->modAttr.attrValuesNumber > 1) {	/*Multiple values */
				unsigned int numAdded = attrMod->modAttr.attrValuesNumber - 1;
				unsigned int i;
				for (i = 1; i <= numAdded; ++i) {
					/*alloc-5 */
					IMMSV_EDU_ATTR_VAL_LIST *al = calloc(1, sizeof(IMMSV_EDU_ATTR_VAL_LIST));
					/*alloc-6 */
					imma_copyAttrValue(&(al->n), attrMod->modAttr.attrValueType, avarr[i]);
					al->next = p->attrValue.attrMoreValues;	/*NULL initially */
					p->attrValue.attrMoreValues = al;
				}	/*for */
			}	/*Multiple values */
		}
		/*At least one value */
		p->next = evt.info.immnd.info.objModify.attrMods;	/*NULL initially. */
		evt.info.immnd.info.objModify.attrMods = p;
	}

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, FALSE);

	TRACE("objectModify send RETURNED:%u", rc);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is after a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here */

		free(out_evt);
	}

 mds_send_fail:
	/*We may be un-locked here but this should not matter. */

	if (evt.info.immnd.info.objModify.objectName.buf) {	/*free-1 */
		free(evt.info.immnd.info.objModify.objectName.buf);
		evt.info.immnd.info.objModify.objectName.buf = NULL;
	}

	while (evt.info.immnd.info.objModify.attrMods) {

		IMMSV_ATTR_MODS_LIST *p = evt.info.immnd.info.objModify.attrMods;
		evt.info.immnd.info.objModify.attrMods = p->next;
		p->next = NULL;

		if (p->attrValue.attrName.buf) {
			free(p->attrValue.attrName.buf);	/*free-3 */
			p->attrValue.attrName.buf = NULL;
		}

		if (p->attrValue.attrValuesNumber) {
			immsv_evt_free_att_val(&(p->attrValue.attrValue),	/*free-4 */
					       p->attrValue.attrValueType);

			while (p->attrValue.attrMoreValues) {
				IMMSV_EDU_ATTR_VAL_LIST *al = p->attrValue.attrMoreValues;
				p->attrValue.attrMoreValues = al->next;
				al->next = NULL;
				immsv_evt_free_att_val(&(al->n), p->attrValue.attrValueType);	/*free-6 */
				free(al);	/*free-5 */
			}
		}

		free(p);	/*free-2 */
	}

 ccb_not_found:
 ao_not_found:
 client_not_found:
 stale_handle:
 ccb_init_fail:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
  Name          :  saImmOmCcbObjectDelete
 
  Description   :  Deletes a config object in the IMM.
                   This a blocking syncronous call.

                   
  Arguments     :  ccbHandle - Ccb Handle
                   objectName - A pointer to the name of the object where
                                delete starts from.

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
SaAisErrorT saImmOmCcbObjectDelete(SaImmCcbHandleT ccbHandle, const SaNameT *objectName)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_CCB_NODE *ccb_node = NULL;
	NCS_BOOL locked = FALSE;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (!objectName || (objectName->length == 0)) {
		TRACE_2("Empty object-name");
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	locked = TRUE;

	/*Get the CCB info */
	proc_rc = imma_ccb_node_get(&cb->ccb_tree, &ccbHandle, &ccb_node);
	if (!ccb_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		TRACE_2("Ccb handle not valid");
		goto ccb_not_found;
	}

	/*Get the Admin Owner info  */
	proc_rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &(ccb_node->mAdminOwnerHdl), &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No Amin-Owner associated with Ccb");
		goto ao_not_found;
	}

	/*Look up client node also, to verify that the client handle
	   is still active. */

	proc_rc = imma_client_node_get(&cb->client_tree, &(ccb_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No valid SaImmHandleT associated with Ccb");
		goto client_not_found;
	}

	assert(ccb_node->mImmHandle == ao_node->mImmHandle);

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ccb_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	if (ccb_node->mTerminated) {
		rc = imma_newCcbId(cb, ccb_node, ao_node, &locked);
		if (rc != SA_AIS_OK) {
			goto ccb_init_fail;
		}
	}

	/* Populate the Object-Delete event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_OBJ_DELETE;

	evt.info.immnd.info.objDelete.adminOwnerId = ao_node->mAdminOwnerId;
	evt.info.immnd.info.objDelete.ccbId = ccb_node->mCcbId;

	evt.info.immnd.info.objDelete.objectName.size = strlen((char *)objectName->value) + 1;

	if (objectName->length + 1 < evt.info.immnd.info.objDelete.objectName.size) {
		evt.info.immnd.info.objDelete.objectName.size = objectName->length + 1;
	}

	/*alloc-1 */
	evt.info.immnd.info.objDelete.objectName.buf = malloc(evt.info.immnd.info.objDelete.objectName.size);
	if (evt.info.immnd.info.objDelete.objectName.buf == NULL) {
		rc = SA_AIS_ERR_NO_MEMORY;
		goto mds_send_fail;
	}
	strncpy(evt.info.immnd.info.objDelete.objectName.buf,
		(char *)objectName->value, evt.info.immnd.info.objDelete.objectName.size);
	evt.info.immnd.info.objDelete.objectName.buf[evt.info.immnd.info.objDelete.objectName.size - 1] = '\0';

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, FALSE);

	TRACE("objectDelete send RETURNED:%u", rc);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is after a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here */

		free(out_evt);
	}

 mds_send_fail:
	/*We may be un-locked here but this should not matter.
	   We are freing heap objects that should only be vissible from this
	   thread. */

	if (evt.info.immnd.info.objDelete.objectName.buf) {	/*free-1 */
		free(evt.info.immnd.info.objDelete.objectName.buf);
		evt.info.immnd.info.objDelete.objectName.buf = NULL;
	}

 ccb_not_found:
 ao_not_found:
 client_not_found:
 stale_handle:
 ccb_init_fail:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

/****************************************************************************
  Name          :  saImmOmCcbApply
 
  Description   :  Applies (commits) a CCB.
                   This a blocking syncronous call.

                   
  Arguments     :  ccbHandle - Ccb Handle

  Return Values :  Refer to SAI-AIS specification for various return values.
 
******************************************************************************/
SaAisErrorT saImmOmCcbApply(SaImmCcbHandleT ccbHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_CCB_NODE *ccb_node = NULL;
	NCS_BOOL locked = FALSE;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	locked = TRUE;

	/*Get the CCB info */
	proc_rc = imma_ccb_node_get(&cb->ccb_tree, &ccbHandle, &ccb_node);
	if (!ccb_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		TRACE_2("Ccb handle not valid");
		goto ccb_not_found;
	}

	/* Get the Admin Owner info  */
	proc_rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &(ccb_node->mAdminOwnerHdl), &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No Amin-Owner associated with Ccb");
		goto ao_not_found;
	}

	proc_rc = imma_client_node_get(&cb->client_tree, &(ccb_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No valid SaImmHandleT associated with Ccb");
		goto client_not_found;
	}

	assert(ccb_node->mImmHandle == ao_node->mImmHandle);

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ccb_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	if (ccb_node->mTerminated) {
		rc = SA_AIS_ERR_FAILED_OPERATION;
		TRACE_2("Ccb was terminated already");
		goto ccb_was_terminated;
	}

	/* Populate the CcbApply event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_CCB_APPLY;

	evt.info.immnd.info.ccbId = ccb_node->mCcbId;

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, FALSE);

	TRACE("ccbApply returned %u", rc);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is after a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here */

		free(out_evt);
	}

	ccb_node->mTerminated = 1;

 mds_send_fail:
 ccb_not_found:
 ao_not_found:
 client_not_found:
 stale_handle:
 ccb_was_terminated:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

/****************************************************************************
  Name          :  saImmOmAdminOperationInvoke/_2
 
  Description   :  Invoke an Administrative Operation on an object in the IMM.
                   This a blocking syncronous call.

                   
  Arguments     :  ownerHandle - AdminOwner handle.
                   objectName - A pointer to the name of the object
                           on which the operation is to be invoked.
                   operationId - An id identifying the operation.
                   params - Parameters to be supplied to the operation.
                   operationReturnValue - The return value from implementer.
                   timeout - Invocation is considered to be failed if this 
                             function does not complete by the time specified.

  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         : Note the TWO return values!
******************************************************************************/
#ifdef IMM_A_01_01
SaAisErrorT saImmOmAdminOperationInvoke(SaImmAdminOwnerHandleT ownerHandle,
					const SaNameT *objectName,
					SaImmAdminOperationIdT operationId,
					const SaImmAdminOperationParamsT
					** params, SaAisErrorT *operationReturnValue, SaTimeT timeout)
{
	/* convert the SaImmAdminOperationParamsT to SaImmAdminOperationParamsT_2 */
	return saImmOmAdminOperationInvoke_2(ownerHandle, objectName, 0, operationId,
					     (const SaImmAdminOperationParamsT_2 **)params, operationReturnValue,
					     timeout);
}
#endif

SaAisErrorT saImmOmAdminOperationInvoke_2(SaImmAdminOwnerHandleT ownerHandle,
					  const SaNameT *objectName,
					  SaImmContinuationIdT continuationId,
					  SaImmAdminOperationIdT operationId,
					  const SaImmAdminOperationParamsT_2 **params,
					  SaAisErrorT *operationReturnValue, SaTimeT timeout)
{
	SaAisErrorT rc = SA_AIS_OK;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = TRUE;

	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((objectName == NULL) || (operationReturnValue == NULL) ||
	    (objectName->length == 0) || (objectName->length >= SA_MAX_NAME_LENGTH)
	    || (params == NULL)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	if (!is_adminop_params_valid(params)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/*Overwrite any old/uninitialized value. */
	*operationReturnValue = SA_AIS_ERR_NO_SECTIONS;	/* Set to bad value to prevent user mistakes. */

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	/*locked == TRUE already */

	/* Get the Admin Owner info  */
	proc_rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &ownerHandle, &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto ao_not_found;
	}

	/* Look up client node also, to verify that the client handle
	   is still active. */

	proc_rc = imma_client_node_get(&cb->client_tree, &(ao_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No valid SaImmHandleT associated with Admin owner");
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ao_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* convert the timeout to 10 ms value and add it to the sync send 
	   timeout */
	timeout = m_IMMSV_CONVERT_SATIME_TEN_MILLI_SEC(timeout);

	if (timeout < NCS_SAF_MIN_ACCEPT_TIME) {
		timeout = IMMSV_WAIT_TIME;
	}

	/* Populate the Admin-op event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_IMM_ADMOP;

	evt.info.immnd.info.admOpReq.adminOwnerId = ao_node->mAdminOwnerId;
	evt.info.immnd.info.admOpReq.operationId = operationId;
	evt.info.immnd.info.admOpReq.continuationId = continuationId;	/*New API */

	/* The timeout is packed here and goes over fevs, need to
	   consider how the timing of FEVS itself should be subtracted.
	   The timeout is also set up in the FEVS call so it IS handled
	   on the agent side. Not sure why we need it on the server, other than
	   as a "leniency indicator". That is if timeout is large we may need to
	   increase the wait time in the server above some default. */
	evt.info.immnd.info.admOpReq.timeout = timeout;

	if ((immInvocations < 0) || (immInvocations == 0x7fffffff)) {
		immInvocations = 0;
	}
	TRACE("immInvocations:%i", immInvocations);
	evt.info.immnd.info.admOpReq.invocation = ++immInvocations;

	evt.info.immnd.info.admOpReq.objectName.size = strnlen((char *)objectName->value, objectName->length) + 1;
	if (objectName->length + 1 < evt.info.immnd.info.admOpReq.objectName.size) {
		evt.info.immnd.info.admOpReq.objectName.size = objectName->length + 1;
	}

	/*alloc-1 */
	evt.info.immnd.info.admOpReq.objectName.buf = malloc(evt.info.immnd.info.admOpReq.objectName.size);

	strncpy(evt.info.immnd.info.admOpReq.objectName.buf,
		(char *)objectName->value, evt.info.immnd.info.admOpReq.objectName.size);
	evt.info.immnd.info.admOpReq.objectName.buf[evt.info.immnd.info.admOpReq.objectName.size - 1] = '\0';

	assert(evt.info.immnd.info.admOpReq.params == NULL);
	const SaImmAdminOperationParamsT_2 *param;
	int i;
	for (i = 0; params[i]; ++i) {
		param = params[i];
		/*alloc-2 */
		IMMSV_ADMIN_OPERATION_PARAM *p = malloc(sizeof(IMMSV_ADMIN_OPERATION_PARAM));
		memset(p, 0, sizeof(IMMSV_ADMIN_OPERATION_PARAM));
		TRACE("PARAM:%s \n", param->paramName);

		p->paramName.size = strlen(param->paramName) + 1;
		if (p->paramName.size >= SA_MAX_NAME_LENGTH) {
			TRACE_2("Param name too long");
			rc = SA_AIS_ERR_INVALID_PARAM;
			free(p);
			goto mds_send_fail;
		}
		/*alloc-3 */
		p->paramName.buf = malloc(p->paramName.size);
		strncpy(p->paramName.buf, param->paramName, p->paramName.size);

		p->paramType = param->paramType;
		/*alloc-4 */
		imma_copyAttrValue(&(p->paramBuffer), param->paramType, param->paramBuffer);

		p->next = evt.info.immnd.info.admOpReq.params;	/*NULL initially. */
		evt.info.immnd.info.admOpReq.params = p;
	}

	/* NOTE: Re-implement to send ND->ND instead of using FEVS, 
	   less resources used and probably faster. */

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, timeout, cl_node->handle, &locked, FALSE);

	TRACE("Fevs send RETURNED:%u", rc);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is after a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		if (out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR) {
			rc = out_evt->info.imma.info.errRsp.error;
			assert(rc != SA_AIS_OK);
			*operationReturnValue = SA_AIS_ERR_NO_SECTIONS;	//Bogus result since error is set.
		} else {
			assert(out_evt->info.imma.type == IMMA_EVT_ND2A_ADMOP_RSP);
			*operationReturnValue = out_evt->info.imma.info.admOpRsp.result;
		}

		/* We dont need the lock here */
		free(out_evt);
	}

 mds_send_fail:
	/*We may be un-locked here but this should not matter.
	   We are freing heap objects that should only be vissible from this
	   thread. */

	/*TODO 12345 The code below should be moved to common ? 
	   This to allow re-use by all allocators of this event type.
	   At least it needs to be duplicated in immnd_evt.c/immnd_evt_destroy */

	if (evt.info.immnd.info.admOpReq.objectName.buf) {	/*free-1 */
		free(evt.info.immnd.info.admOpReq.objectName.buf);
		evt.info.immnd.info.admOpReq.objectName.buf = NULL;
	}
	while (evt.info.immnd.info.admOpReq.params) {
		IMMSV_ADMIN_OPERATION_PARAM *p = evt.info.immnd.info.admOpReq.params;
		evt.info.immnd.info.admOpReq.params = p->next;

		if (p->paramName.buf) {	/*free-3 */
			free(p->paramName.buf);
			p->paramName.buf = NULL;
		}
		immsv_evt_free_att_val(&(p->paramBuffer), p->paramType);	/*free-4 */
		p->next = NULL;
		free(p);	/*free-2 */
	}

 ao_not_found:
 client_not_found:
 stale_handle:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

 done:
	TRACE_LEAVE();
	return rc;
}

static int push_async_adm_op_continuation(IMMA_CB *cb,	//in
					  SaInt32T invocation,	//in
					  SaImmHandleT immHandle,	//in
					  SaInvocationT userInvoc)	//in
{
	IMMA_CONTINUATION_RECORD *cr = cb->imma_continuations;

	TRACE("PUSH %i %llu %llu", invocation, immHandle, userInvoc);

	/* Check that there is not already a continuation with the same */
	/* user invocation id. */

	while (cr) {
		if (cr->userInvoc == userInvoc) {
			return 0;
		}
		cr = cr->next;
	}

	/* Ok add the new continuation. */
	cr = (IMMA_CONTINUATION_RECORD *)
	    malloc(sizeof(IMMA_CONTINUATION_RECORD));
	cr->invocation = invocation;
	cr->userInvoc = userInvoc;
	cr->immHandle = immHandle;
	cr->next = cb->imma_continuations;
	cb->imma_continuations = cr;
	return 1;
}

/****************************************************************************
  Name          :  saImmOmAdminOperationInvokeAsync/_2
 
  Description   :  Invoke an Administrative Operation on an object in the IMM.
                   This an asynchronous non-blocking call.
                   
  Arguments     :  ownerHandle - AdminOwner handle.
                   userInvocation - An invocation id allowing the user
                        to match the asynchronous return with the call.
                   objectName - A pointer to the name of the object
                           on which the operation is to be invoked.
                   operationId - An id identifying the operation.
                   params - Parameters to be supplied to the operation.

  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
#ifdef IMM_A_01_01
SaAisErrorT saImmOmAdminOperationInvokeAsync(SaImmAdminOwnerHandleT ownerHandle,
					     SaInvocationT userInvocation,
					     const SaNameT *objectName,
					     SaImmAdminOperationIdT operationId,
					     const SaImmAdminOperationParamsT ** params)
{
	/* convert the SaImmAdminOperationParamsT to SaImmAdminOperationParamsT_2 */
	return saImmOmAdminOperationInvokeAsync_2(ownerHandle, userInvocation, objectName, 0,
						  operationId, (const SaImmAdminOperationParamsT_2 **)params);
}
#endif

SaAisErrorT saImmOmAdminOperationInvokeAsync_2(SaImmAdminOwnerHandleT ownerHandle,
					       SaInvocationT userInvocation,
					       const SaNameT *objectName,
					       SaImmContinuationIdT continuationId,
					       SaImmAdminOperationIdT operationId,
					       const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc = SA_AIS_OK;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	NCS_BOOL locked = TRUE;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((objectName == NULL) || (objectName->length == 0) ||
	    (objectName->length >= SA_MAX_NAME_LENGTH) || (params == NULL)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	if (!is_adminop_params_valid(params)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	/*locked == TRUE already */

	/* Get the Admin Owner info  */
	imma_admin_owner_node_get(&cb->admin_owner_tree, &ownerHandle, &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto ao_not_found;
	}

	/* Look up client node also, to verify that the client handle
	   is still active. */

	rc = imma_client_node_get(&cb->client_tree, &(ao_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		LOG_ER("No valid SaImmHandleT associated with Admin owner");
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ao_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* TODO: Should the timeout value be set to any sensible value ? */

	if (cl_node->o.mCallbk.saImmOmAdminOperationInvokeCallback == NULL) {
		rc = SA_AIS_ERR_INIT;
		TRACE_2("The SaImmOmAdminOperationInvokeCallbackT "
			"function was not set in the initialization of the handle "
			"=> saImmOmAdminOperationInvokeAsync can not reply");
		goto no_callback;
	}

	/* Populate & Send the Open Event to IMMND */
	memset(&evt, 0, sizeof(IMMSV_EVT));

	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_IMM_ADMOP_ASYNC;

	evt.info.immnd.info.admOpReq.adminOwnerId = ao_node->mAdminOwnerId;
	evt.info.immnd.info.admOpReq.operationId = operationId;
	evt.info.immnd.info.admOpReq.continuationId = continuationId;	/*New API */
	evt.info.immnd.info.admOpReq.timeout = 0;	/* or IMMSV_WAIT_TIME? */
	if ((immInvocations < 0) || (immInvocations == 0x7fffffff)) {
		immInvocations = 0;
	}
	TRACE("immInvocations:%i", immInvocations);

	evt.info.immnd.info.admOpReq.invocation = -(++immInvocations);
	/*Negate invoc to encode async */

	evt.info.immnd.info.admOpReq.objectName.size = strnlen((char *)objectName->value, objectName->length) + 1;
	if (objectName->length + 1 < evt.info.immnd.info.admOpReq.objectName.size) {
		evt.info.immnd.info.admOpReq.objectName.size = objectName->length + 1;
	}

	/*alloc-1 */
	evt.info.immnd.info.admOpReq.objectName.buf = calloc(1, evt.info.immnd.info.admOpReq.objectName.size);

	strncpy(evt.info.immnd.info.admOpReq.objectName.buf,
		(char *)objectName->value, evt.info.immnd.info.admOpReq.objectName.size);
	/* evt.info.immnd.info.admOpReq.objectName.buf[objectName->length] = '\0'; */

	assert(evt.info.immnd.info.admOpReq.params == NULL);

	const SaImmAdminOperationParamsT_2 *param;
	int i;
	for (i = 0; params[i]; ++i) {
		param = params[i];
		/*alloc-2 */
		IMMSV_ADMIN_OPERATION_PARAM *p = malloc(sizeof(IMMSV_ADMIN_OPERATION_PARAM));
		memset(p, 0, sizeof(IMMSV_ADMIN_OPERATION_PARAM));
		TRACE("PARAM:%s \n", param->paramName);

		p->paramName.size = strlen(param->paramName) + 1;
		if (p->paramName.size >= SA_MAX_NAME_LENGTH) {
			TRACE_2("Param name too long");
			rc = SA_AIS_ERR_INVALID_PARAM;
			free(p);
			goto mds_send_fail;
		}

		/*alloc-3 */
		p->paramName.buf = malloc(p->paramName.size);
		strncpy(p->paramName.buf, param->paramName, p->paramName.size);

		p->paramType = param->paramType;
		/*alloc-4 */
		imma_copyAttrValue(&(p->paramBuffer), param->paramType, param->paramBuffer);

		p->next = evt.info.immnd.info.admOpReq.params;	/*NULL initially. */
		evt.info.immnd.info.admOpReq.params = p;
	}

	/* NOTE: Re-implement to send ND->ND instead of using FEVS, 
	   less resources used and probably faster. */

	/* out_evt is NULL => fevs is async */
	rc = imma_evt_fake_evs(cb, &evt, NULL, 0, cl_node->handle, &locked, FALSE);
	TRACE("Fevs send RETURNED:%u", rc);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (!locked) {
		if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
			rc = SA_AIS_ERR_LIBRARY;
			TRACE_1("Lock failed");
			goto mds_send_fail;
		} else {
			locked = TRUE;
		}
	}

	if (!push_async_adm_op_continuation(cb,
					    evt.info.immnd.info.admOpReq.invocation,
					    ao_node->mImmHandle, userInvocation)) {
		TRACE_2("Provided invocation id (%llu) "
			"is not unique, not even in this client instance", userInvocation);
		rc = SA_AIS_ERR_INVALID_PARAM;
	}

 mds_send_fail:
	/*We may be un-locked here but this should not matter.
	   We are freing heap objects that should only be vissible from this
	   thread. */

	/* NOTE: 12345 The code below should be moved to common ? 
	   This to allow re-use by all allocators of this event type.
	   At least it needs to be duplicated in immnd_evt.c/immnd_evt_destroy */
	if (evt.info.immnd.info.admOpReq.objectName.buf) {	/*free-1 */
		free(evt.info.immnd.info.admOpReq.objectName.buf);
		evt.info.immnd.info.admOpReq.objectName.buf = NULL;
		evt.info.immnd.info.admOpReq.objectName.size = 0;
	}
	while (evt.info.immnd.info.admOpReq.params) {
		IMMSV_ADMIN_OPERATION_PARAM *p = evt.info.immnd.info.admOpReq.params;
		evt.info.immnd.info.admOpReq.params = p->next;

		if (p->paramName.buf) {	/*free-3 */
			free(p->paramName.buf);
			p->paramName.buf = NULL;
		}
		immsv_evt_free_att_val(&(p->paramBuffer), p->paramType);	/*free-4 */
		p->next = NULL;
		free(p);	/*free-2 */
	}
 ao_not_found:
 client_not_found:
 stale_handle:
 no_callback:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:
 done:
	TRACE_LEAVE();
	return rc;
}

/****************************************************************************
  Name          :  saImmOmAdminOperationContinue
 
  Description   :  Allows a process to take over the continuation of an
                   administrative operation that was initiated with a 
                   admin owner handle that has been finalized before the
                   reply was received.
                   
                   
  Arguments     :  ownerHandle - AdminOwner handle.
                   userInvocation - An invocation id allowing the user
                        to match the asynchronous return with the call.
                   objectName - A pointer to the name of the object
                           on which the operation is to be invoked.
                           Actually redundant information.
                   continuationId - identifies the corresponding pervious
                   invocation.

  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/

SaAisErrorT saImmOmAdminOperationContinue(SaImmAdminOwnerHandleT ownerHandle,
					  const SaNameT *objectName,
					  SaImmContinuationIdT continuationId, SaAisErrorT *operationReturnValue)
{
	TRACE_2("saImmOmAdminOperationContinue not implemented " "use saImmOmAdminOperationContinueAsync instead");
	return SA_AIS_ERR_NOT_SUPPORTED;
}

SaAisErrorT saImmOmAdminOperationContinueAsync(SaImmAdminOwnerHandleT ownerHandle,
					       SaInvocationT invocation,
					       const SaNameT *objectName, SaImmContinuationIdT continuationId)
{
	TRACE_2("saImmOmAdminOperationContinueAsync not yet implemented");
	return SA_AIS_ERR_NOT_SUPPORTED;
}

SaAisErrorT saImmOmAdminOperationContinuationClear(SaImmAdminOwnerHandleT ownerHandle,
						   const SaNameT *objectName, SaImmContinuationIdT continuationId)
{
	TRACE_2("saImmOmAdminOperationContinuationClear not yet implemented");
	return SA_AIS_ERR_NOT_SUPPORTED;
}

#ifdef IMM_A_01_01
SaAisErrorT saImmOmClassCreate(SaImmHandleT immHandle,
			       const SaImmClassNameT className,
			       SaImmClassCategoryT classCategory, const SaImmAttrDefinitionT ** attrDefinitions)
{
	const SaImmAttrDefinitionT *attr;
	SaImmAttrDefinitionT_2 *attr2;
	const SaImmAttrDefinitionT_2 *attr2c;
	int nrofAttrDefs = 0;
	int ix = 0;
	SaAisErrorT rc = SA_AIS_OK;

	for (attr = attrDefinitions[0]; attr != 0; attr = attrDefinitions[++ix]) {
		++nrofAttrDefs;
	}

	const SaImmAttrDefinitionT_2 **attrDefinitions2 = (const SaImmAttrDefinitionT_2 **)
	    calloc(nrofAttrDefs + 1, sizeof(SaImmAttrDefinitionT_2 *));

	for (attr = attrDefinitions[0], ix = 0; attr != 0; attr = attrDefinitions[++ix]) {
		attrDefinitions2[ix] = (SaImmAttrDefinitionT_2 *)
		    calloc(1, sizeof(SaImmAttrDefinitionT_2));

		attr2 = (SaImmAttrDefinitionT_2 *)attrDefinitions2[ix];
		attr2->attrName = attr->attrName;
		attr2->attrValueType = attr->attrValueType;
		attr2->attrFlags = attr->attrFlags;
		attr2->attrDefaultValue = attr->attrDefaultValue;
	}

	rc = saImmOmClassCreate_2(immHandle, className, classCategory, attrDefinitions2);

	for (attr2c = attrDefinitions2[0], ix = 0; attr2c != 0; attr2c = attrDefinitions2[++ix]) {
		free((SaImmAttrDefinitionT_2 *)attr2c);
	}

	free(attrDefinitions2);
	return rc;
}
#endif

SaAisErrorT saImmOmClassCreate_2(SaImmHandleT immHandle,
				 const SaImmClassNameT className,
				 SaImmClassCategoryT classCategory, const SaImmAttrDefinitionT_2 **attrDefinitions)
{
	SaAisErrorT rc = SA_AIS_OK;
	NCS_BOOL locked = TRUE;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	SaTimeT timeout = IMMSV_WAIT_TIME;
	IMMSV_ATTR_DEF_LIST *sysattr = NULL;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((className == NULL) || (attrDefinitions == NULL)) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* Draft Validations */

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	/*locked is true already */

	rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* Populate the ClassCreate event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_CLASS_CREATE;

	evt.info.immnd.info.classDescr.className.size = strlen(className) + 1;
	if (evt.info.immnd.info.classDescr.className.size == 1) {
		/*ABT bugfix 081029 Empty classname should not be allowed. */
		rc = SA_AIS_ERR_INVALID_PARAM;
		TRACE("Zero length class name, not allowed.");
		goto mds_send_fail;
	}
	evt.info.immnd.info.classDescr.className.buf = malloc(evt.info.immnd.info.classDescr.className.size);	/*alloc-1 */
	strncpy(evt.info.immnd.info.classDescr.className.buf, className,
		(size_t)evt.info.immnd.info.classDescr.className.size);

	evt.info.immnd.info.classDescr.classCategory = classCategory;
	TRACE("name: %s category:%u", className, classCategory);
	const SaImmAttrDefinitionT_2 *attr;
	int persistent = 0;
	int i = 0;
	attr = attrDefinitions[0];
	int attrClNameExist = 0;
	int attrAdmNameExist = 0;
	int attrImplNameExist = 0;
	for (; attr != 0; attr = attrDefinitions[++i]) {
		/* Ignore system attribute definitions that are
		   loaded since they are indistinguishable from being set
		   by the user. Better that they are redefined below each 
		   time by the system, even when the class is loaded from
		   backup. */
		if (strcmp(attr->attrName, sysaClName) == 0) {
			continue;
		} else if (strcmp(attr->attrName, sysaAdmName) == 0) {
			continue;
		} else if (strcmp(attr->attrName, sysaImplName) == 0) {
			continue;
		}

		IMMSV_ATTR_DEF_LIST *p =	/*alloc-2 */
		    malloc(sizeof(IMMSV_ATTR_DEF_LIST));
		memset(p, 0, sizeof(IMMSV_ATTR_DEF_LIST));

		p->d.attrName.size = strlen(attr->attrName) + 1;
		if (p->d.attrName.size == 1) {
			rc = SA_AIS_ERR_INVALID_PARAM;
			TRACE("Zero length attribute name in class %s def, not allowed.",
			      evt.info.immnd.info.classDescr.className.buf);
			free(p);
			goto mds_send_fail;
		}
		p->d.attrName.buf = malloc(p->d.attrName.size);	/* alloc-3 */
		strncpy(p->d.attrName.buf, attr->attrName, p->d.attrName.size);

		p->d.attrValueType = attr->attrValueType;
		if (!is_valid_type(p->d.attrValueType)) {
			rc = SA_AIS_ERR_INVALID_PARAM;
			TRACE("Unknown type not allowed for attr:%s class%s",
			      p->d.attrName.buf, evt.info.immnd.info.classDescr.className.buf);
			free(p->d.attrName.buf);	/* free-3 */
			p->d.attrName.buf = NULL;
			free(p);	/* free-2 */
			goto mds_send_fail;
		}
		p->d.attrFlags = attr->attrFlags;
		persistent = persistent | (attr->attrFlags & SA_IMM_ATTR_PERSISTENT);

		if (attr->attrDefaultValue) {
			p->d.attrDefaultValue =	/*alloc-4.1 */
			    malloc(sizeof(IMMSV_EDU_ATTR_VAL));
			memset(p->d.attrDefaultValue, 0, sizeof(IMMSV_EDU_ATTR_VAL));
			/*alloc-4.2 */
			imma_copyAttrValue(p->d.attrDefaultValue, attr->attrValueType, attr->attrDefaultValue);
		}

		p->next = evt.info.immnd.info.classDescr.attrDefinitions;	/*NULL initially */
		evt.info.immnd.info.classDescr.attrDefinitions = p;
	}

	/* Add system attribute class-name  */
	if (!attrClNameExist) {
		/*TRACE("Creating class name attribute def"); */
		sysattr =	/*alloc-2 */
		    malloc(sizeof(IMMSV_ATTR_DEF_LIST));
		memset(sysattr, 0, sizeof(IMMSV_ATTR_DEF_LIST));

		sysattr->d.attrName.size = strlen(sysaClName) + 1;
		sysattr->d.attrName.buf = malloc(sysattr->d.attrName.size);	/*alloc-3 */
		strncpy(sysattr->d.attrName.buf, sysaClName, sysattr->d.attrName.size);
		sysattr->d.attrValueType = SA_IMM_ATTR_SASTRINGT;
		if (classCategory == SA_IMM_CLASS_CONFIG) {
			sysattr->d.attrFlags |= SA_IMM_ATTR_CONFIG;
		} else if (classCategory == SA_IMM_CLASS_RUNTIME) {
			sysattr->d.attrFlags |= (persistent) ?
			    (SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED |
			     SA_IMM_ATTR_PERSISTENT) : (SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED);
		}
		sysattr->d.attrNtfId = 0;	/*alloc-4.1 */
		sysattr->d.attrDefaultValue = malloc(sizeof(IMMSV_EDU_ATTR_VAL));
		memset(sysattr->d.attrDefaultValue, 0, sizeof(IMMSV_EDU_ATTR_VAL));
		/*alloc-4.2 */
		imma_copyAttrValue(sysattr->d.attrDefaultValue, SA_IMM_ATTR_SASTRINGT, (SaImmAttrValueT)&className);
		sysattr->next = evt.info.immnd.info.classDescr.attrDefinitions;
		evt.info.immnd.info.classDescr.attrDefinitions = sysattr;
	}

	/* Add system attribute admin-owner */
	if (!attrAdmNameExist) {
		/*TRACE("Creating admin-owner name attribute def"); */
		sysattr =	/*alloc-2 */
		    malloc(sizeof(IMMSV_ATTR_DEF_LIST));
		memset(sysattr, 0, sizeof(IMMSV_ATTR_DEF_LIST));

		sysattr->d.attrName.size = strlen(sysaAdmName) + 1;
		sysattr->d.attrName.buf = malloc(sysattr->d.attrName.size);	/*alloc-3 */
		strncpy(sysattr->d.attrName.buf, sysaAdmName, sysattr->d.attrName.size);
		sysattr->d.attrValueType = SA_IMM_ATTR_SASTRINGT;
		/* Should this attribute really be a config attribute ?
		   Should it really be allowed to be persistent ? */
		if (classCategory == SA_IMM_CLASS_CONFIG) {
			sysattr->d.attrFlags |= SA_IMM_ATTR_CONFIG;
		} else if (classCategory == SA_IMM_CLASS_RUNTIME) {
			sysattr->d.attrFlags |= (persistent) ?
			    (SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED |
			     SA_IMM_ATTR_PERSISTENT) : (SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED);
		}
		sysattr->d.attrNtfId = 0;
		sysattr->d.attrDefaultValue = NULL;
		sysattr->next = evt.info.immnd.info.classDescr.attrDefinitions;
		evt.info.immnd.info.classDescr.attrDefinitions = sysattr;
	}

	/* Add system attribute implementer-name */
	if (!attrImplNameExist) {
		/*TRACE("Creating implementer name attribute def"); */
		sysattr =	/*alloc-2 */
		    malloc(sizeof(IMMSV_ATTR_DEF_LIST));
		memset(sysattr, 0, sizeof(IMMSV_ATTR_DEF_LIST));

		sysattr->d.attrName.size = strlen(sysaImplName) + 1;
		sysattr->d.attrName.buf = malloc(sysattr->d.attrName.size);	/*alloc-3 */
		strncpy(sysattr->d.attrName.buf, sysaImplName, sysattr->d.attrName.size);
		sysattr->d.attrValueType = SA_IMM_ATTR_SASTRINGT;
		/* Should this attribute really be a config attribute ?
		   Should it really be allowed to be persistent ? 
		   Config=> persistent but Runtime => can be changed by implementer.
		 */
		if (classCategory == SA_IMM_CLASS_CONFIG) {
			sysattr->d.attrFlags |= SA_IMM_ATTR_CONFIG;
		} else if (classCategory == SA_IMM_CLASS_RUNTIME) {
			sysattr->d.attrFlags |= (persistent) ?
			    (SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED |
			     SA_IMM_ATTR_PERSISTENT) : (SA_IMM_ATTR_RUNTIME | SA_IMM_ATTR_CACHED);
		}
		sysattr->d.attrNtfId = 0;
		sysattr->d.attrDefaultValue = NULL;
		sysattr->next = evt.info.immnd.info.classDescr.attrDefinitions;
		evt.info.immnd.info.classDescr.attrDefinitions = sysattr;
	}

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, timeout, cl_node->handle, &locked, FALSE);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is a blocking op. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;
		TRACE("Return code:%u", rc);

		/* We dont need the lock here */

		free(out_evt);
	}

 mds_send_fail:

	free(evt.info.immnd.info.classDescr.className.buf);	/*free-1 */
	evt.info.immnd.info.classDescr.className.buf = NULL;
	evt.info.immnd.info.classDescr.className.size = 0;

	/*free-2 free-3 free-4.1 free-4.2 */
	immsv_free_attrdefs_list(evt.info.immnd.info.classDescr.attrDefinitions);
	evt.info.immnd.info.classDescr.attrDefinitions = NULL;

 client_not_found:
 stale_handle:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

#ifdef IMM_A_01_01
SaAisErrorT saImmOmClassDescriptionGet(SaImmHandleT immHandle,
				       const SaImmClassNameT className,
				       SaImmClassCategoryT *classCategory, SaImmAttrDefinitionT *** attrDefinition)
{
	SaImmAttrDefinitionT_2 **attrDefinitions2;
	SaAisErrorT rc = SA_AIS_OK;

	rc = saImmOmClassDescriptionGet_2(immHandle, className, classCategory, &attrDefinitions2);

	if (rc == SA_AIS_OK) {
		const SaImmAttrDefinitionT_2 *attr2c;
		SaImmAttrDefinitionT_2 *attr2;
		SaImmAttrDefinitionT *attr;
		int nrofAttrDefs = 0;
		int ix = 0;

		for (attr2c = attrDefinitions2[0]; attr2c; attr2c = attrDefinitions2[++ix]) {
			++nrofAttrDefs;
		}

		SaImmAttrDefinitionT **attrDefinitions = (SaImmAttrDefinitionT **)
		    calloc(nrofAttrDefs + 1, sizeof(SaImmAttrDefinitionT *));

		for (attr2c = attrDefinitions2[0], ix = 0; attr2c; attr2c = attrDefinitions2[++ix]) {
			attrDefinitions[ix] = (SaImmAttrDefinitionT *)
			    calloc(1, sizeof(SaImmAttrDefinitionT));

			attr = (SaImmAttrDefinitionT *) attrDefinitions[ix];
			attr2 = (SaImmAttrDefinitionT_2 *)attr2c;
			attr->attrName = attr2->attrName;
			attr2->attrName = NULL;	//Steal the string
			attr->attrValueType = attr2->attrValueType;
			attr->attrFlags = attr2->attrFlags;
			attr->attrDefaultValue = attr2->attrDefaultValue;
			attr2->attrDefaultValue = NULL;	//Steal the value
		}

		*attrDefinition = attrDefinitions;

		saImmOmClassDescriptionMemoryFree_2(immHandle, attrDefinitions2);
	}

	return rc;
}
#endif

SaAisErrorT saImmOmClassDescriptionGet_2(SaImmHandleT immHandle,
					 const SaImmClassNameT className,
					 SaImmClassCategoryT *classCategory, SaImmAttrDefinitionT_2 ***attrDefinition)
{
	TRACE_ENTER();
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = FALSE;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((className == NULL) || (classCategory == NULL) || (attrDefinition == NULL)) {
		TRACE_LEAVE();
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	locked = TRUE;

	rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* Populate the ClassDescriptionGet event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_CLASS_DESCR_GET;

	evt.info.immnd.info.classDescr.className.size = strlen(className) + 1;
	evt.info.immnd.info.classDescr.className.buf = malloc(evt.info.immnd.info.classDescr.className.size);	/*alloc-0 */
	strncpy(evt.info.immnd.info.classDescr.className.buf,
		className, (size_t)evt.info.immnd.info.classDescr.className.size);

	TRACE("ClassName: %s", className);

	/* Release the CB lock Before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/* IMMND GOES DOWN */
	if (FALSE == cb->is_immnd_up) {
		rc = SA_AIS_ERR_TRY_AGAIN;
		TRACE_2("IMMND is DOWN");
		goto mds_send_fail;
	}

	/* send the request to the IMMND */
	TRACE("ClassDescrGet 3");
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, &evt, &out_evt, IMMSV_WAIT_TIME);

	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;
	case NCSCC_RC_REQ_TIMOUT:
		rc = SA_AIS_ERR_TIMEOUT;
		goto mds_send_fail;
	default:
		rc = SA_AIS_ERR_NO_RESOURCES;
		TRACE_1("MDS returned unexpected error code %u", proc_rc);
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, this was a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		if (out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR) {
			rc = out_evt->info.imma.info.errRsp.error;
			assert(rc != SA_AIS_OK);
		} else {
			assert(out_evt->info.imma.type == IMMA_EVT_ND2A_CLASS_DESCR_GET_RSP);
			int noOfAttributes = 0;
			int i = 0;
			SaImmAttrDefinitionT_2 **attr = NULL;
			size_t attrDataSize = 0;

			*classCategory = out_evt->info.imma.info.classDescr.classCategory;
			IMMSV_ATTR_DEF_LIST *p = out_evt->info.imma.info.classDescr.attrDefinitions;

			while (p) {
				++noOfAttributes;
				p = p->next;
			}

			attrDataSize = sizeof(SaImmAttrDefinitionT_2 *) * (noOfAttributes + 1);
			attr = malloc(attrDataSize);	/*alloc-1 */
			TRACE("Alloc attrdefs array:%p", attr);
			memset(attr, 0, attrDataSize);
			p = out_evt->info.imma.info.classDescr.attrDefinitions;
			for (; i < noOfAttributes; i++) {
				IMMSV_ATTR_DEF_LIST *prev = p;
				IMMSV_ATTR_DEFINITION *q = &(p->d);
				attr[i] = malloc(sizeof(SaImmAttrDefinitionT_2));	/*alloc-2 */
				attr[i]->attrName = malloc(q->attrName.size + 1);	/*alloc-3 */
				strncpy(attr[i]->attrName, (const char *)q->attrName.buf, q->attrName.size + 1);
				attr[i]->attrName[q->attrName.size] = 0;
				attr[i]->attrValueType = q->attrValueType;
				attr[i]->attrFlags = q->attrFlags;
				/* attr[i]->attrNtfId = q->attrNtfId; */

				/*free-Y */
				free(q->attrName.buf);
				q->attrName.buf = NULL;
				q->attrName.size = 0;

				if (!q->attrDefaultValue) {
					attr[i]->attrDefaultValue = 0;
				} else {
					int size = 0;
					switch (q->attrValueType) {
					case SA_IMM_ATTR_SAINT32T:	/*intended fall through */
					case SA_IMM_ATTR_SAUINT32T:
						size = sizeof(SaInt32T);
						break;

					case SA_IMM_ATTR_SAINT64T:	/*intended fall through */
					case SA_IMM_ATTR_SAUINT64T:
						size = sizeof(SaInt64T);
						break;

					case SA_IMM_ATTR_SATIMET:
						size = sizeof(SaTimeT);
						break;

					case SA_IMM_ATTR_SAFLOATT:
						size = sizeof(SaFloatT);
						break;

					case SA_IMM_ATTR_SADOUBLET:
						size = sizeof(SaDoubleT);
						break;

					case SA_IMM_ATTR_SASTRINGT:	/*A bit harsh with assert here ? */
						assert(strlen((char *)q->attrDefaultValue->val.x.buf)
						       <= q->attrDefaultValue->val.x.size);
						size = sizeof(SaStringT);
						break;

					case SA_IMM_ATTR_SANAMET:
						assert(q->attrDefaultValue->val.x.size <= SA_MAX_NAME_LENGTH);
						size = sizeof(SaNameT);
						break;

					case SA_IMM_ATTR_SAANYT:
						size = sizeof(SaAnyT);
						break;
					default:
						assert(0);
					}	/*switch */

					SaNameT *namep;
					SaAnyT *anyp;
					SaStringT *strp;
					SaImmAttrValueT copyv;
					copyv = (SaImmAttrValueT)	/*alloc-4 */
					    malloc(size);
					switch (q->attrValueType) {
					case SA_IMM_ATTR_SAINT32T:
						*((SaInt32T *)copyv) = q->attrDefaultValue->val.saint32;
						break;
					case SA_IMM_ATTR_SAUINT32T:
						*((SaUint32T *)copyv) = q->attrDefaultValue->val.sauint32;
						break;
					case SA_IMM_ATTR_SAINT64T:
						*((SaInt64T *)copyv) = q->attrDefaultValue->val.saint64;
						break;
					case SA_IMM_ATTR_SAUINT64T:
						*((SaUint64T *)copyv) = q->attrDefaultValue->val.sauint64;
						break;
					case SA_IMM_ATTR_SATIMET:
						/*NOTE ABT PROBLEM WITH ASSIGNEMENT. Allignement ?? */
						TRACE_1("WARNING SaTimeT default broken");
						/* *((SaTimeT *)copyv) = q->attrDefaultValue->val.satime; */
						break;
					case SA_IMM_ATTR_SAFLOATT:
						*((SaFloatT *)copyv) = q->attrDefaultValue->val.safloat;
						break;
					case SA_IMM_ATTR_SADOUBLET:
						*((SaDoubleT *)copyv) = q->attrDefaultValue->val.sadouble;
						break;

					case SA_IMM_ATTR_SASTRINGT:
						strp = (SaStringT *)copyv;
						*strp =	/*alloc-5 */
						    malloc(q->attrDefaultValue->val.x.size);
						memcpy(*strp, q->attrDefaultValue->val.x.buf,
						       q->attrDefaultValue->val.x.size);
						break;

					case SA_IMM_ATTR_SANAMET:
						namep = (SaNameT *)copyv;
						memset(namep, 0, sizeof(SaNameT));
						namep->length = strnlen(q->attrDefaultValue->val.x.buf,
									q->attrDefaultValue->val.x.size);
						assert(namep->length <= SA_MAX_NAME_LENGTH);
						memcpy(namep->value, q->attrDefaultValue->val.x.buf, namep->length);
						break;

					case SA_IMM_ATTR_SAANYT:
						anyp = (SaAnyT *)copyv;
						memset(namep, 0, sizeof(SaAnyT));
						anyp->bufferSize = q->attrDefaultValue->val.x.size;
						anyp->bufferAddr = (SaUint8T *)
						    malloc(anyp->bufferSize);	/*alloc-5 */
						memcpy(anyp->bufferAddr, q->attrDefaultValue->val.x.buf,
						       anyp->bufferSize);
						break;
					default:
						assert(0);
					}	/*switch */

					attr[i]->attrDefaultValue = copyv;
					/*Delete source attr-value */
					immsv_evt_free_att_val(q->attrDefaultValue, q->attrValueType);

					free(q->attrDefaultValue);
					q->attrDefaultValue = NULL;
				}
				p = p->next;
				prev->next = NULL;
				free(prev);
			}
			attr[noOfAttributes] = 0;

			*attrDefinition = attr;
			/*Will return a 0 terminated array of pointers to defs */

		}		/*if(out_evtt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR){}else{ */

		/*For better performance we should follow the pattern of searchNext
		   and avoid copying the attribute values and keep the reply struct
		   and free it in ClassDescriptionMemoryFree.
		   But ClassDescriptionGet is not expected to be invoked often.
		 */

		/* We dont need the lock here */

		free(out_evt);
	}
	/*if(out_evt) */
 mds_send_fail:
	if (evt.info.immnd.info.classDescr.className.buf) {	/*free-0 */
		free(evt.info.immnd.info.classDescr.className.buf);
		evt.info.immnd.info.classDescr.className.buf = NULL;
	}
 client_not_found:
 stale_handle:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	TRACE_LEAVE();
	return rc;
}

#ifdef IMM_A_01_01
SaAisErrorT saImmOmClassDescriptionMemoryFree(SaImmHandleT immHandle, SaImmAttrDefinitionT ** attrDefinition)
{
	IMMA_CB *cb = &imma_cb;
	IMMA_CLIENT_NODE *cl_node = NULL;
	TRACE_ENTER();
	/* ABT The code from here.. */

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		TRACE_LEAVE();
		return SA_AIS_ERR_LIBRARY;
	}

	imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		TRACE_1("Client not found");
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
		TRACE_LEAVE();
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		TRACE_LEAVE();
		return SA_AIS_ERR_BAD_HANDLE;
	}

	/* ABT ...to here does not do anything except check handle.
	   Perhaps we should just ignore the handle. */

	if (attrDefinition) {
		int i;
		for (i = 0; attrDefinition[i]; ++i) {
			if (attrDefinition[i]->attrDefaultValue) {
				SaStringT *strp;
				SaAnyT *anyp;
				switch (attrDefinition[i]->attrValueType) {
				case SA_IMM_ATTR_SAINT32T:	//intended fall through
				case SA_IMM_ATTR_SAUINT32T:	//intended fall through
				case SA_IMM_ATTR_SAINT64T:	//intended fall through
				case SA_IMM_ATTR_SAUINT64T:	//intended fall through
				case SA_IMM_ATTR_SATIMET:	//intended fall through
				case SA_IMM_ATTR_SAFLOATT:	//intended fall through
				case SA_IMM_ATTR_SADOUBLET:
				case SA_IMM_ATTR_SANAMET:
					free(attrDefinition[i]->attrDefaultValue);	/*free-4 */
					break;

				case SA_IMM_ATTR_SASTRINGT:
					strp = (SaStringT *)attrDefinition[i]->attrDefaultValue;
					free(*strp);	/*free-5 */
					free(strp);	/*free-4 */
					break;

				case SA_IMM_ATTR_SAANYT:
					anyp = (SaAnyT *)attrDefinition[i]->attrDefaultValue;
					free(anyp->bufferAddr);	/*free-5 */
					anyp->bufferAddr = 0;
					free(anyp);	/*free-4 */
					break;

				default:
					assert(0);
				}	//switch
				attrDefinition[i]->attrDefaultValue = 0;
			}
			free(attrDefinition[i]->attrName);	/*free-3 */
			attrDefinition[i]->attrName = 0;
			free(attrDefinition[i]);	/*free-2 */
			attrDefinition[i] = 0;
		}
		free(attrDefinition);	/*free-1 */
	}
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	TRACE_LEAVE();
	return SA_AIS_OK;
}
#endif

SaAisErrorT saImmOmClassDescriptionMemoryFree_2(SaImmHandleT immHandle, SaImmAttrDefinitionT_2 **attrDefinition)
{
	IMMA_CB *cb = &imma_cb;
	IMMA_CLIENT_NODE *cl_node = NULL;
	TRACE_ENTER();
	/* ABT The code from here.. */

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		TRACE_LEAVE();
		return SA_AIS_ERR_LIBRARY;
	}

	imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		TRACE_1("Client not found");
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
		TRACE_LEAVE();
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		TRACE_LEAVE();
		return SA_AIS_ERR_BAD_HANDLE;
	}

	/* ABT ...to here does not do anything except check handle.
	   Perhaps we should just ignore the handle. */

	if (attrDefinition) {
		int i;
		for (i = 0; attrDefinition[i]; ++i) {
			if (attrDefinition[i]->attrDefaultValue) {
				SaStringT *strp;
				SaAnyT *anyp;
				switch (attrDefinition[i]->attrValueType) {
				case SA_IMM_ATTR_SAINT32T:	//intended fall through
				case SA_IMM_ATTR_SAUINT32T:	//intended fall through
				case SA_IMM_ATTR_SAINT64T:	//intended fall through
				case SA_IMM_ATTR_SAUINT64T:	//intended fall through
				case SA_IMM_ATTR_SATIMET:	//intended fall through
				case SA_IMM_ATTR_SAFLOATT:	//intended fall through
				case SA_IMM_ATTR_SADOUBLET:
				case SA_IMM_ATTR_SANAMET:
					free(attrDefinition[i]->attrDefaultValue);	/*free-4 */
					break;

				case SA_IMM_ATTR_SASTRINGT:
					strp = (SaStringT *)attrDefinition[i]->attrDefaultValue;
					free(*strp);	/*free-5 */
					free(strp);	/*free-4 */
					break;

				case SA_IMM_ATTR_SAANYT:
					anyp = (SaAnyT *)attrDefinition[i]->attrDefaultValue;
					free(anyp->bufferAddr);	/*free-5 */
					anyp->bufferAddr = 0;
					free(anyp);	/*free-4 */
					break;

				default:
					assert(0);
				}	//switch
				attrDefinition[i]->attrDefaultValue = 0;
			}
			free(attrDefinition[i]->attrName);	/*free-3 */
			attrDefinition[i]->attrName = 0;
			free(attrDefinition[i]);	/*free-2 */
			attrDefinition[i] = 0;
		}
		free(attrDefinition);	/*free-1 */
	}

	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	TRACE_LEAVE();
	return SA_AIS_OK;
}

SaAisErrorT saImmOmClassDelete(SaImmHandleT immHandle, const SaImmClassNameT className)
{
	SaAisErrorT rc = SA_AIS_OK;
	NCS_BOOL locked = TRUE;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	SaTimeT timeout = IMMSV_WAIT_TIME;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (className == NULL) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	/*locked is true already */

	/* Get the Client info */
	rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* Populate the ClassDelete event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_CLASS_DELETE;

	evt.info.immnd.info.classDescr.className.size = strlen(className) + 1;
	evt.info.immnd.info.classDescr.className.buf = malloc(evt.info.immnd.info.classDescr.className.size);	/*alloc-1 */
	strncpy(evt.info.immnd.info.classDescr.className.buf, className,
		(size_t)evt.info.immnd.info.classDescr.className.size);

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, timeout, cl_node->handle, &locked, TRUE);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;
		TRACE("Return code:%u", rc);

		/* We dont need the lock here */

		free(out_evt);
	}

 mds_send_fail:
	free(evt.info.immnd.info.classDescr.className.buf);
	evt.info.immnd.info.classDescr.className.buf = NULL;
	evt.info.immnd.info.classDescr.className.size = 0;
 stale_handle:
 client_not_found:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

SaAisErrorT saImmOmAccessorInitialize(SaImmHandleT immHandle, SaImmAccessorHandleT *accessorHandle)
{

	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = TRUE;
	IMMA_CB *cb = &imma_cb;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_SEARCH_NODE *search_node = NULL;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (accessorHandle == NULL) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto release_cb;
	}
	/*locked is true already */

	proc_rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	/*Create search-node & handle */
	search_node = (IMMA_SEARCH_NODE *)calloc(1, sizeof(IMMA_SEARCH_NODE));
	if (!search_node) {
		rc = SA_AIS_ERR_NO_MEMORY;
		goto release_lock;
	}

	search_node->search_hdl = (SaImmSearchHandleT)m_NCS_GET_TIME_NS;
	search_node->mImmHandle = immHandle;
	/*This is the external handle that the application uses.
	   Internally we use the searchId provided by the Node Director. */

	/* Add IMMA_SEARCH_NODE to search_tree */
	proc_rc = imma_search_node_add(&cb->search_tree, search_node);

	if (proc_rc != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Failed to add search node to search tree");
	} else {
		*accessorHandle = search_node->search_hdl;
	}

 release_lock:
	if (locked) {
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	}

 release_cb:

	return rc;
}

SaAisErrorT saImmOmAccessorFinalize(SaImmAccessorHandleT accessorHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	NCS_BOOL locked = TRUE;
	IMMA_CB *cb = &imma_cb;
	IMMA_SEARCH_NODE *search_node = NULL;
	uns32 proc_rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock error");
		locked = FALSE;
		goto release_cb;
	}

	imma_search_node_get(&cb->search_tree, &accessorHandle, &search_node);

	if (!search_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	if (search_node->mSearchId) {
		/*Finalize as a searchHandle */
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
		locked = FALSE;
		/*NOTE: We return directly in the next statement. */
		return saImmOmSearchFinalize((SaImmSearchHandleT)accessorHandle);

	} else {
		/*Search handle was apparently never used, just remove the node. */
		proc_rc = imma_search_node_delete(cb, search_node);
		search_node = NULL;
		if (proc_rc != NCSCC_RC_SUCCESS) {
			TRACE_1("Could not delete search node");
			rc = SA_AIS_ERR_LIBRARY;
		}
	}

 release_lock:
	if (locked) {
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	}

 release_cb:
	TRACE_LEAVE();
	return rc;
}

#ifdef IMM_A_01_01
SaAisErrorT saImmOmAccessorGet(SaImmAccessorHandleT accessorHandle,
			       const SaNameT *objectName,
			       const SaImmAttrNameT *attributeNames, SaImmAttrValuesT *** attributes)
{
	return saImmOmAccessorGet_2(accessorHandle, objectName, attributeNames, (SaImmAttrValuesT_2 ***)attributes);
}
#endif

SaAisErrorT saImmOmAccessorGet_2(SaImmAccessorHandleT accessorHandle,
				 const SaNameT *objectName,
				 const SaImmAttrNameT *attributeNames, SaImmAttrValuesT_2 ***attributes)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = TRUE;
	IMMA_CB *cb = &imma_cb;
	IMMA_SEARCH_NODE *search_node = NULL;
	SaNameT redundantName;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((objectName == NULL) || (objectName->length == 0) || (objectName->length >= SA_MAX_NAME_LENGTH)) {
		TRACE_2("Incorrect parameter contents: objectName");
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (!attributes) {
		TRACE_2("attributes is NULL");
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock error");
		goto release_cb;
	}
	/*locked is true already */

	proc_rc = imma_search_node_get(&cb->search_tree, &accessorHandle, &search_node);

	if (!search_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	SaImmHandleT immHandle = search_node->mImmHandle;

	/* Stale check etc done in searchInit & searchNext below. */

	/* Release locks and cb for normal cases as it uses SearchInit & 
	   SearchNext.
	 */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/*The accessorHandle is actually just a searchhandle in this 
	   implementation. 
	   SaImmSearchHandleT searchHandle = (SaImmSearchHandleT) accessorHandle;
	 */

	if (search_node->mSearchId) {
		TRACE("Closing previous allocation :%u\n", search_node->mSearchId);

		/*Perform a search-next, simply to discard any previous allocation. */
		/*Cant do a searchFinalize since that will discard the node. */

		redundantName.length = 0;
		m_IMMSV_SET_SANAMET((&redundantName));

		rc = saImmOmSearchNext_2(accessorHandle, &redundantName, attributes);

		if (rc != SA_AIS_ERR_NOT_EXIST && rc != SA_AIS_ERR_BAD_HANDLE) {
			TRACE_1("Unexpected return code from internal searchNext: %u", rc);
			return SA_AIS_ERR_LIBRARY;	/*No handles to close here. */
		}

		/* Close the previous search id.
		   Cant use saImmOmSearchfinalize, because this will also close the 
		   search handle and deallocate the search_node.
		 */

		/* NOTE/TODO: Should Send a ImmOmSearchFinalize message to IMMND !! 
		   This to discard the search on the server side. See searchFinalize. */
		search_node->mSearchId = 0;
	}

	rc = saImmOmSearchInitialize_2(immHandle, objectName,	/*root == object */
				       SA_IMM_ONE,	/* Normally illegal in search */
				       0, NULL, attributeNames, &accessorHandle);

	if (rc != SA_AIS_OK) {
		search_node->mSearchId = 0;
		return rc;
	}

	redundantName.length = 0;
	m_IMMSV_SET_SANAMET((&redundantName));
	rc = saImmOmSearchNext_2(accessorHandle, &redundantName, attributes);
	return rc;

	/*error cases only */
 release_lock:
	if (locked) {
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	}

 release_cb:

	return rc;
}

SaAisErrorT immsv_sync(SaImmHandleT immHandle,
		       const SaImmClassNameT className,
		       const SaNameT *objectName, const SaImmAttrValuesT_2 **attrValues)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	NCS_BOOL locked = FALSE;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if ((className == NULL) || (objectName == NULL) || (attrValues == NULL)) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	locked = TRUE;

	proc_rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* Populate the Object-Sync event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_OBJ_SYNC;

	evt.info.immnd.info.obj_sync.className.size = strlen(className) + 1;

	/*alloc-1 */
	evt.info.immnd.info.obj_sync.className.buf = malloc(evt.info.immnd.info.obj_sync.className.size);
	strncpy(evt.info.immnd.info.obj_sync.className.buf, className, evt.info.immnd.info.obj_sync.className.size);

	if (objectName->length) {
		assert(objectName->length < SA_MAX_NAME_LENGTH);
		evt.info.immnd.info.obj_sync.objectName.size = strlen((char *)objectName->value) + 1;

		if (objectName->length + 1 < evt.info.immnd.info.obj_sync.objectName.size) {
			evt.info.immnd.info.obj_sync.objectName.size = objectName->length + 1;
		}

		/*alloc-2 */
		evt.info.immnd.info.obj_sync.objectName.buf = malloc(evt.info.immnd.info.obj_sync.objectName.size);
		strncpy(evt.info.immnd.info.obj_sync.objectName.buf,
			(char *)objectName->value, evt.info.immnd.info.obj_sync.objectName.size);
		evt.info.immnd.info.obj_sync.objectName.buf[evt.info.immnd.info.obj_sync.objectName.size - 1] = '\0';
	} else {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto error_exit_1;
	}

	assert(evt.info.immnd.info.obj_sync.attrValues == NULL);
	const SaImmAttrValuesT_2 *attr;
	int i;
	for (i = 0; attrValues[i]; ++i) {
		attr = attrValues[i];

		if (attr->attrValuesNumber == 0) {
			TRACE("Attribute without values DN:%s ATT:%s, skipped in sync",
			      objectName->value, attr->attrName);
			continue;
		}

		/*alloc-3 */
		IMMSV_ATTR_VALUES_LIST *p = calloc(1, sizeof(IMMSV_ATTR_VALUES_LIST));

		p->n.attrName.size = strlen(attr->attrName) + 1;
		if (p->n.attrName.size >= SA_MAX_NAME_LENGTH) {
			TRACE_1("Attribute name too long: %u", p->n.attrName.size);
			rc = SA_AIS_ERR_INVALID_PARAM;
			free(p);
			goto mds_send_fail;
		}

		/*alloc-4 */
		p->n.attrName.buf = malloc(p->n.attrName.size);
		strncpy(p->n.attrName.buf, attr->attrName, p->n.attrName.size);

		p->n.attrValuesNumber = attr->attrValuesNumber;
		p->n.attrValueType = attr->attrValueType;

		const SaImmAttrValueT *avarr = attr->attrValues;
		/*alloc-5 */
		imma_copyAttrValue(&(p->n.attrValue), attr->attrValueType, avarr[0]);

		if (attr->attrValuesNumber > 1) {
			unsigned int numAdded = attr->attrValuesNumber - 1;
			unsigned int i;
			for (i = 1; i <= numAdded; ++i) {
				/*alloc-6 */
				IMMSV_EDU_ATTR_VAL_LIST *al = calloc(1, sizeof(IMMSV_EDU_ATTR_VAL_LIST));

				/*alloc-7 */
				imma_copyAttrValue(&(al->n), attr->attrValueType, avarr[i]);
				al->next = p->n.attrMoreValues;
				p->n.attrMoreValues = al;
			}
		}

		p->next = evt.info.immnd.info.obj_sync.attrValues;	/* NULL initially */
		evt.info.immnd.info.obj_sync.attrValues = p;
	}

	rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, FALSE);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is after a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here */

		free(out_evt);
	}

 error_exit_1:
 mds_send_fail:
	/*We may be un-locked here but this should not matter.
	   We are freing heap objects that should only be vissible from this
	   thread. */

	if (evt.info.immnd.info.obj_sync.className.buf) {	/*free-1 */
		free(evt.info.immnd.info.obj_sync.className.buf);
		evt.info.immnd.info.obj_sync.className.buf = NULL;
	}

	if (evt.info.immnd.info.obj_sync.objectName.buf) {	/*free-2 */
		free(evt.info.immnd.info.obj_sync.objectName.buf);
		evt.info.immnd.info.obj_sync.objectName.buf = NULL;
	}

	while (evt.info.immnd.info.obj_sync.attrValues) {
		IMMSV_ATTR_VALUES_LIST *p = evt.info.immnd.info.obj_sync.attrValues;
		evt.info.immnd.info.obj_sync.attrValues = p->next;
		p->next = NULL;
		if (p->n.attrName.buf) {	/*free-4 */
			free(p->n.attrName.buf);
			p->n.attrName.buf = NULL;
		}

		immsv_evt_free_att_val(&(p->n.attrValue), p->n.attrValueType);	/*free-5 */

		while (p->n.attrMoreValues) {
			IMMSV_EDU_ATTR_VAL_LIST *al = p->n.attrMoreValues;
			p->n.attrMoreValues = al->next;
			al->next = NULL;
			immsv_evt_free_att_val(&(al->n), p->n.attrValueType);	/*free-7 */

			free(al);	/*free-6 */
		}
		p->next = NULL;
		free(p);	/*free-3 */
	}

 stale_handle:
 client_not_found:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

SaAisErrorT immsv_finalize_sync(SaImmHandleT immHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT finalize_evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = 0;
	NCS_BOOL locked = TRUE;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);

	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Missing client node");
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* populate the structure */
	memset(&finalize_evt, 0, sizeof(IMMSV_EVT));
	finalize_evt.type = IMMSV_EVT_TYPE_IMMND;
	finalize_evt.info.immnd.type = IMMND_EVT_A2ND_SYNC_FINALIZE;
	finalize_evt.info.immnd.info.finReq.client_hdl = cl_node->handle;

	/* Unlock before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/* IMMND GOES DOWN */
	if (cb->is_immnd_up == FALSE) {
		rc = SA_AIS_ERR_TRY_AGAIN;
		TRACE_2("IMMND is DOWN");
		goto mds_send_fail;
	}

	/* send the request to the IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl,
					 &(cb->immnd_mds_dest), &finalize_evt, &out_evt, IMMSV_WAIT_TIME);

	/* MDS error handling */
	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;
	case NCSCC_RC_REQ_TIMOUT:
		rc = SA_AIS_ERR_TIMEOUT;
		goto mds_send_fail;
	default:
		TRACE_1("MDS returned unexpected error code %u", proc_rc);
		rc = SA_AIS_ERR_NO_RESOURCES;
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is a blocking op. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;
		if (rc != SA_AIS_OK) {
			TRACE_1("Returned error: %u", rc);
		}

		free(out_evt);
	}

 mds_send_fail:
 stale_handle:
 client_not_found:

	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:
	/* Release the CB handle */
	TRACE_LEAVE();
	return rc;
}

#ifdef IMM_A_01_01
SaAisErrorT saImmOmSearchInitialize(SaImmHandleT immHandle,
				    const SaNameT *rootName,
				    SaImmScopeT scope,
				    SaImmSearchOptionsT searchOptions,
				    const SaImmSearchParametersT * searchParam,
				    const SaImmAttrNameT *attributeNames, SaImmSearchHandleT *searchHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaImmSearchParametersT_2 searchParam2;
	TRACE_ENTER();

	if (searchParam) {
		searchParam2.searchOneAttr.attrName = searchParam->searchOneAttr.attrName;
		searchParam2.searchOneAttr.attrValueType = searchParam->searchOneAttr.attrValueType;
		searchParam2.searchOneAttr.attrValue = searchParam->searchOneAttr.attrValue;

		rc = saImmOmSearchInitialize_2(immHandle, rootName, scope,
					       searchOptions, &searchParam2, attributeNames, searchHandle);

	} else {
		rc = saImmOmSearchInitialize_2(immHandle, rootName, scope,
					       searchOptions, NULL, attributeNames, searchHandle);
	}
	TRACE_LEAVE();
	return rc;
}
#endif

SaAisErrorT saImmOmSearchInitialize_2(SaImmHandleT immHandle,
				      const SaNameT *rootName,
				      SaImmScopeT scope,
				      SaImmSearchOptionsT searchOptions,
				      const SaImmSearchParametersT_2 *searchParam,
				      const SaImmAttrNameT *attributeNames, SaImmSearchHandleT *searchHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = TRUE;
	NCS_BOOL isAccessor = FALSE;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_SEARCH_NODE *search_node = NULL;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (searchHandle == NULL) {
		TRACE_2("Invalid search handle");
		TRACE_LEAVE();
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto release_cb;
	}
	/*locked is true already */

	proc_rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	if ((scope == SA_IMM_ONE) && (*searchHandle) && !searchParam && !searchOptions) {	/*Special ACCESSOR case */
		TRACE("Special accessor case:%llu\n", *searchHandle);
		isAccessor = TRUE;
		searchOptions = SA_IMM_SEARCH_ONE_ATTR |
		    (attributeNames ? SA_IMM_SEARCH_GET_SOME_ATTR : SA_IMM_SEARCH_GET_ALL_ATTR);

		/*Look up search handle */
		proc_rc = imma_search_node_get(&cb->search_tree, searchHandle, &search_node);
		if ((!search_node) || (search_node->mImmHandle != immHandle)) {
			rc = SA_AIS_ERR_BAD_HANDLE;
			goto release_lock;
		}
	} else {		/*Normal case */
		*searchHandle = 0LL;

		if ((scope != SA_IMM_SUBLEVEL) && (scope != SA_IMM_SUBTREE)) {
			TRACE_2("Invalid scope parameter");
			rc = SA_AIS_ERR_INVALID_PARAM;
			goto release_lock;
		}

		/*Create search-node & handle   */
		search_node = (IMMA_SEARCH_NODE *)
		    calloc(1, sizeof(IMMA_SEARCH_NODE));
		if (!search_node) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto release_lock;
		}

		search_node->search_hdl = (SaImmSearchHandleT)m_NCS_GET_TIME_NS;
		search_node->mImmHandle = immHandle;

		/* Add IMMA_SEARCH_NODE to search_tree */
		proc_rc = imma_search_node_add(&cb->search_tree, search_node);

		if (proc_rc != NCSCC_RC_SUCCESS) {
			rc = SA_AIS_ERR_LIBRARY;
			TRACE_1("Failed to add search node to search tree");
			goto search_node_add_fail;
		}
	}

	/* Populate the SearchInit event */
	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_SEARCHINIT;
	IMMSV_OM_SEARCH_INIT *req = &(evt.info.immnd.info.searchInit);
	req->client_hdl = immHandle;
	if (rootName && rootName->length && (rootName->length < SA_MAX_NAME_LENGTH)) {
		req->rootName.size = strlen((char *)rootName->value) + 1;
		if (rootName->length + 1 < req->rootName.size)
			req->rootName.size = rootName->length + 1;
		req->rootName.buf = malloc(req->rootName.size);	/* alloc-1 */
		strncpy(req->rootName.buf, (char *)rootName->value, (size_t)req->rootName.size);
		req->rootName.buf[req->rootName.size - 1] = 0;
	} else {
		req->rootName.size = 0;
		req->rootName.buf = NULL;
	}

	req->scope = scope;
	req->searchOptions = searchOptions;
	if (!searchParam || (!searchParam->searchOneAttr.attrName)) {
		req->searchParam.present = ImmOmSearchParameter_PR_NOTHING;
	} else {
		req->searchParam.present = ImmOmSearchParameter_PR_oneAttrParam;
		req->searchParam.choice.oneAttrParam.attrName.size = strlen(searchParam->searchOneAttr.attrName) + 1;
		req->searchParam.choice.oneAttrParam.attrName.buf =	/*alloc-2 */
		    malloc(req->searchParam.choice.oneAttrParam.attrName.size);
		strncpy(req->searchParam.choice.oneAttrParam.attrName.buf,
			(char *)searchParam->searchOneAttr.attrName,
			(size_t)req->searchParam.choice.oneAttrParam.attrName.size);
		req->searchParam.choice.oneAttrParam.attrName.buf[req->searchParam.choice.oneAttrParam.attrName.size -
								  1] = 0;
		if (searchParam->searchOneAttr.attrValue) {
			req->searchParam.choice.oneAttrParam.attrValueType = searchParam->searchOneAttr.attrValueType;
			/*alloc-3 */
			imma_copyAttrValue(&req->searchParam.choice.oneAttrParam.attrValue,
					   searchParam->searchOneAttr.attrValueType,
					   searchParam->searchOneAttr.attrValue);
		} else {
			/* Encoding NO VALUE as a string of zero length. */
			req->searchParam.choice.oneAttrParam.attrValueType = SA_IMM_ATTR_SASTRINGT;
			req->searchParam.choice.oneAttrParam.attrValue.val.x.size = 0;
			req->searchParam.choice.oneAttrParam.attrValue.val.x.buf = NULL;
		}
	}

	if (attributeNames) {
		const SaImmAttrNameT *namev;
		for (namev = attributeNames; *namev; namev++) {
			IMMSV_ATTR_NAME_LIST *p = (IMMSV_ATTR_NAME_LIST *)
			    malloc(sizeof(IMMSV_ATTR_NAME_LIST));	/*alloc-4 */
			p->name.size = strlen(*namev) + 1;
			p->name.buf = malloc(p->name.size);	/*alloc-5 */
			strncpy(p->name.buf, *namev, p->name.size);
			p->name.buf[p->name.size - 1] = 0;

			p->next = req->attributeNames;	/*NULL in first iteration */
			req->attributeNames = p;
		}
	}

	if (rootName && rootName->length) {
		TRACE("root: %s param:%p", rootName->value, searchParam);
	}

	/* Release the CB lock Before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/* IMMND GOES DOWN */
	if (cb->is_immnd_up == FALSE) {
		rc = SA_AIS_ERR_TRY_AGAIN;
		TRACE_2("IMMND is DOWN");
		goto mds_send_fail;
	}

	/* send the request to the IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, &evt, &out_evt, IMMSV_WAIT_TIME);

	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;

	case NCSCC_RC_REQ_TIMOUT:
		rc = SA_AIS_ERR_TIMEOUT;
		goto mds_send_fail;

	default:
		rc = SA_AIS_ERR_NO_RESOURCES;
		TRACE_1("MDS returned unexpected error code %u", proc_rc);
		goto mds_send_fail;
	}

	if (out_evt) {
		/*search_node->mLastResult = 0;  -- already zeroed */
		/*search_node->mLastAttributes = 0; -- already zeroed */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_SEARCHINIT_RSP);
		rc = out_evt->info.imma.info.searchInitRsp.error;

		/* Take the CB lock */

		if (!locked) {
			if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
				rc = SA_AIS_ERR_LIBRARY;
				TRACE_1("Lock error");
				return rc;	/* Error case will leak memory. */
			}
			locked = TRUE;
		}

		if (rc == SA_AIS_OK) {
			search_node->mSearchId = out_evt->info.imma.info.searchInitRsp.searchId;
			if (*searchHandle) {
				assert(*searchHandle == search_node->search_hdl);
			} else {
				*searchHandle = search_node->search_hdl;
			}
		} else {
			TRACE_1("Received error code from server:%u", rc);
			if (!isAccessor) {
				proc_rc = imma_search_node_delete(cb, search_node);
				search_node = NULL;	/*Node was removed from tree AND freed. */
				if (proc_rc != NCSCC_RC_SUCCESS) {
					TRACE_1("Could not delete search node");
					rc = SA_AIS_ERR_LIBRARY;
				}
			}
		}
		free(out_evt);
	}

 mds_send_fail:
	assert(req);
	if (req->rootName.buf) {
		free(req->rootName.buf);	/*free-1 */
		req->rootName.buf = NULL;
		req->rootName.size = 0;
	}
	if (req->searchParam.present == ImmOmSearchParameter_PR_oneAttrParam) {
		free(req->searchParam.choice.oneAttrParam.attrName.buf);	/*free-2 */
		req->searchParam.choice.oneAttrParam.attrName.buf = NULL;
		req->searchParam.choice.oneAttrParam.attrName.size = 0;
		if (searchParam->searchOneAttr.attrValue) {	/*free-3 */
			immsv_evt_free_att_val(&(req->searchParam.choice.oneAttrParam.attrValue),
					       searchParam->searchOneAttr.attrValueType);
		}
	}

	if (req->attributeNames) {	/* free-4 & free-5 */
		immsv_evt_free_attrNames(req->attributeNames);
		req->attributeNames = NULL;
	}

 search_node_add_fail:
	if (rc != SA_AIS_OK && search_node != NULL && !isAccessor) {
		/*Node never added to tree */
		free(search_node);
	}

 release_lock:
	if (locked) {
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	}

 release_cb:
	TRACE_LEAVE();
	return rc;
}

#ifdef IMM_A_01_01
SaAisErrorT saImmOmSearchNext(SaImmSearchHandleT searchHandle, SaNameT *objectName, SaImmAttrValuesT *** attributes)
{
	SaAisErrorT error = SA_AIS_OK;

	error = saImmOmSearchNext_2(searchHandle, objectName, (SaImmAttrValuesT_2 ***)attributes);

	return error;
}
#endif

SaAisErrorT saImmOmSearchNext_2(SaImmSearchHandleT searchHandle, SaNameT *objectName, SaImmAttrValuesT_2 ***attributes)
{
	SaAisErrorT error = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = TRUE;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_SEARCH_NODE *search_node = NULL;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (!objectName) {
		TRACE_2("Invalid parameter: objectName");
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (!attributes) {
		TRACE_2("Invalid parameter: attributes");
		return SA_AIS_ERR_INVALID_PARAM;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		error = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		locked = FALSE;
		goto release_cb;
	}
	/*locked is true already */

	/*Look up search node */
	proc_rc = imma_search_node_get(&cb->search_tree, &searchHandle, &search_node);

	if (!search_node) {
		error = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	proc_rc = imma_client_node_get(&cb->client_tree, &search_node->mImmHandle, &cl_node);

	if (!(cl_node && cl_node->isOm)) {
		error = SA_AIS_ERR_LIBRARY;
		LOG_ER("Invalid SaImmHandleT related to search handle");
		goto release_lock;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", search_node->mImmHandle);
		error = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	if (search_node->mLastAttributes) {
		TRACE("Freeing previous result");
		imma_freeSearchAttrs((SaImmAttrValuesT_2 **)search_node->mLastAttributes);
		search_node->mLastAttributes = 0;
	}

	if (search_node->mSearchId == 0) {
		error = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_SEARCHNEXT;
	evt.info.immnd.info.searchOp.searchId = search_node->mSearchId;
	evt.info.immnd.info.searchOp.client_hdl = search_node->mImmHandle;

	/* Release the CB lock Before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/* IMMND GOES DOWN */
	if (cb->is_immnd_up == FALSE) {
		error = SA_AIS_ERR_TRY_AGAIN;
		TRACE_1("IMMND is DOWN");
		goto release_cb;
	}

	/* send the request to the IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, &evt, &out_evt, IMMSV_WAIT_TIME);

	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;

	case NCSCC_RC_REQ_TIMOUT:
		error = SA_AIS_ERR_TIMEOUT;
		goto release_cb;

	default:
		error = SA_AIS_ERR_NO_RESOURCES;
		TRACE_1("MDS returned unexpected error code %u", proc_rc);
		goto release_cb;
	}

	if (out_evt) {
		int noOfAttributes = 0;
		int i = 0;
		size_t attrDataSize = 0;
		IMMSV_OM_RSP_SEARCH_NEXT *res_body = NULL;
		SaImmAttrValuesT_2 **attr = NULL;

		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		if (out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR) {
			error = out_evt->info.imma.info.errRsp.error;
			assert(error && (error != SA_AIS_OK));
			free(out_evt);	/*BUGFIX (leak) 090506 */
			out_evt = NULL;
			goto release_cb;
		}
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_SEARCHNEXT_RSP);

		/* Take the CB lock */

		if (!locked) {
			if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
				error = SA_AIS_ERR_LIBRARY;
				TRACE_1("Lock error");
				return error;	/*Error case will leak memory. */
			}
			locked = TRUE;
		}

		res_body = out_evt->info.imma.info.searchNextRsp;
		assert(res_body);

		objectName->length = 0;
		m_IMMSV_SET_SANAMET(objectName);
		objectName->length = strnlen(res_body->objectName.buf, res_body->objectName.size);
		assert(objectName->length <= SA_MAX_NAME_LENGTH);
		memcpy(objectName->value, res_body->objectName.buf, objectName->length);

		IMMSV_ATTR_VALUES_LIST *p = res_body->attrValuesList;
		while (p) {
			++noOfAttributes;
			p = p->next;
		}

		attrDataSize = sizeof(SaImmAttrValuesT_2 *) * (noOfAttributes + 1);
		attr = calloc(1, attrDataSize);	/*alloc-1 */
		p = res_body->attrValuesList;
		for (; i < noOfAttributes; i++, p = p->next) {
			IMMSV_ATTR_VALUES *q = &(p->n);
			attr[i] = calloc(1, sizeof(SaImmAttrValuesT_2));	/*alloc-2 */
			attr[i]->attrName = malloc(q->attrName.size + 1);	/*alloc-3 */
			strncpy(attr[i]->attrName, (const char *)q->attrName.buf, q->attrName.size + 1);
			attr[i]->attrName[q->attrName.size] = 0;	/*redundant. */
			attr[i]->attrValuesNumber = q->attrValuesNumber;
			attr[i]->attrValueType = (SaImmValueTypeT)q->attrValueType;

			if (q->attrValuesNumber) {
				/*SaNameT* namep;
				   SaAnyT*  anyp;
				   SaStringT* strp;
				   Allocate the array of pointers, even if only one value
				 */
				attr[i]->attrValues = calloc(1, q->attrValuesNumber * sizeof(SaImmAttrValueT));	/*alloc-4 */
				/*alloc-5 */
				attr[i]->attrValues[0] = imma_copyAttrValue3(q->attrValueType, &(q->attrValue));

				if (q->attrValuesNumber > 1) {
					int ix;
					IMMSV_EDU_ATTR_VAL_LIST *r = q->attrMoreValues;
					for (ix = 1; ix < q->attrValuesNumber; ++ix) {
						assert(r);
						attr[i]->attrValues[ix] = imma_copyAttrValue3(q->attrValueType, &(r->n));	/*alloc-5 */
						r = r->next;
					}
				}
			}
		}
		attr[noOfAttributes] = NULL;
		*attributes = attr;
		search_node->mLastAttributes = attr;	/* TODO: Should really look up searh_node again. */
	}

	/* ABT We must be carefull only to deallocate the client_node & search_node,
	   to protect the search_node->mLastAttributes structure from being removed
	   while the user is still using it (lock released).
	 */

	if (out_evt) {
		/* Free the search_next structure. */
		if (out_evt->info.imma.info.searchNextRsp) {
			free(out_evt->info.imma.info.searchNextRsp->objectName.buf);
			out_evt->info.imma.info.searchNextRsp->objectName.buf = NULL;
			out_evt->info.imma.info.searchNextRsp->objectName.size = 0;
			immsv_free_attrvalues_list(out_evt->info.imma.info.searchNextRsp->attrValuesList);
			out_evt->info.imma.info.searchNextRsp->attrValuesList = NULL;
			free(out_evt->info.imma.info.searchNextRsp);
			out_evt->info.imma.info.searchNextRsp = NULL;
		}
		free(out_evt);
	}

 release_cb:
 release_lock:
	if (locked) {
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	}

	return error;
}

void imma_freeSearchAttrs(SaImmAttrValuesT_2 **attr)
{
	SaImmAttrValuesT_2 *att = NULL;
	int ix;
	for (ix = 0; attr[ix]; ++ix) {
		int ix2;

		att = attr[ix];
		free(att->attrName);	/*free-3 */
		att->attrName = NULL;

		for (ix2 = 0; ix2 < att->attrValuesNumber; ++ix2) {
			SaImmAttrValueT aval = att->attrValues[ix2];
			imma_freeAttrValue3(aval, att->attrValueType);	/*free-5 */
		}
		free(att->attrValues);	/*free-4 */
		att->attrValues = NULL;
		att->attrValuesNumber = 0;
		att->attrValueType = 0;

		free(att);	/*free-2 */
		attr[ix] = NULL;
	}

	free(attr);		/*free-1 */
}

SaAisErrorT saImmOmSearchFinalize(SaImmSearchHandleT searchHandle)
{
	SaAisErrorT error = SA_AIS_OK;
	uns32 proc_rc = NCSCC_RC_SUCCESS;
	NCS_BOOL locked = TRUE;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_SEARCH_NODE *search_node = NULL;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		error = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		locked = FALSE;
		goto release_cb;
	}
	/*locked is true already */

	/*Look up search node */
	proc_rc = imma_search_node_get(&cb->search_tree, &searchHandle, &search_node);

	if (!search_node) {
		error = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	proc_rc = imma_client_node_get(&cb->client_tree, &search_node->mImmHandle, &cl_node);

	if (!(cl_node && cl_node->isOm)) {
		error = SA_AIS_ERR_LIBRARY;
		LOG_ER("Invalid SaImmHandleT related to search handle");
		goto release_lock;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", search_node->mImmHandle);
		error = SA_AIS_OK;	/*Dont punish the client for closing stale handle */
		imma_search_node_delete(cb, search_node);
		goto release_lock;
	}

	if (search_node->mSearchId == 0) {
		error = SA_AIS_ERR_BAD_HANDLE;
		goto release_lock;
	}

	if (search_node->mLastAttributes) {
		TRACE("Freeing last result");
		imma_freeSearchAttrs((SaImmAttrValuesT_2 **)search_node->mLastAttributes);
		search_node->mLastAttributes = 0;
	}

	memset(&evt, 0, sizeof(IMMSV_EVT));
	evt.type = IMMSV_EVT_TYPE_IMMND;
	evt.info.immnd.type = IMMND_EVT_A2ND_SEARCHFINALIZE;
	evt.info.immnd.info.searchOp.searchId = search_node->mSearchId;
	evt.info.immnd.info.searchOp.client_hdl = search_node->mImmHandle;

	/* Release the CB lock Before MDS Send */
	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	locked = FALSE;

	/* IMMND GOES DOWN */
	if (cb->is_immnd_up == FALSE) {
		error = SA_AIS_ERR_TRY_AGAIN;
		TRACE_1("IMMND is DOWN");
		goto release_cb;
	}

	/* send the request to the IMMND */
	proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, &evt, &out_evt, IMMSV_WAIT_TIME);

	switch (proc_rc) {
	case NCSCC_RC_SUCCESS:
		break;

	case NCSCC_RC_REQ_TIMOUT:
		error = SA_AIS_ERR_TIMEOUT;
		goto release_cb;

	default:
		error = SA_AIS_ERR_NO_RESOURCES;
		TRACE_1("MDS returned unexpected error code %u", proc_rc);
		goto release_cb;
	}

	if (out_evt) {
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		error = out_evt->info.imma.info.errRsp.error;
		free(out_evt);

		/* Take the CB lock */
		if (!locked) {
			if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
				error = SA_AIS_ERR_LIBRARY;
				TRACE_1("Lock error");
				return error;
			}
			locked = TRUE;
		}

		proc_rc = imma_search_node_delete(cb, search_node);
		assert(proc_rc == NCSCC_RC_SUCCESS);
	}

 release_lock:
	if (locked) {
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
	}

 release_cb:

	return error;
}

SaAisErrorT saImmOmAdminOwnerSet(SaImmAdminOwnerHandleT adminOwnerHandle,
				 const SaNameT **objectNames, SaImmScopeT scope)
{
	SaAisErrorT rc = SA_AIS_OK;
	/*uns32                 proc_rc = NCSCC_RC_SUCCESS; */
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT admo_set_evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = 0;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	const SaNameT *objectName;
	NCS_BOOL locked = TRUE;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (!objectNames || (objectNames[0] == 0)) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	switch (scope) {
	case SA_IMM_ONE:
	case SA_IMM_SUBLEVEL:
	case SA_IMM_SUBTREE:
		break;
	default:
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &adminOwnerHandle, &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto admowner_not_found;
	}

	rc = imma_client_node_get(&cb->client_tree, &(ao_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		/* This case should not be possible here.
		   If the OM-handle is closed then the admin owner should also 
		   have been closed  AUTOMATICALLY.
		   I guess we leak the admin-owner in this case. 
		 */

		LOG_ER("Admin owner associated with closed client");
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ao_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* populate the structure */
	memset(&admo_set_evt, 0, sizeof(IMMSV_EVT));
	admo_set_evt.type = IMMSV_EVT_TYPE_IMMND;
	admo_set_evt.info.immnd.type = IMMND_EVT_A2ND_ADMO_SET;
	admo_set_evt.info.immnd.info.admReq.adm_owner_id = ao_node->mAdminOwnerId;
	admo_set_evt.info.immnd.info.admReq.scope = scope;

	int i;
	for (i = 0; objectNames[i]; ++i) {
		objectName = objectNames[i];
		assert(objectName->length < SA_MAX_NAME_LENGTH);
		IMMSV_OBJ_NAME_LIST *ol = calloc(1, sizeof(IMMSV_OBJ_NAME_LIST));	/*a */
		ol->name.size = strnlen((char *)objectName->value, SA_MAX_NAME_LENGTH) + 1;
		if (ol->name.size > objectName->length) {
			ol->name.size = objectName->length;
		}
		ol->name.buf = malloc(ol->name.size);	/*b */
		memcpy(ol->name.buf, objectName->value, ol->name.size);
		ol->next = admo_set_evt.info.immnd.info.admReq.objectNames;	/*null initially */
		admo_set_evt.info.immnd.info.admReq.objectNames = ol;
	}

	rc = imma_evt_fake_evs(cb, &admo_set_evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, TRUE);

	while (admo_set_evt.info.immnd.info.admReq.objectNames != NULL) {
		IMMSV_OBJ_NAME_LIST *ol = admo_set_evt.info.immnd.info.admReq.objectNames;
		admo_set_evt.info.immnd.info.admReq.objectNames = admo_set_evt.info.immnd.info.admReq.objectNames->next;

		free(ol->name.buf);	/*free-b */
		ol->name.buf = NULL;
		ol->name.size = 0;
		ol->next = NULL;
		free(ol);	/*free-a */
	}

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is a blocking op. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here */
		free(out_evt);
	}

 mds_send_fail:
 stale_handle:
 client_not_found:
 admowner_not_found:

	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	TRACE_LEAVE();
	return rc;
}

SaAisErrorT saImmOmAdminOwnerRelease(SaImmAdminOwnerHandleT adminOwnerHandle,
				     const SaNameT **objectNames, SaImmScopeT scope)
{
	SaAisErrorT rc = SA_AIS_OK;
	/*uns32                 proc_rc = NCSCC_RC_SUCCESS; */
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT admo_set_evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = 0;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	const SaNameT *objectName;
	NCS_BOOL locked = TRUE;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (!objectNames || (objectNames[0] == 0)) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	switch (scope) {
	case SA_IMM_ONE:
	case SA_IMM_SUBLEVEL:
	case SA_IMM_SUBTREE:
		break;
	default:
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &adminOwnerHandle, &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto admowner_not_found;
	}

	rc = imma_client_node_get(&cb->client_tree, &(ao_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		/* This case should not be possible here.
		   If the OM-handle is closed then the admin owner should also 
		   have been closed  AUTOMATICALLY.
		   I guess we leak the admin-owner in this case. 
		 */

		LOG_ER("Admin owner associated with closed client");
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ao_node->mImmHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* populate the structure */
	memset(&admo_set_evt, 0, sizeof(IMMSV_EVT));
	admo_set_evt.type = IMMSV_EVT_TYPE_IMMND;
	admo_set_evt.info.immnd.type = IMMND_EVT_A2ND_ADMO_RELEASE;
	admo_set_evt.info.immnd.info.admReq.adm_owner_id = ao_node->mAdminOwnerId;
	admo_set_evt.info.immnd.info.admReq.scope = scope;

	int i;
	for (i = 0; objectNames[i]; ++i) {
		objectName = objectNames[i];
		assert(objectName->length < SA_MAX_NAME_LENGTH);
		IMMSV_OBJ_NAME_LIST *ol = calloc(1, sizeof(IMMSV_OBJ_NAME_LIST));	/*a */
		ol->name.size = strnlen((char *)objectName->value, SA_MAX_NAME_LENGTH) + 1;
		if (ol->name.size > objectName->length) {
			ol->name.size = objectName->length;
		}
		ol->name.buf = malloc(ol->name.size);	/*b */
		memcpy(ol->name.buf, objectName->value, ol->name.size);
		ol->next = admo_set_evt.info.immnd.info.admReq.objectNames;	/*null initially */
		admo_set_evt.info.immnd.info.admReq.objectNames = ol;
	}

	rc = imma_evt_fake_evs(cb, &admo_set_evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, TRUE);

	while (admo_set_evt.info.immnd.info.admReq.objectNames != NULL) {
		IMMSV_OBJ_NAME_LIST *ol = admo_set_evt.info.immnd.info.admReq.objectNames;
		admo_set_evt.info.immnd.info.admReq.objectNames = admo_set_evt.info.immnd.info.admReq.objectNames->next;

		free(ol->name.buf);	/*free-b */
		ol->name.buf = NULL;
		ol->name.size = 0;
		ol->next = NULL;
		free(ol);	/*free-a */
	}

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this is a blocking op. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here */
		free(out_evt);
	}

 mds_send_fail:
 stale_handle:
 client_not_found:
 admowner_not_found:

	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	TRACE_LEAVE();
	return rc;
}

SaAisErrorT saImmOmAdminOwnerClear(SaImmHandleT immHandle, const SaNameT **objectNames, SaImmScopeT scope)
{
	SaAisErrorT rc = SA_AIS_OK;
	/*uns32                 proc_rc = NCSCC_RC_SUCCESS; */
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT admo_set_evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = 0;
	const SaNameT *objectName;
	NCS_BOOL locked = TRUE;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (!objectNames || (objectNames[0] == 0)) {
		return SA_AIS_ERR_INVALID_PARAM;
	}

	switch (scope) {
	case SA_IMM_ONE:
	case SA_IMM_SUBLEVEL:
	case SA_IMM_SUBTREE:
		break;
	default:
		return SA_AIS_ERR_INVALID_PARAM;
	}

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	rc = imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", immHandle);
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto stale_handle;
	}

	/* populate the structure */
	memset(&admo_set_evt, 0, sizeof(IMMSV_EVT));
	admo_set_evt.type = IMMSV_EVT_TYPE_IMMND;
	admo_set_evt.info.immnd.type = IMMND_EVT_A2ND_ADMO_CLEAR;
	admo_set_evt.info.immnd.info.admReq.adm_owner_id = 0;
	admo_set_evt.info.immnd.info.admReq.scope = scope;

	int i;
	for (i = 0; objectNames[i]; ++i) {
		objectName = objectNames[i];
		assert(objectName->length < SA_MAX_NAME_LENGTH);
		IMMSV_OBJ_NAME_LIST *ol = calloc(1, sizeof(IMMSV_OBJ_NAME_LIST));	/*a */
		ol->name.size = strnlen((char *)objectName->value, SA_MAX_NAME_LENGTH) + 1;
		if (ol->name.size > objectName->length) {
			ol->name.size = objectName->length;
		}
		ol->name.buf = malloc(ol->name.size);	/*b */
		memcpy(ol->name.buf, objectName->value, ol->name.size);
		ol->next = admo_set_evt.info.immnd.info.admReq.objectNames;	/*null initially */
		admo_set_evt.info.immnd.info.admReq.objectNames = ol;
	}

	rc = imma_evt_fake_evs(cb, &admo_set_evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, TRUE);

	while (admo_set_evt.info.immnd.info.admReq.objectNames != NULL) {
		IMMSV_OBJ_NAME_LIST *ol = admo_set_evt.info.immnd.info.admReq.objectNames;
		admo_set_evt.info.immnd.info.admReq.objectNames = admo_set_evt.info.immnd.info.admReq.objectNames->next;

		free(ol->name.buf);	/*free-b */
		ol->name.buf = NULL;
		ol->name.size = 0;
		ol->next = NULL;
		free(ol);	/*free-a */
	}

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We dont need the lock here  */
		free(out_evt);
	}

 mds_send_fail:
 stale_handle:
 client_not_found:

	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	TRACE_LEAVE();
	return rc;
}

SaAisErrorT saImmOmAdminOwnerFinalize(SaImmAdminOwnerHandleT adminOwnerHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	/*uns32                proc_rc = NCSCC_RC_SUCCESS; */
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT finalize_evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_CLIENT_NODE *cl_node = 0;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	NCS_BOOL locked = TRUE;

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		TRACE_1("Lock failed");
		rc = SA_AIS_ERR_LIBRARY;
		goto lock_fail;
	}

	rc = imma_admin_owner_node_get(&cb->admin_owner_tree, &adminOwnerHandle, &ao_node);
	if (!ao_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		goto admowner_not_found;
	}

	rc = imma_client_node_get(&cb->client_tree, &(ao_node->mImmHandle), &cl_node);
	if (!(cl_node && cl_node->isOm)) {
		rc = SA_AIS_ERR_LIBRARY;
		/*This case should not be possible here.
		   If the OM-handle is closed then the admin owner should also 
		   have been closed  AUTOMATICALLY.
		   I guess we leak the admin-owner in this case. 
		 */

		LOG_ER("Admin owner associated with closed client");
		goto client_not_found;
	}

	if (cl_node->stale) {
		TRACE_2("IMM Handle %llu is stale", ao_node->mImmHandle);
		rc = SA_AIS_OK;	/*Dont punish the client for closing stale handle */
		imma_admin_owner_node_delete(cb, ao_node);
		goto stale_handle;
	}

	/* NOTE: Should close any Ccb handles that are open and associated with AO */

	/* populate the structure */
	memset(&finalize_evt, 0, sizeof(IMMSV_EVT));
	finalize_evt.type = IMMSV_EVT_TYPE_IMMND;
	finalize_evt.info.immnd.type = IMMND_EVT_A2ND_ADMO_FINALIZE;
	finalize_evt.info.immnd.info.admFinReq.adm_owner_id = ao_node->mAdminOwnerId;

	rc = imma_evt_fake_evs(cb, &finalize_evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, FALSE);

	if (rc != SA_AIS_OK) {
		goto mds_send_fail;
	}

	if (out_evt) {
		/* Process the outcome, note this was a blocking call. */
		assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
		assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
		rc = out_evt->info.imma.info.errRsp.error;

		/* We do need the lock here since we are deleting the AO node */

		if (!locked) {
			if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
				rc = SA_AIS_ERR_LIBRARY;
				TRACE_1("Lock failed");
				goto lock_fail1;
			}
			locked = TRUE;
		}

		/*Delete the adminOwnerNode. */
		if (rc == SA_AIS_OK) {
			imma_admin_owner_node_delete(cb, ao_node);
			TRACE("AdminOwnerFinalize: client ao_node deleted");
		}

		/* Free the out event */
		free(out_evt);
	}

 lock_fail1:
 mds_send_fail:
 stale_handle:
 client_not_found:
 admowner_not_found:

	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	return rc;
}

SaAisErrorT saImmOmCcbFinalize(SaImmCcbHandleT ccbHandle)
{
	SaAisErrorT rc = SA_AIS_OK;
	IMMA_CB *cb = &imma_cb;
	IMMSV_EVT evt;
	IMMSV_EVT *out_evt = NULL;
	IMMA_ADMIN_OWNER_NODE *ao_node = NULL;
	IMMA_CLIENT_NODE *cl_node = NULL;
	IMMA_CCB_NODE *ccb_node = NULL;
	NCS_BOOL locked = FALSE;
	TRACE_ENTER();

	if (cb->sv_id == 0) {
		TRACE_2("No initialized handle exists!");
		return SA_AIS_ERR_BAD_HANDLE;
	}

	/* get the CB Lock */
	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
		rc = SA_AIS_ERR_LIBRARY;
		TRACE_1("Lock failed");
		goto lock_fail;
	}
	locked = TRUE;

	/*Get the CCB info */
	imma_ccb_node_get(&cb->ccb_tree, &ccbHandle, &ccb_node);
	if (!ccb_node) {
		rc = SA_AIS_ERR_BAD_HANDLE;
		TRACE_2("Ccb handle not valid");
		goto ccb_not_found;
	}

	if (!ccb_node->mTerminated) {

		/* Get the Admin Owner info  */
		imma_admin_owner_node_get(&cb->admin_owner_tree, &(ccb_node->mAdminOwnerHdl), &ao_node);
		if (!ao_node) {
			rc = SA_AIS_ERR_BAD_HANDLE;
			TRACE_2("No Amin-Owner associated with Ccb");
			goto ao_not_found;
		}

		imma_client_node_get(&cb->client_tree, &(ccb_node->mImmHandle), &cl_node);
		if (!(cl_node && cl_node->isOm)) {
			rc = SA_AIS_ERR_BAD_HANDLE;
			TRACE_2("No valid SaImmHandleT associated with Ccb");
			goto client_not_found;
		}

		if (ccb_node->mImmHandle != ao_node->mImmHandle) {
			rc = SA_AIS_ERR_BAD_HANDLE;
			TRACE_2("Corrupt handle");
			goto client_not_found;
		}

		if (cl_node->stale) {
			TRACE_2("IMM Handle %llu is stale", ccb_node->mImmHandle);
			rc = SA_AIS_OK;	/*Dont punish the client for closing stale handle */
			imma_ccb_node_delete(cb, ccb_node);
			goto stale_handle;
		}

		/* Populate the CcbFinalize event */
		memset(&evt, 0, sizeof(IMMSV_EVT));
		evt.type = IMMSV_EVT_TYPE_IMMND;
		evt.info.immnd.type = IMMND_EVT_A2ND_CCB_FINALIZE;

		evt.info.immnd.info.ccbId = ccb_node->mCcbId;

		rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle, &locked, FALSE);

		if (rc != SA_AIS_OK) {
			goto mds_send_fail;
		}

		if (out_evt) {
			/* Process the outcome, note this is after a blocking call. */
			assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
			assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
			rc = out_evt->info.imma.info.errRsp.error;

			TRACE("CcbFinalize returned %u", rc);

			/* Take the CB lock (do we really need the lock here ..?) */

			if (!locked) {
				if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS) {
					rc = SA_AIS_ERR_LIBRARY;
					TRACE_1("Lock failed");
					goto lock_fail;
				}
				locked = TRUE;
			}
			free(out_evt);
		}
	}

	/*Delete the Ccb node */
	if (rc == SA_AIS_OK) {
		imma_ccb_node_delete(cb, ccb_node);
		TRACE("CcbFinalize: client ccb node deleted");
	}

 mds_send_fail:
 ccb_not_found:
 ao_not_found:
 client_not_found:
 stale_handle:
	if (locked)
		m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:

	TRACE_LEAVE();
	return rc;
}
