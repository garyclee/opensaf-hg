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

#define LGS_SVC_PVT_SUBPART_VERSION 1
#define LGS_WRT_LGA_SUBPART_VER_AT_MIN_MSG_FMT 1
#define LGS_WRT_LGA_SUBPART_VER_AT_MAX_MSG_FMT 1
#define LGS_WRT_LGA_SUBPART_VER_RANGE             \
        (LGS_WRT_LGA_SUBPART_VER_AT_MAX_MSG_FMT - \
         LGS_WRT_LGA_SUBPART_VER_AT_MIN_MSG_FMT + 1)

static MDS_CLIENT_MSG_FORMAT_VER
LGS_WRT_LGA_MSG_FMT_ARRAY[LGS_WRT_LGA_SUBPART_VER_RANGE] = {
    1 /*msg format version for LGA subpart version 1*/
};

/**
 * Name          : lgs_evt_destroy
 * 
 * Description   : This is the function which is called to destroy an event.
 * 
 * Arguments     : LGSV_LGS_EVT *
 * 
 * Return Values : NONE
 * 
 * Notes         : None.
 * 
 * @param evt
 */
void lgs_evt_destroy(lgsv_lgs_evt_t *evt)
{
    free(evt);
}

/****************************************************************************
  Name          : lgs_dec_initialize_msg
 
  Description   : This routine decodes an initialize API msg
 
  Arguments     : NCS_UBAID *msg,
                  LGSV_MSG *msg
                  
  Return Values : uns32
 
  Notes         : None.
******************************************************************************/
static uns32 lgs_dec_initialize_msg(NCS_UBAID *uba,  LGSV_MSG *msg)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    LGSV_LGA_INIT_PARAM *param = &msg->info.api_info.param.init;
    uns8      local_data[20];

    TRACE_ENTER();
    assert(uba != NULL);
    /* releaseCode, majorVersion, minorVersion */
    p8 = ncs_dec_flatten_space(uba, local_data, 3);
    param->version.releaseCode  = ncs_decode_8bit(&p8);
    param->version.majorVersion = ncs_decode_8bit(&p8);
    param->version.minorVersion = ncs_decode_8bit(&p8);
    ncs_dec_skip_space(uba, 3);
    total_bytes += 3;

    TRACE_LEAVE();
    return total_bytes;
}


/****************************************************************************
  Name          : lgs_dec_finalize_msg
 
  Description   : This routine decodes a finalize API msg
 
  Arguments     : NCS_UBAID *msg,
                  LGSV_MSG *msg
                  
  Return Values : uns32
 
  Notes         : None.
******************************************************************************/
static uns32 lgs_dec_finalize_msg(NCS_UBAID *uba,  LGSV_MSG *msg)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    LGSV_LGA_FINALIZE_PARAM *param = &msg->info.api_info.param.finalize;
    uns8        local_data[20];

    TRACE_ENTER();
    assert(uba != NULL);
    /* reg_id */
    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    param->reg_id = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    TRACE_ENTER();
    return total_bytes;
}

/****************************************************************************
  Name          : lgs_dec_lstr_open_sync_msg
 
  Description   : This routine decodes a log stream open sync API msg
 
  Arguments     : NCS_UBAID *msg,
                  LGSV_MSG *msg
                  
  Return Values : uns32
 
  Notes         : None.
******************************************************************************/
static uns32 lgs_dec_lstr_open_sync_msg(NCS_UBAID *uba,  LGSV_MSG *msg)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    LGSV_LGA_LSTR_OPEN_SYNC_PARAM *param = 
    &msg->info.api_info.param.lstr_open_sync;
    uns8       local_data[256];

    TRACE_ENTER();
    assert(uba != NULL);

    /* reg_id  */
    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    param->reg_id          = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    /* log stream name length */
    p8 = ncs_dec_flatten_space(uba, local_data, 2);
    param->lstr_name.length = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(uba, 2);
    total_bytes += 2;

    /* log stream name */
    ncs_decode_n_octets_from_uba(uba, param->lstr_name.value,
                                 (uns32)param->lstr_name.length);
    total_bytes += (uns32)param->lstr_name.length;

    /* log file name */
    p8 = ncs_dec_flatten_space(uba, local_data, 2);
    param->logFileName.length = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(uba, 2);
    total_bytes += 2;

    if (param->logFileName.length != 0)
    {
        ncs_decode_n_octets_from_uba(uba, param->logFileName.value,
                                     (uns32)param->logFileName.length);
        total_bytes += (uns32)param->logFileName.length;
    }

    /* log file path name */
    p8 = ncs_dec_flatten_space(uba, local_data, 2);
    param->logFilePathName.length = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(uba, 2);
    total_bytes += 2;

    if (param->logFilePathName.length != 0)
    {
        ncs_decode_n_octets_from_uba(uba, param->logFilePathName.value,
                                     (uns32)param->logFilePathName.length);
        total_bytes += (uns32)param->logFilePathName.length;
    }

    /* log record format length */
    p8 = ncs_dec_flatten_space(uba, local_data, 24);
    param->maxLogFileSize = ncs_decode_64bit(&p8);
    param->maxLogRecordSize = ncs_decode_32bit(&p8);
    param->haProperty = ncs_decode_32bit(&p8);
    param->logFileFullAction = ncs_decode_32bit(&p8);
    param->maxFilesRotated = ncs_decode_16bit(&p8);
    param->logFileFmtLength = ncs_decode_16bit(&p8);
    ncs_dec_skip_space(uba, 24);
    total_bytes += 24;

    /* Decode format string if initiated */
    if (param->logFileFmtLength != 0)
    {
        ncs_decode_n_octets_from_uba(uba, param->logFileFmt,
                                     (uns32)param->logFileFmtLength);
        total_bytes += (uns32)param->logFileFmtLength;
    }

    /* log stream open flags */
    p8 = ncs_dec_flatten_space(uba, local_data, 1);
    param->lstr_open_flags = ncs_decode_8bit(&p8);
    ncs_dec_skip_space(uba, 1);
    total_bytes += 1;

    TRACE_LEAVE();
    return total_bytes;
}

