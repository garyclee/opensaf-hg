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

#include "glnd.h"

/*****************************************************************************
  FILE NAME: GLND_LOG.C

  DESCRIPTION: GLND Logging utilities

  FUNCTIONS INCLUDED in this module:
      glnd_flx_log_reg
      glnd_flx_log_unreg

******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void glnd_log_headline(uint8_t hdln_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_HDLN, GLND_FC_HDLN,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TICL, hdln_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_headline_TIL

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void glnd_log_headline_TIL(uint8_t hdln_id, char *file_name, uint32_t line_no, uint32_t parm1)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_HDLN_TIL, GLND_FC_HDLN_TIL,
		   NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TICLL, hdln_id, file_name, line_no, parm1);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_headline_TILL

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void glnd_log_headline_TILL(uint8_t hdln_id, char *file_name, uint32_t line_no, uint32_t parm1, uint32_t parm2)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_HDLN_TILL, GLND_FC_HDLN_TILL,
		   NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TICLLL, hdln_id, file_name, line_no, parm1, parm2);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_headline_TILLL

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void glnd_log_headline_TILLL(uint8_t hdln_id, char *file_name, uint32_t line_no, uint32_t parm1, uint32_t parm2, uint32_t parm3)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_HDLN_TILLL, GLND_FC_HDLN_TILLL,
		   NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TICLLLL,
		   hdln_id, file_name, line_no, parm1, parm2, parm3);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void glnd_log_memfail(uint8_t mf_id, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_MEMFAIL, GLND_FC_MEMFAIL,
		   NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TICL, mf_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void glnd_log_api(uint8_t api_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_API, GLND_FC_API,
		   NCSFL_LC_API, sev, NCSFL_TYPE_TICL, api_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_evt

  DESCRIPTION:       Event logging info

*****************************************************************************/
void glnd_log_evt(uint8_t evt_id, char *file_name, uint32_t line_no, uint32_t type, uint32_t node, uint32_t hdl, uint32_t rsc, uint32_t lck)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_EVT, GLND_FC_EVT,
		   NCSFL_LC_EVENT, NCSFL_SEV_INFO, NCSFL_TYPE_TICLLLLLL,
		   evt_id, file_name, line_no, type, node, hdl, rsc, lck);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_data_send

  DESCRIPTION:       MDS send logging info

*****************************************************************************/
void glnd_log_data_send(uint8_t id, char *file_name, uint32_t line_no, uint32_t node, uint32_t evt_id)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_DATA_SEND, GLND_FC_DATA_SEND,
		   NCSFL_LC_DATA, NCSFL_SEV_ERROR, NCSFL_TYPE_TICLLL, id, file_name, line_no, node, evt_id);
}

/*****************************************************************************

  PROCEDURE NAME:    glnd_log_timer

  DESCRIPTION:       Timer stast/stop/exp logging info

*****************************************************************************/
void glnd_log_timer(uint8_t id, uint32_t type, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_TIMER, GLND_FC_TIMER,
		   NCSFL_LC_TIMER, NCSFL_SEV_ERROR, NCSFL_TYPE_TICLL, id, file_name, line_no, type);
}

/****************************************************************************
 * Name          : glnd_flx_log_reg
 *
 * Description   : This is the function which registers the GLND logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void glnd_flx_log_reg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_GLND;
	/* fill version no. */
	reg.info.bind_svc.version = GLSV_LOG_VERSION;
	/* fill svc_name */
	if (strlen("GLSv") < sizeof(reg.info.bind_svc.svc_name))
		strncpy(reg.info.bind_svc.svc_name, "GLSv", sizeof(reg.info.bind_svc.svc_name));

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : glnd_flx_log_dereg
 *
 * Description   : This is the function which deregisters the GLND logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void glnd_flx_log_dereg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_GLND;
	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : glnd_log
 *
 * Description   :
 *
 *
 *
 * Arguments     :
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void glnd_log(uint8_t id, uint32_t category, uint8_t sev, uint32_t rc, char *file_name, uint32_t line_no, SaUint64T handle_id,
	      SaUint32T res_id, SaUint64T lock_id)
{
	ncs_logmsg(NCS_SERVICE_ID_GLND, GLND_LID_LOG, GLND_FC_LOG,
		   category, sev, NCSFL_TYPE_TCLILLLL, file_name, line_no, id, rc, handle_id, res_id, lock_id);
}
