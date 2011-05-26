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
  FILE NAME: GLND_API.C

  DESCRIPTION: API's used to init and create PWE of GLND.

  FUNCTIONS INCLUDED in this module:
  glnd_se_lib_create ........ library used to init GLND.
  glnd_amf_init ........... GLND AMF init.
  glnd_se_lib_destroy...... library used to destroy GLND.
  ncsglnd_lib_req ......... SE API to init or create PWE.
  glnd_main_process ....... main process which is given as thread input.
  glnd_process_mbx ........ Process Mail box.  

******************************************************************************/

#include <logtrace.h>

#include "glnd.h"

void glnd_main_process(SYSF_MBX *mbx);

/****************************************************************************
 * Name          : glnd_se_lib_init
 *
 * Description   : This is the function which initalize the GLND libarary.
 *
 * Arguments     : pool_id - This is the pool ID.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 glnd_se_lib_create(uns8 pool_id)
{

	GLND_CB *glnd_cb;

	/* create the CB */
	glnd_cb = glnd_cb_create(pool_id);

	if (!glnd_cb) {
		m_LOG_GLND_HEADLINE(GLND_CB_CREATE_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}


	return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : glnd_se_lib_destroy
 *
 * Description   : This is the function which destroy the GLND libarary.
 *
 * Arguments     : 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 glnd_se_lib_destroy()
{
	GLND_CB *glnd_cb;

	/* take the handle */
	glnd_cb = (GLND_CB *)m_GLND_TAKE_GLND_CB;
	if (!glnd_cb) {
		m_LOG_GLND_HEADLINE(GLND_CB_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	if (glnd_cb_destroy(glnd_cb) != NCSCC_RC_SUCCESS) {
		m_LOG_GLND_HEADLINE(GLND_CB_DESTROY_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}


	return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : glnd_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy or 
 *                 Create/destroy PWE's. This will be called by SBOM.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 glnd_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = glnd_se_lib_create(NCS_HM_POOL_ID_COMMON);
		break;
	case NCS_LIB_REQ_DESTROY:
		res = glnd_se_lib_destroy();
		break;
	default:
		break;
	}
	return (res);
}

/****************************************************************************
 * Name          : glnd_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 GLND.
 *
 * Arguments     : mbx  - This is the mail box pointer on which IfD/IfND is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glnd_process_mbx(GLND_CB *cb, SYSF_MBX *mbx)
{
	GLSV_GLND_EVT *evt = NULL;

	while ((evt = (GLSV_GLND_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt))) {
		if ((evt->type >= GLSV_GLND_EVT_BASE) && (evt->type < GLSV_GLND_EVT_MAX)) {
			/* process mail box */
			glnd_process_evt((NCSCONTEXT)cb, evt);
		} else {
			m_LOG_GLND_EVT(GLND_EVT_UNKNOWN, __FILE__, __LINE__, evt->type, 0, 0, 0, 0);
			m_MMGR_FREE_GLND_EVT(evt);
		}
	}
	TRACE("Exiting the GLND Process MBX Evt");
	return;
}

/****************************************************************************
 * Name          : glnd_main_process
 *
 * Description   : This is the function which is given as a input to the 
 *                 GLND task.
 *
 * Arguments     : mbx  - This is the mail box pointer on which GLND is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void glnd_main_process(SYSF_MBX *mbx)
{
	NCS_SEL_OBJ mbx_fd = m_NCS_IPC_GET_SEL_OBJ(mbx);
	GLND_CB *glnd_cb = NULL;

	NCS_SEL_OBJ_SET all_sel_obj;
	SaAmfHandleT amf_hdl;

	SaSelectionObjectT amf_sel_obj;
	SaAisErrorT amf_error;

	NCS_SEL_OBJ amf_ncs_sel_obj, highest_sel_obj;

	/* take the handle */
	glnd_cb = (GLND_CB *)m_GLND_TAKE_GLND_CB;
	if (!glnd_cb) {
		m_LOG_GLND_HEADLINE(GLND_CB_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return;
	}

	amf_hdl = glnd_cb->amf_hdl;

	/*giveup the handle */
	m_GLND_GIVEUP_GLND_CB;

	m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
	m_NCS_SEL_OBJ_SET(mbx_fd, &all_sel_obj);

	amf_error = saAmfSelectionObjectGet(amf_hdl, &amf_sel_obj);

	if (amf_error != SA_AIS_OK) {
		m_LOG_GLND_HEADLINE(GLND_AMF_GET_SEL_OBJ_FAILURE, NCSFL_SEV_ERROR, __FILE__, __LINE__);
		return;
	}
	m_SET_FD_IN_SEL_OBJ((uns32)amf_sel_obj, amf_ncs_sel_obj);
	m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);

	highest_sel_obj = m_GET_HIGHER_SEL_OBJ(amf_ncs_sel_obj, mbx_fd);

	while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj, &all_sel_obj, 0, 0, 0) != -1) {
		/* process all the AMF messages */
		if (m_NCS_SEL_OBJ_ISSET(amf_ncs_sel_obj, &all_sel_obj)) {
			/* dispatch all the AMF pending function */
			amf_error = saAmfDispatch(amf_hdl, SA_DISPATCH_ALL);
			if (amf_error != SA_AIS_OK) {
				m_LOG_GLND_HEADLINE(GLND_AMF_DISPATCH_FAILURE, NCSFL_SEV_ERROR, __FILE__, __LINE__);
			}
		}
		/* process the GLND Mail box */
		if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &all_sel_obj)) {
			glnd_cb = (GLND_CB *)m_GLND_TAKE_GLND_CB;
			if (glnd_cb) {
				/* now got the IPC mail box event */
				glnd_process_mbx(glnd_cb, mbx);
				m_GLND_GIVEUP_GLND_CB;	/* giveup the handle */
			} else
				break;

		}

		/* do the fd set for the select obj */
		m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
		m_NCS_SEL_OBJ_SET(mbx_fd, &all_sel_obj);

	}
	TRACE("DANGER: Exiting the Select loop of GLND");
	return;
}
