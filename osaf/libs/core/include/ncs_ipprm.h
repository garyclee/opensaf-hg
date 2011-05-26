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

  MODULE NAME:  NCS_IPPRM.H

..............................................................................

  DESCRIPTION:

  CONTENTS:

******************************************************************************
*/

#ifndef NCS_IPPRM_H
#define NCS_IPPRM_H

#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncs_scktprm.h"

#include "ncs_iplib.h"
#include "ncs_ip.h"

#include "ncssysf_lck.h"
#include "ncsusrbuf.h"
#include "ncs_queue.h"

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 **                                                                        **
 **                                                                        **
 **                       General Definitions                              **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ***************************************************************************/

	typedef struct sysf_ip_dispatch_info_tag {

		union {
			struct {
				NCS_IPV4_ADDR i_local_addr;
				uns16 i_local_port;
				NCS_IPV4_ADDR i_remote_addr;
				NCS_IP_ADDR_TYPE addr_family_type;
#if (NCS_IPV6 == 1)
				NCS_IPV6_ADDR i_ipv6_local_addr;
				NCS_IPV6_ADDR i_ipv6_remote_addr;
#endif
				uns16 i_remote_port;
				uns32 i_if_index;

			} rcv;
		} info;

	} SYSF_IP_DISPATCH_INFO;

/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 **                                                                        **
 **                                                                        **
 **           Operating System Specific Implementation Include File        **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ***************************************************************************/

#include "ip_defs.h"

#ifdef __NCSINC_OSE__
#include "sysf_ip.h"
#endif

#if (NCS_SINGLE_NODE_TEST == 1)
#define NCS_GET_IF_INDEX(index) (1)
#else
#define NCS_GET_IF_INDEX(index) (index)
#endif

/* Multicast receive packet filter used to allow
   multicast packets that came through the interface
   the socket is bound to
 */
#ifndef m_TS_IP_SVC_MC_RCV_FILTER
#define m_TS_IP_SVC_MC_RCV_FILTER(ub,addr)
#endif
#ifndef m_IP_SVC_MC_RCV_FILTER
#define m_IP_SVC_MC_RCV_FILTER(ub,addr) m_TS_IP_SVC_MC_RCV_FILTER(ub,addr)
#endif

/* does IP stack require a separate multicast socket;
                         i.e. multicast is not supported */
#ifndef NCS_TS_SOCK_MULTICAST_READ_SOCKET
#define NCS_TS_SOCK_MULTICAST_READ_SOCKET     0	/* no */
#endif

#if (NCS_TS_SOCK_MULTICAST_READ_SOCKET == 1)
	int ncs_mcast_sim_ipv4_rcv_sock(NCS_IPV4_ADDR mcast_addr, fd_set *read_fd, uns32 if_index);

#define m_NCS_MCAST_SIM_IPV4_RCV_SOCK(m_addr,fd,if_index) ncs_mcast_sim_ipv4_rcv_sock(m_addr,fd,if_index)

#define m_IP_SVC_MCRS_FD_SET_READ(se,readfds) \
      { \
         int i; \
         for (i=0; i<MAX_MC_SOCK; i++) \
         { \
            if ((se)->multicast_read_socket[i] != -1) \
            { \
               if (!(FD_ISSET ((se)->multicast_read_socket[i], &readfds))) \
                  FD_SET ((se)->multicast_read_socket[i], &readfds); \
            } \
         } \
      }

#define m_IP_SVC_MCRS_FD_CLR_READ(se,readfds) \
      { \
         int i; \
         for (i=0; i<MAX_MC_SOCK; i++) \
         { \
            if ((se)->multicast_read_socket[i] != -1) \
            { \
               if (!(FD_ISSET ((se)->multicast_read_socket[i], &readfds))) \
                  FD_CLR ((se)->multicast_read_socket[i], &readfds); \
            } \
         } \
      }

