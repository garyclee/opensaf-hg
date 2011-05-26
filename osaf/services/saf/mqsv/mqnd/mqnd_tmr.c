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
  FILE NAME: mqsv_tmr.c

  DESCRIPTION: MQND Timer Processing Routines

******************************************************************************/

#include "mqnd.h"

static void mqnd_timer_expiry(NCSCONTEXT uarg);

/****************************************************************************
 * Name          : mqnd_cleanup_timer_expiry
 *
 * Description   : This function which is registered with the OS tmr function,
 *                 which will post a message to the corresponding mailbox 
 *                 depending on the component type.
 *
 *****************************************************************************/

static void mqnd_timer_expiry(NCSCONTEXT uarg)
{
	MQND_TMR *tmr = (MQND_TMR *)uarg;
	NCS_IPC_PRIORITY priority = NCS_IPC_PRIORITY_HIGH;
	MQND_CB *cb;
	MQSV_EVT *evt;
	uint32_t mqnd_hdl;

	if (tmr != NULL) {
		mqnd_hdl = tmr->uarg;

		if (tmr->is_active)
			tmr->is_active = FALSE;
		/* Destroy the timer if it exists.. */
		if (tmr->tmr_id != TMR_T_NULL) {
			m_NCS_TMR_DESTROY(tmr->tmr_id);
			tmr->tmr_id = TMR_T_NULL;
		}

		/* post a message to the corresponding component */
		if ((cb = (MQND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQND, mqnd_hdl))
		    != NULL) {
			if (cb->mqa_timer.type == MQND_TMR_TYPE_MQA_EXPIRY) {
				m_LOG_MQSV_ND(MQND_MQA_TMR_EXPIRED, NCSFL_LC_TIMER, NCSFL_SEV_NOTICE, NCSCC_RC_SUCCESS,
					      __FILE__, __LINE__);
			}
			evt = m_MMGR_ALLOC_MQSV_EVT(NCS_SERVICE_ID_MQND);
			if (evt == NULL) {
				m_LOG_MQSV_ND(MQND_EVT_ALLOC_FAILED, NCSFL_LC_TIMER, NCSFL_SEV_ERROR, NCSCC_RC_FAILURE,
					      __FILE__, __LINE__);
				return;
			}
			evt->evt_type = MQSV_NOT_DSEND_EVENT;
			evt->type = MQSV_EVT_MQND_CTRL;
			evt->msg.mqnd_ctrl.type = MQND_CTRL_EVT_TMR_EXPIRY;
			evt->msg.mqnd_ctrl.info.tmr_info.qhdl = tmr->qhdl;
			evt->msg.mqnd_ctrl.info.tmr_info.type = tmr->type;
			evt->msg.mqnd_ctrl.info.tmr_info.tmr_id = tmr->tmr_id;

			/* Post the event to MQND Thread */
			m_NCS_IPC_SEND(&cb->mbx, evt, priority);

			ncshm_give_hdl(mqnd_hdl);
		}
	}
	return;
}

/****************************************************************************
 * Name          : mqnd_tmr_start
 *
 * Description   : This function which is used to start the MQND Timer
 *
 *****************************************************************************/
uint32_t mqnd_tmr_start(MQND_TMR *tmr, SaTimeT duration)
{
	m_LOG_MQSV_ND(MQND_TMR_STARTED, NCSFL_LC_TIMER, NCSFL_SEV_INFO, NCSCC_RC_SUCCESS, __FILE__, __LINE__);
	if (tmr->tmr_id == TMR_T_NULL) {
		m_NCS_TMR_CREATE(tmr->tmr_id, duration, mqnd_timer_expiry, (void *)tmr);
	}

	if (tmr->is_active == FALSE) {
		m_NCS_TMR_START(tmr->tmr_id, (uint32_t)duration, mqnd_timer_expiry, (void *)tmr);
		tmr->is_active = TRUE;
	}

	return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : mqnd_tmr_stop
 *
 * Description   : This function which is used to stop the MQND Timer
 *
 * Arguments     : tmr      - Timer needs to be stoped.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void mqnd_tmr_stop(MQND_TMR *tmr)
{
	if (tmr->is_active == TRUE) {
		m_NCS_TMR_STOP(tmr->tmr_id);
		tmr->is_active = FALSE;
	}
	if (tmr->tmr_id != TMR_T_NULL) {
		m_NCS_TMR_DESTROY(tmr->tmr_id);
		tmr->tmr_id = TMR_T_NULL;
	}
	return;
}
