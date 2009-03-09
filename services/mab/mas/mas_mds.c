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

  This file contains MAS specific MDS routines, being:

        mas_mds_rcv()
        mas_mds_evt_cb()
        mas_mds_cb()
        mas_mds_mbx_post()

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mab.h"
#include "ncs_util.h"

MDS_CLIENT_MSG_FORMAT_VER    mas_mac_msg_fmt_table[MAS_WRT_MAC_SUBPART_VER_RANGE] = {1,2};
MDS_CLIENT_MSG_FORMAT_VER    mas_oac_msg_fmt_table[MAS_WRT_OAC_SUBPART_VER_RANGE] = {1,2};

#if (NCS_MAB == 1)

/* processes the MDS UP/DOWN messages */
static uns32 
mas_mds_evt_cb(NCSMDS_CALLBACK_INFO * cbinfo);

/* posts message to MAS mailbox */ 
static uns32 
mas_mds_mbx_post(NCSMDS_CALLBACK_INFO *cbinfo, MAB_SVC_OP evt); 

/****************************************************************************\
*  Name:          mas_mds_rcv                                                * 
*                                                                            *
*  Description:   Posts a message to MAS mailbox from MDS thread.            * 
*                                                                            *
*  Arguments:     NCSCONTEXT - MAS CB, registered with MDS.                  *
*                 NCSCONTEXT - Pinter to the decoded MAB_MSG*                *
*                 MDS_DEST   - fr_card and to_card                           *
*                 SS_SVC_ID  - Service ids                                   *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32 
mas_mds_rcv(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg, 
            MDS_DEST  fr_card,    SS_SVC_ID fr_svc, V_DEST_QA fr_anc,
            MDS_DEST  to_card,    SS_SVC_ID to_svc)
{
    uns32   status;
    MAS_TBL *inst = (MAS_TBL*)m_MAS_VALIDATE_HDL((uns32)yr_svc_hdl);
    
    if (inst == NULL)
    {
        m_LOG_MAB_NO_CB("mas_mds_rcv()");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    
    if (inst)
    {
        m_LOG_MAB_SVC_PRVDR_MSG(NCSFL_SEV_INFO, MAB_SP_MAS_MDS_RCV_MSG, 
                                fr_svc, fr_card,((MAB_MSG*)msg)->op);
    }

    /* plant MAB subcomponent's control block in MAB_MSG */
    ((MAB_MSG*)msg)->yr_hdl = NCS_INT32_TO_PTR_CAST(inst->hm_hdl); 
    ((MAB_MSG*)msg)->fr_card = fr_card;
    if(((MAB_MSG*)msg)->fr_svc == 0)
        ((MAB_MSG*)msg)->fr_svc = fr_svc;

    /* fill in the anchor value */ 
    ((MAB_MSG*)msg)->fr_anc = fr_anc;

    status = m_MAS_SND_MSG(inst->mbx,msg); 
    if(status != NCSCC_RC_SUCCESS)
    {
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MDS_TO_MBX_SND_FAILED, 
                           status, ((MAB_MSG*)msg)->op);
        ncshm_give_hdl((uns32)yr_svc_hdl);
        return m_MAB_DBG_SINK(status);
    }

    ncshm_give_hdl((uns32)yr_svc_hdl);
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          mas_mds_evt_cb                                             * 
*                                                                            *
*  Description:   MAS received Service-ID Up/Down Events of the              *
*                 interested parties                                         *
*  Arguments:     NCSMDS_CALLBACK_INFO - Information from MDS thread         *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32 
mas_mds_evt_cb(NCSMDS_CALLBACK_INFO * cbinfo)
{
    uns32   status;
    MDS_CLIENT_HDL yr_svc_hdl = cbinfo->i_yr_svc_hdl;
    MAS_TBL* inst = (MAS_TBL*)m_MAS_VALIDATE_HDL((uns32)yr_svc_hdl);

    if (inst == NULL)
    {
        m_LOG_MAB_NO_CB("mas_mds_evt_cb()");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    m_LOG_MAB_SVC_PRVDR_EVT(NCSFL_SEV_DEBUG, MAB_SP_MAS_MDS_RCV_EVT, 
                          cbinfo->info.svc_evt.i_svc_id, 
                          cbinfo->info.svc_evt.i_dest,
                          cbinfo->info.svc_evt.i_change, 
                          cbinfo->info.svc_evt.i_anc);

    /* take action based on the evant type and the source */ 
    switch (cbinfo->info.svc_evt.i_change) 
    {
        case NCSMDS_RED_DOWN:
        {
          if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC)
          {
             /* OAC installed on a VDEST is dead */ 
             status = mas_mds_mbx_post(cbinfo, MAB_OAA_DOWN_EVT); 
          }
        }
        break;

        case NCSMDS_DOWN:
             if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC)
            {
                /* OAC installed on an ADEST is dead */ 
                /* You are here means, OAC on ADEST is dead */
                cbinfo->info.svc_evt.i_anc = cbinfo->info.svc_evt.i_dest;
                status = mas_mds_mbx_post(cbinfo, MAB_OAA_DOWN_EVT); 
            }
        break;

        case NCSMDS_UP:
            if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_MAS) /* Its ME! What VCARD do I live on?? */
                inst->my_vcard = cbinfo->info.svc_evt.i_dest;
        break;

        /* there is no active, so adjust the filters, accordingly */ 
        /* MAS will receive the NCSMDS_NO_ACTIVE, only in the following cases */ 
        /* 1. When the Active instance of an application is killed. */ 
        /* 2. ACTIVE ==> QUIESCED state transition */
        case NCSMDS_NO_ACTIVE:
            if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC)
            {
                /* unless this tweak is not done, mas_fltr_ids_adjust() will not adjust the filters */ 
                cbinfo->info.svc_evt.i_role = V_DEST_RL_QUIESCED;
            }
            /* purposeful fall through, I did not forget to put the break */
            
        /* interested in OAA's role change */
        case NCSMDS_CHG_ROLE:
            if (cbinfo->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_OAC)
               status = mas_mds_mbx_post(cbinfo, MAB_OAA_CHG_ROLE);
            break;

        default:
        break;
    }/* end of switch() */

    ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
    return NCSCC_RC_SUCCESS;
}
/****************************************************************************\
*  Name:          mas_mds_cb                                                 * 
*                                                                            *
*  Description:   Posts a message to OAA mailbox from MDS thread.            * 
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO - Information from MDS thread         *         
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 mas_mds_cb(NCSMDS_CALLBACK_INFO *cbinfo)
{
    uns32 status = NCSCC_RC_SUCCESS;

    if (cbinfo == NULL)
    {
        m_LOG_MAB_SVC_PRVDR(NCSFL_SEV_ERROR, MAB_SP_MAS_MDS_CBINFO_NULL, 0, 0);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    switch(cbinfo->i_op)
    {
    case MDS_CALLBACK_COPY:
        status = mab_mds_cpy(cbinfo->i_yr_svc_hdl,
                             cbinfo->info.cpy.i_msg,
                             cbinfo->info.cpy.i_to_svc_id,
                             &cbinfo->info.cpy.o_cpy,
                             cbinfo->info.cpy.i_last);
        break;

    case MDS_CALLBACK_ENC:
    case MDS_CALLBACK_ENC_FLAT:
        if(cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_MAC)
            cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
                                                             MAS_WRT_MAC_SUBPART_VER_AT_MIN_MSG_FMT,
                                                             MAS_WRT_MAC_SUBPART_VER_AT_MAX_MSG_FMT,
                                                             mas_mac_msg_fmt_table);
        else if(cbinfo->info.enc.i_to_svc_id == NCSMDS_SVC_ID_OAC)
            cbinfo->info.enc.o_msg_fmt_ver = m_NCS_ENC_MSG_FMT_GET(cbinfo->info.enc.i_rem_svc_pvt_ver,
                                                             MAS_WRT_OAC_SUBPART_VER_AT_MIN_MSG_FMT,
                                                             MAS_WRT_OAC_SUBPART_VER_AT_MAX_MSG_FMT,
                                                             mas_oac_msg_fmt_table);
        if(cbinfo->info.enc.o_msg_fmt_ver == 0)
        {
           /* log the error */
           m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE,MAB_HDLN_MAS_LOG_MDS_ENC_FAILURE,\
                             cbinfo->info.enc.i_to_svc_id,cbinfo->info.enc.o_msg_fmt_ver);
           return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
        status = mab_mds_enc(cbinfo->i_yr_svc_hdl,
                             cbinfo->info.enc.i_msg,
                             cbinfo->info.enc.i_to_svc_id,
                             cbinfo->info.enc.io_uba,
                             cbinfo->info.enc.o_msg_fmt_ver);
        break;

    case MDS_CALLBACK_DEC:
    case MDS_CALLBACK_DEC_FLAT:

         if(cbinfo->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_MAC)
         {    
           if(0 == m_NCS_MSG_FORMAT_IS_VALID(cbinfo->info.dec.i_msg_fmt_ver,
                                             MAS_WRT_MAC_SUBPART_VER_AT_MIN_MSG_FMT,
                                             MAS_WRT_MAC_SUBPART_VER_AT_MAX_MSG_FMT,
                                             mas_mac_msg_fmt_table))
           {
              /* log the error */
              m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE,MAB_HDLN_MAS_LOG_MDS_DEC_FAILURE,\
                                cbinfo->info.dec.i_fr_svc_id,cbinfo->info.dec.i_msg_fmt_ver);
              return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
           }
         }  
         else if(cbinfo->info.dec.i_fr_svc_id == NCSMDS_SVC_ID_OAC)
         {    
           if(0 == m_NCS_MSG_FORMAT_IS_VALID(cbinfo->info.dec.i_msg_fmt_ver,
                                             MAS_WRT_OAC_SUBPART_VER_AT_MIN_MSG_FMT,
                                             MAS_WRT_OAC_SUBPART_VER_AT_MAX_MSG_FMT,
                                             mas_oac_msg_fmt_table))
           {
              /* log the error */
              m_LOG_MAB_HDLN_II(NCSFL_SEV_NOTICE,MAB_HDLN_MAS_LOG_MDS_DEC_FAILURE,\
                                cbinfo->info.dec.i_fr_svc_id,cbinfo->info.dec.i_msg_fmt_ver);
              return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
           }
         }  
        status = mab_mds_dec(cbinfo->i_yr_svc_hdl,
                             &cbinfo->info.dec.o_msg,
                             cbinfo->info.dec.i_fr_svc_id,
                             cbinfo->info.dec.io_uba,
                             cbinfo->info.dec.i_msg_fmt_ver);
        break;

    case MDS_CALLBACK_RECEIVE:
        status = mas_mds_rcv(cbinfo->i_yr_svc_hdl,
                             cbinfo->info.receive.i_msg,
                             cbinfo->info.receive.i_fr_dest,
                             cbinfo->info.receive.i_fr_svc_id,
                             cbinfo->info.receive.i_fr_anc,
                             cbinfo->info.receive.i_to_dest,
                             cbinfo->info.receive.i_to_svc_id);
        break;

    case MDS_CALLBACK_SVC_EVENT:
        status = mas_mds_evt_cb(cbinfo);
        break;

    /* MDS has completed informing the world that, MAS is in QUISCED state */ 
    /* Time for MAS to inform the AMF about its successful state transition */
    case MDS_CALLBACK_QUIESCED_ACK:
        status = mas_mds_mbx_post(cbinfo, MAB_SVC_VDEST_ROLE_QUIESCED); 
        break; 
        
    default:
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_MDS_CB_INVALID_OP, 
                         cbinfo->i_op);
        status = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        break;
    }

    return status;
}

