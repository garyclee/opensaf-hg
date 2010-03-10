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

  This module is the include file for Availability Node Directors processing
  module.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVND_PROC_H
#define AVND_PROC_H

struct avnd_cb_tag;
struct avnd_evt_tag;
struct avnd_comp_tag;
struct avnd_pxied_rec;

typedef uns32 (*AVND_EVT_HDLR) (struct avnd_cb_tag *, struct avnd_evt_tag *);

EXTERN_C void avnd_main_process(void);

EXTERN_C uns32 avnd_evt_avd_node_up_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_reg_su_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_reg_comp_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_comp_proxied_add(struct avnd_cb_tag *, struct avnd_comp_tag *, struct avnd_comp_tag *, NCS_BOOL);;
EXTERN_C uns32 avnd_comp_proxied_del(struct avnd_cb_tag *, struct avnd_comp_tag *, struct avnd_comp_tag *, NCS_BOOL,
				     struct avnd_pxied_rec *);
EXTERN_C uns32 avnd_evt_avd_info_su_si_assign_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_pg_track_act_rsp_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_pg_upd_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_operation_request_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_hb_info_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_su_pres_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_oper_state_resp_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_comp_validation_resp_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_role_change_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);

EXTERN_C uns32 avnd_evt_ava_finalize(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_comp_reg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_comp_unreg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_pm_start(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_pm_stop(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_hc_start(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_hc_stop(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_hc_confirm(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_csi_quiescing_compl(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_ha_get(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_pg_start(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_pg_stop(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_err_rep(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_err_clear(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ava_resp(struct avnd_cb_tag *, struct avnd_evt_tag *);

EXTERN_C uns32 avnd_evt_cla_finalize(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_cla_track_start(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_cla_track_stop(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_cla_node_get(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_cla_node_async_get(struct avnd_cb_tag *, struct avnd_evt_tag *);

EXTERN_C uns32 avnd_evt_tmr_hc(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_cbk_resp(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_snd_hb(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_rcv_msg_rsp(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_clc_comp_reg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_su_err_esc(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_node_err_esc(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_clc_pxied_comp_inst(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_tmr_clc_pxied_comp_reg(struct avnd_cb_tag *, struct avnd_evt_tag *);

extern void avnd_send_node_up_msg(void);
EXTERN_C uns32 avnd_evt_mds_avd_up(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_mds_avd_dn(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_mds_ava_dn(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_mds_cla_dn(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_mds_avnd_up(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_mds_avnd_dn(struct avnd_cb_tag *, struct avnd_evt_tag *);

EXTERN_C uns32 avnd_evt_clc_resp(struct avnd_cb_tag *, struct avnd_evt_tag *);

EXTERN_C uns32 avnd_evt_comp_pres_fsm_ev(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_shutdown_app_su_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_verify_message(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_ack_message(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_clm_node_on_fover(struct avnd_cb_tag *cb, struct avnd_evt_tag *evt);

EXTERN_C uns32 avnd_evt_last_step_term(struct avnd_cb_tag *, struct avnd_evt_tag *);

EXTERN_C uns32 avnd_evt_avd_set_leds_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_destroy(void);
EXTERN_C uns32 avnd_evt_avnd_avnd_msg(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_ha_state_change(struct avnd_cb_tag *, struct avnd_evt_tag *);
EXTERN_C uns32 avnd_evt_avd_admin_op_req_msg (struct avnd_cb_tag *, struct avnd_evt_tag *);
#endif
