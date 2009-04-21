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
  
  This file contains the IMMA processing routines callback
  processing routines etc.
*****************************************************************************/

#include "imma.h"
#include "immsv_api.h"
#include <string.h>

/* For some reason I have to declare the strnlen function prototype.
   It does not help to include string.h */
size_t strnlen(const char *s, size_t maxlen);


static void imma_process_callback_info(IMMA_CB *cb, 
                                       IMMA_CLIENT_NODE *cl_node,
                                       IMMA_CALLBACK_INFO *callback);

static void imma_proc_free_callback(IMMA_CALLBACK_INFO *callback);

static int popAsyncAdmOpContinuation(IMMA_CB* cb,
    SaInt32T invocation,
    SaImmHandleT* immHandle,
    SaInvocationT* userInvoc);


/****************************************************************************
  Name          : imma_version_validate
 
  Description   : This routine Validates the received version
 
  Arguments     : SaVersionT *version - Version Info
 
  Return Values : SA_AIS_OK/SA_AIS<ERROR>
 
  Notes         : None
******************************************************************************/
/* NOTE ABT: Need to update this validation to the proper version. */
uns32 imma_version_validate (SaVersionT *version) 
{
    if ((version->releaseCode == IMMA_RELEASE_CODE) && 
        (version->majorVersion <= IMMA_MAJOR_VERSION))
    {
        version->majorVersion = IMMA_MAJOR_VERSION;
        version->minorVersion = IMMA_MINOR_VERSION;

#ifndef IMM_A_01_01
        if((version->releaseCode == 'A') &&
            (version->majorVersion == 0x01)) {
            return SA_AIS_ERR_VERSION;
        }
#endif
        return SA_AIS_OK;
    }
    else
    {
        TRACE_1("IMMA - Version Incompatible");

        /* Implementation is supporting the required release code */
        if (IMMA_RELEASE_CODE > version->releaseCode)
        {
            version->releaseCode = IMMA_RELEASE_CODE;
        }
        else if (IMMA_RELEASE_CODE < version->releaseCode)
        {
            version->releaseCode = IMMA_RELEASE_CODE;
        }
        version->majorVersion = IMMA_MAJOR_VERSION;
        version->minorVersion = IMMA_MINOR_VERSION;

        return SA_AIS_ERR_VERSION;
    }

    return SA_AIS_OK;
}

/****************************************************************************
  Name          : imma_callback_ipc_init
  
  Description   : This routine is used to initialize the queue for the callbacks.
 
  Arguments     : client_info - pointer to the client info
                   
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 imma_callback_ipc_init (IMMA_CLIENT_NODE  *client_info)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    if ((rc = m_NCS_IPC_CREATE(&client_info->callbk_mbx)) == NCSCC_RC_SUCCESS)
    {
        if (m_NCS_IPC_ATTACH(&client_info->callbk_mbx) == NCSCC_RC_SUCCESS)
        {
            return NCSCC_RC_SUCCESS;
        }
        m_NCS_IPC_RELEASE(&client_info->callbk_mbx, NULL);
        TRACE_1("Failed to initialize callback queue");
    }
    return rc;
}

/****************************************************************************
  Name          : imma_client_cleanup_mbx
  
  Description   : This routine is used to destroy the queue for the callbacks.
 
  Arguments     : cl_node - pointer to the client info
                   
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static NCS_BOOL imma_client_cleanup_mbx(NCSCONTEXT arg, NCSCONTEXT msg)
{
    IMMA_CALLBACK_INFO  *callback, *pnext;

    pnext = callback = (IMMA_CALLBACK_INFO*)msg;

    while (pnext)
    {
        pnext = callback->next;
        imma_proc_free_callback(callback);  
        callback = pnext;
    }

    return TRUE;
}


/****************************************************************************
  Name          : imma_callback_ipc_destroy
  
  Description   : This routine used to destroy the queue for the callbacks.
 
  Arguments     : client_info - pointer to the client info
                   
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
void imma_callback_ipc_destroy (IMMA_CLIENT_NODE  *cl_node)
{
    TRACE_ENTER();
    /* detach the mail box */
    m_NCS_IPC_DETACH(&cl_node->callbk_mbx, imma_client_cleanup_mbx, cl_node);

    /* delete the mailbox */
    m_NCS_IPC_RELEASE(&cl_node->callbk_mbx, NULL);
}

/****************************************************************************
  Name          : imma_finalize_proc
  
  Description   : This routine is used to process the finalize request at IMMA.
 
  Arguments     : cb - IMMA CB.
                  cl_node - pointer to the client info
                   
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 imma_finalize_proc(IMMA_CB *cb, IMMA_CLIENT_NODE *cl_node)
{
    SaImmAdminOwnerHandleT temp_hdl, *temp_ptr=NULL;
    IMMA_ADMIN_OWNER_NODE  *adm_node=NULL;

    temp_ptr = 0;

    /* Scan the entire Adm Owner DB and close the handles opened by client */
    while ((adm_node = (IMMA_ADMIN_OWNER_NODE *)
            ncs_patricia_tree_getnext(&cb->admin_owner_tree, (uns8*)temp_ptr)))
    {
        temp_hdl = adm_node->admin_owner_hdl;
        temp_ptr = &temp_hdl;

        if (adm_node->mImmHandle == cl_node->handle)
        {
            imma_admin_owner_node_delete(cb, adm_node);
        }
    }

    /* TODO ABT Close CCB handles and Search handles also!!
      We assume that the ND takes care of garbage collecting them on its side.
    */


    imma_callback_ipc_destroy(cl_node);

    imma_client_node_delete(cb, cl_node); /*Will free the node also*/

    return NCSCC_RC_SUCCESS;

}

/****************************************************************************
  Name          : entry point for SaImmOmAdminOperationInvokeAsyncCallbackT.
  Description   : This function will process the AdministartiveOperation
                  result up-call for the OM API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
  Notes         : None
******************************************************************************/
static void imma_proc_admin_op_async_rsp(IMMA_CB *cb, IMMA_EVT *evt)
{
    TRACE_ENTER();
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;


    SaImmHandleT immHandleCont;
    SaInvocationT userInvoc;
    ImmsvInvocation* invoc = (ImmsvInvocation *) 
                             &(evt->info.admOpRsp.invocation);
    /*NOTE: should get handle from immnd also and verify.*/
    if (!popAsyncAdmOpContinuation(cb, invoc->inv,
                                   &immHandleCont, &userInvoc))
    {
        TRACE_1("Missmatch on continuation for "
               "SaImmOmAdminOperationInvokeCallbackT");
        return;
    }

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return;
    }

    /* Get the Client info */
    rc = imma_client_node_get(&cb->client_tree, &immHandleCont, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Failed to find client node");
        return;
    }

    /* Allocate the Callback info */
    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        /* Fill the Call Back Info */   
        callback->type = IMMA_CALLBACK_OM_ADMIN_OP_RSP;
        callback->lcl_imm_hdl = immHandleCont;

        TRACE("Creating callback for async admop inv:%llu rslt:%u err:%u",
                userInvoc, evt->info.admOpRsp.result, evt->info.admOpRsp.error);

        callback->invocation = userInvoc;
        callback->sa_err = evt->info.admOpRsp.error;
        if (callback->sa_err == SA_AIS_OK)
        {
            callback->retval = evt->info.admOpRsp.result;
        }
        else
        {
            callback->retval = 
                SA_AIS_ERR_NO_SECTIONS; //Bogus result since error is set
        }

        /* Send the event */
        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
    }

    /* Release The Lock */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

    TRACE_LEAVE();
}

static SaImmAdminOperationParamsT_2** imma_proc_get_params(
                                                          IMMSV_ADMIN_OPERATION_PARAM* in_params)
{
    int noOfParams = 0;
    IMMSV_ADMIN_OPERATION_PARAM* p = in_params;
    size_t paramDataSize = 0;
    SaImmAdminOperationParamsT_2** out_params;
    int i=0;

    while (p)
    {
        ++noOfParams; 
        p = p->next;
    }

    paramDataSize = sizeof(SaImmAdminOperationParamsT_2*) * (noOfParams + 1);
    out_params = (SaImmAdminOperationParamsT_2**)
                 calloc(1, paramDataSize); /*alloc-1*/
    p = in_params;
    for (;i<noOfParams; i++)
    {
        IMMSV_ADMIN_OPERATION_PARAM* prev=p;
        out_params[i] = (SaImmAdminOperationParamsT_2*)
                        malloc(sizeof(SaImmAdminOperationParamsT_2));/*alloc-2*/
        out_params[i]->paramName = 
            malloc(p->paramName.size+1); /*alloc-3*/
        strncpy(out_params[i]->paramName, p->paramName.buf, 
                p->paramName.size+1);
        out_params[i]->paramName[p->paramName.size]=0;/*string too long=>truncate*/
        free(p->paramName.buf);
        p->paramName.buf=NULL;
        p->paramName.size=0;
        out_params[i]->paramType = (SaImmValueTypeT) p->paramType;
        out_params[i]->paramBuffer = imma_copyAttrValue3(p->paramType, /*alloc-4*/
            &(p->paramBuffer));
        immsv_evt_free_att_val(&(p->paramBuffer),  p->paramType);
        p=p->next;
        prev->next=NULL;
        free(prev); 
    }
    return(SaImmAdminOperationParamsT_2**) out_params;
}

