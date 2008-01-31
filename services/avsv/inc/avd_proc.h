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

  This module is the  include file for Availability Directors processing module.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_PROC_H
#define AVD_PROC_H


typedef void (*AVD_EVT_HDLR) (AVD_CL_CB *,AVD_EVT *);

EXTERN_C void avd_main_proc(AVD_CL_CB *cb);


EXTERN_C void avd_su_oper_state_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_su_si_assign_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C uns32 avd_new_assgn_susi(AVD_CL_CB *cb,AVD_SU *su, AVD_SI *si,
                                  SaAmfHAStateT role,NCS_BOOL ckpt,
                                  AVD_SU_SI_REL **ret_ptr);
EXTERN_C uns32 avd_sg_app_node_su_inst_func(AVD_CL_CB *cb, AVD_AVND *avnd);
EXTERN_C uns32 avd_sg_app_su_inst_func(AVD_CL_CB *cb, AVD_SG *sg);
EXTERN_C uns32 avd_sg_su_oper_list_add(AVD_CL_CB *cb, AVD_SU *su, NCS_BOOL ckpt);
EXTERN_C uns32 avd_sg_su_oper_list_del(AVD_CL_CB *cb, AVD_SU *su, NCS_BOOL ckpt);
EXTERN_C uns32 avd_sg_app_node_admin_func(AVD_CL_CB *cb, AVD_AVND *avnd, 
                                 NCS_ADMIN_STATE new_admin_state);
EXTERN_C uns32 avd_sg_su_asgn_del_util(AVD_CL_CB *cb, AVD_SU *su, NCS_BOOL del_flag,
                              NCS_BOOL q_flag);
