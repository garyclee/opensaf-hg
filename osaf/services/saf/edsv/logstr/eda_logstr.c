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

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/
#include "eda.h"

const NCSFL_STR eda_hdln_set[] = {
	{EDA_ADA_ADEST_CREATE_FAILED, "EDA ADA ADEST CREATE FAILED"},
	{EDA_API_MSG_DISPATCH_FAILED, "EDA API MESSAGE DISPATCH FAILED"},
	{EDA_CB_HDL_CREATE_FAILED, "EDA CB HANDLE CREATE FAILED"},
	{EDA_CB_HDL_TAKE_FAILED, "EDA CB HANDLE TAKE FAILED"},
	{EDA_CHAN_HDL_REC_LOOKUP_FAILED, "EDA CHAN HDL RECORD LOOKUP FAILED"},
	{EDA_CHANNEL_HDL_CREATE_FAILED, "EDA CHANNEL HANDLE CREATE FAILED"},
	{EDA_CHANNEL_HDL_TAKE_FAILED, "EDA CHANNEL HANDLE TAKE FAILED"},
	{EDA_CLIENT_HDL_CREATE_FAILED, "EDA CLIENT HANDLE CREATE FAILED"},
	{EDA_CLIENT_HDL_TAKE_FAILED, "EDA CLIENT HANDLE TAKE FAILED"},
	{EDA_EVENT_HDL_CREATE_FAILED, "EDA EVENT HANDLE CREATE FAILED"},
	{EDA_EVENT_HDL_TAKE_FAILED, "EDA EVENT HANDLE TAKE FAILED"},
	{EDA_EVENT_PROCESSING_FAILED, "EDA EVENT PROCESSING FAILED"},
	{EDA_EVT_HDL_CREATE_FAILED, "EDA EVT HANDLE CREATE FAILED"},
	{EDA_EVT_HDL_TAKE_FAILED, "EDA EVT HANDLE TAKE FAILED"},
	{EDA_EVT_INST_HDL_CREATE_FAILED, "EDA EVT INST HANDLE CREATE FAILED"},
	{EDA_EVT_INST_HDL_TAKE_FAILED, "EDA EVT INST HANDLE TAKE FAILED"},
	{EDA_EVT_INST_REC_ADD_FAILED, "EDA EVT INST REC ADD FAILED"},
	{EDA_HDL_REC_ADD_FAILED, "EDA HDL RECORD ADD FAILED"},
	{EDA_HDL_REC_LOOKUP_FAILED, "EDA HDL RECORD LOOKUP FAILED"},
	{EDA_IPC_CREATE_FAILED, "EDA IPC CREATE FAILED"},
	{EDA_IPC_ATTACH_FAILED, "EDA IPC ATTACH FAILED"},
	{EDA_MDS_CALLBACK_PROCESS_FAILED, "EDA MDS CALLBACK PROCESS FAILED"},
	{EDA_MDS_GET_HANDLE_FAILED, "EDA MDS GET HANDLE FAILED"},
	{EDA_MDS_INIT_FAILED, "EDA MDS INIT FAILED"},
	{EDA_MDS_SUBSCRIPTION_FAILED, "EDA MDS SUBSCRIPTION FAILED"},
	{EDA_MDS_INSTALL_FAILED, "EDA MDS INSTALL FAILED"},
	{EDA_MDS_SUBSCRIBE_FAILED, "EDA MDS SUBSCRIBE FAILED"},
	{EDA_MDS_UNINSTALL_FAILED, "EDA MDS UNINSTALL FAILED"},
	{EDA_VERSION_INCOMPATIBLE, "EDA VERSION INCOMPATIBLE"},
	{EDA_MEMALLOC_FAILED, "EDA MEMORY ALLOCATION FAILED"},
	{EDA_FAILURE, "EDA PROCESSING FAILED"},
	{EDA_SUCCESS, "EDA PROCESSING SUCCESS"},
	{EDA_INITIALIZE_FAILURE, "EDA INITIALIZE API FAILURE"},
	{EDA_INITIALIZE_SUCCESS, "EDA INITIALIZE API SUCCESS"},
	{EDA_SELECTION_OBJ_GET_FAILURE, "EDA SELECTIONOBJ GET API FAILURE"},
	{EDA_SELECTION_OBJ_GET_SUCCESS, "EDA SELECTIONOBJ GET API SUCCESS"},
	{EDA_DISPATCH_FAILURE, "EDA DISPATCH API FAILURE"},
	{EDA_DISPATCH_SUCCESS, "EDA DISPATCH API SUCCESS"},
	{EDA_FINALIZE_FAILURE, "EDA FINALIZE API FAILURE"},
	{EDA_FINALIZE_SUCCESS, "EDA FINALIZE API SUCCESS"},
	{EDA_OPEN_FAILURE, "EDA OPEN API FAILURE"},
	{EDA_OPEN_SUCCESS, "EDA OPEN API SUCCESS"},
	{EDA_OPEN_ASYNC_FAILURE, "EDA OPEN ASYNC API FAILURE"},
	{EDA_OPEN_ASYNC_SUCCESS, "EDA OPEN ASYNC API SUCCESS"},
	{EDA_CLOSE_FAILURE, "EDA CLOSE API FAILURE"},
	{EDA_CLOSE_SUCCESS, "EDA CLOSE API SUCCESS"},
	{EDA_UNLINK_FAILURE, "EDA UNLINK API FAILURE"},
	{EDA_UNLINK_SUCCESS, "EDA UNLINK API SUCCESS"},
	{EDA_EVT_ALLOCATE_FAILURE, "EDA ALLOCATE API FAILURE"},
	{EDA_EVT_ALLOCATE_SUCCESS, "EDA ALLOCATE API SUCCESS"},
	{EDA_EVT_FREE_FAILURE, "EDA FREE API FAILURE"},
	{EDA_ATTRIBUTE_SET_FAILURE, "EDA ATTRIBUTE SET API FAILURE"},
	{EDA_ATTRIBUTE_SET_SUCCESS, "EDA ATTRIBUTE SET API SUCCESS"},
	{EDA_ATTRIBUTE_GET_FAILURE, "EDA ATTRIBUTE GET API FAILURE"},
	{EDA_ATTRIBUTE_GET_SUCCESS, "EDA ATTRIBUTE GET API SUCCESS"},
	{EDA_PATTERN_FREE_FAILURE, "EDA PATTERN FREE API FAILURE"},
	{EDA_PATTERN_FREE_SUCCESS, "EDA PATTERN FREE API SUCCESS"},
	{EDA_DATA_GET_FAILURE, "EDA DATA GET API FAILURE"},
	{EDA_DATA_GET_SUCCESS, "EDA DATA GET API SUCCESS"},
	{EDA_EVT_PUBLISH_FAILURE, "EDA EVT PUBLISH API FAILURE"},
	{EDA_EVT_SUBSCRIBE_FAILURE, "EDA EVT SUBSCRIBE API FAILURE"},
	{EDA_EVT_UNSUBSCRIBE_FAILURE, "EDA EVT UNSUBSCRIBE API FAILURE"},
	{EDA_EVT_RETENTION_TIME_CLEAR_FAILURE, "EDA EVT RETENTIONTIMECLEAR API FAILURE"},
	{EDA_LIMIT_GET_FAILURE, "EDA LIMIT GET API FAILURE"},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET eda_str_set[] = {
	{EDA_FC_HDLN, 0, (NCSFL_STR *)eda_hdln_set},
	{EDA_FC_HDLNF, 0, (NCSFL_STR *)eda_hdln_set},
	{0, 0, 0}
};

NCSFL_FMAT eda_fmat_set[] = {
	{EDA_LID_HDLN, NCSFL_TYPE_TCLILL, "%s EDA %14s:%5lu:%s:%lu:%lu\n"},
	{EDA_LID_HDLNF, NCSFL_TYPE_TCLILLF, "%s EDA %14s:%5lu:%s:%lu:%lu:%s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC eda_ascii_spec = {
	NCS_SERVICE_ID_EDA,	/* EDA subsystem */
	EDSV_LOG_VERSION,	/* EDA log version ID */
	"EDA",
	(NCSFL_SET *)eda_str_set,	/* EDA const strings referenced by index */
	(NCSFL_FMAT *)eda_fmat_set,	/* EDA string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  PROCEDURE NAME:    eda_flx_log_ascii_set_reg

  DESCRIPTION:       Function is used for registering the canned strings
                     with the DTS.

*****************************************************************************/
uint32_t eda_flx_log_ascii_set_reg()
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &eda_ascii_spec;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    eda_flx_log_ascii_set_dereg

  DESCRIPTION:       Function is used for deregistering the canned strings
                     with the DTS.

*****************************************************************************/
uint32_t eda_flx_log_ascii_set_dereg()
{

	NCS_DTSV_REG_CANNED_STR arg;
	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_EDA;
	arg.info.dereg_ascii_spec.version = EDSV_LOG_VERSION;

	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: eda_log_str_lib_req

  DESCRIPTION   : Entry point for eda log string library. 

  ARGUMENTS     : req_info  - A pointer to the input information
                              given by SBOM.
 
  RETURN VALUES : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  NOTES         : None.
 *****************************************************************************/

uint32_t eda_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
	uint32_t res = NCSCC_RC_SUCCESS;
	switch (req_info->i_op) {
	case NCS_LIB_REQ_CREATE:
		res = eda_flx_log_ascii_set_reg();
		break;

	case NCS_LIB_REQ_DESTROY:
		res = eda_flx_log_ascii_set_dereg();
		break;

	default:
		res = NCSCC_RC_FAILURE;
		break;
	}
	return (res);
}
