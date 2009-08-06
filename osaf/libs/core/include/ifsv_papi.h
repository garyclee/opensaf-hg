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

#ifndef IFSV_PAPI_H
#define IFSV_PAPI_H
#include <stdarg.h>
#include "ncsgl_defs.h"
#include "mds_papi.h"
#include "os_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncs_main_papi.h"
#include "ncssysf_tsk.h"
#include "mds_papi.h"

/* Max size of the description */
#define IFSV_IF_DESC_SIZE (200)

/* Max size of interface name */
#define IFSV_IF_NAME_SIZE (56)

/* Info Type Bit Maps */
typedef enum ncs_ifsv_info_type
{
   NCS_IFSV_IF_INFO = 1,
   NCS_IFSV_BIND_GET_LOCAL_INTF,
   NCS_IFSV_IFSTATS_INFO,
   NCS_IFSV_INFO_TYPE_MAX = NCS_IFSV_IFSTATS_INFO
}NCS_IFSV_INFO_TYPE;

#define IFSV_BINDING_SHELF_ID 1
#define IFSV_BINDING_SLOT_ID 0xAB
/* embedding subslot changes */
#define IFSV_BINDING_SUBSLOT_ID 0XAB
/* Definations for Interface attribute Maps */
#define NCS_IFSV_IAM_MTU            (0x00000001)
#define NCS_IFSV_IAM_IFSPEED        (0x00000002)
#define NCS_IFSV_IAM_PHYADDR        (0x00000004)
#define NCS_IFSV_IAM_ADMSTATE       (0x00000008)
#define NCS_IFSV_IAM_OPRSTATE       (0x00000010)
#define NCS_IFSV_IAM_NAME           (0x00000020)
#define NCS_IFSV_IAM_DESCR          (0x00000040)
#define NCS_IFSV_IAM_LAST_CHNG      (0x00000080)
#define NCS_IFSV_IAM_SVDEST        (0x00000100)
#define NCS_IFSV_IAM_CHNG_MASTER (0x00000200)
#define NCS_IFSV_BINDING_CHNG_IFINDEX (0x00000400)

/* Interface index defination */
typedef unsigned int NCS_IFSV_IFINDEX;

/* Bit map defination */
typedef unsigned int NCS_IFSV_BM;

/* Macro to check whether the MDS dest is same */
#define m_NCS_IFSV_IS_MDS_DEST_SAME(dest1,dest2) ((memcmp(dest1,dest2, sizeof(MDS_DEST)) == 0)?TRUE:FALSE)

/*****************************************************************************
 * This is the message type which hardware drv would receive or send to IFND.
 *****************************************************************************/
typedef enum ncs_ifsv_hw_drv_msg_type
{
  NCS_IFSV_HW_DRV_STATS,
  NCS_IFSV_HW_DRV_PORT_REG,
  NCS_IFSV_HW_DRV_PORT_STATUS,
  NCS_IFSV_HW_DRV_SET_PARAM,
  NCS_IFSV_HW_DRV_IFND_UP,
  NCS_IFSV_HW_DRV_PORT_SYNC_UP,
  NCS_IFSV_HW_DRV_MAX_MSG
} NCS_IFSV_HW_DRV_MSG_TYPE;

/*****************************************************************************
 * IANA Interface types.
 *****************************************************************************/
typedef enum NCS_IFSV_INTF_TYPE
{
   NCS_IFSV_INTF_OTHER       =  1,
   NCS_IFSV_INTF_LOOPBACK    = 24,
   NCS_IFSV_INTF_ETHERNET    = 26,
   NCS_IFSV_INTF_TUNNEL      = 131,
   NCS_IFSV_INTF_BINDING     = 240
} NCS_IFSV_INTF_TYPE;

/*****************************************************************************
 * This is the request type which the actual driver is going to call.
 *****************************************************************************/
