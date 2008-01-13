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

/*****************************************************************************
..............................................................................


..............................................................................

  DESCRIPTION:

  This file contains MQSV toolkit sample application. It demonstrates the
  following:
  a) usage of Message Service APIs.

******************************************************************************

*/

/* OpenSAF Include */

#include "ncsgl_defs.h"
#include "os_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"

/* SAF Include */

#include "saAis.h"
#include "saMsg.h"

#define APP_TIMEOUT 10000000000ll /* Timeout in nano seconds */
#define NUM_MESSAGES_SENT 5
#define MAX_MESSAGE_SIZE 1024


void message_send_sync(void);
void message_send_async(void);
void message_send_receive(void);
void message_send_receive_a(void);
void message_rcv_sync(void);
void message_rcv_async(void);
void message_reply_sync(void);
void message_reply_async(void);
void select_thread (NCSCONTEXT arg);

/* Demo Message buffers */

char msg0[64]={"This is demo message0"};
char msg1[128]={"This is demo message1"};
char msg2[256]={"This is demo message2"};
char msg3[512]={"This is demo message3"};
char msg4[1024]={"This is demo message4"};
SaNameT name = { 5, "Demo" };

SaMsgMessageT message[NUM_MESSAGES_SENT] = { {0, 1,21, &name, msg0, 0},
                                             {1, 1, 21,  &name, msg1,  1},
                                             {2, 1, 21,  &name, msg2,  2},
                                             {3, 1, 21,  &name, msg3, 3},
                                             {4, 1, 21, &name, msg4,  3}};

/****************************************************************************
  Name          : saMsgQueueOpenCallback

  Description   : This API is the open callback supplied to message service.

  Arguments     : SaInvocationT invocation
                  SaMsgQueueHandleT queueHandle
                  SaAisErrorT error

  Return Values : None

  Notes         : None

******************************************************************************/

static void saMsgQueueOpenCallback(SaInvocationT invocation,
                                       SaMsgQueueHandleT queueHandle,
                                       SaAisErrorT error)
{
   if(error == SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("Queue Open Callback Success - queueHandle %llu \n",queueHandle);
      m_NCS_CONS_PRINTF("Invocation - %llu\n", invocation);
   }
   else
      m_NCS_CONS_PRINTF("Queue Open Callback Failed. Invocation - %llu,  Error - %u \n",invocation, error); 
}

/****************************************************************************
  Name          : saMsgMessageDeliveredCallback

  Description   : This API is the message delivered callback supplied to message service. This is called in the sender 
                  application when a message is put in the  destination message queue.

  Arguments     : SaInvocationT invocation
                  SaAisErrorT error

  Return Values : None

  Notes         : None

******************************************************************************/

static void saMsgMessageDeliveredCallback(SaInvocationT invocation, SaAisErrorT error)
{
   if(error == SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF(" Message delivered Callback Success - invocation %llu  \n",invocation);
   }
   else
      m_NCS_CONS_PRINTF(" Message delivered Callback - invocation %llu,  error %u \n", invocation, error);
}

/****************************************************************************
  Name          : saMsgMessageReceivedCallback

  Description   : This API is the message delivered callback supplied to message service. This is called in the 
                  receiver when a message is put in the receiver's message queue.

  Arguments     : SaInvocationT invocation
                  SaAisErrorT error

  Return Values : None

  Notes         : None

******************************************************************************/

static void saMsgMessageReceivedCallback(SaMsgQueueHandleT queueHandle)
{
 char data[MAX_MESSAGE_SIZE];
 SaTimeT sendTime;
 SaMsgSenderIdT senderId;
 SaMsgMessageT receive_message;
 SaAisErrorT rc;
 int32 i;

 m_NCS_CONS_PRINTF(" \n\nMessage Received Callback  invoked with Queue Handle - %llu \n",queueHandle );

 memset(&receive_message, '\0', sizeof(receive_message));

 receive_message.data = data;

 receive_message.size = MAX_MESSAGE_SIZE;
 memset(&sendTime, '\0', sizeof(sendTime));
 memset(&senderId, '\0', sizeof(senderId));

 rc =  saMsgMessageGet(queueHandle, &receive_message, &sendTime, &senderId, APP_TIMEOUT);

 if (rc != SA_AIS_OK)
  {
   m_NCS_CONS_PRINTF("saMsgMessageReceivedGet failed with rc - %d\n", rc);
   return;
  }

 m_NCS_CONS_PRINTF("Received Message\n");
 m_NCS_CONS_PRINTF("-------------------\n");

 for(i=0;i<receive_message.size;i++)
   m_NCS_CONS_PRINTF("%c", (((char *)(receive_message.data))[i])); 

 m_NCS_CONS_PRINTF("\n\n");
}

