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
 * Author(s): Ericsson AB
 *
 * This file implements the AMF interface for LGS.
 * The interface exist of one exported function: lgs_amf_init().
 * The AMF callback functions except a number of exported functions from
 * other modules.
 */

#include "lgs.h"

/****************************************************************************
 * Name          : lgs_active_state_handler
 *
 * Description   : This function is called upon receving an active state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the LGS control block. 
 *
 * Return Values : None
 *
 * Notes         : None 
 *****************************************************************************/
static SaAisErrorT lgs_active_state_handler(lgs_cb_t *cb,
                                            SaInvocationT invocation)
{
    log_stream_t *stream;
    SaAisErrorT error = SA_AIS_OK;
    char name[SA_MAX_NAME_LENGTH];

    TRACE_ENTER();
    TRACE("HA ACTIVE request");

    memset(name, 0, SA_MAX_NAME_LENGTH);
    strcpy(name, SA_LOG_STREAM_ALARM);
    stream = log_stream_get_by_name(name);
    if (stream == NULL)
    {
        if (lgs_create_known_streams(cb) != NCSCC_RC_SUCCESS)
        {
            error = SA_AIS_ERR_FAILED_OPERATION;
            goto done;
        }
    }

    /* Open all streams */
    stream = log_stream_getnext_by_name(NULL);
    while (stream != NULL)
    {
        if ((error = log_stream_open(stream)) != SA_AIS_OK)
            goto done;

        stream = log_stream_getnext_by_name(stream->name);
    }

    lgs_cb->mds_role = V_DEST_RL_ACTIVE;

done:
    TRACE_LEAVE();
    return error;
}

/****************************************************************************
 * Name          : lgs_standby_state_handler
 *
 * Description   : This function is called upon receving an standby state
 *                 assignment from AMF.
 *
 * Arguments     : invocation     - This parameter designated a particular
 *                                  invocation of this callback function. The
 *                                  invoke process return invocation when it
 *                                  responds to the Avilability Management
 *                                  FrameWork using the saAmfResponse()
 *                                  function.
 *                 cb              -A pointer to the LGS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
static uns32 lgs_standby_state_handler(lgs_cb_t *cb, SaInvocationT invocation)
{
    TRACE_ENTER();
    cb->mds_role = V_DEST_RL_STANDBY;
    TRACE("HA STANDBY request");
    /* Fix: Close all open files */
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_quiescing_state_handler
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
 *                 cb              -A pointer to the LGS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
