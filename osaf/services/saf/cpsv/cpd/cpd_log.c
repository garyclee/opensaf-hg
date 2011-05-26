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

/****************************************************************************
  MODULE: cpd_loc.c

  DESCRIPTION:

  This module contains the logging/tracing functions for CPD

   cpd_lfx_log_reg....................Routine to register with DTA for looging
   cpd_flx_log_dereg..................Routine to deregister from DTA
   cpd_headline_log...................Routine to log headlines
   cpd_db_status_log..................Routine to log DB status

*****************************************************************************/

#include "cpd.h"

/****************************************************************************
 Name          : cpd_flx_log_reg

 Description   : This is the function which registers the CPD logging with
                 the Flex Log agent.
                 

 Arguments     : None.

 Return Values : None
*****************************************************************************/
void cpd_flx_log_reg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_CPD;
	/* fill version no. */
	reg.info.bind_svc.version = CPSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "CPSv");

	ncs_dtsv_su_req(&reg);
	return;
}	/* End of cpd_flx_log_reg() */

/****************************************************************************
 Name          : cpd_flx_log_dereg
 
 Description   : This is the function which deregisters the CPD logging 
                 with the Flex Log agent.
                 
 Arguments     : None.
 
 Return Values : None 
\*****************************************************************************/
void cpd_flx_log_dereg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_CPD;
	ncs_dtsv_su_req(&reg);
	return;
}	/* End of cpd_flx_log_dereg() */

/*****************************************************************************

  PROCEDURE NAME  :  cpd_headline_log

  DESCRIPTION     :  Headline logging info
   
  ARGUMENTS       :  hdln_id : Headline ID      
*****************************************************************************/
void cpd_headline_log(uint8_t hdln_id, uint8_t sev)
{
	/* Log headlines */
	ncs_logmsg(NCS_SERVICE_ID_CPD, CPD_LID_HEADLINE, CPD_FC_HDLN, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, hdln_id);
}	/* End of cpd_headline_log() */

/*****************************************************************************

  PROCEDURE NAME  :  cpd_db_status_log

  DESCRIPTION     :  Database logging info

  ARGUMENT        :  db_id - Database id
                     str   - log string
*****************************************************************************/
void cpd_db_status_log(uint8_t db_id, char *str)
{
	uint8_t sev = 0;

	switch (db_id) {
	case CPD_DB_ADD_FAILED:
	case CPD_DB_DEL_FAILED:
	case CPD_DB_UPD_FAILED:
		sev = NCSFL_SEV_ERROR;
		break;

	default:
		sev = NCSFL_SEV_INFO;
		break;
	}

	/* Log headlines */
	ncs_logmsg(NCS_SERVICE_ID_CPD, CPD_LID_DB, CPD_FC_DB, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIC, db_id, str);
}	/* End of cpd_headline_log() */

/*****************************************************************************

  PROCEDURE NAME:    cpd_log_memfail

  DESCRIPTION:       memory failure logging info

*****************************************************************************/
void cpd_memfail_log(uint8_t mf_id)
{
	ncs_logmsg(NCS_SERVICE_ID_CPD, CPD_LID_MEMFAIL, CPD_FC_MEMFAIL,
		   NCSFL_LC_MEMORY, NCSFL_SEV_ERROR, NCSFL_TYPE_TI, mf_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpd_mbcsv_log

  DESCRIPTION:       mbcsv failure logging info

*****************************************************************************/

void cpd_mbcsv_log(uint8_t mbcsv_id, uint8_t sev)
{
	ncs_logmsg(NCS_SERVICE_ID_CPD, CPD_LID_MBCSV, CPD_FC_MBCSV, NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, mbcsv_id);
}

/*****************************************************************************

  PROCEDURE NAME:    cpd_log

  DESCRIPTION:       cpd failure logging info

*****************************************************************************/
void _cpd_log(uint8_t severity, const char *function, const char *format, ...)
{
      char preamble[128];
       char str[128];
       va_list ap;

       va_start(ap, format);
       snprintf(preamble, sizeof(preamble), "%s - %s", function, format);
       vsnprintf(str, sizeof(str), preamble, ap);
       va_end(ap);

       ncs_logmsg(NCS_SERVICE_ID_CPD, CPD_LID_GENLOG, CPD_FC_GENLOG, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TC, str);
}
