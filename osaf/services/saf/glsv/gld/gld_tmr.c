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

  This file contains GND timer interface routines.
 
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include <logtrace.h>

#include "gld.h"

/*****************************************************************************
  PROCEDURE NAME : gld_start_tmr

  DESCRIPTION    : Starts the GLD timer. If the timer is already active, it 
                   is restarted (ie. stopped & started without reallocating the 
                   tmr block).

  ARGUMENTS      : cb     - ptr to the GLD control block
                  tmr    - ptr to the GLD timer block
                  type    - timer type
                  period - timer period
                  uarg   - opaque handle that is returned on timer expiry

  RETURNS        : NCSCC_RC_SUCCESS - Success
               NCSCC_RC_FAILURE  - Failure

  NOTES         : None
*****************************************************************************/
uint32_t gld_start_tmr(GLSV_GLD_CB *cb, GLD_TMR *tmr, GLD_TMR_TYPE type, SaTimeT period, uint32_t uarg)
{
	uint32_t my_period = (uint32_t)(m_GLSV_CONVERT_SATIME_TEN_MILLI_SEC(period));

	if (tmr == NULL)
		return NCSCC_RC_FAILURE;

	if (GLD_TMR_MAX <= type) {
		m_LOG_GLD_TIMER(GLD_TIMER_START_FAIL, type, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	if (tmr->tmr_id == TMR_T_NULL) {
		tmr->type = type;
		tmr->cb_hdl = cb->my_hdl;
		m_NCS_TMR_CREATE(tmr->tmr_id, my_period, gld_tmr_exp, (void *)tmr);
	}

	if (tmr->is_active == true) {
		m_NCS_TMR_STOP(tmr->tmr_id);
		tmr->is_active = false;
	}

	tmr->opq_hdl = uarg;
	tmr->cb_hdl = cb->my_hdl;
	m_NCS_TMR_START(tmr->tmr_id, my_period, gld_tmr_exp, (void *)tmr);
	tmr->is_active = true;

	if (TMR_T_NULL == tmr->tmr_id) {
		m_LOG_GLD_TIMER(GLD_TIMER_START_FAIL, type, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	TRACE("Started GLD Timer for %d @ %d ticks", type, my_period);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : gld_stop_tmr

  DESCRIPTION    : Stops the GLD timer.

  ARGUMENTS      : tmr    - ptr to the GLD timer block
               
  RETURNS        : void

  NOTES         : None
*****************************************************************************/
void gld_stop_tmr(GLD_TMR *tmr)
{
	/* If timer type is invalid just return */
	if (tmr == NULL) {
		m_LOG_GLD_TIMER(GLD_TIMER_STOP_FAIL, 0, __FILE__, __LINE__);
		return;
	}
	if (tmr != NULL && GLD_TMR_MAX <= tmr->type) {
		m_LOG_GLD_TIMER(GLD_TIMER_STOP_FAIL, tmr->type, __FILE__, __LINE__);
		return;
	}

	/* Stop the timer if it is active... */
	if (tmr->is_active == true) {
		TRACE("Stopped GLD Timer for %d", tmr->type);
		m_NCS_TMR_STOP(tmr->tmr_id);
		tmr->is_active = false;
	}

	/* Destroy the timer if it exists.. */
	if (tmr->tmr_id != TMR_T_NULL) {
		m_NCS_TMR_DESTROY(tmr->tmr_id);
		tmr->tmr_id = TMR_T_NULL;
	}
	return;
}

/*****************************************************************************
  PROCEDURE NAME : gld_tmr_evt_map
  DESCRIPTION    : Maps a timer type to the corresponding GLD evt type.
  ARGUMENTS      : tmr_type - timer type  
  RETURNS        : GLD event type
  NOTES         : None
*****************************************************************************/
static GLSV_GLD_EVT_TYPE gld_tmr_evt_map(GLD_TMR_TYPE tmr_type)
{
	switch (tmr_type) {
	case GLD_TMR_NODE_RESTART_TIMEOUT:
		return GLSV_GLD_EVT_RESTART_TIMEOUT;
	case GLD_TMR_RES_REELECTION_WAIT:
		return GLSV_GLD_EVT_REELECTION_TIMEOUT;

	default:
		return GLSV_GLD_EVT_MAX;
	}
}

/*****************************************************************************
  PROCEDURE NAME : gld_tmr_exp

  DESCRIPTION    : GLD timer expiry callback routine.It sends corresponding
                  timer events to GLD.

  ARGUMENTS      : uarg - ptr to the GLD timer block

  RETURNS        : void

  NOTES         : None
*****************************************************************************/
void gld_tmr_exp(void *uarg)
{
	GLSV_GLD_CB *cb = 0;
	GLD_TMR *tmr = (GLD_TMR *)uarg;
	GLSV_GLD_EVT *evt = 0;
	uint32_t cb_hdl;

	cb_hdl = tmr->cb_hdl;
	/* retrieve GLD CB */
	cb = (GLSV_GLD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_GLD, tmr->cb_hdl);
	if (!cb) {
		m_LOG_GLD_HEADLINE(GLD_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__, 0);
		return;
	}

	tmr->is_active = false;

	/* create & send the timer event */
	evt = m_MMGR_ALLOC_GLSV_GLD_EVT;
	if (evt == GLSV_GLD_EVT_NULL) {
		m_LOG_GLD_MEMFAIL(GLD_EVT_ALLOC_FAILED, __FILE__, __LINE__);
		ncshm_give_hdl(cb_hdl);
		return;
	}
	memset(evt, 0, sizeof(GLSV_GLD_EVT));
	if (evt) {
		/* assign the timer evt */
		evt->evt_type = gld_tmr_evt_map(tmr->type);
		evt->info.tmr.opq_hdl = tmr->opq_hdl;
		evt->info.tmr.resource_id = tmr->resource_id;
		memcpy(&evt->info.tmr.mdest_id, &tmr->mdest_id, sizeof(MDS_DEST));
		evt->gld_cb = cb;
		/* Push the event and we are done */
		if (m_NCS_IPC_SEND(&cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL) == NCSCC_RC_FAILURE) {
			m_LOG_GLD_HEADLINE(GLD_IPC_SEND_FAIL, NCSFL_SEV_ERROR, __FILE__, __LINE__, 0);
			gld_evt_destroy(evt);
			ncshm_give_hdl(cb_hdl);
			return;
		}

	}

	/* return GLD CB */
	ncshm_give_hdl(cb_hdl);

	return;
}
