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
FILE NAME: IFSVIPXS.H
DESCRIPTION: Enums, Structures and Function prototypes for IPXS
******************************************************************************/

#ifndef IPXS_PAPI_H
#define IPXS_PAPI_H

#include "ifa_papi.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * Data Structure & Enums used to communicate the IF-IP info to applications
 * Used by Request Type: NCS_IPXS_REQ_IPINFO_UPD
 * Used by Response Type: NCS_IPXS_IPINFO_UPD & NCS_IPXS_IPINFO_GET_RSP.
 *****************************************************************************/
#define NCS_IPXS_MAX_IP_ADDR 10
#define NCS_IPXS_MAX_SVDEST  5

typedef uns32  NCS_IPXS_ATTR_BM; /* IP attribute bitmap */
typedef uns32  NCS_NODE_ATTR_BM; /* NODE attribute bitmap */
typedef uns32  NCS_SUBCR_BM;     /* Subscription attribute bitmap */

/* Subscription Attributes */
#define NCS_IPXS_ADD_IFACE       (0x00000001) /* Notify Interface Add events */
#define NCS_IPXS_RMV_IFACE       (0x00000002) /* Notify interfaces Remove events*/
#define NCS_IPXS_UPD_IFACE       (0x00000004) /* Notify interface  updated events */
#define NCS_IPXS_SCOPE_NODE      (0x00000008) /* Notify local (to node) interfaces */
#define NCS_IPXS_SCOPE_CLUSTER   (0x00000010) /* Notify all interfaces */

               

/* Definations for IP Layer attribute Maps */
#define NCS_IPXS_IPAM_ADDR      (0x00000001)
#define NCS_IPXS_IPAM_UNNMBD    (0x00000002)
#define NCS_IPXS_IPAM_RRTRID    (0x00000004)
#define NCS_IPXS_IPAM_RMTRID    (0x00000008)
#define NCS_IPXS_IPAM_RMT_AS    (0x00000010)
#define NCS_IPXS_IPAM_RMTIFID   (0x00000020) 
#define NCS_IPXS_IPAM_UD_1      (0x00000040)
#if (NCS_VIP == 1)
#define NCS_IPXS_IPAM_VIP       (0x00000080)
#define NCS_IPXS_IPAM_REFCNT    (0x00000100)
#endif

/* Enum to specify key type to get interface record */
typedef enum ncs_ipxs_key_type
{
   NCS_IPXS_KEY_IFINDEX = 1,       /* Key is IF-INDEX */
   NCS_IPXS_KEY_SPT,               /* Key is shelf-slot-port-type */
   NCS_IPXS_KEY_IP_ADDR,           /* Key is IP address */
   NCS_IPXS_KEY_MAX = NCS_IPXS_KEY_IP_ADDR
} NCS_IPXS_KEY_TYPE;

/***************************************************************************
* Data Structure used to store Virtual IP related information
*
***************************************************************************/
typedef struct ipxs_ifip_ip_info
{
   NCS_IPPFX            ipaddr;
#if (NCS_VIP == 1)
   uns32                poolHdl;
   uns32                ipPoolType;
   uns32                refCnt;
   NCS_BOOL             vipIp;
#endif
}IPXS_IFIP_IP_INFO;


/******************************************************************************
 * Structure to communicate IP information                                    
 *****************************************************************************/
typedef struct ncs_ipxs_ipinfo
{
   NCS_IPXS_ATTR_BM         ip_attr;        /* IP info Attribute BIT MAP */
   NCS_BOOL                 is_v4_unnmbrd;  /* IPV4 unnumbred Flag */
   IPXS_IFIP_IP_INFO        *addip_list;    /* List of ADD IP addresses */
   uns8                     addip_cnt;      /* IP addresses count in the add list*/
   NCS_IPPFX                *delip_list;    /* List of DEL IP addresses */
   uns8                     delip_cnt;      /* IP addresses count in the del list*/
   NCS_IP_ADDR              rrtr_ipaddr;    /* node addr at other end of this iface   */
   uns32                    rrtr_rtr_id;    /* router id at other end of this iface   */
   uns16                    rrtr_as_id;     /* Autonomous System ID at other end....  */
   NCS_IFSV_IFINDEX         rrtr_if_id;     /* if_index at the otherend of this iface */
   uns32                    ud_1;           /* User Defined 1                         */
#if (NCS_VIP == 1)
   uns8                     intfName[m_NCS_IFSV_VIP_INTF_NAME];
   uns32                    shelfId;
   uns32                    slotId;
   uns32                    nodeId;
   /* Here we need to add list of VipApplNames, as it would be 
    * used for phase-11 */
#endif
} NCS_IPXS_IPINFO;