#define m_IP_SVC_MCRS_INIT(se) \
   { \
      int i; \
      for (i=0; i<MAX_MC_SOCK; i++) \
      { \
         (se)->multicast_read_socket[i] = (SOCKET)-1; \
         (se)->multicast_closing_socket[i] = 0; \
         (se)->muticast_addr[i] = (NCS_IPV4_ADDR) 0; \
      } \
   }

#define m_IP_SVC_MCRS_CLEAN(se) \
   { \
      int i; \
      for (i=0; i<MAX_MC_SOCK; i++) \
      { \
         if(socket_list_walker->multicast_closing_socket[i] != 0) \
         { \
           m_NCSSOCK_CLOSE(socket_list_walker->multicast_closing_socket[i]); \
           socket_list_walker->multicast_closing_socket[i] = 0; \
         } \
      } \
}

#define m_IP_SVC_MCRS_READ(se, read_fds) \
{ \
    int i; \
    for (i=0; i<MAX_MC_SOCK; i++) \
    { \
        if ((se)->multicast_read_socket[i] != -1) \
        { \
           if (m_NCSSOCK_FD_ISSET ((se)->multicast_read_socket[i], &(read_fds))) \
           { \
               socket_entry_dispatch (se,NCS_SOCKET_EVENT_READ_INDICATION, \
                   (struct ncs_ip_request_info_tag*) (i+1)); \
           } \
        } \
    } \
}

	struct ncs_socket_entry_tag;

#define m_IP_SVC_MCRS_OPEN(se,maddr) ip_svc_mcrs_open(se,maddr)
	extern uns32 ip_svc_mcrs_open(struct ncs_socket_entry_tag *se, NCS_IPV4_ADDR muticast_addr);

#define m_IP_SVC_MCRS_CLOSE(se,maddr) ip_svc_mcrs_close(se,maddr)
	extern uns32 ip_svc_mcrs_close(struct ncs_socket_entry_tag *se, NCS_IPV4_ADDR muticast_addr);

#define m_IP_SVC_MCRS_GET_RCV_SOCKET(se,umc,rsock) \
{ \
    if((umc) != 0) \
{ \
    (rsock) = (se)->multicast_read_socket[(umc) - 1]; \
} \
    else \
{ \
    (rsock) = (se)->server_socket; \
} \
}
#else

#define m_NCS_MCAST_SIM_IPV4_RCV_SOCK(m_addr,fd,if_index) NCSCC_RC_SUCCESS

#define m_IP_SVC_MCRS_FD_SET_READ(se,read_fds)
#define m_IP_SVC_MCRS_FD_CLR_READ(se,read_fds)
#define m_IP_SVC_MCRS_INIT(se)
#define m_IP_SVC_MCRS_CLEAN(se)
#define m_IP_SVC_MCRS_READ(se,readfds)
#define m_IP_SVC_MCRS_OPEN(se,maddr)       NCSCC_RC_SUCCESS
#define m_IP_SVC_MCRS_CLOSE(se,maddr)      NCSCC_RC_SUCCESS

#define m_IP_SVC_MCRS_GET_RCV_SOCKET(se,umc,rsock) (rsock) = (se)->server_socket
#endif

/* build ip hdr to send on raw socket */
#ifndef NCS_TS_SOCK_USE_IPHDR
#define NCS_TS_SOCK_USE_IPHDR       1	/* yes */
#endif

#ifndef NCSSOCK_REUSE_PORT
#define NCSSOCK_REUSE_PORT          0
#endif

	struct ncs_socket_entry_tag;

#if (NCS_TS_SOCK_USE_IPHDR == 1)

#define m_IP_SVC_IPHDR_CL_BUILD_ENABLE(se) ip_svc_iphdr_cl_build_enable(se)
	extern uns32 ip_svc_iphdr_cl_build_enable(struct ncs_socket_entry_tag *se);

#define m_IP_SVC_CL_IPHDR_REMOVE(ppub,hlen) m_MMGR_REMOVE_FROM_START((ppub),hlen)

