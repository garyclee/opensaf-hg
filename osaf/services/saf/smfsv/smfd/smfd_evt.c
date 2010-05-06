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
 * Author(s): Ericsson AB
 *
 */

#include <alloca.h>
#include <time.h>
#include <limits.h>

#include <saClm.h>

#include <ncs_main_papi.h>

#include "smfd.h"
#include "smfd_smfnd.h"
#include "smfd_evt.h"
#include "smfsv_defs.h"
#include "smfsv_evt.h"

/****************************************************************************
 * Name          : proc_mds_info
 *
 * Description   : Handle a mds info event 
 *
 * Arguments     : cb  - SMFND control block  
 *                 evt - The MDS_INFO event
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void proc_mds_info(smfd_cb_t * cb, SMFSV_EVT * evt)
{
	smfsv_mds_info *mds_info = &evt->info.smfd.event.mds_info;

	switch (mds_info->change) {

	case NCSMDS_RED_UP:
		/* get the peer mds_red_up */

		break;

	case NCSMDS_UP:

		if (mds_info->svc_id == NCSMDS_SVC_ID_SMFD) {
			return;
		}

		if (mds_info->svc_id == NCSMDS_SVC_ID_SMFND) {
			smfnd_up(mds_info->node_id, mds_info->dest);
		}
		break;

	case NCSMDS_DOWN:

		if (mds_info->svc_id == NCSMDS_SVC_ID_SMFD) {
			return;
		}

		if (mds_info->svc_id == NCSMDS_SVC_ID_SMFND) {
			smfnd_down(mds_info->node_id);
		}
		break;

	default:

		break;
	}

	return;
}

/****************************************************************************
 * Name          : smfd_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 SMFD 
 *
 * Arguments     : mbx  - This is the mail box pointer on which SMFD is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void smfd_process_mbx(SYSF_MBX * mbx)
{
	SMFSV_EVT *evt = (SMFSV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt);

	if (evt != NULL) {
		if (evt->type != SMFSV_EVT_TYPE_SMFD) {
			LOG_ER("SMFD received wrong event type %d", evt->type);
			goto err;
		}

		switch (evt->info.smfd.type) {
		case SMFD_EVT_MDS_INFO:
			{
				proc_mds_info(smfd_cb, evt);
				break;
			}
		case SMFD_EVT_CMD_RSP:
			{
				/* The CMD RSP is always received synchronized so skip it here */
				break;
			}
		default:
			{
				LOG_ER("SMFND received unknown event %d",
				       evt->info.smfnd.type);
				break;
			}
		}
	}

 err:
	smfsv_evt_destroy(evt);
}
