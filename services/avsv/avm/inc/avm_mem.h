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
  

  .............................................................................

  DESCRIPTION:

  This file contains macros for memory operations.
  
 *******************************************************************************
 */

#ifndef AVM_MEM_H
#define AVM_MEM_H

/* Service Sub IDs for AVM */
typedef enum
{
   NCS_SERVICE_AVM_SUB_ID_DEFAULT_VAL = 1,
   NCS_SERVICE_AVM_SUB_ID_CB,
   NCS_SERVICE_AVM_SUB_ID_EVT,
   NCS_SERVICE_AVM_SUB_ID_HPI_EVT,
   NCS_SERVICE_AVM_SUB_ID_ENTITY_PATH,
   NCS_SERVICE_AVM_SUB_ID_INV_DATA_REC,
   NCS_SERVICE_AVM_SUB_ID_LIST_NODE,
   NCS_SERVICE_AVM_SUB_ID_FAULT_DOMAIN,
   NCS_SERVICE_AVM_SUB_ID_ENT_INFO,
   NCS_SERVICE_AVM_SUB_ID_VALID_INFO,
   NCS_SERVICE_AVM_SUB_ID_NODE_INFO,
   NCS_SERVICE_AVM_SUB_ID_ENT_INFO_LIST,
   NCS_SERVICE_AVM_SUB_ID_PATTERN_ARRAY,
   NCS_SERVICE_AVM_SUB_ID_PATTERN,
   NCS_SERVICE_AVM_SUB_ID_EVT_Q_NODE,
   NCS_SERVICE_AVM_SUB_ID_TRAP_VB,
   NCS_SERVICE_AVM_SUB_ID_EDA_TLV_SIZE,
   NCS_SERVICE_AVM_SUB_ID_MAX
} NCS_SERVICE_AVM_SUB_ID;

/*----------------------------------------------------------------------

                        Memory Allocation and Release Macros 

----------------------------------------------------------------------*/


#define m_MMGR_ALLOC_AVM_DEFAULT_VAL(size)  m_NCS_MEM_ALLOC( \
                                                size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_DEFAULT_VAL)

#define m_MMGR_FREE_AVM_DEFAULT_VAL(p)     m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_DEFAULT_VAL)

#define m_MMGR_ALLOC_AVM_CB          (AVM_CB_T*)m_NCS_MEM_ALLOC(sizeof(AVM_CB_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_CB)

#define m_MMGR_FREE_AVM_CB(p)        m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_CB)

#define m_MMGR_ALLOC_AVM_EVT          (AVM_EVT_T*)m_NCS_MEM_ALLOC(sizeof(AVM_EVT_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_EVT)

#define m_MMGR_FREE_AVM_EVT(p)       m_NCS_MEM_FREE(p,\
                                              NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_EVT)

#define m_MMGR_ALLOC_AVM_HPI_EVT          (AVM_HPI_EVT_T*)m_NCS_MEM_ALLOC(sizeof(AVM_HPI_EVT_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_HPI_EVT)

#define m_MMGR_FREE_AVM_HPI_EVT(p)       m_NCS_MEM_FREE(p,\
                                              NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_HPI_EVT)

#define m_MMGR_ALLOC_AVM_ENTITY_PATH     (SaHpiEntityPathT*)m_NCS_MEM_ALLOC(sizeof(SaHpiEntityPathT), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_ENTITY_PATH)

#define m_MMGR_FREE_AVM_ENTITY_PATH(p)    m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_ENTITY_PATH)


#define m_MMGR_ALLOC_AVM_INV_DATA_REC     (SaHpiInventoryRecT*)m_NCS_MEM_ALLOC(sizeof(SaHpiInventoryRecT), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_INV_DATA_REC)

#define m_MMGR_FREE_AVM_INV_DATA_REC(p)    m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_INV_DATA_REC)

#define m_MMGR_ALLOC_AVM_LIST_NODE        (AVM_LIST_NODE_T*)m_NCS_MEM_ALLOC(sizeof(AVM_LIST_NODE_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_LIST_NODE)

#define m_MMGR_FREE_AVM_LIST_NODE(p)       m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_LIST_NODE)

#define m_MMGR_ALLOC_AVM_FAULT_DOMAIN      (AVM_FAULT_DOMAIN_T*)m_NCS_MEM_ALLOC(sizeof(AVM_FAULT_DOMAIN_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_FAULT_DOMAIN)

