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
 
  DESCRIPTION:This file contains function declarations for all AvM utility 
  functions
  ..............................................................................

 
******************************************************************************
*/

#ifndef __AVM_UTIL_H__
#define __AVM_UTIL_H__

extern uns32 avm_proc(void);

extern uns32 avm_rda_initialize(AVM_CB_T *cb);

extern uns32 avm_eda_initialize(AVM_CB_T *cb);
extern uns32 avm_eda_finalize(AVM_CB_T *cb);

extern uns32 avm_mab_init(AVM_CB_T *cb);
extern uns32 avm_mab_destroy(AVM_CB_T *cb);
extern uns32 avm_miblib_init(AVM_CB_T *cb);
extern uns32 avm_mab_send_warmboot_req(AVM_CB_T *cb);

extern uns32 avm_msg_handler(AVM_CB_T *avm_cb, AVM_EVT_T *avm_evt);
extern uns32 avm_fsm_handler(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, AVM_EVT_T *fsm_evt);

extern uns32 avm_map_hpi2fsm(AVM_CB_T *cb, HPI_EVT_T *evt, AVM_FSM_EVT_TYPE_T *fsm_evt_type, AVM_ENT_INFO_T *ent_info);
extern uns32
avm_map_avd2fsm(AVM_EVT_T *avd_avm, AVM_AVD_MSG_TYPE_T msg_type, AVM_FSM_EVT_TYPE_T *fsm_evt_type, SaNameT *node_name);
extern uns32 avm_find_chassis_id(SaHpiEntityPathT *entity_path, uns32 *chassis_id);

extern uns32 avm_convert_entity_path_to_string(SaHpiEntityPathT entity_path, uns8 **entity_path_str);
extern uns32 avm_refine_fault_domain(AVM_ENT_INFO_T *ent_info, AVM_FAULT_DOMAIN_T **fault_domain);
extern uns32 avm_scope_extract_criteria(AVM_ENT_INFO_T *ent_info, uns32 **child_impact);

extern uns32 avm_scope_reset_criteria(AVM_ENT_INFO_T *ent_info, uns32 **child_impact);

extern uns32 avm_scope_sensor_assert_criteria(AVM_ENT_INFO_T *ent_info, uns32 **child_impact);

extern uns32 avm_scope_sensor_deassert_criteria(AVM_ENT_INFO_T *ent_info, uns32 **child_impact);

extern uns32 avm_scope_surp_extract_criteria(AVM_ENT_INFO_T *ent_info, uns32 **child_impact);

extern uns32 avm_scope_active_criteria(AVM_CB_T *active, AVM_ENT_INFO_T *ent_info, uns32 **child_impact);

extern uns32 avm_scope_fault_domain_req_criteria(AVM_ENT_INFO_T *ent_info, uns32 **child_impact);

extern uns32
avm_scope_fault_criteria(AVM_CB_T *avm_cb,
			 AVM_ENT_INFO_T *ent_info, AVM_SCOPE_EVENTS_T fault_type, uns32 *child_impact);

extern uns32
avm_scope_fault_children(AVM_CB_T *avm_cb,
			 AVM_ENT_INFO_T *ent_info,
			 AVM_LIST_HEAD_T *head, AVM_LIST_NODE_T *fault_node, AVM_SCOPE_EVENTS_T fault_type);

extern uns32
avm_handle_fault(AVM_CB_T *avm_cb, AVM_LIST_HEAD_T *head, AVM_ENT_INFO_T *ent_info, AVM_SCOPE_EVENTS_T fault_type);

extern uns32 avm_scope_fault(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, AVM_SCOPE_EVENTS_T fault_type);

extern uns32 avm_handle_ignore_event(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt);

extern uns32 avm_handle_invalid_event(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt);

extern uns32 avm_handle_avd_error(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *en_info, AVM_EVT_T *evt);

extern void avm_sensor_event_initialize(AVM_ENT_INFO_T *ent_info, uns32 n);

extern uns32 avm_reset_ent_info(AVM_ENT_INFO_T *ent_info);

extern uns32 avm_add_dependent_list(AVM_ENT_INFO_T *ent_info, AVM_ENT_INFO_T *dep_ent_info);

extern uns32 avm_hisv_api_cmd(AVM_ENT_INFO_T *ent_info, HISV_API_CMD api_cmd, uns32 arg);

EXTERN_C uns32 avm_get_hpi_hs_state(AVM_ENT_INFO_T *, uns32 *);

extern uns32 avm_bld_validation_info(AVM_CB_T *cb, NCS_HW_ENT_TYPE_DESC *ent_desc_info);

extern uns32 avm_add_dependent(AVM_ENT_INFO_T *dependent, AVM_ENT_INFO_T *dependee);
extern uns32 avm_remove_dependent(AVM_ENT_INFO_T *dependee);
extern uns32 avm_bld_valid_info_parent_child_relation(AVM_CB_T *cb);