#define m_IP_SVC_BUILD_RAW_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) ip_svc_iphdr_cl_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 ip_svc_iphdr_cl_build(struct ncs_socket_entry_tag *se,
					   SOCKET send_socket,
					   NCS_IP_REQUEST_INFO * ipr,
					   struct sockaddr_in *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#if (NCS_IPV6 == 1)
	extern uns32 ipv6_svc_mc_def_set_snd_if(struct ncs_socket_entry_tag *se);
#define m_IP_SVC_IPV6HDR_CL_BUILD_ENABLE  ip_svc_ipv6hdr_cl_build_enable(se)
	extern uns32 ip_svc_ipv6hdr_cl_build_enable(struct ncs_socket_entry_tag *se);
#define m_IPV6_SVC_BUILD_RAW_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) ip_svc_ipv6hdr_cl_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 ip_svc_ipv6hdr_cl_build(struct ncs_socket_entry_tag *se,
					     SOCKET send_socket,
					     NCS_IP_REQUEST_INFO * ipr,
					     struct sockaddr_in6 *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#endif
#else
#define m_IP_SVC_IPHDR_CL_BUILD_ENABLE(se) NCSCC_RC_SUCCESS
#define m_IP_SVC_CL_IPHDR_REMOVE(ppub,hlen)

#define m_IP_SVC_BUILD_RAW_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) ip_svc_iphdr_os_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 ip_svc_iphdr_os_build(struct ncs_socket_entry_tag *se,
					   SOCKET send_socket,
					   NCS_IP_REQUEST_INFO * ipr,
					   struct sockaddr_in *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#if (NCS_IPV6 == 1)
#define m_IP_SVC_IPV6HDR_CL_BUILD_ENABLE(se) NCSCC_RC_SUCCESS
#define m_IPV6_SVC_BUILD_RAW_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) ip_svc_ipv6hdr_os_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 ipv6_svc_iphdr_os_build(struct ncs_socket_entry_tag *se,
					     SOCKET send_socket,
					     NCS_IP_REQUEST_INFO * ipr,
					     struct sockaddr_in6 *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#endif
#endif

#ifndef NCS_UDP_BROADCAST
#define NCS_UDP_BROADCAST    1
#endif

/* build ip hdr to send on raw socket */
#ifndef NCS_TS_SOCK_USE_UDPHDR
#define NCS_TS_SOCK_USE_UDPHDR       0	/* no */
#endif

#define NCS_UDP_HEADER_LEN (8)

#if (NCS_TS_SOCK_USE_UDPHDR == 1)
#define m_UDP_SVC_IPHDR_CL_BUILD_ENABLE(se) udp_svc_iphdr_cl_build_enable(se)
#if (NCS_IPV6 == 1)
	extern uns32 udp_svc_ipv6hdr_cl_build_enable(struct ncs_socket_entry_tag *se);
#define m_UDP_SVC_IPV6HDR_CL_BUILD_ENABLE(se) udp_svc_ipv6hdr_cl_build_enable(se)
#define m_UDP_IPV6_SVC_BUILD_DGRAM_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) udp_svc_ipv6hdr_cl_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 udp_svc_ipv6hdr_cl_build(struct ncs_socket_entry_tag *se,
					      SOCKET send_socket,
					      NCS_IP_REQUEST_INFO * ipr,
					      struct sockaddr_in6 *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#endif
	extern uns32 udp_svc_iphdr_cl_build_enable(struct ncs_socket_entry_tag *se);

#define m_UDP_SVC_CL_IPHDR_REMOVE(ppub,hlen) m_MMGR_REMOVE_FROM_START((ppub),hlen)

#define m_UDP_SVC_BUILD_DGRAM_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) udp_svc_iphdr_cl_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 udp_svc_iphdr_cl_build(struct ncs_socket_entry_tag *se,
					    SOCKET send_socket,
					    NCS_IP_REQUEST_INFO * ipr,
					    struct sockaddr_in *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#else

