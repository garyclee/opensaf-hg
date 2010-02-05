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

  MODULE NAME: rde_log_str.c

..............................................................................

  DESCRIPTION:

******************************************************************************
*/

#include "rde.h"
#include "rde_log.h"
#include "rde_log_str.h"
#include "dta_papi.h"
#include "dts_papi.h"

/**************************************************************************\
 *                                                                          *
 *                          Headlines                                       *
 *                                                                          *
\**************************************************************************/

const NCSFL_STR rde_log_str_headlines[] = {
	{RDE_HDLN_RDE_BANNER, "******************************************"},
	{RDE_HDLN_RDE_BANNER_SPACE, "*                                        *"},
	{RDE_HDLN_RDE_STARTING, "* Starting Role Distribution Entity(RDE) *"},
	{RDE_HDLN_RDE_STOPPING, "* Stopping Role Distribution Entity(RDE) *"},
	{RDE_HDLN_TASK_STARTED_CLI, "Task Started : Interface"},
	{RDE_HDLN_TASK_STOPPED_CLI, "Task Stopped : Interface"},
	{RDE_HDLN_SIG_INT, "Received Signal         : SIGINT"},
	{RDE_HDLN_SIG_TERM, "Received Signal         : SIGTERM"},
	{RDE_HDLN_MAX, "Dummy placeholder"},

	{0, 0}
};

/**************************************************************************\
 *                                                                          *
 *                          Reportable Conditions                           *
 *                                                                          *
\**************************************************************************/

