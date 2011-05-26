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

..............................................................................

  DESCRIPTION:

  This module contains definitions for H&J Subsystem and Service ID's, and
  the NCS_SERVICE_DESCRIPTOR structure.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#ifndef NCS_SVD_H
#define NCS_SVD_H

#include "ncsgl_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

/************************************************************************

  H & J   S u b s y s t e m   or   S e r v i c e

                  I d e n t i f i e r s

*************************************************************************/

	typedef enum ncs_service_id {
  /** The H & J Subsystem and Service Identifiers
   **/
		NCS_SERVICE_ID_BASE = 0,
		NCS_SERVICE_ID_DUMMY = NCS_SERVICE_ID_BASE,
		NCS_SERVICE_ID_LEAP_TMR,
		NCS_SERVICE_ID_MDS,
		NCS_SERVICE_ID_COMMON,
		NCS_SERVICE_ID_OS_SVCS,
		NCS_SERVICE_ID_TARGSVCS,
		NCS_SERVICE_ID_SOCKET,
		NCS_SERVICE_ID_L2SOCKET,
		NCS_SERVICE_ID_DTSV,
		NCS_SERVICE_ID_AVSV,
		NCS_SERVICE_ID_AVD,
		NCS_SERVICE_ID_AVND,
		NCS_SERVICE_ID_AVA,
		NCS_SERVICE_ID_CLA,
		NCS_SERVICE_ID_GLD,
		NCS_SERVICE_ID_GLND,
		NCS_SERVICE_ID_GLA,
		NCS_SERVICE_ID_EDA,
		NCS_SERVICE_ID_EDS,
		NCS_SERVICE_ID_MQA,
		NCS_SERVICE_ID_MQND,
		NCS_SERVICE_ID_MQD,
		NCS_SERVICE_ID_CPA,
		NCS_SERVICE_ID_CPND,
		NCS_SERVICE_ID_CPD,
		NCS_SERVICE_ID_SAF_COMMON,
		NCS_SERVICE_ID_HCD,
		NCS_SERVICE_ID_HPL,
		NCS_SERVICE_ID_MBCSV,

		UD_SERVICE_ID_STUB1,
		UD_SERVICE_ID_STUB2,
		NCS_SERVICE_ID_VDS,
		NCS_SERVICE_ID_RDE,

		UD_DTSV_DEMO_SERVICE1,
		UD_DTSV_DEMO_SERVICE2,

		NCS_SERVICE_ID_LGA,	/* SAF LOG Service */
		NCS_SERVICE_ID_LGS,

		NCS_SERVICE_ID_FMA_GFM,
		NCS_SERVICE_ID_FMA,
		NCS_SERVICE_ID_GFM,

		NCS_SERVICE_ID_IMMA,
		NCS_SERVICE_ID_IMMND,
		NCS_SERVICE_ID_IMMD,

		NCS_SERVICE_ID_NTFA,
		NCS_SERVICE_ID_NTFS,

		NCS_SERVICE_ID_PLMA,
		NCS_SERVICE_ID_PLMS,

		NCS_SERVICE_ID_CLMA,
                NCS_SERVICE_ID_CLMS,

		NCS_SERVICE_ID_MAX,
		NCS_SERVICE_ID_END = 1000,

		NCS_SERVICE_ID_INTERNAL_BASE,
		/* All Motorola internal non-NCS service-ids go in this range */
		NCS_SERVICE_ID_INTERNAL_END = 2000,

		UD_SERVICE_ID_BASE,
		/* All Motorola external service-ids should come from this range */
		UD_SERVICE_ID_MAX,
		UD_SERVICE_ID_END = 3000
	} NCS_SERVICE_ID;

	typedef NCS_SERVICE_ID SS_SVC_ID;	/* Subsystem Service ID     */

/************************************************************************

  H & J   K e y

 An abstraction used to identify 'instance'. Its 'val' content can be
 attributed to a particular object (type) in a particular subsystem
 (svc). The value (val) serves as the qualifier that identifies 'which
 instance' of this <svc>.<type> object we are referring to.

*************************************************************************/