/****************************************************************************
  Name          : imma_proc_admop
  Description   : This function will process the AdministartiveOperation
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
  Notes         : None
******************************************************************************/
static void imma_proc_admop(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;

    /*TODO: correct this, ugly use of continuationId*/
    SaImmOiHandleT implHandle = evt->info.admOpReq.continuationId;

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return;
    }

    /* Get the Client info */
    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Failed to find client node");
        return;
    }

    /* Allocate the Callback info */
    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        /* Fill the Call Back Info */   
        callback->type = IMMA_CALLBACK_OM_ADMIN_OP;
        callback->lcl_imm_hdl = implHandle;

        ImmsvInvocation invoc; 
        invoc.inv = evt->info.admOpReq.invocation;
        invoc.owner = evt->info.admOpReq.adminOwnerId;
        SaInvocationT saInv = *((SaInvocationT *) &invoc);
        callback->invocation = saInv;

        callback->name.length = strnlen(evt->info.admOpReq.objectName.buf,
            evt->info.admOpReq.objectName.size);
        assert(callback->name.length <= SA_MAX_NAME_LENGTH);
        memcpy((char *) callback->name.value, 
                evt->info.admOpReq.objectName.buf,
                callback->name.length);
        free(evt->info.admOpReq.objectName.buf);
        evt->info.admOpReq.objectName.buf = NULL;
        evt->info.admOpReq.objectName.size = 0;
        callback->operationId =  evt->info.admOpReq.operationId;
        callback->params = imma_proc_get_params(evt->info.admOpReq.params);
        evt->info.admOpReq.params=NULL;
        /* Send the event */
        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
    }

    /* Release The Lock */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
}


void imma_proc_stale_dispatch(IMMA_CB *cb, IMMA_CLIENT_NODE* cl_node)
{
    /* We are locked already */
    IMMA_CALLBACK_INFO *callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    assert(callback);


    callback->type = IMMA_CALLBACK_STALE_HANDLE;

    /*assert(*/
    m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
        NCS_IPC_PRIORITY_HIGH)/* == NCSCC_RC_SUCCESS)*/;
}

/****************************************************************************
  Name          : imma_proc_rt_attr_update
  Description   : This function will generate the SaImmOiRtAttrUpdateCallbackT
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
  Notes         : None
******************************************************************************/
static void imma_proc_rt_attr_update(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;

    /* NOTE: correct this, ugly use of continuationId*/
    SaImmOiHandleT implHandle = evt->info.searchRemote.client_hdl;
    TRACE_ENTER();

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        TRACE_LEAVE();
        return;
    }

    /* Get the Client info */
    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Failed to find client node");
        TRACE_LEAVE();
        return;
    }

    /* Allocate the Callback info */
    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        /* Fill the Call Back Info */   
        callback->type = IMMA_CALLBACK_OI_RT_ATTR_UPDATE;
        callback->lcl_imm_hdl = implHandle;

        callback->name.length = strnlen(evt->info.searchRemote.objectName.buf, 
            evt->info.searchRemote.objectName.size);
        assert(callback->name.length <= SA_MAX_NAME_LENGTH);
        memcpy((char *) callback->name.value, 
                evt->info.searchRemote.objectName.buf, 
            callback->name.length);
        free(evt->info.searchRemote.objectName.buf);
        evt->info.searchRemote.objectName.buf = NULL;
        evt->info.searchRemote.objectName.size = 0;

        /* steal the attributeNames list. */
        callback->attrNames =  evt->info.searchRemote.attributeNames;
        evt->info.searchRemote.attributeNames = NULL;

        ImmsvInvocation invoc; 
        invoc.inv = evt->info.searchRemote.searchId;
        invoc.owner = evt->info.searchRemote.remoteNodeId;
        SaInvocationT saInv = *((SaInvocationT *) &invoc);
        callback->invocation = saInv;

        callback->requestNodeId = evt->info.searchRemote.requestNodeId;

        /* Send the event */
        TRACE("Posting RT UPDATE CALLBACK to IPC mailbox");
        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
    }

    /* Release The Lock */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    TRACE_LEAVE();
}

/****************************************************************************
  Name          : imma_proc_ccb_completed
  Description   : This function will process the ccb completed
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
  Notes         : None
******************************************************************************/
static void imma_proc_ccb_completed(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;

    SaImmOiHandleT implHandle = evt->info.ccbCompl.immHandle;

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return;
    }

    /* Get the Client info */
    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Could not find client node");
        return;
    }

    /* Allocate the Callback info */
    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        /* Fill the Call Back Info */   
        callback->type = IMMA_CALLBACK_OI_CCB_COMPLETED;
        callback->lcl_imm_hdl = implHandle;
        callback->ccbID = evt->info.ccbCompl.ccbId;
        callback->implId = evt->info.ccbCompl.implId;
        callback->inv = evt->info.ccbCompl.invocation;

        /* Send the event */
        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
        TRACE("Posted IMMA_CALLBACK_OI_CCB_COMPLETED");
    }

    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
}

/****************************************************************************
  Name          : imma_proc_ccb_apply
  Description   : This function will process the ccb apply
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
  Notes         : None
******************************************************************************/
static void imma_proc_ccb_apply(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;

    SaImmOiHandleT implHandle = evt->info.ccbCompl.immHandle;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return;
    }

    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Could not find client node");
        return;
    }

    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        callback->type = IMMA_CALLBACK_OI_CCB_APPLY;
        callback->lcl_imm_hdl = implHandle;
        callback->ccbID = evt->info.ccbCompl.ccbId;
        callback->implId = evt->info.ccbCompl.implId;
        callback->inv = evt->info.ccbCompl.invocation;

        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
        TRACE("Posted IMMA_CALLBACK_OI_CCB_APPLY");
    }

    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
}

/****************************************************************************
  Name          : imma_proc_ccb_abort
  Description   : This function will process the ccb abort
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
  Notes         : None
******************************************************************************/
static void imma_proc_ccb_abort(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;

    SaImmOiHandleT implHandle = evt->info.ccbCompl.immHandle;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return;
    }

    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Could not find client node");
        return;
    }

    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        callback->type = IMMA_CALLBACK_OI_CCB_ABORT;
        callback->lcl_imm_hdl = implHandle;
        callback->ccbID = evt->info.ccbCompl.ccbId;
        callback->implId = evt->info.ccbCompl.implId;

        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
        TRACE("Posted IMMA_CALLBACK_OI_CCB_ABORT");
    }

    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
}

/****************************************************************************
  Name          : imma_proc_obj_delete
  Description   : This function will process the Object Delete
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
******************************************************************************/
static void imma_proc_obj_delete(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE    *cl_node = NULL;

    SaImmOiHandleT implHandle = evt->info.objDelete.immHandle;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return;
    }

    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Could not find client node");
        return;
    }

    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        callback->type = IMMA_CALLBACK_OI_CCB_DELETE;
        callback->lcl_imm_hdl = implHandle;
        callback->ccbID = evt->info.objDelete.ccbId;
        callback->inv = evt->info.objDelete.adminOwnerId; /*ugly*/
        callback->name.length = strnlen(evt->info.objDelete.objectName.buf,
            evt->info.objDelete.objectName.size);
        assert(callback->name.length <= SA_MAX_NAME_LENGTH);
        memcpy((char *) callback->name.value,  
            evt->info.objDelete.objectName.buf, callback->name.length);
        free(evt->info.objDelete.objectName.buf);
        evt->info.objDelete.objectName.buf=NULL;
        evt->info.objDelete.objectName.size=0;

        /* Send the event */
        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
        TRACE("Posted IMMA_CALLBACK_OI_CCB_DELETE");
    }

    /* Release The Lock */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
}

/****************************************************************************
  Name          : imma_proc_obj_create
  Description   : This function will process the Object Create
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
******************************************************************************/
static void imma_proc_obj_create(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;

    SaImmOiHandleT implHandle = evt->info.objCreate.immHandle;

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return;
    }

    /* Get the Client info */
    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Could not find client node");
        return;
    }

    /* Allocate the Callback info */
    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        /* Fill the Call Back Info */   
        callback->type = IMMA_CALLBACK_OI_CCB_CREATE;
        callback->lcl_imm_hdl = implHandle;
        callback->ccbID = evt->info.objCreate.ccbId;
        callback->inv = evt->info.objCreate.adminOwnerId;/*Actually continuationId*/

        callback->name.length = strnlen(evt->info.objCreate.parentName.buf,
            evt->info.objCreate.parentName.size);
        assert(callback->name.length <= SA_MAX_NAME_LENGTH);
        memcpy((char *) callback->name.value, 
                evt->info.objCreate.parentName.buf,
                callback->name.length);
        free(evt->info.objCreate.parentName.buf);
        evt->info.objCreate.parentName.buf = NULL;
        evt->info.objCreate.parentName.size = 0;

        assert(strlen(evt->info.objCreate.className.buf) <=
               evt->info.objCreate.className.size);
        callback->className = evt->info.objCreate.className.buf;
        evt->info.objCreate.className.buf = NULL; /*steal the string buffer*/
        evt->info.objCreate.className.size = 0;

        callback->attrValues = evt->info.objCreate.attrValues;
        evt->info.objCreate.attrValues = NULL; /*steal attrValues list*/


        /* Send the event */
        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
        TRACE("Posted IMMA_CALLBACK_OI_CCB_CREATE");
    }

    /* Release The Lock */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
}

