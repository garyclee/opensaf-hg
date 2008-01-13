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


#include <stdio.h>
#include "ncsgl_defs.h"
#include "os_defs.h"
#include "ncs_main_papi.h"
#include "mbcsv_demo_app.h"
#include "ncssysfpool.h"


int main(int argc, char *argv[])
{
   MBCSV_FAKE_SS_CREATE_PARMS arg;

   if(argc < 2)
   {
      printf("\n USAGE: ./mbcsv_demo <1..2>\n");
      return 0;
   }
      
  /* initiliase the Environment */
  ncs_agents_startup(argc, argv);

  /* start the application */
  arg.role = atoi(argv[1]);
  arg.svc_id = 1;

  create_fake_ss(&arg);

  printf("\n Press any key to quit .................");

  getchar();
  return 0;
}

