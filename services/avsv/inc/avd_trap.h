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
  MODULE NAME: AVD_TRAP.H

..............................................................................

  DESCRIPTION:   This file contains avd traps related definitions.

******************************************************************************/

#ifndef AVD_TRAP_H
#define AVD_TRAP_H

#define AVD_EDA_EVT_PUBLISHER_NAME "AVD"

EXTERN_C uns32 avd_amf_alarm_service_impaired_trap(AVD_CL_CB *avd_cb,
                                   SaAisErrorT err);

EXTERN_C uns32 avd_gen_cluster_reset_trap(AVD_CL_CB *avd_cb,
                                   AVD_COMP *comp);

EXTERN_C uns32 avd_gen_node_admin_state_changed_trap(AVD_CL_CB *avd_cb,
                                   AVD_AVND *node);

EXTERN_C uns32 avd_gen_sg_admin_state_changed_trap(AVD_CL_CB *avd_cb,
                                   AVD_SG *sg);

EXTERN_C uns32 avd_gen_su_admin_state_changed_trap(AVD_CL_CB *avd_cb,
                                   AVD_SU *su);

EXTERN_C uns32 avd_gen_si_unassigned_trap(AVD_CL_CB *avd_cb,
                                   AVD_SI *si);

EXTERN_C uns32 avd_gen_si_oper_state_chg_trap(AVD_CL_CB *avd_cb,
                                   AVD_SI *si);

EXTERN_C uns32 avd_gen_si_admin_state_chg_trap(AVD_CL_CB *avd_cb,
                                   AVD_SI *si);

EXTERN_C uns32 avd_populate_si_state_traps(AVD_CL_CB *avd_cb,
                                   AVD_SI *si,
                                   NCS_TRAP *trap,
                                   NCS_BOOL isOper);

EXTERN_C uns32 avd_gen_su_ha_state_changed_trap(AVD_CL_CB *avd_cb,
                                   AVD_SU_SI_REL *susi);

EXTERN_C uns32 avd_gen_su_si_assigned_trap(AVD_CL_CB *avd_cb,
                                   AVD_SU_SI_REL *susi);

EXTERN_C uns32 avd_fill_and_send_su_si_ha_state_trap(AVD_CL_CB *avd_cb,
                                   AVD_SU_SI_REL *susi, 
                                   NCS_BOOL isAmfmibTrap);

EXTERN_C uns32 avd_clm_alarm_service_impaired_trap(AVD_CL_CB *avd_cb,
                                   SaAisErrorT err);

EXTERN_C uns32 avd_clm_node_join_trap(AVD_CL_CB *avd_cb,
                                   AVD_AVND *node);

EXTERN_C uns32 avd_clm_node_exit_trap(AVD_CL_CB *avd_cb,
                                   AVD_AVND *node);

EXTERN_C uns32 avd_clm_node_reconfiured_trap(AVD_CL_CB *avd_cb,
                                   AVD_AVND *node);

EXTERN_C uns32 avd_populate_clm_node_traps(AVD_CL_CB *avd_cb,
                                   AVD_AVND *node,
                                   NCS_TRAP *trap);

EXTERN_C uns32 avd_gen_ncs_init_success_trap(AVD_CL_CB *avd_cb,
                                   AVD_AVND *node); 

EXTERN_C uns32 avd_create_and_send_trap(AVD_CL_CB *avd_cb,
                                   NCS_TRAP *trap,
                                   SaEvtEventPatternT *pattern); 

EXTERN_C uns32 avd_eda_initialize(AVD_CL_CB  *avd_cb);

EXTERN_C uns32  avd_eda_finalize(AVD_CL_CB *avd_cb);



#endif