/****************************************************************************
  Name          : lgs_dec_lstr_close_msg
 
  Description   : This routine decodes a log stream close API msg
 
  Arguments     : NCS_UBAID *uba,
                  LGSV_MSG *msg
                  
  Return Values : uns32
 
  Notes         : None.
******************************************************************************/
static uns32 lgs_dec_lstr_close_msg(NCS_UBAID *uba, LGSV_MSG *msg)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    LGSV_LGA_LSTR_CLOSE_PARAM *param = &msg->info.api_info.param.lstr_close;
    uns8        local_data[20];
    TRACE_ENTER();

    if (uba == NULL)
    {
        TRACE("lstr close FAILED uba = NULL");
        return 0;
    }

    /* reg_id, lstr_id, lstr_open_id */
    p8 = ncs_dec_flatten_space(uba, local_data, 12);
    param->reg_id       = ncs_decode_32bit(&p8);
    param->lstr_id      = ncs_decode_32bit(&p8);
    param->lstr_open_id = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 12);
    total_bytes += 12;

    TRACE_LEAVE();
    return total_bytes;
}

/****************************************************************************
  Name          : lgs_dec_write_log_async_msg
 
  Description   : This routine decodes a write async log API msg
 
  Arguments     : NCS_UBAID *msg,
                  LGSV_MSG *msg
                  
  Return Values : uns32
 
  Notes         : None.
******************************************************************************/
static uns32 lgs_dec_write_log_async_msg(NCS_UBAID *uba, LGSV_MSG *msg)
{
    uns8         *p8;
    uns32        total_bytes = 0;
    /*uns32        fake_value;
    uns32        x;*/
    LGSV_LGA_WRITE_LOG_ASYNC_PARAM *param = &msg->info.api_info.param.write_log_async;
    uns8         local_data[1024];
    TRACE_ENTER();
    assert(uba != NULL);
    p8 = ncs_dec_flatten_space(uba, local_data, 24);
    param->invocation   = ncs_decode_64bit(&p8);
    param->ack_flags    = ncs_decode_32bit(&p8);
    param->reg_id       = ncs_decode_32bit(&p8);
    param->lstr_id      = ncs_decode_32bit(&p8);
    param->lstr_open_id = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 24);
    total_bytes += 24;

    param->logRecord = malloc(sizeof(SaLogRecordT));
    if (!param->logRecord)
    {
        TRACE("could not alloc memory");
        return(0);
    }
    /* ************* SaLogRecord decode ***************/
    p8 = ncs_dec_flatten_space(uba, local_data, 12);
    param->logRecord->logTimeStamp = ncs_decode_64bit(&p8);
    param->logRecord->logHdrType = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 12);
    total_bytes += 12;

    /*Only alarm, and application log streams so far..*/
    SaLogNtfLogHeaderT  *ntfLogH;
    SaLogGenericLogHeaderT *genLogH;
    ntfLogH = &param->logRecord->logHeader.ntfHdr;
    genLogH = &param->logRecord->logHeader.genericHdr;

    switch (param->logRecord->logHdrType)
    {
        case SA_LOG_NTF_HEADER:
            p8 = ncs_dec_flatten_space(uba, local_data, 14);
            ntfLogH->notificationId = ncs_decode_64bit(&p8); 
            ntfLogH->eventType = ncs_decode_32bit(&p8);

            ntfLogH->notificationObject= calloc(1, sizeof(SaNameT));
            if (!ntfLogH->notificationObject)
            {
                TRACE("could not alloc memory");
                return(0);
            }

            ntfLogH->notificationObject->length = ncs_decode_16bit(&p8);
            if (SA_MAX_NAME_LENGTH < ntfLogH->notificationObject->length)
            {
                TRACE("notificationObject to big");
                return(0);
            }
            ncs_dec_skip_space(uba, 14);
            total_bytes += 14;

            ncs_decode_n_octets_from_uba(uba,
                                         ntfLogH->notificationObject->value, 
                                         ntfLogH->notificationObject->length);
            total_bytes += ntfLogH->notificationObject->length;

            ntfLogH->notifyingObject = calloc(1, sizeof(SaNameT));
            if (!ntfLogH->notifyingObject)
            {
                TRACE("could not alloc memory");
                return(0);
            }
            p8 = ncs_dec_flatten_space(uba, local_data, 2);
            ntfLogH->notifyingObject->length = ncs_decode_16bit(&p8);
            ncs_dec_skip_space(uba, 2);
            total_bytes += 2;

            if (SA_MAX_NAME_LENGTH < ntfLogH->notifyingObject->length)
            {
                TRACE("notifyingObject to big");
                return(0);
            }

            ncs_decode_n_octets_from_uba(uba,
                                         ntfLogH->notifyingObject->value, 
                                         ntfLogH->notifyingObject->length);
            total_bytes += ntfLogH->notifyingObject->length;

            ntfLogH->notificationClassId = malloc(sizeof(SaNtfClassIdT));
            if (!ntfLogH->notificationClassId)
            {
                TRACE("could not alloc memory");
                return(0);
            }
            p8 = ncs_dec_flatten_space(uba, local_data, 16);
            ntfLogH->notificationClassId->vendorId = ncs_decode_32bit(&p8);
            ntfLogH->notificationClassId->majorId = ncs_decode_16bit(&p8);
            ntfLogH->notificationClassId->minorId = ncs_decode_16bit(&p8);
            ntfLogH->eventTime  = ncs_decode_64bit(&p8);
            ncs_dec_skip_space(uba, 16);
            total_bytes += 16;

            break;

        case SA_LOG_GENERIC_HEADER:
            genLogH->notificationClassId = malloc(sizeof(SaNtfClassIdT));
            if (!genLogH->notificationClassId)
            {
                TRACE("could not alloc memory");
                return(0);
            }
            p8 = ncs_dec_flatten_space(uba, local_data, 10);
            genLogH->notificationClassId->vendorId = ncs_decode_32bit(&p8);
            genLogH->notificationClassId->majorId = ncs_decode_16bit(&p8);
            genLogH->notificationClassId->minorId = ncs_decode_16bit(&p8);
            SaNameT        *logSvcUsrName;
            logSvcUsrName = malloc(sizeof(SaNameT));
            if (!logSvcUsrName)
            {
                TRACE("could not alloc memory");
                return(0);
            }

            /*
            ** A const value in genLogHeader is fucking up...
            ** Extra instance used.
            */
            logSvcUsrName->length = ncs_decode_16bit(&p8); 

            if (SA_MAX_NAME_LENGTH < logSvcUsrName->length)
            {
                TRACE("notificationObject to big");
                return(0);
            }
            ncs_dec_skip_space(uba, 10);
            total_bytes += 10;    

            ncs_decode_n_octets_from_uba(uba,
                                         logSvcUsrName->value , 
                                         logSvcUsrName->length);
            total_bytes += logSvcUsrName->length;
            genLogH->logSvcUsrName = logSvcUsrName;
            p8 = ncs_dec_flatten_space(uba, local_data, 2);
            genLogH->logSeverity = ncs_decode_16bit(&p8);
            ncs_dec_skip_space(uba, 2);
            total_bytes += 2;    
            break;

        default:
            TRACE("ERROR IN logHdrType in logRecord");
            break;
    }
    param->logRecord->logBuffer = malloc(sizeof(SaLogBufferT));
    if (!param->logRecord->logBuffer)
    {
        TRACE("could not alloc memory");
        return(0); /* FIX no error handling! */
    }
    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    param->logRecord->logBuffer->logBufSize = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    /* Make sure at least one byte is allocated for later */
    param->logRecord->logBuffer->logBuf = malloc(param->logRecord->logBuffer->logBufSize + 1);
    if (param->logRecord->logBuffer->logBuf == NULL)
    {
        TRACE("malloc failed");
        return(0); /* FIX no error handling! */
    }
    if (param->logRecord->logBuffer->logBufSize > 0)
    {
        ncs_decode_n_octets_from_uba(uba,
                                     param->logRecord->logBuffer->logBuf,
                                     (uns32)param->logRecord->logBuffer->logBufSize);
        total_bytes += (uns32)param->logRecord->logBuffer->logBufSize;
    }

    /************ end saLogRecord decode ****************/
    TRACE_LEAVE();
    return total_bytes;
}

