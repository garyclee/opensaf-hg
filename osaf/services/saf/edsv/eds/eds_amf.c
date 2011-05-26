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
        
This include file contains AMF interaction logic for health-check and other
stuff.
*******************************************************************************/
#include "eds.h"

/* HA AMF statemachine & State handler definitions */

/****************************************************************************
 * Name          : eds_quiescing_state_handler
 *
 * Description   : This function is called upon receving an Quiescing state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

uint32_t eds_quiescing_state_handler(EDS_CB *cb, SaInvocationT invocation)
{
	SaAisErrorT error = SA_AIS_OK;
	error = saAmfCSIQuiescingComplete(cb->amf_hdl, invocation, error);

	saAmfResponse(cb->amf_hdl, invocation, error);
	TRACE("I AM IN HA AMF QUIESCING STATE");
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_quiesced_state_handler
 *
 * Description   : This function is called upon receving an Quiesced state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

uint32_t eds_quiesced_state_handler(EDS_CB *cb, SaInvocationT invocation)
{
	V_DEST_RL mds_role;
	SaAisErrorT error = SA_AIS_OK;

	mds_role = V_DEST_RL_QUIESCED;

	/** set the CB's anchor value & mds role */

	cb->mds_role = mds_role;
	eds_mds_change_role(cb);
	cb->amf_invocation_id = invocation;

	cb->is_quisced_set = true;
	TRACE("I AM IN HA AMF QUIESCED STATE");

	/* Give up our implementer role */
	error = immutil_saImmOiImplementerClear(cb->immOiHandle);
	if (error != SA_AIS_OK)
		LOG_ER("saImmOiImplementerClear failed with rc = %d", error);

	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : eds_invalid_state_handler
 *
 * Description   : This function is called upon receving an invalid state(as
 *                 in the FSM) assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the EDS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/

uint32_t eds_invalid_state_handler(EDS_CB *cb, SaInvocationT invocation)
{
	saAmfResponse(cb->amf_hdl, invocation, SA_AIS_ERR_BAD_OPERATION);
	return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : eds_amf_health_chk_callback
 *
 * Description   : This is the callback function which will be called 
 *                 when the AMF framework needs to health check for the component.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *                 checkType      - The type of healthcheck to be executed. 
 *
 * Return Values : None
 *
 * Notes         : At present we are just support a simple liveness check.
 *****************************************************************************/
void eds_amf_health_chk_callback(SaInvocationT invocation, const SaNameT *compName, SaAmfHealthcheckKeyT *checkType)
{
	EDS_CB *eds_cb;
	SaAisErrorT error = SA_AIS_OK;

	if (NULL == (eds_cb = (NCSCONTEXT)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl))) {
		m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return;
	} else {
		saAmfResponse(eds_cb->amf_hdl, invocation, error);
		ncshm_give_hdl(gl_eds_hdl);
	}
	return;
}

