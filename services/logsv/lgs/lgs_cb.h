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
 * Author(s): Ericsson AB
 *
 */

#ifndef LGS_CB_H
#define LGS_CB_H

#include <saLog.h>
#include "lgs_stream.h"

/* Default HA state assigned locally during lgs initialization */
#define LGS_HA_INIT_STATE 0 

/* CHECKPOINT status */
typedef enum checkpoint_status
{
    COLD_SYNC_IDLE = 0,
    REG_REC_SENT,
    CHANNEL_REC_SENT,
    SUBSCRIPTION_REC_SENT,
    COLD_SYNC_COMPLETE,
    WARM_SYNC_IDLE,
    WARM_SYNC_CSUM_SENT,
    WARM_SYNC_COMPLETE,
} CHECKPOINT_STATE;

typedef struct lgs_stream_list
{
   uns32                  stream_id;   
   struct lgs_stream_list *next;
} lgs_stream_list_t;

typedef struct lga_reg_list
{
    NCS_PATRICIA_NODE       pat_node;
    uns32                   reg_id;
    uns32                   reg_id_Net;
    MDS_DEST                lga_client_dest;
    lgs_stream_list_t       *first_stream;
    lgs_stream_list_t       *last_stream;
} lga_reg_rec_t;

typedef struct lgs_cb
{
   SYSF_MBX mbx;                      /* LGS's mailbox                             */
   MDS_HDL mds_hdl;                   /* PWE Handle for interacting with LGAs      */
   V_DEST_RL mds_role;                /* Current MDS role - ACTIVE/STANDBY         */
   MDS_DEST vaddr;                    /* My identification in MDS                  */
   SaVersionT log_version;            /* The version currently supported           */
   log_stream_t *alarmStream;          /* Alarm log stream */
   log_stream_t *notificationStream;   /* Notification log stream */
   log_stream_t *systemStream;         /* System log stream */
   NCS_PATRICIA_TREE lga_reg_list;    /* LGA Library instantiation list            */
   SaNameT comp_name;                  /* Components's name LGS                     */
   SaAmfHandleT amf_hdl;               /* AMF handle, obtained thru AMF init        */ 
   SaInvocationT amf_invocation_id;    /* AMF InvocationID - needed to handle Quiesed state */
   NCS_BOOL is_quisced_set; 
   SaSelectionObjectT amfSelectionObject; /*Selection Object to wait for amf events */
   SaAmfHAStateT ha_state;                /* present AMF HA state of the component     */
   uns32 last_reg_id;                     /* Value of last reg_id assigned             */
   uns32 async_upd_cnt;                   /* Async Update Count for Warmsync */
   CHECKPOINT_STATE ckpt_state;           /* Current record that has been checkpointed */
   NCS_MBCSV_HDL mbcsv_hdl;               /* Handle obtained during mbcsv init */
   SaSelectionObjectT mbcsv_sel_obj;      /* Selection object to wait for MBCSv events */
   NCS_MBCSV_CKPT_HDL mbcsv_ckpt_hdl;     /* MBCSv handle obtained during checkpoint open */
   EDU_HDL edu_hdl;                       /* Handle from EDU for encode/decode operations */
   NCS_BOOL csi_assigned;
} lgs_cb_t;

extern uns32 lgs_cb_init(lgs_cb_t *);
extern uns32 lgs_add_reglist_entry (lgs_cb_t *, MDS_DEST, uns32, lgs_stream_list_t *stream_list);
extern NCS_BOOL lgs_lga_entry_valid(lgs_cb_t *, MDS_DEST);
extern void lgs_process_mbx(SYSF_MBX *mbx);
extern uns32 lgs_stream_add(lgs_cb_t *cb, log_stream_t* stream);
SaAisErrorT lgs_app_stream_store(uns32 reg_id, log_stream_t *stream);

#endif