/****************************************************************************
  Name          : imma_proc_obj_modify
  Description   : This function will process the Object Modify
                  up-call for the Object Implementer API.
  Arguments     : cb - IMMA CB.
                  evt - IMMA_EVT.
  Return Values : None
******************************************************************************/
static void imma_proc_obj_modify(IMMA_CB *cb, IMMA_EVT *evt)
{
    uns32        proc_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT  rc = SA_AIS_OK;
    IMMA_CALLBACK_INFO  *callback;
    IMMA_CLIENT_NODE      *cl_node = NULL;
    TRACE_ENTER();

    SaImmOiHandleT implHandle = evt->info.objModify.immHandle;

    /* get the CB Lock*/
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        TRACE_LEAVE();
        return;
    }

    /* Get the Client info */
    rc = imma_client_node_get(&cb->client_tree, &implHandle, &cl_node);
    if (!cl_node)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        TRACE_1("Could not find client node");
        TRACE_LEAVE();
        return;
    }

    /* Allocate the Callback info */
    callback = calloc(1, sizeof(IMMA_CALLBACK_INFO));
    if (!callback)
    {
        proc_rc = NCSCC_RC_OUT_OF_MEM;
    }
    else
    {
        /* Fill the Call Back Info */   
        callback->type = IMMA_CALLBACK_OI_CCB_MODIFY;
        callback->lcl_imm_hdl = implHandle;
        callback->ccbID = evt->info.objModify.ccbId;
        callback->inv = evt->info.objModify.adminOwnerId;
        /*Actually continuationId*/

        callback->name.length = strnlen(evt->info.objModify.objectName.buf,
            evt->info.objModify.objectName.size);
        assert(callback->name.length <= SA_MAX_NAME_LENGTH);
        memcpy((char *) callback->name.value, 
                evt->info.objModify.objectName.buf,
                callback->name.length);
        free(evt->info.objModify.objectName.buf);
        evt->info.objModify.objectName.buf = NULL;
        evt->info.objModify.objectName.size = 0;

        callback->attrMods = evt->info.objModify.attrMods;
        evt->info.objModify.attrMods = NULL; /*steal attrMods list*/


        /* Send the event */
        TRACE("Posting IMMA_CALLBACK_OI_CCB_MODIFY");
        proc_rc = m_NCS_IPC_SEND(&cl_node->callbk_mbx, callback, 
                                 NCS_IPC_PRIORITY_NORMAL);
        TRACE("IMMA_CALLBACK_OI_CCB_MODIFY Posted");
    }

    /* Release The Lock */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    TRACE_LEAVE();
}

void imma_proc_free_pointers(IMMA_CB *cb, IMMA_EVT *evt)
{
    TRACE_ENTER();
    switch (evt->type)
    {
        case IMMA_EVT_ND2A_IMM_ADMOP:
            /*TODO See TODO 12345 code repeated (almost) in imma_om_api.c
              free-1*/
            if(evt->info.admOpReq.objectName.size) {
                free(evt->info.admOpReq.objectName.buf);
                evt->info.admOpReq.objectName.buf = NULL;
                evt->info.admOpReq.objectName.size = 0;
            }
            while(evt->info.admOpReq.params) {
                IMMSV_ADMIN_OPERATION_PARAM* p = evt->info.admOpReq.params;
                evt->info.admOpReq.params = p->next;

                if(p->paramName.buf) { /*free-3*/
                    free(p->paramName.buf);
                    p->paramName.buf = NULL;
                    p->paramName.size = 0;
                }
                immsv_evt_free_att_val(&(p->paramBuffer), p->paramType); /*free-4*/
                p->next=NULL;
                free(p); /*free-2*/
            }
            break;

        case IMMA_EVT_ND2A_ADMOP_RSP: 
            break;

        case IMMA_EVT_ND2A_SEARCH_REMOTE:
            free(evt->info.searchRemote.objectName.buf);
            evt->info.searchRemote.objectName.buf=NULL;
            evt->info.searchRemote.objectName.size=0;
            immsv_evt_free_attrNames(evt->info.searchRemote.attributeNames);
            evt->info.searchRemote.attributeNames = NULL;
            break;

        case IMMA_EVT_ND2A_OI_OBJ_CREATE_UC:
            free(evt->info.objCreate.className.buf);
            evt->info.objCreate.className.buf = NULL;
            evt->info.objCreate.className.size = 0;

            free(evt->info.objCreate.parentName.buf);
            evt->info.objCreate.parentName.buf = NULL;
            evt->info.objCreate.parentName.size = 0;

            immsv_free_attrvalues_list(evt->info.objCreate.attrValues);
            evt->info.objCreate.attrValues = NULL;
            break;

        case IMMA_EVT_ND2A_OI_OBJ_DELETE_UC:
            free(evt->info.objDelete.objectName.buf);
            evt->info.objDelete.objectName.buf = NULL;
            evt->info.objDelete.objectName.size = 0;
            break;

        case IMMA_EVT_ND2A_OI_OBJ_MODIFY_UC:
            free(evt->info.objModify.objectName.buf);
            evt->info.objModify.objectName.buf = NULL;
            evt->info.objModify.objectName.size = 0;

            immsv_free_attrmods(evt->info.objModify.attrMods);
            evt->info.objModify.attrMods = NULL;
            break;

        case IMMA_EVT_ND2A_OI_CCB_COMPLETED_UC:
        case IMMA_EVT_ND2A_OI_CCB_APPLY_UC:
        case IMMA_EVT_ND2A_OI_CCB_ABORT_UC:
            break;

        default:
            TRACE("Unknown event type %u", evt->type);
            break;
    }
    TRACE_LEAVE();
}

