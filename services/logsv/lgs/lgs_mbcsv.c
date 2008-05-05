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

#include "lgs.h"

/*
LGS_CKPT_DATA_HEADER
       4                4               4                 2            
-----------------------------------------------------------------
| ckpt_rec_type | num_ckpt_records | tot_data_len |  checksum   | 
-----------------------------------------------------------------

LGSV_CKPT_COLD_SYNC_MSG
-----------------------------------------------------------------------------------------------------------------------
| LGS_CKPT_DATA_HEADER|LGSV_CKPT_REC 1st| next |LGSV_CKPT_REC 2nd| next ..|..|..|..|LGSV_CKPT_REC "num_ckpt_records" th |
-----------------------------------------------------------------------------------------------------------------------
*/

static uns32 lgs_ckpt_proc_close_stream(lgs_cb_t *cb, LGS_CKPT_DATA *data);
static uns32 lgs_edp_ed_close_stream_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                         NCSCONTEXT ptr, uns32 *ptr_data_len,
                                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                         EDU_ERR *o_err);

LGS_CKPT_HDLR lgs_ckpt_data_handler[LGS_CKPT_MSG_MAX - LGS_CKPT_MSG_BASE] = 
{
    lgs_ckpt_proc_reg_rec,
    lgs_ckpt_proc_finalize_rec,
    lgs_ckpt_proc_agent_down_rec,
    lgs_ckpt_proc_log_write,
    lgs_ckpt_proc_open_stream,
    lgs_ckpt_proc_close_stream
};

/****************************************************************************
 * Name          : lgsv_mbcsv_init 
 *
 * Description   : This function initializes the mbcsv interface and
 *                 obtains a selection object from mbcsv.
 *                 
 * Arguments     : LGS_CB * - A pointer to the lgs control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 lgs_mbcsv_init(lgs_cb_t *cb)
{
    uns32 rc;
    NCS_MBCSV_ARG arg;

    TRACE_ENTER();

    /* Initialize with MBCSv library */
    arg.i_op = NCS_MBCSV_OP_INITIALIZE;
    arg.info.initialize.i_mbcsv_cb = lgs_mbcsv_callback;
    arg.info.initialize.i_version = LGS_MBCSV_VERSION;
    arg.info.initialize.i_service = NCS_SERVICE_ID_LGS;

    if ((rc = ncs_mbcsv_svc(&arg)) != NCSCC_RC_SUCCESS)
    {
        TRACE("NCS_MBCSV_OP_INITIALIZE FAILED");
        goto done;
    }

    cb->mbcsv_hdl = arg.info.initialize.o_mbcsv_hdl;

    /* Open a checkpoint */
    arg.i_op = NCS_MBCSV_OP_OPEN;
    arg.i_mbcsv_hdl = cb->mbcsv_hdl;
    arg.info.open.i_pwe_hdl = (uns32)cb->mds_hdl;
    arg.info.open.i_client_hdl = 0;

    if ((rc = ncs_mbcsv_svc(&arg) != NCSCC_RC_SUCCESS))
    {
        TRACE("NCS_MBCSV_OP_OPEN FAILED");
        goto done;
    }
    cb->mbcsv_ckpt_hdl = arg.info.open.o_ckpt_hdl;

    /* Get Selection Object */
    arg.i_op = NCS_MBCSV_OP_SEL_OBJ_GET;
    arg.i_mbcsv_hdl = cb->mbcsv_hdl;
    arg.info.sel_obj_get.o_select_obj=0;
    if (NCSCC_RC_SUCCESS != (rc = ncs_mbcsv_svc(&arg)))
    {
        TRACE("NCS_MBCSV_OP_SEL_OBJ_GET FAILED");
        goto done;
    }

    cb->mbcsv_sel_obj = arg.info.sel_obj_get.o_select_obj;
    cb->ckpt_state = COLD_SYNC_IDLE;

    /* Disable warm sync */
    arg.i_op = NCS_MBCSV_OP_OBJ_SET;
    arg.i_mbcsv_hdl = cb->mbcsv_hdl;
    arg.info.obj_set.i_ckpt_hdl = cb->mbcsv_ckpt_hdl;
    arg.info.obj_set.i_obj = NCS_MBCSV_OBJ_WARM_SYNC_ON_OFF;
    arg.info.obj_set.i_val = FALSE;
    if (ncs_mbcsv_svc(&arg) != NCSCC_RC_SUCCESS)
    {
        TRACE("NCS_MBCSV_OP_OBJ_SET FAILED");
        goto done;
    }

done:
    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_mbcsv_change_HA_state 
 *
 * Description   : This function inform mbcsv of our HA state. 
 *                 All checkpointing operations are triggered based on the 
 *                 state.
 *
 * Arguments     : LGS_CB * - A pointer to the lgs control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : This function should be ideally called only once, i.e.
 *                 during the first CSI assignment from AVSv  .
 *****************************************************************************/

uns32 lgs_mbcsv_change_HA_state(lgs_cb_t *cb)
{
    NCS_MBCSV_ARG     mbcsv_arg;
    uns32             rc = SA_AIS_OK;
    memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

    /* Set the mbcsv args */
    mbcsv_arg.i_op = NCS_MBCSV_OP_CHG_ROLE;
    mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_hdl;
    mbcsv_arg.info.chg_role.i_ckpt_hdl = cb->mbcsv_ckpt_hdl;
    mbcsv_arg.info.chg_role.i_ha_state = cb->ha_state;

    if (SA_AIS_OK != (rc = ncs_mbcsv_svc(&mbcsv_arg) ))
    {
        TRACE("ncs_mbcsv_svc FAILED");
        return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;

}/*End lgs_mbcsv_change_HA_state*/


/****************************************************************************
 * Name          : lgs_mbcsv_change_HA_state 
 *
 * Description   : This function inform mbcsv of our HA state. 
 *                 All checkpointing operations are triggered based on the 
 *                 state.
 *
 * Arguments     : NCS_MBCSV_HDL - Handle provided by MBCSV during op_init. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : This function should be ideally called only once, i.e.
 *                 during the first CSI assignment from AVSv  .
 *****************************************************************************/
uns32 lgs_mbcsv_dispatch(NCS_MBCSV_HDL mbcsv_hdl)
{
    NCS_MBCSV_ARG mbcsv_arg;

    memset(&mbcsv_arg, 0, sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op = NCS_MBCSV_OP_DISPATCH;
    mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;
    mbcsv_arg.info.dispatch.i_disp_flags = SA_DISPATCH_ALL;

    return ncs_mbcsv_svc(&mbcsv_arg);
}

/****************************************************************************
 * Name          : lgs_mbcsv_callback
 *
 * Description   : This callback is the single entry point for mbcsv to 
 *                 notify lgs of all checkpointing operations. 
 *
 * Arguments     : NCS_MBCSV_CB_ARG - Callback Info pertaining to the mbcsv
 *                 event from ACTIVE/STANDBY LGS peer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Based on the mbcsv message type, the corresponding mbcsv
 *                 message handler shall be invoked.
 *****************************************************************************/
uns32 lgs_mbcsv_callback(NCS_MBCSV_CB_ARG *arg)
{
    uns32 rc = NCSCC_RC_SUCCESS;

    TRACE_ENTER();
    assert(arg != NULL);

    switch (arg->i_op)
    {
        case NCS_MBCSV_CBOP_ENC:
            /* Encode Request from MBCSv */
            rc = lgs_ckpt_encode_cbk_handler(arg);
            break;
        case NCS_MBCSV_CBOP_DEC:
            /* Decode Request from MBCSv */
            rc = lgs_ckpt_decode_cbk_handler(arg);
            if (rc != NCSCC_RC_SUCCESS)
                TRACE("lgs_ckpt_decode_cbk_handler FAILED");
            break;
        case NCS_MBCSV_CBOP_PEER:
            TRACE_2("NCS_MBCSV_CBOP_PEER ");
            /* LGS Peer info from MBCSv */
            rc = lgs_ckpt_peer_info_cbk_handler(arg);
            if (rc != NCSCC_RC_SUCCESS)
                TRACE("lgs_ckpt_peer_info_cbk_handler FAILED");
            break;
        case NCS_MBCSV_CBOP_NOTIFY:
            TRACE_2("NCS_MBCSV_CBOP_NOTIFY ");
            /* NOTIFY info from LGS peer */
            rc = lgs_ckpt_notify_cbk_handler(arg);
            if (rc != NCSCC_RC_SUCCESS)
                TRACE("lgs_ckpt_notify_cbk_handler FAILED");
            break;
        case NCS_MBCSV_CBOP_ERR_IND:
            TRACE_2("NCS_MBCSV_CBOP_ERR_IND ");
            /* Peer error indication info */
            rc = lgs_ckpt_err_ind_cbk_handler(arg);
            if (rc != NCSCC_RC_SUCCESS)
                TRACE("lgs_ckpt_err_ind_cbk_handler FAILED");
            break;
        default:
            rc  = NCSCC_RC_FAILURE;
            if (rc != NCSCC_RC_SUCCESS)
                TRACE("default FAILED");
            break;
    }

    TRACE_LEAVE(); 
    return rc;
}

/****************************************************************************
 * Name          : lgs_ckpt_encode_cbk_handler
 *
 * Description   : This function invokes the corresponding encode routine
 *                 based on the MBCSv encode request type.
 *
 * Arguments     : arg - Pointer to NCS_MBCSV_CB_ARG with encode info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_ckpt_encode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg)
{
    uns32 rc=NCSCC_RC_SUCCESS;
    uns16 mbcsv_version ;

    TRACE_ENTER();
    assert(cbk_arg != NULL);

    mbcsv_version = m_NCS_MBCSV_FMT_GET(
                                       cbk_arg->info.encode.i_peer_version,
                                       LGS_MBCSV_VERSION,
                                       LGS_MBCSV_VERSION_MIN);
    if (0 == mbcsv_version)
    {
        TRACE("Wrong mbcsv_version!!!\n");
        TRACE_LEAVE();
        return NCSCC_RC_FAILURE;
    }

    switch (cbk_arg->info.encode.io_msg_type)
    {
        case NCS_MBCSV_MSG_ASYNC_UPDATE:
            TRACE_2("ASYNC UPDATE ENCODE CALLED"); 
            /* Encode async update */
            if ((rc=lgs_ckpt_encode_async_update(lgs_cb, lgs_cb->edu_hdl, cbk_arg)) != NCSCC_RC_SUCCESS)
                TRACE("  lgs_ckpt_encode_async_update FAILED");
            break;

        case NCS_MBCSV_MSG_COLD_SYNC_REQ:
            TRACE_2("COLD SYNC REQ ENCODE CALLED"); 
            break;

        case NCS_MBCSV_MSG_COLD_SYNC_RESP:
            TRACE_2("COLD SYNC RESPONSE ENCODE CALLED"); 
            /* Encode cold sync response */
            rc = lgs_ckpt_enc_cold_sync_data(lgs_cb, cbk_arg, FALSE);
            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE(" COLD SYNC ENCODE FAIL....");
            }
            else
            {
                lgs_cb->ckpt_state = COLD_SYNC_COMPLETE;
                TRACE_2(" COLD SYNC RESPONSE SEND SUCCESS....");
            }
            break;

        case NCS_MBCSV_MSG_WARM_SYNC_REQ:
        case NCS_MBCSV_MSG_WARM_SYNC_RESP:
            TRACE_2("WARM SYNC called not used");
            break;

        case NCS_MBCSV_MSG_DATA_REQ:
            TRACE_2("DATA REQ DECODE called");
            break;

        case NCS_MBCSV_MSG_DATA_RESP:
        case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
            TRACE_2("DATA RESPONSE ENCODE CALLED"); 
            if ((rc = lgs_ckpt_enc_cold_sync_data(lgs_cb,cbk_arg,TRUE)) != NCSCC_RC_SUCCESS)
                TRACE("  lgs_ckpt_enc_cold_sync_data FAILED");
            break;
        default:
            TRACE_2("INCORRECT ENCODE CALLED"); 
            rc = NCSCC_RC_FAILURE;
            TRACE("  default FAILED");
            break;
    } /*End switch(io_msg_type)*/

    TRACE_LEAVE();
    return rc;

} /*End lgs_ckpt_encode_cbk_handler()*/

