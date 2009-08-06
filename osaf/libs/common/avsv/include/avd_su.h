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
  service unit structure.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_SU_H
#define AVD_SU_H

struct avd_su_si_rel_tag;
struct avd_cmp_tag;

/* The semantics the SU is undergoing. */
typedef enum {
	AVD_SU_NO_STATE = 0,
	AVD_SU_OPER,
	AVD_SU_NODE_OPER
} AVD_SU_STATE;

/* Avialability directors Service Unit structure(AVD_SU):
 * This data structure lives in the AvD and reflects data points
 * associated with the Service Unit (SU) on the AvD.
 */
typedef struct avd_su_tag {

	NCS_PATRICIA_NODE tree_node;	/* key will be the SU name */

	SaNameT name_net;	/* name of the SU with the length
				 * field in the network order.
				 *  It is used as the index.
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 rank;		/* The rank of the SU in the SG
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 num_of_comp;	/* The number of components
				 * that will make up this SU
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 curr_num_comp;	/* The current number of components
				 * configured.
				 * Checkpointing - Calculated at Standby.
				 */

	uns32 si_max_active;	/* The maximum number of active instance of
				 * Sis that can be assigned to this SU.
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 si_max_standby;	/* The maximum number of standby instance of
				 * Sis that can be assigned to this SU.
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 si_curr_active;	/* The current number of active instance of
				 * Sis that has been assigned to this SU.
				 * Checkpointing - Updated independently.
				 */

	uns32 si_curr_standby;	/* The current number of standby instance 
				 * of Sis that has been assigned to this SU
				 * Checkpointing - Updated independently.
				 */

	NCS_BOOL is_su_failover;	/* denotes whether su is configured to
					 * failover as a single entity
					 * Checkpointing - Sent as a one time update.
					 */

	NCS_ADMIN_STATE admin_state;	/* admin state of the service unit
					 * Checkpointing - Updated independently.
					 */

	NCS_BOOL term_state;	/* admin state to terminate the
				 * service unit.
				 * Checkpointing - Updated independently.
				 */

	SaToggleState su_switch;	/* The field that indicates if
					 * the active SIs assigned to 
					 * this SU needs to be Toggled.
					 * Checkpointing - Updated independently.
					 */

	NCS_OPER_STATE oper_state;	/* SU operational state
					 * Checkpointing - Updated independently.
					 */

	NCS_PRES_STATE pres_state;	/* SU presence state.
					 * Checkpointing - Updated independently.
					 */

	SaNameT sg_name;	/* name of the parent SG 
				 * Checkpointing - Sent as a one time update.
				 */

	NCS_READINESS_STATE readiness_state;	/* the readiness state of the SU
						 * Checkpointing - Updated independently.
						 */

	NCS_BOOL su_is_external;	/* indicates if this SU is external */

	NCS_ROW_STATUS row_status;	/* row status of this MIB row 
					 * Checkpointing - Sent as a one time update.
					 */

	AVD_SU_STATE su_act_state;	/* The current action flow of
					 * the SU w.r.t to the node.
					 * Checkpointing - Updated independently.
					 */

	NCS_BOOL su_preinstan;	/* Field indicates if the SU is
				 * is pre instantiable or not.
				 * Checkpointing - Updated independently.
				 */

	AVD_SG *sg_of_su;	/* the service group of this SU */
	AVD_AVND *su_on_node;	/*  the node on which this SU resides */
	struct avd_su_si_rel_tag *list_of_susi;	/* the list of su si relationship elements */
	struct avd_cmp_tag *list_of_comp;	/* the list of  components in this SU */
	struct avd_su_tag *sg_list_su_next;	/* the next SU in the SG */
	struct avd_su_tag *avnd_list_su_next;	/* the next SU in the AvND */

} AVD_SU;

/* SG-SU-Rank table index structure */
typedef struct avd_sg_su_rank_indx_tag {

	SaNameT sg_name_net;	/* Name of the SG with the length
				 * field in the network order. */
	uns32 su_rank_net;	/* The rank of the SU for the SG
				 * In the network order */
} AVD_SG_SU_RANK_INDX;

/* Availability directors SUs organised per RANK in a SG. 
 * This Data structure lives in the AVD and is maintained as a patricia tree 
 * from the AVD Control Block.
 */
typedef struct avd_sg_su_rank_tag {

	NCS_PATRICIA_NODE tree_node;	/* key will be the SG name and Rank */
	AVD_SG_SU_RANK_INDX indx;	/* Index */
	SaNameT su_name;	/* Name of the Service Unit */

} AVD_SG_SU_RANK;