#define m_UDP_SVC_IPHDR_CL_BUILD_ENABLE(se) NCSCC_RC_SUCCESS
#if (NCS_IPV6 == 1)
#define m_UDP_SVC_IPV6HDR_CL_BUILD_ENABLE(se) NCSCC_RC_SUCCESS
#define m_UDP_IPV6_SVC_BUILD_DGRAM_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) udp_svc_ipv6hdr_os_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 udp_svc_ipv6hdr_os_build(struct ncs_socket_entry_tag *se,
					      SOCKET send_socket,
					      NCS_IP_REQUEST_INFO * ipr,
					      struct sockaddr_in6 *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#endif
#define m_UDP_SVC_CL_IPHDR_REMOVE(ppub,hlen)

#define m_UDP_SVC_BUILD_DGRAM_IPHDR(se,ss,ipr,saddr,tl,hl,ppub) udp_svc_iphdr_os_build(se,ss,ipr,saddr,tl,hl,ppub)
	extern uns32 udp_svc_iphdr_os_build(struct ncs_socket_entry_tag *se,
					    SOCKET send_socket,
					    NCS_IP_REQUEST_INFO * ipr,
					    struct sockaddr_in *saddr, int *total_len, uns8 *hlen, USRBUF **usrbuf);
#endif

/* build ip hdr to send on raw socket */
#ifndef NCS_TS_SOCK_USE_MULTICAST_LOOP
#define NCS_TS_SOCK_USE_MULTICAST_LOOP        1	/* yes */
#endif

/* is bind to device supported ? */
#ifndef NCS_TS_SOCK_BINDTODEVICE_SUPPORTED
#define NCS_TS_SOCK_BINDTODEVICE_SUPPORTED    0	/* no */
#endif

#if (NCS_TS_SOCK_BINDTODEVICE_SUPPORTED == 1)
#define m_IP_SVC_BIND_TO_IFACE(se) ip_svc_bind_to_iface(se)
	extern uns32 ip_svc_bind_to_iface(struct ncs_socket_entry_tag *se);
#else
#define m_IP_SVC_BIND_TO_IFACE(se) NCSCC_RC_SUCCESS
#endif

/* Should multicast read socket be bound to INADDR_ANY */
#ifndef NCS_OS_SOCKS_MCASTBIND_INADDRANY
#define NCS_OS_SOCKS_MCASTBIND_INADDRANY      0	/* no */
#endif

/* is linger supported ? */
#ifndef NCS_TS_SOCK_LINGER_SUPPORTED
#define NCS_TS_SOCK_LINGER_SUPPORTED          1	/* yes */
#endif

#if (NCS_TS_SOCK_LINGER_SUPPORTED == 1)
#define m_IP_SVC_SO_LINGER_ENABLE(se) ip_svc_so_linger_enable(se)
	extern uns32 ip_svc_so_linger_enable(struct ncs_socket_entry_tag *se);
#else
#define m_IP_SVC_SO_LINGER_ENABLE(se) NCSCC_RC_SUCCESS
#endif

/* set local addr in socket addr structure for udp only */
#ifndef m_NCS_TS_SOCK_SET_UDP_SADDR
#define m_NCS_TS_SOCK_SET_UDP_SADDR(sa, la, ra)                            \
{ if ((((ra) & 0xf0000000) == 0xe0000000) || (se->is_multicast_raw_udp == TRUE)) /* multicast destination ? */  \
  (sa) = htonl(INADDR_ANY);                                         \
 else                                                                  \
  (sa) = htonl(la);                                                 \
}
#endif

#if (NCS_IPV6 == 1)
#ifndef m_NCS_TS_SOCK_SET_IPV6_UDP_SADDR
#define m_NCS_TS_SOCK_SET_IPV6_UDP_SADDR(sa, la, ra, se)     \
{ \
   if (((ra.m_ipv6_addr[0] & 0xf0) == 0xe0) || (se->is_multicast_raw_udp == TRUE))\
   {\
      memcpy(&sa, &in6addr_any, m_IPV6_ADDR_SIZE);\
   } else\
   {\
      memcpy(&sa, &la, m_IPV6_ADDR_SIZE);\
   } \
}

