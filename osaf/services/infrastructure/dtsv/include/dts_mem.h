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

  This file contains macros for memory operations.
..............................................................................
*/

/*
 * Module Inclusion Control...
 */
#ifndef DTS_MEM_H
#define DTS_MEM_H

#include "ncssysf_mem.h"

#define DTSV_MEM_DTS_CB 1
#define DTS_MEM_DTSV_MSG  2
#define DTS_MEM_SVC_REG_TBL  3
#define DTS_MEM_VCARD_TBL 4
#define DTS_MEM_SVC_KEY   5
#define DTS_MEM_CIR_BUFFER  6
#define DTS_MEM_SEQ_BUFFER  7
/*New defines for SVC_ENTRY & DTA_ENTRY*/
#define DTS_MEM_SVC_ENTRY  8
#define DTS_MEM_DTA_ENTRY  9
#define DTS_MEM_LOG_FILE   10
#define DTS_MEM_SPEC_ENTRY 11
#define DTS_MEM_LIBNAME    12
#define DTS_MEM_CONS_DEV   13
/* Versioning support */
#define DTS_MEM_SVC_SPEC 14

#define m_MMGR_ALLOC_DTS_CB      (DTS_CB *)m_NCS_MEM_ALLOC(sizeof(DTS_CB), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTS_CB)

#define m_MMGR_FREE_DTS_CB(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTS_CB)

#define m_MMGR_ALLOC_SVC_REG_TBL   (DTS_SVC_REG_TBL *)m_NCS_MEM_ALLOC(sizeof(DTS_SVC_REG_TBL), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_SVC_REG_TBL)

#define m_MMGR_FREE_SVC_REG_TBL(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_SVC_REG_TBL)

#define m_MMGR_ALLOC_VCARD_TBL      (DTA_DEST_LIST *)m_NCS_MEM_ALLOC(sizeof(DTA_DEST_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_VCARD_TBL)

#define m_MMGR_FREE_VCARD_TBL(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_VCARD_TBL)

#define m_MMGR_ALLOC_DTS_LOG_FILE  (DTS_LL_FILE *)m_NCS_MEM_ALLOC(sizeof(DTS_LL_FILE), \
                                   NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_DTSV, DTS_MEM_LOG_FILE)

#define m_MMGR_FREE_DTS_LOG_FILE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_DTSV, DTS_MEM_LOG_FILE)

#define m_MMGR_ALLOC_DTS_SPEC_ENTRY (SYSF_ASCII_SPECS *)m_NCS_MEM_ALLOC(sizeof(SYSF_ASCII_SPECS), \
                                     NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_DTSV, DTS_MEM_SPEC_ENTRY)

#define m_MMGR_FREE_DTS_SPEC_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                      NCS_SERVICE_ID_DTSV, DTS_MEM_SPEC_ENTRY)

#define m_MMGR_ALLOC_DTS_LIBNAME   (ASCII_SPEC_LIB *)m_NCS_MEM_ALLOC(sizeof(ASCII_SPEC_LIB), \
                                    NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_DTSV, DTS_MEM_LIBNAME)

#define m_MMGR_FREE_DTS_LIBNAME(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_LIBNAME)

/* Added for console printing */
#define m_MMGR_ALLOC_DTS_CONS_DEV  (DTS_CONS_LIST *)m_NCS_MEM_ALLOC(sizeof(DTS_CONS_LIST), \
                                    NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_DTSV, DTS_MEM_CONS_DEV)

#define m_MMGR_FREE_DTS_CONS_DEV(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_DTSV, DTS_MEM_CONS_DEV)

/*Added macros for new SVC_ENTRY & DTA_ENTRY datastructures*/
#define m_MMGR_ALLOC_DTS_DTA_ENTRY (DTA_ENTRY *)m_NCS_MEM_ALLOC(sizeof(DTA_ENTRY), \
                                    NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_DTSV, \
                                    DTS_MEM_DTA_ENTRY)

#define m_MMGR_FREE_DTS_DTA_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                     NCS_SERVICE_ID_DTSV, DTS_MEM_DTA_ENTRY)

#define m_MMGR_ALLOC_DTS_SVC_ENTRY (SVC_ENTRY *)m_NCS_MEM_ALLOC(sizeof(SVC_ENTRY), \
                                   NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_DTSV, \
                                   DTS_MEM_SVC_ENTRY)

#define m_MMGR_FREE_DTS_SVC_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                     NCS_SERVICE_ID_DTSV, DTS_MEM_SVC_ENTRY)

/* Versioning support */
#define m_MMGR_ALLOC_DTS_SVC_SPEC (SPEC_ENTRY *)m_NCS_MEM_ALLOC(sizeof(SPEC_ENTRY), \
                                   NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_DTSV, \
                                   DTS_MEM_SVC_SPEC)

#define m_MMGR_FREE_DTS_SVC_SPEC(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                     NCS_SERVICE_ID_DTSV, DTS_MEM_SVC_SPEC)

#define m_MMGR_ALLOC_DTS_SVC_KEY   (SVC_KEY *)m_NCS_MEM_ALLOC(sizeof(SVC_KEY), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_SVC_KEY)

#define m_MMGR_FREE_DTS_SVC_KEY(p)  m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_SVC_KEY)

#define m_MMGR_ALLOC_CIR_BUFF(s)   m_NCS_MEM_ALLOC((s * sizeof(uint8_t)), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_CIR_BUFFER)

#define m_MMGR_FREE_CIR_BUFF(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_CIR_BUFFER)

#define m_MMGR_ALLOC_SEQ_BUFF(s)   (SEQ_ARRAY *)m_NCS_MEM_ALLOC((s * sizeof(SEQ_ARRAY)), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_SEQ_BUFFER)

#define m_MMGR_FREE_SEQ_BUFF(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTS_MEM_SEQ_BUFFER)

#endif
