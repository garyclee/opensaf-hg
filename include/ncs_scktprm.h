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

  MODULE NAME:  NCS_SCKTPRM.H


..............................................................................

  DESCRIPTION:This file contains the socket primitives which were previously 
              defined in ncs_ipprm.h  Since IP functionality is no longer  
              a standard basic LEAP feature, but sockets are, IP primitives
              had to be split, and thus, this file ncs_scktprm.h has been
              created to house the socket definitions.

  CONTENTS:


******************************************************************************
*/

#ifndef NCS_SCKTPRM_H
#define NCS_SCKTPRM_H

#include "ncs_osprm.h"
#include "ncsgl_defs.h"
#include "ncssysf_lck.h" /* Steve's code.. Another version of the same Layer V */
#include "ncsusrbuf.h"
#include "ncs_queue.h"

#include "sckt_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef NCS_IF_NAMESIZE
#define NCS_IF_NAMESIZE 16
#endif

/* all the below three macros are inter dependent, so enable all the macro/ disable all */
#ifndef m_SYSF_NCSSOCK_SUPPORT_RECVMSG
#define m_SYSF_NCSSOCK_SUPPORT_RECVMSG (0)
#endif

#ifndef m_SYSF_NCSSOCK_SUPPORT_SENDMSG
#define m_SYSF_NCSSOCK_SUPPORT_SENDMSG (0)
#endif

#ifndef NCS_SUPPORT_GET_PKT_INFO
#define NCS_SUPPORT_GET_PKT_INFO (0)
#endif

#if (m_SYSF_NCSSOCK_SUPPORT_SENDMSG == 0)
#define m_SYSF_TMP_NCSSOCK_SUPPORT_SENDMSG (0)
#else
#define m_SYSF_TMP_NCSSOCK_SUPPORT_SENDMSG (1)
#endif

#if (m_SYSF_NCSSOCK_SUPPORT_RECVMSG == 0)
#define m_SYSF_TMP_NCSSOCK_SUPPORT_RECVMSG (0)
#else
#define m_SYSF_TMP_NCSSOCK_SUPPORT_RECVMSG (1)
#endif

/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **             Socket macro definitions                                    **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/

/* is socket id shared across threads by default ? */
#ifndef NCS_TS_SOCK_SHAREABLE_SOCKET
#define NCS_TS_SOCK_SHAREABLE_SOCKET          1   /* yes */
#endif

#if (NCS_TS_SOCK_SHAREABLE_SOCKET == 0)

#define m_IP_SVC_SHARE_CL_SOCK(se) (se)->server_socket = m_NCSSOCK_SHARE((se)->client_socket,(int)(se)->socket_context->task_handle)
#define m_IP_SVC_SHARE_SRV_SOCK(se) (se)->client_socket = m_NCSSOCK_SHARE((se)->server_socket,(int)(se)->client_task_handle)
#define m_IP_SVC_SHARED_SOCK_SAVE_CUR_TSK_HDL(se) m_NCS_TASK_CURRENT((void**)&(se)->client_task_handle)
#define m_IP_SVC_CLOSE_SHARED_SOCK(se) m_NCSSOCK_CLOSE((se)->server_socket)
#define m_IP_SVC_SAVE_SHARED_SOCK(se) (se)->old_client_socket = (se)->client_socket
#define m_IP_SVC_SHARED_SOCK_CPY_TSK_HDL(src_se,dst_se) (dst_se)->client_task_handle = (src_se)->client_task_handle
#define m_IP_SVC_CLOSE_SAVED_SHARED_SOCK(se) \
    { \
   if (se->old_client_socket != 0) \
   {             \
      m_NCSSOCK_CLOSE(se->old_client_socket); \
      se->old_client_socket  = 0; \
   } \
   }
#define m_IP_SVC_SE_CLEAR(se) \
{ \
   se->next               = NULL; \
   se->prev               = NULL; \
   se->next_listener      = NULL; \
   se->prev_listener      = NULL; \
   se->state              = NCS_SOCKET_STATE_IDLE; \
   se->client_socket      = 0; \
   se->server_socket      = 0; \
   se->old_client_socket  = 0; \
   se->client_task_handle = 0; \
   }

#else

