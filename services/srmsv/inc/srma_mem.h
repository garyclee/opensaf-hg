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

#ifndef SRMA_MEM_H
#define SRMA_MEM_H

/* Service Sub IDs for SRMA */
typedef enum
{
   NCS_SERVICE_SRMA_SUB_ID_SRMND_INFO = 1,
   NCS_SERVICE_SRMA_SUB_ID_SRMA_CB,
   NCS_SERVICE_SRMA_SUB_ID_SRMND_APPL_NODE,
   NCS_SERVICE_SRMA_SUB_ID_PEND_CBK_REC,
   NCS_SERVICE_SRMA_SUB_ID_CBK_INFO,
   NCS_SERVICE_SRMA_SUB_ID_USR_APPL,
   NCS_SERVICE_SRMA_SUB_ID_RSRC_MON,
   NCS_SERVICE_SRMA_SUB_ID_SRMA_MSG,
   NCS_SERVICE_SRMA_SUB_ID_MAX
} NCS_SERVICE_SRMA_SUB_ID;

/****************************************************************************
                        Memory Allocation and Release Macros 
****************************************************************************/
#define m_MMGR_ALLOC_SRMA_CB                 (SRMA_CB*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_CB), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_SRMA_CB)
#define m_MMGR_FREE_SRMA_CB(p)               m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_SRMA_CB)

#define m_MMGR_ALLOC_SRMA_SRMND_INFO         (SRMA_SRMND_INFO*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_SRMND_INFO), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_SRMND_INFO)
#define m_MMGR_FREE_SRMA_SRMND_INFO(p)       m_NCS_MEM_FREE(p, \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_SRMND_INFO)

#define m_MMGR_ALLOC_SRMA_SRMND_APPL_NODE    (SRMA_SRMND_USR_NODE*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_SRMND_USR_NODE), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_SRMND_APPL_NODE)
#define m_MMGR_FREE_SRMA_SRMND_APPL_NODE(p)  m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_SRMND_APPL_NODE)

#define m_MMGR_ALLOC_SRMA_PEND_CBK_REC       (SRMA_PEND_CBK_REC*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_PEND_CBK_REC), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_PEND_CBK_REC)
#define m_MMGR_FREE_SRMA_PEND_CBK_REC(p)     m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_PEND_CBK_REC)

#define m_MMGR_ALLOC_SRMA_CBK_INFO           (NCS_SRMSV_RSRC_CALLBACK_INFO*)m_NCS_MEM_ALLOC( \
                                             sizeof(NCS_SRMSV_RSRC_CALLBACK_INFO), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_CBK_INFO)
#define m_MMGR_FREE_SRMA_CBK_INFO(p)         m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_CBK_INFO)

#define m_MMGR_ALLOC_SRMA_USR_APPL_NODE      (SRMA_USR_APPL_NODE*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_USR_APPL_NODE), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_USR_APPL)
#define m_MMGR_FREE_SRMA_USR_APPL_NODE(p)    m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_USR_APPL)

#define m_MMGR_ALLOC_SRMA_RSRC_MON           (SRMA_RSRC_MON*)m_NCS_MEM_ALLOC( \
                                             sizeof(SRMA_RSRC_MON), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_RSRC_MON)
#define m_MMGR_FREE_SRMA_RSRC_MON(p)         m_NCS_MEM_FREE(p,\
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SRMA, \
                                             NCS_SERVICE_SRMA_SUB_ID_RSRC_MON)

#endif /* !SRMA_MEM_H */


