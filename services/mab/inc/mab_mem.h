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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.0/os_svcs/products/mab/mab_mem.h 3     1/27/01 11:16a Kquest $


..............................................................................

  DESCRIPTION:

..............................................................................
*/

/*
 * Module Inclusion Control...
 */
#ifndef MAB_MEM_H
#define MAB_MEM_H


/*******************************************************************************
   M e m o r y   M a n a g e r   P r i m i t i v e s   F o r   M A B


                  Populate accordingly during portation.


   The following macros provide the front-end to the target system memory
   manager. These macros must be changed such that the desired operation is
   accomplished by using the target system memory manager primitives.

********************************************************************************/

/*******************************************************************************
 Memory Allocation Primitives for NetPlane MIB Access Broker (MAB)
********************************************************************************/

typedef enum
{   
    MAB_MEM_SUB_ID_MIN,
    MAB_MEM_MAC_INST,
    MAB_MEM_MAC_BLK,
    MAB_MEM_MAC_TBL_BUFR,
    MAB_MEM_MAS_TBL,
    MAB_MEM_OAC_TBL,
    MAB_MEM_OAC_TBL_REC,
    MAB_MEM_OAC_FLTR,
    MAB_MEM_PCN_STRING,
    MAB_MEM_MAB_MSG,
    MAB_MEM_MAS_ROW_REC,
    MAB_MEM_MAS_FLTR,
    MAB_MEM_MAS_CSI_NODE,
    MAB_MEM_MAB_INST_NODE,
    MAB_MEM_FLTR_ANCHOR_NODE, 
    MAB_MEM_PSS_CB,
    MAB_MEM_PSS_PWE_CB,
    MAB_MEM_PSS_TBL_INFO,
    MAB_MEM_PSS_NCSMIB_TBL_INFO, 
    MAB_MEM_PSS_MIB_TBL_DATA,
    MAB_MEM_PSS_VAR_INFO,
    MAB_MEM_PSS_NCSMIB_OCT_OBJ,
    MAB_MEM_PSS_NCSMIB_INT_OBJ_RANGE,
    MAB_MEM_PSS_TBL_RNK,
    MAB_MEM_PSS_PAT_NODE,
    MAB_MEM_PSS_CLIENT_ENTRY,
    MAB_MEM_PSS_TBL_REC,
    MAB_MEM_PSS_DATA,
    MAB_MEM_PSS_QELEM,
    MAB_MEM_PSS_OAA_ENTRY,
    MAB_MEM_PSS_RFRSH_TBL_LIST,
    MAB_MEM_OAA_PCN_LIST,
    MAB_MEM_NCSOAC_PSS_TBL_LIST,
    MAB_MEM_OAA_WBREQ_PEND_LIST,
    MAB_MEM_OAA_WBREQ_HDL_LIST,
    MAB_MEM_OAA_BUFR_HASH_LIST,
    MAB_MEM_PSS_TBL_BIND_EVT,
    MAB_MEM_PSS_TBL_LIST,
    MAB_MEM_PSS_WARMBOOT_REQ,
    MAB_MEM_PSS_WBSORT_ENTRY,
    MAB_MEM_PSS_ODSORT_ENTRY,
    MAB_MEM_PSS_ODSORT_TBLREC,
    MAB_MEM_PSS_OAA_CLT_ID,
    MAB_MEM_PSS_SPCN_LIST,
    MAB_MEM_PSS_CSI_NODE,
    MAB_MEM_PSS_SPCN_WBREQ_PEND_LIST,
    MAB_MEM_FLTR_WARM_SYNC_CNTXT,  /* used only in NCS_MAS_RED flag */ 
    MAB_MEM_FLTR_WARM_SYNC_BKT_LIST, /* used only in NCS_MAS_RED flag */ 
    MAB_MEM_STDBY_PSS_BUFFER_NODE, /* Fix for IR00085164 */
    MAB_MEM_PSS_TBL_DETAILS_HDR,
    MAB_MEM_SUB_ID_MAX
}MAB_MEM_SUB_ID;

#define m_MMGR_ALLOC_MAS_TBL      (MAS_TBL *)m_NCS_MEM_ALLOC(sizeof(MAS_TBL), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAS_TBL)

#define m_MMGR_ALLOC_OAC_TBL      (OAC_TBL *)m_NCS_MEM_ALLOC(sizeof(OAC_TBL), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAC_TBL)

#define m_MMGR_ALLOC_OAC_TBL_REC  (OAC_TBL_REC *)m_NCS_MEM_ALLOC(sizeof(OAC_TBL_REC), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAC_TBL_REC)

