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
 

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCSMIB_MEM_H
#define NCSMIB_MEM_H

#ifdef  __cplusplus
extern "C" {
#endif



/***************************************************************************
 *
 *      MIB Memory management MACROS
 *
 ***************************************************************************/



#define m_MMGR_ALLOC_MIB_TIMED (MIB_TIMED*)                              \
                                m_NCS_MEM_ALLOC(sizeof(MIB_TIMED),        \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_COMMON,     \
                                               0)

#define m_MMGR_ALLOC_NCS_KEY (NCS_KEY*)                                    \
                                m_NCS_MEM_ALLOC(sizeof(NCS_KEY),           \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_COMMON,     \
                                               1)

#define m_MMGR_ALLOC_MIB_OCT(size) (uns8*)                               \
                                 m_NCS_MEM_ALLOC(size,                    \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_COMMON,     \
                                               2)


#define m_MMGR_ALLOC_NCSMIB_ARG  (NCSMIB_ARG*)                             \
                                 m_NCS_MEM_ALLOC(sizeof(NCSMIB_ARG),       \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_COMMON,     \
                                               3)


#define m_MMGR_ALLOC_MIB_INST_IDS(size) (uns32*)                         \
                                  m_NCS_MEM_ALLOC((sizeof(uns32) * size), \
                                         NCS_MEM_REGION_PERSISTENT,       \
                                         NCS_SERVICE_ID_COMMON,           \
                                               4)

#define m_MMGR_ALLOC_NCSMIB_IDX   (NCSMIB_IDX*)                            \
                                  m_NCS_MEM_ALLOC(sizeof(NCSMIB_IDX),      \
                                         NCS_MEM_REGION_PERSISTENT,       \
                                         NCS_SERVICE_ID_COMMON,           \
                                               5);


#define m_MMGR_FREE_MIB_TIMED(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,   \
                                                  NCS_SERVICE_ID_COMMON,       \
                                                  0) 

#define m_MMGR_FREE_NCS_KEY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,      \
                                                  NCS_SERVICE_ID_COMMON,       \
                                                  1) 

#define m_MMGR_FREE_MIB_OCT(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,     \
                                                  NCS_SERVICE_ID_COMMON,       \
                                                  2)

#define m_MMGR_FREE_NCSMIB_ARG(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,   \
                                                  NCS_SERVICE_ID_COMMON,       \
                                                  3) 

#define m_MMGR_FREE_MIB_INST_IDS(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,\
                                                  NCS_SERVICE_ID_COMMON,       \
                                                  4)

#define m_MMGR_FREE_NCSMIB_IDX(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,   \
                                                  NCS_SERVICE_ID_COMMON,       \
                                                  5)


#ifdef  __cplusplus
}
#endif


#endif
