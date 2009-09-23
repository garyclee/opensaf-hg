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

  This module is the include file for handling Availability Node Directors 
  service unit structure.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVND_SU_H
#define AVND_SU_H

struct avnd_cb_tag;

/***************************************************************************
 **********  S T R U C T U R E / E N U M  D E F I N I T I O N S  ***********
 ***************************************************************************/

/*##########################################################################
                      SERVICE INSTANCE (SI) DEFINITIONS                     
 ##########################################################################*/

/* 
 * SU-SI Assign Message contains some message specific parameters that are 
 * not a part of SU-SI parameters. Concealing those params thru this typedef.
 */
typedef AVSV_D2N_INFO_SU_SI_ASSIGN_MSG_INFO AVND_SU_SI_PARAM;

/* declaration clc event handler */
typedef uns32 (*AVND_SU_PRES_FSM_FN) (struct avnd_cb_tag *, struct avnd_su_tag *, AVND_COMP *);

/* su presence state fsm events */
typedef enum avnd_su_pres_fsm_ev {
	AVND_SU_PRES_FSM_EV_INST = 1,
	AVND_SU_PRES_FSM_EV_TERM,
	AVND_SU_PRES_FSM_EV_RESTART,
	AVND_SU_PRES_FSM_EV_COMP_INSTANTIATED,
	AVND_SU_PRES_FSM_EV_COMP_INST_FAIL,
	AVND_SU_PRES_FSM_EV_COMP_RESTARTING,
	AVND_SU_PRES_FSM_EV_COMP_TERM_FAIL,
	AVND_SU_PRES_FSM_EV_COMP_UNINSTANTIATED,
	AVND_SU_PRES_FSM_EV_COMP_TERMINATING,
	AVND_SU_PRES_FSM_EV_MAX
} AVND_SU_PRES_FSM_EV;

/* SI assignment state type definition */
typedef enum avnd_su_si_assign_state {
	AVND_SU_SI_ASSIGN_STATE_UNASSIGNED = 1,
	AVND_SU_SI_ASSIGN_STATE_ASSIGNING,
	AVND_SU_SI_ASSIGN_STATE_ASSIGNED,
	AVND_SU_SI_ASSIGN_STATE_REMOVING,
	AVND_SU_SI_ASSIGN_STATE_REMOVED,
	AVND_SU_SI_ASSIGN_STATE_MAX,
} AVND_SU_SI_ASSIGN_STATE;

/* SI definition */
typedef struct avnd_su_si_rec {
	NCS_DB_LINK_LIST_NODE su_dll_node;	/* node in the su-si dll */
	SaNameT name_net;	/* si name */

	SaAmfHAStateT curr_state;	/* current si ha state */
	SaAmfHAStateT prv_state;	/* prv si ha state */

	AVND_SU_SI_ASSIGN_STATE curr_assign_state;	/* si assignment state wrt 
							   current ha state */
	AVND_SU_SI_ASSIGN_STATE prv_assign_state;	/* si assignment state wrt 
							   prv ha state */

	NCS_DB_LINK_LIST csi_list;	/* ordered csi list (based on csi rank) */

	/* links to other entities */
	struct avnd_su_tag *su;	/* bk ptr to su */
	SaNameT su_name_net;	/* For checkpointing su name */
} AVND_SU_SI_REC;

/* SU-SI buffer record definition */
typedef struct avnd_su_siq_rec {
	NCS_DB_LINK_LIST_NODE su_dll_node;	/* node in the siq dll */
	AVND_SU_SI_PARAM info;	/* su-si param */
} AVND_SU_SIQ_REC;

/*##########################################################################
                      SERVICE UNIT DEFINITION (TOP LEVEL)                   
 ##########################################################################*/

typedef AVSV_SU_INFO_MSG AVND_SU_PARAM;