/****************************************************************************
 * Name          : lgs_ckpt_enc_cold_sync_data
 *
 * Description   : This function encodes cold sync data., viz
 *                 1.REGLIST
 *                 2.OPEN STREAMS
 *                 3.Async Update Count
 *                 in that order.
 *                 Each records contain a header specifying the record type
 *                 and number of such records.
 *
 * Arguments     : lgs_cb - pointer to the lgs control block. 
 *                 cbk_arg - Pointer to NCS_MBCSV_CB_ARG with encode info.
 *                 data_req - Flag to specify if its for cold sync or data
 *                 request for warm sync.
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_ckpt_enc_cold_sync_data(lgs_cb_t *lgs_cb, NCS_MBCSV_CB_ARG *cbk_arg, NCS_BOOL data_req)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    /* asynsc Update Count */
    uns8* async_upd_cnt =NULL;

    /* Currently, we shall send all data in one send.
     * This shall avoid "delta data" problems that are associated during
     * multiple sends
     */
    TRACE_2("COLD SYNC ENCODE START........");
    /* Encode Registration list */
    rc = lgs_edu_enc_reg_list(lgs_cb,&cbk_arg->info.encode.io_uba);
    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE("  lgs_edu_enc_reg_list FAILED");
        return NCSCC_RC_FAILURE;
    }
    rc = lgs_edu_enc_streams(lgs_cb,&cbk_arg->info.encode.io_uba);
    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE("  lgs_edu_enc_streams FAILED");
        return NCSCC_RC_FAILURE;
    }
    /* Encode the Async Update Count at standby*/

    /* This will have the count of async updates that have been sent,
       this will be 0 initially*/
    async_upd_cnt = ncs_enc_reserve_space(&cbk_arg->info.encode.io_uba,sizeof(uns32));
    if (async_upd_cnt == NULL)
    {
        /* Log this error */
        TRACE("  ncs_enc_reserve_space FAILED");
        return NCSCC_RC_FAILURE;
    }

    ncs_encode_32bit(&async_upd_cnt,lgs_cb->async_upd_cnt);
    ncs_enc_claim_space(&cbk_arg->info.encode.io_uba,sizeof(uns32));

    /* Set response mbcsv msg type to complete */
    if (data_req == TRUE)
        cbk_arg->info.encode.io_msg_type=NCS_MBCSV_MSG_DATA_RESP_COMPLETE;
    else
        cbk_arg->info.encode.io_msg_type=NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
    TRACE_2("COLD SYNC ENCODE END........");
    return rc;
}/*End  lgs_ckpt_enc_cold_sync_data() */

static uns32 lgs_ckpt_stream_open_set(
    log_stream_t *logStream,
    LGS_CKPT_STREAM_OPEN *stream_open)
{
    uns32 rc = NCSCC_RC_SUCCESS;

    TRACE_ENTER();
    memset(stream_open, 0, sizeof(LGS_CKPT_STREAM_OPEN));
    stream_open->regId = -1; /* not used in this message */
    stream_open->streamId = logStream->streamId;
    stream_open->logFile = logStream->fileName;
    stream_open->logPath = logStream->pathName;
    stream_open->logFileCurrent = logStream->logFileCurrent;
    stream_open->fileFmt = logStream->logFileFormat;
    stream_open->logStreamName = (char*)logStream->name;
    stream_open->logFileCurrent = logStream->logFileCurrent;
    stream_open->maxFileSize = logStream->maxLogFileSize;
    stream_open->maxLogRecordSize = logStream->fixedLogRecordSize;
    stream_open->logFileFullAction = logStream->logFullAction;
    stream_open->maxFilesRotated = logStream->maxFilesRotated;
    stream_open->creationTimeStamp = logStream->creationTimeStamp;
    stream_open->numOpeners = logStream->numOpeners;
    stream_open->streamType = logStream->streamType;
    TRACE_LEAVE();
    return rc;
}   