const NCSFL_STR rde_log_str_conditions[] = {
	{RDE_COND_UNSUPPORTED_RDA_CMD, "Unsupported RDA command        :"},
	{RDE_COND_INVALID_RDA_CMD, "Invalid RDA command"},
	{RDE_LOG_COND_NO_PID_FILE, "No PID  file specified for -p option"},
	{RDE_LOG_COND_NO_SHELF_NUMBER, "No shelfnumber specified for -f option"},
	{RDE_LOG_COND_NO_SLOT_NUMBER, "No slotnumber specified for -s option"},
	{RDE_LOG_COND_NO_SITE_NUMBER, "No sitenumber specified for -t option"},
	{RDE_LOG_COND_PID_FILE_OPEN, "Failed to open PID file"},
	{RDE_COND_RDE_VERSION, "Build Version                  :"},
	{RDE_COND_RDE_BUILD_DATE, "Build Date                     :"},
	{RDE_COND_TASK_CREATE_FAILED, "Task creation failed           :"},
	{RDE_COND_SEM_CREATE_FAILED, "Semaphore creation failed      :"},
	{RDE_COND_SEM_TAKE_FAILED, "Semaphore take failed          :"},
	{RDE_COND_SEM_GIVE_FAILED, "Semaphore give failed          :"},
	{RDE_COND_LOCK_CREATE_FAILED, "Lock created failed            :"},
	{RDE_COND_LOCK_CREATE_SUCCESS, "Lock created success           :"},
	{RDE_COND_LOCK_TAKE_FAILED, "Lock take failed               :"},
	{RDE_COND_LOCK_TAKE_SUCCESS, "Lock take success              :"},
	{RDE_COND_LOCK_GIVE_FAILED, "Lock give failed               :"},
	{RDE_COND_LOCK_GIVE_SUCCESS, "Lock give success              :"},
	{RDE_COND_BAD_COMMAND, "Invalid Generic Resp           :"},
	{RDE_COND_RDA_CMD, "Received RDA command           :"},
	{RDE_COND_RDA_RESPONSE, "Sent RDA command response      :"},
	{RDE_COND_SELECT_ERROR, "Select error                   :"},
	{RDE_COND_SOCK_CREATE_FAIL, "Socket create failed           :"},
	{RDE_COND_SOCK_REMOVE_FAIL, "Socket remove failed           :"},
	{RDE_COND_SOCK_BIND_FAIL, "Socket bind   failed           :"},
	{RDE_COND_SOCK_ACCEPT_FAIL, "Socket accpet failed           :"},
	{RDE_COND_SOCK_CLOSE_FAIL, "Socket close  failed           :"},
	{RDE_COND_SOCK_SEND_FAIL, "Sendto        failed           :"},
	{RDE_COND_SOCK_RECV_FAIL, "Recvfrom      failed           :"},
	{RDE_COND_SOCK_NULL_NAME, "Null socket name        "},
	{RDE_COND_SOCK_NULL_ADDR, "Null socket address     "},
	{RDE_COND_RDA_SOCK_CREATED, "RDA socket created"},
	{RDE_COND_MEM_DUMP, "Memory Dump                    :"},
	{RDE_COND_NO_MEM_DEBUG, "No Memory Dump (MMGR_DEBUG not defined)"},
	{RDE_COND_MEM_IGNORE, "Ignoring all currently allocated memory"},
	{RDE_COND_MEM_REMEMBER, "Cancelling ignoring of allocated memory"},
	{RDE_COND_HA_ROLE, "Current HA role or state: "},
	{RDE_COND_ACTIVE_PLANE, "Current active plane: "},

	/* HA for RDE */
	{RDE_COND_SETENV_COMP_NAME_FAIL, "Set env variable for componet name failed:"},
	{RDE_COND_SETENV_HC_KEYE_FAIL, "Set env variable for healthcheck key failed:"},
	{RDE_COND_AMF_DISPATCH_FAIL, "AMF dispatch failed: "},
	{RDE_COND_CSI_SET_INVOKED, "CSI set callback invoked: "},
	{RDE_COND_HEALTH_CHK_INVOKED, "Health check callback invoked: "},
	{RDE_COND_CSI_REMOVE_INVOKED, "CSI remove callback invoked: "},
	{RDE_COND_COMP_TERMINATE_INVOKED, "Component terminate callback invoked: "},
	{RDE_COND_AMF_INIT_FAILED, "AMF initialize failed: "},
	{RDE_COND_AMF_INIT_DONE, "AMF initialize done: "},
	{RDE_COND_AMF_HEALTH_CHK_START_FAIL, "AMF health check start failed: "},
	{RDE_COND_AMF_HEALTH_CHK_START_DONE, "AMF health check start done: "},
	{RDE_COND_AMF_GET_OBJ_FAILED, "AMF selection object get failed: "},
	{RDE_COND_AMF_GET_NAME_FAILED, "AMF component name get failed: "},
	{RDE_COND_AMF_REG_FAILED, "AMF registration failed: "},
	{RDE_COND_AMF_REG_DONE, "AMF registation done: "},
	{RDE_COND_AMF_UNREG_FAILED, "AMF unregistration failed: "},
	{RDE_COND_AMF_UNREG_DONE, "AMF unregistration done: "},
	{RDE_COND_AMF_DESTROY_DONE, "AMF finalize and lib destroy done: "},
	{RDE_COND_PIPE_OPEN_FAILED, "Named pipe open failed: "},
	{RDE_COND_PIPE_OPEN_DONE, "Named pipe open done: "},
	{RDE_COND_PIPE_READ_FAILED, "Named pipe read failed: "},
	{RDE_COND_PIPE_CREATE_FAILED, "Named pipe creation failed: "},
	{RDE_COND_NCS_LIB_AVA_CREATE_FAILED, "NCS AVA lib create failed: "},
	{RDE_COND_NCS_LIB_INIT_DONE, "NCS lib (AVA) init done: "},
	{RDE_COND_CLUSRTER_ID_INFO, "Cluster ID information:"},
	{RDE_COND_NODE_ID_INFO, "Node ID information:"},
	{RDE_COND_PCON_ID_INFO, "PCON ID information:"},
	{RDE_COND_HEALTH_CHK_RSP_SENT, "Health check response sent: "},
	{RDE_COND_CSI_SET_RSP_SENT, "CSI assignment response sent; "},
	{RDE_COND_GET_NODE_ID_FAILED, "Core agents startup: get node id fail"},
	{RDE_COND_CORE_AGENTS_START_FAILED, "Core agents startup failed"},
	{RDE_COND_PIPE_COMP_NAME, "Component name read from pipe:"},
	{RDE_COND_PIPE_HC_KEY, "Healthcheck key read from pipe:"},

	/* RDE TO RDE */
	{RDE_RDE_SOCK_CREATE_FAIL, "RDE_RDE_SOCK_CREATE_FAIL: "},
	{RDE_RDE_SOCK_BIND_FAIL, "RDE_RDE_SOCK_BIND_FAIL: "},
	{RDE_RDE_SOCK_ACCEPT_FAIL, "RDE_RDE_SOCK_ACCEPT_FAIL: "},
	{RDE_RDE_SOCK_CLOSE_FAIL, "RDE_RDE_SOCK_CLOSE_FAIL: "},
	{RDE_RDE_SOCK_WRITE_FAIL, "RDE_RDE_SOCK_WRITE_FAIL: "},
	{RDE_RDE_SOCK_LISTEN_FAIL, "RDE_RDE_SOCK_LISTEN_FAIL: "},
	{RDE_RDE_SOCK_READ_FAIL, "RDE_RDE_SOCK_READ_FAIL: "},
	{RDE_RDE_SOCK_CONN_FAIL, "RDE_RDE_SOCK_CONN_FAIL: "},
	{RDE_RDE_SOCK_REMOVE_FAIL, "RDE_RDE_SOCK_REMOVE_FAIL: "},
	{RDE_RDE_CLIENT_SOCK_CREATE_FAIL, "RDE_RDE_CLIENT_SOCK_CREATE_FAIL: "},
	{RDE_RDE_SELECT_ERROR, "RDE_RDE_SELECT_ERROR: "},
	{RDE_RDE_SELECT_TIMED_OUT, "RDE_RDE_SELECT_TIMED_OUT: "},
	{RDE_RDE_RECV_INVALID_MESSAGE, "RDE_RDE_RECV_INVALID_MESSAGE: "},
	{RDE_RDE_UNABLE_TO_OPEN_CONFIG_FILE, "RDE_RDE_UNABLE_TO_OPEN_CONFIG_FILE: "},
	{RDE_RDE_FAIL_TO_CLOSE_CONFIG_FILE, "RDE_RDE_FAIL_TO_CLOSE_CONFIG_FILE: "},
	{RDE_RDE_ROLE_IS_NOT_VALID, "RDE_RDE_ROLE_IS_NOT_VALID_NUMBER :"},
	{RDE_RDE_SET_SOCKOPT_ERROR, "RDE_RDE_SET_SOCKOPT_ERROR :"},
	{RDE_RDE_INFO, "RDE_RDE Information :"},
	{RDE_RDE_ERROR, "RDE_RDE Error :"},

	{RDE_LOG_COND_MAX, "Dummy placeholder"},

	{0, 0}
};

