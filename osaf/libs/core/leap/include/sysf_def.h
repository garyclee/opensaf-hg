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

  This module contains target system specific declarations related to
  System "hooks" and other assorted defines.

..............................................................................
*/

/*
 * Module Inclusion Control...
 */
#ifndef SYSF_DEF_H
#define SYSF_DEF_H

#include "ncssysf_def.h"
#include "ncssysf_mem.h"

struct ncs_ipaddr_entry;
struct ncsxlim_if_rec;

#define m_OSSVC_MMGR_ALLOC_NCSIPAE (struct ncs_ipaddr_entry*)m_NCS_MEM_ALLOC(sizeof(struct ncs_ipaddr_entry), \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_COMMON, \
                                                 0)

#define m_OSSVC_MMGR_FREE_NCSIPAE(p)   m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_COMMON, \
                                                 0)

#define m_OSSVC_MMGR_ALLOC_NCSXIR (struct ncsxlim_if_rec*)m_NCS_MEM_ALLOC(sizeof(struct ncsxlim_if_rec), \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_COMMON, \
                                                 1)

#define m_OSSVC_MMGR_FREE_NCSXIR(p)   m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_COMMON, \
                                                 1)

#define m_OSSVC_MMGR_ALLOC_NCSSVENT (struct ncs_svcard_entry*)m_NCS_MEM_ALLOC(sizeof(struct ncs_svcard_entry), \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_COMMON, \
                                                 2)

#define m_OSSVC_MMGR_FREE_NCSSVENT(p)   m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_COMMON, \
                                                 2)

/** Imcomplete structure definitions - required by some compilers.
 **/
struct ncscc_call_data;
struct ncscc_conn_id;
struct ncs_sar_ctrl_info_tag;

#define m_NATIVE_PORT_NUM(x)    (unsigned int)((x)->native_ifcb)

/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 **                                                                        **
 **                                                                        **
 **                  Cache Allocation Interface                            **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ***************************************************************************/

#define m_NCS_CACHE_PREALLOC_ENTRIES(c)               NCSCC_RC_SUCCESS
#define m_NCS_CACHE_FREE_PREALLOC_ENTRIES(c)

#if (NCS_CACHING == 1)
/* Generic Cache Macros */

#define m_MMGR_ALLOC_CACHE  \
    (CACHE *)m_NCS_MEM_ALLOC(sizeof(CACHE), \
                            NCS_MEM_REGION_PERSISTENT, \
                            NCS_SERVICE_ID_OS_SVCS, 5)
#define m_MMGR_ALLOC_HASH_TBL(s)  \
    (CACHE_ENTRY *)m_NCS_MEM_ALLOC(sizeof(CACHE_ENTRY) * s, \
                                  NCS_MEM_REGION_PERSISTENT, \
                                  NCS_SERVICE_ID_OS_SVCS, 6)

#define m_MMGR_FREE_CACHE(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_OS_SVCS, 5)
#define m_MMGR_FREE_HASH_TBL(p)  m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_OS_SVCS, 6)

#define m_MMGR_FREE_CACHE_ENTRY(p)  m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT \
                                                  NCS_SERVICE_ID_OS_SVCS, 8)
#endif

#define m_MMGR_ALLOC_NCS_STREE_ENTRY \
      (NCS_STREE_ENTRY*)m_NCS_MEM_ALLOC(sizeof(NCS_STREE_ENTRY), \
                                      NCS_MEM_REGION_PERSISTENT, \
                                      NCS_SERVICE_ID_OS_SVCS, 7)
#define m_MMGR_FREE_NCS_STREE_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                                                    NCS_SERVICE_ID_OS_SVCS, 7)

/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 **                                                                        **
 **                                                                        **
 **                 Pointer Queueing Allocation Interface                  **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ***************************************************************************/

#define m_MMGR_ALLOC_NCS_QLINK  \
      (NCS_QLINK*)m_NCS_MEM_ALLOC(sizeof(NCS_QLINK), \
                                      NCS_MEM_REGION_PERSISTENT, \
                                      NCS_SERVICE_ID_OS_SVCS, 9)

#define m_MMGR_FREE_NCS_QLINK(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,\
                                              NCS_SERVICE_ID_OS_SVCS, 9)

/* Encode/Decode(EDU) Malloc-Free macros */
#define m_MMGR_ALLOC_EDP_ENTRY  m_NCS_MEM_ALLOC(sizeof(EDP_ENTRY),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 10)

#define m_MMGR_FREE_EDP_ENTRY(p)    \
            m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                            NCS_SERVICE_ID_OS_SVCS, 10)

