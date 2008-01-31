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


#ifndef MQND_PROC_H
#define MQND_PROC_H

#define m_MQND_IS_SA_MSG_QUEUE_CREATE_SET(openFlags)     \
            ((openFlags & SA_MSG_QUEUE_CREATE) == SA_MSG_QUEUE_CREATE)



/* Error validation macros at MQND */
#define m_MQND_IS_Q_NOT_EXIST(errcode) (errcode == SA_AIS_ERR_NOT_EXIST)


/* NTOH & HTON macros for MQSV_MESSAGE structure */

#define m_MQND_NTOH_MQSV_MESSAGE(mqsv_message)  \
           { \
              SaInt64T  my_long_long; \
              mqsv_message->type = m_NCS_OS_NTOHL(mqsv_message->type); \
\
              my_long_long = m_NCS_OS_NTOHLL_P(&mqsv_message->info.msg.message_info.sendTime); \
              m_NCS_OS_MEMCPY(&mqsv_message->info.msg.message_info.sendTime, &my_long_long, 8); \
\
              mqsv_message->info.msg.message_info.sendReceive \
                            = m_NCS_OS_NTOHL(mqsv_message->info.msg.message_info.sendReceive); \
\
              if (mqsv_message->info.msg.message_info.sendReceive) {  \
                 my_long_long = m_NCS_OS_NTOHLL_P(&mqsv_message->info.msg.message_info.sender.sender_context.sender_dest); \
                 m_NCS_OS_MEMCPY(&mqsv_message->info.msg.message_info.sender.sender_context.sender_dest, &my_long_long, sizeof(MDS_DEST)); \
\
                 my_long_long = m_NCS_OS_NTOHLL_P(&mqsv_message->info.msg.message_info.sender.sender_context.reply_buffer_size); \
                 m_NCS_OS_MEMCPY(&mqsv_message->info.msg.message_info.sender.sender_context.reply_buffer_size, &my_long_long, 8); \
              } \
              else { \
                 my_long_long = m_NCS_OS_NTOHLL_P(&mqsv_message->info.msg.message_info.sender.senderId); \
                 m_NCS_OS_MEMCPY(&mqsv_message->info.msg.message_info.sender.senderId, &my_long_long, 8); \
              } \
\
              mqsv_message->info.msg.message.type = m_NCS_OS_NTOHL(mqsv_message->info.msg.message.type); \
              mqsv_message->info.msg.message.version = m_NCS_OS_NTOHL(mqsv_message->info.msg.message.version); \
\
              my_long_long = m_NCS_OS_NTOHLL_P(&mqsv_message->info.msg.message.size); \
              m_NCS_OS_MEMCPY(&mqsv_message->info.msg.message.size, &my_long_long, 8); \
\
              mqsv_message->info.msg.message.senderName.length \
                = m_NCS_OS_NTOHS(mqsv_message->info.msg.message.senderName.length); \
           } 