#ifndef m_NCS_TS_SOCK_SET_IPV6_MULTIADDR
#define m_NCS_TS_SOCK_SET_IPV6_MULTIADDR(s, ma, la, i)     \
{ memcpy( &(s.ipv6mr_multiaddr.s6_addr), &ma,m_IPV6_ADDR_SIZE);            \
  s.ipv6mr_interface = (i);  \
}
#endif
#endif

#ifndef m_NCS_TS_SOCK_IPV6_MREQ_OBJ
#define m_NCS_TS_SOCK_IPV6_MREQ_OBJ struct ipv6_mreq
#endif

#ifndef m_IP_SVC_MC_SET_IPV6_SND_IF
#define m_IP_SVC_MC_SET_IPV6_SND_IF(se) ipv6_svc_mc_def_set_snd_if(se)
	extern uns32 ipv6_svc_mc_def_set_snd_if(struct ncs_socket_entry_tag *se);
#endif
#endif   /* end of (NCS_IPV6 == 1) */

/* structure needed when joining a socket to a multicast group */
#ifndef NCS_TS_SOCK_MREQ_OBJ
#define NCS_TS_SOCK_MREQ_OBJ struct ip_mreq
#endif

/* set up the socket multicast address structure */
#ifndef m_NCS_TS_SOCK_SET_MULTIADDR
#define m_NCS_TS_SOCK_SET_MULTIADDR(s, ma, la, i)     \
{ (s).imr_multiaddr.s_addr = htonl(ma);            \
 (s).imr_interface.s_addr = htonl(la);            \
}
#endif

#ifndef m_IP_SVC_MC_SET_SND_IF
#define m_IP_SVC_MC_SET_SND_IF(se) ip_svc_mc_def_set_snd_if(se)
	extern uns32 ip_svc_mc_def_set_snd_if(struct ncs_socket_entry_tag *se);
#endif

#ifndef IP_HEADER_LENGTH_OFFSET_HOST_NETWORK_BYTE_FEATURE_BUG
#define IP_HEADER_LENGTH_OFFSET_HOST_NETWORK_BYTE_FEATURE_BUG    0
#endif

/* Does OS support unnumbered IPv4 interfaces*/
#ifndef NCS_OS_SUPPORTS_UNNUMBERED
#define NCS_OS_SUPPORTS_UNNUMBERED   0	/* No */
#endif

/****************************************************************************
 * OS interface index to IPv4 address converstion Primitive definition
 *
 * ifIndex -- interface index
 * returns -- IPv4 address (host byte order) or 0 if ERROR
 *
 ***************************************************************************/

#ifndef m_NCS_OS_IFIDX_TO_IPV4
#define m_NCS_OS_IFIDX_TO_IPV4(ifIndex)  os_ifidx_to_ipv4(ifIndex)
	extern unsigned int os_ifidx_to_ipv4(unsigned int ifIndex);
#endif

#if (NCS_IPV6 == 1)
#ifndef m_NCS_OS_IFIDX_TO_IPV6
#define m_NCS_OS_IFIDX_TO_IPV6(ifIndex, ipv6_addr)  memset(&ipv6_addr, 0x0, sizeof(ipv6_addr))
	extern unsigned int os_ifidx_to_ipv6(unsigned int ifIndex, NCS_IPV6_ADDR * ipaddr);
#endif
#endif

/****************************************************************************
 * OS IPv4 address to interface index converstion Primitive definition
 *
 * hbo_ipv4 -- IPv4 address in host byte order
 * returns interface index
 *
 ***************************************************************************/

#ifndef m_NCS_OS_IPV4_TO_IFIDX
#define m_NCS_OS_IPV4_TO_IFIDX(hbo_ipv4)  os_ipv4_to_ifidx(hbo_ipv4)
	int os_ipv4_to_ifidx(unsigned int hbo_ipv4);
