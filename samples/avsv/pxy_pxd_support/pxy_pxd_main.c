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
 */

/*****************************************************************************
..............................................................................



..............................................................................

  DESCRIPTION:

  This file contains the main() of AvSv proxy for testing internode and 
  external application. It initializes 
  the basic infrastructure services & then triggers AvSv toolkit application.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/


#include <stdio.h>
#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncs_main_papi.h>

int main(int argc, char **argv)
{
   /* Initialize the environment */
   ncs_agents_startup(argc, argv);
   
   m_NCS_CONS_PRINTF("\n\n ############################################## \n");
   m_NCS_CONS_PRINTF(" #   Proxy for Internode and External Component!!! #\n");
   m_NCS_CONS_PRINTF(" ############################################## \n");

   /* Start the AMF thread */ 
   pxy_pxd_amf_init();

   /* Keep waiting forever */
   while (1)
      m_NCS_TASK_SLEEP(30000);
   
   return 0;    
}

