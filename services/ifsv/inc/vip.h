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

/* The following defines are used for forming an IPXS event */

#if (NCS_VIP == 1)
#ifndef VIP_H 
#define VIP_H

#include "ifa_papi.h"
#include "ncsdlib.h"


#define IFSV_VIP_IPXS_ADD         1
#define IFSV_VIP_IPXS_DEL         2
#define IFSV_VIP_IPXS_DEC_REFCNT  3
#define IFSV_VIP_IPXS_INC_REFCNT  4
#define NCSCC_IFND_VIP_INSTALL            12 /* This is used by ifnd to inform 
                                              * IfA to install the ip on requested intf
                                              * used as a resp to IFA_VIP_INFO_ADD_REQ */ 
#define MAX_IFND_NODES            16

/******************************************************************************
* Following are the IP Address Virtualization event types.
NOTE : All the events specified in this enum are not supported in this phase (Phase-I). These events are to be added to the IFSV_EVT_TYPES
******************************************************************************/

/*****************************************************************************
*  Following structure is used as unique key to access VIPD/VIPDC databases
******************************************************************************/
typedef struct ncs_ifsv_vip_int_hdl
{
   uns8                     vipApplName[m_NCS_IFSV_VIP_APPL_NAME];
   uns32                    poolHdl;
   uns32                    ipPoolType;
}NCS_IFSV_VIP_INT_HDL;

/*******************************************************************************
* The below message could be used for the following type of Event
* IFA_VIPD_INFO_ADD
*******************************************************************************/
typedef struct ifa_vipd_info_add
{
    NCS_IFSV_VIP_INT_HDL   handle;
    uns8                   intfName[m_NCS_IFSV_VIP_INTF_NAME];      /* Interface onto which Virtual IP is to be installed */
    NCS_IPPFX              ipAddr;        /* IP Address to be installed */
#if (VIP_HALS_SUPPORT == 1)
    uns32               infraFlag;
#endif

}IFA_VIPD_INFO_ADD;

typedef struct ifnd_vip_info_add_resp
{
   uns32  err;
}IFND_VIPD_INFO_ADD_RESP;

typedef struct ifnd_vip_install
{
   uns32  err;
}IFND_VIP_INSTALL;



/*******************************************************************************
* The below message could be used for the following type of Event
* IFND_VIPD_INFO_ADD
*******************************************************************************/

typedef struct ifnd_vipd_info_add
{
    NCS_IFSV_VIP_INT_HDL   handle;
    uns8                   intfName[m_NCS_IFSV_VIP_INTF_NAME];      /* Interface onto which Virtual IP is to be installed */
    NCS_IPPFX              ipAddr;        /* IP Address to be installed */
#if (VIP_HALS_SUPPORT == 1)
    uns32               infraFlag;
#endif

}IFND_VIPD_INFO_ADD;

typedef struct ifd_vipd_info_add_resp
{
   uns32  err;
}IFD_VIPD_INFO_ADD_RESP;
/******************************************************************************
* The following message would be used for the following type of event
* IFA_VIP_FREE
*******************************************************************************/

typedef struct ifa_vip_free
{
   NCS_IFSV_VIP_INT_HDL      handle;
#if (VIP_HALS_SUPPORT == 1)
   uns32                 infraFlag;
#endif
}IFA_VIP_FREE;


typedef struct ifnd_vip_free_resp
{
   uns32  err;
}IFND_VIP_FREE_RESP;

/******************************************************************************
* The following message would be used for the following type of event
* IFND_VIP_FREE
*******************************************************************************/

typedef struct ifnd_vip_free
{
   NCS_IFSV_VIP_INT_HDL      handle;
#if (VIP_HALS_SUPPORT == 1)
   uns32                  infraFlag;
#endif
}IFND_VIP_FREE;

typedef struct ifd_vip_free_resp
{
   uns32  err;
}IFD_VIP_FREE_RESP;
/******************************************************************************
* The following message would be used for all the error cases 
* IFSV_VIP_ERROR
*******************************************************************************/
typedef struct ifsv_vip_error_evt
{
   uns32       err;
}IFSV_VIP_ERROR_EVT;

/*******************************************************************************
* following evt type is used for IfA crash scenario
*
******************************************************************************/
typedef struct ifnd_vip_del_vipd
{
   NCS_IFSV_VIP_INT_HDL  handle;
}IFND_VIP_DEL_VIPD;


/*****************************************************************************
* These are newly added structures for vip enhancements.
******************************************************************************/

/******************************************************************************
* Following structure is used by the following events
* IFND_VIP_MARK_VIPD_STALE -- for marking an entry as stale 
* VIP_GET_IP_FROM_STALE_ENTRY -- for getting an ip addr from stale entry
******************************************************************************/

