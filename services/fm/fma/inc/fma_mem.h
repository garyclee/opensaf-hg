/*****************************************************************************
 *                                                                           *
 * NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
 * AGREEMENT FROM EMERSON, INC.                                              *
 *                                                                           *
 * THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
 * SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
 *                                                                           *
 * IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
 * PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
 *                                                                           *
 *****************************************************************************
 
 
 ..............................................................................

DESCRIPTION:

******************************************************************************
*/

typedef enum
{
   FMA_SVC_SUB_ID_FMA_CB,
   FMA_SVC_SUB_ID_EVT,
   FMA_SVC_SUB_ID_FMA_HDL_REC,
   FMA_SVC_SUB_ID_FMA_PEND_CBK_REC,
   FMA_SVC_SUB_ID_FMA_CBK_INFO,
   FMA_SVC_SUB_ID_MAX
} FMA_SVC_SUB_ID;
   

/** Memory Allocation and Release Macros **/
      
   
#define m_MMGR_ALLOC_FMA_CB               (FMA_CB *)m_NCS_MEM_ALLOC(sizeof(FMA_CB), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_CB)
 
#define m_MMGR_FREE_FMA_CB(p)             m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_CB)

#define m_MMGR_ALLOC_FMA_MBX_EVT          (FMA_MBX_EVT_T*)m_NCS_MEM_ALLOC(sizeof(FMA_MBX_EVT_T), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_EVT)
 
#define m_MMGR_FREE_FMA_MBX_EVT(p)        m_NCS_MEM_FREE(p,\
                                              NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_EVT)

#define m_MMGR_ALLOC_FMA_HDL_REC       (FMA_HDL_REC*)m_NCS_MEM_ALLOC(sizeof(FMA_HDL_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_HDL_REC)
#define m_MMGR_FREE_FMA_HDL_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_HDL_REC)

#define m_MMGR_ALLOC_FMA_PEND_CBK_REC       (FMA_PEND_CBK_REC*)m_NCS_MEM_ALLOC(sizeof(FMA_PEND_CBK_REC), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_PEND_CBK_REC)

#define m_MMGR_FREE_FMA_PEND_CBK_REC(p)     m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_PEND_CBK_REC)

#define m_MMGR_ALLOC_FMA_CBK_INFO          (FMA_CBK_INFO*)m_NCS_MEM_ALLOC(sizeof(FMA_CBK_INFO), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_CBK_INFO)

#define m_MMGR_FREE_FMA_CBK_INFO(p)        m_NCS_MEM_FREE(p,\
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_FMA, \
                                                FMA_SVC_SUB_ID_FMA_CBK_INFO)
