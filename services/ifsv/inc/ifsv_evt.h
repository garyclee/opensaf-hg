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

#ifndef IFSV_EVT_H
#define IFSV_EVT_H

/*#include "vip.h"*/

/* Macros for Sending and Receiving the Events */
#define m_IFD_EVT_SEND(mbx,msg,prio) m_NCS_IPC_SEND(mbx,(NCSCONTEXT)msg,prio)
#define m_IFND_EVT_SEND(mbx,msg,prio) m_NCS_IPC_SEND(mbx,(NCSCONTEXT)msg,prio)
#define m_IFSV_MBX_RECV(mbx,evt) m_NCS_IPC_RECEIVE(mbx,evt)


/*****************************************************************************
 * Event Type of IfD, IfND and IfA.
 *****************************************************************************/
typedef enum ifsv_evt_type
{
   IFSV_EVT_BASE,

   /* events received by the IFD */
   IFD_EVT_BASE= IFSV_EVT_BASE,
   IFD_EVT_INTF_CREATE =  IFD_EVT_BASE,
   IFD_EVT_INTF_DESTROY,
   IFD_EVT_INIT_DONE,
   IFD_EVT_IFINDEX_REQ,
   IFD_EVT_IFINDEX_CLEANUP,   
   IFD_EVT_TMR_EXP,
   IFD_EVT_INTF_REC_SYNC,
   IFD_EVT_SVCD_UPD,
   IFD_EVT_QUISCED_STATE_INFO,
   IFD_EVT_IFND_DOWN,
   IFD_EVT_MAX,

   /* events received by the IFND */
   IFND_EVT_BASE,
   IFND_EVT_INTF_CREATE = IFND_EVT_BASE,
   IFND_EVT_INTF_DESTROY,
   IFND_EVT_INIT_DONE,   
   IFND_EVT_INTF_INFO_GET,
   IFND_EVT_IDIM_STATS_RESP,
   IFND_EVT_TMR_EXP,
   IFND_EVT_SVCD_UPD_FROM_IFA,
   IFND_EVT_SVCD_UPD_FROM_IFD,
   IFND_EVT_SVCD_GET,
   IFND_EVT_IFD_DOWN,
   IFND_EVT_IFA_DOWN_IN_OPER_STATE,
   IFND_EVT_IFA_DOWN_IN_DATA_RET_STATE,
   IFND_EVT_IFA_DOWN_IN_MDS_DEST_STATE,
   IFND_EVT_IFDRV_DOWN_IN_OPER_STATE,
   IFND_EVT_IFDRV_DOWN_IN_DATA_RET_STATE,
   IFND_EVT_IFDRV_DOWN_IN_MDS_DEST_STATE,
   IFND_EVT_IFD_UP,
   IFND_EVT_IFINDEX_RESP,
   IFND_EVT_INTF_CREATE_RSP,
   IFND_EVT_INTF_DESTROY_RSP,
   IFND_EVT_SVCD_UPD_RSP_FROM_IFD,
   IFND_EVT_MAX,
   /*  Events received by the IFA */
   IFA_EVT_BASE,
   IFA_EVT_INTF_CREATE,
   IFA_EVT_INTF_UPDATE,
   IFA_EVT_INTF_DESTROY,
   IFA_EVT_IFINFO_GET_RSP,
   IFA_EVT_INTF_CREATE_RSP,
   IFA_EVT_INTF_DESTROY_RSP,
   IFA_EVT_SVCD_UPD_RSP,
   IFA_EVT_SVCD_GET_SYNC_RSP,
   IFA_EVT_SVCD_GET_RSP,
   IFA_EVT_MAX = IFA_EVT_SVCD_GET_RSP,
   IFSV_EVT_MAX = IFA_EVT_MAX,

#if(NCS_IFSV_IPXS == 1)
   IFSV_IPXS_EVT = IFSV_EVT_MAX+1,
   IFSV_IPXS_EVT_MAX = IFSV_IPXS_EVT,
#endif
#if (NCS_VIP == 1)
   IFA_VIPD_INFO_ADD_REQ = IFSV_IPXS_EVT_MAX+1,
   IFND_VIPD_INFO_ADD_REQ,
   IFA_VIP_FREE_REQ,
   IFND_VIP_FREE_REQ,
   IFND_VIPD_INFO_ADD_REQ_RESP,
   IFD_VIPD_INFO_ADD_REQ_RESP,
   IFND_VIP_INSTALL_MSG,
   IFND_VIP_FREE_REQ_RESP,
   IFD_VIP_FREE_REQ_RESP,
   IFSV_VIP_ERROR,
   IFND_IFND_VIP_DEL_VIPD, 
   IFND_VIP_MARK_VIPD_STALE,
   IFA_GET_IP_FROM_STALE_ENTRY_REQ,
   IFND_GET_IP_FROM_STALE_ENTRY_REQ,
   IFD_IP_FROM_STALE_ENTRY_RESP,
   IFND_IP_FROM_STALE_ENTRY_RESP,
   IFSV_VIP_EVT_MAX = IFND_IP_FROM_STALE_ENTRY_RESP,
#endif
   IFSV_EVT_NETLINK_LINK_UPDATE,
   IFSV_EVT_LAST
} IFSV_EVT_TYPE;

