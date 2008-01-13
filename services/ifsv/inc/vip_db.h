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
#ifndef VIP_DB_H
#define VIP_DB_H

#include "ifa.h"

/* This file contains the structures which are used as database for VIP functionality */

/* Macros Used for vip db */

#define        IFSV_VIP_ALLOCATED         0x01 /* Indicates Whether VIPD entry is allocated or not */
#define        IFSV_VIP_POOL              0x02 /* Indicates whether the IP is single or pool */
#define        IFSV_VIP_CONTINUOUS        0x04 /* Indicates whether the IP list is continuous or not */
#define        IFSV_VIP_APPL_IP           0x08 /* Indicates whether the IP is APPL IP or NODE IP */
#define        IFSV_VIP_ENTRY_COMPLETE    0x10 /* Indicates whether the VIPDC entry is compelte or not */
#define        IFSV_VIP_APPL_CONFIGURED   0x20 /* Indicates whether the entry is configured by appln or admin */
#define        IFSV_VIP_STALE_ENTRY       0x40 /* Indicates That the entry is stale and not in use */



/*****************************************************************************
 * Macros used to check the presence of attributes in the record data.
 *****************************************************************************/
#define m_IFSV_IS_VIP_ALLOCATED(attr)   \
            (((attr & IFSV_VIP_ALLOCATED) != 0)?TRUE:FALSE)
#define m_IFSV_IS_VIP_POOL(attr)  \
            (((attr & IFSV_VIP_POOL) != 0)?TRUE:FALSE)
#define m_IFSV_IS_VIP_CONTINUOUS(attr)  \
            (((attr & IFSV_VIP_CONTINUOUS) != 0)?TRUE:FALSE)
#define m_IFSV_IS_VIP_APPLIP(attr) \
            (((attr & IFSV_VIP_APPL_IP) != 0)?TRUE:FALSE)
#define m_IFSV_IS_VIP_ENTRY_COMPLETE(attr) \
            (((attr & IFSV_VIP_ENTRY_COMPLETE) != 0)?TRUE:FALSE)
#define m_IFSV_IS_VIP_APPL_CONFIGURED(attr) \
            (((attr & IFSV_VIP_APPL_CONFIGURED) != 0)?TRUE:FALSE)
#define m_IFSV_IS_VIP_ENTRY_STALE(attr) \
            (((attr & IFSV_VIP_STALE_ENTRY) != 0)?TRUE:FALSE)

/******************************************************************************
 Macros to set the attributes
 *****************************************************************************/
#define m_IFSV_VIP_ALLOCATED_SET(attr)        (attr = attr | IFSV_VIP_ALLOCATED)
#define m_IFSV_VIP_POOL_SET(attr)             (attr = attr | IFSV_VIP_SINGLE)
#define m_IFSV_VIP_CONTINUOUS_SET(attr)       (attr = attr | IFSV_VIP_CONTINUOUS)
#define m_IFSV_VIP_APPLIP_SET(attr)           (attr = attr | IFSV_VIP_APPL_IP)
#define m_IFSV_VIP_ENTRY_COMPLETE_SET(attr)   (attr = attr | IFSV_VIP_ENTRY_COMPLETE)
#define m_IFSV_VIP_APPL_CONFIGURED_SET(attr)  (attr = attr | IFSV_VIP_APPL_CONFIGURED)
#define m_IFSV_VIP_STALE_ENTRY_SET(attr)      (attr = attr | IFSV_VIP_STALE_ENTRY)




/******************************************************************************
 Macros to free the attributes
 *****************************************************************************/
#define m_IFSV_VIP_ALLOCATED_FREE(attr)         (attr = attr & ~IFSV_VIP_ALLOCATED)
#define m_IFSV_VIP_POOL_FREE(attr)              (attr = attr & ~IFSV_VIP_SINGLE)
#define m_IFSV_VIP_CONTINUOUS_FREE(attr)        (attr = attr & ~IFSV_VIP_CONTINUOUS)
#define m_IFSV_VIP_APPLIP_FREE(attr)            (attr = attr & ~IFSV_VIP_APPL_IP)
#define m_IFSV_VIP_ENTRY_COMPLETE_FREE(attr)    (attr = attr & ~IFSV_VIP_ENTRY_COMPLETE)
#define m_IFSV_VIP_APPL_CONFIGURED_FREE(attr)   (attr = attr & ~IFSV_VIP_APPL_CONFIGURED)
#define m_IFSV_VIP_ENTRY_STALE_FREE(attr)       (attr = attr & ~IFSV_VIP_STALE_ENTRY)

#define IFSV_VIP_REC_TYPE_VIPD    0
#define IFSV_VIP_REC_TYPE_VIPDC   1


