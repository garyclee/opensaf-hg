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


#include <stdio.h>
#include <opensaf/ncsgl_defs.h>
#include <opensaf/os_defs.h>
#include <opensaf/ncs_main_papi.h>

extern void glsv_test_sync_app1_process(void *info);

int main(int argc, char *argv[])
{

   if (argc != 1)
   {
      printf("\nWrong Arguments USAGE: <glsv_demo> \n");
      return (NCSCC_RC_FAILURE);
   }

  /* start the application */ 
  glsv_test_sync_app1_process(NULL);
  return 0;    
}
