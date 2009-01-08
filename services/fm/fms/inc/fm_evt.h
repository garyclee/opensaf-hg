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

  $$


  MODULE NAME: FM
  AUTHOR: vishal soni (vishal.soni@emerson.com)

..............................................................................

  DESCRIPTION:



******************************************************************************/
#ifndef FM_EVT_H
#define FM_EVT_H

/* EVT from other GFM over MDS.*/
typedef enum {
    GFM_GFM_EVT_NODE_RESET_IND,
    GFM_GFM_EVT_MAX
}GFM_GFM_MSG_TYPE;

typedef struct node_reset_info{
    uns8        shelf;
    uns8        slot;
    uns8        sub_slot;
}NODE_RESET_INFO;

typedef struct gfm_gfm_msg {
    /* TODO: fill for FMS to FMS communication */
    GFM_GFM_MSG_TYPE msg_type;

    union {
        NODE_RESET_INFO     node_reset_ind_info;
    }info;

}GFM_GFM_MSG;

/* FM generated events.*/
typedef enum {
    FM_EVT_TMR_EXP,
    FM_EVT_FMA_START,
    FM_EVT_HB_LOSS = FM_EVT_FMA_START,
    FM_EVT_HB_RESTORE,
    FM_EVT_CAN_SMH_SW,
    FM_EVT_AVM_NODE_RESET_IND,
    FM_FSM_EVT_MAX
}FM_FSM_EVT_CODE;


typedef struct fm_evt {
    struct fm_evt        *next;   
    uns8                 slot;
    uns8                 sub_slot;
    FM_FSM_EVT_CODE      evt_code;   
    NCS_IPC_PRIORITY     priority;
    MDS_SYNC_SND_CTXT    mds_ctxt;
    MDS_DEST             fr_dest;
    union{
        FM_TMR           *fm_tmr;
        GFM_GFM_MSG      gfm_msg;
    } info;
} FM_EVT;


EXTERN_C void 
fm_mbx_evt_handler(FM_CB *fm_cb, FM_EVT *fm_evt); 

#endif


