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

  This file contains the initialization and destroy routines for MQA library.
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#include "mqa.h"

/* global cb handle */
uint32_t gl_mqa_hdl = 0;
MQA_CB mqa_cb;
static uint32_t mqa_use_count = 0;
/* MQA Agent creation specific LOCK */
static uint32_t mqa_agent_lock_create = 0;
NCS_LOCK mqa_agent_lock;

#define m_MQA_AGENT_LOCK                        \
   if (!mqa_agent_lock_create++)                \
   {                                            \
      m_NCS_LOCK_INIT(&mqa_agent_lock);         \
   }                                            \
   mqa_agent_lock_create = 1;                   \
   m_NCS_LOCK(&mqa_agent_lock, NCS_LOCK_WRITE);

#define m_MQA_AGENT_UNLOCK m_NCS_UNLOCK(&mqa_agent_lock, NCS_LOCK_WRITE)

static void mqa_sync_with_mqnd(MQA_CB *cb);
static void mqa_sync_with_mqd(MQA_CB *cb);
static void mqa_asapi_unregister(MQA_CB *cb);
static void mqa_client_tree_destroy(MQA_CB *mqa_cb);
static void mqa_client_tree_cleanup(MQA_CB *mqa_cb);
static void mqa_queue_tree_destroy(MQA_CB *mqa_cb);
static void mqa_queue_tree_cleanup(MQA_CB *mqa_cb);
static uint32_t mqa_create(NCS_LIB_CREATE *create_info);
static uint32_t mqa_destroy(NCS_LIB_DESTROY *destroy_info);
static uint32_t mqa_asapi_register(MQA_CB *cb);
static uint32_t mqa_client_tree_init(MQA_CB *cb);
static uint32_t mqa_queue_tree_init(MQA_CB *cb);

/****************************************************************************
  Name          : mqa_lib_req
 
  Description   : This routine is exported to the external entities & is used
                  to create & destroy the MQSv library.
 
  Arguments     : req_info - ptr to the request info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uint32_t mqa_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t rc = NCSCC_RC_SUCCESS;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		rc = mqa_create(&req_info->info.create);
		if (rc == NCSCC_RC_SUCCESS)
			m_LOG_MQSV_A(MQA_SE_API_CREATE_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_INFO, rc, __FILE__,
				     __LINE__);
		else
			m_LOG_MQSV_A(MQA_SE_API_CREATE_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
		break;

	case NCS_LIB_REQ_DESTROY:
		rc = mqa_destroy(&req_info->info.destroy);
		if (rc == NCSCC_RC_SUCCESS)
			m_LOG_MQSV_A(MQA_SE_API_DESTROY_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_INFO, rc, __FILE__,
				     __LINE__);
		else
			m_LOG_MQSV_A(MQA_SE_API_DESTROY_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
		break;

	default:
		m_LOG_MQSV_A(MQA_SE_API_UNKNOWN_REQUEST, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__);
		break;
	}

	return rc;
}

/********************************************************************
 Name    :  mqa_sync_with_mqd

 Description : This is for MQA to sync with MQD when it gets MDS callback

**********************************************************************/
static void mqa_sync_with_mqd(MQA_CB *cb)
{
	NCS_SEL_OBJ_SET set;
	uint32_t timeout = 3000;

	m_NCS_LOCK(&cb->mqd_sync_lock, NCS_LOCK_WRITE);

	if (cb->is_mqd_up) {
		m_LOG_MQSV_A(MQA_MQD_ALREADY_UP, NCSFL_LC_MQSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__);
		m_NCS_UNLOCK(&cb->mqd_sync_lock, NCS_LOCK_WRITE);
		return;
	}

	cb->mqd_sync_awaited = TRUE;
	m_NCS_SEL_OBJ_CREATE(&cb->mqd_sync_sel);
	m_NCS_UNLOCK(&cb->mqd_sync_lock, NCS_LOCK_WRITE);

	/* Await indication from MDS saying MQND is up */
	m_NCS_SEL_OBJ_ZERO(&set);
	m_NCS_SEL_OBJ_SET(cb->mqd_sync_sel, &set);
	m_NCS_SEL_OBJ_SELECT(cb->mqd_sync_sel, &set, 0, 0, &timeout);

	/* Destroy the sync - object */
	m_NCS_LOCK(&cb->mqd_sync_lock, NCS_LOCK_WRITE);

	cb->mqd_sync_awaited = FALSE;
	m_NCS_SEL_OBJ_DESTROY(cb->mqd_sync_sel);

	m_NCS_UNLOCK(&cb->mqd_sync_lock, NCS_LOCK_WRITE);
	m_LOG_MQSV_A(MQA_SYNC_WITH_MQD_UP, NCSFL_LC_MQSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__);
	return;
}