/****************************************************************************
  Name          : imma_process_evt
  Description   : This routine will process the callback event received from
                  IMMND.
  Arguments     : cb - IMMA CB.
                  evt - IMMSV_EVT.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : None
******************************************************************************/
void imma_process_evt(IMMA_CB *cb, IMMSV_EVT *evt)
{
    TRACE("** Event type:%u", evt->info.imma.type);
    switch (evt->info.imma.type)
    {
        case IMMA_EVT_ND2A_IMM_ADMOP:
            imma_proc_admop(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_ADMOP_RSP: 
            imma_proc_admin_op_async_rsp(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_SEARCH_REMOTE:
            imma_proc_rt_attr_update(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_OI_OBJ_CREATE_UC:
            imma_proc_obj_create(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_OI_OBJ_DELETE_UC:
            imma_proc_obj_delete(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_OI_OBJ_MODIFY_UC:
            imma_proc_obj_modify(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_OI_CCB_COMPLETED_UC:
            imma_proc_ccb_completed(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_OI_CCB_APPLY_UC:
            imma_proc_ccb_apply(cb, &evt->info.imma);
            break;

        case IMMA_EVT_ND2A_OI_CCB_ABORT_UC:
            imma_proc_ccb_abort(cb, &evt->info.imma);
            break;

        default:
            TRACE("Unknown event type %u", evt->info.imma.type);
            break;
    }
    imma_proc_free_pointers(cb, &evt->info.imma);
    return;
}


/****************************************************************************
  Name          : imma_callback_ipc_rcv
 
  Description   : This routine is used Receive the message posted to callback
                  MBX.
 
  Return Values : pointer to the callback
 
  Notes         : None
******************************************************************************/
IMMA_CALLBACK_INFO *imma_callback_ipc_rcv (IMMA_CLIENT_NODE  *cl_node)
{
    IMMA_CALLBACK_INFO *cb_info=NULL;

    /* remove it to the queue */
    cb_info =  (IMMA_CALLBACK_INFO*)
               m_NCS_IPC_NON_BLK_RECEIVE(&cl_node->callbk_mbx,NULL);

    return cb_info;
}

/****************************************************************************
  Name          : imma_hdl_callbk_dispatch_one
 
  Description   : This routine dispatches one pending callback.
 
  Arguments     : cb      - ptr to the IMMA control block
                  immHandle - IMM OM service handle
                  SaBoolT  - SA_TRUE => OM, SA_FALSE => OI
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 imma_hdl_callbk_dispatch_one (IMMA_CB *cb, SaImmHandleT immHandle)
{
    IMMA_CALLBACK_INFO *callback=NULL;
    IMMA_CLIENT_NODE    *cl_node=NULL;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return SA_AIS_ERR_LIBRARY;
    }

    imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);

    if (cl_node == NULL)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        return SA_AIS_ERR_BAD_HANDLE;
    }

    /* get it from the queue */
    while ((callback = imma_callback_ipc_rcv(cl_node)))
    {
        imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
        if (cl_node)
        {
            
            m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
            if(cl_node->stale) {return SA_AIS_ERR_BAD_HANDLE;}

            imma_process_callback_info(cb, cl_node, callback);
            return SA_AIS_OK;
        }
        else
        {
            imma_proc_free_callback(callback);
            continue;
        }
    }

    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    return SA_AIS_OK;
}

/****************************************************************************
  Name          : imma_hdl_callbk_dispatch_all
 
  Description   : This routine dispatches all pending callback.
 
  Arguments     : cb      - ptr to the IMMA control block
                  immHandle - IMM OM service handle
                  SaBoolT - isOm  SA_TRUE => OM, SA_FALSE => OI
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 imma_hdl_callbk_dispatch_all(IMMA_CB *cb, SaImmHandleT immHandle)
{
    IMMA_CALLBACK_INFO   *callback=NULL;
    IMMA_CLIENT_NODE     *cl_node=NULL;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return SA_AIS_ERR_LIBRARY;
    }

    imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);

    if (cl_node == NULL)
    {
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        return SA_AIS_ERR_BAD_HANDLE;
    }

    while ((callback = imma_callback_ipc_rcv(cl_node)))
    {
        imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);
        if (cl_node)
        {
            m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
            if(cl_node->stale) {return SA_AIS_ERR_BAD_HANDLE;} 

            imma_process_callback_info(cb, cl_node, callback);
        }
        else
        {
            imma_proc_free_callback(callback);
            break;
        } 

        if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
        {
            TRACE_1("Lock failure");
            return SA_AIS_ERR_LIBRARY;
        }


        /* Is this a way of detecting closed handle => terminate ?*/
        imma_client_node_get(&cb->client_tree, &immHandle, &cl_node);

        if (cl_node == NULL)
        {
            m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
            return SA_AIS_ERR_BAD_HANDLE;
        }
    }

    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    return SA_AIS_OK;
}

/****************************************************************************
  Name          : imma_hdl_callbk_dispatch_block
 
  Description   : This routine dispatches all pending callback and blocks
                  when there are no more
  Arguments     : cb      - ptr to the IMMA control block
                  immHandle - immsv handle
                  isOm      - SA_TRUE => OM, SA_FALSE => OI.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 imma_hdl_callbk_dispatch_block(IMMA_CB  *cb, SaImmHandleT immHandle)
{
    IMMA_CALLBACK_INFO   *callback=NULL;
    SYSF_MBX            *callbk_mbx=NULL;
    IMMA_CLIENT_NODE     *client_info=NULL;

    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Lock failure");
        return SA_AIS_ERR_LIBRARY;
    }

    imma_client_node_get(&cb->client_tree, &immHandle, &client_info);

    if (client_info == NULL)
    {
        /* Another thread called Finalize */
        m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
        return SA_AIS_ERR_BAD_HANDLE;
    }

    callbk_mbx=&(client_info->callbk_mbx);

    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

    callback = (IMMA_CALLBACK_INFO*)m_NCS_IPC_RECEIVE(callbk_mbx,NULL);
    while (1)
    {
        /* Take the CB Lock */
        if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
        {
            TRACE_1("Lock failure");
            return SA_AIS_ERR_LIBRARY;
        }

        imma_client_node_get(&cb->client_tree, &immHandle, &client_info);

        if (callback)
        {
            if (client_info)
            {
                m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
                if(client_info->stale) {return SA_AIS_ERR_BAD_HANDLE;} 

                imma_process_callback_info(cb, client_info, callback);
            }
            else
            {
                /* Another thread called Finalize? */
                TRACE("Client dead?");

                imma_proc_free_callback(callback);
                m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
                return SA_AIS_OK;
            }
        }
        else
        {
            m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
            return SA_AIS_ERR_LIBRARY;
        }

        callback = (IMMA_CALLBACK_INFO*)m_NCS_IPC_RECEIVE(callbk_mbx,NULL);
    }

    return SA_AIS_OK; 
}

static int popAsyncAdmOpContinuation(IMMA_CB* cb,            //in
    SaInt32T invocation,   //in
    SaImmHandleT* immHandle, //out
    SaInvocationT* userInvoc)//out
{
    IMMA_CONTINUATION_RECORD* cr = cb->imma_continuations;
    IMMA_CONTINUATION_RECORD** prevCr = &(cb->imma_continuations);

    TRACE("POP continuation %i", invocation);

    while (cr)
    {
        if (cr->invocation == invocation)
        {
            *immHandle = cr->immHandle;
            *userInvoc = cr->userInvoc;
            *prevCr = cr->next;
            cr->next=NULL;
            free(cr);
            return 1;
        }

        prevCr = &(cr->next);
        cr = cr->next;
    }
    TRACE("POP continuation %i not found", invocation);
    return 0;
}


/****************************************************************************
  Name          : imma_process_callback_info
 
  Description   : This routine invokes the registered callback routine.
 
  Arguments     : cb  - ptr to the IMMA control block
                  cl_node - Client Node
                  callback - ptr to the registered callbacks
                  isOm - SA_TRUE => OM, SA_FALSE => OI
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
static void imma_process_callback_info (IMMA_CB *cb, IMMA_CLIENT_NODE *cl_node,
                                        IMMA_CALLBACK_INFO *callback)
{
    /* No locking */
    TRACE_ENTER();
    /* invoke the corresponding callback */
#ifdef IMMA_OM
    switch (callback->type)
    {
        case IMMA_CALLBACK_OM_ADMIN_OP_RSP:  /*Async reply via OM.*/
            if (cl_node->o.mCallbk.saImmOmAdminOperationInvokeCallback)
            {
                TRACE("Upcall for callback for async admop inv:%llu rslt:%u err:%u",
                      callback->invocation,
                      callback->retval, 
                      callback->sa_err);        

                cl_node->o.mCallbk.saImmOmAdminOperationInvokeCallback(
                                                                      callback->invocation,
                                                                      callback->retval, 
                                                                      callback->sa_err);
            }
            else
            {
                TRACE_1("No callback to deliver AdminOperationInvokeAsync - invoc:%llu ret:%u err:%u",
                        callback->invocation, callback->retval, callback->sa_err);
            }

            break;

        case IMMA_CALLBACK_STALE_HANDLE:
            TRACE("Stale handle indication received");
            break;
        default:
            TRACE_1("Unrecognized OM callback type:%u", callback->type);
            break;
    }
#endif

#ifdef IMMA_OI
    switch (callback->type)
    {
        case IMMA_CALLBACK_OM_ADMIN_OP: 
#ifdef IMM_A_01_01
            TRACE("Admin op callback isOiA1:%u, iCallbk1:%p iCallbk:%p",
                  cl_node->isOiA1,
                  cl_node->o.iCallbk1.saImmOiAdminOperationCallback,
                  cl_node->o.iCallbk.saImmOiAdminOperationCallback);

            if (cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiAdminOperationCallback)
            {
                cl_node->o.iCallbk1.saImmOiAdminOperationCallback(
                                                                 callback->lcl_imm_hdl,
                                                                 callback->invocation,
                                                                 &(callback->name),
                                                                 callback->operationId,
                                                                 (const SaImmAdminOperationParamsT**) callback->params);

            }
            else if (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiAdminOperationCallback)
            {
#else
                if (cl_node->o.iCallbk.saImmOiAdminOperationCallback)
            {
#endif
                cl_node->o.iCallbk.saImmOiAdminOperationCallback(
                                                                callback->lcl_imm_hdl,
                                                                callback->invocation,
                                                                &(callback->name),
                                                                callback->operationId,
                                                                (const SaImmAdminOperationParamsT_2**) callback->params);

            }
            else
            {
                /*No callback registered for admin-op!!*/
                SaAisErrorT localErr = 
                    saImmOiAdminOperationResult(callback->lcl_imm_hdl,
                                                callback->invocation,
                                                SA_AIS_ERR_FAILED_OPERATION);
                if (localErr == SA_AIS_OK)
                {
                    TRACE("Object %s has implementer but "
                          "saImmOiAdminOperationCallback is set to NULL", 
                          callback->name.value);
                }
                else
                {
                    TRACE("Object %s has implementer but "
                          "saImmOiAdminOperationCallback is set to NULL "
                          "and could not send error result, error: %u",
                          callback->name.value, localErr);
                }
            }
            break;

        case IMMA_CALLBACK_OI_CCB_COMPLETED:
            TRACE("ccb-completed op callback");
            do
            {
                SaAisErrorT localEr = SA_AIS_OK;
                IMMSV_EVT   ccbCompletedRpl;
                NCS_BOOL locked=FALSE;
#ifdef IMM_A_01_01
                if ((cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiCcbCompletedCallback) ||
                    (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiCcbCompletedCallback))
#else
                if (cl_node->o.iCallbk.saImmOiCcbCompletedCallback)
#endif
                {
                    SaImmOiCcbIdT ccbid = callback->ccbID;
#ifdef IMM_A_01_01
                    if (cl_node->isOiA1)
                        localEr = cl_node->o.iCallbk1.saImmOiCcbCompletedCallback(
                                                                                 callback->lcl_imm_hdl,
                                                                                 ccbid);
                    else
#endif
                        localEr = cl_node->o.iCallbk.saImmOiCcbCompletedCallback(
                                                                                callback->lcl_imm_hdl,
                                                                                ccbid);
                    if (!(localEr==SA_AIS_OK ||
                          localEr==SA_AIS_ERR_NO_MEMORY ||
                          localEr==SA_AIS_ERR_NO_RESOURCES ||
                          localEr==SA_AIS_ERR_BAD_OPERATION))
                    {
                        TRACE_2("Illegal return value from "
                                "saImmOiCcbCompletedCallback %u. "
                                "Allowed are %u %u %u %u", localEr, SA_AIS_OK,
                                SA_AIS_ERR_NO_MEMORY, SA_AIS_ERR_NO_RESOURCES,
                                SA_AIS_ERR_BAD_OPERATION);
                        localEr=SA_AIS_ERR_FAILED_OPERATION;
                    }

                }
                else
                {
                    /* No callback function registered for completed upcall.
                       The standard is not clear on how this case should be handled.
                       We take the strict approach of demanding that, if there is
                       a registered implementer, then that implementer must 
                       implement the completed callback, for the callback to succeed
                    */
                    TRACE_2("saImmOiCcbCompletedCallback is not implemented, yet "
                            "implementer is registered and CCBs are used. "
                            "Ccb will fail");
                    localEr=SA_AIS_ERR_FAILED_OPERATION;
                }

                memset(&ccbCompletedRpl, 0, sizeof(IMMSV_EVT));
                ccbCompletedRpl.type = IMMSV_EVT_TYPE_IMMND;
                ccbCompletedRpl.info.immnd.type = IMMND_EVT_A2ND_CCB_COMPLETED_RSP;
                ccbCompletedRpl.info.immnd.info.ccbUpcallRsp.result = localEr;
                ccbCompletedRpl.info.immnd.info.ccbUpcallRsp.oi_client_hdl = 
                    callback->lcl_imm_hdl;
                ccbCompletedRpl.info.immnd.info.ccbUpcallRsp.ccbId = callback->ccbID;
                ccbCompletedRpl.info.immnd.info.ccbUpcallRsp.implId = 
                    callback->implId;
                ccbCompletedRpl.info.immnd.info.ccbUpcallRsp.inv = callback->inv;

                assert(m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) == NCSCC_RC_SUCCESS);
                locked = TRUE;
                /*async  fevs*/
                localEr = imma_evt_fake_evs(cb, &ccbCompletedRpl, NULL, 0,
                                            cl_node->handle, &locked, FALSE);

                if (locked)
                {
                    assert(m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE) == 
                           NCSCC_RC_SUCCESS);
                    locked = FALSE;
                }
                if (localEr != NCSCC_RC_SUCCESS)
                {
                    /*Cant do anything but log error and drop this reply.*/
                    TRACE_1("CcbCompletedCallback: send reply to IMMND failed");
                }
            } while (0);

            break;

        case IMMA_CALLBACK_OI_CCB_APPLY:
            TRACE("ccb-apply op callback");
            do
            {
#ifdef IMM_A_01_01
                if ((cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiCcbApplyCallback) ||
                    (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiCcbApplyCallback))
#else
                if (cl_node->o.iCallbk.saImmOiCcbApplyCallback)
#endif
                {
                    /* Anoying type diff for ccbid between OM and OI */
                    SaImmOiCcbIdT ccbid = callback->ccbID; 
#ifdef IMM_A_01_01
                    if (cl_node->isOiA1)
                        cl_node->o.iCallbk1.saImmOiCcbApplyCallback(callback->lcl_imm_hdl,ccbid);
                    else
#endif
                        cl_node->o.iCallbk.saImmOiCcbApplyCallback(callback->lcl_imm_hdl, ccbid);
                }
                else
                {
                    /* No callback function registered for apply upcall.
                       There is nothign we can do about this since the CCB is
                       commited already. It also makes sense that some applications
                       may want to ignore the apply upcall.
                    */
                    TRACE_2("saImmOiCcbApplyCallback is not implemented, yet "
                            "implementer is registered and CCBs are used. "
                            "Ccb will commit in any case");
                }
            } while (0);

            break;

        case IMMA_CALLBACK_OI_CCB_CREATE:
            TRACE("ccb-object-create op callback");
            do
            {
                SaAisErrorT localEr = SA_AIS_OK;
                IMMSV_EVT  ccbObjCrRpl;
                NCS_BOOL locked=FALSE;
                SaImmAttrValuesT_2** attr = NULL;
                size_t attrDataSize = 0;
                int i=0;
#ifdef A_01_01
                if ((cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiCcbObjectCreateCallback) ||
                    (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiCcbObjectCreateCallbac))
#else
                if (cl_node->o.iCallbk.saImmOiCcbObjectCreateCallback)
#endif
                {
                    /* Anoying type diff for ccbid between OM and OI */
                    SaImmOiCcbIdT ccbid = callback->ccbID; 

                    SaNameT parentName = callback->name;
                    const SaImmClassNameT className = callback->className; /*0*/
                    callback->className = NULL; 
                    int noOfAttributes =0;

                    /* NOTE: The code below is practically a copy of the code
                      in immOm searchNext, for serving the attrValues structure.
                      This code should be factored out into some common function.
                    */
                    IMMSV_ATTR_VALUES_LIST* p = callback->attrValues;
                    while (p)
                    {
                        ++noOfAttributes; 
                        p = p->next;
                    }

                    attrDataSize = sizeof(SaImmAttrValuesT_2*) * (noOfAttributes+1);
                    attr = calloc(1, attrDataSize); /*alloc-1*/
                    p = callback->attrValues;
                    for (;i<noOfAttributes; i++, p = p->next)
                    {
                        IMMSV_ATTR_VALUES* q = &(p->n);
                        attr[i] = calloc(1, sizeof(SaImmAttrValuesT_2)); /*alloc-2*/
                        attr[i]->attrName = malloc(q->attrName.size+1);/*alloc-3*/
                        strncpy(attr[i]->attrName, (const char *) q->attrName.buf,
                                q->attrName.size + 1);
                        attr[i]->attrName[q->attrName.size]=0;/*redundant.*/
                        attr[i]->attrValuesNumber = q->attrValuesNumber;
                        attr[i]->attrValueType= (SaImmValueTypeT) q->attrValueType;
                        if (q->attrValuesNumber)
                        {
                            attr[i]->attrValues = calloc(q->attrValuesNumber, 
                                                         sizeof(SaImmAttrValueT)); /*alloc-4*/
                            /*alloc-5*/
                            attr[i]->attrValues[0] =
                                imma_copyAttrValue3(q->attrValueType, &(q->attrValue));

                            if (q->attrValuesNumber > 1)
                            {
                                int ix;
                                IMMSV_EDU_ATTR_VAL_LIST* r = q->attrMoreValues;
                                for (ix=1;ix<q->attrValuesNumber;++ix)
                                {
                                    assert(r);
                                    attr[i]->attrValues[ix] =
                                        imma_copyAttrValue3(q->attrValueType, &(r->n)); /*alloc-5*/
                                    r = r->next;
                                }//for
                            }//if
                        }//if
                    }//for
                    attr[noOfAttributes] = NULL; /*redundant*/

                    /*Need a separate const pointer just to avoid an INCORRECT warning
                      by the stupid compiler. This compiler warns when assigning 
                      non-const to a const !!!! and it is not even possible to do the 
                      cast in the function call. Note: const is MORE restrictive than 
                      non-const so assigning to a const should ALWAYS be allowed. 
                    */

                    TRACE("ccb-object-create make the callback");
#ifdef IMM_A_01_01
                    if (cl_node->isOiA1)
                    {
                        const SaImmAttrValuesT** constPtrForStupidCompiler = 
                            (const SaImmAttrValuesT **) attr;

                        localEr =
                            cl_node->o.iCallbk1.saImmOiCcbObjectCreateCallback(
                                                                              callback->lcl_imm_hdl,
                                                                              ccbid,
                                                                              className,
                                                                              &parentName,
                                                                              constPtrForStupidCompiler);
                    }
                    else
                    {
#endif
                        const SaImmAttrValuesT_2** constPtrForStupidCompiler = 
                            (const SaImmAttrValuesT_2 **) attr;

                        localEr =
                            cl_node->o.iCallbk.saImmOiCcbObjectCreateCallback(
                                                                             callback->lcl_imm_hdl,
                                                                             ccbid,
                                                                             className,
                                                                             &parentName,
                                                                             constPtrForStupidCompiler);
#ifdef IMM_A_01_01
                    }
#endif
                    TRACE("ccb-object-create callback returned RC:%u", localEr);
                    if (!(localEr==SA_AIS_OK ||
                          localEr==SA_AIS_ERR_NO_MEMORY ||
                          localEr==SA_AIS_ERR_NO_RESOURCES ||
                          localEr==SA_AIS_ERR_BAD_OPERATION))
                    {
                        TRACE_2("Illegal return value from "
                                "saImmOiCcbObjectCreateCallback %u. "
                                "Allowed are %u %u %u %u", localEr, SA_AIS_OK,
                                SA_AIS_ERR_NO_MEMORY, SA_AIS_ERR_NO_RESOURCES,
                                SA_AIS_ERR_BAD_OPERATION);
                        localEr=SA_AIS_ERR_FAILED_OPERATION; 
                        /*Change to BAD_OP if only aborting create and not ccb.*/
                    }

                    free(className);/*free-0*/
                    for (i=0; attr[i]; ++i)
                    {
                        free(attr[i]->attrName); /*free-3*/
                        attr[i]->attrName=0;
                        if (attr[i]->attrValuesNumber)
                        {
                            if ((attr[i]->attrValueType == SA_IMM_ATTR_SANAMET) ||
                                (attr[i]->attrValueType == SA_IMM_ATTR_SAANYT) ||
                                (attr[i]->attrValueType == SA_IMM_ATTR_SASTRINGT))
                            {
                                int j;
                                for (j=0;j<attr[i]->attrValuesNumber;++j)
                                {
                                    imma_freeAttrValue3(attr[i]->attrValues[j],
                                                        attr[i]->attrValueType); /*free-5*/
                                    attr[i]->attrValues[j] = 0;
                                }
                            }
                            free(attr[i]->attrValues); /*4*/
                            //SaImmAttrValueT[] array-of-void*
                            attr[i]->attrValues=0;
                        }
                        free(attr[i]);/*2*/ 
                        //SaImmAttrValuesT struct
                        attr[i]=0;
                    }
                    free(attr); /*1*/
                    //SaImmAttrValuesT*[]  array-off-pointers

                }
                else
                {
                    /* No callback function registered for obj-create upcall.
                       The standard is not clear on how this case should be 
                       handled. We take the strict approach of demanding that, 
                       if there is a registered implementer, then that 
                       implementer must implement the create callback, for the
                       callback to succeed.
                   */
                    TRACE_2("saImmOiCcbObjectCreateCallback is not implemented, "
                            "yet implementer is registered and CCBs are used. "
                            "Ccb will fail");

                    localEr=SA_AIS_ERR_FAILED_OPERATION;
                }

                assert(m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) == NCSCC_RC_SUCCESS);
                locked = TRUE;
                memset(&ccbObjCrRpl, 0, sizeof(IMMSV_EVT));
                ccbObjCrRpl.type = IMMSV_EVT_TYPE_IMMND;
                ccbObjCrRpl.info.immnd.type = IMMND_EVT_A2ND_CCB_OBJ_CREATE_RSP;
                ccbObjCrRpl.info.immnd.info.ccbUpcallRsp.result = localEr;
                ccbObjCrRpl.info.immnd.info.ccbUpcallRsp.oi_client_hdl = 
                    callback->lcl_imm_hdl;
                ccbObjCrRpl.info.immnd.info.ccbUpcallRsp.ccbId = 
                    callback->ccbID;

                ccbObjCrRpl.info.immnd.info.ccbUpcallRsp.inv = callback->inv;

                /*async fevs*/
                localEr = imma_evt_fake_evs(cb, &ccbObjCrRpl, NULL, 0,
                                            cl_node->handle, &locked, FALSE);

                if (locked)
                {
                    assert(m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE) ==
                           NCSCC_RC_SUCCESS);
                    locked = FALSE;
                }

                if (localEr != NCSCC_RC_SUCCESS)
                {
                    /*Cant do anything but log error and drop this reply.*/
                    TRACE_1("CcbObjectCreatedCallback: "
                            "send reply to IMMND failed");
                }
            } while (0);

            break;

        case IMMA_CALLBACK_OI_CCB_DELETE:
            TRACE("ccb-delete op callback");
            do
            {
                SaAisErrorT localEr = SA_AIS_OK;
                IMMSV_EVT   ccbObjDelRpl;
                NCS_BOOL locked=FALSE;
#ifdef IMM_A_01_01
                if ((cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiCcbObjectDeleteCallback) ||
                    (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiCcbObjectDeleteCallback))
#else                        
                if (cl_node->o.iCallbk.saImmOiCcbObjectDeleteCallback)
#endif
                {
                    /* Anoying type diff for ccbid between OM and OI */
                    SaImmOiCcbIdT ccbid = callback->ccbID; 
#ifdef IMM_A_01_01
                    if (cl_node->isOiA1)
                        localEr =
                            cl_node->o.iCallbk1.saImmOiCcbObjectDeleteCallback(
                                                                              callback->lcl_imm_hdl,
                                                                              ccbid,
                                                                              &(callback->name));
                    else
#endif
                        localEr =
                        cl_node->o.iCallbk.saImmOiCcbObjectDeleteCallback(
                                                                         callback->lcl_imm_hdl,
                                                                         ccbid,
                                                                         &(callback->name));

                    if (!(localEr==SA_AIS_OK ||
                          localEr==SA_AIS_ERR_NO_MEMORY ||
                          localEr==SA_AIS_ERR_NO_RESOURCES ||
                          localEr==SA_AIS_ERR_BAD_OPERATION))
                    {
                        TRACE_2("Illegal return value from "
                                "saImmOiCcbObjectDeleteCallback %u. "
                                "Allowed are %u %u %u %u", localEr, SA_AIS_OK,
                                SA_AIS_ERR_NO_MEMORY, SA_AIS_ERR_NO_RESOURCES,
                                SA_AIS_ERR_BAD_OPERATION);
                        localEr=SA_AIS_ERR_FAILED_OPERATION;
                        /*Change to BAD_OP if only aborting delete and not ccb.*/
                    }
                }
                else
                {
                    /* No callback function registered for obj delete upcall.
                       The standard is not clear on how this case should be handled.
                       We take the strict approach of demanding that, if there is
                       a registered implementer, then that implementer must 
                       implement the delete callback, for the callback to succeed
                   */
                    TRACE_2("saImmOiCcbObjectDeleteCallback is not implemented, yet "
                            "implementer is registered and CCBs are used. "
                            "Ccb will fail");
                    localEr=SA_AIS_ERR_FAILED_OPERATION;
                }

                memset(&ccbObjDelRpl, 0, sizeof(IMMSV_EVT));
                ccbObjDelRpl.type = IMMSV_EVT_TYPE_IMMND;
                ccbObjDelRpl.info.immnd.type =  IMMND_EVT_A2ND_CCB_OBJ_DELETE_RSP;
                ccbObjDelRpl.info.immnd.info.ccbUpcallRsp.result = localEr;
                ccbObjDelRpl.info.immnd.info.ccbUpcallRsp.oi_client_hdl = 
                    callback->lcl_imm_hdl;
                ccbObjDelRpl.info.immnd.info.ccbUpcallRsp.ccbId = callback->ccbID;
                ccbObjDelRpl.info.immnd.info.ccbUpcallRsp.inv = callback->inv;
                ccbObjDelRpl.info.immnd.info.ccbUpcallRsp.name = callback->name;

                assert(m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) == NCSCC_RC_SUCCESS);
                locked = TRUE;
                /*async  fevs*/
                localEr = imma_evt_fake_evs(cb, &ccbObjDelRpl, NULL, 0,
                                            cl_node->handle, &locked, FALSE);   

                if (locked)
                {
                    assert(m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE) == 
                           NCSCC_RC_SUCCESS);
                    locked = FALSE;
                }
                if (localEr != NCSCC_RC_SUCCESS)
                {
                    /*Cant do anything but log error and drop this reply.*/
                    TRACE_1("CcbObjectDeleteCallback: send reply to IMMND failed");
                }
            } while (0);

            break;

        case IMMA_CALLBACK_OI_CCB_MODIFY:
            TRACE("ccb-object-modify op callback");
            do
            {
                SaAisErrorT localEr = SA_AIS_OK;
                IMMSV_EVT  ccbObjModRpl;
                NCS_BOOL locked=FALSE;
                SaImmAttrModificationT_2** attr = NULL;
                int i=0;  
#ifdef A_01_01
                if ((cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiCcbObjectModifyCallback) ||
                    (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiCcbObjectModifyCallback))
#else
                if (cl_node->o.iCallbk.saImmOiCcbObjectModifyCallback)
#endif
                {
                    /* Anoying type diff for ccbid between OM and OI */
                    SaImmOiCcbIdT ccbid = callback->ccbID; 

                    SaNameT objectName = callback->name;
                    int noOfAttrMods =0;

                    IMMSV_ATTR_MODS_LIST* p = callback->attrMods;
                    while (p)
                    {
                        ++noOfAttrMods; 
                        p = p->next;
                    }

                    /*alloc-1*/
                    attr = calloc(noOfAttrMods+1, sizeof(SaImmAttrModificationT_2*));
                    p = callback->attrMods;
                    for (;i<noOfAttrMods; i++, p = p->next)
                    {

                        attr[i] = calloc(1, sizeof(SaImmAttrModificationT_2)); /*alloc-2*/
                        attr[i]->modType = p->attrModType;

                        attr[i]->modAttr.attrName = p->attrValue.attrName.buf; /* steal/alloc-3 */
                        p->attrValue.attrName.buf = NULL;
                        p->attrValue.attrName.size = 0;
                        attr[i]->modAttr.attrValuesNumber = p->attrValue.attrValuesNumber;
                        attr[i]->modAttr.attrValueType= (SaImmValueTypeT) 
                                                        p->attrValue.attrValueType;
                        if (p->attrValue.attrValuesNumber)
                        {
                            attr[i]->modAttr.attrValues = 
                                calloc(p->attrValue.attrValuesNumber,
                                       sizeof(SaImmAttrValueT)); /*alloc-4*/
                            /*alloc-5*/
                            attr[i]->modAttr.attrValues[0] =
                                imma_copyAttrValue3(p->attrValue.attrValueType, 
                                                    &(p->attrValue.attrValue));

                            if (p->attrValue.attrValuesNumber > 1)
                            {
                                int ix;
                                IMMSV_EDU_ATTR_VAL_LIST* r = p->attrValue.attrMoreValues;
                                for (ix=1;ix<p->attrValue.attrValuesNumber;++ix)
                                {
                                    assert(r);
                                    attr[i]->modAttr.attrValues[ix] =
                                        imma_copyAttrValue3(p->attrValue.attrValueType, 
                                                            &(r->n)); /*alloc-5*/

                                    r = r->next;
                                } //for all extra values
                            } //if multivalued
                        } //if there was any value
                    } //for all attrMods
                    /*attr[noOfAttrMods] = NULL; redundant*/

                    /*Need a separate const pointer just to avoid an INCORRECT warning
                      by the stupid compiler. This compiler warns when assigning 
                      non-const to a const !!!! and it is not even possible to do the 
                      cast in the function call. Note: const is MORE restrictive than 
                      non-const so assigning to a const should ALWAYS be allowed. 
                    */
                    TRACE("ccb-object-modify: make the callback");
#ifdef IMM_A_01_01
                    if (cl_node->isOiA1)
                    {
                        const SaImmAttrModificationT** constPtrForStupidCompiler = 
                            (const SaImmAttrModificationT **) attr;

                        localEr =
                            cl_node->o.iCallbk1.saImmOiCcbObjectModifyCallback(
                                                                              callback->lcl_imm_hdl,
                                                                              ccbid,
                                                                              &objectName,
                                                                              constPtrForStupidCompiler);
                    }
                    else
                    {
#endif
                        const SaImmAttrModificationT_2** constPtrForStupidCompiler = 
                            (const SaImmAttrModificationT_2 **) attr;

                        localEr =
                            cl_node->o.iCallbk.saImmOiCcbObjectModifyCallback(
                                                                             callback->lcl_imm_hdl,
                                                                             ccbid,
                                                                             &objectName,
                                                                             constPtrForStupidCompiler);
#ifdef IMM_A_01_01
                    }
#endif

                    TRACE("ccb-object-modify callback returned RC:%u", localEr);
                    if (!(localEr==SA_AIS_OK ||
                          localEr==SA_AIS_ERR_NO_MEMORY ||
                          localEr==SA_AIS_ERR_NO_RESOURCES ||
                          localEr==SA_AIS_ERR_BAD_OPERATION))
                    {
                        TRACE_2("Illegal return value from "
                                "saImmOiCcbObjectModifyCallback %u. "
                                "Allowed are %u %u %u %u", localEr, SA_AIS_OK,
                                SA_AIS_ERR_NO_MEMORY, SA_AIS_ERR_NO_RESOURCES,
                                SA_AIS_ERR_BAD_OPERATION);
                        localEr=SA_AIS_ERR_FAILED_OPERATION;
                    }

                    for (i=0; attr[i]; ++i)
                    {
                        free(attr[i]->modAttr.attrName); /*free-3*/
                        attr[i]->modAttr.attrName=0;
                        if (attr[i]->modAttr.attrValuesNumber)
                        {
                            if ((attr[i]->modAttr.attrValueType == SA_IMM_ATTR_SANAMET) ||
                                (attr[i]->modAttr.attrValueType == SA_IMM_ATTR_SAANYT) ||
                                (attr[i]->modAttr.attrValueType == SA_IMM_ATTR_SASTRINGT))
                            {
                                int j;
                                for (j=0;j<attr[i]->modAttr.attrValuesNumber;++j)
                                {
                                    imma_freeAttrValue3(attr[i]->modAttr.attrValues[j],
                                                        attr[i]->modAttr.attrValueType); /*free-5*/
                                    attr[i]->modAttr.attrValues[j] = 0;
                                }
                            }
                            free(attr[i]->modAttr.attrValues); /*4*/
                            //SaImmAttrValueT[] array-of-void*
                            attr[i]->modAttr.attrValues=0;
                        }
                        free(attr[i]);/*2*/ 
                        //SaImmAttrValuesT struct
                        attr[i]=0;
                    }
                    free(attr); /*1*/
                    //SaImmAttrValuesT*[]  array-off-pointers
                }
                else
                {
                    /* No callback function registered for obj-modify upcall.
                       The standard is not clear on how this case should be 
                       handled. We take the strict approach of demanding that, 
                       if there is a registered implementer, then that 
                       implementer must implement the create callback, for the
                       callback to succeed.
                   */
                    TRACE_2("saImmOiCcbObjectModifyCallback is not implemented, "
                            "yet implementer is registered and CCBs are used. "
                            "Ccb will fail");

                    localEr=SA_AIS_ERR_FAILED_OPERATION;
                    /*Change to BAD_OP if only aborting modify and not ccb.*/
                }

                assert(m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) == NCSCC_RC_SUCCESS);
                locked = TRUE;
                memset(&ccbObjModRpl, 0, sizeof(IMMSV_EVT));
                ccbObjModRpl.type = IMMSV_EVT_TYPE_IMMND;
                ccbObjModRpl.info.immnd.type = IMMND_EVT_A2ND_CCB_OBJ_MODIFY_RSP;
                ccbObjModRpl.info.immnd.info.ccbUpcallRsp.result = localEr;
                ccbObjModRpl.info.immnd.info.ccbUpcallRsp.oi_client_hdl = 
                    callback->lcl_imm_hdl;
                ccbObjModRpl.info.immnd.info.ccbUpcallRsp.ccbId = 
                    callback->ccbID;
                ccbObjModRpl.info.immnd.info.ccbUpcallRsp.inv = callback->inv;

                /*async fevs*/
                localEr = imma_evt_fake_evs(cb, &ccbObjModRpl, NULL, 0,
                                            cl_node->handle, &locked, FALSE);

                if (locked)
                {
                    assert(m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE) ==
                           NCSCC_RC_SUCCESS);
                    locked = FALSE;
                }

                if (localEr != NCSCC_RC_SUCCESS)
                {
                    /*Cant do anything but log error and drop this reply.*/
                    TRACE_1("CcbObjectModifyCallback: send reply to IMMND failed");
                }
            } while (0);

            break;

        case IMMA_CALLBACK_OI_CCB_ABORT:
            TRACE("ccb-abort op callback");
            do
            {
#ifdef IMM_A_01_01
                if ((cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiCcbAbortCallback) ||
                    (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiCcbAbortCallback))
#else
                if (cl_node->o.iCallbk.saImmOiCcbAbortCallback)
#endif
                {
                    /* Anoying type diff for ccbid between OM and OI */
                    SaImmOiCcbIdT ccbid = callback->ccbID; 
#ifdef IMM_A_01_01
                    if (cl_node->isOiA1)
                        cl_node->o.iCallbk1.saImmOiCcbAbortCallback(callback->lcl_imm_hdl,ccbid);
                    else
#endif
                        cl_node->o.iCallbk.saImmOiCcbAbortCallback(callback->lcl_imm_hdl, ccbid);
                }
                else
                {
                    /* No callback function registered for apply upcall.
                       There is nothign we can do about this since the CCB is
                       commited already. It also makes sense that some applications
                       may want to ignore the apply upcall.
                    */
                    TRACE_2("saImmOiCcbAbortCallback is not implemented, yet "
                            "implementer is registered and CCBs are used. "
                            "Ccb will abort anyway");
                }
            } while (0);

            break;

        case IMMA_CALLBACK_OI_RT_ATTR_UPDATE:
            TRACE("rt-attr-update callback");
            do
            {
                SaAisErrorT localEr = SA_AIS_OK;
                uns32       proc_rc = NCSCC_RC_SUCCESS;
                IMMSV_EVT   rtAttrUpdRpl;
#ifdef IMM_A_01_01
                if ((cl_node->isOiA1 && cl_node->o.iCallbk1.saImmOiRtAttrUpdateCallback) ||
                    (!cl_node->isOiA1 && cl_node->o.iCallbk.saImmOiRtAttrUpdateCallback))
#else
                if (cl_node->o.iCallbk.saImmOiRtAttrUpdateCallback)
#endif
                {
                    SaImmAttrNameT* attributeNames;
                    int noOfAttrNames=0;
                    int i=0;

                    IMMSV_ATTR_NAME_LIST* p = callback->attrNames;
                    while (p)
                    {
                        ++noOfAttrNames;
                        p = p->next;
                    }
                    assert(noOfAttrNames);                           /*alloc-1*/
                    attributeNames = calloc(noOfAttrNames+1, sizeof(SaImmAttrNameT));
                    p = callback->attrNames;
                    for (;i<noOfAttrNames; i++, p = p->next)
                    {
                        attributeNames[i] = p->name.buf;
                    }

                    /*attributeNames[noOfAttrNames] = NULL; calloc=> redundant*/

                    TRACE("Invoking saImmOiRtAttrUpdateCallback");
#ifdef IMM_A_01_01
                    if (cl_node->isOiA1)
                        localEr = cl_node->o.iCallbk1.saImmOiRtAttrUpdateCallback(
                                                                                 callback->lcl_imm_hdl,
                                                                                 &callback->name,
                                                                                 attributeNames);
                    else
#endif
                        localEr = cl_node->o.iCallbk.saImmOiRtAttrUpdateCallback(
                                                                                callback->lcl_imm_hdl,
                                                                                &callback->name,
                                                                                attributeNames);


                    TRACE("saImmOiRtAttrUpdateCallback returned RC:%u", localEr);
                    if (!(localEr==SA_AIS_OK ||
                          localEr==SA_AIS_ERR_NO_MEMORY ||
                          localEr==SA_AIS_ERR_NO_RESOURCES ||
                          localEr==SA_AIS_ERR_FAILED_OPERATION))
                    {
                        TRACE_2("Illegal return value from "
                                "saImmOiCcbObjectModifyCallback %u. "
                                "Allowed are %u %u %u %u", localEr, SA_AIS_OK,
                                SA_AIS_ERR_NO_MEMORY, SA_AIS_ERR_NO_RESOURCES,
                                SA_AIS_ERR_FAILED_OPERATION);
                        localEr=SA_AIS_ERR_FAILED_OPERATION;
                    }

                    free(attributeNames); /*We do not leak the attr names here because they are still
                                            attached to, and deallocated by, the callback structure. */
                }
                else
                {
                    /* No callback function registered for rt-attr-update upcall.
                       The standard is not clear on how this case should be 
                       handled. We take the strict approach of demanding that, 
                       if there is a registered implementer, then that 
                       implementer must implement the callback, for the
                       callback to succeed.
                   */
                    TRACE_2("saImmOiRtAttrUpdateCallback is not implemented, "
                            "yet implementer is registered and pure runtime attrs "
                            "are fetched.");

                    localEr=SA_AIS_ERR_FAILED_OPERATION;
                }
                memset(&rtAttrUpdRpl, 0, sizeof(IMMSV_EVT));
                rtAttrUpdRpl.type = IMMSV_EVT_TYPE_IMMND;
                rtAttrUpdRpl.info.immnd.type = IMMND_EVT_A2ND_RT_ATT_UPPD_RSP;
                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.result = localEr;
                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.sr.client_hdl = 
                    callback->lcl_imm_hdl;
                ImmsvInvocation* invocp = (ImmsvInvocation *) &(callback->invocation);

                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.sr.searchId = invocp->inv;
                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.sr.remoteNodeId = 
                    invocp->owner;

                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.sr.objectName.size = 
                    callback->name.length + 1; /*Adding one to get the terminating null sent*/
                /* Only borowing the name string from the SaName in the callback */
                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.sr.objectName.buf = 
                    (char *) callback->name.value;
                /* Only borowing the attributeNames list from callback. */
                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.sr.attributeNames = 
                    callback->attrNames;
                rtAttrUpdRpl.info.immnd.info.rtAttUpdRpl.sr.requestNodeId = 
                    callback->requestNodeId;
                /* No structures allocated, all pointers borowed => nothing to free. */

                if (cb->is_immnd_up == FALSE)
                {
                    proc_rc = SA_AIS_ERR_NO_RESOURCES;
                    TRACE_2("IMMND_DOWN");
                }
                else
                {
                    /* send the reply to the IMMND asyncronously */
                    proc_rc = imma_mds_msg_send(cb->imma_mds_hdl, &cb->immnd_mds_dest, 
                                                &rtAttrUpdRpl, NCSMDS_SVC_ID_IMMND);
                    if (proc_rc != NCSCC_RC_SUCCESS)
                    {
                        /*Cant do anything but log error and drop this reply.*/
                        TRACE_1("oiRtAttrUpdateCallback: send reply to IMMND failed");
                    }
                }

            } while (0);

            break;

        case IMMA_CALLBACK_STALE_HANDLE:
            TRACE("Stale handle indication received on OM handle");
            break;

        default:
            TRACE_1("Unrecognized OI callback type: %u",callback->type);
            break;
    }
