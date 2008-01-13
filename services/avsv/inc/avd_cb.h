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
..............................................................................



..............................................................................

  DESCRIPTION:

  This module is the include file for Availability Director's control block.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_CB_H
#define AVD_CB_H

typedef enum {
   AVD_INIT_BGN,
   AVD_CFG_READY,
   AVD_CFG_DONE,
   AVD_ROLE_CHG_RDY,
   AVD_RESTART,
   AVD_INIT_DONE,
   AVD_APP_STATE,
   AVD_INIT_STATE_MAX
} AVD_INIT_STATE;

/*
 * Sync state of the Standby.
 */
typedef enum {
   AVD_STBY_OUT_OF_SYNC,
   AVD_STBY_IN_SYNC,
   AVD_STBY_SYNC_STATE_MAX
} AVD_STBY_SYNC_STATE;

/*
 * Message Queue for holding the AVND messages
 * which we are going to send after processing the
 * entire event.
 */
typedef struct avsv_nd_msg_queue
{
   NCSMDS_INFO     snd_msg;
   struct avsv_nd_msg_queue  *next;
}AVSV_ND_MSG_QUEUE;

typedef struct avsv_nd_msg_list
{
   AVSV_ND_MSG_QUEUE    *nd_msg_queue;
   AVSV_ND_MSG_QUEUE    *tail;
}AVSV_ND_MSG_LIST;


/*
 * Message Queue for holding the AVD events
 * during fail over.
 */
typedef struct avd_evt_queue
{
  struct avd_evt_tag  *evt;
   struct avd_evt_queue  *next;
}AVD_EVT_QUEUE;

typedef struct avd_evt_queue_list
{
   AVD_EVT_QUEUE    *evt_msg_queue;
   AVD_EVT_QUEUE    *tail;
}AVD_EVT_QUEUE_LIST;


/* Cluster Control Block (Cl_CB): This data structure lives
 * in the AvD and is the anchor structure for all internal data structures.
 */
