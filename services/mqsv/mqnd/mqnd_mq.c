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

  DESCRIPTION: This file contains the Library functions for Native message 
               queue operations.
  
    This file inclused following routines:
    mqnd_mq_create
    mqnd_mq_open
    mqnd_mq_destroy
    mqnd_mq_msg_send
    mqnd_mq_msg_rcv

******************************************************************************/
#if(NCS_MQND == 1)
#include "mqnd.h"
#include "ncs_osprm.h"

uns32 gl_key = 1;

NCS_OS_MQ_MSG transfer_mq_msg; /* used in queue owner ship */

/****************************************************************************
 * Function Name: mqnd_mq_create
 * Purpose: Used to create the new physical message queue
 * Return Value:  NCSCC_RC_SUCCESS
 ****************************************************************************/
uns32 mqnd_mq_create(MQND_QUEUE_INFO *q_info)
{
    NCS_OS_POSIX_MQ_REQ_INFO  info;
    char                queue_name[SA_MAX_NAME_LENGTH];
    uns8 i;
    uns32 size=0;
    MQND_QUEUE_INFO   zero_q;
    uns32 rc = NCSCC_RC_SUCCESS;

    
    m_NCS_MEMSET(&info, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
    info.req = NCS_OS_POSIX_MQ_REQ_OPEN;
    memcpy(queue_name, q_info->queueName.value, 
                    q_info->queueName.length);
    queue_name[q_info->queueName.length] = '\0';
    info.info.open.qname = queue_name;
    info.info.open.node = m_NCS_NODE_ID_FROM_MDS_DEST(q_info->rcvr_mqa);
    info.info.open.iflags = O_CREAT;

    for (i=SA_MSG_MESSAGE_HIGHEST_PRIORITY; 
                             i <= SA_MSG_MESSAGE_LOWEST_PRIORITY; i++) 
    {
       size += q_info->size[i];
    }

    info.info.open.attr.mq_msgsize = size+MQSV_MSG_OVERHEAD; 

    /* Create a New message queue */
    if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
       return (NCSCC_RC_FAILURE);

    if(info.info.open.o_mqd == 0) 
    {
        /* MQSv particia can't handle zero Handle, get the new queue */
        zero_q = *q_info;
        zero_q.queueHandle = 0;
        if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
           rc = NCSCC_RC_FAILURE;
        mqnd_mq_destroy(&zero_q);
    }
    
    if (rc == NCSCC_RC_SUCCESS)
       q_info->queueHandle = info.info.open.o_mqd;

    return rc;
}

/****************************************************************************
 * Function Name: mqnd_mq_open
 * Purpose: Used to open the existing message queue
 * Return Value:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 ****************************************************************************/
uns32 mqnd_mq_open(MQND_QUEUE_INFO *q_info)
{
    NCS_OS_POSIX_MQ_REQ_INFO  info;
    char                queue_name[SA_MAX_NAME_LENGTH];


    m_NCS_MEMSET(&info, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
    info.req = NCS_OS_POSIX_MQ_REQ_OPEN;
    memcpy(queue_name, q_info->queueName.value, 
                    q_info->queueName.length);
    queue_name[q_info->queueName.length] = '\0';
    info.info.open.qname = queue_name;
    info.info.open.node = m_NCS_NODE_ID_FROM_MDS_DEST(q_info->rcvr_mqa);
    info.info.open.iflags = 0;

    /* Create a New message queue */
    if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
       return (NCSCC_RC_FAILURE);

    q_info->queueHandle = info.info.open.o_mqd;

    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Function Name: mqnd_mq_destroy
 * Purpose: Used to destroy the existing message queue
 * Return Value:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 ****************************************************************************/
uns32 mqnd_mq_destroy(MQND_QUEUE_INFO *q_info)
{
    NCS_OS_POSIX_MQ_REQ_INFO  info;

    m_NCS_MEMSET(&info, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
    info.req = NCS_OS_POSIX_MQ_REQ_CLOSE;
    info.info.close.mqd = q_info->queueHandle;

    if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
       return (NCSCC_RC_FAILURE);

    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Function Name: mqnd_mq_msg_send
 * Purpose: Used to Send the message to message queue
 * Return Value:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 ****************************************************************************/
uns32 mqnd_mq_msg_send(uns32 qhdl, MQSV_MESSAGE *mqsv_msg, uns32 size)
{
   NCS_OS_POSIX_MQ_REQ_INFO  info;
   NCS_OS_MQ_MSG       mq_msg;

   m_NCS_OS_MEMSET(&mq_msg, 0, sizeof(NCS_OS_MQ_MSG));
   memcpy(mq_msg.data, mqsv_msg, size);

   m_NCS_OS_MEMSET(&info, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
   info.req = NCS_OS_POSIX_MQ_REQ_MSG_SEND_ASYNC;
   info.info.send.mqd = qhdl;
   info.info.send.datalen = size;
   info.info.send.i_msg = &mq_msg;
   /* Priority 1 will be used for control messages like MQP_EVT_CANCEL_REQ 
      Priority 2 will be used for re-sending the messages that failed to deliver
      to applications in saMsgMessageGet. 
      Priority 3 to 6 will be used for SAF priorities 0 to 3 respectivelyp */

   info.info.send.i_mtype = mqsv_msg->info.msg.message.priority+3; 

   if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
      return (NCSCC_RC_FAILURE);

    

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Function Name: mqnd_mq_empty
 * Purpose: Used to empty the message queue
 * Return Value:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 ****************************************************************************/
uns32 mqnd_mq_empty(SaMsgQueueHandleT handle)
{
   NCS_OS_POSIX_MQ_REQ_INFO  mq_req;
   NCS_OS_MQ_MSG       mq_msg;
   uns32 num_messages;
   uns64 count;

   m_NCS_OS_MEMSET(&mq_msg, 0, sizeof(NCS_OS_MQ_MSG));
   m_NCS_OS_MEMSET(&mq_req, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
   
   mq_req.req = NCS_OS_POSIX_MQ_REQ_GET_ATTR;
   mq_req.info.attr.i_mqd = handle;
   if (m_NCS_OS_POSIX_MQ(&mq_req) != NCSCC_RC_SUCCESS)
   { 
       return NCSCC_RC_FAILURE;
   } 
   num_messages = mq_req.info.attr.o_attr.mq_curmsgs;

   mq_req.req = NCS_OS_POSIX_MQ_REQ_MSG_RECV_ASYNC;
   mq_req.info.recv.mqd = handle;

   /* TBD: When POSIX is ready, pass the timeout. right now 
    * wait indefinitely 
    */
   m_NCS_OS_MEMSET(&mq_req.info.recv.timeout, 
                   0, sizeof(NCS_OS_POSIX_TIMESPEC)); 

   /* Convert nano seconds to micro seconds. */
   mq_req.info.recv.timeout.tv_nsec = 0;
   mq_req.info.recv.timeout.tv_sec = 0;

   mq_req.info.recv.i_msg = &mq_msg;
   mq_req.info.recv.datalen = NCS_OS_MQ_MAX_PAYLOAD;
   mq_req.info.recv.dataprio = 0;
   mq_req.info.recv.i_mtype = -7;

   for (count=0; count<num_messages; count++)
       m_NCS_OS_POSIX_MQ(&mq_req);

   return NCSCC_RC_SUCCESS;
}
/****************************************************************************
 * Function Name: mqnd_mq_rcv
 * Purpose: Used to read from the message queue
 * Return Value:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 ****************************************************************************/
uns32 mqnd_mq_rcv(SaMsgQueueHandleT handle)
{
   NCS_OS_POSIX_MQ_REQ_INFO  mq_req;
   

   m_NCS_OS_MEMSET(&mq_req, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
   m_NCS_OS_MEMSET(&transfer_mq_msg, 0, sizeof(NCS_OS_MQ_MSG));

   mq_req.req = NCS_OS_POSIX_MQ_REQ_MSG_RECV;
   mq_req.info.recv.mqd = handle;

   /* TBD: When POSIX is ready, pass the timeout. right now 
    * wait indefinitely 
    */
   m_NCS_OS_MEMSET(&mq_req.info.recv.timeout, 
                   0, sizeof(NCS_OS_POSIX_TIMESPEC)); 

   /* Convert nano seconds to micro seconds. */
   mq_req.info.recv.timeout.tv_nsec = 0;
   mq_req.info.recv.timeout.tv_sec = 0;

   mq_req.info.recv.i_msg = &transfer_mq_msg;
   mq_req.info.recv.datalen = NCS_OS_MQ_MAX_PAYLOAD;
   mq_req.info.recv.dataprio = 0;
   mq_req.info.recv.i_mtype = -7; /* Read only the priorities brtween 1 and 6,
                                        with 1 as highest priority */

   if (m_NCS_OS_POSIX_MQ(&mq_req) != NCSCC_RC_SUCCESS)
   {
       return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Function Name: mqnd_listenerq_create
 * Purpose: Used to create the new listener queue
 * Return Value:  NCSCC_RC_SUCCESS
 ****************************************************************************/
uns32 mqnd_listenerq_create(MQND_QUEUE_INFO *q_info)
{
    NCS_OS_POSIX_MQ_REQ_INFO  info;
    char                queue_name[SA_MAX_NAME_LENGTH];
    MQND_QUEUE_INFO   zero_q;
    uns32 rc = NCSCC_RC_SUCCESS;

    m_NCS_MEMSET(&info, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
    info.req = NCS_OS_POSIX_MQ_REQ_OPEN;
    m_NCS_MEMSET(&queue_name, 0, SA_MAX_NAME_LENGTH);
    sprintf(queue_name, "NCS_MQSV%llu", q_info->queueHandle);
    info.info.open.qname = queue_name;
    info.info.open.node = m_NCS_NODE_ID_FROM_MDS_DEST(q_info->rcvr_mqa);
    info.info.open.iflags = O_CREAT;
    info.info.open.attr.mq_msgsize = MQND_LISTENER_QUEUE_SIZE; 

    /* Create a New message queue */
    if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
       return (NCSCC_RC_FAILURE);

    if(info.info.open.o_mqd == 0) {
        /* MQSv particia can't handle zero Handle, get the new queue */
        zero_q = *q_info;
        zero_q.listenerHandle = 0;
        if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
           rc = NCSCC_RC_FAILURE;
        mqnd_listenerq_destroy(&zero_q);
    }
    
    if (rc == NCSCC_RC_SUCCESS)
        q_info->listenerHandle = info.info.open.o_mqd;

    return rc;
}


/****************************************************************************
 * Function Name: mqnd_listenerq_destroy
 * Purpose: Used to destroy the existing listener queue
 * Return Value:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 ****************************************************************************/
uns32 mqnd_listenerq_destroy(MQND_QUEUE_INFO *q_info)
{
    NCS_OS_POSIX_MQ_REQ_INFO  info;
 
    /* No listener queue present, return */
    if (!q_info->listenerHandle)
       return NCSCC_RC_SUCCESS;

    m_NCS_MEMSET(&info, 0, sizeof(NCS_OS_POSIX_MQ_REQ_INFO));
    info.req = NCS_OS_POSIX_MQ_REQ_CLOSE;
    info.info.close.mqd = q_info->listenerHandle;

    if (m_NCS_OS_POSIX_MQ(&info) != NCSCC_RC_SUCCESS)
       return (NCSCC_RC_FAILURE);

    return NCSCC_RC_SUCCESS;
}



#endif /* (NCS_MQND == 1) */