/**************************************************************************\
 *                                                                          *
 *                          Memory Allocation Failures                      *
 *                                                                          *
\**************************************************************************/

const NCSFL_STR rde_log_str_mem_fail[] = {
	{RDE_MEM_FAIL_IPMC_SERIAL, "IPMC Serial Interface"},
	{RDE_MEM_FAIL_IPMC_CMD, "IPMC Command"},
	{RDE_MEM_FAIL_CLI_SOCK, "CLI  Socket Interface"},
	{RDE_MEM_FAIL_CLI_CMD, "CLI  Command"},
	{RDE_MEM_FAIL_CLI_ADDR, "CLI  Address"},

	{0, 0}
};

/**************************************************************************\
 *                                                                          *
 *                                                                          *
 *         Construct a structure that collects the set of constant strings  *
 *         associated with each kind of log message                         *
 *                                                                          *
 *                                                                          *
   \**************************************************************************/

NCSFL_SET rde_log_strings_set[] = {
	{RDE_FC_HEADLINE, 0, (NCSFL_STR *)rde_log_str_headlines},
	{RDE_FC_CONDITION, 0, (NCSFL_STR *)rde_log_str_conditions},
	{RDE_FC_MEM_FAIL, 0, (NCSFL_STR *)rde_log_str_mem_fail},

	{0, 0, 0}
};

