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

#ifndef CPD_PROC_H
#define CPD_PROC_H

/* CPD definations */
#define CPD_CPND_DOWN_RETENTION_TIME 600	/* 100 Milli Sec */
#define NCS_SAF_ACCEPT_TIME 1000
/* The count of non colloc replicas created by CPSv (Policy) */
#define CPD_NON_COLLOC_CREATED_REPLICA_CNT 1

#define CPD_MBCSV_MAX_MSG_CNT  10

/* Event Handler */
void cpd_process_evt(CPSV_EVT *evt);

uint32_t cpd_amf_init(CPD_CB *cpd_cb);
void cpd_amf_de_init(CPD_CB *cpd_cb);
uint32_t cpd_amf_register(CPD_CB *cpd_cb);
uint32_t cpd_amf_deregister(CPD_CB *cpd_cb);

/* AMF Function Declerations */
void
cpd_amf_csi_rmv_callback(SaInvocationT invocation,
			 const SaNameT *compName, const SaNameT *csiName, SaAmfCSIFlagsT csiFlags);
void cpd_amf_comp_terminate_callback(SaInvocationT invocation, const SaNameT *compName);

void cpd_saf_csi_set_cb(SaInvocationT invocation,
				 const SaNameT *compName, SaAmfHAStateT haState, SaAmfCSIDescriptorT csiDescriptor);

void cpd_saf_hlth_chk_cb(SaInvocationT invocation, const SaNameT *compName, SaAmfHealthcheckKeyT *checkType);

uint32_t cpd_ckpt_db_entry_update(CPD_CB *cb,
					MDS_DEST *cpnd_dest,
					CPSV_ND2D_CKPT_CREATE *ckpt_create,
					CPD_CKPT_INFO_NODE **o_ckpt_node, CPD_CKPT_MAP_INFO **io_map_info);

uint32_t cpd_process_ckpt_delete(CPD_CB *cb,
				       CPD_CKPT_INFO_NODE *ckpt_node,
				       CPSV_SEND_INFO *sinfo,
				       NCS_BOOL *o_ckpt_node_deleted, NCS_BOOL *o_is_active_changed);

uint32_t cpd_noncolloc_ckpt_rep_create(CPD_CB *cb,
					     MDS_DEST *cpnd_dest,
					     CPD_CKPT_INFO_NODE *ckpt_node, CPD_CKPT_MAP_INFO *map_info);
uint32_t cpd_noncolloc_ckpt_rep_delete(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node, CPD_CKPT_MAP_INFO *map_info);

uint32_t cpd_process_cpnd_down(CPD_CB *cb, MDS_DEST *cpnd_dest);

uint32_t cpd_proc_active_set(CPD_CB *cb, SaCkptCheckpointHandleT ckpt_id,
				   MDS_DEST mds_dest, CPD_CKPT_INFO_NODE **ckpt_node);

uint32_t cpd_proc_retention_set(CPD_CB *cb, SaCkptCheckpointHandleT ckpt_id,
				      SaTimeT reten_time, CPD_CKPT_INFO_NODE **ckpt_node);

uint32_t cpd_proc_unlink_set(CPD_CB *cb, CPD_CKPT_INFO_NODE **ckpt_node,
				   CPD_CKPT_MAP_INFO *map_info, SaNameT *ckpt_name);

void cpd_cb_dump(void);

uint32_t cpd_mbcsv_chgrole(CPD_CB *cb);

void cpd_ckpt_reflist_del(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node, CPSV_CPND_DEST_INFO *dest_list);
uint32_t cpd_mbcsv_enc_msg_resp(CPD_CB *cb, NCS_MBCSV_CB_ARG *arg);
void cpd_a2s_ckpt_unlink_set(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node);
void cpd_a2s_ckpt_rdset(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node);
void cpd_a2s_ckpt_arep_set(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node);
void cpd_a2s_ckpt_dest_add(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node, MDS_DEST *sinfo);
void cpd_a2s_ckpt_dest_del(CPD_CB *cb, SaCkptCheckpointHandleT ckpt_hdl, MDS_DEST *cpnd_dest,
				    NCS_BOOL ckptid_flag);
void cpd_a2s_ckpt_usr_info(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node);
void cpd_a2s_ckpt_dest_down(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node, MDS_DEST *sinfo);
uint32_t cpd_mbcsv_dec_async_update(CPD_CB *cb, NCS_MBCSV_CB_ARG *arg);
uint32_t cpd_mbcsv_dec_warm_sync_resp(CPD_CB *cb, NCS_MBCSV_CB_ARG *arg);
uint32_t cpd_mbcsv_decode_proc(NCS_MBCSV_CB_ARG *arg);
uint32_t cpd_mbcsv_enc_warm_sync_resp(CPD_CB *cb, NCS_MBCSV_CB_ARG *arg);
uint32_t cpd_mbcsv_encode_proc(NCS_MBCSV_CB_ARG *arg);
uint32_t cpd_mbcsv_dec_sync_resp(CPD_CB *cb, NCS_MBCSV_CB_ARG *arg);
uint32_t cpd_process_cpnd_del(CPD_CB *cb, MDS_DEST *cpnd_dest);
uint32_t cpd_get_slot_sub_id_from_mds_dest(MDS_DEST dest);
uint32_t cpd_get_slot_sub_slot_id_from_node_id(NCS_NODE_ID i_node_id);
uint32_t cpd_mbcsv_register(CPD_CB *cb);
uint32_t cpd_mbcsv_finalize(CPD_CB *cb);
uint32_t cpd_mbcsv_enc_async_update(CPD_CB *cb, NCS_MBCSV_CB_ARG *arg);
uint32_t cpd_mbcsv_close(CPD_CB *cb);
NCS_BOOL cpd_is_noncollocated_replica_present_on_payload(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node);
uint32_t cpd_ckpt_reploc_imm_object_delete(CPD_CB *cb,  CPD_CKPT_REPLOC_INFO *ckpt_reploc_node ,NCS_BOOL is_unlink_set);
#endif
