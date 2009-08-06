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

  This file contains Create/Destroy Public APIs for the Persistent Store & 
  Restore (PSS) portion of the MIB Acess Broker (MAB) subsystem.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if(NCS_MAB == 1)
#if(NCS_PSR == 1)
#include "psr.h"
#include "ncs_mda_pvt.h"
#include "psr_rfmt.h"

/* definition of the global variable, which holds the information 
 * abou the whole of PSS 
 */
PSS_ATTRIBS gl_pss_amf_attribs;

MABPSR_API uns32 pss_lib_init(void);
MABPSR_API uns32 pss_lib_shut(void);
MABPSR_API uns32 pss_ts_create(NCSPSS_CREATE *, uns32 ps_format_version);
MABPSR_API uns32 pss_ts_destroy(uns32 pss_hdl);
MABPSR_API uns32 pss_create(NCSPSS_CREATE *);
MABPSR_API uns32 dummy_mab_lm_cbfn(MAB_LM_EVT *);

extern MABPSR_API uns32 ncs_pssts_api(NCS_PSSTS_ARG *);

/****************************************************************************\
  Name          : ncspss_lib_req
 
  Description   : This is the NCS SE API which is used to init/destroy or 
                  Create/destroy PSS's. This will be called by SBOM.
 
  Arguments     : req  - This is the pointer to the input information which
                         SBOM gives.  
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 
  Notes         : None.
\*****************************************************************************/
uns32 ncspss_lib_req(NCS_LIB_REQ_INFO *req)
{
	uns32 rc = NCSCC_RC_FAILURE;

	/* validate the input */
	if (req == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	/* see, how can PSS help the caller? */
	switch (req->i_op) {
		/* create PSS */
	case NCS_LIB_REQ_CREATE:
		rc = pss_lib_init();
		break;

		/* destroy PSS */
	case NCS_LIB_REQ_DESTROY:
		rc = pss_lib_shut();
		break;

		/* Sorry, PSS can not help the caller */
	default:
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		break;
	}

	/* return to the caller */
	return rc;
}

/****************************************************************************\
  Name          : pss_lib_init
 
  Description   : This is the function which initialize the PSS libarary.
                  This function creates an IPC mail Box and spawns PSS
                  thread.                  
 
  Arguments     : Nothing
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
\*****************************************************************************/
uns32 pss_lib_init()
{
	uns32 rc;
	NCSPSS_CREATE pwe;

	memset(&pwe, 0, sizeof(NCSPSS_CREATE));

	/* bind with DTA/Tracing service */
	if (pss_dtsv_bind() != NCSCC_RC_SUCCESS) {
		/* sorry, can not log this error */
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* log the PSSv version */
	m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_PSS_VERSION, m_PSS_SERVICE_VERSION);

	/* create PSS mailbox */
	rc = m_NCS_IPC_CREATE(&gl_pss_amf_attribs.mbx_details.pss_mbx);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR, PSS_ERR_IPC_CREATE_FAILED);

		/* unbind with DTA */
		pss_dtsv_unbind();
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* attach the IPC */
	if (m_NCS_IPC_ATTACH(&gl_pss_amf_attribs.mbx_details.pss_mbx) != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR, PSS_ERR_IPC_ATTACH_FAILED);

		/* unbind with DTS */
		pss_dtsv_unbind();

		/* relase the IPC */
		m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);

		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Store the Mailbox pointer in amf_init_timer structure */
	gl_pss_amf_attribs.amf_attribs.amf_init_timer.mbx = &gl_pss_amf_attribs.mbx_details.pss_mbx;

	/* initialize the signal handler */
	if ((ncs_app_signal_install(SIGUSR1, pss_amf_sigusr1_handler)) == -1) {
		/* log the error */
		m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_FUNC_RET_FAIL, PSS_ERR_SIG_INSTALL_FAILED);

		/*unbind with DTA */
		pss_dtsv_unbind();

		/* relase the IPC */
		m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);
		(void)m_NCS_IPC_DETACH(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL, NULL);

		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Create the selection object associated with USR1 signal handler */
	m_NCS_SEL_OBJ_CREATE(&gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj);

	/* Create PSS Target Service (File System/FLASH/SQA DB?) */
	rc = pss_ts_create(&pwe, PSS_PS_FORMAT_VERSION);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_TGT_SVSC_CREATE_FAILED, rc);

		/* unbind with DTA */
		pss_dtsv_unbind();

		/* stop the task */
		m_NCS_TASK_STOP(gl_pss_amf_attribs.mbx_details.pss_task_hdl);

		/* release the task */
		m_NCS_TASK_RELEASE(gl_pss_amf_attribs.mbx_details.pss_task_hdl);

		/* release the IPC */
		m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);
		(void)m_NCS_IPC_DETACH(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL, NULL);

		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* update the target services handle into the global variable */
	gl_pss_amf_attribs.handles.pssts_hdl = pwe.i_pssts_hdl;

	/* upload the mailbox pointer an LM CREATE input */
	pwe.i_mbx = &gl_pss_amf_attribs.mbx_details.pss_mbx;

	/* create PSS for defailt environement */
	rc = pss_create(&pwe);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_DEF_ENV_CREATE_FAILED, rc);

		(void)pss_ts_destroy(gl_pss_amf_attribs.handles.pssts_hdl);

		/* unbind with DTA */
		pss_dtsv_unbind();

		/* stop the task */
		m_NCS_TASK_STOP(gl_pss_amf_attribs.mbx_details.pss_task_hdl);

		/* release the task */
		m_NCS_TASK_RELEASE(gl_pss_amf_attribs.mbx_details.pss_task_hdl);

		/* release the IPC */
		m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);
		(void)m_NCS_IPC_DETACH(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL, NULL);

		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* create the task */
	if (m_NCS_TASK_CREATE((NCS_OS_CB)pss_mbx_amf_process, &gl_pss_amf_attribs.mbx_details.pss_mbx, NCS_MAB_PSR_TASKNAME,	/* "PSS" */
			      NCS_MAB_PSR_PRIORITY, NCS_MAB_PSR_STACKSIZE,	/* 8192 */
			      &gl_pss_amf_attribs.mbx_details.pss_task_hdl) != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR, PSS_ERR_TASK_CREATE_FAILED);

		(void)pss_ts_destroy(gl_pss_amf_attribs.handles.pssts_hdl);

		/* unbind with DTA */
		pss_dtsv_unbind();

		/* relase the IPC */
		m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);
		(void)m_NCS_IPC_DETACH(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL, NULL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* start the PSS task */
	if (m_NCS_TASK_START(gl_pss_amf_attribs.mbx_details.pss_task_hdl) != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR, PSS_ERR_TASK_START_FAILED);

		(void)pss_ts_destroy(gl_pss_amf_attribs.handles.pssts_hdl);

		/* unbind with DTA */
		pss_dtsv_unbind();

		/* release the task */
		m_NCS_TASK_RELEASE(gl_pss_amf_attribs.mbx_details.pss_task_hdl);

		/* relase the IPC */
		m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);
		(void)m_NCS_IPC_DETACH(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL, NULL);

		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* log a headline that PSS is intialized successfully */
	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_CREATE_SUCCESS);

	return rc;
}

