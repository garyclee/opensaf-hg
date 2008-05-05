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

#include <alloca.h>
#include <time.h>
#include <limits.h>
#include "lgs.h"
#include "lgs_util.h"
#include "lgs_fmt.h"

static uns32 lgs_process_api_evt (lgsv_lgs_evt_t  *evt);
static uns32 lgs_proc_lga_updn_mds_msg(lgsv_lgs_evt_t  *evt);
static uns32 lgs_proc_quiesced_ack_evt(lgsv_lgs_evt_t  *evt);
static uns32 lgs_proc_init_msg (lgs_cb_t *, lgsv_lgs_evt_t  *evt);        
static uns32 lgs_proc_finalize_msg (lgs_cb_t *, lgsv_lgs_evt_t  *evt);
static uns32 lgs_proc_lstr_open_sync_msg (lgs_cb_t *, lgsv_lgs_evt_t  *evt);
static uns32 lgs_proc_lstr_close_msg (lgs_cb_t *, lgsv_lgs_evt_t  *evt);
static uns32 lgs_proc_write_log_async_msg (lgs_cb_t *, lgsv_lgs_evt_t  *evt);

static const 
LGSV_LGS_EVT_HANDLER 
lgs_lgsv_top_level_evt_dispatch_tbl[LGSV_LGS_EVT_MAX - LGSV_LGS_EVT_BASE] =
{
    lgs_process_api_evt,
    lgs_proc_lga_updn_mds_msg,
    lgs_proc_lga_updn_mds_msg,
    lgs_proc_quiesced_ack_evt
};

/* Dispatch table for LGA_API realted messages */
static const
LGSV_LGS_LGA_API_MSG_HANDLER lgs_lga_api_msg_dispatcher[LGSV_API_MAX - LGSV_API_BASE_MSG] =
{
    lgs_proc_init_msg,
    lgs_proc_finalize_msg,
    lgs_proc_lstr_open_sync_msg,
    lgs_proc_lstr_close_msg,
    lgs_proc_write_log_async_msg,
};

/****************************************************************************
 *
 * lgs_add_reglist_entry() - Inserts a LGA_REG_REC registration list element.
 *
 ***************************************************************************/
uns32 lgs_add_reglist_entry (lgs_cb_t *cb, MDS_DEST dest, uns32 reg_id,
                             lgs_stream_list_t *stream_list)
{
    uns32          rs = NCSCC_RC_SUCCESS;
    lga_reg_rec_t    *rec;
    lgs_stream_list_t *tmp_sl;

    TRACE_ENTER();
    if (NULL == (rec = calloc(1, sizeof(lga_reg_rec_t))))
    {
        TRACE("calloc FAILED");
        return(NCSCC_RC_OUT_OF_MEM);
    }

    /** Initialize the record **/
    if ((cb->ha_state == SA_AMF_HA_STANDBY) && (reg_id > cb->last_reg_id))
        cb->last_reg_id = reg_id;
    rec->reg_id = reg_id;
    rec->lga_client_dest = dest;
    rec->reg_id_Net = m_NCS_OS_HTONL(rec->reg_id);
    rec->pat_node.key_info = (uns8*)&rec->reg_id_Net;
    rec->first_stream = stream_list;
    rec->last_stream = stream_list;
    tmp_sl = stream_list;
    while (tmp_sl != NULL)
    {
        TRACE("sync, add stream_id: %u to regid: %u",
              tmp_sl->stream_id, reg_id);
        rec->last_stream = tmp_sl;
        tmp_sl = tmp_sl->next;
    }

    /** Insert the record into the patricia tree **/
    if (NCSCC_RC_SUCCESS != ncs_patricia_tree_add (&cb->lga_reg_list, &rec->pat_node))
    {
        TRACE("FAILED: ncs_patricia_tree_add");
        free(rec);
        return NCSCC_RC_FAILURE;
    }

    TRACE_LEAVE();
    return rs;

}

 /****************************************************************************
 *
 * lgs_lga_entry_valid
 * 
 *  Searches the cb->lga_reg_list for an reg_id entry whos MDS_DEST equals
 *  that passed DEST and returns TRUE if itz found.
 *
 * This routine is typically used to find the validity of the lga down rec from standby 
 * LGA_DOWN_LIST as  LGA client has gone away.
 *
 ****************************************************************************/
NCS_BOOL lgs_lga_entry_valid(lgs_cb_t *cb, MDS_DEST mds_dest)
{
    lga_reg_rec_t    *rp = NULL;

    rp = (lga_reg_rec_t *)ncs_patricia_tree_getnext(&cb->lga_reg_list,(uns8 *)0);

    while (rp != NULL)
    {
        if (m_NCS_MDS_DEST_EQUAL(&rp->lga_client_dest, &mds_dest))
        {
            return TRUE;
        }

        rp = (lga_reg_rec_t *)ncs_patricia_tree_getnext(
            &cb->lga_reg_list,(uns8 *)&rp->reg_id_Net);
    }

    return FALSE;
}

