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

extern void glsv_test_sync_app_process(void *info);

int main(int argc, char *argv[])
{

   if (argc != 1)
   {
      printf("\nWrong Arguments USAGE: <lck_demo> \n");
      return 1;
   }

  /* start the application */ 
  glsv_test_sync_app_process(NULL);

  return 0;    
}
