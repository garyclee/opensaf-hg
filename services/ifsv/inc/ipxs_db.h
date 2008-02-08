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
FILE NAME: IPXS_DB.H
DESCRIPTION: Enums, Structures and Function prototypes for IPXS
******************************************************************************/

#ifndef IPXS_DB_H
#define IPXS_DB_H

/* Will go into ipxs_papi.h */

/* IPXS is supporting IPV4 & IPV6 address families */
#define  IPXS_AFI_MAX   2
#define  NET_LINK_NUM_IP_ADDR_MAX   50

/* Macro to get the IPXS CB handle */
#define m_IPXS_CB_HDL_GET( ) (gl_ipxs_cb_hdl)

#define m_IPXS_STORE_HDL(cb_hdl) (gl_ipxs_cb_hdl = (cb_hdl))

extern uns32 gl_ipxs_cb_hdl;

/* IP Data Base */
/* N/W cmd database */
typedef struct ipxs_ipdb
{
   NCS_PATRICIA_TREE    ptree;
   NCS_BOOL             up;      /* Has the IP DB is UP*/
   NCS_IP_ADDR_TYPE     type;
} IPXS_IPDB;

/* IPXS Subscription information */
typedef struct ipxs_subcr_info
{
   NCS_DB_LINK_LIST_NODE   lnode;   /* Linked list node */
   NCS_IPXS_SUBCR          info;    /* Subscription information received 
                                       from application */
} IPXS_SUBCR_INFO;

typedef enum {
   IPXS_NETLINK_OK,
   IPXS_NETLINK_REFRESH_CACHE,
   IPXS_NETLINK_EOF,
   IPXS_NETLINK_EINTR,
   IPXS_NETLINK_SENDER_ADDR_LEN_MISMATCH,
   IPXS_NETLINK_WRONG_NLMSG_LEN,
   IPXS_NETLINK_ALLOC_IPXS_EVT_FAIL,
   IPXS_NETLINK_ALLOC_IFSV_EVT_FAIL,
   IPXS_NETLINK_SEND_IFSV_EVT_TO_MBX_FAIL,
   IPXS_NETLINK_NEW_INTF
}IPXS_NETLINK_RETVAL;

typedef struct ipxs_ifindex_cache_tag
{
   char                 ifname[IFSV_IF_NAME_SIZE];
   int                  interface_number; /* Learnt from kernel */
   NCS_IFSV_IFINDEX     if_index;  /* IFSv specific */
}IPXS_IFINDEX_CACHE;

/**************************************************************
* This structure will be included in the IPXS_CB.
* Used to hold NETLINK IP addresses when switchover happens.
**************************************************************/
typedef struct ifindex_ip_addr_list
{
   NCS_IFSV_IFINDEX  if_index;
   NCS_IPPFX         ippfx;
}IFINDEX_IP_ADDR_LIST;

typedef struct netlink_addr_info 
{
   uns32                  num_ip_addr;
   IFINDEX_IP_ADDR_LIST   list[NET_LINK_NUM_IP_ADDR_MAX];
}NETLINK_ADDR_INFO;


/* Global Data base used by the IPXS */
typedef struct ipxs_cb
{
   uns32                cb_hdl;    /* Handle manager Struct Handle */
   NCS_SERVICE_ID       my_svc_id; /* Service ID of the IfD ro IfND  */
   MDS_HDL              my_mds_hdl;/* MDS PWE handle   */
   MDS_DEST             my_dest;
   IPXS_IPDB            ip_tbl[IPXS_AFI_MAX];  /* IP address database */
   NCS_PATRICIA_TREE    ifip_tbl;   /* Interface table that stores IP info */
   NCS_BOOL             ifip_tbl_up;
   NCSIPXS_NODE_REC     node_info;
   NCS_BOOL             is_full_netlink_updt_in_progress;
   IPXS_IFINDEX_CACHE   ifndx_cache[512];  /* Index here is the interface-number */
   
   uns32                oac_hdl;     /* OAC Handle */
   uns8                 hm_pid;     /* Handle Manager Pool ID */
   uns32                netlink_fd; /* Netlink socket fd */
   SYSF_MBX             mbx;
   NCS_BOOL             netlink_updated;
   NETLINK_ADDR_INFO    nl_addr_info;
   NCS_LOCK             ipxs_db_lock; /* Lock to sync DB between IfND and Netlink thread*/
}IPXS_CB;

/*****************************************************************************
 * Data Structure Used to hold the ip address node, used for IPXS.
 * IP ADDRESS is the key for searching this record 
 *****************************************************************************/

typedef struct ipxs_ip_key
{
   union
   {
      NCS_IPV4_ADDR v4;
#if (NCS_IPV6 == 1)
      NCS_IPV6_ADDR v6;
#endif
   } ip;
}IPXS_IP_KEY;


/*****************************************************************************
 * Data Structure Used to hold the IP information (IP Database )
 *****************************************************************************/
typedef struct ipxs_ip_info
{
   IPXS_IP_KEY          keyNet;
   NCS_IP_ADDR          addr;
   uns8                 mask;
   NCS_IFSV_IFINDEX     if_index;
   uns32                status;
} IPXS_IP_INFO;

#define IPXS_IP_INFO_NULL ((IPXS_IP_INFO*)0)

typedef struct ipxs_ip_node
{
   NCS_PATRICIA_NODE    pat_node;
   IPXS_IP_INFO         ipinfo;
} IPXS_IP_NODE;

