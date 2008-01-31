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


/****************************************************************************
 * POSIX Message-queues Primitive definition
 * The actual function ncs_os_posix_mq must be resolved in the os_defs.h file.
 *
 * Macro arguments
 *  'req'        is an NCS_OS_POSIX_MQ_REQ_INFO structure.
 *
 * Macro return codes
 * The ncs_os_posix_mq implemention must return one of the following codes:
 *   NCSCC_RC_SUCCESS - interface call successful (normal return code)
 *   NCSCC_RC_FAILURE - interface call failed.
 *
 ***************************************************************************/

#if 0
#ifndef __POSIX_H__
#define __POSIX_H__

typedef enum
{
   NCS_OS_POSIX_MQ_REQ_MIN=1,
   NCS_OS_POSIX_MQ_REQ_OPEN,         /* Strictly open i.e. do not create */
   NCS_OS_POSIX_MQ_REQ_CLOSE,
   NCS_OS_POSIX_MQ_REQ_UNLINK,
   NCS_OS_POSIX_MQ_REQ_MSG_SEND,     
   NCS_OS_POSIX_MQ_REQ_MSG_RECV,     /* Blocking recv call */
   NCS_OS_POSIX_MQ_REQ_MAX
}NCS_OS_POSIX_MQ_REQ_TYPE;

#ifndef NCS_OS_POSIX_MQ_MAX_PAYLOAD
#define NCS_OS_POSIX_MQ_MAX_PAYLOAD 100  /* Dummy definition */
#endif

#ifdef MQSV_USE_POSIX
#define NCS_OS_POSIX_MQD  mqd_t
#else
#define NCS_OS_POSIX_MQD  uns32
#endif


typedef struct ncs_os_posix_mq_msg
{
   uns32                  datalen;
   uns32                  dataprio;
   uns8                   *data;
}NCS_OS_POSIX_MQ_MSG;


typedef struct ncs_os_posix_mq_attr
{
   uns32                  datalen;
   uns32                  dataprio;
   uns8                   *data;
}NCS_OS_POSIX_MQ_ATTR;

typedef struct ncs_os_posix_mq_timeout
{
    struct timespec      timeout;
}NCS_OS_POSIX_MQ_TIMEOUT;
/*-----------------------------------*/
typedef struct ncs_mq_req_open_info
{
   uns8  *qname;
   uns32 iflags;
   NCS_OS_POSIX_MQD *o_mqd;
   NCS_OS_POSIX_MQ_ATTR attr;
}NCS_OS_POSIX_MQ_REQ_OPEN_INFO;


/*-----------------------------------*/
typedef struct ncs_mq_req_close_info
{
   NCS_OS_POSIX_MQD mqd;
}NCS_OS_POSIX_MQ_REQ_CLOSE_INFO;

/*-----------------------------------*/
typedef struct ncs_mq_req_unlink_info
{
   uns8  *qname;
}NCS_OS_POSIX_MQ_REQ_UNLINK_INFO;

/*-----------------------------------*/
typedef struct ncs_posix_mq_req_msg_send_info
{
   NCS_OS_POSIX_MQD mqd;
   NCS_OS_POSIX_MQ_MSG        i_msg;
}NCS_OS_POSIX_MQ_REQ_MSG_SEND_INFO;

/*-----------------------------------*/
typedef struct ncs_posix_mq_req_msg_recv_info
{
   NCS_OS_POSIX_MQD mqd;
   NCS_OS_POSIX_MQ_MSG        i_msg;
   NCS_OS_POSIX_MQ_TIMEOUT  timeout;
}NCS_OS_POSIX_MQ_REQ_MSG_RECV_INFO;

/*-----------------------------------*/
typedef struct ncs_posix_mq_req_info
{
   NCS_OS_POSIX_MQ_REQ_TYPE req;

   union 
   {
      NCS_OS_POSIX_MQ_REQ_OPEN_INFO      open;
      NCS_OS_POSIX_MQ_REQ_CLOSE_INFO     close;
      NCS_OS_POSIX_MQ_REQ_UNLINK_INFO    unlink;
      NCS_OS_POSIX_MQ_REQ_MSG_SEND_INFO  send;
      NCS_OS_POSIX_MQ_REQ_MSG_RECV_INFO  recv;
   }info;
}NCS_OS_POSIX_MQ_REQ_INFO;

#endif /* __POSIX_H__ */
#endif
