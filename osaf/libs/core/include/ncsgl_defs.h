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

#define NCSCC_RC_SUCCESS               1	/* All went well -success           */
#define NCSCC_RC_FAILURE               2	/* Unspecified failure              */

#define NCSCC_RC_UNK_CONTEXT           3
#define NCSCC_RC_BUSY_PROF             4	/* Profile is in use                */
#define NCSCC_RC_UNK_PROF              5	/* Unknown User Profile             */
#define NCSCC_RC_UNK_ATTR              6	/* Unknown profile attribute type   */
#define NCSCC_RC_BAD_ATTR              7	/* Attribute has bad format         */
#define NCSCC_RC_NO_ATTR               8	/* Attribute not present            */
#define NCSCC_RC_UNK_OP                9	/* Operation type unknown           */

#define NCSCC_RC_CALL_ACCEPT          11
#define NCSCC_RC_CALL_RELEASE         12
#define NCSCC_RC_CALL_PEND            13

#define NCSCC_RC_SKIP_MSG             14
#define NCSCC_RC_CONT_MSG             15

#define NCSCC_RC_INV_VAL              16
#define NCSCC_RC_INV_SPECIFIC_VAL     17

#define NCSCC_RC_NO_OBJECT            18
#define NCSCC_RC_NO_INSTANCE          19
#define NCSCC_RC_NOSUCHNAME           20

#define NCSCC_RC_OUT_OF_MEM           21	/* Out of memory                    */
#define NCSCC_RC_CALL_GATED           22	/* NCSCC_CALL_DATA passed on         */
#define NCSCC_RC_SIG_FREE_CD          23	/* Signalling alloc'ed the call data */
											/* Signalling should free it        */

#define NCSCC_RC_SIG_CLEARS_ALL       24	/* Signalling drives iface clearing */
#define NCSCC_RC_USR_CLEARS_ALL       25	/* Application drives iface clearing */
#define NCSCC_RC_EACH_CLEARS_OWN      26	/* each (sig/app) clears own world  */

#define NCSCC_RC_CAPD_NOT_CHANGED     31	/* Attachment point not changed     */
#define NCSCC_RC_CAPD_CHANGED         32	/* Attachment point changed         */
#define NCSCC_RC_NOT_SUPPORTED        33	/* Request not supported            */

#define NCSCC_RC_INTF_NOT_UP          34
#define NCSCC_RC_DUPLICATE_FLTR       35
#define NCSCC_RC_INVALID_PING_TIMEOUT 36

#define NCSCC_RC_ENQUEUED            125
#define NCSCC_RC_INOPPORTUNE_REQUEST 126
#define NCSCC_RC_DISABLED            127

#define NCSCC_RC_TMR_STOPPED         128
#define NCSCC_RC_TMR_DESTROYED       129

#define NCSCC_RC_NO_SUCH_TBL         130
#define NCSCC_RC_REQ_TIMOUT          131

#define NCSCC_RC_NO_TO_SVC           132

#define NCSCC_RC_NO_ACCESS           133
#define NCSCC_RC_NO_CREATION         134
#define NCSCC_RC_NO_MAS              135
#define NCSCC_RC_PLBK_IN_PROGRESS    136
#define NCSCC_RC_INVALID_INPUT       137	/* Input is invalid */
#define NCSCC_RC_NOT_WRITABLE        138
/* NCSCC_RC_NOT_WRITABLE is mapped to SNMP_ERR_NOTWRITABLE. If the variable binding's name specifies a variable
   which exists but can not be modified no matter what new value is specified, then the value of the Response-PDU's 
   error-status field is set to `notWritable' (set on read-only, not-accessible objects returns this error)
*/

#define NCSCC_RC_CONTINUE           1023

/* MPOA Return Codes */

#define NCSCC_RC_DUPLICATE_ENTRY                2011
#define NCSCC_RC_TOO_MANY_MPS                   2012
#define NCSCC_RC_BAD_LEC                        2013
#define NCSCC_RC_ENCAP_TOO_LONG                 2014
#define NCSCC_RC_TOO_MANY_PROTOS                2015
#define NCSCC_RC_VCC_RCV_BUSY                   2016
#define NCSCC_RC_PDU_RUNT                       2017
#define NCSCC_RC_PDU_TOO_BIG                    2018
#define NCSCC_RC_UNSUPPORTED_PROTOCOL           2019
#define NCSCC_RC_DROP_PACKET                    2020
#define NCSCC_RC_SEND_VIA_LANE                  2021
#define NCSCC_RC_PKT_TOO_SHORT                  2022
#define NCSCC_RC_PKT_NOT_NHRP                   2023
#define NCSCC_RC_NHRP_BAD_PKT_TYPE              2024

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                      Product Specific Function Return Code Bases

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define NCSCC_RC_SSS_BASE      1025	/* up to  2048 */
#define NCSCC_RC_ILMI_BASE     2049	/* up to  2399 */
#define NCSCC_RC_FRS_BASE      2400	/* up to  3072 */
#define NCSCC_RC_LEC_BASE      3073	/* up to  4096 */
#define NCSCC_RC_IPOA_BASE     4097	/* up to  5120 */
#define NCSCC_RC_NCSLES_BASE   5121	/* up to  6144 */
#define NCSCC_RC_FRF5_BASE     6145	/* up to  7168 */
#define NCSCC_RC_PNNI_BASE     7169	/* up to  8192 */
#define NCSCC_RC_FRF8_BASE     8193	/* up to  9216 */
	/* unexplained gap here! */
#define NCSCC_RC_CMS_BASE     11265	/* up to 12288 */
#define NCSCC_RC_LTCS_BASE    12289	/* up to 13312 */
#define NCSCC_RC_RMS_BASE     13313	/* up to 14336 */
#define NCSCC_RC_MDS_BASE     14337	/* up to 15360 */
#define NCSCC_RC_BGP_BASE     15361	/* up to 15450 */
#define NCSCC_RC_VPN_BASE     15451	/* up to 15500 */
#define NCSCC_RC_IGMP_BASE    15501	/* up to 15550 */
#define NCSCC_RC_PIM_BASE     15551	/* up to 15600 */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                    MACROS

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

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
