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
                                          
.....................................................................  
                                              
DESCRIPTION: This file describes MBCSv Interface implementation for MAS
*****************************************************************************/ 
#if (NCS_MAS_RED == 1)
#include "mab.h"
#include "ncs_util.h"

/* encodes the reqd data into userbufs based on the requested type */ 
static uns32 
mas_red_msg_enc(MAS_TBL* inst, NCS_MBCSV_CB_ENC* enc);

/* to decode the request from MBCSv */ 
static uns32 
mas_red_msg_dec(MAS_TBL* inst, NCS_MBCSV_CB_DEC* dec); 

/* encode the async count of all the buckets into a user buf */
static uns32
mas_red_async_count_enc(uns32  *async_count, NCS_UBAID *uba); 

/* to decode the async count array */ 
static uns32
mas_red_async_count_dec(uns32  *async_count, NCS_UBAID *uba);

/* MBCSv COLD Sync Resp encode callback */ 
/* Encodes one bucket of hash table */ 
static uns32 
mas_red_cold_sync_enc(MAS_TBL* inst, NCS_MBCSV_CB_ENC *enc); 

/* MBCSv COLD Sync Resp decode callback */ 
/* Decodes one bucket of hash table */ 
static uns32 
mas_red_cold_sync_dec(MAS_TBL* inst, NCS_MBCSV_CB_DEC *dec); 

static uns32
mas_red_warm_sync_resp_dec(MAS_TBL *inst, NCS_MBCSV_CB_DEC  *dec);

/* Active MAS decodes the Datareq and sets the context for Data Resp */
static uns32
mas_red_data_req_dec(NCS_MBCSV_CB_DEC *dec);

/* Active MAS sends DATA reposne to Standby MAS */
static uns32 
mas_red_data_rsp_enc_cb(MAS_TBL* inst, NCS_MBCSV_CB_ENC *enc); 

/* Standby MAS received the data response */    
static uns32
mas_red_data_resp_dec_cb(MAS_TBL *inst, NCS_MBCSV_CB_DEC *dec); 

/* Encode all the table-records in a bucket and the associated async count */
static uns32
mas_red_bucket_enc(uns8 bucket_id, MAS_ROW_REC *tbl_list, 
                  uns32  async_count, NCS_MBCSV_CB_ENC *enc);

/* Decode all the table-records in a bucket */
static uns32 
mas_red_bucket_dec(uns8 *bucket_id, MAS_ROW_REC **tbl_list, 
                  uns32 *async_count, NCS_MBCSV_CB_DEC *dec);

/* encode filter list */ 
static uns32
mas_red_mas_fltr_enc(MAS_FLTR *fltr_list, NCS_UBAID *data_uba);

/* decode the filter list */ 
static uns32
mas_red_mas_fltr_dec(MAS_FLTR **fltr_list, NCS_UBAID *data_uba);

/* encode the default filter attributes */ 
static uns32
mas_red_mas_dfltr_enc(MAS_DFLTR *dfltr, NCS_UBAID *data_uba); 

/* decode the default filter attributes */ 
static uns32
mas_red_mas_dfltr_dec(MAS_DFLTR *dfltr, NCS_UBAID *data_uba);

/* encode the filter-ids linked list */ 
static uns32
mas_red_mas_fltr_ids_enc(MAB_FLTR_ANCHOR_NODE *fltr_ids, NCS_UBAID *data_uba); 

/* decode the filter-ids linked list */ 
static uns32
mas_red_mas_fltr_ids_dec(MAB_FLTR_ANCHOR_NODE **fltr_ids, NCS_UBAID *data_uba); 

/* Encode the Async update to be sent to Standby MAS */ 
static uns32 
mas_red_updt_enc(NCS_MBCSV_CB_ENC* enc); 

/* Decode the Async update of Active at Standby MAS */ 
static uns32 
mas_red_updt_dec(MAS_TBL* inst, NCS_MBCSV_CB_DEC* dec); 

static uns32 
mas_red_process_mab_msg(MAS_TBL *inst, MAS_ASYNC_UPDATE_TYPE async_type);

static uns32
mas_red_err_ind_handle(MAS_TBL *inst, NCS_MBCSV_CB_ARG *arg);  

/* dump the contents the filters of a table */ 
static void 
mas_dump_tbl_rec(FILE *fp, MAS_ROW_REC *tbl_rec);

/* dump the MDS_DEST */ 
static void
mas_dump_vcard(FILE *fp, MDS_DEST mds_dest);  

/* dump the filter information */ 
static void 
mas_dump_fltr_ids_list(FILE *fp, MAS_FLTR_IDS *fltr_ids); 

#if (NCS_MAS_RED_UNIT_TEST == 1)
extern uns32 verify_async_count_enc_dec(void); 
extern uns32 verify_mas_fltr_ids_enc_dec(void); 
#endif
/**************************************************************************\
* Function :  mas_mbcsv_cb                                                 *
*                                                                          *
* Purpose  :  Main entry function for MBCSv Agent, it's provided to MBCSV  *
*             at the time of registration of MAS RE with MBCSv             *
*                                                                          *
* Input    :  NCS_MBCSV_CB_ARG* arg  :  Argument structure from MBCSv      *
*                                                                          *
* Returns  :  NCSCC_RC_SUCCESS   RE was able to act on MBCSv message       *
*             NCSCC_RC_FAILURE   RE was unable to act on MBCSv message     *
*                                                                          *
* Notes    :                                                               *
\**************************************************************************/
uns32 mas_mbcsv_cb(NCS_MBCSV_CB_ARG* arg)
{
    uns32     rc = NCSCC_RC_FAILURE;
    uns32     hm_hdl;
    MAS_TBL*  inst;

    m_MAS_LK_INIT;

    if (arg == NULL)
    {
        /* log the failure */
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_SVC_PRVDR, 
                                MAB_MAS_ERR_MBCSV_CB_NULL_ARG);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* get the controlblock of MAS from the Handle */ 
    hm_hdl = arg->i_client_hdl;
    if ((inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(hm_hdl)) == NULL)
    {
        /* log the failure */
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_SVC_PRVDR, 
                                MAB_MAS_ERR_MBCSV_CB_INVALID_HDL);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    
    /* take the lock */ 
    m_MAS_LK(&inst->lock);

    /** Dispatch into handler for each op type.
    **/
    switch(arg->i_op)
    {
        case NCS_MBCSV_CBOP_ENC:
          rc = mas_red_msg_enc(inst, &arg->info.encode);
          break;

        case NCS_MBCSV_CBOP_DEC:
          rc = mas_red_msg_dec(inst, &arg->info.decode);
          break;

        case NCS_MBCSV_CBOP_ERR_IND: 
        /* log the error */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR, 
                           MAB_MAS_ERR_MBCSV_CB_ERR_IND, 
                           arg->info.error.i_code, arg->info.error.i_err); 
        rc = mas_red_err_ind_handle(inst, arg); 
        rc = NCSCC_RC_SUCCESS;
        break; 

        case NCS_MBCSV_CBOP_NOTIFY:
        case NCS_MBCSV_CBOP_PEER:
        default:
            m_LOG_MAB_ERROR_I(NCSFL_SEV_INFO, 
                          MAB_MAS_ERR_MBCSV_OP_NOT_SUPPORTED, 
                          arg->i_op);  
          rc = NCSCC_RC_SUCCESS;
          break;
    } /* switch(arg->i_op) */

    /* unlcok the cb lock */ 
    m_MAS_UNLK(&inst->lock);
    
    /* give the handle */ 
    ncshm_give_hdl(hm_hdl);

    return rc;
}

/****************************************************************************\
*  Name:          mas_red_msg_enc                                             * 
*                                                                            *
*  Description:   Encodes the MAS data as requested by MBCSv                 *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 NCS_MBCSV_CB_ENC - Place to keep the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
static uns32 
mas_red_msg_enc(MAS_TBL* inst, NCS_MBCSV_CB_ENC* enc)
{
    uns32       i;
    uns32       rc = NCSCC_RC_SUCCESS; /* Return code */

    /* log the function entry and type of message */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_MBCSV_ENC, 
                     enc->io_msg_type); 

    /* Distribute the encode job to smaller workers */
    switch (enc->io_msg_type)
    {
            /* update the Async (MAB_MSG*) message to Standby */ 
        case NCS_MBCSV_MSG_ASYNC_UPDATE:
            rc = mas_red_updt_enc(enc);
            break;

            /* Active MAS is responding */ 
        case NCS_MBCSV_MSG_COLD_SYNC_RESP:
        case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
            rc = mas_red_cold_sync_enc(inst, enc);
            inst->red.cold_sync_done = TRUE; 
            break; 

            /* STANDBY MAS is sending COLD_SYNC request */ 
        case NCS_MBCSV_MSG_COLD_SYNC_REQ:
            m_MAS_LK(&inst->lock);
            
            /* STANDBY MAS is not yet done with the cold-sync */
            inst->red.cold_sync_done = FALSE; 
           
            /* clean the table - records */
            m_NCS_MEMSET(inst->red.async_count, 0, 
                            sizeof(uns32)*MAB_MIB_ID_HASH_TBL_SIZE);

            /* free the filter details of all the tables of this CSI */ 
            for(i = 0;i < MAB_MIB_ID_HASH_TBL_SIZE;i++)
            {
                /* free the table-records in this bucket */ 
                mas_table_rec_list_free(inst->hash[i]); 
                inst->hash[i] = NULL; 
            }/* for all the tables in the hash table */ 

            m_MAS_UNLK(&inst->lock);
            break; 

            /* STANDBY MAS is sending the warm-sync request */
        case NCS_MBCSV_MSG_WARM_SYNC_REQ:
            break; 
            
            /* sync the standby MAS with async counts */
        case NCS_MBCSV_MSG_WARM_SYNC_RESP:
        case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
            rc = mas_red_async_count_enc(inst->red.async_count, &enc->io_uba); 
            enc->io_msg_type = NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE;
            break; 

            /* Active MAS is sending the response */
        case NCS_MBCSV_MSG_DATA_RESP:
        case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
            rc = mas_red_data_rsp_enc_cb(inst, enc);
            break; 

        default:
            rc = NCSCC_RC_FAILURE;
            m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                MAB_MAS_ERR_MBCSV_ENC_UNSUP_MSG_TYPE, 
                                enc->io_msg_type); 
            break; 
    }

    if (rc != NCSCC_RC_SUCCESS)
    {
        /* log the encode error */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, 
              MAB_MAS_ERR_MBCSV_ENC_FAILED, enc->io_msg_type, rc);  
    }

    return rc;
}

