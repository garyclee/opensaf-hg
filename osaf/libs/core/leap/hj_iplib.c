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

  MODULE NAME: NCS_IPV6.C

..............................................................................

  DESCRIPTION: Contain library functions for handling IPv6 addresses. 

******************************************************************************
*/

#include <ncsgl_defs.h>
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncs_iplib.h"

int32 ncs_cmp_ip_addr(NCS_IP_ADDR *addr1, NCS_IP_ADDR *addr2)
{
	if (addr1->type < addr2->type)
		return -1;
	else if (addr1->type > addr2->type)
		return 1;
	else {
		switch (addr1->type) {
		case NCS_IP_ADDR_TYPE_IPV4:
			if (addr1->info.v4 < addr2->info.v4)
				return -1;
			else if (addr1->info.v4 > addr2->info.v4)
				return 1;
			else
				return 0;
			break;
#if(NCS_IPV6 == 1)
		case NCS_IP_ADDR_TYPE_IPV6:
			return (m_NCS_CMP_IPV6_ADDR(&addr1->info.v6, &addr2->info.v6));
			break;
#endif
		default:
			break;
		}
	}
	return -1;
}

void ncs_ip_addr_to_display_str(NCS_IP_ADDR *i_addr, int8 *o_str)
{
	NCS_IPV4_ADDR ipaddr = 0;

	switch (i_addr->type) {
	case NCS_IP_ADDR_TYPE_IPV4:
		ipaddr = i_addr->info.v4;
		sprintf(o_str, "%d.%d.%d.%d", ((ipaddr >> 24) & 0xFF),
			((ipaddr >> 16) & 0xFF), ((ipaddr >> 8) & 0xFF), (ipaddr) & 0xFF);
		break;
#if(NCS_IPV6 == 1)
	case NCS_IP_ADDR_TYPE_IPV6:
		m_NCS_IPV6ADDR_TO_STR(&i_addr->info.v6, o_str);
		break;
#endif   /* NCS_IPV6 == 1 */
	default:
		return;
		break;
	}
}

/**************************************************************************
 Function: ncs_ip_addr_to_octdata

 Purpose: This function will convert the IP_ADDR to octet data.

 Input:    
   uns8        *o_oct_data    : IP address in the form of octet data
                                in nwk order
   NCS_IP_ADDR  *i_addr        : IP address (assuming that "type" is 
                                already filled.
   NCS_BOOL     is_nwk_order   : Byte order of IP address (Applicable only for
                                IPv4 address.)

  Return : Length of the IP address in octets.

 Notes:  
**************************************************************************/
uns16 ncs_ip_addr_to_octdata(uns8 *o_oct_data, NCS_IP_ADDR *i_addr, NCS_BOOL is_nwk_order)
{
	NCS_IPV4_ADDR *v4_addrNet = 0;
	uns16 len = 0;

	switch (i_addr->type) {
	case NCS_IP_ADDR_TYPE_IPV4:
		v4_addrNet = (NCS_IPV4_ADDR *)o_oct_data;
		if (is_nwk_order)
			*v4_addrNet = i_addr->info.v4;
		else
			*v4_addrNet = m_NCS_OS_HTONL(i_addr->info.v4);
		len = 4;
		break;
#if(NCS_IPV6 == 1)
	case NCS_IP_ADDR_TYPE_IPV6:
		memcpy(o_oct_data, &i_addr->info.v6.m_ipv6_addr, NCS_IPV6_ADDR_UNS8_CNT);
		len = NCS_IPV6_ADDR_UNS8_CNT;
		break;
#endif
	default:
		break;
	}

	return len;
}

void ncs_oct_data_to_ip_addr(uns8 *i_oct_data, NCS_IP_ADDR *o_addr, NCS_BOOL is_nwk_order)
{

	/* This routine  will assume that the IP_ADDR_TYPE is already
	   filled in o_addr */

	switch (o_addr->type) {
	case NCS_IP_ADDR_TYPE_IPV4:
		o_addr->info.v4 = (uns32)((*((uns32 *)i_oct_data)));
		if (!is_nwk_order)
			o_addr->info.v4 = m_NCS_OS_NTOHL(o_addr->info.v4);
		break;
#if(NCS_IPV6 == 1)
	case NCS_IP_ADDR_TYPE_IPV6:
		memcpy(&o_addr->info.v6.m_ipv6_addr, i_oct_data, NCS_IPV6_ADDR_UNS8_CNT);
		break;
#endif
	default:
		break;
	}

	return;
}

NCS_BOOL ncs_is_ip_addr_all_zero(NCS_IP_ADDR *addr)
{
	NCS_IP_ADDR lcl_addr;

	memset(&lcl_addr, '\0', sizeof(NCS_IP_ADDR));
	if (memcmp((char *)addr, (char *)&lcl_addr, sizeof(NCS_IP_ADDR))
	    == 0) {
		return TRUE;
	}
	return FALSE;
}

NCS_BOOL ncs_is_ip_addr_zero(NCS_IP_ADDR *addr)
{
	NCS_IP_ADDR lcl_addr;
	memset(&lcl_addr, 0, sizeof(NCS_IP_ADDR));

	lcl_addr.type = addr->type;
	if (memcmp((char *)addr, (char *)&lcl_addr, sizeof(NCS_IP_ADDR))
	    == 0) {
		return TRUE;
	}
	return FALSE;
}
