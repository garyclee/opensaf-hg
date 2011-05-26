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
  DESCRIPTION:

  This module contains data type defs usable throughout the system.
  Inclusion of this module into all C-source modules is strongly
  recommended.

******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef NCSGL_DEFS_H
#define NCSGL_DEFS_H

#include <inttypes.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*****************************************************************************
 
                             BASIC DECLARATIONS

 ****************************************************************************/

	typedef uint8_t uns8;	/*  8-bit */
	typedef uint16_t uns16;	/* 16-bit */
	typedef uint32_t uns32;	/* 32-bit */
	typedef uint64_t uns64;	/* 64-bit */

	typedef int8_t int8;
	typedef int16_t int16;
	typedef int32_t int32;
	typedef int64_t int64;

	typedef bool NCS_BOOL;	/* move to this solves BOOLEAN problem */

	typedef void* NCSCONTEXT;	/* opaque context between svc-usr/svc-provider */

#define NCS_PTR_TO_INT32_CAST(x)   ((int32)(long)(x))
#define NCS_PTR_TO_UNS64_CAST(x)   ((uns64)(long)(x))
#define NCS_PTR_TO_UNS32_CAST(x)   ((uns32)(long)(x))
#define NCS_INT32_TO_PTR_CAST(x)   ((void*)(long)(x))
#define NCS_INT64_TO_PTR_CAST(x)   ((void*)(long)(x))
#define NCS_UNS32_TO_PTR_CAST(x)   ((void*)(long)(x))

	typedef uns32 NCS_IPV4_ADDR;
#if (NCS_IPV6 == 1)

#define NCS_IPV6_ADDR_UNS8_CNT    16	/* Number of uns8's in an IPv6 address */
#define NCS_IPV6_ADDR_UNS16_CNT   8	/* Number of uns16's in an IPv6 address */
#define NCS_IPV6_ADDR_UNS32_CNT   4	/* Number of uns32's in an IPv6 address */

	typedef struct ncs_ipv6_addr_tag {
		union {
			uns8 ipv6_addr8[NCS_IPV6_ADDR_UNS8_CNT];
			uns16 ipv6_addr16[NCS_IPV6_ADDR_UNS16_CNT];
			uns32 ipv6_addr32[NCS_IPV6_ADDR_UNS32_CNT];
		} ipv6;
	} NCS_IPV6_ADDR;

#define m_ipv6_addr              ipv6.ipv6_addr8
#define m_ipv6_addr16            ipv6.ipv6_addr16
#define m_ipv6_addr32            ipv6.ipv6_addr32
#define m_IPV6_ADDR_SIZE         sizeof (NCS_IPV6_ADDR)
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        Manifest Constants

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#ifndef TRUE
#define TRUE    true
#endif

#ifndef FALSE
#define FALSE   false
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                      Generic Function Return Codes                     

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/** Return Codes shared across the entire H&J product line...
    Reserved Area: 0-thru-1023
 **/

#define NCSCC_RC_SUCCESS               1
#define NCSCC_RC_FAILURE               2
#define NCSCC_RC_BAD_ATTR              7
#define NCSCC_RC_INV_VAL              16
#define NCSCC_RC_NO_OBJECT            18
#define NCSCC_RC_OUT_OF_MEM           21
#define NCSCC_RC_DISABLED            127
#define NCSCC_RC_TMR_STOPPED         128
#define NCSCC_RC_REQ_TIMOUT          131
#define NCSCC_RC_NO_CREATION         134
#define NCSCC_RC_INVALID_INPUT       137
#define NCSCC_RC_CONTINUE           1023
#define NCSCC_RC_DUPLICATE_ENTRY    2011

/*************************************************
 * Maximum Slots (Including sub slots) supported
 * 16 slots x 8 subslots
 *************************************************/
#define NCS_SLOT_MAX 16

#define NCS_SUB_SLOT_MAX 16

#define NCS_MAX_SLOTS ((NCS_SLOT_MAX *  NCS_SUB_SLOT_MAX) + NCS_SLOT_MAX)

/***************************************************************************
 * The IP address definations are removed from this file, and moved into the 
 * followng include files.
 * Including this files here is a work around to avoide compilation errors in 
 * all subsytems that are using IP address definations. Going forward all 
 * subsystems need to include the following include files seperately along with
 * gl_defs.h, and remove the include defenations from this file 
 *****************************************************************************/

#define NCS_MDS  1

#define NCS_IPV6 0

	typedef uns64 MDS_DEST;

	typedef uns32 NCS_NODE_ID;
	typedef uns8 NCS_CHASSIS_ID;
	typedef uns8 NCS_PHY_SLOT_ID;
	typedef uns8 NCS_SUB_SLOT_ID;

/* m_NCS_NODE_ID_FROM_MDS_DEST: Returns node-id if the MDS_DEST provided
                                is an absolute destination. Returns 0
                                if the MDS_DEST provided is a virtual
                                destination.
*/
#define m_NCS_NODE_ID_FROM_MDS_DEST(mdsdest) ((uns32) (((uns64)(mdsdest))>>32))

#ifdef  __cplusplus
}
#endif

#endif   /* ifndef NCSGL_DEFS_H */