void lgs_remove_stream_id_from_rec(uns32 reg_id, uns32 stream_id)
{
   lgs_stream_list_t *cur_rec;
   lgs_stream_list_t *last_rec, *tmp_rec;
   uns32        regId_Net;
   lga_reg_rec_t *rec;
   TRACE_ENTER();

   regId_Net = m_NCS_OS_HTONL(reg_id);

   /** Get the node pointer from the patricia tree **/
   if (NULL == (rec = 
                (lga_reg_rec_t *)ncs_patricia_tree_get(&lgs_cb->lga_reg_list, 
                                                     (uns8*)&regId_Net)))
   {
       TRACE("ncs_patricia_tree_get FAILED for reg_id: %u", reg_id);
       goto all_done;
   }

   if (NULL == rec->first_stream) 
   {
      goto all_done;
   }
   cur_rec = rec->first_stream;
   last_rec = rec->first_stream;
   do {
      TRACE_4("reg_id: %u, stream id: %u", reg_id, cur_rec->stream_id);
      if (stream_id == cur_rec->stream_id) 
      {
         TRACE_4("reg_id: %u, REMOVE stream id: %u", reg_id, cur_rec->stream_id);
         tmp_rec = cur_rec->next;
         last_rec->next = tmp_rec;
         if (rec->first_stream == cur_rec)
            rec->first_stream = tmp_rec;
         free(cur_rec);
         break;
      }
      last_rec = cur_rec;
      cur_rec =last_rec->next;
   }while (NULL != cur_rec);

all_done:
      TRACE_LEAVE();   
}


static void lgs_remove_reg_rec_open_streams(lga_reg_rec_t *rec)
{
   lgs_stream_list_t *cur_rec;
   lgs_stream_list_t *tmp_rec;

   TRACE_ENTER();
   if (NULL == rec->first_stream) 
   {
      goto all_done;
   }
   cur_rec = rec->first_stream;
   do {
       log_stream_t *stream = log_stream_get_by_id(cur_rec->stream_id);
       TRACE_4("reg_id: %u, REMOVE stream id: %u", rec->reg_id, cur_rec->stream_id);
       log_stream_close(stream);
       tmp_rec = cur_rec->next;
       free(cur_rec);
       cur_rec = tmp_rec;
   } while (NULL != cur_rec);
all_done:
      TRACE_LEAVE();   
}

/****************************************************************************
 *
 * lgs_remove_reglist_entry() - Remove a LGA_REG_REC registration list element
 *                              from an ordered list.
 *
 *  If the regid is zero, which is not a valid regid normally, and the
 *  remove_all flag is TRUE, remove all registrations. This is only called
 *  upon a shutdown of LGS.
 *
 ***************************************************************************/
uns32 lgs_remove_reglist_entry (lgs_cb_t *cb, uns32 reg_id, NCS_BOOL remove_all)
{
    lga_reg_rec_t *rec_to_del;
    uns32        status = NCSCC_RC_SUCCESS;
    uns32        regId_Net;

    TRACE_ENTER();

    /** decide if all records are to be deleted **/
    if ((reg_id == 0) && (remove_all == TRUE))
    {
        rec_to_del = (lga_reg_rec_t *)
                     ncs_patricia_tree_getnext(&cb->lga_reg_list,(uns8 *)0);

        if (rec_to_del)
        {
            while (rec_to_del)
            {
                /** Close all open channels (and remove subscriptions)
                 ** for this registration ID.
                 **/

                /** delete the node from the tree 
                 **/
                ncs_patricia_tree_del(&cb->lga_reg_list, 
                                      &rec_to_del->pat_node); 

                /** Store the regId_Net for get Next
                 **/
                regId_Net = rec_to_del->reg_id_Net;

                /** Free the record 
                 **/
                free(rec_to_del);

                /** Fetch the next record 
                 **/
                rec_to_del = 
                (lga_reg_rec_t *)ncs_patricia_tree_getnext(&cb->lga_reg_list,
                                                         (uns8 *)&regId_Net);
            }
        }
    }
    else
    {
        /** Remove only one record specified by reg_id 
         **/ 
        regId_Net = m_NCS_OS_HTONL(reg_id);

        /** Get the node pointer from the patricia tree 
         **/
        if (NULL == (rec_to_del = 
                     (lga_reg_rec_t *)ncs_patricia_tree_get(&cb->lga_reg_list, 
                                                          (uns8*)&regId_Net)))
        {
            TRACE("ncs_patricia_tree_get - not found");
            status = NCSCC_RC_FAILURE;
            goto done;
        }

        lgs_remove_reg_rec_open_streams(rec_to_del);

        if (NCSCC_RC_SUCCESS != (status = ncs_patricia_tree_del(&cb->lga_reg_list, 
                                                                &rec_to_del->pat_node)))
        {
            TRACE("ncs_patricia_tree_del FAILED");
            goto done;
        }

        /* Free the record */
        free(rec_to_del);
    }

done:
    TRACE_LEAVE();
    return status;
}

/****************************************************************************
 *
 * lgs_remove_regid_by_mds_dest
 *
 *  Searches the reglist for a registration entry whos MDS_DEST equals
 *  that passed in and removes the registration, and all associated records
 *  pertaining to this registration from our internal lists.
 *
 * This routine is typically used to cleanup after being notified that an
 * LGA client has gone away.
 *
 ****************************************************************************/
