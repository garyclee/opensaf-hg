/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2010 The OpenSAF Foundation
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

#ifndef CLMS_IMM_H
#define CLMS_IMM_H

extern SaAisErrorT clms_node_ccb_apply_modify(CcbUtilOperationData_t *opdata);
extern void clms_cluster_update_rattr(CLMS_CLUSTER_INFO *osaf_cluster);
extern void clms_node_update_rattr(CLMS_CLUSTER_NODE * nd);
extern void clms_admin_state_update_rattr(CLMS_CLUSTER_NODE * nd);
extern uns32 clms_chk_sub_start(void);
extern void clms_imm_impl_set(CLMS_CB *cb);
extern CLMS_CLUSTER_NODE * clms_node_new(SaNameT *name,const SaImmAttrValuesT_2 ** attrs);
extern void  clms_send_track(CLMS_CB *cb,CLMS_CLUSTER_NODE *node,SaClmChangeStepT step);
extern uns32 clms_node_dn_chk(SaNameT *objName);
extern SaAisErrorT clms_cluster_config_get(void);
extern  SaAisErrorT clms_node_create_config(void);
extern uns32 clms_cluster_dn_chk(SaNameT *objName);
extern SaClmClusterNotificationT_4 * clms_notbuffer_changes_only(SaClmChangeStepT step);
extern SaClmClusterNotificationT_4 * clms_notbuffer_changes(SaClmChangeStepT step);
extern uns32 clms_node_delete(CLMS_CLUSTER_NODE *nd,int i);
extern uns32 clms_nodedb_lookup(int i);
extern SaAisErrorT clms_node_ccb_comp_modify(CcbUtilOperationData_t *opdata);
extern void clms_lock_timer_exp(int signo, siginfo_t *info, void *context);
extern SaAisErrorT clms_node_ccb_apply_cb(CcbUtilOperationData_t *opdata);
extern CLMS_CLUSTER_NODE * clms_node_get_by_eename(SaNameT *name);
extern uns32 clms_prep_and_send_track(CLMS_CB *cb,CLMS_CLUSTER_NODE * node,
				CLMS_CLIENT_INFO *client,SaClmChangeStepT step, 
				SaClmClusterNotificationT_4 *notify);

#endif