typedef struct vip_common_event
{
    NCS_IFSV_VIP_INT_HDL    handle;
}VIP_COMMON_EVENT;

/*****************************************************************************
* Following Event is used for getting an IP from stale entry
*
*****************************************************************************/
typedef struct ip_val_from_stale_entry
{
    NCS_IPPFX    ipAddr;
}VIP_IP_FROM_STALE_ENTRY;


/*******************************************************************************
* Following structure defines the events used for IP Address virtualization
* This event type is to be added in the structure of IFSV_EVT (ifsv_evt.h file)
*******************************************************************************/
typedef struct vip_evt
{
    union
    {
        IFA_VIPD_INFO_ADD         ifaVipAdd;
        IFND_VIPD_INFO_ADD        ifndVipAdd;
        IFA_VIP_FREE              ifaVipFree;
        IFND_VIP_FREE             ifndVipFree;
        IFND_VIPD_INFO_ADD_RESP    ifndVipAddResp;
        IFD_VIPD_INFO_ADD_RESP     ifdVipAddResp;
        IFND_VIP_FREE_RESP        ifndVipFreeResp;
        IFD_VIP_FREE_RESP         ifdVipFreeResp;
        IFSV_VIP_ERROR_EVT            errEvt;
        IFND_VIP_DEL_VIPD         ifndVipDel;
        VIP_COMMON_EVENT          vipCommonEvt;
        VIP_IP_FROM_STALE_ENTRY   staleIp;
    }info;
}VIP_EVT;

/**************************************************************
* This structure will be included in the IFSV_CB.
* Used to hold ifnd addresses when failover happens.
**************************************************************/

typedef struct down_ifnd_addr
{
   NCS_BOOL     valid;
   MDS_DEST     ifndAddr;
}DOWN_IFND_ADDR;

/*
 *Common Function Prototypes
 */
uns32  m_ncs_uninstall_vip(NCS_IPPFX *ipAddr,uns8 * intf);
uns32  m_ncs_install_vip_and_send_garp(NCS_IPPFX *ipAddr,uns8 * intf);  

#define  m_NCS_UNINSTALL_VIP( ptr,str)  m_ncs_uninstall_vip(ptr,str)
#define  m_NCS_INSTALL_VIP_AND_SEND_GARP( ptr,str)  m_ncs_install_vip_and_send_garp(ptr,str)

NCS_PATRICIA_NODE * ifsv_vipd_vipdc_rec_get(NCS_PATRICIA_TREE *pTree,NCS_IFSV_VIP_INT_HDL *pHandle);
                                                                                                                              
uns32    ifsv_vipd_vipdc_rec_del(NCS_PATRICIA_TREE *pPatTree,NCS_PATRICIA_NODE *pNode,uns32 recordType);
                                                                                                                              
uns32    ifsv_vipd_vipdc_rec_add(NCS_PATRICIA_TREE *pPatTree,NCS_PATRICIA_NODE *pNode);



                                                                                                                              


/*
 * Functions Commomnly Being Used by IFD as well as IFND
 */
uns32 ifsv_vip_add_owner_node (NCS_DB_LINK_LIST *list, MDS_DEST *dest
#if (VIP_HALS_SUPPORT == 1)
                              , uns32       infraFlag
#endif 
 );
uns32 ifsv_vip_init_ip_list(NCS_DB_LINK_LIST *ipList);
uns32 ifsv_vip_init_intf_list(NCS_DB_LINK_LIST *intfList);
uns32 ifsv_vip_init_owner_list(NCS_DB_LINK_LIST *ownerList);
uns32 ifsv_vip_init_allocated_ip_list(NCS_DB_LINK_LIST *allocIpList);
uns32 ifsv_vip_add_ip_node (NCS_DB_LINK_LIST *list, NCS_IPPFX *ipAddr,uns8 *str);
uns32 ifsv_vip_add_intf_node (NCS_DB_LINK_LIST *list,uns8 *str);

                                                                                                                              


/*********************************************************************************
* Prototypes for the functions used for VIP functionality
*********************************************************************************/


/* Prototypes for IFA interface -install */
/*uns32  ncs_ifsv_vip_install( IFA_CB *ifa_cb, NCS_IFSV_VIP_INSTALL  *instArg);*/

/* Prototypes for IFA interface -free -*/
/*uns32  ncs_ifsv_vip_free(IFA_CB *ifa_cb,NCS_IFSV_VIP_FREE  *handle);*/

/*extern uns32 ifnd_ipxs_proc_ifip_upd(IPXS_CB *cb, IPXS_EVT *ipxs_evt,
                                     IFSV_SEND_INFO *sinfo); */

/*uns32 ifnd_vip_evt_process(IFSV_CB *cb,IFSV_EVT *evt);*/




#endif /* End of VIP_H */ 
#endif