#define m_IP_SVC_SHARE_CL_SOCK(se) (se)->server_socket = (se)->client_socket
#define m_IP_SVC_SHARE_SRV_SOCK(se) (se)->client_socket = (se)->server_socket
#define m_IP_SVC_SHARED_SOCK_SAVE_CUR_TSK_HDL(se)
#define m_IP_SVC_CLOSE_SHARED_SOCK(se) (se)->server_socket = 0
#define m_IP_SVC_SAVE_SHARED_SOCK(se)
#define m_IP_SVC_SHARED_SOCK_CPY_TSK_HDL(src_se,dst_se)
#define m_IP_SVC_CLOSE_SAVED_SHARED_SOCK(se) 
#define m_IP_SVC_SE_CLEAR(se) \
{ \
   se->next               = NULL; \
   se->prev               = NULL; \
   se->next_listener      = NULL; \
   se->prev_listener      = NULL; \
   se->state              = NCS_SOCKET_STATE_IDLE; \
   se->client_socket      = 0; \
   se->server_socket      = 0; \
   }

#endif


/* must socket be owned by a single thread or process ? */
#ifndef NCS_TS_SOCK_ONE_OWNER
#define NCS_TS_SOCK_ONE_OWNER                 0   /* no */
#endif

/* If you use L2 socket, then this below macro should be set to "1" for mcast forwarder */
#ifndef NCS_USE_L2_MCAST_SOCK
#define NCS_USE_L2_MCAST_SOCK (0)
#endif


#ifndef NCS_TS_SOCK_ERROR
#define NCS_TS_SOCK_ERROR                     -1
#endif

#ifndef NCS_TS_SOCK_INVALID
#define NCS_TS_SOCK_INVALID                   -1
#endif

#ifndef m_NCS_TS_SOCK_CREATE
#define m_NCS_TS_SOCK_CREATE                  NCSCC_RC_SUCCESS
#endif

#ifndef m_NCS_TS_SOCK_DESTROY
#define m_NCS_TS_SOCK_DESTROY
#endif

#ifndef m_NCS_TS_SOCK_SOCKET
#define m_NCS_TS_SOCK_SOCKET                  socket
#endif

#ifndef m_NCS_TS_SOCK_BIND
#define m_NCS_TS_SOCK_BIND(s,n,l)             bind(s,n,l)
#endif

#ifndef m_NCS_TS_SOCK_CONNECT
#define m_NCS_TS_SOCK_CONNECT(s,n,l)          connect(s,n,l)
#endif

#ifndef m_NCS_TS_SOCK_ACCEPT
#define m_NCS_TS_SOCK_ACCEPT(s,n,l)           accept(s,n,l)
#endif

#ifndef m_NCS_TS_SOCK_LISTEN
#define m_NCS_TS_SOCK_LISTEN                  listen
#endif

#ifndef m_NCS_TS_SOCK_SELECT
#define m_NCS_TS_SOCK_SELECT                  select
#endif

#ifndef m_NCS_TS_SOCK_RECV
#define m_NCS_TS_SOCK_RECV                    recv
#endif

#ifndef m_NCS_TS_SOCK_RECVFROM
#define m_NCS_TS_SOCK_RECVFROM(s,b,bl,f,n,nl) recvfrom(s,b,bl,f,n,nl)
#endif

#ifndef m_NCS_TS_SOCK_SENDTO
#define m_NCS_TS_SOCK_SENDTO(s,b,bl,f,n,nl)   sendto(s,b,bl,f,n,nl)
#endif

#ifndef m_NCS_TS_SOCK_SEND
#define m_NCS_TS_SOCK_SEND                    send
#endif

#ifndef m_NCS_TS_SOCK_CLOSE
#define m_NCS_TS_SOCK_CLOSE                   close
#endif

#ifndef m_NCS_TS_SOCK_ERROR
#define m_NCS_TS_SOCK_ERROR                   ncs_bsd_sock_error()
#endif

#ifndef m_NCS_TS_SOCK_IOCTL
#define m_NCS_TS_SOCK_IOCTL(s,c,m)            ioctl(s,c,m)
#endif

/* Commands used for m_NCS_TS_SOCK_IOCTL: */
#ifndef m_NCS_TS_SOCK_FIONBIO
#define m_NCS_TS_SOCK_FIONBIO                 FIONBIO  /* Set blocking mode of socket */
#endif

#ifndef m_NCS_TS_SOCK_FIONREAD
#define m_NCS_TS_SOCK_FIONREAD                FIONREAD /* Get size of next packet in socket */
#endif

#ifndef m_NCS_TS_SOCK_SETSOCKOPT
#define m_NCS_TS_SOCK_SETSOCKOPT              setsockopt
#endif

#ifndef m_NCS_TS_SOCK_GETSOCKNAME
#define m_NCS_TS_SOCK_GETSOCKNAME             getsockname
#endif

#ifndef m_NCS_TS_SOCK_FD_ISSET
#define m_NCS_TS_SOCK_FD_ISSET                FD_ISSET
#endif