#define m_MQND_HTON_MQSV_MESSAGE(mqsv_message); \
        { \
           uns8 buff[8]; \
           MQSV_MESSAGE *mqsv_msg = (MQSV_MESSAGE *)mqsv_message; \
\
           mqsv_msg->type = m_NCS_OS_HTONL(mqsv_msg->type); \
\
           m_NCS_OS_HTONLL_P(buff, mqsv_msg->info.msg.message_info.sendTime); \
           m_NCS_OS_MEMCPY(&mqsv_msg->info.msg.message_info.sendTime, buff, 8); \
\
           if (mqsv_msg->info.msg.message_info.sendReceive) { \
              m_NCS_OS_HTONLL_P(buff, mqsv_msg->info.msg.message_info.sender.sender_context.sender_dest); \
              m_NCS_OS_MEMCPY(&mqsv_msg->info.msg.message_info.sender.sender_context.sender_dest, buff, sizeof(MDS_DEST)); \
\
              m_NCS_OS_HTONLL_P(buff, mqsv_msg->info.msg.message_info.sender.sender_context.reply_buffer_size); \
              m_NCS_OS_MEMCPY(&mqsv_msg->info.msg.message_info.sender.sender_context.reply_buffer_size, buff, 8); \
           } \
           else { \
              m_NCS_OS_HTONLL_P(buff, mqsv_msg->info.msg.message_info.sender.senderId); \
              m_NCS_OS_MEMCPY(&mqsv_msg->info.msg.message_info.sender.senderId, buff, 8); \
           } \
\
           mqsv_msg->info.msg.message_info.sendReceive \
                            = m_NCS_OS_HTONL(mqsv_msg->info.msg.message_info.sendReceive); \
\
           mqsv_msg->info.msg.message.type = m_NCS_OS_HTONL(mqsv_msg->info.msg.message.type); \
           mqsv_msg->info.msg.message.version = m_NCS_OS_HTONL(mqsv_msg->info.msg.message.version); \
\
           m_NCS_OS_HTONLL_P(buff, mqsv_msg->info.msg.message.size); \
           m_NCS_OS_MEMCPY(&mqsv_msg->info.msg.message.size, buff, 8); \
\
           mqsv_msg->info.msg.message.senderName.length \
                = m_NCS_OS_HTONS(mqsv_msg->info.msg.message.senderName.length); \
        }

/* Function Prototypes written in mqnd_util.c */
EXTERN_C NCS_BOOL mqnd_compare_create_attr(SaMsgQueueCreationAttributesT *open_ca,
                                  SaMsgQueueCreationAttributesT *curr_ca);

EXTERN_C uns32 mqnd_queue_create(MQND_CB *cb, MQP_OPEN_REQ *open, 
                                 MDS_DEST *rcvr_mqa, SaMsgQueueHandleT *qhdl, 
                                 MQP_TRANSFERQ_RSP *transfer_rspi, SaAisErrorT *err);

EXTERN_C uns32 mqnd_queue_reg_with_mqd(MQND_CB *cb, MQND_QUEUE_NODE *qnode, SaAisErrorT *err, NCS_BOOL is_q_reopen);

/* Function Prototypes written in mqnd_proc.c */

EXTERN_C uns32 mqnd_proc_queue_open (MQND_CB *cb, MQP_REQ_MSG *mqp_req, 
                               MQSV_SEND_INFO *sinfo,
                               ASAPi_NRESOLVE_RESP_INFO *qinfo);

EXTERN_C uns32 mqnd_proc_queue_close(MQND_CB *cb, 
                                     MQND_QUEUE_NODE *qnode, 
                                     SaAisErrorT *err);
EXTERN_C uns32 mqnd_send_mqp_ulink_rsp(MQND_CB *cb, MQSV_SEND_INFO *sinfo, 
                              SaAisErrorT err, MQP_UNLINK_REQ *ulink_req);

EXTERN_C uns32 mqnd_send_mqp_open_rsp(MQND_CB *cb, MQSV_SEND_INFO *sinfo, 
                             MQP_REQ_MSG *mqp_req, SaAisErrorT err, 
                             uns32 qhdl, uns32 existing_msg_count);

EXTERN_C uns32 mqnd_send_mqp_close_rsp(MQND_CB *cb, MQSV_SEND_INFO *sinfo, 
                              SaAisErrorT err, uns32 qhdl);
                              
EXTERN_C uns32 mqnd_evt_proc_mqp_qtransfer_response(MQND_CB *cb, MQSV_EVT *evt);

EXTERN_C uns32 mqnd_evt_proc_mqp_qtransfer_complete(MQND_CB *cb, MQSV_EVT *req);

EXTERN_C uns32 mqnd_evt_proc_mqp_qtransfer(MQND_CB *cb, MQSV_EVT *req);                              

EXTERN_C uns32 mqnd_fill_queue_from_transfered_buffer(MQND_CB *cb, MQND_QUEUE_NODE *qnode, MQP_TRANSFERQ_RSP *transfer_rsp);

#endif