/****************************************************************************
  Name          : lgs_enc_initialize_rsp_msg
 
  Description   : This routine encodes an initialize resp msg
 
  Arguments     : NCS_UBAID *msg,
                  LGSV_MSG *msg
                  
  Return Values : uns32
 
  Notes         : None.
******************************************************************************/
static uns32 lgs_enc_initialize_rsp_msg(NCS_UBAID *uba,  LGSV_MSG *msg)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    LGSV_LGA_INITIALIZE_RSP *param = &msg->info.api_resp_info.param.init_rsp;

    TRACE_ENTER();
    assert(uba != NULL);

    /* reg_id */
    p8 = ncs_enc_reserve_space(uba, 4);
    if (p8 == NULL)
    {
        TRACE("ncs_enc_reserve_space failed");
        goto done;
    }

    ncs_encode_32bit(&p8, param->reg_id);
    ncs_enc_claim_space(uba, 4);
    total_bytes += 4;

    done:
    TRACE_LEAVE();
    return total_bytes;
}

/****************************************************************************
  Name          : lgs_enc_lstr_open_sync_rsp_msg
 
  Description   : This routine decodes a log stream open sync resp msg
 
  Arguments     : NCS_UBAID *msg,
                  LGSV_MSG *msg
                  
  Return Values : uns32
 
  Notes         : None.
******************************************************************************/
static uns32 lgs_enc_lstr_open_sync_rsp_msg(NCS_UBAID *uba,  LGSV_MSG *msg)
{
    uns8        *p8;
    uns32       total_bytes = 0;
    LGSV_LGA_LSTR_OPEN_SYNC_RSP *param = 
    &msg->info.api_resp_info.param.lstr_open_rsp;

    TRACE_ENTER();
    assert(uba != NULL);

    /* chan_id, chan_open_id */
    p8 = ncs_enc_reserve_space(uba, 8);
    if (p8 == NULL)
    {
        TRACE("ncs_enc_reserve_space failed");
        goto done;
    }
    ncs_encode_32bit(&p8, param->lstr_id);
    ncs_enc_claim_space(uba, 4);
    total_bytes += 4;

    done:
    TRACE_LEAVE();
    return total_bytes;
}

