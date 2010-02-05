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
 */

/******************************************************************************
  DESCRIPTION:

  This file contains macros for allocation and freeing memory.
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef PDRBD_MEM_H
#define PDRBD_MEM_H

#include "pdrbd.h"
#include "ncssysf_mem.h"

#define MBCSV_MEM_PDRBD_EVT     1

/* Allocate and free PDRBD message */
#define m_MMGR_ALLOC_PDRBD_EVT      (PDRBD_EVT *)m_NCS_MEM_ALLOC(sizeof(PDRBD_EVT), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PDRBD, MBCSV_MEM_PDRBD_EVT)

#define m_MMGR_FREE_PDRBD_EVT(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PDRBD, MBCSV_MEM_PDRBD_EVT)

#endif   /*PDRBD_MEM_H */
