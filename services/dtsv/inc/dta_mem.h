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
#ifndef DTA_MEM_H
#define DTA_MEM_H


#define DTSV_MEM_DTA_CB 1
#define DTSV_MEM_DTA_REG 2
/* Changes for buffering of logs */
#define DTSV_MEM_DTA_BUF 3

#define m_MMGR_ALLOC_DTA_CB      (DTA_CB *)m_NCS_MEM_ALLOC(sizeof(DTA_CB), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTA_CB)

#define m_MMGR_FREE_DTA_CB(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTA_CB)


#define m_MMGR_ALLOC_DTA_REG_TBL  (REG_TBL_ENTRY *)m_NCS_MEM_ALLOC(sizeof(REG_TBL_ENTRY), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTA_REG)

#define m_MMGR_FREE_DTA_REG_TBL(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTA_REG)

#define m_MMGR_ALLOC_DTA_BUFFERED_LOG (DTA_BUFFERED_LOG *)m_NCS_MEM_ALLOC(sizeof(DTA_BUFFERED_LOG), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTA_BUF)

#define m_MMGR_FREE_DTA_BUFFERED_LOG(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                     NCS_SERVICE_ID_DTSV, DTSV_MEM_DTA_BUF)

#endif
