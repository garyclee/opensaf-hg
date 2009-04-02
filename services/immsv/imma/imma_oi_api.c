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

#include "imma.h"
#include "immsv_api.h"

static const char * sysaClName = SA_IMM_ATTR_CLASS_NAME;
static const char * sysaAdmName = SA_IMM_ATTR_ADMIN_OWNER_NAME;
static const char * sysaImplName = SA_IMM_ATTR_IMPLEMENTER_NAME;

/****************************************************************************
  Name          :  SaImmOiInitialize
 
  Description   :  This function initializes the IMM OI Service for the
                   invoking process and registers the callback functions.
                   The handle 'immOiHandle' is returned as the reference to
                   this association between the process and the ImmOi Service.
                   
 
  Arguments     :  immOiHandle -  A pointer to the handle designating this 
                                particular initialization of the IMM OI
                                service that is to be returned by the 
                                ImmOi Service.
                   immOiCallbacks  - Pointer to a SaImmOiCallbacksT structure, 
                                containing the callback functions of the
                                process that the ImmOi Service may invoke.
                   version    - Is a pointer to the version of the Imm
                                Service that the invoking process is using.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
#ifdef IMM_A_01_01
SaAisErrorT saImmOiInitialize(SaImmOiHandleT *immOiHandle,
                              const SaImmOiCallbacksT *immOiCallbacks,
                              SaVersionT *version)

{
    return saImmOiInitialize_2(immOiHandle, 
                               (const SaImmOiCallbacksT_2 *) immOiCallbacks,
                               version);
}
#endif

SaAisErrorT saImmOiInitialize_2(SaImmOiHandleT *immOiHandle,
                                const SaImmOiCallbacksT_2 *immOiCallbacks,
                                SaVersionT *version)

{
    IMMA_CB           *cb = &imma_cb;
    SaAisErrorT       rc = SA_AIS_OK;
    IMMSV_EVT         init_evt;
    IMMSV_EVT         *out_evt=NULL;
    uns32             proc_rc = NCSCC_RC_SUCCESS;
    IMMA_CLIENT_NODE  *cl_node=0;
    NCS_BOOL          locked = TRUE;

    TRACE_ENTER();

    proc_rc = imma_startup(NCSMDS_SVC_ID_IMMA_OI);
    if (NCSCC_RC_SUCCESS != proc_rc)
    {
        TRACE_1("Agents_startup failed");
        TRACE_LEAVE();
        return SA_AIS_ERR_LIBRARY; 
    }

    if ((!immOiHandle) || (!version))
    {
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto end;
    }

    *immOiHandle = 0;

    /* Alloc the client info data structure & put it in the Pat tree */
    cl_node = (IMMA_CLIENT_NODE *) calloc(1, sizeof(IMMA_CLIENT_NODE));
    if (cl_node == NULL)
    {
        TRACE_1("IMMA_CLIENT_NODE alloc failed");
        rc = SA_AIS_ERR_NO_MEMORY;
        goto cnode_alloc_fail;
    }

    cl_node->isOm=FALSE;
#ifdef IMM_A_01_01
    if((version->releaseCode == 'A') &&
        (version->majorVersion == 0x01)) {
        TRACE("THIS IS A VERSION A.1.x implementer %c %u %u",
            version->releaseCode, version->majorVersion,
            version->minorVersion);
        cl_node->isOiA1 = TRUE;
    } else {
        TRACE("THIS IS A VERSION A.2.x implementer %c %u %u",
            version->releaseCode, version->majorVersion,
            version->minorVersion);
        cl_node->isOiA1 = FALSE;
    }
#endif

    /* Take the CB Lock */
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("LOCK failed");
        rc = SA_AIS_ERR_LIBRARY;
        goto lock_fail;
    }

    /* Draft Validations : Version this is the politically correct validatioin
     distinct from the pragmatic validation we do above. */
    rc = imma_version_validate(version);
    if (rc != SA_AIS_OK)
    {
        TRACE_2("Version validation failed");
        goto version_fail;
    }


    /* Store the callback functions, if set */
    if (immOiCallbacks)
    {
#ifdef IMM_A_01_01
        if(cl_node->isOiA1) {
            cl_node->o.iCallbk1 = *((const SaImmOiCallbacksT *)immOiCallbacks);
            TRACE("Version A.1.x callback registered:");
            TRACE("RtAttrUpdateCb:%p", 
                cl_node->o.iCallbk1.saImmOiRtAttrUpdateCallback);
            TRACE("CcbObjectCreateCb:%p", 
                cl_node->o.iCallbk1.saImmOiCcbObjectCreateCallback);
            TRACE("CcbObjectDeleteCb:%p", 
                cl_node->o.iCallbk1.saImmOiCcbObjectDeleteCallback);
            TRACE("CcbObjectModifyCb:%p", 
                cl_node->o.iCallbk1.saImmOiCcbObjectModifyCallback);
            TRACE("CcbCompletedCb:%p", 
                cl_node->o.iCallbk1.saImmOiCcbCompletedCallback);
            TRACE("CcbApplyCb:%p", 
                cl_node->o.iCallbk1.saImmOiCcbApplyCallback);
            TRACE("CcbAbortCb:%p", 
                cl_node->o.iCallbk1.saImmOiCcbAbortCallback);
            TRACE("AdminOperationCb:%p", 
                cl_node->o.iCallbk1.saImmOiAdminOperationCallback);
        } else {
            cl_node->o.iCallbk = *immOiCallbacks;
        }
#else
        cl_node->o.iCallbk = *immOiCallbacks;
#endif

        proc_rc = imma_callback_ipc_init(cl_node);
    }

    if (proc_rc != NCSCC_RC_SUCCESS)
    {
        /* Error handling */
        rc = SA_AIS_ERR_LIBRARY;
        /* ALready log'ed by imma_callback_ipc_init */
        goto ipc_init_fail;
    }

    /* populate the EVT structure */
    memset(&init_evt, 0, sizeof(IMMSV_EVT));
    init_evt.type = IMMSV_EVT_TYPE_IMMND;
    init_evt.info.immnd.type = IMMND_EVT_A2ND_IMM_OI_INIT;
    init_evt.info.immnd.info.initReq.version = *version;
    init_evt.info.immnd.info.initReq.client_pid = cb->process_id;

    /* Release the CB lock Before MDS Send */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    locked = FALSE;

    /* IMMND GOES DOWN */
    if (FALSE == cb->is_immnd_up)
    {
        rc = SA_AIS_ERR_TRY_AGAIN;
        TRACE_2("IMMND is DOWN");
        goto mds_fail;
    }


    /* send the request to the IMMND */
    proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, 
                                     &init_evt, &out_evt, IMMSV_WAIT_TIME);

    /* Error Handling */
    switch (proc_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            goto mds_fail;
        default:
            TRACE_1("MDS returned unexpected error code %u", proc_rc);
            rc = SA_AIS_ERR_LIBRARY;
            goto mds_fail;
    }

    if (out_evt)
    {
        rc = out_evt->info.imma.info.initRsp.error;
        if (rc != SA_AIS_OK)
        {
            goto rsp_not_ok;
        }

        /* Take the CB lock after MDS Send */
        if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
        {
            TRACE_1("LOCK failed");
            rc = SA_AIS_ERR_LIBRARY;
            goto lock_fail1;
        }
        else
        {
            locked = TRUE;
        }

        if (FALSE == cb->is_immnd_up)
        {
            /*IMMND went down during THIS call!*/
            rc = SA_AIS_ERR_TRY_AGAIN;
            TRACE_2("IMMND is DOWN");
            goto rsp_not_ok;
        }


        cl_node->handle = out_evt->info.imma.info.initRsp.immHandle;

        proc_rc = imma_client_node_add(&cb->client_tree, cl_node);
        if (proc_rc != NCSCC_RC_SUCCESS)
        {
            IMMA_CLIENT_NODE  *stale_node=NULL;
            (void) imma_client_node_get(&cb->client_tree, &(cl_node->handle), 
                &stale_node);

            if((stale_node != NULL) && stale_node->stale)
            {
                TRACE_2("Removing stale client");
                imma_finalize_proc(cb, stale_node);
                imma_shutdown(NCSMDS_SVC_ID_IMMA_OI);
                TRACE_2("Retrying add of client node");
                proc_rc = imma_client_node_add(&cb->client_tree, cl_node);
            }

            if(proc_rc != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("client_node_add failed");
                goto node_add_fail;
            }
        }
    }
    else
    {
        TRACE_1("Empty reply received");
        rc = SA_AIS_ERR_NO_RESOURCES;
    }


    /*Error handling */
    node_add_fail:
    lock_fail1: 
    if (rc != SA_AIS_OK)
    {
        IMMSV_EVT    finalize_evt, *out_evt1 ; 

        out_evt1 = NULL;
        memset(&finalize_evt, 0, sizeof(IMMSV_EVT));
        finalize_evt.type = IMMSV_EVT_TYPE_IMMND;
        finalize_evt.info.immnd.type = IMMND_EVT_A2ND_IMM_OI_FINALIZE;
        finalize_evt.info.immnd.info.finReq.client_hdl = cl_node->handle;

        if (locked)
        {
            m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
            locked = FALSE;
        }

        /* send the request to the IMMND */
        proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, 
                                         &(cb->immnd_mds_dest), 
                                         &finalize_evt,&out_evt1,
                                         IMMSV_WAIT_TIME);
        if (out_evt1)
        {
            free(out_evt1);
        }
    }

    rsp_not_ok:   
    mds_fail:

    /* Free the IPC initialized for this client */
    if (rc != SA_AIS_OK)
        imma_callback_ipc_destroy(cl_node);