/*****************************************************************************
 * Data Structure Used to hold the IF - IP information per interface
 * if_index is the key for searching this record 
 *****************************************************************************/
typedef struct ipxs_ipif_info
{
   uns32                ifindexNet;
   NCS_IFSV_BM          ipam;           /* IP info Attribute MAP */
   uns8                 ipaddr_cnt;     /* IP addresses count in this interface */
   IPXS_IFIP_IP_INFO   *ipaddr_list;   /* List of IP addresses */
   NCS_BOOL             is_v4_unnmbrd;  /* IPV4 unnumbred Flag */
   NCS_SVDEST           *svdest_list;   /* List of SVC-Card ID Dest Pair */
   uns8                 svdest_cnt;     /* Count of the SVC Dests */
   NCS_IP_ADDR          rrtr_ipaddr;    /* node addr at other end of this iface   */
   uns32                rrtr_rtr_id;    /* router id at other end of this iface   */
   NCS_IFSV_IFINDEX     rrtr_if_id;     /* if_index at the otherend of this iface */
   uns16                rrtr_as_id;     /* Autonomous System ID at other end....  */
   uns32                ud_1;           /* User Defined 1                         */
#if (NCS_VIP == 1)
   uns8                 intfName[m_NCS_IFSV_VIP_INTF_NAME];
   uns32                shelfId;
   uns32                slotId;
   uns32                nodeId;
   /* Here we need to add list of VipApplNames, as it would be
    * used for phase-11 */
#endif

} IPXS_IFIP_INFO;

#define IPXS_IFIP_INFO_NULL ((IPXS_IFIP_INFO*)0)

typedef struct ipxs_ipif_node
{
   NCS_PATRICIA_NODE    patnode;
   IPXS_IFIP_INFO       ifip_info;
} IPXS_IFIP_NODE;

/* Seperate IP database is maintained per address family */
#define  m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, type)  \
{                                            \
   if(type == NCS_IP_ADDR_TYPE_IPV4)         \
      index = 0;                             \
   else if(type == NCS_IP_ADDR_TYPE_IPV6)    \
      index = IPXS_AFI_MAX;                  \
   else                                      \
      index = IPXS_AFI_MAX;                  \
}

#define m_IPXS_PFX_KEYLEN_SET(keylen, type)    \
   ipxs_pfx_keylen_set(&keylen, type)            

EXTERN_C void ipxs_db_index_from_addrtype_get (uns32 *index, NCS_IP_ADDR_TYPE type);
EXTERN_C void ipxs_pfx_keylen_set(uns32 *keylen, NCS_IP_ADDR_TYPE type);

EXTERN_C void ipxs_ifip_db_destroy(NCS_PATRICIA_TREE *ifip_tbl, NCS_BOOL *up);

EXTERN_C IPXS_IFIP_NODE* 
ipxs_ifip_record_get(NCS_PATRICIA_TREE *ifip_tbl, uns32 ifindex);

EXTERN_C uns32
ipxs_ifip_record_add(IPXS_CB *cb, NCS_PATRICIA_TREE *ifip_tbl, IPXS_IFIP_NODE *ifip_node);

EXTERN_C uns32
ipxs_ifip_record_del(IPXS_CB *cb, NCS_PATRICIA_TREE *ifip_tbl, IPXS_IFIP_NODE *ifip_node);

EXTERN_C uns32 
ipxs_ifip_ippfx_add(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, IPXS_IFIP_IP_INFO *ippfx,
                    uns32 cnt, NCS_BOOL *add_flag);

EXTERN_C uns32 
ipxs_ifip_ippfx_del(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, IPXS_IFIP_IP_INFO *ippfx,
                    uns32 cnt, NCS_BOOL *del_flag);
EXTERN_C uns32 
ipxs_ifip_ippfx_add_to_os(IPXS_CB *cb, NCS_IFSV_IFINDEX ifindex,
                          IPXS_IFIP_IP_INFO *ippfx, uns32 cnt);

EXTERN_C uns32 
ipxs_ifip_ippfx_del_from_os(IPXS_CB *cb, NCS_IFSV_IFINDEX ifindex,
                            NCS_IPPFX *ippfx, uns32 cnt);
EXTERN_C uns32
ifsv_add_to_os(IFSV_INTF_DATA *intf_data, IPXS_IFIP_IP_INFO *ippfx, uns32 cnt, IFSV_CB *ifsv_cb);
 
EXTERN_C uns32
ifsv_del_from_os(IFSV_INTF_DATA *intf_data, NCS_IPPFX *ippfx, uns32 cnt, IFSV_CB *ifsv_cb);

EXTERN_C void ipxs_ifd_ip_rec_del(IPXS_IPDB *ip_tbl, IPXS_IP_NODE *ipnode);

EXTERN_C void ipxs_ipdb_destroy(IPXS_IPDB *iptbl);

EXTERN_C uns32 ipxs_ipdb_init(IPXS_IPDB *iptbl);

EXTERN_C uns32 ipxs_ip_record_del(IPXS_CB *cb, IPXS_IPDB *ip_db, IPXS_IP_NODE *ip_node);

EXTERN_C void ipxs_get_ipkey_from_ipaddr(IPXS_IP_KEY  *ip_key, NCS_IP_ADDR *addr);

EXTERN_C void ipxs_ip_record_list_del(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info);

#endif /* IPXS_DB_H */