typedef struct ncs_ipxs_intf_rec
{
   NCS_IFSV_IFINDEX     if_index;     /* Interface Index */
   NCS_IFSV_SPT         spt;          /* Shelf-Slot-Port-Type */
   NCS_BOOL             is_lcl_intf;  /* Is this intf is local to this node (blade) */
   NCS_IFSV_INTF_INFO   if_info;      /* Interface attribute information */
   NCS_IPXS_IPINFO      ip_info;      /* IP attribute information */
   struct ncs_ipxs_intf_rec *next;    /* Next Record */
} NCS_IPXS_INTF_REC;

/***************************************************************************
 * Possible Attributes associated with each NCSIPXS_NODE_REC 
 ***************************************************************************/

typedef uns32  NCS_NODE_AM; /* NODE attribute bitmap */

/* Node Attribute Map (NAM) values */
#define NCS_IPXS_NAM_RTR_ID    0x01  /* This box's Router ID */
#define NCS_IPXS_NAM_AS_ID     0x02  /* This box's Autonomous System ID */
#define NCS_IPXS_NAM_LBIA_ID   0x04  /* This box's Loopback Interface Address */
#define NCS_IPXS_NAM_UD1_ID    0x08  /* User Defined 1 global stuff */
#define NCS_IPXS_NAM_UD2_ID    0x10  /* User Defined 2 global stuff */
#define NCS_IPXS_NAM_UD3_ID    0x20  /* User Defined 3 global stuff */


/***************************************************************************
 * Node Record DATA Struct 
 ***************************************************************************/
typedef struct ncsipxs_node_rec
{
  NCS_NODE_ATTR_BM  ndattr;  /*NODE record attribute presence bitmap */
  uns32             rtr_id;  /* Router ID */
  uns32             lb_ia;   /* Loopback Interface Address */
  uns32             ud_1;    /* User Defined 1 */
  /* Currently these fields are not used */
  uns32             as_id;   /* Autonomous System ID */
  uns32             ud_2;    /* User Defined 2 */
  uns32             ud_3;    /* User Defined 3 */
}NCSIPXS_NODE_REC;


 /*****************************************************************************
 * Callback which the Application would registered with IFSV-IPXS
 *****************************************************************************/
struct ncs_ipxs_rsp_info;
typedef uns32 (*NCS_IPXS_SUBSCR_CB)(struct ncs_ipxs_rsp_info *);

/*****************************************************************************
 * Requests types for IP Info 
 ****************************************************************************/
typedef enum ncs_ifsv_ipxs_req_type
{
   NCS_IPXS_REQ_SUBSCR,      /* Track the  Ipinfo changes */
   NCS_IPXS_REQ_UNSUBSCR,    /* Ungerister the tracking */
   NCS_IPXS_REQ_IPINFO_GET,  /* Get the IP Info of a perticular Record */
   NCS_IPXS_REQ_IPINFO_SET,  /* Update the IP Information */
   NCS_IPXS_REQ_IS_LOCAL,    /* Is this addr is on this machine ? */
   NCS_IPXS_REQ_NODE_INFO_GET, /* To get the node information */
   NCS_IPXS_REQ_TYPE_MAX = NCS_IPXS_REQ_NODE_INFO_GET
}NCS_IFSV_IPXS_REQ_TYPE;

/***************************************************************************
 * To Register the Track for IP Info Changes 
 * Used by request Type: NCS_IPXS_REQ_SUBSCR
 ***************************************************************************/