EXTERN_C uns32 avd_sg_app_sg_admin_func(AVD_CL_CB *cb, AVD_SG *sg);
EXTERN_C uns32 avd_sg_su_si_mod_snd(AVD_CL_CB *cb,AVD_SU *su,SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_su_si_del_snd(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C void avd_ncs_su_mod_rsp(AVD_CL_CB *cb,AVD_AVND *avnd, 
                           AVSV_N2D_INFO_SU_SI_ASSIGN_MSG_INFO *assign); 


/* The following are for 2N redundancy model */
EXTERN_C uns32 avd_sg_2n_si_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_2n_su_insvc_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_2n_suswitch_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_2n_siswitch_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_2n_su_fault_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_2n_susi_sucss_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_2n_susi_fail_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_2n_realign_func(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C uns32 avd_sg_2n_su_admin_fail(AVD_CL_CB *cb,AVD_SU *su, AVD_AVND *avnd);
EXTERN_C uns32 avd_sg_2n_si_admin_down(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_2n_sg_admin_down(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C void avd_sg_2n_node_fail_func(AVD_CL_CB *cb,AVD_SU *su);

/* The following are for N-Way redundancy model */
EXTERN_C uns32 avd_sg_nway_si_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_nway_su_insvc_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_nway_siswitch_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_nway_su_fault_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_nway_susi_sucss_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_nway_susi_fail_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_nway_realign_func(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C uns32 avd_sg_nway_su_admin_fail(AVD_CL_CB *cb,AVD_SU *su, AVD_AVND *avnd);
EXTERN_C uns32 avd_sg_nway_si_admin_down(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_nway_sg_admin_down(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C void avd_sg_nway_node_fail_func(AVD_CL_CB *cb,AVD_SU *su);


/* The following are for N+M redundancy model */
EXTERN_C uns32 avd_sg_npm_si_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_npm_su_insvc_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_npm_siswitch_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_npm_su_fault_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_npm_susi_sucss_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_npm_susi_fail_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_npm_realign_func(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C uns32 avd_sg_npm_su_admin_fail(AVD_CL_CB *cb,AVD_SU *su, AVD_AVND *avnd);
EXTERN_C uns32 avd_sg_npm_si_admin_down(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_npm_sg_admin_down(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C void avd_sg_npm_node_fail_func(AVD_CL_CB *cb,AVD_SU *su);

/* The following are for No redundancy model */
EXTERN_C uns32 avd_sg_nored_si_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_nored_su_insvc_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_nored_su_fault_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_nored_susi_sucss_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_nored_susi_fail_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_nored_realign_func(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C uns32 avd_sg_nored_su_admin_fail(AVD_CL_CB *cb,AVD_SU *su, AVD_AVND *avnd);
EXTERN_C uns32 avd_sg_nored_si_admin_down(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_nored_sg_admin_down(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C void avd_sg_nored_node_fail_func(AVD_CL_CB *cb,AVD_SU *su);


/* The following are for N-way Active redundancy model */
EXTERN_C uns32 avd_sg_nacvred_si_func(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_nacvred_su_insvc_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_nacvred_su_fault_func(AVD_CL_CB *cb,AVD_SU *su);
EXTERN_C uns32 avd_sg_nacvred_susi_sucss_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_nacvred_susi_fail_func(AVD_CL_CB *cb,AVD_SU *su,AVD_SU_SI_REL *susi,
                                AVSV_SUSI_ACT act, SaAmfHAStateT state);
EXTERN_C uns32 avd_sg_nacvred_realign_func(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C uns32 avd_sg_nacvred_su_admin_fail(AVD_CL_CB *cb,AVD_SU *su, AVD_AVND *avnd);
EXTERN_C uns32 avd_sg_nacvred_si_admin_down(AVD_CL_CB *cb,AVD_SI *si);
EXTERN_C uns32 avd_sg_nacvred_sg_admin_down(AVD_CL_CB *cb,AVD_SG *sg);
EXTERN_C void avd_sg_nacvred_node_fail_func(AVD_CL_CB *cb,AVD_SU *su);


EXTERN_C void avd_node_up_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_reg_hlth_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_reg_su_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_reg_comp_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_nd_heartbeat_msg_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_tmr_rcv_hb_nd_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_oper_req_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_mds_avnd_up_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_ack_nack_event(AVD_CL_CB *cb, AVD_EVT *evt);
EXTERN_C void avd_fail_over_event(AVD_CL_CB *cb);
EXTERN_C void avd_mds_avnd_down_func(AVD_CL_CB *cb,struct avd_evt_tag *evt);
EXTERN_C void avd_data_update_req_func(AVD_CL_CB *cb,AVD_EVT *evt);
EXTERN_C void avd_shutdown_app_su_resp_func(AVD_CL_CB *cb,AVD_EVT *evt);
EXTERN_C void avd_role_switch_ncs_su(AVD_CL_CB *cb,AVD_EVT *evt);
EXTERN_C void avd_mds_qsd_role_func(AVD_CL_CB *cb,AVD_EVT *evt);
EXTERN_C void avd_node_susi_fail_func(AVD_CL_CB *cb,AVD_AVND *avnd);
EXTERN_C void avd_node_down_func(AVD_CL_CB *cb,AVD_AVND *avnd);
EXTERN_C uns32 avd_node_down(AVD_CL_CB *cb,SaClmNodeIdT node_id);
EXTERN_C AVD_AVND  * avd_msg_sanity_chk(AVD_CL_CB *cb, AVD_EVT *evt,
                     SaClmNodeIdT node_id, AVSV_DND_MSG_TYPE msg_typ);
EXTERN_C void avd_nd_reg_comp_evt_hdl(AVD_CL_CB *cb,AVD_AVND *avnd);
EXTERN_C void avd_nd_ncs_su_assigned(AVD_CL_CB *cb,AVD_AVND *avnd);
EXTERN_C void avd_nd_ncs_su_failed(AVD_CL_CB *cb,AVD_AVND *avnd);
EXTERN_C void avd_rcv_hb_d_msg(AVD_CL_CB *cb, struct avd_evt_tag *evt);
EXTERN_C void avd_hb_init_proc(uns32 *);
EXTERN_C void avd_process_hb_event(AVD_CL_CB *cb_now, struct avd_evt_tag *evt);
#endif