ipc_init_fail:   
version_fail:
    if (rc != SA_AIS_OK)
        free(cl_node);

    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

cnode_alloc_fail:
lock_fail:
    if (out_evt)
        free(out_evt);

    if (rc == SA_AIS_OK)
    {
        /* Went well, return immHandle to the application */   
        *immOiHandle = cl_node->handle;
    }

    end:
    if (rc != SA_AIS_OK)
    {
        imma_shutdown(NCSMDS_SVC_ID_IMMA_OI);
    }
    TRACE_LEAVE();
    return rc;    
}

/****************************************************************************
  Name          :  saImmOiSelectionObjectGet
 
  Description   :  This function returns the operating system handle 
                   associated with the immOiHandle.
 
  Arguments     :  immOiHandle - Imm OI service handle.
                   selectionObject - Pointer to the operating system handle.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOiSelectionObjectGet(SaImmOiHandleT immOiHandle,
                                      SaSelectionObjectT *selectionObject)
{
    SaAisErrorT       rc = SA_AIS_OK;
    IMMA_CB           *cb = &imma_cb;
    IMMA_CLIENT_NODE  *cl_node=0;
    uns32             proc_rc = NCSCC_RC_FAILURE;

    if (!selectionObject)
        return SA_AIS_ERR_INVALID_PARAM;

    /* Take the CB lock */
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("LOCK failed");
        rc = SA_AIS_ERR_LIBRARY;
        goto lock_fail;
    }

    proc_rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);

    if (!cl_node || cl_node->isOm)
    {
        TRACE_2("Bad handle %llu", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto node_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    *selectionObject = (SaSelectionObjectT)
                       m_GET_FD_FROM_SEL_OBJ(m_NCS_IPC_GET_SEL_OBJ(&cl_node->callbk_mbx));

 stale_handle:
 node_not_found:
    /* Unlock CB */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

lock_fail:

    return rc;
}


/****************************************************************************
  Name          :  saImmOiDispatch
 
  Description   :  This function invokes, in the context of the calling
                   thread, pending callbacks for the handle immOiHandle in a 
                   way that is specified by the dispatchFlags parameter.
 
  Arguments     :  immOiHandle - IMM OI Service handle
                   dispatchFlags - Flags that specify the callback execution
                                   behaviour of this function.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOiDispatch(SaImmOiHandleT immOiHandle,
                            SaDispatchFlagsT dispatchFlags)

{
    SaAisErrorT    rc = SA_AIS_OK;
    IMMA_CB      *cb = &imma_cb;
    IMMA_CLIENT_NODE   *cl_node=0;

    TRACE_ENTER();

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("LOCK failed");
        rc = SA_AIS_ERR_LIBRARY;
        goto fail;
    }

    /* get the client_info*/
    rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node|| cl_node->isOm)
    {
        TRACE_2("client_node_get failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        goto fail;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        goto fail;
    }

    /* Do unlock and then do the dispatch processing to avoid deadlock in 
       arrival callback ,*/
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

    switch (dispatchFlags)
    {
        case SA_DISPATCH_ONE:
            rc = imma_hdl_callbk_dispatch_one(cb, immOiHandle); 
            break;

        case SA_DISPATCH_ALL:
            rc = imma_hdl_callbk_dispatch_all(cb, immOiHandle); 
            break;

        case SA_DISPATCH_BLOCKING:
            rc = imma_hdl_callbk_dispatch_block(cb, immOiHandle); 
            break;

        default:
            rc = SA_AIS_ERR_INVALID_PARAM;
            break;
    } /* switch */

 fail:

     TRACE_LEAVE();
    return rc;
}