typedef struct ncs_ipxs_subcr
{
   NCS_IPXS_SUBSCR_CB   i_ipxs_cb;  /* Call back function provided by application */
   NCS_SUBCR_BM         i_subcr_flag;  /* Subscription options */
   NCS_IFSV_BM          i_ifattr;   /* Bit map of IF attributes */
   NCS_IPXS_ATTR_BM     i_ipattr;   /* Bit map of IP attributes */
   uns32                i_usrhdl;   /* Handle provided by user  */
   uns32                o_subr_hdl; /* IPXS wants back for unsubscribe    */
}NCS_IPXS_SUBCR;


/***************************************************************************
 * To Unregister the Track for IP Info Changes
 * Used by request Type: NCS_IPXS_REQ_UNSUBSCR
 ***************************************************************************/
typedef struct ncs_ipxs_unsubcr
{
   uns32          i_subr_hdl; /* same as o_subr_hdl from NCS_IPXS_SUBCR */
}NCS_IPXS_UNSUBCR;

/******************************************************************************
 * Key used to uniquely identify the IPXS interface Record
 *****************************************************************************/
typedef struct ncs_ipxs_ifkey
{
   NCS_IPXS_KEY_TYPE       type;    /* Type of the key */
   union
   {
      NCS_IFSV_IFINDEX     iface;   /* the IF_INDEX for the iface to view */
      NCS_IFSV_SPT         spt;     /* Shelf-Slot-Port-Type */
      NCS_IP_ADDR          ip_addr; /* IP address */
   }info;
} NCS_IPXS_IFKEY;


/*****************************************************************************
 * Data Structure Used to Update IP Info to IPXS
 * Used by Request Type: NCS_IPXS_IPINFO_GET_RSP
 *****************************************************************************/
typedef struct ncs_ipxs_ipinfo_get_rsp
{
   NCS_IFSV_IFGET_RSP_ERR  err;
   NCS_IPXS_INTF_REC       *ipinfo;
} NCS_IPXS_IPINFO_GET_RSP;


/***************************************************************************
 * Given an IF_INDEX/SPT/IP address Get its associated IP Info 
 * Used by request Type: NCS_IPXS_REQ_IPINFO_GET
 ***************************************************************************/
typedef struct ncs_ipxs_ipinfo_get
{
   NCS_IPXS_IFKEY          i_key;      /* Interface Key */
   NCS_IFSV_GET_RESP_TYPE  i_rsp_type; /* Sync/Async response */
   uns32                   i_subr_hdl; /* Subscription Handle in case of async resp */
   NCS_IPXS_IPINFO_GET_RSP o_rsp;      /* Sync resp */
} NCS_IPXS_IPINFO_GET;


/*****************************************************************************
 * Data Structure Used to Update IP Info to IPXS
 * Used by Request Type: NCS_IPXS_REQ_IPINFO_SET
 *****************************************************************************/
typedef struct ncs_ipxs_ipinfo_set
{
   NCS_IFSV_IFINDEX     if_index;   /* Interface Index */
   NCS_IPXS_IPINFO      ip_info;    /* IP attribute information */
} NCS_IPXS_IPINFO_SET;


/*****************************************************************************
 * Data Structure Used to Update IP Info to IPXS
 * Used by Response Type: NCS_IPXS_REC_UPD_NTFY
 *****************************************************************************/
typedef struct ncs_ipxs_ipinfo_upd
{
   NCS_IPXS_INTF_REC   *i_ipinfo;
} NCS_IPXS_IPINFO_UPD;

/*****************************************************************************
 * Data Structure Used to Update IP Info to IPXS
 * Used by Request Type: NCS_IPXS_REC_DEL_NTFY
 *****************************************************************************/
typedef struct ncs_ipxs_ipinfo_del
{
   NCS_IPXS_INTF_REC   *i_ipinfo;
} NCS_IPXS_IPINFO_DEL;


/*****************************************************************************
 * Data Structure Used to Update IP Info to IPXS
 * Used by Request Type: NCS_IPXS_REC_ADD_NTFY
 *****************************************************************************/
typedef struct ncs_ipxs_ipinfo_add
{
   NCS_IPXS_INTF_REC   *i_ipinfo;
} NCS_IPXS_IPINFO_ADD;


/***************************************************************************
 * Given an IP Address and subnet mask, Get the IF-Index if the IP address
 * is configured.
 ***************************************************************************/
