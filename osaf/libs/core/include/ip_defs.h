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

  MODULE NAME:  IP_DEFS.H

  DESCRIPTION:

******************************************************************************
*/

#ifndef IP_DEFS_H
#define IP_DEFS_H

/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                     OPERATING SYSTEM INCLUDE FILES                      **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/
#include "ncsgl_defs.h"
#include "os_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

	int os_ipv4_to_ifidx(unsigned int hbo_ipv4);
	unsigned int os_ifidx_to_ipv4(unsigned int ifIndex);

/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **             Socket macro definitions                                    **
 **                                                                         **
 ** Override default definition in ncs_ipprm.h                               **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/

/* all the below three macros are inter dependent, so enable all the macro/ disable all */
#if (NCS_IPV6 == 1)
#define m_SYSF_NCSSOCK_SUPPORT_RECVMSG (1)
#define m_SYSF_NCSSOCK_SUPPORT_SENDMSG (1)
#define NCS_SUPPORT_GET_PKT_INFO (1)
#else
#define m_SYSF_NCSSOCK_SUPPORT_RECVMSG (0)
#define m_SYSF_NCSSOCK_SUPPORT_SENDMSG (0)
#define NCS_SUPPORT_GET_PKT_INFO (0)
#endif

#define NCS_TS_SOCK_USE_MULTICAST_LOOP        1	/* yes */

/* Does OS support unnumbered IPv4 interfaces*/
#define NCS_OS_SUPPORTS_UNNUMBERED   1	/* Yes */

/* does IP stack require a separate multicast socket;
                         i.e. multicast is not supported */
#define NCS_TS_SOCK_MULTICAST_READ_SOCKET     1	/* yes */

/* is bind to device supported ? */
#define NCS_TS_SOCK_BINDTODEVICE_SUPPORTED    1	/* yes */

/* build ip hdr to send on raw socket - set to 1 (yes) */
#define NCS_TS_SOCK_USE_IPHDR        1	/* yes */
/* build udp hdr to send on udp socket - set to 1 (yes) */
#define NCS_TS_SOCK_USE_UDPHDR        0	/* No */

/* use L2 simulated sockets for operating on multicast packets */
/* Undefine/redefine if defined already in ncs_scktprm.h */
#ifdef NCS_USE_L2_MCAST_SOCK
#undef NCS_USE_L2_MCAST_SOCK
#define NCS_USE_L2_MCAST_SOCK (1)	/* No */
#endif

/* structure needed when joining a socket to a multicast group */
#define NCS_TS_SOCK_MREQ_OBJ struct ip_mreqn

	struct ncs_socket_entry_tag;
#if (NCS_IPV6 == 1)
#define m_NCS_TS_SOCK_IPV6_MREQ_OBJ struct ipv6_mreq
#define m_IP_SVC_MC_SET_IPV6_SND_IF(se) ipv6_svc_mc_lin_set_snd_if(se)
	extern uns32 ipv6_svc_mc_lin_set_snd_if(struct ncs_socket_entry_tag *se);
	extern int os_ipv6_to_ifidx(NCS_IPV6_ADDR hbo_ipv6);
#endif

/* set up the socket multicast address structure */
#define m_NCS_TS_SOCK_SET_MULTIADDR(s, ma, la, i)     \
{ (s).imr_multiaddr.s_addr = htonl(ma);            \
 (s).imr_address.s_addr   = htonl(la);            \
 (s).imr_ifindex          = (i);                  \
}

#define m_IP_SVC_MC_SET_SND_IF(se) ip_svc_mc_lin_set_snd_if(se)
	extern uns32 ip_svc_mc_lin_set_snd_if(struct ncs_socket_entry_tag *se);

#ifdef  __cplusplus
}
#endif

#endif   /* IP_DEFS_H */