#define m_MMGR_ALLOC_OAC_FLTR     (OAC_FLTR *)m_NCS_MEM_ALLOC(sizeof(OAC_FLTR), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAC_FLTR)

#define m_MMGR_ALLOC_MAB_PCN_STRING(cnt)  (char *)m_NCS_MEM_ALLOC((sizeof(char)*cnt), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PCN_STRING)

#define m_MMGR_ALLOC_MAB_MSG      (MAB_MSG *)m_NCS_MEM_ALLOC(sizeof(MAB_MSG), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAB_MSG)

#define m_MMGR_ALLOC_MAC_INST     (MAC_INST *)m_NCS_MEM_ALLOC(sizeof(MAC_INST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAC_INST)

#define m_MMGR_ALLOC_MAC_BLK      (MAC_BLK *)m_NCS_MEM_ALLOC(sizeof(MAC_BLK), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAC_BLK)

#define m_MMGR_ALLOC_MAC_TBL_BUFR(n) (uns16*)m_NCS_MEM_ALLOC((n + 1) * sizeof(uns16), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAC_TBL_BUFR)

#define m_MMGR_ALLOC_MAS_ROW_REC  (MAS_ROW_REC *)m_NCS_MEM_ALLOC(sizeof(MAS_ROW_REC), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAS_ROW_REC)
#define m_MMGR_ALLOC_MAB_INST_NODE \
                           (MAB_INST_NODE *)m_NCS_MEM_ALLOC(sizeof(MAB_INST_NODE), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAB_INST_NODE)

#define m_MMGR_ALLOC_MAS_FLTR    (MAS_FLTR *)m_NCS_MEM_ALLOC(sizeof(MAS_FLTR), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAS_FLTR)

#define m_MMGR_ALLOC_MAS_CSI_NODE \
                (MAS_CSI_NODE*)m_NCS_MEM_ALLOC(sizeof(MAS_CSI_NODE), \
                               NCS_MEM_REGION_TRANSIENT, \
                               NCS_SERVICE_ID_MAB, MAB_MEM_MAS_CSI_NODE)

#define m_MMGR_ALLOC_MAB_OAA_PCN_LIST  (OAA_PCN_LIST *)m_NCS_MEM_ALLOC(sizeof(OAA_PCN_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_PCN_LIST)

#define m_MMGR_ALLOC_NCSOAC_PSS_TBL_LIST  (NCSOAC_PSS_TBL_LIST *)m_NCS_MEM_ALLOC(sizeof(NCSOAC_PSS_TBL_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_NCSOAC_PSS_TBL_LIST)

#define m_MMGR_ALLOC_MAB_OAA_WBREQ_PEND_LIST  (OAA_WBREQ_PEND_LIST *)m_NCS_MEM_ALLOC(sizeof(OAA_WBREQ_PEND_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_WBREQ_PEND_LIST)

#define m_MMGR_ALLOC_OAA_WBREQ_HDL_LIST (OAA_WBREQ_HDL_LIST *)m_NCS_MEM_ALLOC(sizeof(OAA_WBREQ_HDL_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_WBREQ_HDL_LIST)

#define m_MMGR_ALLOC_OAA_BUFR_HASH_LIST   (OAA_BUFR_HASH_LIST*)m_NCS_MEM_ALLOC(sizeof(OAA_BUFR_HASH_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_BUFR_HASH_LIST)

#define m_MMGR_ALLOC_MAB_PSS_TBL_BIND_EVT  (MAB_PSS_TBL_BIND_EVT *)m_NCS_MEM_ALLOC(sizeof(MAB_PSS_TBL_BIND_EVT), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PSS_TBL_BIND_EVT)

#define m_MMGR_ALLOC_MAB_PSS_TBL_LIST  (MAB_PSS_TBL_LIST *)m_NCS_MEM_ALLOC(sizeof(MAB_PSS_TBL_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PSS_TBL_LIST)

#define m_MMGR_ALLOC_MAB_PSS_WARMBOOT_REQ  (MAB_PSS_WARMBOOT_REQ*)m_NCS_MEM_ALLOC(sizeof(MAB_PSS_WARMBOOT_REQ), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PSS_WARMBOOT_REQ)

#define m_MMGR_ALLOC_PSS_CB      (PSS_CB *)m_NCS_MEM_ALLOC(sizeof(PSS_CB), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_CB)

