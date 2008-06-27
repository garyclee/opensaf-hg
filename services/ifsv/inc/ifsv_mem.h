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

#ifndef IFSV_MEM_H
#define IFSV_MEM_H

/* Sub service ID of IfSv serices */
typedef enum ncs_service_ifsv_sub_id
{
   NCS_IFSV_SVC_SUB_ID_IFSV_EVT,
   NCS_IFSV_SVC_SUB_ID_CB,
   NCS_IFSV_SVC_SUB_ID_INTF_REC,
   NCS_IFSV_SVC_SUB_ID_IFINDEX_MAP,
   NCS_IFSV_SVC_SUB_ID_IFINDEX_RESV_QUE,
   NCS_IFSV_SVC_SUB_ID_PORT_REG_TBL,
   NCS_IFSV_SVC_SUB_ID_IDIM_EVT,
   NCS_IFSV_SVC_SUB_ID_IDIM_CB,
   NCS_IFSV_SVC_SUB_ID_DRV_EVT,
   NCS_IFSV_SVC_SUB_ID_SUBSCR_INFO,
   NCS_IFSV_SVC_SUB_ID_IFA_CB,
   NCS_IFSV_SVC_SUB_ID_NCS_SVDEST,
   NCS_IFSV_SVC_SUB_ID_NCS_IAPS,
   NCS_IFSV_SVC_SUB_ID_IFSV_BIND_NODE,
#if(NCS_IFSV_IPXS == 1)               /* Put IPXS at the end */
   NCS_IFSV_SVC_SUB_ID_IPXS_CB,
   NCS_IFSV_SVC_SUB_ID_IPXS_IP_NODE,
   NCS_IFSV_SVC_SUB_ID_IPXS_IFIP_NODE,
   NCS_IFSV_SVC_SUB_ID_IPXS_DEFAULT,
   NCS_IFSV_SVC_SUB_ID_IPXS_EVT,
   NCS_IFSV_SVC_SUB_ID_IPXS_SUBCR_INFO,
   NCS_IFSV_SVC_SUB_ID_NCS_IPXS_INTF_REC,
#endif
#if(NCS_VIP == 1)
   NCS_IFSV_SVC_SUB_ID_VIPDC_REC,
   NCS_IFSV_SVC_SUB_ID_VIPD_REC,
   NCS_IFSV_SVC_SUB_ID_IFADB_REC,
   NCS_IFSV_SVC_SUB_ID_IP_NODE,
   NCS_IFSV_SVC_SUB_ID_INTF_NODE,
   NCS_IFSV_SVC_SUB_ID_OWNER_NODE,
   NCS_IFSV_SVC_SUB_ID_ALLOC_IP_NODE,

/* For VIP CheckPointing Purpose */
   NCS_IFSV_SVC_SUB_ID_VIP_REDUNDANCY_REC,
   NCS_IFSV_SVC_SUB_ID_VIP_RED_IP_REC,
   NCS_IFSV_SVC_SUB_ID_VIP_RED_INTF_REC,
   NCS_IFSV_SVC_SUB_ID_VIP_RED_OWNER_REC,
#endif
   NCS_IFSV_SVC_SUB_ID_IFSV_TMR,
   NCS_IFSV_SVC_SUB_ID_MAX
}NCS_SERVICE_IFSV_SUB_ID;