/****************************************************************************
  Name          : message_send_sync

 Description    :This function demonstrates the use of saMsgMessageSend API to send a message 
                 to the queue synchronously.

  Arguments     : None

  Return Values : None

  Notes         : None

******************************************************************************/

void message_send_sync(void)
{
  SaMsgCallbacksT      callbk;
  SaMsgHandleT msgHandle;
  SaVersionT version;
  SaAisErrorT  rc;
  SaNameT       queueName;
  SaTimeT timeout=APP_TIMEOUT;

  sleep(5);

  callbk.saMsgQueueOpenCallback = saMsgQueueOpenCallback;
  callbk.saMsgMessageDeliveredCallback = saMsgMessageDeliveredCallback;
  callbk.saMsgMessageReceivedCallback = saMsgMessageReceivedCallback;
  callbk.saMsgQueueGroupTrackCallback = NULL;

  version.releaseCode= 'B';
  version.majorVersion = 1;
  version.minorVersion = 1;

  m_NCS_CONS_PRINTF("\nSCENARIO#1:Sending Messages via Sync API - saMsgMessageSend START\n\n");

  rc = saMsgInitialize(&msgHandle,&callbk,&version); 

  if ( rc != SA_AIS_OK ) 
   {
    m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
    return;
   }

  sprintf(queueName.value, "queue_demo1"); 

  queueName.length = strlen(queueName.value);

  m_NCS_CONS_PRINTF(" Sending Message0 using saMsgMessageSend()...");

  rc = saMsgMessageSend(msgHandle, &queueName, &message[0], timeout); 

  if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSend0 failed with rc - %d\n", rc);
     goto finalize;
   }

  m_NCS_CONS_PRINTF("success.\n"); 
  m_NCS_CONS_PRINTF(" Sending Message1 using saMsgMessageSend...");

  rc = saMsgMessageSend(msgHandle, &queueName, &message[1], timeout);

  if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSend1 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF(" Sending Message2 using saMsgMessageSend...");

   rc = saMsgMessageSend(msgHandle, &queueName, &message[2], timeout);

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSend2 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF(" Sending Message3 using saMsgMessageSend...");

   rc = saMsgMessageSend(msgHandle, &queueName, &message[3], timeout);

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSend3 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF(" Sending Message4 using saMsgMessageSend...");

   rc = saMsgMessageSend(msgHandle, &queueName, &message[4], timeout);

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSend4 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF("SCENARIO#1:Sending Messages via Sync API - saMsgMessageSend END\n");
   m_NCS_CONS_PRINTF(" \n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }
}

/****************************************************************************

  Name          : message_send_async

  Description   : This function demonstrates the use of saMsgMessageSendAsync 
                  API to send a message to the queue asynchronously. It also
                  demostrates that if the ackFlags and are set in the API, the 
                  selection object will be SET, if the application is 
                  waiting on select.

  Arguments     : None

  Return Values : None

  Notes         : None

******************************************************************************/

