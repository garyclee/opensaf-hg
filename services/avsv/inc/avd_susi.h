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

  This module is the include file for Availability Directors su si relationship
  structures.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_SUSI_H
#define AVD_SUSI_H

struct avd_comp_csi_rel_tag;
struct avd_csi_tag;

typedef enum {
   AVD_SU_SI_STATE_ABSENT = 1,
   AVD_SU_SI_STATE_ASGN,
   AVD_SU_SI_STATE_ASGND,
   AVD_SU_SI_STATE_UNASGN,
   AVD_SU_SI_STATE_MODIFY
} AVD_SU_SI_STATE;

/* Availability directors SU SI relationship structure(AVD_SU_SI_REL):
 * This data structure lives in the AvD and reflects relationship between
 * the SU and SI on the AvD.
 */
typedef struct avd_su_si_rel_tag {

   AVD_SI                      *si;             /* SI to which this relationship with SU
                                                 * exists */
   AVD_SU                      *su;             /* SU to which this relationship with SI
                                                 * exists */
   SaAmfHAStateT               state;           /* The current HA state for the SI w.r.t SU */
   struct avd_comp_csi_rel_tag *list_of_csicomp;/* The List of component CSI assignments that
                                                 * belong to this SU SI relationship */
   AVD_SU_SI_STATE             fsm;             /* The SU SI FSM state */
   NCS_BOOL                    is_per_si;       /* indicates if the susi rel is added as per 
                                                   the su rank in the per si list */
   struct avd_su_si_rel_tag    *su_next;        /* The next element in the list w.r.t to SU */
   struct avd_su_si_rel_tag    *si_next;        /* The next element in the list w.r.t to SI */

} AVD_SU_SI_REL;


/* SusperSiRank table index structure */
typedef struct avd_sus_per_si_rank_index_tag {

   SaNameT                   si_name_net;       /* Name of the SI with the length
                                                 * field in the network order. */
   uns32                     su_rank_net;       /* The rank of the SU for that SI
                                                 * In the network order */

}AVD_SUS_PER_SI_RANK_INDX;


/* Availability directors SUs organised per RANK in a SI. 
 * This Data structure lives in the AVD and is maintained as a patricia tree 
 * from the AVD Control Block.
 */
typedef struct avd_sus_per_si_rank_tag {

   NCS_PATRICIA_NODE         tree_node;         /* key will be the SI name and Rank */
   AVD_SUS_PER_SI_RANK_INDX  indx;              /* Table index */
   SaNameT                   su_name;           /* name of the SU as SaNameT */ 
   NCS_ROW_STATUS            row_status;        /* row status of this MIB row */

}AVD_SUS_PER_SI_RANK;


#define   AVD_SU_PER_SI_RANK_NULL   ((AVD_SUS_PER_SI_RANK *)0)

#define AVD_SU_SI_REL_NULL ((AVD_SU_SI_REL *)0)

#define m_AVD_SU_SI_CHK_QSD_ASGN(l_su,flag) \
{\
   AVD_SU_SI_REL *i_susi;\
   i_susi = (l_su)->list_of_susi;\
   flag = TRUE;\
   while ((i_susi != AVD_SU_SI_REL_NULL) && (flag == TRUE))\
   {\
      if ((i_susi->fsm != AVD_SU_SI_STATE_UNASGN) &&\
         (i_susi->state != SA_AMF_HA_QUIESCED))\
      {\
         flag = FALSE;\
         continue;\
      }\
      i_susi = i_susi->su_next;\
   }\
}

#define m_AVD_SU_SI_CHK_ASGND(l_su,flag) \
{\
   AVD_SU_SI_REL *i_susi;\
   i_susi = (l_su)->list_of_susi;\
   flag = TRUE;\
   while ((i_susi != AVD_SU_SI_REL_NULL) && (flag == TRUE))\
   {\
      if (i_susi->fsm != AVD_SU_SI_STATE_ASGND)\
      {\
         flag = FALSE;\
         continue;\
      }\
      i_susi = i_susi->su_next;\
   }\
}

#define m_AVD_SU_SI_TRG_DEL(cb,susi) \
{\
   AVD_SI *l_si=susi->si;\
   m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, susi, AVSV_CKPT_AVD_SU_SI_REL);\
   avd_susi_struc_del(cb,susi,FALSE);\
   if ((l_si != AVD_SI_NULL) && (l_si->list_of_sisu == AVD_SU_SI_REL_NULL))\
   {\
      avd_gen_si_unassigned_trap(cb,l_si);\
      avd_gen_si_oper_state_chg_trap(cb,l_si);\
   }\
}

EXTERN_C AVD_SU_SI_REL * avd_susi_struc_crt(AVD_CL_CB *cb,AVD_SI *si,AVD_SU *su);
EXTERN_C AVD_SU_SI_REL * avd_susi_struc_find(AVD_CL_CB *cb,SaNameT su_name,
                                     SaNameT si_name,NCS_BOOL host_order);
EXTERN_C AVD_SU_SI_REL * avd_su_susi_struc_find(AVD_CL_CB *cb,AVD_SU *su,
                                    SaNameT si_name,NCS_BOOL host_order);
EXTERN_C AVD_SU_SI_REL * avd_susi_struc_find_next(AVD_CL_CB *cb,SaNameT su_name,
                                    SaNameT si_name,NCS_BOOL host_order);
EXTERN_C uns32 avd_susi_struc_del(AVD_CL_CB *cb,AVD_SU_SI_REL *susi, NCS_BOOL ckpt);
EXTERN_C uns32 saamfsusitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 saamfsusitableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 saamfsusitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfsusitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 saamfsusitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);

EXTERN_C uns32 saamfsusitableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX* idx);





EXTERN_C AVD_SUS_PER_SI_RANK * avd_sus_per_si_rank_struc_crt(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx);

EXTERN_C AVD_SUS_PER_SI_RANK * avd_sus_per_si_rank_struc_find(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx);

EXTERN_C AVD_SUS_PER_SI_RANK * avd_sus_per_si_rank_struc_find_next(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx);
EXTERN_C AVD_SUS_PER_SI_RANK * avd_sus_per_si_rank_struc_find_valid_next(AVD_CL_CB *, AVD_SUS_PER_SI_RANK_INDX,
                                                                         AVD_SU **);


EXTERN_C uns32 avd_sus_per_si_rank_struc_del(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK *rank_elt);

EXTERN_C uns32 saamfsuspersirankentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                  NCSCONTEXT* data);

EXTERN_C uns32 saamfsuspersirankentry_extract(NCSMIB_PARAM_VAL* param,
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);

EXTERN_C uns32 saamfsuspersirankentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);

EXTERN_C uns32 saamfsuspersirankentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);

EXTERN_C uns32 saamfsuspersirankentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
#endif