/****************************************************************************\
*  Name:          mas_red_msg_dec                                             * 
*                                                                            *
*  Description:   Decodes the MAS data as requested by MBCSv                 *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 NCS_MBCSV_CB_DEC - Place to get the encoded buffer and     * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* to decode the request from MBCSv */ 
static uns32 
mas_red_msg_dec(MAS_TBL* inst, NCS_MBCSV_CB_DEC* dec)
{
    uns32             rc = NCSCC_RC_SUCCESS; /* Return code */

    /* log the msg_type */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_MBCSV_DEC, 
                     dec->i_msg_type); 

    /* Distribute the decode job to smaller workers */
    switch (dec->i_msg_type)
    {
            /* Standby MAS Decodes the Async update of Active MAS */ 
        case NCS_MBCSV_MSG_ASYNC_UPDATE:
            rc = mas_red_updt_dec(inst, dec);
            break;

            /* Standby MAS decodes the Cold sync response of Active MAS */ 
        case NCS_MBCSV_MSG_COLD_SYNC_RESP:
            rc = mas_red_cold_sync_dec(inst, dec);
            break; 

        case NCS_MBCSV_MSG_COLD_SYNC_REQ:
#if 0
            printf("Cold Sync Request received from MBCSv\n");
#endif
            break;

            /* Standby MAS decodes the Cold Sync Complete response message */ 
        case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
            rc = mas_red_cold_sync_dec(inst, dec);
            if (rc == NCSCC_RC_SUCCESS)
            {
#if 0
                if ((gl_mas_amf_attribs.amf_attribs.amfHandle != 0) && 
                    (inst->amf_invocation_id != 0))
                {
                    /* send the AMF response succcess to AMF */ 
                    saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                                  inst->amf_invocation_id, SA_AIS_OK); 

                    /* log that pending respone to AMF is sent now */ 
                    m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, MAB_HDLN_AMF_SBY_RESPONSE_SENT,
                                      (uns32)inst->amf_invocation_id, inst->vrid);

                    /* reset the invocation id */ 
                    inst->amf_invocation_id = 0;
                }
#endif

                /* Standby MAS is in sync with Active MAS */ 
                inst->red.cold_sync_done = TRUE; 

                /* log that Cold-sync is complete, AMF response is being sent */ 
                m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE,MAB_HDLN_MAS_COLD_SYNC_DEC_COMPL);
            }
            break; 

        case NCS_MBCSV_MSG_WARM_SYNC_REQ:
            break; 
            
            /* decode the async counts of Active peer */
        case NCS_MBCSV_MSG_WARM_SYNC_RESP:
        case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
            rc = mas_red_warm_sync_resp_dec(inst, dec); 
            break; 
    
            /* Active MAS servicing the Data Request of Standby */
        case NCS_MBCSV_MSG_DATA_REQ:
            rc = mas_red_data_req_dec(dec);
            break; 
           
           /* Standby MAS received the data response */  
        case NCS_MBCSV_MSG_DATA_RESP:
        case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
            rc = mas_red_data_resp_dec_cb(inst, dec);
        break; 

        default:
            rc = NCSCC_RC_FAILURE;
            m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                MAB_MAS_ERR_MBCSV_DEC_UNSUP_MSG_TYPE, 
                                dec->i_msg_type); 
        break; 
    }

    if (rc != NCSCC_RC_SUCCESS)
    {
        /* log the encode error */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, 
              MAB_MAS_ERR_MBCSV_DEC_FAILED, dec->i_msg_type, rc);  
    }
    m_MAB_DBG_TRACE("\nmas_red_msg_dec():left.");
    return rc;
}

/****************************************************************************\
*  Name:          mas_red_cold_sync_enc                                       * 
*                                                                            *
*  Description:   Encodes the table-record in a bucket.  This function at a  * 
*                 time encodes a single bucket and sends the reponse to      * 
*                 MBCSv.  MBCSv has to call this function as many times as   * 
*                 the number of buckets in the hash table.                   *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 NCS_MBCSV_CB_ENC - Place to keep the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* to encode the table-records per bucket */
static uns32 
mas_red_cold_sync_enc(MAS_TBL* inst, NCS_MBCSV_CB_ENC *enc)
{
    uns8   bucket_id = (uns8)enc->io_reo_hdl;
    uns32  status; 

    /* encode this bucket */ 
#if 0
    printf("ColdSync: Encoding the bucket-id: %d\n",bucket_id); 
#endif
    status = mas_red_bucket_enc(bucket_id, inst->hash[bucket_id], 
                               inst->red.async_count[bucket_id], enc);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                      MAB_MAS_ERR_COLD_SYNC_ENC_BKT_FAILED, 
                      bucket_id);  
        return NCSCC_RC_FAILURE;  /* to continue on syncing for the other buckets */
    }
    
    /* if this is the last bucket, set the cold sync response complete */ 
    /* reset all our local values */ 
    if (bucket_id == MAB_MIB_ID_HASH_TBL_SIZE-1)
    {
        m_NCS_MEMSET(inst->red.this_bucket_synced, 0, sizeof(inst->red.this_bucket_synced));
        enc->io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE; 
        enc->io_reo_hdl = 0;
        m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_COLD_SYNC_ENC_COMPL, inst->vrid);
    }
    else
    {
        /* set the stage for next callback, next bucket-id */ 
        enc->io_reo_hdl += 1; 
        m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE,MAB_HDLN_MAS_COLD_SYNC_ENC_BKT, bucket_id, inst->vrid);
#if 0
        if ((bucket_id == 1))
            sleep(5);
#endif
    }

    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_cold_sync_dec                                       * 
*                                                                            *
*  Description:   Encodes the table-record in a bucket.  This function at a  * 
*                 time decodes a single bucket and sends the reponse to      * 
*                 MBCSv.  MBCSv has to call this function as many times as   * 
*                 the number of buckets in the hash table.                   *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 NCS_MBCSV_CB_DEC - Place to read the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
static uns32 
mas_red_cold_sync_dec(MAS_TBL* inst, NCS_MBCSV_CB_DEC *dec)
{
    uns8    dec_bucket_id; 
    uns32   status; 
    uns32   async_count;
    MAS_ROW_REC *tbl_rec; 
    
    /* decode the response */ 
    status = mas_red_bucket_dec(&dec_bucket_id, &tbl_rec, &async_count, dec);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                      MAB_MAS_ERR_COLD_SYNC_DEC_BKT_FAILED, 
                      dec_bucket_id);  
        return status; 
    }
#if 0
    printf("ColdSync: Decoding the bucket-id: %d\n",dec_bucket_id); 