static uns32 lgs_quiescing_state_handler(lgs_cb_t *cb, SaInvocationT invocation)
{
    SaAisErrorT error = SA_AIS_OK;

    TRACE_ENTER();
    error =  saAmfCSIQuiescingComplete(cb->amf_hdl, invocation, error);
    saAmfResponse(cb->amf_hdl, invocation, error);
    TRACE("I AM IN HA AMF QUIESCING STATE\n");
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_quiesced_state_handler
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
 *                 cb              -A pointer to the LGS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
static uns32 lgs_quiesced_state_handler(lgs_cb_t *cb, SaInvocationT invocation)
{
    TRACE_ENTER();
    cb->mds_role = V_DEST_RL_QUIESCED;
    lgs_mds_change_role(cb);
    cb->amf_invocation_id = invocation;
    cb->is_quisced_set = TRUE; 
    TRACE("I AM IN HA AMF QUIESCED STATE\n");
    TRACE_LEAVE();
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : lgs_invalid_state_handler
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
 *                 cb              -A pointer to the LGS control block.
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
static SaAisErrorT lgs_invalid_state_handler(lgs_cb_t *cb,
                                             SaInvocationT invocation)
{
    TRACE_ENTER();
    TRACE_LEAVE();
    return SA_AIS_ERR_BAD_OPERATION;
}

/****************************************************************************
 * Name          : lgs_amf_health_chk_callback
 *
 * Description   : This is the callback function which will be called 
 *                 when the AMF framework nelgs to health check for the component.
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
static void lgs_amf_health_chk_callback (SaInvocationT invocation,
                                         const SaNameT *compName,
                                         SaAmfHealthcheckKeyT *checkType)
{
    saAmfResponse(lgs_cb->amf_hdl, invocation, SA_AIS_OK);
}

/****************************************************************************
 * Name          : lgs_amf_CSI_set_callback
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
static void lgs_amf_CSI_set_callback (SaInvocationT invocation,
                                      const SaNameT  *compName,
                                      SaAmfHAStateT  new_haState,
                                      SaAmfCSIDescriptorT csiDescriptor)
{
    SaAisErrorT error  = SA_AIS_OK;
    SaAmfHAStateT      prev_haState;
    NCS_BOOL           role_change = TRUE;
    uns32              rc = NCSCC_RC_SUCCESS;

    TRACE_ENTER();

    /*
     *  Handle Active to Active role change.
     */
    prev_haState = lgs_cb->ha_state;

    /* Invoke the appropriate state handler routine */
    switch (new_haState)
    {
        case SA_AMF_HA_ACTIVE:
            error = lgs_active_state_handler(lgs_cb, invocation);
            break;
        case SA_AMF_HA_STANDBY:
            lgs_standby_state_handler(lgs_cb, invocation);
            break;
        case SA_AMF_HA_QUIESCED:
            lgs_quiesced_state_handler(lgs_cb, invocation);
            break;
        case SA_AMF_HA_QUIESCING: 
            lgs_quiescing_state_handler(lgs_cb, invocation);
            break;
        default:
            LOG_WA("invalid state: %d ", new_haState);
            error = lgs_invalid_state_handler(lgs_cb, invocation);
            break;
    } 

    if (error != SA_AIS_OK)
        goto done;

    if (new_haState == SA_AMF_HA_QUIESCED)
    {
        TRACE("SA_AMF_HA_QUIESCED");
        goto done;
    }

    /* Update control block */
    lgs_cb->ha_state = new_haState;

    if (lgs_cb->csi_assigned == FALSE)
    {
        lgs_cb->csi_assigned = TRUE;
        /* We shall open checkpoint only once in our life time. currently doing at lib init  */
    }
    else if ((new_haState == SA_AMF_HA_ACTIVE) || (new_haState == SA_AMF_HA_STANDBY))
    {  /* It is a switch over */
        lgs_cb->ckpt_state = COLD_SYNC_IDLE;
        /* NOTE: This behaviour has to be checked later, when scxb redundancy is available 
         * Also, change role of mds, mbcsv during quiesced has to be done after mds
         * supports the same.  TBD
         */
    }
    if ((prev_haState == SA_AMF_HA_ACTIVE) && (new_haState == SA_AMF_HA_ACTIVE))
    {
        role_change = FALSE;
    }

    /*
     * Handle Stby to Stby role change.
     */
    if ((prev_haState == SA_AMF_HA_STANDBY) && (new_haState == SA_AMF_HA_STANDBY))
    {
        role_change = FALSE;
    }

    if (role_change == TRUE)
    {
        if ((rc = lgs_mds_change_role(lgs_cb) ) != NCSCC_RC_SUCCESS)
        {
            TRACE(" lgs_amf_CSI_set_callback: MDS role change to %d FAILED\n",lgs_cb->mds_role);
            error = SA_AIS_ERR_FAILED_OPERATION;
        }
    }
    /* Inform MBCSV of HA state change */
    if (NCSCC_RC_SUCCESS != (error=lgs_mbcsv_change_HA_state(lgs_cb)))
    {
        TRACE(" lgs_amf_CSI_set_callback: MBCSV state change to (in CSI AssignMent Cbk): %d FAILED \n",lgs_cb->ha_state);
        error = SA_AIS_ERR_FAILED_OPERATION;
    }

done:
    saAmfResponse(lgs_cb->amf_hdl, invocation, error);
    TRACE_LEAVE();
}

/****************************************************************************
 * Name          : lgs_amf_comp_terminate_callback
 *
 * Description   : This is the callback function which will be called 
 *                 when the AMF framework nelgs to terminate LGS. This does
 *                 all required to destroy LGS(except to unregister from AMF)
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
static void lgs_amf_comp_terminate_callback(SaInvocationT invocation,
                                            const SaNameT *compName)
{
    TRACE_ENTER();

    saAmfResponse(lgs_cb->amf_hdl,invocation, SA_AIS_OK);

    /* Detach from IPC */
    m_NCS_IPC_DETACH(&lgs_cb->mbx, NULL, lgs_cb);

    /* Disconnect from MDS */
    lgs_mds_finalize(lgs_cb);
    sleep(1);
    exit(0);
}