#ifndef m_NCS_TS_SOCK_FD_ZERO
#define m_NCS_TS_SOCK_FD_ZERO                 FD_ZERO
#endif


/* Option flag for recv indicating 'PEEK': */
#ifndef NCS_TS_SOCK_FLAG_MSG_PEEK
#define NCS_TS_SOCK_FLAG_MSG_PEEK             MSG_PEEK
#endif

#ifndef m_SYSF_NCS_TS_SOCK_SUPPORT_RECVMSG
#define m_SYSF_NCS_TS_SOCK_SUPPORT_RECVMSG    0  /* Says 'recvmsg' is not supported. */
#endif

typedef struct msghdr NCS_TS_SOCK_MSGHDR; /* Needed for 'recvmsg' */
typedef struct iovec  NCS_TS_SOCK_IOVEC;  /* Needed for 'recvmsg' */

/* Set up an iovec element (needed for recvmsg): */
#ifndef m_NCS_TS_SOCK_SET_IOVEC
#define m_NCS_TS_SOCK_SET_IOVEC(iov, b, l)    iov->iov_base = b, iov->iov_len = (int)l
#endif

/* Size of in-stack iovec array. Larger array will need malloc. */
#ifndef NCS_TS_SOCK_MAX_STACK_IOVEC
#define NCS_TS_SOCK_MAX_STACK_IOVEC           32
#endif

#ifndef m_NCS_TS_SOCK_RECVMSG
#if (NCS_BSD == 44)
#define m_NCS_TS_SOCK_RECVMSG(s, m, iov, iovl) \
   (m)->msg_namelen = 0, \
   (m)->msg_iov = iov,   \
   (m)->msg_iovlen = (int)iovl, \
   (m)->msg_controllen = 0, \
   recvmsg(s, m, 0)
#else
#define m_NCS_TS_SOCK_RECVMSG(s, m, iov, iovl) \
   (m)->msg_namelen = 0, \
   (m)->msg_iov = iov,   \
   (m)->msg_iovlen = (int)iovl, \
   (m)->msg_accrightslen = 0, \
   recvmsg(s, m, 0)
#endif
#endif

#ifndef NCS_USE_BSD_STYLE_ERROR
#define NCS_USE_BSD_STYLE_ERROR       1
#endif

#ifndef NCS_USE_WINSOCK_STYLE_ERROR
#define NCS_USE_WINSOCK_STYLE_ERROR   0
#endif


/* share socket with another task */
#ifndef m_NCS_TS_SOCK_SHARE
#define m_NCS_TS_SOCK_SHARE(s, t)
#endif

/* give socket ownership to another task */
#ifndef m_NCS_TS_SOCK_NEW_OWNER
#define m_NCS_TS_SOCK_NEW_OWNER(s, t)
#endif


#ifndef SOCKET_VAR
#define SOCKET_VAR
#define SOCKET int
#endif

#ifndef SOCKET_VAR_INVALID
#define SOCKET_VAR_INVALID
#define INVALID_SOCKET -1
#endif

typedef enum
{
   NCS_SOCKET_STATE_IDLE,
   NCS_SOCKET_STATE_LISTENING,
   NCS_SOCKET_STATE_CONNECTING,
   NCS_SOCKET_STATE_ACTIVE,
   NCS_SOCKET_STATE_DISCONNECT,
   NCS_SOCKET_STATE_CLOSING,
   NCS_SOCKET_STATE_MAX = NCS_SOCKET_STATE_CLOSING
} NCS_SOCKET_STATE;



typedef enum
{
   NCS_SOCKET_EVENT_OPEN,
   NCS_SOCKET_EVENT_OPEN_ESTABLISH,
   NCS_SOCKET_EVENT_CLOSE,
   NCS_SOCKET_EVENT_SET_ROUTER_ALERT,
   NCS_SOCKET_EVENT_MULTICAST_JOIN,
   NCS_SOCKET_EVENT_MULTICAST_LEAVE,
   NCS_SOCKET_EVENT_SET_TTL,
   NCS_SOCKET_EVENT_SET_TOS,
   NCS_SOCKET_EVENT_SEND_DATA,
   NCS_SOCKET_EVENT_READ_INDICATION,
   NCS_SOCKET_EVENT_WRITE_INDICATION,
   NCS_SOCKET_EVENT_ERROR_INDICATION,
   NCS_SOCKET_EVENT_MAX = NCS_SOCKET_EVENT_ERROR_INDICATION
}NCS_SOCKET_EVENT;

#ifdef  __cplusplus
}
#endif

#endif   /* NCS_SCKTPRM_H */