#endif
    /* add to the hash table */ 
    if (inst->hash[dec_bucket_id] == NULL)
    {
        /* update the table records */ 
        inst->hash[dec_bucket_id] = tbl_rec; 

        /* update the async count of this bucket */
        inst->red.async_count[dec_bucket_id] = async_count; 

        /* update that cold_sync is done for this bucket */ 
        inst->red.this_bucket_synced[dec_bucket_id] = TRUE; 

        /* log that this bucket is decoded and added successfully */ 
        m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_COLD_SYNC_DEC_SUCCESS, 
                         dec_bucket_id); 
    }
    else
    {
#if 0
        printf("mas_red_cold_sync_dec()=======================================\n"); 
        mas_dictionary_dump(); 
#endif

        /* log the failure */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                          MAB_MAS_ERR_COLD_SYNC_DEC_BKT_NON_NULL, 
                          dec_bucket_id);  
        return NCSCC_RC_FAILURE; 
    }
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_data_rsp_enc_cb                                     * 
*                                                                            *
*  Description:   Encodes the Data Request message of MBCSv                  *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 NCS_MBCSV_CB_ENC - Place to keep the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* Active MAS sends DATA reposne to Standby MAS */
static uns32 
mas_red_data_rsp_enc_cb(MAS_TBL* inst, NCS_MBCSV_CB_ENC *enc)
{
    uns8   to_be_sent_bkt; 
    uns32  status; 
    MAS_WARM_SYNC_CNTXT *warm_sync_cntxt; 

    /* get the context information */ 
    warm_sync_cntxt = (MAS_WARM_SYNC_CNTXT *)(long)enc->io_req_context;
    if ((warm_sync_cntxt == NULL) || (warm_sync_cntxt->bkt_count == 0))
    {
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                MAB_MAS_ERR_MBCSV_WARM_SYNC_BKT_CNT_ZERO);
        return NCSCC_RC_FAILURE; 
    }

    /* get the first bucket-id to be sent, from the contxt information */ 
    to_be_sent_bkt = warm_sync_cntxt->bkt_list[enc->io_reo_hdl];

    /* encode the to be synced bucket */
    /* In the warm-sync case, Buckets with no tables needs to be synced */ 
    status = mas_red_bucket_enc(to_be_sent_bkt, inst->hash[to_be_sent_bkt], 
                               inst->red.async_count[to_be_sent_bkt], enc);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, 
                          MAB_MAS_ERR_MBCSV_DATA_RSP_ENCODE_FAILED_FOR_BKT, 
                          to_be_sent_bkt, inst->hash[to_be_sent_bkt]);  
        return status;  /* to continue on syncing for the other buckets */
    }
    
    /* if this is the last bucket, set the data response complete */ 
    /* reset all our local values */ 
    if ((enc->io_reo_hdl+1) == warm_sync_cntxt->bkt_count)
    {
        enc->io_msg_type = NCS_MBCSV_MSG_DATA_RESP_COMPLETE; 

        /* free the context information */ 
        m_MMGR_MAS_WARM_SYNC_BKT_LIST_FREE(warm_sync_cntxt->bkt_list); 
        m_MMGR_MAS_WARM_SYNC_CNTXT_FREE(warm_sync_cntxt);
        enc->io_req_context = 0; 
        enc->io_reo_hdl = 0; 
    }
    else
        enc->io_reo_hdl += 1; 
    
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_bucket_enc                                          * 
*                                                                            *
*  Description:   Encodes the Table records in a bucket                      *
*                                                                            *
*  Arguments:     bucket-id - Bucket - id to be encoded                      * 
*                 tbl_list  - List of tables to be send to Standby           *
*                 async_count - Number of updates for this bucket            *
*                 NCS_MBCSV_CB_ENC - Place to keep the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* Encode all the table-records in a bucket */
static uns32
mas_red_bucket_enc(uns8 bucket_id, MAS_ROW_REC *tbl_list, 
                  uns32  async_count, NCS_MBCSV_CB_ENC *enc)
{
    uns8        *data, *p_tbl_cnt; 
    uns32       tbl_cnt = 0;
    uns32       status; 
    NCS_UBAID   *data_uba = &enc->io_uba;
    MAS_ROW_REC *enc_me = NULL; 

    /* encode the bucket-id */ 
    data = ncs_enc_reserve_space(data_uba, sizeof(uns8));
    if (data == NULL)
       return NCSCC_RC_FAILURE;
    ncs_encode_8bit(&data, bucket_id);
    ncs_enc_claim_space(data_uba, sizeof(uns8));

    /* encode the async count of this bucket */ 
    data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
    if (data == NULL)
       return NCSCC_RC_FAILURE;
    ncs_encode_32bit(&data, async_count);
    ncs_enc_claim_space(data_uba, sizeof(uns32));

    /* reserve the space for number of tables in this bucket */
    data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
    if (data == NULL)
       return NCSCC_RC_FAILURE;
    p_tbl_cnt = data;    
    ncs_enc_claim_space(data_uba, sizeof(uns32));
   
    /* encode all the tables in this bucket */ 
    enc_me = tbl_list; 
    while (enc_me)
    {
        /* encode the table-id, service-id */ 
        data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        ncs_encode_32bit(&data, enc_me->tbl_id);
        ncs_enc_claim_space(data_uba, sizeof(uns32));

        data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        ncs_encode_32bit(&data, enc_me->ss_id);
        ncs_enc_claim_space(data_uba, sizeof(uns32));

        /* encode filter list */ 
        status = mas_red_mas_fltr_enc(enc_me->fltr_list, data_uba); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the failure */ 
            return status; 
        }

        /* encode the default filter details */ 
        data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        ncs_encode_32bit(&data, enc_me->dfltr_regd);
        ncs_enc_claim_space(data_uba, sizeof(uns32));

        status = mas_red_mas_dfltr_enc(&enc_me->dfltr, data_uba); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the failure */ 
            return status; 
        }

        /* go to the next table in the bucket */ 
        enc_me = enc_me->next; 
        tbl_cnt++; 
    } /* for all the tables in this bucket */ 
    
    ncs_encode_32bit(&p_tbl_cnt, tbl_cnt);
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_bucket_dec                                          * 
*                                                                            *
*  Description:   Decodes the Table records in a bucket                      *
*                                                                            *
*  Arguments:     bucket-id - Bucket - id to be decoded and synced           * 
*                 tbl_list  - List of tables to be synced into Standby       *
*                 async_count - To store the async counts of this bucket     *
*                 NCS_MBCSV_CB_ENC - Place to read the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* Decode all the table-records in a bucket */
static uns32 
mas_red_bucket_dec(uns8 *bucket_id, MAS_ROW_REC **tbl_list, 
                  uns32 *async_count, NCS_MBCSV_CB_DEC *dec)
{
    uns8        *data; 
    uns8        data_buff[10]; 
    uns32       tbl_cnt = 0;
    uns32       status; 
    MAS_ROW_REC *tbl_rec = NULL; 
    MAS_ROW_REC *temp_tbl = NULL; 

    /* decode the bucket-id */ 
    data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns8));
    if (data == NULL)
        return NCSCC_RC_FAILURE; 
    *bucket_id = ncs_decode_8bit(&data);
    ncs_dec_skip_space(&dec->i_uba, sizeof(uns8));

    /* decode the bucket-id */ 
    data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns32));
    if (data == NULL)
        return NCSCC_RC_FAILURE; 
    *async_count = ncs_decode_32bit(&data);
    ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));
    
    /* decode the number of tables in the bucket */
    data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns32));
    if (data == NULL)
        return NCSCC_RC_FAILURE; 
    tbl_cnt = ncs_decode_32bit(&data);
    ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

    /* set the default for tbl_cnt == 0 */ 
    *tbl_list = NULL; 
   
    /* decode all the tables in this bucket */ 
    while (tbl_cnt)
    {
        /* allocate the memory for the new table-record */ 
        tbl_rec = m_MMGR_ALLOC_MAS_ROW_REC; 
        if (tbl_rec == NULL)
        {
            /* log the failure */ 
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_ROW_REC_ALLOC_FAILED,
                               "mas_red_bucket_dec()");
            return NCSCC_RC_OUT_OF_MEM; 
        }
        m_NCS_MEMSET(tbl_rec, 0, sizeof(MAS_ROW_REC)); 
        
        /* decode the table-id, service-id */ 
        data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns32));
        if (data == NULL)
            return NCSCC_RC_FAILURE; 
        tbl_rec->tbl_id = ncs_decode_32bit(&data);
        ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

        data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns32));
        if (data == NULL)
            return NCSCC_RC_FAILURE; 
        tbl_rec->ss_id = ncs_decode_32bit(&data);
        ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

        /* decode filter list */ 
        status = mas_red_mas_fltr_dec(&tbl_rec->fltr_list, &dec->i_uba); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the failure */ 
            return status; 
        }

        /* decode the default filter details */ 
        data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns32));
        if (data == NULL)
            return NCSCC_RC_FAILURE; 
        tbl_rec->dfltr_regd = ncs_decode_32bit(&data);
        ncs_dec_skip_space(&dec->i_uba, sizeof(uns32));

        status = mas_red_mas_dfltr_dec(&tbl_rec->dfltr, &dec->i_uba); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the failure */ 
            return status; 
        }

        /* add to the list */ 
        if (*tbl_list == NULL)
            *tbl_list = tbl_rec; 
        else 
        {
            /* append at the end */ 
            temp_tbl = *tbl_list; 
            while (temp_tbl->next)
                temp_tbl = temp_tbl->next; 
            temp_tbl->next = tbl_rec; 
        }
        tbl_cnt--; 
    } /* for all the tables in this bucket */ 
    
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_mas_fltr_enc                                        * 
*                                                                            *
*  Description:   Encodes the Filter information of a Table                  *
*                                                                            *
*  Arguments:     MAS_FLTR* - List of filters to be encoded                  * 
*                 NCS_UBAID* - Place to keep the encoded buffer and          * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* encode filter list */ 
static uns32
mas_red_mas_fltr_enc(MAS_FLTR *fltr_list, NCS_UBAID *data_uba) 
{
    uns8    *data, *p_fltr_cnt; 
    uns32   status; 
    uns32   fltr_cnt = 0; 
    MAS_FLTR    *enc_me = 0; 
   
    /* reserve the space for fltr count */ 
    data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
    if (data == NULL)
       return NCSCC_RC_FAILURE;
    p_fltr_cnt = data;    
    ncs_enc_claim_space(data_uba, sizeof(uns32));

    fltr_cnt = 0; 
    enc_me = fltr_list; 
    while (enc_me)
    {
        /* encode the filter */ 
        if(mab_fltr_encode(&(enc_me->fltr),data_uba) != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE; 

        /* encode the MDS_DEST */ 
        data = ncs_enc_reserve_space(data_uba, sizeof(MDS_DEST));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        mds_st_encode_mds_dest(&data,&enc_me->vcard);
        ncs_enc_claim_space(data_uba, sizeof(MDS_DEST));
        
        /* encode fltr-ids */ 
        status = mas_red_mas_fltr_ids_enc(enc_me->fltr_ids.active_fltr, data_uba);
        if (status == NCSCC_RC_FAILURE)
        {
            /* log the failure */ 
            return status; 
        }
        status = mas_red_mas_fltr_ids_enc(enc_me->fltr_ids.fltr_id_list, data_uba);
        if (status == NCSCC_RC_FAILURE)
        {
            /* log the failure */ 
            return status; 
        }

        /* encode ref_cnt */ 
        data = ncs_enc_reserve_space(data_uba, sizeof(uns16));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        ncs_encode_16bit(&data, enc_me->ref_cnt );
        ncs_enc_claim_space(data_uba, sizeof(uns16));

        /* encode wild card */
        data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        ncs_encode_32bit(&data, enc_me->wild_card);
        ncs_enc_claim_space(data_uba, sizeof(uns32));

        /* go to the next filter */ 
        enc_me = enc_me->next; 
        fltr_cnt++; 
    } /* go to the next filter */ 

    ncs_encode_32bit(&p_fltr_cnt, fltr_cnt);
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_mas_fltr_dec                                        * 
*                                                                            *
*  Description:   Decodes the Filter information of a Table                  *
*                                                                            *
*  Arguments:     MAS_FLTR* - List of filters to be decoded into             * 
*                 NCS_UBAID* - Place to read the encoded buffer and          * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* decode the filter list */ 
static uns32
mas_red_mas_fltr_dec(MAS_FLTR **fltr_list, NCS_UBAID *data_uba) 
{
    uns8    *data; 
    uns8    data_buff[30]; 
    uns32   status; 
    uns32   fltr_cnt; 
    MAS_FLTR    *new_fltr, *temp_fltr;
    
    /* decode the number of filters */
    data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(uns32));
    if (data == NULL)
        return NCSCC_RC_FAILURE; 
    fltr_cnt = ncs_decode_32bit(&data);
    ncs_dec_skip_space(data_uba, sizeof(uns32));

    *fltr_list = NULL; 
    while (fltr_cnt)
    {
        /* allocate the new filter */ 
        new_fltr = m_MMGR_ALLOC_MAS_FLTR; 
        if (new_fltr == NULL)
        {
            /* log the failure */ 
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_FLTR_CREATE_FAILED,
                               "mas_red_mas_fltr_dec()");
            return NCSCC_RC_OUT_OF_MEM; 
        }
        m_NCS_MEMSET(new_fltr, 0, sizeof(MAS_FLTR)); 

        /* decode the filter */ 
        if(mab_fltr_decode(&(new_fltr->fltr),data_uba) != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE; 

        /* set the classify function, based on the filter type */ 
        switch (new_fltr->fltr.type)
        {
            case NCSMAB_FLTR_SAME_AS:
                new_fltr->test = mas_classify_same_as;
                break;
                                                                                                                    
            case NCSMAB_FLTR_ANY:
                new_fltr->test = mas_classify_any;
                break;
            
           case NCSMAB_FLTR_RANGE:
               new_fltr->test = mas_classify_range;
               break; 

           case NCSMAB_FLTR_DEFAULT:
               new_fltr->test = NULL;
               break;
           
           case NCSMAB_FLTR_EXACT:
               new_fltr->test = mas_classify_exact;
               break;

           default: 
                return NCSCC_RC_FAILURE; 
        }

        /* decode the MDS_DEST */ 
        data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(MDS_DEST));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        mds_st_decode_mds_dest(&data,&new_fltr->vcard);
        ncs_dec_skip_space(data_uba, sizeof(MDS_DEST));
        
        /* decode fltr-ids */ 
        status = mas_red_mas_fltr_ids_dec(&new_fltr->fltr_ids.active_fltr, data_uba);
        if (status == NCSCC_RC_FAILURE)
        {
            /* log the failure */ 
            return status; 
        }
        status = mas_red_mas_fltr_ids_dec(&new_fltr->fltr_ids.fltr_id_list, data_uba);
        if (status == NCSCC_RC_FAILURE)
        {
            /* log the failure */ 
            return status; 
        }

        /* decode ref_cnt */ 
        data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(uns16));
        if (data == NULL)
            return NCSCC_RC_FAILURE; 
        new_fltr->ref_cnt = ncs_decode_16bit(&data);
        ncs_dec_skip_space(data_uba, sizeof(uns16));

        /* decode wild card */
        data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(uns32));
        if (data == NULL)
            return NCSCC_RC_FAILURE; 
        new_fltr->wild_card = ncs_decode_32bit(&data);
        ncs_dec_skip_space(data_uba, sizeof(uns32));

        /* filter order must be maintained */ 
        /* otherwise, after switchover GETNEXT does not work properly */ 
        if (*fltr_list == NULL)
        {
            /* insert as a first node */ 
            *fltr_list = new_fltr; 
        }
        else
        {
            /* append at the end */ 
            temp_fltr = *fltr_list; 
            while (temp_fltr->next)
                temp_fltr = temp_fltr->next; 
            temp_fltr->next = new_fltr; 
        }
        fltr_cnt--; 
    } /* for all the filters of this table */ 
    return NCSCC_RC_SUCCESS; 
}
  