/****************************************************************************
  Name          :  saImmOiFinalize
 
  Description   :  This function closes the association, represented by
                   immOiHandle, between the invoking process and the IMM OI
                   service.
 
  Arguments     :  immOiHandle - IMM OI Service handle.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOiFinalize(SaImmOiHandleT immOiHandle)
{
    SaAisErrorT          rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             finalize_evt;
    IMMSV_EVT             *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node=0;
    uns32                proc_rc = NCSCC_RC_SUCCESS;
    NCS_BOOL             locked = TRUE;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("LOCK failed");
        rc = SA_AIS_ERR_LIBRARY;
        goto lock_fail;
    }

    /* get the client_info*/
    proc_rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node || cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("client_node_get failed");
        goto node_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    /* populate the structure */
    memset(&finalize_evt, 0, sizeof(IMMSV_EVT));
    finalize_evt.type = IMMSV_EVT_TYPE_IMMND;
    finalize_evt.info.immnd.type = IMMND_EVT_A2ND_IMM_OI_FINALIZE;
    finalize_evt.info.immnd.info.finReq.client_hdl = cl_node->handle;

    /* Unlock before MDS Send */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    locked = FALSE;

    if (cb->is_immnd_up == FALSE)
    {
        rc = SA_AIS_ERR_TRY_AGAIN;
        TRACE_2("IMMND is DOWN");
        goto mds_send_fail; 
    }

    /* send the request to the IMMND */
    proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &(cb->immnd_mds_dest), 
                                     &finalize_evt,&out_evt, IMMSV_WAIT_TIME);

    /* MDS error handling */
    switch (proc_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            goto mds_send_fail;
        default:
            TRACE_1("MDS returned unexpected error code %u",
                   proc_rc);
            rc = SA_AIS_ERR_LIBRARY;
            goto mds_send_fail;
    }

    /* Read the received error (if any)  */
    if (out_evt)
    {
        rc = out_evt->info.imma.info.errRsp.error;
        free(out_evt);
    }
    else
    {
        rc = SA_AIS_ERR_NO_RESOURCES;
        TRACE_1("Received empty reply from server");
    }

    /* Do the finalize processing at IMMA */
    if (rc == SA_AIS_OK)
    {
        /* Take the CB lock  */
        if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
        {
            rc = SA_AIS_ERR_LIBRARY;
            TRACE_1("LOCK failed");
            goto lock_fail1;
        }

        locked = TRUE;
        imma_finalize_proc(cb, cl_node);
    }

 lock_fail1:   
 mds_send_fail:   
 node_not_found:
 stale_handle:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:
    if (rc==SA_AIS_OK)
    {
        imma_shutdown(NCSMDS_SVC_ID_IMMA_OI);
    }
    return rc;
}

/****************************************************************************
  Name          :  saImmOiAdminOperationResult
 
  Description   :  
 
  Arguments     :  immOiHandle - IMM OI Service handle.
 
  Return Values :  Refer to SAI-AIS specification for various return values.
 
  Notes         :
******************************************************************************/
SaAisErrorT saImmOiAdminOperationResult(SaImmOiHandleT immOiHandle,
                                        SaInvocationT invocation,
                                        SaAisErrorT result)
{
    SaAisErrorT          rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             adminOpRslt_evt;
    IMMA_CLIENT_NODE      *cl_node=0;
    uns32                proc_rc = NCSCC_RC_SUCCESS;
    NCS_BOOL             locked = TRUE;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("LOCK failed");
        rc = SA_AIS_ERR_LIBRARY;
        goto lock_fail;
    }

    /* get the client_info*/
    proc_rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node || cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("client_node_get failed");
        goto node_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    ImmsvInvocation theInvocation = (*((ImmsvInvocation *) &invocation));

    /* populate the structure */
    memset(&adminOpRslt_evt, 0, sizeof(IMMSV_EVT));
    adminOpRslt_evt.type = IMMSV_EVT_TYPE_IMMND;
    /*Need to encode async/sync variant.*/
    if (theInvocation.inv < 0)
    {
        adminOpRslt_evt.info.immnd.type = IMMND_EVT_A2ND_ASYNC_ADMOP_RSP;
    }
    else
    {
        adminOpRslt_evt.info.immnd.type = IMMND_EVT_A2ND_ADMOP_RSP;
    }
    adminOpRslt_evt.info.immnd.info.admOpRsp.oi_client_hdl = immOiHandle;
    adminOpRslt_evt.info.immnd.info.admOpRsp.invocation = invocation;
    adminOpRslt_evt.info.immnd.info.admOpRsp.result = result;
    adminOpRslt_evt.info.immnd.info.admOpRsp.error = SA_AIS_OK;

    /* Unlock before MDS Send */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    locked = FALSE;


    /* IMMND GOES DOWN */
    if (cb->is_immnd_up == FALSE)
    {
        rc = SA_AIS_ERR_NO_RESOURCES;
        TRACE_2("IMMND_DOWN");
        goto mds_send_fail; 
    }

    /* send the reply to the IMMND asyncronously */
    proc_rc = imma_mds_msg_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, 
                                &adminOpRslt_evt, NCSMDS_SVC_ID_IMMND);

    /* MDS error handling */
    switch (proc_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            goto mds_send_fail;
        default:
            TRACE_1("MDS returned unexpected error code %u", proc_rc);
            rc = SA_AIS_ERR_LIBRARY;
            goto mds_send_fail;
    }

 mds_send_fail:
 stale_handle:
 node_not_found:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:
    return rc;
}


/****************************************************************************
  Name          :  saImmOiImplementerSet
 
  Description   :  Initialize an object implementer, associating this process
                   with an implementer name.
                   This is a blocking call.

                   
  Arguments     :  immOiHandle - IMM OI handle
                   implementerName - The name of the implementer.

  Dont need any library local implementerInstance corresponding to
  say the adminowner instance of the OM-API. Because there is
  nothing corresponding to the adminOwnerHandle.
  Instead the fact that implementer has been set can be associated with
  the immOiHandle.

  The standard is unclear on the question if a process can represent
  several implementer (names) simultaneously. We ASSUME the answer is no.
  Mainly because it is simpler to implement, but also because there is
  no separate "name handle". 
  This then means that any new invocation of this method using the same
  immOiHandle must either return with error, or perform an implicit clear 
  of the previous implementer name. We choose to return error.


  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
SaAisErrorT saImmOiImplementerSet(SaImmOiHandleT immOiHandle,
                                  const SaImmOiImplementerNameT implementerName)
{
    SaAisErrorT           rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             evt;
    IMMSV_EVT             *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL; 
    SaUint32T             proc_rc = NCSCC_RC_SUCCESS;
    SaBoolT               locked = SA_TRUE;
    SaUint32T             nameLen = 0;


    if ((implementerName == NULL) || 
        (nameLen = strlen(implementerName)) == 0)
    {
        return SA_AIS_ERR_INVALID_PARAM;
    }
    ++nameLen; /*Add 1 for the null.*/

    if (nameLen >= SA_MAX_NAME_LENGTH)
    {
        TRACE_1("Implementer name too long, size: %u max:%u", 
               nameLen, SA_MAX_NAME_LENGTH);
        return SA_AIS_ERR_LIBRARY;     
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }

    /*locked == TRUE already*/

    rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    if (cl_node->mImplementerId)
    {
        rc = SA_AIS_ERR_EXIST;
        /* Clarified in SAI-AIS-IMM-A.03.01 */
        TRACE_2("Implementer already set for this handle");
        goto bad_handle;
    }


    /* Populate & Send the Open Event to IMMND */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_IMPL_SET; 
    evt.info.immnd.info.implSet.client_hdl = immOiHandle;
    evt.info.immnd.info.implSet.impl_name.size = nameLen;
    evt.info.immnd.info.implSet.impl_name.buf = implementerName;

    /* Unlock before MDS Send */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    locked = FALSE;

    /* IMMND GOES DOWN */
    if (cb->is_immnd_up == FALSE)
    {
        rc = SA_AIS_ERR_TRY_AGAIN;
        TRACE_2("IMMND is DOWN");
        goto mds_send_fail;
    }

    /* Send the evt to IMMND */
    proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &(cb->immnd_mds_dest), 
                                     &evt, &out_evt, IMMSV_WAIT_TIME);

    evt.info.immnd.info.implSet.impl_name.buf = NULL;
    evt.info.immnd.info.implSet.impl_name.size = 0;

    /* Generate rc from proc_rc */
    switch (proc_rc)
    {
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

    if (out_evt)
    {
        /* Process the received Event */
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMPLSET_RSP);
        rc = out_evt->info.imma.info.implSetRsp.error;

        /* Take the CB lock */
        if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
        {
            rc = SA_AIS_ERR_LIBRARY;
            TRACE_1("Lock failed");
            goto mds_send_fail;
        }

        locked = TRUE;

        if (rc == SA_AIS_OK)
        {
            cl_node->mImplementerId = out_evt->info.imma.info.implSetRsp.implId;
        }
        free(out_evt);
    }

 mds_send_fail:
 bad_handle:
 stale_handle:
 client_not_found:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

    lock_fail:   

    return rc;

}

