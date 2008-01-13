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


#include "ncsgl_defs.h"
#include "os_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncs_main_papi.h"


#define MQSV_DEMO_MAIN_MAX_INPUT 9


void message_send_sync(void);
void message_send_async(void);
void message_send_receive(void);
void message_send_receive_a(void);
void message_rcv_sync(void);
void message_rcv_async(void);
void message_reply_sync(void);
void message_reply_async(void);

int main(int argc, char **argv)
{
   uns32 temp_var;

   if (argc != 2)
   {
      printf("\nWrong Arguments USAGE: <mqsv_demo><1(Sender)/0(Receiver)>\n");
      return (NCSCC_RC_FAILURE);
   }

   temp_var = atoi(argv[1]);

   
   m_NCS_CONS_PRINTF("\nSTARTING THE MQSv DEMO\n");
   m_NCS_CONS_PRINTF("======================\n");



/*   ncs_agents_startup(0,0);*/

    /* start the application */ 
   if (temp_var == 1)
   {
      sleep(5);
      message_send_sync();
      message_send_async(); 
      message_send_receive();
      message_send_receive_a();
    }
    else
    {
       sleep(3);
       message_rcv_sync();
       message_rcv_async(); 
       message_reply_sync();
       message_reply_async(); 

    }
    sleep(500);
    return 0;
}