/****************************************************************************
 * Name          : lgs_mds_cpy
 *
 * Description   : MDS copy.
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_cpy(struct ncsmds_callback_info *info)
{
    /* TODO; */
    return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : lgs_mds_enc
 *
 * Description   : MDS encode.
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_enc(struct ncsmds_callback_info *info)
{
    LGSV_MSG   *msg;
    NCS_UBAID  *uba;
    uns8       *p8;
    uns32      total_bytes = 0;
    MDS_CLIENT_MSG_FORMAT_VER msg_fmt_version;

    TRACE_ENTER();

    msg_fmt_version = m_NCS_ENC_MSG_FMT_GET(
                                           info->info.enc.i_rem_svc_pvt_ver,
                                           LGS_WRT_LGA_SUBPART_VER_AT_MIN_MSG_FMT,
                                           LGS_WRT_LGA_SUBPART_VER_AT_MAX_MSG_FMT,
                                           LGS_WRT_LGA_MSG_FMT_ARRAY);
    if (0 == msg_fmt_version)
    {
        LOG_ER("msg_fmt_version FAILED!");
        return NCSCC_RC_FAILURE;
    }
    info->info.enc.o_msg_fmt_ver = msg_fmt_version;

    msg = (LGSV_MSG*)info->info.enc.i_msg;
    uba = info->info.enc.io_uba;

    if (uba == NULL)
    {
        LOG_ER("uba == NULL");
        goto err;
    }

    /** encode the type of message **/
    p8 = ncs_enc_reserve_space(uba, 4);
    if (p8 == NULL)
    {
        TRACE("ncs_enc_reserve_space failed");
        goto err;
    }
    ncs_encode_32bit(&p8, msg->type);
    ncs_enc_claim_space(uba, 4);
    total_bytes += 4;

    if (LGSV_LGA_API_RESP_MSG == msg->type)
    {
        /** encode the API RSP msg subtype **/
        p8 = ncs_enc_reserve_space(uba, 4);
        if (!p8)
        {
            TRACE("ncs_enc_reserve_space failed");
            goto err;
        }
        ncs_encode_32bit(&p8, msg->info.api_resp_info.type);
        ncs_enc_claim_space(uba, 4);
        total_bytes += 4;

        /* rc */
        p8 = ncs_enc_reserve_space(uba, 4);
        if (!p8)
        {
            TRACE("ncs_enc_reserve_space failed");
            goto err;
        }
        ncs_encode_32bit(&p8, msg->info.api_resp_info.rc);
        ncs_enc_claim_space(uba, 4);
        total_bytes += 4;

        switch (msg->info.api_resp_info.type)
        {
            case LGSV_LGA_INITIALIZE_RSP_MSG:
                total_bytes += lgs_enc_initialize_rsp_msg(uba, msg);
                break;
            case LGSV_LGA_LSTR_OPEN_SYNC_RSP_MSG:
                total_bytes += lgs_enc_lstr_open_sync_rsp_msg(uba, msg);
                break;
            default:
                TRACE("Unknown API RSP type = %d", msg->info.api_resp_info.type);
                break;
        }
    }
    else if (LGSV_LGS_CBK_MSG == msg->type)
    {
        /** encode the API RSP msg subtype **/
        p8 = ncs_enc_reserve_space(uba, 16);
        if (!p8)
        {
            TRACE("ncs_enc_reserve_space failed");
            goto err;
        }
        ncs_encode_32bit(&p8, msg->info.cbk_info.type);
        ncs_encode_32bit(&p8, msg->info.cbk_info.lgs_reg_id);
        ncs_encode_64bit(&p8, msg->info.cbk_info.inv);
        ncs_enc_claim_space(uba, 16);
        total_bytes += 16;
        if (msg->info.cbk_info.type == LGSV_LGS_WRITE_LOG_CBK)
        {
            p8 = ncs_enc_reserve_space(uba, 4);
            if (!p8)
            {
                TRACE("ncs_enc_reserve_space failed");
                goto err;
            }
            ncs_encode_32bit(&p8, msg->info.cbk_info.write_cbk.error);
            total_bytes += 4;
        }
        else
        {
            TRACE("unknown callback type %d", msg->info.cbk_info.type);
            goto err;
        }
    }
    else
    {
        TRACE("unknown msg type %d", msg->type);
        goto err;
    }

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;

err:
    TRACE_LEAVE();
    return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : lgs_mds_dec
 *
 * Description   : MDS decode
 *
 * Arguments     : pointer to ncsmds_callback_info 
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_dec(struct ncsmds_callback_info *info)
{
    uns8          *p8;
    lgsv_lgs_evt_t  *evt;
    NCS_UBAID *uba = info->info.dec.io_uba;
    uns8      local_data[20];
    uns32     total_bytes = 0;

    TRACE_ENTER();

    if (0 == m_NCS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
                                       LGS_WRT_LGA_SUBPART_VER_AT_MIN_MSG_FMT,
                                       LGS_WRT_LGA_SUBPART_VER_AT_MAX_MSG_FMT,
                                       LGS_WRT_LGA_MSG_FMT_ARRAY))
    {
        TRACE("Wrong format version");
        goto err;
    }

    /** allocate an LGSV_LGS_EVENT now **/
    if (NULL == (evt = calloc(1, sizeof(lgsv_lgs_evt_t))))
    {
        TRACE("calloc failed");
        goto err;
    }

    /* Assign the allocated event */
    info->info.dec.o_msg = (uns8*)evt;

    p8 = ncs_dec_flatten_space(uba, local_data, 4);
    evt->info.msg.type = ncs_decode_32bit(&p8);
    ncs_dec_skip_space(uba, 4);
    total_bytes += 4;

    if (LGSV_LGA_API_MSG == evt->info.msg.type)
    {
        p8 = ncs_dec_flatten_space(uba, local_data, 4);
        evt->info.msg.info.api_info.type = ncs_decode_32bit(&p8);
        ncs_dec_skip_space(uba, 4);
        total_bytes += 4;

        /* FIX error handling for dec functions */
        switch (evt->info.msg.info.api_info.type)
        {
            case LGSV_LGA_INITIALIZE:
                total_bytes += lgs_dec_initialize_msg(uba, &evt->info.msg);
                break;
            case LGSV_LGA_FINALIZE:
                total_bytes += lgs_dec_finalize_msg(uba, &evt->info.msg);
                break;
            case LGSV_LGA_LSTR_OPEN_SYNC:
                total_bytes += lgs_dec_lstr_open_sync_msg(uba, &evt->info.msg);
                break;
            case LGSV_LGA_LSTR_CLOSE:
                total_bytes += lgs_dec_lstr_close_msg(uba, &evt->info.msg);
                break;     
            case LGSV_LGA_WRITE_LOG_ASYNC:
                total_bytes += lgs_dec_write_log_async_msg(uba, &evt->info.msg);
                break;
            default:
                TRACE("Unknown API type = %d", evt->info.msg.info.api_info.type);
                break;
        }
        if (total_bytes == 4)
            goto err;
    }
    else
    {
        TRACE("unknown msg type = %d", (int)evt->info.msg.type);
        goto err;
    }

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;

err:
    TRACE_LEAVE();
    return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : lgs_mds_enc_flat
 *
 * Description   : MDS encode and flatten
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_enc_flat(struct ncsmds_callback_info *info)
{
    uns32 rc= NCSCC_RC_SUCCESS;

    /* Retrieve info from the enc_flat */
    MDS_CALLBACK_ENC_INFO enc = info->info.enc_flat;
    /* Modify the MDS_INFO to populate enc */
    info->info.enc = enc;
    /* Invoke the regular mds_enc routine */
    rc = lgs_mds_enc(info);
    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE("lgs_mds_enc FAILED");
    }
    return rc;
}

