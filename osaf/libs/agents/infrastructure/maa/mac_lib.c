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

..............................................................................

******************************************************************************
*/

#if (NCS_MAB == 1)
#include "mac.h"

MABMAC_API uns32 gl_mac_handle;

typedef struct macmbx_struct {
	SYSF_MBX mac_mbx;
	void *mac_mbx_hdl;
} MACMBX_STRUCT;

static MACMBX_STRUCT gl_mac_mbx;
static NCS_BOOL gl_mac_inited = FALSE;

/* global list of all MAA instances */
static MAB_INST_NODE *gl_mac_inst_list;

/* to create MAA */
static uns32 maclib_mac_create(NCS_LIB_REQ_INFO *req_info);

/* creates and starts MAA thread */
static uns32 maclib_mac_create_ipc_task();

/* instantiate MAA in a particular PWE */
static uns32 maclib_mac_instantiate(NCS_LIB_REQ_INFO *req_info);

/* un-instantiate MAA in a particular PWE */
static uns32
maclib_mac_uninstantiate(PW_ENV_ID env_id, SaNameT i_inst_name, uns32 i_mac_handle, NCS_BOOL i_spir_cleanup);

/* to add to the global list of instances */
static uns32 mac_inst_list_add(PW_ENV_ID env_id, uns32 i_mac_hdl, SaNameT i_inst_name);

/* to delete and release a particular MAA instance from the list */
static void mac_inst_list_release(PW_ENV_ID env_id);

/* callback to inform the creator about a failure in MAA */
static uns32 mac_evt_cb(MAB_LM_EVT *evt);

/* callback to free the message in MAC's mailbox */
static NCS_BOOL mac_leave_on_queue_cb(void *arg1, void *arg2);

/*****************************************************************************\
 *  Name:          mac_evt_cb                                                 * 
 *                                                                            *
 *  Description:   MAA will call this call back function, when there is       *
 *                 a failure in MAA                                           *
 *                                                                            *
 *  Arguments:     MAB_LM_EVT * - Information to be informed to the creator   *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS                                           *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static uns32 mac_evt_cb(MAB_LM_EVT *evt)
{
	USE(evt);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
 *  Name:          maclib_request                                             * 
 *                                                                            *
 *  Description:   Single Entry point for MAA component of MASv               *
 *                                                                            *
 *  Arguments:     NCS_LIB_REQ_INFO* - input arguments                        * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS - Everything went well                    * 
 *                 NCSCC_RC_FAILURE = Something went wrong                    *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32 maclib_request(NCS_LIB_REQ_INFO *req_info)
{
	uns32 status = NCSCC_RC_FAILURE;
	MAB_MSG *post_me = NULL;

	/* make sure there is somedata */
	if (req_info == NULL)
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

	/* see what can we do for the caller */
	switch (req_info->i_op) {
		/* create MAA */
	case NCS_LIB_REQ_CREATE:
		status = maclib_mac_create(req_info);
		if (status != NCSCC_RC_SUCCESS)
			return m_MAB_DBG_SINK(status);

		/* Precreate the MAC instance for the default PWE */
		{
			NCS_SPIR_REQ_INFO spir_req;

			/* Get the PWE-HDL */
			memset(&spir_req, 0, sizeof(spir_req));
			spir_req.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
			spir_req.i_environment_id = 1;
			spir_req.i_sp_abstract_name = m_MAA_SP_ABST_NAME;
			spir_req.info.lookup_create_inst.i_inst_attrs.number = 0;
			spir_req.i_instance_name.length = 0;

			if (ncs_spir_api(&spir_req) != NCSCC_RC_SUCCESS) {
				return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
			}

			gl_mac_handle = spir_req.info.lookup_create_inst.o_handle;
		}
		break;

		/* Instantiate MAA in a paerticulr PWE/Environment */
	case NCS_LIB_REQ_INSTANTIATE:
		status = maclib_mac_instantiate(req_info);
		break;

		/* uninstantiate a particular instance of MAA */
	case NCS_LIB_REQ_UNINSTANTIATE:
		status = maclib_mac_uninstantiate(req_info->info.uninst.i_env_id,
						  req_info->info.uninst.i_inst_name,
						  req_info->info.uninst.i_inst_hdl, FALSE);
		break;

		/* destroy MAA (destroys all the instances of MAA) */
	case NCS_LIB_REQ_DESTROY:
		post_me = m_MMGR_ALLOC_MAB_MSG;
		if (post_me == NULL) {
			return m_MAB_DBG_SINK(NCSCC_RC_OUT_OF_MEM);
		}
		memset(post_me, 0, sizeof(MAB_MSG));
		post_me->op = MAB_MAC_DESTROY;

		/* post a message to MAC's thread */
		status = m_NCS_IPC_SEND(&gl_mac_mbx.mac_mbx, (NCS_IPC_MSG *)post_me, NCS_IPC_PRIORITY_HIGH);
		if (status != NCSCC_RC_SUCCESS) {
			m_MMGR_FREE_MAB_MSG(post_me);
			return m_MAB_DBG_SINK(status);
		}

		break;

		/* MAA can not help with this type of request */
	default:
		return m_MAB_DBG_SINK(status);
		break;

	}

	/* time to report the status to the caller */
	return status;
}

