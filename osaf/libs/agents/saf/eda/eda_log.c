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

#include "eda.h"

/*****************************************************************************
  FILE NAME: EDA_LOG.C

  DESCRIPTION: EDA Logging utilities

  FUNCTIONS INCLUDED in this module:
      eda_flx_log_reg
      eda_flx_log_dereg

******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME  :  eda_log

  DESCRIPTION     :  EDA logs

  ARGUMENT        :  id - Canned string id
                     category- Category of the log
                     sev - Severity of the log
                     rc - return code of the API or function
                     fname- filename
                     fno  - filenumber
                     dataa - Miscellineousdata
*****************************************************************************/

void eda_log(uint8_t id, uns32 category, uint8_t sev, uns32 rc, char *fname, uns32 fno, uns32 data)
{

	/* Log New type logs */
	ncs_logmsg(NCS_SERVICE_ID_EDA, EDA_LID_HDLN, EDA_FC_HDLN,
		   category, sev, NCSFL_TYPE_TCLILL, fname, fno, id, rc, data);

}	/* End of eda_log()  */

/*****************************************************************************

  PROCEDURE NAME  :  eda_log_f

  DESCRIPTION     :  EDA logs with the uns64 handle

  ARGUMENT        :  id - Canned string id
                     category- Category of the log
                     sev - Severity of the log
                     rc - return code of the API or function
                     fname- filename
                     fno  - filenumber
                     dataa - Miscellineousdata
                     handle - uns64 handle
*****************************************************************************/

void eda_log_f(uint8_t id, uns32 category, uint8_t sev, uns32 rc, char *fname, uns32 fno, uns32 dataa, uns64 handle)
{

	/* Log New type logs */
	ncs_logmsg(NCS_SERVICE_ID_EDA, EDA_LID_HDLNF, EDA_FC_HDLNF,
		   category, sev, NCSFL_TYPE_TCLILLF, fname, fno, id, rc, dataa, (double)handle);

}	/* End of eda_log_f()  */

/****************************************************************************
 * Name          : eda_flx_log_reg
 *
 * Description   : This is the function which registers the EDA logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void eda_flx_log_reg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_EDA;
	/* fill version no. */
	reg.info.bind_svc.version = EDSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "EDSv");

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : eda_flx_log_dereg
 *
 * Description   : This is the function which deregisters the EDA logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void eda_flx_log_dereg()
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_EDA;
	ncs_dtsv_su_req(&reg);
	return;
}