/* This is the function prototype for event handling */
typedef uns32 (*IFSV_EVT_HANDLER) (struct ifsv_evt* evt, IFSV_CB *cb);

/* Enum to define the get types for interface record */
typedef enum ifsv_intf_get_type
{
   IFSV_INTF_GET_ALL = 1,
   IFSV_INTF_GET_ONE
}IFSV_INTF_GET_TYPE;

/*****************************************************************************
 * This Event structure is sent by IFA to IFND to get the interface record
 *****************************************************************************/
typedef struct ifsv_evt_intf_info_get
{
   IFSV_INTF_GET_TYPE      get_type;    /* Get All/One */
   NCS_IFSV_IFKEY          ifkey;       /* Interface Key */
   NCS_IFSV_INFO_TYPE      info_type;   /* IFINFO and/or STATS */
   NCSMDS_SVC_ID           my_svc_id;   /* Svc ID & Vcard ID are used to send*/
   MDS_DEST                my_dest;     /* the MDS Message back to IFA */ 
   uns32                   usr_hdl;     /* Applications Handle */
} IFSV_EVT_INTF_INFO_GET;


/*****************************************************************************
 * This Event structure is used to exchange shelf/slot/port/type/scope->
 * Ifindex mapping between IfD and IfND.
 *****************************************************************************/
typedef struct ifsv_evt_slot_port_map_info
{
   NCS_IFSV_SPT_MAP     spt_map;
   NCSMDS_SVC_ID        app_svc_id;
   MDS_DEST             app_dest;
}IFSV_EVT_SPT_MAP_INFO;


/*****************************************************************************
 * This Event structure is used send a create or modify request to IfD or IfND.
 *****************************************************************************/
typedef struct ifsv_intf_create_info
{
   uns32                if_attr;    /* JAGS : TBD This needs to be removed */
   NCS_IFSV_EVT_ORIGN   evt_orig;
   IFSV_INTF_DATA       intf_data;
} IFSV_INTF_CREATE_INFO;

/*****************************************************************************
 * This Event structure is used send a destroy request to IfD or IfND.
 *****************************************************************************/
typedef struct ifsv_intf_destroy_info
{
   NCS_IFSV_SPT          spt_type;  /* TBD : JAGS Change this to spt_info */
   NCS_IFSV_EVT_ORIGN    orign;     /* orign, how wants to destroy this record (IfA/IfD/IfND(in the case of proxy)/Hw Driver*/
   MDS_DEST              own_dest; /* the dest of the interface which is owning this record.*/
}IFSV_INTF_DESTROY_INFO;

/*****************************************************************************
 * This Event structure is used to inform that the particular component is 
 * initialized (used for IfD init or IfND init).
 *****************************************************************************/
typedef struct ifsv_evt_init_done_info
{
   NCS_BOOL             init_done; /* TRUE successfully initialized/ FALSE failure */
   MDS_DEST             my_dest;
} IFSV_EVT_INIT_DONE_INFO;

/*****************************************************************************
 * This Event structure is used to inform that the particular component is
 * DOWN (used for IfD/IfDrv/IfA).
 *****************************************************************************/