/****************************************************************************\
*  Name:          maclib_mac_create                                          * 
*                                                                            *
*  Description:   to create MAA                                              *
*                  - Register with DTSv                                      *
*                  - Register with SPLR                                      *
*                  - Create Mail box                                         *
*                  - create and start a thread for MAA                       *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO  - input parameters                       * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - something went wrong                    * 
\****************************************************************************/
static uns32 maclib_mac_create(NCS_LIB_REQ_INFO *req_info)
{
	uns32 status;
	NCS_SPLR_REQ_INFO splr_info;

	if (req_info == NULL)
		return NCSCC_RC_FAILURE;

	/* MAA is installed already */
	if (gl_mac_inited == TRUE)
		return NCSCC_RC_SUCCESS;

	/* register with DTS */
	status = mab_log_bind();
	if (status != NCSCC_RC_SUCCESS)
		return status;

	/* register with SPLR data base */
	/* SPLR: Service Provider Library Registry */
	memset(&splr_info, 0, sizeof(NCS_SPLR_REQ_INFO));
	splr_info.type = NCS_SPLR_REQ_REG;
	splr_info.i_sp_abstract_name = m_MAA_SP_ABST_NAME;
	splr_info.info.reg.instantiation_flags = NCS_SPLR_INSTANTIATION_PER_ENV_ID;
	splr_info.info.reg.instantiation_api = maclib_request;
	status = ncs_splr_api(&splr_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR, MAB_MAC_ERR_SPLR_REG_FAILED, status, 0);
		/* unbind with DTS */
		mab_log_unbind();
		return status;
	}
	m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_MAC_CREATE_SUCCESS, MAA_MDS_SUB_PART_VERSION);
	gl_mac_inited = TRUE;
	return status;
}

/****************************************************************************\
*  Name:          maclib_mac_create_ipc_task                                 * 
*                                                                            *
*  Description:   Creates mailbox and thread for MAA                         *
*                                                                            *
*  Arguments:     
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:          'gl_mac_mbx' has been accessed                              * 
\****************************************************************************/
static uns32 maclib_mac_create_ipc_task()
{
	uns32 status;

	/* create mail-box for MAA */
	status = m_NCS_IPC_CREATE(&gl_mac_mbx.mac_mbx);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_IPC_CREATE_FAILED, status);
		/* unbind with DTS */
		mab_log_unbind();
		return status;
	}

	/* Attach the IPC */
	status = m_NCS_IPC_ATTACH(&gl_mac_mbx.mac_mbx);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_IPC_ATTACH_FAILED, status);

		/* unbind with DTS */
		mab_log_unbind();

		/* relase the IPC */
		m_NCS_IPC_RELEASE(&gl_mac_mbx.mac_mbx, NULL);
		return status;
	}

	/* create the MAA thread */
	status = m_NCS_TASK_CREATE((NCS_OS_CB)mac_do_evts,
				   &gl_mac_mbx.mac_mbx,
				   NCS_MAB_MAC_TASKNAME,
				   NCS_MAB_MAC_PRIORITY, NCS_MAB_MAC_STACKSIZE, &gl_mac_mbx.mac_mbx_hdl);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_TASK_CREATE_FAILED, status);

		/* unbind with DTS */
		mab_log_unbind();

		/* detach the IPC */
		m_NCS_IPC_DETACH(&gl_mac_mbx.mac_mbx, NULL, NULL);

		/* release the IPC */
		m_NCS_IPC_RELEASE(&gl_mac_mbx.mac_mbx, NULL);
		return status;
	}

	/* start the MAA thread */
	status = m_NCS_TASK_START(gl_mac_mbx.mac_mbx_hdl);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the error */
		m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAC_ERR_TASK_START_FAILED, status);

		/* unbind with DTS */
		mab_log_unbind();

		/* release the task and IPC */
		m_NCS_TASK_RELEASE(gl_mac_mbx.mac_mbx_hdl);
		/* detach the IPC */
		m_NCS_IPC_DETACH(&gl_mac_mbx.mac_mbx, NULL, NULL);

		m_NCS_IPC_RELEASE(&gl_mac_mbx.mac_mbx, NULL);
		return status;
	}
	return status;
}

