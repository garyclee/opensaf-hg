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
#include <stdlib.h>

extern void cpsv_test_sync_app_process(void *info);

int main(int argc, char *argv[])
{
   unsigned int temp_var;
  
   if (argc != 2)
   {
      printf("\nWrong Arguments USAGE: <ckpt_demo><1(Writer)/0(Reader)>\n");
      return -1;
   }

   temp_var = atoi(argv[1]);  
   
   /* initiliase the Environment */
/*   ncs_agents_startup(0, 0); */


  /* start the application */ 
  cpsv_test_sync_app_process((void *)(long)temp_var);
  return 0;    
}
