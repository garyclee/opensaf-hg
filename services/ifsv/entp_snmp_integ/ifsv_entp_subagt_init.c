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
  MODULE NAME: IFSV_ENTP_SUBAGT_INIT.C 
******************************************************************************/


#include "ifsv_entp_subagt_init.h" 

#include  "ncs_ifsv_mib_agt_stub.h"



/**************************************************************************
Name          :  subagt_register_ifsv_entp_subsys

Description   :  This function is a common registration routine (application
                 specific) where it calls all the Module (MIBs defined for
                 an application) registration routines

Arguments     :  - NIL -

Return values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

Notes         :  Basically it registers all the modules (for all the TABLEs
                 defined the MIB) with SNMP agent
**************************************************************************/
uns32 subagt_register_ifsv_entp_subsys()
{
   uns32 status = NCSCC_RC_FAILURE;

   /* Register 'ncs_ifsv_mib' module (MIB) with SNMP Agent */
   status = ncs_snmpsubagt_init_deinit_msg_post("__register_ncs_ifsv_mib_module");
   if (status != NCSCC_RC_SUCCESS)
       return status;

   return NCSCC_RC_SUCCESS;
}


/**************************************************************************
Name          :  subagt_unregister_ifsv_entp_subsys

Description   :  This function is a common unregistration routine (application
                 specific) where it calls all the Module (MIBs defined for
                 an application) unregistration routines

Arguments     :  - NIL -

Return values :  Nothing

Notes         :  Basically it de-registers all the modules (for all the TABLEs
                 defined the MIB) from the SNMP agent
**************************************************************************/
uns32 subagt_unregister_ifsv_entp_subsys()
{
   uns32 status = NCSCC_RC_FAILURE;

   /* De-register 'ncs_ifsv_mib' module (MIB) from SNMP Agent */
   status = ncs_snmpsubagt_init_deinit_msg_post("__unregister_ncs_ifsv_mib_module");
   if (status != NCSCC_RC_SUCCESS)
       return status;

   return NCSCC_RC_SUCCESS;
}
  