/****************************************************************************
 * Name          : eds_amf_CSI_set_callback
 *
 * Description   : AMF callback function called 
 *                 when there is any change in the HA state.
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The 
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *                 haState        - The new HA state to be assumeb by the 
 *                                  component service instance identified by 
 *                                  csiName.
 *                 csiDescriptor - This will indicate whether or not the 
 *                                  component service instance for 
 *                                  ativeCompName went through quiescing.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
eds_amf_CSI_set_callback(SaInvocationT invocation,
			 const SaNameT *compName, SaAmfHAStateT new_haState, SaAmfCSIDescriptorT csiDescriptor)
{
	EDS_CB *eds_cb;
	SaAisErrorT error = SA_AIS_OK;
	SaAmfHAStateT prev_haState;
	bool role_change = true;
	uint32_t rc = NCSCC_RC_SUCCESS;

	if (NULL == (eds_cb = (NCSCONTEXT)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl))) {
		m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return;
	} else {
		/*
		 *  Handle Active to Active role change.
		 */
		m_LOG_EDSV_S(EDS_AMF_RCVD_CSI_SET_CLBK, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, eds_cb->ha_state,
			     __FILE__, __LINE__, new_haState);
		prev_haState = eds_cb->ha_state;

		/* Invoke the appropriate state handler routine */
		switch (new_haState) {
		case SA_AMF_HA_ACTIVE:
			eds_cb->mds_role = V_DEST_RL_ACTIVE;
			TRACE("I AM HA AMF ACTIVE");
			break;
		case SA_AMF_HA_STANDBY:
			eds_cb->mds_role = V_DEST_RL_STANDBY;
			TRACE("I AM HA AMF STANDBY");
			break;
		case SA_AMF_HA_QUIESCED:
			eds_quiesced_state_handler(eds_cb, invocation);
			break;
		case SA_AMF_HA_QUIESCING:
			eds_quiescing_state_handler(eds_cb, invocation);
			break;
		default:
			m_LOG_EDSV_S(EDS_AMF_CSI_SET_HA_STATE_INVALID, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE,
				     eds_cb->ha_state, __FILE__, __LINE__, new_haState);
			eds_invalid_state_handler(eds_cb, invocation);
		}

		if (new_haState == SA_AMF_HA_QUIESCED)
			return;

		/* Update control block */
		eds_cb->ha_state = new_haState;

		if (eds_cb->csi_assigned == false) {
			eds_cb->csi_assigned = true;
			/* We shall open checkpoint only once in our life time. currently doing at lib init  */
		} else if ((new_haState == SA_AMF_HA_ACTIVE) || (new_haState == SA_AMF_HA_STANDBY)) {	/* It is a switch over */
			eds_cb->ckpt_state = COLD_SYNC_IDLE;
		}
		if ((prev_haState == SA_AMF_HA_ACTIVE) && (new_haState == SA_AMF_HA_ACTIVE)) {
			role_change = false;
		}

		/*
		 * Handle Stby to Stby role change.
		 */
		if ((prev_haState == SA_AMF_HA_STANDBY) && (new_haState == SA_AMF_HA_STANDBY)) {
			role_change = false;
		}

		if (role_change == true) {
			if ((rc = eds_mds_change_role(eds_cb)) != NCSCC_RC_SUCCESS) {
				m_LOG_EDSV_S(EDS_MDS_CSI_ROLE_CHANGE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc,
					     __FILE__, __LINE__, eds_cb->mds_role);
			} else {
				m_LOG_EDSV_S(EDS_MDS_CSI_ROLE_CHANGE_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, rc,
					     __FILE__, __LINE__, eds_cb->mds_role);
				/* Declare as Implementer if state changed only from STANDBY to ACTIVE, 
				 * Need to check for transition from QUEISCED back to ACTIVE 
				 */
				if (eds_cb->ha_state == SA_AMF_HA_ACTIVE) {
					eds_imm_declare_implementer(&eds_cb->immOiHandle);
				}
			}
		}
		/* Inform MBCSV of HA state change */
		if (NCSCC_RC_SUCCESS != (error = eds_mbcsv_change_HA_state(eds_cb))) {
			m_LOG_EDSV_S(EDS_MBCSV_FAILURE, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, error, __FILE__, __LINE__,
				     new_haState);
		} else {
			m_LOG_EDSV_S(EDS_MBCSV_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__, 1);
		}
		/* Send the response to AMF */
		saAmfResponse(eds_cb->amf_hdl, invocation, SA_AIS_OK);

		/* Do some OverHead Processing At New Active For EDA Down Events Cleanup if those reccords are present */
		if (eds_cb->ha_state == SA_AMF_HA_ACTIVE) {
			EDA_DOWN_LIST *eda_down_rec = NULL;
			EDA_DOWN_LIST *temp_eda_down_rec = NULL;

			eda_down_rec = eds_cb->eda_down_list_head;
			while (eda_down_rec) {
				/*Remove the EDA DOWN REC from the EDA_DOWN_LIST */
				/* Free the EDA_DOWN_REC */
				/* Remove this EDA entry from our processing lists */
				temp_eda_down_rec = eda_down_rec;
				eds_remove_regid_by_mds_dest(eds_cb, eda_down_rec->mds_dest);
				eda_down_rec = eda_down_rec->next;
				m_MMGR_FREE_EDA_DOWN_LIST(temp_eda_down_rec);
			}
			eds_cb->eda_down_list_head = NULL;
			eds_cb->eda_down_list_tail = NULL;
		}
		/*Give handles */
		ncshm_give_hdl(gl_eds_hdl);
	}
}	/* End CSI set callback */

