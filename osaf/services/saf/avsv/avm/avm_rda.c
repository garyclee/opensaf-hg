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

  DESCRIPTION: This module contains interface with RDE.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avm_rda_initialize
  avm_register_rda_cb
  avm_unregister_rda_cb
  avm_rda_cb
  avm_notify_rde_set_role
    
******************************************************************************
*/
#include "avm.h"

#define PRINT_ROLE(role)\
{\
    (role == 0)?m_NCS_DBG_PRINTF("\n %s %s \n", "RDA giving role to avm : ", "PCS_RDA_ACTIVE"):\
    (role == 1)?m_NCS_DBG_PRINTF("\n %s %s \n", "RDA giving role to avm : ", "PCS_RDA_STANDBY"):\
                m_NCS_DBG_PRINTF("\n %s %d \n", "RDA giving role to avm : ", role);\
}

static void avm_rda_cb(uns32 cb_hdl, PCS_RDA_CB_INFO *cb_info, PCSRDA_RETURN_CODE error_code);

/***********************************************************************
 ******
 * Name          : avm_rda_initialize
 *
 * Description   : This function  initiailizes rda interface 
 *                 and gets init role
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : During initialization Quiesced is ignored and hence
 *                 ignored even in the switch statement.
*********************************************************************
 *****/
extern uns32 avm_rda_initialize(AVM_CB_T *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	PCS_RDA_REQ pcs_rda_req;

	m_AVM_LOG_FUNC_ENTRY("avm_rda_initialize");

	pcs_rda_req.req_type = PCS_RDA_LIB_INIT;
	if (PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req)) {
		return NCSCC_RC_FAILURE;
	}

	pcs_rda_req.callback_handle = cb->cb_hdl;
	pcs_rda_req.req_type = PCS_RDA_REGISTER_CALLBACK;
	pcs_rda_req.info.call_back = avm_rda_cb;
	if (PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req)) {
		return NCSCC_RC_FAILURE;
	}

	pcs_rda_req.req_type = PCS_RDA_GET_ROLE;
	if (PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req)) {
		return NCSCC_RC_FAILURE;
	}

	m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_GET_ROLE, pcs_rda_req.info.io_role, NCSFL_SEV_INFO);

	switch (pcs_rda_req.info.io_role) {
	case PCS_RDA_ACTIVE:
		{
			cb->ha_state = SA_AMF_HA_ACTIVE;
		}
		break;

	case PCS_RDA_STANDBY:
		{
			cb->ha_state = SA_AMF_HA_STANDBY;
		}
		break;

	default:
		{
			m_AVM_LOG_INVALID_VAL_ERROR(pcs_rda_req.info.io_role);
			rc = NCSCC_RC_FAILURE;
			break;
		}
	}

	return rc;
}

/******************************************************************
 * Name          : avm_register_rda_cb
 *
 * Description   : This function registers rda callback.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32 avm_register_rda_cb(AVM_CB_T *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	PCS_RDA_REQ pcs_rda_req;

	pcs_rda_req.callback_handle = cb->cb_hdl;
	pcs_rda_req.req_type = PCS_RDA_REGISTER_CALLBACK;
	pcs_rda_req.info.call_back = avm_rda_cb;

	if (PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req)) {
		return NCSCC_RC_FAILURE;
	}

	m_AVM_LOG_ROLE(AVM_LOG_RDA_AVM_SET_ROLE, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_NOTICE);

	return rc;

}

/******************************************************************
 * Name          : avm_unregister_rda_cb
 *
 * Description   : This function unregister rda callback.
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32 avm_unregister_rda_cb(AVM_CB_T *cb)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	PCS_RDA_REQ pcs_rda_req;

	pcs_rda_req.req_type = PCS_RDA_UNREGISTER_CALLBACK;

	if (PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req)) {
		return NCSCC_RC_FAILURE;
	}

	m_AVM_LOG_ROLE(AVM_LOG_RDA_AVM_SET_ROLE, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_NOTICE);

	return rc;

}

/******************************************************************
 * Name          : avm_notify_rde_set_role
 *
 * Description   : This function informs role to RDE
 *
 * Arguments     : AVM_CB_T*    - Pointer to AvM CB
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
extern uns32 avm_notify_rde_set_role(AVM_CB_T *cb, SaAmfHAStateT role)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	PCS_RDA_REQ pcs_rda_req;

	pcs_rda_req.req_type = PCS_RDA_SET_ROLE;

	if (role == SA_AMF_HA_ACTIVE)
		pcs_rda_req.info.io_role = PCS_RDA_ACTIVE;
	else if (role == SA_AMF_HA_STANDBY)
		pcs_rda_req.info.io_role = PCS_RDA_STANDBY;
	else if (role == SA_AMF_HA_QUIESCED)
		pcs_rda_req.info.io_role = PCS_RDA_QUIESCED;
	else
		pcs_rda_req.info.io_role = role;

	if (PCSRDA_RC_SUCCESS != pcs_rda_request(&pcs_rda_req)) {
		return NCSCC_RC_FAILURE;
	}

	m_AVM_LOG_ROLE(AVM_LOG_RDA_AVM_SET_ROLE, AVM_LOG_RDA_SUCCESS, NCSFL_SEV_NOTICE);

	return rc;

}

/******************************************************************
 * Name          : avm_rda_cb
 *
 * Description   : This function is used by RDE to give a role to 
 *                 AvM
 *
 * Arguments     : uns32              - callback handle
 *                 PCS_RDA_CB_INFO    - callback info
 *                 PCSRDA_RETURN_CODE - error code
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None
*********************************************************************
 *****/
static void avm_rda_cb(uns32 cb_hdl, PCS_RDA_CB_INFO *cb_info, PCSRDA_RETURN_CODE error_code)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	AVM_CB_T *cb;
	AVM_EVT_T *avm_evt;
	PCS_RDA_ROLE role;

	role = cb_info->info.io_role;
	PRINT_ROLE(role);

	if (AVM_CB_NULL == (cb = ncshm_take_hdl(NCS_SERVICE_ID_AVM, cb_hdl))) {
		m_AVM_LOG_CB(AVM_LOG_CB_RETRIEVE, AVM_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		return;
	}

	m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_SET_ROLE, cb_info->info.io_role, NCSFL_SEV_NOTICE);
	m_AVM_LOG_ROLE_OP(AVM_LOG_RDA_AVM_ROLE, cb->ha_state, NCSFL_SEV_NOTICE);

	avm_evt = (AVM_EVT_T *)m_MMGR_ALLOC_AVM_EVT;

	avm_evt->src = AVM_EVT_RDE;
	avm_evt->fsm_evt_type = AVM_ROLE_EVT_RDE_SET;
	avm_evt->evt.rde_evt.role = cb_info->info.io_role;
	avm_evt->evt.rde_evt.ret_code = error_code;

	if (m_NCS_IPC_SEND(&cb->mailbox, avm_evt, NCS_IPC_PRIORITY_HIGH)
	    == NCSCC_RC_FAILURE) {
		m_AVM_LOG_MBX(AVM_LOG_MBX_SEND, AVM_LOG_MBX_FAILURE, NCSFL_SEV_CRITICAL);
		m_MMGR_FREE_AVM_EVT(avm_evt);
		rc = NCSCC_RC_FAILURE;
	}

	ncshm_give_hdl(g_avm_hdl);

	return;
}
