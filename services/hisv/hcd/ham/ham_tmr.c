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
 *                                                                           *
 *  MODULE NAME:  ham_tmr.c                                                  *
 *                                                                           *
 * DESCRIPTION                                                               *
 * This module does the initialisation of TIMER interface and                *
 * provides functionality for adding, deleting and expiry of timers.         *
 * The expiry routine packages the timer expiry information as local         *
 * event information and posts it to the main loop for processing.           *
 * currently this timer handles the periodical clearance of the HPI's        *
 * system event logging.                                                     *
 *                                                                           *
 *****************************************************************************/

#include "hcd.h"


/*****************************************************************************
 * Name            : ham_start_tmr
 *
 * Description     : This function creates the timer (if not already created)
 *                   and starts the HAM timer to clear the system event log
 *                   of HPI.
 *
 * Arguments       : HAM_CB - ptr to the HAM control block
 *                   period - timer period in nano seconds
 *
 * Returns Values  : NCSCC_RC_SUCCESS - Success
 *                   NCSCC_RC_FAILURE - Failure
 *
 * Notes           : None
 ****************************************************************************/

uns32 ham_start_tmr (HAM_CB *cb, SaTimeT period)
{

   /* create the timer (if not already created */
   if (cb->tmr_id == TMR_T_NULL)
   {
      m_NCS_TMR_CREATE (cb->tmr_id, period , ham_tmr_exp, (void*)cb->cb_hdl);
   }

   /* start the timer to execure ham_tmr_exp routine after expiry */
   m_NCS_TMR_START (cb->tmr_id, (uns32)period, ham_tmr_exp, (void*)cb->cb_hdl);

   if (TMR_T_NULL == cb->tmr_id)
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;

}


/*****************************************************************************
 * Name           : ham_stop_tmr
 *
 * Description    : Stops the system event log clear timer and destroys the
 *                  timer created.
 *
 * Arguments      : pointer to HAM control block
 *
 * Return Values  : void
 *
 * Notes          : None
 ****************************************************************************/

void ham_stop_tmr (HAM_CB *cb)
{

   /* Stop the timer */
   m_NCS_TMR_STOP (cb->tmr_id);

   /* Destroy the timer if it exists. */
   if (cb->tmr_id != TMR_T_NULL)
   {
      m_NCS_TMR_DESTROY(cb->tmr_id);
      cb->tmr_id = TMR_T_NULL;
   }

   return;
}

/*****************************************************************************
 * Name           : ham_tmr_exp
 *
 * Description    : HAM timer expiry callback routine. It sends corresponding
 *                  message to HAM to clear the system event log of HPI and
 *                  re-starts the timer to happen periodically.
 *
 * Arguments      : uarg - ptr to the HAM control block handle
 *
 * Return Values  : void
 *
 * Notes          : None
 ****************************************************************************/

void ham_tmr_exp (void *uarg)
{
   HISV_EVT * hisv_evt;
   HAM_CB  *cb = NULL;
   uns32 ham_cb_hdl = (uns32)uarg;

  /* clearing SEL is affecting other functionality of HPI so
   * for now, just return here
   */
   return;

   /* retrieve HAM CB */
   cb = (HAM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_HCD, ham_cb_hdl);
   if (cb == NULL)
   {
      /* return HAM CB handle */
      ncshm_give_hdl(ham_cb_hdl);
      return;
   }
   if (NULL == (hisv_evt = m_MMGR_ALLOC_HISV_EVT))
   {
      /* restart the timer */
      ham_start_tmr(cb, HPI_SYS_LOG_CLEAR_INTERVAL);
      /* return HAM CB handle */
      ncshm_give_hdl(ham_cb_hdl);
      return;
   }
   /* create the request message to clear system event log */
   m_HAM_HISV_LOG_CMD_MSG_FILL(hisv_evt->msg, HISV_TMR_SEL_CLR);

   /* send the request to HAM mailbox */
   if(m_NCS_IPC_SEND(&cb->mbx, hisv_evt, NCS_IPC_PRIORITY_NORMAL)
                     == NCSCC_RC_FAILURE)
   {
      m_LOG_HISV_DEBUG("failed to deliver msg on mail-box\n");
      hisv_evt_destroy(hisv_evt);
   }

   /* restart the timer */
   ham_start_tmr(cb, HPI_SYS_LOG_CLEAR_INTERVAL);
   /* return HAM CB handle*/
   ncshm_give_hdl(ham_cb_hdl);

   return;
}



