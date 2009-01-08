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

  DESCRIPTION:

  This module is the include file for handling Availability Directors
  service Instance structure and its relationship structures.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_SI_DEP_H
#define AVD_SI_DEP_H


struct avd_si_tag;

typedef enum {
  AVD_SI_DEP_SPONSOR_ASSIGNED = 1,
  AVD_SI_DEP_SPONSOR_UNASSIGNED
} AVD_SI_DEP_SPONSOR_SI_STATE;


/* Spons-SI node of the spons-list in SI struct */
typedef struct avd_spons_si_tag
{
  struct avd_si_tag *si;
  struct avd_spons_si_tag *next;
} AVD_SPONS_SI_NODE;


/*
 * Following struct is used to make a list which is used while checking the
 * SI-SI dependencies are CYCLIC.
 */
typedef struct avd_si_dep_name_list
{
   /* SI name */
   SaNameT  si_name;

   struct avd_si_dep_name_list *next;
} AVD_SI_DEP_NAME_LIST;

/*
 * Index data to retrieve a specific SI-SI dep. data record from its
 * database (si_dep: dep_anchor, spons_anchor) defined AvD CB .
 */
typedef struct avd_si_si_dep_indx_tag
{
   /* primary-key: SI name, either of sponsor SI or dependent SI  */
   SaNameT  si_name_prim;

   /* secondary-key: SI name, other than primary */
   SaNameT  si_name_sec;
} AVD_SI_SI_DEP_INDX;


/*
 * Data structure that holds SI-SI dependency data, same record is updated
 * in two SI_SI dep anch tree's. One of the tree hold sponsor SI name as the
 * primary indx & dependent SI name as secondary indx (as per SI_SI dep MIB).
 * The other one has vis-versa.
 */
typedef struct avd_si_si_dep
{
   NCS_PATRICIA_NODE   tree_node_mib;
   NCS_PATRICIA_NODE   tree_node;

   /* Index info to retrieve the record */
   AVD_SI_SI_DEP_INDX  indx_mib;
   AVD_SI_SI_DEP_INDX  indx;

   uns32            si_dep_hdl;
   SaTimeT          tolerance_time;
   NCS_ROW_STATUS   row_status;
   AVD_TMR          si_dep_timer;   /* SI-SI dep. tol timer */
   NCS_BOOL         unassign_event;
} AVD_SI_SI_DEP;

#define AVD_SI_SI_DEP_NULL ((AVD_SI_SI_DEP *)0)


#define m_AVD_SET_SI_DEP_STATE(cb, si, state) {\
if ((state != AVD_SI_TOL_TIMER_RUNNING) && (state != AVD_SI_READY_TO_UNASSIGN)) \
   avd_si_dep_stop_tol_timer(cb, si); \
si->si_dep_state = state; \
}

EXTERN_C uns32 avd_si_dep_spons_list_add(AVD_CL_CB *avd_cb, struct avd_si_tag *dep_si, struct avd_si_tag *spons_si);
EXTERN_C void avd_si_dep_delete(AVD_CL_CB *cb, struct avd_si_tag *si);
EXTERN_C void avd_si_dep_spons_list_del(AVD_CL_CB *cb, AVD_SI_SI_DEP *si_dep_rec);
EXTERN_C AVD_SI_SI_DEP *avd_si_si_dep_struc_crt(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx);
EXTERN_C AVD_SI_SI_DEP *avd_si_si_dep_find(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx,
                                           NCS_BOOL isMibIdx);
EXTERN_C AVD_SI_SI_DEP *avd_si_si_dep_find_next(AVD_CL_CB *cb, AVD_SI_SI_DEP_INDX *indx,
                                                NCS_BOOL isMibIdx);
EXTERN_C uns32 avd_si_si_dep_del_row(AVD_CL_CB *cb, AVD_SI_SI_DEP *rec);
EXTERN_C void avd_tmr_si_dep_tol_func(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_si_dep_spons_unassign(AVD_CL_CB *cb, struct avd_si_tag *si, struct avd_si_tag *si_dep);
EXTERN_C void avd_process_si_dep_state_evt(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_screen_sponsor_si_state(AVD_CL_CB *cb, struct avd_si_tag *si, NCS_BOOL start_assignment);
EXTERN_C void avd_sg_screen_si_si_dependencies(AVD_CL_CB *cb, AVD_SG *sg);

EXTERN_C uns32 saamfsisideptableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data);
EXTERN_C uns32 saamfsisideptableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer);
EXTERN_C uns32 saamfsisideptableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag);
EXTERN_C uns32 saamfsisideptableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len);
EXTERN_C uns32 saamfsisideptableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag);
EXTERN_C void avd_si_dep_stop_tol_timer(AVD_CL_CB *cb, struct avd_si_tag *si);

#endif
