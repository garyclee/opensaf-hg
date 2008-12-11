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


#ifndef IFSV_DB_H
#define IFSV_DB_H

/*****************************************************************************
FILE NAME: IFSVDB.H
DESCRIPTION: This file contains:
      1) Enums, Structures for interface record and interface attributes.
      2) Enums, Structures for IFSV Database (Interface database, Interface Map
         Table)
      3) Function prototypes for interface database manipulations.       
******************************************************************************/

/* forward declerations to prevent compilation errors*/
struct ifsv_intf_create_info;
struct ifsv_intf_destroy_info;

/* Macro to get the IFND CB handle */
#define m_IFND_CB_HDL_GET( ) (gl_ifnd_cb_hdl)

#define m_IFND_STORE_HDL(cb_hdl) (gl_ifnd_cb_hdl = (cb_hdl))

extern uns32 gl_ifnd_cb_hdl;

/* Macro to get the IFD CB handle */
#define m_IFD_CB_HDL_GET( ) (gl_ifd_cb_hdl)

#define m_IFD_STORE_HDL(cb_hdl) (gl_ifd_cb_hdl = (cb_hdl))

extern uns32 gl_ifd_cb_hdl;

/*****************************************************************************
 * Macros for interface attribute type definition and Manipulation
 *****************************************************************************/
#define IFSV_INTF_ATTR uns32
#define m_IFSV_IS_ATTR_ZERO(attr) ((attr == 0)?TRUE:FALSE)
#define m_IFSV_COPY_ATTR(src,dst) (src=dst)
#define m_IFSV_ATTR_SET_ZERO(attr) (attr=0)


/*****************************************************************************
 * Macros used to check the presence of attributes in the record data.
 *****************************************************************************/
#define m_IFSV_IS_MTU_ATTR_SET(attr)   \
            (((attr & NCS_IFSV_IAM_MTU) != 0)?TRUE:FALSE)
#define m_IFSV_IS_IFSPEED_ATTR_SET(attr)  \
            (((attr & NCS_IFSV_IAM_IFSPEED) != 0)?TRUE:FALSE)
#define m_IFSV_IS_PHYADDR_ATTR_SET(attr)  \
            (((attr & NCS_IFSV_IAM_PHYADDR) != 0)?TRUE:FALSE)
#define m_IFSV_IS_ADMSTATE_ATTR_SET(attr) \
            (((attr & NCS_IFSV_IAM_ADMSTATE) != 0)?TRUE:FALSE)
#define m_IFSV_IS_OPRSTATE_ATTR_SET(attr) \
            (((attr & NCS_IFSV_IAM_OPRSTATE) != 0)?TRUE:FALSE)
#define m_IFSV_IS_NAME_ATTR_SET(attr) \
            (((attr & NCS_IFSV_IAM_NAME) != 0)?TRUE:FALSE)
#define m_IFSV_IS_DESCR_ATTR_SET(attr) \
            (((attr & NCS_IFSV_IAM_DESCR) != 0)?TRUE:FALSE)
#define m_IFSV_IS_LAST_CHNG_ATTR_SET(attr) \
            (((attr & NCS_IFSV_IAM_LAST_CHNG) != 0)?TRUE:FALSE)
#define m_IFSV_IS_SVDEST_ATTR_SET(attr) \
            (((attr & NCS_IFSV_IAM_SVDEST) != 0)?TRUE:FALSE)
#if (NCS_IFSV_BOND_INTF == 1)
#define m_IFSV_IS_MASTER_CHNG_ATTR_SET(attr) \
         (((attr & NCS_IFSV_IAM_CHNG_MASTER) != 0)?TRUE:FALSE)
#define m_IFSV_IS_BINDING_IFINDEX_ATTR_SET(attr) \
                      (((attr & NCS_IFSV_BINDING_CHNG_IFINDEX) != 0)?TRUE:FALSE)
#endif

/******************************************************************************
 Macros to set the attributes 
 *****************************************************************************/