typedef struct avnd_su_tag {
	NCS_PATRICIA_NODE tree_node;	/* su tree node (key is su name) */
	SaNameT name_net;	/* su name */

	uns32 mab_hdl;		/* mab handle for this su */
	uns32 su_hdl;		/* hdl returned by hdl-mngr */

	/* su attributes */
	uns32 flag;		/* su attributes */

	/* Update received flag, which will normally be FALSE and will be
	 * TRUE if updates are received from the AVD on fail-over.*/
	NCS_BOOL avd_updt_flag;

	/* error recovery escalation params */
	AVND_ERR_ESC_LEVEL su_err_esc_level;	/* curr escalation level of this su */
	SaTimeT comp_restart_prob;	/* comp restart probation period (config) */
	uns32 comp_restart_max;	/* max comp restart count (config) */
	SaTimeT su_restart_prob;	/* su restart probation period (config) */
	uns32 su_restart_max;	/* max su restart count (config) */
	uns32 comp_restart_cnt;	/* comp restart counts within comp-prob period */
	uns32 su_restart_cnt;	/* su restart counts within su-prob period */
	AVND_TMR su_err_esc_tmr;	/* su err esc tmr */

	/* su states */
	NCS_OPER_STATE oper;	/* oper state of the su */
	NCS_PRES_STATE pres;	/* presence state of the su */

	/* statistical info */
	uns32 si_active_cnt;	/* no of active SIs assigned to this su */
	uns32 si_standby_cnt;	/* no of standby SIs assigned to this su */

	/* 
	 * Ordered comp list (based on inst level). Note that as the 
	 * lexicographic key (comp-name) & the keys used for 
	 * comparision (inst-level) are different, only a limited number 
	 * of DLL APIs can be used.
	 */
	NCS_DB_LINK_LIST comp_list;

	NCS_DB_LINK_LIST si_list;	/* si list (si-name is the index) */
	NCS_DB_LINK_LIST siq;	/* susi msg buf (no index) maintains fifo order */

	/* To have the knowledge in AvND if this su belongs to NCS_SG */
	SaBoolT is_ncs;
	NCS_BOOL su_is_external;	/*indicates if this SU is external */

} AVND_SU;

#define AVND_SU_NULL ((AVND_SU *)0)

/***************************************************************************
 ******************  M A C R O   D E F I N I T I O N S  ********************
 ***************************************************************************/

/* macros for su si assignment state */
#define m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_UNASSIGNED(x) \
           ((AVND_SU_SI_ASSIGN_STATE_UNASSIGNED == (x)->curr_assign_state))
#define m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_ASSIGNING(x) \
           ((AVND_SU_SI_ASSIGN_STATE_ASSIGNING == (x)->curr_assign_state))
#define m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_ASSIGNED(x) \
           ((AVND_SU_SI_ASSIGN_STATE_ASSIGNED == (x)->curr_assign_state))
#define m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_REMOVING(x) \
           ((AVND_SU_SI_ASSIGN_STATE_REMOVING == (x)->curr_assign_state))
#define m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_REMOVED(x) \
           ((AVND_SU_SI_ASSIGN_STATE_REMOVED == (x)->curr_assign_state))
#define m_AVND_SU_SI_CURR_ASSIGN_STATE_SET(x, val)  \
           (((x)->curr_assign_state = val))

#define m_AVND_SU_SI_PRV_ASSIGN_STATE_IS_UNASSIGNED(x) \
           ((AVND_SU_SI_ASSIGN_STATE_UNASSIGNED == (x)->prv_assign_state))
#define m_AVND_SU_SI_PRV_ASSIGN_STATE_IS_ASSIGNING(x) \
           ((AVND_SU_SI_ASSIGN_STATE_ASSIGNING == (x)->prv_assign_state))
#define m_AVND_SU_SI_PRV_ASSIGN_STATE_IS_ASSIGNED(x) \
           ((AVND_SU_SI_ASSIGN_STATE_ASSIGNED == (x)->prv_assign_state))
#define m_AVND_SU_SI_PRV_ASSIGN_STATE_IS_REMOVING(x) \
           ((AVND_SU_SI_ASSIGN_STATE_REMOVING == (x)->prv_assign_state))
#define m_AVND_SU_SI_PRV_ASSIGN_STATE_IS_REMOVED(x) \
           ((AVND_SU_SI_ASSIGN_STATE_REMOVED == (x)->prv_assign_state))
#define m_AVND_SU_SI_PRV_ASSIGN_STATE_SET(x, val)  \
           (((x)->prv_assign_state = val))

/* macro to determine if all the SIs that belong to su are assigned */
#define m_AVND_SU_ARE_ALL_SI_ASSIGNED(su, are) \
{ \
   AVND_SU_SI_REC *curr = 0; \
   (are) = TRUE; \
   for (curr = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_FIRST(&su->si_list); \
        curr && m_AVND_SU_SI_CURR_ASSIGN_STATE_IS_ASSIGNED(curr); \
        curr = (AVND_SU_SI_REC *)m_NCS_DBLIST_FIND_NEXT(&curr->su_dll_node)); \
   if (curr) (are) = FALSE; \
}

/* macros for su oper state */
#define m_AVND_SU_OPER_STATE_IS_ENABLED(x) \
                      ((NCS_OPER_STATE_ENABLE == (x)->oper))