/****************************************************************************
 * Name          : lgs_mds_dec_flat
 *
 * Description   : MDS decode and flatten
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32
lgs_mds_dec_flat(struct ncsmds_callback_info *info)
{
    uns32 rc = NCSCC_RC_SUCCESS;
    TRACE_ENTER();
    /* Retrieve info from the dec_flat */
    MDS_CALLBACK_DEC_INFO dec = info->info.dec_flat;
    /* Modify the MDS_INFO to populate dec */
    info->info.dec = dec;
    /* Invoke the regular mds_dec routine */
    rc = lgs_mds_dec(info);  
    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE("lgs_mds_dec FAILED ");
    }
    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_mds_rcv
 *
 * Description   : MDS rcv evt.
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_rcv(struct ncsmds_callback_info *mds_info)
{
    lgsv_lgs_evt_t *lgsv_evt = (lgsv_lgs_evt_t *)mds_info->info.receive.i_msg;
    uns32 rc = NCSCC_RC_SUCCESS;

    TRACE_ENTER();

    lgsv_evt->evt_type      = LGSV_LGS_LGSV_MSG;
    lgsv_evt->cb_hdl        = (uns32)mds_info->i_yr_svc_hdl;
    lgsv_evt->fr_node_id    = mds_info->info.receive.i_node_id;
    lgsv_evt->fr_dest       = mds_info->info.receive.i_fr_dest;
    lgsv_evt->rcvd_prio     = mds_info->info.receive.i_priority;
    lgsv_evt->mds_ctxt      = mds_info->info.receive.i_msg_ctxt;

    /* Send the message to lgs */
    rc =  m_NCS_IPC_SEND(&lgs_cb->mbx, lgsv_evt, mds_info->info.receive.i_priority);
    if (rc != NCSCC_RC_SUCCESS)
        TRACE("IPC send failed %d", rc);

    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_mds_quiesced_ack
 *
 * Description   : MDS quised ack.
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_quiesced_ack(struct ncsmds_callback_info *mds_info)
{
    lgsv_lgs_evt_t *lgsv_evt;

    TRACE_ENTER();

    /** allocate an LGSV_LGS_EVENT now **/
    if (NULL == (lgsv_evt = calloc(1, sizeof(lgsv_lgs_evt_t))))
    {
        TRACE("memory alloc FAILED");
        goto err;
    }

    if (lgs_cb->is_quisced_set == TRUE)
    {
        /** Initialize the Event here **/
        lgsv_evt->evt_type      = LGSV_EVT_QUIESCED_ACK;
        lgsv_evt->cb_hdl        = (uns32)mds_info->i_yr_svc_hdl;

        /* Push the event and we are done */
        if (NCSCC_RC_FAILURE == 
            m_NCS_IPC_SEND(&lgs_cb->mbx, lgsv_evt,NCS_IPC_PRIORITY_NORMAL))
        {
            TRACE("ipc send failed");
            lgs_evt_destroy(lgsv_evt);
            goto err;
        }
    }
    else
    {
        lgs_evt_destroy(lgsv_evt);
    }

    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;

    err:
    TRACE_LEAVE();
    return NCSCC_RC_FAILURE;
}


/****************************************************************************
 * Name          : lgs_mds_svc_event
 *
 * Description   : MDS subscription evt.
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_svc_event(struct ncsmds_callback_info *info)
{
    lgsv_lgs_evt_t *evt=NULL;
    uns32 rc = NCSCC_RC_SUCCESS;

    TRACE_ENTER();
    TRACE_2("LGS Rcvd MDS subscribe evt from svc %d", info->info.svc_evt.i_svc_id);

    /* First make sure that this event is indeed for us*/
    if (info->info.svc_evt.i_your_id != NCSMDS_SVC_ID_LGS)
    {
        TRACE_2("event not NCSMDS_SVC_ID_LGS");
        rc = NCSCC_RC_FAILURE;
        goto done;
    }

    /* If this evt was sent from LGA act on this */
    if (info->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_LGA)
    {
        TRACE_2("message from LGA ");
        if (info->info.svc_evt.i_change == NCSMDS_DOWN)
        {
            TRACE_2("MDS DOWN MDS DEST: %lx Node ID: %x in MDS svc event svc_id: %d\n",
                   (long unsigned int) info->info.svc_evt.i_dest,
                   info->info.svc_evt.i_node_id,
                   info->info.svc_evt.i_svc_id);

            /* As of now we are only interested in LGA events */
            if (NULL == (evt = calloc(1, sizeof(lgsv_lgs_evt_t))))
            {
                rc = NCSCC_RC_FAILURE;
                TRACE_2("mem alloc FAILURE  ");
                goto done;
            }

            evt->evt_type = LGSV_LGS_EVT_LGA_DOWN;

            /** Initialize the Event Header **/
            evt->cb_hdl        = 0;
            evt->fr_node_id    = info->info.svc_evt.i_node_id;
            evt->fr_dest       = info->info.svc_evt.i_dest;

            /** Initialize the MDS portion of the header **/
            evt->info.mds_info.node_id     = info->info.svc_evt.i_node_id;
            evt->info.mds_info.mds_dest_id = info->info.svc_evt.i_dest;

            /* Push the event and we are done */
            if (m_NCS_IPC_SEND(&lgs_cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL) != NCSCC_RC_SUCCESS)
            {
                TRACE_2("ipc send failed");
                lgs_evt_destroy(evt);
                rc = NCSCC_RC_FAILURE;
                goto done;
            }
        }
    }

    done:
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : lgs_mds_sys_evt
 *
 * Description   : MDS sys evt .
 *
 * Arguments     : pointer to ncsmds_callback_info
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_sys_event(struct ncsmds_callback_info *mds_info)
{
    TRACE_ENTER();
    /* Not supported now */
    return NCSCC_RC_FAILURE;
    TRACE_LEAVE();
} 

/****************************************************************************
 * Name          : lgs_mds_callback
 *
 * Description   : Callback Dispatcher for various MDS operations.
 *
 * Arguments     : cb   : LGS control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 lgs_mds_callback(struct ncsmds_callback_info *info)
{
    uns32  rc = NCSCC_RC_SUCCESS;
    static NCSMDS_CALLBACK_API cb_set[MDS_CALLBACK_SVC_MAX] = 
    {
        lgs_mds_cpy,      /* MDS_CALLBACK_COPY      */
        lgs_mds_enc,      /* MDS_CALLBACK_ENC       */
        lgs_mds_dec,      /* MDS_CALLBACK_DEC       */
        lgs_mds_enc_flat, /* MDS_CALLBACK_ENC_FLAT  */
        lgs_mds_dec_flat, /* MDS_CALLBACK_DEC_FLAT  */
        lgs_mds_rcv,      /* MDS_CALLBACK_RECEIVE   */
        lgs_mds_svc_event, /* MDS_CALLBACK_SVC_EVENT */
        lgs_mds_sys_event,/* MDS_CALLBACK_SYS_EVENT */
        lgs_mds_quiesced_ack/* MDS_CALLBACK_QUIESCED_ACK */
    };

    if (info->i_op <= MDS_CALLBACK_QUIESCED_ACK)
    {
        return (*cb_set[info->i_op])(info);
    }
    else
    {
        TRACE("mds callback out of range");
        rc = NCSCC_RC_FAILURE;
    }

    return rc;
}


