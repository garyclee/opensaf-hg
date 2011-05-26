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
  
  This file contains the CPSv's service part CPA's Init/Destory routines.
    
******************************************************************************/

#include "cpa.h"

/*****************************************************************************
 global data used by CPA
 *****************************************************************************/
uint32_t gl_cpa_hdl = 0;
static uint32_t cpa_use_count = 0;

/* CPA Agent creation specific LOCK */
static uint32_t cpa_agent_lock_create = 0;
NCS_LOCK cpa_agent_lock;

#define m_CPA_AGENT_LOCK                        \
   if (!cpa_agent_lock_create++)                \
   {                                            \
      m_NCS_LOCK_INIT(&cpa_agent_lock);         \
   }                                            \
   cpa_agent_lock_create = 1;                   \
   m_NCS_LOCK(&cpa_agent_lock, NCS_LOCK_WRITE);

#define m_CPA_AGENT_UNLOCK m_NCS_UNLOCK(&cpa_agent_lock, NCS_LOCK_WRITE)

static uint32_t cpa_create(NCS_LIB_CREATE *create_info);
static uint32_t cpa_destroy(NCS_LIB_DESTROY *destroy_info);
static void cpa_sync_with_cpnd(CPA_CB *cb);

/****************************************************************************
  Name          : cpa_lib_req
 
  Description   : This routine is exported to the other NCS entities & is used
                  to create & destroy the CPA library.
 
  Arguments     : req_info - ptr to the request info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uint32_t cpa_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t rc = NCSCC_RC_SUCCESS;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		rc = cpa_create(&req_info->info.create);
		break;

	case NCS_LIB_REQ_DESTROY:
		rc = cpa_destroy(&req_info->info.destroy);
		break;

	default:
		m_LOG_CPA_CCL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "lib_req:unknown", __FILE__,
			      __LINE__);
		rc = NCSCC_RC_FAILURE;
		break;
	}

	return rc;
}

/********************************************************************
 Name    :  cpa_sync_with_cpnd

 Description : This is for CPA to sync with CPND when it gets MDS callback
 
**********************************************************************/
void cpa_sync_with_cpnd(CPA_CB *cb)
{
	NCS_SEL_OBJ_SET set;
	uint32_t timeout = 3000;

	m_NCS_LOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);

	if (cb->is_cpnd_up) {
		m_NCS_UNLOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);
		return;
	}

	cb->cpnd_sync_awaited = true;
	m_NCS_SEL_OBJ_CREATE(&cb->cpnd_sync_sel);
	m_NCS_UNLOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);

	/* Await indication from MDS saying CPND is up */
	m_NCS_SEL_OBJ_ZERO(&set);
	m_NCS_SEL_OBJ_SET(cb->cpnd_sync_sel, &set);
	m_NCS_SEL_OBJ_SELECT(cb->cpnd_sync_sel, &set, 0, 0, &timeout);

	/* Destroy the sync - object */
	m_NCS_LOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);

	cb->cpnd_sync_awaited = false;
	m_NCS_SEL_OBJ_DESTROY(cb->cpnd_sync_sel);

	m_NCS_UNLOCK(&cb->cpnd_sync_lock, NCS_LOCK_WRITE);
	return;
}

/****************************************************************************
  Name          : cpa_create
 
  Description   : This routine creates & initializes the CPA control block.
 
  Arguments     : create_info - ptr to the create info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static uint32_t cpa_create(NCS_LIB_CREATE *create_info)
{
	CPA_CB *cb = NULL;
	uint32_t rc;

	/* validate create info */
	if (create_info == NULL)
		return NCSCC_RC_FAILURE;

	/* Register with Logging subsystem */
	cpa_flx_log_reg();

	/* Malloc the CB for CPA */
	cb = m_MMGR_ALLOC_CPA_CB;
	if (cb == NULL) {
		/* m_LOG_CPA_MEMFAIL(CPA_CB_ALLOC_FAILED); */
		m_LOG_CPA_CCL(CPA_MEM_ALLOC_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "CPA_CB", __FILE__, __LINE__);
		goto cb_alloc_fail;
	}
	memset(cb, 0, sizeof(CPA_CB));

	/* assign the CPA pool-id (used by hdl-mngr) */
	cb->pool_id = NCS_HM_POOL_ID_COMMON;

	/* create the association with hdl-mngr */
	if (!(cb->agent_handle_id = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_CPA, (NCSCONTEXT)cb))) {
		m_LOG_CPA_CCL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "ncshm_create", __FILE__, __LINE__);
		goto hm_create_fail;
	}
	/*store the hdl in the global variable */
	gl_cpa_hdl = cb->agent_handle_id;

	/* get the process id */
	cb->process_id = getpid();

	/* initialize the cpa cb lock */
	if (m_NCS_LOCK_INIT(&cb->cb_lock) != NCSCC_RC_SUCCESS) {
		m_LOG_CPA_CCL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "LOCK_INIT", __FILE__, __LINE__);
		goto lock_init_fail;
	}

	/* Initalize the CPA Trees & Linked lists */
	rc = cpa_db_init(cb);
	if (rc != NCSCC_RC_SUCCESS) {
		/* No need to log here, already logged in cpa_db_init */
		goto db_init_fail;
	}

	/* register with MDS */
	if (cpa_mds_register(cb) != NCSCC_RC_SUCCESS) {
		/* No need to log here, already logged in cpa_mds_register  */
		goto mds_reg_fail;
	}

	cpa_sync_with_cpnd(cb);

	/* EDU initialisation */
	if (m_NCS_EDU_HDL_INIT(&cb->edu_hdl) != NCSCC_RC_SUCCESS) {
		m_LOG_CPA_CCL(CPA_API_FAILED, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_ERROR, "EDU:INIT", __FILE__, __LINE__);
		goto edu_init_fail;
	}

	m_LOG_CPA_CCL(CPA_PROC_SUCCESS, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_NOTICE, "Lib Init", __FILE__, __LINE__);
	return NCSCC_RC_SUCCESS;