#define m_AVND_SU_OPER_STATE_IS_DISABLED(x) \
                      ((NCS_OPER_STATE_DISABLE == (x)->oper))
#define m_AVND_SU_OPER_STATE_SET(x, val)  (((x)->oper= val))

#define m_AVND_SU_OPER_STATE_SET_AND_SEND_NTF(cb, x, val)\
{\
  m_AVND_SU_OPER_STATE_SET(x,val);\
  avnd_gen_su_oper_state_chg_ntf(cb,x);\
}

/* macro to determine if all the comps in an su are operationally enabled */
#define m_AVND_SU_IS_ENABLED(su, is) \
{ \
   AVND_COMP *curr = 0; \
   (is) = TRUE; \
   for (curr = m_AVND_COMP_FROM_SU_DLL_NODE_GET(m_NCS_DBLIST_FIND_FIRST(&su->comp_list)); \
        curr && m_AVND_COMP_OPER_STATE_IS_ENABLED(curr); \
        curr = m_AVND_COMP_FROM_SU_DLL_NODE_GET(m_NCS_DBLIST_FIND_NEXT(&curr->su_dll_node))); \
   if (curr) (is) = FALSE; \
}

/* macro to determine if all the pi comps in an su are instantiated */
#define m_AVND_SU_IS_INSTANTIATED(su, is) \
{ \
   AVND_COMP *curr = 0; \
   (is) = TRUE; \
   for (curr = m_AVND_COMP_FROM_SU_DLL_NODE_GET(m_NCS_DBLIST_FIND_FIRST(&su->comp_list)); \
        curr; \
        curr = m_AVND_COMP_FROM_SU_DLL_NODE_GET(m_NCS_DBLIST_FIND_NEXT(&curr->su_dll_node))) { \
      if ( m_AVND_COMP_TYPE_IS_PREINSTANTIABLE(curr) && \
           !m_AVND_COMP_PRES_STATE_IS_INSTANTIATED(curr) ) { \
         (is) = FALSE; \
         break; \
} \
} \
}

/* macros to manage the presence state */
#define m_AVND_SU_PRES_STATE_SET(x, val)  ((x)->pres = val)
#define m_AVND_SU_PRES_STATE_IS_INSTANTIATED(x) \
           ( NCS_PRES_INSTANTIATED == (x)->pres )
#define m_AVND_SU_PRES_STATE_IS_INSTANTIATING(x) \
           ( NCS_PRES_INSTANTIATING == (x)->pres )
#define m_AVND_SU_PRES_STATE_SET_AND_SEND_NTF(cb, x, val)\
{\
   m_AVND_SU_PRES_STATE_SET(x,val);\
   avnd_gen_su_pres_state_chg_ntf(cb,x);\
}\


/* pre-configured su params */
#define AVND_SU_FLAG_RESTART_DIS  0x00000001
#define AVND_SU_FLAG_PREINSTANTIABLE   0x00000002

/* su state (restart, failed etc.) values */
#define AVND_SU_FLAG_RESTART  0x00000100
#define AVND_SU_FLAG_FAILOVER 0x00000200
#define AVND_SU_FLAG_FAILED   0x00000400
#define AVND_SU_FLAG_ALL_SI   0x00000800
#define AVND_SU_ADMN_TERM     0x00001000	/* set; when a admn term req for an SU is received */
#define AVND_SU_ASSIGN_PEND   0x00002000	/* set; when su si assign starts */
#define AVND_SU_ALL_TERM      0x00004000	/* set; when all resp for term has come for all comps
						   used only in inst failed state */

/* macros for checking the su states */
#define m_AVND_SU_IS_RESTART(x)     (((x)->flag) & AVND_SU_FLAG_RESTART)
#define m_AVND_SU_IS_FAILOVER(x)    (((x)->flag) & AVND_SU_FLAG_FAILOVER)
#define m_AVND_SU_IS_FAILED(x)      (((x)->flag) & AVND_SU_FLAG_FAILED)
#define m_AVND_SU_IS_ALL_SI(x)      (((x)->flag) & AVND_SU_FLAG_ALL_SI)
#define m_AVND_SU_IS_ADMN_TERM(x)   (((x)->flag) & AVND_SU_ADMN_TERM)
#define m_AVND_SU_IS_ASSIGN_PEND(x) (((x)->flag) & AVND_SU_ASSIGN_PEND)
#define m_AVND_SU_IS_ALL_TERM(x)    (((x)->flag) & AVND_SU_ALL_TERM)