/********************************************************************
 Name    :  mqa_sync_with_mqnd

 Description : This is for MQA to sync with MQND when it gets MDS callback
 
**********************************************************************/
static void mqa_sync_with_mqnd(MQA_CB *cb)
{
	NCS_SEL_OBJ_SET set;
	uint32_t timeout = 3000;

	m_NCS_LOCK(&cb->mqnd_sync_lock, NCS_LOCK_WRITE);

	if (cb->is_mqnd_up) {
		m_LOG_MQSV_A(MQA_MQND_ALREADY_UP, NCSFL_LC_MQSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__);
		m_NCS_UNLOCK(&cb->mqnd_sync_lock, NCS_LOCK_WRITE);
		return;
	}

	cb->mqnd_sync_awaited = TRUE;
	m_NCS_SEL_OBJ_CREATE(&cb->mqnd_sync_sel);
	m_NCS_UNLOCK(&cb->mqnd_sync_lock, NCS_LOCK_WRITE);

	/* Await indication from MDS saying MQND is up */
	m_NCS_SEL_OBJ_ZERO(&set);
	m_NCS_SEL_OBJ_SET(cb->mqnd_sync_sel, &set);
	m_NCS_SEL_OBJ_SELECT(cb->mqnd_sync_sel, &set, 0, 0, &timeout);

	/* Destroy the sync - object */
	m_NCS_LOCK(&cb->mqnd_sync_lock, NCS_LOCK_WRITE);

	cb->mqnd_sync_awaited = FALSE;
	m_NCS_SEL_OBJ_DESTROY(cb->mqnd_sync_sel);

	m_NCS_UNLOCK(&cb->mqnd_sync_lock, NCS_LOCK_WRITE);
	m_LOG_MQSV_A(MQA_SYNC_WITH_MQND_UP, NCSFL_LC_MQSV_INIT, NCSFL_SEV_INFO, 1, __FILE__, __LINE__);
	return;
}

