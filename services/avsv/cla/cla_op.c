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

  This file contains routines for miscellaneous operations.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "cla.h"


/****************************************************************************
  Name          : cla_avnd_msg_prc
 
  Description   : This routine process the messages from AvND.
 
  Arguments     : cb  - ptr to the CLA control block
                  msg - ptr to the AvND message
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 cla_avnd_msg_prc (CLA_CB *cb, AVSV_NDA_CLA_MSG *msg)
{
   CLA_HDL_REC       *hdl_rec = 0;
   AVSV_CLM_CBK_INFO *cbk_info = 0;
   uns32             hdl = 0, size=0;
   uns32             rc = NCSCC_RC_SUCCESS;

   /* 
    * CLA receives either AVSV_AVND_CLM_CBK_MSG or AVSV_AVND_CLM_API_RESP_MSG 
    * from AvND. Response to APIs is handled by synchronous blocking calls. 
    * Hence, in this flow, the message type can only be AVSV_AVND_CLM_CBK_MSG.
    */
   m_AVSV_ASSERT(msg->type == AVSV_AVND_CLM_CBK_MSG);

   /* get the callbk info */
   cbk_info = m_MMGR_ALLOC_AVSV_CLM_CBK_INFO;
   if(!cbk_info)
      return NCSCC_RC_FAILURE;
   
   m_NCS_MEMCPY(cbk_info, &msg->info.cbk_info, sizeof(AVSV_CLM_CBK_INFO));

   if(cbk_info->type == AVSV_CLM_CBK_TRACK)
   {
      cbk_info->param.track.notify.notification = 0;
      size = (msg->info.cbk_info.param.track.notify.numberOfItems) * 
                   (sizeof (SaClmClusterNotificationT));
      cbk_info->param.track.notify.notification = 
                   (SaClmClusterNotificationT *)malloc(size); 

      if (0 == cbk_info->param.track.notify.notification)
      {
         rc = NCSCC_RC_FAILURE;
         goto done;
      }

      m_NCS_MEMCPY(cbk_info->param.track.notify.notification, 
                   msg->info.cbk_info.param.track.notify.notification, 
                   size);
   }

   hdl = cbk_info->hdl;

   /* retrieve the handle record */
   hdl_rec = (CLA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl);

   if(hdl_rec)
   {
      /* push the callbk parameters in the pending callbk list */
      rc = cla_hdl_cbk_param_add(cb, hdl_rec, cbk_info);
      if (NCSCC_RC_SUCCESS != rc)
         goto done;

         /* => the callbk info ptr is used in the pend callbk list */
         cbk_info = 0;

      /* return the handle record */
      ncshm_give_hdl(hdl);
      return rc;
   }

done:
   /* Free the above allocated mem */
   if(hdl_rec) ncshm_give_hdl(hdl);
   
   if(cbk_info->param.track.notify.notification)
      free(cbk_info->param.track.notify.notification);
   
   if(cbk_info) m_MMGR_FREE_AVSV_CLM_CBK_INFO(cbk_info);

   return rc;
}