/****************************************************************************\
*  Name:          maclib_mac_instantiate                                     * 
*                                                                            *
*  Description:   To instantiate MAA in a specific environment               *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO - instantiate inputs                      * 
*                 o_mac_hdl - Handle to the newly instiated MAA              *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:          'gl_mac_mbx' has been accessed                             * 
\****************************************************************************/
static uns32 maclib_mac_instantiate(NCS_LIB_REQ_INFO *req_info)
{
	uns32 status;
	NCSMAC_LM_ARG arg;
	NCS_SEL_OBJ_SET set;
	uns32 timeout = 300;

	if (gl_mac_inited == FALSE) {
		/* SPLR registry is not yet done */
		return NCSCC_RC_FAILURE;
	}

	if (gl_mac_inst_list == NULL) {
		/* create MAA thread and mail box */
		status = maclib_mac_create_ipc_task();
		if (status != NCSCC_RC_SUCCESS) {
			/* log the error */
			return status;
		}
	}

	/* instantiate MAA in the asked PWE */
	memset(&arg, 0, sizeof(arg));
	arg.i_op = NCSMAC_LM_OP_CREATE;

	/* environement id */
	arg.info.create.i_vrid = req_info->info.inst.i_env_id;

	/* get the instance name of the application */
	memcpy(&arg.info.create.i_inst_name, &req_info->info.inst.i_inst_name, sizeof(SaNameT));

	arg.info.create.i_mbx = &gl_mac_mbx.mac_mbx;
	arg.info.create.i_lm_cbfnc = (MAB_LM_CB)mac_evt_cb;
	arg.info.create.i_hm_poolid = 0;
	status = ncsmac_lm(&arg);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR,
				   MAB_MAC_ERR_CREATE_FAILED, status, req_info->info.inst.i_env_id);
		return status;
	}

	/* Get the mac key value into creator space */
	req_info->info.inst.o_inst_hdl = arg.info.create.o_mac_hdl;

	MAC_INST *inst = (MAC_INST *)ncshm_take_hdl(NCS_SERVICE_ID_MAB, (uns32)req_info->info.inst.o_inst_hdl);
	if (inst == NULL)
		return NCSCC_RC_FAILURE;

	/* waiting for the mas sync */
	if (inst->mas_here == FALSE) {
		m_NCS_SEL_OBJ_ZERO(&set);
		m_NCS_SEL_OBJ_SET(inst->mas_sync_sel, &set);
		m_NCS_SEL_OBJ_SELECT(inst->mas_sync_sel, &set, 0, 0, &timeout);
	}
	m_NCS_SEL_OBJ_DESTROY(inst->mas_sync_sel);
	memset(&inst->mas_sync_sel, 0, sizeof(NCS_SEL_OBJ));

	ncshm_give_hdl((uns32)req_info->info.inst.o_inst_hdl);
	/* add the Environment to the list of all the envs */
	status = mac_inst_list_add(req_info->info.inst.i_env_id,
				   req_info->info.inst.o_inst_hdl, req_info->info.inst.i_inst_name);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
				   MAB_MAC_ERR_INST_LIST_ADD_FAILED,
				   req_info->info.inst.i_env_id, req_info->info.inst.o_inst_hdl);
		return status;
	}

	/* log that MAA has been installed successfully */
	m_LOG_MAB_HDLN_II(NCSFL_SEV_INFO, MAB_HDLN_MAC_INSTANTIATE_SUCCESS,
			  req_info->info.inst.i_env_id, req_info->info.inst.o_inst_hdl);

	/* time to report the status */
	return status;
}

