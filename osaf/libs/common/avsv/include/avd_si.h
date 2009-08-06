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
  service Instance structure and its relationship structures.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_SI_H
#define AVD_SI_H

struct avd_su_si_rel_tag;
struct avd_spons_si_tag;

/* Enum values defines different SI-SI dependency FSM states. */
typedef enum {
   AVD_SI_NO_DEPENDENCY = 1,   
   AVD_SI_SPONSOR_UNASSIGNED,
   AVD_SI_ASSIGNED,
   AVD_SI_TOL_TIMER_RUNNING,
   AVD_SI_READY_TO_UNASSIGN,
   AVD_SI_UNASSIGNING_DUE_TO_DEP,
   AVD_SI_DEP_MAX_STATE
} AVD_SI_DEP_STATE;


/* Availability directors Service Instance structure(AVD_SI):
 * This data structure lives in the AvD and reflects data points 
 * associated with the Service Instance (SI) on the AvD.
 */
typedef struct avd_si_tag {

   NCS_PATRICIA_NODE        tree_node;          /* key will be the SI name */

   SaNameT                  name_net;           /* name of the SI with the length
                                                 * field in the network order.
                                                 * It is used as the index. 
                                                 * Checkpointing - Sent as a one time update.
                                                 */

   uns32                    rank;               /* The rank of the SI in the SG 
                                                 * Checkpointing - Sent as a one time update.
                                                 */

   uns32                    su_config_per_si;   /* The number of standby SU assignments
                                                 * this SI can have in N-Way redundancy model
                                                 * Or the number of active SU assignments
                                                 * this SI can have in N-Way active redundancy model */
   uns32                    su_curr_active;     /* The number of active SU assignments this
                                                 * SI has 
                                                 * Checkpointing - Updated independently.
                                                 */

   uns32                    su_curr_standby;    /* The number of standby SU assignments this
                                                 * SI has 
                                                 * Checkpointing - Updated independently.
                                                 */

   uns32                    max_num_csi;        /* The number of CSIs that will
                                                 * be part of this SI.
                                                 * Checkpointing - Sent as a one time update.
                                                 */

   uns32                    num_csi;            /* The number of CSIs this SI
                                                 * currently has.
                                                 * Checkpointing - Calculated at the Standby.
                                                 */

   SaToggleState            si_switch;          /* The field that indicates if
                                                 * the SI needs to be Toggled.
                                                 * Checkpointing - Updated independently.
                                                 */

   NCS_ADMIN_STATE          admin_state;        /* admin state of the SI
                                                 * Checkpointing - Updated independently.
                                                 */

   NCS_ROW_STATUS           row_status;         /* row status of this MIB row
                                                 * Checkpointing - Updated independently.
                                                 */

   SaNameT                  sg_name;            /* name of the parent SG 
                                                 * Checkpointing - Updated independently.
                                                 */

   AVD_SG                   *sg_of_si;          /* the service group of this SI */       
   struct avd_csi_tag       *list_of_csi;       /* The list of component service instances in
                                                 * the SI */
   struct avd_si_tag        *sg_list_of_si_next;/* the next SI in the list of Service instances
                                                 * in this group */
   struct avd_su_si_rel_tag *list_of_sisu;      /* the list of su si relationship elements */

   AVD_SI_DEP_STATE         si_dep_state;      /* SI-SI dep state of this SI */
   struct avd_spons_si_tag  *spons_si_list;
   uns32                    tol_timer_count;
} AVD_SI;


/* SG-SI-Rank table index structure */
typedef struct avd_sg_si_rank_indx_tag {

  SaNameT                   sg_name_net;       /* Name of the SG with the length
                                                * field in the network order. */
  uns32                     si_rank_net;       /* The rank of the SI for the SG
                                                * In the network order */
}AVD_SG_SI_RANK_INDX;


/* Availability directors SIs organised per RANK in a SG. 
 * This Data structure lives in the AVD and is maintained as a patricia tree 
 * from the AVD Control Block.
 */
typedef struct avd_sg_si_rank_tag {

 NCS_PATRICIA_NODE        tree_node;            /* key will be the SG name and Rank */
 AVD_SG_SI_RANK_INDX      indx;                 /* Index */
 SaNameT                  si_name;              /* Name of the Service Instance */

}AVD_SG_SI_RANK;