typedef  struct  cl_cb_tag {

   SYSF_MBX             avd_mbx;                /* mailbox on which AvD waits */
   NCS_SEL_OBJ_SET      sel_obj_set;            /* The set of selection objects
                                                 * on which the AvD does select
                                                 */
   NCS_SEL_OBJ          sel_high;               /*the highest selection object*/

   /* for HB thread */
   SYSF_MBX             avd_hb_mbx;             /* mailbox on which AvD waits */
   NCS_SEL_OBJ_SET      hb_sel_obj_set;         /* The set of selection objects
                                                 * on which the AvD does select
                                                 */
   NCS_SEL_OBJ          hb_sel_high;            /*the highest selection object*/
   uns32                cb_handle;              /*the control block handle of AVD
                                                 */
   NCSCONTEXT           mds_handle;             /* The handle returned by MDS 
                                                 * when initialized
                                                 */
   NCS_LOCK             lock;                   /* the control block lock */
   NCS_LOCK             avnd_tbl_lock;          /* the control block lock,
                                                   will be taken while deleting
                                                   or adding avnd struct */

   AVD_INIT_STATE       init_state;             /* when the AvD is comming up for
                                                 * the first time it will come up
                                                 * as down in this mode the HA
                                                 * state for the AvD will be
                                                 * determined.
                                                 * Checkpointing - Sent as a one time update.
                                                 */

   NCS_BOOL             avd_fover_state;        /* TRUE if avd is trying to 
                                                 * recover from the fail-over */

   NCS_BOOL             role_set;               /* TRUE - Initial role is set.
                                                 * FALSE - Initial role is not set */
   NCS_BOOL             role_switch;             /* TRUE - In middle of role switch. */

   SaAmfHAStateT        avail_state_avd;        /* the redundancy state for 
                                                 * Availability director
                                                 */
   SaAmfHAStateT        avail_state_avd_other;  /* The redundancy state for 
                                                 * Availability director (other).
                                                 */

   MDS_HDL             vaddr_pwe_hdl;          /* The pwe handle returned when
                                                 * vdest address is created.
                                                 */
   MDS_HDL             vaddr_hdl;              /* The handle returned by mds
                                                 * for vaddr creation
                                                 */
   MDS_HDL             adest_hdl;              /* The handle returned by mds
                                                 * for adest creation
                                                 */
   MDS_DEST             vaddr;                  /* vcard address of the this AvD
                                                 */
   MDS_DEST             other_avd_adest;         /* ADEST of  other AvD
                                                 */
   V_DEST_QA            vcard_anchor;           /* the anchor value that 
                                                 * differentiates the primary 
                                                 * and standby vcards of AvD.
                                                 */
   V_DEST_QA            other_vcard_anchor;     /*  The anchor value of the other
                                                 * AvD.
                                                 */

   /*
    * Message queue to hold messages to be sent to the ND.
    * This is a FIFO queue.
    */
   AVSV_ND_MSG_LIST    nd_msg_queue_list;

   /* Event Queue to hold the events during fail-over */
   AVD_EVT_QUEUE_LIST       evt_queue;
   /* 
    * MBCSv related variables.
    */
   NCS_MBCSV_HDL            mbcsv_hdl;
   uns32                    ckpt_hdl;
   SaSelectionObjectT       mbcsv_sel_obj;
   SaVersionT               avsv_ver;
   AVD_STBY_SYNC_STATE      stby_sync_state;

   uns32                    synced_reo_type; /* Count till which sync is done */
   AVSV_ASYNC_UPDT_CNT      async_updt_cnt; /* Update counts for different async updates */
   NCS_BOOL                 sync_required ; /* if TRUE, we need to send SYNC send to the standby 
                                               after mailbox processing */

   /* Queue for keeping async update messages  on Standby*/
   AVSV_ASYNC_UPDT_MSG_QUEUE_LIST async_updt_msgs;

   EDU_HDL              edu_hdl;                /* EDU handle */
   uns32                mab_hdl;                /* MAB handle returned during initilisation. */
   uns32                mab_row_hdl_list[(NCSMIB_TBL_AVSV_AVD_END - NCSMIB_TBL_AVSV_BASE) + 1];
                                                /* The list of row handles returned
                                                 * by MAB. Only active will
                                                 * have them. */
   uns32                hpi_hdl;                /* HPI handle returned during initilisation. */ 
   SaEvtHandleT         *eds_hdl;               /* The EDS handle returned during 
                                                 * initilisation. 
                                                 */ 
   SaSelectionObjectT   *eds_sel_obj;           /* The selection object returned by EDS. */

   SaTimeT              cluster_init_time;      /* The time when the firstnode joined the cluster.
                                                 * Checkpointing - Sent as a one time update.
                                                 */

   uns32                cluster_num_nodes;      /* The number of member nodes in the cluster
                                                 * Checkpointing - Calculated at standby
                                                 */
                                                 

   SaUint64T            cluster_view_number;    /* the current cluster membership view number 
                                                 * Checkpointing - Sent as an independent update.
                                                 */
   SaClmNodeIdT         node_id_avd;            /* node id on which this AvD is  running */
   SaClmNodeIdT         node_id_avd_other;      /* node id on which the other AvD is
                                                 * running 
                                                 */
   SaClmNodeIdT         node_avd_failed;        /* node id where AVD is down */

   SaTimeT              rcv_hb_intvl;           /* The interval within which
                                                 * atleast one hearbeat should
                                                 * be received by the AvD from 
                                                 * the corresponding nodes and
                                                 * the other AvD.
                                                 * Checkpointing - Sent as a one time update.
                                                 */

   SaTimeT              snd_hb_intvl;           /* The interval at which
                                                 * the corresponding nodes and
                                                 * the other AvD should 
                                                 * heartbeat with this AvD.
                                                 * Checkpointing - Sent as a one time update.
                                                 */

   SaTimeT              amf_init_intvl;         /* This is the amount of time
                                                 * avsv will wait before
                                                 * assigning SIs in the SG.
                                                 * Checkpointing - Sent as a one time update.
                                                 */
   NCS_PATRICIA_TREE    node_list;              /* Tree of AvND nodes indexed by
                                                 * node id. used for storing the 
                                                 * nodes on f-over.
                                                 */
   NCS_PATRICIA_TREE    avnd_anchor;            /* Tree of AvND nodes indexed by
                                                 * node id.
                                                 */
   NCS_PATRICIA_TREE    avnd_anchor_name;       /* Tree of AvND nodes indexed by
                                                 * node name with length in
                                                 * network order.
                                                 */
   NCS_PATRICIA_TREE    sg_anchor;              /* Tree of service groups */
   NCS_PATRICIA_TREE    su_anchor;              /* Tree of Service units */
   NCS_PATRICIA_TREE    si_anchor;              /* Tree of Service instances */
   NCS_PATRICIA_TREE    comp_anchor;            /* Tree of components */
   NCS_PATRICIA_TREE    csi_anchor;             /* Tree of component service instances */
   NCS_PATRICIA_TREE    hlt_anchor;             /* Tree of health check records */
   NCS_PATRICIA_TREE    su_per_si_rank_anchor;  /* Tree of su per si ranks */
   NCS_PATRICIA_TREE    sg_si_rank_anchor;      /* Tree of SG-SI rank */
   NCS_PATRICIA_TREE    sg_su_rank_anchor;      /* Tree of SG-SU rank */
   NCS_PATRICIA_TREE    comp_cs_type_anchor;    /* Tree of comp csi type */
   NCS_PATRICIA_TREE    cs_type_param_anchor;   /* Tree of csi type param */
   AVD_TMR              heartbeat_send_avd;     /* The timer for sending the heartbeat */
   AVD_TMR              heartbeat_rcv_avd;      /* The timer for receiving the heartbeat */
   AVD_TMR              amf_init_tmr;           /* The timer for amf initialisation.*/
   NCS_ADMIN_STATE      cluster_admin_state;
   uns32                nodes_exit_cnt;         /* The counter to identifies the number
                                                   of nodes that have exited the membership
                                                   since the cluster boot time */

   uns32                num_cfg_msgs;           /* Number of cfg messages received */
   union {
      AVD_TMR           cfg_tmr;                /* Timer waiting for conf msgs */
   }init_phase_tmr;
  MDS_DEST              bam_mds_dest; 
  MDS_DEST              avm_mds_dest;

  /********** EDA related params      ***********/

   SaVersionT  safVersion; /* version info of the SAF implementation */

   SaEvtHandleT     evtHandle;   /* EDA Handle */

   SaNameT          publisherName; /* Publisher name */

   SaNameT             evtChannelName;   /* EDA Channel Name */

   SaEvtChannelHandleT evtChannelHandle; /* EDA Channel Handle */
 
} AVD_CL_CB;



