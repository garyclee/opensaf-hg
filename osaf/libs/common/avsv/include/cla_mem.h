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
  
******************************************************************************
*/

#ifndef CLA_MEM_H
#define CLA_MEM_H

/* Service Sub IDs for CLA */
typedef enum {
	NCS_SERVICE_CLA_SUB_ID_CLA_DEFAULT_VAL = 1,
	NCS_SERVICE_CLA_SUB_ID_CLA_CB,
	NCS_SERVICE_CLA_SUB_ID_CLA_PEND_CBK_REC,
	NCS_SERVICE_CLA_SUB_ID_CLA_HDL_REC,
	NCS_SERVICE_CLA_SUB_ID_AVSV_NDA_CLA_MSG,
	NCS_SERVICE_CLA_SUB_ID_AVSV_CLM_CBK_INFO,
	NCS_SERVICE_CLA_SUB_ID_MAX
} NCS_SERVICE_CLA_SUB_ID;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Memory Allocation and Release Macros 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define m_MMGR_ALLOC_CLA_DEFAULT_VAL(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_DEFAULT_VAL)
#define m_MMGR_FREE_CLA_DEFAULT_VAL(p)    m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_DEFAULT_VAL)

#define m_MMGR_ALLOC_CLA_CB       (CLA_CB*)m_NCS_MEM_ALLOC(sizeof(CLA_CB), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_CB)
#define m_MMGR_FREE_CLA_CB(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_CB)

#define m_MMGR_ALLOC_CLA_PEND_CBK_REC       (CLA_PEND_CBK_REC*)m_NCS_MEM_ALLOC(sizeof(CLA_PEND_CBK_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_PEND_CBK_REC)
#define m_MMGR_FREE_CLA_PEND_CBK_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_PEND_CBK_REC)

#define m_MMGR_ALLOC_CLA_HDL_REC       (CLA_HDL_REC*)m_NCS_MEM_ALLOC(sizeof(CLA_HDL_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_HDL_REC)
#define m_MMGR_FREE_CLA_HDL_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLA, \
                                                NCS_SERVICE_CLA_SUB_ID_CLA_HDL_REC)

#endif   /* !CLA_MEM_H */