#define AVD_SG_SI_RANK_NULL ((AVD_SG_SI_RANK *)0)

#define AVD_SI_NULL ((AVD_SI *)0)
#define m_AVD_SI_ACTV_MAX_SU(l_si) (l_si)->su_config_per_si
#define m_AVD_SI_ACTV_CURR_SU(l_si) (l_si)->su_curr_active
#define m_AVD_SI_INC_ACTV_CURR_SU(l_si) \
{\
 (l_si)->su_curr_active ++; \
 m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb,l_si,AVSV_CKPT_SI_SU_CURR_ACTIVE); \
}

#define m_AVD_SI_DEC_ACTV_CURR_SU(l_si)\
{\
   if ((l_si)->su_curr_active != 0)\
   {\
      (l_si)->su_curr_active --;\
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb,l_si,AVSV_CKPT_SI_SU_CURR_ACTIVE); \
   }\
}

#define m_AVD_SI_STDBY_MAX_SU(l_si)       (l_si)->su_config_per_si
#define m_AVD_SI_STDBY_CURR_SU(l_si)      (l_si)->su_curr_standby

#define m_AVD_SI_INC_STDBY_CURR_SU(l_si)  \
{ \
   ((l_si)->su_curr_standby++); \
   m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb,l_si,AVSV_CKPT_SI_SU_CURR_STBY); \
}

#define m_AVD_SI_DEC_STDBY_CURR_SU(l_si)\
{\
   if ((l_si)->su_curr_standby != 0)\
   { \
      (l_si)->su_curr_standby--;\
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb,l_si,AVSV_CKPT_SI_SU_CURR_STBY); \
   } \
}




#define m_AVD_SET_SI_SWITCH(cb,si,state) {\
si->si_switch = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, si, AVSV_CKPT_SI_SWITCH);\
}

#define m_AVD_SET_SI_ADMIN(cb,si,state) {\
si->admin_state = state;\
m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, si, AVSV_CKPT_SI_ADMIN_STATE);\
avd_gen_si_admin_state_chg_trap(cb,si);\
}

EXTERN_C AVD_SI * avd_si_struc_crt(AVD_CL_CB *cb,SaNameT si_name,NCS_BOOL ckpt);
EXTERN_C AVD_SI * avd_si_struc_find(AVD_CL_CB *cb,SaNameT si_name,NCS_BOOL host_order);
EXTERN_C AVD_SI * avd_si_struc_find_next(AVD_CL_CB *cb,SaNameT si_name,NCS_BOOL host_order);
EXTERN_C uns32 avd_si_struc_del(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C void avd_si_add_sg_list(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C void avd_si_del_sg_list(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 saamfsitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 saamfsitableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 saamfsitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfsitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 saamfsitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);



EXTERN_C uns32 ncssitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 ncssitableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 ncssitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncssitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 ncssitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
                             
EXTERN_C uns32 ncssitableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX* idx);

EXTERN_C AVD_SG_SI_RANK * avd_sg_si_rank_add_row(AVD_CL_CB *cb, AVD_SI *si);

EXTERN_C AVD_SG_SI_RANK * avd_sg_si_rank_struc_find(AVD_CL_CB *cb,
                                        AVD_SG_SI_RANK_INDX indx);

EXTERN_C AVD_SG_SI_RANK * avd_sg_si_rank_struc_find_next(AVD_CL_CB *cb,
                                             AVD_SG_SI_RANK_INDX indx);

EXTERN_C uns32 avd_sg_si_rank_del_row(AVD_CL_CB *cb, AVD_SI *si);

EXTERN_C uns32 saamfsgsirankentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                             NCSCONTEXT* data);

EXTERN_C uns32 saamfsgsirankentry_extract(NCSMIB_PARAM_VAL* param,
                                 NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                                 NCSCONTEXT buffer);

EXTERN_C uns32 saamfsgsirankentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSCONTEXT* data, uns32* next_inst_id,
                              uns32 *next_inst_id_len);

EXTERN_C uns32 saamfsgsirankentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                             NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);

EXTERN_C uns32 saamfsgsirankentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);

EXTERN_C uns32 saamfsgsirankentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);


#endif
