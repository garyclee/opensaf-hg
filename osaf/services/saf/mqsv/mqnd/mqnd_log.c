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

#include "mqnd.h"

/*****************************************************************************
  FILE NAME: MQND_LOG.C

  DESCRIPTION: MQND Logging utilities

  FUNCTIONS INCLUDED in this module:
      mqnd_flx_log_reg
      mqnd_flx_log_unreg

******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME  :  mqnd_log

  DESCRIPTION     :  MQND logs

  ARGUMENT        :  id - Canned string id
                     category- Category of the log
                     sev - Severity of the log
                     rc - return code of the API or function
                     fname- filename
                     fno  - filenumber
*****************************************************************************/

void mqnd_log(uint8_t id, uint32_t category, uint8_t sev, uint32_t rc, char *fname, uint32_t fno)
{

	/* Log New type logs */
	ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_HDLN, MQND_FC_HDLN,
		   category, sev, NCSFL_TYPE_TCLIL, fname, fno, id, rc);

}	/* End of mqnd_new_log()  */

/****************************************************************************
 * Name          : mqnd_flx_log_reg
 *
 * Description   : This is the function which registers the MQND logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void mqnd_flx_log_reg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_MQND;
	/* fill version no. */
	reg.info.bind_svc.version = MQSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "MQSv");

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : mqnd_flx_log_dereg
 *
 * Description   : This is the function which deregisters the MQND logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void mqnd_flx_log_dereg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_MQND;
	ncs_dtsv_su_req(&reg);
	return;
}

void _mqnd_genlog(uint8_t severity, const char *function, const char *format, ...)
{
	char preamble[128];
	char str[128];
	va_list ap;

	va_start(ap, format);
	snprintf(preamble, sizeof(preamble), "%s - %s", function, format);
	vsnprintf(str, sizeof(str), preamble, ap);
	va_end(ap);

	ncs_logmsg(NCS_SERVICE_ID_MQND, MQND_LID_GENLOG, MQND_FC_GENLOG, NCSFL_LC_HEADLINE, severity, NCSFL_TYPE_TC,
		   str);
}
