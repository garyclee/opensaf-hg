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

#if (NCS_EDSV_LOG == 1)
#include "eds.h"

/*****************************************************************************
  FILE NAME: EDS_LOG.C

  DESCRIPTION: EDS Logging utilities

  FUNCTIONS INCLUDED in this module:
      eds_flx_log_reg
      eds_flx_log_dereg

******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME  :  eds_log

  DESCRIPTION     :  EDS logs

  ARGUMENT        :  id - Canned string id
                     category- Category of the log
                     sev - Severity of the log
                     rc - return code of the API or function
                     fname- filename
                     fno  - filenumber
                     data - Miscellineousdata
*****************************************************************************/

void eds_log(uint8_t id, uint32_t category, uint8_t sev, long rc, char *fname, uint32_t fno, uint32_t data)
{

	/* Log New type logs */
	ncs_logmsg(NCS_SERVICE_ID_EDS, EDS_LID_HDLN, EDS_FC_HDLN,
		   category, sev, NCSFL_TYPE_TCLILL, fname, fno, id, rc, data);

}	/* End of eds_log()  */

/*****************************************************************************

  PROCEDURE NAME  :  eds_log_f

  DESCRIPTION     :  EDS logs with the uint64_t MDSDEST

  ARGUMENT        :  id - Canned string id
                     category- Category of the log
                     sev - Severity of the log
                     rc - return code of the API or function
                     fname- filename
                     fno  - filenumber
                     dataa - Miscellineousdata
                     dest - uint64_t mdsdest
*****************************************************************************/

void eds_log_f(uint8_t id, uint32_t category, uint8_t sev, uint32_t rc, char *fname, uint32_t fno, uint32_t dataa, uint64_t dest)
{

	/* Log New type logs */
	ncs_logmsg(NCS_SERVICE_ID_EDS, EDS_LID_HDLNF, EDS_FC_HDLNF,
		   category, sev, NCSFL_TYPE_TCLILLF, fname, fno, id, rc, dataa, (double)dest);

}	/* End of eds_log_f()  */

/*****************************************************************************

  PROCEDURE NAME:    eds_log_event

  DESCRIPTION:       Log event header info.

*****************************************************************************/
void eds_log_event(uint8_t id, int8_t *pub_name, uint32_t evt_id, uint32_t pubtime, uint32_t pri, uint32_t rettime)
{
	ncs_logmsg(NCS_SERVICE_ID_EDS, EDS_LID_EVENT, EDS_FC_EVENT,
		   NCSFL_LC_EVENT, NCSFL_SEV_INFO, NCSFL_TYPE_TICLLLL, id, pub_name, evt_id, pubtime, pri, rettime);
}

/*****************************************************************************

  PROCEDURE NAME:    eds_log_lost_event

  DESCRIPTION:       Log event header info.

*****************************************************************************/
void eds_log_lost_event(uint8_t id, int8_t *pub_name, uint32_t evt_id, uint32_t pubtime, uint32_t pri)
{
	ncs_logmsg(NCS_SERVICE_ID_EDS, EDS_LID_EVENT, EDS_FC_EVENT,
		   NCSFL_LC_EVENT, NCSFL_SEV_INFO, NCSFL_TYPE_TICLLLL, id, pub_name, evt_id, pubtime, pri);

}

/****************************************************************************
 * Name          : eds_flx_log_reg
 *
 * Description   : This is the function which registers the EDS logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/

void eds_flx_log_reg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_EDS;
	/* fill version no. */
	reg.info.bind_svc.version = EDSV_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "EDSv");

	ncs_dtsv_su_req(&reg);
	return;
}

/****************************************************************************
 * Name          : eds_flx_log_dereg
 *
 * Description   : This is the function which deregisters the EDS logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void eds_flx_log_dereg(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(NCS_DTSV_RQ));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_EDS;
	ncs_dtsv_su_req(&reg);
	return;
}

#endif