/****************************************************************************
 * Name          : eds_amf_comp_terminate_callback
 *
 * Description   : This is the callback function which will be called 
 *                 when the AMF framework needs to terminate EDS. This does
 *                 all required to destroy EDS(except to unregister from AMF)
 *
 * Arguments     : invocation     - This parameter designated a particular 
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it 
 *                                  responds to the Avilability Management 
 *                                  FrameWork using the saAmfResponse() 
 *                                  function.
 *                 compName       - A pointer to the name of the component 
 *                                  whose readiness stae the Availability 
 *                                  Management Framework is setting.
 *
 * Return Values : None
 *
 * Notes         : At present we are just support a simple liveness check.
 *****************************************************************************/
void eds_amf_comp_terminate_callback(SaInvocationT invocation, const SaNameT *compName)
{
	EDS_CB *eds_cb;
	SaAisErrorT error = SA_AIS_OK;

	m_LOG_EDSV_S(EDS_AMF_TERMINATE_CALLBACK_CALLED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__, 1);
	if (NULL == (eds_cb = (NCSCONTEXT)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl))) {
		m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return;
	} else {
		saAmfResponse(eds_cb->amf_hdl, invocation, error);
		/* Clean up all internal structures */
		eds_remove_reglist_entry(eds_cb, 0, true);

		/* Destroy the cb */
		eds_cb_destroy(eds_cb);
		/* Give back the handle */
		ncshm_give_hdl(gl_eds_hdl);
		ncshm_destroy_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl);

		/* Detach from IPC */
		m_NCS_IPC_DETACH(&eds_cb->mbx, NULL, eds_cb);

		/* Disconnect from MDS */
		eds_mds_finalize(eds_cb);
		m_MMGR_FREE_EDS_CB(eds_cb);
		sleep(1);
		exit(0);

	}

	return;
}

/****************************************************************************
 * Name          : eds_amf_csi_rmv_callback
 *
 * Description   : This callback routine is invoked by AMF during a
 *                 CSI set removal operation. 
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 compName       - A pointer to the name of the component
 *                                  whose readiness stae the Availability
 *                                  Management Framework is setting.
 *                 csiName        - A const pointer to csiName
 *                 csiFlags       - csi Flags
 * Return Values : None 
 *****************************************************************************/
void
eds_amf_csi_rmv_callback(SaInvocationT invocation,
			 const SaNameT *compName, const SaNameT *csiName, const SaAmfCSIFlagsT csiFlags)
{
	EDS_CB *eds_cb;
	SaAisErrorT error = SA_AIS_OK;

	m_LOG_EDSV_S(EDS_REMOVE_CALLBACK_CALLED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__, 1);
	if (NULL == (eds_cb = (NCSCONTEXT)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl))) {
		m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__, 0);
		return;
	} else {
		saAmfResponse(eds_cb->amf_hdl, invocation, error);
		ncshm_give_hdl(gl_eds_hdl);
	}
	return;
}

/*****************************************************************************\
 *  Name:          eds_healthcheck_start                           * 
 *                                                                            *
 *  Description:   To start the health check                                  *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\******************************************************************************/