typedef struct ncs_ipxs_islocal
{
   NCS_IP_ADDR     i_ip_addr;      /* a candidate ip addr that may be local */
   uns8            i_maskbits;     /* subnet bitmask for the passed ip addr */
   NCS_BOOL        i_obs;          /* if this flag is set, then ti should be 
                                    check against full address */
   NCS_IFINDEX     o_iface;        /* interface index; use XLIM for details */
   NCS_BOOL        o_answer;       /* TRUE or FALSE                         */
}NCS_IPXS_ISLOCAL;


/***************************************************************************
 * Get Node-wide info for IPXS
 ***************************************************************************/
typedef struct ncs_ipxs_get_node_rec
{
   NCSIPXS_NODE_REC o_node_rec;
}NCS_IPXS_GET_NODE_REC;

/*****************************************************************************
 SE API: Request Data Struct Used by Applications 
 *****************************************************************************/
typedef struct ncs_ipxs_req_info
{
   NCS_IFSV_IPXS_REQ_TYPE     i_type;
   union
   {
      NCS_IPXS_SUBCR         i_subcr;
      NCS_IPXS_UNSUBCR        i_unsubr;
      NCS_IPXS_IPINFO_GET     i_ipinfo_get;
      NCS_IPXS_IPINFO_SET     i_ipinfo_set;
      NCS_IPXS_ISLOCAL        i_is_local;
      NCS_IPXS_GET_NODE_REC   i_node_rec;
   }info;
}NCS_IPXS_REQ_INFO;

/* Function used by Applications to Subscribe/Unsubscribe/Get IP Info */
IFADLL_API uns32 ncs_ipxs_svc_req(NCS_IPXS_REQ_INFO *info);
IFADLL_API uns32 ncs_ipxs_mem_free_req(NCS_IPXS_REQ_INFO *info);
/*****************************************************************************
 *  Response Types from IPXS
 *****************************************************************************/
typedef enum ncs_ifsv_ipxs_rsp_type
{
   NCS_IPXS_REC_GET_RSP,
   NCS_IPXS_REC_ADD_NTFY,
   NCS_IPXS_REC_DEL_NTFY,
   NCS_IPXS_REC_UPD_NTFY,
   NCS_IPXS_RSP_MAX = NCS_IPXS_REC_UPD_NTFY
}NCS_IPXS_RSP_TYPE;


/*****************************************************************************
 * API used by IFSV-IPXS in the Response
 *****************************************************************************/
typedef struct ncs_ipxs_rsp_info
{
   NCS_IPXS_RSP_TYPE       type;
   uns32                   usrhdl;
   union
   {
      NCS_IPXS_IPINFO_GET_RSP  get_rsp;
      NCS_IPXS_IPINFO_ADD      add;
      NCS_IPXS_IPINFO_UPD      upd;
      NCS_IPXS_IPINFO_DEL      del;
   }info;
}NCS_IPXS_RSP_INFO;

/* Memory Alloc & Free Macros used by Applications */
#define m_NCS_IPXS_INTF_REC_ALLOC ncs_ipxs_intf_rec_alloc( )
#define m_NCS_IPXS_INTF_REC_FREE(i_rec) ncs_ipxs_intf_rec_free(i_rec)

#define m_NCS_IPXS_IPPFX_LIST_ALLOC(i_pfx_cnt) ncs_ipxs_ippfx_list_alloc(i_pfx_cnt)
#define m_NCS_IPXS_IPPFX_LIST_FREE(i_pfx_list) ncs_ipxs_ippfx_list_free(i_pfx_list)

/* Memory alloc & Free APIs */
IFADLL_API NCS_IPXS_INTF_REC* ncs_ipxs_intf_rec_alloc(void );
IFADLL_API void ncs_ipxs_intf_rec_free(NCS_IPXS_INTF_REC *i_intf_rec);

IFADLL_API IPXS_IFIP_IP_INFO* ncs_ipxs_ippfx_list_alloc(uns32 i_pfx_cnt);
IFADLL_API void ncs_ipxs_ippfx_list_free(void *i_intf_rec);

#ifdef  __cplusplus
}
#endif

#endif /* IPXS_PAPI_H */