typedef struct ncs_ifsv_vip_ip_list
{
   NCS_DB_LINK_LIST_NODE   lnode; /* Linked list node */
   NCS_IPPFX               ip_addr;
   NCS_BOOL                ipAllocated; /* specifies if the IP IS currently installed or not */
   uns8                    intfName[m_NCS_IFSV_VIP_INTF_NAME]; /* Specifies the intf on which ip is installed */
}NCS_IFSV_VIP_IP_LIST ;

typedef struct ncs_ifsv_vip_intf_list
{
   NCS_DB_LINK_LIST_NODE       lnode; /* Linked list node */
   uns8                        intf_name[m_NCS_IFSV_VIP_INTF_NAME]; /* this include <node><Plane A or B>-<interface name> Eg."Node1-A:Eth0", means Node 1, plane-A,  Eth0*/
/* TBD : match this definitions with SAF definitions */
#define  NCS_VIP_INTERFACE_ACTIVE   0
#define  NCS_VIP_INTERFACE_STANDBY  1
   NCS_BOOL                    active_standby; /* This indicates whether the interface is Active/standby */
}NCS_IFSV_VIP_INTF_LIST;

typedef struct ifsv_ifd_vipd_record
{
   NCS_PATRICIA_NODE             pNode;
   NCS_IFSV_VIP_INT_HDL          handle;
   uns32                         vip_entry_attr;
   uns32                         ref_cnt;
/* TBD : may be in future this two attribues ip_list and intf_list will be merged */

   NCS_DB_LINK_LIST              ip_list;
   NCS_DB_LINK_LIST              intf_list;
   NCS_DB_LINK_LIST              alloc_ip_list;
   NCS_DB_LINK_LIST              owner_list;
   /* add an entry here to specify, which IfND is the owner of given entry */
}IFSV_IFD_VIPD_RECORD;


/* This datastructure is used by IfND as a cache of VIPD for
the temporary storage of VIP and application information */
typedef struct ifsv_ifnd_vipdc
{
   NCS_PATRICIA_NODE         pNode;
   NCS_IFSV_VIP_INT_HDL      handle;
   NCS_BOOL                  state; /* partial - 0-- This is the case when IfND learns the info from IPXS sync after IfND crash and waiting for info from IfA .   Complete : This is when IfND learns the info both from IfADb and IPXS sync after IfND crash */
   uns32                     ref_cnt;
   uns32                     vip_entry_attr;

   NCS_DB_LINK_LIST          ip_list;
   NCS_DB_LINK_LIST          intf_list;
   NCS_DB_LINK_LIST          owner_list;
}IFSV_IFND_VIPDC;



typedef struct owner_list
{
   NCS_DB_LINK_LIST_NODE     lnode;
   MDS_DEST                  owner;
/* The following infraFlag is strictly for future use */
#if (VIP_HALS_SUPPORT == 1)
   uns32                  infraFlag;
#endif
}NCS_IFSV_VIP_OWNER_LIST;


/* This datastructure is used by IfA as a local database */
typedef struct ifsv_vip_ifadb
{
   NCS_PATRICIA_NODE         pNode;
   NCS_IFSV_VIP_INT_HDL       handle;
}NCS_IFSV_VIP_IFADB ;

/*Added By Achyut for MIB Purpose, Do not Remove */
typedef struct vip_mib_attrib_tag{
        uns32 ipType;
        uns32 ipRange;
        uns32 row_status;
} NCS_VIP_MIB_ATTRIB;


NCS_DB_LINK_LIST_NODE * ifsv_vip_search_infra_owner(NCS_DB_LINK_LIST * ownerList);
extern uns32 ifsv_vip_free_ip_list(NCS_DB_LINK_LIST *ipList);
extern uns32 ifsv_vip_free_intf_list(NCS_DB_LINK_LIST *intfList);
extern uns32 ifsv_vip_free_alloc_ip_list(NCS_DB_LINK_LIST *allocIpList);
extern uns32 ifsv_vip_free_owner_list(NCS_DB_LINK_LIST *ownerList);
uns32 ifd_vip_process_ifnd_crash(MDS_DEST *pIfndDst,IFSV_CB *pIfsvCb);
uns32 ifd_proc_ifnd_get_ip_from_stale_entry(IFSV_CB *cb, IFSV_EVT *pEvt);
uns32 ifd_vip_mark_entry_stale(IFSV_CB *cb,IFSV_EVT *pEvt);
uns32 ifnd_proc_vip_mark_vipd_stale(IFSV_CB *cb, IFSV_EVT  *pEvt);
uns32 ifnd_ifa_proc_get_ip_from_stale_entry(IFSV_CB *cb, IFSV_EVT *evt);


/*uns32 ifa_vip_ip_lib_init(IFA_CB *cb);*/
#endif
#endif