void message_send_async(void)
{
  SaMsgCallbacksT      callbk;
  SaInvocationT invocation=1234;
  SaMsgHandleT msgHandle;
  SaVersionT version;
  SaAisErrorT  rc;
  SaNameT       queueName;
  SaMsgAckFlagsT ackFlags;
  NCSCONTEXT thread_handle;

  sleep(5);

  callbk.saMsgQueueOpenCallback = saMsgQueueOpenCallback;
  callbk.saMsgMessageDeliveredCallback = saMsgMessageDeliveredCallback;
  callbk.saMsgMessageReceivedCallback = saMsgMessageReceivedCallback;
  callbk.saMsgQueueGroupTrackCallback = NULL;

  version.releaseCode= 'B';
  version.majorVersion = 1;
  version.minorVersion = 1;

  m_NCS_CONS_PRINTF("\nSCENARIO#2:Sending Messages via Async API - saMsgMessageSendAsync START\n\n");

  rc = saMsgInitialize(&msgHandle,&callbk,&version); 

  if ( rc != SA_AIS_OK ) 
   {
    m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
       return;
   }

  /* Create a thread and wait in select() */
  rc = m_NCS_TASK_CREATE((NCS_OS_CB)select_thread, (NCSCONTEXT)&msgHandle,
                          "mqa_sendasync_thread", 5, 8000, &thread_handle);

  if (rc != NCSCC_RC_SUCCESS)
  {
   m_NCS_CONS_PRINTF("failed to create thread\n");
   goto finalize;
  }

  rc = m_NCS_TASK_START(thread_handle);

  if (rc != NCSCC_RC_SUCCESS)
  {
   m_NCS_CONS_PRINTF("failed to start thread\n");
   goto finalize;
  }

   sprintf(queueName.value, "queue_demo2");
 
   queueName.length = strlen(queueName.value);
   ackFlags = 0;
   invocation=1;

   m_NCS_CONS_PRINTF(" Sending Message0 using saMsgMessageSendAsync...");

   rc = saMsgMessageSendAsync(msgHandle, invocation,  &queueName, 
                              &message[0], ackFlags); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSendAsync0 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 

   ackFlags = 0;

   m_NCS_CONS_PRINTF(" Sending Message1 using saMsgMessageSendAsync...");

   invocation=2;

   rc = saMsgMessageSendAsync(msgHandle, invocation,  &queueName, 
                              &message[1], ackFlags); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSendAsync1 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF(" Sending Message2 using saMsgMessageSendAsync with SA_MSG_MESSAGE_DELIVERED_ACK set and invocation=3...");

   ackFlags = SA_MSG_MESSAGE_DELIVERED_ACK;
   ackFlags = 0;
   invocation=3;

   rc = saMsgMessageSendAsync(msgHandle, invocation,  &queueName, 
                              &message[2], ackFlags); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSendAsync2 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 

   ackFlags = SA_MSG_MESSAGE_DELIVERED_ACK;
   ackFlags = 0;
   invocation=4;

   m_NCS_CONS_PRINTF(" Sending Message3 using saMsgMessageSendAsync with SA_MSG_MESSAGE_DELIVERED_ACK set and invocation=4...");

   rc = saMsgMessageSendAsync(msgHandle, invocation,  &queueName, 
                              &message[3], ackFlags); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSendAsync3 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 

   ackFlags = SA_MSG_MESSAGE_DELIVERED_ACK;
   ackFlags = 0;
   invocation=5;

   m_NCS_CONS_PRINTF(" Sending Message4 using saMsgMessageSendAsync with SA_MSG_MESSAGE_DELIVERED_ACK set and invocation=5..");
   rc = saMsgMessageSendAsync(msgHandle, invocation,  &queueName, 
                              &message[4], ackFlags); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSendAsync4 failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF("\nSCENARIO#2:Sending Messages via Async API - saMsgMessageSendAsync END\n");
   m_NCS_CONS_PRINTF(" \n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }

}

/****************************************************************************

  Name          : message_send_receive

  Description   : This function demonstrates the use of saMsgMessageSendReceive
                  in conjunction with saMsgMessageReply
                  API to send a message to the queue and reply to sender 
                  synchronously. 

  Arguments     : None

  Return Values : None

****************************************************************************/

void message_send_receive(void)
{

  SaMsgCallbacksT      callbk;
  SaMsgHandleT msgHandle;
  SaVersionT version;
  SaAisErrorT  rc;
  SaNameT       queueName;
  SaTimeT timeout=APP_TIMEOUT;
  SaMsgMessageT receive_msg;
  SaTimeT  receive_time;
  SaMsgAckFlagsT ackFlags;

  sleep(5);

  callbk.saMsgQueueOpenCallback = saMsgQueueOpenCallback;
  callbk.saMsgMessageDeliveredCallback = saMsgMessageDeliveredCallback;
  callbk.saMsgMessageReceivedCallback = saMsgMessageReceivedCallback;
  callbk.saMsgQueueGroupTrackCallback = NULL;

  version.releaseCode= 'B';
  version.majorVersion = 1;
  version.minorVersion = 1;

  m_NCS_CONS_PRINTF("\nSCENARIO#3:Sending & Receiving Messages via Sync API - saMsgMessageSendReceive START \n\n");

  rc = saMsgInitialize(&msgHandle,&callbk,&version); 

  if ( rc != SA_AIS_OK ) 
   {
    m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
      return;
   }

   sprintf(queueName.value, "queue_demo3"); 

   queueName.length = strlen(queueName.value);

   m_NCS_CONS_PRINTF(" Sending Message using saMsgMessageSendReceive()...");

   ackFlags = SA_MSG_MESSAGE_DELIVERED_ACK;
   receive_msg.senderName = &name;
   receive_msg.data =NULL;

   rc = saMsgMessageSendReceive(msgHandle, &queueName, &message[4], 
                                &receive_msg, &receive_time, timeout); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSendReceive failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF("\nSCENARIO#3:Sending & Receiving Messages via Sync API - saMsgMessageSendReceive END\n");
   m_NCS_CONS_PRINTF(" \n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }
}


/****************************************************************************

  Name          : message_send_receive_a

  Description   : This function demonstrates the use of saMsgMessageSendReceive
                  in conjunction with saMsgMessageReplyAsync  
                  API to send a message to the queue and reply to sender 
                  asynchronously. 

  Arguments     : None

  Return Values : None

****************************************************************************/

void message_send_receive_a(void)
{
  SaMsgCallbacksT      callbk;
  SaMsgHandleT msgHandle;
  SaVersionT version;
  SaAisErrorT  rc;
  SaNameT       queueName;
  SaTimeT timeout=APP_TIMEOUT;
  SaMsgMessageT receive_msg;
  SaTimeT  receive_time;
  SaMsgAckFlagsT ackFlags;

  sleep(5);

  callbk.saMsgQueueOpenCallback = saMsgQueueOpenCallback;
  callbk.saMsgMessageDeliveredCallback = saMsgMessageDeliveredCallback;
  callbk.saMsgMessageReceivedCallback = saMsgMessageReceivedCallback;
  callbk.saMsgQueueGroupTrackCallback = NULL;

  version.releaseCode= 'B';
  version.majorVersion = 1;
  version.minorVersion = 1;
  
  m_NCS_CONS_PRINTF("\nSCENARIO#4:Sending & Receiving Messages via Async API - saMsgMessageSendReceive \n");

  rc = saMsgInitialize(&msgHandle,&callbk,&version); 
  if ( rc != SA_AIS_OK ) 
   {
    m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
      return;
   }

   sprintf(queueName.value, "queue_demo4"); 

   queueName.length = strlen(queueName.value);
   ackFlags = SA_MSG_MESSAGE_DELIVERED_ACK;

   m_NCS_CONS_PRINTF(" Sending Message using saMsgMessageSendReceive()...");

   receive_msg.senderName = &name;
   receive_msg.data =NULL;

   rc = saMsgMessageSendReceive(msgHandle, &queueName, &message[4], 
                                &receive_msg, &receive_time, timeout); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageSendReceive failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("success.\n"); 
   m_NCS_CONS_PRINTF("\nSCENARIO#4:Sending & Receiving Messages via Async API - saMsgMessageSendReceive END \n");
   m_NCS_CONS_PRINTF(" \n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }
}

/****************************************************************************

  Name          : message_rcv_sync

  Description   : This function demonstrates the use of saMsgMessageGet 
                  API to receive messages synchronously.

  Arguments     : None

  Return Values : None

  Notes         : None

******************************************************************************/

void message_rcv_sync(void) 
{
 SaMsgHandleT msgHandle;
 SaMsgQueueHandleT queue_handle;
 SaMsgMessageT receive_message;
 SaTimeT sendTime;
 SaMsgSenderIdT senderId;
 SaVersionT version;
 SaAisErrorT  rc;
 SaNameT       queueName;
 SaMsgQueueCreationAttributesT creation_attributes;
 SaMsgQueueOpenFlagsT open_flags;
 SaTimeT timeout = APP_TIMEOUT;
 char data[MAX_MESSAGE_SIZE];
 int32 i,j;

 version.releaseCode= 'B';
 version.majorVersion = 1;
 version.minorVersion = 1;

 m_NCS_CONS_PRINTF("\nSCENARIO#1: Receiving messages via Sync API - saMsgMessageGet START\n\n");

 rc = saMsgInitialize(&msgHandle,NULL,&version); 

 if ( rc != SA_AIS_OK ) 
   {
    m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
    return;
   }

 creation_attributes.creationFlags = 0;
 creation_attributes.size[0] = 512;
 creation_attributes.size[1] = 512;
 creation_attributes.size[2] = 512;
 creation_attributes.size[3] = 512;
 creation_attributes.retentionTime = 10000000000ll;

 /* Receiver creates the message queue */
 open_flags = SA_MSG_QUEUE_CREATE; 

 sprintf(queueName.value, "queue_demo1"); 

 queueName.length = strlen(queueName.value);

 rc = saMsgQueueOpen(msgHandle, &queueName, &creation_attributes, 
                      open_flags, timeout, &queue_handle);

 if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgQueueOpen failed with rc - %d\n", rc);
     goto finalize;
   }

 m_NCS_CONS_PRINTF("Opening message queue with params: Queue Name - %s,creation flags %lu, size - %llu, retention time - %lld\n",queueName.value, creation_attributes.creationFlags, creation_attributes.size[0], creation_attributes.retentionTime);

 for (i=0;i< NUM_MESSAGES_SENT; i++)
   {
    memset(&receive_message, '\0', sizeof(receive_message));
    receive_message.data = data;
    receive_message.size = message[i].size ;

    m_NCS_CONS_PRINTF("Reading the message using saMsgMessageGet()\n");

    memset(&sendTime, '\0', sizeof(sendTime));
    memset(&senderId, '\0', sizeof(senderId));

    rc =  saMsgMessageGet(queue_handle, &receive_message, &sendTime, &senderId, APP_TIMEOUT);

    if (rc != SA_AIS_OK)
     {
      m_NCS_CONS_PRINTF("saMsgMessageGet failed with rc - %d\n", rc);
      goto api_error;
     }

    m_NCS_CONS_PRINTF("\nReceived message%d\n", i);
    m_NCS_CONS_PRINTF("-------------------\n");

    for (j=0; j<receive_message.size;j++)
      {
       m_NCS_CONS_PRINTF("%c", (((char *)(receive_message.data))[j])); 
      }
            
    m_NCS_CONS_PRINTF("\nmessage%d info \n",i);
    m_NCS_CONS_PRINTF("------------- \n");
    m_NCS_CONS_PRINTF("send time - %lld\n", sendTime);
    m_NCS_CONS_PRINTF("sender id - %llu\n", senderId);
   }

 rc = saMsgQueueClose(queue_handle);

 if(rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("saMsgQueueClose failed with rc - %d\n", rc);
    goto api_error;
   }

api_error:
 rc = saMsgQueueUnlink(msgHandle, &queueName); 

 if(rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("saMsgQueueUnlink failed with rc - %d\n", rc);
    goto finalize;
   }

 m_NCS_CONS_PRINTF("\nSCENARIO#1: Receiving messages via Sync API - saMsgMessageGet END\n");
 m_NCS_CONS_PRINTF(" \n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }
}

/****************************************************************************

  Name          : message_rcv_async

  Description   : This function demonstrates the use of saMsgMessageGet AND
                  saMsgSelectionObjectGet AND select API to receive messages
                  asynchronously.

  Arguments     : None

  Return Values : None

  Notes         : None

******************************************************************************/

void message_rcv_async(void)
{
  SaMsgCallbacksT      callbk;
  SaMsgHandleT msgHandle;
  SaMsgQueueHandleT queue_handle;
  SaVersionT version;
  SaAisErrorT  rc;
  int32         sel_rc;
  SaNameT       queueName;
  SaMsgQueueCreationAttributesT creation_attributes;
  SaMsgQueueOpenFlagsT open_flags;
  SaTimeT timeout=APP_TIMEOUT;
  SaSelectionObjectT       selection_object;
  NCS_SEL_OBJ_SET io_readfds;
  NCS_SEL_OBJ sel_obj;
  int32 num_messages;

  callbk.saMsgQueueOpenCallback = saMsgQueueOpenCallback;
  callbk.saMsgMessageDeliveredCallback = saMsgMessageDeliveredCallback;
  callbk.saMsgMessageReceivedCallback = saMsgMessageReceivedCallback;
  callbk.saMsgQueueGroupTrackCallback = NULL;

  version.releaseCode= 'B';
  version.majorVersion = 1;
  version.minorVersion = 1;

  m_NCS_CONS_PRINTF("\nSCENARIO#2: Receiving messages via Async API - m_NCS_OS_SEL_OBJ_SELECT and saMsgDispatch START\n\n");

  rc = saMsgInitialize(&msgHandle,&callbk,&version); 

  if ( rc != SA_AIS_OK ) 
   {
     m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
     return;
   }

  sprintf(queueName.value, "queue_demo2"); 

  queueName.length = strlen(queueName.value);

  creation_attributes.creationFlags = 0;
  creation_attributes.size[0] = 512;
  creation_attributes.size[1] = 512;
  creation_attributes.size[2] = 512;
  creation_attributes.size[3] = 512;
  creation_attributes.retentionTime = 100000000000ll;

  open_flags = SA_MSG_QUEUE_RECEIVE_CALLBACK| SA_MSG_QUEUE_CREATE; 

  m_NCS_CONS_PRINTF("Open Flags - SA_MSG_QUEUE_SELECTION_OBJECT_SET\n");

  rc = saMsgQueueOpen(msgHandle,  &queueName, &creation_attributes, open_flags, timeout, &queue_handle);

  if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("saMsgQueueOpen failed with rc - %d\n", rc);
    goto finalize;
   }

   m_NCS_CONS_PRINTF("Opening message queue with params: Queue Name - %s,creation flags %lu, size - %llu, retention time - %lld\n",queueName.value, creation_attributes.creationFlags, creation_attributes.size[0], creation_attributes.retentionTime);

   rc = saMsgSelectionObjectGet(msgHandle, &selection_object);

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("Error Getting Selection Object - %d", rc);
     goto api_error;
   }

   m_SET_FD_IN_SEL_OBJ(selection_object, sel_obj);
   m_NCS_SEL_OBJ_ZERO(&io_readfds);
   m_NCS_SEL_OBJ_SET(sel_obj, &io_readfds);

   for (num_messages=0;num_messages <5;num_messages++)
   {
       m_NCS_CONS_PRINTF("\nWaiting on Select...\n");

       sel_rc = m_NCS_SEL_OBJ_SELECT(sel_obj, &io_readfds, NULL, NULL, NULL);

       if (sel_rc < 1)
       {
         m_NCS_CONS_PRINTF("Select failed\n");
         goto api_error;
       }

       if (m_NCS_SEL_OBJ_ISSET(sel_obj, &io_readfds) )
       {
            m_NCS_CONS_PRINTF("Dispatching the notification using saMsgDispatch\n");

            rc = saMsgDispatch(msgHandle, SA_DISPATCH_ONE);

            if (rc != SA_AIS_OK)
                m_NCS_CONS_PRINTF("Dispatch failed\n");

       }
   }

   sleep(2); 

api_error:
   rc = saMsgQueueUnlink(msgHandle, &queueName);  

   if (rc != SA_AIS_OK)
     {
      m_NCS_CONS_PRINTF("saMsgQueueUnlink failed with rc - %d\n", rc);
      goto finalize;
     }

   m_NCS_CONS_PRINTF("\nSCENARIO#2: Receiving messages via Async API - m_NCS_OS_SEL_OBJ_SELECT and saMsgDispatch END\n");
   m_NCS_CONS_PRINTF(" \n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }
}


/****************************************************************************

  Name          : message_reply_sync

  Description   : This function demonstrates the use of saMsgMessageReply
                  API to reply to sender of the message synchronously. 

  Arguments     : None

  Return Values : None

  Notes         : None

******************************************************************************/

void message_reply_sync(void) 
{
  SaMsgCallbacksT      callbk;
  SaMsgHandleT msgHandle;
  SaMsgQueueHandleT queue_handle;
  SaMsgMessageT receive_message;
  SaTimeT sendTime;
  SaMsgSenderIdT senderId;
  SaVersionT version;
  SaAisErrorT  rc;
  SaNameT       queueName;
  SaMsgQueueCreationAttributesT creation_attributes;
  SaMsgQueueOpenFlagsT open_flags;
  SaTimeT timeout=APP_TIMEOUT;
  char data[1024];
  SaMsgAckFlagsT ackFlags;
  uns32 j;

  callbk.saMsgQueueOpenCallback = saMsgQueueOpenCallback;
  callbk.saMsgMessageDeliveredCallback = saMsgMessageDeliveredCallback;
  callbk.saMsgMessageReceivedCallback = saMsgMessageReceivedCallback;
  callbk.saMsgQueueGroupTrackCallback = NULL;

  version.releaseCode= 'B';
  version.majorVersion = 1;
  version.minorVersion = 1;

  m_NCS_CONS_PRINTF("\nSCENARIO#3:Replying Messages via Sync API - saMsgMessageReply START \n\n");

  rc = saMsgInitialize(&msgHandle,&callbk,&version); 

  if ( rc != SA_AIS_OK ) 
   {
    m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
      return;
   }

  sprintf(queueName.value, "queue_demo3"); 

  queueName.length = strlen(queueName.value);

  creation_attributes.creationFlags = 0;
  creation_attributes.size[0] = 512;
  creation_attributes.size[1] = 512;
  creation_attributes.size[2] = 512;
  creation_attributes.size[3] = 512;
  creation_attributes.retentionTime = 10000000000ll;

  open_flags = SA_MSG_QUEUE_CREATE; 

  rc = saMsgQueueOpen(msgHandle, &queueName, &creation_attributes, open_flags, timeout, &queue_handle);

  if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgQueueOpen failed with rc - %d\n",  rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("Opening message queue with params: Queue Name - %s,creation flags %lu, size - %llu, retention time - %lld\n",queueName.value, creation_attributes.creationFlags, creation_attributes.size[0], creation_attributes.retentionTime);

   memset(&receive_message, '\0', sizeof(receive_message));
   receive_message.data = data;
   receive_message.size = 1024;

   memset(&sendTime, '\0', sizeof(sendTime));
   memset(&senderId, '\0', sizeof(senderId));

   rc =  saMsgMessageGet(queue_handle, &receive_message, &sendTime, &senderId, APP_TIMEOUT);

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgMessageGet failed with rc - %d\n", rc);
     goto api_error;
   }

   m_NCS_CONS_PRINTF("\nReceived message\n");

   m_NCS_CONS_PRINTF("-------------------\n");

   for (j=0; j<receive_message.size;j++)
      {
       m_NCS_CONS_PRINTF("%c", (((char *)(receive_message.data))[j])); 
      }

   m_NCS_CONS_PRINTF("\nmessage info \n");
   m_NCS_CONS_PRINTF("------------- \n");
   m_NCS_CONS_PRINTF("send time - %lld\n", sendTime);
   m_NCS_CONS_PRINTF("sender id - %llu\n", senderId);

   ackFlags = SA_MSG_MESSAGE_DELIVERED_ACK;

   /* In B.1.1 the struct SaMsgMessageInfoT is obsolate */
   if (senderId)
   {
    m_NCS_CONS_PRINTF("Received a  message with sendReceive Flags set to TRUE\n");
    m_NCS_CONS_PRINTF("Replying to the sender using saMsgMessageReply\n");

    rc = saMsgMessageReply(msgHandle, &receive_message, &senderId, timeout);

    if (rc != SA_AIS_OK)
       {
        m_NCS_CONS_PRINTF("saMsgMessageReply failed with rc - %d\n", rc);
        goto api_error;
       }

    m_NCS_CONS_PRINTF("saMsgMessageReply success\n");
   }

   else
     m_NCS_CONS_PRINTF("sendReceive Flag not set to SA_TRUE in the received message. No Reply sent\n");

api_error:
   rc = saMsgQueueUnlink(msgHandle, &queueName); 

   if (rc != SA_AIS_OK)
    {
     m_NCS_CONS_PRINTF("saMsgQueueUnlink failed with rc - %d\n", rc);
     goto finalize;
    }

   m_NCS_CONS_PRINTF("\nSCENARIO#3:Replying Messages via Sync API - saMsgMessageReply END \n");
   m_NCS_CONS_PRINTF("\n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }
}

/****************************************************************************

  Name          : message_reply_async

  Description   : This function demonstrates the use of saMsgMessageReplyAsync 
                  API to reply to sender of the message asynchronously. It also
                  demostrates that if the ackFlags and are set in the API, the 
                  selection object will be SET, if the application is 
                  waiting on select.

  Arguments     : None

  Return Values : None

  Notes         : None

******************************************************************************/

void message_reply_async(void) 
{
  SaMsgCallbacksT      callbk;
  SaMsgHandleT msgHandle;
  SaMsgQueueHandleT queue_handle;
  SaMsgMessageT receive_message;
  SaTimeT sendTime;
  SaMsgSenderIdT senderId;
  SaVersionT version;
  SaAisErrorT  rc;
  SaNameT       queueName;
  SaMsgQueueCreationAttributesT creation_attributes;
  SaMsgQueueOpenFlagsT open_flags;
  SaTimeT timeout=APP_TIMEOUT;
  char data[1024];
  SaMsgAckFlagsT ackFlags;
  uns32 j;
  SaInvocationT invocation=10;

  callbk.saMsgQueueOpenCallback = saMsgQueueOpenCallback;
  callbk.saMsgMessageDeliveredCallback = saMsgMessageDeliveredCallback;
  callbk.saMsgMessageReceivedCallback = saMsgMessageReceivedCallback;
  callbk.saMsgQueueGroupTrackCallback = NULL;

  version.releaseCode= 'B';
  version.majorVersion = 1;
  version.minorVersion = 1;

  m_NCS_CONS_PRINTF("\nSCENARIO#4:Replying Messages via Async API - saMsgMessageReplyAsync START \n\n");

  rc = saMsgInitialize(&msgHandle,&callbk,&version); 

  if ( rc != SA_AIS_OK ) 
   {
    m_NCS_CONS_PRINTF(" Error Initialising saMsgInitialize with error - %d", rc);
    return;
   }

  sprintf(queueName.value, "queue_demo4"); 

  queueName.length = strlen(queueName.value);

  creation_attributes.creationFlags = 0;
  creation_attributes.size[0] = 512;
  creation_attributes.size[1] = 512;
  creation_attributes.size[2] = 512;
  creation_attributes.size[3] = 512;
  creation_attributes.retentionTime = 10000000000ll;

  open_flags = SA_MSG_QUEUE_RECEIVE_CALLBACK| SA_MSG_QUEUE_CREATE; 

  rc = saMsgQueueOpen(msgHandle, &queueName, &creation_attributes, open_flags, timeout, &queue_handle);

  if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("saMsgQueueOpen failed with rc - %d\n", rc);
    goto finalize;
   }

  m_NCS_CONS_PRINTF("Opening message queue with params: Queue Name - %s,creation flags %lu, size - %llu, retention time - %lld\n",queueName.value, creation_attributes.creationFlags, creation_attributes.size[0], creation_attributes.retentionTime);

  memset(&receive_message, '\0', sizeof(receive_message));

  receive_message.data = data;
  receive_message.size = 1024;

  memset(&sendTime, '\0', sizeof(sendTime));
  memset(&senderId, '\0', sizeof(senderId));

  rc =  saMsgMessageGet(queue_handle, &receive_message, &sendTime, &senderId, APP_TIMEOUT);

  if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("saMsgMessageGet failed with rc - %d\n", rc);
    goto api_error;
   }

  m_NCS_CONS_PRINTF("\nReceived message\n");
  m_NCS_CONS_PRINTF("-------------------\n");

  for(j=0; j<receive_message.size;j++)
     {
      m_NCS_CONS_PRINTF("%c", (((char *)(receive_message.data))[j])); 
     }

  m_NCS_CONS_PRINTF("\nmessage info \n");
  m_NCS_CONS_PRINTF("------------- \n");
  m_NCS_CONS_PRINTF("send time - %lld\n", sendTime);
  m_NCS_CONS_PRINTF("sender id - %llu\n", senderId);

  ackFlags = SA_MSG_MESSAGE_DELIVERED_ACK;

  /* SaMsgMessageInfoT is deleted in B-Spec */
  if (senderId)
   {
     m_NCS_CONS_PRINTF("Received a  message with sendReceive Flags set to TRUE\n");
     m_NCS_CONS_PRINTF("Replying to the sender using saMsgMessageReplyAsync\n");

     rc = saMsgMessageReplyAsync(msgHandle, invocation, &receive_message, &senderId, ackFlags);

     if (rc != SA_AIS_OK)
      {
        m_NCS_CONS_PRINTF("saMsgMessageReplyAsync failed with rc - %d\n", rc);
        goto api_error;
      }

      m_NCS_CONS_PRINTF("saMsgMessageReplyAsync success\n");
   }
   else
     m_NCS_CONS_PRINTF("sendReceive Flag not set to SA_TRUE in the received message. No Reply sent\n");

   sleep(100);

   rc = saMsgQueueClose(queue_handle);

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgQueueClose failed with rc - %d\n", rc);
     goto api_error;
   }

