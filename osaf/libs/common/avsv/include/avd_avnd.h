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

  This module is the include file for handling Availability Directors 
  Node Director structure.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_AVND_H
#define AVD_AVND_H

struct avd_su_tag;

typedef enum {

	AVD_AVND_STATE_ABSENT,
	AVD_AVND_STATE_NO_CONFIG,
	AVD_AVND_STATE_PRESENT,
	AVD_AVND_STATE_GO_DOWN,
	AVD_AVND_STATE_SHUTTING_DOWN,
	AVD_AVND_STATE_NCS_INIT
} AVD_AVND_STATE;

/* Fail-over Node List */
typedef struct avd_fail_over_node {

	NCS_PATRICIA_NODE tree_node_id_node;
	SaClmNodeIdT node_id;
} AVD_FAIL_OVER_NODE;

/* Availability directors Node Director structure(AVD_AVND): 
 * This data structure lives in the AvD and reflects data points
 * associated with the AvD's remote, distributed AvNDs. In addition
 * this data structure is also a home for data-points required to
 * support the cluster membership service.
 */

typedef struct avd_avnd_tag {

	NCS_PATRICIA_NODE tree_node_id_node;	/* key will be the node id */
	NCS_PATRICIA_NODE tree_node_name_node;	/* key will be the node name with len
						 * in network order.
						 */

	SaClmClusterNodeT node_info;	/* the node information of the node on
					 * which this AvND exists. The length
					 * field of nodeName structure is in
					 * network order. The nodename is
					 * used as the index.
					 * Checkpointing - Sent as a one time update.
					 */

	MDS_DEST adest;		/* the Adest address of this
				 * nodes AvND.
				 * Checkpointing - Sent on node up.
				 */

	NCS_ADMIN_STATE su_admin_state;	/* admin STATEs of all the SUs in the
					 * node.
					 * Checkpointing - Sent independent update 
					 */

	SaNameT hpi_entity_path;	/* The entity path of the HPI entity that
					   implements the CLM node */

	NCS_OPER_STATE oper_state;	/* operation state of the node 
					 * Checkpointing - Sent independent update 
					 */
	NCS_OPER_STATE avm_oper_state;	/* operation state of the node */
	NCS_ROW_STATUS row_status;	/* row status of this MIB row */

	AVD_AVND_STATE node_state;	/* F.S.M state of the AVND 
					 * Checkpointing - Sent independent update 
					 */

	struct avd_su_tag *list_of_ncs_su;	/* the list of NCS service units on
						 * this node.
						 */
	struct avd_su_tag *list_of_su;	/* the list of service units on this
					 * node that are application specific.
					 */
	NCS_DB_LINK_LIST pg_csi_list;	/* list of csis for which pg is tracked 
					 * from this node */
	AVD_TMR heartbeat_rcv_avnd;	/* The timer for receiving the heartbeat from AvND */

	AVSV_AVND_CARD type;	/* field that describes if this node is sytem
				 * controller or not.
				 * Checkpointing - Sent as a one time update.
				 */

	SaTimeT su_failover_prob;	/* SU failover probation period 
					 * Checkpointing - Sent as a one time update.
					 */

	uns32 su_failover_max;	/* max SU failover count 
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 rcv_msg_id;	/* The receive message id counter 
				 * Checkpointing - Sent independent update 
				 */

	uns32 snd_msg_id;	/* The send message id counter 
				 * Checkpointing - Sent independent update 
				 */
	AVD_HLT *list_of_hlt;	/* list oof health check for this node */

	NCS_BOOL hrt_beat_rcvd;	/* Boolean Showing First heart beat recevied from AvND */

} AVD_AVND;

#define AVD_AVND_NULL     ((AVD_AVND *)0)

#define m_AVD_SET_AVND_SU_ADMIN(cb,node,state) {\
node->su_admin_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, node, AVSV_CKPT_AVND_SU_ADMIN_STATE);\
avd_gen_node_admin_state_changed_ntf(cb,node);\
}

#define m_AVD_SET_AVND_OPER(cb,node,state) {\
node->oper_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, node, AVSV_CKPT_AVND_OPER_STATE);\
}

#define m_AVD_SET_AVND_RCV_ID(cb,node,rcvid) {\
node->rcv_msg_id = rcvid;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, node, AVSV_CKPT_AVND_RCV_MSG_ID);\
}

#define m_AVD_IS_NODE_LOCK(node,flag)\
{\
   AVD_SU *i_su;\
   AVD_SU_SI_REL *curr_susi = 0; \
   flag = TRUE;\
   i_su = node->list_of_su;\
   while ((i_su != AVD_SU_NULL) && (flag == TRUE))\
   {\
      if ((i_su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SU_OPER) ||\
          (i_su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SG_REALIGN)) { \
         for (curr_susi = i_su->list_of_susi; \
              (curr_susi) && ((SA_AMF_HA_QUIESCING != curr_susi->state) ||\
              ((AVD_SU_SI_STATE_UNASGN == curr_susi->fsm))); \
              curr_susi = curr_susi->su_next); \
         if (curr_susi) flag = FALSE; \
      } \
      i_su = i_su->avnd_list_su_next;\
   }\
}

EXTERN_C AVD_AVND *avd_avnd_struc_crt(AVD_CL_CB *cb, SaNameT node_name, NCS_BOOL ckpt);
EXTERN_C uns32 avd_avnd_struc_add_nodeid(AVD_CL_CB *cb, AVD_AVND *avnd);
EXTERN_C AVD_AVND *avd_avnd_struc_find(AVD_CL_CB *cb, SaNameT node_name);
EXTERN_C AVD_AVND *avd_avnd_struc_find_nodeid(AVD_CL_CB *cb, SaClmNodeIdT node_id);
EXTERN_C AVD_AVND *avd_avnd_struc_find_next(AVD_CL_CB *cb, SaNameT node_name);
EXTERN_C AVD_AVND *avd_avnd_struc_find_next_nodeid(AVD_CL_CB *cb, SaClmNodeIdT node_id);
EXTERN_C uns32 avd_avnd_struc_del(AVD_CL_CB *cb, AVD_AVND *avnd);
EXTERN_C uns32 avd_avnd_struc_rmv_nodeid(AVD_CL_CB *cb, AVD_AVND *avnd);

EXTERN_C uns32 ncsndtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 ncsndtableentry_extract(NCSMIB_PARAM_VAL *param,
				       NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 ncsndtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncsndtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				    NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 ncsndtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				      NCSMIB_SETROW_PARAM_VAL *params,
				      struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 ncsndtableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);
EXTERN_C uns32 saamfnodetableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 saamfnodetableentry_extract(NCSMIB_PARAM_VAL *param,
					   NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 saamfnodetableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfnodetableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
					NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 saamfnodetableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
					  NCSMIB_SETROW_PARAM_VAL *params,
					  struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 saclmnodetableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 saclmnodetableentry_extract(NCSMIB_PARAM_VAL *param,
					   NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 saclmnodetableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saclmnodetableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
					NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 saclmnodetableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
					  NCSMIB_SETROW_PARAM_VAL *params,
					  struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);
EXTERN_C uns32 saclmnodetableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);

EXTERN_C uns32 avd_add_avnd_anchor_nodeid(AVD_CL_CB *cb, SaNameT node_name, SaClmNodeIdT node_id);

#endif
