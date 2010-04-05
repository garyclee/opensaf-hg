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

  DESCRIPTION:This module has all the functionality that deals with
  the DTSv modules agent for logging debug messages.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_flx_log_reg - registers the AVD logging with DTA.
  avd_flx_log_dereg - unregisters the AVD logging with DTA.

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

/****************************************************************************
  Name          : avd_log_admin_state_ntfs

  Description   : This routine logs the admin state related ntfs.

  Arguments     : State    - Admin State
                  name     - ptr to the name (node/su/sg/si name)
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avd_log_admin_state_ntfs(AVD_ADMIN_STATE_FLEX state, SaNameT *name, uns8 sev)
{
	char lname[SA_MAX_NAME_LENGTH];

	memset(lname, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (name)
		strncpy(lname, (char *)name->value, name->length);

	ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_ADMIN, AVD_FC_ADMIN, NCSFL_LC_HEADLINE, sev, "TCI", lname, state);

	return;
}

/****************************************************************************
  Name          : avd_log_si_unassigned_ntfs

  Description   : This routine logs the si unassigned related ntfs.

  Arguments     : State    - State
                  name     - ptr to the si name
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avd_log_si_unassigned_ntfs(AVD_NTF_FLEX state, SaNameT *name, uns8 sev)
{
	char lname[SA_MAX_NAME_LENGTH];

	memset(lname, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (name)
		strncpy(lname, (char *)name->value, name->length);

	ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_SI_UNASSIGN, AVD_FC_NTF, NCSFL_LC_HEADLINE, sev, "TCI", lname, state);

	return;
}

/****************************************************************************
  Name          : avd_log_oper_state_ntfs

  Description   : This routine logs the oper state related ntfs.

  Arguments     : State    - oper State
                  name     - ptr to the name 
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avd_log_oper_state_ntfs(AVD_OPER_STATE_FLEX state, SaNameT *name, uns8 sev)
{
	char lname[SA_MAX_NAME_LENGTH];

	memset(lname, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (name)
		strncpy(lname, (char *)name->value, name->length);

	ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_OPER, AVD_FC_OPER, NCSFL_LC_HEADLINE, sev, "TCI", lname, state);

	return;
}

/****************************************************************************
  Name          : avd_log_clm_node_ntfs

  Description   : This routine logs the clm node related ntfs.

  Arguments     : op    - Operation
                  cl    - "Cluster"
                  name - ptr to the node name
                  sev      - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avd_log_clm_node_ntfs(AVD_NTF_FLEX cl, AVD_NTF_FLEX op, SaNameT *name, uns8 sev)
{
	char lname[SA_MAX_NAME_LENGTH];

	memset(lname, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (name)
		strncpy(lname, (char *)name->value, name->length);

	ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_CLM, AVD_FC_NTF, NCSFL_LC_HEADLINE, sev, "TCII", lname, op, cl);

	return;
}

/****************************************************************************
  Name          : avd_log_susi_ha_ntfs

  Description   : This routine logs the SUSI HA related ntfs.

  Arguments     : state         - HA State
                  su_name       - ptr to the su name
                  si_name       - ptr to the si name
                  sev           - severity
                  isStateChanged- Flag to specify that this routine is called in context of 
                                  HA State changed or HA Sate changing ntfs.

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avd_log_susi_ha_ntfs(AVD_HA_STATE_FLEX state,
			  SaNameT *su_name, SaNameT *si_name, uns8 sev, NCS_BOOL isStateChanged)
{
	char lsu_name[SA_MAX_NAME_LENGTH];
	char lsi_name[SA_MAX_NAME_LENGTH];

	memset(lsu_name, '\0', SA_MAX_NAME_LENGTH);
	memset(lsi_name, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (su_name)
		strncpy(lsu_name, (char *)su_name->value, su_name->length);

	/* convert name into string format */
	if (si_name)
		strncpy(lsi_name, (char *)si_name->value, si_name->length);

	if (isStateChanged)
		ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_SUSI_HA, AVD_FC_SUSI_HA,
			   NCSFL_LC_HEADLINE, sev, "TCCI", lsu_name, lsi_name, state);
	else
		ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_SUSI_HA_CHG_START, AVD_FC_SUSI_HA,
			   NCSFL_LC_HEADLINE, sev, "TCCI", lsu_name, lsi_name, state);

	return;
}

/****************************************************************************
  Name          : avd_log_shutdown_failure

  Description   : This routine logs the failure of shutdown ntfs.

  Arguments     : node_name     - Node name
                  sev           - severity
                  errcode       - Error code indicating the reason of failure
                                  of shutdown

  Return Values : None

  Notes         : errcode =
                  1: Node is active system controller
                  2: SUs are in same SG on node
                  3: SG is unstable
                  Index to the string is formed by subtracting 1 from errcode
 *****************************************************************************/
void avd_log_shutdown_failure(SaNameT *node_name, uns8 sev, AVD_SHUTDOWN_FAILURE_FLEX errcode)
{
	char lnode_name[SA_MAX_NAME_LENGTH];

	memset(lnode_name, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (node_name)
		strncpy(lnode_name, (char *)node_name->value, node_name->length);

	ncs_logmsg(NCS_SERVICE_ID_AVD, AVD_LID_SHUTDOWN_FAILURE, AVD_FC_SHUTDOWN_FAILURE,
		   NCSFL_LC_HEADLINE, sev, "TCI", lnode_name, (errcode - 1));
}

/****************************************************************************
 * Name          : avd_flx_log_reg
 *
 * Description   : This is the function which registers the AVD logging with
 *                 the Flex Log agent.
 *                 
 *
 * Arguments     : None.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void avd_flx_log_reg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_AVD;
	/* fill version no. */
	reg.info.bind_svc.version = AVSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "AvSv");

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : avd_flx_log_dereg
 *
 * Description   : This is the function which deregisters the AVD logging 
 *                 with the Flex Log agent.
 *                 
 *
 * Arguments     : None.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void avd_flx_log_dereg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_AVD;
	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
  Name          : avd_pxy_pxd_log

  Description   : This routine logs the proxy-proxied communication.

  Arguments     : sev       - Severity
                  index     - Index
                  info      - Ptr to general info abt logs (Strings)
                  comp_name - Ptr to the comp name, if any.
                  Info1     - Value corresponding to Info description
                  Info2     - Value corresponding to Info description
                  Info3     - Value corresponding to Info description
                  Info4     - Value corresponding to Info description

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void avd_pxy_pxd_log(uns32 sev,
		     uns32 index, char *info, SaNameT *comp_name, uns32 info1, uns32 info2, uns32 info3, uns32 info4)
{
	char comp[SA_MAX_NAME_LENGTH];

	memset(comp, '\0', SA_MAX_NAME_LENGTH);

	/* convert name into string format */
	if (comp_name)
		strncpy(comp, (char *)comp_name->value, comp_name->length);

	ncs_logmsg(NCS_SERVICE_ID_AVD, (uns8)AVD_PXY_PXD, (uns8)AVD_FC_PXY_PXD,
		   NCSFL_LC_HEADLINE, (uns8)sev, NCSFL_TYPE_TICCLLLL, index, info, comp, info1, info2, info3, info4);
	return;
}

