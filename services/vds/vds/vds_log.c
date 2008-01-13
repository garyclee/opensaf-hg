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

  DESCRIPTION: This file contains VDS logging utility routines.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "vds.h"

#if (NCS_VDS_LOG == 1)
/****************************************************************************
  Name          :  vds_log_reg
 
  Description   :  This routine registers AvA with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 vds_log_reg()
{
   NCS_DTSV_RQ reg;
   uns32       rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&reg, 0, sizeof(NCS_DTSV_RQ));

   reg.i_op = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_VDS;
   /* fill version no. */
   reg.info.bind_svc.version = VDS_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "VDS");

   rc = ncs_dtsv_su_req(&reg);

   return rc;
}


/****************************************************************************
  Name          :  vds_log_unreg
 
  Description   :  This routine unregisters AvA with flex log service.
                  
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
 *****************************************************************************/
uns32 vds_log_unreg ()
{
   NCS_DTSV_RQ reg;
   uns32       rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&reg, 0, sizeof(NCS_DTSV_RQ));

   reg.i_op = NCS_DTSV_OP_UNBIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_VDS;

   rc = ncs_dtsv_su_req(&reg);

   return rc;
}

#endif  




