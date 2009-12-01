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

#include <saAmf.h>
#include <saImm.h>
#include <ncspatricia.h>
#include <ncsdlib.h>
#include <mds_papi.h>
#include <avd_su.h>
#include <avsv_d2nmsg.h>
#include <avd_tmr.h>

struct avd_cluster_tag;
struct avd_node_sw_bundle_tag;
struct CcbUtilOperationData;

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
   /************ AMF B.04 **************************************************/
	SaNameT saAmfNodeClmNode;
	char *saAmfNodeCapacity;
	SaTimeT saAmfNodeSuFailOverProb;
	SaUint32T saAmfNodeSuFailoverMax;
	SaBoolT saAmfNodeAutoRepair;
	SaBoolT saAmfNodeFailfastOnTerminationFailure;
	SaBoolT saAmfNodeFailfastOnInstantiationFailure;
	SaAmfAdminStateT saAmfNodeAdminState;
	SaAmfOperationalStateT saAmfNodeOperState;
   /************ AMF B.04 **************************************************/

	SaNameT hpi_entity_path;	/* The entity path of the HPI entity that
					   implements the CLM node */

	SaAmfOperationalStateT avm_oper_state;	/* operation state of the node */

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

	uns32 rcv_msg_id;	/* The receive message id counter 
				 * Checkpointing - Sent independent update 
				 */

	uns32 snd_msg_id;	/* The send message id counter 
				 * Checkpointing - Sent independent update 
				 */

	NCS_BOOL hrt_beat_rcvd;	/* Boolean Showing First heart beat recevied from AvND */
	struct avd_avnd_tag *cluster_list_node_next;
	struct avd_cluster_tag *node_on_cluster;
	struct avd_node_sw_bundle_tag *list_of_avd_sw_bdl;

} AVD_AVND;

typedef struct avd_ng_tag {

	NCS_PATRICIA_NODE tree_node;	/* key will be AMF  node group name */
	SaNameT ng_name;
	uns32 number_nodes;	/* number of element in saAmfNGNodeList */
	SaNameT *saAmfNGNodeList;	/* array of node names in group */

	struct avd_ng_tag *cluster_list_ng_next;
	struct avd_cluster_tag *ng_on_cluster;

} AVD_AMF_NG;

typedef struct avd_node_sw_bundle_tag {

	NCS_PATRICIA_NODE tree_node;	/* key will be amf cluster name */
	SaNameT sw_bdl_name;
	char *saAmfNodeSwBundlePathPrefix;
	struct avd_node_sw_bundle_tag *node_list_sw_bdl_next;
	AVD_AVND *node_sw_bdl_on_node;

} AVD_NODE_SW_BUNDLE;

#define AVD_AVND_NULL     ((AVD_AVND *)0)

#define m_AVD_SET_AVND_SU_ADMIN(cb,node,state) {\
node->saAmfNodeAdminState = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, node, AVSV_CKPT_AVND_ADMIN_STATE);\
avd_gen_node_admin_state_changed_ntf(cb,node);\
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
   while ((i_su != NULL) && (flag == TRUE))\
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

/* AMF Node */
extern AVD_AVND *avd_node_new(const SaNameT *dn);
extern void avd_node_delete(AVD_AVND **avnd);
extern void avd_node_db_add(AVD_AVND *node);
extern AVD_AVND *avd_node_get(const SaNameT *node_name);
extern AVD_AVND *avd_node_getnext(const SaNameT *node_name);
extern uns32 avd_node_add_nodeid(AVD_AVND *avnd);
extern void avd_node_delete_nodeid(AVD_AVND *node);
extern AVD_AVND *avd_node_find_nodeid(SaClmNodeIdT node_id);
extern AVD_AVND *avd_node_getnext_nodeid(SaClmNodeIdT node_id);
extern SaAisErrorT avd_node_config_get(void);
extern void avd_node_state_set(AVD_AVND *node, AVD_AVND_STATE node_state);
extern void avd_node_oper_state_set(AVD_AVND *node, SaAmfOperationalStateT oper_state);
extern void avd_node_constructor(void);
extern void avd_node_add_su(struct avd_su_tag *su);
extern void avd_node_remove_su(struct avd_su_tag *su);
extern void avd_node_add_swbdl(AVD_NODE_SW_BUNDLE *sw_bdl);
extern void avd_node_remove_swbdl(AVD_NODE_SW_BUNDLE *sw_bdl);

/* AMF Node group */
extern SaAisErrorT avd_ng_config_get(void);
extern AVD_AMF_NG *avd_ng_get(const SaNameT *dn);
extern void avd_ng_constructor(void);

/* AMF Node SW Bundle */
extern AVD_NODE_SW_BUNDLE *avd_nodeswbdl_get(const SaNameT *dn);
extern SaAisErrorT avd_nodeswbdl_config_get(AVD_AVND *node);
extern void avd_nodeswbundle_constructor(void);

#endif
