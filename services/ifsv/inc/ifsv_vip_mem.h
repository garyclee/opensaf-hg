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

#if (NCS_VIP == 1)

#ifndef IFSV_VIP_MEM_H
#define IFSV_VIP_MEM_H

#define m_MMGR_ALLOC_IFSV_VIPDC_REC (IFSV_IFND_VIPDC*)m_NCS_MEM_ALLOC(sizeof(IFSV_IFND_VIPDC), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIPDC_REC)

#define m_MMGR_FREE_IFSV_VIPDC_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIPDC_REC)

#define m_MMGR_ALLOC_IFSV_VIPD_REC (IFSV_IFD_VIPD_RECORD*)m_NCS_MEM_ALLOC(sizeof(IFSV_IFD_VIPD_RECORD), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIPD_REC)

#define m_MMGR_FREE_IFSV_VIPD_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIPD_REC)

#define m_MMGR_ALLOC_IFSV_IFADB   (NCS_IFSV_VIP_IFADB*)m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_VIP_IFADB), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_IFADB_REC)

#define m_MMGR_FREE_IFSV_IFADB(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_IFADB_REC)

#define m_MMGR_ALLOC_IFSV_IP_NODE (NCS_IFSV_VIP_IP_LIST *)m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_VIP_IP_LIST), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_IP_NODE)

#define m_MMGR_FREE_IFSV_IP_NODE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_IP_NODE)

#define m_MMGR_ALLOC_IFSV_INTF_NODE (NCS_IFSV_VIP_INTF_LIST *)m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_VIP_INTF_LIST), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_INTF_NODE)

#define m_MMGR_FREE_IFSV_INTF_NODE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_INTF_NODE)

#define m_MMGR_ALLOC_IFSV_OWNER_NODE (NCS_IFSV_VIP_OWNER_LIST * )m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_VIP_OWNER_LIST), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_OWNER_NODE)

#define m_MMGR_FREE_IFSV_OWNER_NODE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_OWNER_NODE)

#define m_MMGR_ALLOC_IFSV_ALLOC_IP_NODE (NCS_IFSV_VIP_ALLOC_IP_LIST * )m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_VIP_ALLOC_IP_LIST), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_ALLOC_IP_NODE)

#define m_MMGR_FREE_IFSV_ALLOC_IP_NODE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_ALLOC_IP_NODE)

/* For CheckPointing Record Creation*/
#define m_MMGR_ALLOC_IFSV_VIP_REDUNDANCY_REC (VIP_REDUNDANCY_RECORD*)m_NCS_MEM_ALLOC(sizeof(VIP_REDUNDANCY_RECORD), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_REDUNDANCY_REC)

#define m_MMGR_FREE_IFSV_VIP_REDUNDANCY_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_REDUNDANCY_REC)


#define m_MMGR_ALLOC_IFSV_VIP_RED_IP_REC(ip_count) (VIP_RED_IP_NODE*)m_NCS_MEM_ALLOC((ip_count) * sizeof(VIP_RED_IP_NODE), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_RED_IP_REC)


#define m_MMGR_FREE_IFSV_VIP_RED_IP_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_RED_IP_REC)


#define m_MMGR_ALLOC_IFSV_VIP_RED_INTF_REC(interface_count) (VIP_RED_INTF_NODE*)m_NCS_MEM_ALLOC((interface_count) * sizeof(VIP_RED_INTF_NODE), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_RED_INTF_REC)


#define m_MMGR_FREE_IFSV_VIP_RED_INTF_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_RED_INTF_REC)


#define m_MMGR_ALLOC_IFSV_VIP_RED_OWNER_REC(owner_count) (VIP_RED_OWNER_NODE*)m_NCS_MEM_ALLOC((owner_count) * sizeof(VIP_RED_OWNER_NODE), \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_RED_OWNER_REC)


#define m_MMGR_FREE_IFSV_VIP_RED_OWNER_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_IFSV, \
                                     NCS_IFSV_SVC_SUB_ID_VIP_RED_OWNER_REC)


#endif
#endif  /* if NCS_VIP == 1 */