/****************************************************************************
 * Name          : lgs_mds_vdest_create
 *
 * Description   : This function created the VDEST for LGS
 *
 * Arguments     : 
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_mds_vdest_create(lgs_cb_t *lgs_cb)
{
    NCSVDA_INFO vda_info;
    uns32 rc = NCSCC_RC_SUCCESS;

    memset(&vda_info, 0, sizeof(NCSVDA_INFO));
    lgs_cb->vaddr = LGS_VDEST_ID;
    vda_info.req = NCSVDA_VDEST_CREATE;
    vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
    vda_info.info.vdest_create.i_create_oac = TRUE;/* To simplify. Check this. TBD */
    vda_info.info.vdest_create.i_persistent = FALSE; /* Up-to-the application */
    vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
    vda_info.info.vdest_create.info.specified.i_vdest = lgs_cb->vaddr;

    /* Create the VDEST address */
    if (NCSCC_RC_SUCCESS != (rc =ncsvda_api(&vda_info)))
    {
        LOG_ER("VDEST_CREATE_FAILED");
        return rc;
    }

    /* Store the info returned by MDS */
    lgs_cb->mds_hdl          = vda_info.info.vdest_create.o_mds_pwe1_hdl;

    return rc;
}

/****************************************************************************
 * Name          : lgs_mds_init
 *
 * Description   : This function creates the VDEST for lgs and installs/suscribes
 *                 into MDS.
 *
 * Arguments     : cb   : LGS control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 lgs_mds_init(lgs_cb_t *cb)
{
    NCSMDS_INFO          mds_info;
    uns32                rc;
    MDS_SVC_ID           svc = NCSMDS_SVC_ID_LGA;

    TRACE_ENTER();

    /* Create the VDEST for LGS */
    if (NCSCC_RC_SUCCESS != (rc = lgs_mds_vdest_create(cb)))
    {
        LOG_ER(" lgs_mds_init: named vdest create FAILED\n");
        return rc;
    }

    /* Set the role of MDS */
    if (cb->ha_state == SA_AMF_HA_ACTIVE)
        cb->mds_role = V_DEST_RL_ACTIVE;

    if (cb->ha_state == SA_AMF_HA_STANDBY)
        cb->mds_role = V_DEST_RL_STANDBY;

    if (NCSCC_RC_SUCCESS != (rc = lgs_mds_change_role(cb)))
    {
        LOG_ER("MDS role change to %d FAILED\n",cb->mds_role);
        return rc;
    }

    /* Install your service into MDS */
    memset(&mds_info,'\0',sizeof(NCSMDS_INFO));
    mds_info.i_mds_hdl        = cb->mds_hdl;
    mds_info.i_svc_id         = NCSMDS_SVC_ID_LGS;
    mds_info.i_op             = MDS_INSTALL;
    mds_info.info.svc_install.i_yr_svc_hdl      = 0;
    mds_info.info.svc_install.i_install_scope   = NCSMDS_SCOPE_NONE;
    mds_info.info.svc_install.i_svc_cb          = lgs_mds_callback;
    mds_info.info.svc_install.i_mds_q_ownership = FALSE;
    mds_info.info.svc_install.i_mds_svc_pvt_ver = LGS_SVC_PVT_SUBPART_VERSION;

    if (NCSCC_RC_SUCCESS != (rc = ncsmds_api(&mds_info)))
    {
        LOG_ER("MDS Install FAILED");
        return rc;
    }

    /* Now subscribe for LGS events in MDS, TODO: WHy this? */
    memset(&mds_info,'\0',sizeof(NCSMDS_INFO));
    mds_info.i_mds_hdl        = cb->mds_hdl;
    mds_info.i_svc_id         = NCSMDS_SVC_ID_LGS;
    mds_info.i_op             = MDS_SUBSCRIBE;
    mds_info.info.svc_subscribe.i_scope         = NCSMDS_SCOPE_NONE;
    mds_info.info.svc_subscribe.i_num_svcs      = 1;
    mds_info.info.svc_subscribe.i_svc_ids       = &svc;

    rc = ncsmds_api(&mds_info);
    if (rc != NCSCC_RC_SUCCESS)
    {
        LOG_ER("MDS subscribe FAILED");
        return rc;
    }

    TRACE_LEAVE();
    return rc;
}