/****************************************************************************\
*  Name:          mas_red_mas_dfltr_enc                                       * 
*                                                                            *
*  Description:   Encode the Default Filter information of a Table           *
*                                                                            *
*  Arguments:     MAS_DFLTR* - Default Filter information of a table         * 
*                 NCS_UBAID* - Place to keep the encoded buffer and          * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* encode the default filter attributes */ 
static uns32
mas_red_mas_dfltr_enc(MAS_DFLTR *dfltr, NCS_UBAID *data_uba) 
{
    uns8    *data; 
    uns32   status; 

    /* encode the vcard */ 
    data = ncs_enc_reserve_space(data_uba, sizeof(MDS_DEST));
    if (data == NULL)
       return NCSCC_RC_FAILURE;
    mds_st_encode_mds_dest(&data,&dfltr->vcard);
    ncs_enc_claim_space(data_uba, sizeof(MDS_DEST));

    /* encode the filter-id anchor mappings */ 
    status = mas_red_mas_fltr_ids_enc(dfltr->fltr_ids.active_fltr, data_uba);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        return status; 
    }
    status = mas_red_mas_fltr_ids_enc(dfltr->fltr_ids.fltr_id_list, data_uba);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
    }
    return status; 
}

/****************************************************************************\
*  Name:          mas_red_mas_dfltr_dec                                       * 
*                                                                            *
*  Description:   Decode the Default Filter information of a Table           *
*                                                                            *
*  Arguments:     MAS_DFLTR* - Default Filter information of a table         * 
*                 NCS_UBAID* - Place to read the encoded buffer and          * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* decode the default filter attributes */ 
static uns32
mas_red_mas_dfltr_dec(MAS_DFLTR *dfltr, NCS_UBAID *data_uba) 
{
    uns8    *data;
    uns8    data_buff[30]; 
    uns32   status; 

    /*decode the vacrd */ 
    data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(MDS_DEST));
    if (data == NULL)
       return NCSCC_RC_FAILURE;
    mds_st_decode_mds_dest(&data,&dfltr->vcard);
    ncs_dec_skip_space(data_uba, sizeof(MDS_DEST));

    /* decode the flter-id and anchor mappings */
    status = mas_red_mas_fltr_ids_dec(&dfltr->fltr_ids.active_fltr, data_uba);
    if (status == NCSCC_RC_FAILURE)
    {
        /* log the failure */ 
        return status; 
    }
    status = mas_red_mas_fltr_ids_dec(&dfltr->fltr_ids.fltr_id_list, data_uba);
    if (status == NCSCC_RC_FAILURE)
    {
        /* log the failure */ 
    }
    return status; 
}

/****************************************************************************\
*  Name:          mas_red_mas_fltr_ids_enc                                    * 
*                                                                            *
*  Description:   Encode the filter-id to Anchor values mapping              *
*                                                                            *
*  Arguments:     MAB_FLTR_ANCHOR_NODE* - list of mapping(fltr-id, anchor)   * 
*                 NCS_UBAID* - Place to keep the encoded buffer and          * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* encode the filter-ids linked list */ 
static uns32
mas_red_mas_fltr_ids_enc(MAB_FLTR_ANCHOR_NODE *fltr_anc_ids, NCS_UBAID *data_uba)
{
    uns8    *data, *p_anc_cnt; 
    uns32   anc_count; 
    MAB_FLTR_ANCHOR_NODE  *enc_me; 

    /* reserve space for the count */ 
    data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
    if (data == NULL)
       return NCSCC_RC_FAILURE;
    p_anc_cnt = data;    
    ncs_enc_claim_space(data_uba, sizeof(uns32));
    anc_count = 0; 

    enc_me = fltr_anc_ids; 
    while (enc_me)
    {
        /* encode the anchor value */ 
        data = ncs_enc_reserve_space(data_uba, sizeof(MAB_ANCHOR));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        mds_st_encode_mds_dest(&data,&enc_me->anchor);
        ncs_enc_claim_space(data_uba, sizeof(MAB_ANCHOR));

        /* encode the filter-id */ 
        data = ncs_enc_reserve_space(data_uba, sizeof(uns32));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        ncs_encode_32bit(&data, enc_me->fltr_id);
        ncs_enc_claim_space(data_uba, sizeof(uns32));

        /* go to the next association */ 
        enc_me = enc_me->next; 
        anc_count++; 
    } /* for all the mappings */
    ncs_encode_32bit(&p_anc_cnt, anc_count);
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_mas_fltr_ids_dec                                    * 
*                                                                            *
*  Description:   Decode the filter-id to Anchor values mapping              *
*                                                                            *
*  Arguments:     MAB_FLTR_ANCHOR_NODE** - list of mapping(fltr-id, anchor)  * 
*                 NCS_UBAID* - Place to read the encoded buffer and          * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* decode the filter-ids linked list */ 
static uns32
mas_red_mas_fltr_ids_dec(MAB_FLTR_ANCHOR_NODE **fltr_ids, NCS_UBAID *data_uba)
{
    uns8    *data; 
    uns8    data_buff[30]; 
    uns32   anc_count; 
    MAB_FLTR_ANCHOR_NODE  *new_mapping; 
    MAB_FLTR_ANCHOR_NODE  *last = NULL;  

    /* decode the number of mappings */ 
    data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(uns32));
    if (data == NULL)
        return NCSCC_RC_FAILURE; 
    anc_count = ncs_decode_32bit(&data);
    ncs_dec_skip_space(data_uba, sizeof(uns32));

    *fltr_ids = NULL; 
    while (anc_count)
    {
        /* allocate the memory for the new mapping */ 
        new_mapping = m_MMGR_ALLOC_FLTR_ANCHOR_NODE;
        if (new_mapping == NULL)
        {
            /* log the error */ 
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_MAB_FLTR_ANCHOR_NODE_ALLOC_FAILED,
                               "mas_red_mas_fltr_ids_dec()");
            return NCSCC_RC_OUT_OF_MEM; 
        }
        m_NCS_MEMSET(new_mapping, 0, sizeof(MAB_FLTR_ANCHOR_NODE));

        /* decode the anchor value */ 
        data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(MAB_ANCHOR));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        mds_st_decode_mds_dest(&data,&new_mapping->anchor);
        ncs_dec_skip_space(data_uba, sizeof(MAB_ANCHOR));

        /* encode the filter-id */ 
        data = ncs_dec_flatten_space(data_uba, data_buff, sizeof(uns32));
        if (data == NULL)
            return NCSCC_RC_FAILURE; 
        new_mapping->fltr_id = ncs_decode_32bit(&data);
        ncs_dec_skip_space(data_uba, sizeof(uns32));

        /* add to the list at the end */ 
        if (*fltr_ids == NULL)
        {
            /* first node */ 
            *fltr_ids = new_mapping; 
            last = new_mapping; 
        }
        else
        {
            /* insert at the end */ 
            last->next = new_mapping; 
            last = new_mapping; 
        }
            
        /* go to the next association */ 
        anc_count--; 
    } /* for all the mappings */

    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_async_count_enc                                     * 
