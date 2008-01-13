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

#ifndef IFD_RED_H
#define IFD_RED_H
/*****************************************************************************
 * This file contains the data structures, enums, #defines, function 
 * prototypes, needed for IfD 2N redundancy and IfND no redundancy design.
 * 
 * Please insert all data structures, enums, #defs, function prototypes needed
 * for Standby IfD in STANDBY_IFD_BLOCK, for Active IfD in ACTIVE_IFD_BLOCK 
 * Any common things should be put in the global block GLOBAL_BLOCK.
 *****************************************************************************/
/*****************************************************************************
 *          GLOBAL_BLOCK starts here...
 *****************************************************************************/
#define MAX_NO_IFD_MSGS_A2S   2500 /* No of messages to be sent from Active ifd
                                    to Standby Ifd. */
/* all the mbcsv version from IFD_MBCSV_VER_MIN to IFD_MBCSV_VERSION
   should be supported*/
#define IFD_MBCSV_VERSION_MIN 1
#define IFD_MBCSV_VERSION 1

/*****************************************************************************
 * This is the event type that Active will send and Standby IfD will receive. 
 *****************************************************************************/
typedef enum ifd_a2s_evt_type
{
   IFD_A2S_EVT_BASE = 1,
   IFD_A2S_EVT_IFINDEX_SPT_MAP = IFD_A2S_EVT_BASE,
   IFD_A2S_EVT_INTF_DATA,
   IFD_A2S_EVT_SVC_DEST_UPD,
   IFD_A2S_EVT_IFND_UP_DOWN,
   IFD_A2S_EVT_IFINDEX_UPD,
#if(NCS_IFSV_IPXS == 1)
   IFD_A2S_EVT_IPXS_INTF_INFO,
#endif
#if(NCS_VIP == 1)
   IFD_A2S_EVT_VIP_REC_INFO,
#endif
   /* Add event type before here. */
   IFD_A2S_EVT_MAX,
} IFD_A2S_EVT_TYPE;

/*****************************************************************************
 * This is message subtype of SPT map message, when Active will send and 
 * Standby IfD will receive. This would be SPT map CREATE/DELETE.
 *****************************************************************************/
typedef enum ifd_a2s_ifindex_spt_map_type
{
   IFD_A2S_IFINDEX_SPT_MAP_EVT_BASE = 1,
   IFD_A2S_IFINDEX_SPT_MAP_EVT_CREATE = 
                                   IFD_A2S_IFINDEX_SPT_MAP_EVT_BASE,
   IFD_A2S_IFINDEX_SPT_MAP_EVT_DELETE,
   /* Add message subtype before here.*/
   IFD_A2S_IFINDEX_SPT_MAP_EVT_MAX,
} IFD_A2S_IFINDEX_SPT_MAP_EVT_TYPE;

/*****************************************************************************
 * Thiss is record type that will be sent from Active Ifd to standby Ifd.
 *****************************************************************************/
typedef enum ifd_data_res_ifsv_vip_rec_type
{
  IFD_DATA_RES_BASE = 1,
  IFD_DATA_RES_IFSV_REC=IFD_DATA_RES_BASE,
#if (NCS_VIP == 1)
  IFD_DATA_RES_VIP_REC,
#endif
  IFD_DATA_RES_MAX_REC_TYPE
} IFD_DATA_RES_IFSV_VIP_REC_TYPE;

/*****************************************************************************
 * This is the message subtype of interface record message. 
 *****************************************************************************/
typedef enum ifd_a2s_intf_data_evt_type
{
   IFD_A2S_INTF_DATA_EVT_BASE = 1,
   IFD_A2S_INTF_DATA_EVT_CREATE = IFD_A2S_INTF_DATA_EVT_BASE,
   IFD_A2S_INTF_DATA_EVT_MODIFY,
   IFD_A2S_INTF_DATA_EVT_MARKED_DELETE,
   IFD_A2S_INTF_DATA_EVT_DELETE,
   /* Add message subtype before here.*/
   IFD_A2S_INTF_DATA_EVT_MAX,
} IFD_A2S_INTF_DATA_EVT_TYPE;