/****************************************************************************
 * Name          : lgs_mds_change_role
 *
 * Description   : This function informs mds of our vdest role change 
 *
 * Arguments     : cb   : LGS control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 lgs_mds_change_role(lgs_cb_t *cb)
{
    NCSVDA_INFO arg;

    memset(&arg, 0, sizeof(NCSVDA_INFO));

    arg.req = NCSVDA_VDEST_CHG_ROLE;
    arg.info.vdest_chg_role.i_vdest = cb->vaddr;
    arg.info.vdest_chg_role.i_new_role = cb->mds_role;

    return ncsvda_api(&arg);
}

/****************************************************************************
 * Name          : lgs_mds_vdest_destroy
 *
 * Description   : This function Destroys the Virtual destination of LGS
 *
 * Arguments     : cb   : LGS control Block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 lgs_mds_vdest_destroy (lgs_cb_t *lgs_cb)
{
    NCSVDA_INFO    vda_info;
    uns32          rc;

    memset(&vda_info, 0, sizeof(NCSVDA_INFO));
    vda_info.req                             = NCSVDA_VDEST_DESTROY;   
    vda_info.info.vdest_destroy.i_vdest      = lgs_cb->vaddr;

    if (NCSCC_RC_SUCCESS != ( rc = ncsvda_api(&vda_info)))
    {
        LOG_ER("NCSVDA_VDEST_DESTROY failed");
        return rc;
    }

    return rc;
}

/****************************************************************************
 * Name          : lgs_mds_finalize
 *
 * Description   : This function un-registers the LGS Service with MDS.
 *
 * Arguments     : Uninstall LGS from MDS.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 lgs_mds_finalize (lgs_cb_t *cb)
{
    NCSMDS_INFO          mds_info;
    uns32                rc;

    /* Un-install LGS service from MDS */
    memset(&mds_info, 0, sizeof(NCSMDS_INFO));
    mds_info.i_mds_hdl        = cb->mds_hdl;
    mds_info.i_svc_id         = NCSMDS_SVC_ID_LGS;
    mds_info.i_op             = MDS_UNINSTALL;

    rc = ncsmds_api(&mds_info);

    if (rc != NCSCC_RC_SUCCESS)
    {
        LOG_ER("MDS_UNINSTALL_FAILED");
        return rc;
    }

    /* Destroy the virtual Destination of LGS */
    rc = lgs_mds_vdest_destroy(cb);
    return rc;
}