uns32 lgs_remove_regid_by_mds_dest(lgs_cb_t *cb, MDS_DEST mds_dest)
{
    uns32          rc = NCSCC_RC_SUCCESS;
    lga_reg_rec_t    *rp = NULL;
    uns32           regId_Net; 
    TRACE_ENTER();
    rp = (lga_reg_rec_t *)ncs_patricia_tree_getnext(&cb->lga_reg_list,(uns8 *)0);

    while (rp != NULL)
    {
        /** Store the regId_Net for get Next
         **/
        regId_Net = rp->reg_id_Net;
        if (m_NCS_MDS_DEST_EQUAL(&rp->lga_client_dest, &mds_dest))
        {
            TRACE_1("remove rp->reg_id = %d",
                   (int)rp->reg_id);
            rc = lgs_remove_reglist_entry(cb, rp->reg_id, FALSE);
        }

        rp = (lga_reg_rec_t *)ncs_patricia_tree_getnext
             (&cb->lga_reg_list,(uns8 *)&regId_Net);
    }
    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_proc_lga_updn_mds_msg
 *
 * Description   : This is the function which is called when lgs receives any
 *                 a LGA UP/DN message via MDS subscription.
 *
 * Arguments     : msg  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_proc_lga_updn_mds_msg (lgsv_lgs_evt_t *evt)
{
    TRACE_ENTER();

    switch (evt->evt_type)
    {
        case LGSV_LGS_EVT_LGA_UP:
            break;
        case LGSV_LGS_EVT_LGA_DOWN:
            /* Remove this LGA entry from our processing lists */
            lgs_remove_regid_by_mds_dest(lgs_cb, evt->fr_dest);
            break;
        default:
            TRACE("Unknown evt type!!!");
            break;
    }

    TRACE_LEAVE();   
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_proc_quiesced_ack_evt
 *
 * Description   : This is the function which is called when lgs receives an
 *                       quiesced ack event from MDS 
 *
 * Arguments     : evt  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_proc_quiesced_ack_evt(lgsv_lgs_evt_t *evt)
{
    TRACE_ENTER();
    if (lgs_cb->is_quisced_set == TRUE)
    {
        lgs_cb->ha_state = SA_AMF_HA_QUIESCED;
        /* Inform MBCSV of HA state change */
        if (lgs_mbcsv_change_HA_state(lgs_cb) != NCSCC_RC_SUCCESS)
            TRACE("lgs_mbcsv_change_HA_state FAILED");

        /* Update control block */
        saAmfResponse(lgs_cb->amf_hdl, lgs_cb->amf_invocation_id, SA_AIS_OK);  
        lgs_cb->is_quisced_set = FALSE;
    }
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_cb_init
 *
 * Description   : This function initializes the LGS_CB including the 
 *                 Patricia trees.
 *                 
 *
 * Arguments     : lgs_cb * - Pointer to the LGS_CB.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 lgs_cb_init(lgs_cb_t *lgs_cb)
{
    NCS_PATRICIA_PARAMS reg_param;

    TRACE_ENTER();

    memset(&reg_param, 0, sizeof(NCS_PATRICIA_PARAMS));

    reg_param.key_size = sizeof(uns32);

    /* Assign Initial HA state */
    lgs_cb->ha_state = LGS_HA_INIT_STATE;
    lgs_cb->csi_assigned = FALSE; 

    /* Assign Version. Currently, hardcoded, This will change later */
    m_GET_MY_VERSION(lgs_cb->log_version);

    /* Initialize patricia tree for reg list*/
    if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&lgs_cb->lga_reg_list, &reg_param))
        return NCSCC_RC_FAILURE;

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_send_ack
 *
 * Description   : This is the function which is called when lgs receives a
 *                 LGSV_LGA_INITIALIZE message.
 *
 * Arguments     : msg  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_send_ack (lgs_cb_t *cb,
                           lgsv_lgs_evt_t  *evt,
                           SaAisErrorT error)
{
    uns32              rc = NCSCC_RC_SUCCESS;
    LGSV_MSG           msg;

    TRACE_ENTER();   
    memset(&msg, 0, sizeof(LGSV_MSG));
    msg.type = LGSV_LGS_CBK_MSG;
    msg.info.cbk_info.type = LGSV_LGS_WRITE_LOG_CBK;
    msg.info.cbk_info.lgs_reg_id = 
    evt->info.msg.info.api_info.param.write_log_async.reg_id;
    msg.info.cbk_info.inv = 
    evt->info.msg.info.api_info.param.write_log_async.invocation;
    msg.info.cbk_info.write_cbk.error = error;

    rc = lgs_mds_msg_send(cb, 
                          &msg,
                          &evt->fr_dest,
                          &evt->mds_ctxt,
                          MDS_SEND_PRIORITY_HIGH);
    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE("send failed"); /* TODO: find out why ev. retry? */
    }
    TRACE_LEAVE();
    return(rc);
}   

/****************************************************************************
 * Name          : lgs_proc_init_msg
 *
 * Description   : This is the function which is called when lgs receives a
 *                 LGSV_LGA_INITIALIZE message.
 *
 * Arguments     : msg  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_proc_init_msg (lgs_cb_t *cb, lgsv_lgs_evt_t  *evt)
{
    uns32              rc = NCSCC_RC_SUCCESS;
    uns32              async_rc = NCSCC_RC_SUCCESS;
    uns32              loop_count=0;
    SaVersionT         *version=NULL;
    LGSV_MSG           msg;
    LGS_CKPT_DATA      ckpt;
    TRACE_ENTER();   
    /* Validate the version */
    version = &(evt->info.msg.info.api_info.param.init.version);
    if (!m_LGA_VER_IS_VALID(version))
    {
        /* Send response back with error code */
        rc = SA_AIS_ERR_VERSION;
        TRACE("version FAILED");
        m_LGS_LGSV_INIT_MSG_FILL(msg, rc, 0)
        rc = lgs_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                              MDS_SEND_PRIORITY_HIGH);
        if (rc != NCSCC_RC_SUCCESS)
            TRACE("version FAILED");
        TRACE_LEAVE();
        return(rc);
    }
    /*
     * I'm allocating new reg_id's this way on the wild chance we wrap
     * around the MAX_INT value and try to use a value that's still in use
     * and valid at the begining of the range. This way we'll keep trying
     * until we find the next open slot. But we'll only try a max of
     * MAX_REG_RETRIES times before giving up completly and returning an error.
     * It's highly unlikely we'll ever wrap, but at least we'll handle it.
     */