static uint32_t eds_healthcheck_start(EDS_CB *eds_cb)
{
	SaAisErrorT error;
	SaAmfHealthcheckKeyT Healthy;
	char *health_key = 0;

	if (eds_cb->healthCheckStarted == true) {
		return NCSCC_RC_SUCCESS;
	}

   /** start the AMF health check **/
	memset(&Healthy, 0, sizeof(Healthy));
	health_key = getenv("EDSV_ENV_HEALTHCHECK_KEY");
	if (health_key == NULL || strlen(health_key) > SA_AMF_HEALTHCHECK_KEY_MAX) {
		strcpy((char *)Healthy.key, "E5F6");
		/* Log it */
	} else {
		strncpy((char *)Healthy.key, health_key, SA_AMF_HEALTHCHECK_KEY_MAX);
	}
	Healthy.keyLen = strlen((char *)Healthy.key);

	error = saAmfHealthcheckStart(eds_cb->amf_hdl, &eds_cb->comp_name, &Healthy,
				      SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_FAILOVER);

	if (error != SA_AIS_OK) {
		return NCSCC_RC_FAILURE;
	} else {
		eds_cb->healthCheckStarted = true;
		return NCSCC_RC_SUCCESS;
	}
}

/****************************************************************************
 * Name          : eds_amf_init
 *
 * Description   : EDS initializes AMF for invoking process and registers 
 *                 the various callback functions.
 *
 * Arguments     : EDS_CB - EDS control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error Code.
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t eds_amf_init(EDS_CB *eds_cb)
{

	SaAmfCallbacksT amfCallbacks;
	SaVersionT amf_version;
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* Initialize amf callbacks */
	memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

	amfCallbacks.saAmfHealthcheckCallback = eds_amf_health_chk_callback;
	amfCallbacks.saAmfCSISetCallback = eds_amf_CSI_set_callback;
	amfCallbacks.saAmfComponentTerminateCallback = eds_amf_comp_terminate_callback;
	amfCallbacks.saAmfCSIRemoveCallback = eds_amf_csi_rmv_callback;

	m_EDSV_GET_AMF_VER(amf_version);

	/*Initialize the amf library */

	rc = saAmfInitialize(&eds_cb->amf_hdl, &amfCallbacks, &amf_version);

	if (rc != SA_AIS_OK) {
		m_LOG_EDSV_S(EDS_AMF_INIT_ERROR, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__, 0);
		return NCSCC_RC_FAILURE;
	}
	m_LOG_EDSV_S(EDS_AMF_INIT_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	/* Obtain the amf selection object to wait for amf events */
	if (SA_AIS_OK != (rc = saAmfSelectionObjectGet(eds_cb->amf_hdl, &eds_cb->amfSelectionObject))) {
		m_LOG_EDSV_S(EDS_AMF_GET_SEL_OBJ_FAILURE, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__,
			     0);
		return rc;
	}
	m_LOG_EDSV_S(EDS_AMF_GET_SEL_OBJ_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	/* get the component name */
	rc = saAmfComponentNameGet(eds_cb->amf_hdl, &eds_cb->comp_name);
	if (rc != SA_AIS_OK) {
		m_LOG_EDSV_S(EDS_AMF_COMP_NAME_GET_FAIL, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__,
			     0);
		return NCSCC_RC_FAILURE;
	}
	m_LOG_EDSV_S(EDS_AMF_COMP_NAME_GET_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__, 1);

	return rc;

}	/*End eds_amf_init */

/**************************************************************************
 Function: eds_amf_register

 Purpose:  Function which registers EDS with AMF.  

 Input:    Pointer to the EDS control block. 

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  Here we call eds_amf_init after reading the component name file and
         setting the environment varaiable in our own context.
         Proceed to register with AMF, since it has come up. 
**************************************************************************/
uint32_t eds_amf_register(EDS_CB *eds_cb)
{

	SaAisErrorT error;
	uint32_t rc = NCSCC_RC_SUCCESS;

	m_NCS_LOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);

	/* Initialize amf framework for hc interface */
	if ((rc = eds_amf_init(eds_cb)) != NCSCC_RC_SUCCESS) {
		m_LOG_EDSV_S(EDS_AMF_INIT_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__, 0);
		m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
		return NCSCC_RC_FAILURE;
	}
	m_LOG_EDSV_S(EDS_AMF_INIT_OK, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__, 1);

	/* register EDS component with AvSv */
	error = saAmfComponentRegister(eds_cb->amf_hdl, &eds_cb->comp_name, (SaNameT *)NULL);
	if (error != SA_AIS_OK) {
		m_LOG_EDSV_S(EDS_AMF_COMP_REGISTER_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, error, __FILE__,
			     __LINE__, 0);
		m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
		return NCSCC_RC_FAILURE;
	}
	m_LOG_EDSV_S(EDS_AMF_COMP_REGISTER_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__, 1);
	if (NCSCC_RC_SUCCESS != (rc = eds_healthcheck_start(eds_cb))) {
		m_LOG_EDSV_S(EDS_AMF_HLTH_CHK_START_FAIL, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__,
			     0);
		m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
		return NCSCC_RC_FAILURE;
	}
	m_LOG_EDSV_S(EDS_AMF_HLTH_CHK_START_DONE, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__, 1);
	m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 * Function: eds_clm_init
 *
 * Purpose:  Function to register with the CLM service (Currently B01.01). 
 * Input  :    None
 *
 * Returns: SaAisErrorT  type
 *
 * Notes  : The CLM registration is done only after AMF registration is
 * done and not during the pre-amf period.
 **************************************************************************/

SaAisErrorT eds_clm_init(EDS_CB *cb)
{
	SaVersionT clm_version;
	SaClmCallbacksT clm_cbk;
	SaClmClusterNodeT cluster_node;
	SaClmClusterNotificationBufferT notify_buff;
	SaAisErrorT rc = SA_AIS_OK;
	SaTimeT timeout = EDSV_CLM_TIMEOUT;

	memset(&clm_version, 0, sizeof(SaVersionT));
	memset(&cluster_node, 0, sizeof(SaClmClusterNodeT));
	memset(&notify_buff, 0, sizeof(SaClmClusterNotificationBufferT));

	/* Fill version */
	m_EDSV_GET_CLM_VER(clm_version);

	clm_cbk.saClmClusterNodeGetCallback = NULL;
	clm_cbk.saClmClusterTrackCallback = eds_clm_cluster_track_cbk;

	/* Say Hello */
	rc = saClmInitialize(&cb->clm_hdl, &clm_cbk, &clm_version);
	if (rc != SA_AIS_OK) {
		m_LOG_EDSV_S(EDS_CLM_INIT_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__, 0);
		return rc;
	}
	/* Get the FD */
	if (SA_AIS_OK != (rc = saClmSelectionObjectGet(cb->clm_hdl, &cb->clm_sel_obj))) {
		m_LOG_EDSV_S(EDS_CLM_SEL_OBJ_GET_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__,
			     0);
		return rc;
	}
	/* Get your own node id */
	rc = saClmClusterNodeGet(cb->clm_hdl, SA_CLM_LOCAL_NODE_ID, timeout, &cluster_node);
	if (rc != SA_AIS_OK) {
		m_LOG_EDSV_S(EDS_CLM_NODE_GET_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__, 0);
		return rc;
	}
	/* Get my node_id. Not sure if i need this at all */
	cb->node_id = cluster_node.nodeId;

	notify_buff.notification = NULL;
	rc = saClmClusterTrack(cb->clm_hdl, (SA_TRACK_CURRENT | SA_TRACK_CHANGES), NULL);
	if (rc != SA_AIS_OK)
		m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__,
			     0);

	return rc;

}