/****************************************************************************
  Name          :  saImmOiImplementerClear
 
  Description   :  Clear implementer. Severs the association between
                   this process/connection and the implementer name set
                   by saImmOiImplementerSet.
                   This is a blocking call.

                   
  Arguments     :  immOiHandle - IMM OI handle

  This call will fail if not a prior call to implementerSet has been made.

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
SaAisErrorT saImmOiImplementerClear(SaImmOiHandleT immOiHandle)
{
    SaAisErrorT           rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             evt;
    IMMSV_EVT             *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL; 
    SaBoolT               locked = SA_TRUE;

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }

    /*locked == TRUE already*/

    rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    if (cl_node->mImplementerId == 0)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("No implementer is set for this handle");
        goto bad_handle;
    }

    /* Populate & Send the Open Event to IMMND */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_IMPL_CLR; 
    evt.info.immnd.info.implSet.client_hdl = immOiHandle;
    evt.info.immnd.info.implSet.impl_id = cl_node->mImplementerId;

    rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle,
                           &locked, TRUE);

    if (rc!= SA_AIS_OK)
    {
        goto mds_send_fail;
    }

    if (out_evt)
    {
        /* Process the received Event */
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

        /* Take the CB lock */
        if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
        {
            rc = SA_AIS_ERR_LIBRARY;
            TRACE_1("Lock failed");
            goto mds_send_fail;
        }

        locked = TRUE;

        if (rc == SA_AIS_OK)
        {
            /*NOTE: should probably look up cl_node again after locking. */
            cl_node->mImplementerId = 0;
        }
        free(out_evt);
    }

 mds_send_fail:
 bad_handle:
 stale_handle:
 client_not_found:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:   

    return rc;
}

/****************************************************************************
  Name          :  saImmOiClassImplementerSet
 
  Description   :  Set implementer for class and all instances.
                   This is a blocking call.

                   
  Arguments     :  immOiHandle - IMM OI handle
                   className - The name of the class.

  This call will fail if not a prior call to implementerSet has been made.

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
SaAisErrorT saImmOiClassImplementerSet(SaImmOiHandleT immOiHandle,
                                       const SaImmClassNameT className)
{
    SaAisErrorT           rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             evt;
    IMMSV_EVT             *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL; 
    SaBoolT               locked = SA_TRUE;
    SaUint32T             nameLen = 0;


    if ((className == NULL) || 
        (nameLen = strlen(className)) == 0)
    {
        return SA_AIS_ERR_INVALID_PARAM;
    }
    ++nameLen; /*Add 1 for the null.*/

    if (nameLen >= SA_MAX_NAME_LENGTH)
    {
        TRACE_1("Implementer name too long, size: %u max:%u", 
               nameLen, SA_MAX_NAME_LENGTH);

        return SA_AIS_ERR_LIBRARY;     
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }

    /*locked == TRUE already*/

    rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    if (cl_node->mImplementerId == 0)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("No implementer is set for this handle");
        goto bad_handle;
    }

    /* Populate & Send the Open Event to IMMND */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_CL_IMPL_SET; 
    evt.info.immnd.info.implSet.client_hdl = cl_node->handle;
    evt.info.immnd.info.implSet.impl_name.size = nameLen;
    evt.info.immnd.info.implSet.impl_name.buf = className;
    evt.info.immnd.info.implSet.impl_id = cl_node->mImplementerId;

    rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle,
                           &locked, TRUE);

    evt.info.immnd.info.implSet.impl_name.buf = NULL;
    evt.info.immnd.info.implSet.impl_name.size = 0;

    if (rc!= SA_AIS_OK)
    {
        goto mds_send_fail;
    }

    if (out_evt)
    {
        /* Process the received Event */
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

#if 0 /* Dont need to lock here! In fact it just adds an error case. */
        if (!locked)
        {
            /* Take the CB lock */
            if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("Lock failed");
                /*NOTE - should undo the class implementer towards server.*/
                goto mds_send_fail;
            }
        }

        locked = TRUE;
#endif

        free(out_evt);
    }

 mds_send_fail:
 bad_handle:
 stale_handle:
 client_not_found:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

    lock_fail:   

    return rc;

}

/****************************************************************************
  Name          :  saImmOiClassImplementerRelease
 
  Description   :  Release implementer for class and all instances.
                   This is a blocking call.

                   
  Arguments     :  immOiHandle - IMM OI handle
                   className - The name of the class.

  This call will fail if not a prior call to ClassImplementerSet has been made.

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
SaAisErrorT saImmOiClassImplementerRelease(SaImmOiHandleT immOiHandle,
                                           const SaImmClassNameT className)
{
    SaAisErrorT           rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             evt;
    IMMSV_EVT             *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL; 
    SaBoolT               locked = SA_TRUE;
    SaUint32T             nameLen = 0;


    if ((className == NULL) || 
        (nameLen = strlen(className)) == 0)
    {
        return SA_AIS_ERR_INVALID_PARAM;
    }
    ++nameLen; /*Add 1 for the null.*/

    if (nameLen >= SA_MAX_NAME_LENGTH)
    {
        TRACE_1("Implementer name too long, size: %u max:%u", 
               nameLen, SA_MAX_NAME_LENGTH);

        return SA_AIS_ERR_LIBRARY;     
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }

    /*locked == TRUE already*/

    rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    /* Populate & Send the Open Event to IMMND */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_CL_IMPL_REL; 
    evt.info.immnd.info.implSet.client_hdl = cl_node->handle;
    evt.info.immnd.info.implSet.impl_name.size = nameLen;
    evt.info.immnd.info.implSet.impl_name.buf = className;
    evt.info.immnd.info.implSet.impl_id = cl_node->mImplementerId;

    rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle,
                           &locked, TRUE);

    evt.info.immnd.info.implSet.impl_name.buf = NULL;
    evt.info.immnd.info.implSet.impl_name.size = 0;

    if (rc!= SA_AIS_OK)
    {
        goto mds_send_fail;
    }

    if (out_evt)
    {
        /* Process the received Event */
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

#if 0 /* Dont need to lock here! In fact it just adds an error case. */
        if (!locked)
        {
            /* Take the CB lock */
            if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("Lock failed");
                /*NOTE - should undo the class implementer towards server.*/
                goto mds_send_fail;
            }
        }

        locked = TRUE;