#define MAX_REG_RETRIES 100
    loop_count = 0;
    do
    {
        loop_count++;

        /* Allocate a new regID */
        if (cb->last_reg_id == INT_MAX)   /* Handle integer wrap-around */
            cb->last_reg_id = 0;

        cb->last_reg_id++;

        /* Add this regid to the registration linked list.             */
        rc = lgs_add_reglist_entry(cb, evt->fr_dest, cb->last_reg_id, NULL);
    }while ((rc == NCSCC_RC_FAILURE) && (loop_count < MAX_REG_RETRIES));

    /* If we still have a bad status, return failure */
    if (rc != NCSCC_RC_SUCCESS)
    {
        /* Send response back with error code */
        rc = SA_AIS_ERR_LIBRARY;
        TRACE("FAILED");
        m_LGS_LGSV_INIT_MSG_FILL(msg, rc, 0)
        rc = lgs_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                              MDS_SEND_PRIORITY_HIGH);
        if (rc != NCSCC_RC_SUCCESS)
            TRACE("FAILED");
        return(rc);
    }

    /* Send response back with assigned reg_id value */
    rc = SA_AIS_OK;
    m_LGS_LGSV_INIT_MSG_FILL(msg, rc, cb->last_reg_id)
    rc = lgs_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                          MDS_SEND_PRIORITY_HIGH);
    if (rc != NCSCC_RC_SUCCESS)
        TRACE("lgs_mds_msg_send FAILED");

    if (cb->ha_state == SA_AMF_HA_ACTIVE) /*Revisit this */
    {
        memset(&ckpt, 0, sizeof(ckpt)); 
        m_LGSV_FILL_ASYNC_UPDATE_REG(ckpt,cb->last_reg_id,evt->fr_dest)
        async_rc = lgs_send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD); 
        if (async_rc != NCSCC_RC_SUCCESS)
        {
            /* log it */
            TRACE("send_async_update");
        }
        else
        {
            TRACE_4("REG_REC ASYNC UPDATE SEND SUCCESS....."); 
        }

    }
    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_proc_finalize_msg
 *
 * Description   : This is the function which is called when lgs receives a
 *                 LGSV_LGA_FINALIZE message.
 *
 * Arguments     : msg  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_proc_finalize_msg(lgs_cb_t *cb, lgsv_lgs_evt_t  *evt)
{
    LGS_CKPT_DATA ckpt;
    uns32 reg_id = evt->info.msg.info.api_info.param.finalize.reg_id;

    TRACE_ENTER();

    /* This insure all resources allocated by this registration are freed. */
    (void) lgs_remove_reglist_entry(cb, reg_id, FALSE);

    m_LGSV_FILL_ASYNC_UPDATE_FINALIZE(ckpt, reg_id);
    if (lgs_send_async_update(lgs_cb, &ckpt, NCS_MBCSV_ACT_RMV) != NCSCC_RC_SUCCESS)
        TRACE("send_async_update");

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

static uns32 lgs_send_stream_open_async_update(
    lgs_cb_t *cb,
    log_stream_t *logStream,
    LGSV_LGA_LSTR_OPEN_SYNC_PARAM  *open_sync_param)
{
    LGS_CKPT_DATA    ckpt;
    uns32 async_rc = NCSCC_RC_SUCCESS;
    TRACE_ENTER();
    if (cb->ha_state == SA_AMF_HA_ACTIVE)
    {
        memset(&ckpt, 0, sizeof(ckpt));
        ckpt.header.ckpt_rec_type=LGS_CKPT_OPEN_STREAM;
        ckpt.header.num_ckpt_records = 1;
        ckpt.header.data_len = 1;
        ckpt.ckpt_rec.stream_open.regId = open_sync_param->reg_id;
        ckpt.ckpt_rec.stream_open.streamId = logStream->streamId;

        ckpt.ckpt_rec.stream_open.logFile = logStream->fileName;
        ckpt.ckpt_rec.stream_open.logPath = logStream->pathName;
        ckpt.ckpt_rec.stream_open.logFileCurrent = logStream->logFileCurrent;
        ckpt.ckpt_rec.stream_open.fileFmt = logStream->logFileFormat;
        ckpt.ckpt_rec.stream_open.logStreamName = (char*)logStream->name;
        ckpt.ckpt_rec.stream_open.logFileCurrent = logStream->logFileCurrent;

        ckpt.ckpt_rec.stream_open.maxFileSize = logStream->maxLogFileSize;
        ckpt.ckpt_rec.stream_open.maxLogRecordSize = logStream->fixedLogRecordSize;
        ckpt.ckpt_rec.stream_open.logFileFullAction = logStream->logFullAction;
        ckpt.ckpt_rec.stream_open.maxFilesRotated = logStream->maxFilesRotated;
        ckpt.ckpt_rec.stream_open.creationTimeStamp = logStream->creationTimeStamp;
        ckpt.ckpt_rec.stream_open.numOpeners = logStream->numOpeners;
        ckpt.ckpt_rec.stream_open.streamType = logStream->streamType;

        async_rc = lgs_send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
        if (async_rc == NCSCC_RC_SUCCESS)
        {
            TRACE_4("REG_REC ASYNC UPDATE SEND SUCCESS...");
        }
    }
    TRACE_LEAVE();
    return async_rc;
}   

int lgs_add_stream_to_reg_id(uns32 reg_id, log_stream_t *stream)
{
   uns32 rs = NCSCC_RC_SUCCESS;
   lga_reg_rec_t *rec;
   lgs_stream_list_t *cur_rec;
   uns32           regId_Net; 

TRACE_ENTER();
   regId_Net = m_NCS_OS_HTONL(reg_id);

   /** Get the node pointer from the patricia tree 
    **/
   if (NULL == (rec = 
                (lga_reg_rec_t *)ncs_patricia_tree_get(&lgs_cb->lga_reg_list, 
                                                     (uns8*)&regId_Net)))
   {
       TRACE("ncs_patricia_tree_get FAILED");
       rs = NCSCC_RC_FAILURE;
       goto err_exit;
   }
   TRACE_4("insert in regid %u", reg_id);
   if (NULL != rec->first_stream) 
   {
      cur_rec = rec->first_stream;
      while (NULL != cur_rec->next)
      {
         cur_rec = cur_rec->next;
      }
      TRACE_4("insert in regid %u, stream id %u", reg_id, stream->streamId);
      cur_rec->next = calloc(1, sizeof(lgs_stream_list_t));
      cur_rec->next->stream_id = stream->streamId;
      rec->last_stream = cur_rec->next;
   }
   else 
   {
      TRACE_4("First element, insert in regid %u, stream id %u", reg_id, stream->streamId);
      rec->first_stream = calloc(1, sizeof(lgs_stream_list_t));
      rec->last_stream = rec->first_stream;
      rec->first_stream->stream_id = stream->streamId;
   }
err_exit:
TRACE_LEAVE();
      return rs;
}

SaAisErrorT lgs_create_new_app_stream(
    LGSV_LGA_LSTR_OPEN_SYNC_PARAM *open_sync_param,
    log_stream_t **o_stream)
{
    SaAisErrorT ais_rv = SA_AIS_OK;
    log_stream_t *stream;

    TRACE_ENTER();

    if (open_sync_param->lstr_name.length > SA_MAX_NAME_LENGTH)
    {
        TRACE("Name too long");
        ais_rv = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* Check the format string */
    SaBoolT flag = lgs_validateFormatExpression(open_sync_param->logFileFmt,
        STREAM_TYPE_APPLICATION, &stream->twelveHourModeFlag);
    if (flag == SA_FALSE)
    {
        TRACE("format expression failure");
        ais_rv = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* Verify that path and file are unique */
    stream = log_stream_getnext_by_name(NULL);
    while (stream != NULL)
    {
        if ((strncmp(stream->fileName, open_sync_param->logFileName.value, NAME_MAX) == 0) &&
            (strncmp(stream->pathName, open_sync_param->logFilePathName.value, SA_MAX_NAME_LENGTH) == 0))
        {
            TRACE("pathname already exist");
            ais_rv = SA_AIS_ERR_INVALID_PARAM;
            goto done;
        }
        stream = log_stream_getnext_by_name(stream->name);
    }

    stream = log_stream_new(&open_sync_param->lstr_name,
                            open_sync_param->logFileName.value,
                            open_sync_param->logFilePathName.value,
                            open_sync_param->maxLogFileSize,
                            open_sync_param->maxLogRecordSize,
                            SA_TRUE,
                            open_sync_param->logFileFullAction,
                            open_sync_param->maxFilesRotated,
                            open_sync_param->logFileFmt,
                            STREAM_TYPE_APPLICATION,
                            STREAM_NEW);
    if (stream == NULL)
    {
        ais_rv = SA_AIS_ERR_NO_MEMORY;
        goto done;
    }
    *o_stream = stream;

done:
    TRACE_LEAVE();
    return ais_rv;
}

static SaAisErrorT lgs_file_attribute_cmp(
    LGSV_LGA_LSTR_OPEN_SYNC_PARAM  *open_sync_param,
    log_stream_t* applicationStream)
{
    SaAisErrorT  rs = SA_AIS_OK;

    TRACE_ENTER();
    TRACE_2("old applicationStream: %s", applicationStream->name);
    TRACE_2("New app stream: %s", open_sync_param->lstr_name.value);

    TRACE_2("log path, new: %s existing: %s",
           open_sync_param->logFilePathName.value,
           applicationStream->pathName);

    if (open_sync_param->maxLogFileSize != applicationStream->maxLogFileSize ||
        open_sync_param->maxLogRecordSize != applicationStream->fixedLogRecordSize ||
        open_sync_param->maxFilesRotated != applicationStream->maxFilesRotated)
    {
        TRACE("create params differ, new: mfs %llu, mlrs %u , mr %u"
              " existing: mfs %llu, mlrs %u , mr %u",
              open_sync_param->maxLogFileSize,
              open_sync_param->maxLogRecordSize,
              open_sync_param->maxFilesRotated,
              applicationStream->maxLogFileSize,
              applicationStream->fixedLogRecordSize,
              applicationStream->maxFilesRotated);
        rs = SA_AIS_ERR_INVALID_PARAM;
    }
    else if (applicationStream->logFullAction != open_sync_param->logFileFullAction)
    {
        TRACE("logFileFullAction create params differs, new: %d, old: %d",
              open_sync_param->logFileFullAction,
              applicationStream->logFullAction);
        rs = SA_AIS_ERR_INVALID_PARAM;
    }
/* TODO implement haProperty? */
/*    else if (applicationStream->haProperty != open_sync_param->haProperty) */
/*    { */
/*       TRACE("Different haProperty, new: %d existing: %d", */
/*                   open_sync_param->haProperty, */
/*                   applicationStream->haProperty; */
/*       rs = SA_AIS_ERR_INVALID_PARAM; */
/*    } */
#if 0
    else if (applicationStream->name.length != open_sync_param->lstr_name.length)
    {
        TRACE("Stream name length differs, new: %d existing: %d",
              open_sync_param->lstr_name.length,
              applicationStream->name.length);
        rs = SA_AIS_ERR_INVALID_PARAM;
    }
    else if (strncmp((const char *)applicationStream->name,
                     (const char *)open_sync_param->lstr_name.value,
                     applicationStream->name.length)  != 0)
    {
        /* TODO: make all SaNameT safe could be without null termitation */
        open_sync_param->lstr_name.value[open_sync_param->lstr_name.length+1]='\0';
        TRACE("Stream name differs, new: %s existing: %s",
              open_sync_param->lstr_name.value, applicationStream->name);
        rs = SA_AIS_ERR_INVALID_PARAM;
    }
#endif
    else if (strcmp(applicationStream->fileName,
                    open_sync_param->logFileName.value) != 0)
    {
        /* TODO: make all SaNameT safe could be without null termitation */
        open_sync_param->lstr_name.value[open_sync_param->logFileName.length+1]='\0';
        TRACE("logFileName differs, new: %s existing: %s",
              open_sync_param->logFileName.value,
              applicationStream->fileName);
        rs = SA_AIS_ERR_INVALID_PARAM;
    }
    else if (strcmp((const char *)applicationStream->pathName,
                    (const char *)open_sync_param->logFilePathName.value)  != 0)
    {
        TRACE("log file path differs, new: %s existing: %s",
              open_sync_param->logFilePathName.value,
              applicationStream->pathName);
        rs = SA_AIS_ERR_INVALID_PARAM;
    }
    else if (strcmp((const char *)applicationStream->logFileFormat,
                    (const char *)open_sync_param->logFileFmt)  != 0)
    {
        TRACE("logFile format differs, new: %s existing: %s",
              open_sync_param->logFileFmt,
              applicationStream->logFileFormat);
        rs = SA_AIS_ERR_INVALID_PARAM;
    }
    TRACE_LEAVE();
    return rs;
}

/****************************************************************************
 * Name          : lgs_proc_lstr_open_sync_msg
 *
 * Description   : This is the function which is called when lgs receives a
 *                 LGSV_LGA_LSTR_OPEN (log stream open) message.
 *
 * Arguments     : msg  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_proc_lstr_open_sync_msg(lgs_cb_t *cb, lgsv_lgs_evt_t  *evt)
{
    uns32  rc = NCSCC_RC_SUCCESS, async_rc = NCSCC_RC_SUCCESS;
    SaAisErrorT ais_rv = SA_AIS_OK;
    uns32 lstr_id = (uns32)-1;
    LGSV_MSG  msg;
    LGSV_LGA_LSTR_OPEN_SYNC_PARAM *open_sync_param =
        &(evt->info.msg.info.api_info.param.lstr_open_sync);
    log_stream_t *logStream;
    char name[SA_MAX_NAME_LENGTH + 1];

    TRACE_ENTER();

    /* Create null-terminated stream name */
    memcpy(name, open_sync_param->lstr_name.value, open_sync_param->lstr_name.length);
    memset(&name[open_sync_param->lstr_name.length], 0,
           SA_MAX_NAME_LENGTH + 1 - open_sync_param->lstr_name.length);

    TRACE("open stream '%s', reg_id %u", name, open_sync_param->reg_id);

    logStream = log_stream_get_by_name(name);
    if (logStream != NULL)
    {
        TRACE("existing stream - id %u", logStream->streamId);
        if (logStream->streamType == STREAM_TYPE_APPLICATION)
        {
            /* Verify the creation attributes for an existing appl. stream */
            ais_rv = lgs_file_attribute_cmp(open_sync_param, logStream);
            if (ais_rv != SA_AIS_OK)
                goto snd_rsp;
        }
    }
    else
    {
        TRACE("New app stream");
        ais_rv = lgs_create_new_app_stream(open_sync_param, &logStream);
        if (ais_rv != SA_AIS_OK)
            goto snd_rsp;
    }

    ais_rv = log_stream_open(logStream);
    if (ais_rv != SA_AIS_OK)
    {
        if (logStream != NULL)
            log_stream_delete(&logStream);

        goto snd_rsp;
    }

    log_stream_print(logStream);

    /* Create an association between this reg_id and the stream */
    rc = lgs_add_stream_to_reg_id(open_sync_param->reg_id, logStream);
    if (rc != NCSCC_RC_SUCCESS)
    {
        log_stream_close(logStream);
        ais_rv = SA_AIS_ERR_NO_RESOURCES;
        goto snd_rsp;
    }

    TRACE_4("logStream->streamId = %u", logStream->streamId);
    lstr_id = logStream->streamId;

snd_rsp:
    memset(&msg, 0, sizeof(LGSV_MSG));
    msg.type = LGSV_LGA_API_RESP_MSG;
    msg.info.api_resp_info.type = LGSV_LGA_LSTR_OPEN_SYNC_RSP_MSG;
    msg.info.api_resp_info.rc = ais_rv;
    msg.info.api_resp_info.param.lstr_open_rsp.lstr_id = lstr_id;
    TRACE_4("lstr_open_rsp.lstr_id: %u, rv: %u", lstr_id, ais_rv);
    rc = lgs_mds_msg_send(cb, &msg, &evt->fr_dest, &evt->mds_ctxt,
                          MDS_SEND_PRIORITY_HIGH);
    if (rc != NCSCC_RC_SUCCESS){
       TRACE("lgs_mds_msg_send FAILED");
       /* TODO: what to do exit here? */
    }

    if (ais_rv == SA_AIS_OK)
    {
       async_rc = lgs_send_stream_open_async_update(cb,
                                                    logStream,
                                                    open_sync_param);
       if (async_rc != NCSCC_RC_SUCCESS)
          TRACE("lgs_send_stream_open_async_update failed");
    }
    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_proc_lstr_close_msg
 *
 * Description   : This is the function which is called when lgs receives a
 *                 LGSV_LGA_LSTR_CLOSE message.
 *
 * Arguments     : msg  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_proc_lstr_close_msg(lgs_cb_t *cb, lgsv_lgs_evt_t *evt)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGSV_LGA_LSTR_CLOSE_PARAM  *close_param;
    log_stream_t *stream;
    LGS_CKPT_DATA ckpt;

    TRACE_ENTER();
    close_param = &(evt->info.msg.info.api_info.param.lstr_close);

    if ((stream = log_stream_get_by_id(close_param->lstr_id)) == NULL)
    {
        TRACE("Bad stream ID");
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    TRACE("close stream %s, id: %u", stream->name, stream->streamId);
    lgs_remove_stream_id_from_rec(close_param->reg_id, close_param->lstr_id);
    log_stream_close(stream);

    ckpt.header.ckpt_rec_type = LGS_CKPT_CLOSE_STREAM;
    ckpt.header.num_ckpt_records = 1;
    ckpt.header.data_len = 1;
    ckpt.ckpt_rec.stream_close.regId = close_param->reg_id;
    ckpt.ckpt_rec.stream_close.streamId = stream->streamId;

    if (lgs_send_async_update(cb, &ckpt, NCS_MBCSV_ACT_RMV) != NCSCC_RC_SUCCESS)
        LOG_WA("lgs_send_async_update failed");

done:
    TRACE_LEAVE();
    return rc;
}

static void free_write_log_async_param(LGSV_LGA_WRITE_LOG_ASYNC_PARAM  *param)
{
    if (param->logRecord->logHdrType == SA_LOG_GENERIC_HEADER)
    {
        SaLogGenericLogHeaderT *genLogH = &param->logRecord->logHeader.genericHdr;
        free(param->logRecord->logBuffer->logBuf);
        free(param->logRecord->logBuffer);
        free(genLogH->notificationClassId);
        free((void*)param->logRecord->logHeader.genericHdr.logSvcUsrName);
        free(param->logRecord);
    }
    else
    {
        SaLogNtfLogHeaderT *ntfLogH = &param->logRecord->logHeader.ntfHdr;
        free(param->logRecord->logBuffer->logBuf);
        free(param->logRecord->logBuffer);
        free(ntfLogH->notificationClassId);
        free(ntfLogH->notifyingObject);
        free(ntfLogH->notificationObject);
        free(param->logRecord);
    }
}

/****************************************************************************
 * Name          : lgs_proc_write_log_async_msg
 *
 * Description   : This is the function which is called when lgs receives a
 *                 LGSV_LGA_WRITE_LOG_ASYNC message.
 *
 * Arguments     : msg  - Message that was posted to the Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_proc_write_log_async_msg(lgs_cb_t *cb, lgsv_lgs_evt_t *evt)
{
    LGSV_LGA_WRITE_LOG_ASYNC_PARAM *write_log_async_param;
    write_log_async_param = &(evt->info.msg.info.api_info.param).write_log_async;
    log_stream_t* stream;
    SaAisErrorT error = SA_AIS_OK;
    SaStringT logOutputString;

    TRACE_ENTER();

    stream = log_stream_get_by_id(write_log_async_param->lstr_id);
    if (stream == NULL)
    {
        TRACE("Bad stream ID");
        error = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }

    logOutputString = alloca(stream->fixedLogRecordSize + 1);
    if (lgs_formatLogRecord(write_log_async_param->logRecord,
                    stream->logFileFormat,
                    stream->fixedLogRecordSize + 1,
                    logOutputString,
                    ++stream->logRecordId) != SA_AIS_OK)
    {
        error = SA_AIS_ERR_INVALID_PARAM; /* FIX? */
        goto done;
    }

    if (log_stream_write(stream, logOutputString) != 0)
    {
        error = SA_AIS_ERR_NO_RESOURCES;
        goto done;
    }

    /*
    ** FIX: Optimization: we only need to sync when file has been rotated
    ** Standby does stat() and calculates recordId and curFileSize.
    */
    /* TODO: send fail back if ack is wanted, Fix counter for application stream!! */
    if (cb->ha_state == SA_AMF_HA_ACTIVE)
    {
        uns32 async_rc;
        LGS_CKPT_DATA ckpt;
        memset(&ckpt, 0, sizeof(ckpt));
        ckpt.header.ckpt_rec_type=LGS_CKPT_LOG_WRITE;
        ckpt.header.num_ckpt_records = 1;
        ckpt.header.data_len = 1;
        ckpt.ckpt_rec.writeLog.recordId = stream->logRecordId;
        ckpt.ckpt_rec.writeLog.streamId = stream->streamId;
        ckpt.ckpt_rec.writeLog.curFileSize = stream->curFileSize;
        ckpt.ckpt_rec.writeLog.logFileCurrent = stream->logFileCurrent;

        async_rc = lgs_send_async_update(cb,&ckpt,NCS_MBCSV_ACT_ADD);
        if (async_rc != NCSCC_RC_SUCCESS)
            LOG_WA("lgs_send_async_update failed");
    }

done:
    if (write_log_async_param->ack_flags == SA_LOG_RECORD_WRITE_ACK)
        lgs_send_ack(cb, evt, error);

    free_write_log_async_param(write_log_async_param);

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : lgs_process_api_evt
 *
 * Description   : This is the function which is called when lgs receives an
 *                 event either because of an API Invocation or other internal
 *                 messages from LGA clients
 *
 * Arguments     : evt  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_process_api_evt (lgsv_lgs_evt_t  *evt)
{
    TRACE_ENTER();
    if (evt->evt_type  == LGSV_LGS_LGSV_MSG)
    {
        /* ignore one level... */
        if ((evt->info.msg.type  >= LGSV_BASE_MSG) && 
            (evt->info.msg.type  <  LGSV_MSG_MAX))
        {
            TRACE_1("API Type = %d", evt->info.msg.info.api_info.type);
            if ((evt->info.msg.info.api_info.type >= LGSV_API_BASE_MSG) && 
                (evt->info.msg.info.api_info.type <  LGSV_API_MAX))
            {
                if (lgs_lga_api_msg_dispatcher[evt->info.msg.info.api_info.type](lgs_cb, evt) != NCSCC_RC_SUCCESS)
                {
                    TRACE_2("lgs_lga_api_msg_dispatcher FAILED type: %d", (int)evt->info.msg.type);
                }
            }
        }
    }
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_process_evt
 *
 * Description   : This is the function which is called when lgs receives an
 *                 event of any kind.
 *
 * Arguments     : msg  - Message that was posted to the LGS Mail box.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_process_evt(lgsv_lgs_evt_t  *evt)
{
    if (lgs_cb->ha_state == SA_AMF_HA_ACTIVE)
    {
        if ((evt->evt_type >= LGSV_LGS_EVT_BASE) && 
            (evt->evt_type <=  LGSV_LGS_EVT_LGA_DOWN))
        {
            /** Invoke the evt dispatcher **/
            lgs_lgsv_top_level_evt_dispatch_tbl[evt->evt_type](evt);
        }
        else if (evt->evt_type == LGSV_EVT_QUIESCED_ACK)
        {
            TRACE("lgs_proc_quiesced_ack_evt call not done.");
            lgs_proc_quiesced_ack_evt(evt);
        }
        else
            TRACE("lgs_process_evt: FAIL event type not found");
    }
    else
    {
        if (evt->evt_type == LGSV_LGS_EVT_LGA_DOWN)
        {
            /** Invoke the evt dispatcher **/
            lgs_lgsv_top_level_evt_dispatch_tbl[evt->evt_type](evt);
        }
    }

    /* Free the event */
    if (NULL != evt)
        lgs_evt_destroy(evt);

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 LGS 
 *
 * Arguments     : mbx  - This is the mail box pointer on which LGS is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void lgs_process_mbx(SYSF_MBX *mbx)
{
    lgsv_lgs_evt_t *evt = NULL;

    evt = (lgsv_lgs_evt_t *) m_NCS_IPC_NON_BLK_RECEIVE(mbx,evt);
    if (evt != NULL)
    {
        if ((evt->evt_type >= LGSV_LGS_EVT_BASE) &&
            (evt->evt_type < LGSV_LGS_EVT_MAX))
        {
            /* This event belongs to LGS main event dispatcher */
            lgs_process_evt(evt);
        }
        else
        {
            TRACE_1("lgs_process_mbx: unknown event type");
            lgs_evt_destroy(evt);
        }

    }
}

