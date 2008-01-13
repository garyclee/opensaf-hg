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

#ifndef AVD_MEM_H
#define AVD_MEM_H

/* Service Sub IDs for AVD */
typedef enum
{
   NCS_SERVICE_AVD_SUB_ID_AVD_DEFAULT_VAL = 1,
   NCS_SERVICE_AVD_SUB_ID_AVD_CL_CB,
   NCS_SERVICE_AVD_SUB_ID_AVD_AVND,
   NCS_SERVICE_AVD_SUB_ID_AVD_PG_NODE_CSI,
   NCS_SERVICE_AVD_SUB_ID_AVD_COMP,
   NCS_SERVICE_AVD_SUB_ID_AVD_CSI,
   NCS_SERVICE_AVD_SUB_ID_AVD_PG_CSI_NODE,
   NCS_SERVICE_AVD_SUB_ID_AVD_CSI_PARAM,
   NCS_SERVICE_AVD_SUB_ID_AVD_COMP_CSI_REL,
   NCS_SERVICE_AVD_SUB_ID_AVD_HLT,
   NCS_SERVICE_AVD_SUB_ID_AVD_D2D_MSG,
   NCS_SERVICE_AVD_SUB_ID_AVD_DND_MSG,
   NCS_SERVICE_AVD_SUB_ID_AVD_EVT,
   NCS_SERVICE_AVD_SUB_ID_AVD_SG,
   NCS_SERVICE_AVD_SUB_ID_AVD_SG_OPER,
   NCS_SERVICE_AVD_SUB_ID_AVD_SI,
   NCS_SERVICE_AVD_SUB_ID_AVD_SU,
   NCS_SERVICE_AVD_SUB_ID_AVD_SU_SI_REL,
   NCS_SERVICE_AVD_SUB_ID_AVD_ASYNC_UPDT,
   NCS_SERVICE_AVD_SUB_ID_AVD_D2N_MSG,
   NCS_SERVICE_AVD_SUB_ID_EVT_QUEUE,
   NCS_SERVICE_AVD_SUB_ID_CLM_NODE_ID,
   NCS_SERVICE_AVD_SUB_ID_AVD_SU_PER_SI_RANK,
   NCS_SERVICE_AVD_SUB_ID_AVD_SG_SI_RANK,
   NCS_SERVICE_AVD_SUB_ID_AVD_COMP_CS_TYPE,
   NCS_SERVICE_AVD_SUB_ID_AVD_SG_SU_RANK,
   NCS_SERVICE_AVD_SUB_ID_AVD_CS_TYPE_PARAM,
   NCS_SERVICE_AVD_SUB_ID_EDA_TLV_MSG,
   NCS_SERVICE_AVD_SUB_ID_MAX
} NCS_SERVICE_AVD_SUB_ID;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Memory Allocation and Release Macros 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


#define m_MMGR_ALLOC_AVD_DEFAULT_VAL(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_DEFAULT_VAL)
#define m_MMGR_FREE_AVD_DEFAULT_VAL(p)    m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_DEFAULT_VAL)

#define m_MMGR_ALLOC_AVD_CL_CB       (AVD_CL_CB*)m_NCS_MEM_ALLOC(sizeof(AVD_CL_CB), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CL_CB)
#define m_MMGR_FREE_AVD_CL_CB(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CL_CB)

#define m_MMGR_ALLOC_AVD_AVND       (AVD_AVND*)m_NCS_MEM_ALLOC(sizeof(AVD_AVND), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_AVND)
#define m_MMGR_FREE_AVD_AVND(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_AVND)

#define m_MMGR_ALLOC_AVD_PG_NODE_CSI       (AVD_PG_NODE_CSI*)m_NCS_MEM_ALLOC(sizeof(AVD_PG_NODE_CSI), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_PG_NODE_CSI)
#define m_MMGR_FREE_AVD_PG_NODE_CSI(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_PG_NODE_CSI)

#define m_MMGR_ALLOC_AVD_COMP       (AVD_COMP*)m_NCS_MEM_ALLOC(sizeof(AVD_COMP), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_COMP)
#define m_MMGR_FREE_AVD_COMP(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_COMP)

#define m_MMGR_ALLOC_AVD_CSI       (AVD_CSI*)m_NCS_MEM_ALLOC(sizeof(AVD_CSI), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CSI)
#define m_MMGR_FREE_AVD_CSI(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CSI)

#define m_MMGR_ALLOC_AVD_PG_CSI_NODE       (AVD_PG_CSI_NODE*)m_NCS_MEM_ALLOC(sizeof(AVD_PG_CSI_NODE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_PG_CSI_NODE)
#define m_MMGR_FREE_AVD_PG_CSI_NODE(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_PG_CSI_NODE)

#define m_MMGR_ALLOC_AVD_CSI_PARAM (AVD_CSI_PARAM*)m_NCS_MEM_ALLOC(sizeof(AVD_CSI_PARAM), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CSI_PARAM)
#define m_MMGR_FREE_AVD_CSI_PARAM(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CSI_PARAM)

#define m_MMGR_ALLOC_AVD_COMP_CSI_REL  (AVD_COMP_CSI_REL*)m_NCS_MEM_ALLOC(sizeof(AVD_COMP_CSI_REL), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_COMP_CSI_REL)
#define m_MMGR_FREE_AVD_COMP_CSI_REL(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_COMP_CSI_REL)

#define m_MMGR_ALLOC_AVD_HLT       (AVD_HLT*)m_NCS_MEM_ALLOC(sizeof(AVD_HLT), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_HLT)
#define m_MMGR_FREE_AVD_HLT(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_HLT)