uns32 lgs_edu_enc_streams(lgs_cb_t *cb, NCS_UBAID *uba)
{
   log_stream_t *log_stream_rec=NULL;
   LGS_CKPT_STREAM_OPEN *ckpt_stream_rec;
   EDU_ERR   ederror=0;
   uns32     rc=NCSCC_RC_SUCCESS, num_rec=0;
   uns8 *pheader=NULL;
   LGS_CKPT_HEADER ckpt_hdr;

   /* Prepare reg. structure to encode */
   ckpt_stream_rec = malloc(sizeof(LGS_CKPT_STREAM_OPEN));
   if (ckpt_stream_rec == NULL)
   {
       TRACE("malloc FAILED");
       return(NCSCC_RC_FAILURE);
   }

   /*Reserve space for "Checkpoint Header" */
   pheader = ncs_enc_reserve_space(uba,sizeof(LGS_CKPT_HEADER));
   if (pheader == NULL)
   {
       TRACE("ncs_enc_reserve_space FAILED");
       free(ckpt_stream_rec);
       return(rc=EDU_ERR_MEM_FAIL);
   }
   ncs_enc_claim_space(uba,sizeof(LGS_CKPT_HEADER));
   log_stream_rec = log_stream_getnext_by_name(NULL);

   /* Walk through the reg list and encode record by record */
   while (log_stream_rec != NULL)
   {
       lgs_ckpt_stream_open_set(log_stream_rec, ckpt_stream_rec);
       rc = m_NCS_EDU_EXEC(&cb->edu_hdl, 
                           lgs_edp_ed_open_stream_rec,
                           uba,
                           EDP_OP_TYPE_ENC,
                           ckpt_stream_rec,
                           &ederror);

       if (rc != NCSCC_RC_SUCCESS)
       {
           m_NCS_EDU_PRINT_ERROR_STRING(ederror);
           TRACE("m_NCS_EDU_EXEC FAILED");
           free(ckpt_stream_rec);
           return rc;
       }
       ++num_rec;
       log_stream_rec = log_stream_getnext_by_name(log_stream_rec->name);
   }/* End while RegRec */

   /* Encode RegHeader */
   ckpt_hdr.ckpt_rec_type = LGS_CKPT_OPEN_STREAM;
   ckpt_hdr.num_ckpt_records = num_rec;
   ckpt_hdr.data_len = 0; /*Not in Use for Cold Sync */

   lgs_enc_ckpt_header(pheader,ckpt_hdr);

   free(ckpt_stream_rec);
   return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : lgs_edu_enc_reg_list
 *
 * Description   : This function walks through the reglist and encodes all
 *                 records in the reglist, using the edps defined for the
 *                 same.
 *
 * Arguments     : cb - Pointer to LGS control block.
 *                 uba - Pointer to ubaid provided by mbcsv 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ***************************************************************************/

uns32 lgs_edu_enc_reg_list(lgs_cb_t *cb, NCS_UBAID *uba)
{
    lga_reg_rec_t *reg_rec=NULL;
    LGS_CKPT_REG_MSG *ckpt_reg_rec;
    EDU_ERR   ederror=0;
    uns32     rc=NCSCC_RC_SUCCESS, num_rec=0;
    uns8 *pheader=NULL;
    LGS_CKPT_HEADER ckpt_hdr;
TRACE_ENTER();
    /* Prepare reg. structure to encode */
    ckpt_reg_rec = malloc(sizeof(LGS_CKPT_REG_MSG));
    if (ckpt_reg_rec == NULL)
    {
        TRACE("malloc FAILED");
        return(NCSCC_RC_FAILURE);
    }

    /*Reserve space for "Checkpoint Header" */
    pheader = ncs_enc_reserve_space(uba,sizeof(LGS_CKPT_HEADER));
    if (pheader == NULL)
    {
        TRACE("  ncs_enc_reserve_space FAILED");
        free(ckpt_reg_rec);
        return(rc=EDU_ERR_MEM_FAIL);
    }
    ncs_enc_claim_space(uba,sizeof(LGS_CKPT_HEADER));

    reg_rec=(lga_reg_rec_t *)ncs_patricia_tree_getnext(&cb->lga_reg_list,(uns8 *)0);

    /* Walk through the reg list and encode record by record */
    while (reg_rec != NULL)
    {
        ckpt_reg_rec->reg_id = reg_rec->reg_id;
        ckpt_reg_rec->lga_client_dest = reg_rec->lga_client_dest;
        if (NULL != reg_rec->first_stream)
        {
            ckpt_reg_rec->stream_list = reg_rec->first_stream;
        }

        rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_reg_rec,uba,
                            EDP_OP_TYPE_ENC,ckpt_reg_rec,&ederror);

        if (rc != NCSCC_RC_SUCCESS)
        {
            m_NCS_EDU_PRINT_ERROR_STRING(ederror);
            TRACE("  m_NCS_EDU_EXEC FAILED");
            free(ckpt_reg_rec);
            return rc;
        }
        ++num_rec;

        /* length+=lgs_edp_ed_reg_rec(reg_rec,o_ub);*/
        reg_rec=(lga_reg_rec_t *)ncs_patricia_tree_getnext(&cb->lga_reg_list,(uns8*)&reg_rec->reg_id_Net);
    }/* End while RegRec */

    /* Encode RegHeader */
    ckpt_hdr.ckpt_rec_type = LGS_CKPT_INITIALIZE_REC;
    ckpt_hdr.num_ckpt_records = num_rec;
    ckpt_hdr.data_len = 0; /*Not in Use for Cold Sync */

    lgs_enc_ckpt_header(pheader,ckpt_hdr);

    free(ckpt_reg_rec);
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;

}/* End lgs_edu_enc_reg_list() */

/****************************************************************************
 * Name          : lgs_ckpt_encode_async_update
 *
 * Description   : This function encodes data to be sent as an async update.
 *                 The caller of this function would set the address of the
 *                 record to be encoded in the reo_hdl field(while invoking
 *                 SEND_CKPT option of ncs_mbcsv_svc. 
 *
 * Arguments     : cb - pointer to the LGS control block.
 *                 cbk_arg - Pointer to NCS MBCSV callback argument struct.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/

uns32 lgs_ckpt_encode_async_update(lgs_cb_t *lgs_cb,EDU_HDL edu_hdl,NCS_MBCSV_CB_ARG *cbk_arg)
{
    LGS_CKPT_DATA *data = NULL;
    EDU_ERR ederror = 0;
    uns32 rc = NCSCC_RC_SUCCESS;
    TRACE_ENTER();
    /* Set reo_hdl from callback arg to ckpt_rec */
    data=(LGS_CKPT_DATA *)(long)cbk_arg->info.encode.io_reo_hdl;
    if (data == NULL)
    {
        TRACE("   data == NULL, FAILED");
        TRACE_LEAVE();
        return NCSCC_RC_FAILURE;
    }
    /* Encode async record,except publish & subscribe */
    rc = m_NCS_EDU_EXEC(&edu_hdl,lgs_edp_ed_ckpt_msg,
                        &cbk_arg->info.encode.io_uba,
                        EDP_OP_TYPE_ENC,data,&ederror);

    if (rc != NCSCC_RC_SUCCESS)
    {
        m_NCS_EDU_PRINT_ERROR_STRING(ederror);
        /* free(data); FIX ??? */
        TRACE_2("eduerr: %x",ederror);
        TRACE_LEAVE();
        return rc;
    }

    /* Update the Async Update Count at standby*/
    lgs_cb->async_upd_cnt++;
    /* free (data); FIX??? */
    return rc;
}

/****************************************************************************
 * Name          : lgs_ckpt_decode_cbk_handler
 *
 * Description   : This function is the single entry point to all decode
 *                 requests from mbcsv. 
 *                 Invokes the corresponding decode routine based on the 
 *                 MBCSv decode request type.
 *
 * Arguments     : arg - Pointer to NCS_MBCSV_CB_ARG with decode info
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_ckpt_decode_cbk_handler(NCS_MBCSV_CB_ARG *cbk_arg)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    uns16 msg_fmt_version;

    TRACE_ENTER();
    assert(cbk_arg != NULL);

    msg_fmt_version =  m_NCS_MBCSV_FMT_GET(cbk_arg->info.decode.i_peer_version,
                                           LGS_MBCSV_VERSION,
                                           LGS_MBCSV_VERSION_MIN);
    if (0 == msg_fmt_version)
    {
        TRACE("wrong msg_fmt_version!!!\n");
        TRACE_LEAVE();
        return NCSCC_RC_FAILURE;
    }

    switch (cbk_arg->info.decode.i_msg_type)
    {
        case NCS_MBCSV_MSG_COLD_SYNC_REQ:
            TRACE_2(" COLD SYNC REQ DECODE called"); 
            break;

        case NCS_MBCSV_MSG_COLD_SYNC_RESP:
        case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
            TRACE_2(" COLD SYNC RESP DECODE called"); 
            if (lgs_cb->ckpt_state != COLD_SYNC_COMPLETE)/*this check is needed to handle repeated requests */
            {
                if ((rc=lgs_ckpt_decode_cold_sync(lgs_cb,cbk_arg)) != NCSCC_RC_SUCCESS)
                {
                    TRACE(" COLD SYNC RESPONSE DECODE ....");
                }
                else
                {
                    TRACE_2(" COLD SYNC RESPONSE DECODE SUCCESS....");
                    lgs_cb->ckpt_state = COLD_SYNC_COMPLETE;
                }
            }
            break;

        case NCS_MBCSV_MSG_ASYNC_UPDATE:
            TRACE_2(" ASYNC UPDATE DECODE called");
            if ((rc=lgs_ckpt_decode_async_update(lgs_cb,cbk_arg)) != NCSCC_RC_SUCCESS)
                TRACE("  lgs_ckpt_decode_async_update FAILED");
            break;

        case NCS_MBCSV_MSG_WARM_SYNC_REQ:
        case NCS_MBCSV_MSG_WARM_SYNC_RESP:
        case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
        case NCS_MBCSV_MSG_DATA_REQ:
            TRACE_2("WARM SYNC called, not used");
            break;
        case NCS_MBCSV_MSG_DATA_RESP:
        case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
            TRACE_2("DATA RESP COMPLETE DECODE called"); 
            if ((rc=lgs_ckpt_decode_cold_sync(lgs_cb, cbk_arg)) != NCSCC_RC_SUCCESS)
                TRACE("   FAILED");
            break;

        default:
            TRACE_2(" INCORRECT DECODE called"); 
            rc = NCSCC_RC_FAILURE;
            TRACE("  INCORRECT DECODE called, FAILED");
            m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            break;
    } /*End switch(io_msg_type)*/

    TRACE_LEAVE();
    return rc;

}/*End lgs_ckpt_decode_cbk_handler()*/