/*****************************************************************************
 * This is the message subtype of the SVC MDS map. Right now, no type is here. 
 *****************************************************************************/
typedef enum ifd_a2s_svc_dest_upd_evt_type
{
   IFD_A2S_SVC_DEST_UPD_EVT_BASE = 1,
   /* Add  before here.*/
   IFD_A2S_SVC_DEST_UPD_EVT_MAX,
} IFD_A2S_SVC_DEST_UPD_EVT_TYPE;

#if(NCS_IFSV_IPXS == 1)
/*****************************************************************************
 * This is the message subtype of the IPXS. Right now, no type is here. 
 *****************************************************************************/
typedef enum ifd_a2s_ipxs_intf_info_evt_type
{
   IFD_A2S_IPXS_INTF_INFO_EVT_BASE = 1,
   IFD_A2S_IPXS_INTF_INFO_EVT_MAX
} IFD_A2S_IPXS_INTF_INFO_EVT_TYPE;
#endif

#if(NCS_VIP == 1)
/*****************************************************************************
 * This is the message subtype of the VIP.
 *****************************************************************************/
typedef enum ifd_a2s_vip_rec_info_evt_type
{
   IFD_A2S_VIP_REC_INFO_EVT_BASE = 1,
   IFD_A2S_VIP_REC_CREATE = IFD_A2S_VIP_REC_INFO_EVT_BASE,
   IFD_A2S_VIP_REC_MODIFY, 
   IFD_A2S_VIP_REC_DEL,
   IFD_A2S_VIP_REC_INFO_EVT_MAX
} IFD_A2S_VIP_REC_INFO_EVT_TYPE;
#endif

/*****************************************************************************
 * This is the message subtype of IFND UP/DOWN Event.
 *****************************************************************************/
typedef enum ifd_a2s_ifnd_down_evt_type
{
   IFD_A2S_IFND_UP_DOWN_EVT_BASE = 1,
   IFD_A2S_IFND_DOWN_EVT = IFD_A2S_IFND_UP_DOWN_EVT_BASE,
   IFD_A2S_IFND_UP_EVT,
   IFD_A2S_IFND_DOWN_EVT_MAX,
} IFD_A2S_IFND_UP_DOWN_EVT_TYPE;

/*****************************************************************************
 * This is the message subtype of IF INDEX alloc/dealloc Event. Right now, no 
 * type is here.
 *****************************************************************************/
typedef enum ifd_a2s_ifindex_upd_evt_type
{
   IFD_A2S_IFINDEX_UPD_EVT_BASE = 1,
   IFD_A2S_IFINDEX_EVT_ALLOC = IFD_A2S_IFINDEX_UPD_EVT_BASE,
   IFD_A2S_IFINDEX_EVT_DEALLOC,
   IFD_A2S_IFINDEX_UPD_EVT_MAX,
} IFD_A2S_IFINDEX_UPD_EVT_TYPE;

/*****************************************************************************
 * This is the SPT message, which has SPT map information.  
 *****************************************************************************/
typedef struct ifd_a2s_ifindex_spt_map_evt
{
   IFD_A2S_IFINDEX_SPT_MAP_EVT_TYPE type;
   NCS_IFSV_SPT_MAP info;   
} IFD_A2S_IFINDEX_SPT_MAP_EVT;

/*****************************************************************************
 * This is the interface record message, contains all the information about
 * the interface. 
 *****************************************************************************/
typedef struct ifd_a2s_intf_data_evt
{
   IFD_A2S_INTF_DATA_EVT_TYPE type;
   IFSV_INTF_DATA info;   
} IFD_A2S_INTF_DATA_EVT;

/*****************************************************************************
 * This is the SVC MDS map information message. 
 *****************************************************************************/
typedef struct ifd_a2s_svc_dest_upd_evt
{
   IFD_A2S_SVC_DEST_UPD_EVT_TYPE type;
   NCS_IFSV_SVC_DEST_UPD info;   
} IFD_A2S_SVC_DEST_UPD_EVT;