#define m_IFSV_MTU_ATTR_SET(attr)  (attr = attr | NCS_IFSV_IAM_MTU)
#define m_IFSV_IFSPEED_ATTR_SET(attr)  (attr = attr | NCS_IFSV_IAM_IFSPEED)
#define m_IFSV_PHYADDR_ATTR_SET(attr)  (attr = attr | NCS_IFSV_IAM_PHYADDR)
#define m_IFSV_ADMSTATE_ATTR_SET(attr) (attr = attr | NCS_IFSV_IAM_ADMSTATE)
#define m_IFSV_OPRSTATE_ATTR_SET(attr) (attr = attr | NCS_IFSV_IAM_OPRSTATE)
#define m_IFSV_NAME_ATTR_SET(attr)     (attr = attr | NCS_IFSV_IAM_NAME)
#define m_IFSV_DESCR_ATTR_SET(attr)    (attr = attr | NCS_IFSV_IAM_DESCR)
#define m_IFSV_LAST_CHNG_ATTR_SET(attr) (attr = attr | NCS_IFSV_IAM_LAST_CHNG)
#define m_IFSV_SVDEST_ATTR_SET(attr)   (attr = attr | NCS_IFSV_IAM_SVDEST)
#if (NCS_IFSV_BOND_INTF == 1)
#define m_IFSV_CHNG_MASTER_ATTR_SET(attr)   (attr = attr | NCS_IFSV_IAM_CHNG_MASTER)
#endif

#define m_IFSV_ALL_ATTR_SET(attr) \
     (attr = (NCS_IFSV_IAM_MTU | NCS_IFSV_IAM_IFSPEED | NCS_IFSV_IAM_PHYADDR |      \
              NCS_IFSV_IAM_ADMSTATE | NCS_IFSV_IAM_OPRSTATE | NCS_IFSV_IAM_OPRSTATE \
              | NCS_IFSV_IAM_NAME | NCS_IFSV_IAM_DESCR | NCS_IFSV_IAM_LAST_CHNG | NCS_IFSV_IAM_CHNG_MASTER))


/* TBD:RSR Visit this latter when IPXS comes into picture */
#define IFSV_INTF_MAX_REC_TYPE (2)

/*****************************************************************************
 * The below typedefs callback are used to abstract all the protocol 
 * specific action.
 *****************************************************************************/
typedef uns32 (*IFSV_NW_LAYER_ADD)(NCSCONTEXT actual_data, NCSCONTEXT mod_data, IFSV_INTF_ATTR attr, IFSV_CB *cb);
typedef uns32 (*IFSV_NW_LAYER_DEL)(NCSCONTEXT actual_data, NCSCONTEXT mod_data, IFSV_INTF_ATTR attr, IFSV_CB *cb);
typedef uns32 (*IFSV_NW_LAYER_MODIFY)(NCSCONTEXT actual_data, NCSCONTEXT mod_data, IFSV_INTF_ATTR attr, IFSV_CB *cb);

/*****************************************************************************
 * Enumurated type used to identify the action taken on the interface record
 * for DB operations
 *****************************************************************************/
typedef enum ifsv_intf_rec_evt
{
   IFSV_INTF_REC_ADD,
   IFSV_INTF_REC_DEL,
   IFSV_INTF_REC_MODIFY,
   IFSV_INTF_REC_MARKED_DEL,
   IFSV_INTF_REC_MAX_ACTION
} IFSV_INTF_REC_EVT;

/*****************************************************************************
 * Enumurated type used to identify the type of record (IP/Tunnel etc..).
 *****************************************************************************/
typedef enum ncs_ifsv_intf_rec_type
{
   NCS_IFSV_INTF_IP_REC = 0x01,
   NCS_IFSV_INTF_TUNNEL_REC,
   NCS_IFSV_INTF_MAX_REC_TYPE
} NCS_IFSV_INTF_REC_TYPE;

/*****************************************************************************
 * This is the message type which informs how has originated the event
 * (IfA/IfD/IfND/HwDrv).
 *****************************************************************************/
typedef enum ncs_ifsv_evt_orign
{
   NCS_IFSV_EVT_ORIGN_IFA,
   NCS_IFSV_EVT_ORIGN_IFD,
   NCS_IFSV_EVT_ORIGN_IFND,
   NCS_IFSV_EVT_ORIGN_HW_DRV,
   NCS_IFSV_EVT_ORIGN_MAX
} NCS_IFSV_EVT_ORIGN;

typedef enum ncs_ifsv_owner
{
   NCS_IFSV_OWNER_IFND,
   NCS_IFSV_OWNER_IFD,
   NCS_IFSV_OWNER_MAX
}NCS_IFSV_OWNER;

/*****************************************************************************
 * Data structure which holds the total interface record data information,
 * which constist of the common interface info and the interface specific info
 * like IP info, Tunnel info etc.
 *****************************************************************************/