typedef struct ifsv_evt_mds_svc_info
{
   MDS_DEST mds_dest;
} IFSV_EVT_MDS_SVC_INFO;

/*****************************************************************************
 * This Event structure is used to indicate that the interface aging timer has
 * expired and it needs to cleanup that interface record.
 *****************************************************************************/
typedef struct ifsv_evt_tmr
{
   NCS_IFSV_TMR_TYPE  tmr_type;
   union /* To be filled and used by user. */
   {
    /* Based on timer type, we can use the following fields. */
    uns32  ifindex; /* For NCS_IFSV_IFND_EVT_INTF_AGING_TMR and
                       NCS_IFSV_IFD_EVT_INTF_AGING_TMR.*/
    uns32  reserved; /* Right now, not in used. Added just for uniformity. For
                        NCS_IFSV_IFND_MDS_DEST_GET_TMR */
    uns32  node_id;  /* For NCS_IFSV_IFD_EVT_RET_TMR. */
   } info;
}IFSV_EVT_TMR;

/*****************************************************************************
 * This Event structure is used to indicate IfD that the IfND came up, so that
 * IfD will sync up the database with IfND.
 *****************************************************************************/
typedef struct ifsv_evt_intf_rec_sync
{
   MDS_DEST    ifnd_vcard_id;
}IFSV_EVT_INTF_REC_SYNC;

/*****************************************************************************
 * This Event structure is used to get the statistics from the hardware driver.
 *****************************************************************************/
typedef struct ifsv_evt_stats_info
{   
   NCS_IFSV_SPT               spt_type;
   NCS_BOOL                   status; /* TRUE - successfully fetched */
   NCSMDS_SVC_ID              svc_id;
   MDS_DEST                   dest;
   uns32                      usr_hdl; /* user handle which IFA would be used to send it to the Application */
   NCS_IFSV_INTF_STATS        stat_info;
}IFSV_EVT_STATS_INFO;

/*****************************************************************************
 * IfD event data structure.
 *****************************************************************************/
typedef struct ifd_evt
{   
   union
   {
      IFSV_INTF_CREATE_INFO         intf_create;         /* IFD_EVT_INTF_CREATE                               */
      IFSV_INTF_DESTROY_INFO        intf_destroy;        /* IFD_EVT_INTF_DESTROY                              */
      IFSV_EVT_INIT_DONE_INFO       init_done;           /* IFD_EVT_INIT_DONE                                 */      
      IFSV_EVT_SPT_MAP_INFO         spt_map;             /* IFD_EVT_IFINDEX_REQ, IFD_EVT_IFINDEX_CLEANUP */      
      IFSV_EVT_TMR            tmr_exp;             /* IFD_EVT_INTF_AGING_TMR_EXP                        */
      IFSV_EVT_INTF_REC_SYNC        rec_sync;            /* IFD_EVT_INTF_REC_SYNC                             */
      NCS_IFSV_SVC_DEST_UPD         svcd;                /* IFD_EVT_SVCD_UPD */
      MDS_DEST                      mds_dest;            /* IFD_EVT_IFND_DOWN */
   }info;
}IFD_EVT;

/*****************************************************************************
 * IfND event data structure.
 *****************************************************************************/
