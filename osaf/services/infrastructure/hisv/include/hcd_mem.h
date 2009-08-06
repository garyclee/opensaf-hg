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

  This file contains macros for memory operations for HCD.

*****************************************************************************/

#ifndef HCD_MEM_H
#define HCD_MEM_H

/* Service Sub IDs for HISv - HAM */
typedef enum {
	NCS_SERVICE_HAM_CB = 1,
	NCS_SERVICE_HISV_MSG,
	NCS_SERVICE_HSM_CB,
	NCS_SERVICE_SIM_CB,
	NCS_SERVICE_HISV_DATA,
	NCS_SERVICE_HSM_INV_DATA,
	NCS_SERVICE_HISV_EVT,
	NCS_SERVICE_SIM_EVT,
	NCS_SERVICE_HPI_DOM_ARGS,
	NCS_SERVICE_HAM_ADEST
} NCS_SERVICE_HAM_SUBID;

/****************************************
 * Memory Allocation and Release Macros *
 ***************************************/

#define m_MMGR_ALLOC_HISV_HCD_CB      (HCD_CB *)m_NCS_MEM_ALLOC(sizeof(HCD_CB),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HAM_CB)

#define m_MMGR_FREE_HISV_HCD_CB(p)    m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HAM_CB)

#define m_MMGR_ALLOC_HAM_CB           (HAM_CB *)m_NCS_MEM_ALLOC(sizeof(HAM_CB),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HAM_CB)

#define m_MMGR_FREE_HAM_CB(p)         m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HAM_CB)

#define m_MMGR_ALLOC_HISV_EVT         (HISV_EVT *)m_NCS_MEM_ALLOC(sizeof(HISV_EVT),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HISV_EVT)

#define m_MMGR_FREE_HISV_EVT(p)       m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HISV_EVT)

#define m_MMGR_ALLOC_HAM_ADEST        (HAM_ADEST_LIST *)m_NCS_MEM_ALLOC(sizeof(HAM_ADEST_LIST),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HAM_ADEST)

#define m_MMGR_FREE_HAM_ADEST(p)       m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HAM_ADEST)

#define m_MMGR_ALLOC_SIM_EVT         (SIM_EVT *)m_NCS_MEM_ALLOC(sizeof(SIM_EVT),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_SIM_EVT)

#define m_MMGR_FREE_SIM_EVT(p)       m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_SIM_EVT)

#define m_MMGR_ALLOC_HISV_DATA(n)     (uns8 *)m_NCS_MEM_ALLOC((n),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HISV_DATA)

#define m_MMGR_FREE_HISV_DATA(p)       if ((p) != NULL) \
                                           m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HISV_DATA)

#define m_MMGR_ALLOC_HPI_SESSION_ARGS     (HPI_SESSION_ARGS *)m_NCS_MEM_ALLOC(sizeof(HPI_SESSION_ARGS),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HPI_DOM_ARGS)

#define m_MMGR_FREE_HPI_SESSION_ARGS(p)    m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HPI_DOM_ARGS)

#define m_MMGR_ALLOC_HSM_CB           (HSM_CB *)m_NCS_MEM_ALLOC(sizeof(HSM_CB),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HSM_CB)

#define m_MMGR_FREE_HSM_CB(p)         m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HSM_CB)

#define m_MMGR_ALLOC_SIM_CB           (SIM_CB *)m_NCS_MEM_ALLOC(sizeof(SIM_CB),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_SIM_CB)

#define m_MMGR_FREE_SIM_CB(p)         m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_SIM_CB)

#define m_MMGR_ALLOC_HISV_MSG         (HISV_MSG *)m_NCS_MEM_ALLOC(sizeof(HISV_MSG),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HISV_MSG)

#define m_MMGR_FREE_HISV_MSG(p)       m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HISV_MSG)

#define m_MMGR_ALLOC_HPI_INV_DATA(n)    (uns8 *)m_NCS_MEM_ALLOC(n,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HSM_INV_DATA)

#define m_MMGR_FREE_HPI_INV_DATA(p)   m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HCD, \
                                           NCS_SERVICE_HSM_INV_DATA)

#endif   /* !HCD_MEM_H */
