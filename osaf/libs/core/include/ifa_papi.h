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

#ifndef IFA_API_H
#define IFA_API_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ifsv_papi.h"
#include "ncs_iplib.h"
/*****************************************************************************
FILE NAME: IFA_API.H
DESCRIPTION: Enums, Structures and Function prototypes used by Interface Agent
             Applications.
******************************************************************************/

/* Forward Decleration to fix warning */
struct ncs_ifsv_svc_rsp;

/* Applications will subscribe to IFSV for the following Event Types.
   subscriber wants to see (in any combo) */

#define NCS_IFSV_ADD_IFACE  0x01   /* just added interfaces                  */
#define NCS_IFSV_RMV_IFACE  0x02   /* just removed interfaces                */
#define NCS_IFSV_UPD_IFACE  0x04   /* just updated or adjusted interfaces    */

#if (NCS_VIP == 1)
#define m_NCS_IFSV_VIP_APPL_NAME          100
#define m_NCS_IFSV_VIP_INTF_NAME          25

typedef enum ncs_ifsv_vip_types
{
      NCS_IFSV_VIP_IP_INTERNAL,
      NCS_IFSV_VIP_IP_EXTERNAL,
      NCS_IFSV_VIP_IP_DEFAULT
}NCS_IFSV_VIP_IPTYPES;

/* Here the error codes have been started from 4 
   to avoid conflicts with success and failure */
typedef enum ncs_ifsv_vip_errors
{
    NCSCC_RC_INTF_ADMIN_DOWN = 4,
    NCSCC_RC_INVALID_INTF,
    NCSCC_RC_INVALID_IP,
    NCSCC_RC_IP_ALREADY_EXISTS,
    NCSCC_RC_INVALID_PARAM,
    NCSCC_RC_INVALID_VIP_HANDLE,
    NCSCC_RC_NULL_APPLNAME,
    NCSCC_RC_VIP_RETRY,
    NCSCC_RC_VIP_INTERNAL_ERROR,
    NCSCC_VIP_EXISTS_ON_OTHER_INTERFACE,
    NCSCC_RC_IP_EXISTS_BUT_NOT_VIP, 
    NCSCC_VIP_HDL_IN_USE_FOR_DIFF_IP,
    NCSCC_RC_APPLNAME_LEN_EXCEEDS_LIMIT,
    NCSCC_VIP_EXISTS_FOR_OTHER_HANDLE,
    NCSCC_RC_APPLICAION_NOT_OWNER_OF_IP,
    NCSCC_RC_INVALID_FLAG,
    NCSCC_RC_IP_ADDR_TYPE_NOT_SUPPORTED,
    NCSCC_RC_INVALID_MASKLEN,
    NCSCC_VIP_APPL_NAME_ALREADY_IN_USE_FOR_SAME_IP 

}NCS_IFSV_VIP_ERRORS;


/* The following flags will be used for differeniating
   the behavior for freeing the application IP */
typedef enum ncs_ifsv_vip_flags
{
    VIP_ADDR_NORMAL = 10,
    VIP_ADDR_SPECIFIC,
    VIP_ADDR_SWITCH
}NCS_IFSV_VIP_FLAGS;


typedef struct ncs_ifsv_vip_handle
{
   uns8                     vipApplName[m_NCS_IFSV_VIP_APPL_NAME];
   uns32                    poolHdl;
}NCS_IFSV_VIP_HANDLE;

#endif

/* Enum to specify key type to get interface record */
typedef enum ncs_ifsv_key_type
{
   NCS_IFSV_KEY_IFINDEX = 1,       /* Key is IF-INDEX */
   NCS_IFSV_KEY_SPT,               /* Key is slot-port-type-scope */
   NCS_IFSV_KEY_MAX = NCS_IFSV_KEY_SPT
} NCS_IFSV_KEY_TYPE;

/* Enum to specify the response type */
typedef enum ncs_ifsv_resp_type
{
   NCS_IFSV_GET_RESP_SYNC,
   NCS_IFSV_GET_RESP_ASYNC,
   NCS_IFSV_GET_RESP_MAX = NCS_IFSV_GET_RESP_ASYNC
} NCS_IFSV_GET_RESP_TYPE;


/* Enum to specify Servie request Type */
typedef enum ncs_ifsv_svc_req_type
{
   NCS_IFSV_SVC_REQ_SUBSCR = 1,     /* Request for Subscription */
   NCS_IFSV_SVC_REQ_UNSUBSCR,       /* Request for Unsubscription */
   NCS_IFSV_SVC_REQ_IFREC_GET,      /* Request for Interface Record */
   NCS_IFSV_SVC_REQ_IFREC_ADD,      /* Add/update the logical interface */
   NCS_IFSV_SVC_REQ_IFREC_DEL,      /* Delete the logical interface */
   NCS_IFSV_REQ_SVC_DEST_UPD,       /* Update the SVC-VDEST */
   NCS_IFSV_REQ_SVC_DEST_GET,       /* Update the SVC-VDEST */
#if (NCS_VIP == 1)
   NCS_IFSV_REQ_VIP_INSTALL,
   NCS_IFSV_REQ_VIP_FREE,
   NCS_IFSV_SVC_REQ_MAX = NCS_IFSV_REQ_VIP_FREE
#else
   NCS_IFSV_SVC_REQ_MAX = NCS_IFSV_REQ_SVC_DEST_GET
#endif
} NCS_IFSV_SVC_REQ_TYPE;