/**************************************************************************
 * Function: eds_clm_cluster_track_cbk
 *
 * Purpose: The cluster track callback invoked by CLM whenever thereis a 
 *          change in the status of a node in the CLUSTER. 
 *          This callback is invoked whenever any of the following events
 *          occur on a cluster node:
 *          - SA_CLM_NODE_JOINED
 *          - SA_CLM_NODE_EXITED
 *          - SA_CLM_NODE_NO_CHANGE
 *          - SA_CLM_NODE_RECONFIGURED
 * Input  : none 
 * Output : NotificationBuffer that contains information of all nodes in the 
 *          cluster and the corresponding change that occured on that node.
 * Returns:   None 
 *
 * Notes  : Upon invocation of this callback, if a node joined or left the cluster,
 * EDS shall send an update to all the EDAs on that particular node.
 * This function updates the nodes list (in the cluster) accordingly
 * This list is checked while processing init messages from EDAs. All current 
 * member nodes of the cluster are maintained in this list. SA_AIS_ERR_UNAVAILABLE 
 * is retured for an saEvtInitialize() originating from a non member node.
 **************************************************************************/

void eds_clm_cluster_track_cbk(const SaClmClusterNotificationBufferT *notificationBuffer,
			       SaUint32T numberOfMembers, SaAisErrorT error)
{
	EDS_CB *cb = NULL;
	NODE_ID node_id;
	SaClmClusterChangesT cluster_change;
	SaBoolT is_member;
	uint32_t counter = 0;

	TRACE("CLM Cluster track callback");
	if (error == SA_AIS_OK) {
		/* Get EDS CB Handle. */
		if (NULL == (cb = (NCSCONTEXT)ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl))) {
			m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_CBK_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, error,
				     __FILE__, __LINE__, 1);
			return;
		}

		if (notificationBuffer != NULL) {
			for (counter = 0; counter < notificationBuffer->numberOfItems; counter++) {
				is_member = notificationBuffer->notification[counter].clusterNode.member;
				node_id = notificationBuffer->notification[counter].clusterNode.nodeId;
				if ((cluster_change = update_node_db(cb, node_id, is_member))) {
					if (is_member)
						cluster_change = SA_CLM_NODE_JOINED;
					else
						cluster_change = SA_CLM_NODE_LEFT;
					/* Send to all EDAs on node_id */
					send_clm_status_change(cb, cluster_change, node_id);
				}
			}
		}
		else
			TRACE("Notification buffer is NULL");
		TRACE("After processing, clust list contains: %d nodes",ncs_patricia_tree_size(&cb->eds_cluster_nodes_list));
		TRACE("=======================================");
		NODE_INFO *tmp=NULL;
		tmp = (NODE_INFO *)ncs_patricia_tree_getnext(&cb->eds_cluster_nodes_list, (uint8_t *)0);
		while (tmp != NULL)
		{
			TRACE("NODE = %d\n", tmp->node_id);
			tmp=(NODE_INFO *)ncs_patricia_tree_getnext(&cb->eds_cluster_nodes_list, (uint8_t *)&tmp->node_id);
		}
		TRACE("=======================================");
		ncshm_give_hdl(gl_eds_hdl);
		m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_CBK_SUCCESS, NCSFL_LC_EDSV_INIT, NCSFL_SEV_NOTICE, error, __FILE__,
			     __LINE__, 0);
	} else
		m_LOG_EDSV_S(EDS_CLM_CLUSTER_TRACK_CBK_FAILED, NCSFL_LC_EDSV_INIT, NCSFL_SEV_ERROR, error, __FILE__,
			     __LINE__, 1);

	return;

}