typedef enum ncs_ifsv_drv_req_type
{
   NCS_IFSV_DRV_INIT_REQ =1,
   NCS_IFSV_DRV_DESTROY_REQ,
   NCS_IFSV_DRV_PORT_REG_REQ,
   NCS_IFSV_DRV_SEND_REQ
}NCS_IFSV_DRV_REQ_TYPE;

/*****************************************************************************
 * This is the function prototype which would be registered by the hardware drv
 * and whenever IDIM needs any hardware info, it is going to call this function.
 *****************************************************************************/
/* forward decleration */
struct ncs_ifsv_hw_drv_req;
typedef uns32 (*NCS_IFSV_HW_DRV_GET_SET_CB)(struct ncs_ifsv_hw_drv_req *drv_req);

/*****************************************************************************
 * Slot-Port-Type-Scope Data structure defination
 *****************************************************************************/
/* The enum for recognising whether the record being created is for internal or 
 * external interfaces. */
typedef enum ncs_ifsv_subscr_scope
{
   NCS_IFSV_SUBSCR_EXT = 0,
   NCS_IFSV_SUBSCR_INT,
   NCS_IFSV_SUBSCR_ALL,
}NCS_IFSV_SUBSCR_SCOPE;

typedef struct ncs_ifsv_spt
{
   uns32                shelf;
   uns32                slot;
   /* embedding subslot changes */
   uns32                subslot;
   uns32                port;
   NCS_IFSV_INTF_TYPE   type;
   NCS_IFSV_SUBSCR_SCOPE    subscr_scope;
}NCS_IFSV_SPT;

/*****************************************************************************
 * This is the structure with port id and interface type.
 *****************************************************************************/
typedef struct ncs_ifsv_port_type
{
   uns32              port_id;
   NCS_IFSV_INTF_TYPE type;   
} NCS_IFSV_PORT_TYPE;

/*****************************************************************************
 * Data Structure Used to hold the shelf/slot/subslot/port/type/subscr_scope Vs Ifindex 
 * maping
 *****************************************************************************/

typedef struct ncs_ifsv_spt_map
{
   NCS_IFSV_SPT         spt;
   NCS_IFSV_IFINDEX     if_index;
} NCS_IFSV_SPT_MAP;

/***************************************************************************
 * MDS Service ID and VDEST pair of services
 ***************************************************************************/
typedef struct ncs_svdest
{
   NCSMDS_SVC_ID         svcid;    /* the SVC_ID for the iface  */
   MDS_DEST              dest;     /* the MDS V-DEST of Service */
}NCS_SVDEST;

#define NCS_IFSV_SPT_MAP_NULL ((NCS_IFSV_SPT_MAP*)0)
#define IFSV_INTF_DATA_NULL ((IFSV_INTF_DATA*)0)

/*****************************************************************************
 * Data structure which is used to hold the statistic information defined in
 * RFC 2863.
 *****************************************************************************/
typedef struct ncs_ifsv_intf_stats
{
   uns32       last_chg;
   uns32       in_octs;
   uns32       in_upkts;
   uns32       in_nupkts;
   uns32       in_dscrds;
   uns32       in_errs;
   uns32       in_unknown_prots;
   uns32       out_octs;
   uns32       out_upkts;
   uns32       out_nupkts;
   uns32       out_dscrds;
   uns32       out_errs;
   uns32       out_qlen;
   uns32       if_specific;
} NCS_IFSV_INTF_STATS;

typedef struct ncs_ifsv_nodeid_ifname
 {
    NODE_ID              node_id;
    uns8                 if_name[20];
 }NCS_IFSV_NODEID_IFNAME;


/*****************************************************************************
 * Data structure which is used to hold the common interface information which
 * all the interface should have. (Defined in RFC 2863)
 *****************************************************************************/