#endif

        free(out_evt);
    }

 mds_send_fail:
 stale_handle:
 client_not_found:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:   

    return rc;

}

/****************************************************************************
  Name          :  saImmOiObjectImplementerSet
 
  Description   :  Set implementer for the objects identified by scope
                   and objectName. This is a blocking call.

                   
  Arguments     :  immOiHandle - IMM OI handle
                   objectName - The name of the top object.

  This call will fail if not a prior call to implementerSet has been made.

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
SaAisErrorT saImmOiObjectImplementerSet(SaImmOiHandleT immOiHandle,
                                        const SaNameT *objectName,
                                        SaImmScopeT scope)
{
    SaAisErrorT           rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             evt;
    IMMSV_EVT             *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL; 
    SaBoolT               locked = SA_TRUE;
    SaUint32T             nameLen = 0;


    if ((objectName == NULL) || (objectName->length > SA_MAX_NAME_LENGTH) ||
        (objectName->length == 0 ))
    {
        return SA_AIS_ERR_INVALID_PARAM;
    }

    TRACE_1("value:'%s' len:%u", objectName->value, objectName->length);
    nameLen = objectName->length + 1;

    TRACE_1("Corrected nameLen = %u", nameLen);

    switch(scope)
    {
        case SA_IMM_ONE:
        case SA_IMM_SUBLEVEL:
        case SA_IMM_SUBTREE:
            break;
        default:
            return SA_AIS_ERR_INVALID_PARAM;
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }

    /*locked == TRUE already*/

    rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    if (cl_node->mImplementerId == 0)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("No implementer is set for this handle");
        goto bad_handle;
    }

    /* Populate & Send the Open Event to IMMND */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_OBJ_IMPL_SET; 
    evt.info.immnd.info.implSet.client_hdl = cl_node->handle;
    evt.info.immnd.info.implSet.impl_name.size = nameLen;
    evt.info.immnd.info.implSet.impl_name.buf = malloc(nameLen);
    strncpy(evt.info.immnd.info.implSet.impl_name.buf, (char *) objectName->value, 
           nameLen-1);
    evt.info.immnd.info.implSet.impl_name.buf[nameLen-1] = 0;
    TRACE_1("Sending size:%u val:'%s'", nameLen-1, 
        evt.info.immnd.info.implSet.impl_name.buf);
    evt.info.immnd.info.implSet.impl_id = cl_node->mImplementerId;
    evt.info.immnd.info.implSet.scope = scope;

    rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle,
                           &locked, TRUE);
    free(evt.info.immnd.info.implSet.impl_name.buf);
    evt.info.immnd.info.implSet.impl_name.buf = NULL;
    evt.info.immnd.info.implSet.impl_name.size = 0;

    if (rc!= SA_AIS_OK)
    {
        goto mds_send_fail;
    }

    if (out_evt)
    {
        /* Process the received Event */
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

#if 0 /* Dont need to lock here! In fact it just adds an error case. */
        if (!locked)
        {
            /* Take the CB lock */
            if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("Lock failed");
                /*NOTE - should undo the object implementer towards server.*/
                goto mds_send_fail;
            }
        }

        locked = TRUE;
#endif

        free(out_evt);
    }

 mds_send_fail:
 bad_handle:   
 stale_handle:
 client_not_found:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

    lock_fail:   

    return rc;
}

/****************************************************************************
  Name          :  saImmOiObjectImplementerRelease
 
  Description   :  Release implementer for the objects identified by scope
                   and objectName. This is a blocking call.

                   
  Arguments     :  immOiHandle - IMM OI handle
                   objectName - The name of the top object.

  This call will fail if not a prior call to objectImplementerSet has been 
  made.

  Return Values :  Refer to SAI-AIS specification for various return values.
******************************************************************************/
SaAisErrorT saImmOiObjectImplementerRelease(SaImmOiHandleT immOiHandle,
                                            const SaNameT *objectName,
                                            SaImmScopeT scope)
{
    SaAisErrorT           rc = SA_AIS_OK;
    IMMA_CB               *cb = &imma_cb;
    IMMSV_EVT             evt;
    IMMSV_EVT             *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL; 
    SaBoolT               locked = SA_TRUE;
    SaUint32T             nameLen = 0;

    if ((objectName == NULL) || 
        ((nameLen = strlen((char *) objectName->value)) == 0) ||
        (objectName->length > SA_MAX_NAME_LENGTH))
    {
        return SA_AIS_ERR_INVALID_PARAM;
    }
    if (objectName->length < nameLen)
    {
        nameLen = objectName->length;
    }
    ++nameLen; /*Add 1 for the null.*/

    switch(scope)
    {
        case SA_IMM_ONE:
        case SA_IMM_SUBLEVEL:
        case SA_IMM_SUBTREE:
            break;
        default:
            return SA_AIS_ERR_INVALID_PARAM;
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }

    /*locked == TRUE already*/

    rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    /* Populate & Send the Open Event to IMMND */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_OBJ_IMPL_REL; 
    evt.info.immnd.info.implSet.client_hdl = cl_node->handle;
    evt.info.immnd.info.implSet.impl_name.size = nameLen;
    evt.info.immnd.info.implSet.impl_name.buf = calloc(1, nameLen);
    strncpy(evt.info.immnd.info.implSet.impl_name.buf,
        (char *) objectName->value, nameLen);
    evt.info.immnd.info.implSet.impl_id = cl_node->mImplementerId;
    evt.info.immnd.info.implSet.scope = scope;

    rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle,
                           &locked, TRUE);

    free(evt.info.immnd.info.implSet.impl_name.buf);
    evt.info.immnd.info.implSet.impl_name.buf = NULL;
    evt.info.immnd.info.implSet.impl_name.size = 0;

    if (rc!= SA_AIS_OK)
    {
        goto mds_send_fail;
    }

    if (out_evt)
    {
        /* Process the received Event */
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

#if 0 /* Dont need to lock here! In fact it just adds an error case. */
        if (!locked)
        {
            /* Take the CB lock */
            if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("Lock failed");
                /*NOTE - should undo the object implementer towards server.*/
                goto mds_send_fail;
            }
        }

        locked = TRUE;
#endif

        free(out_evt);
    }

 mds_send_fail:
 client_not_found:
 stale_handle:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

 lock_fail:   

    return rc;
}