#endif

#ifndef NCS_OS_IPSVC_AIO_OP_TYPE
	typedef enum sysf_io_op_type {
		SYSF_IO_OP_DUMMY = 0x1735
	} SYSF_IO_OP_TYPE;
#define NCS_OS_IPSVC_AIO_OP_TYPE SYSF_IO_OP_TYPE
#endif

#ifndef NCS_OS_IPSVC_AIO_OP_DATA
	typedef struct sysf_io_op_data {
		unsigned int dummy;

	} SYSF_IO_OP_DATA;
#define NCS_OS_IPSVC_AIO_OP_DATA SYSF_IO_OP_DATA
#endif

#ifndef m_NCS_OS_IPSVC_ASYNC_HDL_INIT
#define m_NCS_OS_IPSVC_ASYNC_HDL_INIT(hdl)
#endif

#ifndef m_NCS_OS_IPSVC_ASYNC_IO_START
#define m_NCS_OS_IPSVC_ASYNC_IO_START(se,as,op,a1,a2)
#endif

#ifndef m_NCS_OS_IPSVC_ASYNC_IO_END
#define m_NCS_OS_IPSVC_ASYNC_IO_END(se,as)
#endif

#ifndef m_NCS_OS_IPSVC_ASYNC_IO_COMPLETE
#define m_NCS_OS_IPSVC_ASYNC_IO_COMPLETE(se,di)
#endif

	struct ncs_socket_entry_tag;
#if (NCS_TS_SOCK_ASYNC_IO == 1)
	struct sysf_io_op_data;

