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

#include "ava.h"

/****************************************************************************
  Name          : ava_avnd_msg_prc
 
  Description   : This routine process the messages from AvND.
 
  Arguments     : cb  - ptr to the AvA control block
                  msg - ptr to the AvND message
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 ava_avnd_msg_prc(AVA_CB *cb, AVSV_NDA_AVA_MSG *msg)
{
	AVA_HDL_REC *hdl_rec = 0;
	AVSV_AMF_CBK_INFO *cbk_info = 0;
	uns32 hdl = 0;
	uns32 rc = NCSCC_RC_SUCCESS;
	TRACE_ENTER();

	/* 
	 * AvA receives either AVSV_AVND_AMF_CBK_MSG or AVSV_AVND_AMF_API_RESP_MSG 
	 * from AvND. Response to APIs is handled by synchronous blocking calls. 
	 * Hence, in this flow, the message type can only be AVSV_AVND_AMF_CBK_MSG.
	 */
	assert(msg->type == AVSV_AVND_AMF_CBK_MSG);

	/* get the callbk info */
	cbk_info = msg->info.cbk_info;

	/* convert csi attributes into the form that amf-cbk understands */
	if ((AVSV_AMF_CSI_SET == cbk_info->type) && (SA_AMF_CSI_ADD_ONE == cbk_info->param.csi_set.csi_desc.csiFlags)) {
		rc = avsv_amf_csi_attr_convert(cbk_info);
		if (NCSCC_RC_SUCCESS != rc) {
			TRACE_2("csi attributes convertion failed");
			goto done;
		}
	}

	/* retrieve the handle record */
	hdl = cbk_info->hdl;
	hdl_rec = (AVA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_AVA, hdl);
	if (hdl_rec) {
		/* push the callbk parameters in the pending callbk list */
		rc = ava_hdl_cbk_param_add(cb, hdl_rec, cbk_info);
		if (NCSCC_RC_SUCCESS == rc) {
			/* => the callbk info ptr is used in the pend callbk list */
			msg->info.cbk_info = 0;
		} else
			TRACE_2("Handle callback param add failed");

		/* return the handle record */
		ncshm_give_hdl(hdl);
	}

 done:
	TRACE_LEAVE();
	return rc;
}
