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

  This module is the include file for Availability Node Directors control block.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVND_CB_H
#define AVND_CB_H


typedef struct avnd_cb_tag {
   SYSF_MBX mbx; /* mailbox on which AvND waits */

   /* mds params for communication */
   MDS_HDL     mds_hdl;   /* mds handle */
   MDS_DEST    avnd_dest; /* AvND mds addr */
   MDS_DEST    avd_dest;  /* AvD mds addr */
   NCS_BOOL    is_avd_down;  /* Temp: Indicates if AvD went down */

   /* cb related params */
   uns32     cb_hdl; /* hdl returned by hdl mngr */
   NCS_LOCK  lock;   /* cb lock */

   /* external interface related params */
   uns8               pool_id;      /* pool-id used by hdl mngr */
   EDU_HDL            edu_hdl;      /* edu handle */
   EDU_HDL            edu_hdl_ava;  /* edu handle for ava interface */
   EDU_HDL            edu_hdl_cla;  /* edu handle for cla interface */
   uns32              mab_hdl;      /* mab hdl */
   uns32              mab_node_hdl; /* mab hdl for this node in node status tbl */
   uns32              hpi_hdl;      /* hpi hdl */
   SaEvtHandleT       *eds_hdl;     /* edsv hdl */
   SaSelectionObjectT *eds_sel_obj; /* edsv selection object */
   uns32              srm_hdl;      /* srmsv handle*/
   SaSelectionObjectT srm_sel_obj; /* srmsv selection object */

   /* node states */
   NCS_ADMIN_STATE  admin_state; /* node admin state */
   NCS_OPER_STATE   oper_state;  /* node oper state  */
   uns32            flag;        /* cb flag */

   /* avnd timers */
   AVND_TMR  hb_tmr; /* AvD heartbeat timer */

   /* timeout interval values */
   SaTimeT  hb_intv;       /* AvD heartbeat interval */
   SaTimeT  cbk_resp_intv; /* callback response interval */
   SaTimeT  msg_resp_intv; /* AvD message response interval */

   /* error recovery escalation params */
   AVND_ERR_ESC_LEVEL  node_err_esc_level; /* curr escalation level of this node */
   SaTimeT             su_failover_prob;   /* su failover probation period (config) */
   uns32               su_failover_max;    /* max SU failovers (config) */
   uns32               su_failover_cnt;    /* su failover cnt within a probation period */
   AVND_TMR            node_err_esc_tmr;   /* node err esc tmr */

   /* database declarations */
   AVND_CLM_DB        clmdb;  /* clm db */
   NCS_PATRICIA_TREE  sudb;   /* su db */
   NCS_PATRICIA_TREE  compdb; /* comp db */
   NCS_PATRICIA_TREE  hcdb;   /* healthcheck db */
   NCS_PATRICIA_TREE  pgdb;   /* pg db */

   /* srmsv resource request mapping list (res mon hdl is the key) */
   NCS_DB_LINK_LIST   srm_req_list;

   MDS_DEST       active_avd_adest;
   uns32          rcv_msg_id; /* Message ID of the last message received */
   /* AvD messaging params (retransmit list etc.) */
   uns32          snd_msg_id; /* send msg id */
   AVND_DND_LIST  dnd_list;   /* list of messages sent to AvD */

   AVND_TERM_STATE term_state;
   AVND_LED_STATE  led_state;
   NCS_BOOL destroy;

   /********** EDA related params      ***********/

   SaVersionT  safVersion; /* version info of the SAF implementation */

   SaEvtHandleT     evtHandle;   /* EDA Handle */

   SaNameT          publisherName; /* Publisher name */

   SaNameT             evtChannelName;   /* EDA Channel Name */

   SaEvtChannelHandleT evtChannelHandle; /* EDA Channel Handle */

} AVND_CB;

#define AVND_CB_NULL ((AVND_CB *)0)

/* AvD state values */
#define AVND_CB_FLAG_AVD_UP     0x00000001

/* macros for AvD state values */
#define m_AVND_CB_IS_AVD_UP(x)    (((x)->flag) & AVND_CB_FLAG_AVD_UP)
#define m_AVND_CB_AVD_UP_SET(x)   (((x)->flag) |= AVND_CB_FLAG_AVD_UP)
#define m_AVND_CB_AVD_UP_RESET(x) (((x)->flag) &= ~AVND_CB_FLAG_AVD_UP)


/* 
 * macro to determine if AvND should heartbeat with AvD. 
 * Note that on scxb, avd & avnd reside in the same process.
 */
#define m_AVND_IS_AVD_HB_ON(cb)  (cb->clmdb.type != AVSV_AVND_CARD_SYS_CON)

/* 
 * Macros for managing the error escalation levels 
 */
#define m_AVND_NODE_ERR_ESC_LEVEL_IS_NONE(x)  \
           ((x)->node_err_esc_level == AVND_ERR_ESC_LEVEL_NONE)

#define m_AVND_NODE_ERR_ESC_LEVEL_IS_3(x)  \
           ((x)->node_err_esc_level == AVND_ERR_ESC_LEVEL_3)

#define m_AVND_NODE_ERR_ESC_LEVEL_SET(x, val)  ((x)->node_err_esc_level = (val))


EXTERN_C uns32 ncssndtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 ncssndtableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 ncssndtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncssndtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 ncssndtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
EXTERN_C uns32 ncssndtableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);

EXTERN_C void avnd_dnd_list_destroy(AVND_CB *);

EXTERN_C uns32 avnd_cb_destroy (AVND_CB *);

#endif