typedef struct ncs_ifsv_intf_info
{
   NCS_IFSV_BM          if_am;   /* Attribute Map Flags */
   uns8                 if_descr[IFSV_IF_DESC_SIZE];
   uns8                 if_name[IFSV_IF_NAME_SIZE];
   uns32                mtu;
   uns32                if_speed;
   uns8                 phy_addr[6];
   uns32                admin_state; /* Need to define ENUM */
   NCS_STATUS           oper_state;
   time_t               last_change;
   NCS_SVDEST           *addsvd_list;   /* List of ADD SVDEST */
   uns8                 addsvd_cnt;     /* IP addresses count in the add SVDEST*/
   NCS_SVDEST           *delsvd_list;   /* List of DEL SVDEST */
   uns8                 delsvd_cnt;     /* IP addresses count in the del SVDEST*/

   NCS_IFSV_IFINDEX bind_master_ifindex;
   NCS_IFSV_IFINDEX bind_slave_ifindex;
   uns8             bind_num_slaves;
   NCS_IFSV_NODEID_IFNAME bind_master_info;
   NCS_IFSV_NODEID_IFNAME bind_slave_info;
}NCS_IFSV_INTF_INFO;


/*****************************************************************************
 * Interface Record data structure used for External Communication
 * This is used in application's interface, management access interface,
 * and driver interface
 *****************************************************************************/
typedef struct ncs_ifsv_intf_rec
{
   NCS_IFSV_IFINDEX     if_index;   /* Interface Index of this Record */
   NCS_IFSV_SPT         spt_info;   /* Slot-port-type-scope information */
   NCS_IFSV_INFO_TYPE   info_type;  /* Intf info/Stats Info/All */
   NCS_IFSV_INTF_INFO   if_info;    /* Interface Information */
   NCS_IFSV_INTF_STATS  if_stats;   /* Interface Statistics */
}NCS_IFSV_INTF_REC;

#define NCS_IFSV_INTF_REC_NULL ((NCS_IFSV_INTF_REC*)0)


/*****************************************************************************
 * Structure used by the Hardware drv to indicate IDIM about its statistics.
 *****************************************************************************/
typedef struct ncs_ifsv_port_stats
{
   NCS_BOOL                   status; /* TRUE - successfully fetched, FALSE - failed to fetch the stats */
   NCSMDS_SVC_ID              app_svc_id; /* This is filled from the event which Driver would have got request */
   MDS_DEST                   app_dest;   /* This is filled from the event which Driver would have got request */
   uns32                      usr_hdl;    /* This is filled from the event which Driver would have got request */
   NCS_IFSV_INTF_STATS        stats;
}NCS_IFSV_PORT_STATS;

/*****************************************************************************
 * Structure used by the Hardware drv to indicate Port status.
 *****************************************************************************/
typedef struct ncs_ifsv_idim_port_status
{
   NCS_STATUS    oper_state;
}NCS_IFSV_PORT_STATUS;

/*****************************************************************************
 * Hardware information which Hardware drv informs to IDIM while registeration.
 *****************************************************************************/
typedef struct ncs_ifsv_port_info
{
   NCS_IFSV_BM              if_am;   /* Attribute Map Flags */
   NCS_IFSV_PORT_TYPE       port_type;
   uns8                     phy_addr[6];
   NCS_STATUS               oper_state;
   uns32                    admin_state; /* Need to define ENUM */
   uns32                    mtu;
   uns8                     if_name[IFSV_IF_NAME_SIZE];
   uns32                    speed;
   MDS_DEST                 dest; /* Vcard ID of the hardware driver process */
} NCS_IFSV_PORT_INFO;


/*****************************************************************************
 * Structure used by the Hardware drv to register its port with IDIM. While 
 * Hardware drv registering with IDIM it should register a API with the IDIM
 * which would be called when any request comes from the IDIM.
 *****************************************************************************/
