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



..............................................................................

  DESCRIPTION:This module has all the functionality that deals with
  the DTSv modules agent for logging debug messages.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avm_flx_log_reg - registers the AVM logging with DTA.
  avm_flx_log_dereg - unregisters the AVM logging with DTA.


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avm.h"
#if (NCS_AVM_LOG == 1)

/****************************************************************************
 * Name          : avm_flx_log_reg
 *
 * Description   : This is the function which registers the AvM logging with
 *                 the Flex Log agent.
 *                 
 *
 * Arguments     : None.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
avm_flx_log_reg ()
{
   NCS_DTSV_RQ            reg;

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                 = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_AVM;
   /* fill version no. */
   reg.info.bind_svc.version = AVM_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "AVM");

   ncs_dtsv_su_req(&reg);
   return;
}

/****************************************************************************
 * Name          : avm_flx_log_dereg
 *
 * Description   : This is the function which deregisters the AvM logging 
 *                 with the Flex Log agent.
 *                 
 *
 * Arguments     : None.
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void
avm_flx_log_dereg ()
{
   NCS_DTSV_RQ        reg;
   
   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_AVM;
   ncs_dtsv_su_req(&reg);
   return;
}

#endif /* (NCS_AVM_LOG == 1) */
