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

#ifndef AVSV_N2CLAMEM_H
#define AVSV_N2CLAMEM_H

typedef enum
{
   NCS_SERVICE_AVSV_N2CLA_SUB_DEFAULT_VAL = 1,
   NCS_SERVICE_AVSV_N2CLA_SUB_AVSV_NDA_CLA_MSG,
   NCS_SERVICE_AVSV_N2CLA_SUB_AVSV_CLM_CBK_INFO,
   NCS_SERVICE_AVSV_N2CLA_SUB_MAX
} NCS_SERVICE_AVSV_N2CLA_SUB_ID;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Memory Allocation and Release Macros 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


#define m_MMGR_ALLOC_AVSV_NDA_CLA_MSG      (AVSV_NDA_CLA_MSG*)m_NCS_MEM_ALLOC(sizeof(AVSV_NDA_CLA_MSG), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2CLA_SUB_AVSV_NDA_CLA_MSG)
#define m_MMGR_FREE_AVSV_NDA_CLA_MSG(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2CLA_SUB_AVSV_NDA_CLA_MSG)

#define m_MMGR_ALLOC_AVSV_CLM_CBK_INFO      (AVSV_CLM_CBK_INFO*)m_NCS_MEM_ALLOC(sizeof(AVSV_CLM_CBK_INFO), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2CLA_SUB_AVSV_CLM_CBK_INFO)
#define m_MMGR_FREE_AVSV_CLM_CBK_INFO(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2CLA_SUB_AVSV_CLM_CBK_INFO)

#define m_MMGR_ALLOC_AVSV_CLA_DEFAULT_VAL(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2CLA_SUB_DEFAULT_VAL)

#define m_MMGR_FREE_AVSV_CLA_DEFAULT_VAL(p)    m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2CLA_SUB_DEFAULT_VAL) 

#endif /* !AVSV_N2CLAMEM_H */
