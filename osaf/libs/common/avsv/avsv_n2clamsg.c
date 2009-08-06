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

  This file contains utility routines for managing the messages between AvD & 
  CLA.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avsv.h"
#include "avsv_clmparam.h"
#include "avsv_n2clamsg.h"
#include "avsv_n2clamem.h"

/****************************************************************************
  Name          : avsv_nda_cla_msg_free
 
  Description   : This routine frees the CLA message.
 
  Arguments     : msg - ptr to the CLA msg
 
  Return Values : None
 
  Notes         : None.
******************************************************************************/
void avsv_nda_cla_msg_free(AVSV_NDA_CLA_MSG *msg)
{
	if (!msg) {
		return;
	}

	if (msg->type == AVSV_AVND_CLM_API_RESP_MSG) {
		if (msg->info.api_resp_info.param.track.num) {
			m_MMGR_FREE_AVSV_CLA_DEFAULT_VAL(msg->info.api_resp_info.param.track.notify);
		}
	} else if (msg->type == AVSV_AVND_CLM_CBK_MSG) {
		if (msg->info.cbk_info.type == AVSV_CLM_CBK_TRACK) {
			if (msg->info.cbk_info.param.track.mem_num) {
				m_MMGR_FREE_AVSV_CLA_DEFAULT_VAL(msg->info.cbk_info.param.track.notify.notification);
			}
		} else if (msg->info.cbk_info.type == AVSV_CLM_NODE_ASYNC_GET) {
			/* nothing */
		}
	}
	m_MMGR_FREE_AVSV_NDA_CLA_MSG(msg);
	return;
}

/****************************************************************************
  Name          : avsv_nda_cla_msg_copy
 
  Description   : This routine copies the CLA message.
 
  Arguments     : dmsg - ptr to the dest msg
                  smsg - ptr to the source msg
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None.
******************************************************************************/
uns32 avsv_nda_cla_msg_copy(AVSV_NDA_CLA_MSG *dmsg, AVSV_NDA_CLA_MSG *smsg)
{				/* TBD */
	uns32 rc = NCSCC_RC_SUCCESS;

	return rc;
}
