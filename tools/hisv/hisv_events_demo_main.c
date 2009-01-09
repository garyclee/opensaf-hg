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
 *            Hewlett-Packard Company
 */

/*****************************************************************************
  DESCRIPTION:

  This file contains the main() of HISv events sample application.
  It initializes the basic infrastructure services & then triggers
  the HISv events sample application.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

int raw = 0;

#include "hisv_events_demo_app.h"

void err_usage() {
   m_NCS_CONS_PRINTF("\nINCORRECT ARGUMENTS:\n USAGE: hisv_events_demo\n");
   m_NCS_CONS_PRINTF(" or\n");
   m_NCS_CONS_PRINTF("        hisv_events_demo --raw\n");
}

int main(int argc, char*argv[])
{
   if (argc > 2) {
     err_usage();
     return (NCSCC_RC_FAILURE);
   }
   else {
      if (argc == 2) {
         if (strcmp(argv[1], "--raw") == 0) {
            raw = 1;
         }
         else {
            err_usage();
            return (NCSCC_RC_FAILURE);
         }
      }
   }

   m_NCS_CONS_PRINTF("\n\n ################################################### \n");
   m_NCS_CONS_PRINTF(" #                                                 # \n");
   m_NCS_CONS_PRINTF(" #   You are about to witness HISv Events Demo !!! # \n");
   m_NCS_CONS_PRINTF(" #   To start the demo, press any key              # \n");
   m_NCS_CONS_PRINTF(" #                                                 # \n");
   m_NCS_CONS_PRINTF(" ################################################### \n");

   /* Wait for the start trigger from the user */
   if ( 'q' == getchar() )
      return 0;
 
   /* Start the AvSv toolkit application */ 
   ncs_hisv_run();

   m_NCS_CONS_PRINTF("\n ### HISv Events Demo over, To quit, press 'q' and <Enter> ### \n");

   /* Check if it's time to exit */
   while ( 'q' != getchar() );
   
   return 0;    
}