#define m_MMGR_FREE_AVM_FAULT_DOMAIN(p)    m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_FAULT_DOMAIN)


#define m_MMGR_ALLOC_AVM_ENT_INFO         (AVM_ENT_INFO_T*)m_NCS_MEM_ALLOC(sizeof(AVM_ENT_INFO_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_ENT_INFO)

#define m_MMGR_FREE_AVM_ENT_INFO(p)        m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_ENT_INFO)

#define m_MMGR_ALLOC_AVM_VALID_INFO      (AVM_VALID_INFO_T*)m_NCS_MEM_ALLOC(sizeof(AVM_VALID_INFO_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_VALID_INFO)

#define m_MMGR_FREE_AVM_VALID_INFO(p)    m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_VALID_INFO)

#define m_MMGR_ALLOC_AVM_NODE_INFO      (AVM_NODE_INFO_T*)m_NCS_MEM_ALLOC(sizeof(AVM_NODE_INFO_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_NODE_INFO)

#define m_MMGR_FREE_AVM_NODE_INFO(p)    m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_NODE_INFO)

#define m_MMGR_ALLOC_AVM_ENT_INFO_LIST         (AVM_ENT_INFO_LIST_T*)m_NCS_MEM_ALLOC(sizeof(AVM_ENT_INFO_LIST_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_ENT_INFO_LIST)

#define m_MMGR_FREE_AVM_ENT_INFO_LIST(p)    m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_ENT_INFO_LIST)

#define m_MMGR_ALLOC_AVM_EVENT_PATTERN_ARRAY (SaEvtEventPatternArrayT*)\
                                                 m_NCS_MEM_ALLOC(sizeof(SaEvtEventPatternArrayT), \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM, \
                                                 NCS_SERVICE_AVM_SUB_ID_PATTERN_ARRAY)

#define m_MMGR_FREE_AVM_EVENT_PATTERN_ARRAY(p) m_NCS_MEM_FREE(p, \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM, \
                                                 NCS_SERVICE_AVM_SUB_ID_PATTERN_ARRAY)

#define m_MMGR_ALLOC_AVM_EVENT_PATTERNS(p)   (SaEvtEventPatternT*)\
                                                m_NCS_MEM_ALLOC(p * sizeof(SaEvtEventPatternT), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_PATTERN)   


#define m_MMGR_FREE_AVM_EVENT_PATTERNS(p)      m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVM, \
                                                NCS_SERVICE_AVM_SUB_ID_PATTERN)   

#define m_MMGR_ALLOC_AVM_EVT_Q_NODE       (AVM_EVT_Q_NODE_T*)\
                                               m_NCS_MEM_ALLOC(sizeof(AVM_EVT_Q_NODE_T), \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_AVM, \
                                               NCS_SERVICE_AVM_SUB_ID_EVT_Q_NODE)
                                                                                
                                                                                
#define m_MMGR_FREE_AVM_EVT_Q_NODE(p)      m_NCS_MEM_FREE(p, \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_AVM, \
                                               NCS_SERVICE_AVM_SUB_ID_EVT_Q_NODE)

#define m_MMGR_ALLOC_AVM_TRAP_VB       (NCS_TRAP_VARBIND*)\
                                               m_NCS_MEM_ALLOC(sizeof(NCS_TRAP_VARBIND), \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_AVM, \
                                               NCS_SERVICE_AVM_SUB_ID_TRAP_VB)
                                                                                
                                                                                
#define m_MMGR_FREE_AVM_TRAP_VB(p)      m_NCS_MEM_FREE(p, \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_AVM, \
                                               NCS_SERVICE_AVM_SUB_ID_TRAP_VB)

#define m_MMGR_ALLOC_AVM_EDA_TLV_SIZE(size)  (uns8*)\
                                               m_NCS_MEM_ALLOC(size, \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_AVM, \
                                               NCS_SERVICE_AVM_SUB_ID_TRAP_VB)
                                                                                
                                                                                
#define m_MMGR_FREE_AVM_EDA_TLV_SIZE(p)      m_NCS_MEM_FREE(p, \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               NCS_SERVICE_ID_AVM, \
                                               NCS_SERVICE_AVM_SUB_ID_TRAP_VB)

#endif /*__AVM_MEM_H__ */
