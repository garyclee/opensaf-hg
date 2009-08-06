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

  This module is the include file for Availability Modules checkpointing.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVM_CKPT_UPDT_H
#define AVM_CKPT_UPDT_H

/* Function Definitions of avm_ckpt_updt.c */

extern uns32 avm_ckpt_add_rmv_updt_ent(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info, NCS_MBCSV_ACT_TYPE action);
extern uns32 avm_ckpt_update_valid_info_db(AVM_CB_T *cb, AVM_VALID_INFO_T *valid_info, NCS_MBCSV_ACT_TYPE action);

extern uns32
avm_ckpt_update_ent_db(AVM_CB_T *cb,
		       AVM_ENT_INFO_T *ent_info, NCS_MBCSV_ACT_TYPE action, AVM_ENT_INFO_T **temp_ent_info);

extern uns32 avm_updt_evt_q(AVM_CB_T *cb, AVM_EVT_T *hpi_evt, uns32 event_id, NCS_BOOL bool);

extern uns32 avm_proc_evt_q(AVM_CB_T *cb);

extern uns32 avm_empty_evt_q(AVM_CB_T *cb);

extern uns32 avm_cleanup_db(AVM_CB_T *cb);

extern uns32 avm_dhcp_conf_per_label_update(AVM_PER_LABEL_CONF *dhcp_label, AVM_PER_LABEL_CONF *temp_label);
extern uns32 avm_decode_ckpt_dhcp_conf_update(AVM_ENT_DHCP_CONF *dhcp_conf, AVM_ENT_DHCP_CONF *temp_conf);

extern uns32 avm_decode_ckpt_dhcp_state_update(AVM_ENT_DHCP_CONF *dhcp_conf, AVM_ENT_DHCP_CONF *temp_conf);

extern uns32 avm_decode_ckpt_upgd_state_update(AVM_ENT_DHCP_CONF *dhcp_conf, AVM_ENT_DHCP_CONF *temp_conf);

#endif
