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

  This file contains canned strings that are used for logging AvA information.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#if (NCS_DTS == 1)

#include "ncsgl_defs.h"
#include "t_suite.h"
#include "saAis.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "avsv_log.h"
#include "ava_log.h"
#include "avsv_logstr.h"

uns32 ava_str_reg(void);
uns32 ava_str_unreg(void);

/******************************************************************************
                      Canned string for selection object
 ******************************************************************************/
const NCSFL_STR ava_sel_obj_set[] = {
	{AVA_LOG_SEL_OBJ_CREATE, "Sel Obj Create"},
	{AVA_LOG_SEL_OBJ_DESTROY, "Sel Obj Destroy"},
	{AVA_LOG_SEL_OBJ_IND_SEND, "Sel Obj Ind Send"},
	{AVA_LOG_SEL_OBJ_IND_REM, "Sel Obj Ind Rem"},
	{AVA_LOG_SEL_OBJ_SUCCESS, "Success"},
	{AVA_LOG_SEL_OBJ_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                          Canned string for AMF APIs
 ******************************************************************************/
/* ensure that the string set ordering matches that of API type definition */
const NCSFL_STR ava_api_set[] = {
	{AVA_LOG_API_FINALIZE, "saAmfFinalize()"},
	{AVA_LOG_API_COMP_REG, "saAmfComponentRegister()"},
	{AVA_LOG_API_COMP_UNREG, "saAmfComponentUnregister()"},
	{AVA_LOG_API_PM_START, "saAmfPmStart()"},
	{AVA_LOG_API_PM_STOP, "saAmfPmStop()"},
	{AVA_LOG_API_HC_START, "saAmfHealthcheckStart()"},
	{AVA_LOG_API_HC_STOP, "saAmfHealthcheckStop()"},
	{AVA_LOG_API_HC_CONFIRM, "saAmfHealthcheckConfirm()"},
	{AVA_LOG_API_CSI_QUIESCING_COMPLETE, "saAmfCSIQuiescingComplete()"},
	{AVA_LOG_API_HA_STATE_GET, "saAmfHAStateGet()"},
	{AVA_LOG_API_PG_START, "saAmfProtectionGroupTrackStart()"},
	{AVA_LOG_API_PG_STOP, "saAmfProtectionGroupTrackStop()"},
	{AVA_LOG_API_ERR_REP, "saAmfErrorReport()"},
	{AVA_LOG_API_ERR_CLEAR, "saAmfComponentErrorClear()"},
	{AVA_LOG_API_RESP, "saAmfResponse()"},
	{AVA_LOG_API_INITIALIZE, "saAmfInitialize()"},
	{AVA_LOG_API_SEL_OBJ_GET, "saAmfSelectionObjectGet()"},
	{AVA_LOG_API_DISPATCH, "saAmfDispatch()"},
	{AVA_LOG_API_COMP_NAME_GET, "saAmfComponentNameGet()"},
	{AVA_LOG_API_ERR_SA_OK, "Ok"},
	{AVA_LOG_API_ERR_SA_LIBRARY, "Lib Err"},
	{AVA_LOG_API_ERR_SA_VERSION, "Ver Err"},
	{AVA_LOG_API_ERR_SA_INIT, "Init Err"},
	{AVA_LOG_API_ERR_SA_TIMEOUT, "Timeout Err"},
	{AVA_LOG_API_ERR_SA_TRY_AGAIN, "Try Again Err"},
	{AVA_LOG_API_ERR_SA_INVALID_PARAM, "Inv Param Err"},
	{AVA_LOG_API_ERR_SA_NO_MEMORY, "No Mem Err"},
	{AVA_LOG_API_ERR_SA_BAD_HANDLE, "Bad Hdl Err"},
	{AVA_LOG_API_ERR_SA_BUSY, "Busy Err"},
	{AVA_LOG_API_ERR_SA_ACCESS, "Access Err Err"},
	{AVA_LOG_API_ERR_SA_NOT_EXIST, "Does Not Exist Err"},
	{AVA_LOG_API_ERR_SA_NAME_TOO_LONG, "Name Too Long Err"},
	{AVA_LOG_API_ERR_SA_EXIST, "Exist Err"},
	{AVA_LOG_API_ERR_SA_NO_SPACE, "No Space Err"},
	{AVA_LOG_API_ERR_SA_INTERRUPT, "Interrupt Err"},
	{AVA_LOG_API_ERR_SA_NAME_NOT_FOUND, "Name Not Found Err"},
	{AVA_LOG_API_ERR_SA_NO_RESOURCES, "No Resource Err"},
	{AVA_LOG_API_ERR_SA_NOT_SUPPORTED, "Not Supp Err"},
	{AVA_LOG_API_ERR_SA_BAD_OPERATION, "Bad Oper Err"},
	{AVA_LOG_API_ERR_SA_FAILED_OPERATION, "Failed Oper Err"},
	{AVA_LOG_API_ERR_SA_MESSAGE_ERROR, "Msg Err"},
	{AVA_LOG_API_ERR_SA_QUEUE_FULL, "Q Full Err"},
	{AVA_LOG_API_ERR_SA_QUEUE_NOT_AVAILABLE, "Q Not Available Err"},
	{AVA_LOG_API_ERR_SA_BAD_CHECKPOINT, "Bad Checkpoint Err"},
	{AVA_LOG_API_ERR_SA_BAD_FLAGS, "Bad Flags Err"},
	{AVA_LOG_API_ERR_SA_TOO_BIG, "Too Big Err"},
	{AVA_LOG_API_ERR_SA_NO_SECTIONS, "No Sections Err"},
	{0, 0}
};

/******************************************************************************
                     Canned string for AvA handle database
 ******************************************************************************/
const NCSFL_STR ava_hdl_db_set[] = {
	{AVA_LOG_HDL_DB_CREATE, "Hdl DB Create"},
	{AVA_LOG_HDL_DB_DESTROY, "Hdl DB Destroy"},
	{AVA_LOG_HDL_DB_REC_ADD, "Hdl DB Rec Add"},
	{AVA_LOG_HDL_DB_REC_CBK_ADD, "Hdl DB Rec Cbk Add"},
	{AVA_LOG_HDL_DB_REC_DEL, "Hdl DB Rec Del"},
	{AVA_LOG_HDL_DB_SUCCESS, "Success"},
	{AVA_LOG_HDL_DB_FAILURE, "Failure"},
	{0, 0}
};

/******************************************************************************
                    Canned string for miscellaneous AvA events
 ******************************************************************************/
const NCSFL_STR ava_misc_set[] = {
	{AVA_LOG_MISC_AVND_UP, "AvND Up"},
	{AVA_LOG_MISC_AVND_DN, "AvND Down"},
	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/
NCSFL_SET ava_str_set[] = {
	{AVA_FC_SEAPI, 0, (NCSFL_STR *)avsv_seapi_set},
	{AVA_FC_MDS, 0, (NCSFL_STR *)avsv_mds_set},
	{AVA_FC_EDU, 0, (NCSFL_STR *)avsv_edu_set},
	{AVA_FC_LOCK, 0, (NCSFL_STR *)avsv_lock_set},
	{AVA_FC_CB, 0, (NCSFL_STR *)avsv_cb_set},
	{AVA_FC_CBK, 0, (NCSFL_STR *)avsv_cbk_set},
	{AVA_FC_SEL_OBJ, 0, (NCSFL_STR *)ava_sel_obj_set},
	{AVA_FC_API, 0, (NCSFL_STR *)ava_api_set},
	{AVA_FC_HDL_DB, 0, (NCSFL_STR *)ava_hdl_db_set},
	{AVA_FC_MISC, 0, (NCSFL_STR *)ava_misc_set},
	{0, 0, 0}
};

NCSFL_FMAT ava_fmat_set[] = {
	/* <SE-API Create/Destroy> <Success/Failure> */
	{AVA_LID_SEAPI, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <MDS Register/Install/...> <Success/Failure> */
	{AVA_LID_MDS, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <EDU Init/Finalize> <Success/Failure> */
	{AVA_LID_EDU, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <Lock Init/Finalize/Take/Give> <Success/Failure> */
	{AVA_LID_LOCK, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* <CB Create/Destroy/...> <Success/Failure> */
	{AVA_LID_CB, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* Invoked <cbk-name> for Comp: <comp-name> */
	{AVA_LID_CBK, NCSFL_TYPE_TIC, "[%s] Invoked %s for Comp: %s\n"},

	/* <Sel Obj Create/Destroy/...> <Success/Failure> */
	{AVA_LID_SEL_OBJ, NCSFL_TYPE_TII, "[%s] %s %s\n"},

	/* Processed <API Name>: <Ok/...(amf-err-code)> for comp <Comp Name>  */
	{AVA_LID_API, NCSFL_TYPE_TIIC, "[%s] Processed %s: %s for comp %s\n"},

	/* <Hdl DB Create/Destroy/...> <Success/Failure>: Hdl: <hdl> */
	{AVA_LID_HDL_DB, NCSFL_TYPE_TIIL, "[%s] %s %s: Hdl: %lu\n"},

	/* <AvND Up/Down...> */
	{AVA_LID_MISC, NCSFL_TYPE_TI, "[%s] %s\n"},

	{0, 0, 0}
};

NCSFL_ASCII_SPEC ava_ascii_spec = {
	NCS_SERVICE_ID_AVA,	/* AvA service id */
	AVSV_LOG_VERSION,	/* AvA log version id */
	"AvA",			/* used for generating log filename */
	(NCSFL_SET *)ava_str_set,	/* AvA const strings ref by index */
	(NCSFL_FMAT *)ava_fmat_set,	/* AvA string format info */
	0,			/* placeholder for str_set cnt */
	0			/* placeholder for fmat_set cnt */
};

/****************************************************************************
  Name          : ava_str_reg
 
  Description   : This routine registers the AvA canned strings with DTS.
                  
  Arguments     : None
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 ava_str_reg(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &ava_ascii_spec;

	rc = ncs_dtsv_ascii_spec_api(&arg);

	return rc;
}

/****************************************************************************
  Name          : ava_str_unreg
 
  Description   : This routine unregisters the AvA canned strings from DTS.
                  
  Arguments     : None
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
 *****************************************************************************/
uns32 ava_str_unreg(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_AVA;
	arg.info.dereg_ascii_spec.version = AVSV_LOG_VERSION;

	rc = ncs_dtsv_ascii_spec_api(&arg);

	return rc;
}

#endif   /* (NCS_DTS == 1) */