#define m_MMGR_ALLOC_PSS_PWE_CB      (PSS_PWE_CB *)m_NCS_MEM_ALLOC(sizeof(PSS_PWE_CB), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_PWE_CB)

#define m_MMGR_ALLOC_PSS_TBL_INFO (PSS_MIB_TBL_INFO *) m_NCS_MEM_ALLOC(sizeof(PSS_MIB_TBL_INFO), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_INFO)

#define m_MMGR_ALLOC_PSS_NCSMIB_TBL_INFO (PSS_NCSMIB_TBL_INFO *) m_NCS_MEM_ALLOC(sizeof(PSS_NCSMIB_TBL_INFO), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_NCSMIB_TBL_INFO)

#define m_MMGR_ALLOC_PSS_MIB_TBL_DATA (PSS_MIB_TBL_DATA *) m_NCS_MEM_ALLOC(sizeof(PSS_MIB_TBL_DATA), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_MIB_TBL_DATA)

#define m_MMGR_ALLOC_PSS_VAR_INFO(num) (PSS_VAR_INFO *) m_NCS_MEM_ALLOC((sizeof(PSS_VAR_INFO) * num), \
                                       NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_PSS, MAB_MEM_PSS_VAR_INFO)

#define m_MMGR_ALLOC_PSS_NCSMIB_OCT_OBJ(num) (NCSMIB_OCT_OBJ*) m_NCS_MEM_ALLOC((sizeof(NCSMIB_OCT_OBJ) * num), \
                                       NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_PSS, MAB_MEM_PSS_NCSMIB_OCT_OBJ)

#define m_MMGR_ALLOC_PSS_NCSMIB_INT_OBJ_RANGE(num) (NCSMIB_INT_OBJ_RANGE*) m_NCS_MEM_ALLOC((sizeof(NCSMIB_INT_OBJ_RANGE) * num), \
                                       NCS_MEM_REGION_TRANSIENT, \
                                       NCS_SERVICE_ID_PSS, MAB_MEM_PSS_NCSMIB_INT_OBJ_RANGE)

#define m_MMGR_ALLOC_PSS_TBL_RNK  (PSS_MIB_TBL_RANK *) m_NCS_MEM_ALLOC(sizeof(PSS_MIB_TBL_RANK), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_RNK)

#define m_MMGR_ALLOC_PSS_PAT_NODE (NCS_PATRICIA_NODE *) m_NCS_MEM_ALLOC(sizeof(NCS_PATRICIA_NODE), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_PAT_NODE)

#define m_MMGR_ALLOC_PSS_CLIENT_ENTRY (PSS_CLIENT_ENTRY *) m_NCS_MEM_ALLOC(sizeof(PSS_CLIENT_ENTRY), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_CLIENT_ENTRY)

#define m_MMGR_ALLOC_PSS_TBL_REC  (PSS_TBL_REC *) m_NCS_MEM_ALLOC(sizeof(PSS_TBL_REC), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_REC)

#define m_MMGR_ALLOC_PSS_OCT(len) (uns8 *) m_NCS_MEM_ALLOC(len, \
                                    NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_DATA)

#define m_MMGR_ALLOC_PSS_QELEM     (PSS_QELEM *) m_NCS_MEM_ALLOC(sizeof(PSS_QELEM), \
                                    NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_QELEM)

#define m_MMGR_ALLOC_PSS_OAA_ENTRY (PSS_OAA_ENTRY *) m_NCS_MEM_ALLOC(sizeof(PSS_OAA_ENTRY), \
                                    NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_OAA_ENTRY)

#define m_MMGR_ALLOC_PSS_RFRSH_TBL_LIST (PSS_RFRSH_TBL_LIST*) m_NCS_MEM_ALLOC(sizeof(PSS_RFRSH_TBL_LIST), \
                                    NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_RFRSH_TBL_LIST)

#define m_MMGR_ALLOC_PSS_WBSORT_ENTRY  (PSS_WBSORT_ENTRY*)m_NCS_MEM_ALLOC(sizeof(PSS_WBSORT_ENTRY), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_WBSORT_ENTRY)

#define m_MMGR_ALLOC_PSS_ODSORT_ENTRY  (PSS_ODSORT_ENTRY*)m_NCS_MEM_ALLOC(sizeof(PSS_ODSORT_ENTRY), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_ODSORT_ENTRY)

#define m_MMGR_ALLOC_PSS_ODSORT_TBL_REC  (PSS_ODSORT_TBL_REC*)m_NCS_MEM_ALLOC(sizeof(PSS_ODSORT_TBL_REC), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_ODSORT_TBLREC)

