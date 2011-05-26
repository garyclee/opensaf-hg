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

  MODULE NAME: SYSF_IP.H

..............................................................................

  DESCRIPTION:

******************************************************************************
*/
#ifndef SYSF_IP_H
#define SYSF_IP_H

#include "ncsgl_defs.h"
#include "ncs_ipprm.h"

#ifdef __NCSINC_OSE__
#include "ncssysf_ipc.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/** For now, this sits here.  It is used by both our IP socket implementation and
 ** our base code.  It is modifiable by the user, as it defines the max length
 ** of the handles used by any ip implementation.
 **/
#define SYSF_IP_MAX_LAYER_HANDLE_LEN (sizeof(void *))
#define SYSF_IP_MAX_CLIENT_HANDLE_LEN (sizeof(void *))

	struct ncs_ip_request_info_tag;

	uns32 sysf_ip_request(struct ncs_ip_request_info_tag *ip_request);

	int ncssock_ipv6_send_msg(NCS_SOCKET_ENTRY * se, char *bufp, uns32 total_len,
					   struct sockaddr_in6 *saddr);

	uns32 ncssock_ipv6_rcv_msg(NCS_SOCKET_ENTRY * se, char *buf, uns32 buf_len,
					    struct sockaddr_in6 *p_saddr, NCS_IP_PKT_INFO * pkt_info);

#ifdef __NCSINC_OSE__

/* Async control Block */
	typedef struct ose_async_sock {
		struct ncs_socket_context_tag *sock_context;	/* It holds the socket context pointer */
		NCS_BOOL stop_flag;	/* If "TRUE" then the async task exit */
		SYSF_MBX ip_mail_box;	/* This holds the mail box */
		void *async_tsk_hdl;	/* async task handle */
		NCS_IPV4_ADDR remote_ip;	/* remote address to be bind */
		int protocol;	/* protocol to be bind with */
		uns32 port;	/* Local port to be bind with */
		uns32 sock_type;	/* socket type RAW IP / UDP */
		struct ncs_socket_entry_tag *sock_assoc;	/* pointer to the socket 
								   entry associated */
		SOCKET sock_desc;	/*socket handle async asociated with */
	} NCS_OSE_ASYNC_SOCK;

/* Async socket EVENT structure */
	typedef struct ose_ip_send_evt {
		uns32 evt_type;	/* this will tell what type of event we 
				   are posting */
		struct ncs_socket_entry_tag *sock_entry;	/* It holds the socket entry 
								   pointer */
		uns32 data_len;	/* length of the data */
		void *data;	/* this will hold received data */
	} NCS_IP_SEND_EVT;
#endif

#if (NCS_IPV6 == 1)
	typedef enum ipv6_proc_result {
		IPV6_ADDR_GREATER_THAN = 1,
		IPV6_ADDR_LESSER_THEN,
		IPV6_ADDR_NOT_EQUAL,
		IPV6_ADDR_EQUAL
	} IPV6_PROC_RESULT;

	NCS_BOOL ncs_ipv6_addr_is_zero(NCS_IPV6_ADDR * ipv6_scr_addr);
	uns32 ncs_ipv6_addr_compare(NCS_IPV6_ADDR * ipv6_scr_addr,
				    NCS_IPV6_ADDR * ipv6_dst_addr, IPV6_PROC_RESULT * res);
#endif

#ifdef  __cplusplus
}
#endif

#endif
