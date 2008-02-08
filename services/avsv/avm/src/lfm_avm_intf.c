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


#if (MOT_ATCA == 1)

#include "lfm_avm_intf.h"
#include "ncssysf_def.h"
#include "ncssysfpool.h"
#include "ncsencdec_pub.h"

/* 
 * This file contains general MDS routines for the messages between 
 * AVM and LFM
 */
#define LFM_AVM_MSG_HDR_SIZE  (sizeof(uns8))


/* The entity path is an array of entities and hence contiguous memory
* so do it one shot. The other way is to convert the SaHpiEntityPathT 
* structure to string and then pass it on. The advantage this way is 
* smaller message size. But the overhead is conversion routines. LFM 
* doesnt need to know how to break the structure and build it. So for 
* now lets live with passing the entity_path as a structure itself.
*/
static uns32 lfm_avm_encode_entity_path(NCS_UBAID *uba, SaHpiEntityPathT *ent_path)
{
   uns8 *data=NULL;
   uns32 index=0;
   
   data = ncs_enc_reserve_space(uba, (sizeof(SaHpiEntityPathT)));
   if(data == NULL)
   {
      m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   for(index=0; index < SAHPI_MAX_ENTITY_PATH; index++)
   {
      ncs_encode_32bit(&data, ent_path->Entry[index].EntityType);
      ncs_encode_32bit(&data, ent_path->Entry[index].EntityInstance);       
   }
   ncs_enc_claim_space(uba, (sizeof(SaHpiEntityPathT)) );

   return NCSCC_RC_SUCCESS;

}

static uns32 lfm_avm_decode_entity_path(NCS_UBAID *uba, SaHpiEntityPathT *ent_path)
{
   uns8 *data=NULL;
   uns32 index=0;
   uns8 data_buff[256];
   
   data = ncs_dec_flatten_space(uba,data_buff,sizeof(SaHpiEntityPathT));
   if(data == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   for(index=0; index < SAHPI_MAX_ENTITY_PATH; index++)
   {
      ent_path->Entry[index].EntityType = 
            (SaHpiEntityTypeT) ncs_decode_32bit(&data);
      ent_path->Entry[index].EntityInstance = 
            (SaHpiUint32T) ncs_decode_32bit(&data);
   }
   ncs_dec_skip_space(uba,sizeof(SaHpiEntityPathT));

   return NCSCC_RC_SUCCESS;
}



/**************************************************************************
 * Name        :  lfm_avm_mds_enc
 * 
 * Description :  
 *
 * Arguments   :  MDS_CALLBACK_ENC_INFO
 *
 * Return Value:   NCSCC_RC_SUCCESS/Error Code.
 * 
 * NOTES:
 * Since these message are not final for the final release of NFM
 * lets not go through EDU routines
 *
 * These functions need to be invoked after taking necessary locks or
 * handles as needed by the subsystem.
 *
 * Also note that the encode routines need to be modified whenever the 
 * Data Structure changes or more enum values are added that cant fit 8 bits.
 **************************************************************************/
uns32 lfm_avm_mds_enc(MDS_CALLBACK_ENC_INFO *enc_info)
{
    uns8*  data = NULL;
    uns8*  p_count = NULL;
    LFM_AVM_MSG *msg = NULL;
    AVM_LFM_FRU_HSSTATE_INFO *p_info = NULL;
    NCS_UBAID *uba = NULL;
    uns16    count=0;

    if(enc_info == NULL) 
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

    if((enc_info->i_msg == NULL) || 
       (enc_info->io_uba == NULL) )
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

    uba = enc_info->io_uba;

    msg = (LFM_AVM_MSG *)enc_info->i_msg;

    data = ncs_enc_reserve_space(uba, LFM_AVM_MSG_HDR_SIZE);
    if(data == NULL)
    {
       m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

    ncs_encode_8bit(&data, msg->msg_type);
    ncs_enc_claim_space(uba, LFM_AVM_MSG_HDR_SIZE);

    switch(msg->msg_type)
    {
    case AVM_LFM_HEALTH_STATUS_RSP:
       
       data = ncs_enc_reserve_space(uba, 4*(sizeof(uns8)));
       if(data == NULL)
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       
       ncs_encode_8bit(&data, msg->info.hlt_status_rsp.plane);
       ncs_encode_8bit(&data, msg->info.hlt_status_rsp.plane_status);
       ncs_encode_8bit(&data, msg->info.hlt_status_rsp.sam);
       ncs_encode_8bit(&data, msg->info.hlt_status_rsp.sam_status);
       ncs_enc_claim_space(uba, 4*(sizeof(uns8)) );
       break;

    case AVM_LFM_NODE_RESET_IND:
    case LFM_AVM_NODE_RESET_IND:

       if(lfm_avm_encode_entity_path(uba, &(msg->info.node_reset_ind)) 
                     != NCSCC_RC_SUCCESS )
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }

       break;

    case AVM_LFM_FRU_HSSTATUS:
       p_count = ncs_enc_reserve_space(uba, (sizeof(uns16)));
       if(p_count == NULL)
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }

       ncs_enc_claim_space(uba, (sizeof(uns16)));

       p_info = msg->info.fru_hsstate_info;
       while(p_info != NULL)
       {
          if(lfm_avm_encode_entity_path(uba, &(p_info->fru)) 
                     != NCSCC_RC_SUCCESS )
          {
             m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
          }

          data = ncs_enc_reserve_space(uba, (sizeof(uns8)));
          if(data == NULL)
          {
             m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
          }
          ncs_encode_8bit(&data, p_info->hs_state);
          ncs_enc_claim_space(uba, (sizeof(uns8)) );          

          p_info = p_info->next;
          count++;
       }
       ncs_encode_16bit(&p_count, count);

    case LFM_AVM_HEALTH_STATUS_REQ:
       
       data = ncs_enc_reserve_space(uba, (sizeof(uns8)));
       if(data == NULL)
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       
       ncs_encode_8bit(&data, msg->info.hlt_status_req);
       ncs_enc_claim_space(uba, (sizeof(uns8)) );

       break;

    case LFM_AVM_HEALTH_STATUS_UPD:
       
       data = ncs_enc_reserve_space(uba, 4*(sizeof(uns8)));
       if(data == NULL)
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       
       ncs_encode_8bit(&data, msg->info.hlt_status_upd.plane);
       ncs_encode_8bit(&data, msg->info.hlt_status_upd.plane_status);
       ncs_encode_8bit(&data, msg->info.hlt_status_upd.sam);
       ncs_encode_8bit(&data, msg->info.hlt_status_upd.sam_status);
       ncs_enc_claim_space(uba, 4*(sizeof(uns8)) );
       break;

    case LFM_AVM_SWOVER_REQ:
    case LFM_AVM_HEALTH_ALL_FRU_HS_REQ:
       break;
    default:
       m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       break;
    }

    return NCSCC_RC_SUCCESS;
}


/**************************************************************************
 * Name        :  lfm_avm_mds_dec
 * 
 * Description :  
 *
 * Arguments   :  MDS_CALLBACK_DEC_INFO
 *
 * Return Value:   NCSCC_RC_SUCCESS/Error Code.
 * 
 * NOTES:
 * Since these message are not final for the final release of NFM
 * lets not go through EDU routines
 *
 * These functions need to be invoked after taking necessary locks or
 * handles as needed by the subsystem.
 *
 * Also note that the encode/decode routines need to be modified 
 * when the structure changes or more enum values are added that 
 * cant fit 8 bits.
 *
 **************************************************************************/
uns32 lfm_avm_mds_dec(MDS_CALLBACK_DEC_INFO *dec_info)
{
    uns8*  data = NULL;
    LFM_AVM_MSG *msg = NULL;
    NCS_UBAID *uba = NULL;
    uns8 data_buff[256];
    uns16   fru_count =0;
    uns16   fru_loop =0;
    AVM_LFM_FRU_HSSTATE_INFO *p_fru_hsstate= NULL;

    if(dec_info->io_uba == NULL) 
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }
    
    msg = m_MMGR_ALLOC_LFM_AVM_MSG;
    if(!msg)
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

    m_NCS_OS_MEMSET(msg, 0, sizeof(LFM_AVM_MSG));

    dec_info->o_msg = msg;
    uba = dec_info->io_uba;

    data = ncs_dec_flatten_space(uba,data_buff,LFM_AVM_MSG_HDR_SIZE);
    if(data == NULL)
    {
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }
    
    msg->msg_type = (LFM_AVM_MSG_TYPE)ncs_decode_8bit(&data);
    ncs_dec_skip_space(uba,LFM_AVM_MSG_HDR_SIZE);

    switch(msg->msg_type)
    {
    case AVM_LFM_HEALTH_STATUS_RSP:
       
       data = ncs_dec_flatten_space(uba,data_buff,4*(sizeof(uns8)));
       if(data == NULL)
       {
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       
       msg->info.hlt_status_rsp.plane = (HEALTH_STATUS_ENTITY)ncs_decode_8bit(&data);
       msg->info.hlt_status_rsp.plane_status = (HEALTH_STATUS)ncs_decode_8bit(&data);
       msg->info.hlt_status_rsp.sam = (HEALTH_STATUS_ENTITY)ncs_decode_8bit(&data);
       msg->info.hlt_status_rsp.sam_status = (HEALTH_STATUS)ncs_decode_8bit(&data);
       ncs_dec_skip_space(uba,4*(sizeof(uns8)));
       break;

    case AVM_LFM_NODE_RESET_IND:
    case LFM_AVM_NODE_RESET_IND:

       if(lfm_avm_decode_entity_path(uba, &(msg->info.node_reset_ind)) 
                     != NCSCC_RC_SUCCESS )
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }

       break;

    case AVM_LFM_FRU_HSSTATUS:

       data = ncs_dec_flatten_space(uba,data_buff,sizeof(uns16));
       if(data == NULL)
       {
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       
       fru_count = ncs_decode_16bit(&data);

       ncs_dec_skip_space(uba,sizeof(uns16));
       
       for(fru_loop = 0; fru_loop < fru_count; fru_loop++)
       {
          if(p_fru_hsstate == NULL)
          {
             p_fru_hsstate = m_MMGR_ALLOC_LFM_AVM_FRU_HSSTATE_INFO;
             if(p_fru_hsstate == NULL)
             {
                return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
             }
             msg->info.fru_hsstate_info = p_fru_hsstate;
             m_NCS_OS_MEMSET(p_fru_hsstate, 0, sizeof(AVM_LFM_FRU_HSSTATE_INFO));
          }
          else
          {
             if((p_fru_hsstate->next = m_MMGR_ALLOC_LFM_AVM_FRU_HSSTATE_INFO ) == NULL)
             {
                return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
             }
             m_NCS_OS_MEMSET(p_fru_hsstate->next, 0, sizeof(AVM_LFM_FRU_HSSTATE_INFO));

             p_fru_hsstate = p_fru_hsstate->next;
          }
          
          if(lfm_avm_decode_entity_path(uba, &(p_fru_hsstate->fru)) 
                != NCSCC_RC_SUCCESS )
          {
             m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
          }

          data = ncs_dec_flatten_space(uba,data_buff,sizeof(uns8));
          if(data == NULL)
          {
             return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
          }
          p_fru_hsstate->hs_state = (SaHpiHsStateT)ncs_decode_8bit(&data);
          ncs_dec_skip_space(uba,sizeof(uns8));
          
       } /* end for */
       break;

    case LFM_AVM_HEALTH_STATUS_REQ:

       data = ncs_dec_flatten_space(uba,data_buff,sizeof(uns8));
       if(data == NULL)
       {
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       msg->info.hlt_status_req = (HEALTH_STATUS_ENTITY)ncs_decode_8bit(&data);
       ncs_dec_skip_space(uba,sizeof(uns8));

       break;

    case LFM_AVM_HEALTH_STATUS_UPD:

       data = ncs_dec_flatten_space(uba,data_buff,4*(sizeof(uns8)));
       if(data == NULL)
       {
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       
       msg->info.hlt_status_upd.plane = (HEALTH_STATUS_ENTITY)ncs_decode_8bit(&data);
       msg->info.hlt_status_upd.plane_status = (HEALTH_STATUS)ncs_decode_8bit(&data);
       msg->info.hlt_status_upd.sam = (HEALTH_STATUS_ENTITY)ncs_decode_8bit(&data);
       msg->info.hlt_status_upd.sam_status = (HEALTH_STATUS)ncs_decode_8bit(&data);

       ncs_dec_skip_space(uba,4*(sizeof(uns8)));
       break;


    case LFM_AVM_SWOVER_REQ:
    case LFM_AVM_HEALTH_ALL_FRU_HS_REQ:
       break;
       
    default:
       return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

    }

    return NCSCC_RC_SUCCESS;
}


/**************************************************************************
 * Name        :  lfm_avm_mds_cpy
 * 
 * Description :  
 *
 * Arguments   :  MDS_CALLBACK_COPY_INFO
 *
 * Return Value:   NCSCC_RC_SUCCESS/Error Code.
 * 
 * NOTES:
 *
 * These functions need to be invoked after taking necessary locks or
 * handles as needed by the subsystem.
 *
 * The message type AVM_LFM_FRU_HSSTATUS has list so copy the list for
 * all other messages copy the message.
 **************************************************************************/
uns32 lfm_avm_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info)
{
   LFM_AVM_MSG *src_msg = NULL;
   LFM_AVM_MSG *dst_msg = NULL;
   AVM_LFM_FRU_HSSTATE_INFO *p_src_fru= NULL, *p_dst_fru=NULL;
   AVM_LFM_FRU_HSSTATE_INFO **prev_fru = NULL;

   if(cpy_info->i_msg == NULL)
   {
      return NCSCC_RC_FAILURE;
   }
   src_msg = cpy_info->i_msg;

   if((dst_msg = m_MMGR_ALLOC_LFM_AVM_MSG) == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   m_NCS_OS_MEMSET(dst_msg, 0, sizeof(LFM_AVM_MSG));

   m_NCS_MEMCPY(dst_msg, src_msg, sizeof(LFM_AVM_MSG));

   if(src_msg->msg_type == AVM_LFM_FRU_HSSTATUS)
   {
      dst_msg->info.fru_hsstate_info = NULL;
      p_src_fru = src_msg->info.fru_hsstate_info;

      prev_fru = &dst_msg->info.fru_hsstate_info;

      while(p_src_fru)
      {
         if(( p_dst_fru = m_MMGR_ALLOC_LFM_AVM_FRU_HSSTATE_INFO ) == NULL)
         {
            m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         m_NCS_MEMCPY(p_dst_fru, p_src_fru, 
                        sizeof(AVM_LFM_FRU_HSSTATE_INFO));
         p_dst_fru->next = NULL;
         *prev_fru = p_dst_fru;
         prev_fru = &p_dst_fru->next;

         /* traverse the next node */
         p_src_fru = p_src_fru->next;
      }
   }

   return NCSCC_RC_SUCCESS;
}

void lfm_avm_msg_free(LFM_AVM_MSG *msg)
{
   AVM_LFM_FRU_HSSTATE_INFO *p_fru_info= NULL;

   if(msg->msg_type == AVM_LFM_FRU_HSSTATUS)
   {
      /* free the list */
      while(msg->info.fru_hsstate_info != NULL)
      {
         p_fru_info = msg->info.fru_hsstate_info;
         msg->info.fru_hsstate_info = p_fru_info->next;
         m_MMGR_FREE_LFM_AVM_FRU_HSSTATE_INFO(p_fru_info);
      }
   }

   /* Now free the entire message */
   m_MMGR_FREE_LFM_AVM_MSG(msg);

   return;
}

#endif /* (MOT_ATCA == 1) */