extern uns32 avm_check_dynamic_ent(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info);

extern uns32 avm_check_deployment(AVM_CB_T *cb);

extern uns32 avm_add_root(AVM_CB_T *cb);

extern uns32 avm_active(AVM_CB_T *, AVM_ENT_INFO_T *);

extern uns32 avm_send_role_chg_notify_msg(AVM_CB_T *cb, SaAmfHAStateT role);

extern uns32 avm_send_role_rsp_notify_msg(AVM_CB_T *cb, SaAmfHAStateT role, uns32 status);
extern uns32 avm_notify_rde_hrt_bt_lost(AVM_CB_T *cb);

extern uns32 avm_notify_rde_hrt_bt_restore(AVM_CB_T *cb);

extern uns32 avm_notify_rde_nd_hrt_bt_lost(AVM_CB_T *, uns32);

extern uns32 avm_notify_rde_nd_hrt_bt_restore(AVM_CB_T *, uns32);

extern uns32 avm_notify_rde_set_role(AVM_CB_T *cb, SaAmfHAStateT role);

extern uns32 avm_add_dependent(AVM_ENT_INFO_T *dependent, AVM_ENT_INFO_T *dependee);
extern uns32 avm_remove_dependents(AVM_ENT_INFO_T *dependee);
extern uns32 avm_update_sensors(AVM_ENT_INFO_T *dest, AVM_ENT_INFO_T *src);
extern uns32 avm_rmv_ent_info(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);

extern uns32 avm_ssu_dhconf(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, void *fsm_evt, uns8 dhcp_restart);

extern uns32
avm_ssu_dhconf_set(AVM_CB_T *avm_cb, AVM_ENT_DHCP_CONF *dhcp_conf, AVM_PER_LABEL_CONF *dhcp_label, uns8 dhcp_restart);

extern uns32 avm_ssu_dhcp_tmr_reconfig(AVM_CB_T *avm_cb, AVM_EVT_T *hpi_evt);

extern uns32 avm_send_boot_upgd_trap(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, uns32 trap_id);

extern uns32 avm_open_trap_chanel(AVM_CB_T *avm_cb);

extern uns32 avm_register_rda_cb(AVM_CB_T *cb);
extern uns32 avm_unregister_rda_cb(AVM_CB_T *cb);
extern uns32 avm_deact_dependents(AVM_CB_T *cb, AVM_ENT_INFO_T *dependee);

extern SaHpiHsStateT avm_map_hs_to_hpi_hs(AVM_FSM_STATES_T hs_state);

EXTERN_C uns32 avm_lfm_msg_handler(AVM_CB_T *, AVM_EVT_T *);
EXTERN_C uns32 avm_send_lfm_nd_reset_ind(AVM_CB_T *, SaHpiEntityPathT *);

extern uns32 avm_adm_switch_trap(AVM_CB_T *avm_cb, uns32 trap_id, AVM_SWITCH_STATUS_T param_val);
extern uns32 avm_entity_locked_trap(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);
extern uns32 avm_entity_inactive_hisv_ret_error_trap(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);

extern uns32 avm_open_trap_channel(AVM_CB_T *avm_cb);

extern uns32 avm_push_admin_mib_set_to_psr(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, AVM_ADM_OP_T adm_state);

extern
uns32 avm_standby_boot_succ_tmr_handler(AVM_CB_T *avm_cb, AVM_EVT_T *hpi_evt,
					AVM_ENT_INFO_T *ent_info, AVM_FSM_EVT_TYPE_T fsm_evt_type);

extern uns32
avm_standby_map_hpi2fsm(AVM_CB_T *cb, HPI_EVT_T *evt, AVM_FSM_EVT_TYPE_T *fsm_evt_type, AVM_ENT_INFO_T *ent_info);

/* FM specific function prototypes */
extern uns32 avm_fma_initialize(AVM_CB_T *cb);

extern uns32 avm_notify_fm_hb_evt(AVM_CB_T *cb, SaNameT node_name, AVM_ROLE_FSM_EVT_TYPE_T evt_type);

extern NCS_BOOL avm_fm_can_switchover_proceed(AVM_CB_T *cb);

extern uns32 avm_notify_fm_node_reset(AVM_CB_T *avm_cb, SaHpiEntityPathT *ent_path);
extern NCS_BOOL avm_is_this_entity_self(AVM_CB_T *avm_cb, SaHpiEntityPathT ep);

extern void
avm_conv_phy_info_to_ent_path(NCS_CHASSIS_ID chassis_id, NCS_PHY_SLOT_ID phy_slot_id, NCS_SUB_SLOT_ID subslot_id,
			      SaHpiEntityPathT *ep);

extern NCS_BOOL avm_compare_ent_paths(SaHpiEntityPathT ent_path1, SaHpiEntityPathT ent_path2);

#endif   /* __AVM_UTIL_H__ */
