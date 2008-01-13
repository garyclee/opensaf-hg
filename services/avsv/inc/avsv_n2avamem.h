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

#ifndef AVSV_N2AVAMEM_H
#define AVSV_N2AVAMEM_H

/* The sub ids are shared between dnd messages and nda
 * messages. The dnd can grow to a max of 30 after which
 * nda sub ids will start. Also see avsv_d2nmem.h
 */

typedef enum
{
   NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_NDA_AVA_MSG = 30,
   NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_AMF_CBK_INFO,
   NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_NDA_INFO,
   NCS_SERVICE_AVSV_N2AVA_SUB_ID_MAX
} NCS_SERVICE_AVSV_N2AVA_SUB_ID;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Memory Allocation and Release Macros 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


#define m_MMGR_ALLOC_AVSV_NDA_AVA_MSG      (AVSV_NDA_AVA_MSG*)m_NCS_MEM_ALLOC(sizeof(AVSV_NDA_AVA_MSG), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_NDA_AVA_MSG)
#define m_MMGR_FREE_AVSV_NDA_AVA_MSG(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_NDA_AVA_MSG)

#define m_MMGR_ALLOC_AVSV_AMF_CBK_INFO      (AVSV_AMF_CBK_INFO*)m_NCS_MEM_ALLOC(sizeof(AVSV_AMF_CBK_INFO), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_AMF_CBK_INFO)
#define m_MMGR_FREE_AVSV_AMF_CBK_INFO(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_AMF_CBK_INFO)

#define m_MMGR_ALLOC_AVSV_NDA_INFO(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_NDA_INFO)
#define m_MMGR_FREE_AVSV_NDA_INFO(p)          m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_N2AVA_SUB_ID_AVSV_NDA_INFO)

#endif /* !AVSV_N2AVAMEM_H */
