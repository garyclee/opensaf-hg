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

  MODULE NAME:  NCS_IPLIB.H

..............................................................................

  DESCRIPTION: Contains common IPV6 definitions, and utilities

******************************************************************************
*/

#ifndef _NCS_IPLIB_H
#define _NCS_IPLIB_H

#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#include "ncs_ipv4.h"
#include "ncs_ipv6.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            Declarations of structures and enum values
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* Constants aligned with Internet (IP) address type
   These values are defined in RFC 3291 
         unknown(0),
         ipv4(1),
         ipv6(2),
         ipv4z(3),
         ipv6z(4),
         dns(16)
*/
	typedef enum ncs_ip_addr_type {
		NCS_IP_ADDR_TYPE_NONE,
		NCS_IP_ADDR_TYPE_IPV4,
		NCS_IP_ADDR_TYPE_IPV6,
		NCS_IP_ADDR_TYPE_MAX	/* Must be last. */
	} NCS_IP_ADDR_TYPE;

/* AFI Values */
#define  NCS_AFI_IPV4    1
#define  NCS_AFI_IPV6    2

	typedef struct ncs_ip_addr {
		NCS_IP_ADDR_TYPE type;
		union {
			NCS_IPV4_ADDR v4;
#if (NCS_IPV6 == 1)
			NCS_IPV6_ADDR v6;
#endif
		} info;
	} NCS_IP_ADDR;

/* IP Prefix Defination */
	typedef struct ncs_ippfx {
		NCS_IP_ADDR ipaddr;	/* an IP Address                   */
		uns8 mask_len;	/* the bitmask that applies        */
	} NCS_IPPFX;

/* Macro to convert peer address from host order to network order 
   In this only IPv4 address is converted into host to network order
   IPv6 address is copied as it is */
#define m_NCS_HTON_IP_ADDR(i_addr, o_addr)                      \
{                                                              \
   if((o_addr) != (i_addr))                                    \
      memcpy((o_addr), (i_addr), sizeof(NCS_IP_ADDR));  \
   if((i_addr)->type == NCS_IP_ADDR_TYPE_IPV4)                  \
      (o_addr)->info.v4 = m_NCS_OS_HTONL((i_addr)->info.v4);    \
}                                                              \

/* Macro to convert peer address from network order to host order 
   In this only IPv4 address is converted into network to host order
   IPv6 address is copied as it is */
#define m_NCS_NTOH_IP_ADDR(i_addr, o_addr)                      \
{                                                              \
   if((o_addr) != (i_addr))                                    \
      memcpy((o_addr), (i_addr), sizeof(NCS_IP_ADDR));  \
   if((i_addr)->type == NCS_IP_ADDR_TYPE_IPV4)                  \
      (o_addr)->info.v4 = m_NCS_OS_NTOHL((i_addr)->info.v4);    \
}                                                              \

/* Macro to compare NCS_IP_ADDR 
   Pass the pointer to NCS_IP_ADDR is input */
#define m_NCS_CMP_IP_ADDR(i_addr1, i_addr2) ncs_cmp_ip_addr(i_addr1, i_addr2)

#define m_NCS_IS_IP_ADDR_ALL_ZERO(i_addr) ncs_is_ip_addr_all_zero(i_addr)\

#define m_NCS_IS_IP_ADDR_ZERO(i_addr) ncs_is_ip_addr_zero(i_addr)

	int32 ncs_cmp_ip_addr(NCS_IP_ADDR *addr1, NCS_IP_ADDR *addr2);

	NCS_BOOL ncs_is_ip_addr_all_zero(NCS_IP_ADDR *addr);

	NCS_BOOL ncs_is_ip_addr_zero(NCS_IP_ADDR *addr);

	uns16 ncs_ip_addr_to_octdata(uns8 *o_oct_data, NCS_IP_ADDR *i_addr, NCS_BOOL is_nwk_order);

	void ncs_oct_data_to_ip_addr(uns8 *i_oct_data, NCS_IP_ADDR *o_addr, NCS_BOOL is_nwk_order);
#ifdef  __cplusplus
}
#endif

#endif   /* _NCS_IPLIB_H */