/****************************************************************************
 * Name          : lgs_amf_csi_rmv_callback
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
static void lgs_amf_csi_rmv_callback(SaInvocationT invocation,
                                     const SaNameT *compName,
                                     const SaNameT *csiName,
                                     const SaAmfCSIFlagsT csiFlags)
{
    TRACE_ENTER();
    saAmfResponse(lgs_cb->amf_hdl, invocation, SA_AIS_OK);
    TRACE_LEAVE();
}

/*****************************************************************************\
 *  Name:          lgs_healthcheck_start                           * 
 *                                                                            *
 *  Description:   To start the health check                                  *
 *                                                                            *
 *  Arguments:     NCSSA_CB* - Control Block                                  * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\******************************************************************************/
static SaAisErrorT amf_healthcheck_start(lgs_cb_t *lgs_cb)
{
    SaAisErrorT          error;
    SaAmfHealthcheckKeyT healthy;
    char                 *health_key;

    TRACE_ENTER();

    /** start the AMF health check **/
    memset(&healthy, 0, sizeof(healthy));
    health_key = (int8*)getenv("LGSV_ENV_HEALTHCHECK_KEY");

    if (health_key == NULL)
        strcpy(healthy.key, "F1B2");
    else
        strcpy(healthy.key, health_key);

    healthy.keyLen = strlen((const char *)healthy.key);

    error = saAmfHealthcheckStart(lgs_cb->amf_hdl, &lgs_cb->comp_name, &healthy,
                                  SA_AMF_HEALTHCHECK_AMF_INVOKED,
                                  SA_AMF_COMPONENT_FAILOVER);

    if (error != SA_AIS_OK)
        LOG_ER("saAmfHealthcheckStart FAILED: %u", error);

    TRACE_LEAVE();
    return error;
}

/**************************************************************************
 Function: lgs_amf_register

 Purpose:  Function which registers LGS with AMF.  

 Input:    None 

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  Here we call lgs_amf_init after reading the component name file and
         setting the environment varaiable in our own context.
         Proceed to register with AMF, since it has come up. 
**************************************************************************/
SaAisErrorT lgs_amf_init(lgs_cb_t *cb)
{
    SaAmfCallbacksT amfCallbacks;
    SaVersionT      amf_version;
    SaAisErrorT     error;

    TRACE_ENTER();

    /* Initialize AMF callbacks */
    memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));
    amfCallbacks.saAmfHealthcheckCallback = lgs_amf_health_chk_callback;
    amfCallbacks.saAmfCSISetCallback      = lgs_amf_CSI_set_callback;
    amfCallbacks.saAmfComponentTerminateCallback
    = lgs_amf_comp_terminate_callback;
    amfCallbacks.saAmfCSIRemoveCallback   = lgs_amf_csi_rmv_callback;

    amf_version.releaseCode = 'B';
    amf_version.majorVersion = 0x01;
    amf_version.minorVersion = 0x01;

    /* Initialize the AMF library */
    error = saAmfInitialize(&cb->amf_hdl, &amfCallbacks, &amf_version);
    if (error != SA_AIS_OK)
    {
        LOG_ER("saAmfInitialize() FAILED: %u", error);
        goto done;
    }

    /* Obtain the AMF selection object to wait for AMF events */
    error = saAmfSelectionObjectGet(cb->amf_hdl, &cb->amfSelectionObject);
    if (error != SA_AIS_OK)
    {
        LOG_ER("saAmfSelectionObjectGet() FAILED: %u", error);
        goto done;
    }

    /* Get the component name */
    error = saAmfComponentNameGet(cb->amf_hdl, &cb->comp_name);
    if (error != SA_AIS_OK)
    {
        LOG_ER("saAmfComponentNameGet() FAILED: %u", error);
        goto done;
    }

    /* Register component with AMF */
    error = saAmfComponentRegister(cb->amf_hdl, &cb->comp_name, (SaNameT*)NULL);
    if (error != SA_AIS_OK)
    {
        LOG_ER("saAmfComponentRegister() FAILED\n");
        goto done;
    }

    /* Start AMF healthchecks */
    if ((error = amf_healthcheck_start(cb)) != SA_AIS_OK)
    {
        goto done;
    }

done:
    TRACE_LEAVE();
    return error;
}