#if(NCS_IFSV_IPXS == 1)
/*****************************************************************************
 * This is the IPXS information message. 
 *****************************************************************************/
typedef struct ifd_a2s_ipxs_intf_info_evt
{
   IFD_A2S_IPXS_INTF_INFO_EVT_TYPE  type;
   IPXS_EVT_IF_INFO  info;   
} IFD_A2S_IPXS_INTF_INFO_EVT;
#endif

#if(NCS_VIP == 1)
/* The follwing structures are used for checkpointing/update the VIPD partially */
typedef struct vip_red_ip_node {
   NCS_IPPFX       ip_addr;
   NCS_BOOL        ipAllocated;
   uns8            intfName[m_NCS_IFSV_VIP_INTF_NAME];
}VIP_RED_IP_NODE;

typedef struct vip_red_intf_node {
   uns8             intfName[m_NCS_IFSV_VIP_INTF_NAME];
   NCS_BOOL         active_standby;
}VIP_RED_INTF_NODE;

typedef struct vip_red_owner_list {
   MDS_DEST    owner;
#if (VIP_HALS_SUPPORT == 1)
   uns32       infraFlag;
#endif
}VIP_RED_OWNER_NODE;

/* This structure is used for checkpointing the VIPD for the first time */
typedef struct vip_redundancy_record {
   NCS_IFSV_VIP_INT_HDL     handle;
   uns32                    vip_entry_attr;
   uns32                    ref_cnt;
   uns32                    ip_list_cnt;
   VIP_RED_IP_NODE         *ipInfo;
   uns32                    intf_list_cnt;
   VIP_RED_INTF_NODE         *intfInfo;
   uns32                    alloc_ip_list_cnt;
   uns32                    owner_list_cnt;
   VIP_RED_OWNER_NODE         *ownerInfo; 
}VIP_REDUNDANCY_RECORD;

typedef struct vip_redundancy_data_base {
   uns32 vip_rec_cnt;
   VIP_REDUNDANCY_RECORD *p_vip_record;
}VIP_REDUNDANCY_DB;
typedef struct vipd_red_partial_record {
   NCS_IFSV_VIP_INT_HDL     handle;
   uns32                    ref_cnt;
   uns32                    ip_list_cnt;
   VIP_RED_IP_NODE         *ipInfo;
   uns32                    intf_list_cnt;
   VIP_RED_INTF_NODE       *intfInfo;
   uns32                    alloc_ip_list_cnt;
   uns32                    owner_list_cnt;
   VIP_RED_OWNER_NODE      *ownerInfo;
}VIPD_RED_PARTIAL_RECORD;



/*****************************************************************************
 * This is the VIP information message.
 *****************************************************************************/
typedef struct ifd_a2s_vip_rec_info_evt
{
   IFD_A2S_VIP_REC_INFO_EVT_TYPE  type;
   VIP_REDUNDANCY_RECORD  info;
} IFD_A2S_VIP_REC_INFO_EVT;
#endif

/*****************************************************************************
 * This is the IFND UP/DOWN information message. 
 *****************************************************************************/
typedef struct ifd_a2s_ifnd_up_down_evt
{
   IFD_A2S_IFND_UP_DOWN_EVT_TYPE  type;
   MDS_DEST  mds_dest;   
} IFD_A2S_IFND_UP_DOWN_EVT;

/*****************************************************************************
 * This is the IF INDEX information message.
 *****************************************************************************/
typedef struct ifd_a2s_ifindex_upd_evt
{
   IFD_A2S_IFINDEX_UPD_EVT_TYPE  type;
   NCS_IFSV_IFINDEX  ifindex;
} IFD_A2S_IFINDEX_UPD_EVT;

/*****************************************************************************
 * This is the structure, which contains event types and corresponding message
 * information as union. 
 *****************************************************************************/