/**************************************************************************
 * Function: update_node_db 
 *
 * Purpose: The cluster track callback invoked by CLM whenever thereis a
 *          change in the status of a node in the CLUSTER.
 * Input  : none
 * Output : NotificationBuffer that contains information of all nodes in the
 *          cluster and the corresponding change that occured on that node.
 * Returns: SA_TRUE if there is a change in membership, else SA_FALSE 
 *
 * Notes  : Upon invocation of this callback, if a node joined or left the cluster,
 *  EDS shall send an update to all the EDAs on that particular node.
 **************************************************************************/

SaBoolT update_node_db(EDS_CB *cb, NODE_ID node_id, SaBoolT is_member)
{
	NODE_INFO *cn = NULL;
	if (is_member) {
		if ((cn = (NODE_INFO *)ncs_patricia_tree_get(&cb->eds_cluster_nodes_list,(uint8_t *)&node_id)) == NULL) {
			TRACE("Node = %d not existing in the DB",node_id);
			cn = (NODE_INFO *)malloc(sizeof(NODE_INFO));
			if (cn == NULL) {
				LOG_CR("Malloc failed for cluster node record");
				return SA_FALSE;
			}
			cn->node_id = node_id;
			cn->pat_node.key_info = (uint8_t *)&cn->node_id;
			if (ncs_patricia_tree_add(&cb->eds_cluster_nodes_list,&cn->pat_node) != NCSCC_RC_SUCCESS) {
				free(cn);
				LOG_ER("Patricia add failed for cluster node %d",cn->node_id);
				return SA_FALSE;
			} else {
				TRACE("Added Node = %d, to the DB, num nodes = %d",node_id,ncs_patricia_tree_size(&cb->eds_cluster_nodes_list));
				return SA_TRUE;
			}
		} else {
			TRACE("Node = %d, already a member",node_id);
			return SA_FALSE;
		}
	}
	else {
		if ((cn = (NODE_INFO *)ncs_patricia_tree_get(&cb->eds_cluster_nodes_list,(uint8_t *)&node_id)) == NULL) {
			TRACE("Node = %d not in the DB, no change to DB",node_id);
			return SA_FALSE;
		}
		if (ncs_patricia_tree_del(&cb->eds_cluster_nodes_list, &cn->pat_node) != NCSCC_RC_SUCCESS) {
			LOG_ER("Patricia tree delete failed for node_id = %d",node_id);
			return SA_FALSE;
		} else {
			if (cn)
				free(cn);
			TRACE("Node = %d deleted from DB",node_id);
			return SA_TRUE;
		}
	}
}

