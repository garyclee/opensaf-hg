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

  This file contains macros for memory operations for HPL.

*****************************************************************************/

#ifndef HPL_MEM_H
#define HPL_MEM_H

/* Service Sub IDs for HISv - HPL */
typedef enum
{
   NCS_SERVICE_HPL_CB = 1,
   NCS_SERVICE_HAM_INFO

} NCS_SERVICE_HPL_SUBID;


/****************************************
 * Memory Allocation and Release Macros *
 ***************************************/

#define m_MMGR_ALLOC_HPL_CB          (HPL_CB *)m_NCS_MEM_ALLOC(sizeof(HPL_CB),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HPL_CB)

#define m_MMGR_FREE_HPL_CB(p)         m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HPL_CB)


#define m_MMGR_ALLOC_HAM_INFO         (HAM_INFO *)m_NCS_MEM_ALLOC(sizeof(HAM_INFO),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HAM_INFO)

#define m_MMGR_FREE_HAM_INFO(p)       m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HAM_INFO)

#define m_MMGR_ALLOC_HPL_DATA(p)        (uns8 *)m_NCS_MEM_ALLOC((p),\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HPL_CB)

#define m_MMGR_FREE_HPL_DATA(p)         m_NCS_MEM_FREE(p,\
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_HPL, \
                                           NCS_SERVICE_HPL_CB)

#endif /* !HPL_MEM_H */
