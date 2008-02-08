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

  DESCRIPTION:This module does the send and receive of messages between 
  the Availability Director and BAM module.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_bam_snd_msg
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"



/****************************************************************************
  Name          : avd_bam_snd_message
 
  Description   : This is a routine that is invoked to send messages
                  to AVD.
 
  Arguments     : cb_hdl    - AvD control block Handle.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : 
******************************************************************************/
static uns32 avd_bam_snd_message(AVD_CL_CB *cb, AVD_BAM_MSG *snd_msg)
{
   NCSMDS_INFO snd_mds;
   uns32 rc;

   m_NCS_MEMSET(&snd_mds,'\0',sizeof(NCSMDS_INFO));

   snd_mds.i_mds_hdl = cb->vaddr_pwe_hdl;
   snd_mds.i_svc_id = NCSMDS_SVC_ID_AVD;
   snd_mds.i_op = MDS_SEND;
   snd_mds.info.svc_send.i_msg = (NCSCONTEXT)snd_msg;
   snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_BAM;
   snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
   snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
   snd_mds.info.svc_send.info.snd.i_to_dest = cb->bam_mds_dest;

   if( (rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS)
   {
      m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SEND);
      return rc;
   }

   m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_SEND);
   return NCSCC_RC_SUCCESS;
}
   

/****************************************************************************
*  Name          : avd_bam_snd_restart
* 
*  Description   : This is a routine that is invoked to send a restart message
*                  to BAM.
* 
*  Arguments     : cb_hdl    - AvD control block Handle.
* 
*  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
* 
*  Notes         : 
******************************************************************************/
uns32 avd_bam_snd_restart(AVD_CL_CB *cb)
{
   AVD_BAM_MSG *snd_msg = m_MMGR_ALLOC_AVD_BAM_MSG;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Fill in the message */
   m_NCS_MEMSET(snd_msg,'\0',sizeof(AVD_BAM_MSG));
   snd_msg->msg_type = AVD_BAM_MSG_RESTART;
   
   rc = avd_bam_snd_message(cb, snd_msg);

   if(rc != NCSCC_RC_SUCCESS)
      avsv_bam_msg_free(snd_msg);

   return rc;
}

/****************************************************************************
  Name          : avd_bam_mds_cpy
 
  Description   : This is a callback routine that is invoked to copy 
                  AvD-BAM messages. 
 
  Arguments     : cpy_info - ptr to the MDS copy info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avd_bam_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info)
{
   AVD_BAM_MSG *msg = cpy_info->i_msg;

   cpy_info->o_cpy = (NCSCONTEXT)msg;
   cpy_info->i_msg = 0; /* memory is not allocated and hence this */
   return NCSCC_RC_SUCCESS;
}