/****************************************************************************
  Name          : mqa_create
 
  Description   : This routine creates & initializes the MQA control block.
 
  Arguments     : create_info - ptr to the create info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static uint32_t mqa_create(NCS_LIB_CREATE *create_info)
{
	MQA_CB *cb = &mqa_cb;

	uint32_t rc = NCSCC_RC_SUCCESS;
	/* validate create info */
	if (create_info == NULL)
		return NCSCC_RC_FAILURE;

	/* Register with Logging subsystem */
	mqa_flx_log_reg();

	memset(cb, 0, sizeof(MQA_CB));

	/* assign the MQA pool-id (used by hdl-mngr) */
	cb->pool_id = NCS_HM_POOL_ID_COMMON;

	/* create the association with hdl-mngr */
	if (!(cb->agent_handle_id = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_MQA, (NCSCONTEXT)cb))) {
		m_LOG_MQSV_A(MQA_CREATE_HANDLE_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__);
		goto error1;
	}

	m_LOG_MQSV_A(MQA_CREATE_HANDLE_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_NOTICE, 1, __FILE__, __LINE__);
	/* everything went off well.. store the hdl in the global variable */
	gl_mqa_hdl = cb->agent_handle_id;

	/* get the process id */
	cb->process_id = getpid();

	/* initialize the mqa cb lock */
	if ((rc = m_NCS_LOCK_INIT(&cb->cb_lock)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_CB_LOCK_INIT_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__, __LINE__);
		goto error2;
	}
	m_LOG_MQSV_A(MQA_CB_LOCK_INIT_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_NOTICE, rc, __FILE__, __LINE__);

	/* initialize the client tree */
	if ((rc = mqa_client_tree_init(cb)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_CLIENT_TREE_INIT_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		goto error3;
	}

	m_LOG_MQSV_A(MQA_CLIENT_TREE_INIT_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_NOTICE, rc, __FILE__, __LINE__);
	/* initialize the queue tree */
	if ((rc = mqa_queue_tree_init(cb)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_QUEUE_TREE_INIT_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		goto error4;
	}
	m_LOG_MQSV_A(MQA_QUEUE_TREE_INIT_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_NOTICE, rc, __FILE__, __LINE__);

	/* EDU initialisation */
	if ((rc = m_NCS_EDU_HDL_INIT(&cb->edu_hdl)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_EDU_HDL_INIT_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		goto error5;
	}

	m_LOG_MQSV_A(MQA_EDU_HDL_INIT_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_NOTICE, rc, __FILE__, __LINE__);
	/* register with MDS */
	if ((rc = mqa_mds_register(cb)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_MDS_REGISTER_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		goto error6;
	} else
		m_LOG_MQSV_A(MQA_MDS_REGISTER_SUCCESS, NCSFL_LC_MQSV_INIT, NCSFL_SEV_INFO, rc, __FILE__, __LINE__);

	/* Update clm_node_joined flag to 1 */

	cb->clm_node_joined = 1;

	mqa_sync_with_mqd(cb);

	mqa_sync_with_mqnd(cb);

	/* initialize the timeout linked list */
	if ((rc = mqa_timer_table_init(cb)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_TIMER_TABLE_INIT_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		goto error7;
	}

	if ((rc = mqa_asapi_register(cb)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_REGISTER_WITH_ASAPi_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
			     __LINE__);
		goto error8;
	}

	return NCSCC_RC_SUCCESS;

 error8:
	mqa_timer_table_destroy(cb);

 error7:
	/* MDS unregister. */
	mqa_mds_unregister(cb);

 error6:
	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);

 error5:
	/* delete the tree */
	mqa_queue_tree_destroy(cb);

 error4:
	/* delete the tree */
	mqa_client_tree_destroy(cb);
 error3:
	/* destroy the lock */
	m_NCS_LOCK_DESTROY(&cb->cb_lock);

 error2:
	/* remove the association with hdl-mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_MQA, cb->agent_handle_id);
 error1:
	/* de register with the flex log */
	mqa_flx_log_dereg();

	return NCSCC_RC_FAILURE;
}

/****************************************************************************
  Name          : mqa_destroy
 
  Description   : This routine destroys the MQA control block.
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static uint32_t mqa_destroy(NCS_LIB_DESTROY *destroy_info)
{
	MQA_CB *cb = 0;

	/* validate the CB */
	cb = (MQA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_MQA, gl_mqa_hdl);
	if (!cb)
		return NCSCC_RC_FAILURE;

	/* return MQA CB */
	ncshm_give_hdl(gl_mqa_hdl);

	/* remove the association with hdl-mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_MQA, cb->agent_handle_id);

	if (m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE) != NCSCC_RC_SUCCESS)	{
		TRACE("mqa_destroy Failed to acquire lock");
		return NCSCC_RC_FAILURE;
	}

	mqa_timer_table_destroy(cb);

	/* Unregister with ASAPi */
	mqa_asapi_unregister(cb);

	/* MDS unregister. */
	mqa_mds_unregister(cb);

	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);

	mqa_queue_tree_destroy(cb);

	/* delete all the client info */
	mqa_client_tree_destroy(cb);

	m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

	/* destroy the lock */
	m_NCS_LOCK_DESTROY(&cb->cb_lock);

	/* de register with the flex log */
	mqa_flx_log_dereg();

	/* reset the global cb handle */
	gl_mqa_hdl = 0;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : mqa_asapi_register
 
  Description   : This routine registers with ASAPi library.
 
  Arguments     : cb - MQA control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static uint32_t mqa_asapi_register(MQA_CB *cb)
{
	ASAPi_OPR_INFO asapi_or;
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* Register with ASAPi library */

	asapi_or.type = ASAPi_OPR_BIND;
	asapi_or.info.bind.i_indhdlr = mqa_asapi_msghandler;
	asapi_or.info.bind.i_mds_hdl = cb->mqa_mds_hdl;
	asapi_or.info.bind.i_mds_id = NCSMDS_SVC_ID_MQA;
	asapi_or.info.bind.i_my_id = NCS_SERVICE_ID_MQA;
	asapi_or.info.bind.i_mydest = cb->mqa_mds_dest;

	if ((rc = asapi_opr_hdlr(&asapi_or)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_ASAPi_REGISTER_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : mqa_asapi_unregister
 
  Description   : This routine registers with ASAPi library.
 
  Arguments     : cb - MQA control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static void mqa_asapi_unregister(MQA_CB *cb)
{
	ASAPi_OPR_INFO asapi_or;
	uint32_t rc = NCSCC_RC_SUCCESS;
	/* Register with ASAPi library */

	asapi_or.type = ASAPi_OPR_UNBIND;

	if ((rc = asapi_opr_hdlr(&asapi_or)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_ASAPi_UNREGISTER_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
	}

	return;
}

/****************************************************************************
  Name          : mqa_client_tree_init
  
  Description   : This routine is used to initialize the client tree
 
  Arguments     : cb - pointer to the MQA Control Block
                   
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static uint32_t mqa_client_tree_init(MQA_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
	param.key_size = sizeof(SaMsgHandleT);
	if (ncs_patricia_tree_init(&cb->mqa_client_tree, &param) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : mqa_client_tree_destroy
 
  Description   : This routine destroys the MQA client tree.
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
static void mqa_client_tree_destroy(MQA_CB *mqa_cb)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	/* take the cb lock */
	if ((rc = m_NCS_LOCK(&mqa_cb->cb_lock, NCS_LOCK_WRITE)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_CLIENT_TREE_DESTROY_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
			     __LINE__);
		return;
	}
	/* cleanup the client tree */
	mqa_client_tree_cleanup(mqa_cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&mqa_cb->mqa_client_tree);

	/* giveup the cb lock */
	m_NCS_UNLOCK(&mqa_cb->cb_lock, NCS_LOCK_WRITE);

	return;
}

/****************************************************************************
  Name          : mqa_client_tree_cleanup
 
  Description   : This routine cleansup the MQA Client tree
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
static void mqa_client_tree_cleanup(MQA_CB *mqa_cb)
{
	MQA_CLIENT_INFO *client_info;
	SaMsgHandleT *temp_ptr = 0;
	SaMsgHandleT temp_hdl = 0;

	/* scan the entire handle db & delete each record */
	while ((client_info = (MQA_CLIENT_INFO *)
		ncs_patricia_tree_getnext(&mqa_cb->mqa_client_tree, (uint8_t *const)temp_ptr))) {
		/* delete the client info */
		temp_hdl = client_info->msgHandle;
		temp_ptr = &temp_hdl;
		mqsv_mqa_callback_queue_destroy(client_info);
		mqa_client_tree_delete_node(mqa_cb, client_info);
	}

	return;
}

/****************************************************************************
  Name          : mqa_is_track_enabled
 
  Description   : This routine checks if there is any tracking
                  enabled for a group
 
  Arguments     : mqa_cb - MQA Control block.
                  queueGroupName - Group name
 
  Return Values : TRUE if this is no tracking enabled for this group. FALSE
                  otherwise
 
  Notes         : None
******************************************************************************/
NCS_BOOL mqa_is_track_enabled(MQA_CB *mqa_cb, SaNameT *queueGroupName)
{
	MQA_CLIENT_INFO *client_info;
	MQA_TRACK_INFO *track_info;
	SaMsgHandleT *temp_ptr = 0;
	SaMsgHandleT temp_hdl = 0;

	/* scan the entire handle db & delete each record */
	while ((client_info = (MQA_CLIENT_INFO *)
		ncs_patricia_tree_getnext(&mqa_cb->mqa_client_tree, (uint8_t *const)temp_ptr))) {
		track_info = mqa_track_tree_find_and_add(client_info, queueGroupName, FALSE);
		if (!track_info) {
			temp_hdl = client_info->msgHandle;
			temp_ptr = &temp_hdl;
			continue;
		} else
			return TRUE;
	}

	return FALSE;
}

/****************************************************************************
  Name          : mqa_client_tree_delete_node
 
  Description   : This routine adds the deletes the client from the client tree
 
  Arguments     : client_info - pointer to the client node.
                  
 
  Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 
  Notes         : None
******************************************************************************/
uint32_t mqa_client_tree_delete_node(MQA_CB *mqa_cb, MQA_CLIENT_INFO *client_info)
{
	MQA_TRACK_INFO *track_info;
	SaNameT *temp_ptr = 0;
	SaNameT temp_name;
	uint8_t *value = NULL;
	uint32_t rc = NCSCC_RC_SUCCESS;
	/* scan the entire group track db & delete each record */
	while ((track_info = (MQA_TRACK_INFO *)ncs_patricia_tree_getnext(&client_info->mqa_track_tree, value))) {
		/* delete the track info */
		temp_name = track_info->queueGroupName;
		temp_ptr = &temp_name;
		value = temp_ptr->value;
		/* delete from the tree */
		if ((rc = ncs_patricia_tree_del(&client_info->mqa_track_tree,
						&track_info->patnode)) != NCSCC_RC_SUCCESS)
			m_LOG_MQSV_A(MQA_TRACK_TREE_DEL_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);

		/* free the mem */
		if (track_info->notificationBuffer.notification)
			m_MMGR_FREE_MQA_TRACK_BUFFER_INFO(track_info->notificationBuffer.notification);
		m_MMGR_FREE_MQA_TRACK_INFO(track_info);
	}

	/* destroy the tree */
	ncs_patricia_tree_destroy(&client_info->mqa_track_tree);

	/* delete from the tree */
	if ((rc = ncs_patricia_tree_del(&mqa_cb->mqa_client_tree, &client_info->patnode)) != NCSCC_RC_SUCCESS)
		m_LOG_MQSV_A(MQA_CLIENT_TREE_DEL_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);

	/* free the mem */
	m_MMGR_FREE_MQA_CLIENT_INFO(client_info);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : mqa_client_tree_find_and_add
 
  Description   : This routine adds the new client to the client tree and creates
                  the selection object for that client.
 
  Arguments     : 
                  mqa_cb : pointer to the mqa control block.
                  hdl_id : the handle id.
                  flag   : TRUE/FALSE if TRUE create the new node if node 
                           doesn't exist.FALSE -> search for an existing node.
 
  Return Values : returns the MQA_CLIENT_INFO node.
 
  Notes         : The caller takes the cb lock before calling this function
                  
******************************************************************************/
MQA_CLIENT_INFO *mqa_client_tree_find_and_add(MQA_CB *mqa_cb, SaMsgHandleT hdl_id, NCS_BOOL flag)
{
	MQA_CLIENT_INFO *client_info = NULL;
	NCS_PATRICIA_PARAMS param;
	uint32_t rc = NCSCC_RC_SUCCESS;

	client_info = (MQA_CLIENT_INFO *)ncs_patricia_tree_get(&mqa_cb->mqa_client_tree, (uint8_t *)&hdl_id);

	if (flag == TRUE) {
		/* create and allocate the memory */
		if (!client_info) {
			client_info = (MQA_CLIENT_INFO *)m_MMGR_ALLOC_MQA_CLIENT_INFO;
			if (!client_info) {
				m_LOG_MQSV_A(MQA_CLIENT_ALLOC_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__,
					     __LINE__);
				return NULL;
			}
			memset(client_info, 0, sizeof(MQA_CLIENT_INFO));
			/* Store the client_info pointer as msghandle. */
			client_info->msgHandle = NCS_PTR_TO_UNS64_CAST(client_info);
			client_info->patnode.key_info = (uint8_t *)&client_info->msgHandle;

			if ((rc =
			     ncs_patricia_tree_add(&mqa_cb->mqa_client_tree,
						   &client_info->patnode)) != NCSCC_RC_SUCCESS) {
				m_LOG_MQSV_A(MQA_CLIENT_TREE_ADD_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc,
					     __FILE__, __LINE__);
				m_MMGR_FREE_MQA_CLIENT_INFO(client_info);
				return NULL;
			}

			/* Create the group track tree */
			memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
			param.key_size = SA_MAX_NAME_LENGTH;
			if (ncs_patricia_tree_init(&client_info->mqa_track_tree, &param) != NCSCC_RC_SUCCESS) {
				m_MMGR_FREE_MQA_CLIENT_INFO(client_info);
				return NULL;
			}

		}
	}
	return client_info;
}

/****************************************************************************
  Name          : mqa_track_tree_find_and_add

  Description   : This routine adds/searches the new group track element to the
                  client tree.

  Arguments     :
                  client_info : pointer to the mqa client.
                  hdl_id : the handle id.
                  flag   : TRUE/FALSE if TRUE create the new node if node
                           doesn't exist.FALSE -> search for an existing node.

  Return Values : returns the MQA_TRACK_INFO node.

  Notes         : The caller takes the cb lock before calling this function

******************************************************************************/
MQA_TRACK_INFO *mqa_track_tree_find_and_add(MQA_CLIENT_INFO *client_info, SaNameT *group, NCS_BOOL flag)
{
	MQA_TRACK_INFO *track_info = NULL;
	uint32_t rc = NCSCC_RC_SUCCESS;

	track_info = (MQA_TRACK_INFO *)ncs_patricia_tree_get(&client_info->mqa_track_tree, (uint8_t *)group->value);

	if (flag == TRUE) {
		/* create and allocate the memory */
		if (!track_info) {
			track_info = (MQA_TRACK_INFO *)m_MMGR_ALLOC_MQA_TRACK_INFO;
			if (!track_info) {
				m_LOG_MQSV_A(MQA_TRACK_ALLOC_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__,
					     __LINE__);
				return NULL;
			}
			memset(track_info, 0, sizeof(MQA_TRACK_INFO));
			track_info->queueGroupName = *group;
			track_info->patnode.key_info = (uint8_t *)track_info->queueGroupName.value;
			if ((rc = ncs_patricia_tree_add(&client_info->mqa_track_tree,
							&track_info->patnode)) != NCSCC_RC_SUCCESS) {
				m_LOG_MQSV_A(MQA_TRACK_TREE_ADD_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc,
					     __FILE__, __LINE__);
				if (track_info->notificationBuffer.notification)
					m_MMGR_FREE_MQA_TRACK_BUFFER_INFO(track_info->notificationBuffer.notification);
				m_MMGR_FREE_MQA_TRACK_INFO(track_info);
				return NULL;
			}

		}
	}
	return track_info;

}

/****************************************************************************
  Name          : mqa_track_tree_find_and_del

  Description   : This routine deletes a new group track element to the
                  client tree.

  Arguments     :
                  client_info : pointer to the mqa client.
                  group : the group name to delete from track list.

  Return Values : returns status of deletion, TRUE if deleted, FALSE otherwise

  Notes         : The caller takes the cb lock before calling this function

******************************************************************************/
NCS_BOOL mqa_track_tree_find_and_del(MQA_CLIENT_INFO *client_info, SaNameT *group)
{
	MQA_TRACK_INFO *track_info = NULL;
	uint32_t rc = NCSCC_RC_SUCCESS;

	track_info = (MQA_TRACK_INFO *)ncs_patricia_tree_get(&client_info->mqa_track_tree, (uint8_t *)group->value);

	if (!track_info)
		return FALSE;

	if ((rc = ncs_patricia_tree_del(&client_info->mqa_track_tree, &track_info->patnode)) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_TRACK_TREE_DEL_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);
		if (track_info->notificationBuffer.notification)
			m_MMGR_FREE_MQA_TRACK_BUFFER_INFO(track_info->notificationBuffer.notification);
		m_MMGR_FREE_MQA_TRACK_INFO(track_info);
		return FALSE;
	}

	if (track_info->notificationBuffer.notification)
		m_MMGR_FREE_MQA_TRACK_BUFFER_INFO(track_info->notificationBuffer.notification);
	m_MMGR_FREE_MQA_TRACK_INFO(track_info);

	return TRUE;
}

/****************************************************************************
  Name          : mqa_queue_tree_init
  
  Description   : This routine is used to initialize the queue tree
 
  Arguments     : cb - pointer to the MQA Control Block
                   
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static uint32_t mqa_queue_tree_init(MQA_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	uint32_t rc = NCSCC_RC_SUCCESS;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
	param.key_size = sizeof(SaMsgQueueHandleT);
	if ((rc = ncs_patricia_tree_init(&cb->mqa_queue_tree, &param)) != NCSCC_RC_SUCCESS) {
		return rc;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : mqa_client_queue_destroy
 
  Description   : This routine destroys the MQA queue tree.
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
static void mqa_queue_tree_destroy(MQA_CB *mqa_cb)
{
	uint32_t rc = NCSCC_RC_SUCCESS;
	/* take the cb lock */
	if ((rc = (m_NCS_LOCK(&mqa_cb->cb_lock, NCS_LOCK_WRITE))) != NCSCC_RC_SUCCESS) {
		m_LOG_MQSV_A(MQA_QUEUE_TREE_DESTROY_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
			     __LINE__);
		return;
	}

	/* cleanup the client tree */
	mqa_queue_tree_cleanup(mqa_cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&mqa_cb->mqa_queue_tree);

	/* giveup the cb lock */
	m_NCS_UNLOCK(&mqa_cb->cb_lock, NCS_LOCK_WRITE);

	return;
}

/****************************************************************************
  Name          : mqa_queue_tree_cleanup
 
  Description   : This routine cleansup the MQA queue tree
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
static void mqa_queue_tree_cleanup(MQA_CB *mqa_cb)
{
	MQA_QUEUE_INFO *queue_info;
	SaMsgQueueHandleT *temp_ptr = NULL;
	SaMsgQueueHandleT temp_hdl;
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* scan the entire handle db & delete each record */
	while ((queue_info = (MQA_QUEUE_INFO *)
		ncs_patricia_tree_getnext(&mqa_cb->mqa_queue_tree, (uint8_t *const)temp_ptr))) {
		temp_hdl = queue_info->queueHandle;
		temp_ptr = &temp_hdl;

		/* delete the client info */
		if ((rc = ncs_patricia_tree_del(&mqa_cb->mqa_queue_tree, &queue_info->patnode)) != NCSCC_RC_SUCCESS) {
			m_LOG_MQSV_A(MQA_QUEUE_TREE_DEL_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__,
				     __LINE__);
		} else {
			/* free the mem */
			m_MMGR_FREE_MQA_QUEUE_INFO(queue_info);
		}

	}

	return;
}

/****************************************************************************
  Name          : mqa_queue_tree_find_and_add
 
  Description   : This routine adds the new queue to the queue tree
 
  Arguments     : 
                  mqa_cb : pointer to the mqa control block.
                  hdl_id : the handle id.
                  flag   : TRUE/FALSE if TRUE create the new node if node 
                           doesn't exist.FALSE -> search for an existing node.
                 MQA_CLIENT_INFO *client_info : Info about the client.
                 SaMsgQueueOpenFlagsT openFlags : 
                          If flag == TRUE, openFlags is IN parameter.
                          If flag != TRUE, openFlags is ignored.
  Return Values : returns the MQA_QUEUE_INFO node.
 
  Notes         : None
******************************************************************************/
MQA_QUEUE_INFO *mqa_queue_tree_find_and_add(MQA_CB *mqa_cb,
					    SaMsgQueueHandleT hdl_id,
					    NCS_BOOL flag, MQA_CLIENT_INFO *client_info, SaMsgQueueOpenFlagsT openFlags)
{
	MQA_QUEUE_INFO *queue_info = NULL;
	uint32_t rc = NCSCC_RC_SUCCESS;
	/* read lock taken by the caller. */
	queue_info = (MQA_QUEUE_INFO *)ncs_patricia_tree_get(&mqa_cb->mqa_queue_tree, (uint8_t *)&hdl_id);

	if (flag == TRUE) {
		/* create and allocate the memory */
		if (!queue_info) {
			queue_info = (MQA_QUEUE_INFO *)m_MMGR_ALLOC_MQA_QUEUE_INFO;
			if (!queue_info) {
				m_LOG_MQSV_A(MQA_QUEUE_ALLOC_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, 0, __FILE__,
					     __LINE__);
				return NULL;
			}
			memset(queue_info, 0, sizeof(MQA_QUEUE_INFO));
			queue_info->queueHandle = hdl_id;
			queue_info->openFlags = openFlags;
			queue_info->client_info = client_info;
			queue_info->patnode.key_info = (uint8_t *)&queue_info->queueHandle;
			queue_info->msg_get_count = 0;

			if ((rc =
			     ncs_patricia_tree_add(&mqa_cb->mqa_queue_tree,
						   &queue_info->patnode)) != NCSCC_RC_SUCCESS) {
				m_LOG_MQSV_A(MQA_QUEUE_TREE_ADD_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc,
					     __FILE__, __LINE__);
				m_MMGR_FREE_MQA_QUEUE_INFO(queue_info);

				return NULL;
			}

		}
	}
	return queue_info;
}

/****************************************************************************
  Name          : mqa_queue_tree_delete_node
 
  Description   : This routine adds the deletes the queue from the queue tree
 
  Arguments     : queue_info - pointer to the queue node.
                  
 
  Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 
  Notes         : None
******************************************************************************/
uint32_t mqa_queue_tree_delete_node(MQA_CB *mqa_cb, SaMsgQueueHandleT hdl_id)
{

	MQA_QUEUE_INFO *queue_info = NULL;
	uint32_t rc = NCSCC_RC_SUCCESS;
	queue_info = (MQA_QUEUE_INFO *)ncs_patricia_tree_get(&mqa_cb->mqa_queue_tree, (uint8_t *)&hdl_id);
	if (!queue_info) {

		return NCSCC_RC_FAILURE;
	}

	/* delete from the tree */
	if ((rc = ncs_patricia_tree_del(&mqa_cb->mqa_queue_tree, &queue_info->patnode)) != NCSCC_RC_SUCCESS)
		m_LOG_MQSV_A(MQA_QUEUE_TREE_DEL_FAILED, NCSFL_LC_MQSV_INIT, NCSFL_SEV_ERROR, rc, __FILE__, __LINE__);

	if (queue_info->task_handle != 0) {
		/* m_NCS_TASK_STOP(queue_info->task_handle);
		   m_NCS_TASK_RELEASE(queue_info->task_handle); */
	}

	/* free the mem */
	m_MMGR_FREE_MQA_QUEUE_INFO(queue_info);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncs_mqa_startup

  Description   :  This routine creates a MQSv agent infrastructure to interface
                   with MQSv service. Once the infrastructure is created from
                   then on use_count is incremented for every startup request.

  Arguments     :  - NIL-

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
unsigned int ncs_mqa_startup(void)
{
	NCS_LIB_REQ_INFO lib_create;
	char *value = NULL;

	m_MQA_AGENT_LOCK;
	if (mqa_use_count > 0) {
		/* Already created, so just increment the use_count */
		mqa_use_count++;
		m_MQA_AGENT_UNLOCK;
		return NCSCC_RC_SUCCESS;
	}

   /*** Init MQA ***/
	memset(&lib_create, 0, sizeof(lib_create));
	lib_create.i_op = NCS_LIB_REQ_CREATE;
	if (mqa_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		m_MQA_AGENT_UNLOCK;
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	} else {
		m_NCS_DBG_PRINTF("\nMQSV:MQA:ON");
		mqa_use_count = 1;
	}

	/* Initialize trace system first of all so we can see what is going. */
	if ((value = getenv("MQA_TRACE_PATHNAME")) != NULL) {
		logtrace_init("mqa", value, CATEGORY_ALL);
	}

	m_MQA_AGENT_UNLOCK;
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncs_mqa_shutdown 

  Description   :  This routine destroys the MQSv agent infrastructure created 
                   to interface MQSv service. If the registered users are > 1, 
                   it just decrements the use_count.   

  Arguments     :  - NIL -

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
unsigned int ncs_mqa_shutdown(void)
{
	uint32_t rc = NCSCC_RC_SUCCESS;

	m_MQA_AGENT_LOCK;
	if (mqa_use_count > 1) {
		/* Still users extis, so just decrement the use_count */
		mqa_use_count--;
	} else if (mqa_use_count == 1) {
		NCS_LIB_REQ_INFO lib_destroy;

		memset(&lib_destroy, 0, sizeof(lib_destroy));
		lib_destroy.i_op = NCS_LIB_REQ_DESTROY;

		rc = mqa_lib_req(&lib_destroy);

		mqa_use_count = 0;
	}

	m_MQA_AGENT_UNLOCK;
	return rc;
}