typedef struct ncs_ifsv_port_reg
{
   NCS_IFSV_PORT_INFO           port_info;    /* Port information                             */
   NCSCONTEXT                   drv_data;     /* this is the data which we would be giving to the driver when the driver registered callback is called */
   NCS_IFSV_HW_DRV_GET_SET_CB   hw_get_set_cb;   /* handle which hardware drv register with IDIM which is used to get/set any parameter from the hardware*/   
}NCS_IFSV_PORT_REG;


/*****************************************************************************
 * This is the structure which will be given to the Interface Hardware driver,
 * when any Hardware driver is coming up.
 *****************************************************************************/
typedef struct ncs_ifnd_up_info
{
   uns32          vrid;
   uns32          nodeid;
   MDS_DEST       ifnd_addr;    
}NCS_IFND_UP_INFO;

/*****************************************************************************
 * This is the information which is used to send back the responce for the 
 * request.
 *****************************************************************************/
typedef struct ncs_ifsv_hw_req_info
{
   NCSMDS_SVC_ID       svc_id;    /* Needs to be passed as such when Driver is responding for any request */
   MDS_DEST            dest_addr; /* Needs to be passed as such when Driver is responding for any request */
   uns32               app_hdl;   /* this is used by the IfA to keep track of the req */      
} NCS_IFSV_HW_REQ_INFO;

/*****************************************************************************
 * This is the request send from IDIM to hardware drv to get 
 * statisctics/port status.
 *****************************************************************************/
typedef struct ncs_ifsv_hw_drv_req
{
   NCS_IFSV_HW_DRV_MSG_TYPE  req_type;
   /* EXT_INT */
   NCS_IFSV_SUBSCR_SCOPE     subscr_scope;
   NCS_IFSV_PORT_TYPE        port_type;   
   NCSCONTEXT                drv_data;   /*this is the data which driver has given to the IDIM during registeration*/
   union 
   {
      NCS_IFSV_HW_REQ_INFO  req_info;        /* NCS_IFSV_HW_DRV_STATS         */    
      NCS_IFND_UP_INFO      ifnd_info;       /* NCS_IFSV_HW_DRV_IFND_UP       */
      NCS_IFSV_INTF_INFO    set_param;       /* NCS_IFSV_HW_DRV_SET_PARAM     */
   } info;   
} NCS_IFSV_HW_DRV_REQ;

/*****************************************************************************
 * This is the event structure used by the Hardware driver to indicate IDIM of
 * its hardware information.
 *****************************************************************************/
typedef struct ncs_ifsv_hw_info
{   
   NCS_IFSV_HW_DRV_MSG_TYPE msg_type;  
   /* EXT_INT */
   NCS_IFSV_SUBSCR_SCOPE     subscr_scope;
   NCS_IFSV_PORT_TYPE       port_type;   
   union 
   {
      NCS_IFSV_PORT_INFO         reg_port;        /* NCS_IFSV_HW_DRV_PORT_REG      */
      NCS_IFSV_PORT_STATUS       port_status;     /* NCS_IFSV_HW_DRV_PORT_STATUS   */
      NCS_IFSV_PORT_STATS        stats;           /* NCS_IFSV_HW_DRV_STATS         */      
   } info;
}NCS_IFSV_HW_INFO;

/*****************************************************************************
 * This is the event structure used by the Hardware driver to do a request
 *****************************************************************************/
typedef struct ncs_ifsv_drv_svc_req_tag
{
   NCS_IFSV_DRV_REQ_TYPE req_type;
   union
   {
      NCS_IFSV_PORT_REG *port_reg;               /* NCS_IFSV_DRV_PORT_REG_REQ */
      NCS_IFSV_HW_INFO  *hw_info;                /* NCS_IFSV_DRV_SEND_INFO_REQ*/
   } info;
}NCS_IFSV_DRV_SVC_REQ;

/*****************************************************************************
 * API used by the Hardware driver to register/send info/init/destroy 
 *****************************************************************************/
IFSVDLL_API uns32 ncs_ifsv_drv_svc_req(NCS_IFSV_DRV_SVC_REQ *info);

#endif 