typedef struct ifd_a2s_msg
{
   IFD_A2S_EVT_TYPE  type;  
   union
   {
       IFD_A2S_IFINDEX_SPT_MAP_EVT     ifd_a2s_ifindex_spt_map_evt;
       IFD_A2S_INTF_DATA_EVT           ifd_a2s_intf_data_evt;
       IFD_A2S_SVC_DEST_UPD_EVT        ifd_a2s_svc_dest_upd_evt;
       IFD_A2S_IFND_UP_DOWN_EVT        ifd_a2s_ifnd_up_down_evt;
       IFD_A2S_IFINDEX_UPD_EVT         ifd_a2s_ifindex_upd_evt;
#if(NCS_IFSV_IPXS == 1)
       IFD_A2S_IPXS_INTF_INFO_EVT      ifd_a2s_ipxs_intf_info_evt;
#endif
#if(NCS_VIP == 1)
       IFD_A2S_VIP_REC_INFO_EVT        ifd_a2s_vip_rec_info_evt;
#endif
   } info;
} IFD_A2S_MSG;

/*****************************************************************************
 *          GLOBAL_BLOCK ends here...
 *****************************************************************************/

/*****************************************************************************
 *          STANDBY_IFD_BLOCK starts here...
 *****************************************************************************/

/*****************************************************************************
 * This is the error types, that may occur in Standby IfD processing.
 *****************************************************************************/
typedef enum ifd_stdby_error_type
{
  IFD_STDBY_ERROR_TYPE_BASE = 1,
  IFD_STDBY_IFINDEX_SPT_MAP_ADD_FAILURE,
  IFD_STDBY_IFINDEX_SPT_MAP_DELETE_FAILURE,
  IFD_STDBY_INTF_DATA_CREATE_FAILURE,
  IFD_STDBY_INTF_DATA_MODIFY_FAILURE,
  IFD_STDBY_INTF_DATA_MARKED_DELETE_FAILURE,
  IFD_STDBY_INTF_DATA_DELETE_FAILURE,
  IFD_STDBY_SVC_DEST_UPD_FAILURE,
  IFD_STDBY_IFND_UP_FAILURE,
  IFD_STDBY_IFND_DOWN_FAILURE,
  IFD_STDBY_IFINDEX_ALLOC_FAILURE,
  IFD_STDBY_IFINDEX_DEALLOC_FAILURE,
#if(NCS_IFSV_IPXS == 1)
  IFD_STDBY_IPXS_INTF_INFO_UPD_FAILURE,
#endif
  /* Add error types, before here.*/
  IFD_STDBY_ERROR_TYPE_MAX,
} IFD_STDBY_ERROR_TYPE;

/*****************************************************************************
 * This is the data structure for storing NODE_ID of IfNDs if IfND does DOWN.
 *****************************************************************************/
typedef struct ifnd_node_id_info
{
   NODE_ID    ifnd_node_id;
   IFSV_TMR   tmr;
}IFND_NODE_ID_INFO;

typedef struct ifd_ifnd_node_id_info_rec
{
   NCS_PATRICIA_NODE     pat_node;
   IFND_NODE_ID_INFO     info;
}IFD_IFND_NODE_ID_INFO_REC;

/*****************************************************************************
 *                  # defines 
 *****************************************************************************/
#define IFD_STANDBY  1

/*****************************************************************************
 *                  STANDBY_IFD_BLOCK ends here...
 *****************************************************************************/

/*****************************************************************************
 * This is the message types that Active IfD will use to send messages to 
 * Stdby IfD. 
 *****************************************************************************/
