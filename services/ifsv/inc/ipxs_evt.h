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
FILE NAME: IPXS_EVT.H
DESCRIPTION: Enums, Structures and Function prototypes for IPXS
******************************************************************************/

#ifndef IPXS_EVT_H
#define IPXS_EVT_H


/* IPXS Event Type */
typedef enum ipxs_evt_type
{
   IPXS_EVT_DTOND_IFIP_INFO = 1,
   IPXS_EVT_OSTOND_IFIP_INFO,
   IPXS_EVT_ATOND_IFIP_GET,
   IPXS_EVT_ATOND_IFIP_UPD,
   IPXS_EVT_NDTOA_IFIP_UPD_RESP,
   IPXS_EVT_ATOND_IS_LOCAL,
   IPXS_EVT_ATOND_NODE_REC_GET,
   IPXS_EVT_NDTOD_IFIP_INFO,
   IPXS_EVT_NDTOA_IFIP_RSP,
   IPXS_EVT_NDTOA_IFIP_ADD,
   IPXS_EVT_NDTOA_IFIP_DEL,
   IPXS_EVT_NDTOA_IFIP_UPD,
   IPXS_EVT_NDTOA_NODE_REC,
   IPXS_EVT_NDTOA_ISLOC_RSP,
   IPXS_EVT_MAX = IPXS_EVT_NDTOA_ISLOC_RSP
}IPXS_EVT_TYPE;

/* Struct for requesting the IP REC */
typedef struct ipxs_evt_if_rec_get
{
   IFSV_INTF_GET_TYPE      get_type;  /* Get All/One */
   NCS_IPXS_IFKEY          key;       /* Interface Key */
   NCSMDS_SVC_ID           my_svc_id; /* Svc ID & Vcard ID are used to send*/
   MDS_DEST                my_dest;   /* the MDS Message back to IFA */ 
   uns32                   usr_hdl;   /* Subscription Handle in case of async resp */
} IPXS_EVT_IF_REC_GET;

/* Structure to find the given IP address is local addr or not???*/
typedef struct ipxs_evt_islocal
{
   NCS_IP_ADDR     ip_addr;      /* a candidate ip addr that may be local */
   uns8            maskbits;     /* subnet bitmask for the passed ip addr */
   NCS_BOOL        obs;          /* if this flag is set, then ti should be 
                                   check against full address */
}IPXS_EVT_ISLOCAL;

/* Structure used by IFD-IPXS to update IFND-IPXS abut the interface rec*/
typedef struct ipxs_evt_if_info
{
   NCS_IFSV_IFINDEX  if_index;   /* IF index */
   NCS_IPXS_IPINFO   ip_info;    /* IP attribute information */ 
}IPXS_EVT_IF_INFO;


/* IPXS Event information */
typedef struct ipxs_evt
{
   IPXS_EVT_TYPE           type;
   NCS_IFSV_SYNC_SEND_ERROR error;
   uns32                   usrhdl;
   NCS_BOOL                netlink_msg;
   union
   {
      union
      {
         IPXS_EVT_IF_REC_GET     get;       /* IPXS_EVT_ATOND_IFIP_REQ */
         NCS_IPXS_INTF_REC       atond_upd; /* IPXS_EVT_ATOND_IFIP_UPD */
         IPXS_EVT_ISLOCAL        is_loc;    /* IPXS_EVT_ATOND_IS_LOCAL */
         IPXS_EVT_IF_INFO        dtond_upd;  /* IPXS_EVT_DTOND_IFIP_INFO */
      }nd;
      union
      {
         IPXS_EVT_IF_INFO        ndtod_upd;  /* IPXS_EVT_NDTOD_IFIP_INFO */
      }d;
      union
      {
         NCS_IPXS_IPINFO_GET_RSP  get_rsp;   /* IPXS_EVT_NDTOA_IFIP_RSP */
         NCS_IPXS_IPINFO_ADD      ip_add;    /* IPXS_EVT_NDTOA_IFIP_ADD */
         NCS_IPXS_IPINFO_DEL      ip_del;    /* IPXS_EVT_NDTOA_IFIP_DEL */
         NCS_IPXS_IPINFO_UPD      ip_upd;    /* IPXS_EVT_NDTOA_IFIP_UPD */
         NCSIPXS_NODE_REC         node_rec;  /* IPXS_EVT_NDTOA_NODE_REC */
         NCS_IPXS_ISLOCAL         isloc_rsp; /* IPXS_EVT_NDTOA_ISLOC_RSP */
      }agent;
   }info;
}IPXS_EVT;

#endif /* IPXS_EVT_H */