#define AVD_CL_CB_NULL ((AVD_CL_CB *)0)


/* macro to push the ND msg in the queue (to the end of the list) */
#define m_AVD_DTOND_MSG_PUSH(cb, msg) \
{ \
   AVSV_ND_MSG_LIST *list = &((cb)->nd_msg_queue_list); \
   if (!(list->nd_msg_queue)) \
       list->nd_msg_queue = (msg); \
   else \
      list->tail->next = (msg); \
   list->tail = (msg); \
}

/* macro to pop the msg (from the beginning of the list) */
#define m_AVD_DTOND_MSG_POP(cb, msg) \
{ \
   AVSV_ND_MSG_LIST *list = &((cb)->nd_msg_queue_list); \
   if (list->nd_msg_queue) { \
      (msg) = list->nd_msg_queue; \
      list->nd_msg_queue = (msg)->next; \
      (msg)->next = 0; \
      if (list->tail == (msg)) list->tail = 0; \
   } else (msg) = 0; \
}


/* macro to enqueue the AVD events in the queue (to the end of the list) */
#define m_AVD_EVT_QUEUE_ENQUEUE(cb, evt) \
{ \
   AVD_EVT_QUEUE_LIST *list = &((cb)->evt_queue); \
   if (!(list->evt_msg_queue)) \
       list->evt_msg_queue = (evt); \
   else \
      list->tail->next = (evt); \
   list->tail = (evt); \
}

