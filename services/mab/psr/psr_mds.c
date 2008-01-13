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


..............................................................................

  DESCRIPTION:

  This file contains PSS specific MDS routines, being:

  pss_mds_rcv    - rcv something from MDS lower layer.
  pss_mds_subevt - subscription event entry point
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_MAB == 1)
#if (NCS_PSR == 1)
#include "ncs_util.h"
#include "psr.h"

MDS_CLIENT_MSG_FORMAT_VER   pss_wrt_mac_table[PSS_WRT_MAC_SUBPART_VER_RANGE] = {1};
MDS_CLIENT_MSG_FORMAT_VER   pss_wrt_oac_table[PSS_WRT_OAC_SUBPART_VER_RANGE] = {1};
MDS_CLIENT_MSG_FORMAT_VER   pss_wrt_bam_table[PSS_WRT_BAM_SUBPART_VER_RANGE] = {1};


MABPSR_API uns32 pss_mds_evt_cb(NCSMDS_CALLBACK_INFO * cbinfo);
static uns32 pss_bam_decode_msg(NCSMDS_CALLBACK_INFO *cbinfo);

/****************************************************************************
 * Function Name: pss_mds_grcv
 * Purpose:        Service ID PSS gets a message
 ****************************************************************************/
uns32 pss_mds_grcv(MDS_HDL pwe_hdl,    MDS_CLIENT_HDL yr_svc_hdl,
                  NCSCONTEXT msg, 
                  MDS_DEST fr_card,     SS_SVC_ID fr_svc,
                  MDS_DEST  to_card,    SS_SVC_ID to_svc)
{
   PSS_PWE_CB* pwe_cb;

   if((pwe_cb = (PSS_PWE_CB*)ncshm_take_hdl(NCS_SERVICE_ID_PSS, (uns32)yr_svc_hdl)) == NULL)
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

  /* plant MAB subcomponent's control block in MAB_MSG */

  ((MAB_MSG*)msg)->yr_hdl  = pwe_cb;
  ((MAB_MSG*)msg)->pwe_hdl = pwe_hdl;
  /* ##### MDS changes here */
  ((MAB_MSG*)msg)->fr_card = fr_card;
  ((MAB_MSG*)msg)->fr_svc  = fr_svc;

  /* Put it in PSS's work queue */

  if(m_PSS_SND_MSG(pwe_cb->mbx,msg) == NCSCC_RC_FAILURE)
  {
    ncshm_give_hdl((uns32)yr_svc_hdl);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  ncshm_give_hdl((uns32)yr_svc_hdl);
  return NCSCC_RC_SUCCESS;
}

uns32 pss_mds_evt_cb(NCSMDS_CALLBACK_INFO * cbinfo)
{
   MDS_CLIENT_HDL  yr_svc_hdl = cbinfo->i_yr_svc_hdl;
   PSS_PWE_CB      *pwe_cb = (PSS_PWE_CB*)m_PSS_VALIDATE_HDL((uns32)yr_svc_hdl);
   MAB_MSG         *mm = NULL;
   NCS_BOOL        post_to_mbx = FALSE;

   if (pwe_cb == NULL)
   {
      /* log the error */ 
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

#if 0
#if (NCS_PSS_RED == 1)
   if(pwe_cb->p_pss_cb->ha_state != SA_AMF_HA_ACTIVE)
   {
      /* Don't process MDS events in non-ACTIVE role. */
      ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
      return NCSCC_RC_SUCCESS;
   }
#endif
#endif

   switch (cbinfo->info.svc_evt.i_change) /* Review change type */ 
   {
      case NCSMDS_DOWN:
      case NCSMDS_NO_ACTIVE:
         /* OAA is down in this PWE */ 
         if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC)
         {
            /* Not sure why this is required. 
            if (cbinfo->info.svc_evt.i_dest.node_id == 0) */
            {
                post_to_mbx = TRUE;
            }
         }
         
         /* MAS is down in this PWE */ 
         if((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_MAS) &&
            (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY))
         {
            post_to_mbx = TRUE;
         }
         /* BAM is down */ 
         else if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_BAM)
         {
            post_to_mbx = TRUE;
         }
         break;

      case NCSMDS_UP:
      case NCSMDS_NEW_ACTIVE:
         /* MAS is UP */ 
         if((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_MAS) &&
            (cbinfo->info.svc_evt.i_role == NCSFT_ROLE_PRIMARY))
         {
            post_to_mbx = TRUE;
         }
         
         /* BAM is UP */ 
         else if ((cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_BAM)&&
                  (m_NCS_GET_NODE_ID == cbinfo->info.svc_evt.i_node_id))
         {
            /* local BAM is up */
            post_to_mbx = TRUE;
         }
         break;

      default:
         break;
   }

   if(post_to_mbx)
   {
      mm = m_MMGR_ALLOC_MAB_MSG;
      if(mm == NULL)
      {
         ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
      m_NCS_MEMSET(mm, '\0', sizeof(MAB_MSG));
      mm->yr_hdl = (NCSCONTEXT)pwe_cb; 
      mm->fr_card = cbinfo->info.svc_evt.i_dest;
      mm->fr_svc = cbinfo->info.svc_evt.i_svc_id;
      mm->data.data.pss_mds_svc_evt.role = cbinfo->info.svc_evt.i_role;
      mm->data.data.pss_mds_svc_evt.change = cbinfo->info.svc_evt.i_change;
      mm->data.data.pss_mds_svc_evt.svc_pwe_hdl = cbinfo->info.svc_evt.svc_pwe_hdl;
      mm->op = MAB_PSS_SVC_MDS_EVT;

      /* Put it in PSS's work queue */
      if(m_PSS_SND_MSG(pwe_cb->mbx, mm) == NCSCC_RC_FAILURE)
      {
         m_MMGR_FREE_MAB_MSG(mm);
      }
   }

   ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: pss_bam_decode_msg
 * Purpose:       Function to decode BAM messages
 ****************************************************************************/
static uns32 pss_bam_decode_msg(NCSMDS_CALLBACK_INFO *cbinfo)
{
   PSS_BAM_EVT evt;
   uns8 data_buff[32];
   uns8 *data = NULL; 
   uns32 status = NCSCC_RC_SUCCESS;
   MAB_MSG *mm = NULL;

   if((mm = m_MMGR_ALLOC_MAB_MSG) != NULL)
   {
      cbinfo->info.dec.o_msg = mm;
      m_NCS_OS_MEMSET(cbinfo->info.dec.o_msg, '\0', sizeof(MAB_MSG));

      data = ncs_dec_flatten_space(cbinfo->info.dec.io_uba,data_buff, sizeof(uns8));
      if(data == NULL)
         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

      evt = (PSS_BAM_EVT)ncs_decode_8bit(&data);
      ncs_dec_skip_space(cbinfo->info.dec.io_uba, sizeof(uns8));

      switch(evt)
      {
      case PSS_BAM_EVT_CONF_DONE:
         mm->op = MAB_PSS_BAM_CONF_DONE;
         status = pss_bam_decode_conf_done(cbinfo->info.dec.io_uba, &mm->data.data.bam_conf_done);
         break;
      default:
         status = NCSCC_RC_FAILURE;
         break;
      } 
   }
   else 
   {
      status = NCSCC_RC_FAILURE;
   }
   return status;
}

/****************************************************************************
 * Function Name: pss_mds_cb
 * Purpose:       PSS's MDS SE callback function
 *
 ****************************************************************************/

uns32 pss_mds_cb (NCSMDS_CALLBACK_INFO *cbinfo)
{
    uns32 status = NCSCC_RC_SUCCESS;

    if (cbinfo == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    switch(cbinfo->i_op)
    {
    case MDS_CALLBACK_COPY:
        cbinfo->info.cpy.o_msg_fmt_ver = cbinfo->info.cpy.i_rem_svc_pvt_ver;
        status = mab_mds_cpy(cbinfo->i_yr_svc_hdl,
                             cbinfo->info.cpy.i_msg,
                             cbinfo->info.cpy.i_to_svc_id,
                             &cbinfo->info.cpy.o_cpy,
                             cbinfo->info.cpy.i_last);
        break;

    case MDS_CALLBACK_ENC:
    case MDS_CALLBACK_ENC_FLAT:
        if(cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_MAC)
        {
           cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
                                                               PSS_WRT_MAC_SUBPART_VER_MIN,
                                                               PSS_WRT_MAC_SUBPART_VER_MAX,
                                                               pss_wrt_mac_table);
           if(cbinfo->info.enc.o_msg_fmt_ver == 0)
           {
              /* log the error */
              m_LOG_PSS_HDLN_CLL(NCSFL_SEV_ERROR, PSS_HDLN_MDS_ENC_FAILURE, "PSS->MAC", \
                                 cbinfo->info.enc.i_to_svc_id,cbinfo->info.enc.o_msg_fmt_ver);
              return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
           } 
        }
        else if(cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_OAC)
        {
           cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
                                                               PSS_WRT_OAC_SUBPART_VER_MIN,
                                                               PSS_WRT_OAC_SUBPART_VER_MAX,
                                                               pss_wrt_oac_table);
           if(cbinfo->info.enc.o_msg_fmt_ver == 0)
           {
              /* log the error */
              m_LOG_PSS_HDLN_CLL(NCSFL_SEV_ERROR, PSS_HDLN_MDS_ENC_FAILURE, "PSS->OAC", \
                                cbinfo->info.enc.i_to_svc_id,cbinfo->info.enc.o_msg_fmt_ver);
              return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
           } 
        }
        else if(cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_BAM)
        {
           cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
                                                               PSS_WRT_BAM_SUBPART_VER_MIN,
                                                               PSS_WRT_BAM_SUBPART_VER_MAX,
                                                               pss_wrt_bam_table);
           if(cbinfo->info.enc.o_msg_fmt_ver == 0)
           {
              /* log the error */
              m_LOG_PSS_HDLN_CLL(NCSFL_SEV_ERROR, PSS_HDLN_MDS_ENC_FAILURE, "PSS->BAM", \
                                cbinfo->info.enc.i_to_svc_id,cbinfo->info.enc.o_msg_fmt_ver);
              return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
           }
        }
        status = mab_mds_enc(cbinfo->i_yr_svc_hdl,
                             cbinfo->info.enc.i_msg,
                             cbinfo->info.enc.i_to_svc_id,
                             cbinfo->info.enc.io_uba,
                             cbinfo->info.enc.o_msg_fmt_ver);
        break;

    case MDS_CALLBACK_DEC:
    case MDS_CALLBACK_DEC_FLAT:
        if(cbinfo->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_BAM)
        { 
           if(0 == m_NCS_MSG_FORMAT_IS_VALID(cbinfo->info.dec.i_msg_fmt_ver,
                                             PSS_WRT_BAM_SUBPART_VER_MIN,
                                             PSS_WRT_BAM_SUBPART_VER_MAX,
                                             pss_wrt_bam_table))
           {
             /* log the error */
              m_LOG_PSS_HDLN_CLL(NCSFL_SEV_ERROR, PSS_HDLN_MDS_DEC_FAILURE, "BAM->PSS", \
                            cbinfo->info.dec.i_fr_svc_id,cbinfo->info.dec.i_msg_fmt_ver);
             return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
           }
           status = pss_bam_decode_msg(cbinfo); 
        } 
        else 
        {
           if(cbinfo->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_OAC)
           {
              if(0 == m_NCS_MSG_FORMAT_IS_VALID(cbinfo->info.dec.i_msg_fmt_ver,
                                                PSS_WRT_OAC_SUBPART_VER_MIN,
                                                PSS_WRT_OAC_SUBPART_VER_MAX,
                                                pss_wrt_oac_table))
              {
                /* log the error */
                m_LOG_PSS_HDLN_CLL(NCSFL_SEV_ERROR, PSS_HDLN_MDS_DEC_FAILURE, "OAC->PSS", \
                               cbinfo->info.dec.i_fr_svc_id,cbinfo->info.dec.i_msg_fmt_ver);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
              }
              status = mab_mds_dec(cbinfo->i_yr_svc_hdl,
                                &cbinfo->info.dec.o_msg,
                                cbinfo->info.dec.i_fr_svc_id,
                                cbinfo->info.dec.io_uba,
                                cbinfo->info.dec.i_msg_fmt_ver);
           }
        }
        break;

    case MDS_CALLBACK_RECEIVE:
        status = pss_mds_grcv(cbinfo->info.receive.sender_pwe_hdl,
                             cbinfo->i_yr_svc_hdl,
                             cbinfo->info.receive.i_msg,
                             cbinfo->info.receive.i_fr_dest,
                             cbinfo->info.receive.i_fr_svc_id,
                             cbinfo->info.receive.i_to_dest,
                             cbinfo->info.receive.i_to_svc_id);
        break;

    case MDS_CALLBACK_SVC_EVENT:
        status = pss_mds_evt_cb(cbinfo);
        break;

    case MDS_CALLBACK_QUIESCED_ACK:
        status = pss_amf_vdest_role_chg_api_callback(cbinfo->i_yr_svc_hdl);
        break;

    default:
        status = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        break;
    }

    return status;
}


#endif /* (NCS_PSR == 1) */
#endif /* (NCS_MAB == 1) */