/* Enum to specify Servie request Type */
typedef enum ncs_ifsv_rsp_type
{
   NCS_IFSV_IFREC_GET_RSP = 1,
   NCS_IFSV_IFREC_ADD_NTFY,
   NCS_IFSV_IFREC_DEL_NTFY,
   NCS_IFSV_IFREC_UPD_NTFY,
   NCS_IFSV_RSP_MAX = NCS_IFSV_IFREC_UPD_NTFY
}NCS_IFSV_RSP_TYPE;

/*****************************************************************************
 * Callback registered which the client would registered with IFA
 *****************************************************************************/
typedef uns32 (*NCS_IFSV_SUBSCR_CB)(struct ncs_ifsv_svc_rsp *);


/*****************************************************************************
 * Data Structure Used to subscribe for IFSV Events 
 *****************************************************************************/
typedef struct ncs_ifsv_subscr
{
  NCS_IFSV_SUBSCR_CB    i_ifsv_cb;    /* callback func when satisfied           */
  NCS_IFSV_SUBSCR_SCOPE subscr_scope; /*To recognise the scope of the interface
                                      record: internal or external. This will be
                                      useful for If Agent in deciding whether 
                                      the record coming from IfNd is for 
                                      Internal Interfaces or External Interfaces
                                      . */

  /* TBD : Need to put more MAPs here */
  NCS_IFSV_BM           i_sevts;      /* events for which the client subscribes */
  NCS_IFSV_BM           i_smap;       /* if attributes not set, don't callback! */
  uns64                 i_usrhdl;     /* Handle provided by user               */
  uns32                 o_subr_hdl; /* XLIM wants back for unsubscribe        */
} NCS_IFSV_SUBSCR;

/***************************************************************************
 * Unsubscribe to IFSV events, as per i_subscr_hdl passed back
 ***************************************************************************/
typedef struct ncs_ifsv_unsubscr
{
   uns32    i_subr_hdl; /* same as o_subr_hdl from NCS_IFSV_SUBSCR */
} NCS_IFSV_UNSUBSCR;


/******************************************************************************
 * Key used to uniquely identify the interface
 *****************************************************************************/
typedef struct ncs_ifsv_ifkey
{
   NCS_IFSV_KEY_TYPE       type;    /* Type of the key */
   union
   {
      NCS_IFSV_IFINDEX     iface;   /* the IF_INDEX for the iface to view */
      NCS_IFSV_SPT         spt;     /* Slot-Port-Type-scope */
   }info;
} NCS_IFSV_IFKEY;


/***************************************************************************
 * Given an IF_INDEX or Slot-Port-Type-scope Get its associated interface record
 ***************************************************************************/
typedef struct ncs_ifsv_ifrec_get
{
   NCS_IFSV_IFKEY          i_key;      /* Interface Key */
   NCS_IFSV_INFO_TYPE      i_info_type; /* Info Type */
   NCS_IFSV_GET_RESP_TYPE  i_rsp_type; /* Sync/Async response */
   uns32                   i_subr_hdl; /* Subscription Handle */
} NCS_IFSV_IFREC_GET;


/***************************************************************************
 * Structure used in response
***************************************************************************/
typedef enum ncs_ifsv_ifget_rsp_err
{
   NCS_IFSV_IFGET_RSP_SUCCESS = 1,
   NCS_IFSV_IFGET_RSP_NO_REC,
   NCS_IFSV_IFGET_RSP_ERR_MAX = NCS_IFSV_IFGET_RSP_NO_REC
}NCS_IFSV_IFGET_RSP_ERR;

typedef struct ncs_ifsv_ifget_rsp
{
   NCS_IFSV_IFGET_RSP_ERR  error;
   NCS_IFSV_INTF_REC       if_rec;
}NCS_IFSV_IFGET_RSP;

typedef enum ncs_ifsv_svcd_evt_type
{
   NCS_IFSV_SVCD_ADD=1,
   NCS_IFSV_SVCD_DEL,
   NCS_IFSV_SVCD_EVT_MAX
}NCS_IFSV_SVCD_EVT_TYPE;

/***************************************************************************
 * Update the IF_INDEX & SVDEST Mapping 
 * Used by request Type: NCS_IFSV_REQ_SVC_DEST_UPD
 ***************************************************************************/