#define m_MMGR_ALLOC_AVD_D2D_MSG       (AVD_D2D_MSG*)m_NCS_MEM_ALLOC(sizeof(AVD_D2D_MSG), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_D2D_MSG)
#define m_MMGR_FREE_AVD_D2D_MSG(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_D2D_MSG)

#define m_MMGR_ALLOC_AVD_EVT       (AVD_EVT*)m_NCS_MEM_ALLOC(sizeof(AVD_EVT), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_EVT)
#define m_MMGR_FREE_AVD_EVT(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_EVT)

#define m_MMGR_ALLOC_AVD_SG       (AVD_SG*)m_NCS_MEM_ALLOC(sizeof(AVD_SG), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG)
#define m_MMGR_FREE_AVD_SG(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG)

#define m_MMGR_ALLOC_AVD_SG_OPER       (AVD_SG_OPER*)m_NCS_MEM_ALLOC(sizeof(AVD_SG_OPER), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG_OPER)
#define m_MMGR_FREE_AVD_SG_OPER(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG_OPER)

#define m_MMGR_ALLOC_AVD_SI       (AVD_SI*)m_NCS_MEM_ALLOC(sizeof(AVD_SI), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SI)
#define m_MMGR_FREE_AVD_SI(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SI)

#define m_MMGR_ALLOC_AVD_SU       (AVD_SU*)m_NCS_MEM_ALLOC(sizeof(AVD_SU), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SU)
#define m_MMGR_FREE_AVD_SU(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SU)

#define m_MMGR_ALLOC_AVD_SU_SI_REL       (AVD_SU_SI_REL*)m_NCS_MEM_ALLOC(sizeof(AVD_SU_SI_REL), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SU_SI_REL)
#define m_MMGR_FREE_AVD_SU_SI_REL(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SU_SI_REL)

#define m_MMGR_ALLOC_ASYNC_UPDT  (AVSV_ASYNC_UPDT_MSG_QUEUE*)m_NCS_MEM_ALLOC(sizeof(AVSV_ASYNC_UPDT_MSG_QUEUE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_ASYNC_UPDT)
#define m_MMGR_FREE_ASYNC_UPDT(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_ASYNC_UPDT)

#define m_MMGR_ALLOC_D2N_MSG     (AVSV_ND_MSG_QUEUE*)m_NCS_MEM_ALLOC(sizeof(AVSV_ND_MSG_QUEUE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_D2N_MSG)
#define m_MMGR_FREE_D2N_MSG(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_D2N_MSG)

#define m_MMGR_ALLOC_D2D_MSG     (AVD_D2D_MSG*)m_NCS_MEM_ALLOC(sizeof(AVD_D2D_MSG), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_D2D_MSG)
#define m_MMGR_FREE_D2D_MSG(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_D2D_MSG)


#define m_MMGR_ALLOC_EVT_ENTRY     (AVD_EVT_QUEUE*)m_NCS_MEM_ALLOC(sizeof(AVD_EVT_QUEUE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_EVT_QUEUE)
#define m_MMGR_FREE_EVT_ENTRY(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_EVT_QUEUE)

#define m_MMGR_ALLOC_CLM_NODE_ID     (AVD_FAIL_OVER_NODE*)m_NCS_MEM_ALLOC(sizeof(AVD_FAIL_OVER_NODE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_CLM_NODE_ID)
#define m_MMGR_FREE_CLM_NODE_ID(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_CLM_NODE_ID)

#define m_MMGR_ALLOC_AVD_SU_PER_SI_RANK  (AVD_SUS_PER_SI_RANK*)m_NCS_MEM_ALLOC(sizeof(AVD_SUS_PER_SI_RANK), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SU_PER_SI_RANK)
#define m_MMGR_FREE_AVD_SU_PER_SI_RANK(p) m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SU_PER_SI_RANK)

#define m_MMGR_ALLOC_AVD_SG_SI_RANK     (AVD_SG_SI_RANK*)m_NCS_MEM_ALLOC(sizeof(AVD_SG_SI_RANK), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG_SI_RANK)
#define m_MMGR_FREE_AVD_SG_SI_RANK(p)   m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG_SI_RANK)

#define m_MMGR_ALLOC_AVD_COMP_CS_TYPE   (AVD_COMP_CS_TYPE*)m_NCS_MEM_ALLOC(sizeof(AVD_COMP_CS_TYPE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_COMP_CS_TYPE)
#define m_MMGR_FREE_AVD_COMP_CS_TYPE(p)  m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_COMP_CS_TYPE)

#define m_MMGR_ALLOC_AVD_SG_SU_RANK     (AVD_SG_SU_RANK*)m_NCS_MEM_ALLOC(sizeof(AVD_SG_SU_RANK), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG_SU_RANK)
#define m_MMGR_FREE_AVD_SG_SU_RANK(p)   m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_SG_SU_RANK)
#define m_MMGR_ALLOC_AVD_CS_TYPE_PARAM   (AVD_CS_TYPE_PARAM*)m_NCS_MEM_ALLOC(sizeof(AVD_CS_TYPE_PARAM), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CS_TYPE_PARAM)
#define m_MMGR_FREE_AVD_CS_TYPE_PARAM(p)  m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_AVD_CS_TYPE_PARAM)

#define m_MMGR_ALLOC_AVD_EDA_TLV_MSG(p)  (uns8*)m_NCS_MEM_ALLOC(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_EDA_TLV_MSG)

#define m_MMGR_FREE_AVD_EDA_TLV_MSG(p)  m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVD, \
                                                NCS_SERVICE_AVD_SUB_ID_EDA_TLV_MSG)

#endif /* !AVD_MEM_H */