api_error:
   rc = saMsgQueueUnlink(msgHandle, &queueName); 

   if (rc != SA_AIS_OK)
   {
     m_NCS_CONS_PRINTF("saMsgQueueUnlink failed with rc - %d\n", rc);
     goto finalize;
   }

   m_NCS_CONS_PRINTF("\nSCENARIO#4:Replying Messages via Async API - saMsgMessageReplyAsync END \n");
   m_NCS_CONS_PRINTF("\n\n\n");

finalize:
   rc = saMsgFinalize(msgHandle);

   if (rc != SA_AIS_OK)
   {
    m_NCS_CONS_PRINTF("Error in Finalize: saMsgFinalize failed with rc - %d\n", rc);
    return;
   }
}

void select_thread (NCSCONTEXT arg)
{
   SaSelectionObjectT       selection_object;
   SaMsgHandleT  msgHandle = *(SaMsgHandleT *)arg;
   NCS_SEL_OBJ_SET io_readfds;
   NCS_SEL_OBJ sel_obj;
   SaAisErrorT rc;
   int32 sel_rc;

   rc = saMsgSelectionObjectGet(msgHandle, &selection_object);

   if (rc != SA_AIS_OK)
   {
       return;
   }

   m_SET_FD_IN_SEL_OBJ(selection_object, sel_obj);

   m_NCS_SEL_OBJ_ZERO(&io_readfds);

   m_NCS_SEL_OBJ_SET(sel_obj, &io_readfds);

   while(1)
   {
      sel_rc = m_NCS_SEL_OBJ_SELECT(sel_obj, &io_readfds, NULL, NULL, NULL);

      if (sel_rc != NCSCC_RC_SUCCESS)
      {
          return;
      }

      if (m_NCS_SEL_OBJ_ISSET(sel_obj, &io_readfds) )
      {
          saMsgDispatch(msgHandle, SA_DISPATCH_ONE);
      }
   }
}

