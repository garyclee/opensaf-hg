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
  MODULE NAME: SNMPTM_CLI_BGN.C

..............................................................................

  DESCRIPTION:   SNMPTM CLI file, contains the procedures to initialise CLI 
                 and register SNMPTM CLI commands.
******************************************************************************
*/
#include "snmptm.h"

/****************************************************************************
 Function Name:  snmptm_cli_main
 
 Purpose      :  This is the main function for SNMPTM CLI, It registers
                 SNMPTM cli command data and the correspondinf CEF info.

 Arguments    : 

 Return Value :  None

 Note         :  This demo only builds and works in INTERNAL TASKING mode.

****************************************************************************/
void  snmptm_cli_main(int argc, char **argv)
{
   NCS_LIB_REQ_INFO req;
   NCSCLI_BINDERY   info;

   /* initiliase the Environment */
   if (ncs_agents_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nSNMPTM-CLI ERR: Not able to initialize Agents\n"); 
      return;
   }

   m_NCS_OS_MEMSET(&req, 0, sizeof(req));
   m_NCS_OS_MEMSET(&info, 0, sizeof(info));

   /* Creating CLI instance */
   req.i_op = NCS_LIB_REQ_CREATE;
   cli_lib_req(&req);
   info.i_cli_hdl = gl_cli_hdl;   /* Global CLI handle */

   maclib_request(&req);
   info.i_mab_hdl = gl_mac_handle; /* Global MAC handle */

   /* Register the SNMPTM commands */
   m_NCS_CONS_PRINTF("\n\n SNMPTM CLI DEMO STARTED ...\n");

   info.i_req_fnc = ncsmac_mib_request; /* MAC request function */
   snmptm_cli_register(&info); 

   m_NCS_CONS_PRINTF(" Registered SNMPTM CLI Commnds  ...\n\n");   
   /* S l e e p      F o r e v e r */ 
   while(1)
      m_NCS_TASK_SLEEP(5000); 

   return;
}

