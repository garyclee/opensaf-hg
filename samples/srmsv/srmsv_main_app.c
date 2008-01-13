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
  DESCRIPTION:

  This file contains the main() of SRMSv toolkit application. It initializes 
  the basic infrastructure services & then triggers SRMSv toolkit application.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  
******************************************************************************
*/


#include <stdio.h>
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncs_main_papi.h"
#include "ncssysf_def.h"
#include "srmsv_demo.h"

int main()
{
   m_NCS_CONS_PRINTF("\n\n ********************************************** \n");
   m_NCS_CONS_PRINTF(" *                                            * \n");
   m_NCS_CONS_PRINTF(" *   You are about to witness SRMSv Demo !!!  * \n");
   m_NCS_CONS_PRINTF(" *   To start the demo, press any key         * \n");
   m_NCS_CONS_PRINTF(" *   To quit, press 'q'                       * \n");
   m_NCS_CONS_PRINTF(" *                                            * \n");
   m_NCS_CONS_PRINTF(" ********************************************** \n");

   /* Wait for the start trigger from the user */
   if ( 'q' == getchar() )
      return 0;
 
   /* Start the SRMSv toolkit application */ 
   ncs_srmsv_init();

   /* Check if it's time to exit */
   while ('q' != getchar());
   
   return 0;    
}