#ifdef IMM_A_01_01  
extern SaAisErrorT saImmOiRtObjectUpdate(SaImmOiHandleT immOiHandle,
                                         const SaNameT *objectName,
                                         const SaImmAttrModificationT **attrMods)
{
    return saImmOiRtObjectUpdate_2(immOiHandle, objectName,
                                   (const SaImmAttrModificationT_2 **) 
                                   attrMods);
}
#endif

SaAisErrorT saImmOiRtObjectUpdate_2(SaImmOiHandleT immOiHandle,
                                    const SaNameT *objectName,
                                    const SaImmAttrModificationT_2 **attrMods)
{
    SaAisErrorT     rc = SA_AIS_OK;
    uns32 proc_rc = NCSCC_RC_SUCCESS;
    IMMA_CB         *cb = &imma_cb;
    IMMSV_EVT        evt;
    IMMSV_EVT        *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL;
    NCS_BOOL       locked = FALSE;
    TRACE_ENTER();

    if ((objectName == NULL) || (objectName->length > SA_MAX_NAME_LENGTH))
    {
        TRACE_2("objectName is NULL or length of name is greater than %u", 
            SA_MAX_NAME_LENGTH);
        TRACE_LEAVE();
        return SA_AIS_ERR_INVALID_PARAM;
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }
    locked = TRUE;

    proc_rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("Non valid SaImmOiHandleT");
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    if(cl_node->mImplementerId == 0) {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("The SaImmOiHandleT is not associated with any implementer name");
        goto stale_handle;
    }

    /* Populate the Object-Update event */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_OBJ_MODIFY;

    evt.info.immnd.info.objModify.immHandle = immOiHandle;

    /*NOTE: should rename member adminOwnerId !!!*/
    evt.info.immnd.info.objModify.adminOwnerId = cl_node->mImplementerId;

    if (objectName->length)
    {
        evt.info.immnd.info.objModify.objectName.size =
            strlen((char *) objectName->value)+1;

        if (objectName->length +1 < 
            evt.info.immnd.info.objModify.objectName.size)
        {
            evt.info.immnd.info.objModify.objectName.size = objectName->length + 1;
        }

        /*alloc-1*/
        evt.info.immnd.info.objModify.objectName.buf= 
            calloc(1, evt.info.immnd.info.objModify.objectName.size);
        strncpy(evt.info.immnd.info.objModify.objectName.buf, 
            (char *) objectName->value,
            evt.info.immnd.info.objModify.objectName.size);
        evt.info.immnd.info.objModify.objectName.buf[evt.info.immnd.info.objModify.objectName.size - 1] = '\0';
    }
    else
    {
        evt.info.immnd.info.objModify.objectName.size = 0;
        evt.info.immnd.info.objModify.objectName.buf = NULL;
    }

    assert(evt.info.immnd.info.objModify.attrMods == NULL);

    const SaImmAttrModificationT_2* attrMod;
    int i;
    for (i=0; attrMods[i]; ++i)
    {
        attrMod = attrMods[i];

        /* TODO Check that the user does not set values for System attributes.*/

        /*alloc-2*/
        IMMSV_ATTR_MODS_LIST* p = calloc(1, sizeof(IMMSV_ATTR_MODS_LIST));
        p->attrModType = attrMod->modType;
        p->attrValue.attrName.size = strlen(attrMod->modAttr.attrName) + 1;

        /* alloc 3*/
        p->attrValue.attrName.buf = malloc(p->attrValue.attrName.size);
        strncpy(p->attrValue.attrName.buf, attrMod->modAttr.attrName,
                p->attrValue.attrName.size);

        p->attrValue.attrValuesNumber = attrMod->modAttr.attrValuesNumber;
        p->attrValue.attrValueType = attrMod->modAttr.attrValueType;

        if (attrMod->modAttr.attrValuesNumber)
        { /*At least one value*/
            const SaImmAttrValueT* avarr = attrMod->modAttr.attrValues;
            /*alloc-4*/
            imma_copyAttrValue(&(p->attrValue.attrValue), attrMod->modAttr.attrValueType,
                          avarr[0]);

            if (attrMod->modAttr.attrValuesNumber > 1)
            { /*Multiple values*/
                unsigned int numAdded = attrMod->modAttr.attrValuesNumber - 1;
                unsigned int i;
                for (i=1; i <= numAdded; ++i)
                {
                    /*alloc-5*/
                    IMMSV_EDU_ATTR_VAL_LIST* al = 
                        calloc(1, sizeof(IMMSV_EDU_ATTR_VAL_LIST));
                    /*alloc-6*/
                    imma_copyAttrValue(&(al->n), attrMod->modAttr.attrValueType, avarr[i]);
                    al->next = p->attrValue.attrMoreValues; /*NULL initially*/
                    p->attrValue.attrMoreValues = al;
                } /*for*/
            }/*Multiple values*/
        } /*At least one value*/
        else
        {
            TRACE_1("Strange update of attribute %s, without any modifications",
                attrMod->modAttr.attrName);
        }
        p->next=evt.info.immnd.info.objModify.attrMods; /*NULL initially.*/
        evt.info.immnd.info.objModify.attrMods=p;
    }

    /* We do not send the rt update over fevs, because the update may
       often be a PURELY LOCAL update, by an object implementor reacting to
       the SaImmOiRtAttrUpdateCallbackT upcall. In that local case, the 
       return of the upcall is the signal that the new values are ready, not
       the invocation of this update call. */

    /* Release the CB lock Before MDS Send */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    locked = FALSE;

    /* IMMND GOES DOWN */
    if (FALSE == cb->is_immnd_up)
    {
        rc = SA_AIS_ERR_TRY_AGAIN;
        TRACE_2("IMMND is DOWN");
        goto mds_send_fail;
    }

    /* send the request to the IMMND */
    proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, 
                                     &evt, &out_evt, IMMSV_WAIT_TIME);

    TRACE("Rt objectUpdate send RETURNED:%u", rc);

    /* Error Handling */
    switch (proc_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            goto mds_send_fail;
        default:
            TRACE_1("MDS returned unexpected error code %u",
                   proc_rc);
            rc = SA_AIS_ERR_LIBRARY;
            goto mds_send_fail;
    }


    if (out_evt)
    {
        /* Process the outcome, note this is after a blocking call.*/
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

        /* Take the CB lock (do we really need the lock here ..?) */

        if (!locked)
        {
            if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("Lock failed");
                TRACE_LEAVE();
                return rc; /* NOTE: Leaking memory for this error case */
            }
            locked = TRUE;
        }

        /* Free the out event */
        /*Will free the root event only, not any pointer structures.*/
        free(out_evt);
    }

    mds_send_fail:
    /*We may be un-locked here but this should not matter.
      We are freing heap objects that should only be vissible from this
      thread. */

    if (evt.info.immnd.info.objModify.objectName.buf)
    { /*free-1*/
        free(evt.info.immnd.info.objModify.objectName.buf);
        evt.info.immnd.info.objModify.objectName.buf = NULL;
    }

    while (evt.info.immnd.info.objModify.attrMods)
    {

        IMMSV_ATTR_MODS_LIST* p = evt.info.immnd.info.objModify.attrMods;
        evt.info.immnd.info.objModify.attrMods = p->next;
        p->next=NULL;

        if (p->attrValue.attrName.buf)
        {
            free(p->attrValue.attrName.buf); /*free-3*/
            p->attrValue.attrName.buf=NULL;
        }

        if (p->attrValue.attrValuesNumber)
        {
            immsv_evt_free_att_val(&(p->attrValue.attrValue), /*free-4*/
                          p->attrValue.attrValueType);

            while (p->attrValue.attrMoreValues)
            {
                IMMSV_EDU_ATTR_VAL_LIST* al = p->attrValue.attrMoreValues;
                p->attrValue.attrMoreValues = al->next;
                al->next = NULL;
                immsv_evt_free_att_val(&(al->n), p->attrValue.attrValueType);/*free-6*/
                free(al); /*free-5*/
            }
        }

        free(p); /*free-2*/
    }
 client_not_found:
 stale_handle:
    if (locked)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    }

 lock_fail:   

    TRACE_LEAVE();
    return rc;
}


