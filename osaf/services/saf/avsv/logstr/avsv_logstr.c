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

  This file contains canned strings that are used for logging AvSv common 
  information.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#if (NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "avsv_log.h"
#include "avnd_logstr.h"
#include "avsv_logstr.h"

/******************************************************************************
                          Canned string for SE-API
 ******************************************************************************/
const NCSFL_STR avsv_seapi_set[] = {
	{AVSV_LOG_SEAPI_CREATE, "SE-API Create"},
	{AVSV_LOG_SEAPI_DESTROY, "SE-API Destroy"},
	{AVSV_LOG_SEAPI_SUCCESS, "Success"},
	{AVSV_LOG_SEAPI_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                          Canned string for MDS
 ******************************************************************************/
const NCSFL_STR avsv_mds_set[] = {
	{AVSV_LOG_MDS_REG, "MDS Register"},
	{AVSV_LOG_MDS_INSTALL, "MDS Install"},
	{AVSV_LOG_MDS_SUBSCRIBE, "MDS Subscribe"},
	{AVSV_LOG_MDS_UNREG, "MDS Un-Register"},
	{AVSV_LOG_MDS_VDEST_CRT, "MDS create VDEST address"},
	{AVSV_LOG_MDS_VDEST_ROL, "MDS set VDEST role"},
	{AVSV_LOG_MDS_SEND, "MDS Send"},
	{AVSV_LOG_MDS_PRM_GET, "MDS Param Get"},
	{AVSV_LOG_MDS_RCV_CBK, "MDS Rcv Cbk"},
	{AVSV_LOG_MDS_CPY_CBK, "MDS Copy Cbk"},
	{AVSV_LOG_MDS_SVEVT_CBK, "MDS Svc Evt Cbk"},
	{AVSV_LOG_MDS_ENC_CBK, "MDS Enc Cbk"},
	{AVSV_LOG_MDS_FLAT_ENC_CBK, "MDS Flat Enc Cbk"},
	{AVSV_LOG_MDS_DEC_CBK, "MDS Dec Cbk"},
	{AVSV_LOG_MDS_FLAT_DEC_CBK, "MDS Flat Dec Cbk"},
	{AVSV_LOG_MDS_SUCCESS, "Success"},
	{AVSV_LOG_MDS_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                          Canned string for EDU
 ******************************************************************************/
const NCSFL_STR avsv_edu_set[] = {
	{AVSV_LOG_EDU_INIT, "EDU Init"},
	{AVSV_LOG_EDU_FINALIZE, "EDU Finalize"},
	{AVSV_LOG_EDU_SUCCESS, "Success"},
	{AVSV_LOG_EDU_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                          Canned string for LOCK
 ******************************************************************************/
const NCSFL_STR avsv_lock_set[] = {
	{AVSV_LOG_LOCK_INIT, "Lck Init"},
	{AVSV_LOG_LOCK_FINALIZE, "Lck Finalize"},
	{AVSV_LOG_LOCK_TAKE, "Lck Take"},
	{AVSV_LOG_LOCK_GIVE, "Lck Give"},
	{AVSV_LOG_LOCK_SUCCESS, "Success"},
	{AVSV_LOG_LOCK_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                       Canned string for control block
 ******************************************************************************/
const NCSFL_STR avsv_cb_set[] = {
	{AVSV_LOG_CB_CREATE, "CB Create"},
	{AVSV_LOG_CB_DESTROY, "CB Destroy"},
	{AVSV_LOG_CB_HDL_ASS_CREATE, "CB Hdl Mngr Association Create"},
	{AVSV_LOG_CB_HDL_ASS_REMOVE, "CB Hdl Mngr Association Rem"},
	{AVSV_LOG_CB_RETRIEVE, "CB Retrieve"},
	{AVSV_LOG_CB_RETURN, "CB Return"},
	{AVSV_LOG_CB_SUCCESS, "Success"},
	{AVSV_LOG_CB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                       Canned string for AMF Callback
 ******************************************************************************/
const NCSFL_STR avsv_cbk_set[] = {
	{AVSV_LOG_AMF_HC, "saAmfHealthcheckCallback()"},
	{AVSV_LOG_AMF_COMP_TERM, "saAmfComponentTerminateCallback()"},
	{AVSV_LOG_AMF_CSI_SET, "saAmfCSISetCallback()"},
	{AVSV_LOG_AMF_CSI_REM, "saAmfCSIRemoveCallback()"},
	{AVSV_LOG_AMF_PG_TRACK, "saAmfProtectionGroupTrackCallback()"},
	{AVSV_LOG_AMF_PXIED_COMP_INST, "saAmfProxiedComponentInstantiateCallback()"},
	{AVSV_LOG_AMF_PXIED_COMP_CLEAN, "saAmfProxiedComponentCleanupCallback()"},
	{0, 0}
};

/******************************************************************************
                       Canned string for Mailbox
 ******************************************************************************/
const NCSFL_STR avsv_mbx_set[] = {
	{AVSV_LOG_MBX_CREATE, "Mbx Create"},
	{AVSV_LOG_MBX_ATTACH, "Mbx Attach"},
	{AVSV_LOG_MBX_DESTROY, "Mbx Destroy"},
	{AVSV_LOG_MBX_DETACH, "Mbx Detach"},
	{AVSV_LOG_MBX_SEND, "Mbx Send"},
	{AVSV_LOG_MBX_SUCCESS, "Success"},
	{AVSV_LOG_MBX_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                       Canned string for Task
 ******************************************************************************/
const NCSFL_STR avsv_task_set[] = {
	{AVSV_LOG_TASK_CREATE, "Task Create"},
	{AVSV_LOG_TASK_START, "Task Start"},
	{AVSV_LOG_TASK_RELEASE, "Task Release"},
	{AVSV_LOG_TASK_SUCCESS, "Success"},
	{AVSV_LOG_TASK_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                       Canned string for Patricia Tree
 ******************************************************************************/
const NCSFL_STR avsv_pat_set[] = {
	{AVSV_LOG_PAT_INIT, "Patricia Init"},
	{AVSV_LOG_PAT_ADD, "Patricia Add"},
	{AVSV_LOG_PAT_DEL, "Patricia Del"},
	{AVSV_LOG_PAT_SUCCESS, "Success"},
	{AVSV_LOG_PAT_FAILURE, "Failure"},
	{0, 0}
};

/****************************************************************************
  Name          : avsv_log_str_lib_req

  Description   : Library entry point for AvSv log string library.

  Arguments     : req_info  - This is the pointer to the input information.

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..

  Notes         : None.
 *****************************************************************************/
uns32 avsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = avnd_str_reg();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = avnd_str_unreg();
		break;

	default:
		break;
	}

	return (res);
}

#endif   /* (NCS_DTS == 1) */
