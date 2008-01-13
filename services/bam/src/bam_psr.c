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

  This file captures the initialization of BOM Access Manager.
  
******************************************************************************
*/

/* NCS specific include files */

#include <gl_defs.h>
#include <t_suite.h>
#include <ncs_mib_pub.h>
#include <ncs_lib.h>
#include <ncsgl_defs.h>
#include <mds_papi.h>

#include "bam.h"
#include "bam_log.h"
#include "psr_bam.h"


EXTERN_C uns32 gl_ncs_bam_hdl;
/*****************************************************************************
 * Function: bam_psr_rcv_msg
 *
 * Purpose: This function processes the message from PSR and if 
 *          BAM is ready to parse will start doing so else it will
 *          have to wait for MAS MDS_UP message.
 * 
 *
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
uns32 bam_psr_rcv_msg(NCS_BAM_CB *bam_cb, PSS_BAM_MSG  *msg)
{
   BAM_EVT *evt;

   if(msg->i_evt == PSS_BAM_EVT_WARMBOOT_REQ)
   {
      evt = m_MMGR_ALLOC_BAM_DEFAULT_VAL(sizeof(BAM_EVT));
      if(evt == NULL)
      {
         /* LOG an error */
         m_MMGR_FREE_PSS_BAM_MSG(msg);
         return NCSCC_RC_FAILURE;
      }

      evt->cb_hdl = bam_cb->cb_handle;
      evt->evt_type = NCS_BAM_PSR_PLAYBACK;
      evt->msg.pss_msg = msg;

      if (m_NCS_IPC_SEND(bam_cb->bam_mbx,evt,NCS_IPC_PRIORITY_HIGH)
         != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_PSS_BAM_MSG(msg);
         m_MMGR_FREE_BAM_DEFAULT_VAL(evt);
         return NCSCC_RC_FAILURE;
      }
   }
   else
   {
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
*  Name          : pss_bam_snd_message
* 
*  Description   : This is a routine that is invoked to send a message
*                  to PSS.
* 
*  Arguments     : PSS_BAM_MSG - the message to be sent.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 pss_bam_snd_message(PSS_BAM_MSG *snd_msg)
{
   NCSMDS_INFO snd_mds;
   uns32 rc;
   NCS_BAM_CB   *bam_cb = NULL;

   if((bam_cb = (NCS_BAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_BAM, gl_ncs_bam_hdl)) == NULL)
   {
      m_LOG_BAM_HEADLINE(BAM_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
  
   m_NCS_MEMSET(&snd_mds,'\0',sizeof(NCSMDS_INFO));

   snd_mds.i_mds_hdl = bam_cb->mds_hdl;
   snd_mds.i_svc_id = NCSMDS_SVC_ID_BAM;
   snd_mds.i_op = MDS_SEND;
   snd_mds.info.svc_send.i_msg = (NCSCONTEXT)snd_msg;
   snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_PSS;
   snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
   snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
   snd_mds.info.svc_send.info.snd.i_to_dest = bam_cb->pss_dest;
   if( (rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS)
   {
      return rc;
   }

   ncshm_give_hdl(gl_ncs_bam_hdl);   

   /* m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_SEND); */
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
*  Name          : pss_bam_send_cfg_done_msg
* 
*  Description   : This is a routine that is invoked to send a config DONE
*                  message to PSS.
* 
*  Arguments     : NONE.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
void pss_bam_send_cfg_done_msg(BAM_PARSE_SUB_TREE subTree)
{
   PSS_BAM_MSG *snd_msg = m_MMGR_ALLOC_PSS_BAM_MSG;

   /* Fill in the message */
   m_NCS_MEMSET(snd_msg,'\0',sizeof(PSS_BAM_MSG));
   snd_msg->i_evt = PSS_BAM_EVT_CONF_DONE;
   if(subTree == BAM_PARSE_NCS)
      snd_msg->info.conf_done.pcn_list.pcn = "AVD";
   else if(subTree == BAM_PARSE_AVM)
      snd_msg->info.conf_done.pcn_list.pcn = "AVM";

   snd_msg->info.conf_done.pcn_list.tbl_list= NULL;

   if(pss_bam_snd_message(snd_msg) != NCSCC_RC_SUCCESS)
   {
      m_LOG_BAM_MDS_ERR(BAM_MDS_SEND_ERROR, NCSFL_SEV_ERROR);
      m_MMGR_FREE_PSS_BAM_MSG(snd_msg);
   }
   else
   {
      m_LOG_BAM_MDS_ERR(BAM_MDS_SEND_SUCCESS, NCSFL_SEV_INFO);
   }

   return;
}