typedef struct ifsv_intf_data
{   
   NCS_IFSV_IFINDEX         if_index;
   NCS_IFSV_SPT             spt_info;  /* Slot-port-type-scope information */
   NCS_IFSV_EVT_ORIGN originator; /* IfA, IfND, IfD, IfDRV */
   MDS_DEST  originator_mds_destination;
   NCS_IFSV_OWNER current_owner; /* IfND or IfD */
   MDS_DEST  current_owner_mds_destination;
   NCS_BOOL                 active;   /* TRUE - active, FALSE - not active */
   NCS_BOOL marked_del; /* Whether the intf has been marked as del. */
   NCS_IFSV_INTF_INFO       if_info;
   NCS_IFSV_INTF_REC_TYPE   rec_type;  /* Used for tech spec data */
   NCS_BOOL last_msg;
   NCS_BOOL no_data; /* If IFD dosn't have any data, it will mark this flag 
                        TRUE. */
   NCS_IFSV_EVT_ORIGN evt_orig; /* IfA, IfND, IfD, IfDRV */
   NCSCONTEXT               mab_rec_row_hdl; /* Row handle for this record */
} IFSV_INTF_DATA;


/*****************************************************************************
 * Data structure which is used to hold all the interface records tree
 *****************************************************************************/
typedef struct ifsv_intf_rec
{
   NCS_PATRICIA_NODE     node;
   IFSV_INTF_DATA       intf_data;
}IFSV_INTF_REC;

/* TBD:RSR Visit this latter when IPXS comes into picture */
/*****************************************************************************
 * Data structure which is used to register the callback 
 * (add/delete/modify record) for all the protocol supported with respect to 
 * the interace record type.
 *****************************************************************************/
typedef struct ifsv_nw_layer_handler
{
   IFSV_NW_LAYER_ADD    add;
   IFSV_NW_LAYER_DEL    del;
   IFSV_NW_LAYER_MODIFY modify;
} IFSV_NW_LAYER_HANDLER;


/*****************************************************************************
 * Data structure used to store the shelf/slot/port/type/scope->Ifindex mapping 
 * tree.
 *****************************************************************************/
typedef struct ifsv_spt_info
{
   NCS_PATRICIA_NODE     pat_node;
   NCS_IFSV_SPT_MAP      spt_map;
   IFSV_TMR              tmr;
}IFSV_SPT_REC;


/* Send info used for Sync Requests */
typedef struct ifsv_send_info {
   MDS_SVC_ID        to_svc;  /* The service at the destination */
   MDS_DEST          dest;    /* Who to send */
   MDS_SENDTYPES     stype;   /* Send type */
   MDS_SYNC_SND_CTXT ctxt;    /* MDS Opaque context */
} IFSV_SEND_INFO;


/*****************************************************************************
 * Data structure used to store Interface data at IfND during the period of 
 * resolving the ifindex.
 *****************************************************************************/
typedef struct ifsv_intf_rec_work_queue
{
   NCS_DB_LINK_LIST_NODE   q_node;
   IFSV_INTF_DATA          if_rec_data;
   IFSV_SEND_INFO          sinfo;   /* Sender information */
} IFSV_IFINDEX_RESV_WORK_QUEUE;


/*****************************************************************************
 * Data structure used to store Events received during switchover - NOTE: At 
 * present we are not using this data structure - TBD.
 *****************************************************************************/
/* forward decleration */
struct ifsv_evt;
typedef struct ifsv_evt_work_queue
{
   NCS_DB_LINK_LIST_NODE   q_node;
   struct ifsv_evt         *evt;
} IFSV_EVT_WORK_QUEUE;

/* Enums used for copy of internal pointers of NCS_IFSV_INTF_INFO */
typedef enum ifsv_malloc_use_type
{
   IFSV_MALLOC_FOR_MDS_SEND = 1,
   IFSV_MALLOC_FOR_INTERNAL_USE
}IFSV_MALLOC_USE_TYPE;

/*****************************************************************************
 * Data Structures to support BIND MIB SET operation 
 * 
 *****************************************************************************/
typedef struct ifsv_bind_info
{
   uns32  bind_port_no;
   uns32  master_ifindex;
   uns32  slave_ifindex;
   uns32  bind_ifindex;
   uns32  status;
} IFSV_BIND_INFO;



typedef struct ifsv_bind_node
{
   NCS_PATRICIA_NODE    pat_node;
   IFSV_BIND_INFO         bind_info;
}IFSV_BIND_NODE;



/*****************************************************************************
 function prototypes related to IFSV base access
 *****************************************************************************/