typedef enum ifd_a2s_msg_type
{
   IFD_A2S_MSG_BASE = 1,
   IFD_A2S_SPT_MAP_MAP_CREATE_MSG,
   IFD_A2S_SPT_MAP_DELETE_MSG,
   IFD_A2S_DATA_CREATE_MSG,
   IFD_A2S_DATA_MODIFY_MSG,
   IFD_A2S_DATA_MARKED_DELETE_MSG,
   IFD_A2S_DATA_DELETE_MSG,
   IFD_A2S_SVC_DEST_UPD_MSG,
   IFD_A2S_IFND_DOWN_MSG,
   IFD_A2S_IFND_UP_MSG,
   IFD_A2S_IFINDEX_ALLOC_MSG,
   IFD_A2S_IFINDEX_DEALLOC_MSG,
#if(NCS_IFSV_IPXS == 1)
   IFD_A2S_IPXS_MSG,
#endif
#if(NCS_VIP == 1)
   IFD_A2S_VIPD_REC_CREATE_MSG,
   IFD_A2S_VIPD_REC_MODIFY_MSG,
   IFD_A2S_VIPD_REC_DEL_MSG,
#endif
   /* Add event type before here. */
   IFD_A2S_MSG_MAX,
} IFD_A2S_MSG_TYPE;
/*****************************************************************************
 *          ACTIVE_IFD_BLOCK starts here...
 *****************************************************************************/

/*****************************************************************************
 *                  ACTIVE_IFD_BLOCK ends here...
 *****************************************************************************/
  
extern uns32 ifd_a2s_ifindex_spt_map_handler(IFD_A2S_IFINDEX_SPT_MAP_EVT *info,
                                             IFSV_CB *cb);
extern uns32 ifd_a2s_intf_data_handler (IFD_A2S_INTF_DATA_EVT *info,
                                        IFSV_CB *cb);
extern uns32 ifd_a2s_svc_dest_upd_handler(IFD_A2S_SVC_DEST_UPD_EVT *info,
                                          IFSV_CB *cb);
void data_dump_VIP_REDUNDANCY_RECORD(VIP_REDUNDANCY_RECORD *pVipChkptPkt);
#if(NCS_IFSV_IPXS == 1)
extern uns32 ifd_a2s_ipxs_intf_info_handler(IFD_A2S_IPXS_INTF_INFO_EVT *info,
                                            IFSV_CB *cb);
#endif
extern uns32 ifd_a2s_ifnd_up_down_info_handler(IFD_A2S_IFND_UP_DOWN_EVT *info,
                                            IFSV_CB *cb);
extern uns32 ifd_a2s_ifindex_upd_handler(IFD_A2S_IFINDEX_UPD_EVT *info,
                                            IFSV_CB *cb);
extern uns32 ifd_a2s_async_update (IFSV_CB *cb, IFD_A2S_MSG_TYPE type, void *msg);
extern IFND_NODE_ID_INFO *ifd_ifnd_node_id_info_get (NODE_ID node_id, IFSV_CB *cb);

extern uns32 ifd_ifnd_node_id_info_del (NODE_ID node_id, IFSV_CB *cb);

extern uns32 ifd_ifnd_node_id_info_add (IFD_IFND_NODE_ID_INFO_REC **rec, NODE_ID *node_id, IFSV_CB *cb);
extern uns32  ifd_mbcsv_dec_data_resp(void *cb, NCS_MBCSV_CB_ARG *arg);
extern uns32  ifd_mbcsv_enc_data_resp(void *cb, NCS_MBCSV_CB_ARG *arg);
extern uns32 ifd_a2s_sync_resp (IFSV_CB *cb, uns8 *intf_msg
#if(NCS_IFSV_IPXS == 1)
, uns8 *ipxs_msg
#endif
);
extern uns32  ifd_mbcsv_chgrole(IFSV_CB *cb);
extern uns32 ifd_mbcsv_register(IFSV_CB *cb);
extern uns32  ifd_mbcsv_finalize(IFSV_CB *cb);

#if(NCS_VIP == 1)
extern uns32
ifd_a2s_vip_rec_handler (IFD_A2S_VIP_REC_INFO_EVT *info, IFSV_CB *cb);
extern uns32
ifd_a2s_vip_rec_create_handler (VIP_REDUNDANCY_RECORD *pVipChkptPkt, IFSV_CB *cb);
extern uns32
ifd_a2s_vip_rec_modify_handler (VIP_REDUNDANCY_RECORD *pVipChkptPkt, IFSV_CB *cb);
extern uns32
ifd_a2s_vip_rec_delete_handler (VIP_REDUNDANCY_RECORD *info, IFSV_CB *cb);
#endif


/************************ file ends here *******************************/
#endif 
