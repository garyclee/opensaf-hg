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

#ifndef AVND_MEM_H
#define AVND_MEM_H

/* Service Sub IDs for AVND */
typedef enum {
	NCS_SERVICE_AVND_SUB_ID_AVND_DEFAULT_VAL = 1,
	NCS_SERVICE_AVND_SUB_ID_AVND_CB,
	NCS_SERVICE_AVND_SUB_ID_AVND_CLM_REC,
	NCS_SERVICE_AVND_SUB_ID_AVND_EVT,
	NCS_SERVICE_AVND_SUB_ID_AVND_SU,
	NCS_SERVICE_AVND_SUB_ID_AVND_SU_SI_REC,
	NCS_SERVICE_AVND_SUB_ID_AVND_SU_SIQ_REC,
	NCS_SERVICE_AVND_SUB_ID_AVND_COMP,
	NCS_SERVICE_AVND_SUB_ID_AVND_COMP_CSI_REC,
	NCS_SERVICE_AVND_SUB_ID_AVND_COMP_HC_REC,
	NCS_SERVICE_AVND_SUB_ID_AVND_COMP_PM_REC,
	NCS_SERVICE_AVND_SUB_ID_AVND_COMP_PXIED_REC,
	NCS_SERVICE_AVND_SUB_ID_AVND_COMP_CBK,
	NCS_SERVICE_AVND_SUB_ID_AVND_HC,
	NCS_SERVICE_AVND_SUB_ID_AVND_PG,
	NCS_SERVICE_AVND_SUB_ID_AVND_PG_MEM,
	NCS_SERVICE_AVND_SUB_ID_AVND_PG_TRK,
	NCS_SERVICE_AVND_SUB_ID_AVND_DND_MSG_LIST,
	NCS_SERVICE_AVND_SUB_ID_AVSV_CLM_TRK_INFO,
	NCS_SERVICE_AVND_SUB_ID_AVND_NODEID_TO_MDSDEST,
	NCS_SERVICE_AVND_SUB_ID_REG_REQ_PENDING,
	NCS_SERVICE_AVND_SUB_ID_AVND_ASYNC_UPDT,
	NCS_SERVICE_AVND_SUB_ID_MAX
} NCS_SERVICE_AVND_SUB_ID;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Memory Allocation and Release Macros 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define m_MMGR_ALLOC_AVND_DEFAULT_VAL(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_DEFAULT_VAL)
#define m_MMGR_FREE_AVND_DEFAULT_VAL(p)    m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_DEFAULT_VAL)

#define m_MMGR_ALLOC_AVND_CB       (AVND_CB*)m_NCS_MEM_ALLOC(sizeof(AVND_CB), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_CB)
#define m_MMGR_FREE_AVND_CB(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_CB)

#define m_MMGR_ALLOC_AVND_CLM_REC       (AVND_CLM_REC*)m_NCS_MEM_ALLOC(sizeof(AVND_CLM_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_CLM_REC)
#define m_MMGR_FREE_AVND_CLM_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_CLM_REC)

#define m_MMGR_ALLOC_AVND_CLM_TRK_INFO  (AVND_CLM_TRK_INFO*)m_NCS_MEM_ALLOC(sizeof(AVND_CLM_TRK_INFO), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVSV_CLM_TRK_INFO)
#define m_MMGR_FREE_AVND_CLM_TRK_INFO(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVSV_CLM_TRK_INFO)

#define m_MMGR_ALLOC_AVND_EVT      (AVND_EVT*)m_NCS_MEM_ALLOC(sizeof(AVND_EVT), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_EVT)
#define m_MMGR_FREE_AVND_EVT(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_EVT)

#define m_MMGR_ALLOC_AVND_SU      (AVND_SU*)m_NCS_MEM_ALLOC(sizeof(AVND_SU), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_SU)
#define m_MMGR_FREE_AVND_SU(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_SU)

#define m_MMGR_ALLOC_AVND_SU_SI_REC      (AVND_SU_SI_REC*)m_NCS_MEM_ALLOC(sizeof(AVND_SU_SI_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_SU_SI_REC)
#define m_MMGR_FREE_AVND_SU_SI_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_SU_SI_REC)