#define m_MMGR_ALLOC_MAB_PSS_OAA_CLT_ID  (PSS_OAA_CLT_ID*)m_NCS_MEM_ALLOC(sizeof(PSS_OAA_CLT_ID), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_OAA_CLT_ID)

#define m_MMGR_ALLOC_MAB_PSS_SPCN_LIST  (PSS_SPCN_LIST*)m_NCS_MEM_ALLOC(sizeof(PSS_SPCN_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_SPCN_LIST)

#define m_MMGR_ALLOC_PSS_CSI_NODE  (PSS_CSI_NODE*)m_NCS_MEM_ALLOC(sizeof(PSS_CSI_NODE), \
                               NCS_MEM_REGION_TRANSIENT, \
                               NCS_SERVICE_ID_PSS, MAB_MEM_PSS_CSI_NODE)

#define m_MMGR_ALLOC_PSS_SPCN_WBREQ_PEND_LIST (PSS_SPCN_WBREQ_PEND_LIST*)m_NCS_MEM_ALLOC(sizeof(PSS_SPCN_WBREQ_PEND_LIST), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_SPCN_WBREQ_PEND_LIST)

/* Fix for IR00085164 */
#define m_MMGR_ALLOC_STDBY_PSS_BUFFER_NODE \
               (PSS_STDBY_OAA_DOWN_BUFFER_NODE *)m_NCS_MEM_ALLOC(sizeof(PSS_STDBY_OAA_DOWN_BUFFER_NODE), \
               NCS_MEM_REGION_TRANSIENT, \
               NCS_SERVICE_ID_PSS, MAB_MEM_STDBY_PSS_BUFFER_NODE)

/* allocate memory for ANCHOR value and Fltr-Id pair */ 
#define m_MMGR_ALLOC_FLTR_ANCHOR_NODE \
                (MAB_FLTR_ANCHOR_NODE*)m_NCS_MEM_ALLOC(sizeof(MAB_FLTR_ANCHOR_NODE), \
                               NCS_MEM_REGION_TRANSIENT, \
                               NCS_SERVICE_ID_MAB, MAB_MEM_FLTR_ANCHOR_NODE)

#define m_MMGR_ALLOC_PSS_TBL_DETAILS_HDR (PSS_TABLE_DETAILS_HEADER *) m_NCS_MEM_ALLOC(sizeof(PSS_TABLE_DETAILS_HEADER), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_DETAILS_HDR)

#if (NCS_MAS_RED == 1)
/* allocate memory to stroe the data request context */ 
#define m_MMGR_MAS_WARM_SYNC_CNTXT_ALLOC \
                (MAS_WARM_SYNC_CNTXT*)m_NCS_MEM_ALLOC(sizeof(MAS_WARM_SYNC_CNTXT), \
                               NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_MAB, \
                               MAB_MEM_FLTR_WARM_SYNC_CNTXT)

/* to store the list of buckets to be synced with Standby MAS */
#define m_MMGR_MAS_WARM_SYNC_BKT_LIST_ALLOC(bkt_count) \
                (uns8*)m_NCS_MEM_ALLOC((sizeof(uns8)*bkt_count), \
                               NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_MAB, \
                               MAB_MEM_FLTR_WARM_SYNC_BKT_LIST)


#endif /* (NCS_MAS_RED == 1) */

/*******************************************************************************
 Memory Free Primitives for NetPlane MIB Access Broker (MAB)
********************************************************************************/

#define m_MMGR_FREE_MAC_INST(p)    m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAC_INST)

#define m_MMGR_FREE_MAC_BLK(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAC_BLK)

#define m_MMGR_FREE_MAC_TBL_BUFR(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAC_TBL_BUFR)

#define m_MMGR_FREE_MAB_INST_NODE(p)  \
                                   m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAB_INST_NODE)

#define m_MMGR_FREE_MAS_TBL(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAS_TBL)

#define m_MMGR_FREE_OAC_TBL(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAC_TBL)

#define m_MMGR_FREE_OAC_TBL_REC(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAC_TBL_REC)

#define m_MMGR_FREE_OAC_FLTR(p)    m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAC_FLTR)

#define m_MMGR_FREE_MAB_MSG(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAB_MSG)

#define m_MMGR_FREE_MAS_ROW_REC(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAS_ROW_REC)

#define m_MMGR_FREE_MAS_FLTR(p)    m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAS_FLTR)

#define m_MMGR_FREE_MAS_CSI_NODE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_MAS_CSI_NODE)