typedef struct ncs_ifsv_svc_dest_upd
{
   NCS_IFSV_SVCD_EVT_TYPE  i_type;    /* ADD/DEL */
   NCS_IFSV_IFINDEX        i_ifindex; /* Interface index, If set to 0 (zero)
                                         delete from all interfaces */
   NCS_SVDEST              i_svdest;  /* Sync/Async response */
} NCS_IFSV_SVC_DEST_UPD;


/***************************************************************************
 * Update the IF_INDEX & SVDEST Mapping 
 * Used by request Type: NCS_IFSV_REQ_SVC_DEST_UPD
 ***************************************************************************/
typedef struct ncs_ifsv_svc_dest_get
{
   NCS_IFSV_IFINDEX  i_ifindex; /* Interface Key */
   NCSMDS_SVC_ID     i_svcid;    /* the SVC_ID for the iface  */
   MDS_DEST          o_dest;     /* the MDS V-DEST of Service */
   NCS_BOOL          o_answer;   /* TRUE - Found, FALSE - Not found */
} NCS_IFSV_SVC_DEST_GET;


#if (NCS_VIP == 1)
/*****************************************************************************
* Install a given Virtual IP Address onto the given Interface
* Used by request type: NCS_IFSV_VIP_INSTALL
*****************************************************************************/
typedef struct ncs_ifsv_vip_install
{
   NCS_IFSV_VIP_HANDLE     i_handle;                               /* used for updating the databases IfADB, VIPDC, IPXS*/
   uns8                    i_intf_name[m_NCS_IFSV_VIP_INTF_NAME];  /* Interface onto which Virtual IP is to be installed */
   NCS_IPPFX               i_ip_addr;                                /* IP Address to be installed */
   uns32                   o_err;                                /* contains the error specific code */
   uns32                   i_infraFlag;
   
}NCS_IFSV_VIP_INSTALL;


/****************************************************************************
* Uninstall the virtual IP address for the given handle
* Used by request type : NCS_IFSV_VIP_FREE
*****************************************************************************/
typedef struct ncs_ifsv_vip_free
{
   NCS_IFSV_VIP_HANDLE     i_handle;
   uns32                   o_err;
   uns32                   i_infraFlag;
   
}NCS_IFSV_VIP_FREE;

#endif


/*****************************************************************************
 * API used by Applications to Subscribe/Unsubscribe/Get interface Records
 *****************************************************************************/
typedef struct ncs_ifsv_svc_req
{
   NCS_IFSV_SVC_REQ_TYPE   i_req_type;
   union
   {
      NCS_IFSV_SUBSCR         i_subr;
      NCS_IFSV_UNSUBSCR       i_unsubr;
      NCS_IFSV_IFREC_GET      i_ifget;
      NCS_IFSV_INTF_REC       i_ifadd;
      NCS_IFSV_SPT            i_ifdel;
      NCS_IFSV_SVC_DEST_UPD   i_svd_upd;
      NCS_IFSV_SVC_DEST_GET   i_svd_get;
#if (NCS_VIP == 1)
     NCS_IFSV_VIP_INSTALL     i_vip_install;
     NCS_IFSV_VIP_FREE        i_vip_free;
#endif
   }info;
   NCS_IFSV_IFGET_RSP      o_ifget_rsp; /* the interface for sync resp */
} NCS_IFSV_SVC_REQ;


/*****************************************************************************
 * API used by IFSV in the Aync response
 *****************************************************************************/

typedef struct ncs_ifsv_svc_rsp
{
   NCS_IFSV_RSP_TYPE       rsp_type;
   uns64                   usrhdl;
   union
   {
      NCS_IFSV_IFGET_RSP   ifget_rsp;     /* Interface Get Response */
      NCS_IFSV_INTF_REC    ifadd_ntfy;    /* Notify the Interface Additions */
      NCS_IFSV_IFINDEX     ifdel_ntfy;    /* Notify the Interface Deletions */
      NCS_IFSV_INTF_REC    ifupd_ntfy;    /* Notify the Interface Updates */
   }info;
} NCS_IFSV_SVC_RSP;


/* Memory Alloc & Free Macros used by Applications */
#define m_NCS_IFSV_SVDEST_LIST_ALLOC(i_svd_cnt) ncs_ifsv_svdest_list_alloc(i_svd_cnt)
#define m_NCS_IFSV_SVDEST_LIST_FREE(i_svd_list) ncs_ifsv_svdest_list_free(i_svd_list)

/*****************************************************************************
 * Function prototypes 
 *****************************************************************************/

/* Function used by Applications to Subscribe/Unsubscribe/Get interface Records */
IFADLL_API uns32 ncs_ifsv_svc_req(NCS_IFSV_SVC_REQ *info);

IFADLL_API NCS_SVDEST* ncs_ifsv_svdest_list_alloc(uns32 i_svd_cnt);
IFADLL_API void ncs_ifsv_svdest_list_free(NCS_SVDEST *i_svd_list);

#ifdef  __cplusplus
}
#endif

#endif 