/* error8:
   m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl); */

 edu_init_fail:
	/* MDS unregister. */
	cpa_mds_unregister(cb);

 mds_reg_fail:
	cb->is_cpnd_joined_clm = false;
	cpa_db_destroy(cb);

 db_init_fail:
	/* destroy the lock */
	cb->is_cpnd_joined_clm = false;
	m_NCS_LOCK_DESTROY(&cb->cb_lock);

 lock_init_fail:
	/* remove the association with hdl-mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_CPA, cb->agent_handle_id);
	gl_cpa_hdl = 0;

 hm_create_fail:
	/* Free the CB */
	m_MMGR_FREE_CPA_CB(cb);

 cb_alloc_fail:

	/* Deregister with the Flex Log */
	cpa_flx_log_dereg();

	return NCSCC_RC_FAILURE;
}

/****************************************************************************
  Name          : cpa_destroy
 
  Description   : This routine destroys the CPA control block.
 
  Arguments     : destroy_info - ptr to the destroy info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
static uint32_t cpa_destroy(NCS_LIB_DESTROY *destroy_info)
{
	CPA_CB *cb = NULL;

	/* validate the CB */
	m_CPA_RETRIEVE_CB(cb);
	if (!cb)
		return NCSCC_RC_FAILURE;

	/* MDS unregister. */
	cpa_mds_unregister(cb);

	m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);

	/* Destroy the CPA database */
	cpa_db_destroy(cb);

	/* destroy the lock */
	m_NCS_LOCK_DESTROY(&cb->cb_lock);

	/* return CPA CB Handle */
	ncshm_give_hdl(cb->agent_handle_id);

	/* remove the association with hdl-mngr */
	ncshm_destroy_hdl(NCS_SERVICE_ID_CPA, cb->agent_handle_id);

	m_LOG_CPA_CCL(CPA_PROC_SUCCESS, NCSFL_LC_CKPT_MGMT, NCSFL_SEV_NOTICE, "Lib Destroy", __FILE__, __LINE__);

	/*  Memory leaks found in cpa_init.c */
	m_MMGR_FREE_CPA_CB(cb);

	/* de register with the flex log */
	cpa_flx_log_dereg();

	/* reset the global cb handle */
	gl_cpa_hdl = 0;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncs_cpa_startup

  Description   :  This routine creates a CPSv agent infrastructure to interface
                   with CPSv service. Once the infrastructure is created from 
                   then on use_count is incremented for every startup request.

  Arguments     :  - NIL-

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
unsigned int ncs_cpa_startup(void)
{
	NCS_LIB_REQ_INFO lib_create;
        char *value;
	m_CPA_AGENT_LOCK;

	if (cpa_use_count > 0) {
		/* Already created, so just increment the use_count */
		cpa_use_count++;
		m_CPA_AGENT_UNLOCK;
		return NCSCC_RC_SUCCESS;
	}

        /*** Init CPA ***/
	memset(&lib_create, 0, sizeof(lib_create));
	lib_create.i_op = NCS_LIB_REQ_CREATE;
	if (cpa_lib_req(&lib_create) != NCSCC_RC_SUCCESS) {
		m_CPA_AGENT_UNLOCK;
		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	} else {
		m_NCS_DBG_PRINTF("\nCPSV:CPA:ON");
		cpa_use_count = 1;
	}

	m_CPA_AGENT_UNLOCK;

       /* Initialize trace system first of all so we can see what is going. */
       if ((value = getenv("CPA_TRACE_PATHNAME")) != NULL) {
              logtrace_init("cpa", value, CATEGORY_ALL);

       }

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  ncs_cpa_shutdown 

  Description   :  This routine destroys the CPSv agent infrastructure created 
                   to interface CPSv service. If the registered users are > 1, 
                   it just decrements the use_count.   

  Arguments     :  - NIL -

  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
unsigned int ncs_cpa_shutdown(void)
{
	uint32_t rc = NCSCC_RC_SUCCESS;

	m_CPA_AGENT_LOCK;

	if (cpa_use_count > 1) {
		/* Still users extis, so just decrement the use_count */
		cpa_use_count--;
	} else if (cpa_use_count == 1) {
		NCS_LIB_REQ_INFO lib_destroy;

		memset(&lib_destroy, 0, sizeof(lib_destroy));
		lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
		rc = cpa_lib_req(&lib_destroy);

		cpa_use_count = 0;
	}

	m_CPA_AGENT_UNLOCK;

	return rc;
}