#endif /* ifdef IMMA_OI */

    /* free the callback info. Note - we are not locked. Still should be ok since
     the callback was dequeue'd by this call. */
    /* Also verify that we free any/all pointer structures that have not 
       been set to NULL*/
    imma_proc_free_callback(callback);
    TRACE_LEAVE();
}


/****************************************************************************
 * Name          : imma_cb_dump
 *
 * Description   : This routine is used for debugging.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
void imma_cb_dump(void )
{
    IMMA_CB *cb = &imma_cb;

    /* Take the CB lock */
    if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)
        return;

    TRACE("*****************Printing IMMA CB Dump****************");
    TRACE("\n MDS Handle:             %x", (uns32)cb->imma_mds_hdl);
    TRACE("\n Handle Manager Pool ID: %d", cb->pool_id);
    TRACE("\n Handle Manager Handle:  %d", cb->agent_handle_id);
    if (cb->is_immnd_up)
    {
        TRACE("\n IMMND UP, Node ID = %d", 
                m_NCS_NODE_ID_FROM_MDS_DEST(cb->immnd_mds_dest));
    }
    else
        TRACE("\n IMMND DOWN");


    if (cb->is_client_tree_up)
    {
        TRACE(
               "\n+++++++++++++Client Tree is UP+++++++++++++++++++++++++++");
        TRACE("\nNumber of nodes in ClientTree:  %d", 
                cb->client_tree.n_nodes);

        /* Print the Client tree Details */
        {
            IMMA_CLIENT_NODE  * clnode;
            SaImmHandleT *temp_ptr=0;
            SaImmHandleT temp_hdl=0;

            temp_ptr = &temp_hdl;

            /* scan the entire handle db & delete each record */
            while ((clnode = (IMMA_CLIENT_NODE *)
                    ncs_patricia_tree_getnext(&cb->client_tree, (uns8 *)temp_ptr)))
            {
                /* delete the client info */
                temp_hdl = clnode->handle;

                TRACE("\n-------------------------------------------");
                TRACE("\n CLient Handle   = %d", 
                        (uns32)clnode->handle);
            }
            TRACE("\n End of Info for this client");
        }
        TRACE("\n End of Client info nodes ");
    }

    /* Print the Lcl Checkpoint Details */
    if (cb->is_admin_owner_tree_up)
    {
        TRACE("\n+++++++++++++Lcl IMM Tree is UP++++++++++++++++++");
        TRACE("\nNumber of nodes in Lcl IMM Tree:  %d", 
                cb->admin_owner_tree.n_nodes);

        /* Print the Lcl IMM Details */
        {
            SaImmAdminOwnerHandleT prev_id=0;
            IMMA_ADMIN_OWNER_NODE *ao_node;

            /* Get the First Node */
            ao_node = (IMMA_ADMIN_OWNER_NODE *)
                      ncs_patricia_tree_getnext(&cb->admin_owner_tree,
                                                (uns8*)&prev_id);

            while (ao_node)
            {
                prev_id = ao_node->admin_owner_hdl;

                TRACE("\n-------------------------------------------");
                TRACE("\n Lcl IMM Hdl:  = %d", 
                        (uns32)ao_node->admin_owner_hdl);
                TRACE("\n Client IMM Hdl:  = %d", 
                        (uns32)ao_node->mImmHandle);
                TRACE("\n End of Local IMM Info");

                ao_node = (IMMA_ADMIN_OWNER_NODE *)
                          ncs_patricia_tree_getnext(&cb->admin_owner_tree,
                                                    (uns8*)&prev_id); 
            }
            TRACE("\n End of Local IMM nodes information ");
        }

    }

    TRACE("*****************End of IMMD CB Dump******************");

    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    return;  
}

