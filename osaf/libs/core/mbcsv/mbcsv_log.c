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

  mbcsv_log_bind.......- Bind with DTSV.
  mbcsv_log_unbind.....- UnBind with DTSV.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mbcsv.h"

#if (MBCSV_LOG == 1)

/*****************************************************************************

  PROCEDURE  : ncsmbcsv_log_bind

  DESCRIPTION: Function is used for binding with MBCSV.

*****************************************************************************/

uint32_t mbcsv_log_bind(void)
{
	NCS_DTSV_RQ reg;

	memset(&reg, '\0', sizeof(NCS_DTSV_RQ));

	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_MBCSV;
	reg.info.bind_svc.version = MBCSV_LOG_VERSION;
	strncpy(reg.info.bind_svc.svc_name, "MBCSv", sizeof(reg.info.bind_svc.svc_name));

	if (ncs_dtsv_su_req(&reg) != NCSCC_RC_SUCCESS)
		return m_MBCSV_DBG_SINK(NCSCC_RC_FAILURE, " mbcsv_log_bind: NCS_SERVICE_ID_MBCSV bind failed");
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE  :ncsmbcsv_log_unbind

  DESCRIPTION: Function is used for unbinding with MBCSV.

*****************************************************************************/

uint32_t mbcsv_log_unbind(void)
{
	NCS_DTSV_RQ dereg;

	dereg.i_op = NCS_DTSV_OP_UNBIND;
	dereg.info.unbind_svc.svc_id = NCS_SERVICE_ID_MBCSV;
	if (ncs_dtsv_su_req(&dereg) != NCSCC_RC_SUCCESS)
		return m_MBCSV_DBG_SINK(NCSCC_RC_FAILURE, "mbcsv_log_bind: Unbind failed for NCS_SERVICE_ID_MBCSV");
	return NCSCC_RC_SUCCESS;
}

#endif   /* (MBCSV_LOG == 1) */
