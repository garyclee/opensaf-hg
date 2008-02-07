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

#ifndef IFSV_CB_H
#define IFSV_CB_H

#include "ifsv_tmr.h"
#include "vip.h"

#define IFSV_NULL    NULL

/**** Macro to store/get the mbx/task/CB handles ****/
#define m_IFSV_GET_MBX_HDL(vrid,comp_type) gl_ifsvlib.ifsv_mbx
#define m_IFSV_GET_TASK_HDL(vrid,comp_type) gl_ifsvlib.ifsv_task_hdl
#define m_IFSV_STORE_HDL(vrid1,comp_type,hdl) gl_ifsv_hdl=hdl;gl_ifsvlib.vrid=vrid1
#define m_IFSV_GET_HDL(vrid,comp_type) gl_ifsv_hdl
#define m_IFSV_GET_CB(cb,compName) cb=ncshm_take_hdl((strcmp((uns8*)compName,m_IFD_COMP_NAME) == 0?NCS_SERVICE_ID_IFD:NCS_SERVICE_ID_IFND), gl_ifsv_hdl)
#define m_IFSV_GIVE_CB(cb) ncshm_give_hdl(cb->cb_hdl);

/**** global variable ******/
EXTERN_C IFSVDLL_API uns32 gl_ifsv_hdl;
EXTERN_C uns32 gl_ifd_cb_hdl;
EXTERN_C uns32 gl_ifnd_cb_hdl;
/* The following structure will be used in deciding whether AMF response is to be
   sent to AMF after cold sync or not. */
typedef struct amf_req_in_cold_sync
{
   NCS_BOOL  amf_req;
   SaInvocationT invocation;
}AMF_REQ_IN_COLD_SYNC;

/*****************************************************************************
 * Data Structure Used to hold IfD/IfND control block
 *****************************************************************************/
