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

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#if (NCS_DTS == 1)
#include "ncsgl_defs.h"
#include "t_suite.h"
#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"
#include "avd_dblog.h"
#include "avsv_logstr.h"

uns32 avd_reg_strings(void);
uns32 avd_unreg_strings(void);

/******************************************************************************
 Logging stuff for Headlines 
 ******************************************************************************/
const NCSFL_STR avd_hdln_set[] = {
	{AVD_INVALID_VAL, "AN INVALID DATA VALUE"},
	{AVD_UNKNOWN_MSG_RCVD, "UNKNOWN EVENT RCVD"},
	{AVD_MSG_PROC_FAILED, "EVENT PROCESSING FAILED"},
	{AVD_ENTERED_FUNC, "ENTRED THE FUNCTION"},
	{AVD_RCVD_VAL, "RECEIVED THE VALUE"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Mem Fail 
 ******************************************************************************/
const NCSFL_STR avd_memfail_set[] = {
	{AVD_CB_ALLOC_FAILED, "CONTROL BLOCK ALLOC FAILED"},
	{AVD_MBX_ALLOC_FAILED, "MAILBOX CREATION ALLOC FAILED"},
	{AVD_AVND_ALLOC_FAILED, "AVD_AVND STRUCTURE ALLOC FAILED"},
	{AVD_COMP_ALLOC_FAILED, "AVD_COMP STRUCTURE ALLOC FAILED"},
	{AVD_CSI_ALLOC_FAILED, "AVD_CSI STRUCTURE ALLOC FAILED"},
	{AVD_COMPCSI_ALLOC_FAILED, "AVD_COMP_CSI_REL STRUCTURE ALLOC FAILED"},
	{AVD_SG_ALLOC_FAILED, "AVD_SG STRUCTURE ALLOC FAILED"},
	{AVD_SI_ALLOC_FAILED, "AVD_SI_NULL STRUCTURE ALLOC FAILED"},
	{AVD_SUSI_ALLOC_FAILED, "AVD_SU_SI_REL STRUCTURE ALLOC FAILED"},
	{AVD_SU_ALLOC_FAILED, "AVD_SU STRUCTURE ALLOC FAILED"},
	{AVD_PG_CSI_NODE_ALLOC_FAILED, "AVD_PG_CSI_NODE_ALLOC_FAILED STRUCTURE ALLOC FAILED"},
	{AVD_PG_NODE_CSI_ALLOC_FAILED, "AVD_PG_NODE_CSI_ALLOC_FAILED STRUCTURE ALLOC FAILED"},
	{AVD_EVT_ALLOC_FAILED, "AVD_EVT STRUCTURE ALLOC FAILED"},
	{AVD_HLT_ALLOC_FAILED, "AVD_HLT STRUCTURE ALLOC FAILED"},
	{AVD_DND_MSG_ALLOC_FAILED, "AVD TO AVND MESSAGE AVSV_DND_MSG ALLOC FAILED"},
	{AVD_D2D_MSG_ALLOC_FAILED, "AVD TO AVD MESSAGE AVD_D2D_MSG ALLOC FAILED"},
	{AVD_CSI_PARAM_ALLOC_FAILED, "AVD_CSI_PARAM STRUCTURE ALLOC FAILED"},
	{AVD_DND_MSG_INFO_ALLOC_FAILED, "AVD TO AVND MESSAGE LOCAL LIST INFORMATION ALLOC FAILED"},
	{AVD_SG_OPER_ALLOC_FAILED, "AVD_SG_OPER STRUCTURE ALLOC FAILED"},
	{AVD_SU_PER_SI_RANK_ALLOC_FAILED, "AVD_SUS_PER_SI_RANK STRUCTURE ALLOC FAILED"},
	{AVD_SG_SI_RANK_ALLOC_FAILED, "AVD_SG_SI_RANK STRUCTURE ALLOC FAILED"},
	{AVD_SG_SU_RANK_ALLOC_FAILED, "AVD_SG_SU_RANK STRUCTURE ALLOC FAILED"},
	{AVD_COMP_CS_TYPE_ALLOC_FAILED, "AVD_COMP_CS_TYPE STRUCTURE ALLOC FAILED"},
	{AVD_CS_TYPE_PARAM_ALLOC_FAILED, "AVD_CS_TYPE_PARAM STRUCTURE ALLOC FAILED"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Messages 
 ******************************************************************************/
const NCSFL_STR avd_msg_set[] = {
	{AVD_LOG_PROC_MSG, "processing the Message from: "},
	{AVD_LOG_RCVD_MSG, "Received the Message for processing from: "},
	{AVD_LOG_SND_MSG, "Send the message to: "},
	{AVD_LOG_DND_MSG, "AVND to AVD Message type: "},
	{AVD_LOG_D2D_MSG, "AVD to AVD Message type: "},
	{AVD_DUMP_DND_MSG, "Dump of the AVD AvND Message : "},
	{AVD_DUMP_D2D_MSG, "Dump of the AVD AVD Message: "},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Event 
 ******************************************************************************/
const NCSFL_STR avd_event_set[] = {
	{AVD_SND_TMR_EVENT, "Send Timer event for processing: "},
	{AVD_SND_AVD_MSG_EVENT, "Send AvD message event for processing: "},
	{AVD_SND_AVND_MSG_EVENT, "Send AvND message event for processing: "},
	{AVD_SND_MAB_EVENT, "Send MAB event for processing: "},
	{AVD_RCVD_EVENT, "Received the event for processing: "},
	{AVD_RCVD_INVLD_EVENT, "Received an invalid event for processing: "},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Checkpoint Role 
 ******************************************************************************/
const NCSFL_STR avsv_ckpt_set[] = {
	/* Role Change Events */
	{AVD_ROLE_CHANGE_ATOS, "Received event for role change - Active to Standby"},
	{AVD_ROLE_CHANGE_STOA, "Received event for role change - Standby to Active"},
	{AVD_ROLE_CHANGE_ATOQ, "Received event for role change - Active to QUIESCED"},
	{AVD_ROLE_CHANGE_QTOS, "Received event for role change - Quiesced to Standby"},
	{AVD_ROLE_CHANGE_QTOA, "Received event for role change - Quiesced to Active"},

	/* MBCSv Events */
	{AVD_MBCSV_MSG_ASYNC_UPDATE, "Async Update Request Received"},
	{AVD_COLD_SYNC_REQ_RCVD, "Cold Sync Request Received"},
	{AVD_MBCSV_MSG_COLD_SYNC_RESP, "Cold Sync Resp Request Received"},
	{AVD_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE, "Cold Sync Resp Cmplt Request Received"},
	{AVD_MBCSV_MSG_WARM_SYNC_REQ, "Warm Sync Request Received"},
	{AVD_MBCSV_MSG_WARM_SYNC_RESP, "Warm Sync Resp Request Received"},
	{AVD_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE, "Warm Sync Resp Cmplt Request Received"},
	{AVD_MBCSV_MSG_DATA_REQ, "Data Sync Request Received"},
	{AVD_MBCSV_MSG_DATA_RESP, "Data Sync Resp Request Received"},
	{AVD_MBCSV_MSG_DATA_RESP_COMPLETE, "Data Sync Resp Cmplt Request Received"},
	{AVD_MBCSV_ERROR_IND, "Error Indication is received for error - "},

	/* Failure's  */
	{AVD_MBCSV_MSG_WARM_SYNC_RESP_FAILURE, "Warm Sync Failure"},
	{AVD_MBCSV_MSG_DATA_RSP_DECODE_FAILURE, "Data Responce Decode failure"},
	{AVD_MBCSV_MSG_DISPATCH_FAILURE, "MBCSv Dispatch failure"},
	{AVD_STBY_UNAVAIL_FOR_RCHG, "Standby Is not Available for Role change"},
	{AVD_ROLE_CHANGE_FAILURE, "AVD Role change Failure"},
	{AVD_HB_MSG_SND_FAILURE, "HB message send Failure"},
	{AVD_HB_MISS_WITH_PEER, "Heart Beat missed with the peer"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for Ntfs 
 ******************************************************************************/
const NCSFL_STR avd_ntf_set[] = {
	{AVD_NTFS_AMF, "AMF"},
	{AVD_NTFS_CLM, "CLM"},
	{AVD_NTFS_CLUSTER, "Cluster"},
	{AVD_NTFS_UNASSIGNED, "Unassigned"},
	{AVD_NTFS_JOINED, "Joined"},
	{AVD_NTFS_EXITED, "Exited"},
	{AVD_NTFS_RECONFIGURED, "Reconfigured"},
	{AVD_NTFS_NCS_INIT_SUCCESS, "Initialization Successful on Node Id :"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for oper state
 ******************************************************************************/
const NCSFL_STR avd_oper_set[] = {
	{AVD_NTF_OPER_STATE_MIN, "do not use"},
	{AVD_NTF_OPER_STATE_ENABLE, "Enable"},
	{AVD_NTF_OPER_STATE_DISABLE, "Disable"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for Proxy-Proxied messages
 ******************************************************************************/
const NCSFL_STR avd_pxy_pxd_set[] = {
	{AVD_PXY_PXD_SUCC_INFO, "PXY-PXD SUCC"},
	{AVD_PXY_PXD_ERR_INFO, "PXY-PXD ERROR"},
	{AVD_PXY_PXD_ENTRY_INFO, "PXY-PXD Entry Info"},
	{0, 0}
};

/******************************************************************************
 Logging stuff for admin state
 ******************************************************************************/
const NCSFL_STR avd_admin_set[] = {
	{AVD_NTF_ADMIN_STATE_MIN, "do not use"},
	{AVD_NTF_ADMIN_STATE_LOCK, "Locked"},
	{AVD_NTF_ADMIN_STATE_UNLOCK, "Unlocked"},
	{AVD_NTF_ADMIN_STATE_SHUTDOWN, "Shuttingdown"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for HA state
 ******************************************************************************/
const NCSFL_STR avd_ha_state_set[] = {
	{AVD_NTF_HA_NONE, "None"},
	{AVD_NTF_HA_ACTIVE, "Active"},
	{AVD_NTF_HA_STANDBY, "StandBy"},
	{AVD_NTF_HA_QUIESCED, "Quiesced"},
	{AVD_NTF_HA_QUIESCING, "Quiescing"},

	{0, 0}
};

/******************************************************************************
 Logging stuff for failure of shutdown
 ******************************************************************************/
const NCSFL_STR avd_shutdown_failure[] = {
	{AVD_NTF_NODE_ACTIVE_SYS_CTRL, "Node is the Active system controller"},
	{AVD_NTF_SUS_SAME_SG, "SUs are in same SG on the node"},
	{AVD_NTF_SG_UNSTABLE, "the SG is unstable"},

	{0, 0}
};

/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET avd_str_set[] = {
	{AVD_FC_HDLN, 0, (NCSFL_STR *)avd_hdln_set},
	{AVD_FC_MEMFAIL, 0, (NCSFL_STR *)avd_memfail_set},
	{AVD_FC_MSG, 0, (NCSFL_STR *)avd_msg_set},
	{AVD_FC_EVT, 0, (NCSFL_STR *)avd_event_set},
	{AVSV_FC_SEAPI, 0, (NCSFL_STR *)avsv_seapi_set},
	{AVSV_FC_MDS, 0, (NCSFL_STR *)avsv_mds_set},
	{AVSV_FC_LOCK, 0, (NCSFL_STR *)avsv_lock_set},
	{AVSV_FC_MBX, 0, (NCSFL_STR *)avsv_mbx_set},
	{AVSV_FC_CKPT, 0, (NCSFL_STR *)avsv_ckpt_set},
	{AVD_FC_NTF, 0, (NCSFL_STR *)avd_ntf_set},
	{AVD_FC_OPER, 0, (NCSFL_STR *)avd_oper_set},
	{AVD_FC_ADMIN, 0, (NCSFL_STR *)avd_admin_set},
	{AVD_FC_SUSI_HA, 0, (NCSFL_STR *)avd_ha_state_set},
	{AVD_FC_PXY_PXD, 0, (NCSFL_STR *)avd_pxy_pxd_set},
	{AVD_FC_SHUTDOWN_FAILURE, 0, (NCSFL_STR *)avd_shutdown_failure},

	{0, 0, 0}
};

NCSFL_FMAT avd_fmat_set[] = {
	{AVD_LID_HDLN, NCSFL_TYPE_TIC, "%s AVD: %s %s\n"},
	{AVD_LID_HDLN_VAL, NCSFL_TYPE_TICLL, "%s AVD: %s at %s:%ld val %ld\n"},
	{AVD_LID_HDLN_VAL_NAME, "TICLP", "%s AVD: %s at %s:%ld val %s\n"},
	{AVD_LID_MEMFAIL, NCSFL_TYPE_TI, "%s AVD: %s\n"},
	{AVD_LID_MEMFAIL_LOC, NCSFL_TYPE_TICL, "%s AVD: %s at %s:%ld%\n"},
	{AVD_LID_MSG_INFO, NCSFL_TYPE_ILTIL, "AVD: %s 0x%08lx %s %s %ld\n"},
	{AVD_LID_MSG_DND_DTL, NCSFL_TYPE_TID, "%s AVD: %s %s\n"},
	{AVD_LID_FUNC_RETVAL, NCSFL_TYPE_TII, "%s AVD: %s %s\n"},
	{AVD_LID_EVT_VAL, NCSFL_TYPE_TIL, "%s AVD: %s %ld\n"},
	{AVD_LID_EVT_CKPT, "TIL", "%s AVD: %s %ld\n"},
	{AVD_LID_ADMIN, "TCI", "%s AVD: %s Admin State Changed to %s\n"},
	{AVD_LID_SI_UNASSIGN, "TCI", "%s AVD: %s %s\n"},
	{AVD_LID_OPER, "TCI", "%s AVD: %s Oper State Changed to %s\n"},
	{AVD_LID_SUSI_HA, "TCCI", "%s AVD: %s %s HA State Changed to %s\n"},
	{AVD_LID_CLM, "TCII", "%s AVD: %s %s %s\n"},
	{AVD_LID_NTFS_NCS_SUCC, "TIL", "%s AVD: %s 0x%08lx\n"},
	{AVD_LID_SUSI_HA_CHG_START, "TCCI", "%s AVD: %s %s HA State Changing to %s\n"},
	{AVD_LID_HDLN_SVAL, "TICLC", "%s AVD: %s at %s:%ld val %s\n"},
	{AVD_PXY_PXD, "TICCLLLL", "[%s]: %s : %s: (%s, %ld, %ld, %ld, %ld)\n"},
	{AVD_LID_SHUTDOWN_FAILURE, "TCI", "%s AVD: Shutdown failed of node %s as %s\n"},
	{AVD_LID_GENLOG, "TC", "%s %s\n"},
	{0, 0, 0}
};

NCSFL_ASCII_SPEC avd_ascii_spec = {
	NCS_SERVICE_ID_AVD,	/* AVD subsystem */
	AVSV_LOG_VERSION,	/* AVD log version ID */
	"AVD",
	(NCSFL_SET *)avd_str_set,	/* AVD const strings referenced by index */
	(NCSFL_FMAT *)avd_fmat_set,	/* AVD string format info */
	0,			/* Place holder for str_set count */
	0			/* Place holder for fmat_set count */
};

/*****************************************************************************

  avd_reg_strings

  DESCRIPTION: Function is used for registering the canned strings with the DTS.

*****************************************************************************/
uns32 avd_reg_strings(void)
{
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
	arg.info.reg_ascii_spec.spec = &avd_ascii_spec;
	if (ncs_dtsv_ascii_spec_api(&arg) == NCSCC_RC_FAILURE)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avd_unreg_strings

  Description   : This routine unregisters the AvD canned strings from DTS.

  Arguments     : None

  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  Notes         : None.
 *****************************************************************************/
uns32 avd_unreg_strings(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	NCS_DTSV_REG_CANNED_STR arg;

	arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
	arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_AVD;
	arg.info.dereg_ascii_spec.version = AVSV_LOG_VERSION;

	rc = ncs_dtsv_ascii_spec_api(&arg);

	return rc;
}

#endif   /* (NCS_DTS == 1) */