/****************************************************************************\
*  Name:          mas_mds_mbx_post                                           * 
*                                                                            *
*  Description:   Posts a message to MAS mailbox                             *
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO  *cbinfo - Information from MDS       * 
*                 MAB_SVC_OP            evt     - type of event              *   
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*                 NCSCC_RC_OUT_OF_MEM - Memory allocation failure            *
*                 NCSCC_RC_FAILURE  - If there is no control block           *
\****************************************************************************/
static uns32 
mas_mds_mbx_post(NCSMDS_CALLBACK_INFO *cbinfo, MAB_SVC_OP evt) 
{
    uns32       status; 
    MAB_MSG     *mm = NULL;
    MDS_CLIENT_HDL  yr_svc_hdl = cbinfo->i_yr_svc_hdl;
    MAS_TBL     *inst = (MAS_TBL*)m_MAS_VALIDATE_HDL((uns32)yr_svc_hdl);

    if (inst == NULL)
        return NCSCC_RC_FAILURE; 

    mm = m_MMGR_ALLOC_MAB_MSG;
    if(mm == NULL)
        return (NCSCC_RC_OUT_OF_MEM); 
      
    m_NCS_MEMSET(mm, 0, sizeof(MAB_MSG)); 
    mm->yr_hdl = NCS_INT32_TO_PTR_CAST(inst->hm_hdl);  
    mm->fr_card = cbinfo->info.svc_evt.i_dest; 
    mm->fr_svc = cbinfo->info.svc_evt.i_svc_id; 
    mm->fr_anc = cbinfo->info.svc_evt.i_anc; 
    mm->fr_role = cbinfo->info.svc_evt.i_role; 
    mm->op = evt; 

    /* send the message to MAS mailbox */
    status = m_MAS_SND_MSG(inst->mbx, mm);
    if (status != NCSCC_RC_SUCCESS)
    {
        m_MMGR_FREE_MAB_MSG(mm); 
        return status; 
    }
    
    ncshm_give_hdl((uns32)cbinfo->i_yr_svc_hdl);
    return status;
}

#endif /* (NCS_MAB == 1) */

