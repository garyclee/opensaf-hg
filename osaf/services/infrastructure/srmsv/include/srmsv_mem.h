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

#ifndef SRMSV_MEM_H
#define SRMSV_MEM_H

/* Service Sub IDs for SRMA */
typedef enum
{
   NCS_SVC_SRMSV_SUB_ID_SRMA_MSG = 1,
   NCS_SVC_SRMSV_SUB_ID_SRMA_CREATE_RSRC,
   NCS_SVC_SRMSV_SUB_ID_SRMND_MSG,
   NCS_SVC_SRMSV_SUB_ID_SRMND_CREATED_RSRC,
   NCS_SERVICE_SRMSV_SUB_ID_MAX
} NCS_SERVICE_SRMSV_SUB_ID;

/****************************************************************************
                        Memory Allocation and Release Macros 
****************************************************************************/
#define m_MMGR_ALLOC_SRMA_MSG                (SRMA_MSG*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_MSG), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMA_MSG)
#define m_MMGR_FREE_SRMA_MSG(p)              m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMA_MSG)

#define m_MMGR_ALLOC_SRMA_CREATE_RSRC        (SRMA_CREATE_RSRC_MON_NODE*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_CREATE_RSRC_MON_NODE), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMA_CREATE_RSRC)
#define m_MMGR_FREE_SRMA_CREATE_RSRC(p)      m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMA_CREATE_RSRC)

#define m_MMGR_ALLOC_SRMND_MSG               (SRMND_MSG*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMND_MSG), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMND_MSG)
#define m_MMGR_FREE_SRMND_MSG(p)             m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMND_MSG)

#define m_MMGR_ALLOC_SRMND_CREATED_RSRC      (SRMND_CREATED_RSRC_MON*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMND_CREATED_RSRC_MON), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMND_CREATED_RSRC)
#define m_MMGR_FREE_SRMND_CREATED_RSRC(p)    m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMSV, \
                                             NCS_SVC_SRMSV_SUB_ID_SRMND_CREATED_RSRC)



#endif /* SRMSV_MEM_H */