#ifdef IMM_A_01_01
extern SaAisErrorT saImmOiRtObjectCreate(SaImmOiHandleT immOiHandle,
                                         const SaImmClassNameT className,
                                         const SaNameT *parentName,
                                         const SaImmAttrValuesT **attrValues)
{
    return saImmOiRtObjectCreate_2(immOiHandle, className, parentName, 
                                   (const SaImmAttrValuesT_2 **) attrValues);
}
#endif

extern SaAisErrorT saImmOiRtObjectCreate_2(SaImmOiHandleT immOiHandle,
                                           const SaImmClassNameT className,
                                           const SaNameT *parentName,
                                           const SaImmAttrValuesT_2 **attrValues)
{
    SaAisErrorT     rc = SA_AIS_OK;
    uns32 proc_rc = NCSCC_RC_SUCCESS;
    IMMA_CB         *cb = &imma_cb;
    IMMSV_EVT        evt;
    IMMSV_EVT        *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL;
    NCS_BOOL       locked = FALSE;
    TRACE_ENTER();

    if (className == NULL)
    {
        TRACE_2("classname is NULL");
        TRACE_LEAVE();
        return SA_AIS_ERR_INVALID_PARAM;
    }

    if (parentName && parentName->length > SA_MAX_NAME_LENGTH)
    {
        return SA_AIS_ERR_NAME_TOO_LONG;
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }
    locked = TRUE;

    proc_rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node||cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("Non valid SaImmOiHandleT");
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    if(cl_node->mImplementerId == 0) {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("The SaImmOiHandleT is not associated with any implementer name");
        goto client_not_found;
    }

    /* Populate the Object-Create event */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_OBJ_CREATE;

    /* NOTE: should rename member adminOwnerId !!!*/
    evt.info.immnd.info.objCreate.adminOwnerId = cl_node->mImplementerId;
    evt.info.immnd.info.objCreate.className.size= strlen(className) +1;

    /*alloc-1*/
    evt.info.immnd.info.objCreate.className.buf= 
        malloc(evt.info.immnd.info.objCreate.className.size);
    strncpy(evt.info.immnd.info.objCreate.className.buf, className,
            evt.info.immnd.info.objCreate.className.size);

    if (parentName && parentName->length)
    {
        evt.info.immnd.info.objCreate.parentName.size =
            strlen((char *) parentName->value)+1;

        if (parentName->length +1 < 
            evt.info.immnd.info.objCreate.parentName.size)
        {
            evt.info.immnd.info.objCreate.parentName.size = parentName->length + 1;
        }

        /*alloc-2*/
        evt.info.immnd.info.objCreate.parentName.buf= 
            calloc(1, evt.info.immnd.info.objCreate.parentName.size);
        strncpy(evt.info.immnd.info.objCreate.parentName.buf, 
                (char *) parentName->value,
                evt.info.immnd.info.objCreate.parentName.size);
        evt.info.immnd.info.objCreate.parentName.buf[evt.info.immnd.info.objCreate.parentName.size - 1] = '\0';
    }
    else
    {
        evt.info.immnd.info.objCreate.parentName.size = 0;
        evt.info.immnd.info.objCreate.parentName.buf = NULL;
    }

    assert(evt.info.immnd.info.objCreate.attrValues == NULL);

    const SaImmAttrValuesT_2* attr;
    int i;
    for (i=0; attrValues[i]; ++i)
    {
        attr = attrValues[i];
        TRACE("attr:%s \n", attr->attrName);

        /*Check that the user does not set value for System attributes. */

        if (strcmp(attr->attrName, sysaClName)==0)
        {
            rc = SA_AIS_ERR_INVALID_PARAM;
            TRACE_2("Not allowed to set attribute %s ", sysaClName);
            goto mds_send_fail;
        }
        else if (strcmp(attr->attrName, sysaAdmName)==0)
        {
            rc = SA_AIS_ERR_INVALID_PARAM;
            TRACE_2("Not allowed to set attribute %s", sysaAdmName);
            goto mds_send_fail;
        }
        else if (strcmp(attr->attrName, sysaImplName)==0)
        {
            rc = SA_AIS_ERR_INVALID_PARAM;
            TRACE_2("Not allowed to set attribute %s", sysaImplName);
            goto mds_send_fail;
        }
        else if (attr->attrValuesNumber == 0)
        {
            TRACE("RtObjectCreate ignoring attribute %s with no values",
                    attr->attrName);
            continue;
        }

        /*alloc-3*/
        IMMSV_ATTR_VALUES_LIST* p = calloc(1, sizeof(IMMSV_ATTR_VALUES_LIST)); 

        p->n.attrName.size = strlen(attr->attrName) + 1;
        if (p->n.attrName.size >= SA_MAX_NAME_LENGTH)
        {
            TRACE_2("Attribute name too long");
            rc = SA_AIS_ERR_INVALID_PARAM;
            /*We leak a bit of memory here, but this is a user error case*/
            goto mds_send_fail;
        }

        /*alloc-4*/
        p->n.attrName.buf = malloc(p->n.attrName.size);
        strncpy(p->n.attrName.buf, attr->attrName, p->n.attrName.size);

        p->n.attrValuesNumber = attr->attrValuesNumber;
        p->n.attrValueType = attr->attrValueType;

        const SaImmAttrValueT* avarr = attr->attrValues;
        /*alloc-5*/
        imma_copyAttrValue(&(p->n.attrValue), attr->attrValueType, avarr[0]);

        if (attr->attrValuesNumber > 1)
        {
            unsigned int numAdded = attr->attrValuesNumber - 1;
            unsigned int i;
            for (i=1; i <= numAdded; ++i)
            {
                /*alloc-6*/
                IMMSV_EDU_ATTR_VAL_LIST* al = 
                    calloc(1, sizeof(IMMSV_EDU_ATTR_VAL_LIST)); 

                /*alloc-7*/
                imma_copyAttrValue(&(al->n), attr->attrValueType, avarr[i]);
                al->next = p->n.attrMoreValues;
                p->n.attrMoreValues = al;
            }
        }

        p->next=evt.info.immnd.info.objCreate.attrValues; /*NULL initially.*/
        evt.info.immnd.info.objCreate.attrValues=p;
    }

    rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle,
                           &locked, TRUE);

    TRACE("objectCreate send RETURNED:%u", rc);

    if (rc!= SA_AIS_OK)
    {
        goto mds_send_fail;
    }

    if (out_evt)
    {
        /* Process the outcome, note this is after a blocking call.*/
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

        /* Take the CB lock (do we really need the lock here ..?) */

        if (!locked)
        {
            if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("Lock failed");
                TRACE_LEAVE();
                return rc; /* NOTE: Leaking memory for this error case */
            }
            locked = TRUE;
        }

        /* Free the out event */
        /*Will free the root event only, not any pointer structures.*/
        free(out_evt);
    }

    mds_send_fail:
    /*We may be un-locked here but this should not matter.
      We are freing heap objects that should only be vissible from this
      thread. */

    if (evt.info.immnd.info.objCreate.className.buf)
    { /*free-1*/
        free(evt.info.immnd.info.objCreate.className.buf);
        evt.info.immnd.info.objCreate.className.buf = NULL;
    }

    if (evt.info.immnd.info.objCreate.parentName.buf)
    { /*free-2*/
        free(evt.info.immnd.info.objCreate.parentName.buf);
        evt.info.immnd.info.objCreate.parentName.buf = NULL;
    }

    while (evt.info.immnd.info.objCreate.attrValues)
    {
        IMMSV_ATTR_VALUES_LIST* p = 
            evt.info.immnd.info.objCreate.attrValues;
        evt.info.immnd.info.objCreate.attrValues = p->next;
        p->next = NULL;
        if (p->n.attrName.buf)
        { /*free-4*/
            free(p->n.attrName.buf);
            p->n.attrName.buf = NULL;
        }

        immsv_evt_free_att_val(&(p->n.attrValue), p->n.attrValueType); /*free-5*/

        while (p->n.attrMoreValues)
        {
            IMMSV_EDU_ATTR_VAL_LIST* al = p->n.attrMoreValues;
            p->n.attrMoreValues = al->next;
            al->next = NULL;
            immsv_evt_free_att_val(&(al->n), p->n.attrValueType); /*free-7*/
            free(al); /*free-6*/
        }

        p->next=NULL;
        free(p); /*free-3*/
    }


 stale_handle:
 client_not_found:
    if (locked)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    }

 lock_fail:   
    TRACE_LEAVE();
    return rc;
}