/****************************************************************************
 * Name          : lgs_ckpt_decode_async_update 
 *
 * Description   : This function decodes async update data, based on the 
 *                 record type contained in the header. 
 *
 * Arguments     : arg - Pointer to NCS_MBCSV_CB_ARG with decode info
 *                 cb - pointer to lgs cb.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_ckpt_decode_async_update(lgs_cb_t *cb, NCS_MBCSV_CB_ARG *cbk_arg)
{
    uns32                             rc = NCSCC_RC_SUCCESS;
    EDU_ERR                           ederror=0;
    LGS_CKPT_DATA                     *ckpt_msg;
    LGS_CKPT_HEADER                   *hdr=NULL;
    LGS_CKPT_REG_MSG                  *reg_rec=NULL;
    LGS_CKPT_WRITE_LOG                *writelog=NULL;
    LGS_CKPT_STREAM_OPEN              *stream=NULL;
    LGS_CKPT_FINALIZE_MSG             *finalize=NULL;  
    MDS_DEST                          *agent_dest=NULL;

    TRACE_ENTER();

    /* Allocate memory to hold the checkpoint message */
    ckpt_msg = calloc(1, sizeof(LGS_CKPT_DATA));

    /* Decode the message header */
    hdr = &ckpt_msg->header;
    rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_header_rec,&cbk_arg->info.decode.i_uba,
                        EDP_OP_TYPE_DEC,&hdr,&ederror);
    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE("m_NCS_EDU_EXEC FAILED");
        m_NCS_EDU_PRINT_ERROR_STRING(ederror);
        goto done;
    }

    ederror = 0;
    TRACE_2("ckpt_rec_type: %d ",(int) hdr->ckpt_rec_type);
    /* Call decode routines appropriately */
    switch (hdr->ckpt_rec_type)
    {
        case LGS_CKPT_INITIALIZE_REC:
            TRACE_2("INITIALIZE REC: AUPDATE");
            reg_rec=&ckpt_msg->ckpt_rec.reg_rec;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_reg_rec,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&reg_rec,&ederror);

            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("   FAILED");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done;
            }
            break;
        case LGS_CKPT_LOG_WRITE:
            TRACE_2("WRITE LOG: AUPDATE");
            writelog=&ckpt_msg->ckpt_rec.writeLog;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_write_rec,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&writelog,&ederror);

            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("   write log FAILED");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done;
            }
            break;

        case LGS_CKPT_OPEN_STREAM:
            TRACE_2("STREAM OPEN: AUPDATE");
            stream=&ckpt_msg->ckpt_rec.stream_open;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_open_stream_rec,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&stream,&ederror);

            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("STREAM OPEN FAILED");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done;
            }
            break;

        case LGS_CKPT_CLOSE_STREAM:
            TRACE_2("STREAM CLOSE: AUPDATE");
            stream=&ckpt_msg->ckpt_rec.stream_open;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_close_stream_rec,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&stream,&ederror);

            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("STREAM CLOSE FAILED");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done;
            }
            break;

        case LGS_CKPT_FINALIZE_REC: 
            TRACE_2("FINALIZE REC: AUPDATE");
            reg_rec=&ckpt_msg->ckpt_rec.reg_rec;
            finalize=&ckpt_msg->ckpt_rec.finalize_rec;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_finalize_rec,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&finalize,&ederror);

            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("   FAILED");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done;
            }
            break;
        case LGS_CKPT_AGENT_DOWN:
            TRACE_2("AGENT DOWN REC: AUPDATE");
            agent_dest=&ckpt_msg->ckpt_rec.agent_dest;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,ncs_edp_mds_dest,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&agent_dest,&ederror);
            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("   FAILED");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done;
            }
            break;

        default:
            rc = NCSCC_RC_FAILURE;
            TRACE("   FAILED");
            goto done;
            break;
    }/*end switch */

    rc = lgs_process_ckpt_data(cb, ckpt_msg);
    /* Update the Async Update Count at standby*/
    cb->async_upd_cnt++;
done:
    free(ckpt_msg);
    TRACE_LEAVE();
    return rc;
    /* if failure, should an indication be sent to active ? */
}


/****************************************************************************
 * Name          : lgs_ckpt_decode_cold_sync 
 *
 * Description   : This function decodes async update data, based on the 
 *                 record type contained in the header. 
 *
 * Arguments     : arg - Pointer to NCS_MBCSV_CB_ARG with decode info
 *                 cb - pointer to lgs cb.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : COLD SYNC RECORDS are expected in an order
 *                 1. REG RECORDS
 *                 2. OPEN STREAMS RECORDS
 *                 
 *                 For each record type,
 *                     a) decode header.
 *                     b) decode individual records for 
 *                        header->num_records times, 
 *****************************************************************************/

uns32 lgs_ckpt_decode_cold_sync(lgs_cb_t *cb,NCS_MBCSV_CB_ARG *cbk_arg)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    EDU_ERR ederror=0;
    LGS_CKPT_DATA *data;
    /*  NCS_UBAID *uba=NULL; */
    uns32 num_rec=0;
    LGS_CKPT_REG_MSG *reg_rec= NULL;
    LGS_CKPT_STREAM_OPEN *stream_rec = NULL;
    uns32 num_of_async_upd;
    uns8* ptr;
    uns8 data_cnt[16];
    TRACE_ENTER();
    /* LGS_CKPT_RETAIN_EVT_MSG *reten_rec=NULL; */

    /* 
        -------------------------------------------------
        | Header|RegRecords1..n|Header|streamRecords1..n|
        -------------------------------------------------
    */           

    while (1)
    {
        TRACE_2("COLD SYNC DECODE START........");
        /* Allocate memory to hold the checkpoint Data */
        data = calloc(1, sizeof(LGS_CKPT_DATA)); /* where is this memory freed? */
        if (data == NULL)
        {
            TRACE("calloc FAILED");
            rc = NCSCC_RC_FAILURE; /*DBG_SINK */
            goto done;
        }

        /* Decode the current message header*/
        if ((rc = lgs_dec_ckpt_header(&cbk_arg->info.decode.i_uba,&data->header)) != NCSCC_RC_SUCCESS)
        {
            goto done_free;
        }
        /* Check if the first in the order of records is reg record */
        if (data->header.ckpt_rec_type != LGS_CKPT_INITIALIZE_REC)
        {
            TRACE("FAILED data->header.ckpt_rec_type != LGS_CKPT_INITIALIZE_REC");
            rc = NCSCC_RC_FAILURE;
            goto done_free;
        }

        /* Process the reg_records */
        num_rec=data->header.num_ckpt_records;
        TRACE("regid: num_rec = %u", num_rec);
        while (num_rec)
        {
            reg_rec=&data->ckpt_rec.reg_rec;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_reg_rec,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&reg_rec,&ederror);

            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("FAILED: COLD SYNC DECODE REG REC");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done_free;
            }
            /* Update our database */
            rc = lgs_process_ckpt_data(cb,data);
            if (rc != NCSCC_RC_SUCCESS)
            {
                goto done_free;
            }
            memset(&data->ckpt_rec, 0, sizeof(data->ckpt_rec));
            --num_rec;
        }/*End while, reg records */


        if ((rc=lgs_dec_ckpt_header(&cbk_arg->info.decode.i_uba,&data->header)) != NCSCC_RC_SUCCESS)
        {
            rc= NCSCC_RC_FAILURE;
            TRACE("lgs_dec_ckpt_header FAILED");
            goto done_free;
        }

        /* Check if record type is open_stream */
        if(data->header.ckpt_rec_type != LGS_CKPT_OPEN_STREAM)
        {
          rc =  NCSCC_RC_FAILURE;
          TRACE("FAILED: LGS_CKPT_OPEN_STREAM type is expected, got %u", data->header.ckpt_rec_type);
          goto done_free;
        }

        /* Process the stream records */
        num_rec=data->header.num_ckpt_records;
        TRACE("opens_streams: num_rec = %u", num_rec);
        while (num_rec)
        {
            stream_rec=&data->ckpt_rec.stream_open;
            rc = m_NCS_EDU_EXEC(&cb->edu_hdl,lgs_edp_ed_open_stream_rec,&cbk_arg->info.decode.i_uba,
                                EDP_OP_TYPE_DEC,&stream_rec,&ederror);

            if (rc != NCSCC_RC_SUCCESS)
            {
                TRACE("FAILED: COLD SYNC DECODE STREAM REC");
                m_NCS_EDU_PRINT_ERROR_STRING(ederror);
                goto done_free;
            }
            /* Update our database */
            rc = lgs_process_ckpt_data(cb,data);

            if (rc != NCSCC_RC_SUCCESS)
            {
                goto done_free;
            }
            memset(&data->ckpt_rec, 0, sizeof(data->ckpt_rec));
            --num_rec;
        }/*End while, stream records */

        /* Get the async update count */
        ptr = ncs_dec_flatten_space(&cbk_arg->info.decode.i_uba,data_cnt,sizeof(uns32));
        num_of_async_upd = ncs_decode_32bit(&ptr);
        cb->async_upd_cnt = num_of_async_upd;
        ncs_dec_skip_space(&cbk_arg->info.decode.i_uba, 4);

        /* If we reached here, we are through. Good enough for coldsync with ACTIVE */
        free(data);
        TRACE_2("COLD SYNC DECODE END........");
        TRACE_LEAVE();
        return rc;
    }/*End while(1) */