#define SYSF_MAX_KEY_LEN  42	/* picked as 'big enough' so far */

	typedef struct ncs_octstr {
		uint8_t len;
		uint8_t data[SYSF_MAX_KEY_LEN];
	} NCS_OCTSTR;

	typedef struct ncs_key {
		NCS_SERVICE_ID svc;	/* an object based in this service */
		uint8_t type;	/* type or component of service   */
		uint8_t fmat;	/* value format NUM|HDL|STR|OCT   */

		union {
			uns32 num;
			NCSCONTEXT hdl;
			NCS_OCTSTR oct;
			uint8_t str[SYSF_MAX_KEY_LEN];	/* null terminated string       */
		} val;

	} NCS_KEY;

/* NCS_KEY::fmat values */

#define NCS_FMT_EMPTY 0
#define NCS_FMT_NUM   1
#define NCS_FMT_HDL   2
#define NCS_FMT_STR   3
#define NCS_FMT_OCT   4

/* put key in start state */

#define m_SET_EMPTY_KEY(k)         { k.svc=NCS_SERVICE_ID_DUMMY;k.type=0; \
                                     k.fmat=NCS_FMT_EMPTY;k.val.num=0; }

/* test for unoccupied key */

#define m_IS_KEY_EMPTY(k)          (k.fmat==NCS_FMT_EMPTY) ? TRUE:FALSE

/* 32 bit Number */

#define m_SET_NUM_KEY(k,s,t,v)     { k.svc=s;k.type=t; \
                                     k.fmat=NCS_FMT_NUM;k.val.num=v; }
#define m_GET_NUM_KEY(k,n)         { m_KEY_CHK_FMT(k,NCS_FMT_NUM); \
                                     n = (uns32)k.val.num; }

/* Handle (NCSCONTEXT) */

#define m_SET_HDL_KEY(k,s,t,v)     { k.svc=s;k.type=t; \
                                     k.fmat=NCS_FMT_HDL;k.val.hdl=v; }
#define m_GET_HDL_KEY(k,h)         { m_KEY_CHK_FMT(k,NCS_FMT_HDL); \
                                     h = k.val.hdl; }

/* NULL terminated String */

#define m_SET_STR_KEY(k,s,t,v)     { m_KEY_CHK_SLEN(v); k.svc=s;k.type=t; \
                                     k.fmat=NCS_FMT_STR;  \
                                     strcpy((char*)k.val.str,(const char*)v); }

#define m_GET_STR_KEY(k,s)         { m_KEY_CHK_FMT(k,NCS_FMT_STR);s=k.val.str; }

/* OctetString of <length> bytes */

#define m_SET_OCT_KEY(k,s,t,l,v)   { m_KEY_CHK_LEN(l); k.svc=s;k.type=t;     \
                                     k.fmat=NCS_FMT_OCT; k.val.oct.len = l;   \
                                     memcpy((void*)(&k.val.oct.data),(const void *)v,l); }

#define m_GET_OCT_KEY(k,o)         { m_KEY_CHK_FMT(k,NCS_FMT_OCT); o = &k.val.oct; }

/************************************************************************

  H & J   S u b s y s t e m   N a m e

 This data type is used as an identity scheme for naming. It is currently
 used in redundancy, were the PRIMARY and BACKUP instances of a redundant
 entity must go by the same name, but set to opposite roles.

NOTE: is designed to be small (32 bits). Don't mess with this!!!
*************************************************************************/

	typedef struct ftcc_name {
		uint8_t ncs_ss;	/* entity type/subsystem; an NCS_SERVICE_ID value   */
		uint8_t ncs_qual;	/* entity qualifier (ex. SIG = UNI31_NET, PNNI,etc. */
		uns16 ncs_inst;	/* some assigned value                             */
	} NCSSS_NAME;

/************************************************************************

  H & J   L o c k  D e s c r i p t o r

 This data type is used as a means of qualifying who is asking for a lock.
 It is used in Signalling. Its intent is to provide enough lock qualifying
 information so that an ambitious customer can decide which locks it
 actually wants to use and which are deemed 'low risk' and choose to ignore.

 If (some) customers didn't ask for it, it would not be done.
*************************************************************************/

	typedef struct ncs_lock_desc {
		NCS_SERVICE_ID subsystem_id;	/* Subsystem specific owner of this lock */
		uns32 lock_id;	/* maps to a subsys specific lock identifier */
		uns32 lock_type;	/* This is one of NCS_LOCK_READ or NCS_LOCK_WRITE */
	} NCS_LOCK_DESC;

#ifdef  __cplusplus
}
#endif

#endif
