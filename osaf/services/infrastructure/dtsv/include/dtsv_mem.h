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

  DESCRIPTION: Common DTSv sub-part messaging formats that travels between 
  DTS and DTA.
 

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef DTSV_MEM_H
#define DTSV_MEM_H

#include "ncssysf_mem.h"

#define DTSV_MEM_DTSV_MSG     1
#define DTSV_MEM_OCT         2

#define m_MMGR_ALLOC_DTSV_MSG      (DTSV_MSG *)m_NCS_MEM_ALLOC(sizeof(DTSV_MSG), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTSV_MSG)

#define m_MMGR_FREE_DTSV_MSG(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_DTSV_MSG)

#define m_MMGR_ALLOC_OCT(n)       (char *)m_NCS_MEM_ALLOC((n * sizeof(uint8_t)), \
                                    NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_OCT)

#define m_MMGR_FREE_OCT(p)         m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_DTSV, DTSV_MEM_OCT)

#endif
