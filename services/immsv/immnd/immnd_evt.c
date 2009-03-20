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
  FILE NAME: immcn_evt.c

  DESCRIPTION: IMMND Event handling routines

  FUNCTIONS INCLUDED in this module:
  immnd_process_evt .........IMMND Event processing routine.
******************************************************************************/

#include "immnd.h"
#include "immsv_api.h"

static uns32 immnd_evt_proc_cb_dump(IMMND_CB *cb);
static uns32 immnd_evt_proc_imm_init(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo,
    SaBoolT isOm);
static uns32 immnd_evt_proc_imm_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo,
    SaBoolT isOm);

static uns32 immnd_evt_proc_sync_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_admowner_init(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);
static uns32 immnd_evt_proc_impl_set(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);
static uns32 immnd_evt_proc_ccb_init(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_rt_update(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static void immnd_evt_proc_discard_impl(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_discard_node(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_adminit_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_admo_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_admo_hard_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_admo_set(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_admo_release(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_admo_clear(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_finalize_sync(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_impl_set_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_impl_clr(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_cl_impl_set(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_cl_impl_rel(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_obj_impl_set(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_obj_impl_rel(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_ccbinit_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_admop(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_class_create(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_class_delete(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_object_create(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_rt_object_create(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_object_modify(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_rt_object_modify(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_object_delete(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_rt_object_delete(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_object_sync(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static void immnd_evt_proc_ccb_apply(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);


static void immnd_evt_proc_ccb_compl_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest);

static uns32 immnd_evt_proc_admop_rsp(IMMND_CB *cb, IMMND_EVT *evt,
    IMMSV_SEND_INFO *sinfo,
    SaBoolT async,
    SaBoolT local);

static uns32 immnd_evt_proc_fevs_forward(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);
static uns32 immnd_evt_proc_fevs_rcv(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_intro_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_sync_req(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_loading_ok(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_dump_ok(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_start_sync(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_class_desc_get(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);
static uns32 immnd_evt_proc_search_init(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);
static uns32 immnd_evt_proc_search_next(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_rt_update_pull(IMMND_CB *cb, IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_oi_att_pull_rpl(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_remote_search_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_search_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo);

static uns32 immnd_evt_proc_mds_evt (IMMND_CB *cb,IMMND_EVT *evt);

/*static uns32 immnd_evt_immd_new_active(IMMND_CB *cb);*/

static void  immnd_evt_ccb_abort(IMMND_CB *cb,
    SaUint32T ccbId, 
    SaBoolT timeout,
    SaUint32T* client);

static uns32 immnd_evt_proc_reset(IMMND_CB *cb, IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo);

static void freeSearchNext(IMMSV_OM_RSP_SEARCH_NEXT* rsp, SaBoolT freeTop);
static void freeAttrNameList(IMMSV_ATTR_NAME_LIST* attn);

#if 0 /* Only for debug */
static void
printImmValue(SaImmValueTypeT t, IMMSV_EDU_ATTR_VAL* v)
{
    switch(t)
    {
        case SA_IMM_ATTR_SAINT32T:
            TRACE_2("INT32: %i", v->val.saint32);
            break;
        case SA_IMM_ATTR_SAUINT32T:
            TRACE_2("UINT32: %u", v->val.sauint32);
            break;
        case SA_IMM_ATTR_SAINT64T:
            TRACE_2("INT64: %lli", v->val.saint64);
            break;
        case SA_IMM_ATTR_SAUINT64T:
            TRACE_2("UINT64: %llu", v->val.sauint64);
            break;
        case SA_IMM_ATTR_SATIMET:
            TRACE_2("TIME: %llu", v->val.satime);
            break;
        case SA_IMM_ATTR_SANAMET:
            TRACE_2("NAME: %u/%s", v->val.x.size, v->val.x.buf);
            break;
        case SA_IMM_ATTR_SAFLOATT:
            TRACE_2("FLOAT: %f", v->val.safloat);
            break;
        case SA_IMM_ATTR_SADOUBLET:
            TRACE_2("DOUBLE: %f", v->val.sadouble);
            break;
        case SA_IMM_ATTR_SASTRINGT:
            TRACE_2("STRING: %u/%s", v->val.x.size, v->val.x.buf);
            break;
        case SA_IMM_ATTR_SAANYT:
            TRACE_2("ANYT: %u/%s", v->val.x.size, v->val.x.buf);
            break;
        default:
            TRACE_2("ERRONEOUS VALUE TAG: %i", t);
            break;
    }
}
#endif

/****************************************************************************
 * Name          : immnd_evt_destroy
 *
 * Description   : Function to process the   
 *                 from Applications. 
 *
 * Arguments     : IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT onheap - false => dont deallocate root
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 immnd_evt_destroy(IMMSV_EVT *evt, SaBoolT onheap, uns32 line)
{
    uns32 rc=NCSCC_RC_SUCCESS;

    if ( evt == NULL) {
        /* LOG */
        return NCSCC_RC_SUCCESS;
    }

    if(evt->info.immnd.dont_free_me == TRUE)  
        return NCSCC_RC_SUCCESS;

    if(evt->type != IMMSV_EVT_TYPE_IMMND)
        return NCSCC_RC_SUCCESS;

    if((evt->info.immnd.type == IMMND_EVT_A2ND_IMM_ADMOP) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_IMM_ADMOP_ASYNC))
    {
        /*TODO See TODO 12345 code repeated (almost) in imma_om_api.c
          free-1*/
        free(evt->info.immnd.info.admOpReq.objectName.buf);
        evt->info.immnd.info.admOpReq.objectName.buf=NULL;
        while(evt->info.immnd.info.admOpReq.params) {
            IMMSV_ADMIN_OPERATION_PARAM* p = evt->info.immnd.info.admOpReq.params;
            evt->info.immnd.info.admOpReq.params = p->next;

            if(p->paramName.buf) { /*free-3*/
                free(p->paramName.buf);
                p->paramName.buf = NULL;
                p->paramName.size = 0;
            }
            immsv_evt_free_att_val(&(p->paramBuffer), p->paramType); /*free-4*/
            p->next=NULL;
            free(p); /*free-2*/
        }
    } else if((evt->info.immnd.type == IMMND_EVT_D2ND_GLOB_FEVS_REQ) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_IMM_FEVS))
    {
            if(evt->info.immnd.info.fevsReq.msg.buf != NULL) {
                free(evt->info.immnd.info.fevsReq.msg.buf);
                evt->info.immnd.info.fevsReq.msg.buf=NULL;
                evt->info.immnd.info.fevsReq.msg.size=0;
            }
    } else if(evt->info.immnd.type == IMMND_EVT_A2ND_RT_ATT_UPPD_RSP) 
    {
        free(evt->info.immnd.info.rtAttUpdRpl.sr.objectName.buf);
        evt->info.immnd.info.rtAttUpdRpl.sr.objectName.buf=NULL;
        evt->info.immnd.info.rtAttUpdRpl.sr.objectName.size=0;
        freeAttrNameList(evt->info.immnd.info.rtAttUpdRpl.sr.attributeNames);
        evt->info.immnd.info.rtAttUpdRpl.sr.attributeNames = NULL;
    } else if(evt->info.immnd.type == IMMND_EVT_ND2ND_SEARCH_REMOTE) 
    {
        free(evt->info.immnd.info.searchRemote.objectName.buf);
        evt->info.immnd.info.searchRemote.objectName.buf=NULL;
        evt->info.immnd.info.searchRemote.objectName.size=0;
        freeAttrNameList(evt->info.immnd.info.searchRemote.attributeNames);
        evt->info.immnd.info.searchRemote.attributeNames = NULL;
    } else if(evt->info.immnd.type == IMMND_EVT_ND2ND_SEARCH_REMOTE_RSP)
    {
        freeSearchNext(&evt->info.immnd.info.rspSrchRmte.runtimeAttrs, FALSE);
    } else if(evt->info.immnd.type == IMMND_EVT_ND2ND_SYNC_FINALIZE)
    {
        immsv_evt_free_admo(evt->info.immnd.info.finSync.adminOwners);
        evt->info.immnd.info.finSync.adminOwners = NULL;
        immsv_evt_free_impl(evt->info.immnd.info.finSync.implementers);
        evt->info.immnd.info.finSync.implementers = NULL;
        immsv_evt_free_classList(evt->info.immnd.info.finSync.classes);
        evt->info.immnd.info.finSync.classes = NULL;
    } else if((evt->info.immnd.type == IMMND_EVT_A2ND_OBJ_CREATE) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_OBJ_CREATE))
    {
        free(evt->info.immnd.info.objCreate.className.buf);
        evt->info.immnd.info.objCreate.className.buf = NULL;
        evt->info.immnd.info.objCreate.className.size = 0;

        free(evt->info.immnd.info.objCreate.parentName.buf);
        evt->info.immnd.info.objCreate.parentName.buf = NULL;
        evt->info.immnd.info.objCreate.parentName.size = 0;

        immsv_free_attrvalues_list(evt->info.immnd.info.objCreate.attrValues);
        evt->info.immnd.info.objCreate.attrValues = NULL;
    } else if((evt->info.immnd.type == IMMND_EVT_A2ND_OBJ_MODIFY) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_OI_OBJ_MODIFY))
    {
        free(evt->info.immnd.info.objModify.objectName.buf);
        evt->info.immnd.info.objModify.objectName.buf = NULL;
        evt->info.immnd.info.objModify.objectName.size = 0;

        immsv_free_attrmods(evt->info.immnd.info.objModify.attrMods);
        evt->info.immnd.info.objModify.attrMods = NULL;
    } else if((evt->info.immnd.type == IMMND_EVT_A2ND_OBJ_DELETE) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_OI_OBJ_DELETE))
    {
        free(evt->info.immnd.info.objDelete.objectName.buf);
        evt->info.immnd.info.objDelete.objectName.buf = NULL;
        evt->info.immnd.info.objDelete.objectName.size = 0;
    } else if(evt->info.immnd.type == IMMND_EVT_A2ND_OBJ_SYNC)
    {
        free(evt->info.immnd.info.obj_sync.className.buf);
        evt->info.immnd.info.obj_sync.className.buf = NULL;
        evt->info.immnd.info.obj_sync.className.size = 0;

        free(evt->info.immnd.info.obj_sync.objectName.buf);
        evt->info.immnd.info.obj_sync.objectName.buf = NULL;
        evt->info.immnd.info.obj_sync.objectName.size = 0;

        immsv_free_attrvalues_list(evt->info.immnd.info.obj_sync.attrValues);
        evt->info.immnd.info.obj_sync.attrValues = NULL;
    } else if((evt->info.immnd.type == IMMND_EVT_A2ND_CLASS_CREATE) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_CLASS_DELETE))
    {
        free(evt->info.immnd.info.classDescr.className.buf);
        evt->info.immnd.info.classDescr.className.buf = NULL;
        evt->info.immnd.info.classDescr.className.size = 0;

        immsv_free_attrdefs_list(evt->info.immnd.info.classDescr.attrDefinitions);
        evt->info.immnd.info.classDescr.attrDefinitions = NULL;
        
    } else if((evt->info.immnd.type == IMMND_EVT_D2ND_IMPLSET_RSP) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_OI_CL_IMPL_SET) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_OI_CL_IMPL_REL) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_OI_OBJ_IMPL_SET) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_OI_OBJ_IMPL_REL))
    {
        free(evt->info.immnd.info.implSet.impl_name.buf);
        evt->info.immnd.info.implSet.impl_name.buf = NULL;
        evt->info.immnd.info.implSet.impl_name.size = 0;
    } else if((evt->info.immnd.type == IMMND_EVT_A2ND_ADMO_SET) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_ADMO_RELEASE) ||
        (evt->info.immnd.type == IMMND_EVT_A2ND_ADMO_CLEAR))
    {
        immsv_evt_free_name_list(evt->info.immnd.info.admReq.objectNames);
        evt->info.immnd.info.admReq.objectNames = NULL;
    }

    if(onheap) {
        free(evt);
    }
    return rc;
}

void immnd_process_evt(void)
{
    IMMND_CB  *cb = immnd_cb;
    uns32    rc=NCSCC_RC_SUCCESS;

    IMMSV_EVT *evt;

    evt = (IMMSV_EVT *) ncs_ipc_non_blk_recv(&immnd_cb->immnd_mbx);

    if (evt == NULL)
    {
        LOG_ER("No mbx message although indicated in fd!");
        TRACE_LEAVE();
        return;
    }

    if(evt->type != IMMSV_EVT_TYPE_IMMND)
    {
        LOG_ER("IMMND - Unknown Event");
        immnd_evt_destroy(evt, SA_TRUE, __LINE__);
        return;
    }

    if (evt->info.immnd.type != IMMND_EVT_D2ND_GLOB_FEVS_REQ)
        immsv_msg_trace_rec(evt->sinfo.dest, evt);

    switch(evt->info.immnd.type)
    {
        case IMMND_EVT_MDS_INFO:
            rc = immnd_evt_proc_mds_evt(cb, &evt->info.immnd);
            break;

        case IMMND_EVT_CB_DUMP:
            rc=immnd_evt_proc_cb_dump(cb); 
            break;

            /* Agent to ND */
        case IMMND_EVT_A2ND_IMM_INIT:
            rc = immnd_evt_proc_imm_init(cb, &evt->info.immnd, &evt->sinfo, SA_TRUE);
            break;

        case IMMND_EVT_A2ND_IMM_OI_INIT:
            rc = immnd_evt_proc_imm_init(cb, &evt->info.immnd, &evt->sinfo,SA_FALSE);
            break;

        case IMMND_EVT_A2ND_IMM_FINALIZE:
            rc= immnd_evt_proc_imm_finalize(cb, &evt->info.immnd, &evt->sinfo,
                SA_TRUE);
            break;

        case IMMND_EVT_A2ND_IMM_OI_FINALIZE:
            rc=immnd_evt_proc_imm_finalize(cb, &evt->info.immnd, &evt->sinfo, 
                SA_FALSE);
            break;

        case IMMND_EVT_A2ND_SYNC_FINALIZE:
            rc= immnd_evt_proc_sync_finalize(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_IMM_ADMINIT:
            rc = immnd_evt_proc_admowner_init(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_OI_IMPL_SET:
            rc = immnd_evt_proc_impl_set(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_CCBINIT:
            rc = immnd_evt_proc_ccb_init(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_OI_OBJ_MODIFY:
            rc = immnd_evt_proc_rt_update(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_IMM_FEVS:
            rc = immnd_evt_proc_fevs_forward(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_CLASS_DESCR_GET:
            rc = immnd_evt_proc_class_desc_get(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_SEARCHINIT:
            rc = immnd_evt_proc_search_init(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_SEARCHNEXT:
            rc = immnd_evt_proc_search_next(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_RT_ATT_UPPD_RSP:
            rc = immnd_evt_proc_oi_att_pull_rpl(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_A2ND_SEARCHFINALIZE:
            rc = immnd_evt_proc_search_finalize(cb, &evt->info.immnd, &evt->sinfo);
            break;


        case IMMND_EVT_A2ND_ADMOP_RSP:
            rc = immnd_evt_proc_admop_rsp(cb, &evt->info.immnd, &evt->sinfo,
                SA_FALSE, SA_TRUE);
            break;

        case IMMND_EVT_A2ND_ASYNC_ADMOP_RSP:
            rc = immnd_evt_proc_admop_rsp(cb, &evt->info.immnd, &evt->sinfo,
                SA_TRUE, SA_TRUE);
            break;

        case IMMND_EVT_ND2ND_ADMOP_RSP:
            rc = immnd_evt_proc_admop_rsp(cb, &evt->info.immnd, &evt->sinfo, 
                SA_FALSE, SA_FALSE);
            break;

        case IMMND_EVT_ND2ND_ASYNC_ADMOP_RSP:
            rc = immnd_evt_proc_admop_rsp(cb, &evt->info.immnd, &evt->sinfo, 
                SA_TRUE, SA_FALSE);
            break;

        case IMMND_EVT_ND2ND_SEARCH_REMOTE:
            rc = immnd_evt_proc_rt_update_pull(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_ND2ND_SEARCH_REMOTE_RSP:
            rc = immnd_evt_proc_remote_search_rsp(cb, &evt->info.immnd, &evt->sinfo);
            break;

            /* D to ND */

        case IMMND_EVT_D2ND_INTRO_RSP:
            rc = immnd_evt_proc_intro_rsp(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_D2ND_SYNC_REQ:
            rc = immnd_evt_proc_sync_req(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_D2ND_LOADING_OK:
            rc = immnd_evt_proc_loading_ok(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_D2ND_DUMP_OK:
            rc = immnd_evt_proc_dump_ok(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_D2ND_SYNC_START:
            rc = immnd_evt_proc_start_sync(cb, &evt->info.immnd, &evt->sinfo);
            break;



        case IMMND_EVT_D2ND_GLOB_FEVS_REQ:
            rc = immnd_evt_proc_fevs_rcv(cb, &evt->info.immnd, &evt->sinfo);
            break;

        case IMMND_EVT_D2ND_RESET:
            rc = immnd_evt_proc_reset(cb, &evt->info.immnd, &evt->sinfo);
            break;

        default:
            rc = NCSCC_RC_FAILURE;
            break;
    }

    /* TODO? handle if(rc != OK)*/

    /* Free the Event */
    immnd_evt_destroy(evt, SA_TRUE, __LINE__);

    return;
}

/****************************************************************************
 * Name          : immnd_evt_proc_imm_init
 *
 * Description   : Function to process the SaImmOmInitialize and 
 *                 SaImmOiInitialize calls from IMM Agents.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - sender info
 *                 SaBoolT isOm - TRUE => OM, FALSE => OI
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_imm_init(IMMND_CB *cb, IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo, SaBoolT isOm)
{
    /*TODO isOm is ignored, should be set in cl_node*/
    IMMSV_EVT                send_evt;
    SaAisErrorT             error;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_HANDLE tmp_hdl;
    memset(&send_evt,'\0', sizeof(IMMSV_EVT));

    int load_pid = immModel_getLoader(cb);
    int sync_pid = load_pid?0:(immModel_getSync(cb));

    if(load_pid > 0) {
        if(evt->info.initReq.client_pid == load_pid) {
            TRACE_2("Loader attached, pid: %u", load_pid);
        } else {
            TRACE_2("Rejecting OM client attach during loading, pid %u != %u",
                evt->info.initReq.client_pid, load_pid);
            error = SA_AIS_ERR_TRY_AGAIN;
            goto agent_rsp;
        }
    } else if(load_pid < 0) {
        TRACE_2("Rejecting OM client attach. Waiting for loading or sync to "
            "complete");
        error = SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    cl_node=calloc(1, sizeof(IMMND_IMM_CLIENT_NODE));
    if( cl_node == NULL)
    {
        LOG_ER("IMMND - Client Alloc Failed");
        error=SA_AIS_ERR_NO_MEMORY;
        goto agent_rsp;
    }


    tmp_hdl.nodeId = cb->node_id;
    tmp_hdl.count = cb->cli_id_gen++;  

    /*The id generated by the ND is only valid as long as this ND process
      survives => restart of ND means loss of all client connections.
      Return ND pid in reply to use in handshake ??
    */
    cl_node->imm_app_hdl = *((SaImmHandleT*) &tmp_hdl);
    cl_node->agent_mds_dest=sinfo->dest;
    cl_node->version=evt->info.initReq.version;
    cl_node->client_pid = evt->info.initReq.client_pid;
    cl_node->sv_id = (isOm)?NCSMDS_SVC_ID_IMMA_OM:NCSMDS_SVC_ID_IMMA_OI;

    if (immnd_client_node_add(cb,cl_node) != NCSCC_RC_SUCCESS)
    {
        LOG_ER("IMMND - Client Tree Add Failed , cli_hdl");
        free(cl_node);
        error=SA_AIS_ERR_NO_MEMORY;
        goto agent_rsp;
    }


    if(sync_pid && (cl_node->client_pid == sync_pid)) {
        TRACE_2("Sync agent attached, pid: %u", sync_pid);
        cl_node->mIsSync = 1;
    }

    send_evt.info.imma.info.initRsp.immHandle= cl_node->imm_app_hdl;
    error=SA_AIS_OK;

 agent_rsp:
    send_evt.type = IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_INIT_RSP;
    send_evt.info.imma.info.initRsp.error = error;

    return immnd_mds_send_rsp(cb, sinfo, &send_evt);
}

/****************************************************************************
 * Name          : immnd_evt_proc_search_init
 *
 * Description   : Function to process the SaImmOmSearchInitialize call.
 *                 Note that this is a read, local to the ND (does not go 
 *                 over FEVS).
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - sender info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_search_init(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    SaAisErrorT             error;
    void* searchOp=NULL;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;

    memset(&send_evt,'\0', sizeof(IMMSV_EVT));
    send_evt.type=IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_SEARCHINIT_RSP;
    TRACE_2("SEARCH INIT for root:%s", evt->info.searchInit.rootName.buf);

    /*Look up client-node*/
    immnd_client_node_get(cb, evt->info.searchInit.client_hdl, &cl_node);
    if (cl_node == NULL) {
        LOG_ER("IMMND - Client Node Get Failed for cli_hdl");
        TRACE_2("Client Node get failed for handle:%llu", 
            evt->info.searchInit.client_hdl);

        send_evt.info.imma.info.searchInitRsp.error = SA_AIS_ERR_BAD_HANDLE;
        goto agent_rsp;
    }

    error = immModel_searchInitialize(cb, &(evt->info.searchInit), &searchOp);

    /*Generate search-id */
    if(error == SA_AIS_OK) {
        IMMND_OM_SEARCH_NODE* sn = calloc(1, sizeof(IMMND_OM_SEARCH_NODE));
        if( sn == NULL) {
            send_evt.info.imma.info.searchInitRsp.error = SA_AIS_ERR_NO_MEMORY;
            goto agent_rsp;
        }

        sn->searchId = cb->cli_id_gen++; /* separate count for search*/
        sn->searchOp = searchOp;         /*TODO: wraparround */
        sn->next = cl_node->searchOpList;
        cl_node->searchOpList = sn;
        send_evt.info.imma.info.searchInitRsp.searchId = sn->searchId;
     
        if(((SaImmSearchOptionsT)evt->info.searchInit.searchOptions) & 
            SA_IMM_SEARCH_PERSISTENT_ATTRS) {/*=>DUMP/BACKUP*/
            if(cb->mCanBeCoord) {
                if(immModel_immNotWritable(cb)) {
                    LOG_WA("Cannot allow official dump/backup when imm-sync "
                           "is in progress");
                    /*TODO: DROP THE search OP !. currently leaking.*/
                    error = SA_AIS_ERR_TRY_AGAIN;
                } else {
                    TRACE_2("SEARCH INIT tentatively adj epoch to "
                        ":%u & adnnounce dump", cb->mMyEpoch+1);
                    immnd_announceDump(cb); 
                }
            } else {
                /* Handle it as a regular search OP. */
                LOG_WA("Dump of IMM not at controller is not an offical dump");
            }
        }

    }

 agent_rsp:
    send_evt.info.imma.info.searchInitRsp.error = error;
    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt);
    return rc;
}

void
search_req_continue(
                    IMMND_CB *cb, 
                    IMMSV_OM_RSP_SEARCH_REMOTE* reply, 
                    SaUint32T reqConn)
{
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_OM_RSP_SEARCH_NEXT* rsp = NULL;
    IMMSV_ATTR_VALUES_LIST *oldRsp, *fetchedRsp;
    IMMND_OM_SEARCH_NODE* sn = NULL;
    SaAisErrorT err = reply->result;
    TRACE_ENTER();
    assert(reply->requestNodeId == cb->node_id);
    memset(&send_evt,'\0', sizeof(IMMSV_EVT));
    send_evt.type=IMMSV_EVT_TYPE_IMMA;
    IMMSV_HANDLE tmp_hdl;
    tmp_hdl.nodeId = cb->node_id;
    tmp_hdl.count = reqConn;

    /*Look up client-node*/
    immnd_client_node_get(cb, *((SaImmHandleT *)  &tmp_hdl), &cl_node);
    if (cl_node == NULL) {
        LOG_ER("IMMND - Client Node Get Failed for cli_hdl:%llu", 
            *((SaImmHandleT *)  &tmp_hdl));
        /*Cant do anything*/
        goto leave;
    }

    TRACE_2("SEARCH NEXT, Look for id:%u", reply->searchId);

    sn = cl_node->searchOpList;
    while(sn) {
        if(sn->searchId == reply->searchId) {break;}
        sn = sn->next;
    }

    if(!sn) {
        LOG_ER("Could not find search node for search-ID:%u", reply->searchId);
        if(err == SA_AIS_OK) {err = SA_AIS_ERR_LIBRARY;}
    }

    immModel_fetchLastResult(sn->searchOp, &rsp);
    immModel_clearLastResult(sn->searchOp);

    if(err != SA_AIS_OK) {
        goto agent_rsp;
    }

    if(!rsp) {
        LOG_ER("Part of continuation lost!");
        err = SA_AIS_ERR_LIBRARY;  /*not the proper error code..*/
        goto agent_rsp;
    }

    TRACE_2("Last result found, add current runtime values");
    assert(strncmp((const char *) rsp->objectName.buf, 
               (const char *) reply->runtimeAttrs.objectName.buf, 
               rsp->objectName.size) == 0);
    //Iterate through reply->runtimeAttrs.attrValuesList
    //Update value in rsp->attrValuesList

    fetchedRsp =  reply->runtimeAttrs.attrValuesList;
    while(fetchedRsp) { //For each fetched rt-attr-value
        IMMSV_ATTR_VALUES *att = &(fetchedRsp->n);
        TRACE_2("Looking for attribute:%s", att->attrName.buf);

        oldRsp = rsp->attrValuesList;
        while(oldRsp) { //Find the old value
            IMMSV_ATTR_VALUES* att2 = &(oldRsp->n);
            TRACE_2("Mattching against:%s", att2->attrName.buf);
            if(att->attrName.size == att2->attrName.size &&
                (strncmp((const char *) att->attrName.buf,
                    (const char *) att2->attrName.buf,
                    att->attrName.size) == 0)) 
            {//We have a match
                TRACE_2("MATCH FOUND nrof old values:%lu", att2->attrValuesNumber);
                assert(att->attrValueType == att2->attrValueType);
                //delete the current old value & set attrValuesNumber to zero.
                //if reply value has attrValuesNumber > 0 then assign this fetched value.
                //Note: dont forget AttrMoreValues !
                if(att2->attrValuesNumber) {
                    immsv_evt_free_att_val(&att2->attrValue, att2->attrValueType);

                    if(att2->attrValuesNumber > 1) {
                        IMMSV_EDU_ATTR_VAL_LIST *att2multi = att2->attrMoreValues;
                        do {
                            IMMSV_EDU_ATTR_VAL_LIST *tmp;
                            assert(att2multi); //must be at least one extra value
                            immsv_evt_free_att_val(&att2multi->n, att2->attrValueType);
                            tmp = att2multi;
                            att2multi = att2multi->next;
                            tmp->next = NULL;
                            free(tmp);
                        } while(att2multi);
                        att2->attrValuesNumber = 0;
                        att2->attrMoreValues = NULL;
                    } //multis deleted

                } //old value in rsp deleted.

                TRACE_2("OLD VALUES DELETED nrof new values:%lu", att->attrValuesNumber);

                if(att->attrValuesNumber) {//there is a new value, copy it.
                    att2->attrValue = att->attrValue; //steals any buffer.
                    att2->attrValuesNumber = att->attrValuesNumber;
                    att->attrValue.val.x.buf = NULL; //ensure stolen buffer is not freed twice.
                    att->attrValue.val.x.size = 0;
                }

                if(att->attrValuesNumber > 1) { //multivalued attribute
                    att2->attrMoreValues = att->attrMoreValues; //steal the whole list
                    att->attrMoreValues = NULL;
                }
                att->attrValuesNumber = 0;

                break; //out of inner loop, since we already matched this fetched value
            }
            oldRsp = oldRsp->next;
        }
        fetchedRsp = fetchedRsp->next;
    }

 agent_rsp:
    if(err == SA_AIS_OK) {
        send_evt.info.imma.type = IMMA_EVT_ND2A_SEARCHNEXT_RSP;
        send_evt.info.imma.info.searchNextRsp = rsp;
    } else { /*err != SA_AIS_OK*/
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error = err;
    }

    if(immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt) != NCSCC_RC_SUCCESS) {
        LOG_WA("Could not send reply to agent for search-next continuaton");
    }

    if(rsp)
    {
        freeSearchNext(rsp, TRUE);
    }
  
 leave:
    TRACE_LEAVE();
}


/****************************************************************************
 * Name          : immnd_evt_proc_oi_att_pull_rpl
 *
 * Description   : Function to process the reply from the invocation
 *                 of the SaImmOiRtAttrUpdateCallbackT.
 *                 Note that this is a call local to the ND (does not arrive
 *                 over FEVS).
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - sender info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_oi_att_pull_rpl(IMMND_CB *cb,
    IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT        send_evt;
    SaAisErrorT      err;
    uns32            rc = NCSCC_RC_SUCCESS;
    SaBoolT          isLocal;
    void* searchOp=NULL;
    TRACE_ENTER();
    memset(&send_evt,'\0', sizeof(IMMSV_EVT));
    IMMSV_OM_RSP_SEARCH_REMOTE* rspo = &send_evt.info.immnd.info.rspSrchRmte;
    rspo->remoteNodeId = evt->info.rtAttUpdRpl.sr.remoteNodeId;
    rspo->searchId = evt->info.rtAttUpdRpl.sr.searchId;
    rspo->requestNodeId = evt->info.rtAttUpdRpl.sr.requestNodeId;
    rspo->runtimeAttrs.objectName = evt->info.rtAttUpdRpl.sr.objectName; /* borrowing buf*/
    err = evt->info.rtAttUpdRpl.result;

    assert(evt->info.rtAttUpdRpl.sr.remoteNodeId == cb->node_id);
    isLocal = evt->info.rtAttUpdRpl.sr.requestNodeId == cb->node_id;
    //Note stack allocated search-initialize struct with members
    //shallow copied to refer into the search-remote struct.
    //No heap allocations! Dont explicitly delete anything of this struct
    //ImmsvOmSearchInit reqo;
    if(err == SA_AIS_OK) {
        //Fetch latest runtime values from the object. Use search/accessor.
        IMMSV_OM_SEARCH_INIT reqo;
        memset(&reqo,'\0', sizeof(IMMSV_OM_SEARCH_INIT));
        reqo.rootName.buf = evt->info.rtAttUpdRpl.sr.objectName.buf; /*borrowing*/
        reqo.rootName.size = evt->info.rtAttUpdRpl.sr.objectName.size;
        reqo.scope = SA_IMM_ONE;
        reqo.searchOptions = SA_IMM_SEARCH_ONE_ATTR | 
            SA_IMM_SEARCH_GET_SOME_ATTR;
        reqo.searchParam.present = ImmOmSearchParameter_PR_NOTHING;
        reqo.attributeNames = evt->info.rtAttUpdRpl.sr.attributeNames; /*borrowing.*/
        TRACE_2("Fetching the folowing rt attributes after implementer has updated");
        while(reqo.attributeNames) {
            TRACE_2("attrname: %s", reqo.attributeNames->name.buf);
            reqo.attributeNames = reqo.attributeNames->next;
        }
        reqo.attributeNames = evt->info.rtAttUpdRpl.sr.attributeNames; /*borrowing. */

        TRACE_2("oi_att_pull_rpl Before searchInit");
        err = immModel_searchInitialize(cb, &reqo, &searchOp);
        if(err == SA_AIS_OK) {
            TRACE_2("oi_att_pull_rpl searchInit returned OK, calling searchNext");
            IMMSV_OM_RSP_SEARCH_NEXT* rsp = 0;
            err = immModel_nextResult(cb, searchOp, &rsp, NULL, NULL, NULL, NULL);
            if(err == SA_AIS_OK) {
                rspo->runtimeAttrs.attrValuesList = rsp->attrValuesList;/*STEALING*/
                rsp->attrValuesList=NULL;

                immModel_clearLastResult(searchOp);
                freeSearchNext(rsp, TRUE);
            } else {
                LOG_WA("Internal IMM server problem - failure from internal nextResult: %u", err);
                err = SA_AIS_ERR_NO_RESOURCES;	
            }
        } else {
            LOG_ER("Internal IMM server problem - failure from internal searchInit: %u", err);
            err = SA_AIS_ERR_NO_RESOURCES;
        }
    }
    if(err == SA_AIS_ERR_FAILED_OPERATION)
    {
        /* We can get FAILED_OPERATION here as response from implementer,
           which is not an allowed return code towards client/agent in the searchNext..
           So what to convert it to ? We will go for NO RESOURCES in the lack of anything
           better.
        */
        err = SA_AIS_ERR_NO_RESOURCES;
    }

    rspo->result = err; 

    TRACE_2("oi_att_pull_rpl searchNext OK");
    //We have the updated values fetched in "rsp", or an error indication.

    if(isLocal) {
        TRACE_2("Originating client is local");
        //Case (B) Fetch request continuation locally.
        SaUint32T reqConn;
        ImmsvInvocation invoc;
        invoc.inv = evt->info.rtAttUpdRpl.sr.searchId;
        invoc.owner = evt->info.rtAttUpdRpl.sr.remoteNodeId;
        immModel_fetchSearchReqContinuation(cb, *((SaInvocationT *) &invoc), 
            &reqConn);
        TRACE_2("FETCHED SEARCH REQ CONTINUATION FOR %u|%u->%u",
            invoc.inv, invoc.owner, reqConn);
        if(!reqConn) {
            LOG_WA("Failed to retrieve search continuation, client died ?");
            //Cant do anything but just drop it.
            goto deleteSearchOp;
        } 
        TRACE_2("Build local reply to agent");
        search_req_continue(cb, rspo, reqConn);
    } else {
        TRACE_2("Originating client is remote");
        //Case (C) Fetch implementer continuation to get reply_dest.
        MDS_DEST reply_dest;
        immModel_fetchSearchImplContinuation(cb, rspo->searchId, rspo->requestNodeId, &reply_dest);
        if(!reply_dest) {
            LOG_WA("Continuation for remote rt attribute fetch was lost, can not complete fetch");
            //Just drop it. The search should have been, or will be, aborted by a time-out.
            goto deleteSearchOp;
        } else {
            TRACE_2("Send the fetched result back to request node");
            send_evt.type=IMMSV_EVT_TYPE_IMMND;
            send_evt.info.immnd.type = IMMND_EVT_ND2ND_SEARCH_REMOTE_RSP;

            rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMND, reply_dest, &send_evt);
            if(rc != NCSCC_RC_SUCCESS) {
                LOG_ER("Problem in replying to peer IMMND over MDS. "
                    "Abandoning searchNext fetch remote rt attrs.");
                goto deleteSearchOp;
            }
        }
    }
  
 deleteSearchOp:
    if(searchOp) {
        //immModel_nextResult(cb, searchOp, &rsp, NULL, NULL, NULL, NULL);
        immModel_deleteSearchOp(searchOp);
    }

    if(rspo->runtimeAttrs.attrValuesList)
    {
        rspo->runtimeAttrs.objectName.buf = NULL; /* was borrowed.*/
        rspo->runtimeAttrs.objectName.size = 0;
        freeSearchNext(&rspo->runtimeAttrs, FALSE);
    }
    TRACE_2("oi_att_pull_rpl finalize of internal search OK");
    TRACE_LEAVE();
    return rc;
}

static void freeAttrNameList(IMMSV_ATTR_NAME_LIST* attn)
{
    TRACE_ENTER();
    IMMSV_ATTR_NAME_LIST* tmpa=attn;
    while(attn)
    {
        TRACE_2("Free attr:%s", attn->name.buf);
        free(attn->name.buf);
        attn->name.buf=NULL;
        attn->name.size=0;
        attn=attn->next;
        tmpa->next=NULL;
        free(tmpa);
        tmpa=attn;
    }
    TRACE_LEAVE();
}

static void freeSearchNext(IMMSV_OM_RSP_SEARCH_NEXT* rsp, SaBoolT freeTop)
{
    IMMSV_ATTR_VALUES_LIST* al=NULL;
    TRACE_ENTER();

    TRACE_2("objectName:%s", rsp->objectName.buf);
    free(rsp->objectName.buf);
    rsp->objectName.buf=NULL;

    al = rsp->attrValuesList;
    while(al)
    {
        free(al->n.attrName.buf);
        al->n.attrName.buf=NULL;
        if(al->n.attrValuesNumber) 
        {
            immsv_evt_free_att_val(&al->n.attrValue, al->n.attrValueType);
            if(al->n.attrValuesNumber > 1) 
            {
                IMMSV_EDU_ATTR_VAL_LIST *att2multi = al->n.attrMoreValues;
                do {
                    IMMSV_EDU_ATTR_VAL_LIST *tmp;
                    assert(att2multi); /* must be at least one extra value */
                    immsv_evt_free_att_val(&att2multi->n, al->n.attrValueType);
                    tmp = att2multi;
                    att2multi = att2multi->next;
                    tmp->next = NULL;
                    free(tmp);
                } while (att2multi);
                al->n.attrValuesNumber = 0;
                al->n.attrMoreValues = NULL;
            } //multis deleted
        }

        al = al->next;
    }
    rsp->attrValuesList = NULL;
    if(freeTop)
    {/*Top object was also heap allocated. */
        free(rsp);
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_search_next
 *
 * Description   : Function to process the SaImmOmSearchNext call.
 *                 Note that this is a read, local to the ND (does not go 
 *                 over FEVS).
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - sender info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_search_next(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    SaAisErrorT             error;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    IMMND_OM_SEARCH_NODE* sn = NULL;
    SaImmOiHandleT          implHandle=0LL;
    IMMSV_HANDLE            tmp_hdl;
    IMMSV_ATTR_NAME_LIST* rtAttrsToFetch = NULL;
    SaUint32T implConn = 0;
    SaUint32T implNodeId =0;
    IMMSV_OM_RSP_SEARCH_NEXT* rsp = NULL;
    MDS_DEST implDest =0LL;

    TRACE_ENTER();

    memset(&send_evt,'\0', sizeof(IMMSV_EVT));

    /*Look up client-node*/
    immnd_client_node_get(cb, evt->info.searchOp.client_hdl, &cl_node);
    if (cl_node == NULL) {
        LOG_ER("IMMND - Client Node Get Failed for cli_hdl:%llu",
            evt->info.searchOp.client_hdl);
        error = SA_AIS_ERR_BAD_HANDLE;
        goto agent_rsp;
    }
    TRACE_2("SEARCH NEXT, Look for id:%u", evt->info.searchOp.searchId);

    sn = cl_node->searchOpList;
    while(sn) {
        if(sn->searchId == evt->info.searchOp.searchId) {break;}
        sn = sn->next;
    }

    if(!sn) {
        LOG_ER("Could not find search node for search-ID:%u",
            evt->info.searchOp.searchId);
        error = SA_AIS_ERR_BAD_HANDLE;
        goto agent_rsp;
    }

    error = immModel_nextResult(cb, sn->searchOp, &rsp, &implConn, 
        &implNodeId, &rtAttrsToFetch, &implDest);
    if(error != SA_AIS_OK) {
        goto agent_rsp;
    }

    /*Three cases:
      A) No runtime attributes to fetch.
      Reply directly with this call.
      B) Runtime attributes to fetch and the implementer is local.
      => (store continuation?) and call on implementer.
      C) Runtime attributes to fetch, but the implementer is remote
      => store continuation and use request remote invoc over
      either FEVS or direct adressing to peer ND.
    */

    if(implNodeId) { /* Case B or C */
        TRACE_2("There is an implementer and there are pure rt's !");
        assert(rtAttrsToFetch);
        /*rsp kept in ImmSearchOp to be used in continuation.
          rtAttrsToFetch is expected to be deleted by this layer after used.
	  
          rsp needs to be appended by the replies.
          ((attrsToFetch needs to be checked/matched when replies received.))
          If the (non cached runtime) attribute is ALSO persistent, then
          the value should be stored (exactly as cached). The only difference
          with respect to officially cached is how the attribute is fetched.
        */

        if(implConn) {
            TRACE_2("The implementer is local");
            /*Case B Invoke rtUpdateCallback directly. */
            assert(implNodeId == cb->node_id);
            send_evt.type=IMMSV_EVT_TYPE_IMMA;
            send_evt.info.imma.type = IMMA_EVT_ND2A_SEARCH_REMOTE;
            IMMSV_OM_SEARCH_REMOTE* rreq = &send_evt.info.imma.info.searchRemote;
            rreq->remoteNodeId = implNodeId;
            rreq->searchId = sn->searchId;
            rreq->requestNodeId = cb->node_id; /* NOTE: obsolete ?*/
            rreq->objectName = rsp->objectName;  /* borrow objectName.buf */
            rreq->attributeNames = rtAttrsToFetch; /* borrow this structure */

            tmp_hdl.nodeId = implNodeId;
            tmp_hdl.count = implConn;

            implHandle = *((SaImmOiHandleT *) &tmp_hdl);

            /*Fetch client node for OI !*/
            immnd_client_node_get(cb, implHandle, &oi_cl_node);
            if(oi_cl_node == NULL) {
                /*Client died ?*/
                LOG_WA("Implementer died");
                error = SA_AIS_ERR_TRY_AGAIN;
                goto agent_rsp;
            } else {
                rreq->client_hdl = implHandle;
                TRACE_2("MAKING OI-IMPLEMENTER rtUpdate upcall towards agent");
                if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                       oi_cl_node->agent_mds_dest,
                       &send_evt) != NCSCC_RC_SUCCESS) {
                    LOG_WA("Agent upcall over MDS for rtUpdate failed");
                    error = SA_AIS_ERR_NO_RESOURCES;
                    goto agent_rsp;
                }
            }
        } else {
            TRACE_2("The implementer is remote");
            /*Case C Send the message directly to nd where implementer resides.*/
            send_evt.type=IMMSV_EVT_TYPE_IMMND;
            send_evt.info.immnd.type = IMMND_EVT_ND2ND_SEARCH_REMOTE;
            IMMSV_OM_SEARCH_REMOTE* rreq = &send_evt.info.immnd.info.searchRemote;
            rreq->remoteNodeId = implNodeId;
            rreq->searchId = sn->searchId;
            rreq->requestNodeId = cb->node_id; /* NOTE: obsolete ?*/
            rreq->objectName = rsp->objectName;  /* borrowed buf;*/
            rreq->attributeNames = rtAttrsToFetch; /* borrowed attrList*/
       
            TRACE_2("FORWARDING TO OTHER ND!");

            rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMND, implDest, &send_evt);
            if(rc != NCSCC_RC_SUCCESS) {
                LOG_ER("Problem in sending to peer IMMND over MDS. "
                    "Aborting searchNext.");
                error = SA_AIS_ERR_NO_RESOURCES;
                goto agent_rsp;
            }
        }

        /*Register request continuation !*/
        ImmsvInvocation invoc;
        invoc.inv = sn->searchId;
        invoc.owner = implNodeId;
        tmp_hdl = *((IMMSV_HANDLE *) &(evt->info.searchOp.client_hdl));

        TRACE_2("SETTING SEARCH REQ CONTINUATION FOR %u|%x->%u",
            sn->searchId, implNodeId, tmp_hdl.count);

        immModel_setSearchReqContinuation(cb, *((SaInvocationT *) &invoc), 
            tmp_hdl.count);

        cl_node->tmpSinfo = *sinfo; //TODO should be part of continuation?

        assert(error == SA_AIS_OK);
        TRACE_LEAVE();
        return NCSCC_RC_SUCCESS;
    }
    /*Case A*/
    immModel_clearLastResult(sn->searchOp);
    /*Simply sets the saved pointer to NULL
      Does NOT free the ImmOmRspSearchNext structure as it is used in the 
      direct reply below.
    */

 agent_rsp:

    send_evt.type=IMMSV_EVT_TYPE_IMMA;

    if(error == SA_AIS_OK) {
        send_evt.info.imma.type = IMMA_EVT_ND2A_SEARCHNEXT_RSP;
        send_evt.info.imma.info.searchNextRsp = rsp;
    } else {
        send_evt.info.imma.type =  IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error = error;
    }

    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt);

    if(rsp) { freeSearchNext(rsp, TRUE); }

    if(rtAttrsToFetch) { freeAttrNameList(rtAttrsToFetch); }

    TRACE_LEAVE();
    return rc;
}


/****************************************************************************
 * Name          : immnd_evt_proc_search_finalize
 *
 * Description   : Function to process the SaImmOmSearchFinalize call.
 *                 Note that this is a read, local to the ND (does not go 
 *                 over FEVS).
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - sender info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
immnd_evt_proc_search_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMND_OM_SEARCH_NODE* sn = NULL;
    IMMND_OM_SEARCH_NODE** prev = NULL;
    IMMSV_OM_RSP_SEARCH_NEXT* rsp = NULL;
    TRACE_ENTER();

    memset(&send_evt,'\0', sizeof(IMMSV_EVT));
    send_evt.type=IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

    /*Look up client-node*/
    immnd_client_node_get(cb, evt->info.searchOp.client_hdl, &cl_node);
    if ( cl_node == NULL ) {
        LOG_ER("IMMND - Client Node Get Failed for cli_hdl:%llu",
            evt->info.searchOp.client_hdl);
        send_evt.info.imma.info.errRsp.error = SA_AIS_ERR_BAD_HANDLE;
        goto agent_rsp;
    }
    TRACE_2("SEARCH FINALIZE, Look for id:%u", evt->info.searchOp.searchId);

    sn = cl_node->searchOpList;
    prev = &(cl_node->searchOpList);
    while(sn) {
        if(sn->searchId == evt->info.searchOp.searchId) {
            *prev = sn->next;
            break;
        }
        prev = &(sn->next);
        sn = sn->next;
    }

    if(!sn) {
        LOG_ER("Could not find search node for search-ID:%u",
            evt->info.searchOp.searchId);
        send_evt.info.imma.info.errRsp.error = SA_AIS_ERR_BAD_HANDLE;
        goto agent_rsp;
    }


    immModel_fetchLastResult(sn->searchOp, &rsp);
    immModel_clearLastResult(sn->searchOp);
    if(rsp)
    {
        TRACE_2("ABT This branch needs testing - ENTER");
        freeSearchNext(rsp, TRUE);
        TRACE_2("ABT This branch needs testing - EXIT");
    }

    immModel_deleteSearchOp(sn->searchOp);
    sn->searchOp = NULL;
    sn->next=NULL;
    free(sn);
    send_evt.info.imma.info.errRsp.error = SA_AIS_OK;
 agent_rsp:

    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt);
    TRACE_LEAVE();
    return rc;
}


/****************************************************************************
 * Name          : immnd_evt_proc_class_desc_get
 *
 * Description   : Function to process the SaImmOmClassDescriptionGet call.
 *                 Note that this is a read, local to the ND (does not go 
 *                 over FEVS).
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - sender info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_class_desc_get(IMMND_CB *cb,
    IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    SaAisErrorT             error;

    memset(&send_evt,'\0', sizeof(IMMSV_EVT));

    TRACE_2("className:%s", evt->info.classDescr.className.buf);
    send_evt.type=IMMSV_EVT_TYPE_IMMA;

    error = immModel_classDescriptionGet(cb, &(evt->info.classDescr.className),
        &(send_evt.info.imma.info.classDescr));
    if(error == SA_AIS_OK) {
        send_evt.info.imma.type = IMMA_EVT_ND2A_CLASS_DESCR_GET_RSP;
    } else {
        send_evt.info.imma.type=IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error=error;
    }

    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt);
    /*Deallocate the extras on the send_message*/
    IMMSV_ATTR_DEF_LIST* p = send_evt.info.imma.info.classDescr.attrDefinitions;
    while(p) {
        IMMSV_ATTR_DEF_LIST* prev = p;
        if(p->d.attrName.size) {
            free(p->d.attrName.buf);/*Free-Y*/
            p->d.attrName.buf=NULL;
        }
        if(p->d.attrDefaultValue) {
            if((p->d.attrValueType == SA_IMM_ATTR_SASTRINGT) ||
                (p->d.attrValueType == SA_IMM_ATTR_SAANYT) ||
                (p->d.attrValueType == SA_IMM_ATTR_SANAMET)) {
                if(p->d.attrDefaultValue->val.x.size) {             /*Free-ZZ*/
                    free(p->d.attrDefaultValue->val.x.buf);
                    p->d.attrDefaultValue->val.x.buf=NULL;
                    p->d.attrDefaultValue->val.x.size=0;
                }
            }
            free(p->d.attrDefaultValue); /*Free-Z*/
            p->d.attrDefaultValue=NULL;
        }
        p = p->next;
        free(prev); /*Free-X*/
    }
    send_evt.info.imma.info.classDescr.attrDefinitions=NULL;
    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_imm_finalize
 *
 * Description   : Function to process the SaImmOmFinalize and SaImmOiFinalize
 *                 from IMM Agents.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT isOm - TRUE=> OM finalize, FALSE => OI finalize.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_imm_finalize(IMMND_CB *cb, IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo, SaBoolT isOm)
{
    /* Note: parameter isOm is ignored, should be noted in cl_node*/
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;


    memset(&send_evt,'\0', sizeof(IMMSV_EVT));

    TRACE_2("finalize for handle: %llu", evt->info.finReq.client_hdl);
    immnd_client_node_get(cb,evt->info.finReq.client_hdl,&cl_node);
    if ( cl_node == NULL ) {
        LOG_ER("IMMND - Client Node Get Failed for cli_hdl %llu",
            evt->info.finReq.client_hdl);
        send_evt.info.imma.info.errRsp.error=SA_AIS_ERR_BAD_HANDLE;
        goto agent_rsp;
    }
 
    immnd_proc_imma_discard_connection(cb, cl_node);

    rc = immnd_client_node_del(cb,cl_node);
    if (rc == NCSCC_RC_FAILURE) {
        LOG_ER("IMMND - Client Tree Del Failed , cli_hdl");
        send_evt.info.imma.info.errRsp.error=SA_AIS_ERR_LIBRARY;
    }

    free(cl_node);
    
    send_evt.info.imma.info.errRsp.error=SA_AIS_OK;

 agent_rsp:
    send_evt.type=IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type=IMMA_EVT_ND2A_IMM_FINALIZE_RSP;
    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt);
    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_admowner_init
 *
 * Description   : Function to process the SaImmOmAdminOwnerInitialize
 *                 from Applications
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_admowner_init(IMMND_CB *cb, IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    SaImmHandleT            client_hdl;

    memset(&send_evt,'\0',sizeof(IMMSV_EVT));

    client_hdl=evt->info.adminitReq.client_hdl;
    immnd_client_node_get(cb,client_hdl,&cl_node);
    if(cl_node == NULL) {
        LOG_ER("IMMND - Client %llu went down so no response", client_hdl);
        return rc; 
    } 

    if(!immnd_is_immd_up(cb)) {
        send_evt.info.imma.info.admInitRsp.error= SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(immModel_immNotWritable(cb)) { /*Avoid broadcasting doomed requests.*/
        send_evt.info.imma.info.admInitRsp.error= SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(cb->messages_pending > IMMND_FEVS_MAX_PENDING){
        send_evt.info.imma.info.admInitRsp.error= SA_AIS_ERR_NO_RESOURCES;
        goto agent_rsp;
    }

    /*aquire a ND sender count and send the fevs to ND (without waiting)*/
    cb->messages_pending++;  /*flow control*/

    send_evt.type=IMMSV_EVT_TYPE_IMMD;
    send_evt.info.immd.type=IMMD_EVT_ND2D_ADMINIT_REQ;
    send_evt.info.immd.info.admown_init.client_hdl = 
        evt->info.adminitReq.client_hdl;

    send_evt.info.immd.info.admown_init.i = evt->info.adminitReq.i;

    /* send the request to the IMMD, reply comes back over fevs.*/

    rc = immnd_mds_msg_send(cb,NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, 
        &send_evt);

    if (rc != NCSCC_RC_SUCCESS) {
        LOG_ER("IMMND - AdminOwner Initialize Failed");
        send_evt.info.imma.info.admInitRsp.error = SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    } 

    /*Save sinfo in continuation. 
      Note: should set up a wait time for the continuation roughly in line
      with IMMSV_WAIT_TIME.*/
    cl_node->tmpSinfo = *sinfo; //TODO should be part of continuation ?

    return rc;

 agent_rsp:
    send_evt.type = IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ADMINIT_RSP;
   
    assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
    TRACE_2("immnd_evt:imm_adminOwner_init: SENDRSP FAIL %u ",
        send_evt.info.imma.info.admInitRsp.error);
    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 

    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_impl_set
 *
 * Description   : Function to process the SaImmOiImplementerSet
 *                 from Applications
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_impl_set(IMMND_CB *cb, IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    SaImmHandleT            client_hdl;

    memset(&send_evt,'\0',sizeof(IMMSV_EVT));

    client_hdl=evt->info.implSet.client_hdl;
    immnd_client_node_get(cb,client_hdl,&cl_node);
    if(cl_node == NULL) {
        LOG_ER("IMMND - Client went down so no response");
        return rc; 
    } 

    if(!immnd_is_immd_up(cb)) {
        send_evt.info.imma.info.implSetRsp.error= SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(immModel_immNotWritable(cb)) { /*Avoid broadcasting doomed requests.*/
        send_evt.info.imma.info.implSetRsp.error= SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(cb->messages_pending > IMMND_FEVS_MAX_PENDING){
        send_evt.info.imma.info.implSetRsp.error= SA_AIS_ERR_NO_RESOURCES;
        goto agent_rsp;
    }

    cb->messages_pending++;  /*flow control*/

    send_evt.type=IMMSV_EVT_TYPE_IMMD;
    send_evt.info.immd.type=IMMD_EVT_ND2D_IMPLSET_REQ;
    send_evt.info.immd.info.impl_set.r.client_hdl = client_hdl;
    send_evt.info.immd.info.impl_set.r.impl_name.size = 
        evt->info.implSet.impl_name.size;
    send_evt.info.immd.info.impl_set.r.impl_name.buf = 
        evt->info.implSet.impl_name.buf; /*Warning re-using buffer, no copy.*/
    send_evt.info.immd.info.impl_set.reply_dest = cb->immnd_mdest_id;
   

    /* send the request to the IMMD, reply comes back over fevs.*/

    rc = immnd_mds_msg_send(cb,NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, 
        &send_evt);

    send_evt.info.immd.info.impl_set.r.impl_name.size = 0;
    send_evt.info.immd.info.impl_set.r.impl_name.buf = NULL;  /*precaution.*/

    if (rc != NCSCC_RC_SUCCESS) {
        LOG_ER("Problem in sending to IMMD over MDS");
        send_evt.info.imma.info.implSetRsp.error = SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    } 

    /*Save sinfo in continuation. 
      Note should set up a wait time for the continuation roughly in line
      with IMMSV_WAIT_TIME.
    */
    cl_node->tmpSinfo = *sinfo; //TODO: should be part of continuation?

    return rc;

 agent_rsp:
    send_evt.type = IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_IMPLSET_RSP;
   
    assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
    TRACE_2("SENDRSP FAIL %u ", send_evt.info.imma.info.implSetRsp.error);
    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 

    return rc;
}


/****************************************************************************
 * Name          : immnd_evt_proc_ccb_init
 *
 * Description   : Function to process the SaImmOmCcbInitialize
 *                 from Applications
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_ccb_init(IMMND_CB *cb, IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    SaImmHandleT            client_hdl;
    TRACE_ENTER();

    memset(&send_evt,'\0',sizeof(IMMSV_EVT));

    client_hdl=evt->info.ccbinitReq.client_hdl;
    immnd_client_node_get(cb,client_hdl,&cl_node);
    if(cl_node == NULL) {
        LOG_ER("IMMND - Client went down so no response");
        TRACE_LEAVE();
        return rc; 
    } 

    if(!immnd_is_immd_up(cb)) {
        send_evt.info.imma.info.ccbInitRsp.error= SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(immModel_immNotWritable(cb)) { /*Avoid broadcasting doomed requests.*/
        send_evt.info.imma.info.ccbInitRsp.error= SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(cb->messages_pending > IMMND_FEVS_MAX_PENDING){
        send_evt.info.imma.info.ccbInitRsp.error= SA_AIS_ERR_NO_RESOURCES;
        goto agent_rsp;
    }

    cb->messages_pending++;  /*flow control*/

    send_evt.type=IMMSV_EVT_TYPE_IMMD;
    send_evt.info.immd.type=IMMD_EVT_ND2D_CCBINIT_REQ;
    send_evt.info.immd.info.ccb_init = evt->info.ccbinitReq;

    /* send the request to the IMMD, reply comes back over fevs.*/

    TRACE_2("Sending CCB init msg to IMMD AO:%u FL:%llx client:%llu", 
        send_evt.info.immd.info.ccb_init.adminOwnerId,
        send_evt.info.immd.info.ccb_init.ccbFlags,
        send_evt.info.immd.info.ccb_init.client_hdl);
    rc = immnd_mds_msg_send(cb,NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, 
        &send_evt);

    if (rc != NCSCC_RC_SUCCESS) {
        LOG_ER("Problem in sending ro IMMD over MDS");
        send_evt.info.imma.info.ccbInitRsp.error = SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    } 

    /*Save sinfo in continuation. 
      Note should set up a wait time for the continuation roughly in line
      with IMMSV_WAIT_TIME.*/
    cl_node->tmpSinfo = *sinfo; //TODO should be part of continuation?
    TRACE_LEAVE();
    return rc;

 agent_rsp:
    send_evt.type = IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_CCBINIT_RSP;
   
    assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
    TRACE_2("SENDRSP FAIL %u ", send_evt.info.imma.info.ccbInitRsp.error);
    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 

    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_rt_update
 *
 * Description   : Function to process the saImmOiRtObjectUpdate/_2
 *                 from local agent.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 immnd_evt_proc_rt_update(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    SaAisErrorT             err = SA_AIS_OK;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    SaImmHandleT            client_hdl;
    IMMSV_HANDLE            tmp_hdl;
    TRACE_ENTER();
    unsigned int isPureLocal = 1;

    client_hdl=evt->info.objModify.immHandle;
    immnd_client_node_get(cb,client_hdl,&cl_node);
    if(cl_node == NULL) {
        LOG_ER("IMMND - Client went down so no response");
        TRACE_LEAVE();
        return rc; 
    } 

    /* Do broadcast checks BEFORE updating model because we dont want
       to do the update and then fail to propagate it if it should be 
       propagated. Dowsnide: Even pure local rtattrs can not be updated if
       we can not communicate with the IMMD. */

    if(!immnd_is_immd_up(cb)) {
        err = SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(cb->messages_pending > IMMND_FEVS_MAX_PENDING){
        LOG_ER("Too many backloged fevs messages (> %u)", 
            IMMND_FEVS_MAX_PENDING);
        err = SA_AIS_ERR_NO_RESOURCES;
        goto agent_rsp;
    }

    tmp_hdl = *((IMMSV_HANDLE *) &client_hdl);

    err = immModel_rtObjectUpdate(cb, &(evt->info.objModify),
        tmp_hdl.count,
        tmp_hdl.nodeId,
        &isPureLocal);


    if(!isPureLocal && (err == SA_AIS_OK)) {
        TRACE_2("immnd_evt_proc_rt_update was not pure local, i.e. cached RT attrs");
        /*This message is allowed even when imm not writable. But isPureLocal should
          never be false if we are not writable, thus we should never get to this
          code branch if imm is not writbale.
         */

        cb->messages_pending++;  /*flow control*/

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type=IMMSV_EVT_TYPE_IMMD;
        send_evt.info.immd.type=IMMD_EVT_ND2D_OI_OBJ_MODIFY;
        send_evt.info.immd.info.objModify = evt->info.objModify;
        /* Borrow pointer structures. */
        /* send the request to the IMMD, reply comes back over fevs.*/

        if(!immnd_is_immd_up(cb)) {
            err = SA_AIS_ERR_TRY_AGAIN;
            goto agent_rsp;
        }

        rc = immnd_mds_msg_send(cb,NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, 
            &send_evt);

        if (rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Problem in sending to IMMD over MDS");
            err = SA_AIS_ERR_TRY_AGAIN;
            goto agent_rsp;
        } 

        /*Save sinfo in continuation. 
          Note should set up a wait time for the continuation roughly in line
          with IMMSV_WAIT_TIME.*/
        cl_node->tmpSinfo = *sinfo; //TODO should be part of continuation?
        goto done;
    }

    TRACE_2("immnd_evt_proc_rt_update was pure local, i.e. pure RT attrs");
    /* Here is where we should end up if the saImmOiRtObjectUpdate_2 call was
       made because of an SaImmOiRtAttrUpdateCallbackT upcall. In that case
       we are pulling values for pure (i.e. local) runtime attributes. 
       If the saImmOiRtObjectUpdate_2 call is a push call from the implementer
       then the attributes are expected to be cached, i.e. non-local. */

 agent_rsp:

    memset(&send_evt,'\0',sizeof(IMMSV_EVT));
    TRACE_2("send immediate reply to client/agent");
    send_evt.type = IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.info.errRsp.error= err;
    send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;   
    assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
    TRACE_2("SENDRSP RESULT %u ", err);
    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 

 done:
    free(evt->info.objModify.objectName.buf);
    evt->info.objModify.objectName.buf=NULL;
    evt->info.objModify.objectName.size=0;
    immsv_free_attrmods(evt->info.objModify.attrMods);
    evt->info.objModify.attrMods=NULL;
    TRACE_LEAVE();
    return rc;
}




#if 0 /*Only for debug*/
static void
dump_usrbuf(USRBUF* ub)
{
    if(ub) {
        TRACE_2("ABT IMMND dump_usrbuf: ");
        TRACE_2("next:%p ", ub->next);
        TRACE_2("link:%p ", ub->link);
        TRACE_2("count:%u ", ub->count);
        TRACE_2("start:%u ", ub->start);
        TRACE_2("pool_ops:%p ", ub->pool_ops);
        TRACE_2("specific.opaque:%u ", ub->specific.opaque);
        TRACE_2("payload:%p", ub->payload);
    } else {
        TRACE_2("EMPTY", ub->start);
    }
}
#endif

/****************************************************************************
 * Name          : immnd_evt_proc_fevs_forward
 *
 * Description   : Function to handle (forward) fevs messages.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *****************************************************************************/
static uns32 immnd_evt_proc_fevs_forward(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    IMMSV_EVT               send_evt;
    uns32                   rc = NCSCC_RC_SUCCESS;
    SaAisErrorT             error;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    SaImmHandleT            client_hdl;

    TRACE_2("sender_count: %llu size: %u ", evt->info.fevsReq.sender_count,
        evt->info.fevsReq.msg.size);


    client_hdl=evt->info.fevsReq.client_hdl;
    immnd_client_node_get(cb,client_hdl,&cl_node);
    if(cl_node == NULL) {
        LOG_ER("IMMND - Client %llu went down so no response", client_hdl);
        error = SA_AIS_ERR_BAD_HANDLE;
        goto agent_rsp;
    } 

    memset(&send_evt,'\0',sizeof(IMMSV_EVT));

    if(!immnd_is_immd_up(cb)){
        LOG_ER("IMMND - Director Service Is Down");
        error= SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    /*sender_count set to 1 if we are to check locally for writability
      before sending to IMMD. This to avoid broadcasting requests that 
      are doomed anyway.  */
    if(evt->info.fevsReq.sender_count && immModel_immNotWritable(cb)) {
        error = SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    }

    if(cb->messages_pending > IMMND_FEVS_MAX_PENDING) {
        LOG_WA("Too many FEVS messages outstanding towards IMMD, rejecting "
            "this message");
        error= SA_AIS_ERR_NO_RESOURCES;
        goto agent_rsp;
    }

    cb->messages_pending++;  /*flow control*/

    send_evt.type=IMMSV_EVT_TYPE_IMMD;
    send_evt.info.immd.type=IMMD_EVT_ND2D_FEVS_REQ;
    send_evt.info.immd.info.fevsReq.reply_dest = cb->immnd_mdest_id;
    send_evt.info.immd.info.fevsReq.client_hdl = client_hdl;
    send_evt.info.immd.info.fevsReq.msg.size = evt->info.fevsReq.msg.size;
    /*Borrow the octet buffer from the input message instead of copying*/
    send_evt.info.immd.info.fevsReq.msg.buf = evt->info.fevsReq.msg.buf;

    /* send the request to the IMMD */

    TRACE_2("SENDING FEVS TO IMMD");
    rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMD, cb->immd_mdest_id, 
        &send_evt);

    if (rc != NCSCC_RC_SUCCESS) {
        LOG_ER("Problem in sending to IMMD over MDS");     
        error = SA_AIS_ERR_TRY_AGAIN;
        goto agent_rsp;
    } 

    /*Save sinfo in continuation. 
      Note should set up a wait time for the continuation roughly in line
      with IMMSV_WAIT_TIME.*/
    assert(rc == NCSCC_RC_SUCCESS);
    cl_node->tmpSinfo = *sinfo; //TODO: should be part of continuation?

    /*                             Only a single continuation per client 
                                   possible, but not when op is async!
                                   This is where the ND sender count is needed.
                                   But we should be able to use the agents
                                   count/continuation-id .. ?*/
    return rc;/*Normal return here => asyncronous reply in fevs_rcv.*/

 agent_rsp:

    send_evt.type = IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
    send_evt.info.imma.info.errRsp.error = error;
   
    assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
    TRACE_2("SENDRSP FAIL %u ", send_evt.info.imma.info.admInitRsp.error);
    rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 

    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_ccb_obj_modify_rsp
 *
 * Description   : Function to process the reply from the 
 *                 SaImmOiCcbObjectModifyCallbackT upcall. 
 *                 Note this call arrived over fevs, to ALL IMMNDs.
 *                 This is because ALL must receive ACK/NACK from EACH 
 *                 implementer.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : None.
 *
 *****************************************************************************/
static void immnd_evt_proc_ccb_obj_modify_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32        rc = NCSCC_RC_SUCCESS;
    IMMSV_EVT    send_evt;
    SaUint32T    reqConn=0;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    TRACE_ENTER();


    immModel_ccbObjModifyContinuation(cb, 
        evt->info.ccbUpcallRsp.ccbId,
        evt->info.ccbUpcallRsp.inv, 
        evt->info.ccbUpcallRsp.result,
        &reqConn);

    if(reqConn) {
        IMMSV_HANDLE tmp_hdl;
        tmp_hdl.nodeId = cb->node_id;
        tmp_hdl.count = reqConn;

        immnd_client_node_get(cb, *((SaImmHandleT *) &tmp_hdl), &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            TRACE_LEAVE();
            return;/*Note, this means that regardles of ccb outcome, 
                     we can not reply to the process that started the ccb.*/
        }
        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client. AND dont forget time-outs.*/
        sinfo = &cl_node->tmpSinfo;

        TRACE_2("SENDRSP %u", evt->info.ccbUpcallRsp.result);

        if(evt->info.ccbUpcallRsp.result != SA_AIS_OK)
        {
            evt->info.ccbUpcallRsp.result = SA_AIS_ERR_FAILED_OPERATION;
        }

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= evt->info.ccbUpcallRsp.result;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        assert(cl_node->tmpSinfo.stype == MDS_SENDTYPE_SNDRSP);

        rc = immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS rc:%u", rc);
        }
    }
				    
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_ccb_obj_create_rsp
 *
 * Description   : Function to process the reply from the 
 *                 SaImmOiCcbObjectCreateCallbackT upcall. 
 *                 Note this call arrived over fevs, to ALL IMMNDs.
 *                 This is because ALL must receive ACK/NACK from EACH 
 *                 implementer.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : None.
 *
 *****************************************************************************/
static void immnd_evt_proc_ccb_obj_create_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32        rc = NCSCC_RC_SUCCESS;
    IMMSV_EVT    send_evt;
    SaUint32T    reqConn=0;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    TRACE_ENTER();


    immModel_ccbObjCreateContinuation(cb, 
        evt->info.ccbUpcallRsp.ccbId,
        evt->info.ccbUpcallRsp.inv, 
        evt->info.ccbUpcallRsp.result,
        &reqConn);

    if(reqConn) {
        IMMSV_HANDLE tmp_hdl;
        tmp_hdl.nodeId = cb->node_id;
        tmp_hdl.count = reqConn;

        immnd_client_node_get(cb, *((SaImmHandleT *) &tmp_hdl), &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            TRACE_LEAVE();
            return;/*Note, this means that regardles of ccb outcome, 
                     we can not reply to the process that started the ccb.*/
        }
        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client. AND dont forget time-outs.*/
        sinfo = &cl_node->tmpSinfo;

        TRACE_2("SENDRSP %u", evt->info.ccbUpcallRsp.result);

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        if(evt->info.ccbUpcallRsp.result != SA_AIS_OK)
        {
            evt->info.ccbUpcallRsp.result = SA_AIS_ERR_FAILED_OPERATION;
        }
        send_evt.info.imma.info.errRsp.error= evt->info.ccbUpcallRsp.result;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        assert(cl_node->tmpSinfo.stype == MDS_SENDTYPE_SNDRSP);

        rc = immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS rc:%u",
                rc);
        }
    }
				    
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_ccb_obj_delete_rsp
 *
 * Description   : Function to process the reply from the 
 *                 SaImmOiCcbObjectDeleteCallbackT upcall. 
 *                 Note this call arrived over fevs, to ALL IMMNDs.
 *                 This is because ALL must receive ACK/NACK from EACH 
 *                 implementer.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : None.
 *
 *****************************************************************************/
static void immnd_evt_proc_ccb_obj_delete_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32        rc = NCSCC_RC_SUCCESS;
    IMMSV_EVT    send_evt;
    SaUint32T    reqConn=0;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    TRACE_ENTER();

    immModel_ccbObjDelContinuation(cb, &(evt->info.ccbUpcallRsp), &reqConn);

    SaAisErrorT err;

    if(!immModel_ccbWaitForDeleteImplAck(cb, evt->info.ccbUpcallRsp.ccbId, &err) 
        && reqConn) {
        IMMSV_HANDLE tmp_hdl;
        tmp_hdl.nodeId = cb->node_id;
        tmp_hdl.count = reqConn;

        immnd_client_node_get(cb, *((SaImmHandleT *) &tmp_hdl), &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            TRACE_LEAVE();
            return;/*Note, this means that regardles of ccb outcome, 
                     we can not reply to the process that started the ccb.*/
        }
        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client. AND dont forget time-outs.*/
        sinfo = &cl_node->tmpSinfo;

        TRACE_2("SENDRSP %u", evt->info.ccbUpcallRsp.result);

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= evt->info.ccbUpcallRsp.result;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        assert(cl_node->tmpSinfo.stype == MDS_SENDTYPE_SNDRSP);

        rc = immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS rc:%u",
                rc);
        }
    }
				    
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_ccb_compl_rsp
 *
 * Description   : Function to process the reply from the 
 *                 SaImmOiCcbCompletedCallbackT upcall. 
 *                 Note this call arrived over fevs, to ALL IMMNDs.
 *                 This is because ALL must receive ACK/NACK from EACH 
 *                 implementer.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : None.
 *
 *****************************************************************************/
static void immnd_evt_proc_ccb_compl_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32            rc = NCSCC_RC_SUCCESS;
    IMMSV_EVT        send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    SaAisErrorT             err;
    SaUint32T        reqConn=0;
    TRACE_ENTER();

    immModel_ccbCompletedContinuation(cb, &(evt->info.ccbUpcallRsp), 
        &reqConn);

    if(!immModel_ccbWaitForCompletedAck(cb,evt->info.ccbUpcallRsp.ccbId, 
           &err)) {
        /*Finished waiting for completed Acks*/
        if(err == SA_AIS_OK) { /*Proceed with commit*/
            /*If we arrive here, the assumption is that all implementors have agreed
              to commit and all immnds are prepared to commit this ccb. Fevs must
              guarantee that all have seen the same replies from implementers.*/
            TRACE_2("DELAYED COMMIT TAKING EFFECT");
            SaUint32T* implConnArr=NULL;
            SaUint32T  arrSize=0;

            immModel_ccbCommit(cb, evt->info.ccbUpcallRsp.ccbId, &arrSize, 
                &implConnArr);
            if(arrSize) {
                int ix;
                IMMSV_HANDLE tmp_hdl;
                tmp_hdl.nodeId = cb->node_id; /*1*/
                memset(&send_evt,'\0',sizeof(IMMSV_EVT));
                send_evt.type = IMMSV_EVT_TYPE_IMMA;
                send_evt.info.imma.type = IMMA_EVT_ND2A_OI_CCB_APPLY_UC;

                for(ix=0; ix<arrSize && err==SA_AIS_OK; ++ix) {
                    /* Send apply callbacks for all implementers at this node involved
                       with the ccb. Re-use evt*/	

                    /*Look up the client node for the implementer, using implConn*/
                    tmp_hdl.count = implConnArr[ix];/*2*/
                    /*Fetch client node for OI !*/
                    immnd_client_node_get(cb, *((SaImmOiHandleT *) &tmp_hdl), 
                        &oi_cl_node);
                    if(oi_cl_node == NULL) {
                        /*Implementer client died ?*/
                        LOG_WA("IMMND - Client went down so can not send apply - "
                            "ignoring!");
                    } else {
                        /*GENERATE apply message to implementer. */
                        send_evt.info.imma.info.ccbCompl.ccbId = 
                            evt->info.ccbUpcallRsp.ccbId;
                        send_evt.info.imma.info.ccbCompl.implId = 
                            evt->info.ccbUpcallRsp.implId;
                        send_evt.info.imma.info.ccbCompl.invocation = 
                            evt->info.ccbUpcallRsp.inv;
                        send_evt.info.imma.info.ccbCompl.immHandle = 
                            *((SaImmOiHandleT *) &tmp_hdl);
                        TRACE_2("MAKING IMPLEMENTER CCB APPLY upcall");
                        if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                               oi_cl_node->agent_mds_dest,
                               &send_evt) != NCSCC_RC_SUCCESS) {
                            LOG_WA("IMMND CCB APPLY: CCB APPLY UPCALL FAILED - ignoring.");
                        }
                    }
                }//for
                free(implConnArr);
            }//if(arrSize
        } else { /*err != SA_AIS_OK => generate SaImmOiCcbAbortCallbackT upcall
                   for all local implementers involved in the Ccb*/
            immnd_evt_ccb_abort(cb, evt->info.ccbUpcallRsp.ccbId, SA_FALSE, NULL);
        }
        /* Either commit or abort has been decided. Ccb is now done.
           If we are at originating request node, then we ALWAYS reply here. 
           No acks to wait for from apply callbacks and we know we have no more
           completed callback replyes pending.
        */

        if(reqConn) {
            IMMSV_HANDLE tmp_hdl;
            tmp_hdl.nodeId = cb->node_id;
            tmp_hdl.count = reqConn;

            immnd_client_node_get(cb, *((SaImmHandleT *) &tmp_hdl), &cl_node);
            if(cl_node == NULL) {
                LOG_ER("IMMND - Client went down so no response");
                TRACE_LEAVE();
                return; /*Note, this means that regardles of ccb outcome, 
                          we can not reply to the process that started the ccb.*/
            }

            /*Asyncronous agent calls can cause more than one continuation to be
              present for the SAME client. AND dont forget time-outs.*/
            sinfo = &cl_node->tmpSinfo;  

            memset(&send_evt,'\0',sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMA;
            send_evt.info.imma.info.errRsp.error= err;
            send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
            assert(cl_node->tmpSinfo.stype == MDS_SENDTYPE_SNDRSP);

            rc = immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt); 
            if(rc != NCSCC_RC_SUCCESS) {
                LOG_ER("Failed to send response to agent/client over MDS rc:%u",rc);
            }
        }
        TRACE_2("CCB COMPLETED: TERMINATING CCB:%u", evt->info.ccbUpcallRsp.ccbId);
        err = immModel_ccbDelete(cb, evt->info.ccbUpcallRsp.ccbId);
        if(err != SA_AIS_OK) {
            LOG_ER("Failure in termination of CCB:%u - ignoring!", 
                evt->info.ccbUpcallRsp.ccbId);
            /* There is really not much we can do here. */
        }
    }//if(!immModel_ccbWait
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_rt_update_pull
 *
 * Description   : Function to process a remote uppcall
 *                 of SaImmOiRtAttrUpdateCallbackT
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_rt_update_pull(IMMND_CB *cb, IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    uns32                   rc = NCSCC_RC_SUCCESS;
    SaUint32T               implConn=0;
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMSV_HANDLE            tmp_hdl;
    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    SaImmOiHandleT          implHandle=0LL;

    TRACE_ENTER();
    memset(&send_evt,'\0', sizeof(IMMSV_EVT));
    send_evt.type=IMMSV_EVT_TYPE_IMMA;


    IMMSV_OM_SEARCH_REMOTE* req = &evt->info.searchRemote;
    assert(cb->node_id == req->remoteNodeId);
    implConn = immModel_findConnForImplementerOfObject(cb, &req->objectName);


    if(implConn) {
        /*Send the upcall to imma*/
        send_evt.info.imma.type = IMMA_EVT_ND2A_SEARCH_REMOTE;
        IMMSV_OM_SEARCH_REMOTE* rreq = &send_evt.info.imma.info.searchRemote;
        rreq->remoteNodeId = req->remoteNodeId;
        rreq->searchId = req->searchId;
        rreq->requestNodeId = req->requestNodeId;
        rreq->objectName = req->objectName; /* borrow, objectname.buf freed in immnd_evt_destroy */
        rreq->attributeNames = req->attributeNames; /* borrow, freed in immnd_evt_destroy */

        tmp_hdl.nodeId = cb->node_id; /* == req->remoteNodeId */
        tmp_hdl.count = implConn;

        implHandle = *((SaImmOiHandleT *) &tmp_hdl);

        /*Fetch client node for OI !*/
        immnd_client_node_get(cb, implHandle, &oi_cl_node);
        if(oi_cl_node == NULL) {
            /*Client died ?*/
            LOG_WA("Implementer exists in ImmModel but not in client tree.");
            err = SA_AIS_ERR_TRY_AGAIN;
        } else {
            rreq->client_hdl = implHandle;
            TRACE_2("MAKING OI-IMPLEMENTER rtUpdate upcall towards agent");
            if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                   oi_cl_node->agent_mds_dest,
                   &send_evt) != NCSCC_RC_SUCCESS) {
                LOG_WA("Agent upcall over MDS for rtUpdate failed");
                err = SA_AIS_ERR_NO_RESOURCES;
            } else {
                /*The upcall was sent. Register a search implementer continuation!*/
                immModel_setSearchImplContinuation(cb, req->searchId, req->requestNodeId, sinfo->dest);
            }
        }
    } else {
        LOG_WA("Could not locate implementer for %s, detached?", req->objectName.buf);
        err = SA_AIS_ERR_TRY_AGAIN; /*Perhaps not the clearest return value*/
    }

    if(err != SA_AIS_OK) {
        /*
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error = err;
        */

        LOG_WA("Missing implementation of error case, error:%u, handled by timeout instead", err);

        /* TODO: something was wrong => immediate reply to avoid block & timeout
        of client. See pull reply.
        */
        /*
        char buffer[BUFSZ];
        size_t resLength = sizeof(struct res_header);
        struct res_header* tmpMessage = (struct res_header *) buffer;
        tmpMessage->error = err;
        tmpMessage->id = MESSAGE_EVS_RT_ATTRUPDATE_REPLY;

        ImmOmRspSearchRemote rspo;
        rspo.remoteNodeId = req->remoteNodeId;
        rspo.searchId = req->searchId;
        rspo.requestNodeId = req->requestNodeId;
        rspo.runtimeAttrs.objectName.buf = req->objectName.buf;
        rspo.runtimeAttrs.objectName.size = req->objectName.size;
        rspo.runtimeAttrs.attrValuesList.list.count = 0;
        rspo.runtimeAttrs.attrValuesList.list.size = 0;
        rspo.runtimeAttrs.attrValuesList.list.free = 0;

        assert(multicastMessage(conn, tmpMessage, tmpMessage->size) == SA_AIS_OK);
        */

    }


    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_remote_search_rsp
 *
 * Description   : Function to process the result of a remote search
 *                 for fetching rt attrs from remote implementer
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 
immnd_evt_proc_remote_search_rsp(IMMND_CB *cb, IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    TRACE_ENTER();
    IMMSV_OM_RSP_SEARCH_REMOTE* rspo = &evt->info.rspSrchRmte;
    assert(cb->node_id == rspo->requestNodeId);
    /*Fetch originating request continuation*/
    ImmsvInvocation invoc;
    invoc.inv =  rspo->searchId;
    invoc.owner = rspo->remoteNodeId;
    SaUint32T reqConn;
    immModel_fetchSearchReqContinuation(cb, *((SaInvocationT *) &invoc), 
        &reqConn);

    TRACE_2("FETCHED SEARCH REQ CONTINUATION FOR %u|%u->%u",
        invoc.inv, invoc.owner, reqConn);

    if(!reqConn) {
        LOG_WA("Failed to retrieve search continuation for %llu, client died ?",
            *((SaUint64T *) &invoc));
        //Cant do anything but just drop it.
        goto leave;
    }

    TRACE_2("Build reply to agent");
    search_req_continue(cb, rspo, reqConn);

 leave:
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_evt_proc_admop_rsp
 *
 * Description   : Function to process the saImmOiAdminOperationResult
 *                 for both sync and async invocations
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT  async - true=> original client call was async
 *                 SaBoolT  local - true=>local upcall from OI,
 *                                  false=>remote forward ND->ND.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_admop_rsp(IMMND_CB *cb, IMMND_EVT *evt,
    IMMSV_SEND_INFO *sinfo,
    SaBoolT async,
    SaBoolT local)
{
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    uns32                   rc = NCSCC_RC_SUCCESS;
    SaImmHandleT            oi_client_hdl;
    SaUint32T implConn=0;
    SaUint32T reqConn=0;
    SaUint64T reply_dest=0LL;

    memset(&send_evt,'\0',sizeof(IMMSV_EVT));

    if(local) {
        oi_client_hdl=evt->info.admOpRsp.oi_client_hdl;
        immnd_client_node_get(cb,oi_client_hdl,&oi_cl_node);
        if(oi_cl_node == NULL) {
            LOG_ER("IMMND - Client %llu went down so no response", oi_client_hdl);
            return rc; /*Note: Error is ignored. Nothing else we can do since
                         OI reply call was async and we cant find original OM call.
                       */
        }
    }

    /*Removed this since code changed to generate direct call ND->ND
      if(!immnd_is_immd_up(cb)) {
      send_evt.info.imma.info.errRsp.error= SA_AIS_ERR_TRY_AGAIN;
      return rc; 
      }
    */

    immModel_fetchAdmOpContinuations(cb, evt->info.admOpRsp.invocation, 
        local, &implConn, &reqConn, &reply_dest);

    TRACE_2("invocation:%llu, result:%u\n impl:%u req:%u dest:%llu me:%llu",
        evt->info.admOpRsp.invocation,
        evt->info.admOpRsp.result,
        implConn,
        reqConn,
        reply_dest,
        cb->immnd_mdest_id);

    if(local && !implConn) {
        /*Continuation with implementer lost => connection probably lost.
          What to do? Do I really need this check? Yes because a faulty
          application may invoke this call when it should not.
          I need a timer associated with the implementer continuation anyway..
        */
        LOG_WA("Received reply from implementer yet continuation lost - "
            "ignoring");
        return rc;
    }

    if(reqConn) {
        /*Original Request was local, send reply.*/
        IMMSV_HANDLE tmp_hdl;
        tmp_hdl.nodeId = cb->node_id;
        tmp_hdl.count = reqConn;

        immnd_client_node_get(cb, *((SaImmHandleT *) &tmp_hdl), &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return rc;/*Note: Error is ignored! should cause bad-handle..if local*/
        }

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_ADMOP_RSP;
        send_evt.info.imma.info.admOpRsp.invocation = 
            evt->info.admOpRsp.invocation;
        send_evt.info.imma.info.admOpRsp.result = 
            evt->info.admOpRsp.result;
        send_evt.info.imma.info.admOpRsp.error = 
            evt->info.admOpRsp.error;


        if(async) { 
            TRACE_2("NORMAL ASYNCRONOUS reply %llu %u %u to OM",
                send_evt.info.imma.info.admOpRsp.invocation,
                send_evt.info.imma.info.admOpRsp.result,
                send_evt.info.imma.info.admOpRsp.error);
            rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OM, 
                cl_node->agent_mds_dest, &send_evt);
            TRACE_2("ASYNC REPLY SENT rc:%u", rc);
        } else {
            TRACE_2("NORMAL syncronous reply %llu %u to OM",
                send_evt.info.imma.info.admOpRsp.invocation,
                send_evt.info.imma.info.admOpRsp.result);
            rc = immnd_mds_send_rsp(cb, &(cl_node->tmpSinfo), &send_evt); 
            TRACE_2("SYNC REPLY SENT rc:%u", rc);
        }
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failure in sending reply for admin-op over MDS");
        }
    } else if(reply_dest) {
        /*Forward reply to relevant ND.*/
        send_evt.type=IMMSV_EVT_TYPE_IMMND;
        send_evt.info.immnd.type=(async)?IMMND_EVT_ND2ND_ASYNC_ADMOP_RSP:
            IMMND_EVT_ND2ND_ADMOP_RSP;
        send_evt.info.immnd.info.admOpRsp.invocation = 
            evt->info.admOpRsp.invocation;
        send_evt.info.immnd.info.admOpRsp.result = evt->info.admOpRsp.result;
        send_evt.info.immnd.info.admOpRsp.error = evt->info.admOpRsp.error;
        send_evt.info.immnd.info.admOpRsp.oi_client_hdl = 0LL;
    
        TRACE_2("FORWARDING TO OTHER ND!");

        rc = immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMND, reply_dest, 
   			&send_evt);

        if (rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Problem in sending to peer IMMND over MDS. "
                "Discarding admin op reply.");     
        }
    } else {
        LOG_WA("Continuation for AdminOp was lost, probably due to timeout");
    }

    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_admop
 *
 * Description   : Function to process the saImmOmAdminOperationInvoke
 *                 or saImmAdminOperationInvokeAsync operation
 *                 from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_admop(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             error = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    SaImmOiHandleT          implHandle=0LL;
    IMMSV_HANDLE            tmp_hdl = *((IMMSV_HANDLE *) &clnt_hdl);
    SaUint32T conn = tmp_hdl.count;
    SaUint32T implConn=0;
    NCS_NODE_ID implNodeId=0;
    SaBoolT async=SA_FALSE;

    async = (evt->type == IMMND_EVT_A2ND_IMM_ADMOP_ASYNC);
    assert(evt->type == IMMND_EVT_A2ND_IMM_ADMOP || async);
    TRACE_ENTER();
    TRACE_1(async?"ASYNC ADMOP":"SYNC ADMOP");

    /*
      Global uniqueness for the invocation is guaranteed by the globally
      unique adminOwnerId which is owned by ONE client connection.
      The client side increments an invocation counter unique for that
      process (thread safe). 
    */

    ImmsvInvocation invoc; 
    invoc.inv = evt->info.admOpReq.invocation;
    invoc.owner = evt->info.admOpReq.adminOwnerId;
    SaInvocationT saInv = *((SaInvocationT *) &invoc);


    error = immModel_adminOperationInvoke(cb, &(evt->info.admOpReq),
        originatedAtThisNd?conn:0,
        (SaUint64T) reply_dest,
        saInv,
        &implConn, 
        &implNodeId);

    /*Check if we have an implementer, if so forward the message.
      If there is no implementer then implNodeId is zero.
      in that case 'error' has been set to SA_AIS_ERR_NOT_EXIST.
    */

    if(error == SA_AIS_OK && implConn) {
        /*Implementer exists and is local, make the up-call!*/

        assert(implNodeId == cb->node_id);
        tmp_hdl.nodeId = implNodeId;
        tmp_hdl.count = implConn;

        implHandle = *((SaImmOiHandleT *) &tmp_hdl);

        /*Fetch client node for OI !*/
        immnd_client_node_get(cb, implHandle, &oi_cl_node);
        if(oi_cl_node == NULL) {
            LOG_ER("IMMND - Client %llu went down so no response", implHandle);
            error = SA_AIS_ERR_NOT_EXIST;
        } else {
            memset(&send_evt,'\0',sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMA;
            send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ADMOP;
            send_evt.info.imma.info.admOpReq.adminOwnerId = 
                evt->info.admOpReq.adminOwnerId;
            send_evt.info.imma.info.admOpReq.operationId =
                evt->info.admOpReq.operationId;

            /* TODO: This is a bit ugly, using the continuationId to transport
              the immOiHandle.*/
            send_evt.info.imma.info.admOpReq.continuationId = implHandle;

            send_evt.info.imma.info.admOpReq.invocation = 
                evt->info.admOpReq.invocation; 

            send_evt.info.imma.info.admOpReq.timeout =
                evt->info.admOpReq.timeout;/*why send tmout to client ??*/

            send_evt.info.imma.info.admOpReq.objectName.size =
                evt->info.admOpReq.objectName.size;

            /*Warning, borowing pointer to string, dont delete at this end!*/
            send_evt.info.imma.info.admOpReq.objectName.buf =
                evt->info.admOpReq.objectName.buf;

            /*Warning, borowing pointer to params list, dont delete at this end!*/
            send_evt.info.imma.info.admOpReq.params =
                evt->info.admOpReq.params; 

            TRACE_2("IMMND sending Agent upcall");
            if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                   oi_cl_node->agent_mds_dest,
                   &send_evt) != NCSCC_RC_SUCCESS) {
                LOG_ER("MDS SEND FAILED");	
                error = SA_AIS_ERR_LIBRARY;
            } else {
                TRACE_2("IMMND UPCALL TO AGENT SEND SUCCEEDED");
            }
        }
    }

    /*Take care of possible immediate reply, when errors already occurred*/
    /* Possibly move this code down inside the error!=OK branch. */
    if(originatedAtThisNd) {  
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /* TODO: Match against continuation.
           Asyncronous agent calls can cause more than one continuation to be
           present for the SAME client. But is this a problem ?*/
 
        if(error != SA_AIS_OK) {
            TRACE_2("Send immediate reply to client");
            memset(&send_evt,'\0',sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMA;
      
            if(async) {
                invoc.inv = evt->info.admOpReq.invocation;
                invoc.owner = evt->info.admOpReq.adminOwnerId;
                saInv = *((SaInvocationT *) &invoc);
                send_evt.info.imma.type = IMMA_EVT_ND2A_ADMOP_RSP;
                send_evt.info.imma.info.admOpRsp.invocation = saInv;
                send_evt.info.imma.info.admOpRsp.error = error;

                TRACE_2("IMMND sending error response to adminOperationInvokeAsync");
                if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OM, 
                       cl_node->agent_mds_dest,
                       &send_evt) != NCSCC_RC_SUCCESS) {
                    LOG_ER("MDS SEND FAILED on response to AdminOpInvokeAsync");	
                }
            } else {
                IMMSV_SEND_INFO* sinfo = &cl_node->tmpSinfo; 
                assert(sinfo); 
                assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
                send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
                send_evt.info.imma.info.errRsp.error= error;

                if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
                    LOG_ER("MDS SEND FAILED");
                }
            }
        } else {
            TRACE_2("Delayed reply, wait for reply from implemener");
            /*Implementer may be on another node.*/
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_class_create
 *
 * Description   : Function to process the saImmOmClassCreate/_2
 *                 operation from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_class_create(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             error = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;

#if 0  /*ABT DEBUG PRINTOUTS START*/
    TRACE_2("ABT immnd_evt_proc_class_create:%s",
        evt->info.classDescr.className.buf);
    TRACE_2("ABT classCategory: %u ",
        evt->info.classDescr.classCategory);
    TRACE_2("ABT attrDefs:%p ", 
        evt->info.classDescr.attrDefinitions);

    IMMSV_ATTR_DEF_LIST* p = evt->info.classDescr.attrDefinitions;

    while(p) {
        TRACE_2("ABT ATTRDEF, NAME: %s type:%u", 
            p->d.attrName.buf, p->d.attrValueType);
        if(p->d.attrDefaultValue) { 
            printImmValue(p->d.attrValueType, p->d.attrDefaultValue);
        }
        p = p->next;
    }
#endif  /*ABT DEBUG PRINTOUTS STOP*/

    error = immModel_classCreate(cb, &(evt->info.classDescr));

    if(originatedAtThisNd) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client %llu went down so no response", clnt_hdl);
            return;
        } else {
            sinfo = &cl_node->tmpSinfo; 
        }

        TRACE_2("Send immediate reply to client");
        if(error == SA_AIS_ERR_EXIST || error == SA_AIS_ERR_TRY_AGAIN) {
            /*Check if this is from a sync-agent and if so reverse logic in reply*/
            if(cl_node->mIsSync) {
                error = SA_AIS_OK;
            }
        }

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= error;
        assert(sinfo); /*fake for test*/
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            TRACE_2("immnd_evt_class_create: SENDRSP FAIL");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_class_delete
 *
 * Description   : Function to process the saImmOmClassDelete
 *                 operation from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                         originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_class_delete(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             error = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    TRACE_ENTER();

    TRACE_2("immnd_evt_proc_class_delete:%s",
        evt->info.classDescr.className.buf);
    error = immModel_classDelete(cb, &(evt->info.classDescr));

    if(originatedAtThisNd) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client %llu went down so no response", clnt_hdl);
            return;
        } else {
            sinfo = &cl_node->tmpSinfo; 
        }

        TRACE_2("Send immediate reply to client");

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= error;
        assert(sinfo); /*fake for test*/
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_WA("immnd_evt_class_delete: SENDRSP FAILED TO SEND");
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_sync_finalize
 *
 * Description   : Function to process the ImmOmObjectFinalizeSync call
 *                 operation from sync-process(loader)
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - sender info
 * Return Values : None
 *
 *****************************************************************************/

static uns32 immnd_evt_proc_sync_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)
{
    SaAisErrorT err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    SaImmHandleT            client_hdl;
    uns32 proc_rc;
    char* tmpData = NULL;  
    NCS_UBAID uba;
    uba.start=NULL;
    TRACE_ENTER();

    client_hdl=evt->info.finReq.client_hdl;
    immnd_client_node_get(cb, client_hdl, &cl_node);
    if(cl_node == NULL) {
        LOG_ER("IMMND - Client went down so no response");
        return 0; 
    } 

    if(cl_node->mIsSync) { 
        assert(cb->mIsCoord);
        memset(&send_evt, 0, sizeof(IMMSV_EVT)); /*No pointers=>no leak*/
        send_evt.type = IMMSV_EVT_TYPE_IMMND;
        send_evt.info.immnd.type = IMMND_EVT_ND2ND_SYNC_FINALIZE;
        err = immModel_finalizeSync(cb, &send_evt.info.immnd.info.finSync, 
            SA_TRUE, SA_FALSE);
        if(err != SA_AIS_OK) {
            LOG_ER("Failed to encode IMMND finalize sync message");
            /*assert(0);*/ /*If we fail in sync then restart the IMMND sync client.*/
        } else {
            /*Pack message for fevs.*/
            proc_rc = ncs_enc_init_space(&uba);
            if(proc_rc != NCSCC_RC_SUCCESS) {
                TRACE_2("Failed init ubaid");
                uba.start=NULL;
                err = SA_AIS_ERR_NO_RESOURCES;
                goto fail;
            }

            proc_rc = immsv_evt_enc(&send_evt, &uba);
            if(proc_rc != NCSCC_RC_SUCCESS) {
                TRACE_2("Failed encode fevs");
                uba.start=NULL;
                err = SA_AIS_ERR_NO_RESOURCES;
                goto fail;
            }

            int32 size = uba.ttl;
            /* TODO: check against "payload max-size" ? */
            tmpData = malloc(size);
            assert(tmpData);
            char* data = m_MMGR_DATA_AT_START(uba.start, size, tmpData);

            immnd_evt_destroy(&send_evt, SA_FALSE, __LINE__);

            memset(&send_evt, 0, sizeof(IMMSV_EVT)); /*No pointers=>no leak*/
            send_evt.type = IMMSV_EVT_TYPE_IMMND;
            send_evt.info.immnd.type = 0;
            send_evt.info.immnd.info.fevsReq.client_hdl = client_hdl;
            send_evt.info.immnd.info.fevsReq.reply_dest = cb->immnd_mdest_id;
            send_evt.info.immnd.info.fevsReq.msg.size = size;
            send_evt.info.immnd.info.fevsReq.msg.buf = data;

            proc_rc = immnd_evt_proc_fevs_forward(cb, &send_evt.info.immnd, sinfo);
            if(proc_rc != NCSCC_RC_SUCCESS) {
                TRACE_2("Failed send fevs message"); /*Error already logged in fevs_fo*/
                uba.start=NULL;
                err = SA_AIS_ERR_NO_RESOURCES;
            }
      
            free(tmpData);
        }
    } else {
        LOG_ER("Will not allow sync messages from any process except sync "
            "process");
        err = SA_AIS_ERR_BAD_OPERATION;
    }

 fail:
    memset(&send_evt,'\0',sizeof(IMMSV_EVT));
    send_evt.type = IMMSV_EVT_TYPE_IMMA;
    send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
    send_evt.info.imma.info.errRsp.error= err;

    if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
        LOG_ER("Send response over MDS failed");
    }
    TRACE_LEAVE();
    return proc_rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_object_sync
 *
 * Description   : Function to process the ImmOmObjectSync call
 *                 operation from sync-process(loader)
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMND_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                       is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *                                       Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_object_sync(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    TRACE_ENTER();

    if(cb->mSync) { /*This node is being synced => Accept the sync message.*/
        err = immModel_objectSync(cb, &(evt->info.obj_sync));
        if(err != SA_AIS_OK) {
            LOG_ER("Failed to apply IMMND sync message");
            assert(0); /*If we fail in sync then restart the IMMND sync client.*/
        }
    } else {
        /* TODO: I should verify that the class & object exists, 
           at least the object.*/
    }

    if(originatedAtThisNd) {
        /*Send ack to client (sync process)*/

        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        } 

        sinfo = &cl_node->tmpSinfo; 

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= err;
        assert(sinfo); 
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send result %u to Agent over MDS", err);
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_rt_object_create
 *
 * Description   : Function to process the saImmOiRtObjectCreate/_2
 *                 operation from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                       is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_rt_object_create(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_HANDLE            tmp_hdl = *((IMMSV_HANDLE *) &clnt_hdl);
    TRACE_ENTER();

    if(originatedAtThisNd) {
        err = immModel_rtObjectCreate(cb, &(evt->info.objCreate),
            tmp_hdl.count,
            tmp_hdl.nodeId);
    } else {
        err = immModel_rtObjectCreate(cb, &(evt->info.objCreate), 
            0, 
            tmp_hdl.nodeId);
    }

    if(originatedAtThisNd) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        sinfo = &cl_node->tmpSinfo; 

        TRACE_2("send immediate reply to client/agent");
        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= err;
        assert(sinfo); 
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send result to implementer over MDS");
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_object_create
 *
 * Description   : Function to process the saImmOmCcbObjectCreate/_2
 *                 operation from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_object_create(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;

    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    SaImmOiHandleT          implHandle=0LL;
    IMMSV_HANDLE            tmp_hdl; 
    SaUint32T implConn=0;
    NCS_NODE_ID implNodeId=0;
    SaUint32T continuationId=0;
    SaBoolT delayedReply = SA_FALSE;
    TRACE_ENTER();

#if 0  /*ABT DEBUG PRINTOUTS START*/
    TRACE_2("ABT immnd_evt_proc_object_create class:%s",
        evt->info.objCreate.className.buf);
    TRACE_2("ABT immnd_evt_proc_object_create parent:%s",
        evt->info.objCreate.parentName.buf);
    TRACE_2("ABT immnd_evt_proc_object_create CCB:%u ADMOWN:%u",
        evt->info.objCreate.ccbId,
        evt->info.objCreate.adminOwnerId);

    TRACE_2("ABT attrvalues:%p ", 
        evt->info.objCreate.attrValues);

    IMMSV_ATTR_VALUES_LIST* p = evt->info.objCreate.attrValues;

    while(p) {
        TRACE_2("ABT FOUND AN ATTRIBUTE, NAME: %s type:%u vlaues:%u", 
            p->n.attrName.buf, p->n.attrValueType,
            p->n.attrValuesNumber);
        if(p->n.attrValuesNumber) {
            printImmValue(p->n.attrValueType, &(p->n.attrValue));
        }
        p = p->next;
    }
#endif  /*ABT DEBUG PRINTOUTS STOP*/

    err = immModel_ccbObjectCreate(cb, &(evt->info.objCreate), &implConn,
        &implNodeId, &continuationId);

    if(err == SA_AIS_OK && implNodeId) {
        /*We have an implementer (somewhere)*/
        delayedReply = SA_TRUE;
        if(implConn) {
            /*The implementer is local, make the up-call*/
            assert(implNodeId == cb->node_id);
            tmp_hdl.nodeId = implNodeId;
            tmp_hdl.count = implConn;

            implHandle = *((SaImmOiHandleT *) &tmp_hdl);
            /*Fetch client node for OI !*/
            immnd_client_node_get(cb, implHandle, &oi_cl_node);
            if(oi_cl_node == NULL) {
                /*Client died ?*/
                LOG_ER("Internal inconsistency in IMMND");
                /*
                err = SA_AIS_ERR_FAILED_OPERATION;
                delayedReply = SA_FALSE;
                */
                assert(0); 
            } else {
                memset(&send_evt,'\0',sizeof(IMMSV_EVT));
                send_evt.type = IMMSV_EVT_TYPE_IMMA;
                send_evt.info.imma.type = IMMA_EVT_ND2A_OI_OBJ_CREATE_UC;
                send_evt.info.imma.info.objCreate = evt->info.objCreate;
                send_evt.info.imma.info.objCreate.adminOwnerId = continuationId;
                /*We re-use the adminOwner member of the ccbCreate message to hold the 
                  invocation id. */

                /*immHandle needed on agent side to find correct node.
                  This to send the message over process internal IPC (sigh).*/
                send_evt.info.imma.info.objCreate.immHandle = implHandle;


                TRACE_2("MAKING OI-IMPLEMENTER OBJ CREATE upcall towards agent");
                if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                       oi_cl_node->agent_mds_dest,
                       &send_evt) != NCSCC_RC_SUCCESS) {
                    LOG_ER("Agent upcall over MDS for ccbObjectCreate failed");
                    err = SA_AIS_ERR_LIBRARY;
                } /*else {
                    THIS Impl continuation is obsoleted for now.
                    We instead come back directly from implemener over fevs
                    to all IMMNDs. 
                    TRACE_2("IMMND UPCALL TO AGENT SEND SUCCEEDED");
                    ImmsvInvocation invoc;
                    invoc.owner = evt->info.objCreate.ccbId;
                    invoc.inv = continuationId;
                    immModel_setCcbObjCreateImplContinuation(cb, 
                    *((SaInvocationT *) &invoc),
                    implConn,
                    reply_dest);
                    }*/
            }
        }
    }

    if(originatedAtThisNd && !delayedReply) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        } 

        sinfo = &cl_node->tmpSinfo; 

        TRACE_2("send immediate reply to client/agent");
        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= err;
        assert(sinfo); 
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send result to Agent over MDS");
        }
    }
    TRACE_LEAVE();
}


/****************************************************************************
 * Name          : immnd_evt_proc_object_modify
 *
 * Description   : Function to process the saImmOmCcbObjectModify/_2
 *                 operation from agent. Arrives over FEVS.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_object_modify(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;

    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    SaImmOiHandleT          implHandle=0LL;
    IMMSV_HANDLE            tmp_hdl = *((IMMSV_HANDLE *) &clnt_hdl);
    SaBoolT delayedReply = SA_FALSE;
    SaUint32T implConn=0;
    NCS_NODE_ID implNodeId=0;
    SaUint32T continuationId=0;

    TRACE_ENTER();
#if 0  /*ABT DEBUG PRINTOUTS START*/
    TRACE_2("ABT immnd_evt_proc_object_modify object:%s",
        evt->info.objModify.objectName.buf);
    TRACE_2("ABT immnd_evt_proc_object_modify CCB:%u ADMOWN:%u",
        evt->info.objModify.ccbId,
        evt->info.objModify.adminOwnerId);

    TRACE_2("ABT attrMods:%p ", 
        evt->info.objModify.attrMods);

    IMMSV_ATTR_MODS_LIST* p = evt->info.objModify.attrMods;

    while(p) {
        TRACE_2("ABT FOUND AN ATTRIBUTE-MOD, MOD_TYPE:%u NAME: %s type:%ld "
            "values:%ld", p->attrModType,
            p->attrValue.attrName.buf, p->attrValue.attrValueType,
            p->attrValue.attrValuesNumber);
        if(p->attrValue.attrValuesNumber) {
            printImmValue(p->attrValue.attrValueType, &(p->attrValue.attrValue));
        }
        p = p->next;
    }
#endif  /*ABT DEBUG PRINTOUTS STOP*/

    err = immModel_ccbObjectModify(cb, &(evt->info.objModify), &implConn,
        &implNodeId, &continuationId);

    if(err == SA_AIS_OK && implNodeId) {
        /*We have an implementer (somewhere)*/
        delayedReply = SA_TRUE;
        if(implConn) {
            /*The implementer is local, make the up-call*/
            assert(implNodeId == cb->node_id);
            tmp_hdl.nodeId = implNodeId;
            tmp_hdl.count = implConn;

            implHandle = *((SaImmOiHandleT *) &tmp_hdl);
            /*Fetch client node for OI !*/
            immnd_client_node_get(cb, implHandle, &oi_cl_node);
            if(oi_cl_node == NULL) {
                /*Client died ?*/
                LOG_ER("Internal inconsistency");
                /*
                err = SA_AIS_ERR_FAILED_OPERATION;
                delayedReply = SA_FALSE;
                */
                assert(0); 
            } else {
                memset(&send_evt,'\0',sizeof(IMMSV_EVT));
                send_evt.type = IMMSV_EVT_TYPE_IMMA;
                send_evt.info.imma.type = IMMA_EVT_ND2A_OI_OBJ_MODIFY_UC;

                send_evt.info.imma.info.objModify = evt->info.objModify; 
                /* shallow copy into stack alocated structure. */
	
                send_evt.info.imma.info.objModify.adminOwnerId = continuationId;
                /*We re-use the adminOwner member of the ccbModify message to hold the 
                  continuation id. */

                /*immHandle needed on agent side to find correct node.
                  This to send the message over process internal IPC .*/
                send_evt.info.imma.info.objModify.immHandle = implHandle;


                TRACE_2("MAKING OI-IMPLEMENTER OBJ MODIFY upcall towards agent");
                if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                       oi_cl_node->agent_mds_dest,
                       &send_evt) != NCSCC_RC_SUCCESS) {
                    LOG_ER("Agent upcall over MDS for ccbObjectModify failed");
                    err = SA_AIS_ERR_LIBRARY;
                } /*else {
                    THIS Impl continuation is obsoleted for now.
                    We instead come back directly from implemener over fevs
                    to all IMMNDs. 
                    TRACE_2("IMMND UPCALL TO AGENT SEND SUCCEEDED");
                    ImmsvInvocation invoc;
                    invoc.owner = evt->info.objModify.ccbId;
                    invoc.inv = continuationId;
                    immModel_setCcbObjModifyImplContinuation(cb, 
                    *((SaInvocationT *) &invoc),
                    implConn,
                    reply_dest);
                    }*/
            }
        }
    }

    if(originatedAtThisNd && !delayedReply) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        } 

        sinfo = &cl_node->tmpSinfo; 

        TRACE_2("send immediate reply to client/agent");
        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= err;
        assert(sinfo); 
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send result to Agent over MDS");
        }
    }

    /*Free the incomming events substructure. */
    free(evt->info.objModify.objectName.buf);
    evt->info.objModify.objectName.buf=NULL;
    evt->info.objModify.objectName.size=0;
    immsv_free_attrmods(evt->info.objModify.attrMods);
    evt->info.objModify.attrMods=NULL;
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_rt_object_modify
 *
 * Description   : Function to process the saImmOiRtObjectUpdate/_2
 *                 operation from agent. Arrives over FEVS.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_rt_object_modify(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_HANDLE            tmp_hdl = *((IMMSV_HANDLE *) &clnt_hdl);
    TRACE_ENTER();

#if 0  /*DEBUG PRINTOUTS START*/
    TRACE_2("ABT immnd_evt_proc_rt_object_modify object:%s",
        evt->info.objModify.objectName.buf);
    TRACE_2("ABT attrMods:%p ", 
        evt->info.objModify.attrMods);

    IMMSV_ATTR_MODS_LIST* p = evt->info.objModify.attrMods;

    while(p) {
        TRACE_2("ABT FOUND AN ATTRIBUTE-MOD, MOD_TYPE:%u NAME: %s type:%ld "
            "values:%ld", p->attrModType,
            p->attrValue.attrName.buf, p->attrValue.attrValueType,
            p->attrValue.attrValuesNumber);
        if(p->attrValue.attrValuesNumber) {
            printImmValue(p->attrValue.attrValueType, &(p->attrValue.attrValue));
        }
        p = p->next;
    }
#endif  /*DEBUG PRINTOUTS STOP*/

    unsigned int isLocal = 0;

    if(originatedAtThisNd) {
        err = immModel_rtObjectUpdate(cb, &(evt->info.objModify),
            tmp_hdl.count,
            tmp_hdl.nodeId,
            &isLocal);
    } else {
        err = immModel_rtObjectUpdate(cb, &(evt->info.objModify),
            0,
            tmp_hdl.nodeId,
            &isLocal);
    }

    //Persistent or cached rt-attributes are updated everywhere.
    //Non cached and non persistent attributes only updated locally. 
    //This implementation is not very efficient if the majority of 
    //th rt-updates are local only. The broadcast is then wasted. 
    //The solution is simpler though since otherwise the message has 
    //to be analyzed by the local server and then possibly forwarded.

    if(originatedAtThisNd) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        sinfo = &cl_node->tmpSinfo; 

        TRACE_2("send reply to client/agent");
        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= err;
        assert(sinfo); 
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send result to OI client over MDS");
        }
    }

    /*Free the incomming events substructure. */
    free(evt->info.objModify.objectName.buf);
    evt->info.objModify.objectName.buf=NULL;
    evt->info.objModify.objectName.size=0;
    immsv_free_attrmods(evt->info.objModify.attrMods);
    evt->info.objModify.attrMods=NULL;
    TRACE_LEAVE();
}

static void immnd_evt_ccb_abort(IMMND_CB *cb,
    SaUint32T ccbId, 
    SaBoolT timeout, 
    SaUint32T* client)
{
    IMMSV_EVT      send_evt;
    SaUint32T* implConnArr=NULL;
    SaUint32T  arrSize=0;
    SaUint32T dummyClient=0;
    IMMSV_HANDLE            tmp_hdl;
    TRACE_ENTER();

    tmp_hdl.nodeId = cb->node_id;/*1*/

    immModel_ccbAbort(cb, ccbId, &arrSize, &implConnArr, &dummyClient);

    if(client) {*client = dummyClient;}

    if(arrSize) {
        IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
        SaImmOiHandleT          implHandle=0LL;

        TRACE_2("THERE ARE LOCAL IMPLEMENTERS in ccb:%u", ccbId);

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_OI_CCB_ABORT_UC;
        int ix=0;
        for(; ix<arrSize; ++ix) {
            /*Look up the client node for the implementer, using implConn*/
            tmp_hdl.count = implConnArr[ix];/*2*/
            implHandle = *((SaImmOiHandleT *) &tmp_hdl);
            /*Fetch client node for OI !*/
            immnd_client_node_get(cb, implHandle, &oi_cl_node);
            if(oi_cl_node == NULL) {
                /*Implementer client died ?*/
                LOG_ER("IMMND - Client went down so can not send abort UC - "
                    "ignoring!");
            } else {
                send_evt.info.imma.info.ccbCompl.ccbId = ccbId;
                send_evt.info.imma.info.ccbCompl.implId = implConnArr[ix];
                send_evt.info.imma.info.ccbCompl.immHandle = implHandle;

                TRACE_2("MAKING IMPLEMENTER CCB ABORT upcall");
                if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                       oi_cl_node->agent_mds_dest,
                       &send_evt) != NCSCC_RC_SUCCESS) {
                    LOG_ER("IMMND CCB ABORT UPCALL SEND FAILED FOR impl: %u",
                        implConnArr[ix]);
                }
            }
        }//for
        free(implConnArr);
    }//if(arrSize

    if(dummyClient) {/* Apparently abort during an ongoing client call.*/
        IMMND_IMM_CLIENT_NODE   *om_cl_node=NULL;
        SaImmHandleT omHandle;
        /*Look up the client node for the implementer, using implConn*/
        tmp_hdl.count = dummyClient;/*2*/
        omHandle = *((SaImmHandleT *) &tmp_hdl);
        /*Fetch client node for OM !*/
        immnd_client_node_get(cb, omHandle, &om_cl_node);
        if(om_cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client went down so can not send reply "
                "ignoring!");
        } else {
            memset(&send_evt,'\0',sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMA;
            send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
            send_evt.info.imma.info.errRsp.error=
                timeout ? SA_AIS_ERR_TIMEOUT : SA_AIS_ERR_FAILED_OPERATION;
    
            if(om_cl_node->tmpSinfo.stype == MDS_SENDTYPE_SNDRSP) {
                TRACE_2("SENDRSP %u", send_evt.info.imma.info.errRsp.error);
                if(immnd_mds_send_rsp(cb, &(om_cl_node->tmpSinfo), &send_evt)
                    != NCSCC_RC_SUCCESS) {
                    LOG_ER("Failed to send response to agent/client over MDS");
                }
            }
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_object_delete
 *
 * Description   : Function to process the saImmOmCcbObjectDelete
 *                 operation from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                       is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_object_delete(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;

    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    SaImmOiHandleT          implHandle=0LL;
    IMMSV_HANDLE            tmp_hdl = *((IMMSV_HANDLE *) &clnt_hdl);
    SaUint32T conn = tmp_hdl.count;

    SaUint32T* implConnArr=NULL;
    SaUint32T* invocArr=NULL;
    SaStringT* objNameArr;
    SaUint32T  arrSize=0;
    SaBoolT delayedReply = SA_FALSE;


#if 0  /*ABT DEBUG PRINTOUTS START*/
    TRACE_2("ABT immnd_evt_proc_object_delete object:%s",
        evt->info.objDelete.objectName.buf);
    TRACE_2("ABT immnd_evt_proc_object_delete CCB:%u ADMOWN:%u",
        evt->info.objDelete.ccbId,
        evt->info.objDelete.adminOwnerId);
#endif  /*ABT DEBUG PRINTOUTS STOP*/


    err = immModel_ccbObjectDelete(cb, &(evt->info.objDelete), 
        originatedAtThisNd?conn:0, &arrSize,
        &implConnArr, &invocArr, &objNameArr);

    if(err == SA_AIS_OK) {
        if(arrSize) {
            /*We have local implementer(s) for deleted object(s)*/
            delayedReply = SA_TRUE;
            memset(&send_evt,'\0',sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMA;
            send_evt.info.imma.type = IMMA_EVT_ND2A_OI_OBJ_DELETE_UC;
            send_evt.info.imma.info.objDelete.ccbId = evt->info.objDelete.ccbId;
            int ix=0;
            tmp_hdl.nodeId = cb->node_id;/*1*/
            for(; ix<arrSize && err==SA_AIS_OK; ++ix) {
                /*Look up the client node for the implementer, using implConn*/
                tmp_hdl.count = implConnArr[ix];/*2*/

                implHandle = *((SaImmOiHandleT *) &tmp_hdl);
                /*Fetch client node for OI !*/
                immnd_client_node_get(cb, implHandle, &oi_cl_node);
                if(oi_cl_node == NULL) {
                    /*Client died ?*/
                    LOG_ER("IMMND - Client went down so no response");
                    /*
                    err = SA_AIS_ERR_FAILED_OPERATION;
                    delayedReply = SA_FALSE;
                    */
                    assert(0);
                } else {
                    send_evt.info.imma.info.objDelete.objectName.size = 
                        strlen(objNameArr[ix]);
                    send_evt.info.imma.info.objDelete.objectName.buf = objNameArr[ix];
                    send_evt.info.imma.info.objDelete.adminOwnerId = invocArr[ix];
                    send_evt.info.imma.info.objDelete.immHandle = implHandle;
                    /*Yes that was a bit ugly. But the upcall message is the same 
                      structure as the request message, except the semantics of one 
                      integer member differs.
                    */
                    TRACE_2("MAKING OI-IMPLEMENTER OBJ DELETE upcall ");
                    if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                           oi_cl_node->agent_mds_dest,
                           &send_evt) != NCSCC_RC_SUCCESS) {
                        LOG_ER("Immnd upcall over MDS for ccbObjectDelete failed");
                        err = SA_AIS_ERR_FAILED_OPERATION;
                        delayedReply = SA_FALSE;
                    } /*else {
                        OBSOLETED
                        ImmsvInvocation invoc;
                        invoc.owner = evt->info.objDelete.ccbId;
                        invoc.inv = invocArr[ix];
                        immModel_setCcbObjDelImplContinuation(cb, 
                        *((SaInvocationT *) &invoc),
                        implConnArr[ix],
                        reply_dest);
                        }*/
                }
            }/*for*/
        } else if(immModel_ccbWaitForDeleteImplAck(cb, evt->info.ccbId, &err)) {
            /*No local implementers, but we need to wait for remote implementers*/
            TRACE_2("No local implementers but wait for remote. ccb: %u",
                evt->info.ccbId);
            /* TODO:
              If err != SA_AIS_OK from above call, should not delayedReply=FALSE??
              but perhaps this can not happen if above returns true.
            */
            delayedReply = SA_TRUE;
        } else {
            TRACE_2("NO IMPLEMENTERS AT ALL. for ccb:%u err:%u sz:%u", 
                evt->info.ccbId, err, arrSize);
        }
    } /* err!=SA_AIS_OK or no implementers =>immediate reply*/

    if(arrSize) {
        int ix;
        free(implConnArr);
        implConnArr=NULL;
        free(invocArr);
        invocArr=NULL;
    
        for(ix=0; ix < arrSize; ++ix) {
            free(objNameArr[ix]);
        }    
        objNameArr=NULL;
        arrSize=0;
    }

    if(originatedAtThisNd && !delayedReply) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client went down so no response");
            return;
        } 

        sinfo = &cl_node->tmpSinfo; 

        TRACE_2("Send immediate reply to agent/client");
        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= err;
        assert(sinfo); 
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send result to Agent over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_rt_object_delete
 *
 * Description   : Function to process the saImmOiRtObjectDelete
 *                 operation from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                         originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_rt_object_delete(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_HANDLE            tmp_hdl = *((IMMSV_HANDLE *) &clnt_hdl);
    TRACE_ENTER();

#if 0  /*ABT DEBUG PRINTOUTS START*/
    TRACE_2("ABT immnd_evt_proc_rt_object_delete object:%s",
        evt->info.objDelete.objectName.buf);
    TRACE_2("ABT immnd_evt_proc_rt_object_delete IMPLID:%u",
        evt->info.objDelete.adminOwnerId);
#endif  /*ABT DEBUG PRINTOUTS STOP*/

    if(originatedAtThisNd) {
        err = immModel_rtObjectDelete(cb, &(evt->info.objDelete), 
            tmp_hdl.count, 
            tmp_hdl.nodeId);
    } else {
        err = immModel_rtObjectDelete(cb, &(evt->info.objDelete), 
            0, 
            tmp_hdl.nodeId);
    }

    if(originatedAtThisNd) {
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        sinfo = &cl_node->tmpSinfo; 

        TRACE_2("send immediate reply to client/agent");
        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.info.errRsp.error= err;
        assert(sinfo); 
        assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send result to implementer over MDS");
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_ccb_finalize
 *
 * Description   : Function to process the saImmOmCcbFinalize
 *                 operation from agent (and internal ccb_abort calls)
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *****************************************************************************/
static void immnd_evt_proc_ccb_finalize(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)

{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaUint32T               client=0;
    TRACE_ENTER();

    assert(evt);
    immnd_evt_ccb_abort(cb, evt->info.ccbId, SA_FALSE, &client);
    err = immModel_ccbDelete(cb, evt->info.ccbId);
    TRACE_2("ccb aborted and deleted err:%u", err);

    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        TRACE_2("ccbFinalize originated at this node => Send reply");
        tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
        /* Perhaps the following assert is a bit strong. A corrupt client/agent
           could "accidentally" use the wrong ccbId if its heap was thrashed. 
           This wrong ccbid could accidentally be an existing used ccbId. 
           But in this case it would also have to originate at this node.
        */
        assert(!client || client == tmp_hdl->count); 
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        sinfo = &cl_node->tmpSinfo;

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error=err;

        TRACE_2("SENDRSP %u", err);

        if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_ccb_apply
 *
 * Description   : Function to process the saImmOmCcbApply
 *                 operation from agent.
 *                 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                         originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_ccb_apply(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err = SA_AIS_OK;
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMND_IMM_CLIENT_NODE   *oi_cl_node=NULL;
    SaImmOiHandleT          implHandle=0LL;
    IMMSV_HANDLE            tmp_hdl = *((IMMSV_HANDLE *) &clnt_hdl);
    SaUint32T conn = tmp_hdl.count;

    SaUint32T* implConnArr=NULL;
    SaUint32T* implIdArr=NULL;
    SaUint32T* ctnArr=NULL;
    SaUint32T  arrSize=0;
    SaBoolT delayedReply = SA_FALSE;


    err = immModel_ccbApply(cb, evt->info.ccbId, originatedAtThisNd?conn:0, 
        &arrSize, &implConnArr, &implIdArr, &ctnArr);

    if(err == SA_AIS_OK) {
        if(arrSize) {
            TRACE_2("THERE ARE LOCAL IMPLEMENTERS in ccb:%u", evt->info.ccbId);
            delayedReply = SA_TRUE;
            memset(&send_evt,'\0',sizeof(IMMSV_EVT));
            send_evt.type = IMMSV_EVT_TYPE_IMMA;
            send_evt.info.imma.type = IMMA_EVT_ND2A_OI_CCB_COMPLETED_UC;
            int ix=0;
            tmp_hdl.nodeId = cb->node_id;/*1*/
            for(; ix<arrSize && err==SA_AIS_OK; ++ix) {

                /*Look up the client node for the implementer, using implConn*/
                tmp_hdl.count = implConnArr[ix];/*2*/

                implHandle = *((SaImmOiHandleT *) &tmp_hdl);
                /*Fetch client node for OI !*/
                immnd_client_node_get(cb, implHandle, &oi_cl_node);
                if(oi_cl_node == NULL) {
                    /*Implementer client died ?*/
                    LOG_ER("IMMND - Client went down so no response");
                    /*
                    err = SA_AIS_ERR_FAILED_OPERATION;
                    delayedReply = SA_FALSE;
                    */
                    assert(0);
                } else {
                    send_evt.info.imma.info.ccbCompl.ccbId = evt->info.ccbId;
                    send_evt.info.imma.info.ccbCompl.implId = implIdArr[ix];
                    send_evt.info.imma.info.ccbCompl.invocation = ctnArr[ix];
                    send_evt.info.imma.info.ccbCompl.immHandle = implHandle;

                    TRACE_2("MAKING IMPLEMENTER CCB COMPLETED upcall");
                    if(immnd_mds_msg_send(cb, NCSMDS_SVC_ID_IMMA_OI, 
                           oi_cl_node->agent_mds_dest,
                           &send_evt) != NCSCC_RC_SUCCESS) {
                        LOG_ER("IMMND CCB APPLY: CCB COMPLETED UPCALL SEND FAILED");
                        err = SA_AIS_ERR_FAILED_OPERATION;
                        /* should abort the entire ccb */
                        delayedReply = SA_FALSE;
                    } else {
                        /*  We skip noting the local implementer continuation as it only 
                            complicates the handling of the reply. The reply is sent over
                            fevs directly: imma_oi -> immnd -> immd -(bcast)-> immnd 
                            ImmsvInvocation invoc;
                            invoc.owner = evt->info.ccbId;
                            invoc.inv = ctnArr[ix];
                            immModel_setCcbCompletedImplContinuation(cb,
                            *((SaInvocationT *)&invoc),
                            implConnArr[ix],
                            reply_dest);
                        */
                        TRACE_2("IMMND UPCALL TO OI, SEND SUCCEEDED");
                    }
                }
            }/*for*/
        } else if(immModel_ccbWaitForCompletedAck(cb, evt->info.ccbId, &err)) {
            /*no local implementers, but we need to wait for remote implementers*/
            TRACE_2("No local implementers but wait for remote. ccb: %u",
                evt->info.ccbId);
            /* TODO:
              If err != SA_AIS_OK from above all should not delayedReply=FALSE??*/
            delayedReply = SA_TRUE;
        } else {
            TRACE_2("NO IMPLEMENTERS AT ALL. for ccb:%u err:%u sz:%u", 
                evt->info.ccbId, err, arrSize);
        }
    } /* err != SA_AIS_OK or no implementers => immediate reply.*/

    if(arrSize) {
        free(implConnArr);
        implConnArr=NULL;

        free(implIdArr);
        implIdArr=NULL;

        free(ctnArr);
        ctnArr=NULL;

        arrSize=0;
    }

    /*REPLY can not be sent immediately if there are implementers
      Must wait for prepare votes. */
    if (!delayedReply) {
        SaUint32T client=0;
        if(err == SA_AIS_OK) {
            /*No implementers anywhere and all is OK*/
            TRACE_2("DIRECT COMMIT");
            /* TODO:
              This is a dangerous variant of commit. If any other ND fails
              to commit => inconsistency. This could happen if implementer disconect
              or node departure, is not handled correctly/consistently over FEVS. 
              We should transform this to a more elaborate immnd_evt_ccb_commit call.
            */
            immModel_ccbCommit(cb, evt->info.ccbId, &arrSize, &implConnArr);
            /* TODO: INFORM DIRECTOR OF CCB-COMMIT DECISION*/
            assert(arrSize == 0);
        } else {
            /*err != SA_AIS_OK => generate SaImmOiCcbAbortCallbackT upcalls
             */
            immnd_evt_ccb_abort(cb, evt->info.ccbId, SA_FALSE, &client);
            assert(!client || originatedAtThisNd);
        }
        TRACE_2("CCB APPLY TERMINATING CCB: %u", evt->info.ccbId);
        immModel_ccbDelete(cb, evt->info.ccbId);

        if(originatedAtThisNd) {
            immnd_client_node_get(cb, clnt_hdl, &cl_node);
            if(cl_node == NULL) {
                /*Client died ?*/
                LOG_ER("IMMND - Client went down so no response");
            } else {
                sinfo = &cl_node->tmpSinfo; 
                TRACE_2("send immediate reply to client");
                memset(&send_evt,'\0',sizeof(IMMSV_EVT));
                send_evt.type = IMMSV_EVT_TYPE_IMMA;
                send_evt.info.imma.info.errRsp.error= err;
                assert(sinfo);
                assert(sinfo->stype == MDS_SENDTYPE_SNDRSP);
                send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

                if(immnd_mds_send_rsp(cb, sinfo, &send_evt) != NCSCC_RC_SUCCESS) {
                    LOG_ER("Failed to send result to Agent over MDS");
                }
            }
        }
    }
}


static uns32 immnd_restricted_ok(IMMND_CB *cb, uns32 id)
{
    if(cb->mSync) {
        if(id == IMMND_EVT_A2ND_CLASS_CREATE ||
            id == IMMND_EVT_A2ND_OBJ_SYNC||
            id == IMMND_EVT_ND2ND_SYNC_FINALIZE || /*probably not over fevs?*/
            id == IMMND_EVT_D2ND_SYNC_ABORT) {    /*probably not over fevs*/
            return 1;
        }
    }

    return 0;
}

/****************************************************************************
 * Name          : immnd_evt_proc_fevs_dispatch
 *
 * Description   : Function to dispatch fevs 
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_OCTET_STRING *msg - Message to unpack and dispatch
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 MDS_DEST reply_dest - The dest of the ND where reply is to
 *                                       go.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static SaAisErrorT
immnd_evt_proc_fevs_dispatch(IMMND_CB* cb, IMMSV_OCTET_STRING* msg, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             error=SA_AIS_OK;
    IMMSV_EVT               frwrd_evt;
    NCS_UBAID               uba;
    uba.start=NULL;


    memset(&frwrd_evt,'\0',sizeof(IMMSV_EVT));

    /*Unpack the embedded message*/
    if(ncs_enc_init_space_pp(&uba, 0,0) != NCSCC_RC_SUCCESS) {
        LOG_ER("Failed init ubaid");
        uba.start=NULL;
        error = SA_AIS_ERR_NO_RESOURCES;
        goto unpack_failure;
    }

    if(ncs_encode_n_octets_in_uba(&uba, (uns8 *) msg->buf, msg->size) != 
        NCSCC_RC_SUCCESS) {
        LOG_ER("Failed buffer copy");
        uba.start=NULL;
        error = SA_AIS_ERR_NO_RESOURCES;
        goto unpack_failure;
    }

    /* for debugging messages
       int size = evt->info.fevsReq.msg.size;
       if(size >340) {
       char* tmpData = calloc(1, size);
       char * data = m_MMGR_DATA_AT_START(uba.ub, 
       size, tmpData);
       TRACE_2("ABT immnd_mds_dec:DUMP:%u/%p [", size, data);
       int ix;
       for(ix = 0; ix < size; ++ix) {
       unsigned char c = data[ix];
       unsigned int i = c;
       TRACE_2("%u|", i);
       }
       TRACE_2("]");
       }
    */

    ncs_dec_init_space(&uba, uba.start);
    uba.bufp=NULL;

    /* Decode non flat. */
    if(immsv_evt_dec(&uba, &frwrd_evt) != NCSCC_RC_SUCCESS) {
        LOG_ER("Edu decode Failed");
        error = SA_AIS_ERR_LIBRARY;
        goto unpack_failure;
    }

    if(frwrd_evt.type != IMMSV_EVT_TYPE_IMMND)
    {
        LOG_ER("IMMND - Unknown Event Type");
        error = SA_AIS_ERR_LIBRARY;
        goto unpack_failure;
    }

    if(!(cb->mAccepted || immnd_restricted_ok(cb, frwrd_evt.info.immnd.type))) {
        error = SA_AIS_ERR_ACCESS;
        goto discard_message;
    }

    /*Dispatch the unpacked FEVS message*/
    immsv_msg_trace_rec(frwrd_evt.sinfo.dest, &frwrd_evt);

    switch(frwrd_evt.info.immnd.type)
    {
        case IMMND_EVT_A2ND_OBJ_CREATE:
            immnd_evt_proc_object_create(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OI_OBJ_CREATE:
            immnd_evt_proc_rt_object_create(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OBJ_MODIFY:
            immnd_evt_proc_object_modify(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OI_OBJ_MODIFY:
            immnd_evt_proc_rt_object_modify(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OBJ_DELETE:
            immnd_evt_proc_object_delete(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;
	 
        case IMMND_EVT_A2ND_OI_OBJ_DELETE:
            immnd_evt_proc_rt_object_delete(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;
	 
        case IMMND_EVT_A2ND_OBJ_SYNC:
            immnd_evt_proc_object_sync(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;
	 
        case IMMND_EVT_A2ND_IMM_ADMOP:
        case IMMND_EVT_A2ND_IMM_ADMOP_ASYNC:
            immnd_evt_proc_admop(cb, &frwrd_evt.info.immnd, 
			    originatedAtThisNd,
			    clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_CLASS_CREATE:
            immnd_evt_proc_class_create(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_CLASS_DELETE:
            immnd_evt_proc_class_delete(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd,
                clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_D2ND_DISCARD_IMPL:
            immnd_evt_proc_discard_impl(cb,&frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_D2ND_DISCARD_NODE:
            immnd_evt_proc_discard_node(cb,&frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_D2ND_ADMINIT:
            immnd_evt_proc_adminit_rsp(cb,&frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_D2ND_IMPLSET_RSP:
            immnd_evt_proc_impl_set_rsp(cb,&frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_D2ND_CCBINIT:
            immnd_evt_proc_ccbinit_rsp(cb,&frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;
       
        case IMMND_EVT_A2ND_CCB_APPLY:
            immnd_evt_proc_ccb_apply(cb,&frwrd_evt.info.immnd, 
				originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_CCB_FINALIZE:
            immnd_evt_proc_ccb_finalize(cb,&frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_D2ND_ABORT_CCB:
            immnd_evt_proc_ccb_finalize(cb,&frwrd_evt.info.immnd, 
                /*originatedAtThisNd*/ SA_FALSE,
                clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OI_IMPL_CLR:
            immnd_evt_proc_impl_clr(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OI_CL_IMPL_SET:
            immnd_evt_proc_cl_impl_set(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OI_CL_IMPL_REL:
            immnd_evt_proc_cl_impl_rel(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OI_OBJ_IMPL_SET:
            immnd_evt_proc_obj_impl_set(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_OI_OBJ_IMPL_REL:
            immnd_evt_proc_obj_impl_rel(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_ADMO_FINALIZE:
            immnd_evt_proc_admo_finalize(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_D2ND_ADMO_HARD_FINALIZE:
            immnd_evt_proc_admo_hard_finalize(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, 
                reply_dest);
            break;

        case IMMND_EVT_A2ND_ADMO_SET:
            immnd_evt_proc_admo_set(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;
				    
        case IMMND_EVT_A2ND_ADMO_RELEASE:
            immnd_evt_proc_admo_release(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;
				    
        case IMMND_EVT_A2ND_ADMO_CLEAR:
            immnd_evt_proc_admo_clear(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;
				    

        case IMMND_EVT_ND2ND_SYNC_FINALIZE:
            immnd_evt_proc_finalize_sync(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_CCB_COMPLETED_RSP:
            immnd_evt_proc_ccb_compl_rsp(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, reply_dest);
            break;

        case IMMND_EVT_A2ND_CCB_OBJ_CREATE_RSP:
            immnd_evt_proc_ccb_obj_create_rsp(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, 
                reply_dest);
            break;

        case IMMND_EVT_A2ND_CCB_OBJ_MODIFY_RSP:
            immnd_evt_proc_ccb_obj_modify_rsp(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, 
                reply_dest);
            break;

        case IMMND_EVT_A2ND_CCB_OBJ_DELETE_RSP:
            immnd_evt_proc_ccb_obj_delete_rsp(cb, &frwrd_evt.info.immnd, 
                originatedAtThisNd, clnt_hdl, 
                reply_dest);
            break;

        default:
            LOG_ER("UNPACK FAILURE, unrecognized message type: %u",
                frwrd_evt.info.immnd.type);
            break;
    }

 discard_message:
 unpack_failure:

    if(uba.start) {
        m_MMGR_FREE_BUFR_LIST(uba.start);
    }
    immnd_evt_destroy(&frwrd_evt, SA_FALSE, __LINE__);
    if((error!=SA_AIS_OK) && (error != SA_AIS_ERR_ACCESS)) {
        TRACE_2("Could not process FEVS message, ERROR:%u",error);
    }
    return error;
}

/****************************************************************************
 * Name          : immnd_evt_proc_dump_ok
 *
 * Description   : Function to increment epoch at all nodes, after dump
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - Sender MDS information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_dump_ok(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    TRACE_ENTER();
    if(cb->mAccepted) {
        cb->mRulingEpoch = evt->info.ctrl.rulingEpoch;
        TRACE_2("Ruling Epoch:%u", cb->mRulingEpoch); 

        if((cb->mMyEpoch +1) == cb->mRulingEpoch) {
            ++(cb->mMyEpoch);
            TRACE_2("MyEpoch incremented to %u", cb->mMyEpoch);

            /*Note that the searchOp for the dump should really be done
              here. Currently it is done before sending the dump request
              which means that technically the epoch/contents of the dump
              is not guaranteed to match exactly the final contents of the 
              epoch at the other nodes. On the other hand, this slight
              difference is currently not persistent. */

            immnd_adjustEpoch(cb);
        } else {
            LOG_ER("Missmatch on epoch mine:%u proposed new epoch:%u",
                cb->mMyEpoch, cb->mRulingEpoch);
        }
    }
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_evt_proc_start_sync
 *
 * Description   : Function to process start sync message
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - Sender MDS information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_start_sync(IMMND_CB *cb, IMMND_EVT *evt,
    IMMSV_SEND_INFO *sinfo)
{
    cb->mRulingEpoch = evt->info.ctrl.rulingEpoch;
    TRACE_2("Ruling Epoch:%u", cb->mRulingEpoch); 

    if(cb->mState == IMM_SERVER_SYNC_PENDING) {
        /*This node wants to be synced.*/
        assert(!cb->mAccepted);
        cb->mSync = TRUE;
        cb->mMyEpoch = cb->mRulingEpoch - 1; 
        TRACE_2("Adjust fevs count:%llu %llu %llu", cb->highestReceived, 
            cb->highestProcessed, evt->info.ctrl.fevsMsgStart);

        assert(cb->highestProcessed <= evt->info.ctrl.fevsMsgStart);
        assert(cb->highestReceived <= evt->info.ctrl.fevsMsgStart);
        cb->highestProcessed = evt->info.ctrl.fevsMsgStart;
        cb->highestReceived = evt->info.ctrl.fevsMsgStart;
    }

    /*Nodes that are not accepted and that have not requested sync do not
      partake in this sync. They dont try to keep up with info on others
    */
    if(cb->mMyEpoch + 1 == cb->mRulingEpoch) {
        /*old members and joining sync'ers*/
        if(!cb->mSync) { /*If I am an old established member then increase 
                           my epoch now!*/
            cb->mMyEpoch++;
        }
        immModel_prepareForSync(cb, cb->mSync);
        cb->mPendSync = 0; //Sync can now begin.
    } else {
        if(cb->mMyEpoch + 1 < cb->mRulingEpoch) {
            if(cb->mState > IMM_SERVER_LOADING_PENDING) {
            LOG_WA("Imm at this evs node has epoch %u, "
                "appears to be a stragler in wrong state %u", 
                cb->mMyEpoch, cb->mState);
                assert(0);
            } else {
                TRACE_2("This nodes apparently missed start of sync");
            }
        } else {
            assert(cb->mMyEpoch + 1 > cb->mRulingEpoch);
            LOG_WA("Imm at this evs node has epoch %u, "
                "COORDINATOR appears to be a stragler!!, aborting.",
                cb->mMyEpoch);
            assert(0);
            /* TODO: 080414 re-inserted the assert ...
              This is an extreemely odd case. Possibly it could occur after a
              failover ??*/
        }
    }
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_evt_proc_reset
 *
 * Description   : Function to process RESET broadcast from IMMD
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - Sender MDS information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_reset(IMMND_CB *cb, IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)
{
    TRACE_ENTER();    
    if(cb->mIntroduced) 
    {
        LOG_ER("IMMND forced to restart on order from IMMD");
        exit(1);
    } else {
        LOG_NO("IMMD received reset order from IMMD, but has just restarted - "
            "ignoring");
    }
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_evt_proc_loading_ok
 *
 * Description   : Function to process loading_ok broadcast
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - Sender MDS information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_loading_ok(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    TRACE_ENTER();
    cb->mRulingEpoch = evt->info.ctrl.rulingEpoch;
    TRACE_2("Loading can start, ruling epoch:%u", cb->mRulingEpoch); 

    if((cb->mState == IMM_SERVER_LOADING_PENDING) ||
        (cb->mState == IMM_SERVER_LOADING_CLIENT)) {
        assert(((cb->mMyEpoch +1) == cb->mRulingEpoch));
        ++(cb->mMyEpoch);
        immModel_prepareForLoading(cb);
        TRACE_2("prepareForLoading non-coord variant. fevsMsgCount reset to %llu",
           cb->highestProcessed);
        cb->highestProcessed = evt->info.ctrl.fevsMsgStart;
        cb->highestReceived = evt->info.ctrl.fevsMsgStart;
    } else if(cb->mState == IMM_SERVER_LOADING_SERVER) {
        assert(cb->mMyEpoch == cb->mRulingEpoch);
        immModel_prepareForLoading(cb);
        cb->highestProcessed = evt->info.ctrl.fevsMsgStart;
        cb->highestReceived = evt->info.ctrl.fevsMsgStart;
        TRACE_2("prepareForLoading coord variant. fevsMsgCount reset to %llu",
             cb->highestProcessed);
    } else {
        /* Note: corrected for case of IMMND at *both* controllers failing.
          This must force down IMMND on all payloads, since these can
          not act as IMM coordinator. */
        if(cb->mState > IMM_SERVER_SYNC_PENDING) {
            LOG_ER("This IMMND can not accept start loading in state %s %u",
                (cb->mState == IMM_SERVER_SYNC_CLIENT)?"IMM_SERVER_SYNC_CLIENT":
                (cb->mState == IMM_SERVER_SYNC_SERVER)?"IMM_SERVER_SYNC_SERVER":
                (cb->mState == IMM_SERVER_READY)?"IMM_SERVER_READY":
                "UNKNOWN", cb->mState);
            exit(1);
        }
    }
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_evt_proc_sync_req
 *
 * Description   : Function to process sync request over IMMD.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - Sender MDS information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_sync_req(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    TRACE_2("SYNC REQUEST RECEIVED VIA IMMD");

    if((evt->info.ctrl.nodeId == cb->node_id) && (cb->mIntroduced) &&
        !(cb->mSync)) {
        TRACE_2("Sync client");
        immModel_recognizedIsolated(cb);
    } else if(cb->mIsCoord) {
        TRACE_2("Set marker for sync at coordinator");
        cb->mSyncRequested = TRUE;
        /*assert(cb->mRulingEpoch == evt->info.ctrl.rulingEpoch);*/
        TRACE_2("At COORD: My Ruling Epoch:%u Cenral Ruling Epoch:%u",
			cb->mRulingEpoch, evt->info.ctrl.rulingEpoch); 
    }  
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_evt_proc_intro_rsp
 *
 * Description   : Function to process intro reply from immd.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - Sender MDS information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_intro_rsp(IMMND_CB *cb,
    IMMND_EVT *evt, IMMSV_SEND_INFO *sinfo)
{
    if(evt->info.ctrl.nodeId != cb->node_id) {
        cb->mNumNodes++;
    } else { /*This node was introduced to the IMM cluster*/
        cb->mIntroduced = TRUE;
        cb->mCanBeCoord = evt->info.ctrl.canBeCoord;
        cb->mIsCoord = evt->info.ctrl.isCoord;
        assert(!cb->mIsCoord || cb->mCanBeCoord);
        cb->mRulingEpoch = evt->info.ctrl.rulingEpoch;
        if(cb->mRulingEpoch) {
            TRACE_2("Ruling Epoch:%u", cb->mRulingEpoch); 
        }
    }
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : immnd_evt_proc_fevs_rcv
 *
 * Description   : Function to process fevs broadcast received  at immnd
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 IMMSV_SEND_INFO *sinfo - Sender MDS information.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 *****************************************************************************/
static uns32 immnd_evt_proc_fevs_rcv(IMMND_CB *cb,
    IMMND_EVT *evt, 
    IMMSV_SEND_INFO *sinfo)/*sinfo not used*/
{
    assert(evt);
    SaUint64T msgNo = evt->info.fevsReq.sender_count; /*Global MsgNo*/
    SaImmHandleT clnt_hdl = evt->info.fevsReq.client_hdl;
    IMMSV_OCTET_STRING* msg = &evt->info.fevsReq.msg;
    MDS_DEST reply_dest = evt->info.fevsReq.reply_dest;
    TRACE_ENTER();

    SaBoolT originatedAtThisNd = 
        ((IMMSV_HANDLE *)&clnt_hdl)->nodeId == cb->node_id;
    
    if(originatedAtThisNd) {
        assert(!reply_dest || (reply_dest == cb->immnd_mdest_id));
        --(cb->messages_pending);  /*flow control towards IMMD */
        TRACE_2("BROADCAST OK! messages from me still pending:%u", 
            cb->messages_pending);
    } else {
        TRACE_2("REMOTE FEVS BCAST received. messages from me still pending:%u",
            cb->messages_pending);
    }

    if(cb->highestProcessed >= msgNo) {
        /*We have already received this message, discard it.*/
        LOG_ER("DISCARD DUPLICATE FEVS message:%llu", msgNo);
        return NCSCC_RC_FAILURE;  /*TODO: ensure evt is discarded by invoker*/
    } 

    if(cb->highestReceived < msgNo) {
        cb->highestReceived = msgNo;
    }

    if((cb->highestProcessed + 1) < msgNo) {
        /*We received an out-of-order (higher msgNo than expected) message*/
        SaUint64T next_expected=0;
        SaUint32T andHowManyMore=0; /*What are these used for ?*/
        if(cb->mAccepted) {
            LOG_WA("MESSAGE:%llu OUT OF ORDER my highest processed:%llu", msgNo,
                cb->highestProcessed);
            immnd_enqueue_fevs_msg(cb, msgNo, clnt_hdl, reply_dest, msg, 
                &next_expected, &andHowManyMore);
        }

        /*If next_expected!=0 we send request to re-send message(s) to Director. 
          This is a bit stupid. we KNOW that next expected is highestProcessed+1
        */
        return NCSCC_RC_SUCCESS;  /* TODO: ensure evt is discarded by invoker*/
    } 

    /*NORMAL CASE: Received the expected in-order message.*/

    do {
        SaAisErrorT err =
            immnd_evt_proc_fevs_dispatch(cb, msg, originatedAtThisNd, clnt_hdl, 
                reply_dest);
        if(err != SA_AIS_OK) {
            if(err == SA_AIS_ERR_ACCESS) {
                TRACE_2("DISCARDING msg no:%llu", msgNo);
            } else {
                LOG_ER("PROBLEM %u WITH msg no:%llu", err, msgNo);
            }
        }

        msgNo = ++(cb->highestProcessed);

        /*Check if the processed message releases some queued messages.*/

        if(cb->highestReceived > msgNo) { /*queue is not empty*/
            msg = immnd_dequeue_fevs_msg(msg, cb, msgNo, &clnt_hdl, &reply_dest);
            /* TODO: Perhaps add next_expected and andHowManyMore arg to trigger
              fetching of lost messages. Otherwise this will be triggered only when
              the next normal message arrives. 
              Or the ND can regularly send a ping over fevs => better, use for 
              GC of stored messages. 
            */
            if(msg) {
                originatedAtThisNd = (((IMMSV_HANDLE *)&clnt_hdl)->nodeId == 
                    cb->node_id);
            }
        } else {
            msg = NULL;
            assert(cb->fevs_msg_list == NULL);
        }
    } while(msg);

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_evt_proc_discard_impl
 *
 * Description   : Internal call over fevs to cleanup after terminated
 *                 implementer.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_discard_impl(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    TRACE_ENTER();
    assert(evt);
    TRACE_2("Global discard implementer for id:%u", 
        evt->info.implSet.impl_id);
    immModel_discardImplementer(cb, evt->info.implSet.impl_id, SA_TRUE);
    TRACE_LEAVE();
}


/****************************************************************************
 * Name          : immnd_evt_proc_discard_node
 *
 * Description   : Internal call over fevs to cleanup after terminated
 *                 IMMND, due to IMMND crash or node crash.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_discard_node(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaUint32T* idArr=NULL;
    SaUint32T arrSize=0;
    TRACE_ENTER();
    assert(evt);
    assert(evt->info.ctrl.nodeId != cb->node_id);
    LOG_NO("Global discard node received for nodeId:%x pid:%u", 
        evt->info.ctrl.nodeId, evt->info.ctrl.ndExecPid);
    /* We should remember the nodeId/pid pair to avoid a redundant message
       causing a newly reattached node being discarded. 
    */
    immModel_discardNode(cb, evt->info.ctrl.nodeId, &arrSize, &idArr);
    if(arrSize) {
        SaAisErrorT err = SA_AIS_OK;
        SaUint32T ix;
        for(ix=0; ix<arrSize; ++ix) {
            LOG_WA("Detected crash at node %x, abort ccbId  %u", 
                evt->info.ctrl.nodeId, idArr[ix]);
            immnd_evt_ccb_abort(cb, idArr[ix], SA_FALSE, NULL);
            err = immModel_ccbDelete(cb, idArr[ix]);
            if(err != SA_AIS_OK) {
                LOG_ER("Failed to remove Ccb %u - ignoring", idArr[ix]);
            }
        }
        free(idArr);
        idArr=NULL;
        arrSize=0;
    }

    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_adminit_rsp
 *
 * Description   : Function to process adminit response from Director
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_adminit_rsp(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID nodeId;
    SaUint32T conn;

    assert(evt);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;
    err = immModel_adminOwnerCreate(cb, &(evt->info.adminitGlobal.i), 
        evt->info.adminitGlobal.globalOwnerId, 
        (originatedAtThisNd)?conn:0, nodeId);

    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Match against continuation.
          Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        if (err != SA_AIS_OK) {
            send_evt.info.imma.info.admInitRsp.error=err;
        } else {
            /*Pick up continuation, if gone (timeout or client termination) => 
              drop.*/
            send_evt.info.imma.info.admInitRsp.error = SA_AIS_OK;
            send_evt.info.imma.info.admInitRsp.ownerId=
                evt->info.adminitGlobal.globalOwnerId;

            TRACE_2("admin owner id:%u", evt->info.adminitGlobal.globalOwnerId);
        }

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ADMINIT_RSP;

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_finalize_sync
 *
 * Description   : Function to process finalize IMMND sync from IMMND coord.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
                                         is to be sent (only relevant if
                                         originatedAtThisNode is false).
*****************************************************************************/
static void immnd_evt_proc_finalize_sync(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    TRACE_ENTER();
    TRACE_2("**********immnd_evt_proc_finalize_sync***********");
    if(cb->mSync) {
        TRACE_2("FinalizeSync for sync client");
        assert(immModel_finalizeSync(cb, &(evt->info.finSync), SA_FALSE,
                   SA_TRUE) == SA_AIS_OK);
        cb->mAccepted = SA_TRUE;/*Accept ALL fevs messages after this one!*/
        cb->mMyEpoch++;
        /*This must bring the epoch of the joiner up to the ruling epoch*/
        assert(cb->mMyEpoch == cb->mRulingEpoch);
    } else {
        if(!(cb->mIsCoord)) {
            TRACE_2("FinalizeSync for veteran node that is non coord");
            /* In this case we use the sync message to verify the state 
               instad of syncing. */
            assert(immModel_finalizeSync(cb, &(evt->info.finSync), SA_FALSE, 
                       SA_FALSE) == SA_AIS_OK);
        }
    }

    immnd_adjustEpoch(cb);
    /*This adjust-epoch will persistify the new epoch for: coord, sync-client
      and all other veteran members.*/

    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_admo_set
 *
 * Description   : Function to process admin owner set from agent
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_admo_set(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    SaAisErrorT             err;

    assert(evt);
    if(evt->info.admReq.adm_owner_id != 0) {
        err = immModel_adminOwnerChange(cb, &(evt->info.admReq), SA_FALSE);
    } else {
        LOG_ER("adminOwnerSet can not have 0 admin owner id");
        err = SA_AIS_ERR_LIBRARY;
    }

    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client. AND dont forget time-outs.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error=err;

        TRACE_2("SENDRSP %u", err);

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_admo_release
 *
 * Description   : Function to process admin owner release from agent
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_admo_release(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    SaAisErrorT             err;

    assert(evt);
    if(evt->info.admReq.adm_owner_id != 0) {
        err = immModel_adminOwnerChange(cb, &(evt->info.admReq), SA_TRUE);
    } else {
        LOG_ER("adminOwnerRelease can not have 0 admin owner id");
        err = SA_AIS_ERR_LIBRARY;
    }

    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            /*Client died ?*/
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client. AND dont forget time-outs.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error=err;

        TRACE_2("SENDRSP %u", err);

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_admo_clear
 *
 * Description   : Function to process admin owner clear from agent
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_admo_clear(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    SaAisErrorT             err;

    assert(evt);
    if(evt->info.admReq.adm_owner_id == 0) {
        err = immModel_adminOwnerChange(cb, &(evt->info.admReq), SA_TRUE);
    } else {
        LOG_ER("adminOwnerClear can not have non ZERO admin owner id");
        err = SA_AIS_ERR_LIBRARY;
    }

    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client. AND dont forget time-outs.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error=err;

        TRACE_2("SENDRSP %u", err);

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_admo_finalize
 *
 * Description   : Function to process admin owner finalize from agent
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                         originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_admo_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    SaAisErrorT             err;

    /* TODO: ABT should really remove any open ccbs owned by this admowner.
      Currently this cleanup is left for the closure of the imm-handle or
      timeout cleanup or process termination to take care of. 
    */

    assert(evt);
    err = immModel_adminOwnerDelete(cb, evt->info.admFinReq.adm_owner_id, 0);

    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        send_evt.info.imma.info.errRsp.error=err;

        TRACE_2("SENDRSP %u", err);

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_admo_hard_finalize
 *
 * Description   : Function for hard admin owner finalize (lost connection)
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                         originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_admo_hard_finalize(IMMND_CB *cb,
    IMMND_EVT *evt, 
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    SaAisErrorT             err;
    TRACE_ENTER();

    /* TODO: ABT should really remove any open ccbs owned by this admowner.
      Currently this cleanup is left for the closure of the imm-handle or
      timeout cleanup or process termination to take care of. 
    */

    assert(evt);
    TRACE("immnd_evt_proc_admo_hard_finalize of adm_owner_id: %u",
        evt->info.admFinReq.adm_owner_id);
    err = immModel_adminOwnerDelete(cb, evt->info.admFinReq.adm_owner_id, 1);
    if(err != SA_AIS_OK) {
        LOG_WA("Failed in hard remove of admin owner %u", 
            evt->info.admFinReq.adm_owner_id);
    }
    TRACE_LEAVE();
}


/****************************************************************************
 * Name          : immnd_evt_proc_impl_set_rsp
 *
 * Description   : Function to process implementer set response from Director
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_impl_set_rsp(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID nodeId;
    SaUint32T conn;

    assert(evt);
    assert(!originatedAtThisNd || reply_dest == cb->immnd_mdest_id);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;

    err = immModel_implementerSet(cb, &(evt->info.implSet.impl_name),
        (originatedAtThisNd)?conn:0, nodeId,
        evt->info.implSet.impl_id, reply_dest);
				   
    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        if (err != SA_AIS_OK) {
            send_evt.info.imma.info.implSetRsp.error=err;
        } else {
            /*Pick up continuation, if gone (timeout or client termination) => 
              drop.*/
            send_evt.info.imma.info.implSetRsp.error = SA_AIS_OK;
            send_evt.info.imma.info.implSetRsp.implId=
                evt->info.implSet.impl_id;

            TRACE_2("Implementer global id:%u", evt->info.implSet.impl_id);
        }

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMPLSET_RSP;

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_impl_clr
 *
 * Description   : Function to process implementer clear over FEVS from Ag.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 ***************************************************************************/
static void immnd_evt_proc_impl_clr(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID nodeId;
    SaUint32T conn;

    assert(evt);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;
    err = immModel_implementerClear(cb, &(evt->info.implSet),
        (originatedAtThisNd)?conn:0, nodeId);
				   
    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.info.imma.info.errRsp.error=err;
        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_cl_impl_set
 *
 * Description   : Function to process class implementer set over FEVS from Ag.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_cl_impl_set(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID nodeId;
    SaUint32T conn;

    assert(evt);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;
    err = immModel_classImplementerSet(cb, &(evt->info.implSet),
        (originatedAtThisNd)?conn:0, nodeId);
				   
    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        if (err != SA_AIS_OK) {
            send_evt.info.imma.info.errRsp.error=err;
        } else {
            send_evt.info.imma.info.errRsp.error = SA_AIS_OK;
        }

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_cl_impl_rel
 *
 * Description   : Function to process class implementer release over FEVS.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_cl_impl_rel(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID nodeId;
    SaUint32T conn;

    assert(evt);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;
    err = immModel_classImplementerRelease(cb, &(evt->info.implSet),
        (originatedAtThisNd)?conn:0, nodeId);
				   
    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        if (err != SA_AIS_OK) {
            send_evt.info.imma.info.errRsp.error=err;
        } else {
            send_evt.info.imma.info.errRsp.error = SA_AIS_OK;
        }

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;
        TRACE_2("SENDRSP OK");

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_obj_impl_set
 *
 * Description   : Function to process object implementer set over FEVS.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_obj_impl_set(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID nodeId;
    SaUint32T conn;
    TRACE_ENTER();

    assert(evt);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;
    err = immModel_objectImplementerSet(cb, &(evt->info.implSet),
        (originatedAtThisNd)?conn:0, nodeId);
				   
    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.info.imma.info.errRsp.error=err;

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_obj_impl_rel
 *
 * Description   : Function to process object implementer release over FEVS.
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 *****************************************************************************/
static void immnd_evt_proc_obj_impl_rel(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID nodeId;
    SaUint32T conn;
    TRACE_ENTER();

    assert(evt);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;
    err = immModel_objectImplementerRelease(cb, &(evt->info.implSet),
        (originatedAtThisNd)?conn:0, 
        nodeId);
				   
    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo;  

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));

        send_evt.info.imma.info.errRsp.error=err;

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_IMM_ERROR;

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : immnd_evt_proc_ccbinit_rsp
 *
 * Description   : Function to process ccbinit response from Director
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *                 SaBoolT originatedAtThisNode - Did it come from this node?
 *                 SaImmHandleT clnt_hdl - The client handle (only relevant if
 *                                         originatedAtThisNode is true).
 *                 IMM_DEST reply_dest - The dest of the ND to where reply
 *                                         is to be sent (only relevant if
 *                                       originatedAtThisNode is false).
 * Return Values : None
 *
 *****************************************************************************/
static void immnd_evt_proc_ccbinit_rsp(IMMND_CB *cb,
    IMMND_EVT *evt,
    SaBoolT originatedAtThisNd,
    SaImmHandleT clnt_hdl,
    MDS_DEST reply_dest)
{
    uns32                   rc=NCSCC_RC_SUCCESS;  
    IMMSV_EVT               send_evt;
    IMMND_IMM_CLIENT_NODE   *cl_node=NULL;
    IMMSV_SEND_INFO         *sinfo=NULL;
    IMMSV_HANDLE            *tmp_hdl;
    SaAisErrorT             err;
    NCS_NODE_ID             nodeId;
    SaUint32T               conn;

    assert(evt);
    tmp_hdl = ((IMMSV_HANDLE *) &clnt_hdl);
    nodeId = tmp_hdl->nodeId;
    conn = tmp_hdl->count;
    err = immModel_ccbCreate(cb, 
        evt->info.ccbinitGlobal.i.adminOwnerId,
        evt->info.ccbinitGlobal.i.ccbFlags, 
        evt->info.ccbinitGlobal.globalCcbId,
        nodeId,
        (originatedAtThisNd)?conn:0);

    if(originatedAtThisNd) { /*Send reply to client from this ND.*/
        immnd_client_node_get(cb, clnt_hdl, &cl_node);
        if(cl_node == NULL) {
            LOG_ER("IMMND - Client went down so no response");
            return;
        }

        /*Asyncronous agent calls can cause more than one continuation to be
          present for the SAME client.*/
        sinfo = &cl_node->tmpSinfo; 

        memset(&send_evt,'\0',sizeof(IMMSV_EVT));
        send_evt.info.imma.info.admInitRsp.error=err;

        if (err == SA_AIS_OK) {
            /*Pick up continuation, if gone (timeout or client termination) => 
              drop.*/
            send_evt.info.imma.info.ccbInitRsp.ccbId=
                evt->info.ccbinitGlobal.globalCcbId;

            TRACE_2("Ccb global id:%u", send_evt.info.imma.info.ccbInitRsp.ccbId);
        }

        send_evt.type = IMMSV_EVT_TYPE_IMMA;
        send_evt.info.imma.type = IMMA_EVT_ND2A_CCBINIT_RSP;

        rc = immnd_mds_send_rsp(cb, sinfo, &send_evt); 
        if(rc != NCSCC_RC_SUCCESS) {
            LOG_ER("Failed to send response to agent/client over MDS");
        }
    }
}

/****************************************************************************
 * Name          : immnd_evt_proc_mds_evt
 *
 * Description   : Function to process the Events received from MDS
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *                 IMMSV_EVT *evt - Received Event structure
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_mds_evt (IMMND_CB *cb,IMMND_EVT *evt)
{
    /*TRACE_ENTER();*/
    uns32 rc = NCSCC_RC_SUCCESS;

    if ( (evt->info.mds_info.change == NCSMDS_DOWN)&& \
        (evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMA_OM ||
            evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMA_OI))
    {
        TRACE_2("IMMA DOWN EVENT");
        assert(immnd_is_immd_up(cb));/*ABT Hmmmm. should be sending from here?*/
        immnd_proc_imma_down(cb,evt->info.mds_info.dest,
            evt->info.mds_info.svc_id);
    }
    else if (( evt->info.mds_info.change == NCSMDS_DOWN) && \
        evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMD) 
    {
        TRACE_2("IMMD DOWN EVENT - We never get here do we?");
        /*immnd_proc_immd_down(cb);*/
    }
    else if (( evt->info.mds_info.change == NCSMDS_UP) && \
        (evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMA_OM ||
            evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMA_OM))
    {
        TRACE_2("IMMA UP EVENT");
    }   
    else if (( evt->info.mds_info.change == NCSMDS_RED_UP) && \
	    ( evt->info.mds_info.role == SA_AMF_HA_ACTIVE) && \
        evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMD)
    {
        TRACE_2("IMMD new activeEVENT");
        /*immnd_evt_immd_new_active(cb);*/
    }
    else if (( evt->info.mds_info.change == NCSMDS_CHG_ROLE) && \
        ( evt->info.mds_info.role == SA_AMF_HA_ACTIVE) && \
        ( evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMD))
    {

        TRACE_2("IMMD FAILOVER ??");
        /* The IMMD has failed over. */
    } else if(evt->info.mds_info.svc_id == NCSMDS_SVC_ID_IMMND)
    {
        LOG_NO("MDS SERVICE EVENT OF TYPE IMMND!!");
    }
    /*TRACE_LEAVE();*/
    return rc;
}

/****************************************************************************
 * Name          : immnd_evt_proc_cb_dump
 * Description   : Function to dump the Control Block
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 * Return Values : NCSCC_RC_SUCCESS/Error.
 * Notes         : None.
 *****************************************************************************/
static uns32 immnd_evt_proc_cb_dump(IMMND_CB *cb)
{
    immnd_cb_dump();

    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Name          : immnd_is_immd_up
 *
 * Description   : This routine tests whether IMMD is up or down
 * Arguments     : cb       - IMMND Control Block pointer
 *
 * Return Values : TRUE/FALSE
 *****************************************************************************/
uns32 immnd_is_immd_up(IMMND_CB *cb)
{  
    uns32 is_immd_up; 

    m_NCS_LOCK(&cb->immnd_immd_up_lock, NCS_LOCK_WRITE);

    is_immd_up = cb->is_immd_up;

    m_NCS_UNLOCK(&cb->immnd_immd_up_lock, NCS_LOCK_WRITE);

    return is_immd_up;
}


