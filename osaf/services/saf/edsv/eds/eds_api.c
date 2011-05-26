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
        
This include file contains SE api instrumentation for EDS
          
*******************************************************************************/
#include <configmake.h>
#include "eds.h"
#include "logtrace.h"

/* global cb handle */
uint32_t gl_eds_hdl = 0;
/****************************************************************************
 * Name          : eds_se_lib_init
 *
 * Description   : Invoked to Initialize the EDS
 *                 
 *
 * Arguments     : 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uint32_t eds_se_lib_init(NCS_LIB_REQ_INFO *req_info)
{
	EDS_CB *eds_cb;
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* Register with the Logging subsystem */
	eds_flx_log_reg();

	/* Allocate and initialize the control block */
	if (NULL == (eds_cb = m_MMGR_ALLOC_EDS_CB)) {
		m_LOG_EDSV_S(EDS_MEM_ALLOC_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return NCSCC_RC_FAILURE;
	}
	memset(eds_cb, '\0', sizeof(EDS_CB));

	/* Obtain the hdl for EDS_CB from hdl-mgr */
	gl_eds_hdl = eds_cb->my_hdl = ncshm_create_hdl(1, NCS_SERVICE_ID_EDS, (NCSCONTEXT)eds_cb);

	if (0 == eds_cb->my_hdl) {
		m_LOG_EDSV_S(EDS_CB_CREATE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__,
			     0);
		ncshm_destroy_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl);
		gl_eds_hdl = 0;
		m_MMGR_FREE_EDS_CB(eds_cb);
		return NCSCC_RC_FAILURE;
	}

	/* initialize the eds cb lock */
	m_NCS_LOCK_INIT(&eds_cb->cb_lock);

	/* Initialize eds control block */
	if (NCSCC_RC_SUCCESS != (rc = eds_cb_init(eds_cb))) {
		m_LOG_EDSV_S(EDS_CB_INIT_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		/* Destroy the hdl for this CB */
		ncshm_destroy_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl);
		gl_eds_hdl = 0;
		/* clean up the CB */
		m_MMGR_FREE_EDS_CB(eds_cb);
		/* log the error */
		return rc;
	}

	m_LOG_EDSV_S(EDS_CB_INIT_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	m_NCS_EDU_HDL_INIT(&eds_cb->edu_hdl);

	/* Create the mbx to communicate with the EDS thread */
	if (NCSCC_RC_SUCCESS != (rc = m_NCS_IPC_CREATE(&eds_cb->mbx))) {
		m_LOG_EDSV_S(EDS_IPC_CREATE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__, 0);
		/* Release EDU handle */
		m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
		/* Destroy the hdl for this CB */
		ncshm_destroy_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl);
		gl_eds_hdl = 0;
		/* Free the control block */
		m_MMGR_FREE_EDS_CB(eds_cb);
		return rc;
	}
	/* Attach the IPC to the created thread */
	m_NCS_IPC_ATTACH(&eds_cb->mbx);
	m_LOG_EDSV_S(EDS_MAIL_BOX_CREATE_ATTACH_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	/* Bind to MDS */
	if (NCSCC_RC_SUCCESS != (rc = eds_mds_init(eds_cb))) {
		m_LOG_EDSV_S(EDS_MDS_INIT_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__, 0);
		m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL);
		/* Release EDU handle */
		m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
		ncshm_destroy_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl);
		gl_eds_hdl = 0;
		m_MMGR_FREE_EDS_CB(eds_cb);
		return rc;
	}
	m_LOG_EDSV_S(EDS_MDS_INIT_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	/* Initialize and Register with CLM */
	rc = eds_clm_init(eds_cb);
	if (rc != SA_AIS_OK) {
		m_LOG_EDSV_S(EDS_CLM_REGISTRATION_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR,
						     rc, __FILE__, __LINE__, 0);
		LOG_ER("CLM Init failed: Exiting.");
		exit(1);
	} else {
		m_LOG_EDSV_S(EDS_CLM_REGISTRATION_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE,
						     rc, __FILE__, __LINE__, 0);
	}

	/* Initialize and Register with AMF */
	rc = eds_amf_register(eds_cb);
	if (rc != NCSCC_RC_SUCCESS) {
		m_LOG_EDSV_S(EDS_AMF_REG_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc,
						     __FILE__, __LINE__, 0);
		LOG_ER("AMF Init failed: Exiting.");
		exit(1);
	} else {
	m_LOG_EDSV_S(EDS_AMF_REG_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, rc,
						     __FILE__, __LINE__, 0);
	}

	/* Initialize mbcsv interface */
	if (NCSCC_RC_SUCCESS != (rc = eds_mbcsv_init(eds_cb))) {
		m_LOG_EDSV_S(EDS_MBCSV_INIT_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__, 0);
		LOG_ER("eds_mbcsv_init() EDS MBCSV INIT FAILED");
		/* Log it */
	} else
		m_LOG_EDSV_S(EDS_MBCSV_INIT_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	m_LOG_EDSV_S(EDS_MAIN_PROCESS_START_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	return (rc);
}

/****************************************************************************
 * Name          : eds_clear_mbx
 *
 * Description   : This is the function which deletes all the messages from 
 *                 the mail box.
 *
 * Arguments     : arg     - argument to be passed.
 *                 msg     - Event start pointer.
 *
 * Return Values : true/false
 *
 * Notes         : None.
 *****************************************************************************/
static bool eds_clear_mbx(NCSCONTEXT arg, NCSCONTEXT msg)
{
	EDSV_EDS_EVT *pEvt = (EDSV_EDS_EVT *)msg;
	EDSV_EDS_EVT *pnext;
	pnext = pEvt;
	while (pnext) {
		pnext = ( /* (EDSV_EDS_EVT *)& */ (pEvt->next));
		eds_evt_destroy(pEvt);
		pEvt = pnext;
	}
	return true;
}

/****************************************************************************
 * Name          : eds_se_lib_destroy
 *
 * Description   : Invoked to destroy the EDS
 *                 
 *
 * Arguments     : 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uint32_t eds_se_lib_destroy(NCS_LIB_REQ_INFO *req_info)
{
    /** Code to destroy the EDS **/
	EDS_CB *eds_cb;
	m_INIT_CRITICAL;
	SaAisErrorT status = SA_AIS_OK;

	if (NULL == (eds_cb = (NCSCONTEXT)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl))) {
		m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return (NCSCC_RC_FAILURE);
	} else {
		m_START_CRITICAL;
      /** Lock EDA_CB
       **/
		m_NCS_LOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);

		/* deregister from AMF */
		status = saAmfComponentUnregister(eds_cb->amf_hdl, &eds_cb->comp_name, NULL);

		/* End association from the AMF lib */
		saAmfFinalize(eds_cb->amf_hdl);

		/* Finalize with CLM */
		saClmFinalize(eds_cb->clm_hdl);

		/* Clean up all internal structures */
		eds_remove_reglist_entry(eds_cb, 0, true);

		/* Destroy the cb */
		eds_cb_destroy(eds_cb);

		/* Give back the handle */
		ncshm_give_hdl(gl_eds_hdl);
		ncshm_destroy_hdl(NCS_SERVICE_ID_GLD, gl_eds_hdl);

		/* Detach from IPC */
		m_NCS_IPC_DETACH(&eds_cb->mbx, eds_clear_mbx, eds_cb);

		/* Disconnect from MDS */
		eds_mds_finalize(eds_cb);

		/* Release the IPC */
		m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL);

      /** UnLock EDA_CB
       **/
		m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
		m_NCS_LOCK_DESTROY(&eds_cb->cb_lock);
		m_MMGR_FREE_EDS_CB(eds_cb);

#if (NCS_EDSV_LOG == 1)
		eds_flx_log_dereg();
#endif
		gl_eds_hdl = 0;
		m_END_CRITICAL;
		TRACE("EDS-CB-LIB DESTROY CALLED.");
	}

	return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ncs_edsv_eds_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy EDS
 *                 module
 *                 
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t ncs_edsv_eds_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t rc = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		rc = eds_se_lib_init(req_info);
		break;
	case NCS_LIB_REQ_DESTROY:
		rc = eds_se_lib_destroy(req_info);
		break;
	default:
		break;
	}
	return (rc);
}

