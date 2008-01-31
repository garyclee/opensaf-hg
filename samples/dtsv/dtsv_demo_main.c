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

#include "dtsv_dummy_ss.h"



/****************************************************************************
 * Name          : main
 *
 * Description   : This is the main function which gonna call the demo start
 *                 up function.
 *
 * Arguments     : argc - Number of arguments passed.
 *                 argv - argument strings.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
int main (int argc, char *argv[])
{

   /** initialize the leap and agent stuffs **/
   ncs_agents_startup(argc, argv);

   /* Start your application below this */

   m_NCS_TASK_SLEEP(2000);

   reg_dummy1_ss(); 
   m_NCS_TASK_SLEEP(500);
   printf("\n Starting dummy-1 application \n");
   log_messages1();
   de_reg_dummy1_ss();

   return NCSCC_RC_SUCCESS;
}