/**************************************************************************
 * Function: send_clm_status_change 
 *
 * Purpose: The cluster track callback invoked by CLM whenever thereis a
 *          change in the status of a node in the CLUSTER.
 * Input  : pointer to eds CB, cluster change and the node_id for this change.
 * Returns:   None
 * Notes  : Sends an asynch update message to all EDAs residing on the nodes
 * that have had a change in their cluster membership ((Re)joined/left).
 **************************************************************************/

void send_clm_status_change(EDS_CB *cb, SaClmClusterChangesT cluster_change, NODE_ID node_id)
{
	EDSV_MSG msg;
	EDA_REG_REC *reg_rec = NULL;
	uint32_t rc = NCSCC_RC_SUCCESS;

	m_EDS_EDSV_CLM_STATUS_CB_MSG_FILL(msg, cluster_change);
	reg_rec = (EDA_REG_REC *)ncs_patricia_tree_getnext(&cb->eda_reg_list, (uint8_t *)0);
	while (reg_rec != NULL) {
		if (node_id == (m_EDS_GET_NODE_ID_FROM_ADEST(reg_rec->eda_client_dest))) {
			rc = eds_mds_msg_send(cb, &msg, &reg_rec->eda_client_dest, NULL, MDS_SEND_PRIORITY_MEDIUM);
			if (rc != NCSCC_RC_SUCCESS)
				m_LOG_EDSV_S(EDS_CLUSTER_CHANGE_NOTIFY_SEND_FAILED, NCSFL_LC_EDSV_INIT,
					     NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__, 1);
			else
				TRACE("Sending ClusterNodeUpdate = %d for %d Success", cluster_change, node_id);
		}
		reg_rec = (EDA_REG_REC *)ncs_patricia_tree_getnext(&cb->eda_reg_list, (uint8_t *)&reg_rec->reg_id_Net);
	}

}

/**************************************************************************
 * Function: is_node_a_member
 *
 * Purpose : This function walks through the cluster_node_list to see if
 * the input node_id is present in the list or not.
 *
 * Input   : pointer to eds CB, the node_id.
 * Returns : true/false. 
 **************************************************************************/

bool is_node_a_member(EDS_CB *cb, NODE_ID node_id)
{
	NODE_INFO *cn = NULL;

	if ((cn = (NODE_INFO *)ncs_patricia_tree_get(&cb->eds_cluster_nodes_list,(uint8_t *)&node_id)) == NULL)
		return false;
	else
		return true;
}