#define m_MMGR_ALLOC_EDU_PPDB_NODE_INFO  m_NCS_MEM_ALLOC(sizeof(EDU_PPDB_NODE_INFO),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 11)

#define m_MMGR_FREE_EDU_PPDB_NODE_INFO(p)    \
            m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                            NCS_SERVICE_ID_OS_SVCS, 11)

#define m_MMGR_ALLOC_EDP_TEST_INSTR_REC  m_NCS_MEM_ALLOC(sizeof(EDP_TEST_INSTR_REC),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 12)

#define m_MMGR_FREE_EDP_TEST_INSTR_REC(p)    \
            m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                            NCS_SERVICE_ID_OS_SVCS, 12)

#define m_MMGR_ALLOC_EDP_LABEL_ELEMENT  m_NCS_MEM_ALLOC(sizeof(EDP_LABEL_ELEMENT),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 13)

#define m_MMGR_FREE_EDP_LABEL_ELEMENT(p)    \
            m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                            NCS_SERVICE_ID_OS_SVCS, 13)

#define m_MMGR_ALLOC_EDU_HDL_NODE       m_NCS_MEM_ALLOC(sizeof(EDU_HDL_NODE),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 14)

#define m_MMGR_FREE_EDU_HDL_NODE(p)    \
            m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                            NCS_SERVICE_ID_OS_SVCS, 14)

#define m_MMGR_ALLOC_EDP_SELECT_ARRAY(cnt)  m_NCS_MEM_ALLOC((cnt*sizeof(int)),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 15)

#define m_MMGR_FREE_EDP_SELECT_ARRAY(p)    \
            m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT, \
                            NCS_SERVICE_ID_OS_SVCS, 15)

/* EDU's EDP malloc calls. */
/* List of basic types :
    - char      (C-type)
    - short     (C-type)
    - int       (C-type)
    - long      (C-type)
    - double    (C-type. Can be made uns32 if not supported by compiler)
    - float     (C-type)
    - ncs_bool  (NCS-type, unsigned int.)
    - uint8_t      (NCS-type. unsigned char.)
    - uns16     (NCS-type. unsigned short.)
    - uns32     (NCS-type. unsigned int.)
    - int8_t      (NCS-type. signed char.)
    - int16     (NCS-type. signed short.)
    - int32     (NCS-type. signed long.)
    - ncsfloat  (C/NCS-type. float.)
    - octet     (C/NCS-type, uint8_t *)
 */
#define m_MMGR_ALLOC_EDP_NCS_BOOL   m_NCS_MEM_ALLOC(sizeof(NCS_BOOL),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_CHAR(n)  m_NCS_MEM_ALLOC(n * sizeof(char),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_SHORT  m_NCS_MEM_ALLOC(sizeof(short),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_INT    m_NCS_MEM_ALLOC(sizeof(int),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_LONG   m_NCS_MEM_ALLOC(sizeof(long),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_DOUBLE   m_NCS_MEM_ALLOC(sizeof(double),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_FLOAT    m_NCS_MEM_ALLOC(sizeof(float),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_UNS8(n)   m_NCS_MEM_ALLOC(n*sizeof(uint8_t),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_UNS16  m_NCS_MEM_ALLOC(sizeof(uns16),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_UNS32  m_NCS_MEM_ALLOC(sizeof(uns32),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_INT8   m_NCS_MEM_ALLOC(sizeof(int8_t),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_INT16  m_NCS_MEM_ALLOC(sizeof(int16),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_INT32  m_NCS_MEM_ALLOC(sizeof(int32),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

#define m_MMGR_ALLOC_EDP_NCSFLOAT32 m_NCS_MEM_ALLOC(sizeof(float),\
            NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_OS_SVCS, 0)

/*
 * m_LEAP_FAILURE
 *
 * If LEAP detects an error that just must be tended to, it will
 * Call this macro. The associated function can be taken over by
 * customers if they don't like NetPlane's default treatment (like
 * forcing a crash).
 *
 * Events are.............
 */
#define NCSFAIL_DOUBLE_DELETE     1
#define NCSFAIL_FREEING_NULL_PTR  2
#define NCSFAIL_MEM_SCREWED_UP    3
#define NCSFAIL_FAILED_BM_TEST    4
#define NCSFAIL_MEM_NULL_POOL     5
#define NCSFAIL_MEM_REC_CORRUPTED 6
#define NCSFAIL_OWNER_CONFLICT    7

uns32 leap_failure(uns32 l, char *f, uns32 e, uns32 ret);

#define m_LEAP_FAILURE(e,r) leap_failure(__LINE__,__FILE__,(uns32)e,(uns32)r)

#endif
