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

  DESCRIPTION:

  This file contains all library creation APIs for the Mbessage Based 
  Checkpointing Service (MBCSV).

  ..............................................................................

  FUNCTIONS INCLUDED in this module:

  PUBLIC:
  mbcsv_lib_req                  - SE API to create MBCSV.

  PRIVATE:
  mbcsv_lib_init                 - Create MBCSV service.
  mbcsv_lib_destroy              - Distroy MBCSV service.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mbcsv.h"

MBCSV_CB mbcsv_cb;

/****************************************************************************
 * PROCEDURE: mbcsv_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy or 
 *                 Create/destroy MBCSV. This will be called by SBOM.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : SA_AIS_OK/Error code..
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t mbcsv_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = SA_AIS_OK;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = mbcsv_lib_init(req_info);
		break;

	case NCS_LIB_REQ_DESTROY:
		res = mbcsv_lib_destroy();
		break;

	default:
		res = m_MBCSV_DBG_SINK(SA_AIS_ERR_INVALID_PARAM,
				       "Lib init request failed: Bad operation type supplied");
		break;
	}
	return (res);
}

/****************************************************************************
 * PROCEDURE: mbcsv_lib_init
 *
 * Description   : This is the function which initalize the mbcsv libarary.
 *                 This function creates an global lock, registers with DTSV,
 *                 creates MBCSV linked list, etc.
 *
 * Arguments     : req_info - Request info.
 *
 * Return Values : SA_AIS_OK/Failure code.
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t mbcsv_lib_init(NCS_LIB_REQ_INFO *req_info)
{
	NCS_PATRICIA_PARAMS pt_params;
	uint32_t rc = SA_AIS_OK;

	if (mbcsv_cb.created == TRUE) {
		return m_MBCSV_DBG_SINK(SA_AIS_ERR_INIT, "Lib init request failed: MBCA already created.");
	}

	/*
	 * Create global lock 
	 */
	m_NCS_LOCK_INIT(&mbcsv_cb.global_lock);

	/* 
	 * Create patricia tree for the MBCA registration instance 
	 */
	pt_params.key_size = sizeof(uint32_t);

	if (ncs_patricia_tree_init(&mbcsv_cb.reg_list, &pt_params) != NCSCC_RC_SUCCESS) {
		rc = m_MBCSV_DBG_SINK(SA_AIS_ERR_FAILED_OPERATION, "Lib init request failed.");
		goto err1;
	}

	if (NCSCC_RC_SUCCESS != mbcsv_initialize_mbx_list()) {
		rc = m_MBCSV_DBG_SINK(SA_AIS_ERR_FAILED_OPERATION, "Lib init request failed.");
		goto err2;
	}

	/* 
	 * Create patricia tree for the peer list 
	 */
	if (mbcsv_initialize_peer_list() != NCSCC_RC_SUCCESS) {
		rc = m_MBCSV_DBG_SINK(SA_AIS_ERR_FAILED_OPERATION, "Lib init request failed.");
		goto err3;
	}
#if (MBCSV_LOG == 1)
	/*
	 * Subscribe with DTSv for logging.
	 */
	if (NCSCC_RC_SUCCESS != mbcsv_log_bind()) {
		rc = m_MBCSV_DBG_SINK(SA_AIS_ERR_FAILED_OPERATION, "Lib init request failed.");
		goto err4;
	}
#endif

	mbcsv_cb.created = TRUE;

	return rc;

	/* Handle Different Error Situations */
#if (MBCSV_LOG == 1)
 err4:
	ncs_patricia_tree_destroy(&mbcsv_cb.peer_list);
	m_NCS_LOCK_DESTROY(&mbcsv_cb.peer_list_lock);
#endif
 err3:
	ncs_patricia_tree_destroy(&mbcsv_cb.mbx_list);
	m_NCS_LOCK_DESTROY(&mbcsv_cb.mbx_list_lock);
 err2:
	ncs_patricia_tree_destroy(&mbcsv_cb.reg_list);
 err1:
	m_NCS_LOCK_DESTROY(&mbcsv_cb.global_lock);
	return rc;
}

/****************************************************************************
 * PROCEDURE     : mbcsv_lib_destroy
 *
 * Description   : This is the function which destroys the MBCSV.  
 *
 * Arguments     : None.
 *
 * Return Values : SA_AIS_OK/Failure code.
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t mbcsv_lib_destroy(void)
{
	MBCSV_REG *mbc_reg;
	SS_SVC_ID svc_id = 0;
	SaAisErrorT rc = SA_AIS_OK;

	if (mbcsv_cb.created == FALSE) {
		return m_MBCSV_DBG_SINK(SA_AIS_ERR_EXIST, "Lib destroy request failed: Create MBCA before destroying.");
	}

	m_NCS_LOCK(&mbcsv_cb.global_lock, NCS_LOCK_WRITE);
	m_LOG_MBCSV_GL_LOCK(MBCSV_LK_LOCKED, &mbcsv_cb.global_lock);

	mbcsv_cb.created = FALSE;
	/* 
	 * Walk through MBCSv reg list and destroy all the registration instances.
	 */
	while (NULL != (mbc_reg = (MBCSV_REG *)ncs_patricia_tree_getnext(&mbcsv_cb.reg_list, (const uint8_t *)&svc_id))) {
		svc_id = mbc_reg->svc_id;

		if (NCSCC_RC_SUCCESS != mbcsv_rmv_reg_inst((MBCSV_REG *)&mbcsv_cb.reg_list, mbc_reg)) {
			/* Not required to return for failure, log the err message and go 
			   ahead with cleanup */
			m_MBCSV_DBG_SINK_SVC(SA_AIS_ERR_LIBRARY,
					     "Failed to remove this service instance", mbc_reg->svc_id);
		}
	}

	ncs_patricia_tree_destroy(&mbcsv_cb.reg_list);

	/*
	 * Call function which will destroy and free all the entries of the peer list.
	 */
	mbcsv_destroy_peer_list();

	/*
	 * Call function which will destroy mail box list.
	 */
	mbcsv_destroy_mbx_list();

#if (MBCSV_LOG == 1)
	/*
	 * Un-Subscribe with DTSv for logging.
	 */
	if (NCSCC_RC_SUCCESS != mbcsv_log_unbind()) {
		/* Not required to return for failure, log the err message and go 
		   ahead with cleanup */
		m_MBCSV_DBG_SINK(SA_AIS_ERR_FAILED_OPERATION, "Lib init request failed.");
	}
#endif

	m_NCS_UNLOCK(&mbcsv_cb.global_lock, NCS_LOCK_WRITE);
	m_NCS_LOCK_DESTROY(&mbcsv_cb.global_lock);

	return rc;
}
