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

#ifndef VDS_MEM_H
#define VDS_MEM_H

/* Service Sub IDs for VDS */
typedef enum
{
   NCS_SERVICE_VDS_SUB_ID_VDS_CB,
   NCS_SERVICE_VDS_SUB_ID_VDS_EVT,
   NCS_SERVICE_VDS_SUB_ID_VDS_ADEST_INFO,
   NCS_SERVICE_VDS_SUB_ID_VDS_DB_INFO,
   NCS_SERVICE_VDS_SUB_ID_NCSVDA_INFO,
   NCS_SERVICE_VDS_SUB_ID_CKPT_BUFFER,
   NCS_SERVICE_VDS_SUB_ID_MAX
} NCS_SERVICE_VDS_SUB_ID;

/****************************************************************************
                        Memory Allocation and Release Macros 
****************************************************************************/
#define m_MMGR_ALLOC_VDS_CB              (VDS_CB*)m_NCS_MEM_ALLOC( \
                                         sizeof(VDS_CB), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_CB)
#define m_MMGR_FREE_VDS_CB(p)            m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_CB)


#define m_MMGR_ALLOC_VDS_EVT             (VDS_EVT*)m_NCS_MEM_ALLOC( \
                                         sizeof(VDS_EVT), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_EVT)
#define m_MMGR_FREE_VDS_EVT(p)           m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_EVT)


#define m_MMGR_ALLOC_VDS_ADEST_INFO      (VDS_VDEST_ADEST_INFO*)m_NCS_MEM_ALLOC( \
                                         sizeof(VDS_VDEST_ADEST_INFO), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_ADEST_INFO)
#define m_MMGR_FREE_VDS_ADEST_INFO(p)    m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_ADEST_INFO)


#define m_MMGR_ALLOC_VDS_DB_INFO         (VDS_VDEST_DB_INFO*)m_NCS_MEM_ALLOC( \
                                         sizeof(VDS_VDEST_DB_INFO), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_DB_INFO)
#define m_MMGR_FREE_VDS_DB_INFO(p)       m_NCS_MEM_FREE(p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_VDS_DB_INFO)

#define m_MMGR_ALLOC_NCSVDA_INFO         (NCSVDA_INFO *)m_NCS_MEM_ALLOC( \
                                         sizeof(NCSVDA_INFO), \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_NCSVDA_INFO)
#define m_MMGR_FREE_NCSVDA_INFO(p)       m_NCS_MEM_FREE(p, \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_NCSVDA_INFO)

#define m_MMGR_ALLOC_CKPT_BUFFER(p)      m_NCS_MEM_ALLOC( \
                                         p,\
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_CKPT_BUFFER)
#define m_MMGR_FREE_CKPT_BUFFER(p)       m_NCS_MEM_FREE(p, \
                                         NCS_MEM_REGION_PERSISTENT, \
                                         NCS_SERVICE_ID_VDS, \
                                         NCS_SERVICE_VDS_SUB_ID_CKPT_BUFFER)


#endif /* !VDS_MEM_H */