typedef struct ifsv_cb
{
   /* Common parameters (IFD/IFND) */
   uns32                shelf;
   uns32                slot;       
   uns32                my_node_id;
   IFSV_COMP_TYPE       comp_type; /* component type (IFD/IFND)              */
   uns8                 comp_name[IFSV_MAX_COMP_NAME];      
   NCS_BOOL             init_done; /* Flag to indicate component initalization */
   uns8                 hm_pid;    /* Pool ID of the Handle Manager          */
   uns32                cb_hdl;    /* Handle to Access/Protect CB            */   
   uns32                oac_hdl;
   NCS_SERVICE_ID       my_svc_id; /* Service ID of the IfD/IfND             */   
   uns32                vrid;      /* virtual router ID of this instance     */   
   V_DEST_QA            my_anc;    /* My anchor  value */
   MDS_HDL              my_mds_hdl;/* MDS PWE handle   */
   MDS_DEST             my_dest;   /* Destination ID of this component instance */
   NCS_BOOL             switchover_flag;/* This flag is used to check the case where the transition between primary and backup period, this would be removed after discussion - TBD */
   SYSF_MBX             mbx;       /* Mail box of this component instance    */   
   NCS_DB_LINK_LIST     evt_que;   /* this is the event que to store the events during the switchover period, which will be requested later , needs to be removed after discussion - TBD */   
   NCS_LOCK             intf_rec_lock; /* Lock to prevent the multiple access
                                          of interface database */
   NCSCONTEXT           health_sem; /* Semaphore used for health check       */      
   NCS_PATRICIA_TREE     if_tbl;     /* Interface database                    */
   NCS_PATRICIA_TREE     if_map_tbl; /* Tree stores Shelf/Slot/Port/Type to 
                                       Ifindex Map Table                     */
#if (NCS_VIP == 1)
   NCS_LOCK             vip_rec_lock; /* Lock to prevent the multiple access
                                         of vip records */
   NCS_PATRICIA_TREE    vipDBase;   /* for IfND it acts as VIPDC, for IfD it acts as VIPD*/ 
#endif

#if (NCS_IFSV_BOND_INTF == 1)
   uns32 bondif_max_portnum;
#endif


   uns32                logmask;    /* LOG mask                              */
   NCS_IFSV_INTF_REC    ifmib_rec;  /* Global location to store if_data for MIB
                                       operations */
   SaAmfHandleT         amf_hdl;       /* AMF handle, which we would have got during AMF init       */   
/* TBD Remove this
   SaAmfReadinessStateT ready_state; */   
   
   /* EDU handle, for performing encode/decode operations. */
   EDU_HDL              edu_hdl;

   /* IFD Parameters */
   SaAmfHAStateT        ha_state;      /* Present AMF HA state of the component*/   
   uns32                if_number;  /* Number of network interfaces present in
                                       the System (MIB variable)             */
   time_t               iftbl_lst_chg; /* Time stamp of the last creation or
                                          deletion of an entry in the ifTable*/      
   IFSV_TMR tmr; /* Timer to wait until all the MDS_DEST info is updated at
                    the IfND, while it is rebooting. */
   /* Parameters for IfND redundancy. */   
   IFND_RESTART_STATE   ifnd_state;   

   /* IfND Parameters */
   NCS_BOOL             ifd_card_up;   /* check whether IfD is UP            */   
   MDS_DEST             ifd_dest;      /* destination reach IfD              */
   SYSF_MBX             idim_mbx;      /* IDIM Mail box handle               */
   NCSCONTEXT           idim_task;     /* IDIM Task handle                   */
   NCS_BOOL             idim_up;    /* Whether IDIM is UP(TRUE)/DOWN(FALSE)  */
   uns32                idim_hdl;
   MDS_DEST             idim_dest;  /* IDIM Destination                    */   
   uns32                max_ifindex;     /* Max Interface present, this should be moved to target service - TBD */   
   NCS_DB_LINK_LIST     free_if_pool;    /* Free ifindex pool maintained here, this should be moved to the target service area. (IAPS) - TBD */

   NCS_MBCSV_HDL         mbcsv_hdl;  /* MBCSV handle */
   NCS_MBCSV_CKPT_HDL    o_ckpt_hdl; /* MBCSv binding value */
   SaSelectionObjectT    mbcsv_sel_obj;
   NCS_BOOL              cold_or_warm_sync_on;
   uns32                 ifd_async_cnt;
   NCS_IFSV_IFINDEX      record_ifindex;
   NCS_PATRICIA_TREE     ifnd_node_id_tbl;  /* Ifnd node id database */
   NCS_PATRICIA_TREE     mds_dest_tbl;  /* MDS Dest database */
#if (NCS_VIP == 1)
   NCS_IFSV_VIP_INT_HDL  record_handle;
#endif
   uns32                 (*ifd_mbcsv_data_resp_enc_func_ptr)(void *, NCS_MBCSV_CB_ARG *);
   uns32                 (*ifd_mbcsv_data_resp_dec_func_ptr)(void *, NCS_MBCSV_CB_ARG *);
   NCS_BOOL                   is_quisced_set;
   SaInvocationT          invocation;
   uns32                  ifnd_down;
   uns32                  ifnd_node;
   uns32                  ifd_down;
   uns32                  ifd_node;
   MDS_DEST               down_ifnd_addr;
   DOWN_IFND_ADDR         ifnd_mds_addr[MAX_IFND_NODES];
   IFSV_TMR              *ifnd_rec_flush_tmr;

   NCS_PATRICIA_TREE     ifsv_bind_mib_rec;
   AMF_REQ_IN_COLD_SYNC  amf_req_in_cold_syn;

}IFSV_CB;

#define IFSV_CB_NULL ((IFSV_CB*)0)

/*****************************************************************************
 * Data Structure Used to hold mapping between handle, vrid and mail box
 *****************************************************************************/
typedef struct ifsvlib_struct
{
   uns32       vrid;
   SYSF_MBX   ifsv_mbx;
   NCSCONTEXT ifsv_task_hdl;
} IFSVLIB_STRUCT;

/* Common IfSv function used to clean the events */
uns32 ifsv_cb_destroy (IFSV_CB *ifsv_cb);

/* The below API's should be removed after unit testing - TBD */

#endif

