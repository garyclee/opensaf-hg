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

DESCRIPTION:

******************************************************************************
*/

#include "ncsgl_defs.h"
#include "mds_papi.h"
#include "fma_fm_intf.h"
#include "ncssysf_def.h"
#include "ncsgl_defs.h"
#include "ncssysfpool.h"
#include "ncs_log.h"
#include "ncsencdec_pub.h"

#define FMA_FM_MSG_HDR_SIZE  (sizeof(uns8))

static uns32 fma_fm_encode_phy_addr(NCS_UBAID *uba, FMA_FM_PHY_ADDR *phy_addr);
static uns32 fma_fm_decode_phy_addr(NCS_UBAID *uba, FMA_FM_PHY_ADDR *phy_addr);


/**************************************************************************
 * Name        :  fma_fm_encode_phy_addr
 *
 * Description :  encode the FMA_FM_PHY_ADDR struct
 *
 * Arguments   :  Pointer to UBA, and Physical Addr location.
 *
 * Return Value:  NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 *
 * NOTESi      : None.
 *
 **************************************************************************/
static uns32 fma_fm_encode_phy_addr(NCS_UBAID *uba, FMA_FM_PHY_ADDR *phy_addr)
{
   uns8 *data=NULL;
    
   data = ncs_enc_reserve_space(uba, (2*sizeof(uns8)));
   if (data == NULL)
   {
      m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
    
   ncs_encode_8bit(&data, phy_addr->slot);
   ncs_encode_8bit(&data, phy_addr->site);
   ncs_enc_claim_space(uba, 2*sizeof(uns8));
    
   return NCSCC_RC_SUCCESS;
}


/**************************************************************************
 * Name        : fma_fm_decode_phy_addr
 *
 * Description : decode the FMA_FM_PHY_ADDR struct
 *
 * Arguments   : Pointer to UBA, and Physical Addr location.
 *
 * Return Value: NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 *
 * NOTES      : None.
 *
 **************************************************************************/
static uns32 fma_fm_decode_phy_addr(NCS_UBAID *uba, FMA_FM_PHY_ADDR *phy_addr)
{
   uns8 *data=NULL;
   uns8 data_buff[256];
    
   data = ncs_dec_flatten_space(uba,data_buff, 2*sizeof(uns8));
   if (data == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
    
   phy_addr->slot = (uns8)ncs_decode_8bit(&data);
   phy_addr->site = (uns8)ncs_decode_8bit(&data);
    
   ncs_dec_skip_space(uba, 2*sizeof(uns8));
    
   return NCSCC_RC_SUCCESS;
}


/**************************************************************************
 * Name        : fma_fm_mds_cpy
 *
 * Description : Copy Source contents into destination buffer.
 *
 * Arguments   :  Pointer to MDS_CALLBACK_COPY_INFO
 *
 * Return Value: NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS  
 *
 * NOTES      :None
 *
 **************************************************************************/
uns32 fma_fm_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info)
{
   FMA_FM_MSG *src_msg = NULL;
   FMA_FM_MSG *dst_msg = NULL;

   if(cpy_info->i_msg == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   src_msg = cpy_info->i_msg;

   if((dst_msg = m_MMGR_ALLOC_FMA_FM_MSG) == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   memset(dst_msg, 0, sizeof(FMA_FM_MSG));
   memcpy(dst_msg, src_msg, sizeof(FMA_FM_MSG));

   /** Is this the rite thing to do !!! **/
   cpy_info->o_cpy = dst_msg;
   
   return NCSCC_RC_SUCCESS;
}


/**************************************************************************
 * Name        : fma_fm_mds_enc
 *
 * Description : Encode the FMA_FM_MSG structure
 *
 * Arguments   : Pointer to MDS_CALLBACK_ENC_INFO
 *
 * Return Value: NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 *
 * NOTES       : None.
 *
 **************************************************************************/
uns32 fma_fm_mds_enc(MDS_CALLBACK_ENC_INFO *enc_info)
{
   uns8*  data = NULL;
   FMA_FM_MSG *msg = NULL;
   NCS_UBAID *uba = NULL;

   if (enc_info == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   if((enc_info->i_msg == NULL) ||
      (enc_info->io_uba == NULL))
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   uba = enc_info->io_uba;

   msg = (FMA_FM_MSG *)enc_info->i_msg;

   data = ncs_enc_reserve_space(uba, sizeof(FMA_FM_MSG_TYPE));
   if(data == NULL)
   {
      m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   ncs_encode_32bit(&data, msg->msg_type);
   ncs_enc_claim_space(uba, sizeof(FMA_FM_MSG_TYPE));

   switch(msg->msg_type)
   {
   case FMA_FM_EVT_HB_LOSS:
   case FMA_FM_EVT_HB_RESTORE:
   case FMA_FM_EVT_NODE_RESET_IND:
       if(fma_fm_encode_phy_addr(uba, &(msg->info.phy_addr)) != NCSCC_RC_SUCCESS)
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       break;
        
   case FMA_FM_EVT_CAN_SMH_SW:
   case FMA_FM_EVT_SYSMAN_SWITCH_REQ:
       break;
        
   case FMA_FM_EVT_SMH_SW_RESP:
       data = ncs_enc_reserve_space(uba, (sizeof(uns8)));
       if(data == NULL)
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       ncs_encode_8bit(&data, msg->info.response);
       ncs_enc_claim_space(uba, (sizeof(uns8)));
       break;
        
   default:
       m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       break;
            
   }

   return NCSCC_RC_SUCCESS;
}


/**************************************************************************
 * Name        :  fma_fm_mds_dec
 *
 * Description :  Decode the FMA_FM_MSG structure
 *
 * Arguments   : Pointer to MDS_CALLBACK_DEC_INFO
 *
 * Return Value: NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 *
 * NOTES       : None.
 *
 **************************************************************************/
uns32 fma_fm_mds_dec(MDS_CALLBACK_DEC_INFO *dec_info)
{
   uns8*  data = NULL;
   FMA_FM_MSG *msg = NULL;
   NCS_UBAID *uba = NULL;
   uns8 data_buff[256];

   if(dec_info->io_uba == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   msg = m_MMGR_ALLOC_FMA_FM_MSG;
   if(!msg)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   memset(msg, 0, sizeof(FMA_FM_MSG));

   dec_info->o_msg = msg;
   uba = dec_info->io_uba;

   data = ncs_dec_flatten_space(uba,data_buff,sizeof(FMA_FM_MSG_TYPE));
   if(data == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   msg->msg_type = (FMA_FM_MSG_TYPE)ncs_decode_32bit(&data);
   ncs_dec_skip_space(uba,sizeof(FMA_FM_MSG_TYPE));
    
   switch(msg->msg_type)
   {
   case FMA_FM_EVT_HB_LOSS:
   case FMA_FM_EVT_HB_RESTORE:
   case FMA_FM_EVT_NODE_RESET_IND:
       if(fma_fm_decode_phy_addr(uba, &(msg->info.phy_addr)) != NCSCC_RC_SUCCESS)
       {
          m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
       break;
        
   case FMA_FM_EVT_CAN_SMH_SW:
   case FMA_FM_EVT_SYSMAN_SWITCH_REQ:
       break;
        
   case FMA_FM_EVT_SMH_SW_RESP:
       data = ncs_dec_flatten_space(uba,data_buff,(sizeof(uns8)));
       if(data == NULL)
       {
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }

       msg->info.response = (FMA_FM_SMH_SW_RESP)ncs_decode_8bit(&data);
       break;
        
   default:
       m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       break;
   }

   return NCSCC_RC_SUCCESS;
}