/****************************************************************************\
*  Name:          maclib_mac_uninstantiate                                   * 
*                                                                            *
*  Description:   To uninstatntiate MAA in a particular environment          *
*                                                                            *
*  Arguments:     PW_ENV_ID  - Environment id from where MAA has to be       *
*                              uninstantiated                                *
*                 SaNameT    - instance name                                 * 
*                 uns32 *i_mac_handle - Handle of the MAA to be uninstalled  * 
*                 NCS_BOOL i_spir_cleanup - Should the SPIR to be cleanedup? *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS  - Everything went well                   *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32
maclib_mac_uninstantiate(PW_ENV_ID env_id, SaNameT i_inst_name, uns32 i_mac_handle, NCS_BOOL i_spir_cleanup)
{
	uns32 status;
	NCSMAC_LM_ARG arg;
	NCS_SPIR_REQ_INFO spir_info;

	/* destroy this MAA instance */
	memset(&arg, 0, sizeof(NCSMAC_LM_ARG));
	arg.i_op = NCSMAC_LM_OP_DESTROY;
	arg.info.destroy.i_env_id = env_id;
	memcpy(&arg.info.destroy.i_inst_name, &i_inst_name, sizeof(SaNameT));
	arg.info.destroy.i_mac_hdl = i_mac_handle;
	status = ncsmac_lm(&arg);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, MAB_OAC_ERR_DESTROY_FAILED, status, env_id);
		return status;
	}

	/* cleanup the SPIR database? */
	if (i_spir_cleanup == TRUE) {
		/* deregister the MAA handle of this Environment with SPIR */
		memset(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO));
		spir_info.type = NCS_SPIR_REQ_REL_INST;
		spir_info.i_sp_abstract_name = m_MAA_SP_ABST_NAME;
		spir_info.i_environment_id = env_id;
		memcpy(&spir_info.i_instance_name, &i_inst_name, sizeof(SaNameT));
		status = ncs_spir_api(&spir_info);
		if (status != NCSCC_RC_SUCCESS) {
			/* log the failure */
			m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
					   MAB_MAC_ERR_SPIR_RMV_INST_FAILED, status, env_id);
			return status;
		}
	}

	/* remove the node from the list of MAAs in the system */
	mac_inst_list_release(env_id);

	/* log that uninstall for MAA in this PWE is successful */
	m_LOG_MAB_HDLN_I(NCSFL_SEV_INFO, MAB_HDLN_MAC_UNINSTANTIATE_SUCCESS, env_id);

	return status;
}

/****************************************************************************\
*  Name:          maclib_mac_destroy                                         * 
*                                                                            *
*  Description:   To Destroy all the MAA instances in this process           *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO * - input details                         * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS                                           *
*  NOTE:          Even if there is an error in uninstantiating a MAA, this   * 
*                 routine will continue to uninstall all the instances of    * 
*                 MAA in a prcoess                                           * 
\****************************************************************************/
/* Removing the static specifier to make it extern function. */
uns32 maclib_mac_destroy(NCS_LIB_REQ_INFO *req_info)
{
	uns32 status = NCSCC_RC_FAILURE;
	NCS_SPLR_REQ_INFO splr_info;
	MAB_INST_NODE *this_mac_inst;
	MAB_INST_NODE *next_mac_inst;

	if (gl_mac_inited == FALSE)
		return status;

	gl_mac_inited = FALSE;

	/* start from the beginig of the list */
	this_mac_inst = gl_mac_inst_list;

	/* Destroy all the MAA instances */
	while (this_mac_inst) {
		/* store the nex MAC address */
		next_mac_inst = this_mac_inst->next;

		/* uninstantiate this MAA */
		status = maclib_mac_uninstantiate(this_mac_inst->i_env_id,
						  this_mac_inst->i_inst_name, this_mac_inst->i_hdl, TRUE);

		/* not bothered about the return code, continue to the next instance */

		/* goto the next instance of MAA */
		this_mac_inst = next_mac_inst;
	}

	/* deregister with SPLR */
	memset(&splr_info, 0, sizeof(NCS_SPLR_REQ_INFO));
	splr_info.type = NCS_SPLR_REQ_DEREG;
	splr_info.i_sp_abstract_name = m_MAA_SP_ABST_NAME;
	status = ncs_splr_api(&splr_info);
	if (status != NCSCC_RC_SUCCESS) {
		/* log the failure */
		m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR, MAB_MAC_ERR_SPLR_DEREG_FAILED, status, 0);
		return status;
	}

	/* unbind with DTS */
	mab_log_unbind();

	/* release the task related stuff */
	m_NCS_TASK_STOP(gl_mac_mbx.mac_mbx_hdl);
	m_NCS_TASK_RELEASE(gl_mac_mbx.mac_mbx_hdl);

	/* detach the IPC */
	m_NCS_IPC_DETACH(&gl_mac_mbx.mac_mbx, NULL, NULL);

	/* release the IPC */
	m_NCS_IPC_RELEASE(&gl_mac_mbx.mac_mbx, mac_leave_on_queue_cb);

	return status;
}

