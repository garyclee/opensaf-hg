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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

  log_init_ncsft..........General function to print log init message
  log_ncsft...............NCSFTlayer logging facility

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "dts.h"

#if (DTS_LOG == 1)

/*****************************************************************************

  ncsdts_log_bind

  DESCRIPTION: Function is used for binding with DTS.

*****************************************************************************/

uint32_t dts_log_bind(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, '\0', sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_DTSV;
	/* fill version no. */
	reg.info.bind_svc.version = DTS_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "DTS");

	if (ncs_dtsv_su_req(&reg) != NCSCC_RC_SUCCESS)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, " dts_log_bind: NCS_SERVICE_ID_DTSV bind failed");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  ncsdts_log_unbind

  DESCRIPTION: Function is used for unbinding with DTS.

*****************************************************************************/

uint32_t dts_log_unbind(void)
{
	NCS_DTSV_RQ dereg;

	dereg.i_op = NCS_DTSV_OP_UNBIND;
	dereg.info.unbind_svc.svc_id = NCS_SERVICE_ID_DTSV;
	if (ncs_dtsv_su_req(&dereg) != NCSCC_RC_SUCCESS)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_log_bind: Unbind failed for NCS_SERVICE_ID_DTSV");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/

void log_dts_headline(uint8_t hdln_id)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_HDLN, DTS_FC_HDLN, NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, "TI", hdln_id);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_svc_prvdr

  DESCRIPTION:       Service Provider logging info

*****************************************************************************/

void log_dts_svc_prvdr(uint8_t sp_id)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_SVC_PRVDR_FLEX, DTS_FC_SVC_PRVDR_FLEX,
		   NCSFL_LC_SVC_PRVDR, NCSFL_SEV_INFO, "TI", sp_id);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_lock

  DESCRIPTION:       lock oriented logging info

*****************************************************************************/

void log_dts_lock(uint8_t lck_id, void *lck)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_LOCKS, DTS_FC_LOCKS,
		   NCSFL_LC_LOCKS, NCSFL_SEV_DEBUG, "TIL", lck_id, (long)lck);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/

void log_dts_memfail(uint8_t mf_id)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_MEMFAIL, DTS_FC_MEMFAIL, NCSFL_LC_MEMORY, NCSFL_SEV_DEBUG, "TI", mf_id);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_api

  DESCRIPTION:       API logging info

*****************************************************************************/

void log_dts_api(uint8_t api_id)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_API, DTS_FC_API, NCSFL_LC_API, NCSFL_SEV_NOTICE, "TI", api_id);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_evt

  DESCRIPTION:       Event logging info

*****************************************************************************/

void log_dts_evt(uint8_t evt_id, SS_SVC_ID svc_id, uint32_t node, uint32_t adest)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_EVT, DTS_FC_EVT,
		   NCSFL_LC_EVENT, NCSFL_SEV_INFO, "TILLL", evt_id, svc_id, node, adest);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_chkp_evt

  DESCRIPTION:       Checkpointing Event logging info

*****************************************************************************/
void log_dts_chkp_evt(uint8_t id)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_CHKP, DTS_FC_UPDT, NCSFL_LC_EVENT, NCSFL_SEV_NOTICE, "TI", id);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_cbop

  DESCRIPTION:       Event logging info

*****************************************************************************/

void log_dts_cbop(uint8_t op_id, SS_SVC_ID svc_id, uint32_t node)
{
	ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_CB_LOG, DTS_FC_CIRBUFF,
		   NCSFL_LC_MISC, NCSFL_SEV_INFO, "TILL", op_id, svc_id, node);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_dbug

  DESCRIPTION:       Debug logging info

*****************************************************************************/

void log_dts_dbg(uint8_t id, char *str, NODE_ID node, SS_SVC_ID svc)
{
	if (id == DTS_SERVICE) {
		char tstr[200];
		sprintf(tstr, "%d %d - %s", node, svc, str);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STR, DTS_FC_STR,
			   NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TIC", id, tstr);
	} else
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STR, DTS_FC_STR,
			   NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TIC", id, str);
}

/*****************************************************************************

  PROCEDURE NAME:    log_dts_dbug_name

  DESCRIPTION:       Debug logging info

*****************************************************************************/

void log_dts_dbg_name(uint8_t id, char *str, uint32_t svc_id, char *svc)
{
	if (id == DTS_SERVICE) {
		char tstr[200];
		sprintf(tstr, "%d %s - %s", svc_id, svc, str);
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STR, DTS_FC_STR,
			   NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TIC", id, tstr);
	} else
		ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_STR, DTS_FC_STR,
			   NCSFL_LC_MISC, NCSFL_SEV_DEBUG, "TIC", id, str);
}




void _dts_log(uint8_t severity, const char *function, const char *format, ...)
{
        char preamble[128];
        char str[128];
        va_list ap;

        va_start(ap, format);
        snprintf(preamble, sizeof(preamble), "%s - %s", function, format);
        vsnprintf(str, sizeof(str), preamble, ap);
        va_end(ap);

        ncs_logmsg(NCS_SERVICE_ID_DTSV, DTS_LID_GENLOG, DTS_FC_GENLOG, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TC, str);
}
#endif   /* (DTS_LOG == 1) */
