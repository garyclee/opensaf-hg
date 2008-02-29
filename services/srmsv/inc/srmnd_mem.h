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

#ifndef SRMND_MEM_H
#define SRMND_MEM_H

/* Service Sub IDs for SRMND */
typedef enum
{
   NCS_SERVICE_SRMND_SUB_ID_RSRC_TYPE = 1,
   NCS_SERVICE_SRMND_SUB_ID_SRMND_CB,
   NCS_SERVICE_SRMND_SUB_ID_SAMPLE,
   NCS_SERVICE_SRMND_SUB_ID_PID,
   NCS_SERVICE_SRMND_SUB_ID_USER,
   NCS_SERVICE_SRMND_SUB_ID_RSRC_MON,
   NCS_SERVICE_SRMND_SUB_ID_SRMND_EVT,
   NCS_SERVICE_SRMND_SUB_ID_MAX
} NCS_SERVICE_SRMND_SUB_ID;

/****************************************************************************
                        Memory Allocation and Release Macros 
****************************************************************************/
#define m_MMGR_ALLOC_SRMND_CB            (SRMND_CB*)m_NCS_MEM_ALLOC( \
                                         sizeof(SRMND_CB), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_SRMND_CB)
#define m_MMGR_FREE_SRMND_CB(p)          m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_SRMND_CB)

#define m_MMGR_ALLOC_SRMND_USER_NODE     (SRMND_MON_SRMA_USR_NODE*)m_NCS_MEM_ALLOC( \
                                         sizeof(SRMND_MON_SRMA_USR_NODE), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_USER)
#define m_MMGR_FREE_SRMND_USER_NODE(p)   m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_USER)

#define m_MMGR_ALLOC_SRMND_RSRC_TYPE     (SRMND_RSRC_TYPE_NODE*)m_NCS_MEM_ALLOC( \
                                         sizeof(SRMND_RSRC_TYPE_NODE), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_RSRC_TYPE)
#define m_MMGR_FREE_SRMND_RSRC_TYPE(p)   m_NCS_MEM_FREE(p, \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_RSRC_TYPE)

#define m_MMGR_ALLOC_SRMND_SAMPLE        (SRMND_SAMPLE_DATA*)m_NCS_MEM_ALLOC( \
                                         sizeof(SRMND_SAMPLE_DATA), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_SAMPLE)
#define m_MMGR_FREE_SRMND_SAMPLE(p)      m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_SAMPLE)

#define m_MMGR_ALLOC_SRMND_PID           (SRMND_PID_DATA*)m_NCS_MEM_ALLOC( \
                                         sizeof(SRMND_PID_DATA), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_PID)
#define m_MMGR_FREE_SRMND_PID(p)         m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_PID)

#define m_MMGR_ALLOC_SRMND_RSRC_MON      (SRMND_RSRC_MON_NODE*)m_NCS_MEM_ALLOC( \
                                         sizeof(SRMND_RSRC_MON_NODE), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_RSRC_MON)
#define m_MMGR_FREE_SRMND_RSRC_MON(p)    m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_RSRC_MON)

#define m_MMGR_ALLOC_SRMND_EVT           (SRMND_EVT*)m_NCS_MEM_ALLOC( \
                                         sizeof(SRMND_EVT), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_SRMND_EVT)
#define m_MMGR_FREE_SRMND_EVT(p)         m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_SRMND, \
                                         NCS_SERVICE_SRMND_SUB_ID_SRMND_EVT)


#endif /* !SRMND_MEM_H */