/****************************************************************************\
*  Name:          mac_inst_list_add                                          * 
*                                                                            *
*  Description:   To add this instance of MAA in the list                    *
*                                                                            *
*  Arguments:     PW_ENV_ID - Environment id to be added to the list         * 
*                 i_mac_hdl - MAA handle of this environment                 *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS  - Everything went well                   *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32 mac_inst_list_add(PW_ENV_ID env_id, uns32 i_mac_hdl, SaNameT i_inst_name)
{
	MAB_INST_NODE *add_me;

	/* allocate the home */
	add_me = m_MMGR_ALLOC_MAB_INST_NODE;
	if (add_me == NULL) {
		/* log the memory failure */
		m_LOG_MAB_MEMFAIL(NCSFL_SEV_ERROR, MAB_MF_MAB_INST_NODE_ALLOC_FAILED, "mac_inst_list_add()");
		return NCSCC_RC_FAILURE;
	}

	/* occupy */
	memset(add_me, 0, sizeof(MAB_INST_NODE));
	add_me->i_env_id = env_id;
	add_me->i_hdl = i_mac_hdl;
	memcpy(&add_me->i_inst_name, &i_inst_name, sizeof(SaNameT));

	/* register with the society */
	add_me->next = gl_mac_inst_list;
	gl_mac_inst_list = add_me;

	m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_MAC_ENV_INST_ADD_SUCCESS, env_id);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          mac_inst_list_release                                      * 
*                                                                            *
*  Description:   To delete and free this instance of MAA in the list        *
*                                                                            *
*  Arguments:     PW_ENV_ID - Environment id to be added to the list         * 
*                 i_mac_hdl - MAA handle of this environment                 *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS  - Everything went well                   *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
static void mac_inst_list_release(PW_ENV_ID env_id)
{
	MAB_INST_NODE *release_me;
	MAB_INST_NODE *prev = NULL;

	/* find the home */
	release_me = gl_mac_inst_list;
	while (release_me) {
		if (release_me->i_env_id == env_id)
			break;

		/* search in the next */
		prev = release_me;
		release_me = release_me->next;
	}

	/* node not found */
	if (release_me == NULL) {
		/* log that reqd env-id is not found in the list */
		m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_MAC_ENV_INST_NOT_FOUND, env_id);
		return;
	}

	if (prev == NULL) {
		/* delete the first node */
		prev = release_me->next;
		gl_mac_inst_list = prev;
	} else {
		/* delete the intermediate node */
		prev->next = release_me->next;
	}

	/* log that reqd env-id is found in the list */
	m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_MAC_ENV_INST_REL_SUCCESS, env_id);

	/* release the home */
	m_MMGR_FREE_MAB_INST_NODE(release_me);

	return;
}

/****************************************************************************\
*  Name:          mac_leave_on_queue_cb                                      * 
*                                                                            *
*  Description:   This is callback function to free the pending message      *
*                 in the mailbox.                                            *
*                                                                            *
*  Arguments:     arg2 - Pointer to the message to be freed.                 * 
*                                                                            * 
*  Returns:       TRUE  - Everything went well                               *
*  NOTE:                                                                     * 
\****************************************************************************/
static NCS_BOOL mac_leave_on_queue_cb(void *arg1, void *arg2)
{
	MAB_MSG *msg;

	if ((msg = (MAB_MSG *)arg2) != NULL) {
		m_MMGR_FREE_MAB_MSG(msg);
	}
	return TRUE;
}

#endif   /* NCS_MAB == 1 */