typedef struct ifnd_evt
{   
   union
   {
      IFSV_INTF_CREATE_INFO        intf_create;         /* IFND_EVT_INTF_CREATE    */
      IFSV_INTF_DESTROY_INFO       intf_destroy;        /* IFND_EVT_INTF_DESTROY   */
      IFSV_EVT_INIT_DONE_INFO      init_done;           /* IFND_EVT_INIT_DONE      */            
      IFSV_EVT_SPT_MAP_INFO        spt_map; /* IFND_EVT_IFINDEX_RESP, 
                                               IFND_EVT_INTF_CREATE_RSP,
                                               IFND_EVT_INTF_DESTROY_RSP */     
      IFSV_EVT_INTF_INFO_GET       if_get;              /* IFND_EVT_INTF_INFO_GET (IfA)    */       
      IFSV_EVT_STATS_INFO          stats_info;          /* IFND_EVT_IDIM_STATS_RESP (IDIM) */
      IFSV_EVT_TMR           tmr_exp;             /* IFND_EVT_INTF_AGING_TMR_EXP     */
      NCS_IFSV_SVC_DEST_UPD        svcd;                /* IFND_EVT_SVCD_UPD_FROM_IFA
                                                           & IFND_EVT_SVCD_UPD_FROM_IFD */
      NCS_IFSV_SVC_DEST_GET        svcd_get;            /* IFND_EVT_SVCD_GET */
      IFSV_EVT_MDS_SVC_INFO         mds_svc_info;  /* IFND_EVT_IFD_DOWN,
                                       IFND_EVT_IFA_DOWN_IN_OPER_STATE,
                                       IFND_EVT_IFA_DOWN_IN_DATA_RET_STATE,
                                       IFND_EVT_IFA_DOWN_IN_MDS_DEST_STATE,
                                       IFND_EVT_IFDRV_DOWN_IN_OPER_STATE,
                                       IFND_EVT_IFDRV_DOWN_IN_DATA_RET_STATE,
                                       IFND_EVT_IFDRV_DOWN_IN_MDS_DEST_STATE */      
   }info;
}IFND_EVT;


/*****************************************************************************
 * IfA event data structure.
 *****************************************************************************/
typedef struct ifa_evt
{  NCSCONTEXT              usrhdl;  /* Need to be filled in case of ifget */
   NCS_IFSV_SUBSCR_SCOPE   subscr_scope;/* This is needed by If Agent to decide 
                                        whether the record coming from IfND is 
                                        for internal or external interface. */
   union
   {
      NCS_IFSV_IFGET_RSP    ifget_rsp;
      NCS_IFSV_INTF_REC     if_add_upd;
      NCS_IFSV_IFINDEX      if_del;
      NCS_IFSV_SVC_DEST_GET svcd_rsp;   /* IFA_EVT_SVCD_GET_RSP */
      NCS_IFSV_IFINDEX      if_add_rsp_idx; /* IFA_EVT_INTF_CREATE_RSP */
      IFSV_EVT_SPT_MAP_INFO spt_map; /* IFA_EVT_INTF_DESTROY_RSP  */      
      NCS_IFSV_SVC_DEST_UPD svcd_sync_resp; /* IFA_EVT_SVCD_GET_SYNC_RSP */
   }info;
}IFA_EVT;

/*****************************************************************************
 * Sync Send Error.
 *****************************************************************************/
typedef enum ncs_ifsv_sync_send_error
{
   NCS_IFSV_SYNC_SEND_ERROR_BASE = 1,
   NCS_IFSV_NO_ERROR = NCSCC_RC_SUCCESS,
   NCS_IFSV_SYNC_TIMEOUT,
   NCS_IFSV_INT_ERROR, /* Internal data processing error. */
   NCS_IFSV_EXT_ERROR, /* External data processing error e.g. MDS failure. */
   NCS_IFSV_SYNC_SEND_UNKNOWN_ERROR,
   NCS_IFSV_IFND_DOWN_ERROR,
   NCS_IFSV_IFND_RESTARTING_ERROR,
   NCS_IFSV_INTF_MOD_ERROR, /* Not able to modify intf rec. */
   NCS_IFSV_SYNC_SEND_ERROR_MAX,
}NCS_IFSV_SYNC_SEND_ERROR;

/* IFSV Event Data Structure */
/*****************************************************************************
 * IfSv event data structure.
 *****************************************************************************/
typedef struct ifsv_evt
{
   struct ifsv_evt        *next;
   uns32                  priority;
   IFSV_EVT_TYPE          type;  
   NCS_IFSV_SYNC_SEND_ERROR error;
   uns32                  vrid;
   uns32                  cb_hdl;
   union
   {
       IFD_EVT        ifd_evt;
       IFND_EVT       ifnd_evt;
       IFA_EVT        ifa_evt;
       NCSCONTEXT     ipxs_evt;
#if (NCS_VIP == 1)
       VIP_EVT        vip_evt;
#endif 
    }info;
   IFSV_SEND_INFO     sinfo;
}IFSV_EVT;

/* prototype to the events received by IfD/IfND */
void ifsv_evt_destroy (IFSV_EVT *evt);

uns32
ifnd_process_evt (IFSV_EVT* evt);

#endif