#define MAX_ASYNC_IO_TASKS 3
#endif

	typedef uns32 (*NCS_SOCKET_HANDLER) (struct ncs_socket_entry_tag * se, NCSCONTEXT arg);

	typedef struct ncs_socket_entry_tag {

		struct ncs_socket_entry_tag *next;
		struct ncs_socket_entry_tag *prev;
		struct ncs_socket_entry_tag *next_listener;	/* when sharing socket for tcp listening */
		struct ncs_socket_entry_tag *prev_listener;	/* when sharing socket for tcp listening */
		const NCS_SOCKET_HANDLER *dispatch;
		struct ncs_socket_context_tag *socket_context;
		NCS_LOCK lock;
		SOCKET client_socket;
		SOCKET server_socket;
		NCS_BOOL is_multicast_raw_udp;	/* If it is TRUE then 
						   it is multicast 
						   RAW */
#ifdef __NCSINC_OSE__

		NCS_IP_SEND_EVT *async_evt;	/* event pointer which will be filled
						 * when async event is received
						 */
		NCS_OSE_ASYNC_SOCK *async_ose_ctrl_blk;	/* async socket control
							 * block 
							 */
		uns32 sock_type;	/* socket type which differnetiate
					 * RAW and UDP packet
					 */
#endif
#if (NCS_TS_SOCK_SHAREABLE_SOCKET == 0)
		SOCKET old_client_socket;
		unsigned long client_task_handle;
#endif
		uns32 hm_hdl;
#if (NCS_TS_SOCK_MULTICAST_READ_SOCKET == 1)
#define MAX_MC_SOCK 2		/* maximum number of multicast groups supported per connection */
		SOCKET multicast_read_socket[MAX_MC_SOCK];
		SOCKET multicast_closing_socket[MAX_MC_SOCK];
		NCS_IPV4_ADDR muticast_addr[MAX_MC_SOCK];
#endif
#if (NCS_TS_SOCK_ASYNC_IO == 1)
#ifdef MAX_MC_SOCK
#define MAX_ASYNC_IOS (MAX_MC_SOCK + 2)
#else
#define MAX_ASYNC_IOS 2
#endif
		NCS_OS_IPSVC_AIO_OP_DATA *async_io_data[MAX_ASYNC_IOS];
		NCS_BOOL async_io_is_set;
#endif
		int state;
		NCS_IP_TYPE ip_type;
		uns8 ip_protocol;	/* only used if ip_type is Raw */
		uns32 mtu;
		int ttl;	/* only for unicast address sends */
		NCSCONTEXT user_handle;
		NCSCONTEXT user_connection_handle;
		NCS_BOOL unnumbered;
		NCS_IPV4_ADDR local_addr;
		/* "fake" ip addr for systems that do not support unnumbered IP interfaces */
		NCS_IPV4_ADDR fake_addr;
		uns16 local_port;
		NCS_IPV4_ADDR remote_addr;
		NCS_IP_ADDR_TYPE addr_family_type;	/* to find which addr family the socket belongs to */
#if (NCS_IPV6 == 1)
		NCS_IPV6_ADDR ipv6_local_addr;
		NCS_IPV6_ADDR ipv6_fake_addr;
		NCS_IPV6_ADDR ipv6_remote_addr;
#endif
		uns16 remote_port;
		uns32 if_index;
		NCS_IP_INDICATION ip_data_indication;
		NCS_IP_INDICATION ip_ctrl_indication;
		NCS_BOOL bindtodevice;	/* If 1, use BIND to Device option - Raw Only */
		NCS_BOOL recv_routeralert;	/* If 1, enable receive with RouterAlert option - Raw Only */
		NCS_BOOL broadcast;	/* If 1, enable to process Broadcast pkts */
		NCS_BOOL enb_pkt_info;	/* If 1, enable receving destination IP, ifindex and hop limit */
		NCS_BOOL link_loc_sock;	/* If 1, enable to open Link Local sockets */
		NCS_BOOL mcast_loc_lbk;	/* TRUE if loopback is allowed on this socket */
		uns32 tos;	/* type of service */
		NCS_IP_SEND_POLICY send_policy;	/* BLOCK or QUEUE */
		NCS_QUEUE send_queue_hi;	/* HI priority send queue */
		NCS_QUEUE send_queue_low;	/* LOW priority send queue */
		NCS_USRDATA_FORMAT rcv_usrdata_format;	/* If equal to NCS_IP_FLAT_DATA, pass on received data as a ptr
							 * to flat memory instead of ptr to usrbuf*/

	} NCS_SOCKET_ENTRY;

	typedef struct ncs_se_send_qe_tag {
		NCS_QELEM qel;
		NCS_BOOL is_flat;
		NCSCONTEXT buffer;
		USRBUF *current_ub;
		uns32 length;
		struct sockaddr_in daddr;
#if (NCS_IPV6 == 1)
		struct sockaddr_in6 ipv6_addr_in;
#endif

	} NCS_SE_SEND_QE;

	typedef struct ncs_socket_list_tag {
		fd_set readfds;
		fd_set writefds;
		int socket_entries_freed;
		NCS_SOCKET_ENTRY *first;
		NCS_SOCKET_ENTRY *last;
		NCS_LOCK lock;
		int num_in_list;
	} NCS_SOCKET_LIST;

	typedef struct ncs_socket_context_tag {
		NCS_SOCKET_LIST ReceiveQueue;
		NCS_LOCK lock;
		/*NCS_LOCK           select_lock; */
		NCS_SOCKET_ENTRY slb_socket_entry;
		int select_timeout;
		int max_connections;
		int stop_flag;
		int socket_timeout_flag;
		void *task_handle;
#if (NCS_TS_SOCK_ASYNC_IO == 1)
		NCSCONTEXT async_handle;
		void *async_tsk_hdl[MAX_ASYNC_IO_TASKS];
#endif
#ifdef __NCSINC_OSE__
		NCS_OSE_ASYNC_SOCK *ose_async_context;
		SYSF_MBX *ip_mbx;
#endif

		NCS_SEL_OBJ fast_open_sel_obj;

	} NCS_SOCKET_CONTEXT;

#ifdef  __cplusplus
}
#endif

#endif   /* NCS_IPPRM_H */