done_free:
    free(data);
done:
    if (rc != NCSCC_RC_SUCCESS)
    {
        LOG_ER("Cold sync failed");
        assert(rc == NCSCC_RC_SUCCESS); /* try again after reboot */
    }
    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_process_ckpt_data
 *
 * Description   : This function updates the lgs internal databases
 *                 based on the data type. 
 *
 * Arguments     : cb - pointer to LGS ControlBlock. 
 *                 data - pointer to  LGS_CHECKPOINT_DATA. 
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_process_ckpt_data(lgs_cb_t *cb, LGS_CKPT_DATA *data)
{
    uns32 rc=NCSCC_RC_SUCCESS;
    if ((!cb) || (data == NULL)){
        TRACE("FAILED: (!cb) || (data == NULL)");
        return(rc=NCSCC_RC_FAILURE);
    }

    if ((cb->ha_state == SA_AMF_HA_STANDBY) || (cb->ha_state == SA_AMF_HA_QUIESCED))
    {
        if (data->header.ckpt_rec_type >= LGS_CKPT_MSG_MAX){
            TRACE("FAILED: data->header.ckpt_rec_type >= LGS_CKPT_MSG_MAX"); 
            return NCSCC_RC_FAILURE;
        }
        /* Update the internal database */
        rc= lgs_ckpt_data_handler[data->header.ckpt_rec_type](cb, data);
        return rc;
    }
    else
    {
        return(rc=NCSCC_RC_FAILURE);
    } 
}/*End lgs_process_ckpt_data()*/


/****************************************************************************
 * Name          : lgs_ckpt_proc_reg_rec
 *
 * Description   : This function updates the lgs reglist based on the 
 *                 info received from the ACTIVE lgs peer.
 *
 * Arguments     : cb - pointer to LGS  ControlBlock.
 *                 data - pointer to  LGS_CHECKPOINT_DATA. 
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/

uns32 lgs_ckpt_proc_reg_rec(lgs_cb_t *cb, LGS_CKPT_DATA *data)
{
    TRACE_ENTER();
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_REG_MSG *param=&data->ckpt_rec.reg_rec;
    TRACE_2("update regid: %d", param->reg_id);
    if (!param->reg_id)
    {
        TRACE("FAILED regid = 0");
        TRACE_LEAVE();
        return NCSCC_RC_FAILURE;
    }
    /* Add this regid to the registration linked list. */
    if ((rc = lgs_add_reglist_entry(cb, param->lga_client_dest, param->reg_id,
                                    param->stream_list)) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}/*End lgs_ckpt_proc_reg_rec */

/****************************************************************************
 * Name          : lgs_ckpt_proc_log_write
 *
 * Description   : This function updates the lgs alarm logRecordId
 *                 received from the ACTIVE lgs peer.
 *                 
 *
 * Arguments     : cb - pointer to LGS  ControlBlock.
 *                 data - pointer to  LGS_CHECKPOINT_DATA. 
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/

uns32 lgs_ckpt_proc_log_write(lgs_cb_t *cb, LGS_CKPT_DATA *data)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_WRITE_LOG *param = &data->ckpt_rec.writeLog;
    log_stream_t* stream;

    TRACE_ENTER();

    stream = log_stream_get_by_id(param->streamId);
    if (stream == NULL)
    {
        /* FIX error handling - reboot instead? */
        TRACE("Bad stream ID: %u", param->streamId);
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    if (cb->ha_state != SA_AMF_HA_STANDBY)
        goto done;

    stream->logRecordId = param->recordId;
    stream->curFileSize = param->curFileSize;
    strcpy(stream->logFileCurrent, param->logFileCurrent);

done:
    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_ckpt_proc_close_stream
 *
 * Description   : This function close a stream.
 *
 * Arguments     : cb - pointer to LGS  ControlBlock.
 *                 data - pointer to  LGS_CHECKPOINT_DATA. 
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/

static uns32 lgs_ckpt_proc_close_stream(lgs_cb_t *cb, LGS_CKPT_DATA *data)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_STREAM_CLOSE *param = &data->ckpt_rec.stream_close;
    log_stream_t* stream;

    TRACE_ENTER();

    if ((stream = log_stream_get_by_id(param->streamId)) == NULL)
    {
        /* FIX error handling - reboot instead? */
        TRACE("Bad stream ID");
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    TRACE("close stream %s, id: %u", stream->name, stream->streamId);
    lgs_remove_stream_id_from_rec(param->regId, param->streamId);
    log_stream_close(stream);

done:
    TRACE_LEAVE();
    return rc;
}

static void freeEduMem(char* ptr)
{
   m_NCS_MEM_FREE(ptr ,NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0);
}

