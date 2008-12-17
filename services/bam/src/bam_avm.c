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

  This file captures all the routines for communication from BAM to AVM
  
******************************************************************************
*/

#include "bam.h"
#include "bam_log.h"

EXTERN_C uns32 gl_ncs_bam_hdl;
EXTERN_C uns32 gl_bam_avm_cfg_msg_num;
/****************************************************************************
*  Name          : avm_bam_snd_message
* 
*  Description   : This is a routine that is invoked to send a message
*                  to AVM.
* 
*  Arguments     : AVM_BAM_MSG - the message to be sent.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 bam_avm_snd_message(BAM_AVM_MSG_T *snd_msg)
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
   snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_AVM;
   snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
   snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
   snd_mds.info.svc_send.info.snd.i_to_dest = bam_cb->avm_dest;
   if( (rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS)
   {
      /*m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SEND);*/
      return rc;
   }

   ncshm_give_hdl(gl_ncs_bam_hdl);   

   /* m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_SEND); */
   return NCSCC_RC_SUCCESS;
}




/****************************************************************************
*  Name          : ncs_bam_avm_send_cfg_done_msg
* 
*  Description   : This is a routine that is invoked to send a config DONE
*                  message to AVM.
* 
*  Arguments     : NONE.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
void ncs_bam_avm_send_cfg_done_msg()
{
   BAM_AVM_MSG_T *snd_msg = m_MMGR_ALLOC_BAM_AVM_MSG;

   /* Fill in the message */
   m_NCS_MEMSET(snd_msg,'\0',sizeof(BAM_AVM_MSG_T));
   snd_msg->msg_type = BAM_AVM_CFG_DONE_MSG;
   snd_msg->msg_info.msg.check_sum = gl_bam_avm_cfg_msg_num;

   if(bam_avm_snd_message(snd_msg) != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_BAM_AVM_MSG(snd_msg);
   }
   return;
}