*                                                                            *
*  Description:   Encodes the Async Count of all the buckets                 *
*                                                                            *
*  Arguments:     uns32* - Array of async counts  of all the buckets         *
*                 NCS_UBAID * - Place to keep the encoded buffer andi        * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
static uns32
mas_red_async_count_enc(uns32  *async_count, NCS_UBAID *uba)
{
    uns8    *data;
    uns32    bucket = 0; 
    NCSFL_MEM idx;

    /* log the async count at the ACTIVE MAS */ 
    m_NCS_MEMSET(&idx, 0, sizeof(NCSFL_MEM));
    idx.len = MAB_MIB_ID_HASH_TBL_SIZE*sizeof(uns32);
    idx.addr = idx.dump = (char*)async_count;
    m_LOG_MAB_MEM(NCSFL_SEV_INFO, MAB_HDLN_MAS_ASYNC_COUNT_ENC, MAB_MIB_ID_HASH_TBL_SIZE, idx);

    /* allocate space for the user buf */ 
       
    /* encode the async_count of each bucket */ 
    for (bucket = 0; bucket < MAB_MIB_ID_HASH_TBL_SIZE; bucket ++)
    {
        /* encode the async count */ 
        data = ncs_enc_reserve_space(uba, sizeof(uns32));
        if (data == NULL)
           return NCSCC_RC_FAILURE;
        ncs_encode_32bit(&data, *async_count);
        ncs_enc_claim_space(uba, sizeof(uns32));

        /* go to the next async count */ 
        async_count++; 
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          mas_red_warm_sync_resp_dec                                  * 
*                                                                            *
*  Description:   Decodes the warm sync response                             *
*                                                                            *
*  Arguments:     MAS_TBL* - to store the warm-sync response                 *
*                 NCS_MBCSV_CB_DEC * - Place to read the encoded buffer and  * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
static uns32
mas_red_warm_sync_resp_dec(MAS_TBL *inst, NCS_MBCSV_CB_DEC  *dec)
{
    uns8    bkt_count = 0; 
    uns32    bucket;
    uns8    *data;
    uns8    *p_bkt_count;
    
    uns32   status;
    uns32   peer_async_count[MAB_MIB_ID_HASH_TBL_SIZE] = {0};

    NCSFL_MEM       idx;
    NCS_UBAID       *data_uba;
    NCS_MBCSV_ARG   mbcsv_arg;


    /* decode the async counts of the peer */ 
    status = mas_red_async_count_dec(peer_async_count, &dec->i_uba); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        return status; 
    }

    /* log the async counts of STANDBY MAS */ 
    m_NCS_MEMSET(&idx, 0, sizeof(NCSFL_MEM));
    idx.len = MAB_MIB_ID_HASH_TBL_SIZE*sizeof(uns32);
    idx.addr = idx.dump = (char*)&(inst->red.async_count);
    m_LOG_MAB_MEM(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_ASYNC_COUNT_SBY, MAB_MIB_ID_HASH_TBL_SIZE, idx);
   
    m_NCS_MEMSET(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG)); 
    mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_DATA_REQ;
    mbcsv_arg.i_mbcsv_hdl = gl_mas_amf_attribs.mbcsv_attribs.mbcsv_hdl;
    mbcsv_arg.info.send_data_req.i_ckpt_hdl = inst->red.ckpt_hdl;
    ncs_enc_init_space(&mbcsv_arg.info.send_data_req.i_uba);
    data_uba = &mbcsv_arg.info.send_data_req.i_uba;

    /* reserve the space for number of buckets */
    if((p_bkt_count = ncs_enc_reserve_space(data_uba, sizeof(uns8))) == NULL)
    {
        /* LOG TBD */
        return NCSCC_RC_FAILURE;
    }
    ncs_enc_claim_space(data_uba, sizeof(uns8));
    

    /* make a list of mismatching asyncs */ 
    for (bucket=0; bucket<MAB_MIB_ID_HASH_TBL_SIZE; bucket++) 
    {
#if 0
        printf("Asyn-cnt for bkt: [%d]: From Active: %d: In SBY: %d\n", bucket, peer_async_count[bucket], inst->red.async_count[bucket]);
        fflush(stdout); 
#endif
        if (peer_async_count[bucket] != inst->red.async_count[bucket])
        {
            /* increment the number buckets to be synced */
            bkt_count++; 
            
            /* encode the bucket number */
            data = ncs_enc_reserve_space(data_uba, sizeof(uns8));
            if (data == NULL)
               return NCSCC_RC_FAILURE;
            ncs_encode_8bit(&data, (uns8)bucket);
            ncs_enc_claim_space(data_uba, sizeof(uns8));

            /* log the bucket which is out of sync */ 
            m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_SBY_OUT_OF_SYNC, bucket); 
        }
    }

    if (bkt_count == 0)
    {
        /* standby is in sync, no need to send data req*/
        m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_SBY_IN_SYNC);
        m_MMGR_FREE_BUFR_LIST(data_uba->start);  
        return NCSCC_RC_SUCCESS;
    }

    /* encode the bucket count */
    ncs_encode_8bit(&p_bkt_count,bkt_count);

    /* standby is out of sync; does a data request */
    status = ncs_mbcsv_svc(&mbcsv_arg); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the return code */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_API_DATA_REQ_FAILED, 
                           bkt_count, status); 
        return status; 
    }

    /* Note that SBY is out of sync, and not ready for a state change at this point
     * of time. 
     */ 
    inst->red.warm_sync_in_progress = TRUE;  
    
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          mas_red_data_resp_dec_cb                                    * 
*                                                                            *
*  Description:   decodes the Data response of Active MAS                    *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 NCS_MBCSV_CB_DEC - Place to read the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* Standby MAS received the data response */    
static uns32
mas_red_data_resp_dec_cb(MAS_TBL *inst, NCS_MBCSV_CB_DEC *dec)
{
    uns8    dec_bucket_id; 
    uns32   status; 
    uns32   async_count; 
    MAS_ROW_REC *tbl_rec; 

    /* decode the response */ 
    status = mas_red_bucket_dec(&dec_bucket_id, &tbl_rec, &async_count, dec);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        return status; 
    }

    /* free the existing records */ 
    mas_table_rec_list_free(inst->hash[dec_bucket_id]); 

    /* add to the hash table */ 
    inst->hash[dec_bucket_id] = tbl_rec; 

    /* update the async count of this bucket */ 
    inst->red.async_count[dec_bucket_id] = async_count; 

    if (dec->i_msg_type == NCS_MBCSV_MSG_DATA_RESP_COMPLETE)
    {
        /* update that standby is in sync */ 
        inst->red.warm_sync_in_progress = FALSE;  

        /* log a message that warm-sync is complete */ 
        m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_MBCSV_DATARSP_CMPLT);
    }

    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_async_count_dec                                     * 
*                                                                            *
*  Description:   Standby MAS Decodes the async counts of each bucket        *
*                                                                            *
*  Arguments:     uns32* - Array to store the async counts                   * 
*                 NCS_UBAID* - Place to read the encoded buffer and          * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* to decode the async count array */ 
static uns32
mas_red_async_count_dec(uns32  *async_count, NCS_UBAID *uba)
{
    uns8*   data;
    /* temporary store, aid in decoding the async counts array */
    uns8    data_buff[MAB_MIB_ID_HASH_TBL_SIZE];
    uns32   bucket = 0; 
    uns32   *tmp; 
    NCSFL_MEM idx;

       
    /* decode the async_count of each bucket */ 
    tmp = async_count; 
    for (bucket = 0; bucket < MAB_MIB_ID_HASH_TBL_SIZE; bucket ++)
    {
        /* decode the async count */ 
        data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
        if(data == NULL)
           return NCSCC_RC_FAILURE;
        *tmp = ncs_decode_32bit(&data);
        ncs_dec_skip_space(uba, sizeof(uns32));

        /* go to the next bucket */ 
        tmp++; 
    }
    
    /* log the decoded async count at the Standby MAS */ 
    m_NCS_MEMSET(&idx, 0, sizeof(NCSFL_MEM));
    idx.len = MAB_MIB_ID_HASH_TBL_SIZE*sizeof(uns32);
    idx.addr = idx.dump = (char*)async_count;
    m_LOG_MAB_MEM(NCSFL_SEV_INFO, MAB_HDLN_MAS_ASYNC_COUNT_DEC, MAB_MIB_ID_HASH_TBL_SIZE, idx);
    
    return NCSCC_RC_SUCCESS;
}   

/****************************************************************************\
*  Name:          mas_red_data_req_dec                                        * 
*                                                                            *
*  Description:   Active MAS decodes the data request of Standby MAS         *
*                                                                            *
*  Arguments:     NCS_MBCSV_CB_DEC - Place to read the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* Active MAS decodes the Datareq and sets the context for Data Resp */
static uns32
mas_red_data_req_dec(NCS_MBCSV_CB_DEC *dec)
{
    uns8    i, out_of_sync_buckets;
    uns8    *data, *bkt_list;
    /* temporary memory, aids in decoding the user buffer */ 
    uns8    data_buff[MAB_MIB_ID_HASH_TBL_SIZE];
    MAS_WARM_SYNC_CNTXT *warm_sync_cntxt; 

    /* decode the list of async buckets of Standby MAS */ 
    data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns8));
    if (data == NULL)
        return NCSCC_RC_FAILURE; 
    out_of_sync_buckets = ncs_decode_8bit(&data);
    ncs_dec_skip_space(&dec->i_uba, sizeof(uns8));

    if (out_of_sync_buckets == 0)
    {
        /* something went wrong */ 
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                MAB_MAS_ERR_MBCSV_DATA_REQ_DEC_BKTCNT_ZERO);
        return NCSCC_RC_FAILURE; 
    }

    /* allocate memory for context */ 
    warm_sync_cntxt = m_MMGR_MAS_WARM_SYNC_CNTXT_ALLOC;
    if (warm_sync_cntxt == NULL)
    {
        /* log the error */ 
        m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_WARM_SYNC_CNTXT_ALLOC_FAILED,
                           "mas_red_data_req_dec()");
        return NCSCC_RC_OUT_OF_MEM; 
    }
    m_NCS_MEMSET(warm_sync_cntxt, 0, sizeof(MAS_WARM_SYNC_CNTXT));
    /* update the number of buckets to be synced */
    warm_sync_cntxt->bkt_count = out_of_sync_buckets;
        
    /* allocate the memory for the list */
    warm_sync_cntxt->bkt_list = m_MMGR_MAS_WARM_SYNC_BKT_LIST_ALLOC(out_of_sync_buckets); 
    if (warm_sync_cntxt->bkt_list == NULL)
    {
        /* log the failure */ 
        m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_WARM_SYNC_BKT_LIST_ALLOC_FAILED,
                           "mas_red_data_req_dec()");
        return NCSCC_RC_OUT_OF_MEM; 
    }
    m_NCS_MEMSET(warm_sync_cntxt->bkt_list, 0, out_of_sync_buckets); 

    /* copy the list of buckets */ 
    bkt_list = warm_sync_cntxt->bkt_list;
    for (i=0; i<out_of_sync_buckets; i++)
    {
        data = ncs_dec_flatten_space(&dec->i_uba, data_buff, sizeof(uns8));
        if (data == NULL)
            return NCSCC_RC_FAILURE; 
        *bkt_list = ncs_decode_8bit(&data);
        ncs_dec_skip_space(&dec->i_uba, sizeof(uns8));

        /* before incrementing the pointer, check for the memory ownership */ 
        if ((i+1) < out_of_sync_buckets)
            bkt_list++;
    }

    /* update the context info to build the data response */
    dec->o_req_context = NCS_PTR_TO_UNS64_CAST(warm_sync_cntxt); 

    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_red_sync_done                                           * 
