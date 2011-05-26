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

  MODULE NAME:  NCS_IPV6.H

..............................................................................

  DESCRIPTION: Contains common IPV6 definitions, and utilities

******************************************************************************
*/

#ifndef _NCS_IPV6_H
#define _NCS_IPV6_H

#ifdef  __cplusplus
extern "C" {
#endif

#if (NCS_IPV6 == 1)

/* Maximum size of the IPv6 display string array */
#define IPV6_DISPLAY_STR_MAX_SIZE   40

	NCS_BOOL ncs_is_ipv6_addr_all_zero(NCS_IPV6_ADDR * addr);
#define m_NCS_IS_IPV6_ADDR_ALL_ZERO(v6addr_ptr) \
        ncs_is_ipv6_addr_all_zero(v6addr_ptr)

#define m_NCS_CMP_IPV6_ADDR(addr_ptr1, addr_ptr2) \
        memcmp((char*)(addr_ptr1), (char*)(addr_ptr2), sizeof(NCS_IPV6_ADDR))

	NCS_BOOL ncs_is_valid_ipv6_uc(NCS_IPV6_ADDR * addr, uns8 bits);
#define m_NCS_IS_VALID_IPV6_UC(addr, bits) ncs_is_valid_ipv6_uc(addr, bits)

	void ncs_mask_prefix_addr(NCS_IPV6_ADDR * addr, uns32 pfxlen, NCS_IPV6_ADDR * o_masked_addr);
#define m_NCS_MASK_IPV6_PREFIX_ADDR(v6addr_ptr, pfxlen, o_masked_v6addr_ptr) \
        ncs_mask_prefix_addr(v6addr_ptr, pfxlen, o_masked_v6addr_ptr)

	void ncs_str_to_ipv6addr(NCS_IPV6_ADDR * o_v6addr, int8 *i_str);
#define m_NCS_STR_TO_IPV6_ADDR(o_v6addr, i_str) \
        ncs_str_to_ipv6addr(o_v6addr, i_str)

	void ncs_str_to_ipv6pfx(NCS_IPV6_ADDR * o_v6addr, uns32 *o_pfx_len, int8 *str);
#define m_NCS_STR_TO_IPV6_PREFIX(o_v6addr, o_pfx_len, i_str) \
        ncs_str_to_ipv6pfx(o_v6addr, o_pfx_len, i_str)

	void ncs_ipv6_str_from_ipv6addr(NCS_IPV6_ADDR * i_v6addr, int8 *o_str);
#define m_NCS_IPV6ADDR_TO_STR(i_v6addr, o_str) \
        ncs_ipv6_str_from_ipv6addr(i_v6addr, o_str)

	NCS_BOOL ncs_is_ipv6_linkloc_addr(NCS_IPV6_ADDR * v6addr_ptr);

	NCS_BOOL ncs_is_ipv6_6to4_addr(NCS_IPV6_ADDR * v6addr_ptr);

#define m_NCS_IS_IPV6_LINKLOC_ADDR(v6addr_ptr) ncs_is_ipv6_linkloc_addr(v6addr_ptr)

#define m_NCS_IS_6TO4_ADDR(v6addr_ptr) ncs_is_ipv6_6to4_addr(v6addr_ptr)
#endif   /* #if (NCS_IPV6 == 1) */

#ifdef  __cplusplus
}
#endif

#endif   /* _NCS_IPV6_H */
