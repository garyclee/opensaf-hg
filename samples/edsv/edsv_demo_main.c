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
  DESCRIPTION:

  This file contains the main() of EDSv toolkit application. It initializes 
  the basic infrastructure services & then triggers EDSv toolkit application.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "edsv_demo_app.h"

int main(int argc, char*argv[])
{

   if (argc != 1)
   {
      m_NCS_CONS_PRINTF("\n INCORRECT ARGUMENTS:\n USAGE: <edsv_demo>\n");
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_CONS_PRINTF("\n\n ############################################## \n");
   m_NCS_CONS_PRINTF(" #                                            # \n");
   m_NCS_CONS_PRINTF(" #   You are about to witness EDSv Demo !!!   # \n");
   m_NCS_CONS_PRINTF(" #   To start the demo, press any key         # \n");
   m_NCS_CONS_PRINTF(" #                                            # \n");
   m_NCS_CONS_PRINTF(" ############################################## \n");

   /* Wait for the start trigger from the user */
   if ( 'q' == getchar() )
      return 0;
 
   /* Start the AvSv toolkit application */ 
   ncs_edsv_run();

   m_NCS_CONS_PRINTF("\n ### EDSv Demo over, To quit, press 'q' and <Enter> ### \n");

   /* Check if it's time to exit */
   while ( 'q' != getchar() );
   
   return 0;    
}

