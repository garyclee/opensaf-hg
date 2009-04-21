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
  Service group structure.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_SG_H
#define AVD_SG_H


struct avd_su_tag;
struct avd_si_tag;


/* The valid SG FSM states. */
typedef enum { 
   AVD_SG_FSM_STABLE = 0,
   AVD_SG_FSM_SG_REALIGN,
   AVD_SG_FSM_SU_OPER,
   AVD_SG_FSM_SI_OPER,
   AVD_SG_FSM_SG_ADMIN
} AVD_SG_FSM_STATE;


/* The structure used for containing the list of SUs
 * undergoing operations on them.
 */
typedef struct avd_sg_oper_tag{
   struct avd_su_tag  *su; /* SU undergoing operation*/
   struct avd_sg_oper_tag *next; /* The next SU undergoing operation.*/
} AVD_SG_OPER;


/* Availability directors Service Group structure(AVD_SG):
 * This data structure lives in the AvD and reflects data points
 * associated with the Service group (SG).
 */
typedef struct avd_sg_tag{

   NCS_PATRICIA_NODE  tree_node;          /* key will be the SG name */

   SaNameT            name_net;           /* the service group name with the length
                                           * field in the network order. It is
                                           * used as the index.
                                           * Checkpointing - Sent as a one time update.
                                           */

   SaReduntantModelT  su_redundancy_model;/* the redundancy model in the service group 
                                           * see sec 4.7 for values
                                           * Checkpointing - Sent as a one time update.
                                           */

   NCS_BOOL           su_failback;        /* Should AvSv fail back the SUs to
                                           * the preferred configuration.
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              pref_num_active_su; /* the N value in the redundancy model, where
                                           * applicable.
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              pre_num_standby_su; /* the M value in the redundancy model, where
                                           * applicable.
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              pref_num_insvc_su;  /* The preferred number of in service
                                           * SUs.
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              pref_num_assigned_su;/* The number of active SU assignments
                                           * an SI can have.
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              su_max_active;      /* The maximum number of active 
                                           * instance of Sis that can be 
                                           * assigned to an SU.
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              su_max_standby;     /* The maximum number of standby 
                                           * instance of Sis that can be
                                           * assigned to an SU.
                                           * Checkpointing - Sent as a one time update.
                                           */

   struct avd_su_tag  *list_of_su;        /* the list of service units in this
                                           * group in the descending order of
                                           * the rank.
                                           */
   struct avd_si_tag  *list_of_si;        /* the list of service instances in 
                                           * this group in the descending order 
                                           * of the rank.
                                           */

   NCS_ADMIN_STATE    admin_state;        /* admin state of the group.
                                           * Checkpointing - Updated independently.
                                           */

   SaAdjustState      adjust_state;       /* Field to re adjust the SG.
                                           * Checkpointing - Updated independently.
                                           */

   SaBoolT            sg_ncs_spec;         /* This is set to true if the SG
                                           * is a NCS specific SG.
                                           * Checkpointing - Sent as a one time update.
                                           */

   NCS_ROW_STATUS     row_status;         /* row status of this MIB row 
                                           * Checkpointing - Sent as a one time update.
                                           */

   SaTimeT            comp_restart_prob;  /* component restart probation period
                                          * Checkpointing - Sent as a one time update.
                                           */

   uns32              comp_restart_max;   /* max component restart count 
                                          * Checkpointing - Sent as a one time update.
                                           */

   SaTimeT            su_restart_prob;    /* SU restart probation period 
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              su_restart_max;     /* max SU restart count
                                           * Checkpointing - Sent as a one time update.
                                           */

   uns32              su_assigned_num;    /* Num of Sus that have been assigned a SI 
                                           * Checkpointing - Updated independently.
                                           */

   uns32              su_spare_num;       /* Num of Sus that are in service but 
                                           * not yet assigned a SI.
                                           * Checkpointing - Updated independently.
                                           */

   uns32              su_uninst_num;      /* Num of Sus that have been 
                                           * configured but not yet instantiated.
                                           * Checkpointing - Updated independently.
                                           */

   AVD_SG_FSM_STATE   sg_fsm_state;       /* The different flows of the SU SI
                                           * transitions for the SUs and SIs
                                           * in the SG is orchestrated based on
                                           * this FSM.
                                           * Checkpointing - Updated independently.
                                           */

   struct avd_si_tag  *admin_si;          /* Applicable when sg_fsm_state has
                                           * AVD_SG_FSM_SI_OPER.It will contain
                                           * the SI undergoing admin
                                           * operation.
                                           */

   AVD_SG_OPER        su_oper_list;       /* The list of SUs that have operations
                                           * happening on them used in parallel
                                           * with sg_fsm_state.
                                           * Checkpointing - Sent as a one time update.
                                           */
       
} AVD_SG;