/**************************************************************************\
 *                                                                          *
 *                                                                          *
 *         Construct a structure that collects the set of format strings    *
 *         with different argument combinations                             *
 *                                                                          *
 *                                                                          *
 *  NCSFL_FMAT                                                              *
 *                                                                          *
 *  This structure maps a relationship between aan offset value (fmat_id)   *
 *  and an array set of canned formatted strings (fmat_str).                *
 *                                                                          *
 *  The fmat_id allows Flexlog to do a simple sanity check to make sure     *
 *  there is alignment between a numeric value expressed as a mnemonic      *
 *  and a particular set value that leads to the correct formatted string.  *
 *
 *  The fmat_type field explains the layout of the argument types that      *
 *  are needed/expected in order to properly use this formatted string.     *
 *                                                                          *
   \**************************************************************************/

NCSFL_FMAT rde_log_format_set[] = {
	{RDE_FMT_HEADLINE, NCSFL_TYPE_TI, "%s %s\n"},
	{RDE_FMT_HEADLINE_NUM, NCSFL_TYPE_TIL, "%s %s %ld\n"},
	{RDE_FMT_HEADLINE_NUM_BOX, NCSFL_TYPE_TIL, "%s %s %ld                 *\n"},
	{RDE_FMT_HEADLINE_TIME, NCSFL_TYPE_TIF, "%s %s %8.2f milliseconds\n"},
	{RDE_FMT_HEADLINE_TIME_BOX, NCSFL_TYPE_TIF, "%s %s %8.2f milliseconds  *\n"},
	{RDE_FMT_HEADLINE_STR, NCSFL_TYPE_TIC, "%s %s %s\n"},
	{RDE_FMT_MEM_FAIL, NCSFL_TYPE_TI, "%s %s\n"},

	{0, 0, 0}
};

/**************************************************************************\
 *                                                                          *
 *                                                                          *
 *         Structure used to register with Flexlog                          *
 *                                                                          *
 *                                                                          *
 * NCSFL_ASCII_SPEC                                                         *
 *                                                                          *
 * This data structure houses the set of information that a Subsystem       *
 * registers with FlexLog at startup time.                                  *
 *                                                                          *
 * It contains everything Flexlog needs to print any logging message        *
 * this particular subsystem can generate.                                  *
 *                                                                          *
   \**************************************************************************/

NCSFL_ASCII_SPEC rde_log_ascii_spec = {
	NCS_SERVICE_ID_RDE,	/* RDE subsystem                   */
	NCS_RDE_VERSION,	/* RDE revision ID                 */
	"RDE",			/* LOG File Name added dynamically */
	(NCSFL_SET *)rde_log_strings_set,	/* RDE const strings referenced by index */
	(NCSFL_FMAT *)rde_log_format_set,	/* RDE string format info          */
	0,			/* Place holder for str_set count  */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  PROCEDURE NAME:    rde_flex_log_ascii_set_reg

  DESCRIPTION:       Register predefined log strings with the DTSv.

*****************************************************************************/
static
uns32 rde_flex_log_ascii_set_reg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &rde_log_ascii_spec;

	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************

  PROCEDURE NAME:    rde_flex_log_ascii_set_dereg

  DESCRIPTION:       Deregister predefined log strings with the DTSv

*****************************************************************************/
static
uns32 rde_flex_log_ascii_set_dereg(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_RDE;
	arg.info.dereg_ascii_spec.version = NCS_RDE_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : rde_log_str_lib_req
 *
 * Description   : Library entry point for rde log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information
 *                             which SBOM gives.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 rde_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uns32 res = NCSCC_RC_FAILURE;

	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = rde_flex_log_ascii_set_reg();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = rde_flex_log_ascii_set_dereg();
		break;

	default:
		break;
	}
	return (res);
}
