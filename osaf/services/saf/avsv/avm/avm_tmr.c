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

  DESCRIPTION:This module does the initialisation of TIMER interface and
  provides functionality for adding, deleting and expiry of timers.
  The expiry routine packages the timer expiry information as local
  event information and posts it to the main loop for processing.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avm_start_tmr - Starts the AvM timer.
  avm_stop_tmr - Stops the AvM timer.
  avm_tmr_exp - AvM timer expiry callback routine.
  avm_start_ssu_tmr - Starts the AvM SSU timer.
  avm_ssu_tmr_exp - AvM SSU timer expiry callback routine.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avm.h"

/*****************************************************************************
  PROCEDURE NAME : avm_start_tmr

  DESCRIPTION    : Starts the AvM timer.

  ARGUMENTS      :
                     tmr    - ptr to the AvM timer block
                     period - timer period in nano seconds

  RETURNS        : NCSCC_RC_SUCCESS - Success
                   NCSCC_RC_FAILURE - Failure

  NOTES         : The timer related info needs to be filled in the
  timer block before calling this routine.
*****************************************************************************/
uns32 avm_start_tmr(AVM_CB_T *avm_cb, AVM_TMR_T *tmr, SaTimeT period)
{
	uns32 tmr_period = period;

	/* tmr_period = (uns32)(period / AVM_NANOSEC_TO_LEAPTM); */

	tmr_period = (uns32)period;

	/* Check if this is a valid timer and status of the timer is RUNNING *
	 * in this case return failure */
	if ((AVM_TMR_MAX <= tmr->type) || (tmr->status == AVM_TMR_RUNNING)) {
		m_AVM_LOG_INVALID_VAL_ERROR(tmr->type);
		return NCSCC_RC_FAILURE;
	}

	if (tmr->tmr_id == TMR_T_NULL) {
		m_NCS_TMR_CREATE(tmr->tmr_id, tmr_period, avm_tmr_exp, (void *)tmr);
		m_AVM_LOG_TMR(AVM_LOG_TMR_CREATED, tmr_period, AVM_LOG_TMR_SUCCESS, NCSFL_SEV_INFO);
	}

	m_NCS_TMR_START(tmr->tmr_id, tmr_period, avm_tmr_exp, (void *)tmr);
	tmr->status = AVM_TMR_RUNNING;

	m_AVM_LOG_TMR(AVM_LOG_TMR_STARTED, tmr_period, AVM_LOG_TMR_SUCCESS, NCSFL_SEV_INFO);

	if (TMR_T_NULL == tmr->tmr_id) {
		m_AVM_LOG_TMR(AVM_LOG_TMR_STARTED, tmr_period, AVM_LOG_TMR_FAILURE, NCSFL_SEV_CRITICAL);
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME : avm_stop_tmr

  DESCRIPTION    : Stops the AvM timer.

  ARGUMENTS      : tmr    - ptr to the AvM timer block
  RETURNS        :

  NOTES          : None
*****************************************************************************/
void avm_stop_tmr(AVM_CB_T *cb, AVM_TMR_T *tmr)
{

	/* If timer type is invalid just return */
	if ((AVM_TMR_MAX <= tmr->type) || (AVM_TMR_INIT_EDA > tmr->type)) {
		m_AVM_LOG_INVALID_VAL_ERROR(tmr->type);
		return;
	}

	if (tmr->status == AVM_TMR_RUNNING) {
		tmr->status = AVM_TMR_STOPPED;
		m_NCS_TMR_STOP(tmr->tmr_id);
		m_AVM_LOG_TMR(AVM_LOG_TMR_STOPPED, AVM_INIT_EDA_INTVL, AVM_LOG_TMR_SUCCESS, NCSFL_SEV_INFO);
	} else {
		m_AVM_LOG_TMR_IGN(AVM_LOG_TMR_STOPPED, "Already stopped.Ignore it", NCSFL_SEV_INFO);
	}

	/* Destroy the timer if it exists.. */
	if (tmr->tmr_id != TMR_T_NULL) {
		m_NCS_TMR_DESTROY(tmr->tmr_id);
		tmr->tmr_id = TMR_T_NULL;
		m_AVM_LOG_TMR(AVM_LOG_TMR_DESTROYED, AVM_INIT_EDA_INTVL, AVM_LOG_TMR_SUCCESS, NCSFL_SEV_INFO);
	}
	return;
}

/*****************************************************************************
  PROCEDURE NAME : avm_tmr_exp

  DESCRIPTION    : AvM timer expiry dispatched event to AvM mail box.

  ARGUMENTS      : uarg - ptr to the AvM timer block

  RETURNS        : void

  NOTES         : None
*****************************************************************************/
void avm_tmr_exp(void *uarg)
{
	AVM_CB_T *cb = AVM_CB_NULL;;
	AVM_TMR_T *tmr = (AVM_TMR_T *)uarg;
	AVM_EVT_T *evt = AVM_EVT_NULL;

	m_AVM_LOG_FUNC_ENTRY("avm_tmr_exp");

	/* retrieve AvM CB */
	cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, tmr->cb_hdl);
	if (cb == AVM_CB_NULL) {
		m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		return;
	}

	/*
	 * Check if this timer was stopped after "avd_tmr_exp" was called
	 * but before entering this code.
	 */

	if (AVM_TMR_STOPPED == tmr->status) {
		ncshm_give_hdl(tmr->cb_hdl);
		return;
	}

	if ((AVM_TMR_MAX <= tmr->type) || (AVM_TMR_INIT_EDA > tmr->type)) {
		/* Invalid timer type. */
		m_AVM_LOG_INVALID_VAL_ERROR(tmr->type);
		return;
	}

	tmr->status = AVM_TMR_EXPIRED;

	/* create & send the timer event */
	evt = m_MMGR_ALLOC_AVM_EVT;
	if (evt != AVM_EVT_NULL) {
		memset(evt, '\0', sizeof(AVM_EVT_T));

		evt->src = AVM_TMR_EVTS_START + (tmr->type - 1);

		/* If it is a EDA timer then set the FSM event */
		if (tmr->type == AVM_TMR_INIT_EDA)
			evt->fsm_evt_type = AVM_EVT_TMR_INIT_EDA_EXP;

		evt->evt.tmr = *tmr;

		/* Send this timer event to my mail-box for processing */
		if (m_NCS_IPC_SEND(&cb->mailbox, evt, NCS_IPC_PRIORITY_HIGH)
		    != NCSCC_RC_SUCCESS) {
			m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
		}
	} else {
		m_AVM_LOG_MEM(AVM_LOG_EVT_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
	}

	/* return AvM CB handle */
	ncshm_give_hdl(tmr->cb_hdl);

	return;
}
