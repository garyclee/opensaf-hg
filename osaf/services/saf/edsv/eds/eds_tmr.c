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

  This file contains EDS timer interface routines.
 
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
*******************************************************************************/
#include "eds.h"

/*****************************************************************************
  PROCEDURE NAME : eds_start_tmr

  DESCRIPTION    : Starts the EDS timer. If the timer is already active, it 
                   is restarted (ie. stopped & started without reallocating the 
                   tmr block).

  ARGUMENTS      : cb     - ptr to the EDS control block
                   tmr    - ptr to the EDS timer block
                   type   - timer type
                   period - timer period
                   uarg   - opaque handle that is returned on timer expiry

  RETURNS        : NCSCC_RC_SUCCESS - Success
                   NCSCC_RC_FAILURE  - Failure

  NOTES         : None
*****************************************************************************/
uint32_t eds_start_tmr(EDS_CB *cb, EDS_TMR *tmr, EDS_TMR_TYPE type, SaTimeT period, uint32_t uarg)
{
	uint32_t tmr_period = (uint32_t)(period / EDSV_NANOSEC_TO_LEAPTM);

	if (EDS_TMR_MAX <= tmr->type) {
		m_LOG_EDSV_S(EDS_TIMER_START_FAIL, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, type, __FILE__, __LINE__,
			     tmr_period);
		return NCSCC_RC_FAILURE;
	}

	if (tmr->tmr_id == TMR_T_NULL) {
		tmr->type = type;
		m_NCS_TMR_CREATE(tmr->tmr_id, (uint32_t)tmr_period, eds_tmr_exp, (void *)tmr);
	}

	if (tmr->is_active == true) {
		m_NCS_TMR_STOP(tmr->tmr_id);
		tmr->is_active = false;
	}

	tmr->opq_hdl = uarg;
	tmr->cb_hdl = cb->my_hdl;
	m_NCS_TMR_START(tmr->tmr_id, (uint32_t)tmr_period, eds_tmr_exp, (void *)tmr);

	tmr->is_active = true;

	if (TMR_T_NULL == tmr->tmr_id) {
		m_LOG_EDSV_S(EDS_TIMER_START_FAIL, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, type, __FILE__, __LINE__,
			     tmr_period);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : eds_stop_tmr

  DESCRIPTION    : Stops the EDS timer.

  ARGUMENTS      : tmr    - ptr to the EDS timer block
               
  RETURNS        : void

  NOTES          : None
*****************************************************************************/
void eds_stop_tmr(EDS_TMR *tmr)
{
	/* If timer type is invalid just return */
	if (tmr == NULL) {
		m_LOG_EDSV_S(EDS_TIMER_STOP_FAIL, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return;
	} else if (EDS_TMR_MAX <= tmr->type) {
		m_LOG_EDSV_S(EDS_TIMER_STOP_FAIL, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, tmr->type, __FILE__, __LINE__,
			     0);
		return;
	}

	/* Stop the timer if it is active... */
	if (tmr->is_active == true) {
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
  PROCEDURE NAME : eds_tmr_evt_map
  DESCRIPTION    : Maps a timer type to the corresponding EDS evt type.
  ARGUMENTS      : tmr_type - timer type  
  RETURNS        : EDS event type
  NOTES          : None
*****************************************************************************/
static EDSV_EDS_EVT_TYPE eds_tmr_evt_map(EDS_TMR_TYPE tmr_type)
{
	switch (tmr_type) {
	case EDS_RET_EVT_TMR:
		return EDSV_EDS_RET_TIMER_EXP;
	default:
		return EDSV_EDS_EVT_MAX;
	}
}

/*****************************************************************************
  PROCEDURE NAME : eds_tmr_exp

  DESCRIPTION    : EDS timer expiry callback routine.It sends corresponding
                   timer events to EDS.

  ARGUMENTS      : uarg - ptr to the EDS timer block

  RETURNS        : void

  NOTES         : None
*****************************************************************************/
void eds_tmr_exp(void *uarg)
{
	EDS_CB *eds_cb = 0;
	EDS_TMR *tmr = (EDS_TMR *)uarg;
	EDSV_EDS_EVT *evt = 0;
	uint32_t temp_tmr_hdl;

	temp_tmr_hdl = tmr->cb_hdl;

	/* retrieve EDS CB */
	if (NULL == (eds_cb = (EDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_EDS, tmr->cb_hdl))) {
		m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return;
	}

	if (tmr->is_active) {
		tmr->is_active = false;
		/* Destroy the timer if it exists.. */
		if (tmr->tmr_id != TMR_T_NULL) {
			m_NCS_TMR_DESTROY(tmr->tmr_id);
			tmr->tmr_id = TMR_T_NULL;
		}

		/* create & send the timer event */
		evt = m_MMGR_ALLOC_EDSV_EDS_EVT;
		if (evt) {
			memset(evt, '\0', sizeof(EDSV_EDS_EVT));

			/* assign the timer evt */
			evt->evt_type = eds_tmr_evt_map(tmr->type);
			evt->info.tmr_info.opq_hdl = tmr->opq_hdl;

			evt->cb_hdl = tmr->cb_hdl;

			if (NCSCC_RC_FAILURE == m_NCS_IPC_SEND(&eds_cb->mbx, evt, NCS_IPC_PRIORITY_HIGH)) {
				m_LOG_EDSV_S(EDS_TIMER_STOP_FAIL, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, evt->cb_hdl,
					     __FILE__, __LINE__, evt->evt_type);
				eds_evt_destroy(evt);
			}

		}

	}

	/* return EDS CB */
	ncshm_give_hdl(temp_tmr_hdl);

	return;
}
