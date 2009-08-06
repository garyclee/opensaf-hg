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

  This file contains the initialization and destroy routines for AvA library.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "ava.h"

/* global cb handle */
uns32 gl_ava_hdl = 0;
static uns32 ava_use_count = 0;

/* AVA Agent creation specific LOCK */
static uns32 ava_agent_lock_create = 0;
NCS_LOCK ava_agent_lock;

#define m_AVA_AGENT_LOCK                        \
   if (!ava_agent_lock_create++)                \
   {                                            \
      m_NCS_LOCK_INIT(&ava_agent_lock);         \
   }                                            \
   ava_agent_lock_create = 1;                   \
   m_NCS_LOCK(&ava_agent_lock, NCS_LOCK_WRITE);

#define m_AVA_AGENT_UNLOCK m_NCS_UNLOCK(&ava_agent_lock, NCS_LOCK_WRITE)

/****************************************************************************
  Name          : ava_lib_req
 
  Description   : This routine is exported to the external entities & is used
                  to create & destroy the AvA library.
 
  Arguments     : req_info - ptr to the request info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 ava_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		rc = ava_create(&req_info->info.create);
		if (NCSCC_RC_SUCCESS == rc) {
			m_AVA_LOG_SEAPI(AVSV_LOG_SEAPI_CREATE, AVSV_LOG_SEAPI_SUCCESS, NCSFL_SEV_INFO);
		} else {
			m_AVA_LOG_SEAPI(AVSV_LOG_SEAPI_CREATE, AVSV_LOG_SEAPI_FAILURE, NCSFL_SEV_CRITICAL);
		}
		break;

	case NCS_LIB_REQ_DESTROY:
		ava_destroy(&req_info->info.destroy);
		if (NCSCC_RC_SUCCESS == rc) {
			m_AVA_LOG_SEAPI(AVSV_LOG_SEAPI_DESTROY, AVSV_LOG_SEAPI_SUCCESS, NCSFL_SEV_INFO);
		} else {
			m_AVA_LOG_SEAPI(AVSV_LOG_SEAPI_DESTROY, AVSV_LOG_SEAPI_FAILURE, NCSFL_SEV_CRITICAL);
		}
		break;

	default:
		break;
	}

	return rc;
}

/****************************************************************************
  Name          : ava_create
 
  Description   : This routine creates & initializes the AvA control block.
 
  Arguments     : create_info - ptr to the create info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 ava_create(NCS_LIB_CREATE *create_info)
{
	AVA_CB *cb = 0;
	NCS_SEL_OBJ_SET set;
	uns32 rc = NCSCC_RC_SUCCESS, timeout = 300;
	EDU_ERR err;

	/* register with dtsv */
#if (NCS_AVA_LOG == 1)
	rc = ava_log_reg();
