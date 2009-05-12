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

  File : psr_bam.c 


..............................................................................

  DESCRIPTION:


  DESCRIPTION:

  This file contains general MDS routines required between PSS and BAM instances.

  pss_bam_mds_dec - decode a message from PSS to BAM
  pss_bam_mds_cpy - copy a message from PSS to BAM

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/
/* #include "mab.h" */
#include "gl_defs.h"
#include "t_suite.h"

#include "mab_tgt.h"

#if (NCS_BAM == 1)
#include "psr_bam.h"

/****************************************************************************
 *  MAB message helper functions
 ****************************************************************************/
uns32  pss_bam_encode_conf_done(NCS_UBAID *uba, PSS_BAM_CONF_DONE *conf_done);
uns32  pss_bam_encode_pcn(NCS_UBAID *uba, char *pcn);
uns32  pss_bam_decode_pcn(NCS_UBAID *uba, char **pcn);
uns32  pss_bam_decode_warmboot_req(NCS_UBAID *uba, PSS_BAM_WARMBOOT_REQ *warmboot_req);

#define PSS_BAM_MSG_HDR_SIZE   (sizeof(uns8))

/****************************************************************************
 * Function Name: pss_bam_mds_enc
 * Purpose:        encode a message to PSS
 ****************************************************************************/