#define m_MMGR_ALLOC_AVND_SU_SIQ_REC      (AVND_SU_SIQ_REC*)m_NCS_MEM_ALLOC(sizeof(AVND_SU_SIQ_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_SU_SIQ_REC)
#define m_MMGR_FREE_AVND_SU_SIQ_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_SU_SIQ_REC)

#define m_MMGR_ALLOC_AVND_COMP      (AVND_COMP*)m_NCS_MEM_ALLOC(sizeof(AVND_COMP), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP)
#define m_MMGR_FREE_AVND_COMP(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP)

#define m_MMGR_ALLOC_AVND_COMP_CSI_REC      (AVND_COMP_CSI_REC*)m_NCS_MEM_ALLOC(sizeof(AVND_COMP_CSI_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_CSI_REC)
#define m_MMGR_FREE_AVND_COMP_CSI_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_CSI_REC)

#define m_MMGR_ALLOC_AVND_COMP_HC_REC      (AVND_COMP_HC_REC*)m_NCS_MEM_ALLOC(sizeof(AVND_COMP_HC_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_HC_REC)
#define m_MMGR_FREE_AVND_COMP_HC_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_HC_REC)

#define m_MMGR_ALLOC_AVND_COMP_PM_REC      (AVND_COMP_PM_REC*)m_NCS_MEM_ALLOC(sizeof(AVND_COMP_PM_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_PM_REC)
#define m_MMGR_FREE_AVND_COMP_PM_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_PM_REC)

#define m_MMGR_ALLOC_AVND_COMP_CBK      (AVND_COMP_CBK*)m_NCS_MEM_ALLOC(sizeof(AVND_COMP_CBK), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_CBK)
#define m_MMGR_FREE_AVND_COMP_CBK(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_CBK)

#define m_MMGR_ALLOC_AVND_HC      (AVND_HC*)m_NCS_MEM_ALLOC(sizeof(AVND_HC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_HC)
#define m_MMGR_FREE_AVND_HC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_HC)

#define m_MMGR_ALLOC_AVND_PG      (AVND_PG*)m_NCS_MEM_ALLOC(sizeof(AVND_PG), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_PG)
#define m_MMGR_FREE_AVND_PG(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_PG)

#define m_MMGR_ALLOC_AVND_PG_MEM      (AVND_PG_MEM*)m_NCS_MEM_ALLOC(sizeof(AVND_PG_MEM), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_PG_MEM)
#define m_MMGR_FREE_AVND_PG_MEM(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_PG_MEM)

#define m_MMGR_ALLOC_AVND_PG_TRK      (AVND_PG_TRK*)m_NCS_MEM_ALLOC(sizeof(AVND_PG_TRK), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_PG_TRK)
#define m_MMGR_FREE_AVND_PG_TRK(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_PG_TRK)

#define m_MMGR_ALLOC_AVND_DND_MSG_LIST      (AVND_DND_MSG_LIST*)m_NCS_MEM_ALLOC(sizeof(AVND_DND_MSG_LIST), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_DND_MSG_LIST)
#define m_MMGR_FREE_AVND_DND_MSG_LIST(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_DND_MSG_LIST)
#define m_MMGR_ALLOC_AVND_COMP_PXIED_REC      (AVND_COMP_PXIED_REC*)m_NCS_MEM_ALLOC(sizeof(AVND_COMP_PXIED_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_PXIED_REC)
#define m_MMGR_FREE_AVND_COMP_PXIED_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_COMP_PXIED_REC)

#define m_MMGR_ALLOC_AVND_NODEID_MDSDEST  (AVND_NODEID_TO_MDSDEST_MAP *)m_NCS_MEM_ALLOC(sizeof(AVND_NODEID_TO_MDSDEST_MAP), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_NODEID_TO_MDSDEST)

#define m_MMGR_FREE_AVND_NODEID_MDSDEST(p)  m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_NODEID_TO_MDSDEST)

#define m_MMGR_ALLOC_AVND_ASYNC_UPDT  (AVND_ASYNC_UPDT_MSG_QUEUE*)m_NCS_MEM_ALLOC(sizeof(AVND_ASYNC_UPDT_MSG_QUEUE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_ASYNC_UPDT)
#define m_MMGR_FREE_AVND_ASYNC_UPDT(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVND, \
                                                NCS_SERVICE_AVND_SUB_ID_AVND_ASYNC_UPDT)

#endif   /* !AVND_MEM_H */