/* macros for setting the su states */
#define m_AVND_SU_RESTART_SET(x)       (((x)->flag) |= AVND_SU_FLAG_RESTART)
#define m_AVND_SU_FAILOVER_SET(x)      (((x)->flag) |= AVND_SU_FLAG_FAILOVER)
#define m_AVND_SU_FAILED_SET(x)        (((x)->flag) |= AVND_SU_FLAG_FAILED)
#define m_AVND_SU_ALL_SI_SET(x)        (((x)->flag) |= AVND_SU_FLAG_ALL_SI)
#define m_AVND_SU_ADMN_TERM_SET(x)     (((x)->flag) |= AVND_SU_ADMN_TERM)
#define m_AVND_SU_ASSIGN_PEND_SET(x)   (((x)->flag) |= AVND_SU_ASSIGN_PEND)
#define m_AVND_SU_ALL_TERM_SET(x)      (((x)->flag) |= AVND_SU_ALL_TERM)

/* macros for resetting the su states */
#define m_AVND_SU_RESTART_RESET(x)     (((x)->flag) &= ~AVND_SU_FLAG_RESTART)
#define m_AVND_SU_FAILOVER_RESET(x)    (((x)->flag) &= ~AVND_SU_FLAG_FAILOVER)
#define m_AVND_SU_FAILED_RESET(x)      (((x)->flag) &= ~AVND_SU_FLAG_FAILED)
#define m_AVND_SU_ALL_SI_RESET(x)      (((x)->flag) &= ~AVND_SU_FLAG_ALL_SI)
#define m_AVND_SU_ADMN_TERM_RESET(x)   (((x)->flag) &= ~AVND_SU_ADMN_TERM)
#define m_AVND_SU_ASSIGN_PEND_RESET(x) (((x)->flag) &= ~AVND_SU_ASSIGN_PEND)
#define m_AVND_SU_ALL_TERM_RESET(x)    (((x)->flag) &= ~AVND_SU_ALL_TERM)

/* macros for checking the su params */
#define m_AVND_SU_IS_SU_RESTART_DIS(x)    (((x)->flag) & AVND_SU_FLAG_RESTART_DIS)
#define m_AVND_SU_IS_PREINSTANTIABLE(x)   (((x)->flag) & AVND_SU_FLAG_PREINSTANTIABLE)

/* macros for setting the su params */
#define m_AVND_SU_RESTART_DIS_SET(x)   (((x)->flag) |= AVND_SU_FLAG_RESTART_DIS)
#define m_AVND_SU_PREINSTANTIABLE_SET(x)  (((x)->flag) |= AVND_SU_FLAG_PREINSTANTIABLE)

/* macros for resetting the su params */
#define m_AVND_SU_RESTART_DIS_RESET(x)  (((x)->flag) &= ~AVND_SU_FLAG_RESTART_DIS)
#define m_AVND_SU_PREINSTANTIABLE_RESET(x) (((x)->flag) &= ~AVND_SU_FLAG_PREINSTANTIABLE)

/* 
 * Macros for managing the error escalation levels 
 */
#define m_AVND_SU_ERR_ESC_LEVEL_IS_0(x)  \
           ((x)->su_err_esc_level == AVND_ERR_ESC_LEVEL_0)

#define m_AVND_SU_ERR_ESC_LEVEL_IS_1(x)  \
           ((x)->su_err_esc_level == AVND_ERR_ESC_LEVEL_1)

#define m_AVND_SU_ERR_ESC_LEVEL_IS_2(x)  \
           ((x)->su_err_esc_level == AVND_ERR_ESC_LEVEL_2)

#define m_AVND_SU_ERR_ESC_LEVEL_SET(x, val)  ((x)->su_err_esc_level = (val))

/* macro to get the SU recrod from the SU database */
#define m_AVND_SUDB_REC_GET(sudb, name_net) \
           (AVND_SU *)ncs_patricia_tree_get(&(sudb), (uns8 *)&(name_net))

/* macro to get the next SU recrod from the SU database */
#define m_AVND_SUDB_REC_GET_NEXT(sudb, name_net) \
           (AVND_SU *)ncs_patricia_tree_getnext(&(sudb), (uns8 *)&(name_net))

/* macro to add a component to the su-comp list */
#define m_AVND_SUDB_REC_COMP_ADD(su, comp, rc) \
{ \
   (comp).su_dll_node.key = (uns8 *)&((comp).inst_level); \
   (rc) = ncs_db_link_list_add(&(su).comp_list, &(comp).su_dll_node); \
};