*                                                                            *
*  Description:   Send the sync done message to the Standby MAS              *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 MAS_MBCSV_ATTRIBS - type of data being sent                * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* send the async done message to the standby MAS */
uns32 
mas_red_sync_done(MAS_TBL* inst, MAS_ASYNC_UPDATE_TYPE type)
{
    uns32   status;
    MAB_MSG *new_msg; 
    
    new_msg =  m_MMGR_ALLOC_MAB_MSG; 
    if (new_msg == NULL)
    {
        /* log the malloc failure */ 
        m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MABMSG_CREATE,
                           "mas_red_sync_done()");
        return NCSCC_RC_OUT_OF_MEM; 
    }
        
    m_NCS_MEMSET(new_msg, 0, sizeof(MAB_MSG));     
    new_msg->op = MAB_MAS_ASYNC_DONE; 
    status = mas_red_sync(inst, new_msg, type); 
    
    m_MMGR_FREE_MAB_MSG(new_msg); 

    return status;
}
    
/****************************************************************************\
*  Name:          mas_red_sync                                                * 
*                                                                            *
*  Description:   Send the sync message to the Standby MAS                   *
*                                                                            *
*  Arguments:     MAS_TBL* - Information about MASv                          * 
*                 MAB_MSG* - Message to be sent to Standby MAS               *
*                 MAS_MBCSV_ATTRIBS - type of data being sent                * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* send the event to Stand MAS */ 
uns32 
mas_red_sync(MAS_TBL* inst, MAB_MSG* msg, MAS_ASYNC_UPDATE_TYPE type)
{
    uns32           status;
    NCS_MBCSV_ARG   mbcsv_arg;
    
    m_MAB_DBG_TRACE("\nmas_red_sync():entered."); 
    
    m_NCS_MEMSET(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_CKPT;
    mbcsv_arg.i_mbcsv_hdl = inst->red.mbcsv_hdl;
    mbcsv_arg.info.send_ckpt.i_action = NCS_MBCSV_ACT_UPDATE;
    mbcsv_arg.info.send_ckpt.i_ckpt_hdl = inst->red.ckpt_hdl;
    mbcsv_arg.info.send_ckpt.i_reo_hdl  = (MBCSV_REO_HDL)(long)msg;
    /* set the type of data */
    mbcsv_arg.info.send_ckpt.i_reo_type = (uns32)type;
    mbcsv_arg.info.send_ckpt.i_send_type = NCS_MBCSV_SND_USR_ASYNC; 
    status = ncs_mbcsv_svc(&mbcsv_arg); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_API_SND_USR_ASYNC_FAILED, 
                           type, status); 
        return status; 
    } 

    /* log the event of successful sending the update */ 
    m_LOG_MAB_HDLN_II(NCSFL_SEV_INFO, MAB_HDLN_MAS_MBCSV_ASYNC_UPDT_SUCCESS, 
                      type, status); 
    
    m_MAB_DBG_TRACE("\nmas_red_sync():left.");
    return status;
}

/****************************************************************************\
*  Name:          mas_red_updt_enc                                            * 
*                                                                            *
*  Description:   Encodes the Async update to be sent to Standby MAS         *
*                                                                            *
*  Arguments:     NCS_MBCSV_CB_ENC - Place to keep the encoded buffer and    * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* Encode the Async update to be sent to Standby MAS */ 
static uns32 
mas_red_updt_enc(NCS_MBCSV_CB_ENC* enc)
{
    uns32           status;
    MAB_MSG         *msg; 
    uns16        mas_mbcsv_msg_fmt_ver;

    m_MAB_DBG_TRACE("\nmas_red_updt_enc():entered.");

    msg = (MAB_MSG*)(long)enc->io_reo_hdl;
    if (msg == NULL)
    {
        /* log the failure */ 
        return NCSCC_RC_FAILURE; 
    }

    /* find msg fmt ver on mbcsv inteface to be used between active & stdby mas */
    mas_mbcsv_msg_fmt_ver = m_NCS_MBCSV_FMT_GET(enc->i_peer_version,
                                           m_NCS_MAS_MBCSV_VERSION,
                                           m_NCS_MAS_MBCSV_VERSION_MIN);
    if(mas_mbcsv_msg_fmt_ver == 0)
    {
           /* log the error */
           m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE,MAB_HDLN_MAS_MBCSV_ENC_FAILURE,enc->i_peer_version);
           return NCSCC_RC_FAILURE;
    }
    /* encode the message */ 
    status = mab_mds_enc(0, msg, 0, &enc->io_uba, mas_mbcsv_msg_fmt_ver); 

#if 0
    /* cleanup the indices in the Filter registration request */
    if(msg->op == MAB_MAS_REG_HDLR) 
        mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);

    /* free the message */ 
    m_MMGR_FREE_MAB_MSG(msg);
#endif

    m_MAB_DBG_TRACE("\nmas_red_updt_enc():left.");
    return status;
}

/****************************************************************************\
*  Name:          mas_red_updt_dec                                            * 
*                                                                            *
*  Description:   Encodes the Async update to be sent to Standby MAS         *
*                                                                            *
*  Arguments:     MAS_TBL * - to store the effect of processing the async    * 
*                             update                                         * 
*                 NCS_MBCSV_CB_ENC*  - Place to read the encoded buffer and  * 
*                                    other input parameters                  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
/* Decode the async update of Active MAS at Standby */
static uns32 
mas_red_updt_dec(MAS_TBL* inst, NCS_MBCSV_CB_DEC* dec)
{
    MAB_MSG     *pmsg;
    uns16 mas_mbcsv_msg_fmt_ver;

    m_MAB_DBG_TRACE("\nmas_red_updt_dec():entered.");

    /* find msg fmt ver on mbcsv interface to be used between active & stdby mas */
    mas_mbcsv_msg_fmt_ver = m_NCS_MBCSV_FMT_GET(dec->i_peer_version,
                                           m_NCS_MAS_MBCSV_VERSION,
                                           m_NCS_MAS_MBCSV_VERSION_MIN);
    if(mas_mbcsv_msg_fmt_ver == 0)
    {
           /* log the error */
           m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE,MAB_HDLN_MAS_MBCSV_DEC_FAILURE,dec->i_peer_version);
           return NCSCC_RC_FAILURE;
    }
    /* decode the message */ 
    if(mab_mds_dec(0, (NCSCONTEXT *)&pmsg, 0, &dec->i_uba, mas_mbcsv_msg_fmt_ver) 
        != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        return NCSCC_RC_FAILURE;
    }

    /* see if this is async done message */ 
    if (pmsg->op == MAB_MAS_ASYNC_DONE)
    {
        /* process the previous message */
        if (inst->red.process_msg != NULL)
        {
            mas_red_process_mab_msg(inst, dec->i_reo_type);
        }
        else  /* there is no previous message */ 
        {
            /* something went wrong */ 
            /* log the failure */ 
            /* time to initiate a warm-sync?? */ 
            return NCSCC_RC_SUCCESS; 
        }

        /* free the current message */  
        m_MMGR_FREE_MAB_MSG(pmsg); 
    }
    else /* buffer the message */ 
    {
        if (inst->red.process_msg != NULL)
            m_MMGR_FREE_MAB_MSG(inst->red.process_msg); 
            
        /* this message will be freed in the next cycle */
        pmsg->yr_hdl = NCS_INT32_TO_PTR_CAST(inst->hm_hdl);
        inst->red.process_msg = pmsg; 
    }

    m_MAB_DBG_TRACE("\nmas_red_updt_dec():left.");
    return NCSCC_RC_SUCCESS; 
}

static uns32 
mas_red_process_mab_msg(MAS_TBL *inst, MAS_ASYNC_UPDATE_TYPE async_type)
{
    uns32   bucket_id; 

    /* based on the type of the message */ 
    switch(async_type)
    {
        case MAS_ASYNC_REG_UNREG:
            /* Cold sync is not yet complete */
            if (inst->red.cold_sync_done == FALSE)
            {
                /* to get the bucket-id */
                switch(inst->red.process_msg->op)
                {
                    case MAB_MAS_REG_HDLR:
                    bucket_id = inst->red.process_msg->data.data.reg.tbl_id%MAB_MIB_ID_HASH_TBL_SIZE; 
                    break; 
                    case MAB_MAS_UNREG_HDLR:
                    bucket_id = inst->red.process_msg->data.data.unreg.tbl_id%MAB_MIB_ID_HASH_TBL_SIZE; 
                    break; 
                    default:
                        return NCSCC_RC_FAILURE;
                    break; 
                }
                /* find out whether this bucket is synced or not? */
                if (inst->red.this_bucket_synced[bucket_id] == FALSE)
                {
                    /* free the message and break from the switch */ 
                    m_MMGR_FREE_MAB_MSG(inst->red.process_msg); 
                    break; 
                }
            }

            /* purposeful fall through to process the message, if the cold-sync is completed */ 
                
        case MAS_ASYNC_OAA_DOWN: 
            /* process the message */ 
            mas_do_evt(inst->red.process_msg);
            break;
        case MAS_ASYNC_MIB_ARG:
            /* do not process the NCSMIB_ARG request */ 
            ncsmib_arg_free_resources(inst->red.process_msg->data.data.snmp,
                  (m_NCSMIB_ISIT_A_REQ(inst->red.process_msg->data.data.snmp->i_op)?TRUE:FALSE));
            m_MMGR_FREE_MAB_MSG(inst->red.process_msg); 
            break; 
        default:
            break; 
    }
    inst->red.process_msg = NULL; 
    return NCSCC_RC_SUCCESS; 
}