uns32 pss_bam_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg, 
                  SS_SVC_ID to_svc,     NCS_UBAID*  uba)
{
   uns8* data;
   PSS_BAM_MSG *pm = NULL;

   if((uba == NULL) || (msg == NULL))
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   
   pm = (PSS_BAM_MSG*)msg;

   data = ncs_enc_reserve_space(uba, PSS_BAM_MSG_HDR_SIZE);
   if(data == NULL)
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

   ncs_encode_8bit(&data, pm->i_evt);
   ncs_enc_claim_space(uba, PSS_BAM_MSG_HDR_SIZE);

   switch(pm->i_evt)
   {
   case PSS_BAM_EVT_CONF_DONE:
      return pss_bam_encode_conf_done(uba, &pm->info.conf_done);
      break;

   default:
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_bam_mds_dec
 * Purpose:        decode a message coming in from PSS
 ****************************************************************************/
uns32 pss_bam_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT* msg, 
                  SS_SVC_ID to_svc,     NCS_UBAID*  uba)
  {
  uns8*    data;
  PSS_BAM_MSG* pm;
  uns8     data_buff[PSS_BAM_MSG_HDR_SIZE];
  
  if(uba == NULL)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

  if(msg == NULL)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  
  pm = m_MMGR_ALLOC_PSS_BAM_MSG;

  if(pm == NULL)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

  memset(pm, '\0', sizeof(PSS_BAM_MSG));

  *msg = pm;

  data = ncs_dec_flatten_space(uba,data_buff,PSS_BAM_MSG_HDR_SIZE);
  if(data == NULL)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

  pm->i_evt   = (PSS_BAM_EVT)ncs_decode_8bit(&data);
  ncs_dec_skip_space(uba,PSS_BAM_MSG_HDR_SIZE);
  
  switch(pm->i_evt)
    {
    case PSS_BAM_EVT_WARMBOOT_REQ:
        return pss_bam_decode_warmboot_req(uba, &pm->info.warmboot_req);
        break;

    default:
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
  
  return NCSCC_RC_SUCCESS;
  }

/****************************************************************************
 * Function Name: pss_bam_mds_cpy
 * Purpose:        copy a PSS_BAM_MSG message going to BAM in this memory space
 ****************************************************************************/
uns32 pss_bam_mds_cpy(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg, 
                  SS_SVC_ID to_svc,     NCSCONTEXT* cpy,
                  NCS_BOOL   last)
  {
  PSS_BAM_MSG* pm;
  
  if(msg == NULL)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  
  pm = m_MMGR_ALLOC_PSS_BAM_MSG;
  
  if(pm == NULL)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  
  *cpy = pm;
  
  memcpy(pm,msg,sizeof(PSS_BAM_MSG));

  switch(pm->i_evt)
    {
    case PSS_BAM_EVT_WARMBOOT_REQ:
      pm->info.warmboot_req = ((PSS_BAM_MSG*)msg)->info.warmboot_req;
      /* This is not yet completed!!!! To be done at a later stage!!!! */
      break;

    default:
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
  
  return NCSCC_RC_SUCCESS;
  }

/****************************************************************************
 * Function Name: pss_bam_ecode_pcn
 * Purpose:       Encodes the <PCN> string into the ubaid, required by
 *                PSS.
 ****************************************************************************/
uns32 pss_bam_encode_pcn(NCS_UBAID *uba, char *pcn)
{
   uns8*    data;
   uns16    len = 0;

   if(pcn == NULL)
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

   len = strlen(pcn);
   data = ncs_enc_reserve_space(uba, 2);
   if(data == NULL)
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_16bit(&data,len);
   ncs_enc_claim_space(uba, 2);

   if(len != 0)
   {
      if(ncs_encode_n_octets_in_uba(uba, pcn, len) != NCSCC_RC_SUCCESS)
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_bam_decode_pcn
 * Purpose:       Decodes the <PCN> string into the ubaid, required by
 *                BAM.
 ****************************************************************************/
uns32 pss_bam_decode_pcn(NCS_UBAID *uba, char **pcn)
{
    uns8*    data;
    uns16    len = 0;

    if(pcn == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    data = ncs_dec_flatten_space(uba, (uns8*)&len, 2);
    if(data == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    len = ncs_decode_16bit(&data);
    ncs_dec_skip_space(uba, 2);

    if(len != 0)
    {
       if(*pcn == NULL)
       {
          if((*pcn = m_MMGR_ALLOC_BAM_PCN_STRING(len)) == NULL)
             return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       }
       memset(*pcn, '\0', len);
       if(ncs_decode_n_octets_from_uba(uba, *pcn, len) != NCSCC_RC_SUCCESS)
       {
           m_MMGR_FREE_BAM_PCN_STRING(*pcn);
           return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       }
    }
    else
    {
       if(*pcn != NULL)
       {
          *pcn[0] = '\0'; /* Nullify the string */
       }
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_bam_encode_conf_done
 * Purpose:       Encodes the conf_done message from BAM to PSS 
 ****************************************************************************/
uns32 pss_bam_encode_conf_done(NCS_UBAID *uba, PSS_BAM_CONF_DONE *conf_done)
{
    MAB_PSS_CLIENT_LIST *p_pcn = NULL;
    MAB_PSS_TBL_LIST    *p_tbl = NULL;
    uns8     *data, *p_tbl_cnt;
    uns16    tbl_cnt = 0;

    p_pcn = &conf_done->pcn_list;
    if(p_pcn->pcn == NULL)
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    if(pss_bam_encode_pcn(uba, p_pcn->pcn) != NCSCC_RC_SUCCESS)
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    tbl_cnt = 0;
    p_tbl_cnt = ncs_enc_reserve_space(uba, 2);
    if(p_tbl_cnt == NULL)
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    ncs_enc_claim_space(uba, 2);

    p_tbl = p_pcn->tbl_list;
    while(p_tbl != NULL)
    {
       data = ncs_enc_reserve_space(uba, 4);
       if(data == NULL)
          return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_32bit(&data, p_tbl->tbl_id);
       ncs_enc_claim_space(uba, 4);

       p_tbl = p_tbl->next;
       tbl_cnt++;
    };
    ncs_encode_16bit(&p_tbl_cnt, tbl_cnt);

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_bam_decode_warmboot_req
 * Purpose:       Decodes the Warmboot playback request structure from
 *                PSS(to BAM) into the NCS_UBAID.
 ****************************************************************************/
uns32 pss_bam_decode_warmboot_req(NCS_UBAID *uba, PSS_BAM_WARMBOOT_REQ *warmboot_req)
{
    uns8*    data;
    uns16    pcn_cnt = 0;
    uns32    pcn_loop = 0;
    PSS_BAM_WARMBOOT_REQ *levt = NULL;

    if(warmboot_req == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    data = ncs_dec_flatten_space(uba,(uns8*)&pcn_cnt, 2);
    if(data == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    pcn_cnt = ncs_decode_16bit(&data);
    ncs_dec_skip_space(uba, 2);

    for(pcn_loop = 0; pcn_loop < pcn_cnt; pcn_loop++)
    {
       if(levt == NULL)
       {
          levt = warmboot_req;
          memset(levt, '\0', sizeof(PSS_BAM_WARMBOOT_REQ));
       }
       else
       {
          if((levt->next = m_MMGR_ALLOC_PSS_BAM_WARMBOOT_REQ) == NULL)
          {
             return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          }
          memset(levt->next, '\0', sizeof(PSS_BAM_WARMBOOT_REQ));
          levt = levt->next;
       }

       if(pss_bam_decode_pcn(uba, &levt->pcn) != NCSCC_RC_SUCCESS)
          return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    return NCSCC_RC_SUCCESS;
}
#endif