/****************************************************************************
  Name          : pss_lib_shut
 
  Description   : This is the function which destroy the PSS library.
                  This function releases the Task and the IPC mail Box.
                  This function destroies handle manager, CB and clean up 
                  all the component specific databases.
 
  Arguments     : none
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. 
\*****************************************************************************/
uns32 pss_lib_shut(void)
{
	NCSPSS_LM_ARG arg;

	/* check if PSS is intialized already */
	if (gl_pss_amf_attribs.handles.pss_cb_hdl == 0)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	/* destroy PSS */
	memset(&arg, 0, sizeof(arg));
	arg.i_usr_key = gl_pss_amf_attribs.handles.pss_cb_hdl;
	arg.i_op = NCSPSS_LM_OP_DESTROY;
	arg.info.destroy.i_meaningless = (void *)0x1234;
	if (ncspss_lm(&arg) != NCSCC_RC_SUCCESS) {
		/* log the error */	/* continue the destroy case */
	}
	gl_pss_amf_attribs.handles.pss_cb_hdl = 0;

	/* destory PSS Target services */
	if (pss_ts_destroy(gl_pss_amf_attribs.handles.pssts_hdl) != NCSCC_RC_SUCCESS) {
		/* log the error */	/* continue the destroy case */
	}
	gl_pss_amf_attribs.handles.pssts_hdl = 0;

	pss_dtsv_unbind();

	/* free the IPC and task */
	m_NCS_TASK_STOP(gl_pss_amf_attribs.mbx_details.pss_task_hdl);
	m_NCS_TASK_RELEASE(gl_pss_amf_attribs.mbx_details.pss_task_hdl);
	(void)m_NCS_IPC_DETACH(&gl_pss_amf_attribs.mbx_details.pss_mbx, pss_flush_mbx_msg, NULL);
	m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);
	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
  Name          : pss_ts_create
 
  Description   : This is the function which creates the PSS Target Services.
 
  Arguments     : pwe - PWE for which PSS is being created
                  ps_format_version - Persistent Store format version
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. 
\*****************************************************************************/
uns32 pss_ts_create(NCSPSS_CREATE *pwe, uns32 ps_format_version)
{
	uns32 rc;
	NCS_PSSTS_LM_ARG arg;
	int8 *pssv_root = NULL;
	char def_pssv_root[256];

	/* set the ground level */
	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCS_PSSTS_LM_OP_CREATE;
	arg.info.create.i_hmpool_id = NCS_HM_POOL_ID_COMMON;
	pssv_root = getenv("NCS_PSSV_ROOT");
	if (pssv_root == NULL) {
		memset(&def_pssv_root, '\0', sizeof(def_pssv_root));
		sprintf(def_pssv_root, "%s%d", NCS_PSS_DEF_PSSV_ROOT_PATH, ps_format_version);
		pssv_root = (char *)&def_pssv_root;
	}

	arg.info.create.i_root_dir = pssv_root;
	/* create the target service */
	rc = ncspssts_lm(&arg);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_TGT_SVSC_LM_CREATE_FAILED, rc);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* update the output parameters */
	pwe->i_hmpool_id = arg.info.create.i_hmpool_id;
	pwe->i_pssts_hdl = arg.info.create.o_handle;

	/* Enjoy, log that Target Services create is success */
	m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_TGT_SVCS_CREATE_SUCCESS, pwe->i_pssts_hdl);

	return rc;
}