#define AVD_SG_NULL ((AVD_SG *)0)
#define AVD_SG_OPER_NULL ((AVD_SG_OPER *)0)

#define m_AVD_SET_SG_ADMIN(cb,sg,state) {\
sg->admin_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_ADMIN_STATE);\
avd_gen_sg_admin_state_changed_trap(cb, sg);\
}

#define m_AVD_SET_SG_ADJUST(cb,sg,state) {\
sg->adjust_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_ADJUST_STATE);\
}

#define m_AVD_SET_SG_FSM(cb,sg,state) {\
sg->sg_fsm_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_FSM_STATE);\
if (state == AVD_SG_FSM_STABLE) \
   avd_sg_screen_si_si_dependencies(cb, sg); \
}

#define m_AVD_SET_SG_ADMIN_SI(cb,si) {\
si->sg_of_si->admin_si = si;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, (si->sg_of_si), AVSV_CKPT_AVD_SG_ADMIN_SI);\
}

#define m_AVD_CLEAR_SG_ADMIN_SI(cb,sg) {\
if (sg->admin_si != AVD_SI_NULL)\
{\
m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, sg, AVSV_CKPT_AVD_SG_ADMIN_SI);\
sg->admin_si = AVD_SI_NULL;\
}\
}

/*****************************************************************************
 * Macro: m_AVD_CHK_OPLIST
 *
 * Purpose:  This macro will search the SU operation list in the SG to
 * identify if the given SU is present in the list
 *
 * Input: su - the pointer to the SU to be checked in the list.
 *        flag - the NCS_BOOL field indicating if found or not.
 *        
 *
 * Return: none.
 *
 * NOTES: 
 *
 * 
 **************************************************************************/
#define m_AVD_CHK_OPLIST(i_su,flag) \
{\
   AVD_SG_OPER *l_suopr;\
   flag = FALSE;\
   if (i_su->sg_of_su->su_oper_list.su == i_su)\
   {\
      flag = TRUE;\
   }else if (i_su->sg_of_su->su_oper_list.next != AVD_SG_OPER_NULL)\
   {\
      l_suopr = i_su->sg_of_su->su_oper_list.next;\
      while (l_suopr != AVD_SG_OPER_NULL)\
      {\
         if (l_suopr->su == i_su)\
         {\
            flag = TRUE;\
            l_suopr = AVD_SG_OPER_NULL;\
            continue; \
         }\
         l_suopr = l_suopr->next;\
      }\
   }\
}


EXTERN_C AVD_SG * avd_sg_struc_crt(AVD_CL_CB *cb,SaNameT sg_name,NCS_BOOL ckpt);
EXTERN_C AVD_SG * avd_sg_struc_find(AVD_CL_CB *cb,SaNameT sg_name,NCS_BOOL host_order);
EXTERN_C AVD_SG * avd_sg_struc_find_next(AVD_CL_CB *cb,SaNameT sg_name,NCS_BOOL host_order);
EXTERN_C uns32 avd_sg_struc_del(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C uns32 ncssgtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 ncssgtableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 ncssgtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncssgtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 ncssgtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
                            
EXTERN_C uns32 ncssgtableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX* idx);
                             
EXTERN_C uns32 saamfsgtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 saamfsgtableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 saamfsgtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfsgtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 saamfsgtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
                             
#endif
