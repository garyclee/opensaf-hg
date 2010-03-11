/*       OpenSAF 
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

#include "smfnd.h"
#include "smfsv_defs.h"
#include "smfsv_evt.h"
#include "smfnd_evt.h"

static void proc_cmd_req(smfnd_cb_t * cb, SMFSV_EVT * evt);

/****************************************************************************
 * Name          : proc_cmd_req
 *
 * Description   : Handle a command request 
 *
 * Arguments     : cb  - SMFND control block  
 *                 evt - The CMD_REQ event
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void proc_cmd_req(smfnd_cb_t * cb, SMFSV_EVT * evt)
{
	int cmd_result;
	SMFSV_EVT result_evt;
	uns32 rc;

	TRACE_ENTER2("dest %llx", evt->fr_dest);

	if ((evt->info.smfnd.event.cmd_req.cmd_len == 0) || (evt->info.smfnd.event.cmd_req.cmd == NULL)) {
		LOG_ER("SMFND received empty command");
		goto err;
	}

	TRACE("Executing command: %s", evt->info.smfnd.event.cmd_req.cmd);
	cmd_result = system(evt->info.smfnd.event.cmd_req.cmd);

	memset(&result_evt, 0, sizeof(SMFSV_EVT));
	result_evt.type = SMFSV_EVT_TYPE_SMFD;
	result_evt.info.smfd.type = SMFD_EVT_CMD_RSP;
	result_evt.info.smfd.event.cmd_rsp.result = cmd_result;

	TRACE("Command %s responded %d", evt->info.smfnd.event.cmd_req.cmd, cmd_result);

	/* Send result back to sender */
	rc = smfsv_mds_send_rsp(cb->mds_handle,
				evt->mds_ctxt,
				evt->fr_svc, evt->fr_dest, NCSMDS_SVC_ID_SMFND, cb->mds_dest, &result_evt);
	if (rc != NCSCC_RC_SUCCESS) {
		LOG_ER("proc_cmd_req: send CMD RSP FAILED\n");
	}
 err:

	TRACE_LEAVE();
}

/****************************************************************************
 * Name          : smfnd_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 SMFND 
 *
 * Arguments     : mbx  - This is the mail box pointer on which SMFND is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void smfnd_process_mbx(SYSF_MBX * mbx)
{
	SMFSV_EVT *evt = (SMFSV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt);

	if (evt != NULL) {
		if (evt->type != SMFSV_EVT_TYPE_SMFND) {
			LOG_ER("SMFND received wrong event type %d", evt->type);
			goto done;
		}

		switch (evt->info.smfnd.type) {
		case SMFND_EVT_CMD_REQ:
			{
				proc_cmd_req(smfnd_cb, evt);
				break;
			}
		default:
			{
				LOG_ER("SMFND received unknown event %d", evt->info.smfnd.type);
				break;
			}
		}
	}

 done:
	smfsv_evt_destroy(evt);
}
