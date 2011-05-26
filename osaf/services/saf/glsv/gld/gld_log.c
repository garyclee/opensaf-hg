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

#include "gld.h"

#if (NCS_GLSV_LOG == 1)
/*****************************************************************************
  FILE NAME: GLD_LOG.C

  DESCRIPTION: GLD Logging utilities

  FUNCTIONS INCLUDED in this module:
      gld_flx_log_reg
      gld_flx_log_unreg
******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    gld_log_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void gld_log_headline(uint8_t hdln_id, uint8_t sev, char *file_name, uint32_t line_no, SaUint32T node_id)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_HDLN, GLD_FC_HDLN,
		   NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TICLL, hdln_id, file_name, line_no, node_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void gld_log_memfail(uint8_t mf_id, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_MEMFAIL, GLD_FC_MEMFAIL,
		   NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TICL, mf_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_api

  DESCRIPTION:       API logging info

*****************************************************************************/
void gld_log_api(uint8_t api_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_API, GLD_FC_API,
		   NCSFL_LC_API, sev, NCSFL_TYPE_TICL, api_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_evt

  DESCRIPTION:       Event logging info

*****************************************************************************/
void gld_log_evt(uint8_t evt_id, uint8_t sev, char *file_name, uint32_t line_no, uint32_t rsc_id, uint32_t node_id)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_EVT, GLD_FC_EVT,
		   NCSFL_LC_EVENT, sev, NCSFL_TYPE_TICLLL, evt_id, file_name, line_no, rsc_id, node_id);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_mbcsv_log

  DESCRIPTION:       mbcsv failure logging info

*****************************************************************************/

void gld_mbcsv_log(uint8_t mbcsv_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_MBCSV, GLD_FC_MBCSV, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TICL, mbcsv_id,
		   file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_svc_prvdr

  DESCRIPTION:       Service Provider logging info

*****************************************************************************/
void gld_log_svc_prvdr(uint8_t sp_id, uint8_t sev, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_SVC_PRVDR, GLD_FC_SVC_PRVDR,
		   NCSFL_LC_SVC_PRVDR, sev, NCSFL_TYPE_TICL, sp_id, file_name, line_no);
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_lck_oper

  DESCRIPTION:       lock oriented logging info

*****************************************************************************/
void gld_log_lck_oper(uint8_t lck_id, uint8_t sev, char *file_name, uint32_t line_no, char *rsc_name, uint32_t rsc_id,
		      uint32_t node_id)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_LCK_OPER, GLD_FC_LCK_OPER,
		   NCSFL_LC_MISC, sev, NCSFL_TYPE_TICLCLL, lck_id, file_name, line_no, rsc_name, rsc_id, node_id);
}

/****************************************************************************
 * Name          : gld_flx_log_reg
 *
 * Description   : This is the function which registers the GLD logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void gld_flx_log_reg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_GLD;
	/* fill version no. */
	reg.info.bind_svc.version = GLSV_LOG_VERSION;
	/* fill svc_name */
	if (strlen("GLSv") < sizeof(reg.info.bind_svc.svc_name))
		strncpy(reg.info.bind_svc.svc_name, "GLSv", sizeof(reg.info.bind_svc.svc_name));

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : gld_flx_log_dereg
 *
 * Description   : This is the function which deregisters the GLD logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void gld_flx_log_dereg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_GLD;
	ncs_dtsv_su_req(&reg);
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    gld_log_timer

  DESCRIPTION:       Timer stast/stop/exp logging info

*****************************************************************************/
void gld_log_timer(uint8_t id, uint32_t type, char *file_name, uint32_t line_no)
{
	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_TIMER, GLD_FC_TIMER,
		   NCSFL_LC_TIMER, NCSFL_SEV_ERROR, NCSFL_TYPE_TICLL, id, file_name, line_no, type);
}

void _gld_log(uint8_t severity, const char *function, const char *format, ...)
{
	char preamble[128];
	char str[128];
	va_list ap;

	va_start(ap, format);
	snprintf(preamble, sizeof(preamble), "%s - %s", function, format);
	vsnprintf(str, sizeof(str), preamble, ap);
	va_end(ap);

	ncs_logmsg(NCS_SERVICE_ID_GLD, GLD_LID_GENLOG, GLD_FC_GENLOG, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TC, str);
}
#else
extern int dummy;

#endif
