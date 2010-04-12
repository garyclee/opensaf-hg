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

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

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
   unsigned int temp_var;

   if (argc != 2)
   {
      printf("\nWrong Arguments USAGE: <msg_demo><1(Sender)/0(Receiver)>\n");
      return -1;
   }

   temp_var = atoi(argv[1]);

   
   printf("\nSTARTING THE MQSv DEMO\n");
   printf("======================\n");

    /* start the application */ 
   if (temp_var == 1)
   {
      printf("MessageQ Sender Application \n");
      sleep(2);
      message_send_sync();
      printf("Press Enter Key to Continue...\n");
      getchar();
      message_send_async(); 
      printf("Press Enter Key to Continue...\n");
      getchar();
      message_send_receive();
      printf("Press Enter Key to Continue...\n");
      getchar();
      message_send_receive_a();
      printf("Press Enter Key to Continue...\n");
      getchar();
    }
    else
    {
       printf("MessageQ Receiver Application \n");
       message_rcv_sync();
       printf("Press Enter Key to Continue...\n");
       getchar();
       message_rcv_async(); 
       printf("Press Enter Key to Continue...\n");
       getchar();
       message_reply_sync();
       printf("Press Enter Key to Continue...\n");
       getchar();
       message_reply_async(); 
       printf("Press Enter Key to Continue...\n");
       getchar();

    }
    printf("End of MessageService Demo. Press Enter Key to Quit\n");
    getchar();
    return 0;
}