#define m_MMGR_ALLOC_MDS_DEST_INFO_REC \
             (IFND_MDS_DEST_INFO_REC *) \
             m_NCS_MEM_ALLOC(sizeof(IFND_MDS_DEST_INFO_REC), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_MDS_DEST_INFO_REC(ptr) m_NCS_MEM_FREE(ptr, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)

#define m_MMGR_ALLOC_IFD_NODE_ID_INFO_REC \
        (IFD_IFND_NODE_ID_INFO_REC *) \
        m_NCS_MEM_ALLOC(sizeof(IFD_IFND_NODE_ID_INFO_REC), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_IFD_NODE_ID_INFO_REC(ptr) m_NCS_MEM_FREE(ptr, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)

#define m_MMGR_ALLOC_IFD_MBCSV_MSG \
          (IFD_A2S_MSG*)m_NCS_MEM_ALLOC(sizeof(IFD_A2S_MSG), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_IFD_MBCSV_MSG(ptr) m_NCS_MEM_FREE(ptr, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)

#define m_MMGR_ALLOC_INFTDATA \
           (IFSV_INTF_DATA *)m_NCS_MEM_ALLOC(sizeof(IFSV_INTF_DATA), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_INFTDATA(ptr)  m_NCS_MEM_FREE(ptr, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)

#define m_MMGR_ALLOC_IFAP_INFO_LIST \
           (IFAP_INFO_LIST_A2S *)m_NCS_MEM_ALLOC(sizeof(IFAP_INFO_LIST_A2S), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_IFAP_INFO_LIST(ptr)  m_NCS_MEM_FREE(ptr, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_ALLOC_IPXSDATA \
              (NCS_IPXS_IPINFO *)m_NCS_MEM_ALLOC(sizeof(NCS_IPXS_IPINFO), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_IPXSDATA(ptr)  m_NCS_MEM_FREE(ptr, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)

#define m_MMGR_ALLOC_IFAP_INFO(num_free_index)  (NCS_IFSV_IFINDEX *) \
                 m_NCS_MEM_ALLOC((num_free_index * sizeof(NCS_IFSV_IFINDEX)),\
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_IFAP_INFO(ptr) m_NCS_MEM_FREE(ptr, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)

#define m_MMGR_ALLOC_IFSV_EVT (IFSV_EVT*)m_NCS_MEM_ALLOC(sizeof(IFSV_EVT), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)
#define m_MMGR_FREE_IFSV_EVT(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_EVT)

#define m_MMGR_ALLOC_IFSV_CB (IFSV_CB*)m_NCS_MEM_ALLOC(sizeof(IFSV_CB), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_CB)
#define m_MMGR_FREE_IFSV_CB(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_CB)

#define m_MMGR_ALLOC_IFSV_INTF_REC (IFSV_INTF_REC*)m_NCS_MEM_ALLOC(sizeof(IFSV_INTF_REC), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_INTF_REC)
#define m_MMGR_FREE_IFSV_INTF_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_INTF_REC)

#define m_MMGR_ALLOC_IFSV_S_P_T_INDEX_MAP (IFSV_SPT_REC*)m_NCS_MEM_ALLOC(sizeof(IFSV_SPT_REC), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFINDEX_MAP)
#define m_MMGR_FREE_IFSV_S_P_T_INDEX_MAP(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFINDEX_MAP)

#define m_MMGR_ALLOC_IFSV_IF_RESV (IFSV_IFINDEX_RESV_WORK_QUEUE*)m_NCS_MEM_ALLOC(sizeof(IFSV_IFINDEX_RESV_WORK_QUEUE), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFINDEX_RESV_QUE)
#define m_MMGR_FREE_IFSV_IF_RESV(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFINDEX_RESV_QUE)

#define m_MMGR_ALLOC_PORT_REG_TBL (IFSV_IDIM_PORT_TYPE_REG_TBL*)m_NCS_MEM_ALLOC(sizeof(IFSV_IDIM_PORT_TYPE_REG_TBL), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_PORT_REG_TBL)
#define m_MMGR_FREE_PORT_REG_TBL(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_PORT_REG_TBL)

#define m_MMGR_ALLOC_IFSV_IDIM_EVT (IFSV_IDIM_EVT*)m_NCS_MEM_ALLOC(sizeof(IFSV_IDIM_EVT), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IDIM_EVT)
#define m_MMGR_FREE_IFSV_IDIM_EVT(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IDIM_EVT)

#define m_MMGR_ALLOC_IFSV_HW_DRV_EVT (NCS_IFSV_HW_DRV_REQ*)m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_HW_DRV_REQ), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_DRV_EVT)
#define m_MMGR_FREE_IFSV_HW_DRV_EVT(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_DRV_EVT)

#define m_MMGR_ALLOC_IFSV_NCS_SVDEST(cnt) (NCS_SVDEST*)m_NCS_MEM_ALLOC(cnt*sizeof(NCS_SVDEST), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_NCS_SVDEST)
#define m_MMGR_FREE_IFSV_NCS_SVDEST(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_NCS_SVDEST)
#define m_MMGR_ALLOC_IFSV_TMR      (IFSV_TMR*)m_NCS_MEM_ALLOC(sizeof(IFSV_TMR), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_TMR)
#define m_MMGR_FREE_IFSV_TMR(ptr)  m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_TMR)

#define m_MMGR_ALLOC_IFSV_BIND_NODE (IFSV_BIND_NODE*)m_NCS_MEM_ALLOC(sizeof(IFSV_BIND_NODE), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_BIND_NODE)
#define m_MMGR_FREE_IFSV_BIND_NODE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFSV_BIND_NODE)


#if (NCS_IFND == 1)
#define m_MMGR_ALLOC_IFND_IDIM_CB (IFSV_IDIM_CB*)m_NCS_MEM_ALLOC(sizeof(IFSV_IDIM_CB), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IDIM_CB)
#define m_MMGR_FREE_IFND_IDIM_CB(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IDIM_CB)
#endif

#if(NCS_IFA == 1)

#define m_MMGR_ALLOC_IFA_CB (IFA_CB*)m_NCS_MEM_ALLOC(sizeof(IFSV_CB), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFA_CB)
#define m_MMGR_FREE_IFA_CB(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IFA_CB)

#define m_MMGR_ALLOC_IFSV_SUBSCR_INFO (IFSV_SUBSCR_INFO*)m_NCS_MEM_ALLOC(sizeof(IFSV_SUBSCR_INFO), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_SUBSCR_INFO)
#define m_MMGR_FREE_IFSV_SUBSCR_INFO(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_SUBSCR_INFO)

#endif  /* (NCS_IFA == 1) */

#endif
