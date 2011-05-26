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

#include "gla.h"

/*****************************************************************************
  FILE NAME: GLA_LOG.C

  DESCRIPTION: GLA Logging utilities

  FUNCTIONS INCLUDED in this module:
      gla_flx_log_reg
      gla_flx_log_unreg

******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    gla_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void gla_log_headline(uint8_t hdln_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLA, GLA_LID_HDLN, GLA_FC_HDLN,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TICL, hdln_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gla_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void gla_log_memfail(uint8_t mf_id, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLA, GLA_LID_MEMFAIL, GLA_FC_MEMFAIL,
		   NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TICL, mf_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gla_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void gla_log_api(uint8_t api_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLA, GLA_LID_API, GLA_FC_API,
		   NCSFL_LC_API, sev, NCSFL_TYPE_TICL, api_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gla_log_lockfail

  DESCRIPTION:       API logging info

*****************************************************************************/
void gla_log_lockfail(uint8_t api_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLA, GLA_LID_NCS_LOCK, GLA_FC_NCS_LOCK,
		   NCSFL_LC_LOCKS, sev, NCSFL_TYPE_TICL, api_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gla_log_sys_call

  DESCRIPTION:       System call logging info

*****************************************************************************/
void gla_log_sys_call(uint8_t id, char *file_name, uint32_t line_no, SaUint64T handle_id)
{
	ncs_logmsg(NCS_SERVICE_ID_GLA, GLA_LID_SYS_CALL, GLA_FC_SYS_CALL,
		   NCSFL_LC_SYS_CALL_FAIL, NCSFL_SEV_ERROR, NCSFL_TYPE_TICLL, id, file_name, line_no, handle_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gla_log_data_send

  DESCRIPTION:       MDS send logging info

*****************************************************************************/
void gla_log_data_send(uint8_t id, char *file_name, uint32_t line_no, uint32_t node, uint32_t evt_id)
{
	ncs_logmsg(NCS_SERVICE_ID_GLA, GLA_LID_DATA_SEND, GLA_FC_DATA_SEND,
		   NCSFL_LC_DATA, NCSFL_SEV_ERROR, NCSFL_TYPE_TICLLL, id, file_name, line_no, node, evt_id);
}

/****************************************************************************
 * Name          : gla_flx_log_reg
 *
 * Description   : This is the function which registers the GLA logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void gla_flx_log_reg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_GLA;
	/* fill version no. */
	reg.info.bind_svc.version = GLSV_LOG_VERSION;
	/* fill svc_name */
	if (strlen("GLSv") < sizeof(reg.info.bind_svc.svc_name))
		strncpy(reg.info.bind_svc.svc_name, "GLSv", sizeof(reg.info.bind_svc.svc_name));

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : gla_flx_log_dereg
 *
 * Description   : This is the function which deregisters the GLA logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void gla_flx_log_dereg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_GLA;
	ncs_dtsv_su_req(&reg);
	return;
}