/* This function finds the interface record, for the given ifindex, whenever any 
   module is using this function and getting the interface record and trying
   to operate on it than it should take a interface db semaphore, b/c this 
   function will return the interface record pointer which it is having in 
   the tree.
*/
IFSV_INTF_DATA * ifsv_intf_rec_find (uns32 ifindex, IFSV_CB *cb);

/* This function Adds the interface record to the interface data base, here the user
   need not allocate 
 */
uns32 ifsv_intf_rec_add (IFSV_INTF_DATA *i_rec_data, IFSV_CB *cb);

/* This function is used to modify the interface record for the given attribute */
uns32 ifsv_intf_rec_modify (IFSV_INTF_DATA *actual_data, IFSV_INTF_DATA *mod_data, IFSV_INTF_ATTR *attr, IFSV_CB *cb);

/* This function is used to modify/delete the interface record for the given attribute */
uns32 ifsv_intf_rec_marked_del (IFSV_INTF_DATA *actual_data, IFSV_INTF_ATTR *attr, IFSV_CB *cb);

/* This function is used to find if_index for the given slot/port/type/scope */

/* This function is used to find if_index for the given slot/port/type/scope */
uns32 ifsv_get_ifindex_from_spt (uns32 *o_ifindex, NCS_IFSV_SPT  spt, IFSV_CB *cb);

/* This function is used to add shelf/slot/port/type/scope Vs ifindex mapping */
uns32 ifsv_ifindex_spt_map_add (NCS_IFSV_SPT_MAP *spt, IFSV_CB *cb);

uns32 ifsv_get_spt_from_ifindex (uns32 ifindex, NCS_IFSV_SPT  *o_spt, IFSV_CB *cb);

/* This is the function used to delete a single interface, which can be called when
   the operation status of the interface is DOWN 
 */
IFSV_INTF_REC *ifsv_intf_rec_del (uns32 ifindex, IFSV_CB *cb);

/* This is the function used to delete the shelf/slot/port/type/scope Vs ifindex mapping */
uns32 ifsv_ifindex_spt_map_del (NCS_IFSV_SPT spt, IFSV_CB *cb);

/* This is the function used to resolve the if_index for the given slot/port/type/scope 
   This function will send an Event to the IFD (in the case of IFND). In the case
   of IFD it resolves the ifindex synchronously*/
uns32 ifsv_ifnd_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map,IFSV_CB *ifsv_cb, NCSCONTEXT *evt);
uns32 ifsv_ifd_ifindex_alloc (NCS_IFSV_SPT_MAP *spt_map,IFSV_CB *ifsv_cb);

/* finds the timer pointer which needs to be stoped or started */
IFSV_TMR * ifsv_cleanup_tmr_find (NCS_IFSV_SPT  *spt, NCS_PATRICIA_TREE *p_tree);

/* delete all the shelf/slot/port/type/scope Vs ifindex maping */
uns32 ifsv_ifindex_spt_map_del_all (IFSV_CB *cb);

/* send RMS message */
uns32
ifsv_rms_if_rec_send (IFSV_INTF_DATA *intf_data, IFSV_INTF_REC_EVT rec_evt,
                      uns32 attr, IFSV_CB *cb);

/* This is the function used to free the interface index which is allocated */
uns32 ifsv_ifindex_free (IFSV_CB *ifsv_cb, NCS_IFSV_SPT_MAP *spt_map);

/* Copy the interface information & its internal pointers */
uns32 ifsv_intf_info_cpy(NCS_IFSV_INTF_INFO *src, 
                         NCS_IFSV_INTF_INFO *dest, 
                         IFSV_MALLOC_USE_TYPE purpose);

/* To copy the SVD list from src intf_info to dest intf_info */
uns32 ifsv_intf_info_svd_list_cpy(NCS_IFSV_INTF_INFO *src, 
                                  NCS_IFSV_INTF_INFO *dest, 
                                  IFSV_MALLOC_USE_TYPE purpose);

uns32 ifsv_modify_svcdest_list(NCS_IFSV_SVC_DEST_UPD *svcd, IFSV_INTF_DATA *dest, IFSV_CB *ifsv_cb);

/* Free the internal pointers of the interface info */
void ifsv_intf_info_free(NCS_IFSV_INTF_INFO *intf_info, IFSV_MALLOC_USE_TYPE purpose);

#if(NCS_IFSV_BOND_INTF == 1)
uns32 ifsv_bonding_assign_bonding_ifindex_to_master(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bondingifindex);
uns32 ifsv_binding_check_ifindex_is_slave(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX ifindex,NCS_IFSV_IFINDEX *bonding_ifindex);

#endif



#endif