/* macro to remove a component from the su-comp list */
#define m_AVND_SUDB_REC_COMP_REM(su, comp) \
           ncs_db_link_list_delink(&(su).comp_list, &(comp).su_dll_node)

/* macro to add a si record to the su-si list */
#define m_AVND_SUDB_REC_SI_ADD(su, si, rc) \
{ \
   (si).su_dll_node.key = (uns8 *)&(si).name_net; \
   (rc) = ncs_db_link_list_add(&(su).si_list, &(si).su_dll_node); \
};

/* macro to remove a si record from the su-si list */
#define m_AVND_SUDB_REC_SI_REM(su, si) \
           ncs_db_link_list_delink(&(su).si_list, &(si).su_dll_node)

/***************************************************************************
 ******  E X T E R N A L   F U N C T I O N   D E C L A R A T I O N S  ******
 ***************************************************************************/

struct avnd_cb_tag;

EXTERN_C uns32 avnd_su_pres_fsm_run(struct avnd_cb_tag *, AVND_SU *, AVND_COMP *, AVND_SU_PRES_FSM_EV);

EXTERN_C uns32 avnd_sudb_init(struct avnd_cb_tag *);
EXTERN_C uns32 avnd_sudb_destroy(struct avnd_cb_tag *);
EXTERN_C AVND_SU *avnd_sudb_rec_add(struct avnd_cb_tag *, AVND_SU_PARAM *, uns32 *);
EXTERN_C uns32 avnd_sudb_rec_del(struct avnd_cb_tag *, SaNameT *);

EXTERN_C AVND_SU_SI_REC *avnd_su_si_rec_modify(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_PARAM *, uns32 *);
EXTERN_C uns32 avnd_su_si_all_modify(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_PARAM *);
EXTERN_C AVND_SU_SI_REC *avnd_su_si_rec_add(struct avnd_cb_tag *cb, AVND_SU *su, AVND_SU_SI_PARAM *param, uns32 *rc);
EXTERN_C uns32 avnd_su_si_rec_del(struct avnd_cb_tag *, SaNameT *, SaNameT *);
EXTERN_C uns32 avnd_su_si_del(struct avnd_cb_tag *, SaNameT *);
EXTERN_C AVND_SU_SI_REC *avnd_su_si_rec_get(struct avnd_cb_tag *, SaNameT *, SaNameT *);
EXTERN_C uns32 avnd_su_si_msg_prc(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_PARAM *);

EXTERN_C AVND_SU_SIQ_REC *avnd_su_siq_rec_add(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_PARAM *, uns32 *);
EXTERN_C void avnd_su_siq_rec_del(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SIQ_REC *);
EXTERN_C AVND_SU_SIQ_REC *avnd_su_siq_rec_buf(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_PARAM *);
EXTERN_C uns32 avnd_su_siq_prc(struct avnd_cb_tag *, AVND_SU *);

EXTERN_C uns32 avnd_su_si_assign(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_REC *);
EXTERN_C uns32 avnd_su_si_remove(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_REC *);
EXTERN_C uns32 avnd_su_si_oper_done(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_REC *);

EXTERN_C uns32 avnd_su_si_unmark(struct avnd_cb_tag *, AVND_SU *);
EXTERN_C uns32 avnd_su_si_rec_unmark(struct avnd_cb_tag *, AVND_SU *, AVND_SU_SI_REC *);
EXTERN_C uns32 avnd_su_si_reassign(struct avnd_cb_tag *, AVND_SU *);
EXTERN_C uns32 avnd_su_curr_info_del(struct avnd_cb_tag *, AVND_SU *);

EXTERN_C uns32 ncsssutableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);
EXTERN_C uns32 ncsssutableentry_extract(NCSMIB_PARAM_VAL *param,
					NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 ncsssutableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncsssutableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				     NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32 ncsssutableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				       NCSMIB_SETROW_PARAM_VAL *params,
				       struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);
EXTERN_C uns32 ncsssutableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx);

EXTERN_C void avnd_check_su_shutdown_done(struct avnd_cb_tag *, NCS_BOOL);
EXTERN_C AVND_COMP_CSI_REC *avnd_mbcsv_su_si_csi_rec_add(struct avnd_cb_tag *cb,
							 AVND_SU *su,
							 AVND_SU_SI_REC *si_rec, AVND_COMP_CSI_PARAM *param, uns32 *rc);
EXTERN_C uns32 avnd_mbcsv_su_si_csi_rec_del(struct avnd_cb_tag *cb,
					    AVND_SU *su, AVND_SU_SI_REC *si_rec, AVND_COMP_CSI_REC *csi_rec);

#endif