/* macro to dequeue the msg (from the beginning of the list) */
#define m_AVD_EVT_QUEUE_DEQUEUE(cb, evt) \
{ \
   AVD_EVT_QUEUE_LIST *list = &((cb)->evt_queue); \
   if (list->evt_msg_queue) { \
      (evt) = list->evt_msg_queue; \
      list->evt_msg_queue = (evt)->next; \
      (evt)->next = 0; \
      if (list->tail == (evt)) list->tail = 0; \
   } else (evt) = 0; \
}


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                         Locking Macros

AVD is designed such that all the writes happen only in the event processing
loop. So the event processing loop doesnt take any read locks It only
takes write locks before doing any modification. All the other threads
take read locks if necessary.
 
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

               /* CB Lock */

#define m_AVD_CB_LOCK_INIT(avd_cb)  m_NCS_LOCK_INIT(&avd_cb->lock)
#define m_AVD_CB_LOCK_DESTROY(avd_cb) \
{\
   m_AVD_LOG_LOCK_SUCC(AVSV_LOG_LOCK_FINALIZE);\
   m_NCS_LOCK_DESTROY(&avd_cb->lock);\
}
#define m_AVD_CB_LOCK(avd_cb, ltype) \
{\
   m_AVD_LOG_LOCK_SUCC(AVSV_LOG_LOCK_TAKE);\
   m_NCS_LOCK(&avd_cb->lock, ltype);\
}
#define m_AVD_CB_UNLOCK(avd_cb, ltype) \
{\
    m_AVD_LOG_LOCK_SUCC(AVSV_LOG_LOCK_GIVE);\
    m_NCS_UNLOCK(&avd_cb->lock, ltype);\
}

/* Below given lock will be taken by the main mailbox thread before deleting
   or adding avnd structure to patritia tree. We can avoid these, if we have 
   read locks  */
#define m_AVD_CB_AVND_TBL_LOCK_INIT(avd_cb)  m_NCS_LOCK_INIT(&avd_cb->avnd_tbl_lock)
#define m_AVD_CB_AVND_TBL_LOCK_DESTROY(avd_cb) \
{\
   m_AVD_LOG_LOCK_SUCC(AVSV_LOG_LOCK_FINALIZE);\
   m_NCS_LOCK_DESTROY(&avd_cb->avnd_tbl_lock);\
}
#define m_AVD_CB_AVND_TBL_LOCK(avd_cb, ltype) \
{\
   m_AVD_LOG_LOCK_SUCC(AVSV_LOG_LOCK_TAKE);\
   m_NCS_LOCK(&avd_cb->avnd_tbl_lock, ltype);\
}
#define m_AVD_CB_AVND_TBL_UNLOCK(avd_cb, ltype) \
{\
    m_AVD_LOG_LOCK_SUCC(AVSV_LOG_LOCK_GIVE);\
    m_NCS_UNLOCK(&avd_cb->avnd_tbl_lock, ltype);\
}

struct avd_evt_tag;

EXTERN_C void avd_init_cmplt(AVD_CL_CB *cb);
EXTERN_C uns32 avd_evd_init (AVD_CL_CB *cb);
EXTERN_C uns32 avd_hpi_init (AVD_CL_CB *cb);
EXTERN_C uns32 avd_mab_init (AVD_CL_CB *cb);
EXTERN_C uns32 avd_mab_reg_rows(AVD_CL_CB *cb);
EXTERN_C uns32 avd_mab_unreg_rows(AVD_CL_CB *cb);
EXTERN_C uns32 avd_miblib_init (AVD_CL_CB *cb);
EXTERN_C void avd_tmr_cl_init_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C uns32 avd_mab_snd_warmboot_req(AVD_CL_CB *cb);

#endif
