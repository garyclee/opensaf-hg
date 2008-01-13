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

/*
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

  pdrbd_log_bind.......- Bind with DTSV.
  pdrbd_log_unbind.....- UnBind with DTSV.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "pdrbd.h"

#if (PDRBD_LOG == 1)

/*****************************************************************************

  PROCEDURE  : ncspdrbd_log_bind

  DESCRIPTION: Function is used for binding with PDRBD.

*****************************************************************************/

uns32 pdrbd_log_bind(void)
{
    NCS_DTSV_RQ        reg;

    reg.i_op = NCS_DTSV_OP_BIND;
    reg.info.bind_svc.svc_id = NCS_SERVICE_ID_PDRBD;
    /* fill version no. */
    reg.info.bind_svc.version = PDRBD_LOG_VERSION;
    /* fill svc_name */
    m_NCS_STRCPY(reg.info.bind_svc.svc_name, "PDRBD");

    if (ncs_dtsv_su_req(&reg) != NCSCC_RC_SUCCESS)
    {
       m_NCS_CONS_PRINTF("pdrbd_log_bind: NCS_SERVICE_ID_PDRBD bind failed");
       return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE  :ncspdrbd_log_unbind

  DESCRIPTION: Function is used for unbinding with PDRBD.

*****************************************************************************/

uns32 pdrbd_log_unbind(void)
{
    NCS_DTSV_RQ        dereg;

    dereg.i_op = NCS_DTSV_OP_UNBIND;
    dereg.info.unbind_svc.svc_id = NCS_SERVICE_ID_PDRBD;
    if (ncs_dtsv_su_req(&dereg) != NCSCC_RC_SUCCESS)
    {
       m_NCS_CONS_PRINTF("pdrbd_log_bind: Unbind failed for NCS_SERVICE_ID_PDRBD");
       return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}

#endif/* (PDRBD_LOG == 1) */