static void imma_proc_free_callback(IMMA_CALLBACK_INFO *callback)
{
    if (!callback) return;
    if (callback->params)
    {
        int i;
        for (i=0;callback->params[i];++i)
        {
            SaImmAdminOperationParamsT_2* q = callback->params[i];
            imma_freeAttrValue3(q->paramBuffer, q->paramType); /*free-4*/

            free(q->paramName); /*free-3*/
            q->paramName = NULL;
            free(q); /*free-2*/
        }

        free(callback->params); /*free-1*/
    }

    if(callback->attrValues) {
        immsv_free_attrvalues_list(callback->attrValues);
        callback->attrValues= NULL;
    }

    if(callback->attrMods) {
        immsv_free_attrmods(callback->attrMods);
        callback->attrMods = NULL;
    }
    
    if(callback->attrNames) {
        immsv_evt_free_attrNames(callback->attrNames);
        callback->attrNames = NULL;
    }
    

    free(callback);
}

/*******************************************************************
 * imma_evt_fake_evs internal function
 *
 * NOTE: The CB must be LOCKED on entry of this function!!
 *       It will usually be unlocked on exit, as reflected in the 'locked' 
 *       parameter.
 *******************************************************************/
SaAisErrorT imma_evt_fake_evs(IMMA_CB *cb,
                              IMMSV_EVT *i_evt, 
                              IMMSV_EVT **o_evt,
                              uns32  timeout,
                              SaImmHandleT immHandle,
                              NCS_BOOL* locked,
                              NCS_BOOL checkWritable)
{
    SaAisErrorT rc = SA_AIS_OK;
    IMMSV_EVT      fevs_evt;
    uns32 proc_rc;
    char* tmpData = NULL;  
    NCS_UBAID uba;
    uba.start=NULL;

    assert(locked && (*locked));
    /*Pack the message for sending over multiple hops.*/

    if (ncs_enc_init_space(&uba) != NCSCC_RC_SUCCESS)
    {
        uba.start=NULL;
        TRACE_1("Failed init ubaid");
        return SA_AIS_ERR_NO_RESOURCES;
    }

    /* Encode non-flat since we broadcast to unknown receivers. */
    rc = immsv_evt_enc(i_evt, &uba);

    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE_1("Failed to pre-pack");
        return SA_AIS_ERR_NO_RESOURCES;
    }

    int32 size = uba.ttl;
    /*NOTE: should check against "payload max-size"*/

    tmpData = malloc(size);
    char* data = m_MMGR_DATA_AT_START(uba.start, size, tmpData);

    memset(&fevs_evt, 0, sizeof(IMMSV_EVT));
    fevs_evt.type = IMMSV_EVT_TYPE_IMMND;
    fevs_evt.info.immnd.type = IMMND_EVT_A2ND_IMM_FEVS;
    fevs_evt.info.immnd.info.fevsReq.client_hdl = immHandle;
    /*Overloaded use of sender_count. IMMA->IMMND we use it as a marker
     if imm writability should be checked before sending to IMMD. */
    fevs_evt.info.immnd.info.fevsReq.sender_count = checkWritable;
    fevs_evt.info.immnd.info.fevsReq.msg.size = size;
    fevs_evt.info.immnd.info.fevsReq.msg.buf = data;

    /* Unlock before MDS Send */
    m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
    *locked = FALSE;


    /* IMMND GOES DOWN */
    if (cb->is_immnd_up == FALSE)
    {
        rc = SA_AIS_ERR_TRY_AGAIN;
        TRACE_2("IMMND is DOWN");
        goto fail;
    }

    if (o_evt)
    {
        /* Send the evt to IMMND syncronously (reply expected) */
        proc_rc = imma_mds_msg_sync_send(cb->imma_mds_hdl, &(cb->immnd_mds_dest), 
                                         &fevs_evt, o_evt, timeout);
    }
    else
    {
        /*Send evt to IMMND asyncronously, no reply expected. */
        assert(timeout == 0);
        proc_rc = imma_mds_msg_send(cb->imma_mds_hdl, &(cb->immnd_mds_dest), 
                                    &fevs_evt, NCSMDS_SVC_ID_IMMND);
    }

    /* Generate rc from proc_rc */
    switch (proc_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            break;

        default:
            rc = SA_AIS_ERR_NO_RESOURCES;
            TRACE_1("MDS returned unexpected error code %u", proc_rc);
            break;
    }

    fail:

    if (tmpData)
    {
        free(tmpData); 
        tmpData = NULL;
    }

    if (uba.start)
    {
        m_MMGR_FREE_BUFR_LIST(uba.start);
    }

    return rc;
}

