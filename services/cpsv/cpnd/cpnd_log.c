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



#include "cpnd.h"

/*****************************************************************************
  FILE NAME: CPND_LOG.C

  DESCRIPTION: CPND Logging utilities

  FUNCTIONS INCLUDED in this module:
      cpnd_flx_log_reg
      cpnd_flx_log_unreg


******************************************************************************/



/****************************************************************************
 * Name          : cpnd_flx_log_reg
 *
 * Description   : This is the function which registers the CPND logging with
 *                 the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void cpnd_flx_log_reg (void)
{
   NCS_DTSV_RQ            reg;

   memset(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_CPND;
   /* fill version no. */
   reg.info.bind_svc.version = CPSV_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "CPSv");
 
   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : cpnd_flx_log_dereg
 *
 * Description   : This is the function which deregisters the CPND logging 
 *                 with the Flex Log service.
 *                 
 *
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
cpnd_flx_log_dereg (void)
{
   NCS_DTSV_RQ        reg;
   
   memset(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_CPND;
   ncs_dtsv_su_req(&reg);
   return;
}