static void setStream(log_stream_t *cbStream, LGS_CKPT_STREAM_OPEN *stream)
{
    TRACE_ENTER();
    cbStream->streamId=stream->streamId;
    cbStream->maxLogFileSize=stream->maxFileSize;
    cbStream->fixedLogRecordSize=stream->maxLogRecordSize;
    cbStream->logFullAction=stream->logFileFullAction;
    cbStream->maxFilesRotated=stream->maxFilesRotated;
    cbStream->creationTimeStamp = stream->creationTimeStamp;
    cbStream->numOpeners = stream->numOpeners;
    cbStream->streamType = stream->streamType;
    strncpy((char*)cbStream->fileName, stream->logFile, NAME_MAX);
    strncpy((char*)cbStream->pathName, stream->logPath, PATH_MAX);
    strncpy(cbStream->logFileCurrent, stream->logFileCurrent, NAME_MAX);
    strcpy(cbStream->name, stream->logStreamName);

    lgs_setFmtString(cbStream, stream->fileFmt);    

    freeEduMem(stream->logFile);
    freeEduMem(stream->logPath);
    freeEduMem(stream->fileFmt);
    freeEduMem(stream->logFileCurrent);
    freeEduMem(stream->logStreamName);
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : lgs_ckpt_proc_open_stream
 *
 * Description   : This function updates the lgs alarm logRecordId
 *                 received from the ACTIVE lgs peer.
 *                 
 *
 * Arguments     : cb - pointer to LGS  ControlBlock.
 *                data - pointer to  LGS_CHECKPOINT_DATA. 
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/

uns32 lgs_ckpt_proc_open_stream(lgs_cb_t *cb, LGS_CKPT_DATA *data)
{
    uns32 rc;
    LGS_CKPT_STREAM_OPEN *param = &data->ckpt_rec.stream_open;
    log_stream_t *stream;

    TRACE_ENTER();

    if (cb->ha_state != SA_AMF_HA_STANDBY)
        goto done;

    stream = log_stream_get_by_name(param->logStreamName);
    if (stream != NULL)
    {
        TRACE("existing stream - id %u", stream->streamId);
        setStream(stream, param);
    }
    else
    {
        SaAisErrorT ais_rv;
        SaNameT name;

        TRACE("New stream %s, id %u", param->logStreamName, param->streamId);
        strcpy(name.value, param->logStreamName);
        name.length = strlen(param->logStreamName);

        stream = log_stream_new(&name,
                                param->logFile,
                                param->logPath,
                                param->maxFileSize,
                                param->maxLogRecordSize,
                                SA_TRUE,
                                param->logFileFullAction,
                                param->maxFilesRotated,
                                param->fileFmt,
                                param->streamType,
                                param->streamId);
        if (stream == NULL)
        {
            ais_rv = SA_AIS_ERR_NO_MEMORY;
            goto done;
        }

        stream->numOpeners = param->numOpeners;
        stream->creationTimeStamp = param->creationTimeStamp;
        strcpy(stream->logFileCurrent, param->logFileCurrent);
    }

    log_stream_print(stream);

    /* Create an association between this reg_id and the stream */
    rc = lgs_add_stream_to_reg_id(param->regId, stream);
    if (rc != NCSCC_RC_SUCCESS)
    {
        goto done;
    }

done:
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_ckpt_proc_finalize_rec
 *
 * Description   : This function clears the lgs reglist and assosicated DB 
 *                 based on the info received from the ACTIVE lgs peer.
 *
 * Arguments     : cb - pointer to LGS ControlBlock.
 *                 data - pointer to  LGS_CHECKPOINT_DATA. 
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ****************************************************************************/

uns32 lgs_ckpt_proc_finalize_rec(lgs_cb_t* cb, LGS_CKPT_DATA *data)
{
    LGS_CKPT_FINALIZE_MSG *param = &data->ckpt_rec.finalize_rec;

    TRACE_ENTER();

    /* This insure all resources allocated by this registration are freed. */
    (void) lgs_remove_reglist_entry(cb, param->reg_id, FALSE);

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_ckpt_proc_agent_down_rec
 *
 * Description   : This function processes a agent down message 
 *                 received from the ACTIVE LGS peer.      
 *
 * Arguments     : cb - pointer to LGS ControlBlock.
 *                 data - pointer to  LGS_CHECKPOINT_DATA. 
 * 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None 
 ****************************************************************************/

uns32 lgs_ckpt_proc_agent_down_rec(lgs_cb_t* cb, LGS_CKPT_DATA *data)
{
    TRACE_ENTER();

    /* Remove this LGA entry from our processing lists */
    lgs_remove_regid_by_mds_dest(lgs_cb, data->ckpt_rec.agent_dest);

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_ckpt_send_async_update
 *
 * Description   : This function makes a request to MBCSV to send an async
 *                 update to the STANDBY LGS for the record held at
 *                 the address i_reo_hdl.
 *
 * Arguments     : cb - A pointer to the lgs control block.
 *                 ckpt_rec - pointer to the checkpoint record to be
 *                 sent as an async update.
 *                 action - type of async update to indiciate whether
 *                 this update is for addition, deletion or modification of
 *                 the record being sent.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : MBCSV, inturn calls our encode callback for this async
 *                 update. We use the reo_hdl in the encode callback to
 *                 retrieve the record for encoding the same.
 *****************************************************************************/

uns32 lgs_send_async_update(lgs_cb_t *cb,LGS_CKPT_DATA *ckpt_rec,uns32 action)
{
    uns32 rc=NCSCC_RC_SUCCESS;
    NCS_MBCSV_ARG mbcsv_arg;
    TRACE_ENTER();
    /* Fill mbcsv specific data */
    memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op=NCS_MBCSV_OP_SEND_CKPT;
    mbcsv_arg.i_mbcsv_hdl=cb->mbcsv_hdl;
    mbcsv_arg.info.send_ckpt.i_action=action;
    mbcsv_arg.info.send_ckpt.i_ckpt_hdl=(NCS_MBCSV_CKPT_HDL)cb->mbcsv_ckpt_hdl;
    mbcsv_arg.info.send_ckpt.i_reo_hdl=NCS_PTR_TO_UNS64_CAST(ckpt_rec); /*Will be used in encode callback*/

    /* Just store the address of the data to be send as an 
     * async update record in reo_hdl. The same shall then be 
     *dereferenced during encode callback */

    mbcsv_arg.info.send_ckpt.i_reo_type=ckpt_rec->header.ckpt_rec_type;
    mbcsv_arg.info.send_ckpt.i_send_type=NCS_MBCSV_SND_SYNC;

    /* Send async update */
    if (NCSCC_RC_SUCCESS != (rc = ncs_mbcsv_svc(&mbcsv_arg)))
    {
        TRACE(" MBCSV send data operation !! rc=%u.", rc);
        TRACE_LEAVE();
        return NCSCC_RC_FAILURE;
    }
    TRACE_LEAVE();
    return rc;
}/*End send_async_update() */


/****************************************************************************
 * Name          : lgs_ckpt_peer_info_cbk_handler 
 *
 * Description   : This callback is invoked by mbcsv when a peer info message
 *                 is received from LGS STANDBY. 
 *
 * Arguments     : NCS_MBCSV_ARG containing info pertaining to the STANDBY.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ***************************************************************************/
uns32 lgs_ckpt_peer_info_cbk_handler(NCS_MBCSV_CB_ARG *arg)
{
    uns16 peer_version;
    assert(arg != NULL);

    peer_version = arg->info.peer.i_peer_version;
    if (peer_version < LGS_MBCSV_VERSION_MIN)
    {
        TRACE("peer_version not correct!!\n");
        return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_ckpt_notify_cbk_handler 
 *
 * Description   : This callback is invoked by mbcsv when a notify message
 *                 is received from LGS STANDBY. 
 *
 * Arguments     : NCS_MBCSV_ARG - contains notification info from STANDBY.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ***************************************************************************/
uns32 lgs_ckpt_notify_cbk_handler (NCS_MBCSV_CB_ARG *arg)
{
    TRACE_ENTER();
    /* Currently nothing to be done */
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}/* End lgs_ckpt_notify_cbk_handler */


/****************************************************************************
 * Name          : lgs_ckpt_err_ind_cbk_handler 
 *
 * Description   : This callback is invoked by mbcsv when a notify message
 *                 is received from LGS STANDBY. 
 *
 *
 * Arguments     : NCS_MBCSV_ARG - contains notification info from STANDBY.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 ***************************************************************************/
uns32 lgs_ckpt_err_ind_cbk_handler (NCS_MBCSV_CB_ARG *arg)
{
    TRACE_ENTER();
    /* Currently nothing to be done. */
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}/* End lgs_ckpt_err_ind_handler */

uns32 lgs_edp_ed_stream_list(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,    
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    lgs_stream_list_t *ckpt_stream_list_msg_ptr = NULL, **ckpt_stream_list_msg_dec_ptr;

    EDU_INST_SET lgs_ckpt_stream_list_ed_rules[ ] = {
        {EDU_START,lgs_edp_ed_stream_list, EDQ_LNKLIST, 0, 0, sizeof(lgs_stream_list_t), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((lgs_stream_list_t*)0)->stream_id,0, NULL},
        {EDU_TEST_LL_PTR,lgs_edp_ed_stream_list,0,0,0,(uns32)&((lgs_stream_list_t*)0)->next,0,NULL}, 
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_stream_list_msg_ptr = (lgs_stream_list_t *)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_stream_list_msg_dec_ptr = (lgs_stream_list_t **)ptr;
        if (*ckpt_stream_list_msg_dec_ptr == NULL)
        {
            *ckpt_stream_list_msg_dec_ptr = calloc(1, sizeof(lgs_stream_list_t));
            if (*ckpt_stream_list_msg_dec_ptr == NULL)
            {
                *o_err = EDU_ERR_MEM_FAIL;
                return NCSCC_RC_FAILURE;
            }
        }
        memset(*ckpt_stream_list_msg_dec_ptr, '\0', sizeof(lgs_stream_list_t));
        ckpt_stream_list_msg_ptr = *ckpt_stream_list_msg_dec_ptr;
    }
    else
    {
        ckpt_stream_list_msg_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn,lgs_ckpt_stream_list_ed_rules, ckpt_stream_list_msg_ptr, ptr_data_len,
                             buf_env, op, o_err);
    return rc;
}
/****************************************************************************
 * Name          : lgs_edp_ed_reg_rec
 *
 * Description   : This function is an EDU program for encoding/decoding
 *                 lgsv checkpoint registration rec.
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_edp_ed_reg_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                         NCSCONTEXT ptr, uns32 *ptr_data_len,
                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                         EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_REG_MSG *ckpt_reg_msg_ptr = NULL, **ckpt_reg_msg_dec_ptr;

    EDU_INST_SET lgs_ckpt_reg_rec_ed_rules[ ] = {
        {EDU_START,lgs_edp_ed_reg_rec, 0, 0, 0, sizeof(LGS_CKPT_REG_MSG), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_REG_MSG*)0)->reg_id,0, NULL},
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, 0, (uns32)&((LGS_CKPT_REG_MSG*)0)->lga_client_dest, 0, NULL},
        {EDU_EXEC, lgs_edp_ed_stream_list, EDQ_POINTER, 0, 0, (uns32)&((LGS_CKPT_REG_MSG*)0)->stream_list,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_reg_msg_ptr = (LGS_CKPT_REG_MSG *)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_reg_msg_dec_ptr = (LGS_CKPT_REG_MSG **)ptr;

        if (*ckpt_reg_msg_dec_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*ckpt_reg_msg_dec_ptr, '\0', sizeof(LGS_CKPT_REG_MSG));
        ckpt_reg_msg_ptr = *ckpt_reg_msg_dec_ptr;
    }
    else
    {
        ckpt_reg_msg_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, lgs_ckpt_reg_rec_ed_rules, ckpt_reg_msg_ptr, ptr_data_len,
                             buf_env, op, o_err);
    return rc;

}/* End lgs_edp_ed_reg_rec */

/****************************************************************************
 * Name          : lgs_edp_ed_write_rec
 *
 * Description   : This function is an EDU program for encoding/decoding
 *                 lgsv checkpoint write log rec.
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_edp_ed_write_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_WRITE_LOG *ckpt_write_msg_ptr = NULL, **ckpt_write_msg_dec_ptr;

    EDU_INST_SET lgs_ckpt_write_rec_ed_rules[ ] = {
        {EDU_START,lgs_edp_ed_write_rec, 0, 0, 0, sizeof(LGS_CKPT_WRITE_LOG), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_WRITE_LOG*)0)->recordId,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_WRITE_LOG*)0)->streamId,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_WRITE_LOG*)0)->curFileSize,0, NULL},
        {EDU_EXEC, ncs_edp_string, 0, 0, 0, (uns32)&((LGS_CKPT_WRITE_LOG*)0)->logFileCurrent,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_write_msg_ptr = (LGS_CKPT_WRITE_LOG *)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_write_msg_dec_ptr = (LGS_CKPT_WRITE_LOG **)ptr;
        if (*ckpt_write_msg_dec_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*ckpt_write_msg_dec_ptr, '\0', sizeof(LGS_CKPT_WRITE_LOG));
        ckpt_write_msg_ptr = *ckpt_write_msg_dec_ptr;
    }
    else
    {
        ckpt_write_msg_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, lgs_ckpt_write_rec_ed_rules, ckpt_write_msg_ptr, ptr_data_len,
                             buf_env, op, o_err);
    return rc;

}/* End lgs_edp_ed_write_rec */

/****************************************************************************
 * Name          : lgs_edp_ed_open_stream_rec
 *
 * Description   : This function is an EDU program for encoding/decoding
 *                 lgsv checkpoint open_stream log rec.
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_edp_ed_open_stream_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                 NCSCONTEXT ptr, uns32 *ptr_data_len,
                                 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                 EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_STREAM_OPEN *ckpt_open_stream_msg_ptr = NULL, **ckpt_open_stream_msg_dec_ptr;

    EDU_INST_SET lgs_ckpt_open_stream_rec_ed_rules[ ] = {
        {EDU_START,lgs_edp_ed_open_stream_rec, 0, 0, 0, sizeof(LGS_CKPT_STREAM_OPEN), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->streamId,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->regId,0, NULL},
        {EDU_EXEC, ncs_edp_string, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->logFile,0, NULL},
        {EDU_EXEC, ncs_edp_string, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->logPath,0, NULL},
        {EDU_EXEC, ncs_edp_string, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->logFileCurrent,0, NULL},
        {EDU_EXEC, ncs_edp_uns64, 0, 0, 0, (uns64)&((LGS_CKPT_STREAM_OPEN*)0)->maxFileSize,0, NULL},
        {EDU_EXEC, ncs_edp_int32, 0, 0, 0, (int32)&((LGS_CKPT_STREAM_OPEN*)0)->maxLogRecordSize,0, NULL},
        {EDU_EXEC, ncs_edp_int32, 0, 0, 0, (int32)&((LGS_CKPT_STREAM_OPEN*)0)->logFileFullAction,0, NULL},
        {EDU_EXEC, ncs_edp_int32, 0, 0, 0, (int32)&((LGS_CKPT_STREAM_OPEN*)0)->maxFilesRotated,0, NULL},
        {EDU_EXEC, ncs_edp_string, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->fileFmt,0, NULL},
        {EDU_EXEC, ncs_edp_string, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->logStreamName,0, NULL},
        {EDU_EXEC, ncs_edp_uns64, 0, 0, 0, (uns64)&((LGS_CKPT_STREAM_OPEN*)0)->creationTimeStamp,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->numOpeners,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_OPEN*)0)->streamType,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_open_stream_msg_ptr = (LGS_CKPT_STREAM_OPEN *)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_open_stream_msg_dec_ptr = (LGS_CKPT_STREAM_OPEN **)ptr;
        if (*ckpt_open_stream_msg_dec_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*ckpt_open_stream_msg_dec_ptr, '\0', sizeof(LGS_CKPT_STREAM_OPEN));
        ckpt_open_stream_msg_ptr = *ckpt_open_stream_msg_dec_ptr;
    }
    else
    {
        ckpt_open_stream_msg_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, lgs_ckpt_open_stream_rec_ed_rules, ckpt_open_stream_msg_ptr, ptr_data_len,
                             buf_env, op, o_err);
    return rc;

}/* End lgs_edp_ed_open_stream_rec */

/****************************************************************************
 * Name          : lgs_edp_ed_close_stream_rec
 *
 * Description   : This function is an EDU program for encoding/decoding
 *                 lgsv checkpoint close_stream log rec.
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_edp_ed_close_stream_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                                         NCSCONTEXT ptr, uns32 *ptr_data_len,
                                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                                         EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_STREAM_CLOSE *ckpt_close_stream_msg_ptr = NULL, **ckpt_close_stream_msg_dec_ptr;

    EDU_INST_SET lgs_ckpt_close_stream_rec_ed_rules[ ] = {
        {EDU_START,lgs_edp_ed_close_stream_rec, 0, 0, 0, sizeof(LGS_CKPT_STREAM_CLOSE), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_CLOSE*)0)->streamId,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_STREAM_CLOSE*)0)->regId,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_close_stream_msg_ptr = (LGS_CKPT_STREAM_CLOSE *)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_close_stream_msg_dec_ptr = (LGS_CKPT_STREAM_CLOSE **)ptr;
        if (*ckpt_close_stream_msg_dec_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*ckpt_close_stream_msg_dec_ptr, '\0', sizeof(LGS_CKPT_STREAM_CLOSE));
        ckpt_close_stream_msg_ptr = *ckpt_close_stream_msg_dec_ptr;
    }
    else
    {
        ckpt_close_stream_msg_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, lgs_ckpt_close_stream_rec_ed_rules,
                             ckpt_close_stream_msg_ptr, ptr_data_len, buf_env,
                             op, o_err);
    return rc;

}/* End lgs_edp_ed_close_stream_rec */

/****************************************************************************
 * Name          : lgs_edp_ed_finalize_rec
 *
 * Description   : This function is an EDU program for encoding/decoding
 *                 lgsv checkpoint finalize async updates record.
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_edp_ed_finalize_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                              NCSCONTEXT ptr, uns32 *ptr_data_len,
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                              EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_FINALIZE_MSG *ckpt_final_msg_ptr = NULL, **ckpt_final_msg_dec_ptr;

    EDU_INST_SET lgs_ckpt_final_rec_ed_rules[ ] = {
        {EDU_START,lgs_edp_ed_finalize_rec, 0, 0, 0, sizeof(LGS_CKPT_FINALIZE_MSG), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_FINALIZE_MSG*)0)->reg_id,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_final_msg_ptr = (LGS_CKPT_FINALIZE_MSG*)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_final_msg_dec_ptr = (LGS_CKPT_FINALIZE_MSG**)ptr;
        if (*ckpt_final_msg_dec_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*ckpt_final_msg_dec_ptr, '\0', sizeof(LGS_CKPT_FINALIZE_MSG));
        ckpt_final_msg_ptr = *ckpt_final_msg_dec_ptr;
    }
    else
    {
        ckpt_final_msg_ptr = ptr;
    }
    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, lgs_ckpt_final_rec_ed_rules, ckpt_final_msg_ptr, ptr_data_len,
                             buf_env, op, o_err);
    return rc;

}/* End lgs_edp_ed_finalize_rec() */

/****************************************************************************
 * Name          : lgs_edp_ed_header_rec 
 *
 * Description   : This function is an EDU program for encoding/decoding
 *                 lgsv checkpoint message header record.
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_edp_ed_header_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                            NCSCONTEXT ptr, uns32 *ptr_data_len,
                            EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                            EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_HEADER *ckpt_header_ptr = NULL, **ckpt_header_dec_ptr;

    EDU_INST_SET lgs_ckpt_header_rec_ed_rules[ ] = {
        {EDU_START,lgs_edp_ed_header_rec, 0, 0, 0, sizeof(LGS_CKPT_HEADER), 0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_HEADER*)0)->ckpt_rec_type,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_HEADER*)0)->num_ckpt_records,0, NULL},
        {EDU_EXEC, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_HEADER*)0)->data_len,0, NULL},
        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_header_ptr = (LGS_CKPT_HEADER*)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_header_dec_ptr = (LGS_CKPT_HEADER**)ptr;
        if (*ckpt_header_dec_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*ckpt_header_dec_ptr, '\0', sizeof(LGS_CKPT_HEADER));
        ckpt_header_ptr = *ckpt_header_dec_ptr;
    }
    else
    {
        ckpt_header_ptr = ptr;
    }
    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, lgs_ckpt_header_rec_ed_rules, ckpt_header_ptr, ptr_data_len,
                             buf_env, op, o_err);
    return rc;

}/* End lgs_edp_ed_header_rec() */

/****************************************************************************
 * Name          : lgs_ckpt_msg_test_type
 *
 * Description   : This function is an EDU_TEST program which returns the 
 *                 offset to call the appropriate EDU programe based on
 *                 based on the checkpoint message type, for use by 
 *                 the EDU program lgs_edu_encode_ckpt_msg.
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

int32 lgs_ckpt_msg_test_type(NCSCONTEXT arg)
{
    typedef enum
    {
        LCL_TEST_JUMP_OFFSET_LGS_CKPT_REG = 1,
        LCL_TEST_JUMP_OFFSET_LGS_CKPT_FINAL,
        LCL_TEST_JUMP_OFFSET_LGS_CKPT_WRITE_LOG,
        LCL_TEST_JUMP_OFFSET_LGS_CKPT_OPEN_STREAM,
        LCL_TEST_JUMP_OFFSET_LGS_CKPT_CLOSE_STREAM,
        LCL_TEST_JUMP_OFFSET_LGS_CKPT_AGENT_DOWN
    }LCL_TEST_JUMP_OFFSET;
    TRACE_ENTER();
    LGS_CKPT_DATA_TYPE    ckpt_rec_type;

    if (arg == NULL)
        return EDU_FAIL;
    ckpt_rec_type = *(LGS_CKPT_DATA_TYPE *)arg;
    TRACE_2("ckpt_rec_type: %d", (int)ckpt_rec_type );

    switch (ckpt_rec_type)
    {
        case LGS_CKPT_INITIALIZE_REC:
            return LCL_TEST_JUMP_OFFSET_LGS_CKPT_REG;
        case LGS_CKPT_FINALIZE_REC:
            return LCL_TEST_JUMP_OFFSET_LGS_CKPT_FINAL;
        case LGS_CKPT_LOG_WRITE:
            return LCL_TEST_JUMP_OFFSET_LGS_CKPT_WRITE_LOG;
        case LGS_CKPT_OPEN_STREAM:
            return LCL_TEST_JUMP_OFFSET_LGS_CKPT_OPEN_STREAM;
        case LGS_CKPT_CLOSE_STREAM:
            return LCL_TEST_JUMP_OFFSET_LGS_CKPT_CLOSE_STREAM;
        case LGS_CKPT_AGENT_DOWN:
            return LCL_TEST_JUMP_OFFSET_LGS_CKPT_AGENT_DOWN;
        default:
            return EDU_EXIT;
            break;
    }
    return EDU_FAIL;

}/*End edp test type */

/****************************************************************************
 * Name          : lgs_edp_ed_ckpt_msg 
 *
 * Description   : This function is an EDU program for encoding/decoding
 *                 lgsv checkpoint messages. This program runs the 
 *                 lgs_edp_ed_hdr_rec program first to decide the
 *                 checkpoint message type based on which it will call the
 *                 appropriate EDU programs for the different checkpoint 
 *                 messages. 
 * 
 * Arguments     : EDU_HDL - pointer to edu handle,
 *                 EDU_TKN - internal edu token to help encode/decode,
 *                 POINTER to the structure to encode/decode from/to,
 *                 data length specifying number of structures,
 *                 EDU_BUF_ENV - pointer to buffer for encoding/decoding.
 *                 op - operation type being encode/decode. 
 *                 EDU_ERR - out param to indicate errors in processing. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_edp_ed_ckpt_msg(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                          NCSCONTEXT ptr, uns32 *ptr_data_len,
                          EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                          EDU_ERR *o_err)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    LGS_CKPT_DATA *ckpt_msg_ptr = NULL, **ckpt_msg_dec_ptr;

    EDU_INST_SET lgs_ckpt_msg_ed_rules[ ] = {
        {EDU_START, lgs_edp_ed_ckpt_msg, 0, 0, 0, sizeof(LGS_CKPT_DATA), 0, NULL},
        {EDU_EXEC, lgs_edp_ed_header_rec, 0, 0, 0, (uns32)&((LGS_CKPT_DATA*)0)->header,0, NULL},

        {EDU_TEST, ncs_edp_uns32, 0, 0, 0, (uns32)&((LGS_CKPT_DATA*)0)->header, 0, (EDU_EXEC_RTINE )lgs_ckpt_msg_test_type},

        /* Reg Record */
        {EDU_EXEC, lgs_edp_ed_reg_rec, 0, 0, EDU_EXIT,
            (uns32)&((LGS_CKPT_DATA*)0)->ckpt_rec.reg_rec, 0, NULL},

        /* Finalize record */
        {EDU_EXEC, lgs_edp_ed_finalize_rec, 0, 0, EDU_EXIT,
            (uns32)&((LGS_CKPT_DATA*)0)->ckpt_rec.finalize_rec, 0, NULL},

        /* write log Record */
        {EDU_EXEC, lgs_edp_ed_write_rec, 0, 0, EDU_EXIT,
            (uns32)&((LGS_CKPT_DATA*)0)->ckpt_rec.writeLog, 0, NULL},

        /* Open stream */
        {EDU_EXEC, lgs_edp_ed_open_stream_rec, 0, 0, EDU_EXIT,
            (uns32)&((LGS_CKPT_DATA*)0)->ckpt_rec.stream_open, 0, NULL},

        /* Close stream */
        {EDU_EXEC, lgs_edp_ed_close_stream_rec, 0, 0, EDU_EXIT,
            (uns32)&((LGS_CKPT_DATA*)0)->ckpt_rec.stream_close, 0, NULL},

        /* Agent dest */
        {EDU_EXEC, ncs_edp_mds_dest, 0, 0, EDU_EXIT,
            (uns32)&((LGS_CKPT_DATA*)0)->ckpt_rec.agent_dest, 0, NULL},

        {EDU_END, 0, 0, 0, 0, 0, 0, NULL},
    };

    if (op == EDP_OP_TYPE_ENC)
    {
        ckpt_msg_ptr = (LGS_CKPT_DATA *)ptr;
    }
    else if (op == EDP_OP_TYPE_DEC)
    {
        ckpt_msg_dec_ptr = (LGS_CKPT_DATA **)ptr;
        if (*ckpt_msg_dec_ptr == NULL)
        {
            *o_err = EDU_ERR_MEM_FAIL;
            return NCSCC_RC_FAILURE;
        }
        memset(*ckpt_msg_dec_ptr, '\0', sizeof(LGS_CKPT_DATA));
        ckpt_msg_ptr = *ckpt_msg_dec_ptr;
    }
    else
    {
        ckpt_msg_ptr = ptr;
    }

    rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn, lgs_ckpt_msg_ed_rules, ckpt_msg_ptr, ptr_data_len,
                             buf_env, op, o_err);
    return rc;

} /* End lgs_edu_enc_dec_ckpt_msg() */

/* Non EDU routines */
/****************************************************************************
 * Name          : lgs_enc_ckpt_header
 *
 * Description   : This function encodes the checkpoint message header
 *                 using leap provided apis. 
 *
 * Arguments     : pdata - pointer to the buffer to encode this struct in. 
 *                 LGS_CKPT_HEADER - lgsv checkpoint message header. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

void lgs_enc_ckpt_header(uns8 *pdata,LGS_CKPT_HEADER header)
{
    ncs_encode_32bit(&pdata,header.ckpt_rec_type);
    ncs_encode_32bit(&pdata,header.num_ckpt_records);
    ncs_encode_32bit(&pdata,header.data_len);
}

/****************************************************************************
 * Name          : lgs_dec_ckpt_header
 *
 * Description   : This function decodes the checkpoint message header
 *                 using leap provided apis. 
 *
 * Arguments     : NCS_UBAID - pointer to the NCS_UBAID containing data.
 *                 LGS_CKPT_HEADER - lgsv checkpoint message header. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/

uns32 lgs_dec_ckpt_header(NCS_UBAID *uba, LGS_CKPT_HEADER *header)
{
    uns8 *p8;
    uns8 local_data[256];
    TRACE_ENTER();
    if ((uba == NULL) || (header == NULL))
    {
        TRACE("NULL pointer, FAILED");
        return NCSCC_RC_FAILURE;
    }
    p8 = ncs_dec_flatten_space(uba,local_data,4);
    header->ckpt_rec_type = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba,4);

    p8 = ncs_dec_flatten_space(uba,local_data,4);
    header->num_ckpt_records = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba,4);

    p8 = ncs_dec_flatten_space(uba,local_data,4);
    header->data_len = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba,4);
    TRACE_LEAVE(); 
    return NCSCC_RC_SUCCESS;
}/*End lgs_dec_ckpt_header */