/*******************************************************************************\
*  PROCEDURE NAME:    mas_mbcsv_interface_initialize                            *
*  DESCRIPTION:       Initialize the interface with MBCSv                       *
*  RETURNS:           SUCCESS - all went well.                                  *
*                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()  *
*                               for details.                                    *
\*******************************************************************************/
uns32 
mas_mbcsv_interface_initialize(MAS_MBCSV_ATTRIBS *mbcsv_attribs)
{
    uns32           status; 
    NCS_MBCSV_ARG   mbcsv_arg; 

    /* validate the input */
    if (mbcsv_attribs == NULL)
        return NCSCC_RC_FAILURE; 

    /* initialize the interface with MBCSv */ 
    m_NCS_MEMSET(&mbcsv_arg, 0, sizeof(NCS_MBCSV_ARG)); 
    mbcsv_arg.i_op = NCS_MBCSV_OP_INITIALIZE; 
    mbcsv_arg.info.initialize.i_mbcsv_cb = mas_mbcsv_cb; 
    mbcsv_arg.info.initialize.i_version = mbcsv_attribs->masv_version; 
    mbcsv_arg.info.initialize.i_service = NCS_SERVICE_ID_MAB; 
    status = ncs_mbcsv_svc(&mbcsv_arg); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_INIT_FAILED, 
                           NCS_SERVICE_ID_MAB, status); 
        return status; 
    }
    /* get the MBCSv handle */ 
    mbcsv_attribs->mbcsv_hdl = mbcsv_arg.info.initialize.o_mbcsv_hdl; 

    /* get the selection object */ 
    m_NCS_MEMSET(&mbcsv_arg, 0, sizeof(NCS_MBCSV_ARG)); 
    mbcsv_arg.i_op = NCS_MBCSV_OP_SEL_OBJ_GET; 
    mbcsv_arg.i_mbcsv_hdl = mbcsv_attribs->mbcsv_hdl; 
    status = ncs_mbcsv_svc(&mbcsv_arg); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_SEL_OBJ_GET_FAILED, 
                           mbcsv_attribs->mbcsv_hdl, status); 

        /* Mahesh TBD == Check for the TRY_AGAIN error code */ 

        /* finalize the interface with MBCSv */ 
        m_NCS_MEMSET(&mbcsv_arg, 0, sizeof(NCS_MBCSV_ARG)); 
        mbcsv_arg.i_op = NCS_MBCSV_OP_FINALIZE; 
        mbcsv_arg.i_mbcsv_hdl = mbcsv_attribs->mbcsv_hdl; 
        status = ncs_mbcsv_svc(&mbcsv_arg); 
        mbcsv_attribs->mbcsv_hdl = 0; 
        return status; 
    }

    /* update the selection object */ 
    mbcsv_attribs->mbcsv_sel_obj = mbcsv_arg.info.sel_obj_get.o_select_obj;

    /* log the MBCSv handle and selection object details */ 
    m_LOG_MAB_HDLN_II(NCSFL_SEV_INFO, MAB_HDLN_MAS_MBCSV_INTF_INIT_SUCCESS, 
                      mbcsv_attribs->mbcsv_hdl, mbcsv_attribs->mbcsv_sel_obj); 

    return status; 
}

/****************************************************************************\
*  Name:          mas_red_err_ind_handle                                     * 
*                                                                            *
*  Description:   Handles the error indications given by MBCSv Interface     * 
*                                                                            *
*  Arguments:     MAS_TBL *inst - MAS control block for this PWE             *
*                 NCS_MBCSV_CB_ARG *arg - Error information                  * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Handled the error properly              *  
\****************************************************************************/
static uns32
mas_red_err_ind_handle(MAS_TBL *inst, NCS_MBCSV_CB_ARG *arg) 
{
    SaAisErrorT    saf_status = SA_AIS_OK; 
    
    m_MAS_LK(&inst->lock);

    /* if this error indication happens in the Active MAS, do nothing */
    if (inst->red.ha_state == SA_AMF_HA_ACTIVE)
    {
        /* ignore the errors in ACTIVE State */ 
        m_MAS_UNLK(&inst->lock);
        return NCSCC_RC_SUCCESS; 
    }
   
    /* process the error code in the standby MAS */ 
    if (inst->red.ha_state == SA_AMF_HA_STANDBY)
    {
        /* based on the error indication, take action */ 
        switch (arg->info.error.i_code)
        {
            /* Cold Sync timer expired without getting response */
            case NCS_MBCSV_COLD_SYNC_TMR_EXP: 
            
            /* Cold Sync complete timer expired */
            case NCS_MBCSV_COLD_SYNC_CMPL_TMR_EXP: 
            
                /* 
                 * anyway,  cold sync encode will be called by MBCSv after this error condition,
                 * at that moment, all the required data in the CB is reset.  
                 * So, no point in doing the same here. 
                 */
                 
                /* expected fall through, since the same error handling to be done */ 

            /* Data Response complete timer expired */
            case NCS_MBCSV_DATA_RSP_CMPLT_TMR_EXP: 

                /* if the big boss is available, escalate the problem */ 
                if (gl_mas_amf_attribs.amf_attribs.amfHandle != 0)
                {
                    saf_status = saAmfComponentErrorReport(gl_mas_amf_attribs.amf_attribs.amfHandle,
                                              &gl_mas_amf_attribs.amf_attribs.compName,
                                              0, /* errorDetectionTime; AvNd will provide this value */
                                              SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);

                    /* log a notify headline, which says that an error report has been sent */ 
                    m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE, MAB_HDLN_MBCSV_ERR_IND_AMF_ERR_REPORT, 
                                      arg->info.error.i_code, saf_status);

                }
                if (saf_status != SA_AIS_OK)
                {
                    m_MAS_UNLK(&inst->lock);
                    return NCSCC_RC_FAILURE; 
                }
            break; 

            /* Warm Sync timer expired without getting response */
            case NCS_MBCSV_WARM_SYNC_TMR_EXP: 

                /* expected fall through, since the same error handling to be done */ 
                
            /* Warm Sync complete timer expired */
            case NCS_MBCSV_WARM_SYNC_CMPL_TMR_EXP:
                /* expected fall through, since the same error handling to be done */ 

            default: 
            break; 
        } /* end of switch */ 
    } /* only if standby */ 

    m_MAS_UNLK(&inst->lock);
    return NCSCC_RC_SUCCESS; 
}

#if (NCS_MAS_RED_UNIT_TEST == 1)
uns32 verify_async_count_enc_dec()
{
    uns32   count_array[MAB_MIB_ID_HASH_TBL_SIZE] = {0};
    uns32   d_array[MAB_MIB_ID_HASH_TBL_SIZE] = {1}; 
    uns32   rc; 
    NCS_UBAID o_uba; 
    USRBUF  *pp_ubuf = NULL;
    int i = 0; 
   
    /* encode all zeros */ 
    memset(&o_uba, 0, sizeof(NCS_UBAID)); 
    o_uba.ub = pp_ubuf; 
    ncs_enc_init_space(&o_uba);
    rc = mas_red_async_count_enc(count_array, &o_uba); 

    ncs_dec_init_space(&o_uba, o_uba.start);
    rc = mas_red_async_count_dec(d_array, &o_uba); 
    if ((memcmp(count_array, d_array, sizeof(uns32)*MAB_MIB_ID_HASH_TBL_SIZE)) != 0)
    {
        printf ("Async Count ZERO for all buckets failed\n"); 
    }
    else
    {
        printf ("Async Count ZERO for all buckets SUCCESS\n"); 
    }

    for (i=0; i<MAB_MIB_ID_HASH_TBL_SIZE; i++)
    {
        count_array[i] = i*1000;
        d_array[i]=0;
    }
    
    memset(&o_uba, 0, sizeof(NCS_UBAID)); 
    o_uba.ub = pp_ubuf; 
    ncs_enc_init_space(&o_uba);
    rc = mas_red_async_count_enc(count_array, &o_uba); 

    ncs_dec_init_space(&o_uba, o_uba.start);
    rc = mas_red_async_count_dec(d_array, &o_uba); 
    if ((memcmp(count_array, d_array, sizeof(uns32)*MAB_MIB_ID_HASH_TBL_SIZE)) != 0)
    {
        printf ("Async Count for non ZERO for all buckets failed\n"); 
    }
    else
    {
        printf ("Async Count ifor non ZERO for all buckets SUCCESS\n"); 
    }

    for (i=0; i<MAB_MIB_ID_HASH_TBL_SIZE; i++)
    {
        count_array[i] = 0xffffffff;
        d_array[i]=0;
    }
    memset(&o_uba, 0, sizeof(NCS_UBAID)); 
    o_uba.ub = pp_ubuf; 
    ncs_enc_init_space(&o_uba);
    rc = mas_red_async_count_enc(count_array, &o_uba); 

    ncs_dec_init_space(&o_uba, o_uba.start);
    rc = mas_red_async_count_dec(d_array, &o_uba); 
    if ((memcmp(count_array, d_array, sizeof(uns32)*MAB_MIB_ID_HASH_TBL_SIZE)) != 0)
    {
        printf ("Async Count for 0xffffffff for all buckets failed\n"); 
    }
    else
    {
        printf ("Async Count for 0xffffffff for all buckets SUCCESS\n"); 
    }

    return rc; 
}

