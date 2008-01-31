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

#ifndef IPXS_MEM_H
#define IPXS_MEM_H

/* Enum NCS_IFSV_SVC_SUB_ID_IPXS_CB is defined in ifsv_mem.h */

#define m_MMGR_ALLOC_IPXS_CB (IPXS_CB*)m_NCS_MEM_ALLOC(sizeof(IPXS_CB), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_CB)
#define m_MMGR_FREE_IPXS_CB(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_CB)

#define m_MMGR_ALLOC_IPXS_IP_NODE (IPXS_IP_NODE*)m_NCS_MEM_ALLOC(sizeof(IPXS_IP_NODE), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_IP_NODE)
#define m_MMGR_FREE_IPXS_IP_NODE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_IP_NODE)

#define m_MMGR_ALLOC_IPXS_IFIP_NODE (IPXS_IFIP_NODE*)m_NCS_MEM_ALLOC(sizeof(IPXS_IFIP_NODE), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_IFIP_NODE)
#define m_MMGR_FREE_IPXS_IFIP_NODE(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_IFIP_NODE)

#define m_MMGR_ALLOC_IPXS_DEFAULT(size) (uns8 *)m_NCS_MEM_ALLOC(size, \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_DEFAULT)
#define m_MMGR_FREE_IPXS_DEFAULT(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_DEFAULT)

#define m_MMGR_ALLOC_IPXS_EVT (IPXS_EVT*)m_NCS_MEM_ALLOC(sizeof(IPXS_EVT), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_EVT)
#define m_MMGR_FREE_IPXS_EVT(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_EVT)

#define m_MMGR_ALLOC_IPXS_SUBCR_INFO (IPXS_SUBCR_INFO*)m_NCS_MEM_ALLOC(sizeof(IPXS_SUBCR_INFO), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_SUBCR_INFO)
#define m_MMGR_FREE_IPXS_SUBCR_INFO(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_IPXS_SUBCR_INFO)

#define m_MMGR_ALLOC_NCS_IPXS_INTF_REC (NCS_IPXS_INTF_REC*)m_NCS_MEM_ALLOC(sizeof(NCS_IPXS_INTF_REC), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_NCS_IPXS_INTF_REC)
#define m_MMGR_FREE_NCS_IPXS_INTF_REC(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFSV, \
                                   NCS_IFSV_SVC_SUB_ID_NCS_IPXS_INTF_REC)
#endif