#define AVD_SG_SU_RANK_NULL ((AVD_SG_SU_RANK *)0)

#define AVD_SU_NULL ((AVD_SU *)0)

#define m_AVD_SET_SU_ADMIN(cb,su,state) {\
su->admin_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_SU_ADMIN_STATE);\
avd_gen_su_admin_state_changed_trap(cb,su);\
}

#define m_AVD_SET_SU_TERM(cb,su,state) {\
su->term_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_SU_TERM_STATE);\
}

#define m_AVD_SET_SU_SWITCH(cb,su,state) {\
su->su_switch = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_SU_SWITCH);\
}

#define m_AVD_SET_SU_OPER(cb,su,state) {\
su->oper_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_SU_OPER_STATE);\
}

#define m_AVD_SET_SU_REDINESS(cb,su,state) {\
su->readiness_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, su, AVSV_CKPT_SU_REDINESS_STATE);\
}

#define m_AVD_APP_SU_IS_INSVC(i_su,su_node_ptr) \
((su_node_ptr->su_admin_state == NCS_ADMIN_STATE_UNLOCK) && \
(su_node_ptr->oper_state == NCS_OPER_STATE_ENABLE) && \
(su_node_ptr->avm_oper_state == NCS_OPER_STATE_ENABLE) && \
(i_su->sg_of_su->admin_state == NCS_ADMIN_STATE_UNLOCK) &&\
(i_su->admin_state == NCS_ADMIN_STATE_UNLOCK) &&\
(i_su->oper_state == NCS_OPER_STATE_ENABLE)\
)

#define m_AVD_GET_SU_NODE_PTR(avd_cb,i_su,su_node_ptr)  \
 if(TRUE == i_su->su_is_external) su_node_ptr = avd_cb->ext_comp_info.local_avnd_node; \
 else su_node_ptr = i_su->su_on_node;

EXTERN_C AVD_SU *avd_su_struc_crt(AVD_CL_CB *cb, SaNameT su_name, NCS_BOOL ckpt);
EXTERN_C AVD_SU *avd_su_struc_find(AVD_CL_CB *cb, SaNameT su_name, NCS_BOOL host_order);
EXTERN_C AVD_SU *avd_su_struc_find_next(AVD_CL_CB *cb, SaNameT su_name, NCS_BOOL host_order);
EXTERN_C uns32 avd_su_struc_del(AVD_CL_CB *cb, AVD_SU *su);
EXTERN_C void avd_su_add_sg_list(AVD_CL_CB *cb, AVD_SU *su);
EXTERN_C void avd_su_del_sg_list(AVD_CL_CB *cb, AVD_SU *su);
EXTERN_C void avd_su_del_avnd_list(AVD_CL_CB *cb, AVD_SU *su);
EXTERN_C uns32 saamfsutableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 saamfsutableentry_extract(NCSMIB_PARAM_VAL *param,
					 NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 saamfsutableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfsutableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				      NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 saamfsutableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
					NCSMIB_SETROW_PARAM_VAL *params,
					struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 ncssutableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 ncssutableentry_extract(NCSMIB_PARAM_VAL *param,
				       NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 ncssutableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncssutableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				    NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 ncssutableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				      NCSMIB_SETROW_PARAM_VAL *params,
				      struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);
EXTERN_C uns32 ncssutableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);
EXTERN_C void avd_su_ack_msg(AVD_CL_CB *cb, AVD_DND_MSG *ack_msg);

EXTERN_C AVD_SG_SU_RANK *avd_sg_su_rank_add_row(AVD_CL_CB *cb, AVD_SU *su);

EXTERN_C AVD_SG_SU_RANK *avd_sg_su_rank_struc_find(AVD_CL_CB *cb, AVD_SG_SU_RANK_INDX indx);

EXTERN_C AVD_SG_SU_RANK *avd_sg_su_rank_struc_find_next(AVD_CL_CB *cb, AVD_SG_SU_RANK_INDX indx);

EXTERN_C uns32 avd_sg_su_rank_del_row(AVD_CL_CB *cb, AVD_SU *su);

EXTERN_C uns32 saamfsgsurankentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 saamfsgsurankentry_extract(NCSMIB_PARAM_VAL *param,
					  NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 saamfsgsurankentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfsgsurankentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				       NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 saamfsgsurankentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
					 NCSMIB_SETROW_PARAM_VAL *params,
					 struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

#endif