#endif

	/* allocate AvA cb */
	if (!(cb = m_MMGR_ALLOC_AVA_CB)) {
		m_AVA_LOG_CB(AVSV_LOG_CB_CREATE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		rc = NCSCC_RC_FAILURE;
		goto error;
	}
	m_AVA_LOG_CB(AVSV_LOG_CB_CREATE, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	memset(cb, 0, sizeof(AVA_CB));

	/* fetch the comp name from the env variable */
	if (getenv("SA_AMF_COMPONENT_NAME")) {
		if (strlen(getenv("SA_AMF_COMPONENT_NAME")) < SA_MAX_NAME_LENGTH) {
			strcpy(cb->comp_name_net.value, getenv("SA_AMF_COMPONENT_NAME"));
			cb->comp_name_net.length = m_NCS_OS_HTONS((uns16)strlen(cb->comp_name_net.value));
			m_AVA_FLAG_SET(cb, AVA_FLAG_COMP_NAME);
		} else {
			rc = NCSCC_RC_FAILURE;
			goto error;
		}
	}

	/* assign the AvA pool-id (used by hdl-mngr) */
	cb->pool_id = NCS_HM_POOL_ID_COMMON;

	/* create the association with hdl-mngr */
	if (!(cb->cb_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVA, (NCSCONTEXT)cb))) {
		m_AVA_LOG_CB(AVSV_LOG_CB_HDL_ASS_CREATE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		rc = NCSCC_RC_FAILURE;
		goto error;
	}
	m_AVA_LOG_CB(AVSV_LOG_CB_HDL_ASS_CREATE, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	/* initialize the AvA cb lock */
	m_NCS_LOCK_INIT(&cb->lock);
	m_AVA_LOG_LOCK(AVSV_LOG_LOCK_INIT, AVSV_LOG_LOCK_SUCCESS, NCSFL_SEV_INFO);

	/* EDU initialisation */
	m_NCS_EDU_HDL_INIT(&cb->edu_hdl);
	m_AVA_LOG_EDU(AVSV_LOG_EDU_INIT, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	rc = m_NCS_EDU_COMPILE_EDP(&cb->edu_hdl, avsv_edp_nda_msg, &err);

	if (rc != NCSCC_RC_SUCCESS) {
		/* Log ERROR */

		goto error;
	}

	/* create the sel obj (for mds sync) */
	m_NCS_SEL_OBJ_CREATE(&cb->sel_obj);

	/* initialize the hdl db */
	if (NCSCC_RC_SUCCESS != ava_hdl_init(&cb->hdl_db)) {
		m_AVA_LOG_HDL_DB(AVA_LOG_HDL_DB_CREATE, AVA_LOG_HDL_DB_FAILURE, 0, NCSFL_SEV_CRITICAL);
		rc = NCSCC_RC_FAILURE;
		goto error;
	}
	m_AVA_LOG_HDL_DB(AVA_LOG_HDL_DB_CREATE, AVA_LOG_HDL_DB_SUCCESS, 0, NCSFL_SEV_INFO);

	m_NCS_SEL_OBJ_ZERO(&set);
	m_NCS_SEL_OBJ_SET(cb->sel_obj, &set);
	m_AVA_FLAG_SET(cb, AVA_FLAG_FD_VALID);

	/* register with MDS */
	if ((NCSCC_RC_SUCCESS != ava_mds_reg(cb))) {
		m_AVA_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
		rc = NCSCC_RC_FAILURE;
		goto error;
	}
	m_AVA_LOG_MDS(AVSV_LOG_MDS_REG, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

	/* block until mds detects avnd */
	m_NCS_SEL_OBJ_SELECT(cb->sel_obj, &set, 0, 0, &timeout);

	/* reset the fd validity flag  */
	m_NCS_LOCK(&cb->lock, NCS_LOCK_WRITE);
	m_AVA_FLAG_RESET(cb, AVA_FLAG_FD_VALID);
	m_NCS_UNLOCK(&cb->lock, NCS_LOCK_WRITE);

	/* This sel obj is no more used */
	m_NCS_SEL_OBJ_DESTROY(cb->sel_obj);

	/* everything went off well.. store the cb hdl in the global variable */
	gl_ava_hdl = cb->cb_hdl;

	return rc;

 error:
	if (cb) {
		/* remove the association with hdl-mngr */
		if (cb->cb_hdl)
			ncshm_destroy_hdl(NCS_SERVICE_ID_AVA, cb->cb_hdl);

		/* delete the hdl db */
		ava_hdl_del(cb);

		/* Flush the edu hdl */
		m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);

		/* destroy the lock */
		m_NCS_LOCK_DESTROY(&cb->lock);

		/* free the control block */
		m_MMGR_FREE_AVA_CB(cb);
	}

	/* unregister with DTSv */
#if (NCS_AVA_LOG == 1)
	ava_log_unreg();
#endif

	return rc;
}

/****************************************************************************
  Name          : ava_destroy
 
  Description   : This routine destroys the AvA control block.
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void ava_destroy(NCS_LIB_DESTROY *destroy_info)
{
	AVA_CB *cb = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* retrieve AvA CB */
	cb = (AVA_CB *)ncshm_take_hdl(NCS_SERVICE_ID_AVA, gl_ava_hdl);
	if (!cb) {
		m_AVA_LOG_CB(AVSV_LOG_CB_RETRIEVE, AVSV_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
		goto done;
	}

	/* delete the hdl db */
	ava_hdl_del(cb);
	m_AVA_LOG_HDL_DB(AVA_LOG_HDL_DB_DESTROY, AVA_LOG_HDL_DB_SUCCESS, 0, NCSFL_SEV_INFO);

	/* unregister with MDS */
	rc = ava_mds_unreg(cb);
	if (NCSCC_RC_SUCCESS != rc)
		m_AVA_LOG_MDS(AVSV_LOG_MDS_UNREG, AVSV_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);
	else
		m_AVA_LOG_MDS(AVSV_LOG_MDS_UNREG, AVSV_LOG_MDS_SUCCESS, NCSFL_SEV_INFO);

	/* EDU cleanup */
	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
	m_AVA_LOG_EDU(AVSV_LOG_EDU_FINALIZE, AVSV_LOG_EDU_SUCCESS, NCSFL_SEV_INFO);

	/* destroy the lock */
	m_NCS_LOCK_DESTROY(&cb->lock);
	m_AVA_LOG_LOCK(AVSV_LOG_LOCK_FINALIZE, AVSV_LOG_LOCK_SUCCESS, NCSFL_SEV_INFO);

	/* return AvA CB */
	ncshm_give_hdl(gl_ava_hdl);

	/* remove the association with hdl-mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_AVA, cb->cb_hdl);
	m_AVA_LOG_CB(AVSV_LOG_CB_HDL_ASS_REMOVE, AVSV_LOG_CB_SUCCESS, NCSFL_SEV_INFO);

	/* free the control block */
	m_MMGR_FREE_AVA_CB(cb);

	/* reset the global cb handle */
	gl_ava_hdl = 0;

 done:
	/* unregister with DTSv */
#if (NCS_AVA_LOG == 1)
	ava_log_unreg();
#endif

	return;
}

/****************************************************************************
  Name          :  ncs_ava_startup

  Description   :  This routine creates a AVSv agent infrastructure to interface
                   with AVSv service. Once the infrastructure is created from
                   then on use_count is incremented for every startup request.

  Arguments     :  - NIL-

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
unsigned int ncs_ava_startup(void)
{
	NCS_LIB_REQ_INFO lib_create;

	m_AVA_AGENT_LOCK;

	if (ava_use_count > 0) {
		/* Already created, so just increment the use_count */
		ava_use_count++;
		m_AVA_AGENT_UNLOCK;
		return NCSCC_RC_SUCCESS;
	}

   /*** Init AVA ***/
	memset(&lib_create, 0, sizeof(lib_create));
	lib_create.i_op = NCS_LIB_REQ_CREATE;
	if (ava_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		m_AVA_AGENT_UNLOCK;
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	} else {
		m_NCS_DBG_PRINTF("\nAVSV:AVA:ON");
		ava_use_count = 1;
	}

	m_AVA_AGENT_UNLOCK;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncs_ava_shutdown 

  Description   :  This routine destroys the AVSv agent infrastructure created 
                   to interface AVSv service. If the registered users are > 1, 
                   it just decrements the use_count.   

  Arguments     :  - NIL -

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
unsigned int ncs_ava_shutdown(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	m_AVA_AGENT_LOCK;

	if (ava_use_count > 1) {
		/* Still users exists, so just decrement the use_count */
		ava_use_count--;
	} else if (ava_use_count == 1) {
		NCS_LIB_REQ_INFO lib_destroy;

		memset(&lib_destroy, 0, sizeof(lib_destroy));
		lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
		rc = ava_lib_req(&lib_destroy);

		ava_use_count = 0;
	}

	m_AVA_AGENT_UNLOCK;

	return rc;
}