/****************************************************************************
  Name          : lgs_mds_msg_send
 
  Description   : This routine sends the LGA message to LGS. The send 
                  operation may be a 'normal' send or a synchronous call that 
                  blocks until the response is received from LGS.
 
  Arguments     : cb  - ptr to the LGA CB
                  i_msg - ptr to the LGSv message
                  dest  - MDS destination to send to.
                  mds_ctxt - ctxt for synch mds req-resp.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/timeout
 
  Notes         : None.
******************************************************************************/

uns32 lgs_mds_msg_send (lgs_cb_t            *cb,
                        LGSV_MSG          *msg,
                        MDS_DEST          *dest,
                        MDS_SYNC_SND_CTXT *mds_ctxt,
                        MDS_SEND_PRIORITY_TYPE prio)
{
    NCSMDS_INFO   mds_info;
    MDS_SEND_INFO *send_info = &mds_info.info.svc_send;
    uns32         rc = NCSCC_RC_SUCCESS;
    TRACE_ENTER();
    /* populate the mds params */
    memset(&mds_info, '\0', sizeof(NCSMDS_INFO));

    mds_info.i_mds_hdl = cb->mds_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_LGS;
    mds_info.i_op = MDS_SEND;

    send_info->i_msg    = msg;
    send_info->i_to_svc = NCSMDS_SVC_ID_LGA;
    send_info->i_priority = prio; /* Discuss the priority assignments TBD */

    if (NULL == mds_ctxt || 
        0 == mds_ctxt->length)
    {
        TRACE_2("MDS_SENDTYPE_SND");
        /* regular send */
        MDS_SENDTYPE_SND_INFO *send = &send_info->info.snd;

        send_info->i_sendtype = MDS_SENDTYPE_SND;
        send->i_to_dest = *dest;
    }
    else
    {
        TRACE_2("MDS_SENDTYPE_RSP");
        /* response message (somebody is waiting for it) */
        MDS_SENDTYPE_RSP_INFO *resp = &send_info->info.rsp;

        send_info->i_sendtype = MDS_SENDTYPE_RSP;
        resp->i_sender_dest = *dest;
        resp->i_msg_ctxt = *mds_ctxt;
    }

    /* send the message */
    rc = ncsmds_api(&mds_info);
    if (rc != NCSCC_RC_SUCCESS)
    {
        TRACE("mds send FAILED");
    }
    TRACE_LEAVE();
    return rc;
}


/****************************************************************************
  Name          : lgs_mds_ack_send
 
  Description   : This routine sends the LGA message to LGS. The send 
                  operation blocks until an MDS ack is received from LGS.
 
  Arguments     : cb  - ptr to the LGA CB
                  msg - ptr to the LGSv message
                  dest - MDS dest to send to.
                  timeout - time to wait for the ack from LGA. 
                  prio - priority of the message to send.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE/timeout
 
  Notes         : None.
******************************************************************************/

uns32 lgs_mds_ack_send (lgs_cb_t   *cb,
                        LGSV_MSG *msg,
                        MDS_DEST dest,
                        uns32    timeout,
                        uns32    prio)
{
    NCSMDS_INFO  mds_info;
    uns32 rc = NCSCC_RC_SUCCESS;

    if (NULL == msg)
        return NCSCC_RC_FAILURE;

    memset(&mds_info, '\0', sizeof(NCSMDS_INFO));
    mds_info.i_mds_hdl = cb->mds_hdl;
    mds_info.i_svc_id = NCSMDS_SVC_ID_LGS;
    mds_info.i_op = MDS_SEND;

    /* Fill the send structure */
    mds_info.info.svc_send.i_msg = (NCSCONTEXT)msg;
    mds_info.info.svc_send.i_priority = prio;
    mds_info.info.svc_send.i_to_svc =   NCSMDS_SVC_ID_LGA;
    mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SNDACK;

    /* fill the sub send rsp strcuture */
    mds_info.info.svc_send.info.sndack.i_time_to_wait = timeout; /* timeto wait in 10ms */
    mds_info.info.svc_send.info.sndack.i_to_dest = dest; /* This dest is lga's. */

    /* send the message */
    if (NCSCC_RC_SUCCESS != (rc = ncsmds_api(&mds_info)))
    {
        TRACE("mds sndack failed");
    }

    return rc;
}