uns32 verify_mas_fltr_ids_enc_dec()
{
    uns32   rc; 
    NCS_UBAID o_uba; 
    USRBUF  *pp_ubuf = NULL;
    MAB_FLTR_ANCHOR_NODE *start = NULL; 
    MAB_FLTR_ANCHOR_NODE *tmp = NULL; 
    MAB_FLTR_ANCHOR_NODE *dec_list = NULL; 
    MAB_FLTR_ANCHOR_NODE *list, *d_list; 

    /* build the filter id list */ 
    start = m_MMGR_ALLOC_FLTR_ANCHOR_NODE;
    memset(start, 0, sizeof(MAB_FLTR_ANCHOR_NODE));
    start->fltr_id = 1223456; 
    start->anchor = 444221223456; 

    tmp = m_MMGR_ALLOC_FLTR_ANCHOR_NODE;
    memset(tmp, 0, sizeof(MAB_FLTR_ANCHOR_NODE));
    tmp->fltr_id = 6543216; 
    tmp->anchor =666666543216; 
    start->next = tmp; 
    
    tmp = m_MMGR_ALLOC_FLTR_ANCHOR_NODE;
    memset(tmp, 0, sizeof(MAB_FLTR_ANCHOR_NODE));
    tmp->fltr_id = 33334444;
    tmp->anchor = 8888833334444;
    start->next->next = tmp; 

    /* encode */ 
    memset(&o_uba, 0, sizeof(NCS_UBAID)); 
    o_uba.ub = pp_ubuf; 
    ncs_enc_init_space(&o_uba);
    rc = mas_red_mas_fltr_ids_enc(start, &o_uba); 

    /* decode */ 
    ncs_dec_init_space(&o_uba, o_uba.start);
    rc = mas_red_mas_fltr_ids_dec(&dec_list, &o_uba); 

    /* compare the decoded list */ 
    list = start; 
    d_list = dec_list; 
    while ((list)&&(d_list))
    {
        if ((list->fltr_id != d_list->fltr_id) && (list->anchor != d_list->anchor))
            break; 
        list = list->next; 
        d_list = d_list->next; 
    }
    if ((list == NULL) && (d_list == NULL))
    {
        printf("MAS_FLTR_IDs encode/decode verification SUCCESS ...\n"); 
    }
    else
    {
        printf("MAS_FLTR_IDs encode/decode verification failed...\n"); 
    }

    return rc; 
}
#endif /* #if (NCS_MAS_RED_UNIT_TEST == 1) */ 

/****************************************************************************\
*  Name:          mas_dictionary_dump                                        * 
*                                                                            *
*  Description:   to print the MAS data structures into a file in /tmp       *
*                                                                            *
*  Arguments:     Nothing                                                    *
*                                                                            * 
*  Returns:       Nothing                                                    *  
*  NOTE:                                                                     * 
\****************************************************************************/
void mas_dictionary_dump()
{
    MAS_CSI_NODE    *this_pwe=gl_mas_amf_attribs.csi_list; 
    MAS_ROW_REC     *tbl_rec = NULL; 
    uns32           bucket;
    time_t          tod; 
    FILE            *fp; 
    int8            asc_tod[70]={0};
    int8            tmp_file[90]={0};

    /* compose the temporary file name */ 
    m_GET_ASCII_DATE_TIME_STAMP(tod, asc_tod); 

    /* get the final name to be used */ 
    strcpy(tmp_file, "/tmp/MAS_DUMP_"); 
    strcat(tmp_file, asc_tod); 
    
    /* open the file in /tmp directory */ 
    fp = fopen(tmp_file, "w"); 
    if (fp == NULL)
        return; 
    
    /* for all the PWEs in the MAS */ 
    while (this_pwe)
    {
        fprintf(fp,"\n======================================================================"); 
        fprintf(fp,"\n\t\tPWE-ID: %d", this_pwe->csi_info.env_id); 
        fprintf(fp,"\n\t\tCSI Name: %s", this_pwe->csi_info.work_desc.value); 
        fprintf(fp,"\n\t\tCSI State: %d", this_pwe->csi_info.ha_state); 
        fprintf(fp,"\n\t\tRED State: %d", this_pwe->inst->red.ha_state);
        fprintf(fp,"\n\t\tCold Sync Done : %d", this_pwe->inst->red.cold_sync_done);
        fprintf(fp,"\n\t\tWarm Sync In Progress : %d", this_pwe->inst->red.warm_sync_in_progress);
        fprintf(fp,"\n======================================================================"); 
        if (this_pwe->inst)
        {
            /* for each bucket in this table */ 
            for (bucket=0; bucket<MAB_MIB_ID_HASH_TBL_SIZE; bucket++)
            {
                fprintf(fp,"\n\tBucket ID: %d", bucket); 
                fprintf(fp,"\n\tAsync Count: %d",this_pwe->inst->red.async_count[bucket]);
                fprintf(fp,"\n\t---------");
                tbl_rec = this_pwe->inst->hash[bucket];

                /* for each table in the bucket */ 
                while (tbl_rec)
                {
                    /* go to the next table_record in this bucket */ 
                    mas_dump_tbl_rec(fp, tbl_rec); 
                    fprintf(fp,"\n\t\t\t\t-------###--------"); 
                    fflush(fp);
                    tbl_rec = tbl_rec->next; 
                } /* for each table */ 
            } /* for each bucket in the hash table */ 
        }
        /* go the next PWE */ 
        fprintf(fp,"\n------------------------------o0o-------------------------------------\n"); 
        fflush(fp); 
        this_pwe = this_pwe->next; 
    }/* for each PWE */ 

    /* close the fp */ 
    fclose(fp); 

    return; 
}

static void mas_dump_tbl_rec(FILE *fp, MAS_ROW_REC *tbl_rec) 
{
    uns32   i; 
    MAS_FLTR *curr_fltr = NULL; 

    fprintf(fp,"\n\t\tSS-ID: %d; Table-ID: %d", tbl_rec->ss_id, tbl_rec->tbl_id); 
    fprintf(fp,"\n\t\t\tDefault Filter Registered: %s", (tbl_rec->dfltr_regd)?"TRUE":"FALSE"); 
    if (tbl_rec->dfltr_regd == TRUE)
    {
        mas_dump_vcard(fp, tbl_rec->dfltr.vcard); 
        mas_dump_fltr_ids_list(fp, &tbl_rec->dfltr.fltr_ids); 
    }
    /* dump of  all the filters */
    curr_fltr = tbl_rec->fltr_list; 
    while (curr_fltr)
    {
        switch (curr_fltr->fltr.type)
        {
            case NCSMAB_FLTR_RANGE:
            fprintf(fp,"\n\t\t\tFilter Type: RANGE"); 
            fprintf(fp,"\n\t\t\tIndex Length: %d", curr_fltr->fltr.fltr.range.i_idx_len); 
            fprintf(fp,"\n\t\t\tBegin Index: %d", curr_fltr->fltr.fltr.range.i_bgn_idx); 
            fprintf(fp,"\n\t\t\tMin Index:\n\t\t\t");
            for (i=0; i<curr_fltr->fltr.fltr.range.i_idx_len; i++) 
            {
                fprintf(fp," %d", *(curr_fltr->fltr.fltr.range.i_min_idx_fltr+i)); 
            }
            fprintf(fp,"\n\t\t\tMax Index:\n\t\t\t");
            for (i=0; i<curr_fltr->fltr.fltr.range.i_idx_len; i++) 
            {
                fprintf(fp," %d", *(curr_fltr->fltr.fltr.range.i_max_idx_fltr+i)); 
            }
            break; 
            case NCSMAB_FLTR_ANY:
            fprintf(fp,"\n\t\t\tFilter Type: ANY"); 
            break; 
            case NCSMAB_FLTR_SAME_AS:
            fprintf(fp,"\n\t\t\tFilter Type: SAME_AS; SameAs Table-ID: %d", curr_fltr->fltr.fltr.same_as.i_table_id); 
            break; 
            case NCSMAB_FLTR_EXACT:
            fprintf(fp,"\n\t\t\tFilter Type: EXACT"); 
            fprintf(fp,"\n\t\t\tIndex Length: %d", curr_fltr->fltr.fltr.exact.i_idx_len); 
            fprintf(fp,"\n\t\t\tBegin Index: %d", curr_fltr->fltr.fltr.exact.i_bgn_idx); 
            fprintf(fp,"\n\t\t\tExact Index:\n\t\t\t");
            for (i=0; i<curr_fltr->fltr.fltr.exact.i_idx_len; i++) 
            {
                fprintf(fp," %d", *(curr_fltr->fltr.fltr.exact.i_exact_idx+i)); 
            }
            break; 
            case NCSMAB_FLTR_DEFAULT:
            fprintf(fp,"\n\t\t\tFilter Type: DEFAULT"); 
            return; 
            break; 
            case NCSMAB_FLTR_SCALAR:
            fprintf(fp,"\n\t\t\tFilter Type: SCALAR"); 
            return;
            break;
            default:
            fprintf(fp,"\n\t\t\tFilter Type: UNSUPPORTED"); 
            return;
            break;
        } /* end of switch (curr_fltr->fltr.type) */ 
        fprintf(fp,"\n\t\t\tIs MoveRow Filter: %s", 
                (curr_fltr->fltr.is_move_row_fltr)?"TRUE":"FALSE"); 

        mas_dump_vcard(fp, curr_fltr->vcard); 
        mas_dump_fltr_ids_list(fp, &curr_fltr->fltr_ids); 
        fprintf(fp,"\n\t\t\tReference Count: %d", curr_fltr->ref_cnt); 
        fprintf(fp,"\n\t\t\tWild Card: %d\n", curr_fltr->wild_card); 
    
        /* go to the next filter */ 
        curr_fltr = curr_fltr->next; 
    } /* end of all the filters */

    return; 
}

static void mas_dump_vcard(FILE *fp, MDS_DEST mds_dest) 
{
    fprintf(fp,"\n\t\t\tMDS_DEST"); 

    /* convert the MDS_DEST into a string */
    if (m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest) == 0)
       fprintf(fp,"\n\t\t\t\tVDEST:%d", 
              m_MDS_GET_VDEST_ID_FROM_MDS_DEST(mds_dest));
    else
       fprintf(fp,"\n\t\t\t\tADEST:node_id:%d, v1.pad16:%d, v1.vcard:%lu",
              m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest), 0, 
              (long)(mds_dest));
    return; 
}

static void mas_dump_fltr_ids_list(FILE *fp, MAS_FLTR_IDS *fltr_ids) 
{
    MAB_FLTR_ANCHOR_NODE *list; 

    fprintf(fp,"\n\t\t\tActive Filter-Id Anchor Mappling list"); 
    list = fltr_ids->active_fltr; 
    if (list != NULL)
    {
        fprintf(fp,"\n\t\t\t\t Anchor Vaue: %llu, Filter-ID: %d", 
                    list->anchor, list->fltr_id); 
    }

    fprintf(fp,"\n\t\t\tOther Filter-Id Anchor Mapping list"); 
    list = fltr_ids->fltr_id_list; 
    while (list)
    {
        fprintf(fp,"\n\t\t\t\t Anchor Vaue: %llu, Filter-ID: %d", 
                list->anchor, list->fltr_id); 
        list = list->next; 
    }
    return; 
}

#endif
