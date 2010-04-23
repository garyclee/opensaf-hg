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
      printf("\n INCORRECT ARGUMENTS:\n USAGE: <evt_demo>\n");
      return 1;
   }

   printf("\n\n ############################################## \n");
   printf(" #                                            # \n");
   printf(" #   You are about to witness EDSv Demo !!!   # \n");
   printf(" #   To start the demo, press <Enter> key     # \n");
   printf(" #                                            # \n");
   printf(" ############################################## \n");

   /* Wait for the start trigger from the user */
   if ( 'q' == getchar() )
      return 0;
 
   /* Start the AvSv toolkit application */ 
   ncs_edsv_run();

   printf("\n ### EDSv Demo over, To quit, press 'q' and <Enter> ### \n");

   /* Check if it's time to exit */
   while ( 'q' != getchar() );
   
   return 0;    
}

