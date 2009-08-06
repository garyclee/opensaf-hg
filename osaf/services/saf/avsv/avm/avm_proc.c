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

  DESCRIPTION:This module is an infinite loop running to receive all the events
  from external entities
  ..............................................................................

  Function Included in this Module:

  avm_proc     -

******************************************************************************
*/

#include "avm.h"

/*****************************************************************************
 * Name          : avm_proc
 *
 * Description   : This function receives all the messages posted to AvM
 *                 from AvD, HiSV, MASv.
 *
 * Arguments     : None
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ******************************************************************************/
extern uns32 avm_proc(void)
{
	AVM_CB_T *avm_cb;
	AVM_EVT_T *avm_evt;
	NCS_SEL_OBJ_SET temp_sel_obj_set;
	NCS_SEL_OBJ temp_eda_sel_obj;
	NCS_SEL_OBJ temp_mbc_sel_obj;
	NCS_SEL_OBJ temp_fma_sel_obj;
	uns32 rc = NCSCC_RC_SUCCESS;
	uns32 msg;

	m_AVM_LOG_FUNC_ENTRY("avm_proc");
	USE(msg);

	avm_cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);

	if (AVM_CB_NULL == avm_cb) {
		m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		return NCSCC_RC_FAILURE;
	}
	m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	m_SET_FD_IN_SEL_OBJ(avm_cb->mbc_sel_obj, temp_mbc_sel_obj);

	m_NCS_SEL_OBJ_ZERO(&temp_sel_obj_set);

	if (avm_cb->eda_init == AVM_EDA_DONE) {
		m_SET_FD_IN_SEL_OBJ(avm_cb->eda_sel_obj, temp_eda_sel_obj);
		m_NCS_SEL_OBJ_SET(temp_eda_sel_obj, &avm_cb->sel_obj_set);
		avm_cb->sel_high = m_GET_HIGHER_SEL_OBJ(temp_eda_sel_obj, avm_cb->sel_high);
	}

	m_SET_FD_IN_SEL_OBJ(avm_cb->fma_sel_obj, temp_fma_sel_obj);
	m_NCS_SEL_OBJ_SET(temp_fma_sel_obj, &avm_cb->sel_obj_set);
	avm_cb->sel_high = m_GET_HIGHER_SEL_OBJ(temp_fma_sel_obj, avm_cb->sel_high);

	temp_sel_obj_set = avm_cb->sel_obj_set;

	ncshm_give_hdl(g_avm_hdl);

	/*Infinite loop here to process the msgs in AvM mailbox */
	while ((m_NCS_SEL_OBJ_SELECT(avm_cb->sel_high, &temp_sel_obj_set, NULL, NULL, NULL) != -1)) {
		if (m_NCS_SEL_OBJ_ISSET(avm_cb->mbx_sel_obj, &temp_sel_obj_set)) {
			m_AVM_LOG_DEBUG("MBX evt received", NCSFL_SEV_INFO);
			avm_cb = (AVM_CB_T *)ncshm_take_hdl(NCS_SERVICE_ID_AVM, g_avm_hdl);

			if (AVM_CB_NULL == avm_cb) {
				m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
				continue;
			}
			m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

			avm_evt = (AVM_EVT_T *)m_NCS_IPC_NON_BLK_RECEIVE(&(avm_cb->mailbox), msg);
			if (AVM_EVT_NULL == avm_evt) {
				m_AVM_LOG_INVALID_VAL_FATAL(0);
			} else {
				rc = avm_msg_handler(avm_cb, avm_evt);
			}
			ncshm_give_hdl(g_avm_hdl);
		}

		if (avm_cb->eda_init == AVM_EDA_DONE) {
			m_SET_FD_IN_SEL_OBJ(avm_cb->eda_sel_obj, temp_eda_sel_obj);
			if (m_NCS_SEL_OBJ_ISSET(temp_eda_sel_obj, &temp_sel_obj_set)) {
				m_AVM_LOG_DEBUG("EDS evt received", NCSFL_SEV_INFO);
				saEvtDispatch(avm_cb->eda_hdl, SA_DISPATCH_ONE);
			}
		}

		if (m_NCS_SEL_OBJ_ISSET(temp_mbc_sel_obj, &temp_sel_obj_set)) {
			m_AVM_LOG_DEBUG("MBCSV evt received", NCSFL_SEV_INFO);
			avm_mbc_dispatch(avm_cb, SA_DISPATCH_ONE);
		}

		if (m_NCS_SEL_OBJ_ISSET(temp_fma_sel_obj, &temp_sel_obj_set)) {
			m_AVM_LOG_DEBUG("Recieved evt from FM", NCSFL_SEV_INFO);
			fmDispatch(avm_cb->fm_hdl, SA_DISPATCH_ONE);
		}

		temp_sel_obj_set = avm_cb->sel_obj_set;
	}

	m_AVM_LOG_DEBUG("AVM Thread Exited", NCSFL_SEV_CRITICAL);
	syslog(LOG_CRIT, "NCS_AvSv: AvM Functional Thread Exited");

	return NCSCC_RC_SUCCESS;
}