#define m_MMGR_FREE_MAB_OAA_PCN_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_PCN_LIST)

#define m_MMGR_FREE_NCSOAC_PSS_TBL_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_NCSOAC_PSS_TBL_LIST)

#define m_MMGR_FREE_MAB_OAA_WBREQ_PEND_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_WBREQ_PEND_LIST)

#define m_MMGR_FREE_OAA_WBREQ_HDL_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_WBREQ_HDL_LIST)

#define m_MMGR_FREE_OAA_BUFR_HASH_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_OAA_BUFR_HASH_LIST)

#define m_MMGR_FREE_MAB_PSS_TBL_BIND_EVT(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PSS_TBL_BIND_EVT)

#define m_MMGR_FREE_MAB_PSS_TBL_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PSS_TBL_LIST)

#define m_MMGR_FREE_MAB_PCN_STRING(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PCN_STRING)

#define m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_PSS_WARMBOOT_REQ)

#define m_MMGR_FREE_PSS_CB(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_CB)

#define m_MMGR_FREE_PSS_PWE_CB(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_PWE_CB)

#define m_MMGR_FREE_PSS_TBL_INFO(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_INFO)

#define m_MMGR_FREE_PSS_NCSMIB_TBL_INFO(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_NCSMIB_TBL_INFO)

#define m_MMGR_FREE_PSS_MIB_TBL_DATA(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_MIB_TBL_DATA)

#define m_MMGR_FREE_PSS_VAR_INFO(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_VAR_INFO)

#define m_MMGR_FREE_PSS_NCSMIB_OCT_OBJ(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_NCSMIB_OCT_OBJ)

#define m_MMGR_FREE_PSS_NCSMIB_INT_OBJ_RANGE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_NCSMIB_INT_OBJ_RANGE)

#define m_MMGR_FREE_PSS_TBL_RNK(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_RNK)

#define m_MMGR_FREE_PSS_PAT_NODE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_PAT_NODE)

#define m_MMGR_FREE_PSS_CLIENT_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_CLIENT_ENTRY)

#define m_MMGR_FREE_PSS_TBL_REC(p)  m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_REC)

#define m_MMGR_FREE_PSS_OCT(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_DATA)

#define m_MMGR_FREE_PSS_QELEM(p)   m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_QELEM)

#define m_MMGR_FREE_PSS_OAA_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_OAA_ENTRY)

#define m_MMGR_FREE_PSS_RFRSH_TBL_LIST(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_RFRSH_TBL_LIST)

#define m_MMGR_FREE_PSS_WBSORT_ENTRY(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_WBSORT_ENTRY)

#define m_MMGR_FREE_PSS_ODSORT_ENTRY(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_ODSORT_ENTRY)

#define m_MMGR_FREE_PSS_ODSORT_TBL_REC(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_ODSORT_TBLREC)

#define m_MMGR_FREE_MAB_PSS_OAA_CLT_ID(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_OAA_CLT_ID)

#define m_MMGR_FREE_MAB_PSS_SPCN_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_SPCN_LIST)

#define m_MMGR_FREE_PSS_CSI_NODE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_CSI_NODE)

#define m_MMGR_FREE_PSS_SPCN_WBREQ_PEND_LIST(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_PSS, MAB_MEM_PSS_SPCN_WBREQ_PEND_LIST)

/* Fix for IR00085164 */
#define m_MMGR_FREE_STDBY_PSS_BUFFER_NODE(p)  m_NCS_MEM_FREE(p,NCS_MEM_REGION_TRANSIENT,  \
                                                   NCS_SERVICE_ID_PSS, MAB_MEM_STDBY_PSS_BUFFER_NODE)

/* free the MAB_FLTR_ANCHOR_NODE */
#define m_MMGR_FREE_FLTR_ANCHOR_NODE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_FLTR_ANCHOR_NODE)

#define m_MMGR_FREE_PSS_TBL_DETAILS_HDR(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                    NCS_SERVICE_ID_PSS, MAB_MEM_PSS_TBL_DETAILS_HDR)

#if (NCS_MAS_RED == 1)
/* free the context information for DATA Request */ 
#define m_MMGR_MAS_WARM_SYNC_CNTXT_FREE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_FLTR_WARM_SYNC_CNTXT)

/* to free the list of buckets to be synced with Standby MAS */ 
#define m_MMGR_MAS_WARM_SYNC_BKT_LIST_FREE(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, MAB_MEM_FLTR_WARM_SYNC_BKT_LIST)

#endif /* #if (NCS_MAS_RED == 1) */ 

#endif