/****************************************************************************
  Name          : pss_ts_destroy
 
  Description   : This is the function which destroys the PSS Target Services.
 
  Arguments     : pss_ts_handle - target services handle
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. 
\*****************************************************************************/
uns32 pss_ts_destroy(uns32 pss_ts_hdl)
{
	uns32 rc;
	NCS_PSSTS_LM_ARG arg;

	/* destory the target services */
	memset(&arg, 0, sizeof(NCS_PSSTS_LM_ARG));
	arg.i_op = NCS_PSSTS_LM_OP_DESTROY;
	arg.info.destroy.i_handle = pss_ts_hdl;
	rc = ncspssts_lm(&arg);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_PSS_ERROR_I(NCSFL_SEV_CRITICAL, PSS_ERR_TGT_SVCS_DESTROY_FAILED, rc);
		return m_MAB_DBG_SINK(rc);
	}

	/* mark the success */
	m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_TGT_SVCS_DESTROY_SUCCESS, pss_ts_hdl);

	return rc;
}

/****************************************************************************
  Name          : pss_create
 
  Description   : Does the following                          
                   - Create the VDEST for PSS 
                   - Create the PSS for default Env-Id
 
  Arguments     : PWE 
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. 
\*****************************************************************************/
uns32 pss_create(NCSPSS_CREATE *pwe)
{
	NCSPSS_LM_ARG arg;
	uns32 rc;
	NCSVDA_INFO vda_info;
	PSS_CSI_NODE *pss_def_csi = NULL;
	SaAmfHAStateT lcl_ha_role;
	NCS_APP_AMF_HA_STATE init_ha_role;
	FILE *fh = NULL;

	/* get the initial role from RDA */
	rc = app_rda_init_role_get(&init_ha_role);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_PSS_ERROR_I(NCSFL_SEV_CRITICAL, PSS_ERR_RDA_INIT_ROLE_GET_FAILED, rc);
		return m_LEAP_DBG_SINK(rc);
	}
	m_LOG_PSS_HDLN2_I(NCSFL_SEV_NOTICE, PSS_HDLN2_RDA_GIVEN_INIT_ROLE, init_ha_role);

	/* Create PSS VDEST */
	memset(&vda_info, 0, sizeof(NCSVDA_INFO));
	vda_info.req = NCSVDA_VDEST_CREATE;
	vda_info.info.vdest_create.i_persistent = FALSE;
	vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
	vda_info.info.vdest_create.i_create_oac = TRUE;
	vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
	vda_info.info.vdest_create.info.specified.i_vdest = PSR_VDEST_ID;
	rc = ncsvda_api(&vda_info);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_PSS_ERROR_I(NCSFL_SEV_CRITICAL, PSS_ERR_VDEST_CREATE_FAILED, rc);
		return m_LEAP_DBG_SINK(rc);
	}

	/* get the reqd data, to create PSS */
	pwe->i_mds_hdl = vda_info.info.vdest_create.o_mds_vdest_hdl;
	pwe->i_oac_key = vda_info.info.vdest_create.o_pwe1_oac_hdl;

	gl_pss_amf_attribs.ha_state = init_ha_role;

	/* create PSS */
	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCSPSS_LM_OP_CREATE;
	arg.info.create.i_vrid = pwe->i_vrid;
	arg.info.create.i_mds_hdl = pwe->i_mds_hdl;
	arg.info.create.i_mbx = pwe->i_mbx;
	arg.info.create.i_lm_cbfnc = dummy_mab_lm_cbfn;
	arg.info.create.i_hmpool_id = pwe->i_hmpool_id;
	arg.info.create.i_save_type = PSS_SAVE_TYPE_IMMEDIATE;
	arg.info.create.i_oac_key = pwe->i_oac_key;
	arg.info.create.i_pssts_hdl = pwe->i_pssts_hdl;
	arg.info.create.i_pssts_api = ncs_pssts_api;
	arg.info.create.i_mib_req_func = ncsmac_mib_request;
	rc = ncspss_lm(&arg);
	if (rc != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_PSS_ERROR_I(NCSFL_SEV_CRITICAL, PSS_ERR_LM_CREATE_FAILED, rc);
		return m_MAB_DBG_SINK(rc);
	}

	/* export the handle to external world */
	gl_pss_amf_attribs.handles.pss_cb_hdl = pwe->o_pss_hdl = arg.info.create.o_pss_hdl;

	/* install PSS in the default environment */
	if (init_ha_role == NCS_APP_AMF_HA_STATE_ACTIVE) {
		lcl_ha_role = SA_AMF_HA_ACTIVE;
	} else
		lcl_ha_role = SA_AMF_HA_STANDBY;

	pss_def_csi = pss_amf_csi_install(m_PSS_DEFAULT_ENV_ID, lcl_ha_role);
	if (pss_def_csi == NULL) {
		/* log the error */
		m_LOG_PSS_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_CRITICAL,
				   PSS_ERR_DEF_ENV_INIT_FAILED, m_PSS_DEFAULT_ENV_ID, lcl_ha_role);

		/* TBD -- Release the control block and other related stuff */
		return m_MAB_DBG_SINK(rc);
	}

	/* log that DEFAULT PWE is installed successfully */
	m_LOG_PSS_HDLN_II(NCSFL_SEV_INFO, PSS_HDLN_DEF_ENV_INIT_SUCCESS, m_PSS_DEFAULT_ENV_ID, lcl_ha_role);

	fh = sysf_fopen(m_PSS_PID_FILE, "w");
	if (fh == NULL) {
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN_PID_FILE_OPEN_FAIL);
		return NCSCC_RC_FAILURE;
	}

	fprintf(fh, "%d\n", getpid());

	fclose(fh);

	return rc;
}

uns32 dummy_mab_lm_cbfn(MAB_LM_EVT *evt)
{
	return NCSCC_RC_SUCCESS;
}
#endif   /* (NCS_PSR == 1) */

#endif   /* (NCS_MAB == 1) */