SaAisErrorT saImmOiRtObjectDelete(SaImmOiHandleT immOiHandle, const SaNameT *objectName)
{
    SaAisErrorT     rc = SA_AIS_OK;
    uns32 proc_rc = NCSCC_RC_SUCCESS;
    IMMA_CB         *cb = &imma_cb;
    IMMSV_EVT        evt;
    IMMSV_EVT        *out_evt=NULL;
    IMMA_CLIENT_NODE      *cl_node = NULL;
    NCS_BOOL       locked = FALSE;

    if (!objectName || (objectName->length == 0))
    {
        TRACE_2("Empty object-name");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    if(objectName->length > SA_MAX_NAME_LENGTH) {
        TRACE_2("Object name too long");
        return SA_AIS_ERR_INVALID_PARAM;        
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        rc = SA_AIS_ERR_LIBRARY;
        TRACE_1("Lock failed");
        goto lock_fail;
    }
    locked = TRUE;

    proc_rc = imma_client_node_get(&cb->client_tree, &immOiHandle, &cl_node);
    if (!cl_node || cl_node->isOm)
    {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("Not a valid SaImmOiHandleT");
        goto client_not_found;
    }

    if(cl_node->stale)
    {
        TRACE_2("Handle %llu is stale", immOiHandle);
        rc = SA_AIS_ERR_BAD_HANDLE; 
        goto stale_handle;
    }

    if(cl_node->mImplementerId == 0) {
        rc = SA_AIS_ERR_BAD_HANDLE;
        TRACE_2("The SaImmOiHandleT is not associated with any implementer name");
        goto client_not_found;
    }

    /* Populate the Object-Delete event */
    memset(&evt, 0, sizeof(IMMSV_EVT));
    evt.type = IMMSV_EVT_TYPE_IMMND;
    evt.info.immnd.type = IMMND_EVT_A2ND_OI_OBJ_DELETE;

    /* NOTE: should rename member adminOwnerId !!!*/
    evt.info.immnd.info.objDelete.adminOwnerId = cl_node->mImplementerId;

    evt.info.immnd.info.objDelete.objectName.size =
        strlen((char *) objectName->value)+1;

    if (objectName->length +1 < 
        evt.info.immnd.info.objDelete.objectName.size)
    {
        evt.info.immnd.info.objDelete.objectName.size = objectName->length + 1;
    }

    /*alloc-1*/
    evt.info.immnd.info.objDelete.objectName.buf= 
        calloc(1, evt.info.immnd.info.objDelete.objectName.size);
    strncpy(evt.info.immnd.info.objDelete.objectName.buf, 
            (char *) objectName->value,
            evt.info.immnd.info.objDelete.objectName.size);
    evt.info.immnd.info.objDelete.objectName.buf[evt.info.immnd.info.objDelete.objectName.size - 1] = '\0';

    rc = imma_evt_fake_evs(cb, &evt, &out_evt, IMMSV_WAIT_TIME, cl_node->handle,
                           &locked, TRUE);

    TRACE("objectDelete send RETURNED:%u", rc);

    if (rc!= SA_AIS_OK)
    {
        goto mds_send_fail;
    }

    if (out_evt)
    {
        /* Process the outcome, note this is after a blocking call.*/
        assert(out_evt->type == IMMSV_EVT_TYPE_IMMA);
        assert(out_evt->info.imma.type == IMMA_EVT_ND2A_IMM_ERROR);
        rc = out_evt->info.imma.info.errRsp.error;

        /* Take the CB lock (do we really need the lock here ..?) */

        if (!locked)
        {
            if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
            {
                rc = SA_AIS_ERR_LIBRARY;
                TRACE_1("Lock failed");
                return rc; /* NOTE leaking memory */
            }
            locked = TRUE;
        }
        free(out_evt); 
    }

 mds_send_fail:
    /*We may be un-locked here but this should not matter.
      We are freing heap objects that should only be vissible from this
      thread. */

    if (evt.info.immnd.info.objDelete.objectName.buf)
    { /*free-1*/
        free(evt.info.immnd.info.objDelete.objectName.buf);
        evt.info.immnd.info.objDelete.objectName.buf = NULL;
    }

 client_not_found:
 stale_handle:
    if (locked)
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
 lock_fail:   
    return rc;
}
